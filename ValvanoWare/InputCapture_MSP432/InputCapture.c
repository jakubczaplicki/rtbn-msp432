// InputCapture.c
// Runs on MSP432
// Use Timer A0 in capture mode to request interrupts on the rising
// edge of P7.3 (TA0CCP0), and call a user function.
// Daniel Valvano
// July 27, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Example 6.1, Program 6.1

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

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void (*CaptureTask)(uint16_t time);// user function

//------------TimerCapture_Init------------
// Initialize Timer A0 in edge time mode to request interrupts on
// the rising edge of P7.3 (TA0CCP0).  The interrupt service routine
// acknowledges the interrupt and calls a user function.
// Input: task is a pointer to a user function called when edge occurs
//             parameter is 16-bit up-counting timer value when edge occurred
// Output: none
void TimerCapture_Init(void(*task)(uint16_t time)){long sr;
  sr = StartCritical();
  CaptureTask = task;              // user function
  // 1) connect the signal to P7.3 (TA0CCP0)
  P7SEL0 |= 0x08;     // 2) configure P7.3 as TA0CCP0
  P7SEL1 &= ~0x08;
  P7DIR &= ~0x08;     // 3) make P7.3 in
  TA0CTL &= ~0x0030;  // 4) halt Timer A0
  // bits15-10=XXXXXX, reserved
  // bits9-8=10,       clock source to SMCLK
  // bits7-6=00,       input clock divider /1
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=0,           interrupt disable
  // bit0=0,           clear interrupt pending
  TA0CTL = 0x0200;    // 5) SMCLK, divide by 1
  TA0EX0 &= ~0x0007;  //    clock divide by 1
  // bits15-14=01,     capture on rising edge
  // bits13-12=00,     capture/compare input on CCI0A
  // bit11=1,          synchronous capture source
  // bit10=X,          synchronized capture/compare input
  // bit9=X,           reserved
  // bit8=1,           capture mode
  // bits7-5=XXX,      output mode
  // bit4=1,           enable capture/compare interrupt
  // bit3=X,           read capture/compare input from here
  // bit2=X,           output this value in output mode 0
  // bit1=X,           capture overflow status
  // bit0=0,           clear capture/compare interrupt pending
  TA0CCTL0 = 0x4910;  // 6) rising, capture, sync, arm
  NVIC_IPR2 = (NVIC_IPR2&0xFFFFFF00)|0x00000040; // 7) priority 2
// interrupts enabled in the main program after all devices initialized
  NVIC_ISER0 = 0x00000100; // 8) enable interrupt 8 in NVIC
  TA0CTL |= 0x0024;        // 9) reset and start Timer A0 in continuous mode
  EndCritical(sr);
}

void TA0_0_IRQHandler(void){
  TA0CCTL0 &= ~0x0001;             // acknowledge capture/compare interrupt 0
  (*CaptureTask)(TA0CCR0);         // execute user task
}
