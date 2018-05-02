#include <stdio.h>
#include "c.h"

int xx[10];
void b_print(void)
{
	int i;

	for (i=0; i<10; i++)
	{
		printf ("B_print: %d \n", xx[i]);
	}

	printf ("B_print_a: %d\n", aa);
}
