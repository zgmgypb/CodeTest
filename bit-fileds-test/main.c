#include <stdio.h>
#include <unistd.h>

int big_little_endian(void)
{
	unsigned n = 0x12345678;

	if (*(char *)&n == 0x12) 
		return 1; /* big_endian */
	else 
		return 0; /* little_endian */
}

int main(int argc, char *argv[])
{
	struct stTmp {
		char m1:2;
		char m2:4;
		char m3:6;
	}lStTmp;

	lStTmp.m1 = 0x07;
	lStTmp.m2 = 0x7;
	lStTmp.m3 = 0x19;
	printf("value = %X\n", *(unsigned *)&lStTmp);
	printf("m1 = %u\n", lStTmp.m1 & 0x03);
	printf("struct size: %d\n", sizeof(lStTmp));
	printf("pc is %s mode\n", (big_little_endian() == 1 ? "big_endian" : "little_endian"));

	exit(0);
}
