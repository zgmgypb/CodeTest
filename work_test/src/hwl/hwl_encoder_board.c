/* Includes-------------------------------------------------------------------- */
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "mpeg2_micro.h"
#include "multi_private.h"
#include "multi_drv.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#ifdef GM8358Q
//#define DEBUG_ENCODER_AUTH

#define FPGA_ID_SN_ADD_1		0x308
#define FPGA_ID_SN_ADD_2		0x309
#define FPGA_ID_SN_ADD_3		0x30A
#define FPGA_ID_SN_ADD_4		0x30B

#define FPGA_VERSION_INFO_H		0x300
#define FPGA_VERSION_INFO_L		0x301



#define  DELAY_TIME		500000

#define MULT_HWLL_EncoderAuthTOTAL_TRAIL_TIME				(3600 * 24 * 30)
#define MULT_HWLL_EncoderAuthCOUNT_DOWN_CYCLE				(300)


//FGPA use
//write
#define  WRITE_CMD_ADD_1			0x251
#define  WRITE_CMD_ADD_2			0x252
#define  WRITE_CMD_ADD_3			0x253
#define  WRITE_CMD_ADD_4			0x254

#define  WRITE_CTRL_INFO			0x255
#define  WRITE_M_F_ADD				0x256
#define  WRITE_SUB_F_R_ADD			0x257
#define  WRITE_LENGTH_ADD			0x258

#define  WRITE_DATA_ADD_0			0x259
#define  WRITE_DATA_ADD_1			0x25A
#define  WRITE_DATA_ADD_2			0x25B
#define  WRITE_DATA_ADD_3			0x25C
#define  WRITE_DATA_ADD_4			0x25D
#define  WRITE_DATA_ADD_5			0x25E
#define  WRITE_DATA_ADD_6			0x25F
#define  WRITE_DATA_ADD_7			0x260
#define  WRITE_DATA_ADD_8			0x261
#define  WRITE_DATA_ADD_9			0x262

#define  WRITE_END_ADD				0x263

//read
#define  READ_CMD_ADD_1				0x264
#define  READ_CMD_ADD_2				0x265
#define  READ_CMD_ADD_3				0x266
#define  READ_CMD_ADD_4				0x267

#define  READ_CTRL_INFO				0x268
#define  READ_M_F_ADD				0x269
#define  READ_SUB_F_R_ADD			0x26A
#define  READ_LENGTH_ADD			0x26B

#define  READ_END_ADD				0x26C

#define  READ_CMD_ADD_RE_1			0x26D
#define  READ_CMD_ADD_RE_2			0x26E
#define  READ_CMD_ADD_RE_3			0x26F
#define  READ_CMD_ADD_RE_4			0x270

#define  READ_CTRL_INFO_RE			0x271
#define  READ_M_F_ADD_RE			0x272
#define  READ_SUB_F_R_ADD_RE		0x273
#define  READ_LENGTH_ADD_RE			0x274

#define  READ_DATA_ADD_0			0x275
#define  READ_DATA_ADD_1			0x276
#define  READ_DATA_ADD_2			0x277
#define  READ_DATA_ADD_3			0x278
#define  READ_DATA_ADD_4			0x279
#define  READ_DATA_ADD_5			0x27A
#define  READ_DATA_ADD_6			0x27B
#define  READ_DATA_ADD_7			0x27C
#define  READ_DATA_ADD_8			0x27D
#define  READ_DATA_ADD_9			0x27E
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

U32 HWL_EncoderGetFPGAID(void)
{
	U32 fpga_id[5];
	fpga_id[0] = READ_FPGA(0, FPGA_ID_SN_ADD_1);
	fpga_id[1] = READ_FPGA(0, FPGA_ID_SN_ADD_2);
	fpga_id[2] = READ_FPGA(0, FPGA_ID_SN_ADD_3);
	fpga_id[3] = READ_FPGA(0, FPGA_ID_SN_ADD_4);

	fpga_id[4] = (((fpga_id[3]<<24) & 0xFF000000)  |  ((fpga_id[2]<<16) &0xFF0000) | ((fpga_id[1]<<8) &0xFF00)
		| ((fpga_id[0]) & 0xFF));

	GLOBAL_TRACE(("Main Board ID is 0x%08X !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",(S32)fpga_id[4]));

	return fpga_id[4];
}

/* FPGA版本 ----------------------------------------------------------------------------------------------------------------------------------------*/

static CHAR_T fpga_version[20] ;

CHAR_T *HWL_EncoderGetFPGAVersion(void)
{
	S32 year,month,day;
	year = (READ_FPGA(0, FPGA_VERSION_INFO_H) & 0x7E) >> 1;
	month = (((READ_FPGA(0, FPGA_VERSION_INFO_H) & 0x01) << 3 )& 0x08) | (((READ_FPGA(0, FPGA_VERSION_INFO_L) &0xE0) >> 5) & 0x07);
	day = READ_FPGA(0, FPGA_VERSION_INFO_L) & 0x1F;

	year = year + 9;
	snprintf(fpga_version,10,"%02d.%02d.%02d",year,month,day) ;

	printf("date is %d-%d-%d\n",year,month,day);

	return fpga_version;
}


/* 授权 --------------------------------------------------------------------------------------------------------------------------------------------*/

void HWLL_EncoderAuthWriteCommand(S32 Lblock,S32 Sblock)
{
	WRITE_FPGA(0, WRITE_CMD_ADD_1,0x20);
	WRITE_FPGA(0, WRITE_CMD_ADD_2,0x04);
	WRITE_FPGA(0, WRITE_CMD_ADD_3,0x00);
	WRITE_FPGA(0, WRITE_CMD_ADD_4,0x00);		

	WRITE_FPGA(0, WRITE_CTRL_INFO,0x00);
	switch (Lblock)
	{
		case 0:
			WRITE_FPGA(0, WRITE_M_F_ADD,0x0B);//选择大块
			break;
		case 1:
			WRITE_FPGA(0, WRITE_M_F_ADD,0x0C);
			break;
	}
	switch (Sblock)
	{
		case 0:
			WRITE_FPGA(0, WRITE_SUB_F_R_ADD,0x10);//选择小块
			break;
		case 1:
			WRITE_FPGA(0, WRITE_SUB_F_R_ADD,0x11);
			break;
		case 2:
			WRITE_FPGA(0, WRITE_SUB_F_R_ADD,0x12);
			break;
		case 3:
			WRITE_FPGA(0, WRITE_SUB_F_R_ADD,0x13);
			break;
	}
	WRITE_FPGA(0, WRITE_LENGTH_ADD,0x0a);//长度

}

void HWLL_EncoderAuthWriteData(U8 *pData)
{
	S32 i;

	CAL_PrintDataBlockWithASCII(__FUNCTION__, pData, 10, 4);

	for (i = 0; i < 10;i ++)
	{
		WRITE_FPGA(0, WRITE_DATA_ADD_0+i, pData[i]);
	}
}


void HWLL_EncoderAuthReadCommand(S32 Lblock,S32 Sblock)
{

	WRITE_FPGA(0, READ_CMD_ADD_1,0x20);
	WRITE_FPGA(0, READ_CMD_ADD_2,0x01);
	WRITE_FPGA(0, READ_CMD_ADD_3,0x00);
	WRITE_FPGA(0, READ_CMD_ADD_4,0x00);

	WRITE_FPGA(0, READ_CTRL_INFO,0x00);
	switch (Lblock)
	{
		case 0:
			WRITE_FPGA(0, READ_M_F_ADD,0x0B);//选择大块
			break;
		case 1:
			WRITE_FPGA(0, READ_M_F_ADD,0x0C);
			break;
	}

	switch (Sblock)
	{
		case 0:
			WRITE_FPGA(0, READ_SUB_F_R_ADD,0x00);//选择小块
			break;
		case 1:
			WRITE_FPGA(0, READ_SUB_F_R_ADD,0x01);
			break;
		case 2:
			WRITE_FPGA(0, READ_SUB_F_R_ADD,0x02);
			break;
		case 3:
			WRITE_FPGA(0, READ_SUB_F_R_ADD,0x03);
			break;
	}
	WRITE_FPGA(0, READ_LENGTH_ADD,0x0a);//长度

}

void HWLL_EncoderAuthReadData(S32 Lblock,S32 Sblock,U8 *HWLL_EncoderAuthdata)
{
	S32 i,j;
	U16 C_command[4];
	U16 C_data[4];

	for (i = 0 ;i < 4; i++)
	{
		C_command[i] = READ_FPGA(0, READ_CMD_ADD_RE_1+i);
	}

	C_data[0] = READ_FPGA(0, READ_CTRL_INFO_RE);
	C_data[1] = READ_FPGA(0, READ_M_F_ADD_RE);
	C_data[2] = READ_FPGA(0, READ_SUB_F_R_ADD_RE);
	C_data[3] = READ_FPGA(0, READ_LENGTH_ADD_RE);

	//for (i = 0 ;i < 4; i++)
	//{
	//	printf("C_data[i]  is  0x%x\n",C_data[i]);
	//}

	//printf("Lblock is %x\n",Lblock);
	//printf("Sblock is %x\n",Sblock);

	if( (C_command[0] == 0x20) && (C_command[1] == 0x04) && (C_command[2] == 0x00) && (C_command[3] == 0x00) )
	{
		if ((C_data[0] == 0x00) && (C_data[1] == 0x0b + Lblock) && (C_data[2] == Sblock) && (C_data[3] == 0x0a))
		{
			for (j = 0 ; j < 10 ; j++)
			{
				HWLL_EncoderAuthdata[j] = READ_FPGA(0, READ_DATA_ADD_0 + j);
			}
			//printf("AUTH READ DATA:\n");
			//for (i = 0 ;i<10;i++)
			//{
			//	printf("0x%x ",HWLL_EncoderAuthdata[i]);
			//}
			//printf("\n");
		}
		else
		{
			printf("C_data error!!\n");
		}
	}
	else
	{
		printf("C_command error!\n");
	}
}

//授权用   获得加密卡信息  
//传入参数范围   Lblock 0~1  Sblock 0~4
void HWLL_EncoderAuthGetData(S32 Lblock,S32 Sblock,U8 *pData)
{

	WRITE_FPGA(0, READ_END_ADD,0);
	
	HWLL_EncoderAuthReadCommand(Lblock,Sblock);//发送命令

	WRITE_FPGA(0, READ_END_ADD,1);
	usleep(DELAY_TIME);
	WRITE_FPGA(0, READ_END_ADD,0);

	usleep(DELAY_TIME);
	HWLL_EncoderAuthReadData(Lblock,Sblock,pData);//读取数据
	
}


//授权用   写入加密卡信息
//传入参数范围   Lblock 0~1  Sblock 0~4
void HWLL_EncoderAuthSetData(S32 Lblock,S32 Sblock,U8 *PData)
{
	S32 i,j;
	//S32 data[10];

	WRITE_FPGA(0, WRITE_END_ADD,0);
	
	HWLL_EncoderAuthWriteCommand(Lblock,Sblock);
	HWLL_EncoderAuthWriteData(PData);//传入参数为要写入的值

	WRITE_FPGA(0, WRITE_END_ADD,1);
	usleep(DELAY_TIME);
	WRITE_FPGA(0, WRITE_END_ADD,0);
	usleep(DELAY_TIME);
}


void HWLL_EncoderAuthR_W_Data(S32 Lblock,S32 Sblock,U8 *pData,BOOL bRead)
{
	if (bRead)//读
	{
		HWLL_EncoderAuthGetData(Lblock,Sblock,pData);
	}
	else
	{
		HWLL_EncoderAuthSetData(Lblock,Sblock,pData);
	}
}

void HWL_EncoderAuthFixStorageCB(void* pUserParam, U32 StartAddress, U8 *pData, S32 DataSize, BOOL bRead)
{
	S32 Lblock,Sblock;
	//转换大块小块表示形式
	if ((StartAddress < 4) && (StartAddress >= 0))	// 0/1/2/3
	{
		Lblock = 0;
	}
	else if ((StartAddress > 3) && (StartAddress < 8))		// 4/5/6/7
	{
		Lblock = 0;
	}

	if (StartAddress > 3)
	{
		Sblock = StartAddress - 4;
	}
	else
	{
		Sblock = StartAddress;
	}

	
	HWLL_EncoderAuthR_W_Data(Lblock,Sblock,pData,bRead);
#ifdef DEBUG_ENCODER_AUTH
	GLOBAL_TRACE(("Start Addr = %X, bRead = %d\n", StartAddress, bRead));
	CAL_PrintDataBlock(__FUNCTION__, pData, DataSize);
#endif
}
#endif













/*EOF*/
