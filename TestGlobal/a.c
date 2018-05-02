#include <stdio.h>
#include "c.h"

int xx[10] = {0};
extern void b_print(void);
void a_print(void)
{
	int i;

	for (i=0; i<10; i++)
		xx[i] = i;

	for (i=0; i<10; i++)
	{
		printf ("A_print: %d \n", xx[i]);
	}
	printf("A_PRINT_AA: %d\n", aa);
	printf("TRUE: %d\n", TRUE);
}

int main(int argc, char *argv[])
{
	a_print();
	b_print();
}


