#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{	
	extern char *optarg;
	extern int optind, opterr, optopt;
	int i;
	int ret;
	
	for (i=0; i<argc; i++)
	{
		printf ("argv[%d] %s\n", i, argv[i]);
	}
	printf ("\n");

	while ((ret = getopt(argc, argv, ":5a:b::c")) != -1)
	{
		switch (ret) {
		case 'a':
			printf ("option: %c argv: %s\n", ret, optarg);
			break;
		case 'b':
			if (optarg)
				printf ("option: %c argv: %s\n", ret, optarg);
			else
				printf ("option: %c no argument\n", ret);
			break;
		case '?':
			printf ("encountered a unrecognized option: %c, argv: %s\n", optopt, argv[optind - 1]);
			break;
		case ':':
			printf ("option: %c missing argument\n", optopt);
			break;
		default:
			printf ("option: %c\n", ret);
			break;
		}
	}

	printf ("\noptind: %d\n\n", optind);
	for (i=optind; i>0 && i<argc; i++)
		printf ("argv[%d] %s\n", i, argv[i]);

	printf ("\n");
	for (i=0; i<argc; i++)
		printf ("argv[%d] %s\n", i, argv[i]);

	return 0;
}
