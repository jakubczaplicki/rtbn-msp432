// Squarewaves.c
// Runs on MSP432
// Initialize P2.1 and P2.2 as outputs with different initial values,
// then toggle them to produce two out of phase square waves.
// Daniel Valvano
// April 23, 2015
/*
 This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
  Program 4.4

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

// built-in LED1 connected to P1.0
// negative logic built-in Button 1 connected to P1.1
// negative logic built-in Button 2 connected to P1.4
// built-in red LED connected to P2.0
// built-in green LED connected to P2.1
// built-in blue LED connected to P2.2

#include <stdint.h>
#include "..\inc\msp432p401r.h"

int main(void){
  P2SEL0 &= ~0x06;
  P2SEL1 &= ~0x06;            // configure built-in RGB LEDs as GPIO
  P2DS |= 0x06;               // make built-in RGB LEDs high drive strength
  P2DIR |= 0x06;              // make built-in RGB LEDs out
  P2OUT |= 0x02;              // P2.1 on (green LED on)
  P2OUT &= ~0x04;             // P2.2 off (blue LED off)
  while(1){
    P2OUT ^= 0x06;            // toggle P2.1 and P2.2
  }
}

// Color    LED(s) Port2
// dark     ---    0
// red      R--    0x01
// blue     --B    0x04
// green    -G-    0x02
// yellow   RG-    0x03
// sky blue -GB    0x06
// white    RGB    0x07
// pink     R-B    0x05
