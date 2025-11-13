;; This program counts from 0 up to 5 and then halts
LDI 5               ; load 5 into register A
MOV B, A            ; copy A into register B
LDI 0               ; load 0 into register A (this is our counter)

START_LOOP:
    INC             ; increment the value in register A (accumulator)
    CMP B, A        ; compare B (5) with A (counter)
    JE END_PROG     ; jump if equal (A == 5) to END_PROG
    JMP START_LOOP  ; if not, loop again

END_PROG:
    HLT             ; halt the CPU