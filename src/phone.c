#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "phone.h"

static int		create_server_socket(int port);

void phone_new_server(const char *port, struct Phone *phone)
{
	phone->type = PHONE_SERVER;
	phone->server_socket = create_server_socket(atoi(port));
	phone->client_socket = -1;
}

void phone_new_client(const char *ip, const char *port, struct Phone *phone)
{
}

void phone_accept(struct Phone *phone)
{
	if ((phone->client_socket = accept(phone->server_socket, NULL, NULL)) < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}
}

const char *phone_readline(struct Phone *phone)
{
	return "\n";
}

void phone_writeline(struct Phone *phone, const char *str)
{
}

void phone_close(struct Phone *phone)
{
}

static int		create_server_socket(int port)
{
	struct sockaddr_in	address = {};
	int					server_socket;
	int					opt;

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
