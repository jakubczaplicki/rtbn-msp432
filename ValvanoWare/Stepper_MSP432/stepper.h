// Stepper.h
// Runs on MSP432
// Provide functions that step the motor once clockwise, step
// once counterclockwise, and initialize the stepper motor
// interface.
// Daniel Valvano
// July 26, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Section 4.7.2, Program 4.4  4.5 and Program 4.6

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

// P4.3 connected to driver for stepper motor coil A
// P4.2 connected to driver for stepper motor coil A'
// P4.1 connected to driver for stepper motor coil B
// P4.0 connected to driver for stepper motor coil B'


// Initialize Stepper interface
void Stepper_Init(void);

// Move 1.8 degrees clockwise,
// delay is the time to wait after each step in bus cycles
void Stepper_CW(unsigned long delay);

// Move 1.8 degrees counterclockwise,
// delay is the time to wait after each step in bus cycles
void Stepper_CCW(unsigned long delay);

// Turn stepper motor to desired position
// (0 <= desired <= 199)
// time is the number of bus cycles to wait after each step
void Stepper_Seek(unsigned char desired, unsigned long time);

