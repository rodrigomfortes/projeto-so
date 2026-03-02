/* idt.h - Definições da Interrupt Descriptor Table (Capítulo 6)
 * Referência: The Little OS Book - Chapter 6: Interrupts and Input
 */

#ifndef IDT_H
#define IDT_H

/* Número total de entradas na IDT (0-255) */
#define IDT_NUM_ENTRIES 256

/* Estrutura de uma entrada na IDT (8 bytes).
 *
 * Campos:
 *   - offset_low  (16 bits): bits 0-15 do endereço do handler
 *   - selector    (16 bits): seletor de segmento de código (0x08 = kernel code)
 *   - zero        (8 bits):  reservado, sempre 0
 *   - type_attr   (8 bits):  P(1) DPL(2) 0(1) D(1) 1 1 0 — tipo e atributos
 *   - offset_high (16 bits): bits 16-31 do endereço do handler
 */
struct idt_entry {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char  zero;
    unsigned char  type_attr;
    unsigned short offset_high;
} __attribute__((packed));

/* Ponteiro para a IDT, usado pela instrução lidt.
 *   - limit (16 bits): tamanho da IDT em bytes - 1
 *   - base  (32 bits): endereço linear da IDT
 */
struct idt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

/* Inicializa a IDT com os handlers de interrupção e a carrega no processador */
void idt_initialize(void);

#endif /* IDT_H */
