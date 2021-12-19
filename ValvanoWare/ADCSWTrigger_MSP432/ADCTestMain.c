// ADCTestMain.c
// Runs on MSP432
// This program periodically samples ADC channel 6 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// June 9, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

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
// center of X-ohm potentiometer connected to P4.7/A6
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V
// voltage reference out connectd to P5.6

#include <stdint.h>
#include "ADC14.h"
#include "..\inc\msp432p401r.h"

#define RED       0x01
#define GREEN     0x02
#define BLUE      0x04
#define WHEELSIZE 8           // must be an integer power of 2
                              //    red, yellow,    green, light blue, blue, purple,   white,          dark
const long COLORWHEEL[WHEELSIZE] = {RED, RED+GREEN, GREEN, GREEN+BLUE, BLUE, BLUE+RED, RED+GREEN+BLUE, 0};

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint32_t ADCvalue;
// This debug function initializes SysTick to request interrupts
// at a 10 Hz frequency.
void SysTick_Init10HzInt(void){
  DisableInterrupts();
  // **** SysTick initialization ****
  SYSTICK_STCSR = 0;               // disable SysTick during setup
  SYSTICK_STRVR = 299999;          // reload value for 10 Hz interrupts
  SYSTICK_STCVR = 0;               // any write to current clears it
  SYSTICK_STCSR = 0x00000007;      // enable SysTick with core clock and interrupts
  // **** interrupt initialization ****
                                   // SysTick=priority 2
  SCB_SHPR3 = (SCB_SHPR3&0x00FFFFFF)|0x40000000; // top 3 bits
}
void SysTick_Handler(void){
  P1OUT ^= 0x01;                   // profile
  P1OUT ^= 0x01;                   // profile
  ADCvalue = ADC_In();
  P1OUT ^= 0x01;                   // profile
}
int main(void){
  ADC0_InitSWTriggerCh6();         // initialize ADC sample P4.7/A6
  SysTick_Init10HzInt();           // set up SysTick for 10 Hz interrupts
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
  EnableInterrupts();
  while(1){
    // update the color wheel according to the ADC measurement
    P2OUT = (P2OUT&~0x07)|(COLORWHEEL[(ADCvalue>>11)&(WHEELSIZE-1)]);
    WaitForInterrupt();
  }
}


