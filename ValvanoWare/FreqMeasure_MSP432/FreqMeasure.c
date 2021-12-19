// FreqMeasure.c
// Runs on MSP432
// Measures frequency on P7.1/TA0CLK input
// Timer A1 in periodic mode to request interrupts at 100 Hz
// Daniel Valvano,  Jonathan Valvano
// July 30, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Program 6.9, example 6.8

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
#include "..\inc\msp432p401r.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

/* Set each measurement, every 10 ms */
uint32_t Freq;   /* Frequency with units of 100 Hz */
int Done;

// ***************** FreqMeasure_Init ****************
// Activate Timer A1 interrupts to take measurements every
// 10 ms by counting rising edges on P7.1/TA0CLK
// Assumes Clock_Init48MHz() has been called
// Inputs: none
// Outputs: none
void FreqMeasure_Init(void){long sr;
  sr = StartCritical();
  // ***************** Timer A0 initialization *****************
  P7SEL0 |= 0x02;
  P7SEL1 &= ~0x02;                 // configure P7.1 as TA0CLK
  P7DIR &= ~0x02;                  // make P7.1 in (TA0CLK not C0OUT)
  TA0CTL &= ~0x0030;               // halt Timer A0
  // bits15-10=XXXXXX, reserved
  // bits9-8=00,       clock source to TA0CLK
  // bits7-6=00,       input clock divider /1
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=0,           interrupt disable
  // bit0=0,           clear interrupt pending
  TA0CTL = 0x0000;
  // bits15-14=00,     no capture mode
  // bits13-12=XX,     capture/compare input select
  // bit11=X,          synchronize capture source
  // bit10=X,          synchronized capture/compare input
  // bit9=X,           reserved
  // bit8=0,           compare mode
  // bits7-5=XXX,      output mode
  // bit4=0,           disable capture/compare interrupt
  // bit3=X,           read capture/compare input from here
  // bit2=0,           output this value in output mode 0
  // bit1=X,           capture overflow status
  // bit0=0,           clear capture/compare interrupt pending
  TA0CCTL0 = 0x0000;
  TA0EX0 &= ~0x0007;               // configure for input clock divider /1
  // ***************** Timer A1 initialization *****************
  TA1CTL &= ~0x0030;               // halt Timer A1
  // bits15-10=XXXXXX, reserved
  // bits9-8=10,       clock source to SMCLK
  // bits7-6=01,       input clock divider /2
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=0,           rollover does not interrupt
  // bit0=0,           clear interrupt pending
  TA1CTL = 0x0240;
  // bits15-14=00,     no capture mode
  // bits13-12=XX,     capture/compare input select
  // bit11=X,          synchronize capture source
  // bit10=X,          synchronized capture/compare input
  // bit9=X,           reserved
  // bit8=0,           compare mode
  // bits7-5=XXX,      output mode
  // bit4=1,           enable capture/compare interrupt
  // bit3=X,           read capture/compare input from here
  // bit2=0,           output this value in output mode 0
  // bit1=X,           capture overflow status
  // bit0=0,           clear capture/compare interrupt pending
  TA1CCTL0 = 0x0010;
  TA1CCR0 = 59999;                 // compare match value for 100 Hz interrupts
  TA1EX0 &= ~0x0007;               // configure for input clock divider /1
  // ***************** interrupt initialization *****************
// interrupts enabled in the main program after all devices initialized
  NVIC_IPR2 = (NVIC_IPR2&0xFF00FFFF)|0x00400000; // priority 2
  NVIC_ISER0 = 0x00000400;         // enable interrupt 10 in NVIC
  TA0CTL |= 0x0024;                // reset and start Timer A0 in continuous mode
  TA1CTL |= 0x0014;                // reset and start Timer A1 in up mode
  EndCritical(sr);
}

// ***************** FreqMeasure_Get ****************
// Get the most recent measurement of the number of rising
// edges on P7.1/TA0CLK within the last 10 ms window.
// Stall if no measurement is ready.
// Assumes FreqMeasure_Init() called and interrupts enabled
// Inputs: none
// Outputs: number of rising edges on P7.1/TA0CLK in 10 ms
//          frequency units are 100 Hz
//  E.g., if it returns 123 it means the frequency is 123edges/10ms = 12300 Hz
uint32_t FreqMeasure_Get(void){
  while(Done == 0){};              // wait for measurement to complete
  Done = 0;
  return Freq;
}

void TA1_0_IRQHandler(void){
  TA1CCTL0 &= ~0x0001;             // acknowledge capture/compare interrupt 0
  TA0CTL &= ~0x0030;               // halt Timer A0
  Freq = TA0R;                     // f = (pulses)/(fixed time)
  Done = -1;
  TA0CTL |= 0x0024;                // reset and start Timer A0 in continuous mode
}
