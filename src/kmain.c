#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "multiboot.h"

/*
 * kmain - Ponto de entrada do kernel em C
 * ebx: ponteiro para a struct multiboot_info_t preenchida pelo GRUB (Capítulo 7)
 */
void kmain(unsigned int ebx)
{
    /* Inicializa as portas seriais COM1 para o arquivo de log do emulador */
    serial_initialize();
    serial_print("Porta Serial (COM1) Inicializada com sucesso!\n");
    serial_print("Kernel do Little OS Book - Teste do Capitulo 7\n");

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

    serial_print("Interrupcoes habilitadas (sti).\n");

    /* === Capítulo 7: Carregamento de Programa Externo via Módulo GRUB ===
     *
     * O GRUB armazena um ponteiro para a struct multiboot_info_t em EBX.
     * Ela descreve os módulos carregados na memória.
     * Verificamos o bit 3 do campo flags para confirmar que mods_count
     * e mods_addr são válidos antes de tentar saltar para o programa.
     */
    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    if (mbinfo->flags & MULTIBOOT_FLAG_MODS) {
        serial_print("Modulos multiboot detectados.\n");

        if (mbinfo->mods_count == 1) {
            multiboot_mod_t *mods = (multiboot_mod_t *) mbinfo->mods_addr;
            unsigned int program_addr = mods->mod_start;

            console_write_colored("\nModulo carregado! Saltando para o programa...\n",
                                  CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);
            serial_print("Saltando para o programa em modo kernel...\n");

            /* Converte o endereço do módulo em ponteiro de função e executa */
            typedef void (*call_module_t)(void);
            call_module_t start_program = (call_module_t) program_addr;
            start_program();
            /* Nunca chegamos aqui, pois o programa entra em loop infinito */
        } else {
            serial_print("AVISO: mods_count != 1, abortando salto.\n");
            console_write_colored("\nAVISO: numero de modulos inesperado!\n",
                                  CONSOLE_LIGHT_RED, CONSOLE_BLACK);
        }
    } else {
        serial_print("Nenhum modulo multiboot encontrado. Aguardando teclado...\n");
        console_write_colored("\nNenhum modulo carregado. Aguardando input...\n",
                              CONSOLE_MAGENTA, CONSOLE_BLACK);
    }

    /* Loop de espera para não travar a CPU */
    while (1) {
    }
}

