/* idt.c - Implementação da Interrupt Descriptor Table (Capítulo 6)
 * Referência: The Little OS Book - Chapter 6: Interrupts and Input
 *
 * A IDT é uma tabela de 256 entradas que mapeia cada número de interrupção
 * (0-255) a um handler (rotina de tratamento). Quando uma interrupção ocorre,
 * o processador consulta esta tabela para saber qual código executar.
 *
 * Interrupções 0-31:  Exceções do CPU (ex: divisão por zero, page fault)
 * Interrupções 32-47: IRQs do hardware (remapeadas pelo PIC)
 * Interrupções 48-255: Disponíveis para software (system calls, etc.)
 */

#include "idt.h"

/* Tabela IDT e ponteiro para lidt */
static struct idt_entry idt_entries[IDT_NUM_ENTRIES];
static struct idt_ptr   idt_pointer;

/* Função externa em Assembly que executa lidt. Definida em interrupt_handlers.s */
extern void load_idt(unsigned int idt_ptr_addr);

/* Endereços dos handlers de interrupção gerados pelas macros em Assembly.
 * Cada interrupt_handler_N é um label global definido em interrupt_handlers.s.
 */
extern void interrupt_handler_0(void);
extern void interrupt_handler_1(void);
extern void interrupt_handler_2(void);
extern void interrupt_handler_3(void);
extern void interrupt_handler_4(void);
extern void interrupt_handler_5(void);
extern void interrupt_handler_6(void);
extern void interrupt_handler_7(void);
extern void interrupt_handler_8(void);
extern void interrupt_handler_9(void);
extern void interrupt_handler_10(void);
extern void interrupt_handler_11(void);
extern void interrupt_handler_12(void);
extern void interrupt_handler_13(void);
extern void interrupt_handler_14(void);
extern void interrupt_handler_15(void);
extern void interrupt_handler_16(void);
extern void interrupt_handler_17(void);
extern void interrupt_handler_18(void);
extern void interrupt_handler_19(void);
extern void interrupt_handler_20(void);
extern void interrupt_handler_21(void);
extern void interrupt_handler_22(void);
extern void interrupt_handler_23(void);
extern void interrupt_handler_24(void);
extern void interrupt_handler_25(void);
extern void interrupt_handler_26(void);
extern void interrupt_handler_27(void);
extern void interrupt_handler_28(void);
extern void interrupt_handler_29(void);
extern void interrupt_handler_30(void);
extern void interrupt_handler_31(void);

/* IRQs 0-15 remapeadas para interrupções 32-47 */
extern void interrupt_handler_32(void);
extern void interrupt_handler_33(void);
extern void interrupt_handler_34(void);
extern void interrupt_handler_35(void);
extern void interrupt_handler_36(void);
extern void interrupt_handler_37(void);
extern void interrupt_handler_38(void);
extern void interrupt_handler_39(void);
extern void interrupt_handler_40(void);
extern void interrupt_handler_41(void);
extern void interrupt_handler_42(void);
extern void interrupt_handler_43(void);
extern void interrupt_handler_44(void);
extern void interrupt_handler_45(void);
extern void interrupt_handler_46(void);
extern void interrupt_handler_47(void);

/**
 * idt_set_entry - Preenche uma entrada na IDT
 *
 * @param index     Índice na tabela (0-255)
 * @param handler   Endereço do handler (função assembly)
 * @param selector  Seletor de segmento de código (0x08 = kernel code segment)
 * @param type_attr Byte de tipo e atributos (P, DPL, tipo)
 */
static void idt_set_entry(int index,
                           unsigned int handler,
                           unsigned short selector,
                           unsigned char type_attr)
{
    idt_entries[index].offset_low  = handler & 0xFFFF;
    idt_entries[index].offset_high = (handler >> 16) & 0xFFFF;
    idt_entries[index].selector    = selector;
    idt_entries[index].zero        = 0;
    idt_entries[index].type_attr   = type_attr;
}

/**
 * idt_initialize - Configura e carrega a IDT no processador
 *
 * Registra handlers para:
 *   - ISR 0-31:  Exceções do CPU
 *   - IRQ 32-47: Hardware interrupts (PIC)
 *
 * type_attr = 0x8E:
 *   P=1 (presente), DPL=00 (ring 0), D=1 (32-bit), Type=110 (interrupt gate)
 *
 * Selector = 0x08 (Kernel Code Segment da GDT)
 */
void idt_initialize(void)
{
    unsigned int i;

    /* Configura o ponteiro da IDT para lidt */
    idt_pointer.limit = (sizeof(struct idt_entry) * IDT_NUM_ENTRIES) - 1;
    idt_pointer.base  = (unsigned int)&idt_entries;

    /* Zera todas as entradas */
    for (i = 0; i < IDT_NUM_ENTRIES; i++) {
        idt_set_entry(i, 0, 0, 0);
    }

    /* ISR 0-31: Exceções do CPU */
    idt_set_entry(0,  (unsigned int)interrupt_handler_0,  0x08, 0x8E);
    idt_set_entry(1,  (unsigned int)interrupt_handler_1,  0x08, 0x8E);
    idt_set_entry(2,  (unsigned int)interrupt_handler_2,  0x08, 0x8E);
    idt_set_entry(3,  (unsigned int)interrupt_handler_3,  0x08, 0x8E);
    idt_set_entry(4,  (unsigned int)interrupt_handler_4,  0x08, 0x8E);
    idt_set_entry(5,  (unsigned int)interrupt_handler_5,  0x08, 0x8E);
    idt_set_entry(6,  (unsigned int)interrupt_handler_6,  0x08, 0x8E);
    idt_set_entry(7,  (unsigned int)interrupt_handler_7,  0x08, 0x8E);
    idt_set_entry(8,  (unsigned int)interrupt_handler_8,  0x08, 0x8E);
    idt_set_entry(9,  (unsigned int)interrupt_handler_9,  0x08, 0x8E);
    idt_set_entry(10, (unsigned int)interrupt_handler_10, 0x08, 0x8E);
    idt_set_entry(11, (unsigned int)interrupt_handler_11, 0x08, 0x8E);
    idt_set_entry(12, (unsigned int)interrupt_handler_12, 0x08, 0x8E);
    idt_set_entry(13, (unsigned int)interrupt_handler_13, 0x08, 0x8E);
    idt_set_entry(14, (unsigned int)interrupt_handler_14, 0x08, 0x8E);
    idt_set_entry(15, (unsigned int)interrupt_handler_15, 0x08, 0x8E);
    idt_set_entry(16, (unsigned int)interrupt_handler_16, 0x08, 0x8E);
    idt_set_entry(17, (unsigned int)interrupt_handler_17, 0x08, 0x8E);
    idt_set_entry(18, (unsigned int)interrupt_handler_18, 0x08, 0x8E);
    idt_set_entry(19, (unsigned int)interrupt_handler_19, 0x08, 0x8E);
    idt_set_entry(20, (unsigned int)interrupt_handler_20, 0x08, 0x8E);
    idt_set_entry(21, (unsigned int)interrupt_handler_21, 0x08, 0x8E);
    idt_set_entry(22, (unsigned int)interrupt_handler_22, 0x08, 0x8E);
    idt_set_entry(23, (unsigned int)interrupt_handler_23, 0x08, 0x8E);
    idt_set_entry(24, (unsigned int)interrupt_handler_24, 0x08, 0x8E);
    idt_set_entry(25, (unsigned int)interrupt_handler_25, 0x08, 0x8E);
    idt_set_entry(26, (unsigned int)interrupt_handler_26, 0x08, 0x8E);
    idt_set_entry(27, (unsigned int)interrupt_handler_27, 0x08, 0x8E);
    idt_set_entry(28, (unsigned int)interrupt_handler_28, 0x08, 0x8E);
    idt_set_entry(29, (unsigned int)interrupt_handler_29, 0x08, 0x8E);
    idt_set_entry(30, (unsigned int)interrupt_handler_30, 0x08, 0x8E);
    idt_set_entry(31, (unsigned int)interrupt_handler_31, 0x08, 0x8E);

    /* IRQ 0-15: Hardware interrupts (remapeadas para 32-47 pelo PIC) */
    idt_set_entry(32, (unsigned int)interrupt_handler_32, 0x08, 0x8E);
    idt_set_entry(33, (unsigned int)interrupt_handler_33, 0x08, 0x8E);
    idt_set_entry(34, (unsigned int)interrupt_handler_34, 0x08, 0x8E);
    idt_set_entry(35, (unsigned int)interrupt_handler_35, 0x08, 0x8E);
    idt_set_entry(36, (unsigned int)interrupt_handler_36, 0x08, 0x8E);
    idt_set_entry(37, (unsigned int)interrupt_handler_37, 0x08, 0x8E);
    idt_set_entry(38, (unsigned int)interrupt_handler_38, 0x08, 0x8E);
    idt_set_entry(39, (unsigned int)interrupt_handler_39, 0x08, 0x8E);
    idt_set_entry(40, (unsigned int)interrupt_handler_40, 0x08, 0x8E);
    idt_set_entry(41, (unsigned int)interrupt_handler_41, 0x08, 0x8E);
    idt_set_entry(42, (unsigned int)interrupt_handler_42, 0x08, 0x8E);
    idt_set_entry(43, (unsigned int)interrupt_handler_43, 0x08, 0x8E);
    idt_set_entry(44, (unsigned int)interrupt_handler_44, 0x08, 0x8E);
    idt_set_entry(45, (unsigned int)interrupt_handler_45, 0x08, 0x8E);
    idt_set_entry(46, (unsigned int)interrupt_handler_46, 0x08, 0x8E);
    idt_set_entry(47, (unsigned int)interrupt_handler_47, 0x08, 0x8E);

    /* Carrega a IDT no processador */
    load_idt((unsigned int)&idt_pointer);
}
