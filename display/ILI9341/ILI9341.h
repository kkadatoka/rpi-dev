/*******************************************************************************
* Function Name  : main
* Description    : Driver for LCD HY28A-LCDB using:
*                  ILI9320 for LCD & ADS7843 for Touch Panel
* Input          : None
* Output         : None
* Return         : None
* Compile/link   : gcc -o spi -lrt main.c -lbcm2835 -lm
*                  gcc -o spi -lrt main.c -lbcm2835 -lm -mfloat-abi=hard -Wall
* Execute        : sudo ./spi
*******************************************************************************/
#ifndef ILI9341_H
#define ILI9341_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "fonts.h"
#include "SPI.h"

//Basic Colors
#define RED		0xf800
#define GREEN	0x07e0
#define BLUE	0x001f
#define BLACK	0x0000
#define YELLOW	0xffe0
#define WHITE	0xffff

//Other Colors
#define CYAN		0x07ff	
#define BRIGHT_RED	0xf810	
#define GRAY1		0x8410  
#define GRAY2		0x4208  

//TFT resolution 240*320
#define MIN_X	0
#define MIN_Y	0
#define MAX_X	239
#define MAX_Y	319


/*
  There are 2 arrow on lcd pcb one left one right of the glass; arrow means up
  start from landscape & rotate 90 degree clockwise

  0 landscape xy origin lower left corner
  1 & 2 not yet implemented
  3 portrait xy origin upper left corner
*/
#define LANDSCAPE 0
#define PORTRAIT 3

#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2


#define RGB565CONVERT(red, green, blue)\
(unsigned short)( (( red   >> 3 ) << 11 ) | \
(( green >> 2 ) << 5  ) | \
( blue  >> 3 ))

#define THRESHOLD 2   /* threshold */

extern INT8U simpleFont[][8];

class TFT
{

private:
	INT16U constrain (INT16U x, INT16U a, INT16U b)
	{
		if (a > x)
		{
			return a;
		}
		if (b > x)
		{
			return b;
		}
		return x;
	}

public:
	SPI spi;

    inline void sendCMD(INT8U index)
    {
        TFT_DC_LOW;
        TFT_CS_LOW;
        spi.transfer(index);
        TFT_CS_HIGH;
    }

    inline void WRITE_DATA(INT8U data)
    {
        TFT_DC_HIGH;
        TFT_CS_LOW;
        spi.transfer(data);
        TFT_CS_HIGH;
    }
    
    inline void sendData(INT16U data)
    {
        INT8U data1 = data>>8;
        INT8U data2 = data&0xff;
        TFT_DC_HIGH;
        TFT_CS_LOW;
        spi.transfer(data1);
        spi.transfer(data2);
        TFT_CS_HIGH;
    }

    void WRITE_Package(INT16U *data, INT8U howmany)
    {
        INT16U  data1 = 0;
        INT8U   data2 = 0;

        TFT_DC_HIGH;
        TFT_CS_LOW;
        INT8U count=0;
        for(count=0;count<howmany;count++)
        {
            data1 = data[count]>>8;
            data2 = data[count]&0xff;
            spi.transfer(data1);
            spi.transfer(data2);
        }
        TFT_CS_HIGH;
    }

    INT8U Read_Register(INT8U Addr, INT8U xParameter)
    {
        INT8U data=0;
        sendCMD(0xd9);                                                      /* ext command                  */
        WRITE_DATA(0x10+xParameter);                                        /* 0x11 is the first Parameter  */
        TFT_DC_LOW;
        TFT_CS_LOW;
        spi.transfer(Addr);
        TFT_DC_HIGH;
        data = spi.transfer(0);
        TFT_CS_HIGH;
        return data;
    }

    
	void TFTinit (void);
	void setCol(INT16U StartCol,INT16U EndCol);
	void setPage(INT16U StartPage,INT16U EndPage);
	void setXY(INT16U poX, INT16U poY);
    void setPixel(INT16U poX, INT16U poY,INT16U color);
	
	void fillScreen(INT16U XL,INT16U XR,INT16U YU,INT16U YD,INT16U color);
	void fillScreen(void);
	INT8U readID(void);
	
	void drawChar(INT8U ascii,INT16U poX, INT16U poY,INT16U size, INT16U fgcolor);
    void drawString(char *string,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
	void fillRectangle(INT16U poX, INT16U poY, INT16U length, INT16U width, INT16U color);
	
	void drawLine(INT16U x0,INT16U y0,INT16U x1,INT16U y1,INT16U color);
    void drawVerticalLine(INT16U poX, INT16U poY,INT16U length,INT16U color);
    void drawHorizontalLine(INT16U poX, INT16U poY,INT16U length,INT16U color);
    void drawRectangle(INT16U poX, INT16U poY, INT16U length,INT16U width,INT16U color);
	
	void drawCircle(int poX, int poY, int r,INT16U color);
    void fillCircle(int poX, int poY, int r,INT16U color);
	
	void drawTraingle(int poX1, int poY1, int poX2, int poY2, int poX3, int poY3, INT16U color);
    INT8U drawNumber(long long_num,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
    INT8U drawFloat(float floatNumber,INT8U decimal,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
    INT8U drawFloat(float floatNumber,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);

};

extern TFT Tft;

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
