ov7670
======

OV7670+fifo camera control with an AVR ATmega1284p. 
Takes pictures in QQVGA resolution, saves it on a SD card and displays it. Some functions to change
brighness, noise reduction and effects plus caption modes (RGB modes and YUV)

Camera Module used:
http://www.suntekstore.com/goods-14004307-640x480_cmos_camera_module_ov7670_vga_yuv_snr_46_db_with_high_quality_lens.html

TFT module (inlcludes SD card slot) used:
http://arduino.cc/en/Guide/TFT

Please note that this camera module needs to initiate the write reset for the AL422 every VSYNC in code.
Other camera models out there are wired to do this automatically.

The code works as well with an Arduino UNO but you need to change the ports definitions in ov7670_ports.h 
No TFT possible since the UNO has not enought i/o pins. You can disable the TFT and SD in main.h

This code uses the mighty-1824p library (not included) from maniacbug (thanks!!) on github: https://github.com/maniacbug/mighty-1284p which 
applies changes to the vanilla Arduino library to run on a ATMEGA1284p. You could get rid of Arduino the library but 
that would involve changes in main.cpp and the SPI and TFT classes.

Breadboard diagram (black text: OV7670 wiring, blue text:TFT wiring): ov7670_breadboard.pdf
ATmega1284p wiring: ov7670_ports.h

Enjoy!!
