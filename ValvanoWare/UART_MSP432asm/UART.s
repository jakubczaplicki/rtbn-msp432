; UART.s
; Runs on MSP432
; Use eUSCI A0 to implement bidirectional data transfer to and from a
; computer running PuTTY.
; Daniel Valvano
; May 19, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015

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

; UCA0RXD (VCP receive) connected to P1.2
; UCA0TXD (VCP transmit) connected to P1.3

P1SEL0    EQU 0x40004C0A  ; Port 1 Select 0
P1SEL1    EQU 0x40004C0C  ; Port 1 Select 1
UCA0CTLW0 EQU 0x40001000  ; eUSCI_Ax Control Word Register 0
UCA0CTLW1 EQU 0x40001002  ; eUSCI_Ax Control Word Register 1
UCA0BRW   EQU 0x40001006  ; eUSCI_Ax Baud Rate Control Word Register
UCA0MCTLW EQU 0x40001008  ; eUSCI_Ax Modulation Control Word Register
UCA0RXBUF EQU 0x4000100C  ; eUSCI_Ax Receive Buffer Register
UCA0TXBUF EQU 0x4000100E  ; eUSCI_Ax Transmit Buffer Register
UCA0IE    EQU 0x4000101A  ; eUSCI_Ax Interrupt Enable Register
UCA0IFG   EQU 0x4000101C  ; eUSCI_Ax Interrupt Flag Register

; standard ASCII symbols
CR                 EQU 0x0D
LF                 EQU 0x0A
BS                 EQU 0x08
ESC                EQU 0x1B
SPA                EQU 0x20
DEL                EQU 0x7F

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT UART_Init
        EXPORT UART_InChar
        EXPORT UART_OutChar

; require C function calls to preserve the 8-byte alignment of 8-byte data objects
        PRESERVE8

;------------UART_Init------------
; Initialize eUSCI A0
; Baud rate is 115200 bits/sec
; Input: none
; Output: none
; Modifies: R0, R1
; Assumes: 3 MHz system clock
UART_Init
    PUSH {LR}                       ; save current value of LR
    ; hold the USCI module in reset mode
    LDR  R1, =UCA0CTLW0             ; R1 = &UCA0CTLW0 (pointer)
    MOV  R0, #0x0001                ; R0 = 0x0001
    STRH R0, [R1]
    ; configure eUSCI_A0 Control Word 0 Register as follows:
    ; bit15=0,      no parity bits
    ; bit14=x,      not used when parity is disabled
    ; bit13=0,      LSB first
    ; bit12=0,      8-bit data length
    ; bit11=0,      1 stop bit
    ; bits10-8=000, asynchronous UART mode
    ; bits7-6=11,   clock source to SMCLK
    ; bit5=0,       reject erroneous characters and do not set flag
    ; bit4=0,       do not set flag for break characters
    ; bit3=0,       not dormant
    ; bit2=0,       transmit data, not address (not used here)
    ; bit1=0,       do not transmit break (not used here)
    ; bit0=1,       hold logic in reset state while configuring
    MOV  R0, #0x00C1;               ; R0 = 0x00C1
    STRH R0, [R1]
    ; set the baud rate
    ; N = clock/baud rate = 3,000,000/115,200 = 26.0417
    ; UCBR = baud rate = int(N) = 26
    LDR  R1, =UCA0BRW               ; R1 = &UCA0BRW (pointer)
    MOV  R0, #26                    ; R0 = 26
    STRH R0, [R1]
    ; clear first and second modulation stage bit fields
    LDR  R1, =UCA0MCTLW             ; R1 = &UCA0MCTLW (pointer)
    LDRH R0, [R1]                   ; R0 = UCA0MCTLW (value)
    ORR  R0, R0, #0x000E            ; R0 = R0&~0xFFF1 = R0|0x000E
    STRH R0, [R1]
    ; configure second modulation stage select (from Table 22-4 on p731 of datasheet)
;    ORR  R0, R0, #(0<<8)            ; UCBRS = N - int(N) = 0.0417; plug this in Table 22-4
    ; configure first modulation stage select (ignored when oversampling disabled)
;    ORR  R0, R0, #(10<<4)           ; UCBRF = int(((N/16) - int(N/16))*16) = 10
    ; enable oversampling mode
;    ORR  R0, R0, #0x0001            ; R0 = UCA0MCTLW|0x0001
    STRH R0, [R1]
    ; initialize P1.3 and P1.2 and make them eUSCI A0 (P1.3 and P1.2 built-in VCP)
    ; configure P1.3 and P1.2 as primary module function
    LDR  R1, =P1SEL0                ; R1 = &P1SEL0 (pointer)
    LDRB R0, [R1]                   ; R0 = P1SEL0 (value)
    ORR  R0, R0, #0x0C              ; R0 = P1SEL0|0x0C (configure P1.3 and P1.2 as primary module function)
    STRB R0, [R1]
    LDR  R1, =P1SEL1                ; R1 = &P1SEL1 (pointer)
    LDRB R0, [R1]                   ; R0 = P1SEL1 (value)
    BIC  R0, R0, #0x0C              ; R0 = P1SEL1&~0x0C (configure P1.3 and P1.2 as primary module function)
    STRB R0, [R1]
    ; enable the eUSCI module
    LDR  R1, =UCA0CTLW0             ; R1 = &UCA0CTLW0 (pointer)
    LDRH R0, [R1]                   ; R0 = UCA0CTLW0 (value)
    BIC  R0, #0x0001                ; R0 = UCA0CTLW0&~0x0001
    STRH R0, [R1]
    ; disable interrupts (transmit ready, start received, transmit empty, receive full)
    LDR  R1, =UCA0IE                ; R1 = &UCA0IE (pointer)
    LDRH R0, [R1]                   ; R0 = UCA0IE (value)
    BIC  R0, R0, #0x000F            ; R0 = UCA0IE&~0x000F
    STRH R0, [R1]
    POP {PC}                        ; restore previous value of LR into PC (return)


;------------UART_InChar------------
; input ASCII character from UART
; spin if no data available
; Input: none
; Output: R0  character in from UART
UART_InChar
       LDR  R1, =UCA0IFG
InLoop LDRH R2, [R1]   ; read flag register
       ANDS R2, R2, #0x0001
       BEQ  InLoop     ; wait while UCRXIFG is 0
       LDR  R1, =UCA0RXBUF
       LDRH R0, [R1]   ; read receive data buffer
       BX   LR


;------------UART_OutChar------------
; output ASCII character to UART
; spin if UART transmit FIFO is full
; Input: R0  character out to UART
; Output: none
; Modifies: R0, R1
UART_OutChar
       LDR  R1, =UCA0IFG
OLoop  LDRH R2, [R1]   ; read FR
       ANDS R2, R2, #0x0002
       BEQ  OLoop      ; wait while UCTXIFG is 0
       LDR  R1, =UCA0TXBUF
       STRH R0, [R1]   ; write transmit data buffer
       BX   LR


    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
