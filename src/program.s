; program.s - Programa simples para testar o carregamento via módulo GRUB (Capítulo 7)
; Compilado como binário flat: nasm -f bin program.s -o program
; Referência: The Little OS Book - Chapter 7

[BITS 32]   ; Gera código 32 bits (nosso kernel roda em protected mode 32-bit)

; Define eax com um valor reconhecível no log do Bochs
mov eax, 0xDEADBEEF

; Loop infinito - $ representa o endereço da instrução atual
jmp $
