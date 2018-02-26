#include "linuxplatform.h"
#include "multi_main_internal.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "multi_drv.h"

#ifndef USE_NEW_QAM_SETTING_FUNCTIONS

#ifdef MULT_VCO_REF_CLK_USE_25M //VCO变成4350，导致FDAC必须变化
#define HWL_AD9789_FDAC0				0x80266580/*2150000000*/  
#define HWL_AD9789_FDAC1				0x7BADFCC0/*2075000000*/
#else
#define HWL_AD9789_FDAC0				0x80BEFC00/*2160000000*/
#define HWL_AD9789_FDAC1				0x7CEE6C00/*2096000000*/
#endif

#define HWL_AD9789_FDAC2				0x8F0D1800/*2400000000*/
#define HWL_AD9789_FDAC3_FOR_3760		0x81C6A800/*2177280000*/
#define HWL_AD9789_FDAC4				0x83215600/*2200000000*/


#if defined(MULT_VCO_REF_CLK_USE_25M)

//2.200
#define AD_4350_REG_DATA_5_4		0x00400005
#define AD_4350_REG_DATA_4_4		0x009C85FC
#define AD_4350_REG_DATA_3_4		0x00010643
#define AD_4350_REG_DATA_2_4		0x14007FC2
#define AD_4350_REG_DATA_1_4		0x08008011
#define AD_4350_REG_DATA_0_4		0x00580000


//2.400
#define AD_4350_REG_DATA_5_2		0x00400005
#define AD_4350_REG_DATA_4_2		0x008C85FC
#define AD_4350_REG_DATA_3_2		0x00010643
#define AD_4350_REG_DATA_2_2		0x14007FC2
#define AD_4350_REG_DATA_1_2		0x08008011
#define AD_4350_REG_DATA_0_2		0x00300000


//2.075
#define AD_4350_REG_DATA_5_1		0x00400005
#define AD_4350_REG_DATA_4_1		0x009C85FC
#define AD_4350_REG_DATA_3_1		0x00010643
#define AD_4350_REG_DATA_2_1		0x14007FC2
#define AD_4350_REG_DATA_1_1		0x08008011
#define AD_4350_REG_DATA_0_1		0x00530000

//2.150
#define AD_4350_REG_DATA_5_0		0x00400005
#define AD_4350_REG_DATA_4_0		0x009C85FC
#define AD_4350_REG_DATA_3_0		0x00010643
#define AD_4350_REG_DATA_2_0		0x14007FC2
#define AD_4350_REG_DATA_1_0		0x08008011
#define AD_4350_REG_DATA_0_0		0x00560000

#else//晶体采用54MHz

//2.200 有小数
#define AD_4350_REG_DATA_5_4		0x00580005
#define AD_4350_REG_DATA_4_4		0x009FF03C
#define AD_4350_REG_DATA_3_4		0x000004B3
#define AD_4350_REG_DATA_2_4		0x14005EC2
#define AD_4350_REG_DATA_1_4		0x080080D9
#define AD_4350_REG_DATA_0_4		0x00288068


//2.40
#define AD_4350_REG_DATA_5_2		0x00580005
#define AD_4350_REG_DATA_4_2		0x008FF03C
#define AD_4350_REG_DATA_3_2		0x000004B3
#define AD_4350_REG_DATA_2_2		0x14005EC2
#define AD_4350_REG_DATA_1_2		0x0800FFB9
#define AD_4350_REG_DATA_0_2		0x00258000  


//2.096 有小数
#define AD_4350_REG_DATA_5_1		0x00580005
#define AD_4350_REG_DATA_4_1		0x009FF03C
#define AD_4350_REG_DATA_3_1		0x000004B3
#define AD_4350_REG_DATA_2_1		0x14005EC2
#define AD_4350_REG_DATA_1_1		0x080080D9	
#define AD_4350_REG_DATA_0_1		0x00268088  

//2.160
#ifdef MULT_VCO_REF_CLK_USE_10M
	#define AD_4350_REG_DATA_5_0		0x00580005
	#define AD_4350_REG_DATA_4_0		0x009A003C
	#define AD_4350_REG_DATA_3_0		0x000004B3
	#define AD_4350_REG_DATA_2_0		0x16005EC2
	#define AD_4350_REG_DATA_1_0		0x08008321  
	#define AD_4350_REG_DATA_0_0		0x006C0000
#else
	#define AD_4350_REG_DATA_5_0		0x00400005
	#define AD_4350_REG_DATA_4_0		0x009D85FC
	#define AD_4350_REG_DATA_3_0		0x000106C3
	#define AD_4350_REG_DATA_2_0		0x1400BFC2
	#define AD_4350_REG_DATA_1_0		0x08008011  
	#define AD_4350_REG_DATA_0_0		0x00500000 
#endif

#endif

//3760
#define AD_4350_REG_DATA_5_3		0x00580005
#define AD_4350_REG_DATA_4_3		0x009F203C
#define AD_4350_REG_DATA_3_3		0x000004B3
#define AD_4350_REG_DATA_2_3		0x14005EC2
#define AD_4350_REG_DATA_1_3		0x08008011
#define AD_4350_REG_DATA_0_3		0x00480000






#define HWL_AD9789_DTMB_DEFAULT_SYMBO_RATE	7560 //KBoud



#define HWL_AD9789_4350_MAX_ADJ_LEVEL	72

/**
*分配和释放与FPGA通信的缓冲区内存.
*/
#define HWL_CONST_REGISTER_MAX 		250

static HWL_ModularChipRegister_t registerTable[HWL_CONST_REGISTER_MAX];
static U32 registerTableSize = 0;

void HWLL_QamCalculatePQN(U32 CurrentFDAC, U32 SymRate, U32 *pP, U32 *pQ, U32 *pN);


static void ICPL_AD9789BroadOperationAdd( BOOL bWrite, U8 ICID, U8 RegAddress, U8 RegData)
{
	if(registerTableSize >= HWL_CONST_REGISTER_MAX)
	{
		return;
	}

	registerTable[registerTableSize].address = RegAddress;
	registerTable[registerTableSize].chipID = ICID;
	registerTable[registerTableSize].rOrw = !bWrite;
	registerTable[registerTableSize].value = RegData;
	registerTableSize++;
	return;
}

static void ICPL_AD9789BroadOperationApply()
{
	//发动到FPGA
	HWL_ModularChipTableSend(registerTable, registerTableSize);
	registerTableSize = 0;
}



typedef enum tagICPL_ErrorType
{
	ICPL_SUCCESS,
	ICPL_NO_MEMORY,
	ICPL_HARDWARE_ERROR,
	ICPL_PARAMETER_ERROR,
	ICPL_FAILURE
} ICPL_ErrorType;


typedef struct
{
	U8						ModuleType;								/*调制模块类型：0，None；1，AD9789；2，BroadLogic*/
	HWL_ModulatorParam_t	pParam[HWL_CONST_MAX_QAM_CHN_NUM];
	U32						CurrentFDAC;
	S32						FrequeceCalibrate;						/*100MHz 时的频率偏移 ±1Hz*/
	BOOL					bUpperFreq;
	BOOL					bLevelOnly;
	U8						p9789GainLevelValue[HWL_AD9789_4350_MAX_ADJ_LEVEL];

} HWL_Modulator;






ICPL_ErrorType ICPL_BL85KMMClockOperation(S32 Standard, S32 Mode, S32 SymbolRate)
{
	return 0;
}
ICPL_ErrorType ICPL_BufferDeviceOperationAddList( U8 DeviceType, BOOL bWrite, U8 *pData, S16 DataLen)
{
	return 0;
}
ICPL_ErrorType ICPL_BufferDeviceOperationApplyList(U8 *pData, S16 DataLen, S16 *pReadDataLen)
{
	return 0;
}


/******************************************************************************
*                        Local macro and type definitions
******************************************************************************/
#define USE_ADL5331			1	/*反之使用AD5330*/
#define USE_50K				0	/*反之使用5K*/

#define AD533X_MAX_LEVEL	41
/******************************************************************************
*                        Local function prototype definitions
******************************************************************************/
/******************************************************************************
*                                Local variables
******************************************************************************/
static HWL_Modulator stModulator;

static U8 stQAMRFLevelTabel[AD533X_MAX_LEVEL] =
{
#if USE_50K

#if USE_ADL5331
	240, 236, 232, 230, 228, 227, 225, 224, 223, 221, 220, 219, 217, 215, 214, 212, 210, 208, 206, 205, 202, 200, 198, 196, 193, 190, 188, 186, 183, 181, 179, 175, 173, 167, 163, 162, 159, 154, 148, 143, 138
#else
	233, 230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 190, 189
#endif

#else
#if USE_ADL5331
	225, 198, 189, 179, 174, 171, 168, 162, 157, 154, 151, 145, 140, 137, 134, 128, 123, 120, 117, 112, 106, 103, 100, 95, 89, 85, 82, 77, 70, 66, 62, 57, 51, 46, 42, 37, 30, 26, 23, 18, 11
#else
	205, 188, 183, 180, 177, 173, 170, 168, 166, 164, 162, 161, 158, 156, 154, 152, 150, 149, 147, 145, 142, 140, 138, 136, 134, 131, 129, 126, 124, 122, 119, 117, 115, 113, 110, 108, 106, 104, 102, 99, 97
#endif

#endif
};

/*AD4360寄存器设置*/
void  HWLL_ApplyADF4360(U32 FDAC)
{
	S16 lModuleIndex;
	U8 lICID = ICPL_IIC_IC_ID_ADF4360;

	GLOBAL_TRACE(("FDAC = %.2f\n", (F64)(FDAC/ 1000)  / 1000));


	if (FDAC == HWL_AD9789_FDAC0)
	{
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH, 0xAc);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH + 1, 0xF9);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH + 2, 0x4F);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH, 0x15);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH + 1, 0x00);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH + 2, 0x30);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH, 0x3A);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH + 1, 0x10);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH + 2, 0x00);
	}
	else if (FDAC == HWL_AD9789_FDAC1)
	{
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH, 0xAc);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH + 1, 0xF9);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH + 2, 0x4F);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH, 0x15);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH + 1, 0x00);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH + 2, 0x30);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH, 0x1A);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH + 1, 0x10);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH + 2, 0x00);
	}
	else if (FDAC == HWL_AD9789_FDAC2)
	{
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH, 0xAc);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH + 1, 0xF9);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_COUNTERLATCH + 2, 0x4F);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH, 0x15);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH + 1, 0x00);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_R_COUNTERLATCH + 2, 0x30);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH, 0x32);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH + 1, 0x12);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_ADF4360_N_COUNTERLATCH + 2, 0x00);
	}
	else if (FDAC == HWL_AD9789_FDAC3_FOR_3760)
	{
		GLOBAL_TRACE(("NO Register Values!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
	}

	ICPL_AD9789BroadOperationApply();
}


void HWLL_ApplyADF4350(U32 FDAC)
{
	U8 lICID = ICPL_IIC_IC_ID_ADF4350;
	GLOBAL_TRACE(("Config 4350\n"));


	GLOBAL_TRACE(("FDAC = %.u Hz\n", FDAC));

	if (FDAC == HWL_AD9789_FDAC0)
	{

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_0 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_0 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_0 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_0 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_0 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_0 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_0 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_0 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_0 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_0 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_0 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_0 >> 24) & 0xFF);


		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_0 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_0 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_0 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_0 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_0 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_0 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_0 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_0 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_0 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_0>> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_0 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_0 >> 24) & 0xFF);
	}
	else if (FDAC == HWL_AD9789_FDAC1)
	{
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_1 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_1 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_1 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_1 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_1 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_1 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_1 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_1 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_1 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_1 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_1 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_1 >> 24) & 0xFF);


		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_1 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_1 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_1 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_1 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_1 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_1 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_1 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_1 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_1 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_1 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_1 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_1 >> 24) & 0xFF);
	}
	else if (FDAC == HWL_AD9789_FDAC2)
	{
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_2 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_2 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_2 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2 >> 24) & 0xFF);


		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_2 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_2 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_2 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2 >> 24) & 0xFF);
	}
	else if (FDAC == HWL_AD9789_FDAC3_FOR_3760)
	{
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_3 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_3 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_3 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_3 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_3 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_3 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_3 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_3 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_3 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_3 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_3 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_3 >> 24) & 0xFF);


		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_3 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_3 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_3 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_3 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_3 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_3 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_3 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_3 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_3 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_3 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_3 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_3 >> 24) & 0xFF);
	}
	else if (FDAC == HWL_AD9789_FDAC4)
	{
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_5_4 >> 0) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_4 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_4 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_4 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_4_4 >> 0) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_4 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_4 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_4 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_3_4 >> 0) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_4 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_4 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_4 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_2_4 >> 0) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_4 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_4 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_4 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_1_4 >> 0) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_4 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_4 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_4 >> 24) & 0xFF);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_0_4 >> 0) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_4 >> 8) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_4 >> 16) & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_4 >> 24) & 0xFF);
	}


	ICPL_AD9789BroadOperationApply();
	PFC_TaskSleep(200);
}

/*DS1775寄存器设置*/
void  HWLL_SetParameterDS1775()
{
	U8 lICID;
	/*DS1775 （温度）初始设置 65-75*/
	lICID = ICPL_IIC_IC_ID_DS1775;
	ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_DS1775_REG_TEMP_HIGHT_H, 0x5A);
	ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_DS1775_REG_TEMP_HIGHT_L, 0x00);

	ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_DS1775_REG_TEMP_LOW_H, 0x41);
	ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_DS1775_REG_TEMP_LOW_L, 0x00);

	ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_DS1775_REG_CONF_H, 0x00);
	ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_DS1775_REG_CONF_L, 0x0A);

	ICPL_AD9789BroadOperationApply();

	PFC_TaskSleep(200);
}

/*AD9789固定寄存器设置*/
void  HWLL_RestAndInitiateAD9789(void)
{
	S16 lModuleIndex;
	U8 lICID;
	HWL_ModulatorParam_t *plSubModulatorParam;
	for (lModuleIndex = 0; lModuleIndex < HWL_AD9789_MODULE_NUM; lModuleIndex++)
	{
		lICID = ICPL_IIC_IC_ID_AD9789_START +  lModuleIndex;

		plSubModulatorParam =  &stModulator.pParam[lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];//每个AD9789的第一个通道的参数

		/*设置AD9789内部寄存器*/
		GLOBAL_TRACE(("Config AD9789 FIX REG\n"));
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, 0x3C);//复位
		ICPL_AD9789BroadOperationApply();

		PFC_TaskSleep(1);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x00, 0x18);//复位结束
		ICPL_AD9789BroadOperationApply();

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x30, 0x80);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x31, 0xf0);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x32, 0x9e);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x3e, 0x38);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x2f, 0x4d);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x33, 0x42);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x39, 0x4e);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x3a, 0x64);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, 0x00);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x04, 0xfe);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x03, 0xfe);
		ICPL_AD9789BroadOperationApply();

		PFC_TaskSleep(10);

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x33, 0x43);
		ICPL_AD9789BroadOperationApply();
	}

	PFC_TaskSleep(200);

}



/*内部参数初始化*/
void HWLL_QamSetParameterDefault(S16 TsIndex)
{
	HWL_ModulatorParam_t *plParam;

	if ((TsIndex >= 0) && (TsIndex < HWL_CONST_MAX_QAM_CHN_NUM))
	{
		//printf("[HWL_SetParamDefault] Set Ts %d Default \n", TsIndex);

		plParam = &stModulator.pParam[TsIndex];
		if (stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QAM)
		{
			plParam->ModulateStandard = HWL_CONST_MODULATOR_STANDARD_ANNEX_A;

			plParam->ModulateFreq = 474000 + (TsIndex * 8000);

			plParam->QamMode = HWL_CONST_MODULATOR_QAM_64;
			plParam->SymRate = 6875;
			plParam->AnalogBand = HWL_CONST_MODULATOR_ANALOG_BAND_8M;
			plParam->Gain = 10;
			plParam->Attenuator = 10;
			plParam->SpectrumInvert = 0;
			plParam->QamSwitch = 1;
			plParam->RFSwitch = 1;

			GLOBAL_TRACE(("QAM Default Parameter!\n"));
		}
		else if (stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_H || stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_L)
		{
			plParam->ModulateStandard = HWL_CONST_MODULATOR_STANDARD_ANNEX_A;

			plParam->ModulateFreq = 950000;

			plParam->QamMode = HWL_CONST_MODULATOR_QAM_QPSK;
			plParam->SymRate = 27500;
			plParam->AnalogBand = HWL_CONST_MODULATOR_ANALOG_BAND_8M;
			plParam->Gain = 10;
			plParam->Attenuator = 10;
			plParam->SpectrumInvert = 0;
			plParam->QamSwitch = 1;
			plParam->RFSwitch = 1;
			plParam->QPSK_FEC = HWL_CONST_MODULATOR_FEC_7_OF_8;

			GLOBAL_TRACE(("QPSK Default Parameter!\n"));
		}
		else if (stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_DTMB)
		{
			plParam->ModulateStandard = 0;

			plParam->ModulateFreq = 474000;

			plParam->QamMode = HWL_CONST_MODULATOR_QAM_64;
			plParam->SymRate = HWL_AD9789_DTMB_DEFAULT_SYMBO_RATE;//设置给9789的DTMB符号率
			plParam->AnalogBand = HWL_CONST_MODULATOR_ANALOG_BAND_8M;
			plParam->Gain = 10;
			plParam->Attenuator = 10;
			plParam->SpectrumInvert = 0;
			plParam->QamSwitch = 1;
			plParam->RFSwitch = 1;

			plParam->DTMB_Carrier = HWL_MODULATOR_DTMB_CARRIER_MULTI;
			plParam->DTMB_CodeRate = HWL_MODULATOR_DTMB_CodeRate_06;
			plParam->DTMB_PN = HWL_MODULATOR_DTMB_PN420;
			plParam->DTMB_TI = HWL_MODULATOR_DTMB_TI_720;

			GLOBAL_TRACE(("DTMB Default Parameter!\n"));
		}

	}
	else
	{
		if (stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QAM)
		{
			S32 i;
			for (i = 0; i < HWL_CONST_MAX_QAM_CHN_NUM; i++)
			{
				HWLL_QamSetParameterDefault(i);
			}
		}
		else
		{
			HWLL_QamSetParameterDefault(0);
		}

	}
}

/*增益参数计算*/


#define HWL_AD9789_MAX_ADJ_LEVEL	52

static U8	s_plValue[HWL_AD9789_MAX_ADJ_LEVEL];



U32 HWLL_GetAD9789GainValue(S16 Level, S16 QAMMode)
{
	U8 lValue;
#ifndef NEW_HARDWARE_FOR_LAST_AMP
	if (QAMMode == HWL_CONST_MODULATOR_QAM_128)
	{
		if (Level < 14)
		{
			lValue = Level * 3 + 80;
		}
		else
		{
			lValue = (Level - 14) * 4 + 122;
		}
	}
	else
	{
		if (Level < 10)
		{
			lValue = Level * 2 + 60;
		}
		else
		{
			lValue = (Level - 10) * 3 + 80;
		}
	}
#else
	S32 i, lBaseLevel, lLevel, lTotalRFOpend;

	/*生成Db->BIN对应表*/
	if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
	{
		for (i = 0; i < HWL_AD9789_MAX_ADJ_LEVEL; i++)
		{
			if (i == 0)
			{
				s_plValue[i] = 60;
			}
			else if (i < 10)
			{
				s_plValue[i] = 2 + s_plValue[i - 1];
			}
			else if (i < 24)
			{
				s_plValue[i] = 3 + s_plValue[i - 1];
			}
			else if (i < 41)
			{
				s_plValue[i] = 4 + s_plValue[i - 1];
			}
			else if (i < 48)
			{
				s_plValue[i] = 7 + s_plValue[i - 1];
			}
			else if (i < 52)
			{
				s_plValue[i] = 4 + s_plValue[i - 1];
			}
		}

	}
	else
	{

		/*生成AD9789 GAIN Db->BIN对应表*/
		for (i = 0; i < HWL_AD9789_4350_MAX_ADJ_LEVEL; i++)
		{
			if (i == 0)
			{
				stModulator.p9789GainLevelValue[i] = 32 + 6;
			}
			else if (i < 6)
			{
				stModulator.p9789GainLevelValue[i] = 1 + stModulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 16 ) 
			{
				stModulator.p9789GainLevelValue[i] = 2 + stModulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 24) 
			{
				stModulator.p9789GainLevelValue[i] = 2 + stModulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 38)
			{
				stModulator.p9789GainLevelValue[i] = 3 + stModulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 55)
			{
				stModulator.p9789GainLevelValue[i] = 4 + stModulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 62)
			{
				stModulator.p9789GainLevelValue[i] = 7 + stModulator.p9789GainLevelValue[i - 1];
			}			
			else if (i < HWL_AD9789_4350_MAX_ADJ_LEVEL)
			{
				if ((3 + stModulator.p9789GainLevelValue[i - 1]) < 255)
				{
					stModulator.p9789GainLevelValue[i] = (3 + stModulator.p9789GainLevelValue[i - 1]);
				}
				else
				{
					stModulator.p9789GainLevelValue[i] = 255;
				}
			}

		}	
	}

	lTotalRFOpend = 0;
	for (i = 0; i < HWL_CONST_MAX_QAM_CHN_NUM; i++)
	{
		if (stModulator.pParam[i].RFSwitch)
		{
			lTotalRFOpend++;
		}
	}

	if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
	{
		if (lTotalRFOpend == 1)
		{
			lBaseLevel = HWL_AD9789_MAX_ADJ_LEVEL - 2;
		}
		else if (lTotalRFOpend == 2)
		{
			lBaseLevel = HWL_AD9789_MAX_ADJ_LEVEL - 2 - 12/*3Db*/;
		}
		else if (lTotalRFOpend == 3)
		{
			lBaseLevel = HWL_AD9789_MAX_ADJ_LEVEL - 2 - 19/*4.75Db*/;;
		}
		else if (lTotalRFOpend == 4)
		{
			lBaseLevel = HWL_AD9789_MAX_ADJ_LEVEL - 2 - 26/*6Db*/;;
		}

		lLevel = lBaseLevel - 20 + Level + ((QAMMode==HWL_CONST_MODULATOR_QAM_128)?10:0) ;

		if (GLOBAL_CHECK_INDEX(lLevel, HWL_AD9789_MAX_ADJ_LEVEL))
		{
			lValue = s_plValue[lLevel];
		}
		else
		{
			lValue = s_plValue[HWL_AD9789_MAX_ADJ_LEVEL - 1];
			GLOBAL_TRACE(("Error! lLevel = %d, Level = %d, lBaseLevel = %d\n", lLevel, Level, lBaseLevel));
		}

	}
	else
	{

		switch (lTotalRFOpend)
		{
		case 1:
			lBaseLevel = HWL_AD9789_4350_MAX_ADJ_LEVEL;/* - (logf(lTotalRFOpend) * 10) * 4*/
			break;
		case 2:
			lBaseLevel = HWL_AD9789_4350_MAX_ADJ_LEVEL - 12/*3Db*/;
			break;
		case 3:
			lBaseLevel = HWL_AD9789_4350_MAX_ADJ_LEVEL - 19/*4.75Db*/;;
			break;
		case 4:
			lBaseLevel = HWL_AD9789_4350_MAX_ADJ_LEVEL - 24/*6Db*/;;
			break;
		default:
			lBaseLevel = HWL_AD9789_4350_MAX_ADJ_LEVEL - 26;
			break;
		}

		lLevel = lBaseLevel + (Level - 40) + ((QAMMode==HWL_CONST_MODULATOR_QAM_128)?10:0) - 10;

		if (GLOBAL_CHECK_INDEX(lLevel, HWL_AD9789_4350_MAX_ADJ_LEVEL))
		{
			lValue = stModulator.p9789GainLevelValue[lLevel];
		}
		else
		{
			if (lLevel >= HWL_AD9789_4350_MAX_ADJ_LEVEL)
			{
				lValue = stModulator.p9789GainLevelValue[HWL_AD9789_4350_MAX_ADJ_LEVEL - 1];
			}
			else
			{
				lValue = stModulator.p9789GainLevelValue[0];
			}
		}
	}
	GLOBAL_TRACE(("lTotalRFOpend = %d, Value = %d\n", lTotalRFOpend, lValue));
#endif

	return lValue;
}

/*调整参数*/
S32 HWLL_QamAdjustParameter(S16 ChnInd)
{

	S16 i;
	S16 lModuleIndex, lSubChannelIndex;
	S32 lFirstFreq;
	HWL_ModulatorParam_t *plModulatorParam, *plSubModulatorParam;

	assert(ChnInd >= 0 && ChnInd < 4);


	plModulatorParam = &stModulator.pParam[ChnInd];

	lModuleIndex = ChnInd / HWL_AD9789_MODULE_SUB_CHENNEL_NUM;
	lSubChannelIndex = ChnInd % HWL_AD9789_MODULE_SUB_CHENNEL_NUM;

	if (plModulatorParam->AnalogBand == HWL_CONST_MODULATOR_ANALOG_BAND_8M)
	{
		lFirstFreq = plModulatorParam->ModulateFreq - 8000 * lSubChannelIndex;
	}
	else
	{
		lFirstFreq = plModulatorParam->ModulateFreq - 6000 * lSubChannelIndex;
	}

	if (plModulatorParam->ModulateStandard == HWL_CONST_MODULATOR_STANDARD_ANNEX_B)
	{
		if (plModulatorParam->QamMode == HWL_CONST_MODULATOR_QAM_64)
		{
			plModulatorParam->SymRate = 5057;
		}
		else
		{
			plModulatorParam->SymRate = 5361;
		}
	}
	/*
	Attenuator 仅一个
	*/

	for (i = 0; i < HWL_CONST_MAX_QAM_CHN_NUM; i++)
	{
		plSubModulatorParam = &stModulator.pParam[i];
		plSubModulatorParam->Attenuator = plModulatorParam->Attenuator;
	}

	/*
	这四个参数按模块独立
	ModulateStandard
	AnalogBand
	QamMode
	SymRate
	ModulateFreq 相关
	*/


	for (i = lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM; i < (lModuleIndex + 1) * HWL_AD9789_MODULE_SUB_CHENNEL_NUM; i++)
	{
		plSubModulatorParam = &stModulator.pParam[i];

		plSubModulatorParam->ModulateStandard = plModulatorParam->ModulateStandard;
		plSubModulatorParam->AnalogBand = plModulatorParam->AnalogBand;
		plSubModulatorParam->QamMode = plModulatorParam->QamMode;
		plSubModulatorParam->SymRate = plModulatorParam->SymRate;
		//plSubModulatorParam->Gain = plModulatorParam->Gain;
		plSubModulatorParam->Attenuator = plModulatorParam->Attenuator;
		plSubModulatorParam->SpectrumInvert = plModulatorParam->SpectrumInvert;

		if (plModulatorParam->AnalogBand == HWL_CONST_MODULATOR_ANALOG_BAND_8M)
		{
			plSubModulatorParam->ModulateFreq = lFirstFreq + 8000 * (i % HWL_AD9789_MODULE_SUB_CHENNEL_NUM);
		}
		else
		{
			plSubModulatorParam->ModulateFreq = lFirstFreq + 6000 * (i % HWL_AD9789_MODULE_SUB_CHENNEL_NUM);
		}


	}


	return HWL_SUCCESS;
}


/* API 函数--------------------------------------------------------------------------------------------------------*/

/*初始化*/
#define MAX_CUR_NODE_NUM	160
static CAL_CURVE_NODE s_pFGCUVENode[MAX_CUR_NODE_NUM];
static S32 s_FGCUVENodeNum = 0;

void HWL_QAMInitialize(U8 ModulatorType)
{
	GLOBAL_ZEROMEM(&stModulator, sizeof(stModulator));

	stModulator.ModuleType = ModulatorType;

	GLOBAL_TRACE(("Modulator Type = %d\n", ModulatorType));
	//设置为默认值.
	HWLL_QamSetParameterDefault(-1);	

	//必须预先配置一次4360 否则有不正常的机子！
	if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
	{
		HWLL_ApplyADF4360(HWL_AD9789_FDAC0);
	}
	else
	{
#if defined(GQ3760A) || defined(GQ3760B) || defined(GQ3763) || defined(GQ3768)
		HWLL_ApplyADF4350(HWL_AD9789_FDAC3_FOR_3760);
#else
		HWLL_ApplyADF4350(HWL_AD9789_FDAC0);
#endif
	}

#if defined(GQ3760A) || defined(GQ3760B) || defined(GQ3763) || defined(GQ3768)
	{
		S32 i;
		/*初始化曲线拟合节点*/
		s_FGCUVENodeNum = 0;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 30 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x51;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 40 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x44;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 100 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 120 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x52;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 140 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x54;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 160 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4F;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 200 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 220 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x54;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 240 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x58;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 260 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5E;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 280 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 300 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x54;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 340 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 380 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 400 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 420 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 440 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x56;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 450 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x49;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 460 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x48;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 480 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x44;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 520 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x44;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 540 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x40;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 550 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3F;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 560 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x47;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 580 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x40;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 600 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 620 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x37;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 640 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x37;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 660 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 690 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 700 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 720 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 740 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x39;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 760 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 770 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x35;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 780 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x38;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 800 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 820 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x40;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 840 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x43;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 860 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x47;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 880 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 900 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4E;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 940 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x52;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 950 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x55;
		s_FGCUVENodeNum ++;

		for (i = 0; i < s_FGCUVENodeNum; i++)
		{
			GLOBAL_TRACE(("Freq = %d Hz, SetValue = 0x%.2X\n", s_pFGCUVENode[i].m_CompareValue, s_pFGCUVENode[i].m_SetValue));
		}
		
	}
#endif

}

/*设置参数*/
BOOL HWL_QAMParameterSet(S16 ChnInd, HWL_ModulatorParam_t *pParam)
{
	HWL_ModulatorParam_t *plInfo;

	if(pParam == NULL)
	{
		return FALSE;
	}

	if( ChnInd < 0 || ChnInd >= HWL_CONST_MAX_QAM_CHN_NUM)
	{
		GLOBAL_TRACE(("Chn Index = %d\n", ChnInd));
		return FALSE;
	}
	plInfo = &stModulator.pParam[ChnInd];

	stModulator.bLevelOnly &= (plInfo->ModulateStandard == pParam->ModulateStandard);
	stModulator.bLevelOnly &= (plInfo->ModulateFreq == pParam->ModulateFreq);
	stModulator.bLevelOnly &= (plInfo->SymRate == pParam->SymRate);
	stModulator.bLevelOnly &= (plInfo->QamMode == pParam->QamMode);
	stModulator.bLevelOnly &= (plInfo->AnalogBand == pParam->AnalogBand);
	stModulator.bLevelOnly &= (plInfo->SpectrumInvert == pParam->SpectrumInvert);
	stModulator.bLevelOnly &= (plInfo->QamSwitch == pParam->QamSwitch);
	stModulator.bLevelOnly &= (plInfo->RFSwitch == pParam->RFSwitch);
	stModulator.bLevelOnly &= (plInfo->QPSK_FEC == pParam->QPSK_FEC);

	GLOBAL_MEMCPY(plInfo, pParam, sizeof(HWL_ModulatorParam_t));

	return HWLL_QamAdjustParameter(ChnInd);
}

/*应用参数*/
S32 HWL_QAMParameterApply(S16 TsIndex)
{
	if (stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QAM)
	{
		U8 lSubChannelEnable;
		U32 lRegData32;
		S16 lICID, lModuleIndex, lSubChannelIndex;
		S32 lCentralFreq, lTmpFreq;
		U32 lFDAC;
		F64 lFTW ,lFTWBPF;
		HWL_ModulatorParam_t *plModulatorParam, *plSubModulatorParam;

		lFDAC = HWL_AD9789_FDAC0;

		lFTW = (F64)(0xFFFFFF) / ((F64)(lFDAC + stModulator.FrequeceCalibrate) / 16);//中心频率计算
		lFTWBPF = (F64)0xFFFF / ((F64)(lFDAC + stModulator.FrequeceCalibrate));//BPF频率计算

		GLOBAL_TRACE(("Start Set DVB-C Parameter\n"));

		if (stModulator.bLevelOnly == FALSE)
		{
			GLOBAL_TRACE(("Reset All!\n"));

			HWLL_RestAndInitiateAD9789();

			if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
			{
				HWLL_ApplyADF4360(lFDAC);
			}
			else
			{
				HWLL_ApplyADF4350(lFDAC);
			}

			plModulatorParam = &stModulator.pParam[0];

			/*
			这四个参数按模块独立
			ModulateStandard
			ModulateFreq 相关
			QamMode
			SymRate
			AnalogBan.
			*/

			for (lModuleIndex = 0; lModuleIndex < HWL_AD9789_MODULE_NUM; lModuleIndex++)
			{
				/*参数预计算*/
				lICID = ICPL_IIC_IC_ID_AD9789_START +  lModuleIndex;
				plModulatorParam =  &stModulator.pParam[lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];//每个AD9789的第一个通道的参数

				/*设置FPGA QAM调制开关*/
				lRegData32 = 0;
				for (lSubChannelIndex = 0; lSubChannelIndex < HWL_AD9789_MODULE_SUB_CHENNEL_NUM; lSubChannelIndex++)
				{
					plSubModulatorParam = &stModulator.pParam[lSubChannelIndex + lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];
					if (plSubModulatorParam->QamSwitch)
					{
						lRegData32 +=  1 << ( lSubChannelIndex + 4);//设置通道开关
					}
				}

				//FPGA QAM调制方式寄存器值
				lRegData32 += plModulatorParam->QamMode & 0x0F;//QAM调制方式寄存器值
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_AD9789_QAM_SP_INFO, (lRegData32 & 0xFF));
				ICPL_AD9789BroadOperationApply();
			}


			for (lModuleIndex = 0; lModuleIndex < HWL_AD9789_MODULE_NUM; lModuleIndex++)
			{
				/*参数预计算*/
				lICID = ICPL_IIC_IC_ID_AD9789_START +  lModuleIndex;
				plModulatorParam =  &stModulator.pParam[lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];//每个AD9789的第一个通道的参数
				if (plModulatorParam->AnalogBand == HWL_CONST_MODULATOR_ANALOG_BAND_8M)
				{
					lCentralFreq = plModulatorParam->ModulateFreq  + 8000 * 3 / 2;
				}
				else
				{
					lCentralFreq = plModulatorParam->ModulateFreq + 6000 * 3 / 2;
				}

				//设置QAM和SRRC寄存器
				//printf("ModulateStandard = %d, QamMode = %d\n", plModulatorParam->ModulateStandard, plModulatorParam->QamMode);
				if (plModulatorParam->ModulateStandard == HWL_CONST_MODULATOR_STANDARD_ANNEX_B)
				{
					switch(plModulatorParam->QamMode)
					{
					case HWL_CONST_MODULATOR_QAM_64:
						lRegData32 = 0x10 + 0;
						//symratet=5057;
						break;
					case HWL_CONST_MODULATOR_QAM_256:
						lRegData32 = 0x00 + 1;
						//symratet=5360;
						break;
					default:
						break;
					}
				}
				else if (plModulatorParam->ModulateStandard == HWL_CONST_MODULATOR_STANDARD_ANNEX_A)
				{
					lRegData32 = (plModulatorParam->QamMode + 2) & 0x07;
					lRegData32 += 0x20;
				}
				else if (plModulatorParam->ModulateStandard == HWL_CONST_MODULATOR_STANDARD_ANNEX_C)
				{
					lRegData32 = (plModulatorParam->QamMode + 2) & 0x07;
					lRegData32 += 0x30;
				}
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x07, (lRegData32 & 0xFF));

				//if(qammode==0)
				//{
				//	ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x08,0x10);
				//}
				//else
				//{
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x08, 0x14);
				//}

				lSubChannelEnable = 0;
				for (lSubChannelIndex = 0; lSubChannelIndex < HWL_AD9789_MODULE_SUB_CHENNEL_NUM; lSubChannelIndex++)
				{
					plSubModulatorParam = &stModulator.pParam[lSubChannelIndex + lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];

					//通道频率设置 单位KHz
					lRegData32 = lFTW * plSubModulatorParam->ModulateFreq * 1000;//频率的单位为KHz所以乘以1000得到HZ级别
					//GLOBAL_TRACE(("-----------------------------------------RegFreq = %.8X--------------------------------------\n", lRegData32));
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0C + (lSubChannelIndex * 3), ((lRegData32 >> 16) & 0xFF));
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0B + (lSubChannelIndex * 3), ((lRegData32 >> 8) & 0xFF));
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0A + (lSubChannelIndex * 3), (lRegData32 & 0xFF));

					if (plSubModulatorParam->RFSwitch)
					{
						lSubChannelEnable += (1 << lSubChannelIndex);
					}

				}

				//printf("Module %d Channel Enable 0x%.4x\n", lModuleIndex, lSubChannelEnable);

#if 0
				//fdac=i*p/q*16*symble_rate ;i=32
				//符号率设置 单位KBoud/s
				//printf("SymRate = %d\n", plModulatorParam->SymRate);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x06, 0x10);


				lRegData32 = plModulatorParam->SymRate * 32 * 65;//??

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x18, ((lRegData32 >> 16) & 0xFF)); //q
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x17, ((lRegData32 >> 8) & 0xFF));
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x16, (lRegData32 & 0xFF));


				lRegData32 = 8775000 ;//((FDAC/16)/10000)*(65*50/5);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1B, ((lRegData32 >> 16) & 0xFF)); //p
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1A, ((lRegData32 >> 8) & 0xFF));
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x19, (lRegData32 & 0xFF));
#else
				{
					U32 P, Q, N;

					HWLL_QamCalculatePQN(lFDAC + stModulator.FrequeceCalibrate, plModulatorParam->SymRate, &P, &Q, &N);

					/*低符号率测试！20120605*/
					if (plModulatorParam->SymRate <= 4218)
					{
						ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x06, 0x00);
					}
					else
					{
						ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x06, 0x10);
					}

					lRegData32 = Q;

					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x18, ((lRegData32 >> 16) & 0xFF));
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x17, ((lRegData32 >> 8) & 0xFF));
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x16, (lRegData32 & 0xFF));

					lRegData32 = P ;
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1B, ((lRegData32 >> 16) & 0xFF));
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1A, ((lRegData32 >> 8) & 0xFF));
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x19, (lRegData32 & 0xFF));
				}
#endif


				//lRegData32 = (((lCentralFreq / 1000) * 0xFFFF)) / 2160;

				//GLOBAL_TRACE(("BPF Reg1 = 0x%.8X\n", lRegData32));

				lRegData32 = lCentralFreq * lFTWBPF * 1000;


				//GLOBAL_TRACE(("BPF Reg2 = 0x%.8X\n", lRegData32));

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1D, ((lRegData32 >> 8) & 0xFF));
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1C, ((lRegData32 & 0xFF)));

#if 1//32分频

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x20, 0xD4);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x21, 0xA3);

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x22, 0x0F);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x23, 0x88);

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x22, 0x2F);

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00); //使得0x22-0x23的寄存器设置有效
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);
#else	//16分频
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x20, 0xc4);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x21, 0xe2); //a1
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x22, 0x0f); //disable
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x23, 0x98); //sdc phase
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x22, 0x1f); //fdaclk/32
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00);
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);
#endif


				//频谱翻转
				if(plModulatorParam->SpectrumInvert)
				{
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x29, 0x01);
				}
				else
				{
					ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x29, 0x00);
				}

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x38, 0x00);

				lTmpFreq = lCentralFreq / 50;
				if( lTmpFreq > 10000)
				{
					lRegData32 = (lTmpFreq - 5400) * 4 / 46;
				}
				else
				{
					lRegData32 = (lTmpFreq + 13500) * 4 / 235;

				}

				if((lCentralFreq > 440000) & (lCentralFreq < 560000))
				{
					lRegData32 = lRegData32 + 130;
				}

				if(lCentralFreq > 760000)
				{
					lRegData32 = 0x03FF;
				}

				if(lRegData32 > 0x03FF)
				{
					lRegData32 = 0x03FF;
				}
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x3D, ((lRegData32 >> 8) & 0x03));
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x3C, (lRegData32 & 0xFF));

				ICPL_AD9789BroadOperationApply();

				PFC_TaskSleep(100);

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1E, 0x80);

				//子通道开关
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x05, (lSubChannelEnable & 0x0F));

				ICPL_AD9789BroadOperationApply();
			}
		}


		GLOBAL_TRACE(("Set Level\n"));
		plModulatorParam = &stModulator.pParam[0];

		/*AD5245（电位器）设置*/
		lICID = ICPL_IIC_IC_ID_AD5245;
		if ((plModulatorParam->Attenuator < 0) || (plModulatorParam->Attenuator > AD533X_MAX_LEVEL))
		{
			plModulatorParam->Attenuator = 0;
		}

		ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_AD5245, stQAMRFLevelTabel[plModulatorParam->Attenuator] );
		ICPL_AD9789BroadOperationApply();


		for (lModuleIndex = 0; lModuleIndex < HWL_AD9789_MODULE_NUM; lModuleIndex++)
		{
			/*参数预计算*/
			lICID = ICPL_IIC_IC_ID_AD9789_START +  lModuleIndex;
			plModulatorParam =  &stModulator.pParam[lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];//每个AD9789的第一个通道的参数

			lSubChannelEnable = 0;
			for (lSubChannelIndex = 0; lSubChannelIndex < HWL_AD9789_MODULE_SUB_CHENNEL_NUM; lSubChannelIndex++)
			{
				plSubModulatorParam = &stModulator.pParam[lSubChannelIndex + lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];

				//通道增益设置
				lRegData32 = HWLL_GetAD9789GainValue(plSubModulatorParam->Gain, plSubModulatorParam->QamMode);

				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25 + lSubChannelIndex,  lRegData32 & 0xFF);
			}
		}
		ICPL_AD9789BroadOperationApply();

		stModulator.bLevelOnly = TRUE;//恢复标志以待下一次设定
	}
	else if (stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_L || stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_H)
	{
		U8	lTmpPowern;
		U32 lFDAC_FCenter;

		U32 lFDAC;
		U32 P, Q, N;
		F64 lPQFactor;

		U32 lFDAC1;
		U32 P1, Q1, N1;
		F64 lPQFactor1;

		U32 lFDAC2;
		U32 P2, Q2, N2;
		F64 lPQFactor2;

		U32 lRegData32;
		S16 lICID;
		S32 lCentralFreq, lTmpFreq, lBFPFreq;
		F64 lFTW, lFTWBPF, lFDAC16;
		HWL_ModulatorParam_t *plModulatorParam;

		/*取得参数*/
		plModulatorParam = &stModulator.pParam[0];

		GLOBAL_TRACE(("Start Set DVB-S Parameter\n"));

		if (stModulator.bLevelOnly == FALSE)
		{
			/*发送FEC给FPGA*/
			lICID = ICPL_IIC_IC_ID_AD9789_START;

			lRegData32 = plModulatorParam->QPSK_FEC;
			GLOBAL_TRACE(("FEC REG= 0x%.2X\n", lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_AD9789_QBSK_SP_INFO, (lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationApply();


			/*计算符号率决定使用的FDAC的值*/

			HWLL_QamCalculatePQN(HWL_AD9789_FDAC1, plModulatorParam->SymRate, &P1, &Q1, &N1);
			lPQFactor1 = (F64)P1 / (F64)Q1;

			HWLL_QamCalculatePQN(HWL_AD9789_FDAC2, plModulatorParam->SymRate, &P2, &Q2, &N2);
			lPQFactor2 = (F64)P2 / (F64)Q2;



#if 1
			if (plModulatorParam->SymRate >= 35727 && plModulatorParam->SymRate <= 36818 || 
				plModulatorParam->SymRate >= 40909 && plModulatorParam->SymRate <= 41666)
			{
				HWLL_QamCalculatePQN(HWL_AD9789_FDAC0, plModulatorParam->SymRate, &P, &Q, &N);
				lFDAC = HWL_AD9789_FDAC0;
				lPQFactor = (F64)P / (F64)Q;
			}
			else if ((lPQFactor1 >= 0.571433 && lPQFactor1 <= 1) && (lPQFactor2 >= 0.571433 && lPQFactor2 <= 1))
			{
				//if (plModulatorParam->SpectrumInvert == FALSE)
				{
					if ((N2 < 7) && (N2 > 1))
					{
						lFDAC = HWL_AD9789_FDAC2;
						lPQFactor = lPQFactor2;
						P = P2;
						Q = Q2;
						N = N2;
					}
					else
					{
						lFDAC = HWL_AD9789_FDAC1;
						lPQFactor = lPQFactor1;
						P = P1;
						Q = Q1;
						N = N1;
					}
				}
				//else
				//{
				//	if ((N1 < 7) && (N1 > 1))
				//	{
				//		lFDAC = HWL_AD9789_FDAC1;
				//		lPQFactor = lPQFactor1;
				//		P = P1;
				//		Q = Q1;
				//		N = N1;
				//	}
				//	else
				//	{
				//		lFDAC = HWL_AD9789_FDAC2;
				//		lPQFactor = lPQFactor2;
				//		P = P2;
				//		Q = Q2;
				//		N = N2;
				//	}

				//}
			}
			else if (lPQFactor1 >= 0.571433 && lPQFactor1 <= 1)
			{
				lFDAC = HWL_AD9789_FDAC1;
				lPQFactor = lPQFactor1;
				P = P1;
				Q = Q1;
				N = N1;
			}
			else if (lPQFactor2 >= 0.571433 && lPQFactor2 <= 1)
			{
				lFDAC = HWL_AD9789_FDAC2;
				lPQFactor = lPQFactor2;
				P = P2;
				Q = Q2;
				N = N2;
			}
			else
			{
				lFDAC = HWL_AD9789_FDAC2;
				lPQFactor = lPQFactor2;
				P = P2;
				Q = Q2;
				N = N2;
			}



#else
			if (plModulatorParam->SpectrumInvert == FALSE)
			{
				lFDAC = HWL_AD9789_FDAC2;
				lPQFactor = lPQFactor2;
				P = P2;
				Q = Q2;
				N = N2;
			}
			else
			{
				lFDAC = HWL_AD9789_FDAC1;
				lPQFactor = lPQFactor1;
				P = P1;
				Q = Q1;
				N = N1;
			}
#endif
			GLOBAL_TRACE(("FDAC = %u, Sym = %d, P = 0x%.6X, Q = 0x%.6X, P/Q = %f, N = %d\n", lFDAC, plModulatorParam->SymRate * 1000, P, Q, lPQFactor, N));
			if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
			{
				HWLL_ApplyADF4360(lFDAC);
			}
			else
			{
				HWLL_ApplyADF4350(lFDAC);
			}

			HWLL_RestAndInitiateAD9789();

			lFDAC += stModulator.FrequeceCalibrate;
			lFDAC16 = (F64)lFDAC / 16;
			lFTW = (F64)(0xFFFFFF) / lFDAC16;
			lFTWBPF = (F64)0xFFFF / ((F64)lFDAC);

			/*参数预计算*/
			lCentralFreq = plModulatorParam->ModulateFreq;
			lBFPFreq = plModulatorParam->ModulateFreq;

			lFDAC_FCenter = lFDAC / 1000 - lCentralFreq;

			while(lCentralFreq > 1000000/*1000MHz*/)
			{
				lCentralFreq -= (S32)(lFDAC16 / 1000);
			}


			/*通道频率设置 参数的单位是KHz所以乘以1000得到HZ*/
			lRegData32 = lFTW * lCentralFreq * 1000;
			GLOBAL_TRACE(("Center Freq = %dKHz, REG= 0x%.6X\n", lCentralFreq, lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0C, ((lRegData32 >> 16) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0B, ((lRegData32 >> 8) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0A, (lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationApply();

			/*16倍插值滤波器设置*/
			lRegData32 = lFTWBPF * lBFPFreq * 1000;
			GLOBAL_TRACE(("BFP Freq = %dKHz, REG= 0x%.6X\n", lBFPFreq, lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1D, ((lRegData32 >> 8) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1C, ((lRegData32 & 0xFF)));
			ICPL_AD9789BroadOperationApply();

			GLOBAL_TRACE(("Center Freq = %d MHz, BFP Freq = %d MHz, FDAC - FOut(BFP) = %d MHz\n", lCentralFreq / 1000, lBFPFreq / 1000, lFDAC_FCenter / 1000));

			/*速率转换设置*/
			lRegData32 = Q;
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x18, ((lRegData32 >> 16) & 0xFF)); //q
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x17, ((lRegData32 >> 8) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x16, (lRegData32 & 0xFF));

			lRegData32 = P;
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1B, ((lRegData32 >> 16) & 0xFF)); //p
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1A, ((lRegData32 >> 8) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x19, (lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationApply();




			/*模式选择寄存器，计算半带插值寄存器的个数和未知*/
			lTmpPowern = 0xDF;
			if (N > 1)
			{
				N--;
			}
			else if (N > 6)
			{
				N = 6;
			}
			while ( N > 0 )
			{
				lTmpPowern &= ~( 1 << ( N - 1 ) );	
				N--;
			}
			GLOBAL_TRACE(( "Reg0x06 Value = 0x%.2X\n", lTmpPowern ));

			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x06, lTmpPowern );//旁路QAM映射器（QAM MAPPING），平方根升余弦（SRRC）滤波器，以及所有的插值滤波器
			ICPL_AD9789BroadOperationApply();


			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x07, 0x02 );//SRRC和QAM映射器参数
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x09, 0xFF );//输入标量寄存器
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x08, 0x2F );//求和标量寄存器
			ICPL_AD9789BroadOperationApply();


			/*QDUC模式设置*/
			//ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x20, 0xC8);//DCO脚正向，QDUC模式开启，无奇偶校验
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x20, 0xD8);
			//ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x21, 0xF2);//32位总线宽度，第4个上升沿对数据进行采样，配置实数数据

			if (plModulatorParam->SymRate >= 32813 && plModulatorParam->SymRate <= 35269)
			{
				lRegData32 = 0x7F;
			}
			else if (plModulatorParam->SymRate >= 35270 && plModulatorParam->SymRate <= 36818)
			{
				lRegData32 = 0x7D;
			}
			else if (plModulatorParam->SymRate >= 36819 && plModulatorParam->SymRate <= 37499)
			{
				lRegData32 = 0x7C;
			}
			else if (plModulatorParam->SymRate >= 37500 && plModulatorParam->SymRate <= 40908)
			{
				lRegData32 = 0x7D;
			}
			else if (plModulatorParam->SymRate >= 40909 && plModulatorParam->SymRate <= 41665)
			{
				lRegData32 = 0x7D;
			}
			else if (plModulatorParam->SymRate >= 41667 && plModulatorParam->SymRate <= 45000)
			{
				lRegData32 = 0x7C;
			}
			else if (lPQFactor >= 0.571433 && lPQFactor < 0.583345)
			{
				lRegData32 = 0x7E;
			}
			else if (lPQFactor >= 0.583345 && lPQFactor < 0.600021)
			{
				lRegData32 = 0x7D;
			}
			else if (lPQFactor >= 0.600021 && lPQFactor < 0.625023)
			{
				lRegData32 = 0x7C;
			}
			else if (lPQFactor >= 0.625023 && lPQFactor < 0.666654)
			{
				lRegData32 = 0x7B;
			}
			else if (lPQFactor >= 0.666654 && lPQFactor < 0.700004)
			{
				lRegData32 = 0x7E;
			}
			else if (lPQFactor >= 0.700004 && lPQFactor < 0.7500033)
			{
				lRegData32 = 0x7D;
			}
			else if (lPQFactor >= 0.7500033 && lPQFactor < 0.833375)
			{
				lRegData32 = 0x7C;
			}
			else if (lPQFactor >= 0.833375 && lPQFactor <= 0.9)
			{
				lRegData32 = 0x7B;
			}
			else if (lPQFactor >= 0.9 && lPQFactor <= 1)
			{
				lRegData32 = 0x7B;
			}
			else
			{
				lRegData32 = 0x7F;
				GLOBAL_TRACE(("Factor Error!!\n"));
			}

			GLOBAL_TRACE(("Reg0x21 Value = 0x%.2X\n", lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x21, lRegData32);


			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x22, 0x0F);//暂时DISABLE
			//ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x23, 0x98);//DSC数据采样时钟/SNC同步时钟设置
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x23, 0x83);

			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00);
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);//应用上述3个寄存器

			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x22, 0x1F);//DCO设置为FDAC/16

			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00);
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);//应用上述3个寄存器
			ICPL_AD9789BroadOperationApply();

			/*频谱翻转 设置*/
			if(plModulatorParam->SpectrumInvert)
			{
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x29, 0x01);
			}
			else
			{
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x29, 0x00);
			}

			/*噪声优化*/
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x36, 0x01);
			/*DAC解码模式，00为普通*/
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x38, 0x00);

			/*DAC电流设置*/
			lTmpFreq = lCentralFreq / 50;
			if( lTmpFreq > 10000)
			{
				lRegData32 = (lTmpFreq - 5400) * 4 / 46;
			}
			else
			{
				lRegData32 = (lTmpFreq + 13500) * 4 / 235;

			}

			if((lCentralFreq > 440000) & (lCentralFreq < 560000))
			{
				lRegData32 = lRegData32 + 130;
			}

			if(lCentralFreq > 760000)
			{
				lRegData32 = 0x03FF;
			}

			if(lRegData32 > 0x03FF)
			{
				lRegData32 = 0x03FF;
			}
			GLOBAL_TRACE(("REG 0x3C/3D = %.6X\n", lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x3D, ((lRegData32 >> 8) & 0x03));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x3C, (lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationApply();

			PFC_TaskSleep(100);

			/*更新速率转换寄存器和16倍插值寄存器寄存器*/
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1E, 0x80);
			ICPL_AD9789BroadOperationApply();


			if (plModulatorParam->QPSK_FEC == HWL_CONST_MODULATOR_FEC_1_OF_2)
			{
				lRegData32 = HWL_CONST_MODULATOR_FEC_7_OF_8;
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_AD9789_QBSK_SP_INFO, (lRegData32 & 0xFF));
				ICPL_AD9789BroadOperationApply();
			}
			else
			{
				lRegData32 = HWL_CONST_MODULATOR_FEC_1_OF_2;
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_AD9789_QBSK_SP_INFO, (lRegData32 & 0xFF));
				ICPL_AD9789BroadOperationApply();
			}

			lRegData32 = plModulatorParam->QPSK_FEC;
			GLOBAL_TRACE(("FEC REG= 0x%.2X\n", lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_AD9789_QBSK_SP_INFO, (lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationApply();
		}




		/*通道增益设置*/
		lRegData32 = HWLL_GetAD9789GainValue(plModulatorParam->Gain, plModulatorParam->QamMode);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25,  lRegData32);
		lRegData32 = 0;
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25 + 1,  lRegData32 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25 + 2,  lRegData32 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25 + 3,  lRegData32 & 0xFF);

		//子通道开关
		if (plModulatorParam->RFSwitch)
		{
			lRegData32 = 1;
		}
		else
		{
			lRegData32 = 0;
		}
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x05, (lRegData32 & 0x0F));
		ICPL_AD9789BroadOperationApply();

		if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
		{
			/*AD5245（电位器）设置*/
			lICID = ICPL_IIC_IC_ID_AD5245;

			GLOBAL_TRACE(("Config 5245-1! Value  = %d\n", plModulatorParam->Attenuator));
			if ((plModulatorParam->Attenuator < 0) || (plModulatorParam->Attenuator > AD533X_MAX_LEVEL))
			{
				plModulatorParam->Attenuator = 0;
			}
			GLOBAL_TRACE(("Config 5245-2! Value  = %d, REG = 0x%.2X\n", plModulatorParam->Attenuator, stQAMRFLevelTabel[plModulatorParam->Attenuator]));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_AD5245, stQAMRFLevelTabel[plModulatorParam->Attenuator] );
			ICPL_AD9789BroadOperationApply();

		}


		if (plModulatorParam->QamSwitch == FALSE)
		{
			ICPL_AD9789BroadOperationAdd(TRUE, ICPL_IIC_IC_ID_AD9789_START, 0x22, 0x0F);//DCO关闭模拟调制关闭现象
			ICPL_AD9789BroadOperationApply();
		}
		//stModulator.bLevelOnly = TRUE;//恢复标志以待下一次设定
	}
	else if (stModulator.ModuleType == HWL_CONST_MODULATOR_AD9789_DTMB)
	{
		U8	lTmpPowern;
		U32 lFDAC_FCenter;

		U32 lFDAC;
		U32 P, Q, N;
		F64 lPQFactor;

		U32 lRegData32;
		S16 lICID;
		S32 lCentralFreq, lTmpFreq, lBFPFreq;
		F64 lFTW, lFTWBPF, lFDAC16;
		HWL_ModulatorParam_t *plModulatorParam;

		/*取得参数*/
		plModulatorParam = &stModulator.pParam[0];

		GLOBAL_TRACE(("Start Set DTMB Parameter\n"));

		if (stModulator.bLevelOnly == FALSE)
		{
			/*发送FEC给FPGA*/
			lICID = ICPL_IIC_IC_ID_AD9789_START;

			GLOBAL_TRACE(("Freq = %d, QAM = %d, Carrier = %d, PN = %d, CodeRate = %d, TI = %d, Pilot = %d\n", plModulatorParam->ModulateFreq, plModulatorParam->QamMode, plModulatorParam->DTMB_Carrier, plModulatorParam->DTMB_PN, plModulatorParam->DTMB_CodeRate, plModulatorParam->DTMB_TI, plModulatorParam->DTMB_DoublePilot));

			/*计算符号率决定使用的FDAC的值*/
			plModulatorParam->SymRate = HWL_AD9789_DTMB_DEFAULT_SYMBO_RATE;//符号率是个固定值！

			lFDAC = HWL_AD9789_FDAC3_FOR_3760;

			if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
			{
				HWLL_ApplyADF4360(lFDAC);
			}
			else
			{
				HWLL_ApplyADF4350(lFDAC);
			}

			lFDAC += stModulator.FrequeceCalibrate;


			HWLL_QamCalculatePQN(lFDAC, plModulatorParam->SymRate, &P, &Q, &N);
			lPQFactor = (F64)P / (F64)Q;
			GLOBAL_TRACE(("FDAC = %u, Sym = %d, P = 0x%.6X, Q = 0x%.6X, P/Q = %f, N = %d\n", lFDAC, plModulatorParam->SymRate * 1000, P, Q, lPQFactor, N));


			HWLL_RestAndInitiateAD9789();

			lFDAC16 = (F64)lFDAC / 16;
			lFTW = (F64)(0xFFFFFF) / lFDAC16;
			lFTWBPF = (F64)0xFFFF / ((F64)lFDAC);

			/*参数预计算*/
			lCentralFreq = plModulatorParam->ModulateFreq;
			lBFPFreq = plModulatorParam->ModulateFreq;

			lFDAC_FCenter = lFDAC / 1000 - lCentralFreq;

			while(lCentralFreq > 950000/*950MHz*/)
			{
				lCentralFreq -= (S32)(lFDAC16 / 1000);
			}

			/*通道频率设置 参数的单位是KHz所以乘以1000得到HZ*/
			lRegData32 = lFTW * lCentralFreq * 1000;
			GLOBAL_TRACE(("Center Freq = %dKHz, REG= 0x%.6X\n", lCentralFreq, lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0C, ((lRegData32 >> 16) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0B, ((lRegData32 >> 8) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x0A, (lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationApply();

			/*16倍插值滤波器设置*/
			lRegData32 = lFTWBPF * lBFPFreq * 1000;
			GLOBAL_TRACE(("BFP Freq = %dKHz, REG= 0x%.6X\n", lBFPFreq, lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1D, ((lRegData32 >> 8) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1C, ((lRegData32 & 0xFF)));
			ICPL_AD9789BroadOperationApply();

			GLOBAL_TRACE(("FDAC Real = %u, FDAC - FOut(BFP) = %d MHz\n", lFDAC, lFDAC_FCenter / 1000));

			/*速率转换设置*/
			lRegData32 = Q;
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x18, ((lRegData32 >> 16) & 0xFF)); //q
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x17, ((lRegData32 >> 8) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x16, (lRegData32 & 0xFF));

			lRegData32 = P;
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1B, ((lRegData32 >> 16) & 0xFF)); //p
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1A, ((lRegData32 >> 8) & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x19, (lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationApply();




			/*模式选择寄存器，计算半带插值寄存器的个数和未知*/
			lTmpPowern = 0xDF;
			if (N > 1)
			{
				N--;
			}
			else if (N > 6)
			{
				N = 6;
			}
			while ( N > 0 )
			{
				lTmpPowern &= ~( 1 << ( N - 1 ) );	
				N--;
			}
			GLOBAL_TRACE(( "Reg0x06 Value = 0x%.2X\n", lTmpPowern ));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x06, lTmpPowern );//旁路QAM映射器（QAM MAPPING），平方根升余弦（SRRC）滤波器，以及所有的插值滤波器
			ICPL_AD9789BroadOperationApply();


			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x07, 0x02 );//SRRC和QAM映射器参数

			//2014-10-14 使用标量求和寄存器来做频响
			lRegData32 = CAL_EXGetValue(s_pFGCUVENode, s_FGCUVENodeNum, lCentralFreq * 1000);

			GLOBAL_TRACE(("Freq  = %d, Reg 0x08 Value = %02X\n", lCentralFreq, lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x08, lRegData32 & 0xFF);//求和标量寄存器
			//ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x08, 0x60 );//求和标量寄存器

			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x09, 0x20 );//输入标量寄存器
			ICPL_AD9789BroadOperationApply();


			
#ifndef MULT_AD9789_USE_CHANNEL_MODE/*QDUC模式/通道模式设置*/
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x20, 0xD8);
			lRegData32 = 0x7F;
#else
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x20, 0xD0);//通道模式
			lRegData32 = 0x79;
#endif
			GLOBAL_TRACE(("Reg0x21 Value = 0x%.2X\n", lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x21, lRegData32);


			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x22, 0x0F);//暂时DISABLE
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x23, 0x83);

			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00);
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);//应用上述3个寄存器

			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x22, 0x1F);//DCO设置为FDAC/16

			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x00);
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x24, 0x80);//应用上述3个寄存器
			ICPL_AD9789BroadOperationApply();

			/*频谱翻转 设置*/
			GLOBAL_TRACE(("Spectrum Invert = %d\n", plModulatorParam->SpectrumInvert));
#ifndef MULT_AD9789_USE_CHANNEL_MODE
			if(plModulatorParam->SpectrumInvert)
			{
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x29, 0x01);
			}
			else
			{
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x29, 0x00);
			}
#else
			if(!plModulatorParam->SpectrumInvert)
			{
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x29, 0x01);
			}
			else
			{
				ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x29, 0x00);
			}
#endif

			/*噪声优化*/
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x36, 0x01);
			/*DAC解码模式，00为普通*/
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x38, 0x00);

			/*DAC电流设置*/
			lTmpFreq = lCentralFreq / 50;
			if( lTmpFreq > 10000)
			{
				lRegData32 = (lTmpFreq - 5400) * 4 / 46;
			}
			else
			{
				lRegData32 = (lTmpFreq + 13500) * 4 / 235;

			}

			if((lCentralFreq > 440000) & (lCentralFreq < 560000))
			{
				lRegData32 = lRegData32 + 130;
			}

			if(lCentralFreq > 760000)
			{
				lRegData32 = 0x03FF;
			}

			if(lRegData32 > 0x03FF)
			{
				lRegData32 = 0x03FF;
			}
			GLOBAL_TRACE(("REG 0x3C/3D = %.6X\n", lRegData32));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x3D, ((lRegData32 >> 8) & 0x03));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x3C, (lRegData32 & 0xFF));
			ICPL_AD9789BroadOperationApply();

			PFC_TaskSleep(100);

			/*更新速率转换寄存器和16倍插值寄存器寄存器*/
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x1E, 0x80);
			ICPL_AD9789BroadOperationApply();


			/*设置调制模块参数*/
			{
				S32 lPNFactor;
				S32 lCarrirMode;
				S32 lSFactor;
				S32 lDoublePilot;

				//发送给FPGA的命令，配置调试相关参数
				GLOBAL_TRACE(("QAM = %d, PN = %d, CodeRate = %d, Carrier = %d, DoublePilot = %d, Interleave = %d\n", plModulatorParam->QamMode, plModulatorParam->DTMB_PN, plModulatorParam->DTMB_CodeRate, plModulatorParam->DTMB_Carrier, plModulatorParam->DTMB_DoublePilot, plModulatorParam->DTMB_TI));

				switch (plModulatorParam->DTMB_PN)
				{
				case HWL_MODULATOR_DTMB_PN420:
					lPNFactor = 0;
					break;
				case HWL_MODULATOR_DTMB_PN595:
					lPNFactor = 1;
					break;
				case HWL_MODULATOR_DTMB_PN945:
					lPNFactor = 2;
					break;
				case HWL_MODULATOR_DTMB_PN420F:
					lPNFactor = 0 + 4;
					break;
				case HWL_MODULATOR_DTMB_PN595F:
					lPNFactor = 1 + 4;
					break;
				case HWL_MODULATOR_DTMB_PN945F:
					lPNFactor = 2 + 4;
					break;
				default:
					GLOBAL_TRACE(("Unknow PNMode = %d\n", plModulatorParam->DTMB_PN));
					lPNFactor = 0;
					break;
				}

				switch (plModulatorParam->DTMB_Carrier)
				{
				case HWL_MODULATOR_DTMB_CARRIER_SINGLE:
					lCarrirMode = 0;
					break;
				case HWL_MODULATOR_DTMB_CARRIER_MULTI:
					lCarrirMode = 1;
					break;
				default:
					GLOBAL_TRACE(("Unknow CarrierMode = %d\n", plModulatorParam->DTMB_Carrier));
					lCarrirMode = 0;
					break;
				}

				switch (plModulatorParam->DTMB_DoublePilot)
				{
				case 0:
					lDoublePilot = 0;
					break;
				case 1:
					lDoublePilot = 1;
					break;
				default:
					GLOBAL_TRACE(("Unknow DoublePilot = %d\n", plModulatorParam->DTMB_DoublePilot));
					lDoublePilot = 0;
					break;
				}


				switch (plModulatorParam->QamMode)
				{
				case HWL_CONST_MODULATOR_QAM_4:
					lSFactor = 0;
					break;
				case HWL_CONST_MODULATOR_QAM_4NR:
					lSFactor = 4;
					break;
				case HWL_CONST_MODULATOR_QAM_16:
					lSFactor = 8;
					break;
				case HWL_CONST_MODULATOR_QAM_32:
					lSFactor = 12;
					break;
				case HWL_CONST_MODULATOR_QAM_64:
					lSFactor = 12;
					break;
				default:
					GLOBAL_TRACE(("Unknow QAM Type = %d\n", plModulatorParam->QamMode));
					lSFactor = 12;
					break;
				}


				switch (plModulatorParam->DTMB_CodeRate)
				{
				case HWL_MODULATOR_DTMB_CodeRate_04:
					lSFactor += 1;
					break;
				case HWL_MODULATOR_DTMB_CodeRate_06:
					lSFactor += 2;
					break;
				case HWL_MODULATOR_DTMB_CodeRate_08:
					lSFactor += 3;
					break;
				default:
					GLOBAL_TRACE(("Unknow CodeRate = %d\n", plModulatorParam->DTMB_CodeRate));
					lSFactor += 3;
					break;
				}


				if (plModulatorParam->QamMode == HWL_CONST_MODULATOR_QAM_4NR)
				{
					lSFactor = 7;
				}
				else if (plModulatorParam->QamMode == HWL_CONST_MODULATOR_QAM_32)
				{
					lSFactor = 12;
				}

				switch (plModulatorParam->DTMB_TI)
				{
				case HWL_MODULATOR_DTMB_TI_240:
					lSFactor += 0;
					break;
				case HWL_MODULATOR_DTMB_TI_720:
					lSFactor += 16;
					break;
				default:
					GLOBAL_TRACE(("Unknow Interleave = %d\n", plModulatorParam->DTMB_TI));
					lSFactor = 0;
					break;
				}

				lRegData32 = ((lPNFactor << 5) & 0xE0) | (lSFactor & 0x1F);
				GLOBAL_TRACE(("Reg0x00 = %.2X\n", lRegData32));
				ICPL_AD9789BroadOperationAdd(TRUE, ICPL_IIC_IC_ID_FPGA_DTMB, 0x00, (lRegData32 & 0xFF));
				lRegData32 = ((lDoublePilot << 1) & 0x02) | (lCarrirMode & 0x01);
				GLOBAL_TRACE(("Reg0x01 = %.2X\n", lRegData32));
				ICPL_AD9789BroadOperationAdd(TRUE, ICPL_IIC_IC_ID_FPGA_DTMB, 0x01, (lRegData32 & 0xFF));
				ICPL_AD9789BroadOperationApply();
			}
		}

		/*通道增益设置*/
		lRegData32 = HWLL_GetAD9789GainValue(plModulatorParam->Gain, plModulatorParam->QamMode);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25,  lRegData32);
		lRegData32 = 0;
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25 + 1,  lRegData32 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25 + 2,  lRegData32 & 0xFF);
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x25 + 3,  lRegData32 & 0xFF);

		//子通道开关
		if (plModulatorParam->RFSwitch)
		{
			lRegData32 = 1;
		}
		else
		{
			lRegData32 = 0;
		}
		ICPL_AD9789BroadOperationAdd(TRUE, lICID, 0x05, (lRegData32 & 0x0F));
		ICPL_AD9789BroadOperationApply();

		if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
		{
			/*AD5245（电位器）设置*/
			lICID = ICPL_IIC_IC_ID_AD5245;

			GLOBAL_TRACE(("Config 5245-1! Value  = %d\n", plModulatorParam->Attenuator));
			if ((plModulatorParam->Attenuator < 0) || (plModulatorParam->Attenuator > AD533X_MAX_LEVEL))
			{
				plModulatorParam->Attenuator = 0;
			}
			GLOBAL_TRACE(("Config 5245-2! Value  = %d, REG = 0x%.2X\n", plModulatorParam->Attenuator, stQAMRFLevelTabel[plModulatorParam->Attenuator]));
			ICPL_AD9789BroadOperationAdd(TRUE, lICID, ICPL_IIC_REG_AD5245, stQAMRFLevelTabel[plModulatorParam->Attenuator] );
			ICPL_AD9789BroadOperationApply();

		}


		if (plModulatorParam->QamSwitch == FALSE)
		{
			ICPL_AD9789BroadOperationAdd(TRUE, ICPL_IIC_IC_ID_AD9789_START, 0x22, 0x0F);//DCO关闭模拟调制关闭现象
			ICPL_AD9789BroadOperationApply();
		}
		//stModulator.bLevelOnly = TRUE;//恢复标志以待下一次设定
	}
	else
	{
		GLOBAL_TRACE(("Unknow Type =- %d\n",stModulator.ModuleType));
	}
	return HWL_SUCCESS;
}


/*设置频率偏移*/
void HWL_QAMFDACOffsetSet(S32 FrequenceCalibrate)
{
	if (stModulator.FrequeceCalibrate != FrequenceCalibrate)
	{
		stModulator.FrequeceCalibrate = FrequenceCalibrate;
	}
}

/*PQ值计算*/
void HWLL_QamCalculatePQN(U32 CurrentFDAC, U32 SymRate, U32 *pP, U32 *pQ, U32 *pN)
{
	U32 P, Q, lMoveCount, N;
	F64 lPQFactor;
	P = CurrentFDAC / 16;
	Q = SymRate * 1000;

	if (P > Q)
	{
		N = 0;
		while(TRUE)
		{
			lPQFactor = (F64)P / (F64)Q;
			if (lPQFactor>= 0.5 && lPQFactor <= 1)
			{
				break;
			}
			else
			{
				P = P / 2;
				N++;
			}
		}

		if (Q > 0xFFFFFF)
		{
			lMoveCount = 0;
			while(Q > 0xFFFFFF)
			{
				Q = Q >> 1;
				lMoveCount++;
			}

			P = P >> lMoveCount;
		}
		else
		{
			lMoveCount = 0;
			while((Q & 0x800000) == 0)
			{
				Q = Q << 1;
				lMoveCount++;
			}

			P = P << lMoveCount;

		}

		GLOBAL_TRACE(("FDAC = %u, P = 0x%.8X Q = 0x%.8X, P/Q = %f, N = %d\n", CurrentFDAC, P, Q, lPQFactor, N));
	}
	else
	{
		GLOBAL_TRACE(("Symbol Rate Over 135M ,P = Q = 0x800000\n"));
		P = Q = 0x800000;
	}

	(*pP) = P;
	(*pQ) = Q;
	(*pN) = N;
}



/*销毁*/
void HWL_QAMTerminate(void)
{

}


/*调试用函数*/
static U32 s_PValue, s_QValue;
static BOOL s_UsePQ;

/*应用PQ值*/
void HWL_QAMApplyParameterWithPQ(U32 PValue, U32 QValue)
{
	s_PValue = PValue;
	s_QValue = QValue;
	s_UsePQ = TRUE;
	HWL_QAMParameterApply(0);
	s_UsePQ = FALSE;
}


/*直接设置寄存器*/
void HWL_QAMDirectRegSet(U8 ICID, U32 Addr, U32 Value)
{
	GLOBAL_TRACE(("ICID = 0x%x, Addr = 0x%x, Value = 0x%x\n", ICID, Addr, Value));
	ICPL_AD9789BroadOperationAdd(TRUE, ICID, Addr, Value);
	ICPL_AD9789BroadOperationApply();
}
#endif

