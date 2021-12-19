// PWM.c
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

//***************************PWM_Init*******************************
// squarewave output on P7.3, PWM outputs on P2.4, P2.5
// Inputs:  period (1.333us)
//          duty1
//          duty2
// Outputs: none
// SMCLK = 48MHz/4 = 12 MHz, 83.33ns
// Counter counts up to TA0CCR0 and back down, and then toggles P7.3
// Let Timerclock period T = 8/12MHz = 666.7ns
// period of P7.3 squarewave is 4*period*666.7ns
// P2.4=1 when timer equals TA0CCR1 on way down, P2.4=0 when timer equals TA0CCR1 on way up
// P2.5=1 when timer equals TA0CCR2 on way down, P2.5=0 when timer equals TA0CCR2 on way up
// Period of P2.4 is period*1.333us, duty cycle is duty1/period
// Period of P2.5 is period*1.333us, duty cycle is duty2/period
void PWM_Init(uint16_t period, uint16_t duty1, uint16_t duty2){
  if(duty1 >= period) return; // bad input
  if(duty2 >= period) return; // bad input
  Clock_Init48MHz();      // 48 MHz HFXTCLK, SMCLK = 12 MHz
  P7DIR |= 0x08;          // 7.3 output
  P7SEL0 |= 0x08;         // P7.3 Timer0A functions
  P7SEL1 &= ~0x08;        // P7.3 Timer0A functions
  P2DIR |= 0x30;          // P2.4, P2.5 output
  P2SEL0 |= 0x30;         // P2.4, P2.5 Timer0A functions
  P2SEL1 &= ~0x30;        // P2.4, P2.5 Timer0A functions
  TA0CCTL0 = 0x0080;      // CCI0 toggle
  TA0CCR0 = period;       // Period is 2*period*8*83.33ns is 1.333*period
  TA0EX0 = 0x0000;        //    divide by 1
  TA0CCTL1 = 0x0040;      // CCR1 toggle/reset
  TA0CCR1 = duty1;        // CCR1 duty cycle is duty1/period
  TA0CCTL2 = 0x0040;      // CCR2 toggle/reset
  TA0CCR2 = duty2;        // CCR2 duty cycle is duty2/period
  TA0CTL = 0x02F0;        // SMCLK=12MHz, divide by 8, up-down mode
// bit  mode
// 9-8  20    TASSEL, SMCLK=12MHz
// 7-6  11    ID, divide by 8
// 5-4  11    MC, up-down mode
// 2    0     TACLR, no clear
// 1    0     TAIE, no interrupt
// 0          TAIFG
}
//***************************PWM_Duty1*******************************
// change duty cycle of PWM output on P2.4
// Inputs:  duty1
// Outputs: none
// period of P2.4 is 2*period*666.7ns, duty cycle is duty1/period
void PWM_Duty1(uint16_t duty1){
  if(duty1 >= TA0CCR0) return; // bad input
  TA0CCR1 = duty1;        // CCR1 duty cycle is duty1/period
}
//***************************PWM_Duty2*******************************
// change duty cycle of PWM output on P2.5
// Inputs:  duty2
// Outputs: none// period of P2.5 is 2*period*666.7ns, duty cycle is duty2/period
void PWM_Duty2(uint16_t duty2){
  if(duty2 >= TA0CCR0) return; // bad input
  TA0CCR2 = duty2;        // CCR2 duty cycle is duty2/period
}
