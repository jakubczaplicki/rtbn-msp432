// SSR.c
// Runs on MSP432
// Provide functions that initialize a GPIO pin and turn it on and off.
// Daniel Valvano
// April 23, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Volume 1 Program 4.3, Figure 4.14

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

// solid state relay connected to P2.2

#include <stdint.h>
#include "..\inc\msp432p401r.h"

//------------SSR_Init------------
// Make P2.2 an output and ensure alt. functions off.
// Input: none
// Output: none
void SSR_Init(void){
  // initialize P2.2 and make it output
  P2SEL0 &= ~0x04;
  P2SEL1 &= ~0x04;            // configure SSR pin as GPIO
  P2DS |= 0x04;               // make SSR pin high drive strength
  P2DIR |= 0x04;              // make SSR pin out
}

//------------SSR_On------------
// Make P2.2 high.
// Input: none
// Output: none
void SSR_On(void){
  P2OUT |= 0x04;              // turn on the appliance
}

//------------SSR_Off------------
// Make P2.2 low.
// Input: none
// Output: none
void SSR_Off(void){
  P2OUT &= ~0x04;             // turn off the appliance
}

//--------SSR_Toggle -----------
// Toggle P2.2
// Input: none    Output: none
void SSR_Toggle(void){
  P2OUT = P2OUT^0x04; // toggle
}

//debug code
// This is an extremely simple test program to demonstrate that the SSR
// can turn on and off.  Press and release left Button 1 to turn the
// SSR on, and press and release right Button 2 to turn the SSR off.
// built-in negative logic switches connected to P1.1 and P1.4
int main(void){
  SSR_Init();                 // initialize P2.2 and make it output
  // initialize P1.1 and P1.4 and make them inputs (P1.1 and P1.4 built-in buttons)
  P1SEL0 &= ~0x12;
  P1SEL1 &= ~0x12;            // configure P1.4 and P1.1 as GPIO
  P1DIR &= ~0x12;             // make P1.4 and P1.1 in
  P1REN |= 0x12;              // enable pull resistors on P1.4 and P1.1
  P1OUT |= 0x12;              // P1.4 and P1.1 are pull-up
  while(1){
    SSR_Off();
    while(P1IN&0x02);         // wait for Button 1 press
    while((P1IN&0x02) == 0);  // wait for Button 1 release
    SSR_On();
    while(P1IN&0x10);         // wait for Button 2 press
    while((P1IN&0x10) == 0);  // wait for Button 2 release
  }
}


// Volume 2 code
#define P23OUT  (*((volatile uint8_t *)(0x4209806C)))
//------------SSR_Init2------------
// Make P2.3 an output and ensure alt. functions off.
// Input: none
// Output: none
void SSR_Init2(void){
  // initialize P2.3 and make it output
  P2SEL0 &= ~0x08;
  P2SEL1 &= ~0x08;            // configure SSR pin as GPIO
  P2DS |= 0x08;               // make SSR pin high drive strength
  P2DIR |= 0x08;              // make SSR pin out
}

//------------SSR_On2------------
// Make P2.3 high.
// Input: none
// Output: none
void SSR_On2(void){
  P23OUT = 0x01;             // turn on the appliance
}

//------------SSR_Off2------------
// Make P2.3 low.
// Input: none
// Output: none
void SSR_Off2(void){
  P23OUT = 0x00;             // turn off the appliance
}

//--------SSR_Toggle2 -----------
// Toggle P2.3
// Input: none    Output: none
void SSR_Toggle2(void){
  P23OUT = P23OUT^0x01; // toggle
}
// This is an extremely simple test program to demonstrate that the SSR
// can turn on and off.  Press and release left Button 1 to turn the
// SSR on, and press and release right Button 2 to turn the SSR off.
// built-in negative logic switches connected to P1.1 and P1.4
int main2(void){
  SSR_Init2();                 // initialize P2.2 and make it output
  // initialize P1.1 and P1.4 and make them inputs (P1.1 and P1.4 built-in buttons)
  P1SEL0 &= ~0x12;
  P1SEL1 &= ~0x12;            // configure P1.4 and P1.1 as GPIO
  P1DIR &= ~0x12;             // make P1.4 and P1.1 in
  P1REN |= 0x12;              // enable pull resistors on P1.4 and P1.1
  P1OUT |= 0x12;              // P1.4 and P1.1 are pull-up
  while(1){
    SSR_Off2();
    while(P1IN&0x02);         // wait for Button 1 press
    while((P1IN&0x02) == 0);  // wait for Button 1 release
    SSR_On2();
    while(P1IN&0x10);         // wait for Button 2 press
    while((P1IN&0x10) == 0);  // wait for Button 2 release
  }
}

