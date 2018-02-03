#include <stdio.h>

const int g = 2;

extern void add_g(void);

int main(void)
{
//	g = 1;

	printf("%s %d\n", __func__, g);
	add_g();
	printf("%s %d\n", __func__, g);
	sizeof g;
}
