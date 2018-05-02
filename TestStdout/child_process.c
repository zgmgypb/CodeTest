#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int i;

	for (i=0; i<100; i++)
	{
		printf ("counter: %d\n", i);
		sleep(1);
		fflush(stdout);
	}

	exit(0);
}
