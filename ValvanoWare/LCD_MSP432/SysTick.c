// SysTick.c
// Runs on MSP432
// Provide functions that initialize the SysTick module, wait at least a
// designated number of clock cycles, and wait approximately a multiple
// of 10 milliseconds using busy wait.  After a power-on-reset, the
// MSP432 gets its clock from the internal digitally controlled
// oscillator, which is set to 3 MHz by default.  One distinct advantage
// of the MSP432 is that it has low-power clock options to reduce power
// consumption by reducing clock frequency.  This matters for the
// function SysTick_Wait10ms(), which will wait longer than 10 ms if the
// clock is slower.
// Daniel Valvano
// May 10, 2015

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

#include <stdint.h>
#include "msp432p401r.h"


// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  SYSTICK_STCSR = 0;                    // disable SysTick during setup
  SYSTICK_STRVR = 0x00FFFFFF;           // maximum reload value
  SYSTICK_STCVR = 0;                    // any write to current clears it
  SYSTICK_STCSR = 0x00000005;           // enable SysTick with no interrupts
}
// Time delay using busy wait.
// The delay parameter is in units of the core clock. (units of 20.83 nsec for 48 MHz clock)
void SysTick_Wait(uint32_t delay){
  // method #1: set Reload Value Register, clear Current Value Register, poll COUNTFLAG in Control and Status Register
  if(delay <= 1){
    // without this step:
    // if delay == 0, this function will wait 0x00FFFFFF cycles
    // if delay == 1, this function will never return (because COUNTFLAG is set on 1->0 transition)
    return;                   // do nothing; at least 1 cycle has already passed anyway
  }
  SYSTICK_STRVR = (delay - 1);// count down to zero
  SYSTICK_STCVR = 0;          // any write to CVR clears it and COUNTFLAG in CSR
  while((SYSTICK_STCSR&0x00010000) == 0){};
  // method #2: repeatedly evaluate elapsed time
/*  volatile uint32_t elapsedTime;
  uint32_t startTime = SYSTICK_STCVR;
  do{
    elapsedTime = (startTime-SYSTICK_STCVR)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);*/
}
// Time delay using busy wait.
// This assumes 48 MHz system clock.
void SysTick_Wait10ms(uint32_t delay){
  uint32_t i;
  for(i=0; i<delay; i++){
    SysTick_Wait(480000);  // wait 10ms (assumes 48 MHz clock)
  }
}
