#ifndef SERIAL_H
#define SERIAL_H

/* Inicializa a porta serial para debugar dados de log */
void serial_initialize();

/* Envia uma string formatada simples pelo canal serial */
void serial_print(char *mensagem);

/* Leitura não bloqueante (COM1) — bit Data Ready no LSR */
int serial_rx_ready(void);

unsigned char serial_read_byte(void);

#endif /* SERIAL_H */
