/* pic.h - Definições do Programmable Interrupt Controller (Capítulo 6)
 * Referência: The Little OS Book - Chapter 6: Interrupts and Input
 */

#ifndef PIC_H
#define PIC_H

/* Portas de I/O dos PICs */
#define PIC1_COMMAND    0x20    /* PIC 1 (Master) - porta de comando */
#define PIC1_DATA       0x21    /* PIC 1 (Master) - porta de dados */
#define PIC2_COMMAND    0xA0    /* PIC 2 (Slave)  - porta de comando */
#define PIC2_DATA       0xA1    /* PIC 2 (Slave)  - porta de dados */

/* Offsets de remapeamento: IRQs do PIC serão mapeadas para esses números */
#define PIC1_START_INTERRUPT  0x20  /* IRQ 0-7  -> Interrupts 32-39 */
#define PIC2_START_INTERRUPT  0x28  /* IRQ 8-15 -> Interrupts 40-47 */
#define PIC2_END_INTERRUPT    (PIC2_START_INTERRUPT + 7)

/* Byte de acknowledge */
#define PIC_ACK         0x20

/* Remapeia as IRQs do PIC para evitar conflito com exceções do CPU (0-31) */
void pic_remap(void);

/* Envia acknowledge ao PIC após tratar uma interrupção de hardware */
void pic_acknowledge(unsigned int interrupt);

#endif /* PIC_H */
