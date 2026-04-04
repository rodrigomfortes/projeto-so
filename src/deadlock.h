#ifndef DEADLOCK_H
#define DEADLOCK_H

/*
 * Demonstração de Deadlock no Kernel
 *
 * Simula transferência de arquivos entre dois usuários (VGA e Serial).
 * Cada transferência precisa travar a "caixa" de saída do remetente
 * e a "caixa" de entrada do destinatário.
 *
 * Cenário 1 (SEM hierarquia): cada processo trava a própria caixa primeiro
 *   → ordem oposta → DEADLOCK
 *
 * Cenário 2 (COM hierarquia): ambos travam Caixa_VGA antes de Caixa_Serial
 *   → ordem fixa → sem deadlock
 */

/* Spinlock simples: flag + nome para log */

typedef struct {
    volatile int locked;
    const char  *name;
} spinlock_t;

#define SPINLOCK_INIT(n) { 0, (n) }

void spinlock_acquire(spinlock_t *lock);
int  spinlock_try_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);

/* Executa a demonstração completa (cenário 1 + cenário 2) */
void deadlock_demo(void);

#endif /* DEADLOCK_H */
