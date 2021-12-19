// *****************Texas.c**************
// Open version of TExaS
//
// Runs on MSP432
// Daniel and Jonathan Valvano
// May 24, 2016

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2016

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

#include <stdint.h>
#include "TExaS.h"
#include "../inc/CortexM.h"
#include "../inc/msp432p401r.h"

char volatile LogicData;

void LogicAnalyzer(void){ // called 10k/sec
  UCA0TXBUF = LogicData;  // send data to PC
}
void Scope(void){  // called 10k/sec
 // sample ADC **to do**
 // send 8-bit ADC data to TExaSdisplay via UART
}


// ------------PeriodicTask2_Init------------
// Activate an interrupt to run a user task periodically.
// Give it a priority 0 to 6 with lower numbers
// signifying higher priority.  Equal priority is
// handled sequentially.
// Input:  task is a pointer to a user function
//         freq is number of interrupts per second
//           1 Hz to 10 kHz
//         priority is a number 0 to 6
// Output: none
void (*PeriodicTask2)(void);   // user function
void PeriodicTask2_Init(void(*task)(void), uint32_t freq, uint8_t priority){long sr;
  if((freq == 0) || (freq > 10000)){
    return;                        // invalid input
  }
  if(priority > 6){
    priority = 6;
  }
  sr = StartCritical();
  PeriodicTask2 = task;            // user function
  // ***************** TimerA3 initialization *****************
  TA3CTL &= ~0x0030;               // halt Timer A3
  // bits15-10=XXXXXX, reserved
  // bits9-8=01,       clock source to ACLK (32,768 Hz REFOCLK after BSP_Clock_InitFastest() called)
  // bits7-6=00,       input clock divider /1
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=0,           no interrupt on timer
  // bit0=0,           clear interrupt pending
  TA3CTL = 0x0100;
  TA3EX0 &= ~0x0007;               // configure for input clock divider /1
  // bits15-14=00,     no capture mode
  // bits13-12=XX,     capture/compare input select
  // bit11=X,          synchronize capture source
  // bit10=X,          synchronized capture/compare input
  // bit9=X,           reserved
  // bit8=0,           compare mode
  // bits7-5=XXX,      output mode
  // bit4=1,           enable capture/compare interrupt on CCIFG
  // bit3=X,           read capture/compare input from here
  // bit2=0,           output this value in output mode 0
  // bit1=X,           capture overflow status
  // bit0=0,           clear capture/compare interrupt pending
  TA3CCTL0 = 0x0010;
  TA3CCR0 = (32768/freq - 1);      // compare match value
// interrupts enabled in the main program after all devices initialized
  NVIC_IPR3 = (NVIC_IPR3&0xFF00FFFF)|(priority<<21); // priority
  NVIC_ISER0 = 0x00004000;         // enable interrupt 14 in NVIC
  TA3CTL |= 0x0014;                // reset and start Timer A3 in up mode
  EndCritical(sr);
}

void TA3_0_IRQHandler(void){
  TA3CCTL0 &= ~0x0001;             // acknowledge capture/compare interrupt 0
  (*PeriodicTask2)();              // execute user task
}

// ------------PeriodicTask2_Stop------------
// Deactivate the interrupt running a user task
// periodically.
// Input: none
// Output: none
void PeriodicTask2_Stop(void){
  TA3CCTL0 &= ~0x0001;             // clear capture/compare interrupt pending
  NVIC_ICER0 = 0x00004000;         // disable interrupt 14 in NVIC
}


//------------UART_Init------------
// Initialize the UART for 115,200 baud rate
// SMCLK = 12 MHz
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void UART_Init(void){
  if(((P1SEL0&0x0C)==0x0C)&&(UCA0BRW==104))return; // already on
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
                                        // N = clock/baud rate = 12,000,000/115,200 = 104.1666667
  UCA0BRW = 104;                        // UCBR = baud rate = int(N) = 104
  UCA0MCTLW &= ~0xFFF1;                 // clear first and second modulation stage bit fields
                                        // configure second modulation stage select (from Table 22-4 on p731 of datasheet)
//  UCA0MCTLW |= (0<<8);                  // UCBRS = N - int(N) = 0.0417; plug this in Table 22-4
                                        // configure first modulation stage select (ignored when oversampling disabled)
//  UCA0MCTLW |= (10<<4);                 // UCBRF = int(((N/16) - int(N/16))*16) = 10
//  UCA0MCTLW |= 0x0001;                  // enable oversampling mode
  P1SEL0 |= 0x0C;
  P1SEL1 &= ~0x0C;                      // configure P1.3 and P1.2 as primary module function
  UCA0CTLW0 &= ~0x0001;                 // enable the USCI module
  UCA0IE &= ~0x000F;                    // disable interrupts (transmit ready, start received, transmit empty, receive full)
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
char UART_InChar(void){
  while((UCA0IFG&0x01) == 0);
  return((char)(UCA0RXBUF));
}
//------------UART_InCharNonBlocking------------
// look for new serial port input
// Input: none
// Output: ASCII code for key typed
//         0 if no key ready
char UART_InCharNonBlocking(void){
  if((UCA0IFG&0x01) == 0){
    return((char)(UCA0RXBUF));
  }
  return 0;
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data){
  while((UCA0IFG&0x02) == 0);
  UCA0TXBUF = data;
}


// toggle bit 0 
void TExaS_Task0(void){
  LogicData ^= 0x01;
}
// toggle bit 1 
void TExaS_Task1(void){
  LogicData ^= 0x02;
}
// toggle bit 2 
void TExaS_Task2(void){
  LogicData ^= 0x04;
}
// toggle bit 3 
void TExaS_Task3(void){ 
  LogicData ^= 0x08;
}
// toggle bit 4 
void TExaS_Task4(void){
}
// toggle bit 5 
void TExaS_Task5(void){
  LogicData ^= 0x20;
}
// toggle bit 6 
void TExaS_Task6(void){
  LogicData ^= 0x40;
}
// set bit 0 
void TExaS_Set0(void){
  LogicData |= 0x01;
}
// set bit 1 
void TExaS_Set1(void){
  LogicData |= 0x02;
}
// set bit 2 
void TExaS_Set2(void){
  LogicData |= 0x04;
}
// set bit 3 
void TExaS_Set3(void){ 
  LogicData |= 0x08;
}
// set bit 4 
void TExaS_Set4(void){
}
// set bit 5 
void TExaS_Set5(void){
  LogicData |= 0x20;
}
// set bit 6 
void TExaS_Set6(void){
  LogicData |= 0x40;
}
// clear bit 0 
void TExaS_Clear0(void){
  LogicData |= 0x01;
}
// clear bit 1 
void TExaS_Clear1(void){
  LogicData |= 0x02;
}
// clear bit 2 
void TExaS_Clear2(void){
  LogicData |= 0x04;
}
// clear bit 3 
void TExaS_Clear3(void){ 
  LogicData |= 0x08;
}
// clear bit 4 
void TExaS_Clear4(void){
}
// clear bit 5 
void TExaS_Clear5(void){
  LogicData |= 0x20;
}
// clear bit 6 
void TExaS_Clear6(void){
  LogicData |= 0x40;
}


// ************TExaS_Init*****************
// Initialize grader, triggered by periodic timer
// This needs to be called once
// Inputs: Scope or Logic analyzer
//         Bus clock frequency in Hz
// Outputs: none
void TExaS_Init(enum TExaSmode mode,uint32_t busfrequency){
  // 10 kHz periodic interrupt
  // logic analyzer will 10 kHz output to serial port : 8 bit bit7 set 

  UART_Init();
  LogicData |= 0x80; // bit 7 means logic data
  if(mode == LOGICANALYZER){
  // enable 10k periodic interrupt if logic analyzer mode
    PeriodicTask2_Init(&LogicAnalyzer,10000,5); // run grader
  } else if(mode == SCOPE){
   // activate as analog input
    PeriodicTask2_Init(&Scope,10000,5); // run scope at 10k
  }
}


// ************TExaS_Stop*****************
// Stop the transfer
// Inputs:  none
// Outputs: none
void TExaS_Stop(void){
  PeriodicTask2_Stop();
}
