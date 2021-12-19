// UARTTestMain.c
// Runs on MSP432
// Used to test the UART.c driver
// Daniel Valvano
// May 19, 2015

/* This example accompanies the book
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

#include <stdint.h>
#include "UART.h"

//---------------------OutCRLF---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void OutCRLF(void){
  UART_OutChar(CR);
  UART_OutChar(LF);
}
//debug code
int main(void){
  char ch;
  char string[20];  // global to assist in debugging
  uint32_t n;

  UART_Init();              // initialize UART

  OutCRLF();
  for(ch='A'; ch<='Z'; ch=ch+1){// print the uppercase alphabet
    UART_OutChar(ch);
  }
  OutCRLF();
  UART_OutChar(' ');
  for(ch='a'; ch<='z'; ch=ch+1){// print the lowercase alphabet
    UART_OutChar(ch);
  }
  OutCRLF();
  UART_OutChar('-');
  UART_OutChar('-');
  UART_OutChar('>');
  while(1){
    UART_OutString("InString: ");
    UART_InString(string,19);
    UART_OutString(" OutString="); UART_OutString(string); OutCRLF();

    UART_OutString("InUDec: ");  n=UART_InUDec();
    UART_OutString(" OutUDec="); UART_OutUDec(n); OutCRLF();

    UART_OutString("InUHex: ");  n=UART_InUHex();
    UART_OutString(" OutUHex="); UART_OutUHex(n); OutCRLF();

  }
}
