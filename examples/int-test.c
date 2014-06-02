#include <stdio.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <mcp23017.h>
#include "../wiringPi/wiringPi/mcp23x0817.h"
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define I2C_ADDR = 0x24;


int g_Interrupt = 0;
int g_ShouldRun = 1;
int g_EndBlink = 0 ;

struct threadParams
{
	pthread_t tid ;
	int fd ;
	int old;
};


void ServiceInt()
{
	// Set flag and exit
	printf("\n\nKernel Interrupt callback handler!\n\n") ;
	g_Interrupt = 1;	
}

void sig_handler()
{
	g_ShouldRun = 0;
	g_EndBlink=1 ;
	g_Interrupt = 0;
	printf("Received SIGINT...quiting...\n") ;
}

void bin_prnt_byte(int x)
{
   int n;
   for(n=0; n<8; n++)
   {
      if((x & 0x80) !=0)
      {
         printf("1");
      }
      else
      {
         printf("0");
      }
      if (n==3)
      {
         printf(" "); /* insert a space between nybbles */
      }
      x = x<<1;
   }
   printf ("\n") ;
}

void invLED(struct threadParams* par, int value)
{
	printf ("Inv LED!\n") ;
	int pin=5 ;
	int bit = 1 << (pin & 7) ;

	if (value == LOW)
	  par->old &= (~bit) ;
	else
	  par->old |=   bit ;
	wiringPiI2CWriteReg8 (par->fd, MCP23x17_GPIOA, par->old);
	printf("Inverted LED\n") ;
}

void* myThread(void* param)
{
	unsigned int nInt = 0;
	struct threadParams* p = (struct threadParams*) param;
	int fd = p->fd ;
	unsigned int nStamp;
	int tid = pthread_self() ;
	int value=HIGH ;
	int longpress = 0;
	// set up to generate interrupts on INTB on pin B0 <> default value (1)
	//wiringPiI2CWriteReg8 (fd, MCP23x17_IODIRB, 0x03) ;
	wiringPiI2CWriteReg8 (fd, MCP23x17_IOCONB, 0b01101100) ; 
	// 7. register banks same, 6.mirror interrupts, 5.enable sequential mode, 4.DSSLW enabled, 3.HAEN=1, 2.open drain=0, 1.active-low, 0.unimpl
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_IOCONB ); // Read
	printf("IOCONB="); bin_prnt_byte(nInt);
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_INTCONB ); // Read
	printf("INTCONB(Before)="); bin_prnt_byte(nInt);
	wiringPiI2CWriteReg8 (fd, MCP23x17_INTCONB, 0b00000001) ; 
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_INTCONB ); // Read
	printf("INTCONB="); bin_prnt_byte(nInt);
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPPUB, 0x01) ; // pull-up resistor for switches - 5 B ports
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_GPPUB ); // Read
	printf("GPPUB="); bin_prnt_byte(nInt);
	wiringPiI2CWriteReg8 (fd, MCP23x17_IPOLB, 0x01) ; // invert polarity of signal
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_IPOLB ); // Read
	printf("IOPOLB="); bin_prnt_byte(nInt);
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPINTENB, 0x01) ; // enable interrupts
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_GPINTENB ); // Read
	printf("GPINTENB="); bin_prnt_byte(nInt);
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_INTCAPB ); // clear interrupt flag
	printf("INTCAPB-") ;
	bin_prnt_byte(nInt) ;
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_GPIOB ); // clear interrupt 
	printf("GPIOB="); bin_prnt_byte(nInt);

	printf("Port B setup completed.\n") ;

	//wiringPiI2CWriteReg8 (fd, MCP23x17_IODIRB, 0x1f) ;
	//wiringPiI2CWriteReg8 (fd, MCP23x17_INTCONB, 0x1f) ;
	//wiringPiI2CWriteReg8 (fd, MCP23x17_DEFVALB, 0x1f) ;
	//wiringPiI2CWriteReg8 (fd, MCP23x17_GPINTENB, 0x1f) ;
	//wiringPiI2CReadReg8 (fd, MCP23x17_INTCAPB ); // clear interrupt flag
	printf("%d: Interrupt thread started...\n", tid) ;
	while (g_ShouldRun)
	{
		if (g_Interrupt)
		{
			nStamp = millis() ;
			printf("\n\nInterrupt handling @ %d\n", nStamp) ;
			while (wiringPiI2CReadReg8 (fd, MCP23x17_GPIOB) == 0x01 ) 
			{
				// Loop to detect long press
				//break ;
				printf(".") ;
				delay (100) ;
			}
			g_Interrupt = 0;
			printf("\n") ;

			longpress = (millis() - nStamp);

			if ( longpress <= 500)
				g_EndBlink = !(g_EndBlink);
			else
			{
				printf("Time interval (for longpress) - %d\n", longpress) ;
				invLED(p, value) ;
				value = !value;
			}
			printf("Processed in %dms\n",longpress) ;
			delay (100); //debounce
			printf("Clearing registers...\n") ;
			nInt = wiringPiI2CReadReg8 (fd, MCP23x17_INTCAPB ); // clear interrupt 
			printf("INTCAPB="); bin_prnt_byte(nInt);
			nInt = wiringPiI2CReadReg8 (fd, MCP23x17_GPIOB ); // clear interrupt 
			printf("GPIOB="); bin_prnt_byte(nInt);

//			nInt = wiringPiI2CReadReg8 (fd, MCP23x17_GPINTENB ); // Read
//			printf("GPINTENB(Before)="); bin_prnt_byte(nInt);
//			wiringPiI2CWriteReg8 (fd, MCP23x17_GPINTENB, 0b00000001) ; 
//			nInt = wiringPiI2CReadReg8 (fd, MCP23x17_GPINTENB ); // Read
//			printf("GPINTENB="); bin_prnt_byte(nInt);
//			nInt = wiringPiI2CReadReg8 (fd, MCP23x17_INTCAPB ); // clear interrupt 
//			printf("INTCAPB="); bin_prnt_byte(nInt);
//			nInt = wiringPiI2CReadReg8 (fd, MCP23x17_GPIOB ); // clear interrupt 
//			printf("GPIOB="); bin_prnt_byte(nInt);
			
		}
		delay(500);
	}
	printf("%d: Interrupt thread cleaning up...\n", tid) ;

	return NULL;
}



void* blinkerThread(void* param)
{
	struct threadParams * p = (struct threadParams *) param;
	int tid = pthread_self() ;
	int pin6=6, pin7=7;
	int bit6 = 1 << (pin6 & 7) ;
	int bit7 = 1 << (pin7 & 7) ;
	int fd = p->fd ;
	int value = LOW;
//	int old = p->old ;
	printf("%d: Blinker thread started...\n", tid) ;

	wiringPiI2CWriteReg8 (fd, MCP23x17_IOCON, 0b01100100) ; // mirror interrupts, disable sequential mode, open drain
	unsigned int nInt = wiringPiI2CReadReg8 (fd, MCP23x17_IOCON ); // Read
	printf("IOCONA="); bin_prnt_byte(nInt);
	wiringPiI2CWriteReg8 (fd, MCP23x17_IODIRA, 0x0) ; // Set direction as output - 5 B ports
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_IODIRA ); // Read
	printf("IODIRA="); bin_prnt_byte(nInt);
	wiringPiI2CWriteReg8 (fd, MCP23x17_IPOLA, 0x0) ; // Set POLARITY bit same as GPIO
	nInt = wiringPiI2CReadReg8 (fd, MCP23x17_IPOLA ); // Read
	printf("IPOLA="); bin_prnt_byte(nInt);
	p->old = wiringPiI2CReadReg8 (fd, MCP23x17_OLATA ); // Read
	printf("OLATA="); bin_prnt_byte(nInt);
	printf ("Port A setup completed.\n") ;

	while (g_ShouldRun)
	{
		while(!g_EndBlink)
		{
			if (value == LOW)
			  p->old &= (~bit6) ;
			else
			  p->old |=   bit6 ;
			wiringPiI2CWriteReg8 (fd, MCP23x17_GPIOA, p->old);
			delay(1000);
			if (value == LOW)
			  p->old &= (~bit7) ;
			else
			  p->old |=   bit7 ;
			wiringPiI2CWriteReg8 (fd, MCP23x17_GPIOA, p->old);
			value = !value ;
			//printf ("Value=") ; bin_prnt_byte(old) ;
		}
		wiringPiI2CWriteReg8 (fd, MCP23x17_GPIOA, 0);
		delay (1000); //grace sleep
	}
	printf("%d: Blinker thread: Cleaning and exiting...\n", tid) ;
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPIOA, 0);

	return NULL;
}



int main(void) 
{
//	int i, mask1, mask2, mask3 ;
//	char * filename = "/usr/local/bin/gpio" ;
	char command[254] ;
	struct threadParams thrPrm1, thrPrm2;
	
	wiringPiSetup() ;

	strcpy(command, "gpio load i2c 400") ;
	system (command) ;

	int fd = wiringPiI2CSetup (0x24) ;
	if (!fd)
	{
		printf ("Failed to initialize I2C\n");
		return 1;
	}
	thrPrm1.fd = fd;
	thrPrm2.fd = fd;

	signal ( SIGINT, sig_handler) ;

	// wiringPiI2CWriteReg8 (fd, MCP23x17_IOCON, IOCON_INIT) ;
//	wiringPiI2CWriteReg8 (fd, MCP23x17_IOCON, 0b01100100) ; // mirror interrupts, disable sequential mode, open drain
//	unsigned int nInt = wiringPiI2CReadReg8 (fd, MCP23x17_IOCON ); // Read
//	printf("IOCONA="); bin_prnt_byte(nInt);


//	thrPrm1.old = wiringPiI2CReadReg8 (fd, MCP23x17_OLATA) ;
//	thrPrm2.old = wiringPiI2CReadReg8 (fd, MCP23x17_OLATB) ;

//	bin_prnt_byte(thrPrm1.old);
//	bin_prnt_byte(thrPrm2.old);

	// set up on INTA on pin 5,6,7 as output`
	// wiringPiI2CWriteReg8 (fd, MCP23x17_IODIRA, 0xE0) ;
	strcpy(command, "gpio edge 0 falling") ;
	system (command) ;

	pinMode( 0, INPUT) ;
	pullUpDnControl (0, PUD_UP) ;
	//digitalWrite (0, HIGH) ;

	pthread_create( &thrPrm1.tid, NULL, &myThread, &thrPrm1) ;
	pthread_create( &thrPrm2.tid, NULL, &blinkerThread, &thrPrm2) ;

	printf("Thread created...\n") ;

	wiringPiISR( 0, INT_EDGE_FALLING, &ServiceInt) ;

	printf("Created ISR...\n") ;
	

	printf("Waiting for threads to finish...\n") ;

	pthread_join(thrPrm1.tid, NULL) ;
	pthread_join(thrPrm2.tid, NULL) ;
	
//	strcpy(command,"gpio reset") ;
//	system (command) ;
	printf("Exiting...\n") ;
	
	return EXIT_SUCCESS;
}