// UART.c
// Runs on MSP432
// Simple device driver for the UART.
// Daniel Valvano
// April 15, 2015

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015

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

// UCA0RXD (VCP receive) connected to P1.2
// UCA0TXD (VCP transmit) connected to P1.3

#include <stdio.h>
#include <stdint.h>
#include "UART.h"
#include "..\inc\msp432p401r.h"

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 3 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void UART_Init(void){
  UCA0CTLW0 = 0x0001;                   // hold the USCI module in reset mode
  // bit15=0,      no parity bits
  // bit14=x,      not used when parity is disabled
  // bit13=0,      LSB first
  // bit12=0,      8-bit data length
  // bit11=0,      1 stop bit
  // bits10-8=000, asynchronous UART mode
  // bits7-6=11,   clock source to SMCLK
  // bit5=0,       reject erroneous characters and do not set flag
  // bit4=0,       do not set flag for break characters
  // bit3=0,       not dormant
  // bit2=0,       transmit data, not address (not used here)
  // bit1=0,       do not transmit break (not used here)
  // bit0=1,       hold logic in reset state while configuring
  UCA0CTLW0 = 0x00C1;
                                        // set the baud rate
                                        // N = clock/baud rate = 3,000,000/115,200 = 26.0417
  UCA0BRW = 26;                         // UCBR = baud rate = int(N) = 26
  UCA0MCTLW &= ~0xFFF1;                 // clear first and second modulation stage bit fields
                                        // configure second modulation stage select (from Table 22-4 on p731 of datasheet)
//  UCA0MCTLW |= (0<<8);                  // UCBRS = N - int(N) = 0.0417; plug this in Table 22-4
                                        // configure first modulation stage select (ignored when oversampling disabled)
//  UCA0MCTLW |= (10<<4);                 // UCBRF = int(((N/16) - int(N/16))*16) = 10
//  UCA0MCTLW |= 0x0001;                  // enable oversampling mode
  UCA0IE &= ~0x000F;                    // disable interrupts (transmit ready, start received, transmit empty, receive full)
  P1SEL0 |= 0x0C;
  P1SEL1 &= ~0x0C;                      // configure P1.3 and P1.2 as primary module function
  UCA0CTLW0 &= ~0x0001;                 // enable the USCI module
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
char UART_InChar(void){
  while((UCA0IFG&0x01) == 0);
  UCA0IFG &= ~0x01;                     // clear receive flag
  return((char)(UCA0RXBUF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data){
  while((UCA0IFG&0x02) == 0);
  UCA0IFG &= ~0x02;                     // clear transmit flag
  UCA0TXBUF = data;
}


// Get input from UART, echo
int fgetc (FILE *f){
  char ch = UART_InChar();  // receive from keyboard
  UART_OutChar(ch);            // echo
  return ch;
}
// Function called when file error occurs.
int ferror(FILE *f){
  /* Your implementation of ferror */
  return EOF;
}

// Abstraction of general output device
// Volume 2 section 3.4.5


// Clear display
void Output_Clear(void){ // Clears the display
  // not implemented on the UART
}
// Turn off display (low power)
void Output_Off(void){   // Turns off the display
  // not implemented on the UART
}
// Turn on display
void Output_On(void){    // Turns on the display
  // not implemented on the UART
}
// set the color for future output
void Output_Color(uint32_t newColor){ // Set color of future output
  // not implemented on the UART
}
#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code

//-----CCS implementation
#include "file.h"
int uart_open(const char *path, unsigned flags, int llv_fd){
  UART_Init();
  return 0;
}
int uart_close( int dev_fd){
	return 0;
}
int uart_read(int dev_fd, char *buf, unsigned count){char ch;
  ch = UART_InChar();    // receive from keyboard
  ch = *buf;         // return by reference
  UART_OutChar(ch);  // echo
  return 1;
}
int uart_write(int dev_fd, const char *buf, unsigned count){ unsigned int num=count;
	while(num){
		if(*buf == 10){
		   UART_OutChar(13);
		}
		UART_OutChar(*buf);
		buf++;
		num--;
	}
	return count;
}
off_t uart_lseek(int dev_fd, off_t ioffset, int origin){
	return 0;
}
int uart_unlink(const char * path){
	return 0;
}
int uart_rename(const char *old_name, const char *new_name){
	return 0;
}

//------------Output_Init------------
// Initialize the UART for 115,200 baud rate (assuming 3 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void Output_Init(void){int ret_val; FILE *fptr;
  UART_Init();
  ret_val = add_device("uart", _SSA, uart_open, uart_close, uart_read, uart_write, uart_lseek, uart_unlink, uart_rename);
  if(ret_val) return; // error
  fptr = fopen("uart","w");
  if(fptr == 0) return; // error
  freopen("uart:", "w", stdout); // redirect stdout to uart
  setvbuf(stdout, NULL, _IONBF, 0); // turn off buffering for stdout

}
#else
  //Keil uVision Code
// Print a character to UART.
//------------Output_Init------------
// Initialize the UART for 115,200 baud rate (assuming 3 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void Output_Init(void){
  UART_Init();
}
int fputc(int ch, FILE *f){
  if((ch == 10) || (ch == 13) || (ch == 27)){
    UART_OutChar(13);
    UART_OutChar(10);
    return 1;
  }
  UART_OutChar(ch);
  return 1;
}
#endif
