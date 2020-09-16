from solace import *
         
client = initialize()
while True:
    while client.message_received != True:
        time.sleep(0.01)

    client.message_received = False
    # print(client.message_contents)
    if client.message_topic == "joystick_axis":
        print(client.message_contents)

        # opening a file in 'w'
        file = open('../vJoySDK/SDK/src/joystick_axis.txt', 'w')
        # write() - it used to write direct text to the file
        # writing data using the write() method
        file.write(client.message_contents)
        # closing the file
        file.close()