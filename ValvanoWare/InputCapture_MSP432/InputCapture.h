// InputCapture.h
// Runs on MSP432
// Use Timer A0 in capture mode to request interrupts on the rising
// edge of P7.3 (TA0CCP0), and call a user function.
// Daniel Valvano
// July 27, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Example 6.1, Program 6.1

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

// external signal connected to P7.3 (TA0CCP0) (trigger on rising edge)

//------------TimerCapture_Init------------
// Initialize Timer A0 in edge time mode to request interrupts on
// the rising edge of P7.3 (TA0CCP0).  The interrupt service routine
// acknowledges the interrupt and calls a user function.
// Input: task is a pointer to a user function called when edge occurs
//             parameter is 16-bit up-counting timer value when edge occurred
// Output: none
void TimerCapture_Init(void(*task)(uint16_t time));
