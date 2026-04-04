#include "chat.h"
#include "framebuffer.h"
#include "serial.h"

#define KBD_RING_SZ 256
#define LINE_MAX 120

static volatile unsigned char kbd_ring[KBD_RING_SZ];
static volatile unsigned char kbd_head;
static volatile unsigned char kbd_tail;

static char kbd_line[LINE_MAX + 1];
static unsigned int kbd_len;

static char ser_line[LINE_MAX + 1];
static unsigned int ser_len;

static void append_formatted(const char *user, const char *msg, char *out, unsigned int out_sz)
{
    unsigned int o = 0;
    const char *p;

    p = "[";
    while (*p && o + 1 < out_sz)
        out[o++] = *p++;
    p = user;
    while (*p && o + 1 < out_sz)
        out[o++] = *p++;
    p = "] ";
    while (*p && o + 1 < out_sz)
        out[o++] = *p++;
    p = msg;
    while (*p && o + 1 < out_sz)
        out[o++] = *p++;
    if (o + 1 < out_sz)
        out[o++] = '\n';
    out[o] = '\0';
}

/*
 * Secção crítica: garante que framebuffer e serial recebem a mesma linha
 * sem intercalar com outra mensagem (comparável a pthread_mutex no userspace).
 */
static void chat_submit(const char *user, const char *msg)
{
    char buf[LINE_MAX + 16];

    if (msg[0] == '\0')
        return;

    append_formatted(user, msg, buf, sizeof buf);

    __asm__ volatile ("cli");
    console_write_colored(buf, CONSOLE_LIGHT_GREEN, CONSOLE_BLACK);
    serial_print(buf);
    __asm__ volatile ("sti");
}

void chat_init(void)
{
    kbd_head = 0;
    kbd_tail = 0;
    kbd_len = 0;
    kbd_line[0] = '\0';
    ser_len = 0;
    ser_line[0] = '\0';
}

void chat_kbd_enqueue(unsigned char c)
{
    unsigned int next = (unsigned int)(kbd_tail + 1U) % KBD_RING_SZ;
    if (next == (unsigned int)kbd_head)
        return;
    kbd_ring[kbd_tail] = c;
    kbd_tail = (unsigned char)next;
}

static int kbd_pop(unsigned char *c)
{
    int ok;

    __asm__ volatile ("cli");
    if (kbd_head == kbd_tail) {
        __asm__ volatile ("sti");
        return 0;
    }
    *c = kbd_ring[kbd_head];
    kbd_head = (unsigned char)(((unsigned int)kbd_head + 1U) % KBD_RING_SZ);
    ok = 1;
    __asm__ volatile ("sti");
    return ok;
}

static void kbd_process_char(unsigned char c)
{
    if (c == '\n') {
        kbd_line[kbd_len] = '\0';
        chat_submit("vga", kbd_line);
        kbd_len = 0;
        kbd_line[0] = '\0';
        return;
    }

    if (c == '\b') {
        if (kbd_len > 0) {
            kbd_len--;
            kbd_line[kbd_len] = '\0';
            console_backspace();
        }
        return;
    }

    if (c < 32)
        return;

    if (kbd_len >= LINE_MAX)
        return;

    kbd_line[kbd_len++] = (char)c;
    kbd_line[kbd_len] = '\0';

    {
        char echo[2];
        echo[0] = (char)c;
        echo[1] = '\0';
        console_write(echo);
    }
}

static void ser_process_char(unsigned char c)
{
    if (c == '\r')
        return;

    if (c == '\n') {
        ser_line[ser_len] = '\0';
        chat_submit("serial", ser_line);
        ser_len = 0;
        ser_line[0] = '\0';
        return;
    }

    if (c == '\b' || c == 127) {
        if (ser_len > 0)
            ser_len--;
        ser_line[ser_len] = '\0';
        return;
    }

    if (c < 32)
        return;

    if (ser_len >= LINE_MAX)
        return;

    ser_line[ser_len++] = (char)c;
    ser_line[ser_len] = '\0';
}

void chat_poll(void)
{
    unsigned char c;

    while (kbd_pop(&c))
        kbd_process_char(c);

    while (serial_rx_ready()) {
        c = serial_read_byte();
        ser_process_char(c);
    }
}
