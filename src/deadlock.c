#include "deadlock.h"
#include "serial.h"
#include "framebuffer.h"

/*
 * Implementacao basica de travas (Spinlocks)
 * Em um sistema operativo single-core sem escalonador preemptivo,
 * a utilizacao de flags simples permite demonstrar passo a passo 
 * como o cruzamento de multiplos processos acessando variaveis 
 * compartilhadas causa um deadlock.
 */

void spinlock_acquire(spinlock_t *lock)
{
    lock->locked = 1;
}

int spinlock_try_acquire(spinlock_t *lock)
{
    if (lock->locked)
        return 0;   /* Retorna 0 se a trava ja estiver em uso */
    lock->locked = 1;
    return 1;       /* Sucesso ao travar */
}

void spinlock_release(spinlock_t *lock)
{
    lock->locked = 0;
}

/*  
 * Recursos compartilhados da simulacao:
 * Simulando a "transferencia de arquivos".
 * Para mover arquivos da Memoria do VGA para a Memoria do Serial, o 
 * processo precisa adquirir a trava (lock) de ambos os recursos ao  
 * mesmo tempo. Isso evita interferencias externas durante a copia.
 */

static spinlock_t caixa_vga    = SPINLOCK_INIT("Caixa_VGA");
static spinlock_t caixa_serial = SPINLOCK_INIT("Caixa_Serial");

/* Pausa inserida intencionalmente para permitir a leitura dos logs */
static void demo_delay(void)
{
    volatile unsigned int i;
    for (i = 0; i < 3000000; i++)
        ;
}

/* Funcao de log segura para imprimir nas duas telas sem quebrar as linhas */
static void log_msg(const char *msg, unsigned char cor)
{
    /*
     * Protege SOMENTE a escrita no framebuffer (rapida).
     * serial_print faz busy-wait e demoraria demais com cli,
     * causando perda de IRQs do teclado.
     */
    __asm__ volatile("cli");
    console_write_colored((char *)msg, cor, CONSOLE_BLACK);
    __asm__ volatile("sti");

    serial_print((char *)msg);
}

/*
 * Cenario 1: Sem hierarquia de travas
 * A execucao deste passo em ordem cruzada gera um bloqueio circular.
 * O processo VGA trava sua caixa e entra em espera pela caixa Serial.
 * O processo Serial trava a sua e entra em espera pela caixa VGA.
 * Como resultado, condicoes para progredir nunca mais ocorrerao.
 */

static void demo_sem_hierarquia(void)
{
    spinlock_release(&caixa_vga);
    spinlock_release(&caixa_serial);

    log_msg("\nCenario 1: Sem hierarquia de travas\n", CONSOLE_LIGHT_RED);
    demo_delay();

    log_msg("VGA: Lock Caixa_VGA... ", CONSOLE_LIGHT_CYAN);
    spinlock_acquire(&caixa_vga);
    log_msg("OK\n", CONSOLE_LIGHT_GREEN);
    demo_delay();

    log_msg("Serial: Lock Caixa_Serial... ", CONSOLE_LIGHT_MAGENTA);
    spinlock_acquire(&caixa_serial);
    log_msg("OK\n", CONSOLE_LIGHT_GREEN);
    demo_delay();

    log_msg("VGA: Lock Caixa_Serial... ", CONSOLE_LIGHT_CYAN);
    if (!spinlock_try_acquire(&caixa_serial)) {
        log_msg("Bloqueado!\n", CONSOLE_LIGHT_RED);
    }
    demo_delay();

    log_msg("Serial: Lock Caixa_VGA... ", CONSOLE_LIGHT_MAGENTA);
    if (!spinlock_try_acquire(&caixa_vga)) {
        log_msg("Bloqueado!\n", CONSOLE_LIGHT_RED);
    }
    demo_delay();

    log_msg("\nErro: Deadlock detectado!\n", CONSOLE_LIGHT_RED);
    log_msg("VGA aguarda Serial, e Serial aguarda VGA.\n\n", CONSOLE_LIGHT_RED);

    spinlock_release(&caixa_vga);
    spinlock_release(&caixa_serial);
}

/*
 * Cenario 2: Com hierarquia de travas
 * A definicao de uma ordem global obrigatoria evita a dependencia circular.
 * Regra do Sistema: "A trava da Caixa_VGA deve ser solicitada primeiro."
 * O prmeiro processo que solicitar obtera as duas livremente, e o segundo 
 * entrara em espera na primeira verificacao, continuando apenas no final.
 */

static void demo_com_hierarquia(void)
{
    spinlock_release(&caixa_vga);
    spinlock_release(&caixa_serial);

    log_msg("Cenario 2: Com hierarquia de travas\n", CONSOLE_LIGHT_GREEN);
    demo_delay();

    log_msg("VGA: Lock Caixa_VGA... ", CONSOLE_LIGHT_CYAN);
    spinlock_acquire(&caixa_vga);
    log_msg("OK\n", CONSOLE_LIGHT_GREEN);
    demo_delay();

    log_msg("Serial: Lock Caixa_VGA... ", CONSOLE_LIGHT_MAGENTA);
    if (!spinlock_try_acquire(&caixa_vga)) {
        log_msg("Aguardando liberacao...\n", CONSOLE_LIGHT_BROWN);
    }
    demo_delay();

    log_msg("VGA: Lock Caixa_Serial... ", CONSOLE_LIGHT_CYAN);
    spinlock_acquire(&caixa_serial);
    log_msg("OK\n", CONSOLE_LIGHT_GREEN);
    demo_delay();

    log_msg("VGA: Transferencia concluida\n", CONSOLE_LIGHT_CYAN);
    spinlock_release(&caixa_serial);
    spinlock_release(&caixa_vga);
    demo_delay();

    log_msg("Serial: Lock Caixa_VGA... ", CONSOLE_LIGHT_MAGENTA);
    spinlock_acquire(&caixa_vga);
    log_msg("OK\n", CONSOLE_LIGHT_GREEN);
    demo_delay();

    log_msg("Serial: Lock Caixa_Serial... ", CONSOLE_LIGHT_MAGENTA);
    spinlock_acquire(&caixa_serial);
    log_msg("OK\n", CONSOLE_LIGHT_GREEN);
    demo_delay();

    log_msg("Serial: Transferencia concluida\n", CONSOLE_LIGHT_MAGENTA);
    spinlock_release(&caixa_serial);
    spinlock_release(&caixa_vga);
    demo_delay();

    log_msg("\nSucesso: Hierarquia evitou o deadlock.\n\n", CONSOLE_LIGHT_GREEN);
}

/* Funcao para ser acessada no envio do chat */

void deadlock_demo(void)
{
    log_msg("\nIniciando demonstracao de deadlock...\n", CONSOLE_WHITE);
    demo_delay();

    demo_sem_hierarquia();
    demo_delay();

    demo_com_hierarquia();

    log_msg("Fim da demonstracao. Voltando ao chat.\n\n", CONSOLE_WHITE);

    /*
     * Limpa qualquer scan-code que o teclado tenha gerado enquanto
     * as interrupcoes estavam desabilitadas (cli dentro de log_msg).
     * Sem isso, o controlador do teclado pode travar esperando leitura.
     */
    __asm__ volatile("cli");
    while (inb(0x64) & 0x01)   /* bit 0 do status = Output Buffer Full */
        (void)inb(0x60);       /* descarta o scan-code pendente        */
    __asm__ volatile("sti");
}
