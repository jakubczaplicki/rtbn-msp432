// Switch.h
// Runs on MSP432
// Provide functions that initialize a GPIO as an input pin and
// allow reading of two negative logic switches on P1.1 and P1.4
// and an external switch on P1.5.
// Daniel and Jonathan Valvano
// April 22, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Section 4.2, Program 4.2, Figure 4.7

  "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Example 2.3, Program 2.9, Figure 2.36

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

//------------Switch_Init------------
// Initialize GPIO Port 1 bit 5 for input.  An external pull-down
// resistor is used.
// Input: none
// Output: none
void Switch_Init(void);

//------------Switch_Input------------
// Read and return the status of GPIO Port 1 bit 5.
// Input: none
// Output: 0x20 if P1.5 is high
//         0x00 if P1.5 is low
uint32_t Switch_Input(void);

//------------Board_Init------------
// Initialize GPIO Port 1 for negative logic switches on P1.1 and
// P1.4 as the LaunchPad is wired.  Weak internal pull-up
// resistors are enabled.
// Input: none
// Output: none
void Board_Init(void);

//------------Board_Input------------
// Read and return the status of the switches.
// Input: none
// Output: 0x10 if only Switch 1 is pressed
//         0x02 if only Switch 2 is pressed
//         0x00 if both switches are pressed
//         0x12 if no switches are pressed
uint32_t Board_Input(void);
