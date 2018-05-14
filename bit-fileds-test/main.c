#include <stdio.h>
#include <unistd.h>
#include <stddef.h>

#pragma pack(8)
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
		int m1:2;
		int m2:4;
		int :0; /* 无名位段，无名位段的作用是手动对齐前面的位段，该位段之后从下一个内存单元对齐 */
		int m3:6;
		int m4;
	}lStTmp;

	printf("m4 offset:%d\n", offsetof(struct stTmp, m4));
	//printf("m2 offset:%d\n", offsetof(struct stTmp, m2));
	//printf("m3 offset:%d\n", offsetof(struct stTmp, m3));
	lStTmp.m1 = 0x07;
	lStTmp.m2 = 0x7;
	lStTmp.m3 = 0x19;
	printf("value = %X\n", *(unsigned *)&lStTmp);
	printf("m1 = %u\n", lStTmp.m1 & 0x03);
	printf("struct size: %d\n", sizeof(lStTmp));
	printf("pc is %s mode\n", (big_little_endian() == 1 ? "big_endian" : "little_endian"));

	exit(0);
}
