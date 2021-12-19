// EdgeInterrupt.c
// Runs on MSP432
// Request an interrupt on the falling edge of P1.1 (when the user
// button is pressed) and increment a counter in the interrupt.  Note
// that button bouncing is not addressed.
// Daniel Valvano
// May 20, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 9.4


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
// negative logic built-in Button 1 connected to P1.1 (increment counter on falling edge)
// negative logic built-in Button 2 connected to P1.4 (increment counter on falling edge)
// built-in red LED connected to P2.0
// built-in green LED connected to P2.1
// built-in blue LED connected to P2.2

#include <stdint.h>
#include "..\inc\msp432p401r.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// global variable visible in Watch window of debugger
// increments at least once per button press
volatile uint32_t FallingEdges = 0;
void EdgeCounter_Init(void){
  DisableInterrupts();             // set the I bit
  FallingEdges = 0;                // initialize counter
// original code (no LEDs)
/*  P1SEL0 &= ~0x02;
  P1SEL1 &= ~0x02;                 // configure P1.1 as GPIO
  P1DIR &= ~0x02;                  // make P1.1 in (built-in Button 1)
  P1REN |= 0x02;                   // enable pull resistor on P1.1
  P1OUT |= 0x02;                   // P1.1 is pull-up
  P1IES |= 0x02;                   // P1.1 is falling edge event
  P1IFG &= ~0x02;                  // clear flag1 (reduce possibility of extra interrupt)
  P1IE |= 0x02;                    // arm interrupt on P1.1*/
// end of original code (no LEDs)
// new code (now with LEDs)
  P1SEL0 &= ~0x12;
  P1SEL1 &= ~0x12;                 // configure P1.4 and P1.1 as GPIO
  P1DIR &= ~0x12;                  // make P1.4 and P1.1 in (built-in Button 1 and Button 2)
  P1REN |= 0x12;                   // enable pull resistors on P1.4 and P1.1
  P1OUT |= 0x12;                   // P1.4 and P1.1 are pull-up
  P1IES |= 0x12;                   // P1.4 and P1.1 are falling edge event
  P1IFG &= ~0x12;                  // clear flag4 and flag1 (reduce possibility of extra interrupt)
  P1IE |= 0x12;                    // arm interrupt on P1.4 and P1.1
// end of new code (now with LEDs)
  NVIC_IPR8 = (NVIC_IPR8&0x00FFFFFF)|0x40000000; // priority 2
  NVIC_ISER1 = 0x00000008;         // enable interrupt 35 in NVIC
  EnableInterrupts();              // clear the I bit
}
// let the I/O address be 0x4000.0000+n, and let b represent the bit 0 to 7.
// The aliased address for this bit will be 0x4200.0000 + 32*n + 4*b
// P1IFG = 0x40000000+0x4C1C
// using bit-banding allows for atomic access to interrupt flags, removing the race condition
// e.g., P1IFG &= ~0x02; may inadvertently clear flag 4 too
#define P1IFG_1   (HWREG8(0x42000000+32*0x4C1C+4*1))  /* Port 1 Interrupt Flag1 */
#define P1IFG_4   (HWREG8(0x42000000+32*0x4C1C+4*4))  /* Port 1 Interrupt Flag4 */
void PORT1_IRQHandler(void){
  FallingEdges = FallingEdges + 1;
  if(P1IFG_1){
    P1IFG_1 = 0;     // clear flag1, acknowledge
    P1OUT ^= 0x01;   // toggle LED1
  }
  if(P1IFG_4){
    P1IFG_4 = 0;     // clear flag4, acknowledge
    P2OUT ^= 0x06;   // toggle blue and green RGB LEDs
  }
}
// no critical section/race because only one input
void PORT1_IRQHandler_version2(void){
  FallingEdges = FallingEdges + 1;
  P1IFG_1 = 0;      // clear flag1, acknowledge
  P1OUT ^= 0x01;    // toggle LED1
}
// Uses P1IV to solve critical section/race
void PORT1_IRQHandler_version3(void){ uint8_t status;
  FallingEdges = FallingEdges + 1;
  status = P1IV;  // 2*(n+1) where n is highest priority
// if P1.1, returns 0x04 and clears flag1
// if P1.4, returns 0x0A and clears flag4
  if(status == 0x04){
    P1OUT ^= 0x01;                 // toggle LED1
    status = P1IV;
  }
  if(status == 0x0A){
    P2OUT ^= 0x06;                 // toggle blue and green RGB LEDs
  }
}
//debug code
int main(void){
// original code (no LEDs)
/*  EdgeCounter_Init();              // initialize P1.1 interrupt*/
// end of original code (no LEDs)
// new code (now with LEDs)
  EdgeCounter_Init();              // initialize P1.4 and P1.1 interrupts
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
// end of new code (now with LEDs)
  while(1){
    WaitForInterrupt();
  }
}
