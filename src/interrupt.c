/* interrupt.c - Handler de interrupções em C (Capítulo 6)
 * Referência: The Little OS Book - Chapter 6: Interrupts and Input
 *
 * Esta função é chamada pelo common_interrupt_handler em Assembly.
 * Ela despacha a interrupção para o handler apropriado (ex: teclado)
 * e envia acknowledge ao PIC quando necessário.
 */

#include "interrupt.h"
#include "pic.h"
#include "keyboard.h"
#include "serial.h"

/* Número da interrupção do teclado (IRQ 1 remapeada para 33) */
#define KEYBOARD_INTERRUPT 33

/**
 * interrupt_handler - Dispatch central de interrupções
 *
 * Chamada pelo assembly com o estado completo dos registradores e da stack.
 * Identifica qual interrupção ocorreu e chama o handler correspondente.
 *
 * @param cpu       Estado dos registradores (salvo pelo pushad)
 * @param interrupt Número da interrupção (0-255)
 * @param error_code Código de erro (0 se não aplicável)
 * @param stack     Estado da stack (eip, cs, eflags)
 */
void interrupt_handler(struct cpu_state cpu,
                       unsigned int interrupt,
                       unsigned int error_code,
                       struct stack_state stack)
{
    /* Evita warnings de parâmetros não usados */
    (void)cpu;
    (void)error_code;
    (void)stack;

    switch (interrupt) {
        case KEYBOARD_INTERRUPT:
            keyboard_handler();
            break;
        default:
            /* Interrupção não tratada - pode logar via serial para debug */
            break;
    }

    /* Envia acknowledge ao PIC para interrupções de hardware (32-47) */
    pic_acknowledge(interrupt);
}
