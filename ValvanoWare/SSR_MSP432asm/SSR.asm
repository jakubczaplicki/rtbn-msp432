; SSR.s
; Runs on MSP432
; Provide functions that initialize a GPIO pin and turn it on and off.
; Daniel Valvano
; April 23, 2015

; This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;   Volume 1 Program 4.3, Figure 4.14

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

; solid state relay connected to P2.2

       .thumb

       .text
       .align  2
P1IN   .field 0x40004C00,32  ; Port 1 Input
P2IN   .field 0x40004C01,32  ; Port 2 Input
P2OUT  .field 0x40004C03,32  ; Port 2 Output
P1OUT  .field 0x40004C02,32  ; Port 1 Output
P1DIR  .field 0x40004C04,32  ; Port 1 Direction
P2DIR  .field 0x40004C05,32  ; Port 2 Direction
P1REN  .field 0x40004C06,32  ; Port 1 Resistor Enable
P2REN  .field 0x40004C07,32  ; Port 2 Resistor Enable
P1DS   .field 0x40004C08,32  ; Port 1 Drive Strength
P2DS   .field 0x40004C09,32  ; Port 2 Drive Strength
P1SEL0 .field 0x40004C0A,32  ; Port 1 Select 0
P2SEL0 .field 0x40004C0B,32  ; Port 2 Select 0
P1SEL1 .field 0x40004C0C,32  ; Port 1 Select 1
P2SEL1 .field 0x40004C0D,32  ; Port 2 Select 1

      .global main
      .thumbfunc main
; This is an extremely simple test program to demonstrate that the SSR
; can turn on and off.  Press and release left Button 1 to turn the
; SSR on, and press and release right Button 2 to turn the SSR off.
; built-in negative logic switches connected to P1.1 and P1.4
main: .asmfunc
    BL  SSR_Init                    ; initialize P2.2 and make it output
    ; initialize P1.1 and P1.4 and make them inputs (P1.1 and P1.4 built-in buttons)
    ; configure P1.4 and P1.1 as GPIO
    LDR  R1, P1SEL0
    LDRB R0, [R1]
    BIC  R0, R0, #0x12              ; configure P1.4 and P1.1 as GPIO
    STRB R0, [R1]
    LDR  R1, P1SEL1
    LDRB R0, [R1]
    BIC  R0, R0, #0x12              ; configure P1.4 and P1.1 as GPIO
    STRB R0, [R1]
    ; make P1.4 and P1.1 in
    LDR  R1, P1DIR
    LDRB R0, [R1]
    BIC  R0, R0, #0x12              ; input direction
    STRB R0, [R1]
    ; enable pull resistors on P1.4 and P1.1
    LDR  R1, P1REN
    LDRB R0, [R1]
    ORR  R0, R0, #0x12              ; enable pull resistors
    STRB R0, [R1]
    ; P1.4 and P1.1 are pull-up
    LDR  R1, P1OUT
    LDRB R0, [R1]
    ORR  R0, R0, #0x12              ; pull-up resistors
    STRB R0, [R1]
    LDR  R4, P1IN
loop
    BL   SSR_Off
waitforpress1                       ; proceed only when Button 1 is pressed
    LDRB R0, [R4]
    AND  R0, R0, #0x02              ; P1IN&0x02
    CMP  R0, #0x02                  ; 0x02?
    BEQ  waitforpress1              ; if so, spin
waitforrelease1                     ; proceed only when Button 1 is released
    LDRB R0, [R4]                   ;
    AND  R0, R0, #0x02              ; P1IN&0x02
    CMP  R0, #0x00                  ; 0x00?
    BEQ  waitforrelease1            ; if so, spin
    BL   SSR_On
waitforpress2                       ; proceed only when Button 2 is pressed
    LDRB R0, [R4]
    AND  R0, R0, #0x10              ; P1IN&0x10
    CMP  R0, #0x10                  ; 0x10?
    BEQ  waitforpress2              ; if so, spin
waitforrelease2                     ; proceed only when Button 2 is released
    LDRB R0, [R4]
    AND  R0, R0, #0x10              ; P1IN&0x10
    CMP  R0, #0x00                  ; 0x00?
    BEQ  waitforrelease2            ; if so, spin
    B    loop
    .endasmfunc
;------------SSR_Init------------
; Make P2.2 an output and ensure alt. functions off.
; Input: none
; Output: none
; Modifies: R0, R1
SSR_Init: .asmfunc
    ; initialize P2.2 and make it output
    LDR  R1, P2SEL0
    LDRB R0, [R1]
    BIC  R0, R0, #0x04              ; configure SSR pin as GPIO
    STRB R0, [R1]
    LDR  R1, P2SEL1
    LDRB R0, [R1]
    BIC  R0, R0, #0x04              ; configure SSR pin as GPIO
    STRB R0, [R1]
    ; make P2.2 high drive strength
    LDR  R1, P2DS
    LDRB R0, [R1]
    ORR  R0, R0, #0x04              ; configure SSR pin as high drive strength
    STRB R0, [R1]
    ; make SSR pin out
    LDR  R1, P2DIR
    LDRB R0, [R1]
    ORR  R0, R0, #0x04              ; output direction
    STRB R0, [R1]
    BX   LR
    .endasmfunc
;------------SSR_On------------
; Make P2.2 high.
; Input: none
; Output: none
; Modifies: R0, R1
SSR_On: .asmfunc
    LDR  R1, P2OUT
    LDRB R0, [R1]
    ORR  R0, R0, #0x04              ; turn on the appliance
    STRB R0, [R1]
    BX   LR
    .endasmfunc
;------------SSR_Off------------
; Make P2.2 low.
; Input: none
; Output: none
; Modifies: R0, R1
SSR_Off: .asmfunc
    LDR  R1, P2OUT
    LDRB R0, [R1]
    BIC  R0, R0, #0x04              ; turn off the appliance
    STRB R0, [R1]
    BX   LR
    .endasmfunc
;------------SSR_Toggle------------
; Toggle P2.2.
; Input: none
; Output: none
; Modifies: R0, R1
SSR_Toggle: .asmfunc
    LDR  R1, P2OUT
    LDRB R0, [R1]
    EOR  R0, R0, #0x04              ; toggle the appliance
    STRB R0, [R1]
    BX   LR

    .endasmfunc
    .end

