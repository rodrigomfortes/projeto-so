
# 🧠 MiniOS Kernel: Estudo de Caso (Little OS Book)

Este projeto contém a implementação fundamental de um **Kernel x86**, desenvolvido seguindo os conceitos dos **Capítulos 1 a 6** do livro *The Little OS Book*.

O objetivo deste repositório não é apenas rodar um código, mas servir como material de estudo sobre **como um computador inicia**, como o hardware transfere o controle para o software, como gerenciamos saída de dados, como configuramos a segmentação de memória e como tratamos interrupções e entrada de dados.

---

## 📚 Conceitos Fundamentais Abordados

### 1. O Processo de Boot (Multiboot) — Capítulos 1-2

Quando o computador liga, ele executa instruções brutas da BIOS, que carrega um **Bootloader** (no nosso caso, o GRUB).

* **O Problema:** O GRUB não sabe onde está o nosso Kernel ou como carregá-lo.
* **A Solução (Magic Numbers):** Implementamos um cabeçalho "Multiboot" no início do nosso arquivo Assembly (`loader.s`). São constantes hexadecimais específicas (`0x1BADB002`) que funcionam como uma assinatura, dizendo ao GRUB: *"Ei, eu sou um Kernel válido, me carregue na memória!"*.

### 2. A Necessidade da Stack (Pilha) — Capítulo 3

* **O Problema:** A linguagem C depende pesadamente de uma estrutura de dados chamada **Stack** para gerenciar variáveis locais, chamadas de função e retornos. O hardware não cria isso sozinho.
* **A Solução:** Antes de chamar a função `kmain` (nosso código C), precisamos reservar manualmente um bloco de memória no Assembly e apontar o registrador `ESP` (Stack Pointer) para o topo desse bloco. Sem isso, qualquer código C causaria um erro fatal (Triple Fault) imediato.

### 3. Saída de Dados (Output) — Capítulo 4

Para exibir informações, implementamos dois drivers de saída:

* **Framebuffer VGA (`framebuffer.c`):** Escreve diretamente na memória de vídeo (`0xB8000`) para exibir texto colorido na tela. Manipula o cursor via portas de I/O (`0x3D4`/`0x3D5`).
* **Porta Serial (`serial.c`):** Configura a COM1 para enviar dados de log/debug para o host. Os dados podem ser lidos no arquivo `com1.out` após a execução.
* **I/O Assembly (`io.s`):** Implementa as instruções `in` e `out` do x86 para acessar portas de hardware.

### 4. Segmentação e a GDT — Capítulo 5

A segmentação é o mecanismo do x86 para acessar memória através de **segmentos** — porções do espaço de endereçamento definidas por um endereço base e um limite.

* **O Problema:** O processador x86 exige que a **Global Descriptor Table (GDT)** esteja configurada para definir como a memória é acessada e quais são os níveis de privilégio (ring 0 = kernel, ring 3 = user).
* **A Solução:** Criamos uma GDT com 3 descritores de segmento:

| Índice  | Segmento           | Base         | Limite       | DPL | Tipo          |
|---------|---------------------|-------------|-------------|-----|---------------|
| `0x00`  | Null Descriptor     | —           | —           | —   | Obrigatório   |
| `0x08`  | Kernel Code Segment | `0x00000000`| `0xFFFFFFFF`| 0   | Execute/Read  |
| `0x10`  | Kernel Data Segment | `0x00000000`| `0xFFFFFFFF`| 0   | Read/Write    |

Os dois segmentos cobrem todo o espaço de endereçamento (4GB), o chamado modelo "flat". Usamos a segmentação apenas para estabelecer privilégios, não para isolar regiões de memória.

**Como funciona a carga:**
1. A função `gdt_initialize()` em C preenche a tabela e chama `gdt_flush()`.
2. `gdt_flush()` (Assembly) executa `lgdt` para carregar a GDT no processador.
3. Os registradores `ds`, `ss`, `es`, `fs`, `gs` recebem `0x10` (data segment).
4. Um **far jump** para `0x08:.flush_cs` atualiza o registrador `cs` (code segment).

### 5. Interrupções e Entrada de Dados — Capítulo 6

Uma interrupção é um sinal que indica ao processador que algo aconteceu — uma tecla foi pressionada, um timer disparou ou ocorreu um erro. Para tratar interrupções, implementamos:

* **IDT (Interrupt Descriptor Table):** Tabela de 256 entradas que mapeia cada número de interrupção a um handler. Cada entrada aponta para código Assembly que salva registradores, chama C e retorna com `iret`.
* **PIC (Programmable Interrupt Controller):** O chip 8259A gerencia as IRQs do hardware. Remapeamos IRQs 0-15 para interrupções 32-47 para evitar conflito com exceções do CPU (0-31).
* **Teclado:** Quando uma tecla é pressionada, o teclado gera a IRQ 1 (interrupção 33 após remapeamento). O handler lê o scan code da porta `0x60`, traduz para ASCII e exibe no framebuffer.

| PIC    | IRQ | Interrupção | Dispositivo |
|--------|-----|-------------|-------------|
| Master | 0   | 32          | Timer (PIT) |
| Master | 1   | 33          | Teclado     |
| Master | 2   | 34          | Cascade     |
| Slave  | 8   | 40          | RTC (CMOS)  |

**Fluxo completo de uma interrupção de teclado:**
1. Tecla pressionada → Hardware gera IRQ 1
2. PIC remapeia para interrupção 33
3. CPU consulta IDT[33] → pula para `interrupt_handler_33` (Assembly)
4. Assembly: pushad → call `interrupt_handler()` (C) → popad → iret
5. C: chama `keyboard_handler()` → lê scan code → traduz para ASCII → exibe
6. `pic_acknowledge(33)` → envia ACK ao PIC Master

### 6. Cross-Compilation (Compilação Cruzada)

Nós estamos programando em um sistema moderno (Linux 64-bit), mas criando código para uma máquina "pelada" (Bare Metal 32-bit).

* Não podemos usar as bibliotecas padrão (`stdio.h`, `stdlib.h`), pois elas dependem de um Sistema Operacional (Linux/Windows) para funcionar. **Nós somos o Sistema Operacional.**
* Por isso, usamos flags especiais no GCC (`-nostdlib`, `-fno-builtin`) para garantir que nosso código não tente chamar funções que não existem.

---

## 📂 Arquitetura do Projeto

```
projeto-so/
├── src/
│   ├── loader.s              # Bootloader Assembly (Multiboot + Stack)
│   ├── kmain.c               # Ponto de entrada do kernel em C
│   ├── link.ld               # Linker script (mapa de memória)
│   ├── io.s / io.h           # Instruções in/out em Assembly
│   ├── framebuffer.c/.h      # Driver do framebuffer VGA
│   ├── serial.c/.h           # Driver da porta serial COM1
│   ├── gdt.c/.h              # Global Descriptor Table
│   ├── gdt_flush.s           # lgdt + atualizar segmentos
│   ├── idt.c/.h              # Interrupt Descriptor Table
│   ├── interrupt_handlers.s  # Macros NASM + common handler + lidt
│   ├── interrupt.c/.h        # Dispatch de interrupções em C
│   ├── pic.c/.h              # Programmable Interrupt Controller
│   └── keyboard.c/.h         # Driver de teclado (scan codes)
├── Makefile                  # Automação do build
├── bochsrc.txt               # Configuração do emulador Bochs
└── README.md                 # Este arquivo
```

### Componentes Principais

| Arquivo                | Função                       | Capítulo |
|------------------------|------------------------------|----------|
| `loader.s`             | Boot, stack, entry point     | 2-3      |
| `kmain.c`              | Lógica principal do kernel   | 3        |
| `link.ld`              | Mapa de memória (1MB+)       | 2        |
| `io.s` / `io.h`       | Acesso a portas de I/O       | 4        |
| `framebuffer.*`        | Driver de vídeo VGA          | 4        |
| `serial.*`             | Driver da porta serial       | 4        |
| `gdt.*` / `gdt_flush.s` | Global Descriptor Table    | 5        |
| `idt.*`                | Interrupt Descriptor Table   | 6        |
| `interrupt_handlers.s` | Handlers Assembly (macros)   | 6        |
| `interrupt.*`          | Dispatch de interrupções (C) | 6        |
| `pic.*`                | PIC remapping + acknowledge  | 6        |
| `keyboard.*`           | Driver de teclado            | 6        |

---

## 🛠️ Entendendo a Toolchain (Ferramentas)

Para transformar esses arquivos de texto em um Sistema Operacional Bootável (`.iso`), usamos um processo automatizado pelo `Makefile`:

1. **Montagem (NASM):** Transforma `loader.s`, `io.s`, `gdt_flush.s`, `interrupt_handlers.s` em código de máquina (`.o`).
2. **Compilação (GCC):** Transforma `kmain.c`, `framebuffer.c`, `serial.c`, `gdt.c`, `idt.c`, `interrupt.c`, `pic.c`, `keyboard.c` em código de máquina (`.o`), com flags de bare-metal 32-bits.
3. **Linkagem (LD):** Pega todos os `.o`, consulta o mapa `link.ld`, e gera o executável `kernel.elf`.
4. **ISO (genisoimage):** Embrulha o executável junto com o GRUB para criar o CD bootável (`os.iso`).

---

## 🧪 Como Validar

### Compilar e Executar

```bash
make clean && make all    # compilar o kernel
make iso                  # gerar a ISO bootável
make run                  # executar no Bochs
make run-qemu             # executar no QEMU (alternativa)
```

### O que esperar

Na tela do emulador, o kernel deve exibir:
```
Hello World!

Finalmente deu certo!
Usando o framebuffer vga...!
GDT carregada com sucesso!
IDT e PIC inicializados!
Teclado habilitado - digite algo!
```

Após essa tela, ao **pressionar teclas** no emulador, os caracteres digitados devem aparecer na tela.

No arquivo `com1.out` (log serial):
```
Porta Serial (COM1) Inicializada com sucesso!
Kernel do Little OS Book - Teste do Capitulo 6
GDT (Global Descriptor Table) carregada com sucesso!
PIC remapeado (IRQs 0-15 -> INT 32-47)
IDT (Interrupt Descriptor Table) carregada com sucesso!
Mensagens enviadas para o framebuffer.
Interrupcoes habilitadas (sti). Aguardando input do teclado...
```

Se o kernel **não** sofrer *triple fault* e responder ao teclado, todos os subsistemas estão funcionando.

---

## 🔗 Referências e Leitura Complementar

* **Little OS Book:** [https://littleosbook.github.io/](https://littleosbook.github.io/)
* **OSDev Wiki (A Bíblia do desenvolvimento de SO):** [https://wiki.osdev.org/](https://wiki.osdev.org/)
* **OSDev - Segmentation:** [https://wiki.osdev.org/Segmentation](https://wiki.osdev.org/Segmentation)
* **OSDev - GDT:** [https://wiki.osdev.org/GDT](https://wiki.osdev.org/GDT)
* **OSDev - IDT:** [https://wiki.osdev.org/IDT](https://wiki.osdev.org/IDT)
* **OSDev - Interrupts:** [https://wiki.osdev.org/Interrupts](https://wiki.osdev.org/Interrupts)
* **OSDev - PIC:** [https://wiki.osdev.org/PIC](https://wiki.osdev.org/PIC)
* **Intel Manual, Chapters 3 e 6:** Detalhes sobre segmentação e interrupções
* **Multiboot Specification:** Documentação oficial sobre como bootloaders conversam com kernels
