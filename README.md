# Kernel Minimalista x86 - The Little OS Book (Capítulos 2 e 3)

Este projeto implementa um kernel minimalista x86 seguindo os capítulos 2 e 3 do "The Little OS Book".

## Estrutura do Projeto

```
projeto-so/
├── src/
│   ├── loader.s      # Bootloader em Assembly (Cap 2 + Cap 3)
│   ├── kmain.c       # Kernel principal em C (Cap 3)
│   └── link.ld       # Linker script para mapeamento de memória
├── iso/
│   └── boot/
│       └── grub/     # Estrutura para GRUB Legacy
├── Makefile          # Automação de build
├── bochsrc.txt       # Configuração do emulador Bochs
└── README.md         # Este arquivo
```

## Pré-requisitos

### Ferramentas necessárias (no WSL Ubuntu):

```bash
sudo apt-get update
sudo apt-get install build-essential nasm genisoimage bochs bochs-sdl qemu-system-x86 gcc-multilib
```

### Obter o stage2_eltorito do GRUB Legacy

O arquivo `stage2_eltorito` é necessário para criar a ISO bootável. Você pode obtê-lo de uma das seguintes formas:

**Opção 1: Instalar o GRUB Legacy**
```bash
sudo apt-get install grub-legacy
```

**Opção 2: Baixar de um sistema com GRUB Legacy instalado**

O arquivo geralmente está em:
- `/usr/lib/grub/i386-pc/stage2_eltorito`
- `/usr/lib/grub-legacy/stage2_eltorito`
- `/boot/grub/stage2_eltorito`

**Opção 3: Copiar manualmente**

Se você tiver o arquivo de outra fonte, copie-o para:
```bash
cp /caminho/para/stage2_eltorito iso/boot/grub/
```

## Como Compilar

### Capítulo 2: Kernel básico em Assembly

```bash
make all        # Compilar o kernel
make iso        # Criar a ISO bootável
```

### Capítulo 3: Adicionar código C

Após implementar o Capítulo 3 (stack + kmain.c):

```bash
make all        # Recompilar com código C
make iso        # Recriar a ISO
```

## Como Executar

### No Bochs:
```bash
make run
```

### No QEMU:
```bash
make run-qemu
```

## Limpar arquivos gerados

```bash
make clean
```

## Progresso

- [x] Capítulo 2: First Steps
  - [x] Estrutura de diretórios
  - [x] loader.s com Multiboot header
  - [x] link.ld (linker script)
  - [x] Makefile básico
  - [x] Configuração do Bochs
  - [x] stage2_eltorito obtido e configurado
  - [x] ISO bootável criada (os.iso)

- [x] Capítulo 3: Getting to C
  - [x] Configuração da stack (16KB)
  - [x] Chamada para kmain
  - [x] kmain.c implementado
  - [x] Makefile atualizado para C

## Status do Projeto

✅ **Projeto completo e funcional!**

- Kernel compilado: `kernel.elf` (5.1KB)
- ISO bootável: `os.iso` (658KB)
- Todos os capítulos 2 e 3 implementados

## Testando o Kernel

O kernel foi compilado com sucesso e está pronto para ser executado em um emulador.

### Verificar a compilação:
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && make all"
```

### Criar a ISO:
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && make iso"
```

### Executar no QEMU (recomendado):
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && make run-qemu"
```

## Referências

- [The Little OS Book](https://littleosbook.github.io/)
- [OSDev Wiki](https://wiki.osdev.org/)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)

