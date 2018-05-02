#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <execinfo.h>

void blackbox_handler(int sig)
{
	printf("Enter blackbox_handler: ");
	printf("SIG name is %s, SIG num is %d\n", strsignal(sig), sig);

	// ¥Ú”°∂—’ª–≈œ¢
	printf("Stack information:\n");
	int j, nptrs;
#define SIZE 100
	void *buffer[100];
	char **strings;

	nptrs = backtrace(buffer, SIZE);
	printf("backtrace() returned %d addresses\n", nptrs);

	strings = backtrace_symbols(buffer, nptrs);
	if (strings == NULL)
	{
		perror("backtrace_symbol");
		exit(EXIT_FAILURE);
	}

	for(j = 0; j < nptrs; j++)
		printf("%s\n", strings[j]);

	free(strings);

	_exit(EXIT_SUCCESS);
}

long count = 0;
void bad_iter()
{
	int a, b, c, d; 
	a = b = c = d = 1;
	a = b + 3;
	c = count + 4;
	d = count + 5 * c;
	count++;
	printf("count:%ld\n", count);

	bad_iter();
}

int main()
{
	stack_t ss;
	struct  sigaction   sa;

	s.ss_sp = malloc(SIGSTKSZ);
	s.ss_size = SIGSTKSZ;
	s.ss_flags = 0;
	if (sigaltstack(&ss, NULL) == -1)
	{
		return EXIT_FAILURE;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = blackbox_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_ONSTACK;

	if (sigaction(SIGSEGV, &sa, NULL) < 0)
	{
		return EXIT_FAILURE;
	}

	bad_iter();

	while(1);

	return EXIT_SUCCESS;
}


