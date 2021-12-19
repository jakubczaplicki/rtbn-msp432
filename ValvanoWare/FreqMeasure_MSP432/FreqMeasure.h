// FreqMeasure.h
// Runs on MSP432
// Measures frequency on P7.1/TA0CLK input
// Timer A1 in periodic mode to request interrupts at 100 Hz
// Daniel Valvano,  Jonathan Valvano
// July 30, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Program 6.9, example 6.8

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

#ifndef __FREQMEASURE_H__ // do not include more than once
#define __FREQMEASURE_H__

// ***************** FreqMeasure_Init ****************
// Activate Timer A1 interrupts to take measurements every
// 10 ms by counting rising edges on P7.1/TA0CLK
// Assumes Clock_Init48MHz() has been called
// Inputs: none
// Outputs: none
void FreqMeasure_Init(void);

// ***************** FreqMeasure_Get ****************
// Get the most recent measurement of the number of rising
// edges on P7.1/TA0CLK within the last 10 ms window.
// Stall if no measurement is ready.
// Assumes FreqMeasure_Init() called and interrupts enabled
// Inputs: none
// Outputs: number of rising edges on P7.1/TA0CLK in 10 ms
//          frequency units are 100 Hz
//  E.g., if it returns 123 it means the frequency is 123edges/10ms = 12300 Hz
uint32_t FreqMeasure_Get(void);

#endif // __FREQMEASURE_H__
