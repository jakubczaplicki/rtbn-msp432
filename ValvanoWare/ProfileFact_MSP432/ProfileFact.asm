; ProfileFact.asm
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

       .global  SysTick_Init
       .global  SysTick_Wait
       .global  SysTick_Wait10ms
       .thumb

;store results of the test to global variables
       .data
       .align  2
Iter1  .space 4
Iter2  .space 4
Iter3  .space 4
Iter4  .space 4
Iter5  .space 4
Rec1   .space 4
Rec2   .space 4
Rec3   .space 4
Rec4   .space 4
Rec5   .space 4
       .global  Iter1

       .text
       .align 2
SYSTICK_STCVR .field 0xE000E018,32  ; SysTick Current Value Register
Iter1Addr     .field Iter1,32
Iter2Addr     .field Iter2,32
Iter3Addr     .field Iter3,32
Iter4Addr     .field Iter4,32
Iter5Addr     .field Iter5,32
Rec1Addr      .field Rec1,32
Rec2Addr      .field Rec2,32
Rec3Addr      .field Rec3,32
Rec4Addr      .field Rec4,32
Rec5Addr      .field Rec5,32

        .global  main

; iterative implementation (22 bytes)
; Input: R0 is n
; Output: R0 is Fact(n)
; Assumes: R0 <= 12 (13! overflows)
FactIter:  .asmfunc
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
    .endasmfunc

; recursive implementation (30 bytes)
; Input: R0 is n
; Output: R0 is Fact(n)
; Assumes: R0 <= 12 (13! overflows)
FactRec:  .asmfunc
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
    .endasmfunc

      .thumbfunc main
main: .asmfunc
    BL   SysTick_Init               ; initialize SysTick timer
    LDR  R6, SYSTICK_STCVR          ; R6 = &SYSTICK_STCVR (pointer to Current Value Register)
    ; test FactIter(1)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #1                     ; R0 = 1 (n = 1)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Iter1Addr              ; R5 = &Iter1
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactIter(2)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #2                     ; R0 = 2 (n = 2)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Iter2Addr              ; R5 = &Iter2
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactIter(3)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #3                     ; R0 = 3 (n = 3)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Iter3Addr              ; R5 = &Iter3
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactIter(4)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #4                     ; R0 = 4 (n = 3)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Iter4Addr              ; R5 = &Iter4
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactIter(5)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #5                     ; R0 = 5 (n = 5)
    BL   FactIter                   ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Iter5Addr              ; R5 = &Iter5
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(1)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #1                     ; R0 = 1 (n = 1)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Rec1Addr               ; R5 = &Rec1
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(2)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #2                     ; R0 = 2 (n = 2)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Rec2Addr               ; R5 = &Rec2
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(3)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #3                     ; R0 = 3 (n = 3)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Rec3Addr               ; R5 = &Rec3
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(4)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #4                     ; R0 = 4 (n = 4)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Rec4Addr               ; R5 = &Rec4
    STR  R4, [R5]                   ; R4 = [R5]
    ; test FactRec(5)
    LDR  R4, [R6]                   ; R4 = [R6] = SysTick Current Value
    MOV  R0, #5                     ; R0 = 5 (n = 5)
    BL   FactRec                    ; R0 = R0! = n!
    LDR  R5, [R6]                   ; R5 = [R6] = SysTick Current Value
    SUB  R4, R4, R5                 ; R4 = R4 - R5 (number of cycles elapsed)
    SUB  R4, R4, #1                 ; R4 = R4 - 1 (subtract overhead so commenting function leaves R4 == 0)
    LDR  R5, Rec5Addr               ; R5 = &Rec5
    STR  R4, [R5]                   ; R4 = [R5]
loop
    B   loop
    .endasmfunc

    .end
