/* multiboot.h - Estrutura Multiboot para leitura de módulos (Capítulo 7)
 * Referência: The Little OS Book - Chapter 7 / GNU Multiboot Specification
 */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

/* Bit 3 do campo flags: mods_count e mods_addr são válidos */
#define MULTIBOOT_FLAG_MODS 0x8

/* Bit 6 do campo flags: mmap_length e mmap_addr são válidos (Capítulo 10) */
#define MULTIBOOT_FLAG_MEMMAP 0x40

/* Estrutura que descreve um módulo carregado pelo GRUB */
typedef struct {
    unsigned int mod_start;   /* Endereço físico de início do módulo */
    unsigned int mod_end;     /* Endereço físico de fim do módulo */
    unsigned int string;      /* Endereço da string de comando do módulo */
    unsigned int reserved;    /* Reservado (deve ser zero) */
} __attribute__((packed)) multiboot_mod_t;

/* Estrutura que descreve uma região da memória física (Capítulo 10) */
typedef struct {
    unsigned int size;        /* Tamanho da estrutura atual */
    unsigned int base_addr_low;
    unsigned int base_addr_high;
    unsigned int length_low;
    unsigned int length_high;
    unsigned int type;        /* type = 1 indica RAM utilizável (available) */
} __attribute__((packed)) multiboot_memory_map_t;

/* Estrutura principal de informações do Multiboot
 * O GRUB preenche esta struct e passa seu endereço em EBX */
typedef struct {
    unsigned int flags;        /* Indica quais campos abaixo são válidos */
    unsigned int mem_lower;    /* Memória baixa disponível (KB) */
    unsigned int mem_upper;    /* Memória alta disponível (KB) */
    unsigned int boot_device;  /* Dispositivo de boot */
    unsigned int cmdline;      /* Endereço da linha de comando do kernel */
    unsigned int mods_count;   /* Número de módulos carregados */
    unsigned int mods_addr;    /* Endereço físico do array de multiboot_mod_t */
    unsigned int syms[4];      /* Informações de símbolos (a.out ou ELF) */
    unsigned int mmap_length;  /* Tamanho do mapa de memória */
    unsigned int mmap_addr;    /* Endereço do mapa de memória */
    unsigned int drives_length;
    unsigned int drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info;
    unsigned int vbe_mode_info;
    unsigned short vbe_mode;
    unsigned short vbe_interface_seg;
    unsigned short vbe_interface_off;
    unsigned short vbe_interface_len;
} __attribute__((packed)) multiboot_info_t;

#endif /* MULTIBOOT_H */
