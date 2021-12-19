// Stepper.c
// Runs on MSP432
// Provide functions that step the motor once clockwise, step
// once counterclockwise, and initialize the stepper motor
// interface.
// Daniel Valvano
// July 26, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Section 4.7.2, Program 4.4  4.5 and Program 4.6

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

// P4.3 connected to driver for stepper motor coil A
// P4.2 connected to driver for stepper motor coil A'
// P4.1 connected to driver for stepper motor coil B
// P4.0 connected to driver for stepper motor coil B'

#include <stdint.h>
#include "..\inc\msp432p401r.h"
#include "systick.h"

struct State{
  uint8_t Out;                 // Output
  const struct State *Next[2]; // CW/CCW
};
typedef const struct State StateType;
typedef StateType *StatePtr;
#define clockwise 0        // Next index
#define counterclockwise 1 // Next index
StateType fsm[4]={
  {10,{&fsm[1],&fsm[3]}},
  { 9,{&fsm[2],&fsm[0]}},
  { 5,{&fsm[3],&fsm[1]}},
  { 6,{&fsm[0],&fsm[2]}}
};
uint8_t Pos;      // between 0 and 199
const struct State *Pt; // Current State

void Stepper_CW(uint32_t delay){ // Move 1.8 degrees clockwise
  Pt = Pt->Next[clockwise];      // circular
  P4OUT = (P4OUT&0xF0)|Pt->Out;  // step motor
  if(Pos==199){      // shaft angle
    Pos = 0;         // reset
  }
  else{
    Pos++; // CW
  }
  SysTick_Wait(delay);
}
void Stepper_CCW(uint32_t delay){  // Move 1.8 deg counterclockwise
  Pt = Pt->Next[counterclockwise]; // circular
  P4OUT = (P4OUT&0xF0)|Pt->Out; // step motor
  if(Pos==0){        // shaft angle
    Pos = 199;       // reset
  }
  else{
    Pos--; // CCW
  }
  SysTick_Wait(delay); // blind-cycle wait
}

// Initialize Stepper interface
void Stepper_Init(void){ // Initialize Stepper interface
  SysTick_Init();        // program 2.12
  Pos = 0; Pt = &fsm[0];
  P4DIR |= 0x0F;}        // enable 6 mA output


// Turn stepper motor to desired position
// (0 <= desired <= 199)
// time is the number of bus cycles to wait after each step
void Stepper_Seek(unsigned char desired, unsigned long time){
short CWsteps;
  if((CWsteps = (desired-Pos))<0){
    CWsteps+=200;
  } // CW steps is 0 to 199
  if(CWsteps > 100){
    while(desired != Pos){
      Stepper_CCW(time);
    }
  }
  else{
    while(desired != Pos){
      Stepper_CW(time);
    }
  }
}

