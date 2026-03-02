; gdt_flush.s - Carrega a GDT e atualiza os registradores de segmento (Capítulo 5)
; Referência: The Little OS Book - Chapter 5: Segmentation
;
; Esta rotina executa a instrução lgdt para carregar a GDT no processador
; e em seguida atualiza todos os registradores de segmento:
;   - cs  via far jump (não pode ser modificado com mov)
;   - ds, ss, es, fs, gs via mov
;
; void gdt_flush(unsigned int gdt_ptr_addr);

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]          ; carrega o endereço do struct gdt_ptr
    lgdt [eax]                  ; executa lgdt para carregar a GDT no processador

    ; Carrega 0x10 (offset do Kernel Data Segment) nos registradores de dados
    mov ax, 0x10                ; 0x10 = offset da 3ª entrada (data segment)
    mov ds, ax                  ; data segment
    mov es, ax                  ; extra segment
    mov fs, ax                  ; general purpose segment
    mov gs, ax                  ; general purpose segment
    mov ss, ax                  ; stack segment

    ; Para atualizar cs, precisamos de um far jump.
    ; O far jump especifica o seletor de segmento (0x08 = kernel code segment)
    ; e o endereço absoluto do label de destino.
    jmp 0x08:.flush_cs          ; far jump: cs = 0x08, EIP = .flush_cs

.flush_cs:
    ; Agora cs contém 0x08 (Kernel Code Segment)
    ; Todos os registradores de segmento estão configurados corretamente
    ret
