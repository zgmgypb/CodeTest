#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	unsigned char left[2], right[2];
	int left_fd, right_fd, stero_fd;

	left_fd = open(argv[1], O_RDONLY);
	right_fd = open(argv[2], O_RDONLY);
	stero_fd = open(argv[3], O_CREAT | O_RDWR);

	while (read(left_fd, left, 2) == 2) {
		if (read(right_fd, right, 2) == 2) {
			write(stero_fd, left, 2);
			write(stero_fd, right,2);
		}
	}
}
