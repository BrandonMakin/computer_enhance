global MOVAllBytesASM
global NOPAllBytesASM
global CMPAllBytesASM
global DECAllBytesASM

section .text

MOVAllBytesASM:
    xor rax, rax
.loop:
    mov     [rdx+rax], al
    inc     rax
    cmp     rax, rcx
    jb      .loop
    ret

NOPAllBytesASM:
    xor rax, rax
.loop:
    db 0x0f, 0x1f, 0x00 ; NOTE(casey): This is the byte sequence for a 3-byte NOP
    inc     rax
    cmp     rax, rcx
    jb      .loop
    ret

CMPAllBytesASM:
    xor rax, rax
.loop:
    inc     rax
    cmp     rax, rcx
    jb      .loop
    ret

DECAllBytesASM:
.loop:
    dec     rcx
    jnz      .loop
    ret