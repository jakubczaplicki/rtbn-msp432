// derived from msp432p401_rtc_lpm35_01.c
// Low power example
// goal is to execute a task (toggle P6.0) every 1sec
// all systems use interrupts
// all systems have no-operation main loop that executes wait for interrupts (wfi)
// System 1: DCO running at 3 MHz ±0.5%. T0_0 periodic interrupts
// System 2: HFXT, running at 48 MHz using crystal T0_0 periodic interrupts
// System 3: LFXT, running at 32 kHz, RTC interrupts, LPM3.5

//                MSP432p401rpz
//             -----------------
//        /|\ |              XIN|-
//         |  |                 | 32kHz
//         ---|RST          XOUT|-
//            |                 |
//            |            P6.0 |--> Toggles every second
//            |            P6.1 |--> Toggles every second
//            |                 |
#include "..\inc\msp432p401r.h"
#include "stdint.h"

void  DisableInterrupts(void) ;
void  EnableInterrupts(void) ;
void  StartCritical(void) ;
void  EndCritical(void);
void  WaitForInterrupt (void);
int main(void){  // GPIO Port Configuration for lowest power configuration
  P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00; P4OUT = 0x00; P5OUT = 0x00; P6OUT = 0x00;
  P7OUT = 0x00; P8OUT = 0x00; P9OUT = 0x00; PJOUT = 0x00; P10OUT = 0x00;
  P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF; P4DIR = 0xFF; P5DIR = 0xFF; P6DIR = 0xFF;
  P7DIR = 0xFF; P8DIR = 0xFF; P9DIR = 0xFF; PJDIR = 0xFF; P10DIR = 0xFF;
  RTCCTL0_H = RTCKEY_H;   // Unlock RTC key protected registers
  RTCCTL0_L = RTCRDYIE;   // RTC read ready interrupt once a second
  RTCCTL1 |= RTCBCD + RTCHOLD + RTCMODE;    // RTC enable, BCD mode, RTC hold
  RTCCTL1 &= ~(RTCHOLD);                    // Start RTC calendar mode
  RTCCTL0_H = 0;                            // Lock the RTC registers
    /* Enable all SRAM bank retentions prior to going to LPM3 (Deep-sleep) */
  SYSCTL_SRAM_BANKRET |= SYSCTL_SRAM_BANKRET_BNK7_RET;
  EnableInterrupts();
  NVIC_ISER0 = 1 << ((INT_RTC_C - 16) & 31);
  SCB_SCR &= ~SCB_SCR_SLEEPONEXIT;        // Wake up on exit from ISR
  while(1){
/* Entering LPM3.5 with GPIO interrupt */
    PCMCTL0 &= ~0xF0; /* Clearing the Power Mode Requests */
    SCB_SCR |= (SCB_SCR_SLEEPDEEP);/* Setting the sleep deep bit */
    WaitForInterrupt();
    __asm ("  NOP\n");             // For debugger
    P6OUT ^= 0x02;
    SCB_SCR &= ~(SCB_SCR_SLEEPDEEP); /* Clearing the sleep deep bit */
  }
}

// RTC interrupt service routine
void RTC_C_IRQHandler(void){   // every 1 sec
  if(RTCCTL0_L & RTCRDYIFG){ // *****does this work without this line??
	RTCCTL0_H = RTCKEY_H;      // unlock
    RTCCTL0_L &= ~ RTCRDYIFG;  // ack
    RTCCTL0_H = 0;             // lock
    P6OUT ^= 0x01;             // user task
  }
}
/*
void  error(void);

int main3(void){  uint32_t currentPowerState;
  volatile uint32_t i;
  P6DIR |= BIT0;
  // Switch MCLK, SMCLK, ACLK sources over to REFO clock for low frequency operation first
  CSKEY = CSKEY_VAL;                     // Unlock CS module
  CSCTL1 = SELM_2 | SELS_2 | SELA_2;
  CSKEY = 0;                             // Lock CS module
  currentPowerState = PCMCTL0 & CPM_M;// Get current power state
  // Transition to LPR from current LDO power state properly
  switch (currentPowerState) {
    case CPM_0:                // AM_LDO_VCORE0, need to switch to AM_LPR_VCORE0
      while((PCMCTL1 & PMR_BUSY));
      PCMCTL0 = PCMKEY_M | AMR_8;
      while((PCMCTL1 & PMR_BUSY));
      if(PCMIFG & AM_INVALID_TR_IFG)
        error();                    // Error if transition was not successful
      break;
    case CPM_1:                // AM_LDO_VCORE1, need to switch to AM_LPR_VCORE1
      while((PCMCTL1 & PMR_BUSY));
      PCMCTL0 = PCMKEY_M | AMR_9;
      while((PCMCTL1 & PMR_BUSY));
      if (PCMIFG & AM_INVALID_TR_IFG)
        error();                    // Error if transition was not successful
      break;
    case CPM_8:                // Device is already in AM_LPR_VCORE0
      break;
    case CPM_9:                // Device is already in AM_LPR_VCORE1
      break;
    default:                            // Device is in some other state, which is unexpected
      error();
  }
// Setup TA0
  TA0CCTL0 =  CCIE;             // TACCR0 interrupt enabled
  TA0CCR0 = 4096;               // 32768 Hz/4096/2 = 4 Hz interrupt period
  TA0CTL = TASSEL_1 + MC_3;     // ACLK, up-down mode
  SCB_SCR |= SCB_SCR_SLEEPONEXIT;           // Enable sleep on exit from ISR
  EnableInterrupts();  // I=0
  NVIC_ISER0 = 1 << ((INT_TA0_0 - 16) & 31);
  // From Low-Frequency Active Mode, go to Low-Frequency LPM0
  while(1){
    WaitForInterrupt();  // wfi
    __asm ("  NOP\n");                      // For debugger
  }
}
*/




// ***************** TimerA0_Init ****************
// Activate Timer A0 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/SMCLK), 16 bits
// Outputs: none
void TimerA0_Init(uint16_t period){
  TA0CTL &= ~0x0030;               // halt Timer A0
  // bits15-10=XXXXXX, reserved
  // bits9-8=10,       clock source to SMCLK
  // bits7-6=00,       input clock divider /1
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=1,           interrupt enable
  // bit0=0,           clear interrupt pending
  TA0CTL = 0x0202;
  // bits15-14=00,     no capture mode
  // bits13-12=XX,     capture/compare input select
  // bit11=X,          synchronize capture source
  // bit10=X,          synchronized capture/compare input
  // bit9=X,           reserved
  // bit8=0,           compare mode
  // bits7-5=XXX,      output mode
  // bit4=1,           enable capture/compare interrupt
  // bit3=X,           read capture/compare input from here
  // bit2=0,           output this value in output mode 0
  // bit1=X,           capture overflow status
  // bit0=0,           clear capture/compare interrupt pending
  TA0CCTL0 = 0x0010;
  TA0CCR0 = (period - 1);          // compare match value
  TA0EX0 &= ~0x0007;               // configure for input clock divider /1
  NVIC_IPR2 = (NVIC_IPR2&0xFFFFFF00)|0x00000040; // priority 2
// interrupts enabled in the main program after all devices initialized
  NVIC_ISER0 = 0x00000100;         // enable interrupt 8 in NVIC
  TA0CTL |= 0x0014;                // reset and start Timer A0 in up mode
}
void TA0_0_IRQHandler(void){
  TA0CCTL0 &= ~0x0001;       // acknowledge capture/compare interrupt 0
  P6OUT ^= 0x01;             // user task
}
enum dcofrequency{
  DCO1_5MHz = 0x00000000,
  DCO3MHz = 0x00010000,
  DCO6MHz = 0x00020000,
  DCO12MHz = 0x00030000
};
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
// runs DCO at 1.5 MHz, oscillates P6.0 at 1kHz
int main1_5(void){
  Clock_Init(DCO1_5MHz);
  P6DIR |= 0x01;         // P6.0 is output
  TimerA0_Init(1500);    // TA0 interrupt every 1ms
  EnableInterrupts();    // I=0
  while(1){
//    WaitForInterrupt();  // wfi
  }
}
// runs DCO at 3 MHz, oscillates P6.0 at 1kHz
int main3(void){
  Clock_Init(DCO3MHz);
  P6DIR |= 0x01;         // P6.0 is output
  TimerA0_Init(3000);    // TA0 interrupt every 1ms
  EnableInterrupts();    // I=0
  while(1){
//    WaitForInterrupt();  // wfi
  }
}
// runs DCO at 6 MHz, oscillates P6.0 at 1kHz
int main6(void){
  Clock_Init(DCO6MHz);
  P6DIR |= 0x01;         // P6.0 is output
  TimerA0_Init(6000);    // TA0 interrupt every 1ms
  EnableInterrupts();    // I=0
  while(1){
  //  WaitForInterrupt();  // wfi
  }
}
// runs DCO at 12 MHz, oscillates P6.0 at 1kHz
int main12(void){
  Clock_Init(DCO12MHz);
  P6DIR |= 0x01;         // P6.0 is output
  TimerA0_Init(12000);   // TA0 interrupt every 1ms
  EnableInterrupts();    // I=0
  while(1){
    WaitForInterrupt();  // wfi
  }
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
// runs HFXT at 48 MHz, SMCLK at 12 MHz, oscillates P6.0 at 1kHz
int main48(void){
  Clock_Init48MHz();
  P6DIR |= 0x01;         // P6.0 is output
  TimerA0_Init(12000);   // TA0 interrupt every 1ms
  EnableInterrupts();    // I=0
  while(1){
    WaitForInterrupt();  // wfi
  }
}
/*
void error(void){
  volatile uint32_t i;
  P1DIR |= 0x01; // red LED
  while(1){
    P1OUT ^= BIT0;
    for(i=0;i<20000;i++);       // Blink LED forever
  }
}
*/

/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2013, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP432 CODE EXAMPLE DISCLAIMER
 *
 * MSP432 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//   MSP432P401 Demo - - RTC, LPM3.5, & alarm
//
//   Description: The RTC module is used to set the time, start RTC operation,
//   and read the time from the respective RTC registers. Software will set the
//   original time to 11:59:45 am on Friday October 7, 2011. Then the RTC will
//   be activated through software, and an alarm will be created for the next
//   minute (12:00:00 pm). The device will then enter LPM3.5 awaiting
//   the event interrupt. Upon being woken up by the event, the LED on the board
//   will be set.
//
//   NOTE: To ensure that LPM3.5 is entered properly, you would need to use an
//   external power supply.
//
//  //* An external watch crystal on XIN XOUT is required for ACLK *//
//   ACLK = 32.768kHz, MCLK = SMCLK = default DCO~1MHz
//
//                MSP432p401rpz
//
//             -----------------
//         /|\|              XIN|-
//          | |                 | 32kHz
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|--> LED
//
//   Dung Dang
//   Texas Instruments Inc.
//   Nov 2013
//   Built with Code Composer Studio V6.0
//******************************************************************************
