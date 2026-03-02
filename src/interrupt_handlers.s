; interrupt_handlers.s - Handlers genéricos de interrupção e load_idt (Capítulo 6)
; Referência: The Little OS Book - Chapter 6: Interrupts and Input
;
; Este arquivo define:
;   1. Macros NASM para gerar handlers de interrupção (com e sem error code)
;   2. O common_interrupt_handler que salva registradores, chama C, restaura e iret
;   3. A função load_idt para carregar a IDT no processador
;   4. Instanciação dos handlers para ISR 0-31 e IRQ 32-47

extern interrupt_handler    ; função C que trata a interrupção

; ============================================================================
; load_idt - Carrega a IDT no processador
; void load_idt(unsigned int idt_ptr_addr);
; ============================================================================
global load_idt
load_idt:
    mov eax, [esp + 4]      ; endereço do struct idt_ptr
    lidt [eax]              ; carrega a IDT
    ret

; ============================================================================
; Macro para interrupções SEM error code
; Empilha 0 como error code falso e o número da interrupção
; ============================================================================
%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0             ; push 0 como error code (fake)
    push dword %1            ; push o número da interrupção
    jmp common_interrupt_handler
%endmacro

; ============================================================================
; Macro para interrupções COM error code
; O error code já foi empilhado pelo CPU, só empilha o número da interrupção
; ============================================================================
%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword %1            ; push o número da interrupção
    jmp common_interrupt_handler
%endmacro

; ============================================================================
; common_interrupt_handler - Handler comum para todas as interrupções
;
; Stack na entrada (do topo para a base):
;   [interrupt_number]  <- empilhado pela macro
;   [error_code]        <- empilhado pelo CPU ou pela macro (0 se fake)
;   [eip]               <- empilhado pelo CPU
;   [cs]                <- empilhado pelo CPU
;   [eflags]            <- empilhado pelo CPU
;
; Este handler:
;   1. Salva todos os registradores de uso geral (pushad)
;   2. Chama interrupt_handler(cpu_state, int_num, error_code, stack_state) em C
;   3. Restaura os registradores (popad)
;   4. Remove interrupt_number e error_code da stack (add esp, 8)
;   5. Retorna com iret
; ============================================================================
common_interrupt_handler:
    ; Salva todos os registradores (pushad empilha: eax, ecx, edx, ebx, esp, ebp, esi, edi)
    pushad

    ; Chama a função C interrupt_handler
    ; A convenção cdecl passa argumentos pela stack.
    ; Neste ponto a stack tem (do topo para a base):
    ;   [edi, esi, ebp, esp_orig, ebx, edx, ecx, eax] <- pushad (32 bytes)
    ;   [interrupt_number]  (4 bytes)
    ;   [error_code]        (4 bytes)
    ;   [eip, cs, eflags]   (12 bytes) <- empilhados pelo CPU
    ;
    ; A função C é chamada com toda a stack como contexto.
    call interrupt_handler

    ; Restaura os registradores
    popad

    ; Remove error_code e interrupt_number da stack
    add esp, 8

    ; Retorna da interrupção (restaura eip, cs, eflags)
    iret

; ============================================================================
; Instanciação dos handlers
; ============================================================================

; ISR 0-7: Exceções do CPU (sem error code)
no_error_code_interrupt_handler 0   ; #DE - Division Error
no_error_code_interrupt_handler 1   ; #DB - Debug Exception
no_error_code_interrupt_handler 2   ; NMI - Non-Maskable Interrupt
no_error_code_interrupt_handler 3   ; #BP - Breakpoint
no_error_code_interrupt_handler 4   ; #OF - Overflow
no_error_code_interrupt_handler 5   ; #BR - Bound Range Exceeded
no_error_code_interrupt_handler 6   ; #UD - Invalid Opcode
no_error_code_interrupt_handler 7   ; #NM - Device Not Available

; ISR 8: Double Fault (COM error code)
error_code_interrupt_handler 8      ; #DF - Double Fault

; ISR 9: Coprocessor Segment Overrun (sem error code, legacy)
no_error_code_interrupt_handler 9   ; Coprocessor Segment Overrun

; ISR 10-14: Exceções COM error code
error_code_interrupt_handler 10     ; #TS - Invalid TSS
error_code_interrupt_handler 11     ; #NP - Segment Not Present
error_code_interrupt_handler 12     ; #SS - Stack-Segment Fault
error_code_interrupt_handler 13     ; #GP - General Protection Fault
error_code_interrupt_handler 14     ; #PF - Page Fault

; ISR 15: Reservado
no_error_code_interrupt_handler 15  ; Reservado

; ISR 16: x87 Floating-Point Exception
no_error_code_interrupt_handler 16  ; #MF - x87 FPU Error

; ISR 17: Alignment Check (COM error code)
error_code_interrupt_handler 17     ; #AC - Alignment Check

; ISR 18-31: Exceções restantes (sem error code)
no_error_code_interrupt_handler 18  ; #MC - Machine Check
no_error_code_interrupt_handler 19  ; #XM - SIMD Floating-Point
no_error_code_interrupt_handler 20  ; Reservado
no_error_code_interrupt_handler 21  ; Reservado
no_error_code_interrupt_handler 22  ; Reservado
no_error_code_interrupt_handler 23  ; Reservado
no_error_code_interrupt_handler 24  ; Reservado
no_error_code_interrupt_handler 25  ; Reservado
no_error_code_interrupt_handler 26  ; Reservado
no_error_code_interrupt_handler 27  ; Reservado
no_error_code_interrupt_handler 28  ; Reservado
no_error_code_interrupt_handler 29  ; Reservado
no_error_code_interrupt_handler 30  ; Reservado
no_error_code_interrupt_handler 31  ; Reservado

; IRQ 0-15: Hardware interrupts (remapeadas pelo PIC para 32-47)
no_error_code_interrupt_handler 32  ; IRQ 0  - Timer (PIT)
no_error_code_interrupt_handler 33  ; IRQ 1  - Teclado
no_error_code_interrupt_handler 34  ; IRQ 2  - Cascade (PIC2)
no_error_code_interrupt_handler 35  ; IRQ 3  - COM2
no_error_code_interrupt_handler 36  ; IRQ 4  - COM1
no_error_code_interrupt_handler 37  ; IRQ 5  - LPT2
no_error_code_interrupt_handler 38  ; IRQ 6  - Floppy Disk
no_error_code_interrupt_handler 39  ; IRQ 7  - LPT1 / Spurious
no_error_code_interrupt_handler 40  ; IRQ 8  - RTC (CMOS)
no_error_code_interrupt_handler 41  ; IRQ 9  - Free
no_error_code_interrupt_handler 42  ; IRQ 10 - Free
no_error_code_interrupt_handler 43  ; IRQ 11 - Free
no_error_code_interrupt_handler 44  ; IRQ 12 - PS/2 Mouse
no_error_code_interrupt_handler 45  ; IRQ 13 - Coprocessor
no_error_code_interrupt_handler 46  ; IRQ 14 - Primary ATA
no_error_code_interrupt_handler 47  ; IRQ 15 - Secondary ATA
