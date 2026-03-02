/* interrupt.h - Definições do handler de interrupções em C (Capítulo 6)
 * Referência: The Little OS Book - Chapter 6: Interrupts and Input
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

/* Estrutura dos registradores salvos pelo pushad.
 * A ordem é a mesma do pushad: eax, ecx, edx, ebx, esp, ebp, esi, edi
 * Mas como pushad empilha na ordem inversa, ao ler da stack
 * o primeiro pop é edi, depois esi, etc.
 */
struct cpu_state {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
} __attribute__((packed));

/* Estrutura do estado da stack empilhado pelo CPU ao entrar na interrupção.
 * Nota: error_code pode ser 0 (fake) para interrupções sem error code.
 */
struct stack_state {
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));

/* Função chamada pelo handler assembly (common_interrupt_handler).
 * Recebe o estado completo via stack.
 */
void interrupt_handler(struct cpu_state cpu,
                       unsigned int interrupt,
                       unsigned int error_code,
                       struct stack_state stack);

#endif /* INTERRUPT_H */
