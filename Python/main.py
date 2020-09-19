from solace import *
import zmq
         
client = initialize()

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:5556")

def parse_3_axis(str):
    vals = str.split(" ")
    if len(vals) != 3:
        return -1, -1, -1

    OldMax = 90
    OldMin = -90
    NewMax = 31000
    NewMin = 0

    scaledVals = [0] * 3

    OldRange = (OldMax - OldMin)  
    NewRange = (NewMax - NewMin)
    for i in range(3):
        scaledVals[i] = (((float(vals[i]) - OldMin) * NewRange) / OldRange) + NewMin

    return scaledVals[0], scaledVals[1], scaledVals[2]

while True:
    while client.message_received != True:
        time.sleep(0.2)

    client.message_received = False
    # print(client.message_contents)
    if client.message_topic == "joystick_axis":
        print(client.message_contents)

        x, y, z = parse_3_axis(client.message_contents)

        # socket.send_string("%s %s %s" % (x, y, z))
        socket.send_string("%d %d %d" % (x, y, z))
        print("sent " + "%d %d %d" % (x, y, z))