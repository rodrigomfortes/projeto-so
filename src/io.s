global outb
global inb

; Envia um byte (data) para uma porta de I/O (port)
; void outb(unsigned short port, unsigned char data)
outb:
    mov dx, [esp + 4]    ; port
    mov al, [esp + 8]    ; data
    out dx, al           ; envia o dado para a porta associada
    ret

; Lê um byte de uma porta de I/O
; unsigned char inb(unsigned short port)
inb:
    mov dx, [esp + 4]    ; port
    in al, dx            ; armazena o que estava na porta em AL
    ret
