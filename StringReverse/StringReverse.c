#include <stdio.h>
#include <unistd.h>
#include <string.h>

void StringReverse(char *pStr)
{
	int i = 0;
	int lStrlen = strlen(pStr);

	for (i=lStrlen; i>=0; i--)
		putchar(*(pStr+i));

	putchar('\n');
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf ("Usage: StringReverse String\n");
		return 0;
	}

	StringReverse(argv[1]);
}
