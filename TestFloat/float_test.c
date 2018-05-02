#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void print(float f, char *p)
{
	printf("float: %f\n", f);
	printf("char: %08X\n", (p[3] << 24) & 0xFF000000 
			| (p[2] << 16) & 0xFF0000 | (p[1] << 8) & 0xFF00 | p[0]);
}

int main(int argc, char *argv[])
{
	float a;
	char *p = &a;

	a = 0.25;
	print(a, p);

	a = -8.25;
	print(a, p);

	a = -100000.05;
	print(a, p);

	p[0] = 0x20;
	p[1] = 0;
	p[2] = 0;
	p[3] = 0;
	print(a, p);

	p[0] = 0xff;
	p[1] = 0xff;
	p[2] = 0xff;
	p[3] = 0xff;
	print(a, p);

}
