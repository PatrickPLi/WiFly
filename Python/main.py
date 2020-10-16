from solace import *
import zmq
         
client = initialize()

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:5556")

x, y, z, t, g, b, fu, fd, vt = 0, 0, 0, 0, 0, 0, 0, 0, 0

button_LUT = {"fu": 1, "fd": 2, "tv": 3, "tb": 4}

def parse_inputs(strg):
    vals = strg.split(" ")
    if len(vals) != 5:
        return -1, -1, -1, -1, -1

    OldMax = 45
    OldMin = -45
    NewMax = 32766
    NewMin = 0

    scaledVals = [0] * 4

    OldRange = (OldMax - OldMin)  
    NewRange = (NewMax - NewMin)
    for i in range(4):
        scaledVals[i] = (((float(vals[i]) - OldMin) * NewRange) / OldRange) + NewMin

    # if vals[6] or vals[7] or vals[8]:
    #     client.publish('btnRecv', "1")

    return scaledVals[0], scaledVals[1], scaledVals[2], scaledVals[3], int(vals[4])

def parse_buttons(strg):
    vals = strg.split(" ")
    if len(vals) != 1:
        return "button error"

    if vals[0] not in button_LUT:
        return "unrecognized button"
    else:
        client.publish('btnRecv', "1")
        return str(button_LUT[vals[0]])

while True:
    while client.message_received != True:
        time.sleep(0.3)

    client.message_received = False
    # print(client.message_contents)
    if client.message_topic == "joystick_axis":
        print(client.message_contents)

        x, y, z, t, buttons = parse_inputs(client.message_contents)

        # socket.send_string("%s %s %s" % (x, y, z))
        socket.send_string("%d %d %d %d %d" % (x, y, z, t, buttons))
        print("sent " + "%d %d %d %d %d" % (x, y, z, t, buttons))