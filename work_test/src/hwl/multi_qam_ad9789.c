/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "multi_main_internal.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "multi_drv.h"

#ifdef USE_NEW_QAM_SETTING_FUNCTIONS

/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#ifndef DEBUG_MODULATOR
#define SUPPORT_SMOOTH_RF_OUTPUT_POWER_CHANGE
#endif

#define HWL_QAM_VCO_FDAC_HZ_2200			0x83215600						//2200000000 Hz
#define HWL_QAM_VCO_FDAC_HZ_2177			0x81C6A800						//2177280000 Hz
#define HWL_QAM_VCO_FDAC_HZ_2160			0x80BEFC00						//2160000000 Hz
#define HWL_QAM_VCO_FDAC_HZ_2380			0x8DDBEB00						//2380000000 Hz
#define HWL_QAM_VCO_FDAC_HZ_2400			0x8F0D1800						//2400000000 Hz

#define HWL_QAM_VCO_REF_CLK_HZ_2500			(25 * 1000 * 1000)				//25MHz
#define HWL_QAM_VCO_REF_CLK_HZ_5400			(54 * 1000 * 1000)				//54MHz
#define HWL_QAM_VCO_REF_CLK_HZ_1000			(10 * 1000 * 1000)				//10MHz
#define HWL_QAM_VCO_REF_CLK_HZ_4000			(40 * 1000 * 1000)				//40MHz
#define HWL_QAM_VCO_REF_CLK_HZ_3024			((U32)(30.24 * 1000 * 1000))	//30.24MHz


//2.200 25M 验证OK
#define AD_4350_REG_DATA_5_2200_2500		0x00580005
#define AD_4350_REG_DATA_4_2200_2500		0x009C803C
#define AD_4350_REG_DATA_3_2200_2500		0x000004B3
#define AD_4350_REG_DATA_2_2200_2500		0x14005EC2
#define AD_4350_REG_DATA_1_2200_2500		0x08008191
#define AD_4350_REG_DATA_0_2200_2500		0x00580000


//2.200 10M 验证OK
#define AD_4350_REG_DATA_5_2200_1000		0x00580005
#define AD_4350_REG_DATA_4_2200_1000		0x009A003C
#define AD_4350_REG_DATA_3_2200_1000		0x000004B3
#define AD_4350_REG_DATA_2_2200_1000		0x16005EC2
#define AD_4350_REG_DATA_1_2200_1000		0x08008141
#define AD_4350_REG_DATA_0_2200_1000		0x006E0000

//2.2G 40M
#define AD_4350_REG_DATA_5_2200_4000		0x00580005
#define AD_4350_REG_DATA_4_2200_4000		0x009A003C
#define AD_4350_REG_DATA_3_2200_4000		0x000004B3
#define AD_4350_REG_DATA_2_2200_4000		0x19005EC2
#define AD_4350_REG_DATA_1_2200_4000		0x08008321
#define AD_4350_REG_DATA_0_2200_4000		0x006E0000

//2380	10M 
#define AD_4350_REG_DATA_5_2380_1000		0x00580005
#define AD_4350_REG_DATA_4_2380_1000		0x008A003C
#define AD_4350_REG_DATA_3_2380_1000		0x000004B3
#define AD_4350_REG_DATA_2_2380_1000		0x16005EC2
#define AD_4350_REG_DATA_1_2380_1000		0x08008011
#define AD_4350_REG_DATA_0_2380_1000		0x003B8000

//2.400 25M
#define AD_4350_REG_DATA_5_2400_2500		0x00580005
#define AD_4350_REG_DATA_4_2400_2500		0x008C803C
#define AD_4350_REG_DATA_3_2400_2500		0x000004B3
#define AD_4350_REG_DATA_2_2400_2500		0x14005EC2
#define AD_4350_REG_DATA_1_2400_2500		0x08008321
#define AD_4350_REG_DATA_0_2400_2500		0x00300000

//2.400 10M
#define AD_4350_REG_DATA_5_2400_1000		0x00580005
#define AD_4350_REG_DATA_4_2400_1000		0x008A003C
#define AD_4350_REG_DATA_3_2400_1000		0x000004B3
#define AD_4350_REG_DATA_2_2400_1000		0x16005EC2
#define AD_4350_REG_DATA_1_2400_1000		0x08008011
#define AD_4350_REG_DATA_0_2400_1000		0x003C0000

//2.160 54M
#define AD_4350_REG_DATA_5_2160_5400		0x00580005
#define AD_4350_REG_DATA_4_2160_5400		0x009FF03C
#define AD_4350_REG_DATA_3_2160_5400		0x000004B3
#define AD_4350_REG_DATA_2_2160_5400		0x14005EC2
#define AD_4350_REG_DATA_1_2160_5400		0x08008361  
#define AD_4350_REG_DATA_0_2160_5400		0x00280000


//2.160 10M
#define AD_4350_REG_DATA_5_2160_1000		0x00580005
#define AD_4350_REG_DATA_4_2160_1000		0x0095003C
#define AD_4350_REG_DATA_3_2160_1000		0x000004B3
#define AD_4350_REG_DATA_2_2160_1000		0x14005EC2
#define AD_4350_REG_DATA_1_2160_1000		0x080080A1  
#define AD_4350_REG_DATA_0_2160_1000		0x00D80000

//2177280000 30.24M 验证OK
#define AD_4350_REG_DATA_5_2177_3024		0x00580005
#define AD_4350_REG_DATA_4_2177_3024		0x009F203C
#define AD_4350_REG_DATA_3_2177_3024		0x000004B3
#define AD_4350_REG_DATA_2_2177_3024		0x14005EC2
#define AD_4350_REG_DATA_1_2177_3024		0x08008011
#define AD_4350_REG_DATA_0_2177_3024		0x00480000


//2.400 40M 验证OK
#define AD_4350_REG_DATA_5_2400_4000		0x00580005
#define AD_4350_REG_DATA_4_2400_4000		0x008A003C
#define AD_4350_REG_DATA_3_2400_4000		0x000004B3
#define AD_4350_REG_DATA_2_2400_4000		0x19005EC2
#define AD_4350_REG_DATA_1_2400_4000		0x08008641
#define AD_4350_REG_DATA_0_2400_4000		0x003C0000


#ifdef USE_MAX287X_2200_40M

#ifdef AD_4350_REG_DATA_5_2200_4000

#undef AD_4350_REG_DATA_5_2200_4000
#undef AD_4350_REG_DATA_4_2200_4000
#undef AD_4350_REG_DATA_3_2200_4000
#undef AD_4350_REG_DATA_2_2200_4000
#undef AD_4350_REG_DATA_1_2200_4000
#undef AD_4350_REG_DATA_0_2200_4000

#endif

#define AD_4350_REG_DATA_5_2200_4000		0x00370000//???2871
#define AD_4350_REG_DATA_4_2200_4000		0x400103E9
#define AD_4350_REG_DATA_3_2200_4000		0x80005F42
#define AD_4350_REG_DATA_2_2200_4000		0x00001F23
#define AD_4350_REG_DATA_1_2200_4000		0x639200FC
#define AD_4350_REG_DATA_0_2200_4000		0x00400005

#endif

#ifdef USE_MAX287X_2200_10M

#ifdef AD_4350_REG_DATA_5_2200_1000

#undef AD_4350_REG_DATA_5_2200_1000
#undef AD_4350_REG_DATA_4_2200_1000
#undef AD_4350_REG_DATA_3_2200_1000
#undef AD_4350_REG_DATA_2_2200_1000
#undef AD_4350_REG_DATA_1_2200_1000
#undef AD_4350_REG_DATA_0_2200_1000

#endif

#define AD_4350_REG_DATA_5_2200_1000		0x00580005
#define AD_4350_REG_DATA_4_2200_1000		0x009A003C
#define AD_4350_REG_DATA_3_2200_1000		0x000004B3
#define AD_4350_REG_DATA_2_2200_1000		0x18009EC2
#define AD_4350_REG_DATA_1_2200_1000		0x08008011
#define AD_4350_REG_DATA_0_2200_1000		0x006E0000

#endif


#ifdef USE_MAX287X_2400_40M

#ifdef AD_4350_REG_DATA_5_2400_4000

#undef AD_4350_REG_DATA_5_2400_4000
#undef AD_4350_REG_DATA_4_2400_4000
#undef AD_4350_REG_DATA_3_2400_4000
#undef AD_4350_REG_DATA_2_2400_4000
#undef AD_4350_REG_DATA_1_2400_4000
#undef AD_4350_REG_DATA_0_2400_4000

#endif

#define AD_4350_REG_DATA_5_2400_4000		0x003C0000
#define AD_4350_REG_DATA_4_2400_4000		0x20010011
#define AD_4350_REG_DATA_3_2400_4000		0x80005F42
#define AD_4350_REG_DATA_2_2400_4000		0x00001F23
#define AD_4350_REG_DATA_1_2400_4000		0x639200FC
#define AD_4350_REG_DATA_0_2400_4000		0x00400005


#endif

#ifdef USE_MAX287X_2400_10M//参数是错误的！！！

#ifdef AD_4350_REG_DATA_5_2400_1000

#undef AD_4350_REG_DATA_5_2400_1000
#undef AD_4350_REG_DATA_4_2400_1000
#undef AD_4350_REG_DATA_3_2400_1000
#undef AD_4350_REG_DATA_2_2400_1000
#undef AD_4350_REG_DATA_1_2400_1000
#undef AD_4350_REG_DATA_0_2400_1000

#endif

#define AD_4350_REG_DATA_5_2400_1000		0x00580005
#define AD_4350_REG_DATA_4_2400_1000		0x009A003C
#define AD_4350_REG_DATA_3_2400_1000		0x000004B3
#define AD_4350_REG_DATA_2_2400_1000		0x18009EC2
#define AD_4350_REG_DATA_1_2400_1000		0x08008011
#define AD_4350_REG_DATA_0_2400_1000		0x006E0000

#endif




#define HWL_QAM_DTMB_FIXED_SYMBO_RATE		7560000 //KBoud
#define HWL_QAM_REGISTER_MAX 				250
#define HWL_QAM_MAX_GAIN_LEVEL				72
#define HWL_QAM_MAX_CUR_NODE_NUM			160


/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef struct
{
	U8						ModuleType;								/*调制模块类型：0，None；1，AD9789；2，BroadLogic*/
	HWL_ModulatorParam_t	pParam[HWL_CONST_MAX_QAM_CHN_NUM];
	U32						CurrentFDAC;
	S32						FrequeceCalibrate;						/*100MHz 时的频率偏移 ±1Hz*/
	BOOL					bUpperFreq;


	HWL_ModModuleResetCB	pResetCB;
	BOOL					bLevelOnly;

	BOOL					bDTMBInited;
	//BOOL					bDTMBFPGAParamNOChange;
	U8						p9789GainLevelValue[HWL_QAM_MAX_GAIN_LEVEL];

} HWL_Modulator;


typedef struct  
{
	U8		m_LAT;
	U8		m_SNC;
	U8		m_DSC;
}TEST_REG;

static TEST_REG s_DelayTest[] = 
{
	//{0, 7, 0},
	//{0, 8, 1},
	//{0, 8, 2},
	//{0, 9, 3},
	//{0, 9, 4},
	//{0, 2, 5},
	{1, 2, 6},
	{1, 3, 7},
	{1, 3, 8},
	{1, 4, 9},
	{1, 4, 10},
	{1, 5, 11},
	{1, 5, 12},
	{1, 6, 13},
	{1, 6, 14},
	{1, 7, 15},
	{1, 7, 0},
	{1, 8, 1},
	{1, 8, 2},
	{1, 9, 3},
	{1, 2, 5},
	{2, 2, 6}
};

/* Private Variables (static)-------------------------------------------------- */
/*寄存器缓存*/
static HWL_ModularChipRegister_t s_RegisterTable[HWL_QAM_REGISTER_MAX];
static S32 s_RegisterTableSize = 0;

/*DTMB频响曲线*/
static CAL_CURVE_NODE s_pFGCUVENode[HWL_QAM_MAX_CUR_NODE_NUM];
static S32 s_FGCUVENodeNum = 0;

/*DTMB频率准确度*/
static CAL_CURVE_NODE s_pFFCUVENode[HWL_QAM_MAX_CUR_NODE_NUM];
static S32 s_FFCUVENodeNum = 0;

/*MAIN HANDLE*/
static HWL_Modulator s_Modulator;

static S32	s_LastAD9789GainRegValue = 0;

#ifdef GQ3760B
static U32	s_HWType	= 2;
static BOOL	s_bVCO2781	= 0;  //1 : 2781  0: 4350
static S32	s_REFCLKHz	= HWL_QAM_VCO_REF_CLK_HZ_1000; //10M
#endif

/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

/*PQ值计算*/
static void HWLL_QAMCalculatePQN(U32 CurrentFDAC, U32 SymRate, U32 *pP, U32 *pQ, U32 *pN)
{
	U32 P, Q, lMoveCount, N;
	F64 lPQFactor;
	P = CurrentFDAC / 16;
	Q = SymRate;

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

		GLOBAL_TRACE(("FDAC = %u, Symbol = %d, P = 0x%.8X Q = 0x%.8X, P/Q = %f, N = %d, Move = %d\n", CurrentFDAC, SymRate, P, Q, lPQFactor, N, lMoveCount));
	}
	else
	{
		GLOBAL_TRACE(("Symbol Rate Over 135M [%d] ,P = Q = 0x800000 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", SymRate));
		P = Q = 0x800000;
	}

	(*pP) = P;
	(*pQ) = Q;
	(*pN) = N;
}

/*增益参数计算*/

U32 HWLL_QAMAD9789GainValueGetDVBS(S16 Level)
{
	return (Level - 10) * 3 + 80;
}


U32 HWLL_QAMAD9789GainValueGet(S16 Level, S16 QAMMode)
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


	lTotalRFOpend = 0;
	for (i = 0; i < HWL_CONST_MAX_QAM_CHN_NUM; i++)
	{
		if (s_Modulator.pParam[i].RFSwitch)
		{
			lTotalRFOpend++;
		}
	}


	switch (lTotalRFOpend)
	{
	case 1:
		lBaseLevel = HWL_QAM_MAX_GAIN_LEVEL;/* - (logf(lTotalRFOpend) * 10) * 4*/
		break;
	case 2:
		lBaseLevel = HWL_QAM_MAX_GAIN_LEVEL - 12/*3Db*/;
		break;
	case 3:
		lBaseLevel = HWL_QAM_MAX_GAIN_LEVEL - 19/*4.75Db*/;;
		break;
	case 4:
		lBaseLevel = HWL_QAM_MAX_GAIN_LEVEL - 24/*6Db*/;;
		break;
	default:
		lBaseLevel = HWL_QAM_MAX_GAIN_LEVEL - 26;
		break;
	}

	lLevel = lBaseLevel + (Level - 40) + ((QAMMode==HWL_CONST_MODULATOR_QAM_128)?10:0) - 10;

	if (GLOBAL_CHECK_INDEX(lLevel, HWL_QAM_MAX_GAIN_LEVEL))
	{
		lValue = s_Modulator.p9789GainLevelValue[lLevel];
	}
	else
	{
		if (lLevel >= HWL_QAM_MAX_GAIN_LEVEL)
		{
			lValue = s_Modulator.p9789GainLevelValue[HWL_QAM_MAX_GAIN_LEVEL - 1];
		}
		else
		{
			lValue = s_Modulator.p9789GainLevelValue[0];
		}
	}
	GLOBAL_TRACE(("lTotalRFOpend = %d, Value = %d\n", lTotalRFOpend, lValue));
#endif

	return lValue;
}


/*寄存器添加并发送*/
void HWLL_QAMRegisterAdd( BOOL bWrite, U8 ICID, U8 RegAddress, U8 RegData)
{
	if(s_RegisterTableSize >= HWL_QAM_REGISTER_MAX)
	{
		GLOBAL_TRACE(("Too Many Register CMD!!!\n"));
		return;
	}

	s_RegisterTable[s_RegisterTableSize].address = RegAddress;
	s_RegisterTable[s_RegisterTableSize].chipID = ICID;
	s_RegisterTable[s_RegisterTableSize].rOrw = !bWrite;
	s_RegisterTable[s_RegisterTableSize].value = RegData;
	s_RegisterTableSize++;
}

void HWLL_QAMRegisterApply()
{
	//发动到FPGA
	HWL_ModularChipTableSend(s_RegisterTable, s_RegisterTableSize);
	s_RegisterTableSize = 0;
}


/*2871 配置*/
static void HWLL_QAMADF2871Apply(U32 FDAC, U32 RefCLK)
{
	BOOL lRet = FALSE;
	PAL_MAX2871ConfigParam lConfig;
	GLOBAL_TRACE(("FDAC = %u, REF = %d \n", FDAC, RefCLK));

#ifdef GQ3710B
	lConfig.m_SPIConfig.m_SCLK = PFCGPIO_PB(15);
	lConfig.m_SPIConfig.m_SDO = PFCGPIO_PB(16);
	lConfig.m_SPIConfig.m_LE = PFCGPIO_PB(17);
	lConfig.m_LD = PFCGPIO_PB(14);
#endif

#if defined(arm)
	lConfig.m_SPIConfig.m_HalfCLKPeriodNS = 1;
#endif
	lConfig.m_MaxLockTimeoutMS = 1000;
	
	if(FDAC == HWL_QAM_VCO_FDAC_HZ_2400) //输出2.4Ghz
	{
		if(RefCLK == HWL_QAM_VCO_REF_CLK_HZ_2500)//25M
		{
			lConfig.m_Reg[0] = 0x80600000;
			lConfig.m_Reg[1] = 0x800103E9;
			lConfig.m_Reg[2] = 0x00001F42;
			lConfig.m_Reg[3] = 0x00001F23;
			lConfig.m_Reg[4] = 0x619F40FC;
			lConfig.m_Reg[5] = 0x00400005;
		}
		else
		{
			GLOBAL_TRACE(("FDAC = %u, REF = %d, Not Supported!\n", FDAC, RefCLK));
		}
	}
	else if(FDAC == HWL_QAM_VCO_FDAC_HZ_2200) //输出2.2Ghz
	{
		if(RefCLK == HWL_QAM_VCO_REF_CLK_HZ_2500)//25M
		{
			lConfig.m_Reg[0] = 0x80580000;
			lConfig.m_Reg[1] = 0x800103E9;
			lConfig.m_Reg[2] = 0x00001F42;
			lConfig.m_Reg[3] = 0x00001F23;
			lConfig.m_Reg[4] = 0x619F40FC;
			lConfig.m_Reg[5] = 0x00400005;
		}
		else
		{
			GLOBAL_TRACE(("FDAC = %u, REF = %d, Not Supported!\n", FDAC, RefCLK));
		}
	}
	else
	{
		GLOBAL_TRACE(("FDAC = %u, REF = %d, Not Supported!\n", FDAC, RefCLK));
	}

	lRet = PAL_MAX2871Config(&lConfig);

	GLOBAL_TRACE(("MAX2871 Lock = %d\n", lRet));
}

static void HWLL_QAMDAT31R5SP1Apply(S32 dBValue)
{
	BOOL lRet = FALSE;
	PAL_DAT31R5SPConfigParam lConfig;

	GLOBAL_TRACE(("dBValue = %d, dB = %.1f \n", dBValue, ((F64)dBValue) / 2));

#ifdef GQ3710B
	lConfig.m_SPIConfig.m_SCLK = PFCGPIO_PB(15);
	lConfig.m_SPIConfig.m_SDO = PFCGPIO_PB(16);
	lConfig.m_SPIConfig.m_LE = PFCGPIO_PB(13);
#endif

#if defined(arm)
	lConfig.m_SPIConfig.m_HalfCLKPeriodNS = 1;
#endif

	lConfig.m_dBValue = dBValue;

	lRet = PAL_DAT31R5SPConfig(&lConfig);
}

/*4350 配置*/
static void HWLL_QAMADF4350Apply(U32 FDAC, U32 ReferenceCLK)
{
	U8 lICID = ICPL_IIC_IC_ID_ADF4350;

	GLOBAL_TRACE(("Config 4350 FDAC = %.u Hz, RCLK = %u\n", FDAC, ReferenceCLK));

	if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2200) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_2500))
	{

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_2200_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2200_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2200_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2200_2500 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_2200_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2200_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2200_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2200_2500 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_2200_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2200_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2200_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2200_2500 >> 24) & 0xFF);


		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_2200_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2200_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2200_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2200_2500 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_2200_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2200_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2200_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2200_2500 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_2200_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2200_2500>> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2200_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2200_2500 >> 24) & 0xFF);
	}
	else if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2200) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_1000))
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_2200_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2200_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2200_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2200_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_2200_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2200_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2200_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2200_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_2200_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2200_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2200_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2200_1000 >> 24) & 0xFF);


		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_2200_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2200_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2200_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2200_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_2200_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2200_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2200_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2200_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_2200_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2200_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2200_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2200_1000 >> 24) & 0xFF);
	}
	else if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2200) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_4000))
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_2200_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2200_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2200_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2200_4000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_2200_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2200_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2200_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2200_4000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_2200_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2200_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2200_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2200_4000 >> 24) & 0xFF);


		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_2200_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2200_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2200_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2200_4000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_2200_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2200_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2200_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2200_4000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_2200_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2200_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2200_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2200_4000 >> 24) & 0xFF);
	}
	else if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2400) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_2500))
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_2400_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2400_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2400_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2400_2500 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_2400_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2400_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2400_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2400_2500 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_2400_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2400_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2400_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2400_2500 >> 24) & 0xFF);


		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_2400_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2400_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2400_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2400_2500 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_2400_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2400_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2400_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2400_2500 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_2400_2500 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2400_2500 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2400_2500 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2400_2500 >> 24) & 0xFF);
	}
	else if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2400) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_1000))
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_2400_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2400_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2400_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2400_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_2400_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2400_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2400_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2400_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_2400_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2400_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2400_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2400_1000 >> 24) & 0xFF);


		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_2400_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2400_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2400_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2400_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_2400_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2400_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2400_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2400_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_2400_1000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2400_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2400_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2400_1000 >> 24) & 0xFF);
	}
	else if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2400) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_4000))
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_2400_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2400_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2400_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2400_4000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_2400_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2400_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2400_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2400_4000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_2400_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2400_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2400_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2400_4000 >> 24) & 0xFF);


		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_2400_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2400_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2400_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2400_4000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_2400_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2400_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2400_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2400_4000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_2400_4000 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2400_4000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2400_4000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2400_4000 >> 24) & 0xFF);
	}
	else if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2177) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_3024))
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_5_2177_3024 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2177_3024 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2177_3024 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2177_3024 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_4_2177_3024 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2177_3024 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2177_3024 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2177_3024 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_3_2177_3024 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2177_3024 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2177_3024 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2177_3024 >> 24) & 0xFF);


		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_2_2177_3024 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2177_3024 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2177_3024 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2177_3024 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_1_2177_3024 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2177_3024 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2177_3024 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2177_3024 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, AD_4350_REG_DATA_0_2177_3024 & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2177_3024 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2177_3024 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2177_3024 >> 24) & 0xFF);
	}
	else if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2160) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_5400))
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_5_2160_5400 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2160_5400 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2160_5400 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2160_5400 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_4_2160_5400 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2160_5400 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2160_5400 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2160_5400 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_3_2160_5400 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2160_5400 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2160_5400 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2160_5400 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_2_2160_5400 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2160_5400 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2160_5400 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2160_5400 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_1_2160_5400 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2160_5400 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2160_5400 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2160_5400 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_0_2160_5400 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2160_5400 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2160_5400 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2160_5400 >> 24) & 0xFF);
	}
	else if ((FDAC == HWL_QAM_VCO_FDAC_HZ_2380) && (ReferenceCLK == HWL_QAM_VCO_REF_CLK_HZ_1000))
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_5_2380_1000 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_5_2380_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_5_2380_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_5_2380_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_4_2380_1000 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_4_2380_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_4_2380_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_4_2380_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_3_2380_1000 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_3_2380_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_3_2380_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_3_2380_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_2_2380_1000 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_2_2380_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_2_2380_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_2_2380_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_1_2380_1000 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_1_2380_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_1_2380_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_1_2380_1000 >> 24) & 0xFF);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, (AD_4350_REG_DATA_0_2380_1000 >> 0) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x01, (AD_4350_REG_DATA_0_2380_1000 >> 8) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x02, (AD_4350_REG_DATA_0_2380_1000 >> 16) & 0xFF);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, (AD_4350_REG_DATA_0_2380_1000 >> 24) & 0xFF);
	}
	else
	{
		GLOBAL_TRACE(("Unknow Combination FDAC = %lu, Reference CLK = %lu\n", FDAC, ReferenceCLK));
	}


	HWLL_QAMRegisterApply();
	PFC_TaskSleep(200);
}


/*AD9789固定寄存器设置*/
void  HWLL_QAMAD9789RestAndInitiate(void)
{
	S16 lModuleIndex;
	U8 lICID;
	HWL_ModulatorParam_t *plSubModulatorParam;
	for (lModuleIndex = 0; lModuleIndex < HWL_AD9789_MODULE_NUM; lModuleIndex++)
	{
		lICID = ICPL_IIC_IC_ID_AD9789_START +  lModuleIndex;

		plSubModulatorParam =  &s_Modulator.pParam[lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];//每个AD9789的第一个通道的参数

		/*设置AD9789内部寄存器*/
		GLOBAL_TRACE(("Config AD9789 FIX REG\n"));
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, 0x3C);//复位
		HWLL_QAMRegisterApply();

		PFC_TaskSleep(1);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x00, 0x18);//复位结束
		HWLL_QAMRegisterApply();


		HWLL_QAMRegisterAdd(TRUE, lICID, 0x30, 0x80);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x31, 0xf0);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x32, 0x1E);//关闭时钟接收
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x3E, 0x38);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x0F);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x2f, 0x4d);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x33, 0x42);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x39, 0x4e);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x3a, 0x64);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, 0x00);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x04, 0xfe);
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x03, 0xfe);
		HWLL_QAMRegisterApply();

		PFC_TaskSleep(10);

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x33, 0x43);
		HWLL_QAMRegisterApply();

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x05, 0);//关闭RF输出
		HWLL_QAMRegisterApply();

		HWLL_QAMRegisterAdd(TRUE, lICID, 0x32, 0x9E);//打开时钟接收
		HWLL_QAMRegisterApply();
	}

	PFC_TaskSleep(200);

}



/*内部参数初始化*/
void HWLL_QAMParameterDefault(S16 TsIndex)
{
	HWL_ModulatorParam_t *plParam;

	if ((TsIndex >= 0) && (TsIndex < HWL_CONST_MAX_QAM_CHN_NUM))
	{
		//printf("[HWL_SetParamDefault] Set Ts %d Default \n", TsIndex);

		plParam = &s_Modulator.pParam[TsIndex];
		if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QAM)
		{
			plParam->ModulateStandard = HWL_CONST_MODULATOR_STANDARD_ANNEX_A;

			plParam->ModulateFreq = 474000 + (TsIndex * 8000);

			plParam->QamMode = HWL_CONST_MODULATOR_QAM_64;
			plParam->SymRate = 6875;
			plParam->Gain = 10;
			plParam->Attenuator = 10;
			plParam->SpectrumInvert = 0;
			plParam->QamSwitch = 1;
			plParam->RFSwitch = 1;

			GLOBAL_TRACE(("QAM Default Parameter!\n"));
		}
		else if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_H || s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_L)
		{
			plParam->ModulateStandard = HWL_CONST_MODULATOR_STANDARD_ANNEX_A;

			plParam->ModulateFreq = 950000;

			plParam->QamMode = HWL_CONST_MODULATOR_QAM_QPSK;
			plParam->SymRate = 27500;
			plParam->Gain = 10;
			plParam->Attenuator = 10;
			plParam->SpectrumInvert = 0;
			plParam->QamSwitch = 1;
			plParam->RFSwitch = 1;
			plParam->QPSK_FEC = HWL_CONST_MODULATOR_FEC_7_OF_8;

			GLOBAL_TRACE(("QPSK Default Parameter!\n"));
		}
		else if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_DTMB)
		{
			plParam->ModulateStandard = 0;

			plParam->ModulateFreq = 474000;

			plParam->QamMode = HWL_CONST_MODULATOR_QAM_64;
			plParam->SymRate = HWL_QAM_DTMB_FIXED_SYMBO_RATE;//设置给9789的DTMB符号率
			plParam->Gain = 10;
			plParam->Attenuator = 10;
			plParam->SpectrumInvert = 0;
			plParam->QamSwitch = 1;
			plParam->RFSwitch = 1;

			plParam->DTMB_Carrier = HWL_MODULATOR_DTMB_CARRIER_MULTI;
			plParam->DTMB_CodeRate = HWL_MODULATOR_DTMB_CodeRate_06;
			plParam->DTMB_PN = GS_MODULATOR_GUARD_INTERVAL_PN_420C;
			plParam->DTMB_TI = HWL_MODULATOR_DTMB_TI_720;

			GLOBAL_TRACE(("DTMB Default Parameter!\n"));
		}

	}
	else
	{
		if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QAM)
		{
			S32 i;
			for (i = 0; i < HWL_CONST_MAX_QAM_CHN_NUM; i++)
			{
				HWLL_QAMParameterDefault(i);
			}
		}
		else
		{
			HWLL_QAMParameterDefault(0);
		}

	}
}

/*初始化频响列表*/
void HWLL_QAMInitFreqGainResponse(void)
{
#if defined(GQ3760A) || defined(LR1800S) || defined(GQ3760)
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


	}
#elif defined(GQ3760B)
{
#ifdef SUPPORT_EXT_DPD_MODULE
	{
		S32 i;
		/*初始化曲线拟合节点*/
		s_FGCUVENodeNum = 0;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 52.5 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x90;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 466 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x21;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 474 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 482 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 490 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 498 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 506 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 514 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 522 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 530 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 538 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 546 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 554 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 562 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 610 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x22;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 618 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x22;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 626 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x22;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 634 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x22;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 642 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x22;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 650 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 658 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 666 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 674 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 682 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 690 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x28;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 698 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2A;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 706 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 714 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2C;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 722 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2C;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 730 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2D;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 738 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2E;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 746 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2F;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 754 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x30;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 762 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x32;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 770 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x33;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 778 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x33;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 786 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x34;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 794 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x35;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 802 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x36;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 810 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x37;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 818 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x39;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 826 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3B;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 834 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3C;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 842 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3E;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 850 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3F;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 858 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x42;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 866 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x44;//
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 898 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4C;//
		s_FGCUVENodeNum ++;







	}

#else
	{
		S32 i;
		/*初始化曲线拟合节点*/
		s_FGCUVENodeNum = 0;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 30 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 50 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x1A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 100 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x20;
		s_FGCUVENodeNum ++;


		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 200 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x21;
		s_FGCUVENodeNum ++;


		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 300 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 400 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 482 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 500 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x27;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 506 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x28;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 514 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x28;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 522 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x27;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 530 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 554 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 570 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 578 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 586 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 600 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 626 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 650 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 658 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 666 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 674 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 682 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 690 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2E;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 700 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x30;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 714 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x31;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 722 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x31;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 730 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x32;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 738 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x35;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 746 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x37;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 754 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x38;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 762 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 770 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 778 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 786 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x44;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 794 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x45;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 800 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x47;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 802 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x47;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 810 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x49;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 818 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 826 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x51;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 834 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x55;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 842 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x57;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 850 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x58;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 858 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 900 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x6A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 1000 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x84;
		s_FGCUVENodeNum ++;
	}

#endif
}
#elif defined(GQ3763) || defined(GQ3768)//目标带DPD的时候是 1dBm
#ifdef SUPPORT_EXT_DPD_MODULE
	{
		S32 i;
		/*初始化曲线拟合节点*/
		s_FGCUVENodeNum = 0;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 80 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0xF2;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 90 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x84;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 100 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 200 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4F;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 250 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 300 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x56;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 350 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5E;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 400 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4E;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 420 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 440 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x34;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 450 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x34;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 460 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x32;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 480 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2F;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 482 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x30;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 490 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x32;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 498 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x33;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 506 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x30;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 514 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 520 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 540 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x29;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 550 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x28;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 560 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 570 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x23;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 580 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 586 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 600 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x28;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 620 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2F;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 640 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x36;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 660 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 690 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x44;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 700 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x44;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 714 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x45;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 720 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x47;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 740 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x52;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 760 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5E;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 770 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x62;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 780 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x64;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 800 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x6E;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 820 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x70;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 832 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x72;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 840 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x79;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 860 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x88;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 872 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x90;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 880 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x99;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 900 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0xB0;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 904 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0xB4;
		s_FGCUVENodeNum ++;
	}

#else
	{
		S32 i;
		/*初始化曲线拟合节点*/
		s_FGCUVENodeNum = 0;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 30 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 50 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x1A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 100 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x20;
		s_FGCUVENodeNum ++;


		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 200 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x21;
		s_FGCUVENodeNum ++;


		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 300 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 400 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 482 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 500 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x27;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 506 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x28;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 514 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x28;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 522 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x27;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 530 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 554 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 570 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x25;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 578 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 586 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x24;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 600 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 626 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x26;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 650 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 658 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 666 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 674 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 682 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2C;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 690 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x2E;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 700 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x30;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 714 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x31;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 722 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x31;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 730 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x32;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 738 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x35;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 746 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x37;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 754 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x38;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 762 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 770 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 778 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x3D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 786 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x44;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 794 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x45;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 800 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x47;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 802 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x47;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 810 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x49;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 818 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x4D;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 826 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x51;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 834 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x55;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 842 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x57;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 850 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x58;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 858 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x5B;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 900 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x6A;
		s_FGCUVENodeNum ++;

		s_pFGCUVENode[s_FGCUVENodeNum].m_CompareValue = 1000 * 1000 * 1000;//Hz
		s_pFGCUVENode[s_FGCUVENodeNum].m_SetValue = 0x84;
		s_FGCUVENodeNum ++;
	}

#endif

#endif

		//for (i = 0; i < s_FGCUVENodeNum; i++)
		//{
		//	GLOBAL_TRACE(("Freq = %d Hz, SetValue = 0x%.2X\n", s_pFGCUVENode[i].m_CompareValue, s_pFGCUVENode[i].m_SetValue));
		//}
}


//typedef struct  
//{
//	S32			m_FreqMHz;//频率
//	S32			m_FreqOffsetValue;//AD9789频率设置偏移
//	S32			m_FreqAdjIntValue;//FPGA NCO 频率偏移
//}HWL_FF_NODE;

//static HWL_FF_NODE	s_FreqAdjNode[120];
//
//static s_FreqAdjNodeCurNum = 0;
/*初始化频率矫正列表*/
void HWLL_QAMInitFreqAccuracy(void)
{
	S32 i;
	/*初始化曲线拟合节点*/
	s_FFCUVENodeNum = 0;

	s_pFFCUVENode[s_FFCUVENodeNum].m_CompareValue = 34000000;//Hz
	s_pFFCUVENode[s_FFCUVENodeNum].m_SetValue = 13;
	s_FFCUVENodeNum ++;

	s_pFFCUVENode[s_FFCUVENodeNum].m_CompareValue = 442000000;//Hz
	s_pFFCUVENode[s_FFCUVENodeNum].m_SetValue = 25;
	s_FFCUVENodeNum ++;

	s_pFFCUVENode[s_FFCUVENodeNum].m_CompareValue = 874000000;//Hz
	s_pFFCUVENode[s_FFCUVENodeNum].m_SetValue = 55;
	s_FFCUVENodeNum ++;

}
/* API 函数--------------------------------------------------------------------------------------------------------*/
/*初始化*/
void HWL_QAMInitialize(U8 ModulatorType)
{
	GLOBAL_ZEROMEM(&s_Modulator, sizeof(s_Modulator));

	s_Modulator.ModuleType = ModulatorType;

	GLOBAL_TRACE(("Modulator Type = %d\n", ModulatorType));
	//设置为默认值.
	HWLL_QAMParameterDefault(-1);	

	//必须预先配置一次4350 否则有不正常的机子！

#ifndef MULT_USE_CHEAP_VCO_REFERENCE
	#if defined(GQ3763) || defined(GQ3768)
		#if defined(USE_MAX287X_2200_40M)
			HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2200, HWL_QAM_VCO_REF_CLK_HZ_4000);
		#elif defined(USE_MAX287X_2400_10M)
			HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2400, HWL_QAM_VCO_REF_CLK_HZ_1000);
		#elif defined(USE_MAX287X_2400_40M)
			HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2400, HWL_QAM_VCO_REF_CLK_HZ_4000);
		#else
			HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2400, HWL_QAM_VCO_REF_CLK_HZ_1000);
		#endif
	#elif defined(LR1800S) || defined(GQ3760A)
		HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2177, HWL_QAM_VCO_REF_CLK_HZ_3024);
	#elif defined(GQ3760)
		#ifdef MULT_QAM_NET_SETTING_USE_VOC_REFERENCE_FREQ_30_24
			HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2177, HWL_QAM_VCO_REF_CLK_HZ_3024);
		#else
			HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2200, HWL_QAM_VCO_REF_CLK_HZ_1000);
		#endif
	#elif defined(GQ3760B)
		HWLL_QAMADF2871Apply(HWL_QAM_VCO_FDAC_HZ_2400, s_REFCLKHz);	
	#elif defined(GQ3710B)
		HWLL_QAMADF2871Apply(HWL_QAM_VCO_FDAC_HZ_2400, HWL_QAM_VCO_REF_CLK_HZ_2500);	
	#else
		HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2200, HWL_QAM_VCO_REF_CLK_HZ_2500);
	#endif
#else
	HWLL_QAMADF4350Apply(HWL_QAM_VCO_FDAC_HZ_2200, HWL_QAM_VCO_REF_CLK_HZ_2500);
#endif


	/*生成AD9789 GAIN Db->BIN对应表*/
	{
		S32 i;
		for (i = 0; i < HWL_QAM_MAX_GAIN_LEVEL; i++)
		{
			if (i == 0)
			{
				s_Modulator.p9789GainLevelValue[i] = 32 + 6;
			}
			else if (i < 6)
			{
				s_Modulator.p9789GainLevelValue[i] = 1 + s_Modulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 16 ) 
			{
				s_Modulator.p9789GainLevelValue[i] = 2 + s_Modulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 24) 
			{
				s_Modulator.p9789GainLevelValue[i] = 2 + s_Modulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 38)
			{
				s_Modulator.p9789GainLevelValue[i] = 3 + s_Modulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 55)
			{
				s_Modulator.p9789GainLevelValue[i] = 4 + s_Modulator.p9789GainLevelValue[i - 1];
			}
			else if (i < 62)
			{
				s_Modulator.p9789GainLevelValue[i] = 7 + s_Modulator.p9789GainLevelValue[i - 1];
			}			
			else if (i < HWL_QAM_MAX_GAIN_LEVEL)
			{
				if ((3 + s_Modulator.p9789GainLevelValue[i - 1]) < 255)
				{
					s_Modulator.p9789GainLevelValue[i] = (3 + s_Modulator.p9789GainLevelValue[i - 1]);
				}
				else
				{
					s_Modulator.p9789GainLevelValue[i] = 255;
				}
			}
		}	
	}

	HWLL_QAMInitFreqGainResponse();

	HWLL_QAMInitFreqAccuracy();

	s_Modulator.bDTMBInited = FALSE;
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
	plInfo = &s_Modulator.pParam[ChnInd];

	s_Modulator.bLevelOnly &= (plInfo->ModulateStandard == pParam->ModulateStandard);
	s_Modulator.bLevelOnly &= (plInfo->ModulateFreq / 1000 == pParam->ModulateFreq);
	s_Modulator.bLevelOnly &= (plInfo->SymRate / 1000 == pParam->SymRate);
	s_Modulator.bLevelOnly &= (plInfo->QamMode == pParam->QamMode);
	s_Modulator.bLevelOnly &= (plInfo->SpectrumInvert == pParam->SpectrumInvert);
	s_Modulator.bLevelOnly &= (plInfo->QamSwitch == pParam->QamSwitch);
	s_Modulator.bLevelOnly &= (plInfo->RFSwitch == pParam->RFSwitch);
	s_Modulator.bLevelOnly &= (plInfo->QPSK_FEC == pParam->QPSK_FEC);


	GLOBAL_MEMCPY(plInfo, pParam, sizeof(HWL_ModulatorParam_t));

	plInfo->ModulateFreq = plInfo->ModulateFreq * 1000;
	plInfo->SymRate = plInfo->SymRate * 1000;

	return TRUE;
}

BOOL HWL_QAMParameterSetNew(S16 ChnInd, HWL_ModulatorParam_t *pParam, HWL_ModModuleResetCB pResetCB)
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
	plInfo = &s_Modulator.pParam[ChnInd];

	/*调制开关变化时，默认需要重新设置所有寄存器*/
	s_Modulator.bDTMBInited &= (plInfo->QamSwitch == pParam->QamSwitch);

	s_Modulator.bLevelOnly &= (plInfo->QamSwitch == pParam->QamSwitch);
	s_Modulator.bLevelOnly &= (plInfo->ModulateFreq == pParam->ModulateFreq);
	s_Modulator.bLevelOnly &= (plInfo->SpectrumInvert == pParam->SpectrumInvert);

	if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QAM)
	{
		s_Modulator.bLevelOnly &= (plInfo->ModulateStandard == pParam->ModulateStandard);
		s_Modulator.bLevelOnly &= (plInfo->QamMode == pParam->QamMode);
		s_Modulator.bLevelOnly &= (plInfo->SymRate == pParam->SymRate);
	}
	else if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_L || s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_H)
	{
		s_Modulator.bLevelOnly &= (plInfo->QPSK_FEC == pParam->QPSK_FEC);
	}
	else
	{
		s_Modulator.bLevelOnly &= (plInfo->QamMode == pParam->QamMode);
		s_Modulator.bLevelOnly &= (plInfo->DTMB_Carrier == pParam->DTMB_Carrier);
		s_Modulator.bLevelOnly &= (plInfo->DTMB_CodeRate == pParam->DTMB_CodeRate);
		s_Modulator.bLevelOnly &= (plInfo->DTMB_DoublePilot == pParam->DTMB_DoublePilot);
		s_Modulator.bLevelOnly &= (plInfo->DTMB_PN == pParam->DTMB_PN);
		s_Modulator.bLevelOnly &= (plInfo->DTMB_TI == pParam->DTMB_TI);

	}




#ifdef DEBUG_MODULATOR
	s_Modulator.bLevelOnly = FALSE;
	s_Modulator.bDTMBInited = FALSE;
#endif

	GLOBAL_TRACE(("s_Modulator.bLevelOnly = %d, bDTMBInited = %d\n", s_Modulator.bLevelOnly, s_Modulator.bDTMBInited));


	GLOBAL_MEMCPY(plInfo, pParam, sizeof(HWL_ModulatorParam_t));

	s_Modulator.pResetCB = pResetCB;

	return TRUE;
}


void HWL_QAMGetCurrentParam(S16 ChnInd, HWL_ModulatorParam_t *pParam)
{
	HWL_ModulatorParam_t *plInfo;

	if(pParam == NULL)
	{
		return;
	}

	if( ChnInd < 0 || ChnInd >= HWL_CONST_MAX_QAM_CHN_NUM)
	{
		GLOBAL_TRACE(("Chn Index = %d\n", ChnInd));
		return;
	}
	plInfo = &s_Modulator.pParam[ChnInd];
	GLOBAL_MEMCPY(pParam, plInfo, sizeof(HWL_ModulatorParam_t));
}


void HWL_QAMForceOutputProgressChange(S32 lNewValue)
{
#ifdef SUPPORT_SMOOTH_RF_OUTPUT_POWER_CHANGE
	U8 lICID;
	lICID = ICPL_IIC_IC_ID_AD9789_START;
	S32 lPowInd, lOffset;

	if (lNewValue == 0)
	{
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x25,  lNewValue);
		HWLL_QAMRegisterApply();
		s_LastAD9789GainRegValue = lNewValue;
	}
	else
	{
		if (s_LastAD9789GainRegValue > lNewValue)
		{
			lOffset = -1;
		}
		else if (s_LastAD9789GainRegValue < lNewValue)
		{
			lOffset = 1;
		}
		else
		{
			lOffset = 0;
		}

		if (lOffset != 0)
		{
			GLOBAL_TRACE(("Start Output Level Adj From %d To %d Offset = %d!!\n", s_LastAD9789GainRegValue, lNewValue, lOffset));

			for (lPowInd = s_LastAD9789GainRegValue + lOffset; lPowInd != lNewValue; lPowInd += lOffset)
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x25,  lPowInd);
				HWLL_QAMRegisterApply();
				if (lPowInd < 10)
				{
					PFC_TaskSleep(200);
				}
				else if (lPowInd < 20)
				{
					PFC_TaskSleep(500);
				}
				else if (lPowInd < 50)
				{
					PFC_TaskSleep(20);
				}
				else
				{
					//PFC_TaskSleep(5);
				}
			}
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x25,  lNewValue);
			HWLL_QAMRegisterApply();

			s_LastAD9789GainRegValue = lNewValue;

			GLOBAL_TRACE(("End Output Level Adj!!\n"));
		}
	}

#endif

}

void HWL_QAMForceNotLevelOnly(void)
{
	s_Modulator.bLevelOnly = FALSE;
}


void HWLL_QAMDTMBSetDTMBModeToFPGA(HWL_ModulatorParam_t *plModulatorParam)
{
	U32 lRegData32;
	S32 lPNFactor;
	S32 lCarrirMode;
	S32 lSFactor;
	S32 lDoublePilot;

	//发送给FPGA的命令，配置调试相关参数
	GLOBAL_TRACE(("DTMB Parameter: QAM = %d, PN = %d, CodeRate = %d, Carrier = %d, DP = %d, INT = %d\n", plModulatorParam->QamMode, plModulatorParam->DTMB_PN, plModulatorParam->DTMB_CodeRate, plModulatorParam->DTMB_Carrier, plModulatorParam->DTMB_DoublePilot, plModulatorParam->DTMB_TI));

	switch (plModulatorParam->DTMB_PN)
	{
	case GS_MODULATOR_GUARD_INTERVAL_PN_420C:
		lPNFactor = 0;
		break;
	case GS_MODULATOR_GUARD_INTERVAL_PN_595:
		lPNFactor = 1 + 4;
		break;
	case GS_MODULATOR_GUARD_INTERVAL_PN_945C:
		lPNFactor = 2;
		break;
	case GS_MODULATOR_GUARD_INTERVAL_PN_420F:
		lPNFactor = 0 + 4;
		break;
	case GS_MODULATOR_GUARD_INTERVAL_PN_945F:
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
	//GLOBAL_TRACE(("Reg0x00 = %.2X\n", lRegData32));
	HWLL_QAMRegisterAdd(TRUE, ICPL_IIC_IC_ID_FPGA_DTMB, 0x00, (lRegData32 & 0xFF));
	lRegData32 = ((lDoublePilot << 1) & 0x02) | (lCarrirMode & 0x01);
	//GLOBAL_TRACE(("Reg0x01 = %.2X\n", lRegData32));
	HWLL_QAMRegisterAdd(TRUE, ICPL_IIC_IC_ID_FPGA_DTMB, 0x01, (lRegData32 & 0xFF));
	HWLL_QAMRegisterApply();
}

/*应用参数*/
S32 HWL_QAMDTMBApplyFPGAParameters(BOOL bTone)
{
	HWLL_QAMDTMBSetDTMBModeToFPGA(&s_Modulator.pParam[0]);
	HWLL_QAMRegisterAdd(TRUE, ICPL_IIC_IC_ID_AD9789_START, 0x81, 0);
	HWLL_QAMRegisterAdd(TRUE, ICPL_IIC_IC_ID_AD9789_START, 0x82, bTone?1:0);
	HWLL_QAMRegisterApply();

	s_Modulator.bDTMBInited = TRUE;
	s_Modulator.bLevelOnly = TRUE;//恢复标志以待下一次设定
}

BOOL HWL_QAMParameterCheckNeedResetRF(void)
{
	return (s_Modulator.bLevelOnly == FALSE);
}

S32 HWL_QAMParameterApply(S16 TsIndex)
{

	if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QAM)
	{
		U8 lSubChannelEnable;
		U32 lRegData32;
		S16 lICID, lModuleIndex, lSubChannelIndex;
		S32 lCentralFreq, lTmpFreq;
		U32 lFDAC, lRCLK;
		F64 lFTW ,lFTWBPF;
		HWL_ModulatorParam_t *plModulatorParam, *plSubModulatorParam;

		lFDAC = HWL_QAM_VCO_FDAC_HZ_2200;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_2500;

		lFTW = (F64)(0xFFFFFF) / ((F64)(lFDAC + s_Modulator.FrequeceCalibrate) / 16);//中心频率计算
		lFTWBPF = (F64)0xFFFF / ((F64)(lFDAC + s_Modulator.FrequeceCalibrate));//BPF频率计算

		GLOBAL_TRACE(("Start Set AD9789 for DVB-C Parameter\n"));

		if (s_Modulator.bLevelOnly == FALSE)
		{
			GLOBAL_TRACE(("Reset All!\n"));

			HWLL_QAMAD9789RestAndInitiate();

#ifdef USE_MAX2871
			HWLL_QAMADF2871Apply(lFDAC, lRCLK);
#else
			HWLL_QAMADF4350Apply(lFDAC, lRCLK);
#endif

			plModulatorParam = &s_Modulator.pParam[0];

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
				plModulatorParam =  &s_Modulator.pParam[lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];//每个AD9789的第一个通道的参数

				/*设置FPGA QAM调制开关*/
				lRegData32 = 0;
				for (lSubChannelIndex = 0; lSubChannelIndex < HWL_AD9789_MODULE_SUB_CHENNEL_NUM; lSubChannelIndex++)
				{
					plSubModulatorParam = &s_Modulator.pParam[lSubChannelIndex + lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];
					if (plSubModulatorParam->QamSwitch)
					{
						lRegData32 +=  1 << ( lSubChannelIndex + 4);//设置通道开关
					}
				}

				//FPGA QAM调制方式寄存器值
				lRegData32 += plModulatorParam->QamMode & 0x0F;//QAM调制方式寄存器值
				HWLL_QAMRegisterAdd(TRUE, lICID, ICPL_IIC_REG_AD9789_QAM_SP_INFO, (lRegData32 & 0xFF));
				HWLL_QAMRegisterApply();
			}

			/*计算32M带通滤波器的中心频点*/


			for (lModuleIndex = 0; lModuleIndex < HWL_AD9789_MODULE_NUM; lModuleIndex++)
			{
				/*参数预计算*/
				lICID = ICPL_IIC_IC_ID_AD9789_START +  lModuleIndex;
				plModulatorParam =  &s_Modulator.pParam[lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];//每个AD9789的第一个通道的参数

				//设置QAM和SRRC寄存器
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

				/*
				AD9789可设置的滚降系统
				00 0.12
				01 0.18
				10 0.15
				11 0.13
				*/
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x07, (lRegData32 & 0xFF));

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x08, 0x14);


				/*设置子通道频率，并得到子通道RF开关状态*/
				lSubChannelEnable = 0;
				for (lSubChannelIndex = 0; lSubChannelIndex < HWL_AD9789_MODULE_SUB_CHENNEL_NUM; lSubChannelIndex++)
				{
					plSubModulatorParam = &s_Modulator.pParam[lSubChannelIndex + lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];
					//通道频率设置 单位KHz
					lRegData32 = lFTW * plSubModulatorParam->ModulateFreq;//新程序修改成了HZ级别
					//GLOBAL_TRACE(("-----------------------------------------RegFreq = %.8X--------------------------------------\n", lRegData32));
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x0C + (lSubChannelIndex * 3), ((lRegData32 >> 16) & 0xFF));
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x0B + (lSubChannelIndex * 3), ((lRegData32 >> 8) & 0xFF));
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x0A + (lSubChannelIndex * 3), (lRegData32 & 0xFF));

					if (plSubModulatorParam->RFSwitch)
					{
						lSubChannelEnable += (1 << lSubChannelIndex);
					}

				}

				/*计算并设置速率转换分子和分母*/
				{
					U32 P, Q, N;

					HWLL_QAMCalculatePQN(lFDAC + s_Modulator.FrequeceCalibrate, plModulatorParam->SymRate, &P, &Q, &N);

					HWLL_QAMRegisterAdd(TRUE, lICID, 0x06, 0x10);

					lRegData32 = Q;

					HWLL_QAMRegisterAdd(TRUE, lICID, 0x18, ((lRegData32 >> 16) & 0xFF));
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x17, ((lRegData32 >> 8) & 0xFF));
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x16, (lRegData32 & 0xFF));

					lRegData32 = P ;
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x1B, ((lRegData32 >> 16) & 0xFF));
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x1A, ((lRegData32 >> 8) & 0xFF));
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x19, (lRegData32 & 0xFF));
				}


				/*通过计算4个子频率的频率得到带宽滤波器的中心频率*/
				{
					S32 lLowFreq, lHighFreq;

					lHighFreq = 0;
					lLowFreq = GLOBAL_S32_MAX;
					for (lSubChannelIndex = 0; lSubChannelIndex < HWL_AD9789_MODULE_SUB_CHENNEL_NUM; lSubChannelIndex++)
					{
						plSubModulatorParam = &s_Modulator.pParam[lSubChannelIndex + lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];

						if (plSubModulatorParam->ModulateFreq < lLowFreq)
						{
							lLowFreq = plSubModulatorParam->ModulateFreq;
						}

						if (plSubModulatorParam->ModulateFreq > lHighFreq)
						{
							lHighFreq = plSubModulatorParam->ModulateFreq;
						}
					}

					lCentralFreq = (lHighFreq + lLowFreq) / 2;

					GLOBAL_TRACE(("BPF Center Freq = %d Hz, High = %d Hz, Low = %d Hz\n", lCentralFreq, lHighFreq, lLowFreq));
					lRegData32 = lCentralFreq * lFTWBPF;


					HWLL_QAMRegisterAdd(TRUE, lICID, 0x1D, ((lRegData32 >> 8) & 0xFF));
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x1C, ((lRegData32 & 0xFF)));
				}

				/*设置FCLK和FDAC的分频关系*/
#if 1//32分频
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x20, 0xD4);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x21, 0xA3);

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x0F);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x23, 0x88);

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x2F);
#else	//16分频
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x20, 0xc4);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x21, 0xe2); 
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x0f); 
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x23, 0x98);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x1f);
#endif

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00); //使得0x22-0x23的寄存器设置有效
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);

				/*频谱翻转*/
				if(plModulatorParam->SpectrumInvert)
				{
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x29, 0x01);
				}
				else
				{
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x29, 0x00);
				}

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x38, 0x00);

				lTmpFreq = lCentralFreq / 50;
				if( lTmpFreq > 10000000)
				{
					lRegData32 = (lTmpFreq - 5400000) * 4 / 46;
				}
				else
				{
					lRegData32 = (lTmpFreq + 13500000) * 4 / 235;

				}

				if((lCentralFreq > 440000000) & (lCentralFreq < 560000000))
				{
					lRegData32 = lRegData32 + 130;
				}

				if(lCentralFreq > 760000000)
				{
					lRegData32 = 0x03FF;
				}

				if(lRegData32 > 0x03FF)
				{
					lRegData32 = 0x03FF;
				}
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x3D, ((lRegData32 >> 8) & 0x03));
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x3C, (lRegData32 & 0xFF));

				HWLL_QAMRegisterApply();

				PFC_TaskSleep(100);

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x1E, 0x80);

				/*子通道开关*/
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x05, (lSubChannelEnable & 0x0F));
				HWLL_QAMRegisterApply();
			}
		}


		GLOBAL_TRACE(("Set Gain\n"));
		/*通道增益设置*/
		{
			plModulatorParam = &s_Modulator.pParam[0];
			for (lModuleIndex = 0; lModuleIndex < HWL_AD9789_MODULE_NUM; lModuleIndex++)
			{
				/*参数预计算*/
				lICID = ICPL_IIC_IC_ID_AD9789_START +  lModuleIndex;
				plModulatorParam =  &s_Modulator.pParam[lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];//每个AD9789的第一个通道的参数

				lSubChannelEnable = 0;
				for (lSubChannelIndex = 0; lSubChannelIndex < HWL_AD9789_MODULE_SUB_CHENNEL_NUM; lSubChannelIndex++)
				{
					plSubModulatorParam = &s_Modulator.pParam[lSubChannelIndex + lModuleIndex * HWL_AD9789_MODULE_SUB_CHENNEL_NUM];

					lRegData32 = HWLL_QAMAD9789GainValueGet(plSubModulatorParam->Gain, plSubModulatorParam->QamMode);

					HWLL_QAMRegisterAdd(TRUE, lICID, 0x25 + lSubChannelIndex,  lRegData32 & 0xFF);
				}
			}
			HWLL_QAMRegisterApply();
		}

		s_Modulator.bLevelOnly = TRUE;//恢复标志以待下一次设定
	}
	else if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_DTMB)
	{
		U8	lTmpPowern;

		U32 lFDAC, lRCLK;
		U32 P, Q, N;
		F64 lPQFactor;

		U32 lRegData32;
		S16 lICID;
		S32 lCentralFreq, lTmpFreq, lRealCenterFreq, lExtFreqOffset;
		F64 lFTW, lFTWBPF, lFDAC16;
		HWL_ModulatorParam_t *plModulatorParam;

		/*取得参数*/
		plModulatorParam = &s_Modulator.pParam[0];

		GLOBAL_TRACE(("Start Set AD9789 for DTMB Parameter Freq = %d, QAM = %d, Carrier = %d, PN = %d, CodeRate = %d, TI = %d, Pilot = %d\n", plModulatorParam->ModulateFreq, plModulatorParam->QamMode, plModulatorParam->DTMB_Carrier, plModulatorParam->DTMB_PN, plModulatorParam->DTMB_CodeRate, plModulatorParam->DTMB_TI, plModulatorParam->DTMB_DoublePilot));

		lICID = ICPL_IIC_IC_ID_AD9789_START;
#ifndef MULT_USE_CHEAP_VCO_REFERENCE

#if defined(GQ3763) || defined(GQ3768)

#if defined(USE_MAX287X_2200_40M)
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2200;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_4000;
#elif defined(USE_MAX287X_2400_40M)
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_4000;
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2400;
#elif defined(USE_MAX287X_2400_10M)
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_1000;
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2400;
#else
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2400;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_1000;
#endif

#elif defined(LR1800S) || defined(GQ3760A)
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2177;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_3024;
#elif defined(GQ3760)
#ifdef MULT_QAM_NET_SETTING_USE_VOC_REFERENCE_FREQ_30_24
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2177;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_3024;
#else
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2200;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_1000;
#endif

#elif defined(GQ3760B)
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2400; //输出
		lRCLK = s_REFCLKHz;				  //参考时钟
#else
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2400;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_2500;
#endif

#else
		lFDAC = HWL_QAM_VCO_FDAC_HZ_2200;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_2500;
#endif



		/* - 初始化 --------------------------------------------------------------------------------------------------------------------------- */
		if (s_Modulator.bDTMBInited == FALSE)
		{
			#ifdef GQ3760B
			HWLL_QAMADF2871Apply(lFDAC, lRCLK, s_bVCO2781);
			#else
			HWLL_QAMADF4350Apply(lFDAC, lRCLK);
			#endif
			HWLL_QAMAD9789RestAndInitiate();
		}

		if (s_Modulator.bLevelOnly == FALSE)
		{
			/*计算符号率决定使用的FDAC的值*/
			plModulatorParam->SymRate = HWL_QAM_DTMB_FIXED_SYMBO_RATE ;//符号率是个固定值！
#if defined(GQ3760B)
			plModulatorParam->SymRate = HWL_QAM_DTMB_FIXED_SYMBO_RATE * 2;//二倍采用
#endif

#if defined(GQ3760B) || defined(GQ3763) || defined(GQ3768)
			s_Modulator.FrequeceCalibrate = 0;
#endif
			lFDAC += s_Modulator.FrequeceCalibrate;

			/* - 修改频率和符号率 ----------------------------------------------------------------------------------------------------------------- */

			HWLL_QAMCalculatePQN(lFDAC, plModulatorParam->SymRate, &P, &Q, &N);
			lPQFactor = (F64)P / (F64)Q;


			lFDAC16 = (F64)lFDAC / 16;
			lFTW = (F64)(0xFFFFFF) / lFDAC16;
			lFTWBPF = (F64)0xFFFF / ((F64)lFDAC);

			/*参数预计算*/
			lCentralFreq = plModulatorParam->ModulateFreq;

			lExtFreqOffset = 0;

#ifdef SUPPORT_MODULATOR_FREQ_ACCURACY
			lExtFreqOffset = CAL_EXGetValue(s_pFFCUVENode, s_FFCUVENodeNum, lCentralFreq);
#endif

			lRealCenterFreq = lCentralFreq + lExtFreqOffset;

			GLOBAL_TRACE(("Center Freq = %d Hz, Real Freq = %d Hz\n", lCentralFreq, lRealCenterFreq));

#ifdef SUPPORT_MODULATOR_FREQ_1_HZ
			lRealCenterFreq = lRealCenterFreq + 16;
#endif

			/*通道频率设置 参数的单位是KHz所以乘以1000得到HZ，2014-12-20修改后不再需要乘以1000*/

			lRegData32 = lFTW * lRealCenterFreq;

			//GLOBAL_TRACE(("NCO Freq = %d Hz, REG= 0x%.6X, Theory Freq = %f\n", lRealCenterFreq, lRegData32, (lRegData32 / lFTW)));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x0C, ((lRegData32 >> 16) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x0B, ((lRegData32 >> 8) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x0A, (lRegData32 & 0xFF));
			HWLL_QAMRegisterApply();

			/*16倍插值滤波器设置*/
			lRegData32 = lFTWBPF * lRealCenterFreq;

			//GLOBAL_TRACE(("BFP Freq = %d Hz, REG= 0x%.6X\n", lRealCenterFreq, lRegData32));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1D, ((lRegData32 >> 8) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1C, ((lRegData32 & 0xFF)));
			HWLL_QAMRegisterApply();

			/*速率转换设置*/
			lRegData32 = Q;
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x18, ((lRegData32 >> 16) & 0xFF)); //q
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x17, ((lRegData32 >> 8) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x16, (lRegData32 & 0xFF));

			lRegData32 = P;
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1B, ((lRegData32 >> 16) & 0xFF)); //p
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1A, ((lRegData32 >> 8) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x19, (lRegData32 & 0xFF));
			HWLL_QAMRegisterApply();


			/*更新速率转换寄存器和16倍插值寄存器寄存器*/
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1E, 0x80);
			HWLL_QAMRegisterApply();


			/* - 修改工作模式，绕过QAM MAP和SRRC滤波器 -------------------------------------------------------------------------------------------- */
			if (s_Modulator.bDTMBInited == FALSE)
			{
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
				//GLOBAL_TRACE(( "Reg0x06 Value = 0x%.2X\n", lTmpPowern ));
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x06, lTmpPowern );//旁路QAM映射器（QAM MAPPING），平方根升余弦（SRRC）滤波器，以及所有的插值滤波器
				HWLL_QAMRegisterApply();


				HWLL_QAMRegisterAdd(TRUE, lICID, 0x07, 0x02 );//SRRC和QAM映射器参数
				HWLL_QAMRegisterApply();
			}


			/*2014-10-14 使用标量求和寄存器来做频响*/
			{
				lRegData32 = CAL_EXGetValue(s_pFGCUVENode, s_FGCUVENodeNum, lCentralFreq);
				if (s_Modulator.pParam[0].QamMode == HWL_CONST_MODULATOR_QAM_32)
				{
					lRegData32 -=1 ;
				}
				GLOBAL_TRACE(("CentralFreq  = %d, Reg 0x08 Value = %02X\n", lCentralFreq, lRegData32 & 0xFF));
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x08, lRegData32 & 0xFF);//求和标量寄存器
				//HWLL_QAMRegisterAdd(TRUE, lICID, 0x08, 0x60 );//求和标量寄存器
				HWLL_QAMRegisterApply();
			}

			if (s_Modulator.bDTMBInited == FALSE)
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x09, 0x20 );//输入标量寄存器
				HWLL_QAMRegisterApply();


#ifndef MULT_AD9789_USE_CHANNEL_MODE/*QDUC模式/通道模式设置*/
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x20, 0xD8);
				//lRegData32 = 0x7F;//2.2G的时候使用
				lRegData32 = 0x64;//2.4G的时候使用
#else
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x20, 0xD0);//通道模式
				lRegData32 = 0x79;
#endif
				//GLOBAL_TRACE(("Reg0x21 Value = 0x%.2X\n", lRegData32));
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x21, lRegData32);//采样接口调整，第2个上升沿


				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x0F);//暂时DISABLE
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x23, 0x83);

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);//应用上述0x22 - 0x23个寄存器
				HWLL_QAMRegisterApply();


				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x1F);//DCO设置为FDAC/16

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);//应用上述0x22 - 0x23个寄存器
				HWLL_QAMRegisterApply();

				/*关闭不使用的通道*/
				lRegData32 = 0;
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x25 + 1,  lRegData32 & 0xFF);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x25 + 2,  lRegData32 & 0xFF);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x25 + 3,  lRegData32 & 0xFF);
				HWLL_QAMRegisterApply();
			}


			/*频谱翻转 设置*/
			GLOBAL_TRACE(("Spectrum Invert = %d\n", plModulatorParam->SpectrumInvert));
#ifndef MULT_AD9789_USE_CHANNEL_MODE
			if(plModulatorParam->SpectrumInvert)
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x29, 0x01);
			}
			else
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x29, 0x00);
			}
#else
			if(!plModulatorParam->SpectrumInvert)
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x29, 0x01);
			}
			else
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x29, 0x00);
			}
#endif

			if (s_Modulator.bDTMBInited == FALSE)
			{
				/*噪声优化*/
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x36, 0x01);
				/*DAC解码模式，00为普通*/
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x38, 0x00);
			}

			/*DAC电流设置*/
			lTmpFreq = lCentralFreq / 50;
			if( lTmpFreq > 10000000)
			{
				lRegData32 = (lTmpFreq - 5400) * 4 / 46;
			}
			else
			{
				lRegData32 = (lTmpFreq + 13500) * 4 / 235;

			}

			if((lCentralFreq > 440000000) & (lCentralFreq < 560000000))
			{
				lRegData32 = lRegData32 + 130;
			}

			if(lCentralFreq > 760000000)
			{
				lRegData32 = 0x03FF;
			}

			if(lRegData32 > 0x03FF)
			{
				lRegData32 = 0x03FF;
			}
			//GLOBAL_TRACE(("REG 0x3C/3D = %.6X\n", lRegData32));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x3D, ((lRegData32 >> 8) & 0x03));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x3C, (lRegData32 & 0xFF));
			HWLL_QAMRegisterApply();

			//PFC_TaskSleep(100);

			///*更新速率转换寄存器和16倍插值寄存器寄存器*/
			//HWLL_QAMRegisterAdd(TRUE, lICID, 0x1E, 0x80);
			//HWLL_QAMRegisterApply();


		}

		/*频率微调*/
#ifdef SUPPORT_MODULATOR_FREQ_1_HZ
		{
			lRegData32 = -plModulatorParam->FreqAdj + 64;
			//GLOBAL_TRACE(("Freq Offset = %d, REG[0x81]Value = %.2X\n", plModulatorParam->FreqAdj, lRegData32));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x81, lRegData32);
			HWLL_QAMRegisterApply();
		}
#endif

		/* RF 开关，注意先打开RF才能进行输出平滑控制 */
		if (plModulatorParam->RFSwitch)
		{
			lRegData32 = 1;
		}
		else
		{

			lRegData32 = 0;
		}
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x05, (lRegData32 & 0x0F));
		HWLL_QAMRegisterApply();


		/*单音模式设置*/
#ifndef SUPPORT_MODULATOR_FPGA_TONE//AD9789做单音
		{
			if (plModulatorParam->QamSwitch == FALSE)
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x0F);//DCO关闭模拟调制关闭现象
			}
			else
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x1F);//DCO设置为FDAC/16
			}
			HWLL_QAMRegisterApply();
			PFC_TaskSleep(100);
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00);
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);//应用上述0x22 - 0x23个寄存器
			HWLL_QAMRegisterApply();
		}
#endif
		PFC_TaskSleep(100);

		/*复位调制模块，因为复位时会导致符号突变，所以这里在电平升高之前复位*/
		if (s_Modulator.pResetCB)
		{
			s_Modulator.pResetCB();
		}

		PFC_TaskSleep(100);

		/*通道增益设置*/
#ifdef SUPPORT_SMOOTH_RF_OUTPUT_POWER_CHANGE
		if (plModulatorParam->RFSwitch == TRUE)
		{
			HWL_QAMForceOutputProgressChange(HWLL_QAMAD9789GainValueGet(plModulatorParam->Gain, plModulatorParam->QamMode));
		}
		else
		{
			HWL_QAMForceOutputProgressChange(0);
		}
#else
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x25,  HWLL_QAMAD9789GainValueGet(plModulatorParam->Gain, plModulatorParam->QamMode));
		HWLL_QAMRegisterApply();
#endif

#if defined(LR1800S) || defined(GQ3760A) || defined(GQ3760)//旧的程序需要最后设置DTMB模块参数并复位
		if (plModulatorParam->DTMB_Carrier == 0)
		{
			plModulatorParam->DTMB_Carrier = 1;
			HWLL_QAMDTMBSetDTMBModeToFPGA(plModulatorParam);
			plModulatorParam->DTMB_Carrier = 0;
		}
		else
		{
			plModulatorParam->DTMB_Carrier = 0;
			HWLL_QAMDTMBSetDTMBModeToFPGA(plModulatorParam);
			plModulatorParam->DTMB_Carrier = 1;
		}
		HWLL_QAMDTMBSetDTMBModeToFPGA(plModulatorParam);
#endif

		s_Modulator.bDTMBInited = TRUE;
		s_Modulator.bLevelOnly = TRUE;//恢复标志以待下一次设定
	}
	else if (s_Modulator.ModuleType == HWL_CONST_MODULATOR_AD9789_QPSK_L)
	{
		U8	lTmpPowern;

		U32 lFDAC, lRCLK;
		U32 P, Q, N;
		F64 lPQFactor;

		U32 lRegData32;
		S16 lICID;
		S32 lCentralFreq, lTmpFreq;
		F64 lFTW, lFTWBPF, lFDAC16;
		HWL_ModulatorParam_t *plModulatorParam;

		GLOBAL_TRACE(("Start Set AD9789 for DVB-S Parameter\n"));

		/*取得参数*/
		plModulatorParam = &s_Modulator.pParam[0];

		lICID = ICPL_IIC_IC_ID_AD9789_START;

		lFDAC = HWL_QAM_VCO_FDAC_HZ_2400;
		lRCLK = HWL_QAM_VCO_REF_CLK_HZ_2500;

		/* - 初始化 --------------------------------------------------------------------------------------------------------------------------- */
		if (s_Modulator.bDTMBInited == FALSE)
		{
			HWLL_QAMADF2871Apply(lFDAC, lRCLK);
			HWLL_QAMAD9789RestAndInitiate();
		}

		if (s_Modulator.bLevelOnly == FALSE)
		{

			/*发送FEC给FPGA*/
			lICID = ICPL_IIC_IC_ID_AD9789_START;


			/*计算符号率决定使用的FDAC的值*/

			lFDAC += s_Modulator.FrequeceCalibrate;

			/* - 修改频率和符号率 ----------------------------------------------------------------------------------------------------------------- */

			HWLL_QAMCalculatePQN(lFDAC, plModulatorParam->SymRate, &P, &Q, &N);
			lPQFactor = (F64)P / (F64)Q;


			lFDAC16 = (F64)lFDAC / 16;
			lFTW = (F64)(0xFFFFFF) / lFDAC16;
			lFTWBPF = (F64)0xFFFF / ((F64)lFDAC);

			/*参数预计算*/
			lCentralFreq = plModulatorParam->ModulateFreq;

			GLOBAL_TRACE(("Center Freq = %d Hz, Real Freq = %d Hz\n", lCentralFreq, lCentralFreq));

			/*通道频率设置 参数的单位是KHz所以乘以1000得到HZ，2014-12-20修改后不再需要乘以1000*/

			lRegData32 = lFTW * lCentralFreq;

			//GLOBAL_TRACE(("NCO Freq = %d Hz, REG= 0x%.6X, Theory Freq = %f\n", lCentralFreq, lRegData32, (lRegData32 / lFTW)));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x0C, ((lRegData32 >> 16) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x0B, ((lRegData32 >> 8) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x0A, (lRegData32 & 0xFF));
			HWLL_QAMRegisterApply();

			/*16倍插值滤波器设置*/
			lRegData32 = lFTWBPF * lCentralFreq;

			//GLOBAL_TRACE(("BFP Freq = %d Hz, REG= 0x%.6X\n", lCentralFreq, lRegData32));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1D, ((lRegData32 >> 8) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1C, ((lRegData32 & 0xFF)));
			HWLL_QAMRegisterApply();

			/*速率转换设置*/
			lRegData32 = Q;
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x18, ((lRegData32 >> 16) & 0xFF)); //q
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x17, ((lRegData32 >> 8) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x16, (lRegData32 & 0xFF));

			lRegData32 = P;
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1B, ((lRegData32 >> 16) & 0xFF)); //p
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1A, ((lRegData32 >> 8) & 0xFF));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x19, (lRegData32 & 0xFF));
			HWLL_QAMRegisterApply();


			/*更新速率转换寄存器和16倍插值寄存器寄存器*/
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x1E, 0x80);
			HWLL_QAMRegisterApply();


			/* - 修改工作模式，绕过QAM MAP和SRRC滤波器 -------------------------------------------------------------------------------------------- */
			if (s_Modulator.bDTMBInited == FALSE)
			{
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
				//GLOBAL_TRACE(( "Reg0x06 Value = 0x%.2X\n", lTmpPowern ));
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x06, lTmpPowern );//旁路QAM映射器（QAM MAPPING），平方根升余弦（SRRC）滤波器，以及所有的插值滤波器
				HWLL_QAMRegisterApply();


				HWLL_QAMRegisterAdd(TRUE, lICID, 0x07, 0x02 );//SRRC和QAM映射器参数
				HWLL_QAMRegisterApply();
			}

#if 0
			/*2014-10-14 使用标量求和寄存器来做频响*/
			{
				lRegData32 = CAL_EXGetValue(s_pFGCUVENode, s_FGCUVENodeNum, lCentralFreq);
				if (s_Modulator.pParam[0].QamMode == HWL_CONST_MODULATOR_QAM_32)
				{
					lRegData32 -=1 ;
				}
				GLOBAL_TRACE(("CentralFreq  = %d, Reg 0x08 Value = %02X\n", lCentralFreq, lRegData32 & 0xFF));
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x08, lRegData32 & 0xFF);//求和标量寄存器
				//HWLL_QAMRegisterAdd(TRUE, lICID, 0x08, 0x60 );//求和标量寄存器
				HWLL_QAMRegisterApply();
			}
#else
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x08, 0x2F );//求和标量寄存器
			HWLL_QAMRegisterApply();
#endif

			if (s_Modulator.bDTMBInited == FALSE)
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x09, 0xFF);//输入标量寄存器
				HWLL_QAMRegisterApply();

				//上变频模式
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x20, 0xD8);

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


				GLOBAL_TRACE(("Reg 21 = 0x%02X\n", lRegData32));
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x21, lRegData32);//采样接口调整，第2个上升沿


				HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x0F);//暂时DISABLE
				lRegData32 = 0x83;
				GLOBAL_TRACE(("Reg 21 = 0x%02X\n", lRegData32));
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x23, lRegData32);

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);//应用上述0x22 - 0x23个寄存器
				HWLL_QAMRegisterApply();


				/*单音模式设置*/
				if (plModulatorParam->QamSwitch == FALSE)
				{
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x0F);//DCO关闭模拟调制关闭现象
				}
				else
				{
					HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x1F);//DCO设置为FDAC/16
					//HWLL_QAMRegisterAdd(TRUE, lICID, 0x22, 0x2F);//DCO设置为FDAC/32
				}
				HWLL_QAMRegisterApply();

				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x00);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x24, 0x80);//应用上述0x22 - 0x23个寄存器
				HWLL_QAMRegisterApply();

				/*关闭不使用的通道*/
				lRegData32 = 0;
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x25 + 1,  lRegData32 & 0xFF);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x25 + 2,  lRegData32 & 0xFF);
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x25 + 3,  lRegData32 & 0xFF);
				HWLL_QAMRegisterApply();
			}


			/*频谱翻转 设置*/
			GLOBAL_TRACE(("Spectrum Invert = %d\n", plModulatorParam->SpectrumInvert));
			if(plModulatorParam->SpectrumInvert)
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x29, 0x01);
			}
			else
			{
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x29, 0x00);
			}

			if (s_Modulator.bDTMBInited == FALSE)
			{
				/*噪声优化*/
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x36, 0x01);
				/*DAC解码模式，00为普通*/
				HWLL_QAMRegisterAdd(TRUE, lICID, 0x38, 0x00);
			}

			/*DAC电流设置*/
			lTmpFreq = lCentralFreq / 50;
			if( lTmpFreq > 10000000)
			{
				lRegData32 = (lTmpFreq - 5400) * 4 / 46;
			}
			else
			{
				lRegData32 = (lTmpFreq + 13500) * 4 / 235;

			}

			if((lCentralFreq > 440000000) & (lCentralFreq < 560000000))
			{
				lRegData32 = lRegData32 + 130;
			}

			if(lCentralFreq > 760000000)
			{
				lRegData32 = 0x03FF;
			}

			if(lRegData32 > 0x03FF)
			{
				lRegData32 = 0x03FF;
			}
			//GLOBAL_TRACE(("REG 0x3C/3D = %.6X\n", lRegData32));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x3D, ((lRegData32 >> 8) & 0x03));
			HWLL_QAMRegisterAdd(TRUE, lICID, 0x3C, (lRegData32 & 0xFF));
			HWLL_QAMRegisterApply();

			//PFC_TaskSleep(100);

			///*更新速率转换寄存器和16倍插值寄存器寄存器*/
			//HWLL_QAMRegisterAdd(TRUE, lICID, 0x1E, 0x80);
			//HWLL_QAMRegisterApply();

			lRegData32 = plModulatorParam->QPSK_FEC;
			GLOBAL_TRACE(("FEC REG= 0x%.2X\n", lRegData32));
			HWLL_QAMRegisterAdd(TRUE, lICID, ICPL_IIC_REG_AD9789_QBSK_SP_INFO, (lRegData32 & 0xFF));
			HWLL_QAMRegisterApply();

		}

		/* RF 开关，注意先打开RF才能进行输出平滑控制 */
		if (plModulatorParam->RFSwitch)
		{
			lRegData32 = 1;
		}
		else
		{

			lRegData32 = 0;
		}
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x05, (lRegData32 & 0x0F));
		HWLL_QAMRegisterApply();

		PFC_TaskSleep(100);


		/*通道增益设置*/
		HWLL_QAMRegisterAdd(TRUE, lICID, 0x25,  HWLL_QAMAD9789GainValueGet(plModulatorParam->Gain, plModulatorParam->QamMode));
		HWLL_QAMRegisterApply();


		/*通道衰减设计*/
		HWLL_QAMDAT31R5SP1Apply(plModulatorParam->Attenuator);
		//s_Modulator.bDTMBInited = TRUE;
		//s_Modulator.bLevelOnly = TRUE;//恢复标志以待下一次设定
	}
	else
	{
		GLOBAL_TRACE(("UnSupported Type = %d\n",s_Modulator.ModuleType));
	}
	return HWL_SUCCESS;
}


/*设置FDAC频率偏移*/
void HWL_QAMFDACOffsetSet(S32 FrequenceCalibrate)
{
	if (s_Modulator.FrequeceCalibrate != FrequenceCalibrate)
	{
		s_Modulator.FrequeceCalibrate = FrequenceCalibrate;
		s_Modulator.bLevelOnly = FALSE;
		s_Modulator.bDTMBInited = FALSE;
	}
}



/*直接设置寄存器*/
void HWL_QAMDirectRegSet(U8 ICID, U32 Addr, U32 Value)
{
	if (ICID == 0xFF)
	{
		S32 i;
		U8 l21Reg, l23Reg;
		for (i = 0; i < sizeof(s_DelayTest) / sizeof(TEST_REG); i++)
		{
			l21Reg = 0x78 | s_DelayTest[i].m_LAT;
			l23Reg = ((s_DelayTest[i].m_DSC << 4) | (s_DelayTest[i].m_SNC));
			GLOBAL_TRACE(("Inde i = %d, LAT = %d, DSC = %d, SNC = %d, 21 = %02X, 23 = %02X\n", i, s_DelayTest[i].m_LAT, s_DelayTest[i].m_DSC, s_DelayTest[i].m_SNC, l21Reg, l23Reg));
			HWLL_QAMRegisterAdd(TRUE, 1, 0x21, l21Reg);
			HWLL_QAMRegisterAdd(TRUE, 1, 0x23, l23Reg);
			HWLL_QAMRegisterApply();
			HWLL_QAMRegisterAdd(TRUE, ICID, 0x24, 0x00);
			HWLL_QAMRegisterApply();
			PFC_TaskSleep(1);
			HWLL_QAMRegisterAdd(TRUE, ICID, 0x24, 0x80);//应用上述0x22 - 0x23个寄存器
			HWLL_QAMRegisterApply();

			PFC_TaskSleep(2000);
		}
	}
	else
	{
		GLOBAL_TRACE(("ICID = 0x%x, Addr = 0x%x, Value = 0x%x\n", ICID, Addr, Value));
		HWLL_QAMRegisterAdd(TRUE, ICID, Addr, Value);
		HWLL_QAMRegisterApply();

		if ((ICID == 1) && ((Addr == 0x22) || (Addr == 0x23)))
		{
			GLOBAL_TRACE(("Apply 22~23\n"));
			HWLL_QAMRegisterAdd(TRUE, ICID, 0x24, 0x00);
			HWLL_QAMRegisterApply();
			PFC_TaskSleep(1);
			HWLL_QAMRegisterAdd(TRUE, ICID, 0x24, 0x80);//应用上述0x22 - 0x23个寄存器
			HWLL_QAMRegisterApply();
		}


	}

}


/*销毁*/
void HWL_QAMTerminate(void)
{

}



#endif

