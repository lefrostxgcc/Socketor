#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "phone.h"

static int		create_server_socket(int port);
static int		create_client_socket(const char *ip, int port);

void phone_new_server(const char *port, struct Phone *phone)
{
	phone->type = PHONE_SERVER;
	phone->server_socket = create_server_socket(atoi(port));
	phone->client_socket = -1;
	phone->inpos = PHONE_BUFSIZE;
	phone->outpos = 0;
}

void phone_new_client(const char *ip, const char *port, struct Phone *phone)
{
	phone->type = PHONE_CLIENT;
	phone->server_socket = -1;
	phone->client_socket = create_client_socket(ip, atoi(port));
	phone->inpos = PHONE_BUFSIZE;
	phone->outpos = 0;
}

void phone_accept(struct Phone *phone, int timeout)
{
	struct pollfd	fds = {};
	int				rc;

	fds.fd = phone->server_socket;
	fds.events = POLLIN;

	rc = poll(&fds, 1, timeout); 
	if (rc < 0)
    {
		perror("poll");
		exit(EXIT_FAILURE);
	}
	else if (rc == 0 || fds.revents != POLLIN)
	{
		phone->client_socket = -1;
		return;
	}

	if ((phone->client_socket = accept(phone->server_socket, NULL, NULL)) < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}
}

void phone_fillbuf(struct Phone *phone)
{
	int		bytes_read;

	if ((bytes_read = read(phone->client_socket, phone->inbuf,
		PHONE_BUFSIZE - 1)) < 0)
	{
		perror("read");
		exit(EXIT_FAILURE);
	}
	phone->inbuf[bytes_read] = '\0';
	phone->inpos = 0;
}

void phone_flushbuf(struct Phone *phone)
{
	if (write(phone->client_socket, phone->outbuf, PHONE_BUFSIZE) < 0)
	{
		perror("write");
		exit(EXIT_FAILURE);
	}
	phone->outpos = 0;
}

void phone_readline(struct Phone *phone, char *buf, int bufsize)
{
	int		i;

	if (phone->inpos >= PHONE_BUFSIZE)
		return;

	for (i = phone->inpos; i < PHONE_BUFSIZE && phone->inbuf[i] != '\n'; i++)
		;
	phone->inbuf[i] = '\0';
	strncpy(buf, phone->inbuf + phone->inpos, bufsize);
	phone->inpos = i + 1;
}

void phone_writeline(struct Phone *phone, const char *line)
{
	size_t		line_len;

	if (phone->outpos >= PHONE_BUFSIZE)
		return;

	line_len = strlen(line);
	snprintf(phone->outbuf + phone->outpos, PHONE_BUFSIZE - phone->outpos,
		"%s\n", line);
	phone->outpos += line_len + 1;
}

void phone_close(struct Phone *phone)
{
	if (phone->client_socket > 0 && close(phone->client_socket) < 0)
	{
		perror("close");
		exit(EXIT_FAILURE);
	}
}

static int		create_server_socket(int port)
{
	struct sockaddr_in	address = {};
	int					server_socket;
	int					opt;
	int					on;

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	opt = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		&opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	if (ioctl(server_socket, FIONBIO, (char *) &on) < 0)
	{
		perror("ioctl");
		close(server_socket);
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_socket, (const struct sockaddr *)&address,
		sizeof(address)) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_socket, 10) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	return server_socket;
}

static int		create_client_socket(const char *ip, int port)
{
	struct sockaddr_in	address = {};
	int					client_socket;

	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (inet_pton(AF_INET, ip, &address.sin_addr.s_addr) <= 0)
	{
		perror("inet_pton");
		exit(EXIT_FAILURE);
	}

	if (connect(client_socket, (const struct sockaddr *) &address,
		sizeof(address)) < 0)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}

	return client_socket;
}
