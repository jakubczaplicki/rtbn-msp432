// FreqMeasureTestMain.c
// Runs on MSP432
// Measures frequency on P7.1/TA0CLK input
// Timer A1 in periodic mode to request interrupts at 100 Hz
// Daniel Valvano,  Jonathan Valvano
// July 30, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Program 6.9, example 6.8

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
// input signal connected to P7.1/TA0CLK (count rising edges)
// for this test, connect P2.0 to P7.1

#include <stdint.h>
#include "ClockSystem.h"
#include "FreqMeasure.h"
#include "..\inc\msp432p401r.h"

// P1IN at 0x40004C00
#define SWITCH_1  (*((volatile uint8_t *)(0x42000000+32*0x4C00+4*1)))  /* Port 1.1 Input */
#define SWITCH_2  (*((volatile uint8_t *)(0x42000000+32*0x4C00+4*4)))  /* Port 1.4 Input */
// P1OUT at 0x40004C02
#define LED_1     (*((volatile uint8_t *)(0x42000000+32*0x4C02+4*0)))  /* Port 1.0 Output */
// P2OUT at 0x40004C03
#define LED_RED   (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*0)))  /* Port 2.0 Output */
#define LED_GREEN (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*1)))  /* Port 2.1 Output */
#define LED_BLUE  (*((volatile uint8_t *)(0x42000000+32*0x4C03+4*2)))  /* Port 2.2 Output */

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

void boardinit(void){
  // initialize P1.0 and make it output (P1.0 built-in LED1)
  P1SEL0 &= ~0x01;
  P1SEL1 &= ~0x01;                 // configure built-in LED1 as GPIO
  P1DIR |= 0x01;                   // make built-in LED1 out
  P1OUT &= ~0x01;                  // LED1 = off
  // initialize P1.4 and P1.1 and make them inputs (P1.4 and P1.1 built-in buttons)
  P1SEL0 &= ~0x12;
  P1SEL1 &= ~0x12;                 // configure P1.4 and P1.1 as GPIO
  P1DIR &= ~0x12;                  // make P1.4 and P1.1 in
  P1REN |= 0x12;                   // enable pull resistors on P1.4 and P1.1
  P1OUT |= 0x12;                   // P1.4 and P1.1 are pull-up
  // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in RGB LEDs)
  P2SEL0 &= ~0x07;
  P2SEL1 &= ~0x07;                 // configure built-in RGB LEDs as GPIO
  P2DS |= 0x07;                    // make built-in RGB LEDs high drive strength
  P2DIR |= 0x07;                   // make built-in RGB LEDs out
  P2OUT &= ~0x07;                  // RGB = off
}

//test code
uint32_t Time;
uint32_t Latest;
void squarewave(void){uint32_t i;
  LED_RED = 0;
  for(i=0; i<Time; i++){};
  LED_RED = 1;
  for(i=0; i<Time; i++){};
}
int main(void){
  Time = 1;
  Clock_Init48MHz();               // bus clock at 48 MHz
  boardinit();
  FreqMeasure_Init();              // initialize Timer A1 (100 Hz) and Timer A0 (edge count)
  EnableInterrupts();

  while(1){
    while(SWITCH_2){squarewave();} // wait for touch
    while(SWITCH_2 == 0){squarewave();}// wait for release
    Latest = FreqMeasure_Get();
    LED_BLUE ^= 1;
    Time = 2*Time; // slower
  }
}
