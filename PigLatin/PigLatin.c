#include <stdio.h>
#include <unistd.h>
#include <string.h>

void PigLatin(char *pStr)
{
	char lVowelTable[] = {'a', 'e', 'i', 'o', 'u'};
	int i;

	for (i=0; i<sizeof(lVowelTable); i++)
	{
		if ((*pStr != lVowelTable[i]) && (*pStr != lVowelTable[i] - 0x20)) /* is consonant */
		{
			
		}
	}
}

int main(int argc, char *argv[])
{
	
}
