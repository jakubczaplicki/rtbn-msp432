// LCDTestMain.c
// Runs on MSP432
// Used to test the LCD.c driver
// Valvano
// July 14, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Section 4.7.1, Program 4.2 and Program 4.3

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
#include "LCD.h"
#include "systick.h"

int main(void){
  uint32_t n;

  LCD_Init();              // set system clock to 48 MHz, initialize LCD, and initialize SysTick
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

