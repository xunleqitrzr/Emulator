;; factorial(n) — computes factorial of A
;; Returns result in A
JMP START_MAIN

FACTORIAL:
    CMP A, D
    JE BASE_CASE
    JL BASE_CASE     ; both jumps together are JLE
    PUSH A           ; save current n
    DEC A
    CALL FACTORIAL   ; recursive call: factorial(n-1)
    POP B            ; restore n into B
    MUL A, B         ; A = A * n
    RET
BASE_CASE:
    LDI 1
    RET

;; --- main program
START_MAIN:
    LDI 5
    CALL FACTORIAL
    HLT
