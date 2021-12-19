; SysTickTestMain.asm
; Runs on MSP432
; Test the SysTick functions by initializing the SysTick timer and
; flashing an LED at a constant rate.
; Daniel Valvano
; May 4, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;  Volume 1, Program 4.7
;
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

; built-in LED1 connected to P1.0
; negative logic built-in Button 1 connected to P1.1
; negative logic built-in Button 2 connected to P1.4
; positive logic switch connected to P1.5
; built-in red LED connected to P2.0
; built-in green LED connected to P2.1
; built-in blue LED connected to P2.2
       .thumb

       .text
       .align  2
       .global  SysTick_Init
       .global  SysTick_Wait
       .global  SysTick_Wait10ms

P2IN    .field 0x40004C01,32  ; Port 2 Input
P2OUT   .field 0x40004C03,32  ; Port 2 Output
P2DIR   .field 0x40004C05,32  ; Port 2 Direction
P2REN   .field 0x40004C07,32  ; Port 2 Resistor Enable
P2DS    .field 0x40004C09,32  ; Port 2 Drive Strength
P2SEL0  .field 0x40004C0B,32  ; Port 2 Select 0
P2SEL1  .field 0x40004C0D,32  ; Port 2 Select 1

        .global  main
      .thumbfunc main
main: .asmfunc
    BL   SysTick_Init               ; initialize SysTick timer
    ; initialize P2.2 and make it output (P2.2 built-in blue LED)
    ; configure built-in blue LED as GPIO
    LDR  R1, P2SEL0                 ; R1 = &P2SEL0 (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    BIC  R0, R0, #0x04              ; R0 = R0&~0x04 (configure built-in blue LED as GPIO)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    LDR  R1, P2SEL1                 ; R1 = &P2SEL1 (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    BIC  R0, R0, #0x04              ; R0 = R0&~0x04 (configure built-in blue LED as GPIO)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    ; make built-in blue LED high drive strength
    LDR  R1, P2DS                   ; R1 = &P2DS (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    ORR  R0, R0, #0x04              ; R0 = R0|0x04 (high drive strength)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    ; make built-in blue LED out
    LDR  R1, P2DIR                  ; R1 = &P2DIR (pointer)
    LDRB R0, [R1]                   ; R0 = [R1] = 8-bit contents of register
    ORR  R0, R0, #0x04              ; R0 = R0|0x04 (output direction)
    STRB R0, [R1]                   ; [R1] = R0 (store into 8-bit register)
    LDR  R4, P2OUT                  ; R4 = &P2OUT (pointer)
loop
    LDRB R5, [R4]                   ; R5 = [R4] = 8-bit contents of register
    EOR  R5, R5, #0x04              ; R5 = R5^0x04 (toggle P2.2)
    STRB R5, [R4]                   ; [R4] = R5 (store into 8-bit register)
;    MOV R0, #1                      ; approximately 27 us(*); crashes if Method #1 is used in SysTick.s; RVR of 0 never triggers interrupt or COUNTFLAG 
;    MOV R0, #2                      ; approximately 27 us(*)
;    MOV R0, #600                    ; approximately 0.23 ms(*)
;    BL  SysTick_Wait
;    MOV R0, #1                      ; approximately 10 ms(*)
    MOV R0, #50                      ; approximately 500 ms
    BL  SysTick_Wait10ms
    B   loop
    ; (*) Note: Durations are measurements taken with the PicoScope and include loop overhead.
    .endasmfunc

    .end
