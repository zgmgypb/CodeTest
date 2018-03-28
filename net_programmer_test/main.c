#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char plString[256];

	if (gethostname(plString, sizeof(plString)) != 0) {
		printf("gethostname error!\n");
		return EXIT_FAILURE;
	}
	printf("hostname: %s\n", plString);
	plString[strlen(plString)] = '.';
	if (getdomainname(plString + strlen(plString) + 1, sizeof(plString) - strlen(plString)) != 0) {
		printf("getdomainname error!\n");
		return EXIT_FAILURE;
	}
	printf("full hostname:%s\n", plString);

	return EXIT_SUCCESS;
}
