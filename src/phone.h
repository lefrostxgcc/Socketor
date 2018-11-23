#ifndef SOCKETOR_PHONE_H
#define SOCKETOR_PHONE_H

enum phone_type { PHONE_SERVER, PHONE_CLIENT };
enum { BUFSIZE = 64 };

struct Phone
{
	char			inbuf[BUFSIZE];
	char			outbuf[BUFSIZE];
	int				inpos;
	int				outpos;
	int				client_socket;
	int				server_socket;
	enum phone_type	type;
};

void phone_new_server(const char *port, struct Phone *phone);
void phone_new_client(const char *ip, const char *port, struct Phone *phone);
void phone_accept(struct Phone *phone);
void phone_fillbuf(struct Phone *phone);
void phone_flushbuf(struct Phone *phone);
void phone_readline(struct Phone *phone, char *buf, int bufsize);
void phone_writeline(struct Phone *phone, const char *line);
void phone_close(struct Phone *phone);

#endif
