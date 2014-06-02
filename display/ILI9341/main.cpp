#include <stdint.h>
#include <unistd.h>
#include "ILI9341.h"
//#include "SPI.h"

int main()
{
    
    Tft.TFTinit();                                      //init TFT library             

    Tft.drawCircle(100, 100, 30,YELLOW);                //center: (100, 100), r = 30 ,color : YELLOW              
    
    Tft.drawCircle(100, 200, 40,CYAN);                  // center: (100, 200), r = 10 ,color : CYAN  
    
    Tft.fillCircle(200, 100, 30,RED);                   //center: (200, 100), r = 30 ,color : RED    
    
    Tft.fillCircle(200, 200, 30,BLUE);                  //center: (200, 200), r = 30 ,color : BLUE       
	sleep (5);
	return 0;
}

