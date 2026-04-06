#ifndef DEADLOCK_H
#define DEADLOCK_H

/*
 * Desafio de Deadlock — "transferência de arquivos" entre vga e serial.
 *
 * Cada usuário tem um recurso (tipo um arquivo) protegido por spinlock.
 * Pra transferir entre dois, precisa travar o recurso dos dois.
 * Se não seguir uma ordem fixa de trava, dá deadlock.
 *
 * Comandos:
 *   /deadlock  — trava o kernel de propósito (sem hierarquia)
 *   /transferir  — transferência segura (com hierarquia de locks)
 */

/* Monta os recursos ("arquivos") de cada usuário */
void deadlock_init(void);

/* Transferência SEM hierarquia — causa deadlock (spin infinito) */
void deadlock_demo_unsafe(void);

/* Transferência COM hierarquia — funciona sem problema */
void deadlock_demo_safe(void);

#endif /* DEADLOCK_H */
