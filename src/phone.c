#include "phone.h"

struct Phone phone_create_server(const char *port)
{
	struct Phone	phone;

	return phone;
}

struct Phone phone_create_client(const char *address, const char *port)
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
