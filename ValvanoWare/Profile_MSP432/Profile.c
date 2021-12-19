// Profile.c
// Runs on MSP432
// This program constantly toggles three output pins when executing
// one of three threads: main program, SysTick handler, and Timer
// A0 handler
// Daniel Valvano
// July 1, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Section 9.9.3, Program 9.13

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

// logic analyzer connected to P2.0 toggles in main
// logic analyzer connected to P3.0 toggles in SysTick handler
// logic analyzer connected to P4.0 toggles in TimerA0 handler

#include <stdint.h>
#include "ClockSystem.h"
#include "SysTickInts.h"
#include "TimerA0.h"
#include "..\inc\msp432p401r.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint32_t Counts;
void TimerA0Function(void){
  P4OUT |= 0x01;
  Counts = Counts + 1;
  P4OUT &= ~0x01;
}
void SysTickFunction(void){
  P3OUT |= 0x01;
  Counts = Counts + 1;
  P3OUT &= ~0x01;
}
int main(void){
  DisableInterrupts();
  Clock_Init(DCO3MHz);             // run at 3 MHz
//  TimerA0_Init(&TimerA0Function, 300);// 10 kHz, 100us
//  SysTick_Init(&SysTickFunction, 340);// 8.8 kHz, 113us
  TimerA0_Init(&TimerA0Function, 3000);// 1 kHz, 1ms (trigger on P4.0 high)
  SysTick_Init(&SysTickFunction, 3040);// 987 Hz, 1.01ms (trigger on rising edge of P3.0)
  P2SEL0 &= ~0x01;
  P2SEL1 &= ~0x01;                 // configure P2.0 as GPIO
  P2DS |= 0x01;                    // make P2.0 high drive strength
  P2DIR |= 0x01;                   // make P2.0 out
  P3SEL0 &= ~0x01;
  P3SEL1 &= ~0x01;                 // configure P3.0 as GPIO
  P3DIR |= 0x01;                   // make P3.0 out
  P4SEL0 &= ~0x01;
  P4SEL1 &= ~0x01;                 // configure P4.0 as GPIO
  P4DIR |= 0x01;                   // make P4.0 out
  EnableInterrupts();
  while(1){
    P2OUT = P2OUT^0x01;
  }
}
