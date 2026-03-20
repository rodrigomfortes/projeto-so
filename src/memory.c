#include "memory.h"
#include "serial.h"

/* Tamanho do Frame = 4KB (4096 bytes) */
#define PAGE_SIZE 4096

/* Gerenciaremos até 4GB de RAM física.
 * 4GB / 4KB = 1.048.576 páginas.
 * Como cada bit representa uma página, precisamos de 1.048.576 bits.
 * Dividindo em inteiros de 32 bits: 1048576 / 32 = 32768 inteiros.
 */
#define PMM_BITMAP_SIZE 32768

/* O Bitmap do PMM: 0 = Livre, 1 = Ocupado. */
static unsigned int pmm_bitmap[PMM_BITMAP_SIZE];

/* Funções auxiliares inlines para manipular os bits */
static inline void bitmap_set(int bit) {
    pmm_bitmap[bit / 32] |= (1 << (bit % 32));
}

static inline void bitmap_clear(int bit) {
    pmm_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static inline int bitmap_test(int bit) {
    return pmm_bitmap[bit / 32] & (1 << (bit % 32));
}

void pmm_initialize(multiboot_info_t* mbinfo, unsigned int kernel_start, unsigned int kernel_end) {
    int i;
    unsigned int addr;

    /* PASSO 1: Marcar precaucionamente toda a memória como ocupada (1) */
    for (i = 0; i < PMM_BITMAP_SIZE; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }

    /* PASSO 2: Usar o Multiboot para DESMARCAR (0) as áreas seguras de RAM livre */
    if (mbinfo->flags & MULTIBOOT_FLAG_MEMMAP) {
        serial_print("PMM: Multiboot Memory Map detectado!\n");

        /* ATENÇÃO: mmap_addr do GRUB é endereço físico, paginacão está ativa = somar base virtual! */
        unsigned int mmap_addr = mbinfo->mmap_addr + 0xC0000000;
        
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mmap_addr;
        
        while ((unsigned int)mmap < mmap_addr + mbinfo->mmap_length) {
            /* Type 1 significa "Available RAM" */
            if (mmap->type == 1) {
                unsigned int region_start = mmap->base_addr_low;
                unsigned int region_end = region_start + mmap->length_low;
                
                /* Limpamos os bits garantindo alinhamento de 4KB */
                for (addr = region_start; addr < region_end; addr += PAGE_SIZE) {
                    bitmap_clear(addr / PAGE_SIZE);
                }
            }
            /* Avança para a próxima entrada (size field + o próprio tamanho da field size) */
            mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
        }
    } else {
        serial_print("PMM: ERRO! GRUB bloqueou o Memory Map. Todo sistema travado como ocupado.\n");
    }

    /* PASSO 3: Re-bloquear a área exata onde NOSSO KERNEL está (para ninguém sobrescrever) 
     * KERNEL_START e KERNEL_END vêm do linker em endereços 0xC0100000. Precisamos do endereço físico.
     */
    unsigned int kernel_phys_start = kernel_start - 0xC0000000;
    unsigned int kernel_phys_end   = kernel_end - 0xC0000000;
    
    for (addr = kernel_phys_start; addr <= kernel_phys_end; addr += PAGE_SIZE) {
        bitmap_set(addr / PAGE_SIZE);
    }
    
    /* PASSO 4: Re-bloquear as áreas de módulos do Multiboot (se existirem programas externos) */
    if (mbinfo->flags & MULTIBOOT_FLAG_MODS) {
        if (mbinfo->mods_count > 0) {
            multiboot_mod_t* mods = (multiboot_mod_t*) (mbinfo->mods_addr + 0xC0000000);
            unsigned int mod_idx;
            for (mod_idx = 0; mod_idx < mbinfo->mods_count; mod_idx++) {
                for (addr = mods[mod_idx].mod_start; addr <= mods[mod_idx].mod_end; addr += PAGE_SIZE) {
                    bitmap_set(addr / PAGE_SIZE);
                }
            }
        }
    }

    /* Padrão de segurança: Manter endereço físico O (0x0) ocupado para evitar Null Pointers no hardware */
    bitmap_set(0);

    serial_print("PMM: Gerenciador de RAM Fisica Inicializado via Bitmap.\n");
}

unsigned int pmm_alloc_page() {
    int i, j, bit;

    /* Varre o array de inteiros */
    for (i = 0; i < PMM_BITMAP_SIZE; i++) {
        /* Se o chunk inteiro for 0xFFFFFFFF, todos os 32 frames desse bloco estão ocupados */
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            /* Alguma página aqui tá livre. Vamos varrer os 32 bits para achar A primeira que é 0 */
            for (j = 0; j < 32; j++) {
                bit = i * 32 + j;
                if (!bitmap_test(bit)) {
                    bitmap_set(bit);
                    return bit * PAGE_SIZE; /* Retorna o ENDEREÇO FÍSICO da alocação! */
                }
            }
        }
    }
    
    /* Out of memory */
    serial_print("PMM PANIC: Out of physical memory!\n");
    return 0;
}

void pmm_free_page(unsigned int phys_addr) {
    int bit = phys_addr / PAGE_SIZE;
    bitmap_clear(bit);
}
