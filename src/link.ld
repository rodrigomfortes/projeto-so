/*
 * Linker script do kernel
 * Ajusta tudo pro Higher-Half Kernel rodando em 3GB+ (0xC0100000),
 * enquanto o GRUB joga fisicamente na gaveta de 1MB.
 */

ENTRY(loader)                        /* o nome da função de entrada */

SECTIONS {
    . = 0xC0100000;                  /* o código deve ser relocado para 3GB + 1MB */

    /* Marca o início do kernel (endereço virtual) */
    KERNEL_START = .;

    /* alinhar em 4 KB e carregar em 1 MB */
    .text ALIGN (0x1000) : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)                     /* todas as seções .text dos arquivos */
    }

    /* dados read-only alinhados em 4 KB */
    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata*)                  /* todas as seções .rodata dos arquivos */
    }

    /* dados read-write alinhados em 4 KB */
    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)                     /* todas as seções .data dos arquivos */
    }

    /* dados não inicializados alinhados em 4 KB */
    .bss ALIGN (0x1000) : AT(ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)                    /* todas as seções COMMON dos arquivos */
        *(.bss)                      /* todas as seções .bss dos arquivos */
    }

    /* Marca o fim do kernel (após BSS, alinhado em 4K pra garantir) */
    . = ALIGN(0x1000);
    KERNEL_END = .;
}
