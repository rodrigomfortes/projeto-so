/*
 * Servidor de chat TCP: uma thread por cliente.
 * Mutex no log e na lista de clientes (inserção/remoção e cópia de fds para broadcast).
 */

#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_NICK 64
#define LISTEN_BACKLOG 16

typedef struct client {
	int fd;
	char nick[MAX_NICK];
	struct client *next;
} client_t;

static client_t *g_clients;
static pthread_mutex_t g_clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *g_log;
static volatile sig_atomic_t g_stop;

static void log_line(const char *fmt, ...)
{
	char buf[MAX_LINE + MAX_NICK + 128];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	pthread_mutex_lock(&g_log_mutex);
	fputs(buf, g_log);
	if (buf[0] != '\0' && buf[strlen(buf) - 1] != '\n')
		fputc('\n', g_log);
	fflush(g_log);
	pthread_mutex_unlock(&g_log_mutex);
}

static void client_add(int fd, const char *nick)
{
	client_t *c = calloc(1, sizeof *c);
	if (!c) {
		perror("calloc");
		return;
	}
	c->fd = fd;
	strncpy(c->nick, nick, MAX_NICK - 1);
	c->nick[MAX_NICK - 1] = '\0';

	pthread_mutex_lock(&g_clients_mutex);
	c->next = g_clients;
	g_clients = c;
	pthread_mutex_unlock(&g_clients_mutex);
}

static void client_remove_fd(int fd)
{
	pthread_mutex_lock(&g_clients_mutex);
	client_t **pp = &g_clients;
	while (*pp) {
		if ((*pp)->fd == fd) {
			client_t *dead = *pp;
			*pp = dead->next;
			free(dead);
			break;
		}
		pp = &(*pp)->next;
	}
	pthread_mutex_unlock(&g_clients_mutex);
}

/*
 * Copia descritores sob lock; envia fora do lock para não bloquear outros
 * com mutex segurado durante I/O de rede.
 */
static void broadcast_line(const char *nick, const char *line, int except_fd)
{
	char msg[MAX_LINE + MAX_NICK + 32];
	int n = snprintf(msg, sizeof msg, "[%s] %s\n", nick, line);
	if (n < 0 || n >= (int)sizeof msg)
		return;

	int *fds = NULL;
	size_t count = 0;

	pthread_mutex_lock(&g_clients_mutex);
	for (client_t *c = g_clients; c; c = c->next) {
		if (except_fd >= 0 && c->fd == except_fd)
			continue;
		int *p = realloc(fds, (count + 1) * sizeof *fds);
		if (!p) {
			free(fds);
			pthread_mutex_unlock(&g_clients_mutex);
			return;
		}
		fds = p;
		fds[count++] = c->fd;
	}
	pthread_mutex_unlock(&g_clients_mutex);

	for (size_t i = 0; i < count; i++) {
		(void)send(fds[i], msg, (size_t)n, MSG_NOSIGNAL);
	}
	free(fds);
}

static ssize_t recv_line(int fd, char *buf, size_t cap, size_t *len)
{
	while (*len < cap - 1) {
		char ch;
		ssize_t r = recv(fd, &ch, 1, 0);
		if (r == 0)
			return 0;
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (ch == '\n') {
			buf[*len] = '\0';
			return (ssize_t)*len;
		}
		if (ch == '\r')
			continue;
		buf[(*len)++] = ch;
	}
	buf[*len] = '\0';
	return (ssize_t)*len;
}

typedef struct {
	int fd;
} client_arg_t;

static void *client_thread(void *arg)
{
	client_arg_t *ca = arg;
	int fd = ca->fd;
	free(ca);

	char nick[MAX_NICK];
	size_t nlen = 0;
	if (recv_line(fd, nick, sizeof nick, &nlen) <= 0 || nick[0] == '\0') {
		close(fd);
		return NULL;
	}

	client_add(fd, nick);
	log_line("JOIN %s fd=%d", nick, fd);
	broadcast_line("server", "entrou no chat.", -1);

	char line[MAX_LINE];
	for (;;) {
		size_t ll = 0;
		ssize_t got = recv_line(fd, line, sizeof line, &ll);
		if (got <= 0)
			break;
		if (line[0] == '\0')
			continue;

		log_line("%s: %s", nick, line);
		broadcast_line(nick, line, fd);
	}

	log_line("PART %s fd=%d", nick, fd);
	broadcast_line("server", "saiu do chat.", -1);
	client_remove_fd(fd);
	close(fd);
	return NULL;
}

static void on_sigint(int sig)
{
	(void)sig;
	g_stop = 1;
}

static int open_listener(uint16_t port)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}
	int one = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one) != 0)
		perror("setsockopt");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(s, (struct sockaddr *)&addr, sizeof addr) != 0) {
		perror("bind");
		close(s);
		return -1;
	}
	if (listen(s, LISTEN_BACKLOG) != 0) {
		perror("listen");
		close(s);
		return -1;
	}
	return s;
}

int main(int argc, char **argv)
{
	uint16_t port = 9000;
	const char *logpath = "chat.log";

	if (argc >= 2)
		port = (uint16_t)atoi(argv[1]);
	if (argc >= 3)
		logpath = argv[2];

	g_log = fopen(logpath, "a");
	if (!g_log) {
		perror(logpath);
		return 1;
	}

	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = on_sigint;
	sigaction(SIGINT, &sa, NULL);
	struct sigaction ign;
	memset(&ign, 0, sizeof ign);
	ign.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &ign, NULL);

	int listener = open_listener(port);
	if (listener < 0) {
		fclose(g_log);
		return 1;
	}

	log_line("--- servidor a escuta na porta %u (log: %s) ---", port, logpath);

	while (!g_stop) {
		struct sockaddr_in peer;
		socklen_t plen = sizeof peer;
		int fd = accept(listener, (struct sockaddr *)&peer, &plen);
		if (fd < 0) {
			if (errno == EINTR) {
				if (g_stop)
					break;
				continue;
			}
			perror("accept");
			break;
		}

		client_arg_t *ca = malloc(sizeof *ca);
		if (!ca) {
			close(fd);
			continue;
		}
		ca->fd = fd;

		pthread_t th;
		if (pthread_create(&th, NULL, client_thread, ca) != 0) {
			perror("pthread_create");
			free(ca);
			close(fd);
			continue;
		}
		pthread_detach(th);
	}

	close(listener);

	pthread_mutex_lock(&g_clients_mutex);
	while (g_clients) {
		client_t *c = g_clients;
		g_clients = c->next;
		close(c->fd);
		free(c);
	}
	pthread_mutex_unlock(&g_clients_mutex);

	fclose(g_log);
	return 0;
}
