# Guia de Teste do Kernel

Este documento fornece instruções detalhadas para testar o kernel implementado.

## Pré-requisitos

Certifique-se de que o WSL Ubuntu está instalado e as ferramentas necessárias estão disponíveis:

```bash
wsl.exe -d Ubuntu bash -c "which nasm gcc ld genisoimage qemu-system-i386"
```

Se alguma ferramenta estiver faltando, instale com:

```bash
wsl.exe -d Ubuntu bash -c "sudo apt-get update && sudo apt-get install -y build-essential nasm genisoimage qemu-system-x86 gcc-multilib"
```

## Teste 1: Compilação do Kernel

### Limpar arquivos anteriores
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && make clean"
```

### Compilar o kernel
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && make all"
```

### Resultado esperado:
```
nasm -f elf32 src/loader.s -o loader.o
gcc -m32 -nostdlib -fno-builtin -ffreestanding -c -Wall -Wextra src/kmain.c -o kmain.o
ld -m elf_i386 -T src/link.ld -o kernel.elf loader.o kmain.o
```

✅ **Sucesso**: Arquivo `kernel.elf` criado (~5KB)

## Teste 2: Criação da ISO

### Criar a ISO bootável
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && make iso"
```

### Resultado esperado:
```
Criando estrutura ISO...
Procurando stage2_eltorito...
stage2_eltorito já existe em iso/boot/grub/
Criando menu.lst...
Gerando ISO...
ISO criada: os.iso
```

✅ **Sucesso**: Arquivo `os.iso` criado (~658KB)

## Teste 3: Verificar Estrutura da ISO

### Listar conteúdo da ISO
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && isoinfo -l -i os.iso"
```

### Estrutura esperada:
```
/boot/
/boot/grub/
/boot/grub/menu.lst
/boot/grub/stage2_eltorito
/boot/kernel.elf
```

## Teste 4: Verificar Multiboot Header

### Verificar magic number no kernel
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && xxd -l 32 kernel.elf | grep 'badb002'"
```

✅ **Sucesso**: Deve mostrar `02 b0 ad 1b` (0x1BADB002 em little-endian)

## Teste 5: Executar no QEMU

### Rodar o kernel no QEMU
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && qemu-system-i386 -cdrom os.iso -display none -serial stdio"
```

### Comportamento esperado:
- O QEMU deve iniciar
- O GRUB deve carregar o kernel
- O kernel deve entrar em loop infinito (sistema não trava)
- Pressione `Ctrl+C` para sair

✅ **Sucesso**: Sistema inicia e mantém-se rodando

## Teste 6: Executar no Bochs (Opcional)

### Verificar se Bochs está instalado
```bash
wsl.exe -d Ubuntu bash -c "which bochs"
```

### Rodar no Bochs
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && bochs -f bochsrc.txt -q"
```

**Nota**: Pode ser necessário ajustar os caminhos no `bochsrc.txt` dependendo da instalação do Bochs.

## Teste 7: Verificar Símbolos do Kernel

### Listar símbolos do kernel
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && nm kernel.elf"
```

### Símbolos esperados:
```
00100000 T loader
00100xxx T kmain
00100xxx B kernel_stack
```

✅ **Sucesso**: Símbolos `loader`, `kmain` e `kernel_stack` estão presentes

## Teste 8: Verificar Seções do ELF

### Mostrar seções do kernel
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && readelf -S kernel.elf"
```

### Seções esperadas:
- `.text` (código executável)
- `.rodata` (dados read-only)
- `.data` (dados inicializados)
- `.bss` (dados não inicializados, incluindo stack)

## Teste 9: Verificar Tamanho das Seções

### Mostrar tamanho das seções
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && size kernel.elf"
```

### Resultado esperado:
```
   text    data     bss     dec     hex filename
    xxx     xxx   16384   xxxxx    xxxx kernel.elf
```

✅ **Sucesso**: Seção `.bss` tem 16384 bytes (16KB para a stack)

## Teste 10: Teste de Recompilação

### Modificar kmain.c
Adicione um comentário em `src/kmain.c` e recompile:

```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && echo '/* Teste */' >> src/kmain.c && make all"
```

✅ **Sucesso**: Apenas `kmain.c` é recompilado (Make detecta dependências)

### Restaurar kmain.c
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && git checkout src/kmain.c"
```

## Troubleshooting

### Erro: "stage2_eltorito não encontrado"

**Solução**: Copie manualmente o arquivo:
```bash
wsl.exe -d Ubuntu bash -c "cd /mnt/c/Users/Rodrigo/Documents/projeto-so && cp /tmp/stage2_eltorito iso/boot/grub/"
```

### Erro: "gcc: error: unrecognized command line option '-m32'"

**Solução**: Instale gcc-multilib:
```bash
wsl.exe -d Ubuntu bash -c "sudo apt-get install gcc-multilib"
```

### Erro: "nasm: command not found"

**Solução**: Instale NASM:
```bash
wsl.exe -d Ubuntu bash -c "sudo apt-get install nasm"
```

### Warning: "missing .note.GNU-stack section"

**Status**: ⚠️ Warning não crítico, pode ser ignorado para este projeto.

### Warning: "RWX permissions"

**Status**: ⚠️ Warning esperado para kernels simples, não é um problema.

## Checklist de Validação

- [ ] Compilação sem erros
- [ ] `kernel.elf` criado (~5KB)
- [ ] `os.iso` criada (~658KB)
- [ ] Multiboot header presente
- [ ] Símbolos `loader`, `kmain` e `kernel_stack` presentes
- [ ] Seção `.bss` com 16KB
- [ ] ISO bootável no QEMU
- [ ] Sistema mantém-se rodando (não trava)

## Conclusão

Se todos os testes passaram, o kernel está funcionando corretamente e implementa com sucesso os Capítulos 2 e 3 do "The Little OS Book"! 🎉

---

**Próximo passo**: Implementar o Capítulo 4 (Output - Framebuffer e Serial Port)

