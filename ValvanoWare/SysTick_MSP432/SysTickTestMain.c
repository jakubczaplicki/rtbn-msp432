// SysTickTestMain.c
// Runs on MSP432
// Test the SysTick functions by initializing the SysTick timer and
// flashing an LED at a constant rate.
// Daniel Valvano
// July 2, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7

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
// positive logic switch connected to P1.5
// built-in red LED connected to P2.0
// built-in green LED connected to P2.1
// built-in blue LED connected to P2.2

#include <stdint.h>
#include "..\inc\msp432p401r.h"
#include "SysTick.h"

int main(void){
  SysTick_Init();             // initialize SysTick timer
  // initialize P2.2 and make it output (P2.2 built-in blue LED)
  P2SEL0 &= ~0x04;
  P2SEL1 &= ~0x04;            // configure built-in blue LED as GPIO
  P2DS |= 0x04;               // make built-in blue LED high drive strength
  P2DIR |= 0x04;              // make built-in blue LED out
  while(1){
    P2OUT ^= 0x04;            // toggle P2.2
//    SysTick_Wait(1);          // approximately 29 us(*)
//    SysTick_Wait(2);          // approximately 29 us(*)
//    SysTick_Wait(600);        // approximately 0.23 ms(*)
//    SysTick_Wait10ms(1);      // approximately 10 ms(*)
    SysTick_Wait10ms(50);      // approximately 500 ms
  }
}

// (*) Note: Durations are measurements taken with the PicoScope and include loop overhead.
