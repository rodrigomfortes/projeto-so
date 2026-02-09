# Makefile - Build automation para o kernel (Capítulos 2.3.1, 2.3.2 e 3.4)
# Referência: The Little OS Book - Chapters 2 & 3

# Compiladores e ferramentas
NASM = nasm
GCC = gcc
LD = ld
GENISOIMAGE = genisoimage

# Flags de compilação
NASM_FLAGS = -f elf32
GCC_FLAGS = -m32 -nostdlib -fno-builtin -ffreestanding -c -Wall -Wextra
LD_FLAGS = -m elf_i386 -T src/link.ld

# Arquivos
LOADER_OBJ = loader.o
KMAIN_OBJ = kmain.o
KERNEL_ELF = kernel.elf
ISO_FILE = os.iso

# Diretórios
SRC_DIR = src
ISO_DIR = iso
GRUB_DIR = $(ISO_DIR)/boot/grub

# Localização do stage2_eltorito (pode variar dependendo da instalação)
STAGE2_PATHS = /usr/lib/grub/i386-pc/stage2_eltorito \
               /usr/lib/grub-legacy/stage2_eltorito \
               /boot/grub/stage2_eltorito

# Target padrão
.PHONY: all
all: $(KERNEL_ELF)

# Compilar loader.s (Assembly)
$(LOADER_OBJ): $(SRC_DIR)/loader.s
	$(NASM) $(NASM_FLAGS) $(SRC_DIR)/loader.s -o $(LOADER_OBJ)

# Compilar kmain.c (C) - será usado no Capítulo 3
$(KMAIN_OBJ): $(SRC_DIR)/kmain.c
	$(GCC) $(GCC_FLAGS) $(SRC_DIR)/kmain.c -o $(KMAIN_OBJ)

# Linkar o kernel (loader + kmain - Capítulo 3)
$(KERNEL_ELF): $(LOADER_OBJ) $(KMAIN_OBJ)
	$(LD) $(LD_FLAGS) -o $(KERNEL_ELF) $(LOADER_OBJ) $(KMAIN_OBJ)

# Criar a ISO bootável
.PHONY: iso
iso: $(KERNEL_ELF)
	@echo "Criando estrutura ISO..."
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_ELF) $(ISO_DIR)/boot/$(KERNEL_ELF)
	@echo "Procurando stage2_eltorito..."
	@if [ -f "$(GRUB_DIR)/stage2_eltorito" ]; then \
		echo "stage2_eltorito já existe em $(GRUB_DIR)/"; \
	else \
		FOUND=0; \
		for path in $(STAGE2_PATHS); do \
			if [ -f "$$path" ]; then \
				echo "Encontrado em $$path"; \
				cp "$$path" $(GRUB_DIR)/stage2_eltorito; \
				FOUND=1; \
				break; \
			fi; \
		done; \
		if [ $$FOUND -eq 0 ]; then \
			echo "ERRO: stage2_eltorito não encontrado!"; \
			echo "Copie manualmente para $(GRUB_DIR)/ ou instale grub-legacy"; \
			exit 1; \
		fi; \
	fi
	@echo "Criando menu.lst..."
	@echo "default=0" > $(GRUB_DIR)/menu.lst
	@echo "timeout=0" >> $(GRUB_DIR)/menu.lst
	@echo "" >> $(GRUB_DIR)/menu.lst
	@echo "title os" >> $(GRUB_DIR)/menu.lst
	@echo "kernel /boot/$(KERNEL_ELF)" >> $(GRUB_DIR)/menu.lst
	@echo "Gerando ISO..."
	$(GENISOIMAGE) -R -b boot/grub/stage2_eltorito -no-emul-boot \
		-boot-load-size 4 -A os -input-charset utf8 -quiet \
		-boot-info-table -o $(ISO_FILE) $(ISO_DIR)
	@echo "ISO criada: $(ISO_FILE)"

# Executar no Bochs
.PHONY: run
run: iso
	bochs -f bochsrc.txt -q

# Executar no QEMU
.PHONY: run-qemu
run-qemu: iso
	qemu-system-i386 -cdrom $(ISO_FILE)

# Limpar arquivos gerados
.PHONY: clean
clean:
	rm -f $(LOADER_OBJ) $(KMAIN_OBJ) $(KERNEL_ELF) $(ISO_FILE)
	rm -f $(ISO_DIR)/boot/$(KERNEL_ELF)
	rm -f $(GRUB_DIR)/menu.lst
	@echo "Nota: $(GRUB_DIR)/stage2_eltorito foi preservado"

# Help
.PHONY: help
help:
	@echo "Targets disponíveis:"
	@echo "  all        - Compilar o kernel (padrão)"
	@echo "  iso        - Criar a ISO bootável"
	@echo "  run        - Executar no Bochs"
	@echo "  run-qemu   - Executar no QEMU"
	@echo "  clean      - Remover arquivos gerados"
	@echo "  help       - Mostrar esta mensagem"

