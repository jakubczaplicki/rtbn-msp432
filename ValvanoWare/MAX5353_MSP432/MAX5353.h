// MAX5353.h
// Runs on MSP432
// Use eUSCI3 to send two 8-bit codes to the MAX5353.
// Daniel Valvano
// July 29, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Volume 2, Example 7.2, Program 7.2

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

// MSP432         MAX5353     External
//                pin 1 OUT   analog out
// P9.4 GPIO      pin 2 CS*
// P9.7 UCA3SIMO  pin 3 DIN
// P9.5 UCA3CLK   pin 4 SCLK
//                pin 5 FB    10k from FB to OUT, 10k from FB to GND
//                pin 6 REF   1.233V ref in
// GND            pin 7 GND
// +3.3 power     pin 8 Vdd
// see Figure 7.19 for complete schematic

//********DAC_Init*****************
// Initialize Max5353 12-bit DAC
// inputs: initial 12-bit digital code in bits 12:1 (0,2,4,...8190)
// outputs: none
void DAC_Init(uint16_t data);

//********DAC_Out*****************
// Send data to Max5353 12-bit DAC
// inputs: code with data (0 to 4095) in bits 12:1
// outputs: none
// 15:13 = 000  three control bits for immediate DAC update
// 12:1  = 12-bit digital data
// 0     = S0  sub-bit should be zero
// book Figure 7.20 shows MAX5353 in unipolar rail-to-rail configuration
// code=0x0000 , output = 0 (min)
// code=0x0002 , output = Vref/2048 (resolution)
// code=0x1000 , output = Vref
// code=0x1FFE , output = 2*Vref (max)
void DAC_Out(uint16_t code);
