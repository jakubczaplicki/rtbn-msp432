// LED.c
// Runs on MSP432
// Provide functions that initialize a GPIO as an input pin and
// allow reading of two negative logic switches on P1.1 and P1.4.
// Use bit-banded I/O.
// Daniel and Jonathan Valvano
// October 8, 2015

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Section 4.2, Program 4.2

   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Example 2.3, Program 2.9, Figure 2.36

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

//------------LED_Init------------
// Initialize GPIO Port 1 for negative logic switches on P1.1 and
// P1.4 as the LaunchPad is wired.  Weak internal pull-up
// resistors are enabled.  Initialize GPIO Port 2 for LEDs on P2.2,
// P2.1, and P2.0.
// Input: none
// Output: none
void LED_Init(void){
  P1SEL0 &= ~0x12;
  P1SEL1 &= ~0x12;                 // configure P1.4 and P1.1 as GPIO
  P1DIR &= ~0x12;                  // make P1.4 and P1.1 in
  P1REN |= 0x12;                   // enable pull resistor on P1.4 and P1.1
  P1OUT |= 0x12;                   // P1.4 and P1.1 are pull-up
  P2SEL0 &= ~0x07;
  P2SEL1 &= ~0x07;                 // configure P2.2-P2.0 as GPIO
  P2DS |= 0x07;                    // make P2.2-P2.0 high drive strength
  P2DIR |= 0x07;                   // make P2.2-P2.0 out
  P2OUT &= ~0x07;                  // all LEDs off
}

//------------Board_Input------------
// Read and return the status of the switches.
// Input: none
// Output: 2 if only Switch 1 is pressed
//         1 if only Switch 2 is pressed
//         3 if both switches are pressed
//         0 if no switches are pressed
#define SWITCH1   (*((volatile uint8_t *)(0x42000000+32*0x4C00+4*1)))  /* Port 1.1 Input */
#define SWITCH2   (*((volatile uint8_t *)(0x42000000+32*0x4C00+4*4)))  /* Port 1.4 Input */
uint32_t Board_Input(void){
  return (~((SWITCH1<<1) + SWITCH2))&0x03;
}
//------------LED_RedOn------------
// Turn on red LED.
// Input: none
// Output: none
#define RED       (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*0)))  /* Port 2.0 Output */
void LED_RedOn(void){
  RED = 0x01;
}

//------------LED_RedOff------------
// Turn off red LED.
// Input: none
// Output: none
void LED_RedOff(void){
  RED = 0x00;
}

//------------LED_RedToggle------------
// Toggle red LED.
// Input: none
// Output: none
void LED_RedToggle(void){
  RED ^= 0x01;
}

//------------LED_GreenOn------------
// Turn on green LED.
// Input: none
// Output: none
#define GREEN     (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*1)))  /* Port 2.1 Output */
void LED_GreenOn(void){
  GREEN = 0x01;
}

//------------LED_GreenOff------------
// Turn off green LED.
// Input: none
// Output: none
void LED_GreenOff(void){
  GREEN = 0x00;
}

//------------LED_GreenToggle------------
// Toggle green LED.
// Input: none
// Output: none
void LED_GreenToggle(void){
  GREEN ^= 0x01;
}

//------------LED_BlueOn------------
// Turn on blue LED.
// Input: none
// Output: none
#define BLUE      (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*2)))  /* Port 2.2 Output */
void LED_BlueOn(void){
  BLUE = 0x01;
}

//------------LED_BlueOff------------
// Turn off blue LED.
// Input: none
// Output: none
void LED_BlueOff(void){
  BLUE = 0x00;
}

//------------LED_BlueToggle------------
// Toggle blue LED.
// Input: none
// Output: none
void LED_BlueToggle(void){
  BLUE ^= 0x01;
}
