#include "SPI.h"

int SPI::begin()
{
	if(!bcm2835_init()) return 0;

	bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // MSB The default
    bcm2835_spi_setDataMode(nMode);								// MODE3
    bcm2835_spi_setClockDivider(DIVIDER_CS0);                     // 16 The default 4096 = 32MHz
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
	setupPins();
	setBLOn();
	return 1;
}

void SPI::setupPins()
{
    bcm2835_gpio_fsel(pReset, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pDC, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pBL, BCM2835_GPIO_FSEL_OUTP);
}

void SPI::setCSLow()
{
	bcm2835_gpio_write(pCS, LOW);   //HIGH=on, LOW=off;
}

void SPI::setCSHigh()
{
	bcm2835_gpio_write(pCS, HIGH);   //HIGH=on, LOW=off;
}

void SPI::setDCLow() 
{
	bcm2835_gpio_write(pDC, LOW);   //HIGH=on, LOW=off;
}

void SPI::setDCHigh() 
{
	bcm2835_gpio_write(pCS, HIGH);   //HIGH=on, LOW=off;
}

void SPI::setBLOff()
{
	bcm2835_gpio_write(pBL, LOW);   //HIGH=on, LOW=off;
}

void SPI::setBLOn() 
{
	bcm2835_gpio_write(pBL, HIGH);   //HIGH=on, LOW=off;
}

void SPI::Reset() 
{
	bcm2835_gpio_write(pReset, LOW);   //HIGH=on, LOW=off;
	bcm2835_gpio_write(pReset, HIGH);   //HIGH=on, LOW=off;
}

INT8U SPI::transfer(INT8U data)
{
	return bcm2835_spi_transfer(data) ;

}

SPI Spi=SPI();