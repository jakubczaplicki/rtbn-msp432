// MatrixKeypadInit.c
// Runs on MSP432
// Provide functions that initialize GPIO ports and timers and
// arm the matrix keypad to respond to a button press.
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
#include "FIFO.h"
#include "..\inc\msp432p401r.h"
#include "MatrixKeypad.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
#define FIFOSIZE   16         // size of the FIFOs (must be power of 2)
#define FIFOSUCCESS 1         // return value on success
#define FIFOFAIL    0         // return value on failure
                              // create index implementation FIFO (see FIFO.h)
AddIndexFifo(, FIFOSIZE, char, FIFOSUCCESS, FIFOFAIL)

//------------MatrixKeypad_Arm------------
// Arm the GPIO interrupt to respond to a matrix keypad
// button press.
// Input: none
// Output: none
void MatrixKeypad_Arm(void){
  P4DIR &= ~0x0F;                  // make P4.3-P4.0 in (P4.3-P4.0 columns)
  P7DIR |= 0x0F;                   // make P7.3-P7.0 out (P7.3-P7.0 rows)
  P7OUT &= ~0x0F;                  // DIRn=0, OUTn=HiZ; DIRn=1, OUTn=0
  P4IFG &= ~0x0F;                  // clear flags3-0
  P4IE |= 0x0F;                    // enable interrupt on P4.3-P4.0
  TA0CTL &= ~0x0030;               // halt Timer A0
  TA0CCTL0 &= ~0x0001;             // clear capture/compare interrupt 0 flag
}

//------------MatrixKeypad_Init------------
// Initialize the matrix keypad interface and arm the
// GPIO interrupt to respond to a button press.
// Input: none
// Output: none
// Assumes: global interrupt enable must happen in outside function
void MatrixKeypad_Init(void){ long sr;
  sr = StartCritical();
  // **** general initialization ****
  Fifo_Init();                     // initialize empty FIFOs
  P7OUT &= ~0x0F;                  // DIRn=0, OUTn=HiZ; DIRn=1, OUTn=0
  P7DIR &= ~0x0F;                  // make P7.3-P7.0 in (P7.3-P7.0 rows)
  P4DIR &= ~0x0F;                  // make P4.3-P4.0 in (P4.3-P4.0 columns)
  // **** GPIO Port 4 interrupt initialization ****
  P4IFG &= ~0x0F;                  // clear flags3-0 (reduce possibility of extra interrupt)
  P4IES |= 0x0F;                   // P4.3-P4.0 are falling edge event
  // **** Timer A0 initialization ****
  TA0CTL &= ~0x0030;               // halt Timer A0
  // bits15-10=XXXXXX, reserved
  // bits9-8=01,       clock source to ACLK (LFXTCLK or REFOCLK 32,768 Hz by default)
  // bits7-6=00,       input clock divider /1
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=1,           interrupt enable
  // bit0=0,           clear interrupt pending
  TA0CTL = 0x0102;
  // bits15-14=00,     no capture mode
  // bits13-12=XX,     capture/compare input select
  // bit11=X,          synchronize capture source
  // bit10=X,          synchronized capture/compare input
  // bit9=X,           reserved
  // bit8=0,           compare mode
  // bits7-5=XXX,      output mode
  // bit4=1,           enable capture/compare interrupt
  // bit3=X,           read capture/compare input from here
  // bit2=0,           output this value in output mode 0
  // bit1=X,           capture overflow status
  // bit0=0,           clear capture/compare interrupt pending
  TA0CCTL0 = 0x0010;
  TA0CCR0 = 327;                   // interrupt after 10 ms
  TA0EX0 &= ~0x0007;               // configure for input clock divider /1
  TA0CTL |= 0x0014;                // reset and start Timer A0 in up mode
  // **** interrupt initialization ****
  MatrixKeypad_Arm();              // P7.3-P7.0 out; P4.3-P4.0 in
                                   // GPIO Port 4 = priority 2
  NVIC_IPR9 = (NVIC_IPR9&0xFF00FFFF)|0x00400000; // priority 2
                                   // Timer A0 = priority 2
  NVIC_IPR2 = (NVIC_IPR2&0xFFFFFF00)|0x00000040; // priority 2
  NVIC_ISER0 = 0x00000100;         // enable interrupt 8 in NVIC
  NVIC_ISER1 = 0x00000040;         // enable interrupt 38 in NVIC
  EndCritical(sr);
}

//------------MatrixKeypad_Get------------
// Return the character associated with the oldest
// button press by looking at the internal FIFO.
// This function will immediately return 0 without
// stalling or waiting if no button has been pressed.
// Input: none
// Output: ASCII code for key pressed (0 if none pressed)
char MatrixKeypad_Get(void){
  char button = 0;
  Fifo_Get(&button);
  return button;
}
