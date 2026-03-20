#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "multiboot.h"
#include "memory.h"

/* Símbolos definidos no linker script (link.ld) */
extern unsigned int KERNEL_START;
extern unsigned int KERNEL_END;

/*
 * kmain - Ponto de entrada do nosso kernel
 * Recebemos em ebx o ponteiro da struct multiboot_info_t do GRUB 
 * (já ajustado para endereço virtual.)
 */
void kmain(unsigned int ebx)
{
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

    /*
     * Lendo os módulos carregados pelo GRUB.
     * Como ativamos a paginação, todos os endereços físicos (mods_addr, mod_start)
     * precisam receber um offset de 0xC0000000 pra dar match na memória virtual.
     */
    #define KERNEL_VIRTUAL_BASE 0xC0000000

    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    /* Inicializa o Physical Memory Manager (Capítulo 10)
     * Passamos pra ele a struct do Multiboot pra ele ler a RAM, e mostramos
     * onde o nosso binário do Kernel de fato começa e termina!
     */
    pmm_initialize(mbinfo, (unsigned int)&KERNEL_START, (unsigned int)&KERNEL_END);
    
    serial_print("Teste PMM: Alocando primeira pagina 4KB... ");
    unsigned int test_page = pmm_alloc_page();
    if (test_page > 0) {
        serial_print("SUCESSO!\n");
    } else {
        serial_print("FALHA!\n");
    }

    if (mbinfo->flags & MULTIBOOT_FLAG_MODS) {
        serial_print("Modulo externo encontrado via GRUB!\n");

        if (mbinfo->mods_count == 1) {
            multiboot_mod_t *mods = (multiboot_mod_t *) (mbinfo->mods_addr + KERNEL_VIRTUAL_BASE);
            unsigned int program_addr = mods->mod_start + KERNEL_VIRTUAL_BASE;

            console_write_colored("\nModulo carregado! Disparando programa...\n",
                                  CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);
            serial_print("Saltando para o programa em modo kernel...\n");

            /* Desabilita interrupções antes de saltar para o programa externo.
             * Isso garante que nenhuma IRQ (teclado, timer) sobrescreva os
             * registradores enquanto o programa roda seu loop infinito (jmp $).
             * Assim, ao pausar o Bochs, eax conterá 0xDEADBEEF conforme esperado.
             */
            __asm__ volatile ("cli");

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

