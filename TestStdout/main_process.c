#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int pipe_fd[2];
	pid_t pid;
	char pipe_buf[1024];
	int ret;

	if (pipe(pipe_fd) < 0)
		fprintf(stderr, "pipe failed!\n");

	if ((pid = fork()) == 0) /* child */
	{
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[1]);
		if (execl("./child_process", "./child_process", NULL) == -1)
			fprintf (stderr, "execl faild!\n");
		exit(0);
	}
	else if (pid > 0) /* parent */
	{
		close(pipe_fd[1]);
		while ((ret = read(pipe_fd[0], pipe_buf, sizeof(pipe_buf) - 1)) > 0)
		{
			pipe_buf[ret] = '\0';
			printf ("%s\n", pipe_buf);
			fflush(stdout);
		}
		close(pipe_fd[0]);
	}
	else
		fprintf (stderr, "fork failed!\n");

	exit(0);
}
