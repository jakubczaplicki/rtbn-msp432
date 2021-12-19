// SysTickInts.c
// Runs on MSP432
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// June 1, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1 Program 9.7

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
void (*SysTickTask)(void);    // user function

// **************SysTick_Init*********************
// Initialize SysTick periodic interrupts
// Input: task is a pointer to a user function
//        period is interrupt period
//        Units of period are 333ns (assuming 3 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(void(*task)(void), uint32_t period){ long sr;
  sr = StartCritical();
  SysTickTask = task;              // user function
  SYSTICK_STCSR = 0;               // 1) disable SysTick during setup
  SYSTICK_STRVR = period - 1;      // 2) reload value sets period
  SYSTICK_STCVR = 0;               // 3) any write to current clears it
  SCB_SHPR3 = (SCB_SHPR3&0x00FFFFFF)|0x40000000; // priority 2
  SYSTICK_STCSR = 0x00000007;      // 4) enable SysTick with core clock and interrupts
  EndCritical(sr);
}
void SysTick_Handler(void){
  (*SysTickTask)();                // execute user task
}
