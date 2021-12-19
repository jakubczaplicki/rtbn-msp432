; EngineControl.asm
; Runs on MSP432 on CCS
; Use a pointer implementation of a Mealy finite state machine to operate
; an engine with a control input, gas and brake outputs, and two states.
; Daniel Valvano
; May 11, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;  Volume 1, Program 6.9, Section 6.5.3
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

; control input connected to P4.0 (1=go)
; gas output connected to P4.1
; brake output connected to P4.2
       .thumb

       .text
       .align  2
       .global  SysTick_Init
       .global  SysTick_Wait
       .global  SysTick_Wait10ms

P4IN   .field 0x40004C21,32  ; Port 4 Input
P4OUT  .field 0x40004C23,32  ; Port 4 Output
P4DIR  .field 0x40004C25,32  ; Port 4 Direction
P4REN  .field 0x40004C27,32  ; Port 4 Resistor Enable
P4SEL0 .field 0x40004C2B,32  ; Port 4 Select 0
P4SEL1 .field 0x40004C2D,32  ; Port 4 Select 1


        .global  main
;Linked data structure
;Put in ROM
OUT   .EQU 0    ;offset for output
DELAY .EQU 8    ;offset for delay between states
NEXT  .EQU 12   ;offset for next
Stop  .long 2,0     ;Outputs for 0,1
      .long 10      ;100 ms
      .long Stop,Go ;Next for 0,1
Go    .long 0,1     ;Outputs for 0,1
      .long 10      ;100 ms
      .long Stop,Go ;Next for 0,1
StopAddr .field Stop,32
      .thumbfunc main
main: .asmfunc
    BL   SysTick_Init   ; enable SysTick

    ; initialize P4.2-P4.0 GPIO
    LDR  R1, P4SEL0
    LDRB R0, [R1]     
    BIC  R0, R0, #0x07    ; GPIO
    STRB R0, [R1]        
    LDR  R1, P4SEL1
    LDRB R0, [R1]          
    BIC  R0, R0, #0x07    ; GPIO
    STRB R0, [R1]                
    ; set direction register
    LDR  R1, P4DIR
    LDRB R0, [R1]        
    ORR  R0, R0, #0x06   ; Gas, Brake out
    BIC  R0, R0, #0x01     
    STRB R0, [R1]               
    LDR  R4, StopAddr   ; state pointer
    LDR  R5, P4IN      ; 0x40004C21
    LDR  R6, P4OUT     ; 0x40004C23
FSM LDRB R3, [R5]       ; read P4IN
    AND  R3, R3, #0x01  ; sensor
    LSL  R3, R3, #2     ; 4 bytes each
    ADD  R1, R3, #OUT   ; R1 is 0 or 4
    LDRB R2, [R4, R1]   ; output value 
    LSL  R2, R2, #1     ; shift into bits 2,1
    LDRB R1, [R6]       ; read P4OUT
    BIC  R1, R1, #0x06  ; friendly
    ORR  R2, R1, R2     ; output
    STRB R2, [R6]      
    LDR  R0, [R4, #DELAY] ; 10ms 
    ADD  R1, R3, #NEXT  ; 12 or 16
    LDR  R4, [R4, R1]   ; next 
    BL   SysTick_Wait10ms
    B    FSM            
    .endasmfunc

    .end
