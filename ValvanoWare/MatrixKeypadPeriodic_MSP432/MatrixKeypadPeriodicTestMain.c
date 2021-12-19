// MatrixKeypadPeriodicTestMain.c
// Runs on MSP432
// Test the periodic matrix keypad interface by continually
// waiting for button presses and toggling built-in LEDs.
// Daniel Valvano
// July 24, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Example 5.4, Figure 5.18, Program 5.13

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

// P4.3 connected to column 3 (keypad pin 4) using 10K pull-up
// P4.2 connected to column 2 (keypad pin 3) using 10K pull-up
// P4.1 connected to column 1 (keypad pin 2) using 10K pull-up
// P4.0 connected to column 0 (keypad pin 1) using 10K pull-up
// P7.3 connected to row 3 (keypad pin 8)
// P7.2 connected to row 2 (keypad pin 7)
// P7.1 connected to row 1 (keypad pin 6)
// P7.0 connected to row 0 (keypad pin 5)

// [1] [2] [3] [A]
// [4] [5] [6] [B]
// [7] [8] [9] [C]
// [*] [0] [#] [D]
// Pin1 . . . . . . . . Pin8
// Pin 1 -> Column 0 (column starting with 1)
// Pin 2 -> Column 1 (column starting with 2)
// Pin 3 -> Column 2 (column starting with 3)
// Pin 4 -> Column 3 (column starting with A)
// Pin 5 -> Row 0 (row starting with 1)
// Pin 6 -> Row 1 (row starting with 4)
// Pin 7 -> Row 2 (row starting with 7)
// Pin 8 -> Row 3 (row starting with *)

// see Figure 4.39

#include <stdint.h>
#include "..\inc\msp432p401r.h"
#include "MatrixKeypadPeriodic.h"

#define RED       0x01
#define GREEN     0x02
#define BLUE      0x04
#define WHEELSIZE 8           // must be an integer power of 2
                              //    red, yellow,    green, light blue, blue, purple,   white,          dark
const long COLORWHEEL[WHEELSIZE] = {RED, RED+GREEN, GREEN, GREEN+BLUE, BLUE, BLUE+RED, RED+GREEN+BLUE, 0};

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile char Buffer[16];
volatile uint32_t KeyCount;
int main(void){ uint32_t i, j;
  MatrixKeypadPeriodic_Init();
  KeyCount = 0;
  i = 0;                           // index into Buffer
  j = 0;                           // index into COLORWHEEL
  // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in RGB LEDs)
  P2SEL0 &= ~0x07;
  P2SEL1 &= ~0x07;                 // configure built-in RGB LEDs as GPIO
  P2DS |= 0x07;                    // make built-in RGB LEDs high drive strength
  P2DIR |= 0x07;                   // make built-in RGB LEDs out
  P2OUT &= ~0x07;                  // RGB = off
  while(1){
    Buffer[i] = MatrixKeypad_InChar();
    i = (i + 1)&0x0F;
    P2OUT = (P2OUT&~0x07)|(COLORWHEEL[j&(WHEELSIZE-1)]);
    j = j + 1;
    KeyCount++;  // debugging
  }
}
