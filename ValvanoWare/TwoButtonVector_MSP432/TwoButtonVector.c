// TwoButtonVector.c
// Runs on MSP432
// Use vectored interrupts to respond to two button presses.  Note
// that button bouncing is not addressed.
// Daniel Valvano
// May 20, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 9.5


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
// negative logic built-in Button 1 connected to P1.1
// negative logic built-in Button 2 connected to P1.4
// built-in red LED connected to P2.0
// built-in green LED connected to P2.1
// built-in blue LED connected to P2.2
// momentary, positive logic button connected to P5.3 (trigger on rising edge)
// momentary, positive logic button connected to P6.3 (trigger on rising edge)

#include <stdint.h>
#include "..\inc\msp432p401r.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// global variables visible in Watch window of debugger
// set when corresponding button pressed
volatile uint32_t SW1, SW2; // semaphores
void VectorButtons_Init(void){
  DisableInterrupts();             // set the I bit
  SW1 = 0;                         // clear semaphores
  SW2 = 0;
  P5SEL0 &= ~0x08;
  P5SEL1 &= ~0x08;                 // configure P5.3 as GPIO
  P5DIR &= ~0x08;                  // make P5.3 in
  P5IES &= ~0x08;                  // P5.3 is rising edge event
  P5IFG &= ~0x08;                  // clear flag3 (reduce possibility of extra interrupt)
  P5IE |= 0x08;                    // arm interrupt on P5.3
  NVIC_IPR9 = (NVIC_IPR9&0x00FFFFFF)|0x40000000; // priority 2
  NVIC_ISER1 = 0x00000080;         // enable interrupt 39 in NVIC
  P6SEL0 &= ~0x08;
  P6SEL1 &= ~0x08;                 // configure P6.3 as GPIO
  P6DIR &= ~0x08;                  // make P6.3 in
  P6IES &= ~0x08;                  // P6.3 is rising edge event
  P6IFG &= ~0x08;                  // clear flag3 (reduce possibility of extra interrupt)
  P6IE |= 0x08;                    // arm interrupt on P6.3
  NVIC_IPR10 = (NVIC_IPR10&0xFFFFFF00)|0x00000040; // priority 2
  NVIC_ISER1 = 0x00000100;         // enable interrupt 40 in NVIC
  EnableInterrupts();              // clear the I bit
}
// let the I/O address be 0x4000.0000+n, and let b represent the bit 0 to 7.
// The aliased address for this bit will be 0x4200.0000 + 32*n + 4*b
// P5IFG = 0x40000000+0x4C5C
// P6IFG = 0x40000000+0x4C5D
// using bit-banding allows for atomic access to interrupt flags, removing the race condition
// e.g., P6IFG &= ~0x04; may inadvertently clear other flags in this register too
#define P5IFG_2   (HWREG8(0x42000000+32*0x4C5C+4*3))  /* Port 5 Interrupt Flag3 */
#define P6IFG_3   (HWREG8(0x42000000+32*0x4C5D+4*3))  /* Port 6 Interrupt Flag3 */
void PORT5_IRQHandler(void){
  if(P5IFG_2){              // poll P5.3
    P5IFG_2 = 0;            // acknowledge flag3
    SW1 = 1;                // signal SW1 occurred
  }
}
void PORT6_IRQHandler(void){
  if(P6IFG_3){              // poll P6.3
    P6IFG_3 = 0;            // acknowledge flag3
    SW2 = 1;                // signal SW2 occurred
  }
}

//debug code
int main(void){
  VectorButtons_Init();            // initialize interrupts and ports
  while(1){
    WaitForInterrupt();
  }
}
