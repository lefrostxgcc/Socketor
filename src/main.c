#include <stdio.h>
#include <string.h>
#include "phone.h"

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
	char			message[32];
	struct Phone	phone;
	const char		*a;
	const char		*b;
	int				result;

	phone = phone_create_server(port);
	printf("Started server with %s operation on %s\n", operation, port);
	while (1)
	{
		phone_accept(&phone);
		a = phone_readline(&phone);
		b = phone_readline(&phone);
		result = calculate(operation, a, b);
		snprintf(message, sizeof(message)/sizeof(message[0]),
			"%s %s %s = %d", a, operation, b, result);
		phone_writeline(&phone, message);
		printf("%s\n", message);
		phone_close(&phone);
	}
}

void run_client(const char *ip, const char *port, const char *a, const char *b)
{
	struct Phone	phone;
	const char		*answer;

	phone = phone_create_client(ip, port);
	phone_writeline(&phone, a);
	phone_writeline(&phone, b);
	answer = phone_readline(&phone);
	printf("%s\n", answer);
	phone_close(&phone);
}

int calculate(const char *operation, const char *a, const char *b)
{
	return 0;
}
