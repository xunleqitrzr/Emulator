;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; GAME OF LIFE (16-REGISTER VERSION)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

JMP MAIN

INIT:
    LDI 1
    MOV H, A          ; H = 1 (Alive State)
    LDI 3
    MOV I, A          ; I = 3 (Survive/Born condition)
    RET

;; --- TRUE 8-WAY NEIGHBOR COUNT (Toroidal Wrap) ---
COUNT_NEIGHBORS:
    LDI 0
    MOV D, A          ; D = sum (start at 0)

    ; 1. Left (-1)
    MOV E, B
    DEC E
    LDA_IDX 0x0200, E
    ADD D, A

    ; 2. Right (+1)
    MOV E, B
    INC E
    LDA_IDX 0x0200, E
    ADD D, A

    ; 3. Top-Center (-32 -> +224)
    MOV E, B
    LDI 224
    ADD E, A
    LDA_IDX 0x0200, E
    ADD D, A

    ; 4. Bottom-Center (+32)
    MOV E, B
    LDI 32
    ADD E, A
    LDA_IDX 0x0200, E
    ADD D, A

    ; 5. Top-Left (-33 -> +223)
    MOV E, B
    LDI 223
    ADD E, A
    LDA_IDX 0x0200, E
    ADD D, A

    ; 6. Top-Right (-31 -> +225)
    MOV E, B
    LDI 225
    ADD E, A
    LDA_IDX 0x0200, E
    ADD D, A

    ; 7. Bottom-Left (+31)
    MOV E, B
    LDI 31
    ADD E, A
    LDA_IDX 0x0200, E
    ADD D, A

    ; 8. Bottom-Right (+33)
    MOV E, B
    LDI 33
    ADD E, A
    LDA_IDX 0x0200, E
    ADD D, A

    RET

APPLY_RULES:
    CMP C, H
    JE LIVE
DEAD:
    CMP D, I
    JE BORN
    LDI 0
    RET
LIVE:
    LDI 2
    MOV F, A
    CMP D, F
    JE SURVIVE
    CMP D, I
    JE SURVIVE
    LDI 0
    RET
SURVIVE:
    LDI 1
    RET
BORN:
    LDI 1
    RET

SEED_BOARD:
    LDI 255
    MOV B, A
SEED_LOOP:
    LDA 0x9000        ; Read RNG
    MOV E, A
    LDI 1
    AND A, E          ; A = Random & 1
    STA_IDX 0x0200, B
    DEC B
    LDI 255
    CMP B, A
    JNE SEED_LOOP
    RET

COPY_BUFFER:
    LDI 255
    MOV B, A
COPY_LOOP:
    LDA_IDX 0x0300, B
    STA_IDX 0x0200, B
    DEC B
    LDI 255
    CMP B, A
    JNE COPY_LOOP
    RET

;; --- MAIN ENTRY ---
MAIN:
    CALL INIT
    CALL SEED_BOARD

MAIN_LOOP:
    LDI 255
    MOV B, A

CELL_LOOP:
    LDA_IDX 0x0200, B
    MOV C, A
    CALL COUNT_NEIGHBORS
    CALL APPLY_RULES
    STA_IDX 0x0300, B

    DEC B
    LDI 255
    CMP B, A
    JNE CELL_LOOP

    ; --- DRAW TO SCREEN ---
    LDI 255
    MOV B, A
DISPLAY_LOOP:
    LDA_IDX 0x0300, B
    MOV C, A
    CMP C, H
    JE DRAW_ALIVE

DRAW_DEAD:
    LDI 32             ; Space
    JMP WRITE
DRAW_ALIVE:
    LDI 35             ; '#'
WRITE:
    STA_IDX 0x4000, B
    DEC B
    LDI 255
    CMP B, A
    JNE DISPLAY_LOOP

    CALL COPY_BUFFER
    JMP MAIN_LOOP