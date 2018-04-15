// LaunchPad.h
// Runs on MSP432
// Input from switches, output to LED
// Jonathan Valvano
// July 19, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2016

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
 
// built-in LED1 connected to P1.0 (not used here)
// negative logic built-in Button 1 connected to P1.1
// negative logic built-in Button 2 connected to P1.4
// built-in red LED connected to P2.0
// built-in green LED connected to P2.1
// built-in blue LED connected to P2.2

#define RED       0x01
#define GREEN     0x02
#define BLUE      0x04
//------------LaunchPad_Init------------
// Initialize Switch input and LED output
// Input: none
// Output: none
void LaunchPad_Init(void);

//------------LaunchPad_Input------------
// Input from Switches 
// Input: none
// Output: 0x00 none
//         0x01 Button1
//         0x02 Button2
//         0x03 both Button1 and Button2
uint8_t LaunchPad_Input(void);

//------------LaunchPad__Output------------
// Input from Switches 
// Input: 0 off, bit0=red, bit1=green, bit2=blue
// Output: none
void LaunchPad_Output(uint8_t data);
