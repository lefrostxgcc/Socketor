#ifndef SOCKETOR_PHONE_H
#define SOCKETOR_PHONE_H

struct Phone
{
};

void phone_new_server(const char *port, struct Phone *phone);
void phone_new_client(const char *ip, const char *port, struct Phone *phone);
void phone_accept(struct Phone *phone);
const char *phone_readline(struct Phone *phone);
void phone_writeline(struct Phone *phone, const char *str);
void phone_close(struct Phone *phone);

#endif
