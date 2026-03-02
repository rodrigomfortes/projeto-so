#include "framebuffer.h"
#include "io.h"

/* Localização da memória de vídeo (VGA text mode) */
#define VGA_ADDRESS 0x000B8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* Portas de I/O para o controlador VGA */
#define VGA_CMD_PORT  0x3D4
#define VGA_DATA_PORT 0x3D5

/* Comandos para manipular o cursor */
#define VGA_CURSOR_HIGH_BYTE 14
#define VGA_CURSOR_LOW_BYTE  15

/* Variáveis de estado do terminal */
static char *video_memory = (char *) VGA_ADDRESS;
static unsigned short current_position = 0;

/**
 * Atualiza a posição atual do cursor piscante na tela
 */
static void update_cursor_position(unsigned short pos) {
    outb(VGA_CMD_PORT, VGA_CURSOR_HIGH_BYTE);
    outb(VGA_DATA_PORT, (pos >> 8) & 0x00FF);
    outb(VGA_CMD_PORT, VGA_CURSOR_LOW_BYTE);
    outb(VGA_DATA_PORT, pos & 0x00FF);
}

/**
 * Põe um caractere em uma certa posição do Framebuffer (letra e cor respectiva)
 */
static void set_character(unsigned int idx, char c, unsigned char fg, unsigned char bg) {
    unsigned int vga_idx = idx * 2;
    video_memory[vga_idx] = c;
    video_memory[vga_idx + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

/**
 * Limpa toda a tela, preenchendo com espaços e cor preta
 */
void console_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        set_character(i, ' ', CONSOLE_WHITE, CONSOLE_BLACK);
    }
    current_position = 0;
    update_cursor_position(current_position);
}

/**
 * Rola a tela uma linha para cima se estiver cheia
 */
static void scroll_console() {
    // Copia dados de (linha Y) para (linha Y - 1)
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        int next_row = i + VGA_WIDTH;
        video_memory[i * 2] = video_memory[next_row * 2];
        video_memory[i * 2 + 1] = video_memory[next_row * 2 + 1];
    }

    // Limpa a última linha residual
    int last_row_start = (VGA_HEIGHT - 1) * VGA_WIDTH;
    for (int i = last_row_start; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        set_character(i, ' ', CONSOLE_WHITE, CONSOLE_BLACK);
    }

    // Define cursor para o começo da última linha
    current_position = last_row_start;
}

/**
 * Escreve uma string colorida avançando o cursor e contornando pular linha
 */
void console_write_colored(char *text, unsigned char fg, unsigned char bg) {
    int i = 0;
    while (text[i] != '\0') {
        char character = text[i];

        if (character == '\n') {
            current_position += VGA_WIDTH - (current_position % VGA_WIDTH);
        } else {
            set_character(current_position, character, fg, bg);
            current_position++;
        }

        // Se passar da capacidade, fazemos scroll na tela inteira
        if (current_position >= VGA_WIDTH * VGA_HEIGHT) {
            scroll_console();
        }
        i++;
    }
    update_cursor_position(current_position);
}

/**
 * Escreve uma string branca no console
 */
void console_write(char *text) {
    console_write_colored(text, CONSOLE_WHITE, CONSOLE_BLACK);
}
