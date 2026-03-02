/* keyboard.h - Definições do driver de teclado (Capítulo 6)
 * Referência: The Little OS Book - Chapter 6: Interrupts and Input
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Porta de dados do teclado */
#define KBD_DATA_PORT   0x60

/* Lê um scan code do teclado (NÃO é um caractere ASCII!) */
unsigned char read_scan_code(void);

/* Handler chamado quando a interrupção do teclado (IRQ 1) ocorre */
void keyboard_handler(void);

#endif /* KEYBOARD_H */
