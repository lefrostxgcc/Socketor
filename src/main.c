#include <stdio.h>
#include <string.h>

void run_server(const char *port, const char *op);
void run_client(const char *port, const char *op, const char *a, const char *b);

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

void run_server(const char *port, const char *op)
{
}

void run_client(const char *port, const char *op, const char *a, const char *b)
{
}
