#ifndef _SPI_FLASH_UPGRADE_H
#define _SPI_FLASH_UPGRADE_H

#include "multi_main_internal.h"

#define UPGRADE_FILE_SIZE_32M 33554432
#define ONE_MB 1048576

#define WRITE_TO_FLASH_FILE_PATH       "/var/tmp/web/upload/upload.bin"   

#define ENCODER_SUB_BOARD_NUM 4
#define WRITE_TO_FLASH_FILE_PATH_LEN  100
#define READ_FROM_FLASH_FILE_PATH_LEN  100
#define SYSTEM_CMD_LEN  150
#define WRITE_FLASH_BUF_SIZE	512//需要是页大小(256)的整数倍
#define READ_FLASH_BUF_SIZE	512 
#define FILE_HEADER_LENGTH  2

#define UPGRADE_STATUS_FINISH 0
#define UPGRADE_STATUS_RUNNING 1

#define UPGRADE_RESULT_SUCCESS 0
#define UPGRADE_RESULT_FAIL 1

typedef struct  
{
	U32 s_FirmwareFileIndex;  //0~7
	U32 s_FirmwareFileNumber; //8
	F32 s_FilewareFileWrotePercent; //已经写入字节数目
	U8  s_EncoderUpgradeStatus;
	U8  s_EncoderUpgradeResult;
}MULT_EncoderUpgradeInfo;

void spiFlashUpgradeThread(void* param);
MULT_EncoderUpgradeInfo* GetEncoderUpgradeInfo();

#endif
