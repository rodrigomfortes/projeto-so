#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"

/*
 * kmain - Ponto de entrada do kernel em C
 */
void kmain(void)
{
    /* Inicializa as portas seriais COM1 para o arquivo de log do emulador */
    serial_initialize();
    serial_print("Porta Serial (COM1) Inicializada com sucesso!\n");
    serial_print("Kernel do Little OS Book - Teste do Capitulo 5\n");

    /* Inicializa a GDT (Global Descriptor Table) - Capítulo 5
     * Configura os segmentos de memória do kernel:
     *   0x00 - Null Descriptor
     *   0x08 - Kernel Code Segment (DPL=0, Execute/Read)
     *   0x10 - Kernel Data Segment (DPL=0, Read/Write)
     */
    gdt_initialize();
    serial_print("GDT (Global Descriptor Table) carregada com sucesso!\n");

    /* Limpa a tela do terminal (VGA text mode) */
    console_clear();

    /* Escreve a primeira mensagem nativamente na memoria de video (0x000B8000) */
    console_write_colored("Hello World!\n\n", CONSOLE_LIGHT_MAGENTA, CONSOLE_BLACK);
    console_write_colored("Finalmente deu certo!\n", CONSOLE_MAGENTA, CONSOLE_BLACK);
    console_write_colored("Usando o framebuffer vga...!\n", CONSOLE_LIGHT_MAGENTA, CONSOLE_BLACK);

    /* Mensagem de confirmação da GDT (Capítulo 5) */
    console_write_colored("GDT carregada com sucesso!\n", CONSOLE_MAGENTA, CONSOLE_BLACK);

    serial_print("Mensagens enviadas para o framebuffer.\n");

    /* Loop de espera para não travar a CPU */
    while (1) {
    }
}
