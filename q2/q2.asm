section .data
    filename db "temperatur.txt", 0

    msg_total db "Total readings: ", 0
    msg_valid db "Valid readings: ", 0
    newline db 10

section .bss
    buffer resb 4096
    numbuf resb 20        ; buffer for number printing

section .text
    global _start

print_string:
    push rdi
    push rsi
    push rdx

    mov rsi, rdi
    xor rdx, rdx

count_len:
    cmp byte [rsi+rdx], 0
    je print
    inc rdx
    jmp count_len

print:
    mov rax, 1      ; write
    mov rdi, 1      ; stdout
    syscall

    pop rdx
    pop rsi
    pop rdi
    ret

print_number:
    mov rbx, 10
    mov rdi, numbuf + 19
    mov byte [rdi], 0

convert_loop:
    dec rdi
    xor rdx, rdx
    div rbx
    add dl, '0'
    mov [rdi], dl
    test rax, rax
    jnz convert_loop

    mov rax, 1
    mov rsi, rdi
    mov rdx, numbuf + 20
    sub rdx, rsi
    mov rdi, 1
    syscall
    ret

_start:

mov rax, 2
mov rdi, filename
mov rsi, 0
syscall

cmp rax, 0
js error_exit

mov r12, rax

mov rax, 0
mov rdi, r12
mov rsi, buffer
mov rdx, 4096
syscall

mov rcx, rax
mov rsi, buffer

xor rbx, rbx    ; total lines
xor rdx, rdx    ; valid lines
xor r8, r8      ; flag (line has data)

loop_lines:
cmp rcx, 0
je done

mov al, [rsi]

cmp al, 10          ; '\n'
je newline_handler

cmp al, 13          ; '\r'
je skip_char

mov r8, 1           ; mark line as non-empty

skip_char:
inc rsi
dec rcx
jmp loop_lines

newline_handler:
inc rbx             ; total++

cmp r8, 1
jne reset_flag

inc rdx             ; valid++

reset_flag:
xor r8, r8

inc rsi
dec rcx
jmp loop_lines

done:

mov rdi, msg_total
call print_string

mov rax, rbx
call print_number

mov rax, 1
mov rdi, 1
mov rsi, newline
mov rdx, 1
syscall

mov rdi, msg_valid
call print_string

mov rax, rdx
call print_number

mov rax, 1
mov rdi, 1
mov rsi, newline
mov rdx, 1
syscall

jmp exit

error_exit:
mov rdi, msg_total
call print_string

exit:
mov rax, 60
xor rdi, rdi
syscall
