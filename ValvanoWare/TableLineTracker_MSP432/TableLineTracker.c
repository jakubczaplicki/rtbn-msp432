// TableLineTracker.c
// Runs on MSP432
// Use a table implementation of a Moore finite state machine to make
// a line following robot.
// Daniel Valvano
// July 14, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Volume 2, Program 3.1, Example 3.1

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

// left line sensor connected to P1.1 (1=line detected)
// right line sensor connected to P1.4 (1=line detected)
// left drive motor connected to P2.0
// right drive motor connected to P2.1

#include <stdint.h>
#include "SysTick.h"
#include "..\inc\msp432p401r.h"

struct State {
  uint8_t Out;       // 2-bit output
  uint32_t Delay;    // time in 10 ms
  uint8_t Next[4];}; // Next if 2-bit input is 0-3
typedef const struct State STyp;
#define Center 0
#define Left 1
#define Right 2
STyp FSM[4]={
  {0x03, 1, { Right, Right,  Left,   Center }},  // Center of line
  {0x02, 1, { Left,  Right,  Center, Center }},  // Left of line
  {0x01, 1, { Right, Center, Left,   Center }}   // Right of line
};
// SENSOR is set to P1IN and MOTOR is set to P2OUT, making the code more readable.
#define SENSOR  (*((volatile uint8_t *)0x40004C00))  /* Port 1 Input */
#define MOTOR   (*((volatile uint8_t *)0x40004C03))  /* Port 2 Output */

void Robot_Init(void){
  SysTick_Init();              // initialize SysTick timer
  // activate Port 1 and Port 2
  // initialize P1.4 and P1.1 and make them inputs
  P1SEL0 &= ~0x12;
  P1SEL1 &= ~0x12;             // configure line sensor pins as GPIO
  P1DIR &= ~0x12;              // make line sensor pins in
  P1REN |= 0x12;               // enable pull resistors on P1.4 and P1.1 (debug)
  P1OUT |= 0x12;               // P1.4 and P1.1 are pull-up (debug)
  // initialize P2.1-P2.0 and make them outputs
  P2SEL0 &= ~0x03;
  P2SEL1 &= ~0x03;             // configure motor pins as GPIO
  P2DS |= 0x03;                // make motor pins high drive strength
  P2DIR |= 0x03;               // make motor pins out
}

int main(void){
  uint8_t n; // state number
  uint8_t Input;
  Robot_Init();                // initialize Port 1, Port 2, and SysTick timer
  n = Center;                  // initial state: go straight
  while(1){
                               // set motors to current state's Out value
    MOTOR = (MOTOR&~0x03)|(FSM[n].Out);
                               // wait 10 ms * current state's Delay value
    SysTick_Wait10ms(FSM[n].Delay);
                               // get new input from line sensors
    Input = ((SENSOR&0x02)>>1) + ((SENSOR&0x10)>>3);
    n = FSM[n].Next[Input];    // transition to next state
  }
}
