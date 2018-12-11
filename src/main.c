#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <signal.h>
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
	pid_t			pid;
	pid_t			ctid;
};

struct thread_info_list
{
	struct thread_info *node;
	struct thread_info_list *prev;
	struct thread_info_list *next;
};

static int run(void *arg);
static void accept_clients(struct thread_info *accept_thread);

void run_server(const char *port, const char *op);
void run_client(const char *ip, const char *port, const char *a, const char *b);
int calculate(const char *operation, const char *a, const char *b);
static struct thread_info_list *
add_thread_to_list(struct thread_info_list *head, struct thread_info *node);
static struct thread_info_list *
remove_from_list(struct thread_info_list **head, struct thread_info *node);

static void termination_handler(int signum);
static void show_usage_message(void);
static void free_finished_threads();

static struct thread_info		*accept_thread;
static struct thread_info_list	*thread_list_head;

int main(int argc, char *argv[])
{
	struct sigaction sigint_action = {};

	sigint_action.sa_handler = termination_handler;

	if (sigaction(SIGINT, &sigint_action, NULL) < 0)
	{
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

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

static void termination_handler(int signum)
{
	free_finished_threads();
	free(accept_thread);
	exit(0);
}

static void show_usage_message(void)
{
	fprintf(stderr,
			"Usage: socketor server port operation\n"
			"Usage: socketor client address port a b\n");
}

static void free_finished_threads(void)
{
	struct thread_info_list *curr;
	struct thread_info_list *next;
	struct thread_info_list *del;

	for (curr = thread_list_head; curr; curr = next)
	{
		next = curr->next;
		if (curr->node->ctid == 0)
		{
			del = remove_from_list(&thread_list_head, curr->node);
			if (del)
			{
				free(del->node);
				free(del);
			}
		}
	}
}

void run_server(const char *port, const char *operation)
{
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
	int						block_timeout_ms;

	flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |
		CLONE_SYSVSEM | CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID;

	while (1)
	{
		block_timeout_ms = thread_list_head ? 10 * 1000 : -1;
		phone_accept(&accept_thread->phone, block_timeout_ms);
		if (accept_thread->phone.client_socket > 0)
		{
			thread = (struct thread_info *) malloc(sizeof(struct thread_info));
			thread_list_head = add_thread_to_list(thread_list_head, thread);
			thread->phone = accept_thread->phone;
			thread->operation = accept_thread->operation;
			thread->pid = clone(run, thread->stack + STACK_SIZE,
				flags, thread, NULL, NULL, &thread->ctid);
			if (thread->pid < 0)
			{
				perror("clone");
				exit(EXIT_FAILURE);
			}
		}
		free_finished_threads();
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
	snprintf(message, sizeof(message)/sizeof(message[0]),
		"%s %s %s = %d", a, operation, b, result);
	phone_writeline(phone, message);
	phone_flushbuf(phone);
	printf("Accepted: %s\n", message);
	phone_close(phone);
}

static struct thread_info_list *
add_thread_to_list(struct thread_info_list *head, struct thread_info *node)
{
	struct thread_info_list *tail;
	struct thread_info_list *curr;

	tail = (struct thread_info_list *) malloc(sizeof(struct thread_info_list));
	tail->node = node;
	tail->next = NULL;

	if (head == NULL)
	{
		tail->prev = NULL;
		head = tail;
	}
	else
	{
		for (curr = head; curr->next != NULL; curr = curr->next)
			;

		tail->prev = curr;
		curr->next = tail;
	}

	return head;
}

static struct thread_info_list *
remove_from_list(struct thread_info_list **head, struct thread_info *node)
{
	struct thread_info_list *curr;

	if (!head || !node)
		return NULL;

	for (curr = *head; curr && curr->node != node; curr = curr->next)
		;

	if (!curr)
		return NULL;
	if (curr->prev != NULL)
		curr->prev->next = curr->next;
	else
	{
		*head = curr->next;
		if (*head)
			(*head)->prev = NULL;
		return curr;
	}
	if (curr->next != NULL)
		curr->next->prev = curr->prev;

	return curr;
}
