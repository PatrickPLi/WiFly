from solace import *
import keyboard  # using module keyboard

x, y, z = 0, 0, 0
         
client = initialize()

while True:
    if keyboard.is_pressed('w'):  # if key 'w' is pressed 
        y += 20
        print("y = " + str(y) + "\n")
        #break  # finishing the loop
    if keyboard.is_pressed('s'):  # if key 's' is pressed 
        y -= 20
        print("y = " + str(y) + "\n")
        #break  # finishing the loop
    if keyboard.is_pressed('a'):  # if key 'a' is pressed 
        x -= 20
        print("x = " + str(x) + "\n")
        #break  # finishing the loop
    if keyboard.is_pressed('d'):  # if key 'd' is pressed 
        x += 20
        print("x = " + str(x) + "\n")     

        #break  # finishing the loop
    if keyboard.is_pressed('q'):  # if key 'q' is pressed 
        z -= 20
        print("z = " + str(z) + "\n")
        #break  # finishing the loop
    if keyboard.is_pressed('e'):  # if key 'e' is pressed 
        z += 20
        print("z = " + str(z) + "\n")
        #break  # finishing the loop

    client.publish('joystick_axis', payload = str(x) + ' ' + str(y) + ' ' + str(z))
    print(str(x) + ' ' + str(y) + ' ' + str(z))

    # print(str(x) + ' ' + str(y) + ' ' + str(z))