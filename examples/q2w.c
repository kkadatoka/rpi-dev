/*
 * q2w.c:
 *      Using the Quick 2 wire board for its mcp23017
 *
 * Copyright (c) 2012-2013 Gordon Henderson. <projects@drogon.net>
 ***********************************************************************
 */

#include <stdio.h>
#include <wiringPi.h>
#include <mcp23017.h>

int main (void)
{
  int i, bit ;

  wiringPiSetup () ;
  mcp23017Setup (100, 0x24) ;

  printf ("Raspberry Pi - MCP23017 Test\n") ;

  for (i = 6 ; i < 8 ; i++)
    pinMode (100 + i, OUTPUT) ;

  pinMode         (100 + 0, INPUT) ;
  pullUpDnControl (100 + 0, PUD_UP) ;

  for (;;)
  {
    for (i = 0 ; i < 1024 ; ++i)
    {
      for (bit = 6 ; bit < 8 ; bit++)
        digitalWrite (100 + bit, i & (1 << bit)) ;
      delay (5) ;
      if (digitalRead (100 + 0) == 0)
      {
	      printf("Got Key\n");
	      delay (1) ;
	      digitalWrite(100 + 6, 0);
	      digitalWrite(100+7,0) ;
	      printf("Exiting now...\n");
	      return 0;
      }
    }
  }
  return 0 ;
}
