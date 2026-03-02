#ifndef IO_H
#define IO_H

/* Função em Assembly para enviar dados à porta de I/O */
void outb(unsigned short port, unsigned char data);

/* Função em Assembly para ler dados de uma porta de I/O */
unsigned char inb(unsigned short port);

#endif /* IO_H */
