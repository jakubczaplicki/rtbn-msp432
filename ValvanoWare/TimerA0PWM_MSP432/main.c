// main.c
// Runs on MSP432
// Squarewave on P7.3 using TimerA0 TA0.CCR0
// PWM on P2.4 using TimerA0 TA0.CCR1
// PWM on P2.5 using TimerA0 TA0.CCR2
// MCLK = SMCLK = 3MHz DCO; ACLK = 32.768kHz
// TACCR0 generates a square wave of freq ACLK/1024 =32Hz
// Derived from msp432p401_portmap_01.c in MSPware
// Jonathan Valvano
// July 24, 2015

/* This example accompanies the book
   "Embedded Systems: Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Volume 2, Program 9.8

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
#include "ClockSystem.h"
#include "PWM.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// 50Hz squarewave on P7.3
// P2.4=1 when timer equals TA0CCR1 on way down, P2.4=0 when timer equals TA0CCR1 on way up
// P2.5=1 when timer equals TA0CCR2 on way down, P2.5=0 when timer equals TA0CCR2 on way up
int main(void){int i; uint16_t duty;
//  PWM_Init(15000,11250,3750);
  // Squarewave Period/4 (20ms)
  // P2.4 CCR1 75% PWM duty cycle, 10ms period
  // P2.5 CCR2 25% PWM duty cycle, 10ms period
  PWM_Init(10,7,2);
  //while(1){};
  // Squarewave Period/4 (20us)
  // P2.4 CCR1 66% PWM duty cycle, 13.3us period
  // P2.5 CCR2 33% PWM duty cycle, 10us period
  while(1){
    for(duty = 50; duty<15000; duty = duty+100){
      PWM_Duty1(duty);
      PWM_Duty2(15000-duty);
      for(i=1000000;i;i--){};
    }
  }
}
