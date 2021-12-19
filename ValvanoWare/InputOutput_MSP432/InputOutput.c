// InputOutput.c
// Runs on MSP432
// Test the GPIO initialization functions by setting the LED
// color according to the status of the switches.
// Daniel and Jonathan Valvano
// June 20, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Section 4.2    Program 4.1

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
// built-in red LED connected to P2.0
// built-in green LED connected to P2.1
// built-in blue LED connected to P2.2

#include <stdint.h>
#include "..\inc\msp432p401r.h"

#define SW1       0x02                  // on the left side of the LaunchPad board
#define SW2       0x10                  // on the right side of the LaunchPad board
#define RED       0x01
#define GREEN     0x02
#define BLUE      0x04
// Volume 1 version is unfriendly
void Port1_Init(void){
  P1SEL0 = 0x00;
  P1SEL1 = 0x00;                        // configure P1.4 and P1.1 as GPIO
  P1DIR = 0x00;                         // make P1.4 and P1.1 in
  P1REN = 0x12;                         // enable pull resistors on P1.4 and P1.1
  P1OUT = 0x12;                         // P1.4 and P1.1 are pull-up
}
uint8_t Port1_Input(void){
  return (P1IN&0x12);                   // read P1.4,P1.1 inputs
}

void Port2_Init(void){
  P2SEL0 = 0x00;
  P2SEL1 = 0x00;                        // configure P2.2-P2.0 as GPIO
  P2DS = 0x07;                          // make P2.2-P2.0 high drive strength
  P2DIR = 0x07;                         // make P2.2-P2.0 out
  P2OUT = 0x00;                         // all LEDs off
}

void Port2_Output(uint8_t data){        // write all of P2 outputs
  P2OUT = data;
}
// Volume 2 version is friendly
void Port1_Initb(void){
  P1SEL0 &= ~0x12;
  P1SEL1 &= ~0x12;    // 1) configure P1.4 and P1.1 as GPIO
  P1DIR &= ~0x12;     // 2) make P1.4 and P1.1 in
  P1REN |= 0x12;      // 3) enable pull resistors on P1.4 and P1.1
  P1OUT |= 0x12;      //    P1.4 and P1.1 are pull-up
}
uint8_t Port1_Inputb(void){
  return (P1IN&0x12);   // read P1.4,P1.1 inputs
}
void Port2_Initb(void){
  P2SEL0 &= ~0x07;
  P2SEL1 &= ~0x07;    // 1) configure P2.2-P2.0 as GPIO
  P2DIR |= 0x07;      // 2) make P2.2-P2.0 out
  P2DS |= 0x07;       // 3) activate increased drive strength
  P2OUT &= ~0x07;     //    all LEDs off
}
void Port2_Outputb(uint8_t data){  // write three outputs bits of P2
  P2OUT = (P2OUT&0xF8)|data;
}

int main(void){ uint8_t status;
  Port1_Init();                         // initialize P1.1 and P1.4 and make them inputs (P1.1 and P1.4 built-in buttons)
  Port2_Init();                         // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in LEDs)
  while(1){
    status = Port1_Input();
    switch(status){                     // switches are negative logic on P1.1 and P1.4
      case 0x10: Port2_Output(BLUE); break;   // SW1 pressed
      case 0x02: Port2_Output(RED); break;    // SW2 pressed
      case 0x00: Port2_Output(GREEN); break;  // both switches pressed
      case 0x12: Port2_Output(0); break;      // neither switch pressed
    }
  }
}
// Color    LED(s) Port2
// dark     ---    0
// red      R--    0x01
// blue     --B    0x04
// green    -G-    0x02
// yellow   RG-    0x03
// sky blue -GB    0x06
// white    RGB    0x07
// pink     R-B    0x05

// examples from Volume 2
void LED_Init(void){
  P1SEL0 &= ~0x01;
  P1SEL1 &= ~0x01;   // 1) configure P1.0 as GPIO
  P1DIR |= 0x01;     // 2) make P1.0 out
}
void LED_On(void){
  P1OUT |= 0x01;   // turn on LED
}
void LED_Off(void){
  P1OUT &= ~0x01;  // turn off LED
}
void LED_Output(uint8_t data){  // 0 for off, 1 for on
  P1OUT = (P1OUT&0xFE)|data;
}
#define LEDOUT (*((volatile uint8_t *)(0x42098040)))
void LED_On2(void){
  LEDOUT = 0x01;   // turn on LED
}
void LED_Off2(void){
  LEDOUT = 0x00;  // turn off LED
}
void LED_Output2(uint8_t data){  // 0 for off, 1 for on
  LEDOUT = data;
}
#define SW2IN (*((volatile uint8_t *)(0x42098010)))
#define SW1IN (*((volatile uint8_t *)(0x42098004)))
#define BLUEOUT  (*((volatile uint8_t *)(0x42098068)))
#define GREENOUT (*((volatile uint8_t *)(0x42098064)))
#define REDOUT   (*((volatile uint8_t *)(0x42098060)))

int main2(void){
  Port1_Init();
  Port2_Init();
  LED_Init();
  BLUEOUT  = 0;   // yellow
  GREENOUT = 1;
  REDOUT   = 1;

  while(1){
	if((SW1IN==0)||(SW2IN==0)){ // if either is pressed
	  LED_On2();
	}else{
	  LED_Off2();
	}
	if((SW1IN==0)){ // if either is pressed
	  BLUEOUT = 1;
	}else{
	  BLUEOUT = 0;
	}
	if((SW2IN==0)){ // if either is pressed
	  REDOUT = 1;
	}else{
	  REDOUT = 0;
	}
  }
}
