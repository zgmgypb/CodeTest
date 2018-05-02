#define _GNU_SOURCE 1
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	float f;

	f = INFINITY;
	printf("INFINITY Mem: %08X\n", *(unsigned int *)&f);
	f = NAN;
	printf("Nan Mem: %08X\n", *(unsigned int *)&f);
}
