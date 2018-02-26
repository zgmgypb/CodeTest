#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 

#include "spi_flash_upgrade.h"
#include "spi_flash.h"

static MULT_EncoderUpgradeInfo EncoderUpgradeInfoEntry;

MULT_EncoderUpgradeInfo* GetEncoderUpgradeInfo()
{
	return &EncoderUpgradeInfoEntry;
}

BOOL writeFileToFlash(U32 WriteFileIndex, U32 TotalFileNumber)
{
	int i;
	int Temp;
	int FileSize = 0;
	int TotalCounter = 0;
	int ReadedBytes = 0;
	int CheckFlashResult;
	int CurrentFileIndex;
	
	int fd;
	U8	CrcBuf[4];
	U32 WriteAddress; 
	U32 FlashReadAddress;
	U32 ReadFlashIdCount;
	U8	SystemCmd[SYSTEM_CMD_LEN];	
	U8	FileHeaderBuf[FILE_HEADER_LENGTH];
	U8	WriteFlashBuf[WRITE_FLASH_BUF_SIZE];
	U32 WritedBytesCount[ENCODER_SUB_BOARD_NUM];
	U8	WriteToFlashFilePath[WRITE_TO_FLASH_FILE_PATH_LEN];

	int EncoderSubBoardChipUpgradeStatus[ENCODER_SUB_BOARD_NUM]; //if change to static para?

	//add by leonli for debug
	//int debug_printf_count = 100;

	FILE *pFD;
	unsigned int lCrc32Key;

	for (i = 0; i < ENCODER_SUB_BOARD_NUM; ++i)
	{
		EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_SUCCESS;
		GLOBAL_TRACE(("Chip %d status is %d\n" , i, EncoderSubBoardChipUpgradeStatus[i]));
	}

	SpiFlahInit();	

	for (i = 0; i < ENCODER_SUB_BOARD_NUM; ++i)
	{
		if (EncoderSubBoardChipUpgradeStatus[i] == UPGRADE_RESULT_FAIL && EncoderSubBoardChipUpgradeStatus[i] != 5) // 5 means correction or redo see ChooseSpi()
		{
			//if this chip had been upgraded success ,so skip this one
			GLOBAL_TRACE(("Chip %d is skipped due to chip status %d\n", i, EncoderSubBoardChipUpgradeStatus[i]));
			continue;
		}
		FileSize=0;
		TotalCounter=0;
		ChooseSpi(i);//Need to change this function code use 0 for the first case

		//Check if flash is detected!
		ReadFlashIdCount = 10;
		while(ReadFlashIdCount)
		{
			CheckFlashResult = CheckFlashId();
			if (!CheckFlashResult)
			{
				break;
			}
			ReadFlashIdCount--;
		}
		if(!ReadFlashIdCount)  //check the id of flash to make sure flash opened correctly
		{
			GLOBAL_TRACE(("N25Q256 spi flash is not detected !\n"));
			EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_FAIL;

			//if can't detecte flash ,so skip this one
			continue;
		}
		////////////////////////////////

		//if the first file to write , need to earse flash
		if(WriteFileIndex == 1) // before writing the first part of updating file into flash WriteFileIndex = 0,otherwise it is nonzero
		{ 
			GLOBAL_TRACE(("-----------------------------------------Erase Flash!\n"));
			ChipErase(); //Erase the flash only for the first time writing. comment out for debugging
			GLOBAL_TRACE(("-----------------------------------------Erase Flash End!\n"));
		}

		GLOBAL_TRACE(("Enter the flash read and write porcess.\n"));

		WritedBytesCount[i] = 0;//initialize the write bytes TotalCounter

		//add by leonli for HTMl show upgrade process
		EncoderUpgradeInfoEntry.s_FilewareFileWrotePercent = 0;//Because WritedBytesCount[i] is zero.
		GLOBAL_TRACE(("Upgrade process percent is : %f \n" , EncoderUpgradeInfoEntry.s_FilewareFileWrotePercent));

		memset(WriteToFlashFilePath,0,WRITE_TO_FLASH_FILE_PATH_LEN);//initialize the address buffer by setting all bytes to 0	
		strcpy(WriteToFlashFilePath, WRITE_TO_FLASH_FILE_PATH);//copy the file path into the address buffer

		GLOBAL_TRACE(("Write file name is %s.\n",WriteToFlashFilePath));

		////////////////////////////Open file process///////////////////////////////
		pFD = fopen(WriteToFlashFilePath,"r"); //open file with binary read mode

		//Get file size
		if (pFD)
		{
			fseek(pFD,0,SEEK_END);
			FileSize = ftell(pFD); // get the length of the file to be written into flash include CRC
			fclose(pFD);
			FileSize = FileSize - FILE_HEADER_LENGTH;// get rid of the length of the header

			GLOBAL_TRACE(("Upgrade File Size is %d.\n" , FileSize));
		}
		else
		{
			GLOBAL_TRACE(("Open File [%s] Failed!\n" , WriteToFlashFilePath));
			EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_FAIL;
			continue;
		}

		fd = open(WriteToFlashFilePath, O_RDONLY);//open the file to be loaded in read-only mode

		if (fd < 0)
		{
			GLOBAL_TRACE(("Open file %s failed\n." , WriteToFlashFilePath));
			EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_FAIL;
			continue;
		}

		////////////////////////////Write file process///////////////////////////////
		memset(WriteFlashBuf,0,WRITE_FLASH_BUF_SIZE);//initialize the write buffer	
		memset(FileHeaderBuf,0,FILE_HEADER_LENGTH);	

		//Read the first two bytes file header 
		ReadedBytes = read(fd, FileHeaderBuf, FILE_HEADER_LENGTH);

		if (ReadedBytes == -1)
		{
			GLOBAL_TRACE(("Read file header failed!\n"));
			return FALSE;
		}
		else
		{
			if (FileHeaderBuf[0] != 0)//first reserved byte is zero
			{
				GLOBAL_TRACE(("File is wrong!\n"));
				return FALSE;
			}
			else
			{  
				if ((UPGRADE_FILE_SIZE_32M / TotalFileNumber % ONE_MB))
				{
					Temp = (UPGRADE_FILE_SIZE_32M / TotalFileNumber / ONE_MB) + 1;
				}
				else
				{
					Temp = (UPGRADE_FILE_SIZE_32M / TotalFileNumber / ONE_MB);
				}

				GLOBAL_TRACE(("File size offset is %d MB.\n", Temp));

				WriteAddress = Temp * (FileHeaderBuf[1] - 1) * ONE_MB;// calculate the WriteAddress according to file header
				//WriteAddress=(UPGRADE_FILE_SIZE_32M/TotalFileNumber)*(FileHeaderBuf[1]-1);

				GLOBAL_TRACE(("Write Address is %d.\n", WriteAddress));

				// CurrentFileIndex=WriteAddress*TotalFileNumber/UPGRADE_FILE_SIZE_32M+1;
				CurrentFileIndex = FileHeaderBuf[1];
			}
		}

		FlashReadAddress = WriteAddress;

		while((ReadedBytes = read(fd,  WriteFlashBuf, WRITE_FLASH_BUF_SIZE)) != -1)
		{
			if (ReadedBytes == 0)//file end
			{
				break;
			}

			TotalCounter += ReadedBytes;

			if (TotalCounter < FileSize)
			{
				if(FlashWrite(WriteFlashBuf, WriteAddress, ReadedBytes) != 0)	
				{
					GLOBAL_TRACE(("Flash write failed!\n"));
					EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_FAIL;
					break;
				}
				WritedBytesCount[i] += ReadedBytes;
				WriteAddress += ReadedBytes;
				memset(WriteFlashBuf,0,WRITE_FLASH_BUF_SIZE);

				//add by leonli for HTMl show upgrade process
				EncoderUpgradeInfoEntry.s_FilewareFileWrotePercent = (F32)WritedBytesCount[i] / (F32)FileSize;
				
				/*if(debug_printf_count == 0)
				{
					GLOBAL_TRACE(("Upgrade process [%d] percent is : %f \n" , i, EncoderUpgradeInfoEntry.s_FilewareFileWrotePercent));
					debug_printf_count = 100;
				}
				else
				{
					debug_printf_count--;
				}*/

			}
			else if (TotalCounter == FileSize)//if this is the last part of the file then do not write the last 4 bytes CRC
			{
				if(FlashWrite(WriteFlashBuf, WriteAddress, ReadedBytes - 4) != 0)
				{
					GLOBAL_TRACE(("Flash write last file part failed!\n"));
					EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_FAIL;
					continue;
				}

				WritedBytesCount[i] += ReadedBytes - 4;
				WriteAddress += ReadedBytes - 4;

				//Temp save crc 4 bytes
				CrcBuf[0] = WriteFlashBuf[ReadedBytes - 4];
				CrcBuf[1] = WriteFlashBuf[ReadedBytes - 3];
				CrcBuf[2] = WriteFlashBuf[ReadedBytes - 2];
				CrcBuf[3] = WriteFlashBuf[ReadedBytes - 1];
				GLOBAL_TRACE(("CRC is 0x%x 0x%x 0x%x 0x%x\n", CrcBuf[0], CrcBuf[1], CrcBuf[2], CrcBuf[3]));
				memset(WriteFlashBuf,0,WRITE_FLASH_BUF_SIZE);
			}
			else 
			{
				break;
			}
		}
		close(fd);

		TotalCounter=0;
		GLOBAL_TRACE(("File %s is wrote to flash.\n" , WriteToFlashFilePath));
		memset(WriteFlashBuf,0,READ_FLASH_BUF_SIZE);
		GLOBAL_TRACE(("There are %d bytes in Flash\n",WritedBytesCount[i]));

		//add by leonli for HTMl show upgrade process
		EncoderUpgradeInfoEntry.s_FilewareFileWrotePercent = (F32)WritedBytesCount[i] / (F32)FileSize;
		GLOBAL_TRACE(("Upgrade process percent is : %f \n" , EncoderUpgradeInfoEntry.s_FilewareFileWrotePercent));

		lCrc32Key = 0xffffffff;
		lCrc32Key = CRYPTO_CRC32(lCrc32Key, (U8*)FileHeaderBuf, FILE_HEADER_LENGTH);//include the file header in CRC calculation 

		if (WritedBytesCount[i] <= WRITE_FLASH_BUF_SIZE)//If the file is very small
		{
			if ((ReadedBytes = FlashRead(WriteFlashBuf, FlashReadAddress, WritedBytesCount[i])) != -1)                
			{
				FlashReadAddress += WritedBytesCount[i];
				lCrc32Key = CRYPTO_CRC32(lCrc32Key, (U8*)WriteFlashBuf, WritedBytesCount[i]);     
			}
			else
			{
				GLOBAL_TRACE(("Flash read failed!\n"));
				EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_FAIL;
				continue;
			}
		}
		else
		{
			while(((ReadedBytes = FlashRead(WriteFlashBuf, FlashReadAddress, WRITE_FLASH_BUF_SIZE)) != -1) && (TotalCounter < WritedBytesCount[i]))
			{
				if (ReadedBytes == 0)//File end
				{
					GLOBAL_TRACE(("Break the loop due to the end of the flash content\n"));
					break;
				}
				TotalCounter += ReadedBytes;
				lCrc32Key = CRYPTO_CRC32(lCrc32Key, (U8*)WriteFlashBuf, ReadedBytes);
				memset(WriteFlashBuf, 0, READ_FLASH_BUF_SIZE);
				FlashReadAddress += ReadedBytes;
			}
		}

		GLOBAL_TRACE(("Size of the file is %d, CRC is %x.\n", WritedBytesCount[i], lCrc32Key));
		GLOBAL_TRACE(("The initial CRC is 0x%x 0x%x 0x%x 0x%x.\n", CrcBuf[0], CrcBuf[1], CrcBuf[2], CrcBuf[3]));

		if ((CrcBuf[0] == ((lCrc32Key >> 24) & 0xff)) &&// compare CRC info to verify the rightness of data
			(CrcBuf[1] == ((lCrc32Key >> 16) & 0xff)) &&
			(CrcBuf[2] == ((lCrc32Key >>  8) & 0xff)) &&
			(CrcBuf[3] == (lCrc32Key& 0xff))  )
		{
			GLOBAL_TRACE(("CRC OK!\n"));//CRC verification passed, update the WriteAddress
			EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_SUCCESS;//Record the success operation
		}
		else 
		{
			GLOBAL_TRACE(("Chip %d has CRC bug\n", i));
			EncoderSubBoardChipUpgradeStatus[i] = UPGRADE_RESULT_FAIL; // record the failure
		} 
		GLOBAL_TRACE(("Bytes written to spi flash is %d\n", WritedBytesCount[i]));
	}

	GLOBAL_TRACE(("Current File Index is %d \n", CurrentFileIndex));

	//Remove the temp upgrade file
	memset(SystemCmd, 0, SYSTEM_CMD_LEN);//initialize the syscmd buf
	strcpy(SystemCmd, "rm -f ");
	strcat(SystemCmd, WRITE_TO_FLASH_FILE_PATH);//set the remove file cmd

	if (system(SystemCmd) <0)
	{
		GLOBAL_TRACE(("System cmd '%s' err\n", SystemCmd));
		return FALSE;
	}

	if (EncoderSubBoardChipUpgradeStatus[0] == UPGRADE_RESULT_FAIL && \
		EncoderSubBoardChipUpgradeStatus[1] == UPGRADE_RESULT_FAIL && \
		EncoderSubBoardChipUpgradeStatus[2] == UPGRADE_RESULT_FAIL && \
		EncoderSubBoardChipUpgradeStatus[3] == UPGRADE_RESULT_FAIL)
	{
		GLOBAL_TRACE(("All 4 spi flash chip failed in CRC\n"));
		return FALSE;
	}
	//add by leonli for HTMl show upgrade process
	EncoderUpgradeInfoEntry.s_FilewareFileWrotePercent = 1;//100%
	GLOBAL_TRACE(("Upgrade process success\n"));

	return TRUE; 
}

void spiFlashUpgradeThread(void* param)
{	
	BOOL WriteFirmwareFileResult = FALSE;

	GLOBAL_TRACE(("Encoder Upgrade Start! EncoderUpgradeInfoEntry.s_FirmwareFileNumber = %d.\n", EncoderUpgradeInfoEntry.s_FirmwareFileNumber));

	if(EncoderUpgradeInfoEntry.s_FirmwareFileNumber > 0)  
	{
		//add by leonli for HTMl show upgrade process
		EncoderUpgradeInfoEntry.s_EncoderUpgradeStatus = UPGRADE_STATUS_RUNNING;

		WriteFirmwareFileResult = writeFileToFlash(EncoderUpgradeInfoEntry.s_FirmwareFileIndex, EncoderUpgradeInfoEntry.s_FirmwareFileNumber);

		if(WriteFirmwareFileResult == TRUE)
		{
			GLOBAL_TRACE(("Write Index %d File To Flash Accomplished.\n", EncoderUpgradeInfoEntry.s_FirmwareFileIndex));
			EncoderUpgradeInfoEntry.s_EncoderUpgradeResult = UPGRADE_RESULT_SUCCESS;
		}
		else
		{
			GLOBAL_TRACE(("Write Index %d File To Flash Fail!\n", EncoderUpgradeInfoEntry.s_FirmwareFileIndex));
			EncoderUpgradeInfoEntry.s_EncoderUpgradeResult = UPGRADE_RESULT_FAIL;
		}
		//add by leonli for HTMl show upgrade process
		EncoderUpgradeInfoEntry.s_EncoderUpgradeStatus = UPGRADE_STATUS_FINISH;
		EncoderUpgradeInfoEntry.s_FirmwareFileNumber = 0;
		GLOBAL_TRACE(("test 00 EncoderUpgradeInfoEntry.s_FirmwareFileNumber = %d.\n", EncoderUpgradeInfoEntry.s_FirmwareFileNumber));
		
	}
	GLOBAL_TRACE(("Exit Encoder Upgrade Pthread!\n"));
}
