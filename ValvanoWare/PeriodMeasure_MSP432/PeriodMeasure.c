// PeriodMeasure.c
// Runs on MSP432
// Use Timer A0 in 16-bit capture mode to request interrupts on the
// rising edge of P7.3 (TA0CCP0), and measure period between pulses.
// Daniel Valvano
// July 27, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Example 7.2, Program 7.2

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

// external signal connected to P7.3 (TA0CCP0) (trigger on rising edge)

#include <stdint.h>
#include "..\inc\msp432p401r.h"
#include "ClockSystem.h"
#include "InputCapture.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

uint16_t Period;              // (1/SMCLK) units = 83.3 ns units
uint16_t First;               // Timer A0 first edge
int Done;                     // set each rising
// max period is (2^16-1)*83.3 ns = 5.4612 ms
// min period determined by time to run ISR, which is about 1 us
void PeriodMeasure(uint16_t time){
  P2OUT = P2OUT^0x02;              // toggle P2.1
  Period = (time - First)&0xFFFF;  // 16 bits, 83.3 ns resolution
  First = time;                    // setup for next
  Done = 1;
}

int main(void){
  First = 0;                       // first will be wrong
  Done = 0;                        // set on subsequent
  // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in RGB LEDs)
  P2SEL0 &= ~0x07;
  P2SEL1 &= ~0x07;                 // configure built-in RGB LEDs as GPIO
  P2DS |= 0x07;                    // make built-in RGB LEDs high drive strength
  P2DIR |= 0x07;                   // make built-in RGB LEDs out
  P2OUT &= ~0x07;                  // RGB = off
  Clock_Init48MHz();               // 48 MHz clock; 12 MHz Timer A clock
  TimerCapture_Init(&PeriodMeasure);// initialize Timer A0 in capture mode
  while(1){
    WaitForInterrupt();
  }
}
