// ADCTA0Trigger.h
// Runs on MSP432
// Use Timer A0 in periodic mode to trigger ADC conversions at a
// particular period.  Request an interrupt when the conversion is
// complete.
// Daniel Valvano
// July 8, 2015

/* This example accompanies the books
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

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

#ifndef __ADCTA0TRIGGER_H__ // do not include more than once
#define __ADCTA0TRIGGER_H__

// ***************** ADC0_InitTA0TriggerCh6 ****************
// Activate Timer A0 to periodically trigger ADC conversions
// on P4.7 = A6. Run the user task when each conversion is
// complete.
// Inputs:  task is a pointer to a user function
//          period in units (1/SMCLK), 16 bits
// Outputs: none
void ADC0_InitTA0TriggerCh6(void(*task)(uint16_t result), uint16_t period);

#endif // __ADCTA0TRIGGER_H__
