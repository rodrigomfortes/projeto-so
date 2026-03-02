/* pic.c - Implementação do Programmable Interrupt Controller (Capítulo 6)
 * Referência: The Little OS Book - Chapter 6: Interrupts and Input
 *
 * O PIC (8259A) é o chip responsável por gerenciar interrupções de hardware.
 * Existem dois PICs encadeados:
 *   - PIC 1 (Master): IRQ 0-7
 *   - PIC 2 (Slave):  IRQ 8-15 (conectado à IRQ 2 do Master)
 *
 * Por padrão, o PIC mapeia as IRQs 0-15 para interrupções 0-15, o que
 * conflita com as exceções do CPU. Precisamos remapear para 32-47.
 */

#include "pic.h"
#include "io.h"

/* ICW (Initialization Command Words) para configurar o PIC */
#define ICW1_INIT       0x11    /* Modo de inicialização (ICW4 necessário) */
#define ICW4_8086       0x01    /* Modo 8086/88 */

/**
 * pic_remap - Remapeia as IRQs do PIC para interrupções 32-47
 *
 * Sequência de inicialização (ICW1 → ICW4):
 *   ICW1: Inicia a sequência de inicialização
 *   ICW2: Define o offset do vetor de interrupções
 *   ICW3: Configura cascata entre Master e Slave
 *   ICW4: Informações adicionais sobre o ambiente
 *
 * Após o remapeamento:
 *   IRQ 0-7  → Interrupções 32-39 (PIC1)
 *   IRQ 8-15 → Interrupções 40-47 (PIC2)
 */
void pic_remap(void)
{
    /* Salva as máscaras atuais */
    unsigned char mask1 = inb(PIC1_DATA);
    unsigned char mask2 = inb(PIC2_DATA);

    /* ICW1: Inicia a sequência de inicialização (modo cascata, ICW4 needed) */
    outb(PIC1_COMMAND, ICW1_INIT);
    outb(PIC2_COMMAND, ICW1_INIT);

    /* ICW2: Define os offsets dos vetores de interrupção */
    outb(PIC1_DATA, PIC1_START_INTERRUPT);  /* PIC1: IRQ 0-7  → INT 32-39 */
    outb(PIC2_DATA, PIC2_START_INTERRUPT);  /* PIC2: IRQ 8-15 → INT 40-47 */

    /* ICW3: Configura a cascata entre Master e Slave */
    outb(PIC1_DATA, 0x04);  /* Master: Slave está conectado na IRQ 2 (bit 2) */
    outb(PIC2_DATA, 0x02);  /* Slave: Identidade de cascata = 2 */

    /* ICW4: Modo 8086 */
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    /* Restaura as máscaras salvas */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

/**
 * pic_acknowledge - Envia acknowledge ao PIC
 *
 * Após tratar uma interrupção de hardware, devemos enviar o byte 0x20
 * ao PIC que gerou a interrupção. Sem isso, o PIC não gera mais IRQs.
 *
 * Se a interrupção veio do PIC2 (IRQ 8-15, INT 40-47), precisamos
 * enviar acknowledge tanto ao PIC2 quanto ao PIC1 (cascata).
 *
 * @param interrupt Número da interrupção (32-47)
 */
void pic_acknowledge(unsigned int interrupt)
{
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) {
        return;
    }

    /* Se a interrupção é do PIC2, envia ACK para o PIC2 também */
    if (interrupt >= PIC2_START_INTERRUPT) {
        outb(PIC2_COMMAND, PIC_ACK);
    }

    /* Sempre envia ACK para o PIC1 (master) */
    outb(PIC1_COMMAND, PIC_ACK);
}
