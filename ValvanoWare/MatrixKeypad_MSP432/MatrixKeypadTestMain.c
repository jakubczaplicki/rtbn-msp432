// MatrixKeypadTestMain.c
// Runs on MSP432
// Test the functions provided by MatrixKeypadInit.c,
// MatrixKeypadScan.c, and MatrixKeypadInterrupts.c by
// initializing the keypad and looking at the contents of the
// FIFO in the debugger (variable name "Fifo").  Expect to see
// letters corresponding with the pressed buttons, which come
// from the struct in MatrixKeypadScan.c.  Be sure that the
// struct looks like the keypad you are interfacing.
// Daniel Valvano
// July 23, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015

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
#include "MatrixKeypad.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

char Button;
int main(void){
  MatrixKeypad_Init();
  EnableInterrupts();
  while(1){
    Button = MatrixKeypad_Get();// get character from background
    while(Button != 0){
      // do something in response
      Button = MatrixKeypad_Get();
    }
    WaitForInterrupt();         // interrupt when any column is pressed
    WaitForInterrupt();         // interrupt 10 ms later and poll all columns
  }
}
