//This file defined the functions that used to manipulate the spi_flash

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "spi_flash.h"
#include "multi_drv.h"

static int SpiFlashFd;
static CurrentFlashBank = FLASH_CURRENT_BANK_UNDEFINED;//为避免频繁的改变flash bank而记录当前的bank, 在读和写时都需要查询这个状态

//Choose Spi
int ChooseSpi(int number)
{

	switch(number)
	{
	case 0: // 1st chip
		GLOBAL_TRACE(("Choose SPI 1 ---------------- \n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x3);
		break;
	case 1:  // 2nd chip
		GLOBAL_TRACE(("Choose SPI 2 ---------------- \n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x3);         
		break;
	case 2: // 3rd chip
		GLOBAL_TRACE(("Choose SPI 3 ---------------- \n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x3);
		break;
	case 3: //4th chip
		GLOBAL_TRACE(("Choose SPI 4 ---------------- \n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x1); 
		break;
	case 4:  // enable all 4 chips at the same time
		GLOBAL_TRACE(("Choose SPI all ----------------\n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x1); 
		break;

	default: return -1;
	}

	return 0;
}

//Open flash driver
int SpiFlahInit(void)
{
	SpiFlashFd = open(SPI_FLASH_DEV_NAME, O_RDWR | O_NDELAY | O_NOCTTY);

    GLOBAL_TRACE(("The flash driver fd is: %x\n", SpiFlashFd));

	if( SpiFlashFd < 0)
	{
		GLOBAL_TRACE(("Open spi flash  fail errno = %s\n", strerror(errno)));
		return -1 ;
	}
	else
	{
		return 0;	
	}
}

void CmdExecution(long Command)
{
    ioctl(SpiFlashFd, Command, NULL);
}

int CheckFlashId(void)
{
	U8 Data;
	U32 Id = 0;
    U32 GeneralId=0;

	CmdExecution(CMD_CLR_CS);//Open I/O
	Data = FLASH_CMD_RDID;
	write(SpiFlashFd, &Data, 1);//Send the read dev ID cmd
	read(SpiFlashFd, (U8*)&Id, 3);//Read dev ID into Id, changed parameter 3 into 6 to include unique ID
	CmdExecution(CMD_SET_CS);//close I/O

	//Id = (Id&0xff00) | (Id>>16&0xff) | (Id<<16&0xff0000);//This is for 3 bytes ID information extraction
    GeneralId = Id & 0xffffff;
    GeneralId = (GeneralId&0xff00) | (GeneralId>>16&0xff) | (GeneralId<<16&0xff0000);

    GLOBAL_TRACE(("The dev general ID is: 0x%x;\n", GeneralId));

	if(GeneralId == N25Q256_ID)
	{
		return 0;
	}
	else
	{
		return -1;
	}
	return 0;
}

static U8 ReadFlashStatusReg(void)//Check flash Status register value,meaning refer to datasheet table 9
{
	U8 Data;
	U8 Status = 0;
	Data = FLASH_CMD_RDSR;

	CmdExecution(CMD_CLR_CS);		//Open the I/O 
	write(SpiFlashFd, &Data, 1);	//Write Data into spi_flash
	read(SpiFlashFd, &Status, 1);	//Read 1byte from spi_flash
	CmdExecution(CMD_SET_CS);		//Close the I/O
    
	//GLOBAL_TRACE(("Status:0x%x\n", Status));
	
    return Status;
}

static int WriteEnable(void)
{	
	U8 Data;
    U8 TempStatus;
	CmdExecution( CMD_CLR_CS);
	Data = FLASH_CMD_WREN;

	write(SpiFlashFd, &Data, 1);
	CmdExecution(CMD_SET_CS);

    TempStatus=ReadFlashStatusReg();

	if ((TempStatus & STS_WEL) != STS_WEL)//Changed 0x02 into STS_WEL
	{
		GLOBAL_TRACE(("TempStatus is 0x%x;\n Write enable could not be set!\n", TempStatus));
		return -1;
	}
	return 0;
}

static int WirteDisable(void)
{	
	U8 Data;
	CmdExecution( CMD_CLR_CS);
	Data = FLASH_CMD_WRDIS;
	write(SpiFlashFd, &Data, 1);
	CmdExecution(CMD_SET_CS);
	if ((ReadFlashStatusReg() & 0x02) != 0x00)
	{
		printf("Write disable could not be set!\n");
		return -1;
	}
	return 0;
}

//read/write/erase function
static int StatusReadyPoll(void)//This one is problematic !!!
{
	U8 ReadData  ;	
	int LoopCount = 1;
	ReadData  = (ReadFlashStatusReg() & STS_WIP);// 最低位是flash write Status, 1=busy, 0=free

	while (ReadData && (LoopCount < TIMEOUT)) 
	{
		ReadData = (ReadFlashStatusReg() & STS_WIP);
        LoopCount++;
	}

	if(LoopCount >= TIMEOUT)     
	{
		return -1;
	}
	return 0;
}

//将program的忙闲状态独立开来，间隔时间调整到更小 
static int StatusReadyPollForPageWrite(void)
{
	U8 ReadData  ;	
	int LoopCount = 0;
	ReadData  = (ReadFlashStatusReg() & STS_WIP);//最低位是flash write Status, 1=busy, 0=free

	while (ReadData && (LoopCount<PAGE_PROGRAM_TIMEOUT)) 
    {
		LoopCount++;
		ReadData = (ReadFlashStatusReg() & STS_WIP);
	}

	if(LoopCount >= PAGE_PROGRAM_TIMEOUT)     
	{
		GLOBAL_TRACE(("Waiting for page program time out!\n"));
		return -1;
	}
	return 0;
}

static int SetBank1()
{
	U8 Data;
	WriteEnable();
	Data = FLASH_CMD_WREAR;
	CmdExecution(CMD_CLR_CS);	
	write(SpiFlashFd, &Data, 1);
	Data = BANK_HIGH_16M_BYTES;
	write(SpiFlashFd, &Data, 1);
	CmdExecution(CMD_SET_CS);

	if(StatusReadyPoll() == -1)
	{
		GLOBAL_TRACE(("Set bank 01 failed\n"));
		return -1;
	}
	WirteDisable();
	return 0;
}

static int SetBank0()
{
	U8 Data;
	WriteEnable();
	Data = FLASH_CMD_WREAR;
	CmdExecution( CMD_CLR_CS);	
	write(SpiFlashFd, &Data, 1);
	Data = BANK_LOW_16M_BYTES;
	write(SpiFlashFd, &Data, 1);
	CmdExecution( CMD_SET_CS);

	if(StatusReadyPoll() == -1)
	{
		GLOBAL_TRACE(("Set bank 00 failed\n"));
		return -1;
	}
	WirteDisable();
	return 0;
}


//页编程，page_aligned_address必须是页对齐地址，Length应当<=页大小
static U32 ProgramPage(U32 Address, U8 *pdata, U32 Length)
{	
	U8 Data;
	int i;
	U8 AddressRevers[3];//发送地址时是高地址在前面
	U32 PageBytes = (Length > N25Q256_FLASH_PAGE_SIZE) ? N25Q256_FLASH_PAGE_SIZE : Length;

	if (Address >= ADDRESS_16M)
	{
		SetBank1();
		CurrentFlashBank = FLASH_CURRENT_BANK_1;
	}
	else
	{
		SetBank0();
		CurrentFlashBank = FLASH_CURRENT_BANK_0;
	}
	WriteEnable();

	Data = FLASH_CMD_PP;
	CmdExecution(CMD_CLR_CS);	
	write(SpiFlashFd, &Data, 1);

	AddressRevers[0] = (Address&0xff0000)>>16;
	AddressRevers[1] = (Address&0x00ff00)>>8;
	AddressRevers[2] = Address&0x0000ff;

	write(SpiFlashFd, AddressRevers, 3);
	write(SpiFlashFd, pdata, PageBytes);
	CmdExecution( CMD_SET_CS);
	if (StatusReadyPollForPageWrite() <0)
	{
		return -1;
	}
	else
	{
		return PageBytes;
	}	
}

int FlashWrite(U8 *DataBuf, U32 Address, U32 Length)
{
	U8 Data;
	int ToPageEnd, PageCount, WritedBytes;

	if(Address + Length > N25Q256_FLASH_SIZE)
	{
		GLOBAL_TRACE(("Flash write einvalid_lenth.\n"));
		return -1;
	}

	//先将与页不对齐地址的数据写入FLASH
    ToPageEnd = N25Q256_FLASH_PAGE_SIZE - Address % N25Q256_FLASH_PAGE_SIZE;

    if((ToPageEnd > 0)&&(ToPageEnd != N25Q256_FLASH_PAGE_SIZE))
    {		
        if(Length >= ToPageEnd)
        {
            WritedBytes = ProgramPage(Address, DataBuf, ToPageEnd);
        }
        else
        {
             WritedBytes = ProgramPage(Address, DataBuf, Length);
        }

	    if (WritedBytes < 0)
	    {
		    GLOBAL_TRACE(("Program page error!\n"));
		    return -1;
	    }
	    Address += WritedBytes;

        if (Length>=WritedBytes)
        {
            Length -= WritedBytes;
        }
	    else
        {
            GLOBAL_TRACE(("Program page error!\n"));
            return -1;
        }
		
        DataBuf += WritedBytes;
    }
     
	PageCount = Length/N25Q256_FLASH_PAGE_SIZE;

    if(Length % N25Q256_FLASH_PAGE_SIZE)
	{
		PageCount += 1;  
	}

	while(PageCount)//Write the page aligned Data
	{	
		WritedBytes = ProgramPage(Address, DataBuf, Length);

		if (WritedBytes < 0)
		{
			GLOBAL_TRACE(("Program page error!\n"));
			return -1;
		}

        if (Length >= WritedBytes)
        {
            Length -= WritedBytes;
        }
        else
        {
            GLOBAL_TRACE(("Program page error!\n"));
            return -1;
        }	

		Address += WritedBytes;
		DataBuf += WritedBytes;
		PageCount = PageCount - 1;
	}
	return 0;
}

int FlashRead(U8 *DataBuf, U32 Address, U32 Length)
{
	U8 Data;
	U8 AddressRevers[3];
	U32 Id = 0;
	U32 ReadBytes;
    int ReadNumber = 0, NumberToReturn = 0;

	if(Address + Length > N25Q256_FLASH_SIZE)
	{
		GLOBAL_TRACE(("Flash read: invalid_length.\n"));
		return -1;
	}

	if (Address >= ADDRESS_16M)
	{
		SetBank1();
		CurrentFlashBank = FLASH_CURRENT_BANK_1;
	}
	else
	{
		SetBank0();
		CurrentFlashBank = FLASH_CURRENT_BANK_0;        //20140625
	}

	CmdExecution(CMD_CLR_CS);	
	Data = FLASH_CMD_RD;
	write(SpiFlashFd, &Data, 1);
	AddressRevers[0] = (Address&0xff0000)>>16;
	AddressRevers[1] = (Address&0x00ff00)>>8;
	AddressRevers[2] = Address&0x0000ff;
	write(SpiFlashFd, AddressRevers, 3);

	while(Length)
	{
		ReadBytes = (Length > APP_READ_FLASH_BUF_SIZE)?APP_READ_FLASH_BUF_SIZE:Length;
		//GLOBAL_TRACE(("Flash read %d bytes\n", ReadBytes));
		ReadNumber=read(SpiFlashFd, DataBuf, ReadBytes);
		Length -= ReadBytes;
		DataBuf += ReadBytes;
        NumberToReturn+=ReadNumber;
	}	
	CmdExecution(CMD_SET_CS);	

	return NumberToReturn;
}

int ChipErase(void)
{
	U8 Data;

    GLOBAL_TRACE(("Start the flash erasing process\n"));

	WriteEnable();
	CmdExecution( CMD_CLR_CS);	
	Data = FLASH_CMD_CE;
	write(SpiFlashFd, &Data, 1);
	CmdExecution(CMD_SET_CS);

	if (StatusReadyPoll()<0)
	{
		GLOBAL_TRACE(("Wait eeprom timeout error\n"));
		return -1;
	}
	return 0;
}

#ifdef DEBUG_SPI_FLASH_INTERFACE//测试SPI接口
#include "fpga_spi_flash.h"
S32 TESTL_WriteAndReadFN(void *pUserParam, S32 SlotInd, U8 *pWData, S32 WDataSize, U8 *pRBuf, S32 RBufSize, S32 DummyClock)
{
	CmdExecution(CMD_CLR_CS);
	if (write(SpiFlashFd, pWData, WDataSize) <= 0)
	{
		GLOBAL_TRACE(("Write Error\n"));
		RBufSize = -1;
	}
	else
	{
		//CAL_PrintDataBlock("SPI Write Data", pWData, WDataSize);

		if (RBufSize > 0)
		{
			ioctl(SpiFlashFd, CMD_SET_DUMMY_CYCLE, DummyClock);

			if (read(SpiFlashFd, pRBuf, RBufSize) > 0)
			{
				//CAL_PrintDataBlock("SPI Read Data", pRBuf, RBufSize);
			}
			else
			{
				GLOBAL_TRACE(("Read Error!\n"));
				RBufSize = -1;
			}
		}
	}

	CmdExecution(CMD_SET_CS);
	return RBufSize;
}

void Test_FSPIF(void)
{
	S32 lSlot;
	U8 *plBuf;
	S32 lActSize;
	U32 lStartAddr;
	FSPIF_Param lParam;
	FSPIF_FlashStructure lFlash;
	FSPIF_Identfication lID;
	FSPIF_StatusReg lStatus;
	FSPIF_FlagStatusReg lFStatus;

	GLOBAL_ZEROSTRUCT(lParam);


	SpiFlahInit();

	lParam.m_MaxOnceReadSize = 1000;
	lParam.m_MaxOnceWriteSize = 1000;
	lParam.m_pSPICB = TESTL_WriteAndReadFN;
	lParam.m_pUserParam = NULL;

	lParam.m_bUseFastRead = TRUE;
	lParam.m_FastReadDummyClock = 8;


	lFlash.m_PageSize = 256;
	lFlash.m_SectorSize = 64 * 1024;
	lFlash.m_TotalBytes = 32 * 1024 *1024;
	lFlash.m_EraseOperationTimeout = 1000;
	lFlash.m_ProgramOperationTimeout = 100;


	MFPGA_SetEncBoardPowerUp();

	GLOBAL_TRACE(("Ready!!\n"));

	plBuf = (U8*)GLOBAL_MALLOC(lFlash.m_SectorSize);

	ChooseSpi(0);

	PFC_TaskSleep(1000);

	GLOBAL_TRACE(("Flash Info: Total = %d, Sector = %d, Page = %d\n", lFlash.m_TotalBytes, lFlash.m_SectorSize, lFlash.m_PageSize));

	while(TRUE)
	{

		GLOBAL_PRINTF(("\n READ ID ---------------------------------------------------------------------------------------------------\n"));
		{
			if (FSPIF_ReadIdentification(&lParam, 0, &lID))
			{
				GLOBAL_TRACE(("Manufacturer = %02X, Type = %02X, Capacity = %02X\n", lID.m_Manufacturer_ID, lID.m_DeviceType, lID.m_DeviceCapacity))
			}
			else
			{
				GLOBAL_TRACE(("Read ID Register Failed!!\n"));
			}

			PFC_TaskSleep(1000);
		}


		//GLOBAL_PRINTF(("\n Total Test -----------------------------------------------------------------------------------------------\n"));
		//{
		//	GLOBAL_FD lFD;
		//	lFD = GLOBAL_FOPEN("/tmp/1.raw", "rb");
		//	if (lFD)
		//	{
		//		S32 lActRead, lTotal, lFSize;
		//		lStartAddr = 0;

		//		lFSize = CAL_FileSize(lFD);
		//		lTotal = 0;
		//		while(TRUE)
		//		{
		//			lActRead = GLOBAL_FREAD(plBuf, 1, lFlash.m_SectorSize, lFD);
		//			if (lActRead > 0)
		//			{
		//				lTotal += lActRead;
		//				lStartAddr += lFlash.m_SectorSize;
		//				FSPIF_FLASHProgram(&lParam, &lFlash, 0, lStartAddr, plBuf, lFlash.m_SectorSize, FALSE, NULL, NULL);
		//				GLOBAL_TRACE(("Data Read = %d, Total = %d, PCT = %3.1f%%\n", lActRead, lTotal, (F64)lTotal / lFSize * 100));
		//			}
		//			else
		//			{
		//				break;
		//			}
		//		}
		//		GLOBAL_FCLOSE(lFD);
		//	}
		//	
		//	PFC_TaskSleep(1000);
		//}

		GLOBAL_PRINTF(("\n READ OPERATION 2 -------------------------------------------------------------------------------------------\n"));
		{

			S32 lActRead, lTotal, lFSize;

			{
				GLOBAL_FD lFD;
				lFD = GLOBAL_FOPEN("/tmp/temp.bin", "wb+");
				if (lFD)
				{
					lStartAddr = 0;
					lTotal = 0;

					lFSize = 4 * 1024 * 1024;
					while(lTotal < lFSize)
					{
						lActRead = lFlash.m_SectorSize;
						if (FSPIF_Read(&lParam, 0, lStartAddr, plBuf, lActRead))
						{
							GLOBAL_FWRITE(plBuf, 1, lActRead, lFD);
							lTotal += lActRead;
							lStartAddr += lActRead;
							GLOBAL_TRACE(("Data Read = %d, Total = %d, PCT = %3.1f%%\n", lActRead, lTotal, (F64)lTotal / lFSize * 100));
						}
						else
						{
							GLOBAL_TRACE(("Read Flash Data Failed!!\n"));
							break;
						}

					}
					GLOBAL_FCLOSE(lFD);
				}

			}

			PFC_TaskSleep(1000);

		}

		//GLOBAL_PRINTF(("\n FLAGE STATUS ----------------------------------------------------------------------------------------------\n"));
		//{
		//	if (FSPIF_ReadFlagStatusRegister(&lParam, 0, &lFStatus))
		//	{
		//		GLOBAL_TRACE(("F Status Busy = %d, Erase Error = %d\n", lFStatus.m_ProgramEraseInProgress, lFStatus.m_EraseError));

		//		if (lFStatus.m_EraseError || lFStatus.m_ProgramError || lFStatus.m_ProtectionError || lFStatus.m_VPPError)
		//		{
		//			if (FSPIF_ClearFlagStatusRegisterEnable(&lParam, 0))
		//			{
		//				GLOBAL_TRACE(("F Flage Cleared!\n"));
		//				if (FSPIF_ReadFlagStatusRegister(&lParam, 0, &lFStatus))
		//				{
		//					GLOBAL_TRACE(("F Status Busy = %d, Erase Error = %d\n", lFStatus.m_ProgramEraseInProgress, lFStatus.m_EraseError));
		//				}
		//				else
		//				{
		//					GLOBAL_TRACE(("Read F Status Register Failed!!\n"));
		//				}
		//			}
		//			else
		//			{
		//				GLOBAL_TRACE(("Clear F Status Register Failed!!\n"));
		//			}
		//		}
		//	}
		//	else
		//	{
		//		GLOBAL_TRACE(("Read F Status Register Failed!!\n"));
		//	}
		//	PFC_TaskSleep(1000);
		//}

		//GLOBAL_PRINTF(("\n SECTOR ERASE OPERATION ------------------------------------------------------------------------------------\n"));
		//{
		//	U32 lTimeTick;


		//	lStartAddr = 0;
		//	PAL_TimeStart(&lTimeTick);
		//	while(lStartAddr < lFlash.m_SectorSize)
		//	{
		//		FSPIF_SetWriteEnable(&lParam, 0, TRUE);

		//		if (FSPIF_SectorErase(&lParam, 0, lStartAddr))
		//		{
		//			while(TRUE)
		//			{
		//				if (FSPIF_ReadStatusRegister(&lParam, 0, &lStatus))
		//				{
		//					//GLOBAL_TRACE(("F Status Busy = %d, Erase Error = %d\n", lFStatus.m_ProgramEraseInProgress, lFStatus.m_EraseError));
		//					if (lStatus.m_WriteInProgress == 0)
		//					{
		//						break;
		//					}
		//				}
		//				else
		//				{
		//					GLOBAL_TRACE(("Read F Status Register Failed!!\n"));
		//					break;
		//				}
		//				PFC_TaskSleep(100);
		//			}


		//			///*检测一下是不是都是0xFF*/
		//			//GLOBAL_ZEROMEM(plBuf, lFlash.m_SectorSize);
		//			//if (FSPIF_Read(&lParam, 0, lStartAddr, plBuf, lFlash.m_SectorSize))
		//			//{
		//			//	S32 k;
		//			//	for (k = 0; k < lFlash.m_SectorSize; k++)
		//			//	{
		//			//		if (plBuf[k] != 0xFF)
		//			//		{
		//			//			GLOBAL_TRACE(("Address = %08X, Erase Failed!!!!!!!!!! index = %d\n", lStartAddr, k));
		//			//			break;
		//			//		}
		//			//	}

		//			//}
		//			//else
		//			//{
		//			//	GLOBAL_TRACE(("Read Flash Data Failed!!\n"));
		//			//}

		//		}
		//		else
		//		{
		//			GLOBAL_TRACE(("Erase Flash Failed!!\n"));
		//			break;
		//		}

		//		lStartAddr += lFlash.m_SectorSize;

		//		GLOBAL_TRACE((" Erase Block Addr = 0x%08X, %3.2f%%\n", lStartAddr, ((F64)lStartAddr / lFlash.m_TotalBytes) * 100));
		//	}
		//	GLOBAL_TRACE(("Erase Time = %d ms\n", PAL_TimeEnd(&lTimeTick)));
		//	PFC_TaskSleep(1000);
		//}

		//GLOBAL_PRINTF(("\n PAGE PROGRAM OPERATION -------------------------------------------------------------------------------------\n"));
		//{
		//	GLOBAL_FD lFD;
		//	lFD = GLOBAL_FOPEN("/tmp/sector_sample.bin", "rb");
		//	if (lFD)
		//	{
		//		GLOBAL_FREAD(plBuf, lFlash.m_SectorSize, 1, lFD);
		//		GLOBAL_FCLOSE(lFD);
		//	}

		//	CAL_PrintDataBlockWithASCII("FILE DATA", plBuf, 256, 16);


		//	FSPIF_SetWriteEnable(&lParam, 0, TRUE);
		//	lStartAddr = 0;

		//	if (FSPIF_PageProgram(&lParam, 0, lStartAddr, plBuf, lFlash.m_SectorSize))
		//	{
		//		while(TRUE)
		//		{
		//			if (FSPIF_ReadStatusRegister(&lParam, 0, &lStatus))
		//			{
		//				if (lStatus.m_WriteInProgress == 0)
		//				{
		//					break;
		//				}
		//				else
		//				{
		//					GLOBAL_TRACE(("F Status Busy = %d, Erase Error = %d\n", lFStatus.m_ProgramEraseInProgress, lFStatus.m_EraseError));
		//				}
		//			}
		//			else
		//			{
		//				GLOBAL_TRACE(("Read F Status Register Failed!!\n"));
		//				break;
		//			}
		//		}
		//		GLOBAL_TRACE(("Program Flash Data Done!!\n"));
		//	}
		//	else
		//	{
		//		GLOBAL_TRACE(("Program Flash Data Failed!!\n"));
		//	}

		//	PFC_TaskSleep(1000);
		//}

		//GLOBAL_PRINTF(("\n READ OPERATION 2 -------------------------------------------------------------------------------------------\n"));
		//{
		//	GLOBAL_ZEROMEM(plBuf, lFlash.m_SectorSize);

		//	lActSize = lFlash.m_SectorSize;
		//	lStartAddr = 0;
		//	if (FSPIF_Read(&lParam, 0, lStartAddr, plBuf, lActSize))
		//	{
		//		CAL_PrintDataBlockWithASCII("FLASH DATA", plBuf, 256, 16);
		//		{
		//			GLOBAL_FD lFD;
		//			lFD = GLOBAL_FOPEN("/tmp/temp.bin", "wb+");
		//			if (lFD)
		//			{
		//				GLOBAL_FWRITE(plBuf, lActSize, 1, lFD);
		//				GLOBAL_FCLOSE(lFD);
		//			}
		//		}
		//	}
		//	else
		//	{
		//		GLOBAL_TRACE(("Read Flash Data Failed!!\n"));
		//	}

		//	PFC_TaskSleep(1000);

		//}


		GLOBAL_TRACE(("All Operation Done!!!!!!!!!!!\n"));

		lSlot++;
		PFC_TaskSleep(10000000);
	}

	GLOBAL_FREE(plBuf);
}


#endif




