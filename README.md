# WiFly
Hello! This has been a project that I have had in mind for years but lacked the technical ability to do. Thanks to the courses I have taken so far in University as well as my co-op experiences, I thought I should finally give it a shot. The idea behind this project is to allow for joystick emulation without the need for any extra hardware --- not even a USB cable! From a phone or tablet, joystick and button data will be sent over the cloud to the PC, where it will be injected into a virtual joystick driver and then hopefully into Flight Simulator!

## Why I'm making this
I love playing Flight Simulator with my trusty joystick. Unfortunately, the lovely folks over at CBSA and Air Canada do not seem as excited when I bring it to the airport (ironic!)

## How to use it
* Clone the repo
* Open Visual Studio (I used 2017 Community Edition) and open *WiFly/vJoySDK/SDK/src/vJoyClient.sln*. Let Visual Studio do its updates + configuration for a bit before everything inevitably goes wrong and doesn't work. Cry.
* ```#include <zmq.h> ``` should be underlined in red because we did not install zmq yet. Clone vcpkg from Microsoft's Github and run the *bootstrap-vcpkg.bat* file from CMD. Next, with the Command Prompt open in the vcpkg folder (the one that we cloned,) use ```vcpkg search zeromq``` to find the name of the package we need. It should be named *zeromq:x86-windows*. Use ```vcpkg install``` to install it, and then ```vcpkg integrate install``` to introduce it to Visual Studio. At this point, the include for zmq.h should be resolved.
* From Visual Studio, run the project in debug mode.
* Now open another Command Prompt window and navigate to *WiFly/Python*, then create and enter a virutal environment. Run *main.py* using *python main.py*, then use *pip install* to install the missing dependecies (TODO: add a *requirements.txt* file)
* To be continued as I develop the app... :)
