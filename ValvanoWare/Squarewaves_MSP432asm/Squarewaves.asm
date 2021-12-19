; Squarewaves.s
; Runs on MSP432
; Initialize P2.1 and P2.2 as outputs with different initial values,
; then toggle them to produce two out of phase square waves.
; Daniel Valvano
; April 23, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;  Program 4.4
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
; built-in red LED connected to P2.0
; built-in green LED connected to P2.1
; built-in blue LED connected to P2.2

       .thumb

       .text
       .align  2
P2IN   .field 0x40004C01,32  ; Port 2 Input
P2OUT  .field 0x40004C03,32  ; Port 2 Output
P2DIR  .field 0x40004C05,32  ; Port 2 Direction
P2REN  .field 0x40004C07,32  ; Port 2 Resistor Enable
P2DS   .field 0x40004C09,32  ; Port 2 Drive Strength
P2SEL0 .field 0x40004C0B,32  ; Port 2 Select 0
P2SEL1 .field 0x40004C0D,32  ; Port 2 Select 1


      .global main
      .thumbfunc main
main: .asmfunc
    ; initialize P2.1 and P2.2 and make them outputs (P2.1 and P2.2 built-in RGB LEDs)
    ; configure built-in RGB LEDs as GPIO
    LDR  R1, P2SEL0
    LDRB R0, [R1]
    BIC  R0, R0, #0x06              ;configure built-in RGB LEDs as GPIO
    STRB R0, [R1]
    LDR  R1, P2SEL1
    LDRB R0, [R1]
    BIC  R0, R0, #0x06              ; configure built-in RGB LEDs as GPIO
    STRB R0, [R1]
    ; make built-in RGB LEDs high drive strength
    LDR  R1, P2DS
    LDRB R0, [R1]
    ORR  R0, #0x06                  ; high drive strength
    STRB R0, [R1]
    ; make built-in RGB LEDs out
    LDR  R1, P2DIR
    LDRB R0, [R1]
    ORR  R0, R0, #0x06              ; output direction
    STRB R0, [R1]
    ; P2.1 on (green LED on) and P2.2 off (blue LED off)
    LDR  R1, P2OUT
    LDRB R0, [R1]
    ORR  R0, R0, #0x02              ; P2OUT = P2OUT|0x02; P2.1 on
    BIC  R0, R0, #0x04              ; P2OUT = P2OUT&~0x04; P2.2 off
loop
    STRB R0, [R1]
    EOR  R0, R0, #0x06              ; P2OUT = P2OUT^0x06; toggle P2.1 and P2.2)
    B    loop

    .endasmfunc
    .end

