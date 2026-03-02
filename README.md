
# 🧠 MiniOS Kernel: Estudo de Caso (Little OS Book)

Este projeto contém a implementação fundamental de um **Kernel x86**, desenvolvido seguindo os conceitos dos **Capítulos 1 a 5** do livro *The Little OS Book*.

O objetivo deste repositório não é apenas rodar um código, mas servir como material de estudo sobre **como um computador inicia**, como o hardware transfere o controle para o software, como gerenciamos saída de dados e como configuramos a segmentação de memória.

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

### 5. Cross-Compilation (Compilação Cruzada)

Nós estamos programando em um sistema moderno (Linux 64-bit), mas criando código para uma máquina "pelada" (Bare Metal 32-bit).

* Não podemos usar as bibliotecas padrão (`stdio.h`, `stdlib.h`), pois elas dependem de um Sistema Operacional (Linux/Windows) para funcionar. **Nós somos o Sistema Operacional.**
* Por isso, usamos flags especiais no GCC (`-nostdlib`, `-fno-builtin`) para garantir que nosso código não tente chamar funções que não existem.

---

## 📂 Arquitetura do Projeto

```
projeto-so/
├── src/
│   ├── loader.s         # Bootloader Assembly (Multiboot + Stack)
│   ├── kmain.c          # Ponto de entrada do kernel em C
│   ├── link.ld          # Linker script (mapa de memória)
│   ├── io.s             # Instruções in/out em Assembly
│   ├── io.h             # Header das funções de I/O
│   ├── framebuffer.c    # Driver do framebuffer VGA
│   ├── framebuffer.h    # Header do framebuffer
│   ├── serial.c         # Driver da porta serial COM1
│   ├── serial.h         # Header da porta serial
│   ├── gdt.c            # Implementação da GDT
│   ├── gdt.h            # Header da GDT (structs + protótipos)
│   └── gdt_flush.s      # Rotina Assembly para lgdt + atualizar segmentos
├── Makefile             # Automação do build
├── bochsrc.txt          # Configuração do emulador Bochs
└── README.md            # Este arquivo
```

### Componentes Principais

| Arquivo           | Função                    | Capítulo |
|-------------------|---------------------------|----------|
| `loader.s`        | Boot, stack, entry point  | 2-3      |
| `kmain.c`         | Lógica principal do kernel| 3        |
| `link.ld`         | Mapa de memória (1MB+)    | 2        |
| `io.s` / `io.h`   | Acesso a portas de I/O   | 4        |
| `framebuffer.*`   | Driver de vídeo VGA       | 4        |
| `serial.*`        | Driver da porta serial    | 4        |
| `gdt.c` / `gdt.h` | Global Descriptor Table  | 5        |
| `gdt_flush.s`     | Carga da GDT (Assembly)   | 5        |

---

## 🛠️ Entendendo a Toolchain (Ferramentas)

Para transformar esses arquivos de texto em um Sistema Operacional Bootável (`.iso`), usamos um processo automatizado pelo `Makefile`:

1. **Montagem (NASM):** Transforma `loader.s`, `io.s`, `gdt_flush.s` em código de máquina (`.o`).
2. **Compilação (GCC):** Transforma `kmain.c`, `framebuffer.c`, `serial.c`, `gdt.c` em código de máquina (`.o`), com flags de bare-metal 32-bits.
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
=== MiniOS Kernel ===

[OK] GDT inicializada (Cap. 5)
[OK] Framebuffer VGA ativo (Cap. 4)
[OK] Porta Serial COM1 ativa (Cap. 4)
```

No arquivo `com1.out` (log serial):
```
Porta Serial (COM1) Inicializada com sucesso!
Kernel do Little OS Book - Capitulos 1-5
GDT (Global Descriptor Table) carregada com sucesso!
Todas as mensagens enviadas para o framebuffer.
```

Se o kernel **não** sofrer *triple fault* (não reiniciar), a GDT foi carregada com sucesso.

---

## 🔗 Referências e Leitura Complementar

* **Little OS Book:** [https://littleosbook.github.io/](https://littleosbook.github.io/)
* **OSDev Wiki (A Bíblia do desenvolvimento de SO):** [https://wiki.osdev.org/](https://wiki.osdev.org/)
* **OSDev - Segmentation:** [https://wiki.osdev.org/Segmentation](https://wiki.osdev.org/Segmentation)
* **OSDev - GDT:** [https://wiki.osdev.org/GDT](https://wiki.osdev.org/GDT)
* **Intel Manual, Chapter 3:** Detalhes sobre descritores de segmento e a GDT
* **Multiboot Specification:** Documentação oficial sobre como bootloaders conversam com kernels
