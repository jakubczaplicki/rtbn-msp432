; ClockSystem.s
; Runs on MSP432
; Change the clock frequency using the Clock System module.
; Daniel Valvano
; June 30, 2015

;  This example accompanies the books
;  "Embedded Systems: Introduction to the MSP432 Microcontroller",
;  ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2015
;  Program 4.6

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

CSKEY     EQU 0x40010400  ; Key Register
CSCTL0    EQU 0x40010404  ; Control 0 Register
CSCTL1    EQU 0x40010408  ; Control 1 Register
CSCTL2    EQU 0x4001040C  ; Control 2 Register
CSIFG     EQU 0x40010448  ; Interrupt Flag Register
CSCLRIFG  EQU 0x40010450  ; Clear Interrupt Flag Register
PJSEL0    EQU 0x40004D2A  ; Port J Select 0
PJSEL1    EQU 0x40004D2C  ; Port J Select 1
PCMCTL0   EQU 0x40010000  ; Control 0 Register
PCMCTL1   EQU 0x40010004  ; Control 1 Register
PCMIFG    EQU 0x4001000C  ; Interrupt Flag Register
PCMCLRIFG EQU 0x40010010  ; Clear Interrupt Flag Register
DCO1_5MHz EQU 0x00000000
DCO3MHz   EQU 0x00010000
DCO6MHz   EQU 0x00020000
DCO12MHz  EQU 0x00030000

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Clock_Init
        EXPORT  Clock_Init48MHz
        EXPORT  Clock_Init32kHz
        EXPORT  Clock_InitLowPower
;------------Clock_Init------------
; Configure for SMCLK = MCLK = (R0 parameter), ACLK = REFOCLK.
; Input: encode R0 as follows for:
;        1.5 MHz: R0 = 0x00000000
;          3 MHz: R0 = 0x00010000
;          6 MHz: R0 = 0x00020000
;         12 MHz: R0 = 0x00030000
; Output: none
; Modifies: R0, R1, R2
Clock_Init
    ; check input parameter R0
    CMP  R0, #DCO1_5MHz             ; R0 == DCO1_5MHz?
    BEQ  clockInitContinue          ; if so, continue
    CMP  R0, #DCO3MHz               ; R0 == DCO3MHz?
    BEQ  clockInitContinue          ; if so, continue
    CMP  R0, #DCO6MHz               ; R0 == DCO6MHz?
    BEQ  clockInitContinue          ; if so, continue
    CMP  R0, #DCO12MHz              ; R0 == DCO12MHz?
    BEQ  clockInitContinue          ; if so, continue
clockInitBadInput
    BX   LR                         ; return
clockInitContinue
    ; unlock CS module for register access
    LDR  R1, =CSKEY
    LDR  R2, =0x695A
    STR  R2, [R1]                   ; CSKEY = 0x0000695A
    ; reset tuning parameters
    LDR  R1, =CSCTL0
    MOV  R2, #0
    ; configure for nominal DCO frequency = 3 MHz (default)
    ORR  R2, R2, #0x00010000        ; CSCTL0 = 0x00010000
    STR  R2, [R1]                   ; DCORSEL = 1
    ; enable DCO external resistor mode (must be done with DCORSEL == 1)
    ORR  R2, R2, #0x00400000        ; CSCTL0 |= 0x00400000 (DCORES = 1)
    STR  R2, [R1]                   ; DCORES = 1
    ; configure for nominal DCO frequency = 1.5,3,6,12,24 or 48 MHz
    BIC  R2, R2, #0x00070000        ; R2 = R2&~0x00070000 (clear DCORSEL bit field)
    ORR  R2, R2, R0                 ; bits 18,17,16
    STR  R2, [R1]
    ; set subsystem clock sources and dividers
    LDR  R1, =CSCTL1
    ; configure for all clock dividers equal to 1
    ; configure for ACLK sourced from REFOCLK
    ; configure for SMCLK and HSMCLK sourced from DCO
    ; configure for MCLK sourced from DCO
    LDR  R2, =0x00000233
    STR  R2, [R1]                   ; CSCTL1 = 0x00000233
    ; lock CS module from unintended access
    LDR  R1, =CSKEY
    MOV  R2, #0
    STR  R2, [R1]                   ; CSKEY = 0
    BX   LR

;------------Clock_Init48MHz------------
; Configure for MCLK = HFXTCLK, HSMCLK = HFXTCLK/2,
; SMCLK = HFXTCLK/4, ACLK = REFOCLK.
; On the LaunchPad, the high-frequency crystal
; oscillator has a 48 MHz crystal attached, which will
; make the bus (master) clock run at 48 MHz.  The sub-
; system master clock (HSMCLK) runs at its maximum of
; 24 MHz.  The low-speed subsystem master clock (SMCLK)
; runs at its maximum of 12 MHz.  In other words, this
; function is similar to Clock_Init(), except instead
; of the variable frequency DCO this uses the fixed
; high-frequency crystal.
; Input: none
; Output: none
; Modifies: R0, R1, R2, R3
Clock_Init48MHz
    ; wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    MOV  R2, #0                     ; R2 is Prewait counter
    LDR  R3, =100000                ; R3 is Prewait counter timeout value
    LDR  R1, =PCMCTL1
clockInit48PrewaitLoop
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000100        ; look at PMR_BUSY bit of PCMCTL1
    BEQ  clockInit48cont0           ; if cleared, continue
    ADD  R2, R2, #1                 ; increment Prewait counter
    CMP  R2, R3                     ; is Prewait (R2) < timeout (R3)?
    BLO  clockInit48PrewaitLoop     ; if so, continue looping
clockInit48PrewaitTimeout
    BX   LR
clockInit48cont0
    ; request power active mode LDO VCORE1 to support the 48 MHz frequency
    LDR  R1, =PCMCTL0
    LDR  R0, [R1]
    LDR  R2, =0xFFFF000F            ; PCMKEY bit field and AMR bit field
    BIC  R0, R0, R2                 ; clear PCMKEY bit field and AMR bit field
    LDR  R2, =0x695A0000            ; PCM key value
    ORR  R0, R0, R2                 ; write the proper PCM key to unlock write access
    ORR  R0, R0, #0x00000001        ; request power active mode LDO VCORE1
    STR  R0, [R1]
    ; check if the transition is invalid (see Figure 7-3 on p344 of datasheet)
    LDR  R1, =PCMIFG
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000004        ; look at AM_INVALID_TR_IFG bit of PCMIFG
    BEQ  clockInit48cont1           ; if cleared, continue
    ; bit 2 set on active mode transition invalid; bits 1-0 are for LPM-related errors; bit 6 is for DC-DC-related error
    ; clear the transition invalid flag
    LDR  R1, =PCMCLRIFG
    MOV  R0, #0x00000004            ; write 1 to clear
    STR  R0, [R1]
    ; to do: look at CPM bit field in PCMCTL0, figure out what mode you're in, and step through the chart to transition to the mode you want
    ; or be lazy and do nothing; this should work out of reset at least, but it WILL NOT work if Clock_Int32kHz() has been called
    BX   LR
clockInit48cont1
    ; wait for the CPM (Current Power Mode) bit field to reflect a change to active mode LDO VCORE1
    MOV  R2, #0                     ; R2 is CPMwait counter
    LDR  R3, =500000                ; R3 is CPMwait counter timeout value
    LDR  R1, =PCMCTL0
clockInit48CPMwaitLoop
    LDR  R0, [R1]
    AND  R0, R0, #0x00003F00        ; look at CPM bit field of PCMCTL0
    CMP  R0, #0x00000100            ; is CPM bit field (R0) == 0x00000100?
    BEQ  clockInit48cont2           ; if so, continue
    ADD  R2, R2, #1                 ; increment CPMwait counter
    CMP  R2, R3                     ; is CPMwait (R2) < timeout (R3)?
    BLO  clockInit48CPMwaitLoop     ; if so, continue looping
clockInit48CPMwaitTimeout
    BX   LR
clockInit48cont2
    ; wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    MOV  R2, #0                     ; R2 is Postwait counter
    LDR  R3, =100000                ; R3 is Postwait counter timeout value
    LDR  R1, =PCMCTL1
clockInit48PostwaitLoop
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000100        ; look at PMR_BUSY bit of PCMCTL1
    BEQ  clockInit48cont3           ; if cleared, continue
    ADD  R2, R2, #1                 ; increment Postwait counter
    CMP  R2, R3                     ; is Postwait (R2) < timeout (R3)?
    BLO  clockInit48PostwaitLoop    ; if so, continue looping
clockInit48PostwaitTimeout
    BX   LR
clockInit48cont3
    ; initialize PJ.3 and PJ.2 and make them HFXT (PJ.3 built-in 48 MHz crystal out; PJ.2 built-in 48 MHz crystal in)
    ; configure built-in 48 MHz crystal pins as HFXT
    LDR  R1, =PJSEL0
    LDRH R0, [R1]
    ORR  R0, R0, #0x000C            ; configure built-in 48 MHz crystal for HFXT operation
    STRH R0, [R1]
    LDR  R1, =PJSEL1
    LDRH R0, [R1]
    BIC  R0, R0, #0x000C            ; configure built-in 48 MHz crystal for HFXT operation
    STRH R0, [R1]
    ; unlock CS module for register access
    LDR  R1, =CSKEY
    LDR  R0, =0x695A
    STR  R0, [R1]                   ; CSKEY = 0x0000695A
    ; configure crystal settings
    LDR  R1, =CSCTL2
    LDR  R0, [R1]
    ORR  R0, R0, #0x01000000        ; enable HFXT
    BIC  R0, R0, #0x00700000        ; clear HFXTFREQ bit field
    ORR  R0, R0, #0x00600000        ; configure for 48 MHz external crystal
    ORR  R0, R0, #0x00010000        ; HFXT oscillator drive selection for crystals >4 MHz
    STR  R0, [R1]
    BIC  R0, R0, #0x02000000        ; disable high-frequency crystal bypass
    STR  R0, [R1]
    ; wait for the HFXT clock to stabilize
    MOV  R2, #0                     ; R2 is Crystalstable counter
    LDR  R3, =100000                ; R3 is Crystalstable counter timeout value
clockInit48CrystalLoop
    LDR  R1, =CSIFG
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000002        ; look at HFXTIFG bit of CSIFG
    BEQ  clockInit48cont4           ; if cleared, continue
    LDR  R1, =CSCLRIFG
    MOV  R0, #0x00000002            ; write 1 to clear
    STR  R0, [R1]                   ; clear the HFXT oscillator interrupt flag
    ADD  R2, R2, #1                 ; increment Crystalstable counter
    CMP  R2, R3                     ; is Crystalstable (R2) < timeout (R3)?
    BLO  clockInit48CrystalLoop     ; if so, continue looping
clockInit48CrystalTimeout
    BX   LR
clockInit48cont4
    ; set subsystem clock sources and dividers
    LDR  R1, =CSCTL1
    ; configure for SMCLK divider equal to /4
    ; configure for HSMCLK divider equal to /2
    ; configure for all other clock dividers equal to /1
    ; configure for ACLK sourced from REFOCLK
    ; configure for SMCLK and HSMCLK sourced from HFXTCLK
    ; configure for MCLK sourced from HFXTCLK
    LDR  R2, =0x20100255
    STR  R2, [R1]                   ; CSCTL1 = 0x20100255
    ; lock CS module from unintended access
    LDR  R1, =CSKEY
    MOV  R2, #0
    STR  R2, [R1]                   ; CSKEY = 0
    BX   LR                         ; return

;------------Clock_Init32kHz------------
; Configure for HSMCLK = MCLK = LFXTCLK,
; SMCLK = LFXTCLK/2, ACLK = REFOCLK.
; On the LaunchPad, the low-frequency crystal
; oscillator has a 32 kHz crystal attached, which will
; make the high speed subsystem master clock and the
; bus (master) clock run at 32 kHz.  The low speed
; subsystem master clock has a maximum frequency of
; 16,384 Hz in LPM3 and LPM3.5, so this function sets
; the SMCLK divider to /2.  This is slower than
; necessary for active mode.  In other words, this
; function is similar to Clock_Init(), except instead
; of the variable frequency DCO this uses the fixed
; low-frequency crystal.
; Input: none
; Output: none
; Modifies: R0, R1, R2, R3
Clock_Init32kHz
    ; wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    MOV  R2, #0                     ; R2 is Prewait counter
    LDR  R3, =100000                ; R3 is Prewait counter timeout value
    LDR  R1, =PCMCTL1
clockInit32PrewaitLoop
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000100        ; look at PMR_BUSY bit of PCMCTL1
    BEQ  clockInit32cont0           ; if cleared, continue
    ADD  R2, R2, #1                 ; increment Prewait counter
    CMP  R2, R3                     ; is Prewait (R2) < timeout (R3)?
    BLO  clockInit32PrewaitLoop     ; if so, continue looping
clockInit32PrewaitTimeout
    BX   LR
clockInit32cont0
    ; initialize PJ.1 and PJ.0 and make them LFXT (PJ.1 built-in 32 kHz crystal out; PJ.0 built-in 32 kHz crystal in)
    ; configure built-in 32 kHz crystal pins as LFXT
    LDR  R1, =PJSEL0
    LDRH R0, [R1]
    ORR  R0, R0, #0x0003            ; configure built-in 32 kHz crystal for LFXT operation
    STRH R0, [R1]
    LDR  R1, =PJSEL1
    LDRH R0, [R1]
    BIC  R0, R0, #0x0003            ; configure built-in 32 kHz crystal for LFXT operation
    STRH R0, [R1]
    ; unlock CS module for register access
    LDR  R1, =CSKEY
    LDR  R0, =0x695A
    STR  R0, [R1]                   ; CSKEY = 0x0000695A
    ; configure crystal settings
    LDR  R1, =CSCTL2
    LDR  R0, [R1]
    ORR  R0, R0, #0x00000100        ; enable LFXT
    BIC  R0, R0, #0x00000003        ; clear LFXTDRIVE bit field
    ORR  R0, R0, #0x00000003        ; configure for maximum drive strength/current consumption
    STR  R0, [R1]
    BIC  R0, R0, #0x00000200        ; disable low-frequency crystal bypass
    STR  R0, [R1]
    ; wait for the LFXT clock to stabilize
    MOV  R2, #0                     ; R2 is Crystalstable counter
    LDR  R3, =100000                ; R3 is Crystalstable counter timeout value
clockInit32CrystalLoop
    LDR  R1, =CSIFG
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000001        ; look at LFXTIFG bit of CSIFG
    BEQ  clockInit32cont1           ; if cleared, continue
    LDR  R1, =CSCLRIFG
    MOV  R0, #0x00000001            ; write 1 to clear
    STR  R0, [R1]                   ; clear the LFXT oscillator interrupt flag
    ADD  R2, R2, #1                 ; increment Crystalstable counter
    CMP  R2, R3                     ; is Crystalstable (R2) < timeout (R3)?
    BLO  clockInit32CrystalLoop     ; if so, continue looping
clockInit32CrystalTimeout
    BX   LR
clockInit32cont1
    ; set subsystem clock sources and dividers
    LDR  R1, =CSCTL1
    ; configure for SMCLK divider equal to /2 (necessary for LPM3 and LPM3.5)
    ; configure for all other clock dividers equal to /1
    ; configure for ACLK sourced from REFOCLK
    ; configure for SMCLK and HSMCLK sourced from LFXTCLK
    ; configure for MCLK sourced from LFXTCLK
    LDR  R2, =0x10000200
    STR  R2, [R1]                   ; CSCTL1 = 0x10000200
    ; lock CS module from unintended access
    LDR  R1, =CSKEY
    MOV  R2, #0
    STR  R2, [R1]                   ; CSKEY = 0
    ; wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    MOV  R2, #0                     ; R2 is Postwait counter
    LDR  R3, =100000                ; R3 is Postwait counter timeout value
    LDR  R1, =PCMCTL1
clockInit32PostwaitLoop
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000100        ; look at PMR_BUSY bit of PCMCTL1
    BEQ  clockInit32cont2           ; if cleared, continue
    ADD  R2, R2, #1                 ; increment Postwait counter
    CMP  R2, R3                     ; is Postwait (R2) < timeout (R3)?
    BLO  clockInit32PostwaitLoop    ; if so, continue looping
clockInit32PostwaitTimeout
    BX   LR
clockInit32cont2
    ; request power active mode LF VCORE0 to support the 32 kHz frequency
    LDR  R1, =PCMCTL0
    LDR  R0, [R1]
    LDR  R2, =0xFFFF000F            ; PCMKEY bit field and AMR bit field
    BIC  R0, R0, R2                 ; clear PCMKEY bit field and AMR bit field
    LDR  R2, =0x695A0000            ; PCM key value
    ORR  R0, R0, R2                 ; write the proper PCM key to unlock write access
    ORR  R0, R0, #0x00000008        ; request power active mode LF VCORE0
    STR  R0, [R1]
    ; check if the transition is invalid (see Figure 7-3 on p344 of datasheet)
    LDR  R1, =PCMIFG
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000004        ; look at AM_INVALID_TR_IFG bit of PCMIFG
    BEQ  clockInit32cont3           ; if cleared, continue
    ; bit 2 set on active mode transition invalid; bits 1-0 are for LPM-related errors; bit 6 is for DC-DC-related error
    ; clear the transition invalid flag
    LDR  R1, =PCMCLRIFG
    MOV  R0, #0x00000004            ; write 1 to clear
    STR  R0, [R1]
    ; to do: look at CPM bit field in PCMCTL0, figure out what mode you're in, and step through the chart to transition to the mode you want
    ; or be lazy and do nothing; this should work out of reset at least, but it WILL NOT work if Clock_Int48MHz() has been called
    BX   LR
clockInit32cont3
    ; wait for the CPM (Current Power Mode) bit field to reflect a change to active mode LF VCORE0
    MOV  R2, #0                     ; R2 is CPMwait counter
    LDR  R3, =500000                ; R3 is CPMwait counter timeout value
    LDR  R1, =PCMCTL0
clockInit32CPMwaitLoop
    LDR  R0, [R1]
    AND  R0, R0, #0x00003F00        ; look at CPM bit field of PCMCTL0
    CMP  R0, #0x00000800            ; is CPM bit field (R0) == 0x00000800?
    BEQ  clockInit32cont4           ; if so, continue
    ADD  R2, R2, #1                 ; increment CPMwait counter
    CMP  R2, R3                     ; is CPMwait (R2) < timeout (R3)?
    BLO  clockInit32CPMwaitLoop     ; if so, continue looping
clockInit32CPMwaitTimeout
    BX   LR
clockInit32cont4
    BX   LR                         ; return

;------------Clock_InitLowPower------------
; Configure for HSMCLK = MCLK = ACLK = REFOCLK,
; SMCLK = REFOCLK/2.
; On the LaunchPad, the low-frequency, low-power
; oscillator has a 32 kHz reference clock, which will
; make the high speed subsystem master clock and the
; bus (master) clock run at 32 kHz.  The low speed
; subsystem master clock has a maximum frequency of
; 16,384 Hz in LPM3 and LPM3.5, so this function sets
; the SMCLK divider to /2.  This is slower than
; necessary for active mode.  In other words, this
; function is similar to Clock_Init32kHz(), except
; instead it should be lower power and slightly less
; accurate.
; Input: none
; Output: none
; Modifies: R0, R1, R2, R3
Clock_InitLowPower
    ; wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    MOV  R2, #0                     ; R2 is Prewait counter
    LDR  R3, =100000                ; R3 is Prewait counter timeout value
    LDR  R1, =PCMCTL1
clockInitLPPrewaitLoop
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000100        ; look at PMR_BUSY bit of PCMCTL1
    BEQ  clockInitLPcont0           ; if cleared, continue
    ADD  R2, R2, #1                 ; increment Prewait counter
    CMP  R2, R3                     ; is Prewait (R2) < timeout (R3)?
    BLO  clockInitLPPrewaitLoop     ; if so, continue looping
clockInitLPPrewaitTimeout
    BX   LR
clockInitLPcont0
    ; unlock CS module for register access
    LDR  R1, =CSKEY
    LDR  R0, =0x695A
    STR  R0, [R1]                   ; CSKEY = 0x0000695A
    ; set subsystem clock sources and dividers
    LDR  R1, =CSCTL1
    ; configure for SMCLK divider equal to /2 (necessary for LPM3 and LPM3.5)
    ; configure for all other clock dividers equal to /1
    ; configure for ACLK sourced from REFOCLK
    ; configure for SMCLK and HSMCLK sourced from REFOCLK
    ; configure for MCLK sourced from REFOCLK
    LDR  R2, =0x10000222
    STR  R2, [R1]                   ; CSCTL1 = 0x10000222
    ; lock CS module from unintended access
    LDR  R1, =CSKEY
    MOV  R2, #0
    STR  R2, [R1]                   ; CSKEY = 0
    ; wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    MOV  R2, #0                     ; R2 is Postwait counter
    LDR  R3, =100000                ; R3 is Postwait counter timeout value
    LDR  R1, =PCMCTL1
clockInitLPPostwaitLoop
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000100        ; look at PMR_BUSY bit of PCMCTL1
    BEQ  clockInitLPcont1           ; if cleared, continue
    ADD  R2, R2, #1                 ; increment Postwait counter
    CMP  R2, R3                     ; is Postwait (R2) < timeout (R3)?
    BLO  clockInitLPPostwaitLoop    ; if so, continue looping
clockInitLPPostwaitTimeout
    BX   LR
clockInitLPcont1
    ; request power active mode LF VCORE0 to support the 32 kHz frequency
    LDR  R1, =PCMCTL0
    LDR  R0, [R1]
    LDR  R2, =0xFFFF000F            ; PCMKEY bit field and AMR bit field
    BIC  R0, R0, R2                 ; clear PCMKEY bit field and AMR bit field
    LDR  R2, =0x695A0000            ; PCM key value
    ORR  R0, R0, R2                 ; write the proper PCM key to unlock write access
    ORR  R0, R0, #0x00000008        ; request power active mode LF VCORE0
    STR  R0, [R1]
    ; check if the transition is invalid (see Figure 7-3 on p344 of datasheet)
    LDR  R1, =PCMIFG
    LDR  R0, [R1]
    ANDS R0, R0, #0x00000004        ; look at AM_INVALID_TR_IFG bit of PCMIFG
    BEQ  clockInitLPcont2           ; if cleared, continue
    ; bit 2 set on active mode transition invalid; bits 1-0 are for LPM-related errors; bit 6 is for DC-DC-related error
    ; clear the transition invalid flag
    LDR  R1, =PCMCLRIFG
    MOV  R0, #0x00000004            ; write 1 to clear
    STR  R0, [R1]
    ; to do: look at CPM bit field in PCMCTL0, figure out what mode you're in, and step through the chart to transition to the mode you want
    ; or be lazy and do nothing; this should work out of reset at least, but it WILL NOT work if Clock_Int48MHz() has been called
    BX   LR
clockInitLPcont2
    ; wait for the CPM (Current Power Mode) bit field to reflect a change to active mode LF VCORE0
    MOV  R2, #0                     ; R2 is CPMwait counter
    LDR  R3, =500000                ; R3 is CPMwait counter timeout value
    LDR  R1, =PCMCTL0
clockInitLPCPMwaitLoop
    LDR  R0, [R1]
    AND  R0, R0, #0x00003F00        ; look at CPM bit field of PCMCTL0
    CMP  R0, #0x00000800            ; is CPM bit field (R0) == 0x00000800?
    BEQ  clockInitLPcont3           ; if so, continue
    ADD  R2, R2, #1                 ; increment CPMwait counter
    CMP  R2, R3                     ; is CPMwait (R2) < timeout (R3)?
    BLO  clockInitLPCPMwaitLoop     ; if so, continue looping
clockInitLPCPMwaitTimeout
    BX   LR
clockInitLPcont3
    BX   LR                         ; return

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
