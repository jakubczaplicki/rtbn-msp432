;Random number generator;
; Linear congruential generator 
; from Numerical Recipes by Press et al.
; Jonathan Valvano

; How to use: 
; 1) call Random_Init once with a seed
;     Random_Init(1);
;     Random_Init(NVIC_CURRENT_R);
; 2) call Random over and over to get a new random number
;   n = Random();    // 32 bit number
;   m = (Random()>>24)%60; // a number from 0 to 59
; we align 32 bit variables to 32-bits
; we align op codes to 16 bits
       .thumb
       .data
       .align 2
M      .space 4
n      .space 4
       .text
       .align 2
       .global  main
       .global  Random_Init
       .global  Random
       .global  Random16
       .global  Random32

;------------Random_Init------------
; Return R0= random number
; Linear congruential generator
; from Numerical Recipes by Press et al.
Random_Init: .asmfunc
      LDR R1,MAddr ; R1=&M
      STR R0,[R1]  ; set M
      BX  LR
      .endasmfunc
Random: .asmfunc
      LDR R2,MAddr ; R2=&M, address of M
      LDR R0,[R2]  ; R0=M, value of M
      LDR R1,Slope
      MUL R0,R0,R1 ; R0 = 1664525*M
      LDR R1,Offst
      ADD R0,R0,R1 ; 1664525*M+1013904223
      STR R0,[R2]  ; store M
      LSR R0,#24   ; 0 to 255
      BX  LR
      .endasmfunc

Random16: .asmfunc
      LDR R2,MAddr ; R2=&M, address of M
      LDR R0,[R2]  ; R0=M, value of M
      LDR R1,Slope
      MUL R0,R0,R1 ; R0 = 1664525*M
      LDR R1,Offst
      ADD R0,R0,R1 ; 1664525*M+1013904223
      STR R0,[R2]  ; store M
      LSR R0,#16   ; 0 to 65535
      BX  LR
      .endasmfunc

Random32: .asmfunc
      LDR R2,MAddr ; R2=&M, address of M
      LDR R0,[R2]  ; R0=M, value of M
      LDR R1,Slope
      MUL R0,R0,R1 ; R0 = 1664525*M
      LDR R1,Offst
      ADD R0,R0,R1 ; 1664525*M+1013904223
      STR R0,[R2]  ; store M
      BX  LR
      .endasmfunc
MAddr .field M,32
Slope .field 1664525,32
Offst .field 1013904223,32
nAddr .field n,32

       .end

