import zmq
from random import randrange
from time import sleep

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:5556")

while True:
    x = randrange(0, 30000)
    y = randrange(0, 30000)
    z = randrange(0, 30000)

    socket.send_string("%i %i %i" % (x, y, z))
    print("sent " + str(x) + " " + str(y) + " " + str(z))
    sleep(1)