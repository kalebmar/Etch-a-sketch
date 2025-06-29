Integrate a 128x64 pixel graphical LCD display with the STM32 NUCLEO-F091RC development board, along with two potentiometers connected to the controller's A/D converters. Using the two potentiometers, implement a so-called "shaky" drawing board on the display.
Additionally, place a push-button that clears the screen, and optionally implement screen clearing via a motion sensor by shaking the device.
Create a PC client application that allows the image to be downloaded, stored, and reloaded via a serial connection. Use a virtual serial port for communication, implemented through the USB port available on the development board.
After designing, building, and commissioning the circuit, develop a demonstration test program system showcasing the device. This system should include the microcontroller software ensuring correct operation as well as the PC client application.

The code available on GitHub was developed specifically for this project. A custom driver was written for the NT7108 LCD controller, and it can be found in the display_driver.c file. The code for the PC client application has not been released.
