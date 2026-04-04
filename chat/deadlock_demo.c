/*
 * Demonstração pedagógica: dois mutexes adquiridos em ordem inversa
 * em dois threads -> deadlock possível (o programa pode ficar bloqueado).
 *
 * Compilar e correr: ./deadlock_demo
 * Interromper com Ctrl+C se travar.
 *
 * Correção típica: sempre lock(A) antes de lock(B) em todo o código.
 */

#define _DEFAULT_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static pthread_mutex_t mutex_a = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_b = PTHREAD_MUTEX_INITIALIZER;

static void *thread1(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&mutex_a);
	sleep(1); /* dá tempo ao thread2 de lockar B */
	pthread_mutex_lock(&mutex_b);
	pthread_mutex_unlock(&mutex_b);
	pthread_mutex_unlock(&mutex_a);
	return NULL;
}

static void *thread2(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&mutex_b);
	sleep(1);
	pthread_mutex_lock(&mutex_a);
	pthread_mutex_unlock(&mutex_a);
	pthread_mutex_unlock(&mutex_b);
	return NULL;
}

int main(void)
{
	puts("Dois threads: T1 = A depois B; T2 = B depois A.");
	puts("Se ficar parado, é deadlock. Ctrl+C para sair.\n");

	pthread_t a, b;
	pthread_create(&a, NULL, thread1, NULL);
	pthread_create(&b, NULL, thread2, NULL);
	pthread_join(a, NULL);
	pthread_join(b, NULL);
	puts("Terminou (nem sempre verá esta linha se houver deadlock).");
	return 0;
}
