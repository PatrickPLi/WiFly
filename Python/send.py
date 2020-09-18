from solace import *
import keyboard  # using module keyboard
from time import sleep

x, y, z = 0, 0, 0
         
client = initialize()

while True:
    changed = False
    if keyboard.is_pressed('w'):  # if key 'w' is pressed 
        y += 200
        print("y = " + str(y) + "\n")
        changed = True
        #break  # finishing the loop
    if keyboard.is_pressed('s'):  # if key 's' is pressed 
        y -= 200
        print("y = " + str(y) + "\n")
        changed = True
        #break  # finishing the loop
    if keyboard.is_pressed('a'):  # if key 'a' is pressed 
        x -= 200
        print("x = " + str(x) + "\n")
        changed = True
        #break  # finishing the loop
    if keyboard.is_pressed('d'):  # if key 'd' is pressed 
        x += 200
        print("x = " + str(x) + "\n") 
        changed = True    

        #break  # finishing the loop
    if keyboard.is_pressed('q'):  # if key 'q' is pressed 
        z -= 200
        print("z = " + str(z) + "\n")
        changed = True
        #break  # finishing the loop
    if keyboard.is_pressed('e'):  # if key 'e' is pressed 
        z += 200
        print("z = " + str(z) + "\n")
        changed = True
        #break  # finishing the loop

    if changed:
        client.publish('joystick_axis', payload = str(x) + ' ' + str(y) + ' ' + str(z))
        print(str(x) + ' ' + str(y) + ' ' + str(z))
    sleep(0.01)