// 74HC595.h
// Runs on MSP432
// Use eUSCI3 to send an 8-bit code to the 74HC595.
// Output port expander
// Daniel Valvano
// July 21, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Program 7.4

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

// MSP432        74HC595
//   +3.3        pin 16 Vcc power
//   Gnd         pin 8  ground
//   Gnd         pin 13 OE*
//  +3.3         pin 10 SRCLR*
// P9.5 UCA3CLK  pin 11 SRCLK
// P9.4 UCA3STE  pin 12 RCLK
// P9.7 UCA3SIMO pin 14 SER

// Port          74HC595
// bit 7 (MSB)   pin 7  Qh
// bit 6         pin 6  Qg
// bit 5         pin 5  Qf
// bit 4         pin 4  Qe
// bit 3         pin 3  Qd
// bit 2         pin 2  Qc
// bit 1         pin 1  Qb
// bit 0 (LSB)   pin 15 Qa

// see Figure 7.19 for complete schematic

//********Port_Init*****************
// Initialize 74HC595 serial shift register
// inputs:  none
// outputs: none
void Port_Init(void);

//********Port_Out*****************
// Send data to 74HC595 8-bit port
// inputs:  output (0 to 255)
// outputs: none
void Port_Out(uint8_t code);
