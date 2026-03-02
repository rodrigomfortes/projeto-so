/* gdt.c - Implementação da Global Descriptor Table (Capítulo 5)
 * Referência: The Little OS Book - Chapter 5: Segmentation
 *
 * A GDT é uma tabela que descreve os segmentos de memória do sistema.
 * Cada entrada define um segmento com endereço base, limite e permissões.
 *
 * No nosso setup mínimo, usamos a segmentação apenas para definir níveis
 * de privilégio (ring 0 para o kernel). Os segmentos de código e dados
 * cobrem todo o espaço de endereçamento linear (0x00000000 - 0xFFFFFFFF).
 */

#include "gdt.h"

/* Número de entradas na GDT: null + kernel code + kernel data */
#define GDT_NUM_ENTRIES 3

/* Tabela GDT e ponteiro para lgdt */
static struct gdt_entry gdt_entries[GDT_NUM_ENTRIES];
static struct gdt_ptr   gdt_pointer;

/* Função externa em Assembly que executa lgdt e atualiza os registradores
 * de segmento (cs, ds, ss, es, fs, gs). Definida em gdt_flush.s.
 */
extern void gdt_flush(unsigned int gdt_ptr_addr);

/**
 * gdt_set_entry - Preenche uma entrada da GDT
 *
 * @param index       Índice da entrada na tabela (0, 1, 2, ...)
 * @param base        Endereço base do segmento (32 bits)
 * @param limit       Limite do segmento (20 bits, em unidades de 4KB se G=1)
 * @param access      Byte de acesso (Present, DPL, Type)
 * @param granularity Nibble superior de granularidade (G, D/B, L, AVL)
 *
 * O campo limit é dividido entre limit_low (bits 0-15) e os 4 bits
 * inferiores de granularity (bits 16-19). O campo base é dividido
 * entre base_low, base_mid e base_high.
 */
static void gdt_set_entry(int index,
                           unsigned int base,
                           unsigned int limit,
                           unsigned char access,
                           unsigned char granularity)
{
    /* Endereço base: dividido em 3 partes */
    gdt_entries[index].base_low  = (base & 0xFFFF);
    gdt_entries[index].base_mid  = (base >> 16) & 0xFF;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;

    /* Limite: bits 0-15 no campo limit_low, bits 16-19 no nibble inferior
     * do campo granularity */
    gdt_entries[index].limit_low   = (limit & 0xFFFF);
    gdt_entries[index].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);

    /* Byte de acesso (tipo, DPL, presente, etc.) */
    gdt_entries[index].access = access;
}

/**
 * gdt_initialize - Monta e carrega a GDT no processador
 *
 * Configura 3 descritores de segmento:
 *
 *   Índice 0x00: Null Descriptor
 *     Obrigatório pela especificação x86. Todos os campos são zero.
 *
 *   Índice 0x08: Kernel Code Segment
 *     Base  = 0x00000000
 *     Limit = 0xFFFFFFFF (4GB)
 *     Access = 0x9A:
 *       P=1 (presente), DPL=00 (ring 0), S=1 (code/data),
 *       Type=1010 (Execute/Read)
 *     Granularity = 0xCF:
 *       G=1 (4KB pages), D=1 (32-bit), Limit[19:16]=0xF
 *
 *   Índice 0x10: Kernel Data Segment
 *     Base  = 0x00000000
 *     Limit = 0xFFFFFFFF (4GB)
 *     Access = 0x92:
 *       P=1 (presente), DPL=00 (ring 0), S=1 (code/data),
 *       Type=0010 (Read/Write)
 *     Granularity = 0xCF:
 *       G=1 (4KB pages), D=1 (32-bit), Limit[19:16]=0xF
 *
 * Após preencher a tabela, configura o ponteiro (limit + base) e chama
 * gdt_flush() para executar lgdt e atualizar os registradores de segmento.
 */
void gdt_initialize(void)
{
    /* Configura o ponteiro da GDT para lgdt */
    gdt_pointer.limit = (sizeof(struct gdt_entry) * GDT_NUM_ENTRIES) - 1;
    gdt_pointer.base  = (unsigned int)&gdt_entries;

    /* Entrada 0: Null Descriptor (obrigatório) */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Entrada 1: Kernel Code Segment (0x08) */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Entrada 2: Kernel Data Segment (0x10) */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Carrega a GDT no processador e atualiza os registradores de segmento */
    gdt_flush((unsigned int)&gdt_pointer);
}
