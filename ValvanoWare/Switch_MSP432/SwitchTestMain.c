// SwitchTestMain.c
// Runs on MSP432
// Test the switch initialization functions by setting the LED
// color according to the status of the switches.
// Daniel and Jonathan Valvano
// July 2, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Section 4.2, Program 4.2, Figure 4.7

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

// built-in LED1 connected to P1.0
// negative logic built-in Button 1 connected to P1.1
// negative logic built-in Button 2 connected to P1.4
// positive logic switch connected to P1.5
// built-in red LED connected to P2.0
// built-in green LED connected to P2.1
// built-in blue LED connected to P2.2

#include <stdint.h>
#include "..\inc\msp432p401r.h"
#include "Switch.h"

#define SW1       0x02                  // on the left side of the LaunchPad board
#define SW2       0x10                  // on the right side of the LaunchPad board
#define SWEXT     0x20                  // external switch
#define RED       0x01
#define GREEN     0x02
#define BLUE      0x04
#define LED1      0x01

int main(void){ uint32_t status;
  Switch_Init();              // P1.5 is input
  Board_Init();               // initialize P1.1 and P1.4 and make them inputs (P1.1 and P1.4 built-in buttons)
  // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in RGB LEDs)
  P2SEL0 &= ~(RED|GREEN|BLUE);
  P2SEL1 &= ~(RED|GREEN|BLUE);// configure built-in RGB LEDs as GPIO
  P2DS |= (RED|GREEN|BLUE);   // make built-in RGB LEDs high drive strength
  P2DIR |= (RED|GREEN|BLUE);  // make built-in RGB LEDs out
  // initialize P1.0 and make it output (P1.0 built-in LED1)
  P1SEL0 &= ~LED1;
  P1SEL1 &= ~LED1;            // configure built-in LED1 as GPIO
  P1DIR |= LED1;              // make built-in LED1 out
  while(1){
    status = Board_Input();
    switch(status){           // switches are negative logic on P1.1 and P1.4
      case 0x10: P2OUT = (P2OUT&~(RED|GREEN))|BLUE; break;   // SW1 pressed
      case 0x02: P2OUT = (P2OUT&~(GREEN|BLUE))|RED; break;   // SW2 pressed
      case 0x00: P2OUT = (P2OUT&~(RED|BLUE))|GREEN; break;   // both switches pressed
      case 0x12: P2OUT = (P2OUT&~(RED|BLUE|GREEN)); break;   // neither switch pressed
      default: P2OUT = P2OUT|(RED|GREEN|BLUE);               // unexpected return value
    }
    status = Switch_Input();  // 0x00 or 0x20
    P1OUT = (P1OUT&~LED1)|((status>>5)&LED1);
  }
}
