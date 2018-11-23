#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "phone.h"

#define		BUFSIZE		32

void run_server(const char *port, const char *op);
void run_client(const char *ip, const char *port, const char *a, const char *b);
int calculate(const char *operation, const char *a, const char *b);

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		fprintf(stderr,
			"Usage: socketor server port operation\n"
			"Usage: socketor client address port a b\n");
		return 1;
	}

	if (strcmp(argv[1], "server") == 0 && argc == 4)
		run_server(argv[2], argv[3]);
	else if (strcmp(argv[1], "client") == 0 && argc == 6)
		run_client(argv[2], argv[3], argv[4], argv[5]);

	return 0;
}

void run_server(const char *port, const char *operation)
{
	char			message[BUFSIZE];
	struct Phone	phone;
	char			a[BUFSIZE];
	char			b[BUFSIZE];
	int				result;

	phone_new_server(port, &phone);
	printf("Started server with %s operation on %s\n", operation, port);
	while (1)
	{
		phone_accept(&phone);
		phone_fillbuf(&phone);
		phone_readline(&phone, a, BUFSIZE);
		phone_readline(&phone, b, BUFSIZE);
		result = calculate(operation, a, b);
		snprintf(message, sizeof(message)/sizeof(message[0]),
			"%s %s %s = %d", a, operation, b, result);
		phone_writeline(&phone, message);
		phone_flushbuf(&phone);
		printf("Accepted: %s\n", message);
		phone_close(&phone);
	}
}

void run_client(const char *ip, const char *port, const char *a, const char *b)
{
	struct Phone	phone;
	char 			answer[BUFSIZE];

	phone_new_client(ip, port, &phone);
	phone_writeline(&phone, a);
	phone_writeline(&phone, b);
	phone_flushbuf(&phone);
	phone_fillbuf(&phone);
	phone_readline(&phone, answer, BUFSIZE);
	printf("%s\n", answer);
	phone_close(&phone);
}

int calculate(const char *operation, const char *a, const char *b)
{
	int		x;
	int		y;

	x = atoi(a);
	y = atoi(b);
	if (operation != NULL)
		switch(operation[0])
		{
			case '-': return x - y;
			case '*': return x * y;
			case '/': return x / y;
			case '+': 
			default : break;
		}

	return x + y;
}
