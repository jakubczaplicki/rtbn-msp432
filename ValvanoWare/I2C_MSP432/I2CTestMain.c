// I2CTestMain.c
// Runs on MSP432
// Provide a function that initializes, sends, and receives
// the eUSCI0B module interfaced with an HMC6352 compass or
// TMP102 thermometer.
// Daniel Valvano
// August 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Section 8.6.4 Programs 8.5, 8.6 and 8.7

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// UCB0SCL connected to P1.7 and to pin 4 of HMC6352 compass or pin 3 of TMP102 thermometer
// UCB0SDA connected to P1.6 and to pin 3 of HMC6352 compass or pin 2 of TMP102 thermometer
// SCL and SDA lines pulled to +3.3 V with 10 k resistors (part of breakout module)
// ADD0 pin of TMP102 thermometer connected to GND

/*
  size is 1*16
  if do not need to read busy, then you can tie R/W=ground
  ground = pin 1    Vss
  power  = pin 2    Vdd   +3.3V or +5V depending on the device
  ground = pin 3    Vlc   grounded for highest contrast
  P9.6   = pin 4    RS    (1 for data, 0 for control/status)
  ground = pin 5    R/W   (1 for read, 0 for write)
  P9.7   = pin 6    E     (enable)
  P7.0   = pin 7    DB0   (8-bit data)
  P7.1   = pin 8    DB1
  P7.2   = pin 9    DB2
  P7.3   = pin 10   DB3
  P7.4   = pin 11   DB4
  P7.5   = pin 12   DB5
  P7.6   = pin 13   DB6
  P7.7   = pin 14   DB7
16 characters are configured as 1 row of 16
addr  00 01 02 03 04 05 ... 0F
*/

#include <stdint.h>
#include "ClockSystem.h"
#include "I2C0.h"
#include "LCD.h"
#include "SysTick.h"

// For debug purposes, this program may peek at the eUSCI
// Interrupt Flag Register to try to provide a more meaningful
// diagnostic message in the event of an error.  The rest of the
// interface with the I2C hardware occurs through the functions
// in I2C0.c.
#define UCB0IFG                 (*((volatile uint16_t *)0x4000202C)) /* eUSCI_Bx Interrupt Flag Register */

// DEBUGPRINTS==0 configures for no test prints, other value prints test text
// This tests the math used to convert the raw temperature value
// from the thermometer to a string that is displayed.  Verify
// that the left and right columns are the same.
#define DEBUGPRINTS 0

// function parses raw 16-bit number from temperature sensor in form:
// rawdata[0] = 0
// rawdata[15:8] 8-bit signed integer temperature
// rawdata[7:4] 4-bit unsigned fractional temperature (units 1/16 degree C)
//  or
// rawdata[0] = 1
// rawdata[15:7] 9-bit signed integer temperature
// rawdata[6:3] 4-bit unsigned fractional temperature (units 1/16 degree C)
void parseTemp(uint16_t rawData, int * tempInt, int * tempFra){
  if(rawData&0x0001){  // 13-bit mode
    *tempInt = rawData>>7;
    if(rawData&0x8000){// negative value
      *tempFra = (16 - ((rawData>>3)&0xF))*10000/16;
                                                 // treat as 9-bit signed
      *tempInt = (*tempInt) - 1512;              // subtract 512 to make integer portion signed
                                                 // subtract extra 1,000 so integer portion is
                                                 // still negative in the case of -0.XXXX
                                                 // (never display thousands digit)
      if(((*tempFra) > 0) && (*tempFra) < 10000){// fractional part "borrows" from integer part
        *tempInt = (*tempInt) + 1;
      }
    }
    else{
      *tempFra = ((rawData>>3)&0xF)*10000/16;
    }
  }
  else{
    *tempInt = rawData>>8;
    if(rawData&0x8000){// negative value
      *tempFra = (16 - ((rawData>>4)&0xF))*10000/16;
                                                 // treat as 8-bit signed
      *tempInt = (*tempInt) - 1256;              // subtract 256 to make integer portion signed
                                                 // subtract extra 1,000 so integer portion is
                                                 // still negative in the case of -0.XXXX
                                                 // (never display thousands digit)
      if(((*tempFra) > 0) && (*tempFra) < 10000){// decimal part "borrows" from integer part
        *tempInt = (*tempInt) + 1;
      }
    }
    else{
      *tempFra = ((rawData>>4)&0xF)*10000/16;
    }
  }
}
// function sends temperature integer and decimal components to UART
// in form:
// XXX.XXXX or -XXX.XXXX
// tempInt is signed integer value of temperature
// tempFra is unsigned fractional value of temperature (units 1/10000 degree C)
void displayTemp(int * tempInt, int * tempFra){
  uint32_t index = 0;               // string index
  char str[10];                           // holds 9 characters
  // first character is '-' if negative
  if((*tempInt) < 0){
    *tempInt = -1*(*tempInt);
    str[index] = '-';
    index = index + 1;
  }
  // next character is hundreds digit if not zero
  if(((*tempInt)/100)%10 != 0){
    str[index] = (((*tempInt)/100)%10) + '0';
    index = index + 1;
  // hundreds digit is not zero so next character is tens digit
    str[index] = (((*tempInt)/10)%10) + '0';
    index = index + 1;
  }
  // hundreds digit is zero so next character is tens digit only if not zero
  else if(((*tempInt)/10)%10 != 0){
    str[index] = (((*tempInt)/10)%10) + '0';
    index = index + 1;
  }
  // next character is ones digit
  str[index] = ((*tempInt)%10) + '0';
  index = index + 1;
  // next character is '.'
  str[index] = '.';
  index = index + 1;
  // next character is tenths digit
  str[index] = (((*tempFra)/1000)%10) + '0';
  index = index + 1;
  // next character is hundredths digit
  str[index] = (((*tempFra)/100)%10) + '0';
  index = index + 1;
  // next character is thousandths digit
  str[index] = (((*tempFra)/10)%10) + '0';
  index = index + 1;
  // next character is ten thousandths digit
  str[index] = ((*tempFra)%10) + '0';
  index = index + 1;
  // fill in any remaining characters with ' '
  while(index < 9){
    str[index] = ' ';
    index = index + 1;
  }
  // final character is null terminater
  str[index] = 0;
  // send string to LCD
  LCD_GoTo(2, 1);
  LCD_OutString(str);
}
//volatile uint16_t heading = 0;
//volatile uint8_t controlReg = 0;
int main(void){
  uint16_t rawData = 0;                   // 16-bit data straight from thermometer
  int tempInt = 0;                        // integer value of temperature (-128 to 127)
  int tempFra = 0;                        // fractional value of temperature (0 to 9375)
  LCD_Init();                             // set system clock to 48 MHz, initialize LCD, and initialize SysTick
  LCD_Clear();
  I2C_Init();
                                          // write commands to 0x48 (ADDR to ground)
  I2C_Send1(0x48, 1);                     // use command 1 to set pointer to config (Figure 7.XX chapter7-10-1.ulb)
                                          // read from 0x48 to get data
  if(I2C_Recv2(0x48) == 0x60A0){          // expected 0x60A0 as power-on default (Figure 7.XX chapter7-10-2.ulb)
    LCD_OutString("Test Passed");
  }
  else{
    if(UCB0IFG&0x0020){
      LCD_OutString("No Acknowledge");
    }
    else{
      LCD_OutString("Test Failed");
    }
  }
  SysTick_Wait10ms(150);                  // short delay
//test display and number parser (can safely be skipped)
#if DEBUGPRINTS
  LCD_GoTo(1, 1);
  LCD_OutString("Test:    Expect:");
  parseTemp(0x7FF0, &tempInt, &tempFra);  // expect 127.9375
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 127.9375");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x6400, &tempInt, &tempFra);  // expect 100.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 100.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x3200, &tempInt, &tempFra);  // expect 50.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 50.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x1900, &tempInt, &tempFra);  // expect 25.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 25.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x0300, &tempInt, &tempFra);  // expect 3.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 3.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x0100, &tempInt, &tempFra);  // expect 1.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 1.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x0040, &tempInt, &tempFra);  // expect 0.2500
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 0.2500");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x0060, &tempInt, &tempFra);  // expect 0.3750
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 0.3750");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x00F0, &tempInt, &tempFra);  // expect 0.9375
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 0.9375");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x0000, &tempInt, &tempFra);  // expect 0.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" 0.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0xFFC0, &tempInt, &tempFra);  // expect -0.2500
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" -0.2500");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0xE700, &tempInt, &tempFra);  // expect -25.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" -25.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0xC900, &tempInt, &tempFra);  // expect -55.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" -55.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0xFF00, &tempInt, &tempFra);  // expect -1.0000
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" -1.0000");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x9D60, &tempInt, &tempFra);  // expect -98.6250
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" -98.6250");
  SysTick_Wait10ms(150);                  // short delay
  parseTemp(0x8490, &tempInt, &tempFra);  // expect -123.4375
  displayTemp(&tempInt, &tempFra);
  LCD_GoTo(2, 9);
  LCD_OutString(" -123.4375");
  SysTick_Wait10ms(150);                  // short delay
#endif
//done testing
//eUSCI0B test
  rawData = (I2C_Recv(0x48)<<8);          // top 8 bits of configuration register 0
  rawData = I2C_Recv2(0x48);              // 16 bits of configuration register 0
  rawData = I2C_Send1(0x69, 1);           // bad address: give up without crashing
  rawData = I2C_Send2(0x69, 3, 3);        // bad address: give up without crashing
  rawData = I2C_Send3(0x69, 7, 0x0F, 0x0E);// bad address: give up without crashing
  rawData = I2C_Recv(0x69);               // bad address: return 0xFF without crashing
  rawData = I2C_Recv2(0x69);              // bad address: return 0xFFFF without crashing
//end of eUSCI0B test
  LCD_Clear();
  LCD_OutString("Current Temp:");
                                          // write commands to 0x48 (ADDR to ground)
  I2C_Send1(0x48, 0);                     // use command 0 to set pointer to temperature
  while(1){
                                          // read from 0x48 to get temperature
    rawData = I2C_Recv2(0x48);            // temperature data stored in 12 or 13 MSBs (Figure 7.XX chapter7-10-3.ulb, reply 0x1660, 22.375 C)
    parseTemp(rawData, &tempInt, &tempFra);
    displayTemp(&tempInt, &tempFra);
    SysTick_Wait10ms(25);                 // wait 250 ms to sample at ~4 Hz
  }
/*  while(1){
                                          // write commands to 0x42
    I2C_Send1(0x42, 'A');                 // use command 'A' to sample
    SysTick_Wait(288000);                 // wait 6,000 us for sampling to finish
                                          // read from 0x43 to get data
    heading = I2C_Recv2(0x43);            // 0 to 3599 (units: 1/10 degree)
// test sending multiple bytes and receiving single byte
                                          // write commands to 0x42
    I2C_Send2(0x42, 'g', 0x74);           // use command 'g' to read from RAM 0x74
    SysTick_Wait(3360);                   // wait 70 us for RAM access to finish
    controlReg = I2C_Recv(0x43);          // expected 0x50 as default
    SysTick_Wait10ms(100);                // wait 1 sec
  }*/
}
