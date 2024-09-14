;  ========================================================================
;
;  (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Please see https://computerenhance.com for more information
;
;  ========================================================================

;  ========================================================================
;  LISTING 143
;  ========================================================================

;
;  NOTE(casey): Regular Homework
;
;                   ; dest  | src0  | src1
                    ;----------------------
    mov rax, 1      ; s0    | imm   |
    mov rbx, 2      ; s1    | imm   |   
    mov rcx, 3      ; s2    | imm   |
    mov rdx, 4      ; s3    | imm   |
    add rax, rbx    ; s4    | s0    | s1
	add rcx, rdx    ; s5    | s2    | s3
	add rax, rcx    ; s6    | s4    | s5
    mov rcx, rbx    ; s7    | s5    | s1
	inc rax         ; s8    | s6    |
    dec rcx         ; s9    | s7    |
    sub rax, rbx    ; s10   | s8    | s1
	sub rcx, rdx    ; s11   | s9    | s3
	sub rax, rcx    ; s12   | s10   | s11

;
;  NOTE(casey): CHALLENGE MODE WITH ULTIMATE DIFFICULTY SETTINGS
;               DO NOT ATTEMPT THIS! IT IS MUCH TOO HARD FOR
;               A HOMEWORK ASSIGNMENT!1!11!!
;
top:
    pop rcx
    sub rsp, rdx
	mov rbx, rax
	shl rbx, 0
	not rbx
	loopne top
