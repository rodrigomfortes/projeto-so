FROM --platform=linux/amd64 ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    make nasm gcc gcc-multilib binutils genisoimage qemu-system-x86 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /os
COPY . .

# Garante build limpo dentro do container
RUN make clean 2>/dev/null || true
RUN make iso

# Roda QEMU headless:
#   -monitor stdio  → recebe comandos pelo stdin
#   -serial file:   → salva output serial em arquivo
# Após 3s envia "info registers" para ver EAX=0xDEADBEEF, depois "quit"
CMD bash -c ' \
    (sleep 3; echo "info registers"; sleep 1; echo "quit") \
    | qemu-system-i386 -cdrom os.iso -nographic \
        -monitor stdio -serial file:/tmp/serial.log \
        -m 32 2>/dev/null; \
    echo ""; \
    echo "========================================"; \
    echo "=== Log Serial (COM1) ==="; \
    echo "========================================"; \
    cat /tmp/serial.log 2>/dev/null'
