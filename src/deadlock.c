#include "deadlock.h"
#include "framebuffer.h"
#include "serial.h"

typedef struct {
    volatile int locked;    /* 0 = livre, 1 = travado */
    const char  *name;      /* nome para o debug      */
} spinlock_t;

/* Recurso do usuário — simula um "arquivo" que precisa de lock */
typedef struct {
    spinlock_t  lock;
    char        data[64];   /* conteúdo do "arquivo" */
    const char *owner;
} user_file_t;

/* Um recurso para cada usuário do chat */
static user_file_t file_vga;
static user_file_t file_serial;

/**
 * atomic_xchg — troca atômica via instrução XCHG.
 * Retorna o valor anterior de *ptr e coloca val no lugar.
 */
static int atomic_xchg(volatile int *ptr, int val)
{
    int result;
    __asm__ volatile (
        "xchgl %0, %1"
        : "=r"(result), "+m"(*ptr)
        : "0"(val)
        : "memory"
    );
    return result;
}

/*
 * spin_lock — adquire a trava atômica.
 * Se o recurso estiver ocupado, entra em busy-wait (loop infinito) até a liberação.
 */
static void spin_lock(spinlock_t *lk)
{
    while (atomic_xchg(&lk->locked, 1) != 0) {
        /* fica preso aqui quando rola deadlock */
    }
}

/**
 * spin_unlock — libera o lock.
 */
static void spin_unlock(spinlock_t *lk)
{
    __asm__ volatile ("" ::: "memory");   /* barreira de memória */
    lk->locked = 0;
}

/* cópia de string sem libc */
static void str_copy(char *dst, const char *src, unsigned int max)
{
    unsigned int i = 0;
    while (src[i] && i + 1 < max) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/* delay artificial para dar tempo de ler o log */
static void busy_delay(void)
{
    volatile unsigned int i;
    for (i = 0; i < 3000000; i++)
        ;
}


void deadlock_init(void)
{
    file_vga.lock.locked = 0;
    file_vga.lock.name   = "lock_vga";
    file_vga.owner        = "vga";
    str_copy(file_vga.data, "foto_ferias.png", sizeof file_vga.data);

    file_serial.lock.locked = 0;
    file_serial.lock.name   = "lock_serial";
    file_serial.owner        = "serial";
    str_copy(file_serial.data, "trabalho.pdf", sizeof file_serial.data);
}

/*
 * Simula dois usuários tentando realizar uma transferência ao mesmo tempo sem hierarquia de locks.
 */
void deadlock_demo_unsafe(void)
{
    /* limpa os locks antes de começar */
    file_vga.lock.locked    = 0;
    file_serial.lock.locked = 0;

    serial_print("  DEMO: DEADLOCK\r\n");

    console_write_colored("\nDEMO: DEADLOCK\n",
                          CONSOLE_LIGHT_RED, CONSOLE_BLACK);

    /* Proc VGA trava o próprio arquivo */
    serial_print("[VGA]    Travando lock_vga...\r\n");
    console_write_colored("[VGA] Travando lock_vga...\n",
                          CONSOLE_LIGHT_CYAN, CONSOLE_BLACK);

    spin_lock(&file_vga.lock);

    serial_print("[VGA]    lock_vga travado!\r\n");
    console_write_colored("[VGA] lock_vga travado!\n",
                          CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);

    busy_delay();

    /*
     * Proc Serial trava o próprio arquivo.
     * Como não há suporte nativo a threads, a simulação injeta
     * o bloqueio manualmente na variável.
     */
    serial_print("[Serial] Travando lock_serial...\r\n");
    console_write_colored("[Serial] Travando lock_serial...\n",
                          CONSOLE_LIGHT_CYAN, CONSOLE_BLACK);

    file_serial.lock.locked = 1;   /* simula outro processo pegando o lock */

    serial_print("[Serial] lock_serial travado!\r\n");
    console_write_colored("[Serial] lock_serial travado!\n",
                          CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);

    busy_delay();

    /* Os dois tentam pegar o recurso do outro */
    serial_print("\r\n[VGA]    Tentando lock_serial...\r\n");
    serial_print("[Serial] Tentando lock_vga...\r\n");
    serial_print("\r\n DEADLOCK: Os dois aguardam na fila infinitamente.\r\n");
    serial_print("Kernel travou. Reset para sair.\r\n\r\n");

    console_write_colored("\n[VGA] Tentando lock_serial...\n",
                          CONSOLE_LIGHT_BROWN, CONSOLE_BLACK);
    console_write_colored("[Serial] Tentando lock_vga...\n",
                          CONSOLE_LIGHT_BROWN, CONSOLE_BLACK);

    console_write_colored("\nDEADLOCK: Kernel travou.\n",
                          CONSOLE_LIGHT_RED, CONSOLE_BLACK);
    console_write_colored("Ctrl+Alt+Del para reiniciar.\n",
                          CONSOLE_LIGHT_RED, CONSOLE_BLACK);

    /*
     * VGA tenta pegar lock_serial que está travado.
     */
    spin_lock(&file_serial.lock);

    /* nunca chega aqui */
    serial_print("OBS: essa mensagem não aparece!\r\n");
}

/*
 * Simula dois usuários tentando realizar uma transferência ao mesmo tempo com hierarquia de locks.
 */
void deadlock_demo_safe(void)
{
    /* limpa os locks */
    file_vga.lock.locked    = 0;
    file_serial.lock.locked = 0;

    serial_print("  TRANSFERENCIA SEGURA (hierarquia)\r\n");

    console_write_colored("\nTRANSFERENCIA SEGURA\n",
                          CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);
    console_write_colored("Regra: lock_vga sempre antes de lock_serial\n",
                          CONSOLE_LIGHT_GREY, CONSOLE_BLACK);

    /* Pega lock_vga (1° na hierarquia) */
    serial_print("[Seguro] Pegando lock_vga (1o)...\r\n");
    console_write_colored("[Seguro] Pegando lock_vga...\n",
                          CONSOLE_LIGHT_CYAN, CONSOLE_BLACK);

    spin_lock(&file_vga.lock);

    serial_print("[Seguro] lock_vga ok!\r\n");
    console_write_colored("[Seguro] lock_vga ok!\n",
                          CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);

    busy_delay();

    /* Pega lock_serial (2° na hierarquia) */
    serial_print("[Seguro] Pegando lock_serial (2o)...\r\n");
    console_write_colored("[Seguro] Pegando lock_serial...\n",
                          CONSOLE_LIGHT_CYAN, CONSOLE_BLACK);

    spin_lock(&file_serial.lock);

    serial_print("[Seguro] lock_serial ok!\r\n");
    console_write_colored("[Seguro] lock_serial ok!\n",
                          CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);

    /* Faz a transferência (troca o conteúdo dos arquivos) */
    serial_print("[Seguro] Transferindo dados...\r\n");

    console_write_colored("[Seguro] Trocando: ",
                          CONSOLE_WHITE, CONSOLE_BLACK);
    console_write_colored(file_vga.data,
                          CONSOLE_LIGHT_MAGENTA, CONSOLE_BLACK);
    console_write_colored(" <-> ",
                          CONSOLE_WHITE, CONSOLE_BLACK);
    console_write_colored(file_serial.data,
                          CONSOLE_LIGHT_MAGENTA, CONSOLE_BLACK);
    console_write_colored("\n", CONSOLE_WHITE, CONSOLE_BLACK);

    /* Troca efetiva */
    char tmp[64];
    str_copy(tmp, file_vga.data, sizeof tmp);
    str_copy(file_vga.data, file_serial.data, sizeof file_vga.data);
    str_copy(file_serial.data, tmp, sizeof file_serial.data);

    /* Libera os locks (ordem inversa) */
    spin_unlock(&file_serial.lock);
    spin_unlock(&file_vga.lock);

    serial_print("[Seguro] Locks liberados!\r\n\r\n");
    console_write_colored("[Seguro] Sem deadlock.\n",
                          CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);
}
