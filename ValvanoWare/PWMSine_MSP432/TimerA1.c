// TimerA1.c
// Runs on MSP432
// Use Timer A1 in periodic mode to request interrupts at a particular
// period.
// Daniel Valvano
// July 20, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 9.8

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
void (*PeriodicTask)(void);   // user function

// ***************** TimerA1_Init ****************
// Activate Timer A1 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/SMCLK), 16 bits
// Outputs: none
void TimerA1_Init(void(*task)(void), uint16_t period){long sr;
  sr = StartCritical();
  PeriodicTask = task;             // user function
  TA1CTL &= ~0x0030;               // halt Timer A1
  // bits15-10=XXXXXX, reserved
  // bits9-8=10,       clock source to SMCLK
  // bits7-6=00,       input clock divider /1
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=0,           interrupt disable
  // bit0=0,           clear interrupt pending
  TA1CTL = 0x0200;
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
  TA1CCR0 = (period - 1);          // compare match value
  TA1EX0 &= ~0x0007;               // configure for input clock divider /1
// interrupts enabled in the main program after all devices initialized
  NVIC_IPR2 = (NVIC_IPR2&0xFF00FFFF)|0x00400000; // priority 2
  NVIC_ISER0 = 0x00000400;         // enable interrupt 10 in NVIC
  TA1CTL |= 0x0014;                // reset and start Timer A1 in up mode
  EndCritical(sr);
}

void TA1_0_IRQHandler(void){
  TA1CCTL0 &= ~0x0001;           // acknowledge capture/compare interrupt 0
  (*PeriodicTask)();             // execute user task
}
