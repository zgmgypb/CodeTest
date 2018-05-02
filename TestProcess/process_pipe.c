#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int pipe_fd[2];
	int pid = -1;

	if (pipe(pipe_fd) < 0)
		printf ("pipe failed!\n");

	{
		int i;
		int fl;
			
		for (i=0; i<2; i++)
		{
			fl = fcntl(pipe_fd[i], F_GETFL);
			fl |= O_NONBLOCK;
			fcntl(pipe_fd[i], F_SETFL, fl);
		}
	}

	if ((pid = fork()) == 0)
	{
		unsigned int cout = 0;
		char str[128] = {0};
		close (pipe_fd[0]);
		while (1)
		{
			sprintf (str, "[%d]abcdefg", cout++);
			write(pipe_fd[1], str, strlen(str));
			usleep(1);
		}
	}
	else if (pid > 0)
	{
		close(pipe_fd[1]);
		sleep (15);
		while (1)
		{
			char buffer[1024] = {0};

			if (read(pipe_fd[0], buffer, 1024) > 0)
				printf ("[P] %s\n", buffer);
		}
	}
	else
	{
		printf ("fork failed!\n");
	}

	return 0;
}
