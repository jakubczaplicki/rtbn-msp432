// 74HC165.c
// Runs on MSP432
// Use eUSCI3 to receive an 8-bit code from the 74HC165.
// Input port expander
// Daniel Valvano
// October 12, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Program 7.3

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

// MSP432        74HC165
//   +3.3        pin 16 Vcc powewr
//   Gnd         pin 8  ground
//   Gnd         pin 15 CLK INH (allow clock to operate)
//   Gnd         pin 10 SER (data in is NA)
// P9.5 UCA3CLK  pin 2  CLK
// P9.4 UCA3STE  pin 1  SH/LD (0 for load, 1 for shift)
// P9.6 UCA3SOMI pin 9  Qh (data shifted out)
//     nc        pin 7  Qh' is not needed

// Port          74HC165
// bit 7 (MSB)   pin 6  H
// bit 6         pin 5  G
// bit 5         pin 4  F
// bit 4         pin 3  E
// bit 3         pin 14 D
// bit 2         pin 13 C
// bit 1         pin 12 B
// bit 0 (LSB)   pin 11 A

#include <stdint.h>
#include "..\inc\msp432p401r.h"


//********Port_Init*****************
// Initialize 74HC165 serial shift register
// inputs:  none
// outputs: none
// 74HC165 clocks out on rise, MSP432 clocks in on fall
void Port_Init(void){
  // initialize eUSCI
  UCA3CTLW0 = 0x0001;                   // hold the eUSCI module in reset mode
  // configure UCA3CTLW0 for:
  // bit15      UCCKPH = 1; data shifts in on first edge, out on following edge
  // bit14      UCCKPL = 1; clock is high when inactive
  // bit13      UCMSB = 1; MSB first
  // bit12      UC7BIT = 0; 8-bit data
  // bit11      UCMST = 1; master mode
  // bits10-9   UCMODEx = 1; UCSTE active high
  // bit8       UCSYNC = 1; synchronous mode
  // bits7-6    UCSSELx = 2; eUSCI clock SMCLK
  // bits5-2    reserved
  // bit1       UCSTEM = 1; UCSTE pin enables slave
  // bit0       UCSWRST = 1; reset enabled
  UCA3CTLW0 = 0xEB83;
  // set the baud rate for the eUSCI which gets its clock from SMCLK
  // Clock_Init48MHz() from ClockSystem.c sets SMCLK = HFXTCLK/4 = 12 MHz
  // if the SMCLK is set to 12 MHz, divide by 3 for 4 MHz baud clock
  UCA3BRW = 3;
  // modulation is not used in SPI mode, so clear UCA3MCTLW
  UCA3MCTLW = 0;
  P9SEL0 |= 0x70;
  P9SEL1 &= ~0x70;                      // configure P9.6, P9.5, and P9.4 as primary module function
  UCA3CTLW0 &= ~0x0001;                 // enable eUSCI module
  UCA3IE &= ~0x0003;                    // disable interrupts
}

//********Port_In*****************
// Receive data from 74HC165 8-bit port
// inputs:  none
// outputs: data (0 to 255)
uint8_t Port_In(void){
  while((UCA3IFG&0x0002)==0x0000){};    // wait until UCA3TXBUF empty
  UCA3TXBUF = 0;                        // data out to start
  while((UCA3IFG&0x0001)==0x0000){};    // wait until UCA3RXBUF full
  return UCA3RXBUF;                     // data in
}

