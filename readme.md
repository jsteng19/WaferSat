# WaferSat

## Changelog 
* Switched to chibios182 from the ChibiStudio for Windows installation
* Moved WaferSat board files to ./WAFERSAT\_TEST\_FIXTURE 
* Corrected some paths in the Makefile

Anyway, it *should* build on Linux and Windows through ChibiStudio without further modification now.

## Notes on Flashing and Debugging the WaferSat
* USB Micro on the WaferSat is only for power
* The Debug serial port is labeled 'CONSOLE' on the board; make sure the black wire goes to the pin labeled GND
* On the ST-LINK board, open the jumpers to use it to program an external board (like the wafersat)
* Make sure the ST-Link to WAFERSAT connections are 1 to 1; make sure that the red wire is connected to pin 1 or P6 on both sides
* Plug the ST-LINK USB MICRO port in; it shows up as an ST-Link, a USB port, and a Thumb Drive; If you put a binary onto the Thumb Drive, it will automatically flash it to the WaferSat; the programmer shows up as ttyACM0
* the FTDI cable from the WaferSat shows up as ttyUSB0 
