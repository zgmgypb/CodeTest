#include <stdio.h>
#include <malloc.h>

int main(int argc, char *argv[])
{
	printf("%#x\n", (0x87654321 & 0xff) | (~0x87654321 & ~0xFF));
}	

