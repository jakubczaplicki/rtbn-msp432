; SysTick.s
; Runs on MSP432
; Provide functions that initialize the SysTick module, wait at least a
; designated number of clock cycles, and wait approximately a multiple
; of 10 milliseconds using busy wait.  After a power-on-reset, the
; MSP432 gets its clock from the internal digitally controlled
; oscillator, which is set to 3 MHz by default.  One distinct advantage
; of the MSP432 is that it has low-power clock options to reduce power
; consumption by reducing clock frequency.  This matters for the
; function SysTick_Wait10ms(), which will wait longer than 10 ms if the
; clock is slower.
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

SYSTICK_STCSR         EQU 0xE000E010  ; SysTick Control and Status Register
SYSTICK_STRVR         EQU 0xE000E014  ; SysTick Reload Value Register
SYSTICK_STCVR         EQU 0xE000E018  ; SysTick Current Value Register

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT   SysTick_Init
        EXPORT   SysTick_Wait
        EXPORT   SysTick_Wait10ms

;------------SysTick_Init------------
; Initialize SysTick with busy wait running at bus clock.
; Input: none
; Output: none
; Modifies: R0, R1
SysTick_Init
    ; disable SysTick during setup
    LDR  R1, =SYSTICK_STCSR         ; R1 = &SYSTICK_STCSR (pointer)
    MOV  R0, #0                     ; ENABLE = 0; TICKINT = 0
    STR  R0, [R1]
    ; maximum reload value
    LDR  R1, =SYSTICK_STRVR         ; R1 = &SYSTICK_STRVR (pointer)
    LDR  R0, =0x00FFFFFF            ; R0 = 0x00FFFFFF = 2^24-1
    STR  R0, [R1]
    ; any write to current clears it
    LDR  R1, =SYSTICK_STCVR         ; R1 = &SYSTICK_STCVR (pointer)
    MOV  R0, #0                     ; R0 = 0
    STR  R0, [R1]
    ; enable SysTick with no interrupts
    LDR  R1, =SYSTICK_STCSR         ; R1 = &SYSTICK_STCSR (pointer)
    MOV  R0, #0x00000005            ; R0 = ENABLE and CLKSOURCE bits set; TICKINT bit cleared
    STR  R0, [R1]
    BX   LR                         ; return

;------------SysTick_Wait------------
; Time delay using busy wait.
; Input: R0  delay parameter in units of the core clock (units of 333 nsec for 3 MHz clock)
; Output: none
; Modifies: R0, R1, R2, R3
SysTick_Wait
; method #1: set Reload Value Register, clear Current Value Register, poll COUNTFLAG in Control and Status Register
    LDR  R1, =SYSTICK_STRVR         ; R1 = &SYSTICK_STRVR (pointer)
    SUB  R0, R0, #1                 ; subtract 1 because SysTick counts from STRVR to 0
    STR  R0, [R1]                   ; [R1] = number of counts to wait
    LDR  R1, =SYSTICK_STCVR         ; R1 = &SYSTICK_STCVR (pointer)
    MOV  R2, #0                     ; any write to CVR clears it and COUNTFLAG in CSR
    STR  R0, [R1]                   ; [R1] = 0
    LDR  R1, =SYSTICK_STCSR         ; R1 = &SYSTICK_STCSR (pointer)
SysTick_Wait_loop
    LDR  R3, [R1]                   ; R3 = SYSTICK_STCSR (value)
    ANDS R3, R3, #0x00010000        ; is COUNTFLAG set?
    BEQ  SysTick_Wait_loop          ; if not, keep polling
; method #2: repeatedly evaluate elapsed time
;    LDR  R1, =SYSTICK_STCVR         ; R1 = &SYSTICK_STCVR (pointer)
;    LDR  R2, [R1]                   ; R2 = startTime = SYSTICK_STCVR (value)
;SysTick_Wait_loop
;    LDR  R3, [R1]                   ; R3 = currentTime = SYSTICK_STCVR (value)
;    SUB  R3, R2, R3                 ; R3 = elapsedTime = (startTime - currentTime)
;    ; handle case where currentTime wraps around and is greater than startTime
;    BIC  R3, R3, #0xFF000000        ; R3 = elapsedTime = (startTime - currentTime)&0x00FFFFFF
;    CMP  R3, R0                     ; is R3 (elapsedTime) <= R0 (delay)?
;    BLS  SysTick_Wait_loop          ; if so, keep polling
    BX   LR                         ; return

;------------SysTick_Wait10ms------------
; Time delay using busy wait.  This assumes 3 MHz system clock.
; Input: R0  number of times to wait 10 ms before returning
; Output: none
; Modifies: R0
DELAY10MS             EQU 30000     ; clock cycles in 10 ms (assumes 3 MHz clock)
SysTick_Wait10ms
    PUSH {R4, LR}                   ; save current value of R4 and LR
    MOVS R4, R0                     ; R4 = R0 = remainingWaits
    BEQ  SysTick_Wait10ms_done      ; R4 == 0, done
SysTick_Wait10ms_loop
    LDR  R0, =DELAY10MS             ; R0 = DELAY10MS
    BL   SysTick_Wait               ; wait 10 ms
    SUBS R4, R4, #1                 ; R4 = R4 - 1; remainingWaits--
    BHI  SysTick_Wait10ms_loop      ; if(R4 > 0), wait another 10 ms
SysTick_Wait10ms_done
    POP  {R4, LR}                   ; restore previous value of R4 and LR
    BX   LR                         ; return

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
