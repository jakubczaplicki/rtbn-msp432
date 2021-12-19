; GPIO.s
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

P4IN      EQU 0x40004C21  ; Port 4 Input
P4OUT     EQU 0x40004C23  ; Port 4 Output
P4DIR     EQU 0x40004C25  ; Port 4 Direction
P4REN     EQU 0x40004C27  ; Port 4 Resistor Enable
P4SEL0    EQU 0x40004C2B  ; Port 4 Select 0
P4SEL1    EQU 0x40004C2D  ; Port 4 Select 1

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
GPIO_Init
    ; initialize P4.3-P4.0 and make them outputs
    LDR  R1, =P4SEL0               
    LDRB R0, [R1]                 
    BIC  R0, R0, #0x0F              ; configure stepper motor/LED pins as GPIO
    STRB R0, [R1]                  
    LDR  R1, =P4SEL1              
    LDRB R0, [R1]                 
    BIC  R0, R0, #0x0F              ; configure stepper motor/LED pins as GPIO
    STRB R0, [R1]                 
    ; make stepper motor/LED pins out
    LDR  R1, =P4DIR                
    LDRB R0, [R1]                
    ORR  R0, R0, #0x0F              ; output direction
    STRB R0, [R1]                
    BX   LR                       

Start
    BL   GPIO_Init
    LDR  R1, =P4OUT                 ; R0 = LEDS
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

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file