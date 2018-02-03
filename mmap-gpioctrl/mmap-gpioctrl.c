/*
 * 使用 mmap 函数映射出 /dev/mem, 直接控制虚拟地址，达到控制 GPIO 的效果
 * 在 GN1828 的硬件平台进行测试
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define GPIO_SET(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + gpio * 0x200 + 0x30) | n)
#define GPIO_CLEAR(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + gpio * 0x200 + 0x34) | n)
#define PIN_OUT(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + gpio * 0x200 + 0x10) | n)
#define PIN_IN(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + gpio * 0x200 + 0x14) | n)

int main(int argc, char **argv)
{
	int fd;
	char *gpio_base;

	if ((fd = open("/dev/mem", O_RDWR)) == -1) {
		perror("open file err!\n");	
		return -1;
	}

	gpio_base = (char *)mmap(0, 0xc00, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xffff400);
	if (gpio_base == NULL) {
		printf("gpio base mmap is error!\n");	
		close(fd);
		return -1;
	}

	PIN_OUT(gpio_base, 1, 17);
	while(1) {
		GPIO_SET(gpio_base, 1, 17);
		sleep(1);
		GPIO_CLEAR(gpio_base, 1, 17);
		sleep(1);
	}

	munmap(0, 0xc00);
	close(fd);
}
