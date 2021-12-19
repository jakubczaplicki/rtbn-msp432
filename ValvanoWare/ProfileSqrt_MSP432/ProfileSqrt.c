// ProfileSqrt.c
// Runs on MSP432
// Daniel Valvano
// July 1, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.12

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

// P2.2 is an output for debugging

#include <stdint.h>
#include "..\inc\msp432p401r.h"

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  SYSTICK_STCSR = 0;                    // disable SysTick during setup
  SYSTICK_STRVR = 0x00FFFFFF;           // maximum reload value
  SYSTICK_STCVR = 0;                    // any write to current clears it
  SYSTICK_STCSR = 0x00000005;           // enable SysTick with no interrupts
}
// Newton's method
// s is an integer
// sqrt(s) is an integer
uint32_t sqrt(uint32_t s){
uint32_t t;   // t*t will become s
int n;             // loop counter
  t = s/16+1;      // initial guess
  for(n = 16; n; --n){ // will finish
    t = ((t*t+s)/t)/2;
  }
  return t;
}
uint32_t before,elapsed,ss,tt;
int main(void){
  SysTick_Init();             // initialize SysTick timer, Program 4.7
  // initialize P2.2 and make it output (P2.2 built-in blue LED)
  P2SEL0 &= ~0x04;
  P2SEL1 &= ~0x04;            // configure built-in blue LED as GPIO
  P2DS |= 0x04;               // make built-in blue LED high drive strength
  P2DIR |= 0x04;              // make built-in blue LED out
  ss = 230400;
  before = SYSTICK_STCVR;
//  tt = sqrt(ss);
  elapsed = (before - SYSTICK_STCVR - 16)&0x00FFFFFF;
  // the number 16 depends on the instructions before and after test
  // if you remove the call to sqrt, elapsed measures 0

  while(1){
    ss = 230400;
    P2OUT |= 0x04;            //  P2.2=1
    tt = sqrt(ss);
    P2OUT &= ~0x04;           //  P2.2=0
  }
}

#define P23OUT   (*((volatile uint8_t *)(0x4209806C)))
#define Debug_Set()   (P23OUT = 0x01)
#define Debug_Clear() (P23OUT = 0x00)
int main2(void){uint32_t volatile Out;
  P2SEL0 &= ~0x08;
  P2SEL1 &= ~0x08;            // configure as GPIO
  P2DIR |= 0x08;              // debugging output
  while(1){
    ss = 230400;
    Debug_Set();      // Program 3.15
    Out = sqrt(ss);
    Debug_Clear();    // Program 3.15
  }
}
uint32_t sqrt3(uint32_t s){
uint32_t t;  // t*t becomes s
int n;            // loop counter 
  P4OUT = 1;
  t = s/10+1;     // initial guess 
  P4OUT = 2;
  for(n = 16; n; --n){  // will finish
    P4OUT = 4;
    t = ((t*t+s)/t)/2;  
    P4OUT = 8;
  }
  P4OUT = 0;
  return t; 
}  

int main3(void){

  // initialize P4.3 - P4.0 and make it output 
  P4SEL0 &= ~0x0F;
  P4SEL1 &= ~0x0F;            // configure as GPIO
  P4DIR |= 0x0F;              // make out
  while(1){
    ss = 230400;
    tt = sqrt3(ss);
  }
}
