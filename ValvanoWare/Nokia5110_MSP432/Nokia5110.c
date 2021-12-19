// Nokia5110.c
// Runs on MSP432
// Use SSI0 to send an 8-bit code to the Nokia5110 48x84
// pixel LCD to display text, images, or other information.
// Daniel Valvano
// October 12, 2015

// Font table, initialization, and other functions based
// off of Nokia_5110_Example from Spark Fun:
// 7-17-2011
// Spark Fun Electronics 2011
// Nathan Seidle
// http://dlnmh9ip6v2uc.cloudfront.net/datasheets/LCD/Monochrome/Nokia_5110_Example.pde

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

// Blue Nokia 5110
// ---------------
// Signal        (Nokia 5110) LaunchPad pin
// Reset         (RST, pin 1) connected to P9.3
// UCA3STE       (CE,  pin 2) connected to P9.4
// Data/Command  (DC,  pin 3) connected to P9.2
// UCA3SIMO      (Din, pin 4) connected to P9.7
// UCA3CLK       (Clk, pin 5) connected to P9.5
// 3.3V          (Vcc, pin 6) power
// back light    (BL,  pin 7) not connected, consists of 4 3.3 V white LEDs which draw ~80mA total
// Ground        (Gnd, pin 8) ground

// Red SparkFun Nokia 5110 (LCD-10168)
// -----------------------------------
// Signal        (Nokia 5110) LaunchPad pin
// 3.3V          (VCC, pin 1) power
// Ground        (GND, pin 2) ground
// UCA3STE       (SCE, pin 3) connected to P9.4
// Reset         (RST, pin 4) connected to P9.3
// Data/Command  (D/C, pin 5) connected to P9.2
// UCA3SIMO      (DN,  pin 6) connected to P9.7
// UCA3CLK       (SCLK, pin 7) connected to P9.5
// back light    (LED, pin 8) not connected, consists of 4 3.3 V white LEDs which draw ~80mA total
#include <stdio.h>
#include <stdint.h>
#include "Nokia5110.h"

// *************************** Screen dimensions ***************************
#define SCREENW     84
#define SCREENH     48

#define DC                      (*((volatile uint8_t *)0x40004C82))   /* Port 9 Output, bit 2 is DC*/
#define DC_BIT                  0x04
#define RESET                   (*((volatile uint8_t *)0x40004C82))   /* Port 9 Output, bit 3 is RESET*/
#define RESET_BIT               0x08
#define P9DIR                   (*((volatile uint8_t *)0x40004C84))   /* Port 9 Direction */
#define P9SEL0                  (*((volatile uint8_t *)0x40004C8A))   /* Port 9 Select 0 */
#define P9SEL1                  (*((volatile uint8_t *)0x40004C8C))   /* Port 9 Select 1 */
#define UCA3CTLW0               (*((volatile uint16_t *)0x40001C00))  /* eUSCI_Ax Control Word Register 0 */
#define UCA3BRW                 (*((volatile uint16_t *)0x40001C06))  /* eUSCI_Ax Baud Rate Control Word Register */
#define UCA3MCTLW               (*((volatile uint16_t *)0x40001C08))  /* eUSCI_Ax Modulation Control Word Register */
#define UCA3STATW               (*((volatile uint16_t *)0x40001C0A))  /* eUSCI_Ax Status Register */
#define UCA3RXBUF               (*((volatile uint16_t *)0x40001C0C))  /* eUSCI_Ax Receive Buffer Register */
#define UCA3TXBUF               (*((volatile uint16_t *)0x40001C0E))  /* eUSCI_Ax Transmit Buffer Register */
#define UCA3IE                  (*((volatile uint16_t *)0x40001C1A))  /* eUSCI_Ax Interrupt Enable Register */
#define UCA3IFG                 (*((volatile uint16_t *)0x40001C1C))  /* eUSCI_Ax Interrupt Flag Register */

enum typeOfWrite{
  COMMAND,                              // the transmission is an LCD command
  DATA                                  // the transmission is data
};


// This table contains the hex values that represent pixels
// for a font that is 5 pixels wide and 8 pixels high
static const uint8_t ASCII[][5] = {
  {0x00, 0x00, 0x00, 0x00, 0x00} // 20
  ,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
  ,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
  ,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
  ,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
  ,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
  ,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
  ,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
  ,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
  ,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
  ,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
  ,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
  ,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
  ,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
  ,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
  ,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
  ,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
  ,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
  ,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
  ,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
  ,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
  ,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
  ,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
  ,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
  ,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
  ,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
  ,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
  ,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
  ,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
  ,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
  ,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
  ,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
  ,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
  ,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
  ,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
  ,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
  ,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
  ,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
  ,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
  ,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
  ,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
  ,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
  ,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
  ,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
  ,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
  ,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
  ,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
  ,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
  ,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
  ,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
  ,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
  ,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
  ,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
  ,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
  ,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
  ,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
  ,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
  ,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
  ,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
  ,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
  ,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c '\'
  ,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
  ,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
  ,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
  ,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
  ,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
  ,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
  ,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
  ,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
  ,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
  ,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
  ,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
  ,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
  ,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
  ,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j
  ,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
  ,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
  ,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
  ,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
  ,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
  ,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
  ,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
  ,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
  ,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
  ,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
  ,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
  ,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
  ,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
  ,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
  ,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
  ,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
  ,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
  ,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
  ,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
  ,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ~
//  ,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f DEL
  ,{0x1f, 0x24, 0x7c, 0x24, 0x1f} // 7f UT sign
};

// The Data/Command pin must be valid when the eighth bit is
// sent.  The eUSCI module has no hardware input or output
// FIFOs, so this implementation is much simpler than it was
// for the Tiva LaunchPads.
// All operations wait until all data has been sent,
// configure the Data/Command pin, and queue the message.

// This is a helper function that sends an 8-bit message to the LCD.
// Inputs: type     COMMAND or DATA
//         message  8-bit code to transmit
// Outputs: none
// Assumes: UCA3 and Port 9 have already been initialized and enabled
void static lcdwrite(enum typeOfWrite type, uint8_t message){
  volatile uint8_t dummy;
  while((UCA3IFG&0x0002)==0x0000){};    // wait until UCA3TXBUF empty
  if(type == COMMAND){
    DC &= ~DC_BIT;
  } else{
    DC |= DC_BIT;
  }
  UCA3TXBUF = message;                  // message out
  dummy = UCA3RXBUF;                    // response meaningless here
}
void static lcddatawrite(uint8_t data){
  volatile uint8_t dummy;
  while((UCA3IFG&0x0002)==0x0000){};    // wait until UCA3TXBUF empty
  DC |= DC_BIT;
  UCA3TXBUF = data;                     // data out
  dummy = UCA3RXBUF;                    // response meaningless here
}

//********Nokia5110_Init*****************
// Initialize Nokia 5110 48x84 LCD by sending the proper
// commands to the PCD8544 driver.  One feature of the
// MSP432 is that its SSIs can get their baud clock from
// either the auxiliary clock (ACLK = REFOCLK/1 = 32,768 Hz
// see ClockSystem.c) or from the low-speed subsystem
// master clock (SMCLK <= 12 MHz see ClockSystem.c).  The
// SSI can further divide this clock signal by using the
// 16-bit Bit Rate Control prescaler Register, UCAxBRW.
// Inputs: none
// Outputs: none
// Assumes: low-speed subsystem master clock 12 MHz
void Nokia5110_Init(void){
  volatile uint32_t delay;
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
  P9SEL0 &= ~0x0C;
  P9SEL1 &= ~0x0C;                      // configure P9.3 and P9.2 as GPIO (Reset and D/C pins)
  P9DIR |= 0x0C;                        // make P9.3 and P9.2 out (Reset and D/C pins)
  UCA3CTLW0 &= ~0x0001;                 // enable eUSCI module
  UCA3IE &= ~0x0003;                    // disable interrupts

  RESET &= ~RESET_BIT;                  // reset the LCD to a known state, RESET low
  for(delay=0; delay<10; delay=delay+1);// delay minimum 100 ns
  RESET |= RESET_BIT;                   // hold RESET high

  lcdwrite(COMMAND, 0x21);              // chip active; horizontal addressing mode (V = 0); use extended instruction set (H = 1)
                                        // set LCD Vop (contrast), which may require some tweaking:
  lcdwrite(COMMAND, CONTRAST);          // try 0xB1 (for 3.3V red SparkFun), 0xB8 (for 3.3V blue SparkFun), 0xBF if your display is too dark, or 0x80 to 0xFF if experimenting
  lcdwrite(COMMAND, 0x04);              // set temp coefficient
  lcdwrite(COMMAND, 0x14);              // LCD bias mode 1:48: try 0x13 or 0x14

  lcdwrite(COMMAND, 0x20);              // we must send 0x20 before modifying the display control mode
  lcdwrite(COMMAND, 0x0C);              // set display control to normal mode: 0x0D for inverse
}

//********Nokia5110_OutChar*****************
// Print a character to the Nokia 5110 48x84 LCD.  The
// character will be printed at the current cursor position,
// the cursor will automatically be updated, and it will
// wrap to the next row or back to the top if necessary.
// One blank column of pixels will be printed on either side
// of the character for readability.  Since characters are 8
// pixels tall and 5 pixels wide, 12 characters fit per row,
// and there are six rows.
// Inputs: data  character to print
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_OutChar(char data){int i;
  lcddatawrite(0x00);        // blank vertical line padding
  for(i=0; i<5; i=i+1){
    lcddatawrite(ASCII[data - 0x20][i]);
  }
  lcddatawrite(0x00);        // blank vertical line padding
}

//********Nokia5110_OutString*****************
// Print a string of characters to the Nokia 5110 48x84 LCD.
// The string will automatically wrap, so padding spaces may
// be needed to make the output look optimal.
// Inputs: ptr  pointer to NULL-terminated ASCII string
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_OutString(char *ptr){
  while(*ptr){
    Nokia5110_OutChar((unsigned char)*ptr);
    ptr = ptr + 1;
  }
}

//********Nokia5110_OutUDec*****************
// Output a 16-bit number in unsigned decimal format with a
// fixed size of five right-justified digits of output.
// Inputs: n  16-bit unsigned number
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_OutUDec(uint16_t n){
  if(n < 10){
    Nokia5110_OutString("    ");
    Nokia5110_OutChar(n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    Nokia5110_OutString("   ");
    Nokia5110_OutChar(n/10+'0'); /* tens digit */
    Nokia5110_OutChar(n%10+'0'); /* ones digit */
  } else if(n<1000){
    Nokia5110_OutString("  ");
    Nokia5110_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); /* tens digit */
    Nokia5110_OutChar(n%10+'0'); /* ones digit */
  }
  else if(n<10000){
    Nokia5110_OutChar(' ');
    Nokia5110_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    Nokia5110_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); /* tens digit */
    Nokia5110_OutChar(n%10+'0'); /* ones digit */
  }
  else {
    Nokia5110_OutChar(n/10000+'0'); /* ten-thousands digit */
    n = n%10000;
    Nokia5110_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    Nokia5110_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); /* tens digit */
    Nokia5110_OutChar(n%10+'0'); /* ones digit */
  }
}

//********Nokia5110_SetCursor*****************
// Move the cursor to the desired X- and Y-position.  The
// next character will be printed here.  X=0 is the leftmost
// column.  Y=0 is the top row.
// Inputs: newX  new X-position of the cursor (0<=newX<=11)
//         newY  new Y-position of the cursor (0<=newY<=5)
// Outputs: none
void Nokia5110_SetCursor(uint8_t newX, uint8_t newY){
  if((newX > 11) || (newY > 5)){        // bad input
    return;                             // do nothing
  }
  // multiply newX by 7 because each character is 7 columns wide
  lcdwrite(COMMAND, 0x80|(newX*7));     // setting bit 7 updates X-position
  lcdwrite(COMMAND, 0x40|newY);         // setting bit 6 updates Y-position
}

//********Nokia5110_Clear*****************
// Clear the LCD by writing zeros to the entire screen and
// reset the cursor to (0,0) (top left corner of screen).
// Inputs: none
// Outputs: none
void Nokia5110_Clear(void){
  int i;
  for(i=0; i<(MAX_X*MAX_Y/8); i=i+1){
    lcddatawrite(0x00);
  }
  Nokia5110_SetCursor(0, 0);
}

//********Nokia5110_DrawFullImage*****************
// Fill the whole screen by drawing a 48x84 bitmap image.
// Inputs: ptr  pointer to 504 byte bitmap
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_DrawFullImage(const uint8_t *ptr){
  int i;
  Nokia5110_SetCursor(0, 0);
  for(i=0; i<(MAX_X*MAX_Y/8); i=i+1){
    lcddatawrite(ptr[i]);
  }
}
uint8_t Screen[SCREENW*SCREENH/8]; // buffer stores the next image to be printed on the screen

//********Nokia5110_PrintBMP*****************
// Bitmaps defined above were created for the LM3S1968 or
// LM3S8962's 4-bit grayscale OLED display.  They also
// still contain their header data and may contain padding
// to preserve 4-byte alignment.  This function takes a
// bitmap in the previously described format and puts its
// image data in the proper location in the buffer so the
// image will appear on the screen after the next call to
//   Nokia5110_DisplayBuffer();
// The interface and operation of this process is modeled
// after RIT128x96x4_BMP(x, y, image);
// Inputs: xpos      horizontal position of bottom left corner of image, columns from the left edge
//                     must be less than 84
//                     0 is on the left; 82 is near the right
//         ypos      vertical position of bottom left corner of image, rows from the top edge
//                     must be less than 48
//                     2 is near the top; 47 is at the bottom
//         ptr       pointer to a 16 color BMP image
//         threshold grayscale colors above this number make corresponding pixel 'on'
//                     0 to 14
//                     0 is fine for ships, explosions, projectiles, and bunkers
// Outputs: none
void Nokia5110_PrintBMP(uint8_t xpos, uint8_t ypos, const uint8_t *ptr, uint8_t threshold){
  int32_t width = ptr[18], height = ptr[22], i, j;
  uint16_t screenx, screeny;
  uint8_t mask;
  // check for clipping
  if((height <= 0) ||              // bitmap is unexpectedly encoded in top-to-bottom pixel order
     ((width%2) != 0) ||           // must be even number of columns
     ((xpos + width) > SCREENW) || // right side cut off
     (ypos < (height - 1)) ||      // top cut off
     (ypos > SCREENH))           { // bottom cut off
    return;
  }
  if(threshold > 14){
    threshold = 14;             // only full 'on' turns pixel on
  }
  // bitmaps are encoded backwards, so start at the bottom left corner of the image
  screeny = ypos/8;
  screenx = xpos + SCREENW*screeny;
  mask = ypos%8;                // row 0 to 7
  mask = 0x01<<mask;            // now stores a mask 0x01 to 0x80
  j = ptr[10];                  // byte 10 contains the offset where image data can be found
  for(i=1; i<=(width*height/2); i=i+1){
    // the left pixel is in the upper 4 bits
    if(((ptr[j]>>4)&0xF) > threshold){
      Screen[screenx] |= mask;
    } else{
      Screen[screenx] &= ~mask;
    }
    screenx = screenx + 1;
    // the right pixel is in the lower 4 bits
    if((ptr[j]&0xF) > threshold){
      Screen[screenx] |= mask;
    } else{
      Screen[screenx] &= ~mask;
    }
    screenx = screenx + 1;
    j = j + 1;
    if((i%(width/2)) == 0){     // at the end of a row
      if(mask > 0x01){
        mask = mask>>1;
      } else{
        mask = 0x80;
        screeny = screeny - 1;
      }
      screenx = xpos + SCREENW*screeny;
      // bitmaps are 32-bit word aligned
      switch((width/2)%4){      // skip any padding
        case 0: j = j + 0; break;
        case 1: j = j + 3; break;
        case 2: j = j + 2; break;
        case 3: j = j + 1; break;
      }
    }
  }
}
// There is a buffer in RAM that holds one screen
// This routine clears this buffer
void Nokia5110_ClearBuffer(void){int i;
  for(i=0; i<SCREENW*SCREENH/8; i=i+1){
    Screen[i] = 0;              // clear buffer
  }
}

//********Nokia5110_DisplayBuffer*****************
// Fill the whole screen by drawing a 48x84 screen image.
// Inputs: none
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void Nokia5110_DisplayBuffer(void){
  Nokia5110_DrawFullImage(Screen);
}

const unsigned char Masks[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
//------------Nokia5110_ClrPxl------------
// Clear the Image pixel at (i, j), turning it dark.
// Input: i  the row index  (0 to 47 in this case),    y-coordinate
//        j  the column index  (0 to 83 in this case), x-coordinate
// Output: none
void Nokia5110_ClrPxl(uint32_t i, uint32_t j){
  Screen[84*(i>>3) + j] &= ~Masks[i&0x07];
}
//------------Nokia5110_SetPxl------------
// Set the Image pixel at (i, j), turning it on.
// Input: i  the row index  (0 to 47 in this case),    y-coordinate
//        j  the column index  (0 to 83 in this case), x-coordinate
// Output: none
void Nokia5110_SetPxl(uint32_t i, uint32_t j){
  Screen[84*(i>>3) + j] |= Masks[i&0x07];
}
// Abstraction of general output device
// Volume 2 section 3.4.5
// Print a character to Nokia 5110 LCD.
int fputc(int ch, FILE *f){
  Nokia5110_OutChar(ch);
  return 1;
}

// No input from Nokia, always return data.
int fgetc (FILE *f){
  return 0;
}

// Function called when file error occurs.
int ferror(FILE *f){
  /* Your implementation of ferror */
  return EOF;
}


// *************** Output_Init ********************
// Standard device driver initialization function for printf
// Initialize Nokia 5110 LCD
// Inputs: none
// Outputs: none
void Output_Init(void){
  Nokia5110_Init();
}

// Clear display
void Output_Clear(void){ // Clears the display
  Nokia5110_Clear();
}

// Turn off display (low power)
void Output_Off(void){   // Turns off the display
  Output_Clear();  // not implemented
}

// Turn on display
void Output_On(void){ // Turns on the display
  Output_Init();      // reinitialize
}

// set the color for future output
// Background color is fixed at black
// Input:  16-bit packed color
// Output: none
void Output_Color(uint32_t newColor){ // Set color of future output
  // not relevant for 1-color LCD
}


