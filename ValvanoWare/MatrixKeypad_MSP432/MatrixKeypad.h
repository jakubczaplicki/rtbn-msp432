// MatrixKeypad.h
// Runs on MSP432
// Header file for the matrix keypad that contains function
// prototypes.
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

#ifndef __MATRIXKEYPAD_H__ // do not include more than once
#define __MATRIXKEYPAD_H__

// initialize index FIFO
void Fifo_Init(void);
// add element to end of index FIFO
// return TXFIFOSUCCESS if successful
int Fifo_Put(char data);
// remove element from front of index FIFO
// return TXFIFOSUCCESS if successful
int Fifo_Get(char *datapt);
// number of elements in index FIFO
// 0 to TXFIFOSIZE-1
unsigned short Fifo_Size(void);

//------------MatrixKeypad_Init------------
// Initialize the matrix keypad interface and arm the
// GPIO interrupt to respond to a button press.
// Input: none
// Output: none
// Assumes: global interrupt enable must happen in outside function
void MatrixKeypad_Init(void);

//------------MatrixKeypad_Arm------------
// Arm the GPIO interrupt to respond to a matrix keypad
// button press.
// Input: none
// Output: none
void MatrixKeypad_Arm(void);

//------------MatrixKeypad_Get------------
// Return the character associated with the oldest
// button press by looking at the internal FIFO.
// This function will immediately return 0 without
// stalling or waiting if no button has been pressed.
// Input: none
// Output: ASCII code for key pressed (0 if none pressed)
char MatrixKeypad_Get(void);

//------------MatrixKeypad_Scan------------
// Scan the matrix keypad and return the character
// associated with the pressed key.  Also return the
// number of pressed keys.
// Input: Num 16-bit pointer to the number of keys pressed
// Output: ASCII code for key pressed (0 if none pressed)
char MatrixKeypad_Scan(int16_t *Num);

#endif // __MATRIXKEYPAD_H__
