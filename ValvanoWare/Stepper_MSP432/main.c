// main.c
// Runs on MSP432
// Used to test the stepper.c driver
// Valvano
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
#include "stepper.h"

#define T1ms 3000
int main(void){
  Stepper_Init();
  Stepper_CW(T1ms);    // Pos=1; Port 4 =9
  Stepper_CW(T1ms);    // Pos=2; Port 4 =5
  Stepper_CW(T1ms);    // Pos=3; Port 4 =6
  Stepper_CW(T1ms);    // Pos=4; Port 4 =10
  Stepper_CW(T1ms);    // Pos=5; Port 4 =9
  Stepper_CW(T1ms);    // Pos=6; Port 4 =5
  Stepper_CW(T1ms);    // Pos=7; Port 4 =6
  Stepper_CW(T1ms);    // Pos=8; Port 4 =10
  Stepper_CW(T1ms);    // Pos=9; Port 4 =9
  Stepper_CCW(T1ms);   // Pos=8; Port 4 =10
  Stepper_CCW(T1ms);   // Pos=7; Port 4 =6
  Stepper_CCW(T1ms);   // Pos=6; Port 4 =5
  Stepper_CCW(T1ms);   // Pos=5; Port 4 =9
  Stepper_CCW(T1ms);   // Pos=4; Port 4 =10
  Stepper_Seek(8,T1ms);// Pos=8; Port 4 =10
  Stepper_Seek(0,T1ms);// Pos=0; Port 4 =10
  while(1){
    Stepper_CW(10*T1ms);   // output every 10ms
  }
}
