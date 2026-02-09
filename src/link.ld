/* link.ld - Linker script para o kernel (Capítulo 2.3.2) */
/* Referência: The Little OS Book - Chapter 2: Linking the Kernel */

ENTRY(loader)                /* o nome da função de entrada */

SECTIONS {
    . = 0x00100000;          /* o código deve ser carregado em 1 MB */

    .text ALIGN(4) : {       /* alinhar em 4 bytes */
        *(.text)             /* todas as seções .text dos arquivos */
    }

    .rodata ALIGN(4) : {     /* dados read-only alinhados em 4 bytes */
        *(.rodata*)          /* todas as seções .rodata dos arquivos */
    }

    .data ALIGN(4) : {       /* dados read-write alinhados em 4 bytes */
        *(.data)             /* todas as seções .data dos arquivos */
    }

    .bss ALIGN(4) : {        /* dados não inicializados alinhados em 4 bytes */
        *(COMMON)            /* todas as seções COMMON dos arquivos */
        *(.bss)              /* todas as seções .bss dos arquivos */
    }
}

