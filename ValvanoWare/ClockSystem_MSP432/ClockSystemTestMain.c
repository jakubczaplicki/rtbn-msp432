// ClockSystemTestMain.c
// Runs on MSP432
// Test the Clock System initialization to verify that the
// system clock is running at the expected rate.  Use the
// debugger if possible or an oscilloscope connected to P2.2.
// When using an oscilloscope to look at LED2, it should be
// clear to see that the LED flashes about 4 (12/3) times
// faster with a 12 MHz clock than with the default 3 MHz
// clock.
// The operation of the Clock System can be tested even more
// precisely by using an oscilloscope to measure P4.3, which
// is configured to output the main clock signal.  The main
// clock is used by the CPU and peripheral module interfaces
// and can be used directly by some peripheral modules.  In
// this case and by default, this is the DCO frequency
// divided by 1.  P4.2 is configured to output the auxiliary
// clock signal.  The auxiliary clock is used by individual
// peripheral modules that select it.  In this case, this is
// the internal, low-power low-frequency oscillator REFOCLK,
// which is 32,768 Hz in this case and by default.  REFOCLK
// can be 128,000 Hz by setting bit 15 of CSCLKEN register.
// In this case and by default, the auxiliary clock is
// divided by 1.  P4.4 is configured to output the subsystem
// master clock signal.  The subsystem master clock (HSMCLK)
// and low-speed subsystem master clock (SMCLK) get their
// clocks from the same source.  By default, this is the DCO
// frequency divided by 1 for both (although both the HSMCLK
// and SMCLK can be programmed with different dividers).
// Both subsystem master clocks are used by individual
// peripheral modules that select them.
// Daniel Valvano
// June 30, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Program 4.6

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

// auxiliary clock output connected to P4.2
// main clock output connected to P4.3
// subsystem master clock output connected to P4.4

#include <stdint.h>
#include "ClockSystem.h"
#include "..\inc\msp432p401r.h"

// delay function for testing
// which delays about 8.1*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code
  void Delay(unsigned long ulCount){
  __asm (  "dloop:   subs    r0, #1\n"
      "    bne     dloop\n");
}

#else
  //Keil uVision Code
  __asm void
  Delay(unsigned long ulCount)
  {
    subs    r0, #1
    bne     Delay
    bx      lr
  }

#endif
  
  
#define SCALE 16
// test
// DCO1_5MHz  SCALE=1/2 blue LED flashs 1Hz
// DCO3MHz    SCALE=1   blue LED flashs 1Hz
// DCO6MHz    SCALE=2   blue LED flashs 1Hz
// DCO12MHz   SCALE=4   blue LED flashs 1Hz
// Init48MHz  SCALE=16  blue LED flashs 1Hz
int main(void){
//  Clock_Init(DCO12MHz);        // configure for 12 MHz clock
//  Clock_Init48MHz();           // configure for 48 MHz clock
//  Clock_Init32kHz();           // configure for 32 kHz clock
  Clock_InitLowPower();        // configure for 32 kHz clock
  // initialize P2.2 and make it output (P2.2 built-in blue LED)
  P2SEL0 &= ~0x04;
  P2SEL1 &= ~0x04;             // configure built-in blue LED as GPIO
  P2DS |= 0x04;                // make built-in blue LED high drive strength
  P2DIR |= 0x04;               // make built-in blue LED out
  // initialize P4.4-P4.2 and make them clock outputs
  P4SEL0 |= 0x1C;
  P4SEL1 &= ~0x1C;             // configure P4.4-P4.2 as primary module function
  P4DIR |= 0x1C;               // make P4.4-P4.2 out
  while(1){
    P2OUT |= 0x04;             // turn on LED2 (blue)
//    Delay(419);                // delay ~1.136 ms at 3 MHz
//    Delay(184543*SCALE);       // delay ~0.5 sec at 3 MHz
    Delay(2016);               // delay ~0.5 sec at 32,768 Hz
    P2OUT &= ~0x04;            // turn off LED2 (blue)
//    Delay(419);                // delay ~1.136 ms at 3 MHz
//    Delay(184543*SCALE);       // delay ~0.5 sec at 3 MHz
    Delay(2016);               // delay ~0.5 sec at 32,768 Hz
  }
}
