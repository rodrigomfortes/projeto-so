; loader.s - Bootloader com Multiboot header, stack e chamada para C (Capítulos 2.3, 3.1, 3.2 e 7)
; Referência: The Little OS Book - Chapters 2, 3 & 7

global loader                   ; o ponto de entrada para o linker (ld)
extern kmain                    ; kmain é definido em kmain.c (Cap 3.2)

MAGIC_NUMBER    equ 0x1BADB002  ; define a constante do magic number
ALIGN_MODULES   equ 0x00000001  ; instrui o GRUB a alinhar módulos em page boundaries (Cap 7)
FLAGS           equ ALIGN_MODULES
CHECKSUM        equ -(MAGIC_NUMBER + FLAGS) ; magic + flags + checksum = 0
KERNEL_STACK_SIZE equ 16384     ; tamanho da stack: 16KB

section .text                   ; início da seção de código
align 4                         ; o código deve estar alinhado a 4 bytes
    dd MAGIC_NUMBER             ; escreve o magic number no binário executável
    dd FLAGS                    ; escreve as flags no binário executável
    dd CHECKSUM                 ; escreve o checksum no binário executável

loader:                         ; o ponto de entrada do loader (definido no linker script)
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; configura o stack pointer (Cap 3.1)
    push ebx                    ; passa o ponteiro da struct multiboot_info como argumento (Cap 7)
    call kmain                  ; chama a função kmain definida em C (Cap 3.2)
.loop:
    jmp .loop                   ; loop infinito caso kmain retorne

section .bss                    ; seção para dados não inicializados
align 4                         ; alinhar em 4 bytes
kernel_stack:                   ; rótulo que aponta para o início da stack
    resb KERNEL_STACK_SIZE      ; reserva 16KB para a stack

