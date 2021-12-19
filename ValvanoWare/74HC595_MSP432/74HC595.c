// 74HC595.c
// Runs on MSP432
// Use eUSCI3 to send an 8-bit code to the 74HC595.
// Output port expander
// Daniel Valvano
// October 12, 2015

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
#include <stdint.h>
#include "..\inc\msp432p401r.h"

//********Port_Init*****************
// Initialize 74HC595 serial shift register
// inputs:  none
// outputs: none
void Port_Init(void){
  // initialize eUSCI
  UCA3CTLW0 = 0x0001;                   // hold the eUSCI module in reset mode
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
  // if the SMCLK is set to 12 MHz, divide by 3 for 4 MHz baud clock
  UCA3BRW = 3;
  // modulation is not used in SPI mode, so clear UCA3MCTLW
  UCA3MCTLW = 0;
  P9SEL0 |= 0xB0;
  P9SEL1 &= ~0xB0;                      // configure P9.7, P9.5, and P9.4 as primary module function
  UCA3CTLW0 &= ~0x0001;                 // enable eUSCI module
  UCA3IE &= ~0x0003;                    // disable interrupts
}

//********Port_Out*****************
// Send data to 74HC595 8-bit port
// inputs:  output (0 to 255)
// outputs: none
void Port_Out(uint8_t code){
  while((UCA3IFG&0x0002)==0x0000){};    // wait until UCA3TXBUF empty
  UCA3TXBUF = code;                     // data out
}
