// PointerTrafficLight.c
// Runs on MSP432
// Use a pointer implementation of a Moore finite state machine to operate
// a traffic light.
// Daniel Valvano
// May 10, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 6.8, Example 6.4
   Volume 2, Program 3.3

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

// east facing red light connected to P4.5
// east facing yellow light connected to P4.4
// east facing green light connected to P4.3
// north facing red light connected to P4.2
// north facing yellow light connected to P4.1
// north facing green light connected to P4.0
// north facing car detector connected to P5.1 (1=car present)
// east facing car detector connected to P5.0 (1=car present)

#include <stdint.h>
#include "SysTick.h"
#include "..\inc\msp432p401r.h"

struct State {
  uint8_t Out;             // 6-bit output
  uint32_t Time;           // 10 ms
  const struct State *Next[4];};// depends on 2-bit input
typedef const struct State STyp;
#define goN   &FSM[0]
#define waitN &FSM[1]
#define goE   &FSM[2]
#define waitE &FSM[3]
STyp FSM[4]={
 {0x21,300,{goN,waitN,goN,waitN}},
 {0x22, 50,{goE,goE,goE,goE}},
 {0x0C,300,{goE,goE,waitE,waitE}},
 {0x14, 50,{goN,goN,goN,goN}}};
// SENSOR is set to P5IN and LIGHT is set to P4OUT, making the code more readable.
#define SENSOR  (*((volatile uint8_t *)0x40004C40))
#define LIGHT   (*((volatile uint8_t *)0x40004C23))

int main(void){
  STyp *Pt;  // state pointer
  uint8_t Input;
  SysTick_Init();              // initialize SysTick timer
  // activate Port 4 and Port 5
  // initialize P4.5-P4.0 and make them outputs
  P4SEL0 &= ~0x3F;
  P4SEL1 &= ~0x3F;             // configure light pins as GPIO
  P4DIR |= 0x3F;               // make light pins out
  // initialize P5.1-P5.0 and make them inputs
  P5SEL0 &= ~0x03;
  P5SEL1 &= ~0x03;             // configure car detector pins as GPIO
  P5DIR &= ~0x03;              // make car detector pins in
  Pt = goN;                    // initial state: Green north; Red east
  while(1){
                               // set lights to current state's Out value
    LIGHT = (LIGHT&~0x3F)|(Pt->Out);
    SysTick_Wait10ms(Pt->Time);// wait 10 ms * current state's Time value
    Input = (SENSOR&0x03);     // get new input from car detectors
    Pt = Pt->Next[Input];      // transition to next state
  }
}

// Volume 2, program 3.3
#define BSP_InPort           P5IN
#define BSP_InPort_DIR       P5DIR
#define BSP_OutPort          P4OUT
#define BSP_OutPort_DIR      P4DIR
#define BSP_In_M             0x00000003  // bit mask for pins 1,0
#define BSP_In_Shift         0x00000000  // shift value for Input pins
#define BSP_Out_M            0x0000003F  // bit mask for pins 5-0
#define BSP_Out_Shift        0x00000000  // shift value for Output pins

int main2(void){ STyp *pt;  // state pointer
  uint32_t input;     
  SysTick_Init();          // initialize SysTick timer, program 2.12
  BSP_InPort_DIR &= ~BSP_In_M;  // make InPort pins inputs
  BSP_OutPort_DIR |= BSP_Out_M; // make OutPort pins out
  pt = goN;
  while(1){
    BSP_OutPort = (BSP_OutPort&(~BSP_Out_M))|((pt->Out)>>BSP_Out_Shift);
    SysTick_Wait10ms(pt->Time);
    input = (BSP_InPort&BSP_In_M)>>BSP_In_Shift; //00,01,10,11
    pt = pt->Next[input];
  }
}

