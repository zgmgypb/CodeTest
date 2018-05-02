#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	pid_t pid;

	fprintf (stderr, "this is stderr string!\n");
	printf ("this is stdout string!\n");
	
	if ((pid = fork()) == 0)
	{
	
	}
	else if (pid > 0)
	{
	
	}
	else
	{
		fprintf (stderr, "fork error! errno: %d errstr: %s\n", errno, strerror(errno));
	}

	exit(0);
}
