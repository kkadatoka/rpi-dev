#!/usr/bin/python
########################################################################
#
# A Python script for controlling the Adafruit 1.8" TFT LCD module
# from a Raspbery Pi.
#
# Author : Bruce E. Hall, W8BH <bhall66@gmail.com>
# Date : 27 Apr 2013
#
# This module uses the ST7735 controller and SPI data interface
#
#
# For more information, see w8bh.net
#
########################################################################
import RPi.GPIO as GPIO
import time
import os
#TFT to RPi connections
# PIN TFT RPi
# 1 backlight 3v3
# 2 MISO <none>
# 3 CLK GPIO 24
# 4 MOSI GPIO 23
# 5 CS-TFT GND
# 6 CS-CARD <none>
# 7 D/C GPIO 25
# 8 RESET <none>
# 9 VCC 3V3
# 10 GND GND

# Electrodragon 2.2" TFT bit-banging
#	TFT to RPI
#	TFT-PIN		RPI-PIN
#	1 Vcc		3v3
#	2 GND		GND
#	3 CS		GND
#	4 RESET		GPIO 22
#	5 D/C		GPIO 25
#	6 MOSI		GPIO 23
#	7 SCK		GPIO 24
#	8 BL LED	3v3
#	9 MISO		-

SCLK = 24
SDAT = 23
DC = 25
HWRESET = 22
BL = 17
pins = [SCLK,SDAT,DC,HWRESET]
#RGB888 Color constants
BLACK = 0x000000
RED = 0xFF0000
GREEN = 0x00FF00
BLUE = 0x0000FF
WHITE = 0xFFFFFF
COLORSET = [RED,GREEN,BLUE,WHITE]
#ST7735 commands (Author has ADAFRUIT, I have Electrodragon ILI9341 >> Commands are modified as necessary and commented)
NOP = 0x00
SWRESET = 0x01 #software reset
RDID = 0x04
RDDST = 0x09
SLPIN = 0x10
SLPOUT = 0x11 #sleep out
DISPON = 0x29 #display on
CASET = 0x2A #column address set
RASET = 0x2B #row address set
RAMWR = 0x2C #RAM write
MADCTL = 0x0b #0x36 #axis control
COLMOD = 0x3A #color mode
########################################################################
#
# Low-level routines
# These routines access GPIO directly
#
def SetPin(pinNumber,value):
	#sets the GPIO pin to desired value (1=on,0=off)
	GPIO.output(pinNumber,value)
def InitIO():
	GPIO.setmode(GPIO.BCM)
	#GPIO.setwarnings(False)
	for pin in pins:
		GPIO.setup(pin,GPIO.OUT)
	GPIO.output(SCLK,0) #start with clock line low
	GPIO.output(HWRESET,1) # start with RESET line high
	GPIO.output(HWRESET,0) # active-low reset
	GPIO.output(HWRESET,1)
########################################################################
#
# Bit-Banging (software) SPI routines:
#
#
def PulseClock():
	#pulses the serial clock line HIGH
	SetPin(SCLK,1) #bit clocked on low-high transition
	SetPin(SCLK,0) #no delay since python is slow
def WriteByte(value, data=True):
	"sends byte to display using software SPI"
	mask = 0x80 #start with bit7 (msb)
	SetPin(DC,data) #low = command; high = data
	for b in range(8): #loop for 8 bits, msb to lsb
		SetPin(SDAT,value & mask) #put bit on serial data line
		PulseClock() #clock in the bit
		mask >>= 1 #go to next bit
def WriteCmd(value):
	"Send command byte to display"
	WriteByte(value,False) #set D/C line to 0 = command
def WriteWord (value):
	"sends a 16-bit word to the display as data"
	WriteByte(value >> 8) #write upper 8 bits
	WriteByte(value & 0xFF) #write lower 8 bits
def WriteList (byteList):
	"Send list of bytes to display, as data"
	for byte in byteList: #grab each byte in list
		WriteByte(byte) #and send it
def Write888(value,reps=1):
	"sends a 24-bit RGB pixel data to display, with optional repeat"
	red = value>>16 #red = upper 8 bits
	green = (value>>8) & 0xFF #green = middle 8 bits
	blue = value & 0xFF #blue = lower 8 bits
	RGB = [red,green,blue] #assemble RGB as 3 byte list
	for a in range(reps): #send RGB x optional repeat
		WriteList(RGB)
########################################################################
#
# ST7735 driver routines:
#
#
def InitDisplay():
	"Resets & prepares display for active use."
	WriteCmd (SWRESET) #software reset, puts display into sleep
	time.sleep(0.5) #wait 200mS for controller register init
	WriteCmd (SLPOUT) #sleep out
	time.sleep(0.5) #wait 200mS for TFT driver circuits
	WriteCmd (DISPON) #display on!
def SetAddrWindow(x0,y0,x1,y1):
	"sets a rectangular display window into which pixel data is placed"
	WriteCmd(CASET) #set column range (x0,x1)
	WriteWord(x0)
	WriteWord(x1)
	WriteCmd(RASET) #set row range (y0,y1)
	WriteWord(y0)
	WriteWord(y1)
def DrawPixel(x,y,color):
	"draws a pixel on the TFT display"
	SetAddrWindow(x,y,x,y)
	WriteCmd(RAMWR)
	Write888(color)
def FillRect(x0,y0,x1,y1,color):
	"fills rectangle with given color"
	width = x1-x0+1
	height = y1-y0+1
	SetAddrWindow(x0,y0,x1,y1)
	WriteCmd(RAMWR)
	Write888(color,width*height)
def FillScreen(color):
	"Fills entire screen with given color"
	FillRect(0,0,239,319,color)
def ClearScreen():
	"Fills entire screen with black"
	FillRect(0,0,239,319,BLACK)
########################################################################
#
# Testing routines:
#
#
def TimeDisplay():
	"Measures time required to fill display twice"
	startTime=time.time()
	print " Now painting screen GREEN"
	FillScreen(GREEN)
	print " Now clearing screen"
	ClearScreen()
	elapsedTime=time.time()-startTime
	print " Elapsed time %0.1f seconds" % (elapsedTime)
########################################################################
#
# Main Program
#
print "Adafruit 1.8 TFT display demo"
InitIO()
os.system("gpio readall")
InitDisplay()
TimeDisplay()
print "Done."
# END ###############################################################