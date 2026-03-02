#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"

/*
 * kmain - Ponto de entrada do kernel em C
 */
void kmain(void)
{
    /* Inicializa as portas seriais COM1 para o arquivo de log do emulador */
    serial_initialize();
    serial_print("Porta Serial (COM1) Inicializada com sucesso!\n");
    serial_print("Kernel do Little OS Book - Teste do Capitulo 6\n");

    /* Inicializa a GDT (Global Descriptor Table) - Capítulo 5
     * Configura os segmentos de memória do kernel:
     *   0x00 - Null Descriptor
     *   0x08 - Kernel Code Segment (DPL=0, Execute/Read)
     *   0x10 - Kernel Data Segment (DPL=0, Read/Write)
     */
    gdt_initialize();
    serial_print("GDT (Global Descriptor Table) carregada com sucesso!\n");

    /* Remapeia o PIC (Programmable Interrupt Controller) - Capítulo 6
     * IRQ 0-7  → Interrupções 32-39 (PIC1)
     * IRQ 8-15 → Interrupções 40-47 (PIC2)
     */
    pic_remap();
    serial_print("PIC remapeado (IRQs 0-15 -> INT 32-47)\n");

    /* Inicializa a IDT (Interrupt Descriptor Table) - Capítulo 6
     * Registra handlers para ISR 0-31 e IRQ 32-47
     */
    idt_initialize();
    serial_print("IDT (Interrupt Descriptor Table) carregada com sucesso!\n");

    /* Limpa a tela do terminal (VGA text mode) */
    console_clear();

    /* Escreve a primeira mensagem nativamente na memoria de video (0x000B8000) */
    console_write_colored("Hello World!\n\n", CONSOLE_LIGHT_MAGENTA, CONSOLE_BLACK);
    console_write_colored("Finalmente deu certo!\n", CONSOLE_MAGENTA, CONSOLE_BLACK);
    console_write_colored("Usando o framebuffer vga...!\n", CONSOLE_LIGHT_MAGENTA, CONSOLE_BLACK);

    /* Mensagem de confirmação da GDT (Capítulo 5) */
    console_write_colored("GDT carregada com sucesso!\n", CONSOLE_MAGENTA, CONSOLE_BLACK);

    /* Mensagens de confirmação do Capítulo 6 */
    console_write_colored("IDT e PIC inicializados!\n", CONSOLE_LIGHT_MAGENTA, CONSOLE_BLACK);
    console_write_colored("Teclado habilitado - digite algo!\n", CONSOLE_MAGENTA, CONSOLE_BLACK);

    serial_print("Mensagens enviadas para o framebuffer.\n");

    /* Habilita interrupções (sti = Set Interrupt Flag) - Capítulo 6
     * A partir daqui, o processador começa a receber interrupções
     * do hardware (teclado, timer, etc.)
     */
    __asm__ volatile ("sti");

    serial_print("Interrupcoes habilitadas (sti). Aguardando input do teclado...\n");

    /* Loop de espera para não travar a CPU */
    while (1) {
    }
}

