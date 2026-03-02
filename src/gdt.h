/* gdt.h - Definições da Global Descriptor Table (Capítulo 5)
 * Referência: The Little OS Book - Chapter 5: Segmentation
 */

#ifndef GDT_H
#define GDT_H

/* Estrutura de uma entrada (descritor de segmento) na GDT.
 * Cada entrada tem 8 bytes e descreve um segmento de memória.
 *
 * Campos do descritor (64 bits no total):
 *   - limit_low  (16 bits): bits 0-15 do limite do segmento
 *   - base_low   (16 bits): bits 0-15 do endereço base
 *   - base_mid   (8 bits):  bits 16-23 do endereço base
 *   - access     (8 bits):  flags de acesso (tipo, DPL, presente)
 *   - granularity (8 bits): bits 16-19 do limite + flags (G, D/B, L, AVL)
 *   - base_high  (8 bits):  bits 24-31 do endereço base
 */
struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_mid;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

/* Estrutura do ponteiro para a GDT, usada pela instrução lgdt.
 * Contém o tamanho (limit) e o endereço base da tabela.
 *
 *   - limit (16 bits): tamanho da GDT em bytes - 1
 *   - base  (32 bits): endereço linear da GDT
 */
struct gdt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

/* Inicializa a GDT com os descritores de segmento do kernel.
 * Configura:
 *   - Null descriptor  (índice 0x00)
 *   - Kernel Code Seg  (índice 0x08, DPL=0, Execute/Read)
 *   - Kernel Data Seg  (índice 0x10, DPL=0, Read/Write)
 *
 * Após preencher a tabela, carrega-a no processador via lgdt.
 */
void gdt_initialize(void);

#endif /* GDT_H */
