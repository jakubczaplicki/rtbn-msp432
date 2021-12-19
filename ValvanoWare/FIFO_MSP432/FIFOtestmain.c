// FIFOTestMain.c
// Runs on any Cortex M microcontroller
/* This example accompanies the book
  "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015


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
// Daniel Valvano
// May 3, 2015



#include <stdint.h>
#include "Fifo.h"
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value


uint32_t RunNumber;
txDataType in,expected,actual;
void TestPut(void){int result;
  result = TxFifo_Put(in); 
  if(result == TXFIFOFAIL){
    while(1){}; // bad
  }
  in++;
}
void TestGet(void){int result;
  result = TxFifo_Get(&actual);
  if(result == TXFIFOFAIL){
    while(1){}; // bad
  }  if(actual != expected){
    while(1){}; // bad
  }
  expected++;
}
//debug code
int main(void){ 

  TxFifo_Init();
  in = expected = 0;
  RunNumber = 0;
  while(1){
    TestPut(); // 1
    TestPut(); // 2
    TestPut(); // 3
    TestGet(); // 1
    TestPut(); // 4
    TestPut(); // 5
    TestGet(); // 2
    TestPut(); // 6
    TestPut(); // 7
    TestGet(); // 3
    TestGet(); // 4
    TestGet(); // 5
    TestGet(); // 6
    TestGet(); // 7
    RunNumber++;
  }
}
