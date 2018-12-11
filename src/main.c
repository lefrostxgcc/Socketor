#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "phone.h"

#define		BUFSIZE		32

enum { STACK_SIZE = 1 << 16, FUTEX_LOCK = 0, FUTEX_UNLOCK = 1 };
struct thread_info
{
	char			stack[STACK_SIZE];
	struct Phone	phone;
	const char		*operation;
	int				pid;
	int				ctid;
};

static int futex(int *uaddr, int futex_op, int val,
					const struct timespec *timeout, int *uaddr2, int val3);
static void wait_on_futex(int *futex_addr, int lock_val, int unlock_val);
static void wake_up_futex(int *futex_addr, int lock_val, int unlock_val);
static int run(void *arg);
static void accept_clients(struct thread_info *accept_thread);

void run_server(const char *port, const char *op);
void run_client(const char *ip, const char *port, const char *a, const char *b);
int calculate(const char *operation, const char *a, const char *b);

static void show_usage_message(void);

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		show_usage_message();
		return 1;
	}

	if (strcmp(argv[1], "server") == 0 && argc == 4)
		run_server(argv[2], argv[3]);
	else if (strcmp(argv[1], "client") == 0 && argc == 6)
		run_client(argv[2], argv[3], argv[4], argv[5]);
	else
	{
		show_usage_message();
		return 1;
	}

	return 0;
}

static void show_usage_message(void)
{
	fprintf(stderr,
			"Usage: socketor server port operation\n"
			"Usage: socketor client address port a b\n");
}

void run_server(const char *port, const char *operation)
{
	struct thread_info		*accept_thread;

	accept_thread = (struct thread_info *) malloc(sizeof(struct thread_info));

	phone_new_server(port, &accept_thread->phone);
	accept_thread->operation = operation;
	printf("Started server with %s operation on %s\n", operation, port);
	accept_clients(accept_thread);
	free(accept_thread);
}

static void accept_clients(struct thread_info *accept_thread)
{
	struct thread_info		*thread;
	int						flags;

	flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |
		CLONE_SYSVSEM;

	while (1)
	{
		phone_accept(&accept_thread->phone);
		thread = (struct thread_info *) malloc(sizeof(struct thread_info));
		thread->phone = accept_thread->phone;
		thread->operation = accept_thread->operation;
		if ((clone(run, thread->stack + STACK_SIZE,
			flags, thread, NULL, NULL, NULL)) < 0)
		{
			perror("clone");
			exit(EXIT_FAILURE);
		}
		sleep(1);
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

static int run(void *arg)
{
	char			message[BUFSIZE];
	char			a[BUFSIZE];
	char			b[BUFSIZE];
	struct Phone	*phone;
	const char		*operation;
	int				result;
	
	phone = &((struct thread_info *) arg)->phone;
	operation = ((struct thread_info *) arg)->operation;
	phone_fillbuf(phone);
	phone_readline(phone, a, BUFSIZE);
	phone_readline(phone, b, BUFSIZE);
	result = calculate(operation, a, b);
	sleep(3);
	snprintf(message, sizeof(message)/sizeof(message[0]),
		"%s %s %s = %d", a, operation, b, result);
	phone_writeline(phone, message);
	phone_flushbuf(phone);
	printf("Accepted: %s\n", message);
	phone_close(phone);
}

static int futex(int *uaddr, int futex_op, int val,
					const struct timespec *timeout, int *uaddr2, int val3)
{
	return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

static void wait_on_futex(int *futex_addr, int lock_val, int unlock_val)
{
	int		status;

	while (1)
	{
		if (__sync_bool_compare_and_swap(futex_addr, unlock_val, lock_val))
			break;

		status = futex(futex_addr, FUTEX_WAIT, lock_val, NULL, NULL, 0);

		if (status < 0 && errno != EAGAIN)
		{
			perror("futex wait");
			exit(1);
		}
	}
}

static void wake_up_futex(int *futex_addr, int lock_val, int unlock_val)
{
	int		status;

	if (__sync_bool_compare_and_swap(futex_addr, lock_val, unlock_val))
	{
		status = futex(futex_addr, FUTEX_WAKE, unlock_val, NULL, NULL, 0);
		if (status < 0)
		{
			perror("futex wake");
			exit(1);
		}
	}
}
