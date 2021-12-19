// GPIO.c
// Runs on MSP432
// Initialize four GPIO pins as outputs.  Continually generate output to
// drive simulated stepper motor.
// Daniel Valvano
// June 20, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Volume 1 Program 4.5

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

// P4.3 is an output to LED3, negative logic
// P4.2 is an output to LED2, negative logic
// P4.1 is an output to LED1, negative logic
// P4.0 is an output to LED0, negative logic

#include <stdint.h>
#include "..\inc\msp432p401r.h"
#include "SysTick.h"

void GPIO_Init(void){
  // initialize P4.3-P4.0 and make them outputs
  P4SEL0 &= ~0x0F;
  P4SEL1 &= ~0x0F;            // configure stepper motor/LED pins as GPIO
  P4DIR |= 0x0F;              // make stepper motor/LED pins out
}

int main1(void){
  GPIO_Init();
  while(1){
    P4OUT = (P4OUT&~0x0F)|10; // 1010, LED is 0101
    P4OUT = (P4OUT&~0x0F)|9;  // 1001, LED is 0110
    P4OUT = (P4OUT&~0x0F)|5;  // 0101, LED is 1010
    P4OUT = (P4OUT&~0x0F)|6;  // 0110, LED is 1001
  }
}

// Program 2.13 from Volume 2
#define STEPPER  (*((volatile uint8_t *)0x40004C23))  /* Port 4 Output, bits 3-0 are stepper motor */
static void step(uint8_t n){
  STEPPER = (STEPPER&~0x0F)|n;  // output to stepper causing it to step once
  SysTick_Wait10ms(10);         // program 2.12
}
int main(void){ // reset clears P4REN, P4DS, P4SEL0, P4SEL1
  SysTick_Init();
  GPIO_Init();
  while(1){
    step(5);  // motor is 0101
    step(6);  // motor is 0110
    step(10); // motor is 1010
    step(9);  // motor is 1001
  }
}
