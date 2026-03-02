/* keyboard.c - Implementação do driver de teclado (Capítulo 6)
 * Referência: The Little OS Book - Chapter 6: Interrupts and Input
 *
 * O teclado não envia caracteres ASCII diretamente. Ele gera scan codes
 * que representam teclas pressionadas (make) e liberadas (break).
 *
 * Usamos a tabela de scan codes do Set 1 (compatível com XT) para
 * traduzir os scan codes em caracteres ASCII.
 *
 * Scan codes com bit 7 = 1 indicam que a tecla foi liberada (break code).
 * Scan codes com bit 7 = 0 indicam que a tecla foi pressionada (make code).
 */

#include "keyboard.h"
#include "io.h"
#include "framebuffer.h"
#include "serial.h"

/* Tabela de tradução: Scan Code Set 1 → ASCII (teclas principais)
 * Índice = scan code, Valor = caractere ASCII (0 = tecla não mapeada)
 *
 * Cobre as teclas mais comuns de um teclado US-QWERTY:
 *   - Letras a-z (minúsculas)
 *   - Números 0-9
 *   - Espaço, Enter, Tab, Backspace
 *   - Pontuação básica
 */
static const char scancode_to_ascii[128] = {
    0,    0,   '1', '2', '3', '4', '5', '6',   /* 0x00 - 0x07 */
    '7', '8', '9', '0', '-', '=',  0,    0,     /* 0x08 - 0x0F (0x0E=Backspace, 0x0F=Tab) */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',   /* 0x10 - 0x17 */
    'o', 'p', '[', ']',  '\n', 0,  'a', 's',   /* 0x18 - 0x1F (0x1C=Enter, 0x1D=LCtrl) */
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 0x20 - 0x27 */
    '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v',  /* 0x28 - 0x2F (0x2A=LShift) */
    'b', 'n', 'm', ',', '.', '/',  0,   '*',   /* 0x30 - 0x37 (0x36=RShift, 0x37=Keypad *) */
    0,   ' ',  0,   0,   0,   0,   0,   0,     /* 0x38 - 0x3F (0x38=LAlt, 0x39=Space) */
    0,    0,   0,   0,   0,   0,   0,   0,     /* 0x40 - 0x47 */
    0,    0,   0,   0,   0,   0,   0,   0,     /* 0x48 - 0x4F */
    0,    0,   0,   0,   0,   0,   0,   0,     /* 0x50 - 0x57 */
    0,    0,   0,   0,   0,   0,   0,   0,     /* 0x58 - 0x5F */
    0,    0,   0,   0,   0,   0,   0,   0,     /* 0x60 - 0x67 */
    0,    0,   0,   0,   0,   0,   0,   0,     /* 0x68 - 0x6F */
    0,    0,   0,   0,   0,   0,   0,   0,     /* 0x70 - 0x77 */
    0,    0,   0,   0,   0,   0,   0,   0      /* 0x78 - 0x7F */
};

/**
 * read_scan_code - Lê um scan code da porta de dados do teclado
 *
 * @return O scan code (NÃO é um caractere ASCII)
 */
unsigned char read_scan_code(void)
{
    return inb(KBD_DATA_PORT);
}

/**
 * keyboard_handler - Trata a interrupção do teclado (IRQ 1)
 *
 * Lê o scan code, traduz para ASCII (se possível) e:
 *   - Exibe o caractere no framebuffer VGA
 *   - Envia o caractere pela porta serial para log
 *
 * Ignora break codes (tecla liberada, bit 7 = 1).
 */
void keyboard_handler(void)
{
    unsigned char scan_code = read_scan_code();

    /* Ignora break codes (tecla liberada) */
    if (scan_code & 0x80) {
        return;
    }

    /* Traduz scan code para ASCII */
    char ascii = scancode_to_ascii[scan_code];

    if (ascii != 0) {
        /* Exibe no framebuffer */
        char str[2];
        str[0] = ascii;
        str[1] = '\0';
        console_write(str);

        /* Envia pela serial para log */
        serial_print(str);
    }
}
