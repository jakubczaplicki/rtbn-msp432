// MatrixKeypadScan.c
// Runs on MSP432
// Provide a function that scans the matrix keypad and returns
// the ASCII code for the key pressed and number of keys pressed.
// Use a data structure based on the appearance of the keypad.
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

struct Row{
  uint8_t direction;// direction register mask
  char keycode[4];};
typedef const struct Row RowType;
RowType ScanTab[5]={
{   0x01, "123A" }, // row 0
{   0x02, "456B" }, // row 1
{   0x04, "789C" }, // row 2
{   0x08, "*0#D" }, // row 3
{   0x00, "    " }};

//------------MatrixKeypad_Scan------------
// Scan the matrix keypad and return the character
// associated with the pressed key.  Also return the
// number of pressed keys.
// Input: Num 16-bit pointer to the number of keys pressed
// Output: ASCII code for key pressed (0 if none pressed)
char MatrixKeypad_Scan(int16_t *Num){
  RowType *pt;
  char key;
  uint8_t column;
  uint16_t j;
  (*Num) = 0;
  key = 0;    // default values
  pt = &ScanTab[0];
  while(pt->direction){
    P7DIR = pt->direction;         // one output
    P7OUT &= ~0x0F;                // DIRn=0, OUTn=HiZ; DIRn=1, OUTn=0
    for(j=1; j<=10; j++);          // very short delay
    column = (P4IN&0x0F);          // read columns
    for(j=0; j<=3; j++){
      if((column&0x01)==0){
        key = pt->keycode[j];
        (*Num)++;
      }
      column>>=1;  // shift into position
    }
    pt++;
  }
  return key;
}

/* Waits for a key be pressed, then released
    Returns ASCII code for key pressed,
   Num is the number of keys pressed
   both equal zero if no key pressed */
char MatrixKeypad_In(void){ int16_t n;
char letter;
  do{
    letter = MatrixKeypad_Scan(&n);
  } while (n != 1); // repeat until exactly one
  do{
    letter = MatrixKeypad_Scan(&n);
  } while (n != 0); // repeat until release
  return letter;
}
