#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int pox_system(const char *cmd)
{
	return system(cmd);
}

void *system_test(void * arg)
{
	int id = (int) arg;
	int i = 0;
	char str[128] = {0};
	
	switch (id) {
	case 0:
	case 1:
	case 2:
		while (1)
			pox_system("ifconfig eth0 120.120.120.20");
		break;
	case 3:
	case 4:
	case 5:
		while (1)
			pox_system("ls -l");
		break;
	default:
		while (1)
		{
			sprintf (str, "echo [%d]count:%d && cp tmp.xml /mnt/mtd/xxx.xml", id, i++);
			pox_system(str);
		}
		break;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int i = 0;
	pthread_t pid;

	for (i=0; i<10; i++)
	{
		pthread_create(&pid, NULL, system_test, (void *)i);
	}

	while (1) sleep(5);

	return 0;
}
