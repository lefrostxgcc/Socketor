#include <stdio.h>
#include <string.h>

void run_server(const char *port, const char *op);
void run_client(const char *port, const char *op, const char *a, const char *b);

struct Phone
{
};

int calculate(const char *operation, const char *a, const char *b);
void run_client(const char *port, const char *op, const char *a, const char *b);
struct Phone phone_create_server(const char *port);
void phone_accept(struct Phone *phone);
const char *phone_readline(struct Phone *phone);
void phone_writeline(struct Phone *phone, const char *str);
void phone_close(struct Phone *phone);

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

void run_client(const char *port, const char *op, const char *a, const char *b)
{
}

int calculate(const char *operation, const char *a, const char *b)
{
	return 0;
}

struct Phone phone_create_server(const char *port)
{
	struct Phone	phone;

	return phone;
}

void phone_accept(struct Phone *phone)
{
}

const char *phone_readline(struct Phone *phone)
{
	return "";
}

void phone_writeline(struct Phone *phone, const char *str)
{
}

void phone_close(struct Phone *phone)
{
}
