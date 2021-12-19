// LED.h
// Runs on MSP432
// Provide functions that initialize a GPIO as an input pin and
// allow reading of two negative logic switches on P1.1 and P1.4.
// Use bit-banded I/O.
// Daniel and Jonathan Valvano
// October 8, 2015

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Section 4., Program 4.2

   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
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


//------------LED_Init------------
// Initialize GPIO Port 1 for negative logic switches on P1.1 and
// P1.4 as the LaunchPad is wired.  Weak internal pull-up
// resistors are enabled.  Initialize GPIO Port 2 for LEDs on P2.2,
// P2.1, and P2.0.
// Input: none
// Output: none
void LED_Init(void);

//------------Board_Input------------
// Read and return the status of the switches.
// Input: none
// Output: 2 if only Switch 1 is pressed
//         1 if only Switch 2 is pressed
//         3 if both switches are pressed
//         0 if no switches are pressed
uint32_t Board_Input(void);

//------------LED_RedOn------------
// Turn on red LED.
// Input: none
// Output: none
void LED_RedOn(void);

//------------LED_RedOff------------
// Turn off red LED.
// Input: none
// Output: none
void LED_RedOff(void);

//------------LED_RedToggle------------
// Toggle red LED.
// Input: none
// Output: none
void LED_RedToggle(void);

//------------LED_GreenOn------------
// Turn on green LED.
// Input: none
// Output: none
void LED_GreenOn(void);

//------------LED_GreenOff------------
// Turn off green LED.
// Input: none
// Output: none
void LED_GreenOff(void);

//------------LED_GreenToggle------------
// Toggle green LED.
// Input: none
// Output: none
void LED_GreenToggle(void);

//------------LED_BlueOn------------
// Turn on blue LED.
// Input: none
// Output: none
void LED_BlueOn(void);

//------------LED_BlueOff------------
// Turn off blue LED.
// Input: none
// Output: none
void LED_BlueOff(void);

//------------LED_BlueToggle------------
// Toggle blue LED.
// Input: none
// Output: none
void LED_BlueToggle(void);
