// MAX5353.c
// Runs on MSP432
// Use eUSCI3 to send two 8-bit codes to the MAX5353.
// Daniel Valvano
// October 12, 2015

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
// P9OUT at 0x40004C82
#define DAC_CS  (*((volatile uint8_t *)(0x42000000+32*0x4C82+4*4)))  /* Port 9.4 Output */

//********DAC_Init*****************
// Initialize Max5353 12-bit DAC
// inputs: initial 12-bit digital code in bits 12:1 (0,2,4,...8190)
// outputs: none
void DAC_Init(uint16_t data){
  P9SEL0 &= ~0x10;
  P9SEL1 &= ~0x10;       // configure P9.4 as GPIO for software-controlled CS*
  P9DIR |= 0x10;         // make P9.4 out
  DAC_CS = 1;            // active low
  // initialize eUSCI
  UCA3CTLW0 = 0x0001;    // hold the eUSCI module in reset mode
  // configure UCA3CTLW0 for:
  // bit15      UCCKPH = 1; data shifts in on first edge, out on following edge
  // bit14      UCCKPL = 0; clock is low when inactive
  // bit13      UCMSB = 1; MSB first
  // bit12      UC7BIT = 0; 8-bit data
  // bit11      UCMST = 1; master mode
  // bits10-9   UCMODEx = 2; UCSTE active low
  // bit8       UCSYNC = 1; synchronous mode
  // bits7-6    UCSSELx = 2; eUSCI clock SMCLK
  // bits5-2    reserved
  // bit1       UCSTEM = 1; UCSTE pin enables slave
  // bit0       UCSWRST = 1; reset enabled
  UCA3CTLW0 = 0xAD83;
  // set the baud rate for the eUSCI which gets its clock from SMCLK
  // Clock_Init48MHz() from ClockSystem.c sets SMCLK = HFXTCLK/4 = 12 MHz
  // if the SMCLK is set to 3 MHz, divide by 2 yields 1.5 MHz baud clock
  // if the SMCLK is set to 12 MHz, divide by 2 yields 6 MHz baud clock
  // SCLK must be less than or equal to 10 MHz (see MAX5353 data sheet)
  UCA3BRW = 2;
  UCA3MCTLW = 0;         // modulation is not used in SPI mode
  P9SEL0 |= 0xA0;
  P9SEL1 &= ~0xA0;       // configure P9.7 and P9.5 as primary module function
  UCA3CTLW0 &= ~0x0001;  // enable eUSCI module
  UCA3IE &= ~0x0003;     // disable interrupts
  DAC_Out(data);         // send initial voltage output
}

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
void DAC_Out(uint16_t code){
  DAC_CS = 0;            // falling edge on CS
  UCA3TXBUF = code>>8;   // send first byte
  while((UCA3IFG&0x0001) == 0x0000){};  // wait until response
  UCA3TXBUF = code;      // send second byte
  while((UCA3IFG&0x0001) == 0x0000){};  // wait until response
  DAC_CS = 1;            // rising edge on CS
}
