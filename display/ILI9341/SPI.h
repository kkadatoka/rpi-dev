#include <bcm2835.h>


#define SPI_START (0x70)   /* Start byte for SPI transfer */
#define SPI_RD (0x01)      /* WR bit 1 within start */
#define SPI_WR (0x00)      /* WR bit 0 within start */
#define SPI_DATA (0x02)    /* RS bit 1 within start byte */
#define SPI_INDEX (0x00)   /* RS bit 0 within start byte */

#define DIVIDER_CS0 BCM2835_SPI_CLOCK_DIVIDER_8
#define DIVIDER_CS1 BCM2835_SPI_CLOCK_DIVIDER_64

/* GPIOs */
#define RESET		RPI_V2_GPIO_P1_15	// GPIO 22
#define DC			RPI_V2_GPIO_P1_22	// GPIO 25
#define CS			RPI_V2_GPIO_P1_16	// GPIO 23
#define BACKLIGHT	RPI_V2_GPIO_P1_12	// GPIO18
#define IRQ			RPI_V2_GPIO_P1_18	// GPIO24

#ifndef INT8U
#define INT8U unsigned char
#endif
#ifndef INT16U
#define INT16U unsigned int
#endif



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

#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2

#define TFT_CS_LOW  spi.setCSLow()
#define TFT_CS_HIGH spi.setCSHigh()
#define TFT_DC_LOW  spi.setDCLow()
#define TFT_DC_HIGH spi.setDCHigh()
#define TFT_BL_OFF  spi.setBLOff()
#define TFT_BL_ON   spi.setBLOn()


extern INT8U simpleFont[][8];


class SPI
{
private:
	int pDC;
	int pReset ;
	int pBL ;
	int pCS ;
	int nMode ;
	int nSpeed ;
	void Close()
	{
		bcm2835_gpio_write(pBL, LOW);   //HIGH=on, LOW=off;
		bcm2835_spi_end();
		bcm2835_close();
	}
	void setupPins();
public:
	SPI(int mode=3, int speed=0, int _pin_dc=DC, int _pin_reset=RESET, int _pin_backlight=BACKLIGHT, int _pin_cs=CS)
	{
		pDC = _pin_dc;
		pReset = _pin_reset;
		pBL = _pin_backlight;
		pCS = _pin_cs;
		nMode = mode;
		nSpeed = speed;
	}
	~SPI()
	{
		Close();
	}
	int begin();
	INT8U transfer(INT8U data);
	void setCSLow() ;
	void setCSHigh() ;
	void setDCLow() ;
	void setDCHigh() ;
	void setBLOff() ;
	void setBLOn() ;
	void Reset() ;
};

extern SPI Spi;
