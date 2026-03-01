#include "serial.h"
#include "io.h"

/* Porta Padrão COM1 usada por muitos emuladores (incluindo Bochs) */
#define COM1_BASE 0x3F8

/* Registradores COM1 e seus offsets correspondentes */
#define UART_DATA        (COM1_BASE + 0)
#define UART_INT_EN      (COM1_BASE + 1)
#define UART_FIFO_CTRL   (COM1_BASE + 2)
#define UART_LINE_CTRL   (COM1_BASE + 3)
#define UART_MODEM_CTRL  (COM1_BASE + 4)
#define UART_LINE_STAT   (COM1_BASE + 5)

/**
 * Procedimentos para preparar a porta com a taxa de bauds necessária
 * 115200 divisor 1, sem verificação de paridade.
 */
void serial_initialize() {
    outb(UART_INT_EN, 0x00);         // Primeiro passo: desabilita as interrupções de hardware associadas
    
    // Habilita a programação do "divisor de bauds", bit mais significativo=1
    outb(UART_LINE_CTRL, 0x80); 
    
    // Configura o baud para os famigerados 115200 (1 divisor)   
    outb(UART_DATA, 0x01);           // LSB = 1 (Porta 0x3F8)
    outb(UART_INT_EN, 0x00);         // MSB = 0 (Porta 0x3F9 quando DLAB está ativo)
    
    // Data Length = 8 bits, bit de parada e paridade vazios
    outb(UART_LINE_CTRL, 0x03);
    
    // Configura buffers de dados FIFO de transmissão
    outb(UART_FIFO_CTRL, 0xC7);
    
    // Informa ao hardware que o dataterminal está pronto para o fluxo
    outb(UART_MODEM_CTRL, 0x03); 
}

/**
 * Booleano simples que olha para o pino de estado bit 5.
 */
static int is_transmit_buffer_ready() {
    // 0x20 é a máscara padrão de FIFO check
    return inb(UART_LINE_STAT) & 0x20;
}

/**
 * Escreve literalmente só um caracter, não destrava o terminal.
 */
static void serial_write_char(char x) {
    while (is_transmit_buffer_ready() == 0) {
        // Rotina de spinlock
    }
    outb(UART_DATA, x);
}

/**
 * Manda a string toda rodando a serial frame por frame.
 */
void serial_print(char *mensagem) {
    int current = 0;
    while (mensagem[current] != '\0') {
        serial_write_char(mensagem[current]);
        current++;
    }
}
