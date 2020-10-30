// vJoyClient.cpp : Simple feeder application with a FFB demo

#include "stdafx.h"
//#include "Devioctl.h"
#include "public.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "vjoyinterface.h"
#include "Math.h"

#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <fstream>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
//#include <zmq.h>
//#include "zmq_helpers.h"
#include <cassert>
#include <paho-mqtt/MQTTClient.h>


// Default device ID (Used when ID not specified)
#define DEV_ID		1

// MQTT Defines

#define ADDRESS     "tcp://mrzpfs1b9tj1n.messaging.solace.cloud:20550"
#define CLIENTID    "solace-cloud-client"
#define TOPIC       "joystick_axis"
#define TOPIC_LEN   14
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     100000L

// Prototypes
void  CALLBACK FfbFunction(PVOID data);
void  CALLBACK FfbFunction1(PVOID cb, PVOID data);

BOOL PacketType2Str(FFBPType Type, LPTSTR Str);
BOOL EffectType2Str(FFBEType Ctrl, LPTSTR Str);
BOOL DevCtrl2Str(FFB_CTRL Type, LPTSTR Str);
BOOL EffectOpStr(FFBOP Op, LPTSTR Str);
int  Polar2Deg(BYTE Polar);
int  Byte2Percent(BYTE InByte);
int TwosCompByte2Int(BYTE in);


int ffb_direction = 0;
int ffb_strenght = 0;
int serial_result = 0;


JOYSTICK_POSITION_V2 iReport; // The structure that holds the full position data

char string[256]; //buffer to hold incoming messages

bool recv_ready = false;



volatile MQTTClient_deliveryToken deliveredtoken;

void parse_inputs(char* string, int* x, int* y, int* z, int* t, int* buttons)
{
	float xf=0, yf=0, zf=0, tf=0;
	sscanf(string, "%f %f %f %f %d", &xf, &yf, &zf, &tf, buttons);
	int OldMax = 45;
	int	OldMin = -45;
	int	NewMax = 32766;
	int	NewMin = 0; 
	int OldRange = (OldMax - OldMin);
	int NewRange = (NewMax - NewMin);
	*x = (((xf - OldMin) * NewRange) / OldRange) + NewMin;
	*y = (((yf - OldMin) * NewRange) / OldRange) + NewMin;
	*z = (((zf - OldMin) * NewRange) / OldRange) + NewMin;
	*t = (((tf - OldMin) * NewRange) / OldRange) + NewMin;
}

void delivered(void* context, MQTTClient_deliveryToken dt)
{
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}
int msgarrvd(void* context, char* topicName, int topicLen, MQTTClient_message* message)
{
	int i;
	char* payloadptr;
	/*printf("Message arrived\n");
	printf("     topic: %s\n", topicName);
	*/printf("   message: ");
	payloadptr = (char*)message->payload;
	if (message->payloadlen > 255)
	{
		printf("Payload length exceeded maximum\n");
	}
	else
	{
		memset(string, 0, 256);
		strncpy(string, payloadptr, message->payloadlen);
		printf("%s\n", string);
	}
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	recv_ready = true;
	return 1;
}
void connlost(void* context, char* cause)
{
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}





int
__cdecl
_tmain(int argc, _TCHAR* argv[])
{
	int stat = 0;
	UINT DevID = DEV_ID;
	USHORT X = 0;
	USHORT Y = 0;
	USHORT Z = 0;
	LONG   Btns = 0;
	

	PVOID pPositionMessage;
	UINT	IoCode = LOAD_POSITIONS;
	UINT	IoSize = sizeof(JOYSTICK_POSITION);
	// HID_DEVICE_ATTRIBUTES attrib;
	BYTE id = 1;
	UINT iInterface = 1;

	// Set the target Joystick - get it from the command-line 
	if (argc>1)
		DevID = _tstoi(argv[1]);

	// Get the driver attributes (Vendor ID, Product ID, Version Number)
	if (!vJoyEnabled())
	{
		_tprintf("Function vJoyEnabled Failed - make sure that vJoy is installed and enabled\n");
		int dummy = getchar();
		stat = - 2;
		goto Exit;
	}
	else
	{
		printf("Starting vJoy Client\n");
		//wprintf(L"Vendor: %s\nProduct :%s\nVersion Number:%s\n", static_cast<TCHAR *> (GetvJoyManufacturerString()), static_cast<TCHAR *>(GetvJoyProductString()), static_cast<TCHAR *>(GetvJoySerialNumberString()));
	};

	// Get the status of the vJoy device before trying to acquire it
	VjdStat status = GetVJDStatus(DevID);

	switch (status)
	{
	case VJD_STAT_OWN:
		_tprintf("vJoy device %d is already owned by this feeder\n", DevID);
		break;
	case VJD_STAT_FREE:
		_tprintf("vJoy device %d is free\n", DevID);
		break;
	case VJD_STAT_BUSY:
		_tprintf("vJoy device %d is already owned by another feeder\nCannot continue\n", DevID);
		return -3;
	case VJD_STAT_MISS:
		_tprintf("vJoy device %d is not installed or disabled\nCannot continue\n", DevID);
		return -4;
	default:
		_tprintf("vJoy device %d general error\nCannot continue\n", DevID);
		return -1;
	};

	// Acquire the vJoy device
	if (!AcquireVJD(DevID))
	{
		_tprintf("Failed to acquire vJoy device number %d.\n", DevID);
		int dummy = getchar();
		stat = -1;
		goto Exit;
	}
	else
	{
		_tprintf("Acquired device number %d - OK\n", DevID);
	}

	// Connect to MQTT
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_create(&client, ADDRESS, CLIENTID,
		MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = "solace-cloud-client";
	conn_opts.password = "p2i7li6ckbaimoe0draq0qdl82";
	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
	int rcc;
	if ((rcc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rcc);
		exit(-1);
	}
	else
	{
		printf("\nMQTT Client Connected Successfully\n");
	}
	MQTTClient_subscribe(client, TOPIC, QOS);


	// Set destination vJoy device
	id = (BYTE)DevID;
	iReport.bDevice = id;

	int buttons_last = 0;

	int x, y, z, t, buttons;

	while(1) {
		/*char* string = s_recv(subscriber);*/
		while (!recv_ready)
		{
			Sleep(0);
		}

		parse_inputs(string, &x, &y, &z, &t, &buttons);

		//printf("%s\n", string);

		//free(string);
		if (x < 0 && y < 0 && z < 0 && t < 0)
		{

			//zmq_close(subscriber);
			//zmq_ctx_destroy(context);
			goto Exit;
		}
		iReport.wAxisX = x;
		iReport.wAxisY = y;
		iReport.wAxisXRot = z;
		iReport.wAxisZ = t;

		Btns = buttons_last ^ (buttons & ~0x07); //mask out first 3 bits
		buttons_last = buttons;

		// Set position data of first 2 buttons
		iReport.lButtons = Btns;

		// Send position data to vJoy device
		pPositionMessage = (PVOID)(&iReport);
		if (!UpdateVJD(DevID, pPositionMessage))
		{
			printf("Feeding vJoy device number %d failed - try to enable device then press enter\n", DevID);
			getchar();
			AcquireVJD(DevID);
		}
		else
		{
			printf("Updated vJoy device with values %d %d %d %d %d\n", iReport.wAxisX, iReport.wAxisY, iReport.wAxisXRot, iReport.wAxisZ, iReport.lButtons);
		}
		recv_ready = false;
	}

Exit:
	RelinquishVJD(DevID);
	return 0;
}










/* HELPER FUNCTIONS */
// Convert PID Device Control to String
BOOL DevCtrl2Str(FFB_CTRL Ctrl, LPTSTR OutStr)
{
	BOOL stat = TRUE;
	LPTSTR Str="";

	switch (Ctrl)
	{
	case CTRL_ENACT:
		Str="Enable Actuators";
		break;
	case CTRL_DISACT:
		Str="Disable Actuators";
		break;
	case CTRL_STOPALL:
		Str="Stop All Effects";
		break;
	case CTRL_DEVRST:
		Str="Device Reset";
		break;
	case CTRL_DEVPAUSE:
		Str="Device Pause";
		break;
	case CTRL_DEVCONT:
		Str="Device Continue";
		break;
	default:
		stat = FALSE;
		break;
	}
	if (stat)
		_tcscpy_s(OutStr, 100, Str);

	return stat;
}

// Convert Effect operation to string
BOOL EffectOpStr(FFBOP Op, LPTSTR OutStr)
{
	BOOL stat = TRUE;
	LPTSTR Str="";

	switch (Op)
	{
	case EFF_START:
		Str="Effect Start";
		break;
	case EFF_SOLO:
		Str="Effect Solo Start";
		break;
	case EFF_STOP:
		Str="Effect Stop";
		break;
	default:
		stat = FALSE;
		break;
	}

	if (stat)
		_tcscpy_s(OutStr, 100, Str);

	return stat;
}

// Polar values (0x00-0xFF) to Degrees (0-360)
int Polar2Deg(BYTE Polar)
{
	return ((UINT)Polar*360)/255;
}

// Convert range 0x00-0xFF to 0%-100%
int Byte2Percent(BYTE InByte)
{
	return ((UINT)InByte*100)/255;
}

// Convert One-Byte 2's complement input to integer
int TwosCompByte2Int(BYTE in)
{
	int tmp;
	BYTE inv = ~in;
	BOOL isNeg = in>>7;
	if (isNeg)
	{
		tmp = (int)(inv);
		tmp = -1*tmp;
		return tmp;
	}
	else
		return (int)in;
}
