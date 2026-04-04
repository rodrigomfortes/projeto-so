# Chat multiusuário (userspace) — independente do kernel MiniOS
CC = cc
CFLAGS = -Wall -Wextra -O2 -pthread
LDFLAGS = -pthread

all: server client deadlock_demo

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)

deadlock_demo: deadlock_demo.c
	$(CC) $(CFLAGS) -o deadlock_demo deadlock_demo.c $(LDFLAGS)

clean:
	rm -f server client deadlock_demo

.PHONY: all clean
