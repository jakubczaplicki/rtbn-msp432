; Switch.s
; Runs on MSP432
; Provide functions that initialize a GPIO as an input pin and
; allow reading of two negative logic switches on P1.1 and P1.4
; and an external switch on P1.5.
; Daniel and Jonathan Valvano
; April 22, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;  Section 4.2.2, Program 4.2, Figure 4.7

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

P1IN      EQU 0x40004C00  ; Port 1 Input
P1OUT     EQU 0x40004C02  ; Port 1 Output
P1DIR     EQU 0x40004C04  ; Port 1 Direction
P1REN     EQU 0x40004C06  ; Port 1 Resistor Enable
P1SEL0    EQU 0x40004C0A  ; Port 1 Select 0
P1SEL1    EQU 0x40004C0C  ; Port 1 Select 1

SW1       EQU 0x02                 ; on the left side of the LaunchPad board
SW2       EQU 0x10                 ; on the right side of the LaunchPad board
SWEXT     EQU 0x20                 ; external switch

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Switch_Init
        EXPORT  Switch_Input
        EXPORT  Board_Init
        EXPORT  Board_Input
;------------Switch_Init------------
; Initialize GPIO Port 1 bit 5 for input.  An external pull-down
; resistor is used.
; Input: none
; Output: none
; Modifies: R0, R1
Switch_Init
    ; configure P1.5 as GPIO
    LDR  R1, =P1SEL0
    LDRB R0, [R1]
    BIC  R0, R0, #0x20              ; configure P1.5 as GPIO
    STRB R0, [R1]
    LDR  R1, =P1SEL1
    LDRB R0, [R1]
    BIC  R0, R0, #0x20              ; configure P1.5 as GPIO
    STRB R0, [R1]
    ; make P1.5 in
    LDR  R1, =P1DIR
    LDRB R0, [R1]
    BIC  R0, R0, #0x20              ; input direction
    STRB R0, [R1]
    ; disable pull resistor on P1.5
    LDR  R1, =P1REN
    LDRB R0, [R1]
    BIC  R0, R0, #0x20              ; disable pull resistor
    STRB R0, [R1]
    BX   LR

;------------Switch_Input------------
; Read and return the status of GPIO Port 1 bit 5.
; Input: none
; Output: R0  0x20 if P1.5 high
;         R0  0x00 if P1.5 low
; Modifies: R1
Switch_Input
    LDR  R1, =P1IN
    LDRB R0, [R1]                   ; 8-bit contents of register
    AND  R0, R0, #0x20              ; get just P1.5
    BX   LR                         ; return 0x20 or 0x00

;------------Board_Init------------
; Initialize GPIO Port 1 for negative logic switches on P1.1 and
; P1.4 as the LaunchPad is wired.  Weak internal pull-up
; resistors are enabled.
; Input: none
; Output: none
; Modifies: R0, R1
Board_Init
    ; configure P1.4 and P1.1 as GPIO
    LDR  R1, =P1SEL0
    LDRB R0, [R1]
    BIC  R0, R0, #0x12              ; configure P1.4 and P1.1 as GPIO
    STRB R0, [R1]
    LDR  R1, =P1SEL1
    LDRB R0, [R1]
    BIC  R0, R0, #0x12              ; configure P1.4 and P1.1 as GPIO
    STRB R0, [R1]
    ; make P1.4 and P1.1 in
    LDR  R1, =P1DIR
    LDRB R0, [R1]
    BIC  R0, R0, #0x12              ; input direction
    STRB R0, [R1]
    ; enable pull resistors on P1.4 and P1.1
    LDR  R1, =P1REN
    LDRB R0, [R1]
    ORR  R0, R0, #0x12              ; enable pull resistors
    STRB R0, [R1]
    ; P1.4 and P1.1 are pull-up
    LDR  R1, =P1OUT
    LDRB R0, [R1]
    ORR  R0, R0, #0x12              ; pull-up resistors
    STRB R0, [R1]
    BX  LR

;------------Board_Input------------
; Read and return the status of the switches.
; Input: none
; Output: 0x10 if only Switch 1 is pressed
;         0x02 if only Switch 2 is pressed
;         0x00 if both switches are pressed
;         0x12 if no switches are pressed
; Modifies: R1
Board_Input
    LDR  R1, =P1IN
    LDRB R0, [R1]                   ; 8-bit contents of register
    AND  R0, R0, #0x12              ; get just input pins P1.4 and P1.1
    BX   LR                         ; return R0 with inputs

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
