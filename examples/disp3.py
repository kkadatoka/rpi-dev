#!/usr/bin/python
########################################################################
#
# A Python script for controlling the Adafruit 1.8" TFT LCD module
# from a Raspbery Pi.
#
# Author : Bruce E. Hall, W8BH <bhall66@gmail.com>
# Date : 21 Feb 2014
#
# This module uses the ST7735 controller and SPI data interface
# PART 3 --- GRAPHICS
#
# For more information, see w8bh.net
#
########################################################################
import RPi.GPIO as GPIO
import time
import spidev #hardware SPI
from random import randint
from math import sqrt
#TFT to RPi connections
# PIN TFT RPi
# 1 backlight 3v3
# 2 MISO <none>
# 3 CLK SCLK (GPIO 11)
# 4 MOSI MOSI (GPIO 10)
# 5 CS-TFT GND
# 6 CS-CARD <none>
# 7 D/C GPIO 25
# 8 RESET <none>
# 9 VCC 3V3
# 10 GND GND
DC = 25
XSIZE = 128
YSIZE = 160
XMAX = XSIZE-1
YMAX = YSIZE-1
X0 = XSIZE/2
Y0 = YSIZE/2
#Color constants
BLACK = 0x0000
BLUE = 0x001F
RED = 0xF800
GREEN = 0x0400
LIME = 0x07E0
CYAN = 0x07FF
MAGENTA = 0xF81F
YELLOW = 0xFFE0
WHITE = 0xFFFF
PURPLE = 0x8010
NAVY = 0x0010
TEAL = 0x0410
OLIVE = 0x8400
MAROON = 0x8000
SILVER = 0xC618
GRAY = 0x8410
COLORSET = [BLACK,BLUE,RED,GREEN,LIME,CYAN,MAGENTA,YELLOW,
WHITE,PURPLE,NAVY,TEAL,OLIVE,MAROON,SILVER,GRAY]
#TFT display constants
SWRESET = 0x01
SLPIN = 0x10
SLPOUT = 0x11
PTLON = 0x12
NORON = 0x13
INVOFF = 0x20
INVON = 0x21
DISPOFF = 0x28
DISPON = 0x29
CASET = 0x2A
RASET = 0x2B
RAMWR = 0x2C
RAMRD = 0x2E
PTLAR = 0x30
MADCTL = 0x36
COLMOD = 0x3A
FRMCT1 = 0xB1
FRMCT2 = 0xB2
FRMCT3 = 0xB3
INVCTR = 0xB4
DISSET = 0xB6
PWRCT1 = 0xC0
PWRCT2 = 0xC1
PWRCT3 = 0xC2
PWRCT4 = 0xC3
PWRCT5 = 0xC4
VMCTR1 = 0xC5
PWRCT6 = 0xFC
GAMCTP = 0xE0
GAMCTN = 0xE1
########################################################################
#
# Low-level routines
#
#
def SetPin(pinNumber,value):
	#sets the GPIO pin to desired value (1=on,0=off)
	GPIO.output(pinNumber,value)
def InitGPIO():
	GPIO.setmode(GPIO.BCM)
	#GPIO.setwarnings(False)
	GPIO.setup(DC,GPIO.OUT)
def InitSPI():
	"returns an opened spi connection to device(0,0) in mode 0"
	spiObject = spidev.SpiDev()
	spiObject.open(0,0)
	spiObject.mode = 0
	return spiObject
########################################################################
#
# ST7735 TFT controller routines:
#
#
def WriteByte(value):
	"sends an 8-bit value to the display as data"
	SetPin(DC,1)
	spi.writebytes([value])
def WriteWord (value):
	"sends a 16-bit value to the display as data"
	SetPin(DC,1)
	spi.writebytes([value>>8, value&0xFF])
def Command(cmd, *bytes):
	"Sends a command followed by any data it requires"
	SetPin(DC,0) #command follows
	spi.writebytes([cmd]) #send the command byte
	if len(bytes)>0: #is there data to follow command?
		SetPin(DC,1) #data follows
		spi.writebytes(list(bytes))#send the data bytes
def InitDisplay():
	"Resets & prepares display for active use."
	Command (SWRESET) #reset TFT controller
	time.sleep(0.5) #wait 200mS for controller init
	Command (SLPOUT) #wake from sleep
	time.sleep(0.5)
	Command (COLMOD, 0x05) #set color mode to 16 bit
	time.sleep(0.2)
	Command (DISPON) #turn on display
def SetAddrWindow(x0,y0,x1,y1):
	"sets a rectangular display window into which pixel data is placed"
	Command (CASET,0,x0,0,x1) #set column range (x0,x1)
	Command (RASET,0,y0,0,y1) #set row range (y0,y1)
def WriteBulk (value,reps,count=1):
	"sends a 16-bit pixel word many, many times using hardware SPI"
	"number of writes = reps * count. Value of reps must be <= 2048"
	SetPin(DC,0) #command follows
	spi.writebytes([RAMWR]) #issue RAM write command
	SetPin(DC,1) #data follows
	valHi = value >> 8 #each pixel is two bytes
	valLo = value & 0xFF
	byteArray = [valHi,valLo]*reps #create buffer of multiple pixels
	for a in range(count):
		spi.writebytes(byteArray) #send this buffer multiple times
########################################################################
#
# Graphics routines:
#
#
def DrawPixel(x,y,color):
	"draws a pixel on the TFT display"
	SetAddrWindow(x,y,x,y)
	Command (RAMWR, color>>8, color&0xFF)
def FastDrawPixel(x,y,color):
	"draws a pixel on the TFT display; increases speed by inlining"
	GPIO.output(DC,0)
	spi.writebytes([CASET])
	GPIO.output(DC,1)
	spi.writebytes([0,x,0,x])
	GPIO.output(DC,0)
	spi.writebytes([RASET])
	GPIO.output(DC,1)
	spi.writebytes([0,y,0,y])
	GPIO.output(DC,0)
	spi.writebytes([RAMWR])
	GPIO.output(DC,1)
	spi.writebytes([color>>8, color&0xFF])
def HLine (x0,x1,y,color):
	"draws a horizontal line in given color"
	width = x1-x0+1
	SetAddrWindow(x0,y,x1,y)
	WriteBulk(color,width)
def VLine (x,y0,y1,color):
	"draws a verticle line in given color"
	height = y1-y0+1
	SetAddrWindow(x,y0,x,y1)
	WriteBulk(color,height)
def Line (x0,y0,x1,y1,color):
	"draws a line in given color"
	if (x0==x1):
		VLine(x0,y0,y1,color)
	elif (y0==y1):
		HLine(x0,x1,y0,color)
	else:
		slope = float(y1-y0)/(x1-x0)
	if (abs(slope)< 1):
		for x in range(x0,x1+1):
			y = (x-x0)*slope + y0
			FastDrawPixel(x,int(y+0.5),color)
	else:
		for y in range(y0,y1+1):
			x = (y-y0)/slope + x0
			FastDrawPixel(int(x+0.5),y,color)
def DrawRect(x0,y0,x1,y1,color):
	"Draws a rectangle in specified color"
	HLine(x0,x1,y0,color)
	HLine(x0,x1,y1,color)
	VLine(x0,y0,y1,color)
	VLine(x1,y0,y1,color)
def FillRect(x0,y0,x1,y1,color):
	"fills rectangle with given color"
	width = x1-x0+1
	height = y1-y0+1
	SetAddrWindow(x0,y0,x1,y1)
	WriteBulk(color,width,height)
def FillScreen(color):
	"Fills entire screen with given color"
	FillRect(0,0,239,319,color)
def ClearScreen():
	"Fills entire screen with black"
	FillRect(0,0,239,319,BLACK)
def Circle(xPos,yPos,radius,color):
	"draws circle at x,y with given radius & color"
	xEnd = int(0.7071*radius)+1
	for x in range(xEnd):
		y = int(sqrt(radius*radius - x*x))
		FastDrawPixel(xPos+x,yPos+y,color)
		FastDrawPixel(xPos+x,yPos-y,color)
		FastDrawPixel(xPos-x,yPos+y,color)
		FastDrawPixel(xPos-x,yPos-y,color)
		FastDrawPixel(xPos+y,yPos+x,color)
		FastDrawPixel(xPos+y,yPos-x,color)
		FastDrawPixel(xPos-y,yPos+x,color)
		FastDrawPixel(xPos-y,yPos-x,color)
def FillCircle(xPos,yPos,radius,color):
	"draws filled circle at x,y with given radius & color"
	r2 = radius * radius
	for x in range(radius):
		y = int(sqrt(r2-x*x))
		y0 = yPos-y
		y1 = yPos+y
		VLine(xPos+x,y0,y1,color)
		VLine(xPos-x,y0,y1,color)
########################################################################
#
# Testing routines:
#
#
def PrintElapsedTime(function,startTime):
	"Formats an output string showing elapsed time since function start"
	elapsedTime=time.time()-startTime
	print "%15s: %8.3f seconds" % (function,elapsedTime)
	time.sleep(1)
def ScreenTest():
	"Measures time required to fill display twice"
	startTime=time.time()
	FillScreen(LIME)
	FillScreen(MAGENTA)
	PrintElapsedTime('ScreenTest',startTime)
def RandRect():
	"Returns four integers x0,y0,x1,y1 as screen rect coordinates"
	x1 = randint(1,240)
	y1 = randint(1,320)
	dx = randint(30,80)
	dy = randint(30,80)
	x2 = x1 + dx
	if x2>237:
		x2 = 237
	y2 = y1 + dy
	if y2>317:
		y2 = 317
	return x1,y1,x2,y2
def RandColor():
	"Returns a random color from BGR565 Colorspace"
	index = randint(0,len(COLORSET)-1)
	return COLORSET[index]
def RectTest(numCycles=50):
	"Draws a series of random open rectangles"
	ClearScreen()
	startTime = time.time()
	for a in range(numCycles):
		x0,y0,x1,y1 = RandRect()
		DrawRect(x0,y0,x1,y1,RandColor())
	PrintElapsedTime('RectTest',startTime)
def FillRectTest(numCycles=70):
	"draws random filled rectangles on the display"
	startTime=time.time()
	ClearScreen()
	for a in range(numCycles):
		x0,y0,x1,y1=RandRect()
		FillRect(x0,y0,x1,y1,RandColor())
	PrintElapsedTime('FillRect',startTime)
def LineTest(numCycles=50):
	"Draw a series of semi-random lines on display"
	ClearScreen()
	startTime=time.time()
	for a in range(numCycles):
		Line(10,10,randint(20,237),randint(20,317),YELLOW)
		Line(120,10,randint(2,237),randint(10,317),CYAN)
	PrintElapsedTime('LineTest',startTime)
def PixelTest(color=BLACK, numPixels=5000):
	"Writes random pixels to the screen"
	ClearScreen()
	startTime = time.time()
	for i in range(numPixels):
		xPos = randint(1,239)
		yPos = randint(1,319)
		DrawPixel(xPos,yPos,LIME)
	PrintElapsedTime('PixelTest',startTime)
def FastPixelTest(color=BLACK, numPixels=5000):
	"Writes random pixels to the screen"
	ClearScreen()
	startTime = time.time()
	for i in range(numPixels):
		xPos = randint(1,239)
		yPos = randint(1,319)
		FastDrawPixel(xPos,yPos,YELLOW)
	PrintElapsedTime('FastPixelTest',startTime)
def MoireTest():
	"Draws a series of concentric circles"
	ClearScreen()
	startTime = time.time()
	for radius in range(6,60,2):
		Circle(X0,Y0,radius,YELLOW)
	PrintElapsedTime('MoireTest',startTime)
def CircleTest(numCycles=40):
	"draw a series of random circles"
	ClearScreen()
	startTime = time.time()
	for a in range(numCycles):
		x = randint(30,120)
		y = randint(30,160)
		radius = randint(10,40)
		Circle(x,y,radius,RandColor())
	PrintElapsedTime('CircleTest',startTime)
def FillCircleTest(numCycles=40):
	"draw a series of random filled circles"
	ClearScreen()
	startTime = time.time()
	for a in range(numCycles):
		x = randint(30,120)
		y = randint(30,160)
		radius = randint(10,40)
		FillCircle(x,y,radius,RandColor())
	PrintElapsedTime('FillCircleTest',startTime)
def RunTests():
	"run a series of graphics test routines & time them"
	startTime = time.time() #keep track of test duration
	ScreenTest() #fill entire screen with color
	RectTest() #draw rectangles
	FillRectTest() #draw filled rectangles
	PixelTest() #draw 5000 random pixels
	FastPixelTest() #same as above, w/ modified routine
	LineTest() #draw straight lines
	MoireTest() #draw concentric circles
	CircleTest() #draw random circles
	FillCircleTest() #draw filled circles
	PrintElapsedTime('Full Suite',startTime)
########################################################################
#
# Main Program
#
print "Adafruit 1.8 TFT display demo with hardware SPI"
spi = InitSPI() #initialize SPI interface
InitGPIO() #initialize GPIO interface
InitDisplay() #initialize TFT controller
RunTests() #run suite of graphics tests
spi.close() #close down SPI interface
print "Done."
# END ###############################################################