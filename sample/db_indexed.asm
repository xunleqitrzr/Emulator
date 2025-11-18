JMP START

; --- Data Section ---
HELLO_MSG:
    .DB "Hello, indexed world!"     ; message
    .DB 13, 10                      ; cr, lf
    .DB 0                           ; null terminator

; --- Code Section ---
START:
    LDI 0                           ; A = 0
    MOV B, A                        ; B = 0, our index
    MOV C, A                        ; for redundancy (C already initialized as 0)

PRINT_LOOP:
    LDA_IDX HELLO_MSG, B
    ; check if the load byte is 0
    CMP A, C                        ; C is initialized as 0 (from emulator)
    JZ END

    STA 0x4000                      ; visual buffer address

    ; increment the index and loop
    INC B
    JMP PRINT_LOOP

END:
    HLT