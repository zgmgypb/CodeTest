/*
 * 使用 mmap 函数映射出 /dev/mem, 直接控制虚拟地址，达到控制 GPIO 的效果
 * 在 GN1828 的硬件平台进行测试
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#define GPIO_SET(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + 0x400 + gpio * 0x200 + 0x30) |= (1 << n))
#define GPIO_CLEAR(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + 0x400 + gpio * 0x200 + 0x34) |= (1 << n))
#define PIN_OUT(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + 0x400 + gpio * 0x200 + 0x10) |= (1 << n))
#define PIN_IN(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + 0x400 + gpio * 0x200 + 0x14) |= (1 << n))
#define PIN_EN(gpio_base_addr, gpio, n) (*(unsigned *)(gpio_base_addr + 0x400 + gpio * 0x200) |= (1 << n))

int main(int argc, char **argv)
{
	int fd;
	char *gpio_base;
	unsigned pin_addr;
	unsigned page_size;

	if ((fd = open("/dev/mem", O_RDWR)) == -1) {
		perror("open file err!\n");	
		return -1;
	}

	// 注意，使用 mmap 一定要使用页对齐映射，所以在实际应用中先要获取内存页大小后在使用
	gpio_base = (char *)mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xfffff000); 
	if (gpio_base == NULL) {
		printf("gpio base mmap is error!\n");	
		close(fd);
		return -1;
	}

	page_size = sysconf(_SC_PAGESIZE);
	printf("page_size:%d\n", page_size);

	PIN_EN(gpio_base, 1, 17);
	PIN_OUT(gpio_base, 1, 17);
	while(1) {
		GPIO_SET(gpio_base, 1, 17);
		printf("LEVEL_HIGH!\n");
		sleep(1);
		GPIO_CLEAR(gpio_base, 1, 17);
		printf("LEVEL_LOW!\n");
		sleep(1);
	}

	munmap(0, 0xc00);
	close(fd);
}
