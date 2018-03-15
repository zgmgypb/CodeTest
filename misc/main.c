#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void strdup_test(void)
{
	char *str="hello world!";

	printf("strdup(\"hello\"):%s\n", strdup("hello"));
}

char *str_invert(char *str)
{
	char *p;

	if (!str) {
		return NULL;
	}

	p = (char *)malloc(strlen(str) + 1);
	p = &p[strlen(str)];
	*p = '\0';
	while (*str != '\0') {
		*(--p) = *(str++);
	}

	return p;
}

unsigned char bit_reverse(unsigned char c)
{
	unsigned char ret = 0;
	int i;

	for (i = 0; i < 8; i++) {
		ret = (ret << 1) | ((c >> i) & 0x01);
	}
	
	return ret;
}

int main(int argc, char *argv[])
{
	strdup_test();
#if 0
	int a = 5, b = 7, c;
	const int *d = &b;

	printf("7 bit reverse:%d\n", bit_reverse(7));
	printf("nihao str_invert:%s\n", str_invert("nihao"));
	{
		int i = 3;
	}

	{
		int j = 5;
	}
#endif
#if 0
	//*d = 200;
	printf("d=%p\n", d);
	d = &a;
	printf("d=%p\n", d);
	//*d = 20;
	printf("*d=%d\n", *d);

	c = a+++b;
	printf("a = %d, b = %d, c = %d\n", a, b, c);
	if ('\0') {
		printf("\'\\0\' = %d\n", '\0');
	}
	else {
		printf("NULL!! \'\\0\' = %d\n", '\0');
	}
#endif
}
