//*****************************************************************************
// user.c
// Runs on MSP432
// An example user program that initializes the simple operating system
//   Schedule three independent threads using preemptive round robin
//   Each thread rapidly toggles a pin on Port 2 and increments its counter
//   TIMESLICE is how long each thread runs
// Daniel Valvano
// August 25, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015

   Programs 4.4 through 4.12, section 4.2

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
#include "os.h"
#include "..\inc\msp432p401r.h"

#define TIMESLICE               TIME_2MS    // thread switch time in system time units

uint32_t Count1;   // number of times thread1 loops
uint32_t Count2;   // number of times thread2 loops
uint32_t Count3;   // number of times thread3 loops
// P2OUT at 0x40004C03
#define LED_RED   (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*0)))  /* Port 2.0 Output */
#define LED_GREEN (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*1)))  /* Port 2.1 Output */
#define LED_BLUE  (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*2)))  /* Port 2.2 Output */

void Task1(void){
  Count1 = 0;
  for(;;){
    Count1++;
    LED_RED ^= 0x01;   // toggle P2.0
  }
}
void Task2(void){
  Count2 = 0;
  for(;;){
    Count2++;
    LED_GREEN ^= 0x01; // toggle P2.1
  }
}
void Task3(void){
  Count3 = 0;
  for(;;){
    Count3++;
    LED_BLUE ^= 0x01;  // toggle P2.2
  }
}
int main(void){
  OS_Init();           // initialize, disable interrupts, 48 MHz
  // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in RGB LEDs)
  P2SEL0 &= ~0x07;
  P2SEL1 &= ~0x07;     // configure built-in RGB LEDs as GPIO
  P2DS |= 0x07;        // make built-in RGB LEDs high drive strength
  P2DIR |= 0x07;       // make built-in RGB LEDs out
  P2OUT &= ~0x07;      // RGB = off
  OS_AddThreads(&Task1, &Task2, &Task3);
  OS_Launch(TIMESLICE);// doesn't return, interrupts enabled in here
  return 0;            // this never executes
}
