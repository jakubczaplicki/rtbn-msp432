// main.c
// Runs on MSP432
// UART runs at 115,200 baud rate
// Daniel Valvano
// April 15, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Section 4.5

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

//*****BUG*BUG*BUG*BUG****in CCS scanf drops the first character*****BUG*BUG*BUG*

#include <stdio.h>
#include <stdint.h> // C99 variable types
#include "uart.h"


void BookExamples(void){ // examples from the book
  int8_t cc = 0x56; // (‘V’)
  int32_t xx = 100;
  int16_t yy = -100;
  float zz = 3.14159265;
  printf("Hello world\n\r");  //Hello world
  printf("cc = %c %d %#x\n\r",cc,cc,cc);  //cc = V 86 0x56
  printf("xx = %c %d %#x\n\r",xx,xx,xx);  //xx = d 100 0x64
  printf("yy = %d %#x\n\r",yy,yy);  //yy = -100 0xffffff9c
  printf("zz = %e %E %f %g %3.2f\n\r",zz,zz,zz,zz,zz);  //zz = 3.14159 3.14
}
int main(void){ int32_t i,n;
  Output_Init();              // initialize output device

  BookExamples();
  n = 0;
  while(1){

    printf("\n\ri= ");
    for(i=0; i<5; i++){
      printf("%d ",i+n);
    }

    n = n+1000000; // notice what happend when this goes above 2,147,483,647
  }
}
