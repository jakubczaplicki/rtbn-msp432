; ProfileFact.s
; Runs on MSP432
; Use the SysTick timer to compare the execution speeds of the
; iterative and recursive implementations of factorial.
; Daniel Valvano
; June 12, 2015

; This example accompanies the books
;  "Embedded Systems: Introduction to MSP432 Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Volume 1, Program 5.20
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

        IMPORT   SysTick_Init
        IMPORT   SysTick_Wait
        IMPORT   SysTick_Wait10ms

SYSTICK_STCVR         EQU 0xE000E018  ; SysTick Current Value Register

;store results of the test to global variables
        AREA    DATA, ALIGN=4
Iter1   SPACE 4
Iter2   SPACE 4
Iter3   SPACE 4
Iter4   SPACE 4
Iter5   SPACE 4
Rec1    SPACE 4
Rec2    SPACE 4
Rec3    SPACE 4
Rec4    SPACE 4
Rec5    SPACE 4
;export for use with debugger
        EXPORT Iter1 [DATA,SIZE=4]
        EXPORT Iter2 [DATA,SIZE=4]
        EXPORT Iter3 [DATA,SIZE=4]
        EXPORT Iter4 [DATA,SIZE=4]
        EXPORT Iter5 [DATA,SIZE=4]
        EXPORT Rec1 [DATA,SIZE=4]
        EXPORT Rec2 [DATA,SIZE=4]
        EXPORT Rec3 [DATA,SIZE=4]
        EXPORT Rec4 [DATA,SIZE=4]
        EXPORT Rec5 [DATA,SIZE=4]

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start

; iterative implementation (22 bytes)
; Input: R0 is n
; Output: R0 is Fact(n)
; Assumes: R0 <= 12 (13! overflows)
FactIter
     MOV R1, #1      ; R1 = 1 = total
FactIloop
     CMP R0, #1      ; is n (R0) <= 1?
     BLS FactIdone   ; if so, skip to FactIdone
     MUL R1, R0, R1  ; total = total*n
     SUB R0, R0, #1  ; n = n – 1
     B   FactIloop
FactIdone
     MOV R0, R1      ; total = Fact(n)
     BX  LR
; recursive implementation (30 bytes)
; Input: R0 is n
; Output: R0 is Fact(n)
; Assumes: R0 <= 12 (13! overflows)
FactRec
     CMP R0, #1     ; is n (R0) <= 1?
     BLS FactRend   ; if so, to FactRend
     PUSH {R0, LR}  ; save R0 and LR
     SUB R0, R0, #1 ; n = n – 1
     BL  FactRec    ; R0 = FactRec(n-1)
     POP {R1, LR}   ; restore R1, LR
     MUL R0, R0, R1 ; R0 = n*FactRec(n-1)
     BX  LR         ; normal return
FactRend
     MOV R0, #1     ; R0 = 1
     BX  LR         ; end case return

Start
    BL   SysTick_Init               ; initialize SysTick timer
    LDR  R6, =SYSTICK_STCVR         ; R6 = &SYSTICK_STCVR (pointer to Current Value Register)
    ; test FactIter(1)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #1                     ; R0 = 1 (n = 1)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Iter1                 ; R5 = &Iter1
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactIter(2)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #2                     ; R0 = 2 (n = 2)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Iter2                 ; R5 = &Iter2
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactIter(3)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #3                     ; R0 = 3 (n = 3)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Iter3                 ; R5 = &Iter3
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactIter(4)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #4                     ; R0 = 4 (n = 3)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Iter4                 ; R5 = &Iter4
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactIter(5)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #5                     ; R0 = 5 (n = 5)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Iter5                 ; R5 = &Iter5
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(1)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #1                     ; R0 = 1 (n = 1)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Rec1                  ; R5 = &Rec1
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(2)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #2                     ; R0 = 2 (n = 2)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Rec2                  ; R5 = &Rec2
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(3)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #3                     ; R0 = 3 (n = 3)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Rec3                  ; R5 = &Rec3
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(4)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #4                     ; R0 = 4 (n = 4)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Rec4                  ; R5 = &Rec4
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(5)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #5                     ; R0 = 5 (n = 5)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, =Rec5                  ; R5 = &Rec5
    STR  R4, [R5]                   ; R4 = [R5]
loop
    B   loop

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
