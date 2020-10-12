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
#include <zmq.h>
#include "zmq_helpers.h"
#include <cassert>


// Default device ID (Used when ID not specified)
#define DEV_ID		1

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
		wprintf(L"Vendor: %s\nProduct :%s\nVersion Number:%s\n", static_cast<TCHAR *> (GetvJoyManufacturerString()), static_cast<TCHAR *>(GetvJoyProductString()), static_cast<TCHAR *>(GetvJoySerialNumberString()));
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

	// Connect to Python ZMQ client
	printf("Collecting updates from python client\n");
	void* context = zmq_ctx_new();
	void* subscriber = zmq_socket(context, ZMQ_SUB);
	int rc = zmq_connect(subscriber, "tcp://localhost:5556");
	assert(rc == 0);

	//  Subscribe to topic
	const char* filter = NULL;
	rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, 0);
	assert(rc == 0);
	while (1)
	{
		// Set destenition vJoy device
		id = (BYTE)DevID;
		iReport.bDevice = id;

		while(1) {
			char* string = s_recv(subscriber);

			int x, y, z, t, g, b, fu, fd, vt;
			sscanf(string, "%d %d %d %d %d %d", &x, &y, &z, &t, &g, &b, &fu, &fd, &vt);
			free(string);
			if (x < 0 && y < 0 && z < 0 && t < 0)
			{

				zmq_close(subscriber);
				zmq_ctx_destroy(context);
				goto Exit;
			}
			iReport.wAxisX = x;
			iReport.wAxisY = y;
			iReport.wAxisXRot = z;
			iReport.wAxisZ = t;

			Btns = 1;
			if (g == 1) //gear down
			{
				Btns = Btns << 1;
			}
			else //gear up
			{
				Btns = Btns << 2;
			}
			Btns |= b;

			//b1 is brake, b2 and b3 are gear
			//b4 is flaps up, b5 is flaps down
			printf("fu fd %d %d\n", fu, fd);
			if (fu == 1)
			{
				Btns |= (1 << 3);
				printf("BUTTON FU\n");
			}

			if (fd == 1)
			{
				Btns |= (1 << 4);
				printf("BUTTON FD\n");
			}

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
				printf("Updated vJoy device with values %d %d %d\n", iReport.wAxisX, iReport.wAxisY, iReport.wAxisZ);
			}
			Sleep(200);
		}













		
		

		/* READ FILE */

		std::string line;
		std::ifstream myfile("joystick_axis.txt");
		if (myfile.is_open())
		{
			while (std::getline(myfile, line))
			{
				/*std::cout << line << '\n';*/
			}
			myfile.close();
		}

		else {
			/*std::cout << "Unable to open file";*/
			continue;
		}

		std::vector<int> result;
		std::stringstream ss(line); //create string stream from the string
		while (ss.good()) {
			std::string substr;
			std::getline(ss, substr, ' '); //get first string delimited by comma
			if (substr.length()) result.push_back(stoi(substr));
		}
		/*****************/

		if (result.size() == 3)
		{
			if (X != result[0] || Y != result[1] || Z != result[2])
			{
			X = result[0];
			Y = result[1];
			Z = result[2];
			std::cout << "X: " << X << '\n' << "Y: " << Y << '\n' << "Z: " << Z << '\n';
			}
		}

		// Set position data of 3 first axes

		//char keyInput = getch();

		//std::cout << "input = " << keyInput << std::endl;

		//if (keyInput == 'e')
		//{
		//	Z += 2000;
		//}

		//else if (keyInput == 'q')
		//{
		//	Z -= 2000;
		//}

		//if (keyInput == 'w')
		//{
		//	Y += 2000;
		//}

		//else if (keyInput == 's')
		//{
		//	Y -= 2000;
		//}

		//if (keyInput == 'd')
		//{
		//	X += 2000;
		//}

		//else if (keyInput == 'a')
		//{
		//	X -= 2000;
		//}

		
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
