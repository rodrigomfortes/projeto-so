#ifndef SERIAL_H
#define SERIAL_H

/* Inicializa a porta serial para debugar dados de log */
void serial_initialize();

/* Envia uma string formatada simples pelo canal serial */
void serial_print(char *mensagem);

#endif /* SERIAL_H */
