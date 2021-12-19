// PortMapping.c
// Runs on MSP432
// Connect P2.7 to TA0.CCR0 compare output Out0
// Toggle P2.7 using TimerA0

// MCLK = SMCLK = 3MHz DCO; ACLK = 32.768kHz
// TACCR0 generates a square wave of freq ACLK/1024 =32Hz
// Derived from msp432p401_portmap_01.c in MSPware
// Jonathan Valvano
// July 24, 2015

/* This example accompanies the book
   "Embedded Systems: Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Volume 2, Program 9.8

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

#include <stdint.h>
#include "..\inc\msp432p401r.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// Version with Port Mapping, squarewave on P2.7
// Frequency = 32.768kHz/4/TA0CCR0 = 32Hz
int main(void){// 3 MHz DCO
  PMAPKEYID = 0x2D52;  // Enable Write-access to modify port mapping registers
  PMAPCTL = 0x0002;    // Allow reconfiguration during runtime
  // the default after reset connects P2.0 to UCA1STE
  P2MAP67 = (P2MAP67&0x00FF)|(PM_TA0CCR0A<<8); // P2.7 connected to TA0.CCR0 compare output Out0
  PMAPKEYID = 0;           // Disable Write-Access to modify port mapping registers
  P2DIR |= 0x80;           // P2.7 output
  P2SEL0 |= 0x80;          // P2.7 Port Map functions
  P2SEL1 &= ~0x80;         // P2.7 Port Map functions
  TA0CCTL0 = 0x0080;       // CCR1 toggle/set
  TA0CCR0 = 256;           // Squarewave Period/4
  TA0CTL = 0x0130;         // ACLK=32kHz, up-down mode
  while(1){
    WaitForInterrupt();
  }
}
// Version without Port Mapping, squarewave on P7.3
// default connection is TA0.CCR0 is P7.3
// Frequency = 32.768kHz/4/TA0CCR0 = 32Hz
int main2(void){          // 3 MHz DCO
  P7DIR |= 0x08;          // P7.3 output
  P7SEL0 |= 0x08;         // P7.3 CCR0 functions
  P7SEL1 &= ~0x08;        // P7.3 CCR0 functions
// Setup TA0
  TA0CCTL0 = 0x0080;      // CCR0 toggle/set
  TA0CCR0 = 256;          // Squarewave Period/4
  TA0CTL = 0x0130;        // ACLK=32kHz, up-down mode
  while(1){
    WaitForInterrupt();
  }
}
