; ClockSystemTestMain.s
; Runs on MSP432
; Test the Clock System initialization to verify that the
; system clock is running at the expected rate.  Use the
; debugger if possible or an oscilloscope connected to P2.2.
; When using an oscilloscope to look at LED2, it should be
; clear to see that the LED flashes about 4 (12/3) times
; faster with a 12 MHz clock than with the default 3 MHz
; clock.
; The operation of the Clock System can be tested even more
; precisely by using an oscilloscope to measure P4.3, which
; is configured to output the main clock signal.  The main
; clock is used by the CPU and peripheral module interfaces
; and can be used directly by some peripheral modules.  In
; this case and by default, this is the DCO frequency
; divided by 1.  P4.2 is configured to output the auxiliary
; clock signal.  The auxiliary clock is used by individual
; peripheral modules that select it.  In this case, this is
; the internal, low-power low-frequency oscillator REFOCLK,
; which is 32,768 Hz in this case and by default.  REFOCLK
; can be 128,000 Hz by setting bit 15 of CSCLKEN register.
; In this case and by default, the auxiliary clock is
; divided by 1.  P4.4 is configured to output the subsystem
; master clock signal.  The subsystem master clock (HSMCLK)
; and low-speed subsystem master clock (SMCLK) get their
; clocks from the same source.  By default, this is the DCO
; frequency divided by 1 for both (although both the HSMCLK
; and SMCLK can be programmed with different dividers).
; Both subsystem master clocks are used by individual
; peripheral modules that select them.
; Daniel Valvano
; June 30, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;  Program 2.10

;Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
;   You may use, edit, run or distribute this file
;   as long as the above copyright notice remains
;THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
;OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
;MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
;VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
;OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;For more information about my classes, my research, and my books, see
;http://users.ece.utexas.edu/~valvano/

; auxiliary clock output connected to P4.2
; main clock output connected to P4.3
; subsystem master clock output connected to P4.4

        IMPORT  Clock_Init
        IMPORT  Clock_Init48MHz
        IMPORT  Clock_Init32kHz
        IMPORT  Clock_InitLowPower

P2OUT     EQU 0x40004C03  ; Port 2 Output
P2DIR     EQU 0x40004C05  ; Port 2 Direction
P2REN     EQU 0x40004C07  ; Port 2 Resistor Enable
P2DS      EQU 0x40004C09  ; Port 2 Drive Strength
P2SEL0    EQU 0x40004C0B  ; Port 2 Select 0
P2SEL1    EQU 0x40004C0D  ; Port 2 Select 1
P4DIR     EQU 0x40004C25  ; Port 4 Direction
P4SEL0    EQU 0x40004C2B  ; Port 4 Select 0
P4SEL1    EQU 0x40004C2D  ; Port 4 Select 1
DCO1_5MHz EQU 0x00000000  ; R0 parameter for ClockInit() for 1.5 MHz DCO
DCO3MHz   EQU 0x00010000  ; R0 parameter for ClockInit() for 3 MHz DCO
DCO6MHz   EQU 0x00020000  ; R0 parameter for ClockInit() for 6 MHz DCO
DCO12MHz  EQU 0x00030000  ; R0 parameter for ClockInit() for 12 MHz DCO
SCALE     EQU 16
; test
; DCO1_5MHz  SCALE=1/2 blue LED flashs 1Hz
; DCO3MHz    SCALE=1   blue LED flashs 1Hz
; DCO6MHz    SCALE=2   blue LED flashs 1Hz
; DCO12MHz   SCALE=4   blue LED flashs 1Hz
; Init48MHz  SCALE=16  blue LED flashs 1Hz

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start

; delay function for testing
; which delays about 8.1*R0 cycles
Delay
    SUBS R0, R0, #1
    BNE  Delay
    BX   LR

Start
    MOV  R0, #DCO12MHz              ; R0 = DCO12MHz
;    BL   Clock_Init                 ; configure for 12 MHz clock
;    BL   Clock_Init48MHz            ; configure for 48 MHz clock
;    BL   Clock_Init32kHz            ; configure for 32 kHz clock
    BL   Clock_InitLowPower         ; configure for 32 kHz clock
    ; initialize P2.2 and make it output (P2.2 built-in blue LED)
    ; configure built-in blue LED as GPIO
    LDR  R1, =P2SEL0                ; R1 = &P2SEL0 (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    BIC  R0, R0, #0x04              ; R0 = R0&~0x04 (configure built-in blue LED as GPIO)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    LDR  R1, =P2SEL1                ; R1 = &P2SEL1 (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    BIC  R0, R0, #0x04              ; R0 = R0&~0x04 (configure built-in blue LED as GPIO)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    ; make built-in blue LED high drive strength
    LDR  R1, =P2DS                  ; R1 = &P2DS (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    ORR  R0, R0, #0x04              ; R0 = R0|0x04 (high drive strength)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    ; make built-in blue LED out
    LDR  R1, =P2DIR                 ; R1 = &P2DIR (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    ORR  R0, R0, #0x04              ; R0 = R0|0x04 (output direction)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    ; initialize P4.4-P4.2 and make them clock outputs
    ; configure P4.4-P4.2 as primary module function
    LDR  R1, =P4SEL0                ; R1 = &P4SEL0 (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    ORR  R0, R0, #0x1C              ; R0 = R0|0x1C (configure P4.4-P4.2 as primary module function)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    LDR  R1, =P4SEL1                ; R1 = &P4SEL1 (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    BIC  R0, R0, #0x1C              ; R0 = R0&~0x1C (configure P4.4-P4.2 as primary module function)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    ; make P4.4-P4.2 out
    LDR  R1, =P4DIR                 ; R1 = &P4DIR (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    ORR  R0, R0, #0x1C              ; R0 = R0|0x1C (output direction)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    LDR  R4, =P2OUT                 ; R4 = &P2OUT (pointer)
loop
    ; turn on LED2 (blue)
    LDRB R0, [R4]                   ; R0 = [R4] = 8-bit contents of register
    ORR  R0, R0, #0x04              ; R0 = R0|0x04 (turn on LED2 (blue))
    STRB R0, [R4]                   ; [R4] = R0 (store into 8-bit register)
    ; delay
;    LDR  R0, =419                   ; R0 = 419 (delay ~1.136 ms at 3 MHz)
;    LDR  R0, =(184543*SCALE)        ; R0 = 184543*SCALE (delay ~0.5 sec at 3 MHz; SCALE = 1)
    LDR  R0, =2016                  ; R0 = 2016 (delay ~0.5 sec at 32,768 Hz)
    BL   Delay                      ; delay about 8.1*R0 cycles
    ; turn off LED2 (blue)
    LDRB R0, [R4]                   ; R0 = [R4] = 8-bit contents of register
    BIC  R0, R0, #0x04              ; R0 = R0&~0x04 (turn off LED2 (blue))
    STRB R0, [R4]                   ; [R4] = R0 (store into 8-bit register)
    ; delay
;    LDR  R0, =419                   ; R0 = 419 (delay ~1.136 ms at 3 MHz)
;    LDR  R0, =(184543*SCALE)        ; R0 = 184543*SCALE (delay ~0.5 sec at 3 MHz; SCALE = 1)
    LDR  R0, =2016                  ; R0 = 2016 (delay ~0.5 sec at 32,768 Hz)
    BL   Delay                      ; delay about 8.1*R0 cycles
    B    loop                       ; unconditional branch to 'loop'

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
