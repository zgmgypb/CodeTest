#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	FILE *fp;

	if (argv[1])
	{
		fp = fopen(argv[1], "r");
		if (!fp)
		{
			printf ("open %s failed! error:%s\n", argv[1], strerror(errno));
			exit(-1);
		}

		fseek(fp, 0, SEEK_END);
		printf ("%s FileSize: %ld \n", argv[1], ftell(fp));

		fclose (fp);
	}

	return 0;
}
