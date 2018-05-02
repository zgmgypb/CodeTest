#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <pthread.h>

void fd_lock(int fd)
{
	flock(fd, LOCK_EX);
}
void fd_unlock(int fd)
{
	flock(fd, LOCK_UN);
}

int s_fd = 0;

void *FileLockThread(void *arg)
{
	int count = 0, i = 0;

	while (1)
	{
		fd_lock(s_fd);
		printf("TID[%d] ", (int)arg);
		for (i=0; i<16; i++)
		{
			printf("%02X ", count++ % 256);
			usleep(1);
		}
		printf("\n");
		fd_unlock(s_fd);

		usleep(10);
	}
}

int main(int argc, char *argv[])
{
	int i;
	pthread_t plThreadPid[10];

	s_fd = open("./file_lock_test.c", O_RDWR);
	for (i=0; i<10; i++)
	{
		pthread_create (&plThreadPid[i], NULL, FileLockThread, (void *)i);
	}

	while (1)
		sleep(100);

	return 0;
}
