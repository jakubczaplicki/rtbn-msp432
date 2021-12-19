// TwoButtonPoll.c
// Runs on MSP432
// Use polled interrupts to respond to two button presses.  Note
// that button bouncing is not addressed.
// Daniel Valvano
// July 2, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 9.6

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
// momentary, positive logic button connected to P6.2 (trigger on rising edge)
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
void PolledButtons_Init(void){
  DisableInterrupts();             // set the I bit
  SW1 = 0;                         // clear semaphores
  SW2 = 0;
  P6SEL0 &= ~0x0C;
  P6SEL1 &= ~0x0C;                 // configure P6.3 and P6.2 as GPIO
  P6DIR &= ~0x0C;                  // make P6.3 and P6.2 in
  P6IES &= ~0x0C;                  // P6.3 and P6.2 are rising edge event
  P6IFG &= ~0x0C;                  // clear flag3 and flag2 (reduce possibility of extra interrupt)
  P6IE |= 0x0C;                    // arm interrupt on P6.3 and P6.2
  NVIC_IPR10 = (NVIC_IPR10&0xFFFFFF00)|0x00000040; // priority 2
  NVIC_ISER1 = 0x00000100;         // enable interrupt 40 in NVIC
  EnableInterrupts();              // clear the I bit
}
// let the I/O address be 0x4000.0000+n, and let b represent the bit 0 to 7.
// The aliased address for this bit will be 0x4200.0000 + 32*n + 4*b
// P6IFG = 0x40000000+0x4C5D
// using bit-banding allows for atomic access to interrupt flags, removing the race condition
// e.g., P6IFG &= ~0x04; may inadvertently clear flag 3 too
#define P6IFG_2   (HWREG8(0x42000000+32*0x4C5D+4*2))  /* Port 6 Interrupt Flag2 */
#define P6IFG_3   (HWREG8(0x42000000+32*0x4C5D+4*3))  /* Port 6 Interrupt Flag3 */
// volume 2 uses bit-banding to solve critical section/race
void PORT6_IRQHandler_version1(void){
  if(P6IFG_2){      // poll P6.2
    P6IFG_2 = 0;    // acknowledge flag2
    SW1 = 1;        // signal SW1 occurred
  }
  if(P6IFG_3){      // poll P6.3
    P6IFG_3 = 0;    // acknowledge flag3
    SW2 = 1;        // signal SW2 occurred
  }
}
// volume 1 uses P6IV to solve critical section/race
void PORT6_IRQHandler(void){ uint8_t status;
  status = P6IV;  // 2*(n+1) where n is highest priority
// if P6.2, returns 0x06 and clears flag2
// if P6.3, returns 0x08 and clears flag3
  if(status == 0x06){
    SW1 = 1;        // signal SW1 occurred
    status = P6IV;
  }
  if(status == 0x08){
    SW2 = 1;        // signal SW2 occurred
  }
}
//debug code
int main(void){
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
  PolledButtons_Init();            // initialize interrupts and ports
  while(1){
    WaitForInterrupt();
    if(SW1){
      SW1 = 0;
      P1OUT ^= 0x01;               // toggle LED1
    }
    if(SW2){
      SW2 = 0;
      P2OUT ^= 0x06;               // toggle blue and green RGB LEDs
    }
  }
}
