// PWMSine.c
// Runs on MSP432
// Use Timer A1 in periodic mode to request interrupts at a
// particular period.
// In those interrupts, change the PWM duty cycle to produce
// a sine wave.
// Daniel Valvano
// July 30, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Program 6.8, Section 6.3.2
   Program 8.7, Example 8.4

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Program 8.4, Section 8.3

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

// oscilloscope connected to P2.4/TA0.1 for PWM output
// P2.1 toggles on each timer interrupt (flash at frequency of sine wave * 32 / 2)

#include <stdint.h>
#include "ClockSystem.h"
#include "PWM.h"
#include "TimerA1.h"
#include "..\inc\msp432p401r.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
uint32_t StartCritical (void);// previous I bit, disable interrupts
void EndCritical(uint32_t sr);// restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

const uint16_t Wave[32] = {
  30,35,40,44,48,51,53,55,55,55,53,
  51,48,44,40,35,30,25,20,16,12,9,
  7,5,5,5,7,9,12,16,20,25
};
void OutputSineWave(void){
  static uint8_t index = 0;        // counting index of output sequence
  PWM_Duty1(Wave[index]);          // output next value in sequence
  index = (index + 1)&0x1F;        // increment counter
  P2OUT ^= 0x02;
}
int main(void){
  Clock_Init48MHz();               // 48 MHz
  // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in RGB LEDs)
  P2SEL0 &= ~0x07;
  P2SEL1 &= ~0x07;                 // configure built-in RGB LEDs as GPIO
  P2DS |= 0x07;                    // make built-in RGB LEDs high drive strength
  P2DIR |= 0x07;                   // make built-in RGB LEDs out
  P2OUT &= ~0x07;                  // RGB = off
  PWM_Init(60, 30);                // initialize PWM, 100kHz, 50% duty
//  TimerA1_Init(&OutputSineWave, 852);// initialize 440 Hz sine wave output
  TimerA1_Init(&OutputSineWave, 375);// initialize 1000 Hz sine wave output
  while(1){
    WaitForInterrupt();
  }
}
