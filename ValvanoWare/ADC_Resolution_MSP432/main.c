// main.c
// Runs on MSP432
// 100 ADC data printed to UART as a measure of resolution
// UART runs at 115,200 baud rate.
// Daniel Valvano
// June 28, 2015
// input on P4.7 = A6
// ADC input, software trigger, 14-bit conversion,
// 2.5 V static (always on) reference

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

#include <stdio.h>
#include <stdint.h> // C99 variable types
#include "ADC14.h"
#include "..\inc\msp432p401r.h"

void Output_Init(void);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint32_t ADCvalue;
volatile uint32_t ADCflag;
volatile uint32_t sum,n;
#define SIZE 8  // n-point average
// This debug function initializes SysTick to request interrupts
// at a 10 Hz frequency.
void SysTick_Init10HzInt(void){
  DisableInterrupts();
  // **** SysTick initialization ****
  SYSTICK_STCSR = 0;               // disable SysTick during setup
  SYSTICK_STRVR = (300000/SIZE)-1; // reload value for 10 Hz interrupts
  SYSTICK_STCVR = 0;               // any write to current clears it
  SYSTICK_STCSR = 0x00000007;      // enable SysTick with core clock and interrupts
  // **** interrupt initialization ****
                                   // SysTick=priority 2
  SCB_SHPR3 = (SCB_SHPR3&0x00FFFFFF)|0x40000000; // top 3 bits
}

void SysTick_Handler(void){ 
  P1OUT ^= 0x01;                   // profile
  P1OUT ^= 0x01;                   // profile
  sum += ADC_In();
  n++;
  if(n == SIZE){
    ADCvalue = (sum+SIZE/2)/SIZE;
    ADCflag = 1;                   // semaphore
    sum = n = 0;
  }
  P1OUT ^= 0x01;                   // profile
}
#define N 100
uint32_t Data[N];
uint32_t Sum;      // sum of data
uint32_t Sum2;     // sum of (data-average)^2
uint32_t Average;  // average of data = sum/N
uint32_t Variance; // =sum2/(N-1)
uint32_t Sigma;    // standard deviation = sqrt(Variance)
// Newton's method
// s is an integer
// sqrt(s) is an integer
uint32_t sqrt(uint32_t s){
uint32_t t;   // t*t will become s
int n;             // loop counter
  t = s/16+1;      // initial guess
  for(n = 16; n; --n){ // will finish
    t = ((t*t+s)/t)/2;
  }
  return t;
}

int main(void){ int32_t n;
  ADCflag = 0;
  sum = n = 0;
  Output_Init();              // initialize output device
  ADC0_InitSWTriggerCh6();    // initialize ADC sample P4.7/A6
  SysTick_Init10HzInt();
  printf("ADC resolution, %d-point average\n",SIZE);
  EnableInterrupts();
  for(n=0; n<10; n++){
    while(ADCflag == 0){};
    ADCflag = 0; // skip the first 10
  }
  Sum = Sum2 = 0;
  for(n=0; n<N; n++){
    while(ADCflag == 0){};
    Sum = Sum+ADCvalue;            // 14bits*100 = 17 bits
	Data[n] = ADCvalue;
    ADCflag = 0;
    printf("%d\n",ADCvalue);
  }
  Average = Sum/N;
  for(n=0; n<N; n++){
    Sum2 = Sum2+(Data[n]-Average)*(Data[n]-Average); // 28bits*100 = 31 bits
  }
  Variance = (100*Sum2)/(N-1);
  Sigma = sqrt(Variance);
  printf("sum      =%d\n",Sum);
  printf("sum2     =%d\n",Sum2);
  printf("variance =%d.%2d\n",Variance/100,Variance%100);
  printf("sigma    =%d.%1d\n",Sigma/10,Sigma%10);

  while(1){

  };
}
