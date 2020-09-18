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
    return vals[0], vals[1], vals[2]

while True:
    while client.message_received != True:
        time.sleep(0.2)

    client.message_received = False
    # print(client.message_contents)
    if client.message_topic == "joystick_axis":
        print(client.message_contents)

        x, y, z = parse_3_axis(client.message_contents)

        socket.send_string("%s %s %s" % (x, y, z))
        print("sent " + str(x) + " " + str(y) + " " + str(z))