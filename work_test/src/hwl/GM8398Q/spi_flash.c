//This file defined the functions that used to manipulate the spi_flash

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../../drivers/fpga_ps_GM8398Q/fpga_ps_driver.h"
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
		GLOBAL_TRACE(("Choose SPI 1\n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x3);
		break;
	case 1:  // 2nd chip
		GLOBAL_TRACE(("Choose SPI 2\n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x3);         
		break;
	case 2: // 3rd chip
		GLOBAL_TRACE(("Choose SPI 3\n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x1);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x3);
		break;
	case 3: //4th chip
		GLOBAL_TRACE(("Choose SPI 4\n"));
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_1,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_2,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_3,0x3);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,SPI_chip_4,0x1); 
		break;
	case 4:  // enable all 4 chips at the same time
		GLOBAL_TRACE(("Choose SPI all\n"));
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
