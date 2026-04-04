#ifndef CHAT_H
#define CHAT_H

/*
 * Chat multiusuário no kernel (sem TCP):
 *   - "vga"  : linhas montadas a partir do teclado (IRQ → fila → main)
 *   - "serial": linhas lidas na COM1 (polling no main)
 *
 * A saída (framebuffer + serial) é feita numa secção crítica cli/sti,
 * análoga a um mutex em núcleo único.
 */

void chat_init(void);

/* Chamada a partir do handler de teclado (IRQ) — só enfileira bytes */
void chat_kbd_enqueue(unsigned char c);

/* Ciclo principal: drena teclado e serial, envia linhas completas */
void chat_poll(void);

#endif /* CHAT_H */
