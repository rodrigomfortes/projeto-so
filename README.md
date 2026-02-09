
# 🧠 MiniOS Kernel: Estudo de Caso (Little OS Book)

Este projeto contém a implementação fundamental de um **Kernel x86**, desenvolvido seguindo os conceitos dos **Capítulos 1, 2 e 3** do livro *The Little OS Book*.

O objetivo deste repositório não é apenas rodar um código, mas servir como material de estudo sobre **como um computador inicia**, como o hardware transfere o controle para o software e como preparamos o terreno para sair do Assembly e programar em C.

---

## 📚 Conceitos Fundamentais Abordados

### 1. O Processo de Boot (Multiboot)

Quando o computador liga, ele não sabe o que é C, Python ou Java. Ele executa instruções brutas da BIOS, que carrega um **Bootloader** (no nosso caso, o GRUB).

* **O Problema:** O GRUB não sabe onde está o nosso Kernel ou como carregá-lo.
* **A Solução (Magic Numbers):** Implementamos um cabeçalho "Multiboot" no início do nosso arquivo Assembly (`loader.s`). São constantes hexadecimais específicas (`0x1BADB002`) que funcionam como uma assinatura, dizendo ao GRUB: *"Ei, eu sou um Kernel válido, me carregue na memória!"*.

### 2. A Necessidade da Stack (Pilha)

Este é o ponto crucial do **Capítulo 3**.

* **O Problema:** A linguagem C depende pesadamente de uma estrutura de dados chamada **Stack** para gerenciar variáveis locais, chamadas de função e retornos. O hardware não cria isso sozinho.
* **A Solução:** Antes de chamar a função `kmain` (nosso código C), precisamos reservar manualmente um bloco de memória no Assembly e apontar o registrador `ESP` (Stack Pointer) para o topo desse bloco. Sem isso, qualquer código C causaria um erro fatal (Triple Fault) imediato.

### 3. Cross-Compilation (Compilação Cruzada)

Nós estamos programando em um sistema moderno (Linux 64-bit), mas criando código para uma máquina "pelada" (Bare Metal 32-bit).

* Não podemos usar as bibliotecas padrão (`stdio.h`, `stdlib.h`), pois elas dependem de um Sistema Operacional (Linux/Windows) para funcionar. **Nós somos o Sistema Operacional.**
* Por isso, usamos flags especiais no GCC (`-nostdlib`, `-fno-builtin`) para garantir que nosso código não tente chamar funções que não existem.

---

## 📂 Arquitetura do Projeto

Abaixo, a explicação de cada componente vital do sistema:

### `src/loader.s` (O Porteiro)

Escrito em **Assembly (NASM)**. É o primeiro código a ser executado.

1. Define o **Multiboot Header**.
2. Reserva espaço para a **Kernel Stack** (seção `.bss`).
3. Configura o Stack Pointer (`esp`).
4. Chama a função externa `kmain` (Assembly -> C).
5. Entra em loop infinito para impedir que o processador desligue.

### `src/kmain.c` (O Cérebro)

Escrito em **C (GCC)**.
É onde a lógica do sistema operacional começa. Como não temos drivers de vídeo ainda, esta função apenas executa lógica interna.

* *Curiosidade:* Funções simples como `printf` não existem aqui. Se quisermos escrever na tela, teremos que manipular diretamente a memória de vídeo (`0xB8000`) no futuro.

### `src/link.ld` (O Mapa)

Escrito em **Linker Script**.
O compilador normalmente joga o código em qualquer lugar da memória virtual. Como estamos em "Bare Metal", precisamos dizer **exatamente** onde o código deve ficar na RAM física.

* Este script diz ao Linker para montar nosso executável começando no endereço `0x00100000` (1MB), que é o padrão seguro para Kernels x86.

---

## 🛠️ Entendendo a Toolchain (Ferramentas)

Para transformar esses arquivos de texto em um Sistema Operacional Bootável (`.iso`), usamos um processo de 3 etapas automatizado pelo `Makefile`:

1. **Montagem (NASM):** Transforma `loader.s` em código de máquina (`.o`).
2. **Compilação (GCC):** Transforma `kmain.c` em código de máquina (`.o`), garantindo compatibilidade 32-bits.
3. **Linkagem (LD):** Pega os dois arquivos `.o`, consulta o mapa `link.ld`, e gera o executável final `kernel.elf`.

Por fim, o `genisoimage` embrulha esse executável junto com o GRUB para criar o CD (`os.iso`).

---

## 🧪 Como Validar (O Teste do "Cafebabe")

Como ainda não escrevemos drivers de vídeo (Capítulo 4), a tela do emulador ficará preta ou mostrará o menu do GRUB. Isso é normal.

Para saber se funcionou:

1. No código, instruímos o processador a escrever o valor mágico `0xCAFEBABE` no registrador `EAX`.
2. Rodamos o emulador (Bochs ou QEMU).
3. Fechamos o emulador e verificamos o Log.
4. Se `EAX = CAFEBABE` aparece no log, significa que:
   * O Boot funcionou.
   * A Stack foi criada.
   * O C foi executado com sucesso.

---

## 🔗 Referências e Leitura Complementar

* **Little OS Book:** [https://littleosbook.github.io/](https://littleosbook.github.io/)
* **OSDev Wiki (A Bíblia do desenvolvimento de SO):** [https://wiki.osdev.org/](https://wiki.osdev.org/)
* **Multiboot Specification:** Documentação oficial sobre como bootloaders conversam com kernels.
