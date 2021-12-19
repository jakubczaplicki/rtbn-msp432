; PointerTrafficLight.s
; Runs on MSP432
; Use a pointer implementation of a Moore finite state machine to operate
; a traffic light.
; Daniel Valvano
; May 10, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;  Volume 1, Program 6.8, Example 6.4
;
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

; east facing red light connected to P4.5
; east facing yellow light connected to P4.4
; east facing green light connected to P4.3
; north facing red light connected to P4.2
; north facing yellow light connected to P4.1
; north facing green light connected to P4.0
; north facing car detector connected to P5.1 (1=car present)
; east facing car detector connected to P5.0 (1=car present)

       .thumb

       .text
       .align  2
       .global  SysTick_Init
       .global  SysTick_Wait
       .global  SysTick_Wait10ms

P4IN     .field  0x40004C21,32  ; Port 4 Input
P4OUT    .field  0x40004C23,32  ; Port 4 Output
P4DIR    .field  0x40004C25,32  ; Port 4 Direction
P4REN    .field  0x40004C27,32  ; Port 4 Resistor Enable
P4SEL0   .field  0x40004C2B,32  ; Port 4 Select 0
P4SEL1   .field  0x40004C2D,32  ; Port 4 Select 1
P5IN     .field  0x40004C40,32  ; Port 5 Input
P5OUT    .field  0x40004C42,32  ; Port 5 Output
P5DIR    .field  0x40004C44,32  ; Port 5 Direction
P5REN    .field  0x40004C46,32  ; Port 5 Resistor Enable
P5SEL0   .field  0x40004C4A,32  ; Port 5 Select 0
P5SEL1   .field  0x40004C4C,32  ; Port 5 Select 1

        .global  main
;Linked data structure
;Put in ROM
OUT   .EQU 0    ;offset for output
WAIT  .EQU 4    ;offset for time
NEXT  .EQU 8    ;offset for next
goN   .long 0x21 ;North green, East red
      .long 300  ;3 sec
      .long goN,waitN,goN,waitN
waitN .long 0x22 ;North yellow, East red
      .long 50   ;0.5 sec
      .long goE,goE,goE,goE
goE   .long 0x0C ;North red, East green
      .long 300  ;3 sec
      .long goE,goE,waitE,waitE
waitE .long 0x14 ;North red, East yellow
      .long 50   ;0.5 sec
      .long goN,goN,goN,goN

goNAddr .field goN,32
      .thumbfunc main
main: .asmfunc
    BL   SysTick_Init   ; enable SysTick

    ; initialize P4.5-P4.0 and make them GPIO outputs
    LDR  R1, P4SEL0
    LDRB R0, [R1]             
    BIC  R0, R0, #0x3F    ; configure light pins as GPIO
    STRB R0, [R1]         
    LDR  R1, P4SEL1
    LDRB R0, [R1]      
    BIC  R0, R0, #0x3F   ; configure light pins as GPIO
    STRB R0, [R1]                   
    ; make light pins out
    LDR  R1, P4DIR
    LDRB R0, [R1]          
    ORR  R0, R0, #0x3F   ; output direction
    STRB R0, [R1]           

    ; initialize P5.1-P5.0 and make them inputs
    LDR  R1, P5SEL0
    LDRB R0, [R1]          
    BIC  R0, R0, #0x03   ; configure car detector pins as GPIO
    STRB R0, [R1]          
    LDR  R1, P5SEL1
    LDRB R0, [R1]          
    BIC  R0, R0, #0x03   ; configure car detector pins as GPIO)
    STRB R0, [R1]     
    ; make car detector pins in
    LDR  R1, P5DIR
    LDRB R0, [R1]    
    BIC  R0, R0, #0x03   ; input direction
    STRB R0, [R1]       

    LDR  R4, goNAddr       ; state pointer
    LDR  R5, P5IN      ; 0x40004C40 (8 bits)
    LDR  R6, P4OUT     ; 0x40004C23 (8 bits)
FSM LDRB R1, [R6]       ; R1 = [R6] = 8-bit contents of register P4OUT
    BIC  R1, R1, #0x3F  ; R1 = P4OUT&~0x3F (clear light output value field)
    LDRB R0, [R4, #OUT] ; R0 = R4->OUT = 8-bit output value for the current state
    ORR  R0, R1, R0     ; R0 = (P4OUT&~0x3F)|(R4->OUT)
    STRB R0, [R6]       ; [R6] = R0 (set lights to current state's OUT value)
    LDR  R0, [R4, #WAIT]; R0 = R4->WAIT = 32-bit time delay for the current state
    BL   SysTick_Wait10ms
    LDRB R0, [R5]       ; R0 = [R5] = 8-bit contents of register P5IN
    AND  R0, R0, #0x03  ; ignore bits of P5IN not connected to sensors
    LSL  R0, R0, #2     ; 4 bytes/address
    ADD  R0, R0, #NEXT  ; 8,12,16,20
    LDR  R4, [R4, R0]   ; R4 = R4->NEXT[P5IN&0x03] (go to next state based on inputs)
    B    FSM
    .endasmfunc

    .end

