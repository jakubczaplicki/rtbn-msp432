// MAX5353TestMain.c
// Runs on MSP432
// Test the functions provided in MAX5353.c by outputting
// a sine wave at a particular frequency.
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

#include <stdint.h>
#include "..\inc\msp432p401r.h"
#include "MAX5353.h"

// 12-bit 32-element sine wave
// multiply each value by 2 to shift into bits 12:1 of SSI packet
// three control bits in 15:13 are all zero for immediate DAC update
// book figure shows MAX5353 in unipolar rail-to-rail configuration
// that means when wave[n] = 0x0000 (LSB = 0), output = 0
//                 wave[n] = 0x1000 (LSB = 0), output = Vref
//                 wave[n] = 0x1FFE (LSB = 0), output = 2*Vref
const unsigned short wave[32] = {
  2048*2,2448*2,2832*2,3186*2,3496*2,3751*2,3940*2,4057*2,4095*2,4057*2,3940*2,
  3751*2,3496*2,3186*2,2832*2,2448*2,2048*2,1648*2,1264*2,910*2,600*2,345*2,
  156*2,39*2,0*2,39*2,156*2,345*2,600*2,910*2,1264*2,1648*2};

int main(void){
  int i = 0;
  DAC_Init(0x1000);        // initialize with command: Vout = Vref
  while(1){
    DAC_Out(wave[i&0x1F]);
    i = i + 1;
// hardware and software delays yield a sin wave about 668 Hz
  }
}
