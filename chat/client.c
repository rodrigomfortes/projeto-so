/*
 * Cliente de chat: thread para ler socket -> stdout; thread principal lê stdin -> socket.
 * Primeira linha enviada ao servidor é o nickname.
 */

#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LINE 1024

static int g_sock = -1;

static void *reader_thread(void *arg)
{
	(void)arg;
	char buf[MAX_LINE];

	for (;;) {
		ssize_t n = recv(g_sock, buf, sizeof buf - 1, 0);
		if (n <= 0) {
			if (n < 0 && errno == EINTR)
				continue;
			break;
		}
		buf[n] = '\0';
		fputs(buf, stdout);
		fflush(stdout);
	}

	shutdown(g_sock, SHUT_RDWR);
	return NULL;
}

static int connect_host(const char *host, const char *port)
{
	struct addrinfo hints, *res = NULL, *p;
	memset(&hints, 0, sizeof hints);
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;

	int err = getaddrinfo(host, port, &hints, &res);
	if (err != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		return -1;
	}

	int fd = -1;
	for (p = res; p; p = p->ai_next) {
		fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (fd < 0)
			continue;
		if (connect(fd, p->ai_addr, p->ai_addrlen) == 0)
			break;
		close(fd);
		fd = -1;
	}
	freeaddrinfo(res);
	return fd;
}

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr, "Uso: %s <host> <porta> <nickname>\n", argv[0]);
		return 1;
	}

	const char *host = argv[1];
	const char *port = argv[2];
	const char *nick = argv[3];

	if (strlen(nick) == 0) {
		fprintf(stderr, "Nickname vazio.\n");
		return 1;
	}

	g_sock = connect_host(host, port);
	if (g_sock < 0) {
		fprintf(stderr, "Falha ao conectar a %s:%s\n", host, port);
		return 1;
	}

	char first[MAX_LINE];
	snprintf(first, sizeof first, "%s\n", nick);
	if (send(g_sock, first, strlen(first), 0) < 0) {
		perror("send");
		close(g_sock);
		return 1;
	}

	pthread_t rd;
	if (pthread_create(&rd, NULL, reader_thread, NULL) != 0) {
		perror("pthread_create");
		close(g_sock);
		return 1;
	}

	char line[MAX_LINE];
	while (fgets(line, sizeof line, stdin)) {
		if (send(g_sock, line, strlen(line), 0) < 0) {
			perror("send");
			break;
		}
	}

	shutdown(g_sock, SHUT_WR);
	pthread_join(rd, NULL);
	close(g_sock);
	return 0;
}
