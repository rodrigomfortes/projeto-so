#ifndef MEMORY_H
#define MEMORY_H

#include "multiboot.h"

/*
 * Inicializa o Physical Memory Manager (PMM).
 * mbd: Ponteiro virtual para a estrutura multiboot_info_t do GRUB
 * kernel_start: Endereço virtual do início do kernel (símbolo do linker)
 * kernel_end: Endereço virtual do fim do kernel (símbolo do linker)
 */
void pmm_initialize(multiboot_info_t* mbd, unsigned int kernel_start, unsigned int kernel_end);

/*
 * Encontra um quadro de página (4KB) livre, marca como usado e o retorna.
 * Retorna: Endereço Físico do quadro alocado, ou 0 se sem memória.
 */
unsigned int pmm_alloc_page();

/*
 * Libera um quadro de página previamente alocado, permitindo reuso.
 * phys_addr: Endereço Físico da página de 4KB a ser liberada.
 */
void pmm_free_page(unsigned int phys_addr);

#endif /* MEMORY_H */
