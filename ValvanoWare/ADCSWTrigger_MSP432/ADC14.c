// ADC14.c
// Runs on MSP432
// ADC input, software trigger, 14-bit conversion,
// 2.5 V static (always on) reference
// Daniel Valvano
// June 11, 2015

/* This example accompanies the books
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1 Programs 10.2 and 10.3

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

// P4.7 = A6
void ADC0_InitSWTriggerCh6(void){
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
  while((REFCTL0&0x1000) == 0){};  // wait for the reference to stabilize before continuing (optional, see REF8 in errata)
  ADC14CTL0 &= ~0x00000002;        // 2) ADC14ENC = 0 to allow programming
  while(ADC14CTL0&0x00010000){};   // 3) wait for BUSY to be zero
  ADC14CTL0 = 0x04203310;          // 4) single, SMCLK, on, disabled, /1, 32 SHM
  // 31-30 ADC14PDIV  predivider,            00b = Predivide by 1
  // 29-27 ADC14SHSx  SHM source            000b = ADC14SC bit
  // 26    ADC14SHP   SHM pulse-mode          1b = SAMPCON the sampling timer
  // 25    ADC14ISSH  invert sample-and-hold  0b =  not inverted
  // 24-22 ADC14DIVx  clock divider         000b = /1
  // 21-19 ADC14SSELx clock source select   100b = SMCLK
  // 18-17 ADC14CONSEQx mode select          00b = Single-channel, single-conversion
  // 16    ADC14BUSY  ADC14 busy              0b (read only)
  // 15-12 ADC14SHT1x sample-and-hold time 0011b = 32 clocks
  // 11-8  ADC14SHT0x sample-and-hold time 0011b = 32 clocks
  // 7     ADC14MSC   multiple sample         0b = not multiple
  // 6-5   reserved                          00b (reserved)
  // 4     ADC14ON    ADC14 on                1b = powered up
  // 3-2   reserved                          00b (reserved)
  // 1     ADC14ENC   enable conversion       0b = ADC14 disabled
  // 0     ADC14SC    ADC14 start             0b = No start (yet)
  ADC14CTL1 = 0x00000030;          // 5) ADC14MEM0, 14-bit, ref on, regular power
  // 20-16 STARTADDx  start addr          00000b = ADC14MEM0
  // 15-6  reserved                  0000000000b (reserved)
  // 5-4   ADC14RES   ADC14 resolution       11b = 14 bit, 16 clocks
  // 3     ADC14DF    data read-back format   0b = Binary unsigned
  // 2     REFBURST   reference buffer burst  0b = reference on continuously
  // 1-0   ADC14PWRMD ADC power modes        00b = Regular power mode
  ADC14MCTL0 = 0x00000186;         // 6) 0 to 2.5V, channel 6
  // 15   ADC14WINCTH Window comp threshold   0b = not used
  // 14   ADC14WINC   Comparator enable       0b = Comparator disabled
  // 13   ADC14DIF    Differential mode       0b = Single-ended mode enabled
  // 12   reserved                            0b (reserved)
  // 11-8 ADC14VRSEL  V(R+) and V(R-)      0001b = V(R+) = VREF, V(R-) = AVSS
  // 7    ADC14EOS    End of sequence         1b = End of sequence
  // 6-5  reserved                           00b (reserved)
  // 4-0  ADC14INCHx  Input channel        0110b = A6, P4.7
  ADC14IER0 = 0; // 7) no interrupts
  ADC14IER1 = 0; // no interrupts
  P4SEL1 |= 0x80;                  // 8) analog mode on A6, P4.7
  P4SEL0 |= 0x80;
  ADC14CTL0 |= 0x00000002;         // 9) enable
}
// ADC14IFGR0 bit 0 is set when conversion done
//                  cleared on read ADC14MEM0
// ADC14CLRIFGR0 bit 0, write 1 to clear flag
// ADC14IVx is 0x0C when ADC14MEM0 interrupt flag; Interrupt Flag: ADC14IFG0
// ADC14MEM0 14-bit conversion in bits 13-0 (31-16 undefined, 15-14 zero)
uint32_t ADC_In(void){
  while(ADC14CTL0&0x00010000){};    // 1) wait for BUSY to be zero
  ADC14CTL0 |= 0x00000001;          // 2) start single conversion
  while((ADC14IFGR0&0x01) == 0){};  // 3) wait for ADC14IFG0
  return ADC14MEM0;                 // 4) return result 0 to 16383
}
