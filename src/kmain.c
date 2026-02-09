/* kmain.c - Kernel principal em C (Capítulo 3.3) */
/* Referência: The Little OS Book - Chapter 3: Getting to C */

/*
 * kmain - Ponto de entrada do kernel em C
 *
 * Esta função é chamada pelo loader.s após a configuração da stack.
 * Por enquanto, ela apenas mantém o sistema rodando em um loop infinito.
 *
 * Nota: Não podemos usar bibliotecas padrão (stdio.h, stdlib.h, etc.)
 * pois estamos em um ambiente freestanding (sem sistema operacional).
 */
void kmain(void)
{
    /* Loop infinito para manter o kernel rodando */
    while (1) {
        /* Nada a fazer por enquanto */
        /* Em capítulos futuros, adicionaremos funcionalidades aqui */
    }
}

