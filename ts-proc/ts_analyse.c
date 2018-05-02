#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd = open(argv[1], O_RDONLY);
	int counter = 0, i, other_counter;
	unsigned char buf[188];
	unsigned int temp = 0, pre = 0;
	unsigned int pre_pid = -1;
	unsigned int cur_pid;

#if 0
	if (fd > 0) {
		printf ("fd:%d\n", fd);
		lseek(fd, SEEK_SET, 0);
		while (read(fd, buf, 188) == 188) {
			counter ++;
			if (buf[2] == 0x64) {
				if (counter < 390 || counter > 410)
					printf("========Ts Num: %d\n", counter);
				else
					printf("Ts Num: %d\n", counter);
				printf("[");
				temp = (buf[6] << 24) | (buf[7] << 16)| (buf[8] << 8) | (buf[9]);
				temp = temp * 2 / 9;
				printf("PCR: %u InterVal: %u Bitrate: %u", temp, temp - pre, counter * 188 * 8 / (temp - pre));
				pre = temp;
				printf("]\n");
				counter = 0;
			}
		}
		close(fd);
	}
	else {
		perror("open error!\n");
	}
#endif
	
	if (fd > 0) {
		printf ("fd:%d\n", fd);
		lseek(fd, SEEK_SET, 0);
		while (read(fd, buf, 188) == 188) {
			cur_pid = ((buf[1] & 0x1f) << 8) | buf[2];
			//if (cur_pid == pre_pid) {
			//if (cur_pid == 0x90){
			if (cur_pid == atoi(argv[2])){
				counter ++;
				if (other_counter) 
					printf("other_pid: %d\n", other_counter);
				other_counter = 0;
			}
			else {
				//if (pre_pid != -1)
				//	printf("pid[%#x]: %d\n", pre_pid, counter + 1);
				if (counter)
					printf("pid[%#x]: %d\n", 0x90, counter);
				counter = 0;
				other_counter ++;
				pre_pid = cur_pid;
			}
		}
		close(fd);
	}
	else {
		perror("open error!\n");
	}
}
