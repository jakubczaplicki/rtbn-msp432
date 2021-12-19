// PeriodicSysTickInts.c
// Runs on MSP432
// Use the SysTick timer to request interrupts at a particular period.
// Jonathan Valvano
// July 1, 2015

/* This example accompanies the books
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

// built-in LED1 connected to P1.0
// negative logic built-in Button 1 connected to P1.1 (interrupt on falling edge)
// negative logic built-in Button 2 connected to P1.4 (interrupt on falling edge)
// built-in red LED connected to P2.0
// built-in green LED connected to P2.1
// built-in blue LED connected to P2.2
// P4.0 is an output to profiling scope/logic analyzer

#include <stdint.h>
#include "SysTickInts.h"
#include "..\inc\msp432p401r.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
#define P1IFG_1  (*((volatile uint8_t *)(0x42000000+32*0x4C1C+4*1)))  
#define P1IFG_4  (*((volatile uint8_t *)(0x42000000+32*0x4C1C+4*4)))

// This interrupt should be higher priority than the SysTick interrupt.
// Pressing one button should toggle the other LED and pause the SysTick
// interrupts until the button is released.
void PORT1_IRQHandler(void){
  if(P1IFG_1){
    P1IFG_1 = 0x00;                // acknowledge flag1
    P2OUT ^= 0x06;                 // toggle blue and green RGB LEDs
    while((P1IN&0x02) == 0x00){};  // wait for Button 1 release
  }
  if(P1IFG_4){
    P1IFG_4 = 0x00;                // acknowledge flag4
    P1OUT ^= 0x01;                 // toggle LED1
    while((P1IN&0x10) == 0x00){};  // wait for Button 2 release
  }
}

int main(void){
  SysTick_Init(30000);             // set up SysTick for 100 Hz interrupts
  // initialize P1.0 and make it output (P1.0 built-in LED1)
  P1SEL0 &= ~0x01;
  P1SEL1 &= ~0x01;                 // configure built-in LED1 as GPIO
  P1DIR |= 0x01;                   // make built-in LED1 out
  P1OUT &= ~0x01;                  // LED1 = off
  // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in RGB LEDs)
  P2SEL0 &= ~0x07;
  P2SEL1 &= ~0x07;                 // configure built-in RGB LEDs as GPIO
  P2DS |= 0x07;                    // make built-in RGB LEDs high drive strength
  P2DIR |= 0x07;                   // make built-in RGB LEDs out
  P2OUT &= ~0x07;                  // RGB = off
  // initialize P1.4 and P1.1 and make them inputs (P1.4 and P1.1 built-in buttons)
  P1SEL0 &= ~0x12;
  P1SEL1 &= ~0x12;                 // configure P1.4 and P1.1 as GPIO
  P1DIR &= ~0x12;                  // make P1.4 and P1.1 in (built-in Button 1 and Button 2)
  P1REN |= 0x12;                   // enable pull resistors on P1.4 and P1.1
  P1OUT |= 0x12;                   // P1.4 and P1.1 are pull-up
  P1IES |= 0x12;                   // P1.4 and P1.1 are falling edge event
  P1IFG &= ~0x12;                  // clear flag4 and flag1 (reduce possibility of extra interrupt)
  P1IE |= 0x12;                    // arm interrupt on P1.4 and P1.1
  NVIC_IPR8 = (NVIC_IPR8&0x00FFFFFF)|0x20000000; // priority 1
  NVIC_ISER1 = 0x00000008;         // enable interrupt 35 in NVIC
  EnableInterrupts();
  while(1){
    WaitForInterrupt();
  }
}

