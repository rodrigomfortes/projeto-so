; Bootloader e Paging (Higher-Half Kernel)
; Prepara o ambiente 32-bits, configura a stack e o diretório de páginas 
; antes de passar o controle pro código em C.

global loader                       ; o ponto de entrada para o linker (ld)
extern kmain                        ; kmain é definido em kmain.c

; === Constantes ===
MAGIC_NUMBER    equ 0x1BADB002      ; define a constante do magic number
ALIGN_MODULES   equ 0x00000001      ; instrui o GRUB a alinhar módulos em page boundaries (Cap 7)
FLAGS           equ ALIGN_MODULES
CHECKSUM        equ -(MAGIC_NUMBER + FLAGS) ; magic + flags + checksum = 0
KERNEL_STACK_SIZE equ 16384         ; tamanho da stack: 16KB

; Endereço virtual base do kernel (3GB) e o índice na Page Directory
KERNEL_VIRTUAL_BASE equ 0xC0000000  
KERNEL_PAGE_NUMBER  equ (KERNEL_VIRTUAL_BASE >> 22)  ; 768

; Flags do PDE: Present, R/W e Page Size de 4MB
PDE_FLAGS       equ 0x00000083

; Tabela do Page Directory. Precisa ser alinhada em 4KB.
; O Page Directory deve estar alinhado em 4096 bytes (seção 9.2)
; Reservamos espaço na seção .data para que tenha endereço físico correto
section .data
align 4096
boot_page_directory:
    ; Começa zerada (1024 dwords)
    times 1024 dd 0

section .text
align 4

; --- Header Multiboot (Cap. 2) ---
    dd MAGIC_NUMBER                 ; escreve o magic number no binário executável
    dd FLAGS                        ; escreve as flags no binário executável
    dd CHECKSUM                     ; escreve o checksum no binário executável

; O GRUB joga a gente aqui em modo protegido (1MB físico).
; O Paging ainda tá desligado, então tudo aqui é endereço físico raiz.
; A struct do multiboot vem no ebx por padrão.
loader:
    ; Salva o ebx no ecx porque ainda não temos stack
    mov ecx, ebx

    ; --- Setup das páginas ---
    ; Como o boot_page_directory foi compilado com o endereço de 3GB,
    ; a gente subtrai KERNEL_VIRTUAL_BASE pra achar a posição na RAM.

    ; Identity mapping pros primeiros 4MB (pra gente não crashar depois de ligar o paging)
    mov eax, PDE_FLAGS              
    mov dword [boot_page_directory - KERNEL_VIRTUAL_BASE + 0*4], eax

    ; Mapeia da área de 3GB (768 * 4 bytes) -> pros mesmos primeiros 4MB reais
    mov dword [boot_page_directory - KERNEL_VIRTUAL_BASE + KERNEL_PAGE_NUMBER*4], eax

    ; --- Ligando o Paging ---

    ; Carrega o endereço da tabela e liga suporte a 4MB (PSE)
    mov eax, boot_page_directory - KERNEL_VIRTUAL_BASE
    mov cr3, eax

    mov ebx, cr4
    or  ebx, 0x00000010             
    mov cr4, ebx

    ; Vira a chave do Paging (PG bit)
    mov ebx, cr0
    or  ebx, 0x80000000             
    mov cr0, ebx

    ; Pulo pro Higher Half! 
    ; Agora os endereços de 3GB vão magicamente apontar pros 4MB iniciais
    lea ebx, [higher_half]          
    jmp ebx                         

higher_half:
    ; Tira as rodinhas: removemos o identity map do começo da memória
    mov dword [boot_page_directory + 0*4], 0
    invlpg [0]                                  

    ; Prepara a stack (agora rodando bonita no mundo virtual)
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    ; Chama o kmain
    ; Temos que corrigir o ponteiro do multiboot (ecx) somando a base virtual
    add ecx, KERNEL_VIRTUAL_BASE
    push ecx
    call kmain

.loop:
    jmp .loop                       ; loop infinito caso kmain retorne

; === Seção BSS: Stack ===
section .bss
align 4
kernel_stack:                       ; rótulo que aponta para o início da stack
    resb KERNEL_STACK_SIZE          ; reserva 16KB para a stack
