#ifndef  __SPI_FLASH__H
#define __SPI_FLASH__H

#include "multi_main_internal.h"

#include "../../../../drivers/flash_driver/flash_driver.h"

#define SPI_FLASH_DEV_NAME "/dev/spi_flash_driver"

#define N25Q256_ID 0x20ba19

//Refer to datasheet for N25Q256
#define FLASH_CMD_RDSR				    0x05			//读状态寄存器 
#define FLASH_CMD_WRSR				    0x01			//写状态寄存器 
#define FLASH_CMD_RD					0x03			//读数据 
#define FLASH_CMD_RD_NOVOLATILE_REG		0xB5			//读不易挥发配置寄存器
#define FLASH_CMD_PP					0x02			//页操作
#define FLASH_CMD_SE					0xd8			//Sector 擦除（分区擦除）
#define FLASH_CMD_WREN				    0x06			//写使能
#define FLASH_CMD_WRDIS				    0x04			//写禁止
#define FLASH_CMD_CE					0xc7			//BULK 擦除（全部擦除）
#define FLASH_CMD_RDID				    0x9f			//读ID, changed from 9f to 9e
#define FLASH_CMD_WREAR			        0xc5			//32M的高部分和低部分选择
#define FLASH_CMD_RD_VOLATILE_REG		0x85			//付易失寄存器
#define BANK_LOW_16M_BYTES				0x00			//写入扩展地址寄存器值，低16M
#define BANK_HIGH_16M_BYTES				0x01			//写入扩展地址寄存器值，高16M
#define FLASH_CMD_sector_count          512
#define ADDRESS_16M						0x1000000

#define FLASH_CMD_page_num_per_sector   256
#define FLASH_CMD_page_size             256
#define APP_READ_FLASH_BUF_SIZE			(2048)
#define APP_WRITE_FLASH_BUF_SIZE		512				//需要是页大小(256)的整数倍

/* FLASH Status Register Mask */
#define STS_WIP		0x01	// Status Write In Progress
#define STS_WEL		0x02	// Status Write Enable Latched
#define STS_SRWP	0x80	// Status Reg. Write Protected
#define TIMEOUT		(0x7FFFFFFF)					// for Polling time out value  changed from 260 to 3*10^12
#define PAGE_PROGRAM_TIMEOUT		(0x7FFFFFFF)	// changed from 100 to 3*10^12

int SpiFlahInit(void);
int CheckFlashId(void);
int FlashWrite(U8 *DataBuf, U32 Address, U32 Length);
int FlashRead(U8 *DataBuf, U32 Address, U32 Length);
int ChipErase(void);
void CmdExecution(long command);
int ChooseSpi(int number);

#define WRITE_ENABLE_NO			0
#define WRITE_ENABLE_YES		1

#define FLASH_CURRENT_BANK_0				   0
#define FLASH_CURRENT_BANK_1				   1
#define FLASH_CURRENT_BANK_UNDEFINED   2

#define CS_CTL_CLR			0
#define CS_CTL_SET			1

#define N25Q256_FLASH_SIZE  0x2000000 //32M bytes
#define N25Q256_FLASH_PAGE_SIZE (256) //256 bytes

//Flash Driver
#define SPI_FLASH_DEV_MINOR 255
#define SPI_FLASH_DRIVER_NAME "spi_flash_driver"

//Choose Spi 
#define SPI_chip_1 0x340
#define SPI_chip_2 0x341
#define SPI_chip_3 0x342
#define SPI_chip_4 0x343


//测试函数
#ifdef DEBUG_SPI_FLASH_INTERFACE
void Test_FSPIF(void);
#endif

#endif
