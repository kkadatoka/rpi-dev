#!/usr/bin/python

import RPi.GPIO as GPIO
import time
import spidev #hardware SPI
from random import randint
from math import sqrt

# GPIO PINS
DC = 25
HWRESET = 22
CS = 23
BL = 18


WIDTH = 240
HEIGHT = 320

XSIZE = WIDTH
YSIZE = HEIGHT
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
GAMMASET = 0x26
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
VMCTR2 = 0xC7
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
def HWReset():
	SetPin(HWRESET,1)
	SetPin(HWRESET,0)
	SetPin(HWRESET,1)
def InitGPIO():
	GPIO.setmode(GPIO.BCM)
	#GPIO.setwarnings(False)
	GPIO.setup(DC,GPIO.OUT)
	GPIO.setup(HWRESET,GPIO.OUT)
	GPIO.setup(CS,GPIO.OUT)
	GPIO.output(CS,0)
	GPIO.setup(BL,GPIO.OUT)
	GPIO.output(BL,1)
def InitSPI():
	"returns an opened spi connection to device(0,0) in mode 0"
	spiObject = spidev.SpiDev()
	spiObject.open(0,0)
	spiObject.mode = 3
	spiObject.cshigh = False
	spiObject.max_speed_hz = 32000000
	return spiObject

########################################################################
#
# ILI9341 TFT controller SPI routines:
#
#
def sendData (value):
	"sends Command of 8 bits to display"
	SetPin(DC,0)
	SetPin(CS,0)
	spi.writebytes([value])
	SetPin(CS,1)
def sendWord (value):
	"Sends a 16-bit value to display"
def WriteByte(value):
	"sends an 8-bit value to the display as data"
	SetPin(DC,1)
	SetPin(CS,0)
	spi.writebytes([value])
	#spi.xfer2([value])
	SetPin(CS,1)
def WriteWord (value):
	"sends a 16-bit value to the display as data"
	SetPin(DC,1)
	SetPin(CS,0)
	spi.writebytes([value>>8, value&0xFF])
	#spi.xfer2([value>>8, value&0xFF])
	SetPin(CS,1)
def Command(cmd, *bytes):
	"Sends a command followed by any data it requires"
	SetPin(DC,0) #command follows
	SetPin(CS,0)
 	spi.writebytes([cmd]) #send the command byte
	SetPin(CS,1)
	#spi.xfer2([cmd]) #send the command byte
	if len(bytes)>0: #is there data to follow command?
		SetPin(DC,1) #data follows
		SetPin(CS,0)
		spi.writebytes(list(bytes))#send the data bytes
		#spi.xfer2(list(bytes))#send the data bytes
		SetPin(CS,1)
########################################################################
#
# ILI9341 TFT controller routines:
#
#
def fillScreen():
	"Fill screen with emptyness"
	print "Filling with 0"
	# Fillscreen
	Command (0x2a) # set col
	sendData(0)
	sendData(XMAX)
	Command (0x2b) # Set page
	sendData(0)
	sendData(YMAX)
	Command (0x2c)
	SetPin(DC,1)
	SetPin(CS,0)
	for i in range(0, 38400):
		WriteByte(0)
		WriteByte(0)
		WriteByte(0)
		WriteByte(0)
	SetPin(CS,1)
def InitDisplay():
	"Resets & prepares display for active use."
	print "Initializing display"
	HWReset()
	time.sleep(0.5)
	Command (SWRESET) #reset TFT controller
	time.sleep(0.5) #wait 200mS for controller init

	# ADAFRUIT's ILI9341 routine
	Command (0xef, 0x03, 0x80, 0x02)
	Command (0xcf, 0x00, 0xc1, 0x30)
	Command (0xed, 0x64, 0x03, 0x12, 0x81)
	Command (0xe8, 0x85, 0x00, 0x78)
	Command (0xcb, 0x39, 0x2c, 0x00, 0x34, 0x02)
	Command (0xf7, 0x20)
	Command (0xea, 0x00, 0x00)
	Command (PWRCT1, 0x23)
	Command (PWRCT2, 0x10)
	Command (VMCTR1, 0x3e, 0x28)
	Command (VMCTR2, 0x86)
	Command (MADCTL, 0x48)
	Command (COLMOD, 0x55)
	Command (FRMCT1, 0x00, 0x18)
	Command (DISSET, 0x08, 0x82, 0x27)
	Command (0xF2, 0x00)
	Command (GAMMASET, 0x01)
	Command (GAMCTP, 0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00)
	Command (GAMCTN, 0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f)


#	Command (0xcf, 0x00, 0x8b, 0x30)
#	Command (0xed, 0x67, 0x03, 0x12, 0x81)
#	Command (0xe8, 0x85, 0x10, 0x71)
#	Command (0xcb, 0x39, 0x2c, 0x00, 0x34, 0x02)
#	Command (0xf7, 0x20)
#	Command (0xea, 0x00, 0x00)
#	Command (0xc0, 0x1b)
#	Command (0xc1, 0x10)
#	Command (0xc5, 0x3f, 0x3c)
#	Command (0xc7, 0xb7)
#	Command (0x36, 0x08)
#	Command (0x3a, 0x55)
#	Command (0xb1, 0x00, 0x1b)
#	Command (0xb6, 0x0a, 0xa2)
#	Command (0xF2, 0x00)
#	Command (0x26, 0x01)
#	Command (0xe0, 0x0f, 0x2a, 0x28, 0x08, 0x0e, 0x08, 0x54, 0xa9, 0x43, 0x0a, 0x0f, 0x00, 0x00, 0x00, 0x00)
#	Command (0xe1, 0x00, 0x15, 0x17, 0x07, 0x11, 0x06, 0x2b, 0x56, 0x3c, 0x05, 0x10, 0x0f, 0x3f, 0x3f, 0x0f)

	Command (SLPOUT) #wake from sleep
	#time.sleep(0.2)
	#Command (COLMOD, 0x05) #set color mode to 16 bit
	time.sleep(0.2)
	Command (DISPON) #turn on display
	print "Done initializaing..."
	time.sleep(2)
	fillScreen()
def SetAddrWindow(x0,y0,x1,y1):
	"sets a rectangular display window into which pixel data is placed"
	Command (CASET)
	#sendData(0)
	sendData(x0)
	#sendData(0) 
	sendData(x1) #set column range (x0,x1)
	Command (RASET)
	#sendData(0)
	sendData(y0) 
	#sendData(0) 
	sendData(y1) #set row range (y0,y1)
	Command (RAMWR)
def WriteBulk (value,reps,count=1):
	"sends a 16-bit pixel word many, many times using hardware SPI"
	"number of writes = reps * count. Value of reps must be <= 2048"
	#SetPin(DC,0) #command follows
	#spi.writebytes([RAMWR]) #issue RAM write command
	#SetPin(DC,1) #data follows
	Command (RAMWR)
	valHi = value >> 8 #each pixel is two bytes
	valLo = value & 0xFF
	byteArray = [valHi,valLo]*reps #create buffer of multiple pixels
	for a in range(count):
		SetPin(DC,1)
		SetPin(CS,0)
		spi.writebytes(byteArray) #send this buffer multiple times
		SetPin(CS,1)
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
	Command (CASET, 0, x, 0, x)
	Command (RASET, 0, y, 0, y)
	Command (RAMWR, color>>8, color&0xFF)
#	GPIO.output(DC,0)
#	spi.writebytes([CASET])
#	GPIO.output(DC,1)
#	spi.writebytes([0,x,0,x])
#	GPIO.output(DC,0)
#	spi.writebytes([RASET])
#	GPIO.output(DC,1)
#	spi.writebytes([0,y,0,y])
#	GPIO.output(DC,0)
#	spi.writebytes([RAMWR])
#	GPIO.output(DC,1)
#	spi.writebytes([color>>8, color&0xFF])
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
	for y in range (height, 0):
		for x in range (width, 0):
			WriteWord(color)
#	WriteBulk(color,width,height)
def FillScreen(color):
	"Fills entire screen with given color"
	FillRect(0,0,XMAX-1,YMAX-1,color)
def ClearScreen():
	"Fills entire screen with black"
	FillRect(0,0,XMAX-1,YMAX-1,BLACK)
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
	FillScreen(RED)
	FillScreen(MAGENTA)
	PrintElapsedTime('ScreenTest',startTime)
def RandRect():
	"Returns four integers x0,y0,x1,y1 as screen rect coordinates"
	x1 = randint(1,XMAX)
	y1 = randint(1,YMAX)
	dx = randint(30,X0)
	dy = randint(30,Y0)
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
		Line(10,10,randint(20,XMAX),randint(20,YMAX),YELLOW)
		Line(120,10,randint(2,XMAX),randint(10,YMAX),CYAN)
	PrintElapsedTime('LineTest',startTime)
def PixelTest(color=BLACK, numPixels=5000):
	"Writes random pixels to the screen"
	ClearScreen()
	startTime = time.time()
	for i in range(numPixels):
		xPos = randint(1,XMAX)
		yPos = randint(1,YMAX)
		DrawPixel(xPos,yPos,LIME)
	PrintElapsedTime('PixelTest',startTime)
def FastPixelTest(color=BLACK, numPixels=5000):
	"Writes random pixels to the screen"
	ClearScreen()
	startTime = time.time()
	for i in range(numPixels):
		xPos = randint(1,XMAX)
		yPos = randint(1,YMAX)
		FastDrawPixel(xPos,yPos,YELLOW)
	PrintElapsedTime('FastPixelTest',startTime)
def MoireTest():
	"Draws a series of concentric circles"
	ClearScreen()
	startTime = time.time()
	for radius in range(6,XMAX,2):
		Circle(X0,Y0,radius,YELLOW)
	PrintElapsedTime('MoireTest',startTime)
def CircleTest(numCycles=40):
	"draw a series of random circles"
	ClearScreen()
	startTime = time.time()
	for a in range(numCycles):
		x = randint(30,XMAX)
		y = randint(30,YMAX)
		radius = randint(10,XMAX)
		Circle(x,y,radius,RandColor())
	PrintElapsedTime('CircleTest',startTime)
def FillCircleTest(numCycles=40):
	"draw a series of random filled circles"
	ClearScreen()
	startTime = time.time()
	for a in range(numCycles):
		x = randint(30,XMAX)
		y = randint(30,YMAX)
		radius = randint(10,YMAX)
		FillCircle(x,y,radius,RandColor())
	PrintElapsedTime('FillCircleTest',startTime)
def RunTests():
	"run a series of graphics test routines & time them"
	ClearScreen()
	print "Cleared screen"
	time.sleep(2)
	startTime = time.time() #keep track of test duration
	ScreenTest() #fill entire screen with color
	print "Fill screen Test"
	time.sleep(2)
	RectTest() #draw rectangles
	print "Rect Test"
	time.sleep(2)
	FillRectTest() #draw filled rectangles
	print "FillRect Test"
	time.sleep(2)
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
print "2.2 TFT display demo with hardware SPI"
spi = InitSPI() #initialize SPI interface
InitGPIO() #initialize GPIO interface
InitDisplay() #initialize TFT controller
RunTests() #run suite of graphics tests
time.sleep(5)
spi.close() #close down SPI interface
print "Done."
# END ###############################################################
