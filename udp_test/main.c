#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int i;
	FILE *fd;
	unsigned int data0, data1, data2;
	unsigned char pData[8];

	if (argc >= 2)
	{
		if ((fd = fopen(argv[1], "rb")))
		{
			fread(pData, 1, 4, fd);
			data1 = (pData[1] << 8) | pData[0];
			data2 = (pData[3] << 8) | pData[2];
			for (i=4; ; i += 2)
			{
				if (fread(pData, 1, 2, fd) < 2)
					break;
				data0 = data1;
				data1 = data2;
				data2 = (pData[1] << 8) | pData[0];
				if ((abs(data1-data0) > 0x1000) && (abs(data2-data1) > 0x1000))
				{
					printf ("%08X\n", i);
				}
			}
		}
	}
}
