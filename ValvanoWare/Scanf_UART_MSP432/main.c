// main.c
// Runs on MSP432
// This program calculates the area of square shaped rooms.
// UART runs at 115,200 baud rate.
// Ramesh Yerraballi & Jon Valvano
// July 16, 2015

//*****BUG*BUG*BUG*BUG****in CCS scanf drops the first character*****BUG*BUG*BUG*
//******interesting work around if you wish to enter 1234 into scanf, type "a1234<enter>"
// I.e., in CCS type a letter before the input you wish to enter

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Program 4.9, Section 4.5

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

// 1. Pre-processor Directives Section
#include <stdio.h>  // Diamond braces for sys lib: Standard I/O
#include <stdint.h> // C99 variable types

// 2. Global Declarations section
long side; // room wall meters
long area; // size squared meters
// Function Prototypes
void Output_Init(void);

// 3. Subroutines Section
// main: Mandatory routine for a C program to be executable
int main(void){
  Output_Init();                // initialize output device
  printf("This program calculates areas of square-shaped rooms\n");
  while(1){
    printf("Give room side:");  // 1) ask for input
    scanf("%ld", &side);        // 2) wait for input
    area = side*side;           // 3) calculation
    printf("\nside = %ld m, area = %ld sqr m\n", side, area); // 4) out
  }
}
