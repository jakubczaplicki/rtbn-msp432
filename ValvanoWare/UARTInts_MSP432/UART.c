// UART.c
// Runs on MSP432
// Use UCA0 to implement bidirectional data transfer to and from a
// computer running PuTTy, uses interrupts and FIFOs.

// Daniel Valvano
// October 12, 2015
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

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
#include "fifo.h"
#include "..\inc\msp432p401r.h"
#define FIFOSIZE   16         // size of the FIFOs (must be power of 2)
#define FIFOSUCCESS 1         // return value on success
#define FIFOFAIL    0         // return value on failure
                              // create index implementation FIFO (see FIFO.h)
AddIndexFifo(Rx, FIFOSIZE, char, FIFOSUCCESS, FIFOFAIL)
AddIndexFifo(Tx, FIFOSIZE, char, FIFOSUCCESS, FIFOFAIL)

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 3 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void UART_Init(void){
  RxFifo_Init();              // initialize FIFOs
  TxFifo_Init();
  UCA0CTLW0 = 0x0001;         // hold the USCI module in reset mode
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
  UCA0BRW = 26;               // UCBR = baud rate = int(N) = 26
  UCA0MCTLW = 0x0000;       // clear first and second modulation stage bit fields
// configure second modulation stage select (from Table 22-4 on p731 of datasheet)
//  UCA0MCTLW |= (0<<8);        // UCBRS = N - int(N) = 0.0417; plug this in Table 22-4
// configure first modulation stage select (ignored when oversampling disabled)
//  UCA0MCTLW |= (10<<4);       // UCBRF = int(((N/16) - int(N/16))*16) = 10
//  UCA0MCTLW |= 0x0001;        // enable oversampling mode
// since TxFifo is empty, we initially disarm interrupts on UCTXIFG, but arm it on OutChar
  P1SEL0 |= 0x0C;
  P1SEL1 &= ~0x0C;            // configure P1.3 and P1.2 as primary module function
  NVIC_IPR4 = (NVIC_IPR4&0xFFFFFF00)|0x00000040; // priority 2
  NVIC_ISER0 = 0x00010000;    // enable interrupt 16 in NVIC
  UCA0CTLW0 &= ~0x0001;       // enable the USCI module
                              // enable interrupts on receive full
  UCA0IE = 0x0001;            // disable interrupts on transmit empty, start, complete
}


//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
// spin if RxFifo is empty
char UART_InChar(void){
  char letter;
  while(RxFifo_Get(&letter) == FIFOFAIL){};
  return(letter);
}

//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
// spin if TxFifo is full
void UART_OutChar(char data){
  while(TxFifo_Put(data) == FIFOFAIL){}; // spin if full
  UCA0IE = 0x0003;     // enable interrupts on transmit empty and receive full
}

// interrupt 16 occurs on either:
// UCTXIFG TX data register is empty
// UCRXIFG RX data register is full
// vector at 0x00000080 in startup_msp432.s
void EUSCIA0_IRQHandler(void){ char data; 
  if(UCA0IFG&0x02){             // TX data register empty
    if(TxFifo_Get(&data) == FIFOFAIL){
      UCA0IE = 0x0001;         // disable interrupts on transmit empty
    }else{
      UCA0TXBUF = data;        // send data, acknowledge interrupt
    }
  }
  if(UCA0IFG&0x01){             // RX data register full
    RxFifo_Put((char)UCA0RXBUF);// clears UCRXIFG
  } 
}

//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART_OutString(char *pt){
  while(*pt){
    UART_OutChar(*pt);
    pt++;
  }
}

//------------UART_InUDec------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 32-bit unsigned number
//     valid range is 0 to 4294967295 (2^32-1)
// Input: none
// Output: 32-bit unsigned number
// If you enter a number above 4294967295, it will return an incorrect value
// Backspace will remove last digit typed
uint32_t UART_InUDec(void){
uint32_t number=0, length=0;
char character;
  character = UART_InChar();
  while(character != CR){ // accepts until <enter> is typed
// The next line checks that the input is a digit, 0-9.
// If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967295
      length++;
      UART_OutChar(character);
    }
// If the input is a backspace, then the return number is
// changed and a backspace is outputted to the screen
    else if((character==BS) && length){
      number /= 10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

//-----------------------UART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void UART_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    UART_OutUDec(n/10);
    n = n%10;
  }
  UART_OutChar(n+'0'); /* n is between 0 and 9 */
}

//---------------------UART_InUHex----------------------------------------
// Accepts ASCII input in unsigned hexadecimal (base 16) format
// Input: none
// Output: 32-bit unsigned number
// No '$' or '0x' need be entered, just the 1 to 8 hex digits
// It will convert lower case a-f to uppercase A-F
//     and converts to a 16 bit unsigned number
//     value range is 0 to FFFFFFFF
// If you enter a number above FFFFFFFF, it will return an incorrect value
// Backspace will remove last digit typed
uint32_t UART_InUHex(void){
uint32_t number=0, digit, length=0;
char character;
  character = UART_InChar();
  while(character != CR){
    digit = 0x10; // assume bad
    if((character>='0') && (character<='9')){
      digit = character-'0';
    }
    else if((character>='A') && (character<='F')){
      digit = (character-'A')+0xA;
    }
    else if((character>='a') && (character<='f')){
      digit = (character-'a')+0xA;
    }
// If the character is not 0-9 or A-F, it is ignored and not echoed
    if(digit <= 0xF){
      number = number*0x10+digit;
      length++;
      UART_OutChar(character);
    }
// Backspace outputted and return value changed if a backspace is inputted
    else if((character==BS) && length){
      number /= 0x10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

//--------------------------UART_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void UART_OutUHex(uint32_t number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    UART_OutUHex(number/0x10);
    UART_OutUHex(number%0x10);
  }
  else{
    if(number < 0xA){
      UART_OutChar(number+'0');
     }
    else{
      UART_OutChar((number-0x0A)+'A');
    }
  }
}

//------------UART_InString------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until <enter> is typed
//    or until max length of the string is reached.
// It echoes each character as it is inputted.
// If a backspace is inputted, the string is modified
//    and the backspace is echoed
// terminates the string with a null character
// uses busy-waiting synchronization on RDRF
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void UART_InString(char *bufPt, uint16_t max) {
int length=0;
char character;
  character = UART_InChar();
  while(character != CR){
    if(character == BS){
      if(length){
        bufPt--;
        length--;
        UART_OutChar(BS);
      }
    }
    else if(length < max){
      *bufPt = character;
      bufPt++;
      length++;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  *bufPt = 0;
}
