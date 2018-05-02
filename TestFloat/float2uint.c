#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	float f;
	int i;

	for (i = 1; i < argc; i++) {
		f = (float)atof(argv[i]);
		printf("float: %f, Uint: %08X\n", f, *(unsigned int *)&f);	
	}
}
