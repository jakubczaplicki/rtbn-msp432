; GPIO.asm
; Runs on MSP432
; Initialize four GPIO pins as outputs.  Continually generate output to
; drive simulated stepper motor.
; Daniel Valvano
; June 20, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;  Volume 1 Program 4.5

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

; P4.3 is an output to LED3, negative logic
; P4.2 is an output to LED2, negative logic
; P4.1 is an output to LED1, negative logic
; P4.0 is an output to LED0, negative logic


       .thumb             
   
       .text             
       .align  2       

P4IN   .field 0x40004C21,32  ; Port 4 Input
P4OUT  .field 0x40004C23,32  ; Port 4 Output
P4DIR  .field 0x40004C25,32  ; Port 4 Direction
P4REN  .field 0x40004C27,32  ; Port 4 Resistor Enable
P4SEL0 .field 0x40004C2B,32  ; Port 4 Select 0
P4SEL1 .field 0x40004C2D,32  ; Port 4 Select 1
        
      .global main    
      .thumbfunc main  
       

GPIO_Init: .asmfunc
    ; initialize P4.3-P4.0 and make them outputs
    LDR  R1, P4SEL0
    LDRB R0, [R1]
    BIC  R0, R0, #0x0F              ; configure stepper motor/LED pins as GPIO
    STRB R0, [R1]
    LDR  R1, P4SEL1
    LDRB R0, [R1]
    BIC  R0, R0, #0x0F              ; configure stepper motor/LED pins as GPIO
    STRB R0, [R1]
    ; make stepper motor/LED pins out
    LDR  R1, P4DIR
    LDRB R0, [R1]
    ORR  R0, R0, #0x0F              ; output direction
    STRB R0, [R1]
    BX   LR
    .endasmfunc   
main: .asmfunc 
    BL   GPIO_Init
    LDR  R1, P4OUT                  ; R0 = LEDS
loop
    ; first output: 1010, LED is 0101
    LDRB R0, [R1]
    BIC  R0, R0, #0x0F
    ORR  R0, R0, #10                ; output value = 10
    STRB R0, [R1]
    ; second output: 1001, LED is 0110
    LDRB R0, [R1]
    BIC  R0, R0, #0x0F
    ORR  R0, R0, #9                 ; output value = 9
    STRB R0, [R1]
    ; third output: 0101, LED is 1010
    LDRB R0, [R1]
    BIC  R0, R0, #0x0F
    ORR  R0, R0, #5                 ; output value = 5
    STRB R0, [R1]
    ; fourth output: 0110, LED is 1001
    LDRB R0, [R1]
    BIC  R0, R0, #0x0F
    ORR  R0, R0, #6                 ; output value = 6
    STRB R0, [R1]
    B    loop
    .endasmfunc     
    .end           
