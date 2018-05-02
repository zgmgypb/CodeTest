#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	FILE *fp1, *fp2;

	if (argv[1] && argv[2])
	{
		fp1 = fopen(argv[1], "r");
		if (!fp1)
		{
			printf ("open %s failed! error:%s\n", argv[1], strerror(errno));
			exit(-1);
		}
		
		fp2 = fopen(argv[2], "r");
		if (!fp2)
		{
			printf ("open %s failed! error:%s\n", argv[2], strerror(errno));
			exit(-1);
		}

		fseek(fp1, 0, SEEK_END);
		fseek(fp2, 0, SEEK_END);

		if (ftell(fp1) != ftell(fp2))
		{
			printf ("two file Not Same!\n");
			fclose (fp1);
			fclose (fp2);
			exit (0);
		}
		fseek(fp1, 0, SEEK_CUR);
		fseek(fp2, 0, SEEK_CUR);
		
		while (!feof(fp1) || !feof(fp2))
		{
			if (fgetc(fp1) != fgetc(fp2))
			{
				printf ("two file Not Same!\n");
				fclose (fp1);
				fclose (fp2);
				exit (0);
			}
		}

		fclose (fp1);
		fclose (fp2);
		printf ("two file is Same!\n");
	}

	return 0;
}
