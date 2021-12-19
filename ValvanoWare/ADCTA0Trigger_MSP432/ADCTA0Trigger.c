// ADCTA0Trigger.c
// Runs on MSP432
// Use Timer A0 in periodic mode to trigger ADC conversions at a
// particular period.  Request an interrupt when the conversion is
// complete.
// Daniel Valvano
// July 8, 2015

/* This example accompanies the books
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

#include <stdint.h>
#include "..\inc\msp432p401r.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void (*FinishedTask)(uint16_t result);// user function called when conversion complete

// ***************** ADC0_InitTA0TriggerCh6 ****************
// Activate Timer A0 to periodically trigger ADC conversions
// on P4.7 = A6. Run the user task when each conversion is
// complete.
// Inputs:  task is a pointer to a user function
//          period in units (1/SMCLK), 16 bits
// Outputs: none
void ADC0_InitTA0TriggerCh6(void(*task)(uint16_t result), uint16_t period){long sr;
  sr = StartCritical();
  FinishedTask = task;             // user function
  // ***************** timer initialization *****************
  TA0CTL &= ~0x0030;               // halt Timer A0
  // bits15-10=XXXXXX, reserved
  // bits9-8=10,       clock source to SMCLK
  // bits7-6=00,       input clock divider /1
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=0,           interrupt disable
  // bit0=0,           clear interrupt pending
  TA0CTL = 0x0200;
  // bits15-14=00,     no capture mode
  // bits13-12=XX,     capture/compare input select
  // bit11=X,          synchronize capture source
  // bit10=X,          synchronized capture/compare input
  // bit9=X,           reserved
  // bit8=0,           compare mode
  // bits7-5=011,      set/reset output mode
  // bit4=0,           disable capture/compare interrupt
  // bit3=X,           read capture/compare input from here
  // bit2=X,           output this value in output mode 0
  // bit1=X,           capture overflow status
  // bit0=0,           clear capture/compare interrupt pending
  TA0CCTL1 = 0x0060;
  TA0CCR1 = (period - 1)/2;        // defines when output signal is set, triggering ADC conversion
  TA0CCR0 = (period - 1);          // defines when output signal is cleared
  TA0EX0 &= ~0x0007;               // configure for input clock divider /1
  // ***************** reference initialization *****************
  while(REFCTL0&0x0400){};         // wait for the reference to be idle before reconfiguring (see REF4 in errata)
  REFCTL0 = 0x0039;                // 1) configure reference for static 2.5V
//debug code
/*  REFCTL0 = 0x003B;                // 1) configure reference for static 2.5V and constant output on P5.6 (see REF7 in errata)
  P5SEL1 |= 0x40;                  // analog mode on P5.6
  P5SEL0 |= 0x40;*/
//end of debug code
  // 15-14 reserved                          00b (reserved)
  // 13    REFBGRDY   bandgap voltage ready   0b (read only)
  // 12    REFGENRDY  reference voltage ready 0b (read only)
  // 11    BGMODE     bandgap mode            0b (read only)
  // 10    REFGENBUSY no reconfigure if busy  0b (read only)
  // 9     REFBGACT   bandgap active          0b (read only)
  // 8     REFGENACT  reference active        0b (read only)
  // 7     REFBGOT    trigger bandgap         0b = no software trigger set
  // 6     REFGENOT   trigger reference       0b = no software trigger set
  // 5-4   REFVSEL    voltage level select   11b = 2.5V
  // 3     REFTCOFF   temperature disable     1b = temperature sensor disabled to save power
  // 2     reserved                           0b (reserved)
  // 1     REFOUT     reference output buffer 0b = reference output not on P5.6 (see also REFBURST in ADC14CTL1, P5SEL1, and P5SEL0)
  // 0     REFON      reference enable        1b = reference enabled in static mode
  // ***************** ADC initialization *****************
  while((REFCTL0&0x1000) == 0){};  // wait for the reference to stabilize before continuing (optional, see REF8 in errata)
  ADC14CTL0 &= ~0x00000002;        // 2) ADC14ENC = 0 to allow programming
  while(ADC14CTL0&0x00010000){};   // 3) wait for BUSY to be zero
  // 31-30 ADC14PDIV  predivider,            00b = Predivide by 1
  // 29-27 ADC14SHSx  SHM source            001b = TA0_C1 bit
  // 26    ADC14SHP   SHM pulse-mode          1b = SAMPCON the sampling timer
  // 25    ADC14ISSH  invert sample-and-hold  0b =  not inverted
  // 24-22 ADC14DIVx  clock divider         000b = /1
  // 21-19 ADC14SSELx clock source select   100b = SMCLK
  // 18-17 ADC14CONSEQx mode select          10b = Repeat-single-channel
  // 16    ADC14BUSY  ADC14 busy              0b (read only)
  // 15-12 ADC14SHT1x sample-and-hold time 0011b = 32 clocks
  // 11-8  ADC14SHT0x sample-and-hold time 0011b = 32 clocks
  // 7     ADC14MSC   multiple sample         0b = one sample per SHI rising edge
  // 6-5   reserved                          00b (reserved)
  // 4     ADC14ON    ADC14 on                1b = powered up
  // 3-2   reserved                          00b (reserved)
  // 1     ADC14ENC   enable conversion       0b = ADC14 disabled
  // 0     ADC14SC    ADC14 start             0b = No start (yet)
  ADC14CTL0 = 0x0C243310;          // 4) repeat single, SMCLK, on, disabled, /1, 32 SHM
  // 20-16 STARTADDx  start addr          00000b = ADC14MEM0
  // 15-6  reserved                  0000000000b (reserved)
  // 5-4   ADC14RES   ADC14 resolution       11b = 14 bit, 16 clocks
  // 3     ADC14DF    data read-back format   0b = Binary unsigned
  // 2     REFBURST   reference buffer burst  0b = reference on continuously
  // 1-0   ADC14PWRMD ADC power modes        00b = Regular power mode
  ADC14CTL1 = 0x00000030;          // 5) ADC14MEM0, 14-bit, ref on, regular power
  // 15   ADC14WINCTH Window comp threshold   0b = not used
  // 14   ADC14WINC   Comparator enable       0b = Comparator disabled
  // 13   ADC14DIF    Differential mode       0b = Single-ended mode enabled
  // 12   reserved                            0b (reserved)
  // 11-8 ADC14VRSEL  V(R+) and V(R-)      0001b = V(R+) = VREF, V(R-) = AVSS
  // 7    ADC14EOS    End of sequence         1b = End of sequence
  // 6-5  reserved                           00b (reserved)
  // 4-0  ADC14INCHx  Input channel        0110b = A6, P4.7
  ADC14MCTL0 = 0x00000186;         // 6) 0 to 2.5V, channel 6
  ADC14IER0 |= 0x00000001;         // 7) enable ADC14IFG0 interrupt
  ADC14IER1 = 0;                   //    disable these interrupts
  P4SEL1 |= 0x80;                  // 8) analog mode on A6, P4.7
  P4SEL0 |= 0x80;
  ADC14CTL0 |= 0x00000002;         // 9) enable
  // ***************** interrupt initialization *****************
  NVIC_IPR6 = (NVIC_IPR6&0xFFFFFF00)|0x00000040; // priority 2
// interrupts enabled in the main program after all devices initialized
  NVIC_ISER0 = 0x01000000;         // enable interrupt 24 in NVIC
  TA0CTL |= 0x0014;                // reset and start Timer A0 in up mode
  EndCritical(sr);
}

void ADC14_IRQHandler(void){ uint16_t result;
  if((ADC14IFGR0&0x00000001) == 0x00000001){
    result = ADC14MEM0;            // read the result and acknowledge ADC completion
    (*FinishedTask)(result);       // execute user task
  }
}
