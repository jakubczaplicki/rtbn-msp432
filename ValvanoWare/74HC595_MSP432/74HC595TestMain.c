// 74HC595TestMain.c
// Runs on MSP432
// Use eUSCI3 to send an 8-bit code to the 74HC595.
// Use the output port expander to interface with a
// parallel LCD using fewer pins (3 or 5 instead of
// 10).  This file is adapted from LCD_MSP432.zip.
// Daniel Valvano
// July 21, 2015

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

// 8-bit solution, LCD using 5 pins instead of 10.
/*
  size is 1*16
  tie R/W=ground because shift register is output only
  ground   = LCD pin 1    Vss
  power    = LCD pin 2    Vdd   +3.3V or +5V depending on the device
  ground   = LCD pin 3    Vlc   grounded for highest contrast
  P9.2     = LCD pin 4    RS    (1 for data, 0 for control/status)
  ground   = LCD pin 5    R/W   (1 for read, 0 for write)
  P9.3     = LCD pin 6    E     (enable)
  Qa 15    = LCD pin 7    DB0   (8-bit data)
  Qb 1     = LCD pin 8    DB1
  Qc 2     = LCD pin 9    DB2
  Qd 3     = LCD pin 10   DB3
  Qe 4     = LCD pin 11   DB4
  Qf 5     = LCD pin 12   DB5
  Qg 6     = LCD pin 13   DB6
  Qh 7     = LCD pin 14   DB7
16 characters are configured as 1 row of 16
addr  00 01 02 03 04 05 ... 0F
*/

#include <stdint.h>
#include "74HC595.h"
#include "ClockSystem.h"  // only used here in the main program
#include "SysTick.h"      // only used here in the main program
#include "..\inc\msp432p401r.h"

//start of modified LCD.c
#define LCD_RS  (*((volatile uint8_t *)0x42099048))  /* Port 9.2 Output */
#define LCD_E   (*((volatile uint8_t *)0x4209904C))  /* Port 9.3 Output */
#define BusFreq 48            // assuming a 48 MHz bus clock
#define T6us 6*BusFreq        // 6us
#define T40us 40*BusFreq      // 40us
#define T160us 160*BusFreq    // 160us
#define T1600us 1600*BusFreq  // 1.60ms
#define T5ms 5000*BusFreq     // 5ms
#define T15ms 15000*BusFreq   // 15ms
void static OutCmd(unsigned char command){
  Port_Out(command);
  LCD_E = 0;
  LCD_RS = 0;                    // E=0, R/W=0, RS=0
  SysTick_Wait(T6us);            // wait 6us
  LCD_E = 1;                     // E=1, R/W=0, RS=0
  SysTick_Wait(T6us);            // wait 6us
  LCD_E = 0;                     // E=0, R/W=0, RS=0
  SysTick_Wait(T40us);           // wait 40us
}

// Initialize LCD
// Inputs: none
// Outputs: none
void LCD_Init(void){
  P9SEL0 &= ~0x0C;
  P9SEL1 &= ~0x0C;               // configure P9.3 and P9.2 as GPIO
  P9DIR |= 0x0C;                 // make P9.3 and P9.2 out
  Clock_Init48MHz();             // set system clock to 48 MHz
  SysTick_Init();                // Volume 1 Program 4.7, Volume 2 Program 2.10
  Port_Init();                   // initialize 74HC595 interface using eUSCI3
  LCD_E = 0;
  LCD_RS = 0;                    // E=0, R/W=0, RS=0
  SysTick_Wait(T15ms);           // Wait >15 ms after power is applied
  OutCmd(0x30);                  // command 0x30 = Wake up
  SysTick_Wait(T5ms);            // must wait 5ms, busy flag not available
  OutCmd(0x30);                  // command 0x30 = Wake up #2
  SysTick_Wait(T160us);          // must wait 160us, busy flag not available
  OutCmd(0x30);                  // command 0x30 = Wake up #3
  SysTick_Wait(T160us);          // must wait 160us, busy flag not available
  OutCmd(0x38);                  // Function set: 8-bit/2-line
  OutCmd(0x10);                  // Set cursor
  OutCmd(0x0C);                  // Display ON; Cursor ON
  OutCmd(0x06);                  // Entry mode set
}

// Output a character to the LCD
// Inputs: letter is ASCII character, 0 to 0x7F
// Outputs: none
void LCD_OutChar(char letter){
  Port_Out(letter);
  LCD_E = 0;
  LCD_RS = 1;                    // E=0, R/W=0, RS=1
  SysTick_Wait(T6us);            // wait 6us
  LCD_E = 1;                     // E=1, R/W=0, RS=1
  SysTick_Wait(T6us);            // wait 6us
  LCD_E = 0;                     // E=0, R/W=0, RS=1
  SysTick_Wait(T40us);           // wait 40us
}

// Clear the LCD
// Inputs: none
// Outputs: none
void LCD_Clear(void){
  OutCmd(0x01);          // Clear Display
  SysTick_Wait(T1600us); // wait 1.6ms
  OutCmd(0x02);          // Cursor to home
  SysTick_Wait(T1600us); // wait 1.6ms
}

//------------LCD_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void LCD_OutString(char *pt){
  while(*pt){
    LCD_OutChar(*pt);
    pt++;
  }
}

//-----------------------LCD_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void LCD_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    LCD_OutUDec(n/10);
    n = n%10;
  }
  LCD_OutChar(n+'0'); /* n is between 0 and 9 */
}

//--------------------------LCD_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void LCD_OutUHex(uint32_t number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    LCD_OutUHex(number/0x10);
    LCD_OutUHex(number%0x10);
  }
  else{
    if(number < 0xA){
      LCD_OutChar(number+'0');
     }
    else{
      LCD_OutChar((number-0x0A)+'A');
    }
  }
}
//end of modified LCD.c

int main1(void){
  uint32_t n;

  LCD_Init();              // set system clock to 48 MHz, initialize LCD, initialize SysTick, and initialize 74HC595 interface
  n = 0;
  LCD_Clear();
  LCD_OutString("Test LCD");
  SysTick_Wait10ms(100);
  while(1){
    LCD_Clear();
    LCD_OutUDec(n);
    SysTick_Wait10ms(50);
    LCD_OutChar(32);
    LCD_OutUHex(n);
    SysTick_Wait10ms(100);
    n++;
  }
}

// 4-bit solution, LCD using 3 pins instead of 10.
/*
  size is 1*16
  tie R/W=ground because shift register is output only
      ground   = LCD pin 1    Vss
      power    = LCD pin 2    Vdd   +3.3V or +5V depending on the device
      ground   = LCD pin 3    Vlc   grounded for highest contrast
  595 Qf 5     = LCD pin 4    RS    (1 for data, 0 for control/status)
      ground   = LCD pin 5    R/W   (1 for read, 0 for write)
  595 Qg 6     = LCD pin 6    E     (enable)
  595 Qa 15    = LCD pin 11   DB4   (4-bit data)
  595 Qb 1     = LCD pin 12   DB5
  595 Qc 2     = LCD pin 13   DB6
  595 Qd 3     = LCD pin 14   DB7
  595 Qe 4     = nc (if friendly, could be used)
  595 Qh 7     = nc (if friendly, could be used)
      nc       = LCD pin 7    DB0
      nc       = LCD pin 8    DB1
      nc       = LCD pin 9    DB2
      nc       = LCD pin 10   DB3
16 characters are configured as 1 row of 16
addr  00 01 02 03 04 05 ... 0F
*/
#define E 0x40
#define RS 0x20
//---------------------outCsrNibble2---------------------
// sends one command code to the LCD control/status
// Input: command is 4-bit function to execute
// Output: none
static void outCsrNibble2(uint8_t command){
  Port_Out(command&0x0F);      // nibble, E=0, RS=0
  Port_Out(E|(command&0x0F));  // E goes 0,1
  SysTick_Wait(T6us);          // wait 6us
  Port_Out(command&0x0F);      // E goes 1,0
  SysTick_Wait(T6us);          // wait 6us
}

//---------------------OutCmd2---------------------
// sends one command code to the LCD control/status
// Input: command is 8-bit function to execute
// Output: none
static void OutCmd2(unsigned char command){
  outCsrNibble2(command>>4);   // ms nibble, E=0, RS=0
  outCsrNibble2(command);      // ls nibble, E=0, RS=0
  SysTick_Wait(T40us);         // wait 40us
}

//---------------------LCD_Clear2---------------------
// clear the LCD display, send cursor to home
// Input: none
// Output: none
void LCD_Clear2(void){
  OutCmd2(0x01);         // Clear Display
  SysTick_Wait(T1600us); // wait 1.6ms
  OutCmd2(0x02);         // Cursor to home
  SysTick_Wait(T1600us); // wait 1.6ms
}
#define LCDINC 2
#define LCDDEC 0
#define LCDSHIFT 1
#define LCDNOSHIFT 0
#define LCDCURSOR 2
#define LCDNOCURSOR 0
#define LCDBLINK 1
#define LCDNOBLINK 0
#define LCDSCROLL 8
#define LCDNOSCROLL 0
#define LCDLEFT 0
#define LCDRIGHT 4
#define LCD2LINE 8
#define LCD1LINE 0
#define LCD10DOT 4
#define LCD7DOT 0

//---------------------LCD_Init2---------------------
// initialize the LCD display, called once at beginning
// Input: none
// Output: none
void LCD_Init2(void){
  Clock_Init48MHz();      // set system clock to 48 MHz
  SysTick_Init();         // Volume 1 Program 4.7, Volume 2 Program 2.10
  Port_Init();            // initialize 74HC595 interface using eUSCI3
  Port_Out(0);            // E=0
  SysTick_Wait(T15ms);    // Wait >15 ms after power is applied
  outCsrNibble2(0x03);    // (DL=1 8-bit mode)
  SysTick_Wait(T5ms);     // must wait 5ms, busy flag not available
  outCsrNibble2(0x03);    // (DL=1 8-bit mode)
  SysTick_Wait(T160us);   // must wait 160us, busy flag not available
  outCsrNibble2(0x03);    // (DL=1 8-bit mode)
  SysTick_Wait(T160us);   // must wait 160us, busy flag not available
  outCsrNibble2(0x02);    // (DL=0 4-bit mode)
  SysTick_Wait(T160us);   // must wait 160us, busy flag not available
/* Entry Mode Set 0,0,0,0,0,1,I/D,S
     I/D=1 for increment cursor move direction
        =0 for decrement cursor move direction
     S  =1 for display shift
        =0 for no display shift   */
  OutCmd2(0x04+LCDINC+LCDNOSHIFT);        // I/D=1 Increment, S=0 no displayshift
/* Display On/Off Control 0,0,0,0,1,D,C,B
     D  =1 for display on
        =0 for display off
     C  =1 for cursor on
        =0 for cursor off
     B  =1 for blink of cursor position character
        =0 for no blink   */
  OutCmd2(0x0C+LCDNOCURSOR+LCDNOBLINK);   // D=1 displayon, C=0 cursoroff, B=0 blink off
/* Cursor/Display Shift  0,0,0,1,S/C,R/L,*,*
     S/C=1 for display shift
        =0 for cursor movement
     R/L=1 for shift to left
        =0 for shift to right   */
  OutCmd2(0x10+LCDNOSCROLL+LCDRIGHT);   // S/C=0 cursormove, R/L=1 shiftright
/* Function Set   0,0,1,DL,N,F,*,*
     DL=1 for 8 bit
       =0 for 4 bit
     N =1 for 2 lines
       =0 for 1 line
     F =1 for 5 by 10 dots
       =0 for 5 by 7 dots */
  OutCmd2(0x20+LCD2LINE+LCD7DOT);   // DL=0 4bit, N=1 2 line, F=0 5by7 dots
}

//---------------------LCD_OutChar2---------------------
// sends one ASCII to the LCD display
// Input: letter is ASCII code
// Output: true if successful
void LCD_OutChar2(unsigned char letter){
  Port_Out(RS|(0x0F&(letter>>4)));     // ms nibble, E=0, RS=1
  Port_Out(E|RS|(0x0F&(letter>>4)));   // E goes 0 to 1
  SysTick_Wait(T6us);                  // wait 6us
  Port_Out(RS|(0x0F&(letter>>4)));     // E goes 1 to 0
  SysTick_Wait(T6us);                  // wait 6us
  Port_Out(RS|(0x0F&letter));          // ls nibble, E=0, RS=1
  Port_Out(E|RS|(0x0F&letter));        // E goes 0 to 1
  SysTick_Wait(T6us);                  // wait 6us
  Port_Out(RS|(0x0F&letter));          // E goes 1 to 0
  SysTick_Wait(T40us);                 // wait 40us
}

//------------LCD_OutString2------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void LCD_OutString2(char *pt){
  while(*pt){
    LCD_OutChar2(*pt);
    pt++;
  }
}

//-----------------------LCD_OutUDec2-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void LCD_OutUDec2(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    LCD_OutUDec2(n/10);
    n = n%10;
  }
  LCD_OutChar2(n+'0'); /* n is between 0 and 9 */
}

//--------------------------LCD_OutUHex2----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void LCD_OutUHex2(uint32_t number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    LCD_OutUHex2(number/0x10);
    LCD_OutUHex2(number%0x10);
  }
  else{
    if(number < 0xA){
      LCD_OutChar2(number+'0');
     }
    else{
      LCD_OutChar2((number-0x0A)+'A');
    }
  }
}

//-----------------------LCD_GoTo-----------------------
// Move cursor
// Input: line number is 1 to 4, column from 1 to 20
// Output: none
// errors: it will check for legal address
void LCD_GoTo2(int line, int column){
  unsigned char DDaddr;
  if((line<1) || (line>4)) return;
  if((column<1) || (column>20)) return;
  if(line==1) DDaddr = column-1;
  if(line==2) DDaddr = 0x3F+column;
  if(line==3) DDaddr = 0x13+column;
  if(line==4) DDaddr = 0x53+column;
  DDaddr += 0x80;
  OutCmd2(DDaddr);
}

int main(void){
  uint32_t n;

  LCD_Init2();  // set system clock to 48 MHz, initialize LCD, initialize SysTick, and initialize 74HC595 interface
  n = 0;
  LCD_Clear2();
  LCD_OutString2("Test LCD");
  SysTick_Wait10ms(100);
  while(1){
    LCD_GoTo2(2, 1);
    LCD_OutUDec2(n);
    SysTick_Wait10ms(10);
    LCD_OutString2(",  0x");
    LCD_OutUHex2(n);
    LCD_OutString2("      ");
    SysTick_Wait10ms(100);
    n++;
  }
}
