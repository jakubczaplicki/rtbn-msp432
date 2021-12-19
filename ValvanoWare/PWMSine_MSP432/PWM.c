// PWM.c
// Runs on MSP432
// This file is slightly modified from the original example.
// Single channel PWM implemented using Timer A0.0 and A0.1.
// Run at maximum speed of 12 MHz (assumes Clock_Init48MHz()
// is called elsewhere) and use timer clock dividers of /1
// and /1.
// MCLK = 48MHz HFXT; SMCLK = 12MHz; ACLK = 32.768kHz
// PWM on P2.4 using TimerA0 TA0.CCR1
// The timer counts up to TA0CCR0, clearing P2.4=0 when it
// matches TA0CCR1 on the way up.  When it reaches TA0CCR0,
// it begins counting down to 0, setting P2.4=1 when it
// matches TA0CCR1 on the way down.  The counts occur at 12
// MHz from the low-speed subsystem master clock (SMCLK),
// but since the timer counts from 0 to TA0CCR0 and back to
// 0, that gives TA0CCR0 and TA0CCR1 (i.e. the 'period' and
// 'duty1' parameters) units of 6 MHz (0.1667 us).
// Derived from msp432p401_portmap_01.c in MSPware
// Jonathan Valvano
// July 31, 2015

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

//***************************PWM_Init*******************************
// PWM output on P2.4
// Inputs:  period (0.1667us)
//          duty1
// Outputs: none
// SMCLK = 48MHz/4 = 12 MHz, 83.33ns
// Counter counts up to TA0CCR0, and then counts back down
// Let Timerclock period T = 1/12MHz = 83.33ns
// P2.4=1 when timer equals TA0CCR1 on way down, P2.4=0 when timer equals TA0CCR1 on way up
// period of P2.4 is period*0.1667us, duty cycle is duty1/period
void PWM_Init(uint16_t period, uint16_t duty1){
  if(duty1 >= period) return; // bad input
//  Clock_Init48MHz();      // 48 MHz HFXTCLK, SMCLK = 12 MHz
  P2DIR |= 0x10;          // make P2.4 output
  P2SEL0 |= 0x10;
  P2SEL1 &= ~0x10;        // configure P2.4 as Timer A0.1 function
  TA0CCTL0 = 0x0080;      // CCI0 toggle (P7.3 free for other use)
  TA0CCR0 = period;       // Squarewave 4*period*8 is
  TA0CCTL1 = 0x0040;      // CCR1 toggle/reset
  TA0CCR1 = duty1;        // CCR1 duty cycle is duty1/period
  TA0CTL = 0x0230;        // SMCLK=12MHz, divide by 1, up-down mode
// bit  mode
// 9-8  20    TASSEL, SMCLK=12MHz
// 7-6  00    ID, divide by 1
// 5-4  11    MC, up-down mode
// 2    0     TACLR, no clear
// 1    0     TAIE, no interrupt
// 0          TAIFG
}
//***************************PWM_Duty1*******************************
// change duty cycle of PWM output on P2.4
// Inputs:  duty1
// Outputs: none
// period of P2.4 is period*0.1667us, duty cycle is duty1/period
void PWM_Duty1(uint16_t duty1){
  if(duty1 >= TA0CCR0) return; // bad input
  TA0CCR1 = duty1;        // CCR1 duty cycle is duty1/period
}
