// ClockSystem.c
// Runs on MSP432
// Change the clock frequency using the Clock System module.
// Daniel Valvano
// June 30, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
   Program 4.6

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
#include "ClockSystem.h"
#include "..\inc\msp432p401r.h"

//------------Clock_Init------------
// Configure for SMCLK = MCLK = speed, ACLK = REFOCLK.
// Input: 1.5, 3, 6, or 12 MHz, 0.5% accuracy
// Output: none   12 MHz is the max speed for SMCLK
void Clock_Init(enum dcofrequency speed){
  CSKEY = 0x695A;                       // unlock CS module for register access
  CSCTL0 = 0;                           // reset tuning parameters
  CSCTL0 |= 0x00010000;                 // configure for nominal DCO frequency = 3 MHz (default)
  CSCTL0 |= 0x00400000;                 // enable DCO external resistor mode (must be done with DCORSEL == 1)
  CSCTL0 = (CSCTL0&~0x00070000) |       // clear DCORSEL bit field
           speed;                       // configure for nominal DCO frequency = 1.5,3,6, or 12 MHz
  CSCTL1 = 0x00000200 |                 // configure for ACLK sourced from REFOCLK
           0x00000030 |                 // configure for SMCLK and HSMCLK sourced from DCO
           0x00000003;                  // configure for MCLK sourced from DCO
  CSKEY = 0;                            // lock CS module from unintended access
}

//------------Clock_Init48MHz------------
// Configure for MCLK = HFXTCLK, HSMCLK = HFXTCLK/2,
// SMCLK = HFXTCLK/4, ACLK = REFOCLK.
// On the LaunchPad, the high-frequency crystal
// oscillator has a 48 MHz crystal attached, which will
// make the bus (master) clock run at 48 MHz.  The sub-
// system master clock (HSMCLK) runs at its maximum of
// 24 MHz.  The low-speed subsystem master clock (SMCLK)
// runs at its maximum of 12 MHz.  In other words, this
// function is similar to Clock_Init(), except instead
// of the variable frequency DCO this uses the fixed
// high-frequency crystal.
// Input: none
// Output: none
uint32_t Prewait = 0;                   // loops between Clock_Init48MHz() called and PCM idle (expect 0)
uint32_t CPMwait = 0;                   // loops between Power Active Mode Request and Current Power Mode matching requested mode (expect small)
uint32_t Postwait = 0;                  // loops between Current Power Mode matching requested mode and PCM module idle (expect about 0)
uint32_t IFlags = 0;                    // non-zero if transition is invalid
uint32_t Crystalstable = 0;             // loops before the crystal stabilizes (expect small)
void Clock_Init48MHz(void){
  // wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
  while(PCMCTL1&0x00000100){
    Prewait = Prewait + 1;
    if(Prewait >= 100000){
      return;                           // time out error
    }
  }
  // request power active mode LDO VCORE1 to support the 48 MHz frequency
  PCMCTL0 = (PCMCTL0&~0xFFFF000F) |     // clear PCMKEY bit field and AMR bit field
            0x695A0000 |                // write the proper PCM key to unlock write access
            0x00000001;                 // request power active mode LDO VCORE1
  // check if the transition is invalid (see Figure 7-3 on p344 of datasheet)
  if(PCMIFG&0x00000004){
    IFlags = PCMIFG;                    // bit 2 set on active mode transition invalid; bits 1-0 are for LPM-related errors; bit 6 is for DC-DC-related error
    PCMCLRIFG = 0x00000004;             // clear the transition invalid flag
    // to do: look at CPM bit field in PCMCTL0, figure out what mode you're in, and step through the chart to transition to the mode you want
    // or be lazy and do nothing; this should work out of reset at least, but it WILL NOT work if Clock_Int32kHz() or Clock_InitLowPower() has been called
    return;
  }
  // wait for the CPM (Current Power Mode) bit field to reflect a change to active mode LDO VCORE1
  while((PCMCTL0&0x00003F00) != 0x00000100){
    CPMwait = CPMwait + 1;
    if(CPMwait >= 500000){
      return;                           // time out error
    }
  }
  // wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
  while(PCMCTL1&0x00000100){
    Postwait = Postwait + 1;
    if(Postwait >= 100000){
      return;                           // time out error
    }
  }
  // initialize PJ.3 and PJ.2 and make them HFXT (PJ.3 built-in 48 MHz crystal out; PJ.2 built-in 48 MHz crystal in)
  PJSEL0 |= 0x0C;
  PJSEL1 &= ~0x0C;                      // configure built-in 48 MHz crystal for HFXT operation
//  PJDIR |= 0x08;                        // make PJ.3 HFXTOUT (unnecessary)
//  PJDIR &= ~0x04;                       // make PJ.2 HFXTIN (unnecessary)
  CSKEY = 0x695A;                       // unlock CS module for register access
  CSCTL2 = (CSCTL2&~0x00700000) |       // clear HFXTFREQ bit field
           0x00600000 |                 // configure for 48 MHz external crystal
           0x00010000 |                 // HFXT oscillator drive selection for crystals >4 MHz
           0x01000000;                  // enable HFXT
  CSCTL2 &= ~0x02000000;                // disable high-frequency crystal bypass
  // wait for the HFXT clock to stabilize
  while(CSIFG&0x00000002){
    CSCLRIFG = 0x00000002;              // clear the HFXT oscillator interrupt flag
    Crystalstable = Crystalstable + 1;
    if(Crystalstable > 100000){
      return;                           // time out error
    }
  }
  CSCTL1 = 0x20000000 |                 // configure for SMCLK divider /4
           0x00100000 |                 // configure for HSMCLK divider /2
           0x00000200 |                 // configure for ACLK sourced from REFOCLK
           0x00000050 |                 // configure for SMCLK and HSMCLK sourced from HFXTCLK
           0x00000005;                  // configure for MCLK sourced from HFXTCLK
  CSKEY = 0;                            // lock CS module from unintended access
}

//------------Clock_Init32kHz------------
// Configure for HSMCLK = MCLK = LFXTCLK,
// SMCLK = LFXTCLK/2, ACLK = REFOCLK.
// On the LaunchPad, the low-frequency crystal
// oscillator has a 32 kHz crystal attached, which will
// make the high speed subsystem master clock and the
// bus (master) clock run at 32 kHz.  The low speed
// subsystem master clock has a maximum frequency of
// 16,384 Hz in LPM3 and LPM3.5, so this function sets
// the SMCLK divider to /2.  This is slower than
// necessary for active mode.  In other words, this
// function is similar to Clock_Init(), except instead
// of the variable frequency DCO this uses the fixed
// low-frequency crystal.
// Input: none
// Output: none
void Clock_Init32kHz(void){
  // wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
  while(PCMCTL1&0x00000100){
    Prewait = Prewait + 1;
    if(Prewait >= 100000){
      return;                           // time out error
    }
  }
  // initialize PJ.1 and PJ.0 and make them LFXT (PJ.1 built-in 32 kHz crystal out; PJ.0 built-in 32 kHz crystal in)
  PJSEL0 |= 0x03;
  PJSEL1 &= ~0x03;                      // configure built-in 32 kHz crystal for LFXT operation
//  PJDIR |= 0x02;                        // make PJ.1 LFXTOUT (unnecessary)
//  PJDIR &= ~0x01;                       // make PJ.0 LFXTIN (unnecessary)
  CSKEY = 0x695A;                       // unlock CS module for register access
  CSCTL2 = (CSCTL2&~0x00000003) |       // clear LFXTDRIVE bit field
           0x00000003 |                 // configure for maximum drive strength/current consumption
           0x00000100;                  // enable LFXT
  CSCTL2 &= ~0x00000200;                // disable low-frequency crystal bypass
  // wait for the LFXT clock to stabilize
  while(CSIFG&0x00000001){
    CSCLRIFG = 0x00000001;              // clear the LFXT oscillator interrupt flag
    Crystalstable = Crystalstable + 1;
    if(Crystalstable > 100000){
      return;                           // time out error
    }
  }
  CSCTL1 = 0x10000000 |                 // configure for SMCLK divider /2 (necessary for LPM3 and LPM3.5)
           0x00000000 |                 // configure for HSMCLK divider /1
           0x00000200 |                 // configure for ACLK sourced from REFOCLK
           0x00000000 |                 // configure for SMCLK and HSMCLK sourced from LFXTCLK
           0x00000000;                  // configure for MCLK sourced from LFXTCLK
  CSKEY = 0;                            // lock CS module from unintended access
  // wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
  while(PCMCTL1&0x00000100){
    Postwait = Postwait + 1;
    if(Postwait >= 100000){
      return;                           // time out error
    }
  }
  // request power active mode LF VCORE0 to support the 32 kHz frequency
  PCMCTL0 = (PCMCTL0&~0xFFFF000F) |     // clear PCMKEY bit field and AMR bit field
            0x695A0000 |                // write the proper PCM key to unlock write access
            0x00000008;                 // request power active mode LF VCORE0
  // check if the transition is invalid (see Figure 7-3 on p344 of datasheet)
  if(PCMIFG&0x00000004){
    IFlags = PCMIFG;                    // bit 2 set on active mode transition invalid; bits 1-0 are for LPM-related errors; bit 6 is for DC-DC-related error
    PCMCLRIFG = 0x00000004;             // clear the transition invalid flag
    // to do: look at CPM bit field in PCMCTL0, figure out what mode you're in, and step through the chart to transition to the mode you want
    // or be lazy and do nothing; this should work out of reset at least, but it WILL NOT work if Clock_Int48MHz() has been called
    return;
  }
  // wait for the CPM (Current Power Mode) bit field to reflect a change to active mode LF VCORE0
  while((PCMCTL0&0x00003F00) != 0x00000800){
    CPMwait = CPMwait + 1;
    if(CPMwait >= 500000){
      return;                           // time out error
    }
  }
}

//------------Clock_InitLowPower------------
// Configure for HSMCLK = MCLK = ACLK = REFOCLK,
// SMCLK = REFOCLK/2.
// On the LaunchPad, the low-frequency, low-power
// oscillator has a 32 kHz reference clock, which will
// make the high speed subsystem master clock and the
// bus (master) clock run at 32 kHz.  The low speed
// subsystem master clock has a maximum frequency of
// 16,384 Hz in LPM3 and LPM3.5, so this function sets
// the SMCLK divider to /2.  This is slower than
// necessary for active mode.  In other words, this
// function is similar to Clock_Init32kHz(), except
// instead it should be lower power and slightly less
// accurate.
// Input: none
// Output: none
void Clock_InitLowPower(void){
  // wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
  while(PCMCTL1&0x00000100){
    Prewait = Prewait + 1;
    if(Prewait >= 100000){
      return;                           // timed out
    }
  }
  CSKEY = 0x695A;                       // unlock CS module for register access
  CSCTL1 = 0x10000000 |                 // configure for SMCLK divider /2 (necessary for LPM3 and LPM3.5)
           0x00000000 |                 // configure for HSMCLK divider /1
           0x00000200 |                 // configure for ACLK sourced from REFOCLK
           0x00000020 |                 // configure for SMCLK and HSMCLK sourced from REFOCLK
           0x00000002;                  // configure for MCLK sourced from REFOCLK
  CSKEY = 0;                            // lock CS module from unintended access
  // wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
  while(PCMCTL1&0x00000100){
    Postwait = Postwait + 1;
    if(Postwait >= 100000){
      return;                           // timed out
    }
  }
  // request power active mode LF VCORE0 to support the 32 kHz frequency
  PCMCTL0 = (PCMCTL0&~0xFFFF000F) |     // clear PCMKEY bit field and AMR bit field
            0x695A0000 |                // write the proper PCM key to unlock write access
            0x00000008;                 // request power active mode LF VCORE0
  // check if the transition is invalid (see Figure 7-3 on p344 of datasheet)
  if(PCMIFG&0x00000004){
    IFlags = PCMIFG;                    // bit 2 set on active mode transition invalid; bits 1-0 are for LPM-related errors; bit 6 is for DC-DC-related error
    PCMCLRIFG = 0x00000004;             // clear the transition invalid flag
    // to do: look at CPM bit field in PCMCTL0, figure out what mode you're in, and step through the chart to transition to the mode you want
    // or be lazy and do nothing; this should work out of reset at least, but it WILL NOT work if Clock_Int48MHz() has been called
    return;
  }
  // wait for the CPM (Current Power Mode) bit field to reflect a change to active mode LF VCORE0
  while((PCMCTL0&0x00003F00) != 0x00000800){
    CPMwait = CPMwait + 1;
    if(CPMwait >= 500000){
      return;                           // timed out
    }
  }
}
