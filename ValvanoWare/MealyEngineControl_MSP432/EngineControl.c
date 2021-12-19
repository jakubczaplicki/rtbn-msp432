// EngineControl.c
// Runs on MSP432
// Use a pointer implementation of a Mealy finite state machine to operate
// an engine with a control input, gas and brake outputs, and two states.
// Daniel Valvano
// May 11, 2015

//  This example accompanies the book
//   "Embedded Systems: Introduction to the MSP432 Microcontroller",
//   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
//  Volume 1, Program 6.9, Section 6.5.3
//
//Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
//   You may use, edit, run or distribute this file
//   as long as the above copyright notice remains
//THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
//OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
//MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
//VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
//OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
//For more information about my classes, my research, and my books, see
//http://users.ece.utexas.edu/~valvano/

// control input connected to P4.0 (1=go)
// gas output connected to P4.1
// brake output connected to P4.2

#include <stdint.h>
#include "SysTick.h"
#include "..\inc\msp432p401r.h"

// Linked data structure
struct State {
  uint8_t Out[2];              // 2-bit outputs
  uint32_t Delay;              // 10 ms
  const struct State *Next[2];};// depends on 1-bit input and current state
typedef const struct State STyp;
#define Stop  &FSM[0]
#define Go    &FSM[1]
STyp FSM[2]={
 {{2,0},10,{Stop,Go}},
 {{0,1},10,{Stop,Go}}};

int main(void){
  STyp *Pt;  // state pointer
  uint8_t Input;
  SysTick_Init();              // initialize SysTick timer
  // activate Port 4
  // initialize P4.2-P4.0
  P4SEL0 &= ~0x07;
  P4SEL1 &= ~0x07;             // configure all pins as GPIO
  P4DIR |= 0x06;               // make Gas and Brake pins out
  P4DIR &= ~0x01;              // make Control pin in
  Pt = Stop;                   // initial state: stopped
  while(1){
    Input = (P4IN&0x01);       // get new input from Control
                               // output to Brake and Gas
    P4OUT = (P4OUT&~0x06)|(Pt->Out[Input]<<1);
    SysTick_Wait10ms(Pt->Delay);// wait 10 ms * current state's Delay value
    Pt = Pt->Next[Input];      // transition to next state
  }
}
