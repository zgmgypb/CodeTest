//这个是有自己的线程和进程的的模块

/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_tsp.h"

//#define SUPPORT_CLK_ADJ_MODULE

#ifdef SUPPORT_CLK_ADJ_MODULE
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */

#define CLK_SYNC_DEBUG
//#define CLK_SYNC_DEBUG_PROTOCOL
#define MULT_CLK_TASK_STATCK_SIZE					(1024*1024)
#define MULT_CLK_STORAGE_NUM					(360)//存长时间的数据用作计算
#define MULT_CLK_SAMPLE_CLK_FREQ_HZ				(150 * 1000000)
#define MULT_CLK_DESC_CLK_FREQ_HZ				(10 * 1000000)
#define CLK_DA_MAX_OFFSET_VALUE					(0x6000)
#define CLK_ERR_TOLERANCE_NUMBER				(32)
#define CLK_STATE_4_TIMEOUT						(40)
#define CLK_STATE_5_TIMEOUT						(120)
#define CLK_STATE_6_TIMEOUT						(240)
#define CLK_DEFAULT_DA_VALUE					(0x7000)
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */



typedef struct  
{
	S32					m_SumOffset;
	//S32				m_OtherCount;
}MULT_CLKNode;


typedef struct
{
	S32					m_InitDAValue;

	S32					m_CurCLKSYNCSrc;

	MULT_CLKNode		m_pClkOffset[MULT_CLK_STORAGE_NUM];
	S32					m_CurUseSlot;
	S32					m_TotalNum;
	U32					m_1PPSCC;

	/*状态机*/
	S32					m_State;
	S32					m_Timeout;

	F64					m_OffsetMax;
	F64					m_OffsetMin;
	S32					m_CurSumOffset;
	F64					m_REG1Hz;

	S32					m_LockToSumOffset;
	S32					m_DAValue;
	S32					m_StableDAValue;

	S32					m_CLKSyncErrorCount;

	MULT_Handle			*m_pMainHandle;
}MULT_CLK_Handle;


/* Private Variables (static)-------------------------------------------------- */
static MULT_CLK_Handle *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
static F64 MULTL_CLKGetSampleOffsetAverage(S32 Start, S32 Duration, S32 *pOffsetSum);
static void MULTL_CLKSetCurrentOffset(S32 Offset);
static void MULTL_CLKStateFunc(void);
static void MULTL_CLKProtocolResetFPGACounterPacker(void);
static void MULTL_CLKPrintFunc(void);
static void MULTL_CLKRecodeCLKOffsetToFile(void);

/* Functions ------------------------------------------------------------------ */
BOOL MULT_CLKInitiate(MULT_Handle *pMainHandle)
{
	BOOL lRet = FALSE;
	if (s_pHandle == NULL)
	{
		MULT_CLK_Handle *plHandle = (MULT_CLK_Handle*) GLOBAL_ZMALLOC(sizeof(MULT_CLK_Handle));
		if (plHandle)
		{
			plHandle->m_REG1Hz = 0;
			plHandle->m_Timeout = 360;
			plHandle->m_State = 0;
			plHandle->m_DAValue = CLK_DEFAULT_DA_VALUE;
#ifdef GQ3765
			plHandle->m_Timeout -= 300;
#endif
#ifdef CLK_SYNC_DEBUG
			//plHandle->m_Timeout = 0;
#endif
			plHandle->m_pMainHandle = pMainHandle;
			plHandle->m_1PPSCC = GLOBAL_U32_MAX;


			GLOBAL_TRACE(("CLK_MODULE Initiate Complete!\n"));
			lRet = TRUE;
		}
		s_pHandle = plHandle;

		MULTL_CLKProtocolPacker();
	}
	return lRet;
}


void MULT_CLKSetDefaultDAValue(U32 DAValue)
{
	if (s_pHandle)
	{
		GLOBAL_TRACE(("DAValue = %.4X !!!\n", DAValue));
		s_pHandle->m_InitDAValue = DAValue;
		if ((s_pHandle->m_InitDAValue < (CLK_DEFAULT_DA_VALUE - CLK_DA_MAX_OFFSET_VALUE)) || (s_pHandle->m_InitDAValue > (CLK_DEFAULT_DA_VALUE + CLK_DA_MAX_OFFSET_VALUE)))
		{
			s_pHandle->m_InitDAValue = CLK_DEFAULT_DA_VALUE;
		}
	}
}



U32 MULT_CLKGetDefaultDAValue(void)
{
	U32 lDAValue = GLOBAL_U32_MAX;
	if (s_pHandle)
	{
		lDAValue = s_pHandle->m_InitDAValue;
	}
	return lDAValue;
}

void MULT_CLKSetTranningValue(F64 TrainningValue)
{
	if (s_pHandle)
	{
		GLOBAL_TRACE(("REG1Hz = %f !!!\n", TrainningValue));
		s_pHandle->m_REG1Hz = TrainningValue;
	}
}


F64 MULT_CLKGetTranningValue(void)
{
	if (s_pHandle)
	{
		return s_pHandle->m_REG1Hz;
	}
	return 0.0;
}


BOOL MULT_CLKGet10MLockStatus(void)
{
	if (s_pHandle)
	{

		if (s_pHandle->m_CurCLKSYNCSrc != 0)
		{
			F64 lCurrOffsetHz;
			lCurrOffsetHz = MULTL_CLKGetSampleOffsetAverage(1, 1, NULL);
			if (fabs(lCurrOffsetHz) <= CLK_ERR_TOLERANCE_NUMBER)
			{
				return (s_pHandle->m_State >= 5)?TRUE:FALSE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

void MULT_CLKSet10MCLKSYNCSyc(S32 NewSRC)
{
	if (s_pHandle->m_CurCLKSYNCSrc != NewSRC)
	{
		GLOBAL_TRACE(("CLK SYC CHANGE From %d To %d!!!\n", s_pHandle->m_CurCLKSYNCSrc, NewSRC));
		if (s_pHandle->m_State >= 4)
		{
			s_pHandle->m_Timeout = 0;
		}
		s_pHandle->m_State = 0;
		//s_pHandle->m_InitDAValue = CLK_DEFAULT_DA_VALUE;
	}
	s_pHandle->m_CurCLKSYNCSrc = NewSRC;
}

S32 MULT_CLKGetCurSumOffset(void)
{
	if (s_pHandle)
	{
		return s_pHandle->m_CurSumOffset;
	}
}

/*消息解析函数，解析FPGA给出的值，并对DA进行调整*/
BOOL MULT_CLKProtocolParser(U8 *pData, S32 DataSize)
{
	BOOL lRet = FALSE;

	if (s_pHandle)
	{
		if (pData)
		{
			if ((pData[0] == 0x33) && (DataSize >= 12))
			{
				U8 *plTmpData;
				U32 lTmpValue1;
				U32 lTmpValue2;
				U32 lTmpValue3;

				plTmpData = pData;

				if (pData[1] == 0x03)
				{
					S32 lCurCC;
					S32 lTotal150MOffset, l10MOffset;
					F64 l1PPS10MPulseNum;
#ifdef CLK_SYNC_DEBUG_PROTOCOL
					CAL_PrintDataBlock(__FUNCTION__, pData, DataSize);
#endif

					GLOBAL_BYTES_S(plTmpData, 8);
					GLOBAL_MSB16_D(plTmpData, lTmpValue1);

					/*得到10M相对于1PPS的上升沿的绝对误差时间用150M采样的个数*/
					lTotal150MOffset = (lTmpValue1 - CLK_DEFAULT_DA_VALUE)/*FPGA计数的起点为CLK_DEFAULT_DA_VALUE*/;

					//GLOBAL_TRACE(("Total Offset = %d ---------\n", lTotal150MOffset));

					GLOBAL_MSB8_D(plTmpData, lTmpValue1);
					GLOBAL_MSB8_D(plTmpData, lTmpValue2);

					/*判断连续计数*/
					//lCurCC = ((lTmpValue1 >> 2) & 0x3F);

					//if ((s_pHandle->m_1PPSCC != lCurCC) && (s_pHandle->m_1PPSCC <= 0x3F))
					//{
					//	GLOBAL_TRACE(("1PPS CC Lost Recv = %d, Should be = %d\n", lCurCC, s_pHandle->m_1PPSCC));
					//}

					//s_pHandle->m_1PPSCC = lCurCC + 1;

					//if (s_pHandle->m_1PPSCC > 0x3F)
					//{
					//	s_pHandle->m_1PPSCC = 0;
					//}


					///*把10M误差计算成频率误差*/
					//lTmpValue3 = (((lTmpValue1 << 3) & 0x18) | ((lTmpValue2 >> 5) & 0x07));


					//l1PPS10MPulseNum = (F64)(((lTmpValue3 & 0x10) > 0)?((((~lTmpValue3) + 1) & 0x1F) * (-1.0)):lTmpValue3) * 15;//补码得到整数部分的误差

					//lTmpValue3 = (lTmpValue2 & 0x1F);

					////GLOBAL_TRACE(("OffsetFloat = %.2X\n", lTmpValue3));


					//l1PPS10MPulseNum += (F64)(((lTmpValue3 & 0x10) > 0)?((((~lTmpValue3) + 1) & 0x1F) * (-1.0)):lTmpValue3);

					//GLOBAL_TRACE(("Current Offset1 = %f Hz\n", l1PPS10MPulseNum / 15));


					/*1PPS递增状态*/
					GLOBAL_MSB16_D(plTmpData, lTmpValue1);


					lCurCC = lTmpValue1;

					if (s_pHandle->m_1PPSCC != lCurCC)
					{
						if (s_pHandle->m_1PPSCC != GLOBAL_U32_MAX)
						{
							//GLOBAL_TRACE(("10M 1PPS CC Lost Recv = %d, Should be = %d\n", lCurCC, s_pHandle->m_1PPSCC));
						}
					}
					s_pHandle->m_1PPSCC = lCurCC + 1;
					if (s_pHandle->m_1PPSCC > GLOBAL_U16_MAX)
					{
						s_pHandle->m_1PPSCC = 0;
					}


					/*10M的采样值*/
					GLOBAL_MSB16_D(plTmpData, lTmpValue2);

					if ((lTmpValue2 & 0x8000) > 0)
					{
						l10MOffset = -(((~lTmpValue2) & 0x7FFF) + 1) ;
					}
					else
					{
						l10MOffset = lTmpValue2;
					}

					/*设置当前累计偏移，并进行处理*/
					MULTL_CLKSetCurrentOffset(l10MOffset);

					lRet = TRUE;

				}
				else
				{
					CAL_PrintDataBlock(__FUNCTION__, pData, DataSize);
					GLOBAL_TRACE(("Len Error = %d\n", pData[1]));
				}
			}
		}
		else
		{
			GLOBAL_TRACE(("NULL Ptr!!!!!!!!!!\n"));
		}

		{
			MULTL_CLKStateFunc();
		}
	}
	else
	{
		//GLOBAL_TRACE(("Module Not Initiated!\n"));
	}
	return lRet;
}

BOOL MULT_CLKGetSyncError(BOOL bClear)
{
	BOOL lRet = FALSE;

	if (s_pHandle->m_CLKSyncErrorCount > 0)
	{
		lRet = TRUE;
	}

	if (bClear)
	{
		s_pHandle->m_CLKSyncErrorCount = 0;
	}
	
	return lRet;
}

/*模块销毁函数*/
void MULT_CLKTerminate(void)
{
	if (s_pHandle)
	{
		GLOBAL_FREE(s_pHandle);
		s_pHandle = NULL;

		GLOBAL_TRACE(("MODULE Terminate Complete!\n"));
	}
}


/* 本地函数 --------------------------------------------------------------------------------------------------------------------------------------- */

/*得到FPGA给出的累计误差的相对值，得出频率变化的速度*/
F64 MULTL_CLKGetSampleOffsetAverage(S32 Start, S32 Duration, S32 *pOffsetSum)
{
	S32 i, lInd1, lInd2, lOffsetCount;
	F64 lAverage;

	lAverage = 0;

	if (Start <= 0)
	{
		return 0;
	}

	if (Duration <= 0)
	{
		return 0;
	}

	if (Start + Duration >= s_pHandle->m_TotalNum)
	{
		return 0;
	}

	if (Duration > 0)
	{
		/*计算起始和终止目标*/
		lInd1 = s_pHandle->m_CurUseSlot - Start;
		if (lInd1 < 0)
		{
			lInd1 = MULT_CLK_STORAGE_NUM + lInd1;
		}

		lInd2 = s_pHandle->m_CurUseSlot - Start - Duration;
		if (lInd2 < 0)
		{
			lInd2 = MULT_CLK_STORAGE_NUM + lInd2;
		}

		/*计算10M的误差*/

		lAverage = (s_pHandle->m_pClkOffset[lInd1].m_SumOffset - s_pHandle->m_pClkOffset[lInd2].m_SumOffset);

		lOffsetCount = s_pHandle->m_pClkOffset[lInd1].m_SumOffset - s_pHandle->m_pClkOffset[lInd2].m_SumOffset;

		//GLOBAL_TRACE(("V%d = %d, V%d = %d\n", lInd1, s_pHandle->m_pClkOffset[lInd1].m_SumOffset, lInd2, s_pHandle->m_pClkOffset[lInd2].m_SumOffset));

		lAverage /= Duration;//求平均

		if (pOffsetSum)
		{
			(*pOffsetSum) = lOffsetCount;
		}
	}

	return lAverage;
}

/*保存当前立即误差值*/
void MULTL_CLKSetCurrentOffset(S32 Offset)
{
	if (s_pHandle->m_State > 0)
	{
#ifdef CLK_SYNC_DEBUG
		GLOBAL_TRACE(("Slot = %d,  TotalNum = %d, 10MSumOffset = [%d], DAValue = 0x%.4X, state = %d, Timeout = %d\n", s_pHandle->m_CurUseSlot, s_pHandle->m_TotalNum + 1, Offset, s_pHandle->m_DAValue, s_pHandle->m_State, s_pHandle->m_Timeout));
#endif

		s_pHandle->m_CurSumOffset = Offset;

		s_pHandle->m_pClkOffset[s_pHandle->m_CurUseSlot].m_SumOffset = Offset;
		//s_pHandle->m_pClkOffset[s_pHandle->m_CurUseSlot].m_OtherCount = 0;

		s_pHandle->m_CurUseSlot ++;

		if (s_pHandle->m_CurUseSlot == MULT_CLK_STORAGE_NUM)
		{
			s_pHandle->m_CurUseSlot = 0;
		}

		if (s_pHandle->m_TotalNum < MULT_CLK_STORAGE_NUM)
		{
			s_pHandle->m_TotalNum++;
		}
	}
}


BOOL MULTL_CLKDetectSyncOK(void)
{
	BOOL lRet = TRUE;
	S32 lCurrOffsetHz;

	/*时钟校正程序错误检测*/
	lCurrOffsetHz = MULTL_CLKGetSampleOffsetAverage(1, 1, NULL);
	if (fabs(lCurrOffsetHz) > CLK_ERR_TOLERANCE_NUMBER)
	{
		/*外部同步源丢失*/
		GLOBAL_TRACE(("CLK Sync Error!!!!!!! Current Freq Offset = %d, Reset To InitState\n", lCurrOffsetHz));

		s_pHandle->m_CLKSyncErrorCount++;

		s_pHandle->m_State = 0;
		s_pHandle->m_Timeout = 16;

		lRet = FALSE;
	}


	

	return lRet;
}

BOOL MULTL_CLKDetectCLKSrcOK(void)
{
	BOOL lRet = TRUE;
#ifdef SUPPORT_SFN_MODULATOR
	{
		MULT_SFN_Param lParam;
		MULT_SFN_Status lSFNStatus;
		MULT_SFNGetSFNStatus(&lSFNStatus);
		MULT_SFNProcSFNParam(&lParam, TRUE);

		if (lParam.m_bUse10MClkSynSrc == 1)
		{
			if (lSFNStatus.m_bExt10MStatus == FALSE)
			{
				lRet = FALSE;
			}
		}

		if (lParam.m_bUse10MClkSynSrc == 2)
		{
			if (lParam.m_bUseEx1PPS == TRUE)
			{
				if (lSFNStatus.m_bExt1PPSStatus == FALSE)
				{
					lRet = FALSE;
				}
			}
			else
			{
				if (lSFNStatus.m_bInt1PPSStatus == FALSE)
				{
					lRet = FALSE;
				}
			}
		}
	}
#endif
	{
		
	}
	return lRet;
}




/*状态机函数*/
void MULTL_CLKStateFunc(void)
{
	F64 lCurrOffsetHz;
	S32 lCurrOffsetNum;
	
	/*内部时钟使用固定值！*/
	if ((s_pHandle->m_CurCLKSYNCSrc == 0))
	{
		if (s_pHandle->m_DAValue != CLK_DEFAULT_DA_VALUE)
		{
			s_pHandle->m_DAValue = CLK_DEFAULT_DA_VALUE;
			MULTL_CLKProtocolPacker();
		}
		s_pHandle->m_State = 0;
	}
	else if ((MULTL_CLKDetectCLKSrcOK() == FALSE))
	{
		if ((s_pHandle->m_InitDAValue < (CLK_DEFAULT_DA_VALUE - CLK_DA_MAX_OFFSET_VALUE)) || (s_pHandle->m_InitDAValue > (CLK_DEFAULT_DA_VALUE + CLK_DA_MAX_OFFSET_VALUE)))
		{
			s_pHandle->m_InitDAValue = CLK_DEFAULT_DA_VALUE;
		}
		if (s_pHandle->m_DAValue != s_pHandle->m_InitDAValue)
		{
			s_pHandle->m_DAValue = s_pHandle->m_InitDAValue;
			MULTL_CLKProtocolPacker();
		}
		s_pHandle->m_Timeout = 10;
		s_pHandle->m_State = 0;
	}
	else
	{
		if (s_pHandle->m_Timeout > 0)
		{
			s_pHandle->m_Timeout--;
		}

		switch (s_pHandle->m_State)
		{
		case 0://启动等待兼错误处理
			{
				if (s_pHandle->m_Timeout <= 0)
				{
					GLOBAL_TRACE(("CLK SYNC New Start!!!!!!!!!,\n"));

					if (s_pHandle->m_InitDAValue < (CLK_DEFAULT_DA_VALUE - CLK_DA_MAX_OFFSET_VALUE) || s_pHandle->m_InitDAValue > (CLK_DEFAULT_DA_VALUE + CLK_DA_MAX_OFFSET_VALUE))
					{
						s_pHandle->m_InitDAValue = CLK_DEFAULT_DA_VALUE;
					}

					s_pHandle->m_DAValue = s_pHandle->m_InitDAValue;

					MULTL_CLKProtocolPacker();

					s_pHandle->m_CurUseSlot = 0;
					s_pHandle->m_TotalNum = 0;

					if (s_pHandle->m_REG1Hz != 0)
					{
						s_pHandle->m_Timeout = CLK_STATE_4_TIMEOUT + 2;
						s_pHandle->m_State = 4;//越过学习程序
					}
					else
					{
						s_pHandle->m_State = 1;//学习程序
					}

				}
				else
				{
#ifdef CLK_SYNC_DEBUG
					GLOBAL_TRACE(("Starup Delay Left = %ds\n", s_pHandle->m_Timeout))
#endif
				}
			}
			break;
		case 1://学习等待
			{
				if (s_pHandle->m_Timeout <= 0)
				{
//#ifdef CLK_SYNC_DEBUG
//#endif
					GLOBAL_TRACE(("Start Learning Reg VS FreqHz!!!!!!!!!!!!!!!!\n"));
					/*清空历史数据*/
					s_pHandle->m_CurUseSlot = 0;
					s_pHandle->m_TotalNum = 0;
					s_pHandle->m_DAValue = CLK_DEFAULT_DA_VALUE - CLK_DA_MAX_OFFSET_VALUE;

					MULTL_CLKProtocolPacker();
					s_pHandle->m_Timeout = 52;
					s_pHandle->m_State = 2;
				}
			}
			break;
		case 2://最大正偏计算
			{

				MULTL_CLKPrintFunc();

				if (s_pHandle->m_Timeout <= 0)
				{
					/*统计最大正偏*/
					s_pHandle->m_OffsetMin = MULTL_CLKGetSampleOffsetAverage(1, 50, NULL);

					GLOBAL_TRACE(("MIN Offset = %f !!!!!!!!!!!!!!!!!!!!!\n", s_pHandle->m_OffsetMin));

					s_pHandle->m_DAValue = CLK_DEFAULT_DA_VALUE + CLK_DA_MAX_OFFSET_VALUE;

					MULTL_CLKProtocolPacker();

					/*状态转换*/
					s_pHandle->m_Timeout = 52;
					s_pHandle->m_State = 3;
				}
			}
			break;
		case 3://最大负偏计算
			{

				MULTL_CLKPrintFunc();

				if (s_pHandle->m_Timeout <= 0)
				{
					/*统计最大正偏*/
					s_pHandle->m_OffsetMax = MULTL_CLKGetSampleOffsetAverage(1, 50, NULL);

					GLOBAL_TRACE(("MAX Offset = %f !!!!!!!!!!!!!!!!!!!!!\n", s_pHandle->m_OffsetMax));

					if (s_pHandle->m_OffsetMax - s_pHandle->m_OffsetMin != 0)
					{
						s_pHandle->m_REG1Hz = CLK_DA_MAX_OFFSET_VALUE * 2 / (s_pHandle->m_OffsetMax - s_pHandle->m_OffsetMin);
					}
					else
					{
						s_pHandle->m_REG1Hz = 0;
					}

					GLOBAL_TRACE(("1Hz= %f REG, 1Bit %f Hz, \n", s_pHandle->m_REG1Hz, 1 / s_pHandle->m_REG1Hz));

					/*状态转换*/
					s_pHandle->m_Timeout = 0;
					s_pHandle->m_State = 0;
				}
			}
			break;
		case 4://粗调模式，调整到误差增量最小
			{
				MULTL_CLKPrintFunc();

				/*时钟校正程序错误检测*/
				if (MULTL_CLKDetectSyncOK())
				{
					if (s_pHandle->m_Timeout <= 0)
					{
						lCurrOffsetHz = MULTL_CLKGetSampleOffsetAverage(1, CLK_STATE_4_TIMEOUT, &lCurrOffsetNum);

						GLOBAL_TRACE(("Current FreqErr = %eHz, FreqOffsetNum = %d, State = %d\n", lCurrOffsetHz, lCurrOffsetNum, s_pHandle->m_State));

						s_pHandle->m_Timeout = CLK_STATE_4_TIMEOUT + 2;

						s_pHandle->m_DAValue += -(lCurrOffsetHz * s_pHandle->m_REG1Hz);
						MULTL_CLKProtocolPacker();

						if (fabs(lCurrOffsetHz) < 0.05)
						{
							s_pHandle->m_State = 5;
							s_pHandle->m_Timeout = CLK_STATE_5_TIMEOUT + 2;
							s_pHandle->m_InitDAValue = s_pHandle->m_DAValue;

							/*时钟锁定，清除累计计数器，复位调制模块*/
//#ifdef SUPPORT_SFN_MODULATOR
//							MULT_SFNApplyByQAMModule();
//#endif
//
//#ifdef SUPPORT_SFN_ADAPTER
//							MULT_SFNAApplyParameter();
//#endif
							MULTL_CLKProtocolResetFPGACounterPacker();

						}
						else if (fabs(lCurrOffsetHz) < 3.0)
						{
							/*状态不变！继续矫正*/
						}
						else
						{
							GLOBAL_TRACE(("Freq Offset [%f] OverLimits !!!!1, Reset To Initiate!\n", lCurrOffsetHz));
							/*异常处理*/
							s_pHandle->m_State = 0;
							s_pHandle->m_Timeout = 0;
							s_pHandle->m_InitDAValue = CLK_DEFAULT_DA_VALUE;
						}
					}
				}



			}
			break;
		case 5://细调模式
			{
				MULTL_CLKPrintFunc();
				if (MULTL_CLKDetectSyncOK())
				{
					if (s_pHandle->m_Timeout <= 0)
					{

						lCurrOffsetHz = MULTL_CLKGetSampleOffsetAverage(1, CLK_STATE_5_TIMEOUT, &lCurrOffsetNum);

						GLOBAL_TRACE(("Current FreqErr = %eHz, FreqOffsetNum = %d, State = %d\n", lCurrOffsetHz, lCurrOffsetNum, s_pHandle->m_State));

						s_pHandle->m_Timeout = CLK_STATE_5_TIMEOUT + 2;

						s_pHandle->m_DAValue += -(((lCurrOffsetHz > 0)?0.01:-0.01) * s_pHandle->m_REG1Hz);
						MULTL_CLKProtocolPacker();

						if (fabs(lCurrOffsetHz) < 0.01)
						{
							s_pHandle->m_State = 6;
							s_pHandle->m_Timeout = CLK_STATE_6_TIMEOUT + 2;
							//s_pHandle->m_LockToSumOffset = s_pHandle->m_CurSumOffset;
							s_pHandle->m_LockToSumOffset = 0;//修正锁定值，将其调整为0，确保单频网延迟不变
							s_pHandle->m_InitDAValue = s_pHandle->m_DAValue;
							GLOBAL_TRACE(("Lock Value = %d\n", s_pHandle->m_LockToSumOffset));
						}
						else if (fabs(lCurrOffsetHz) < 0.05)
						{
							/*状态不变！继续矫正*/
						}
						else if (fabs(lCurrOffsetHz) < 3.0)
						{
							s_pHandle->m_State = 4;
							s_pHandle->m_Timeout = 0;
						}
						else
						{
							/*异常处理*/
							s_pHandle->m_State = 0;
							s_pHandle->m_Timeout = 0;
							s_pHandle->m_InitDAValue = CLK_DEFAULT_DA_VALUE;
						}

					}
				}


			}
			break;
		case 6:
			{
				MULTL_CLKPrintFunc();
				//MULTL_CLKRecodeCLKOffsetToFile();
				if (MULTL_CLKDetectSyncOK())
				{
					if (s_pHandle->m_Timeout <= 0)
					{

						lCurrOffsetHz = MULTL_CLKGetSampleOffsetAverage(1, CLK_STATE_6_TIMEOUT, &lCurrOffsetNum);

						s_pHandle->m_Timeout = CLK_STATE_6_TIMEOUT + 2;

						GLOBAL_TRACE(("Current FreqErr = %eHz, FreqOffsetNum = %d, LockValue = %d, CurSum = %d, State = %d\n", lCurrOffsetHz, lCurrOffsetNum, s_pHandle->m_LockToSumOffset, s_pHandle->m_CurSumOffset, s_pHandle->m_State));

						//记录下10M时钟的误差累计值

						//if (fabs(lCurrOffsetHz) < 0.02)
						{
							S32 lDeltaValue, lLargeDeltaValue;

							lDeltaValue = 0.002 * s_pHandle->m_REG1Hz;
							lLargeDeltaValue = fabs(lCurrOffsetHz) * s_pHandle->m_REG1Hz;

							//if (lDeltaValue == 0)
							//{
							//	lDeltaValue = 1;
							//}

							if ((s_pHandle->m_CurSumOffset /*- s_pHandle->m_LockToSumOffset*/) > 0)
							{
								/*频率正偏*/
								if (lCurrOffsetHz == 0)
								{
									GLOBAL_TRACE(("Add Offset = -%d, \n", lDeltaValue));
									s_pHandle->m_DAValue += - lDeltaValue;
								}
								else if (lCurrOffsetHz > 0)
								{
									GLOBAL_TRACE(("Add Offset = -%d\n", lLargeDeltaValue));
									s_pHandle->m_DAValue += - lLargeDeltaValue - lDeltaValue;
								}
								MULTL_CLKProtocolPacker();
							}
							else if ((s_pHandle->m_CurSumOffset /*- s_pHandle->m_LockToSumOffset*/) < 0)
							{
								if (lCurrOffsetHz == 0)
								{
									GLOBAL_TRACE(("Add Offset = +%d\n", lDeltaValue));
									s_pHandle->m_DAValue += lDeltaValue;
								}
								else if (lCurrOffsetHz < 0)
								{
									GLOBAL_TRACE(("Add Offset = +%d\n", lLargeDeltaValue));
									s_pHandle->m_DAValue += lLargeDeltaValue + lDeltaValue;
								}
								MULTL_CLKProtocolPacker();
							}
						}
						//else if (fabs(lCurrOffsetHz) < 0.03)
						//{
						//	s_pHandle->m_State = 5;
						//	s_pHandle->m_Timeout = 0;
						//}
						//else if (fabs(lCurrOffsetHz) < 1.0)
						//{
						//	s_pHandle->m_State = 4;
						//	s_pHandle->m_Timeout = 0;
						//}
						//else 
						//{
						//	/*异常处理*/
						//	s_pHandle->m_State = 0;
						//	s_pHandle->m_Timeout = 0;
						//	s_pHandle->m_InitDAValue = CLK_DEFAULT_DA_VALUE;
						//}
					}
				}
			}
			break;
		default:
			{
				GLOBAL_TRACE(("Error Status = %d\n", s_pHandle->m_State));
			}
			break;
		}
	}






}

/*设置给DA的值，CLK_DEFAULT_DA_VALUE为量程中点*/
void MULTL_CLKProtocolPacker(void)
{
	S32 lLen;
	U8 plCMDBuf[1024], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

#ifdef CLK_SYNC_DEBUG
	GLOBAL_TRACE(("Current DAValue = 0x%.4X\n", s_pHandle->m_DAValue));
#endif

	{
		GLOBAL_MSB8_EC(plTmpBuf, 0x35/*串口协议*/, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
		GLOBAL_BYTES_SC(plTmpBuf, 2, lLen);
		GLOBAL_MSB16_EC(plTmpBuf, s_pHandle->m_DAValue, lLen);
	}

#ifdef CLK_SYNC_DEBUG_PROTOCOL
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);
}

void MULTL_CLKProtocolResetFPGACounterPacker(void)
{
	S32 lLen;
	U8 plCMDBuf[1024], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	{
		GLOBAL_MSB8_EC(plTmpBuf, 0x35/*串口协议*/, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	}

	GLOBAL_TRACE(("Reset FPGA CLK Counter!!!!!!!!!!!!!\n"));
#ifdef CLK_SYNC_DEBUG_PROTOCOL
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);
}



/*打印函数*/
void MULTL_CLKPrintFunc(void)
{
//#ifdef CLK_SYNC_DEBUG
//	F64 lAvl1 = MULTL_CLKGetSampleOffsetAverage(1, CLK_STATE_4_TIMEOUT, NULL);
//	F64 lAvl2 = MULTL_CLKGetSampleOffsetAverage(1, CLK_STATE_5_TIMEOUT, NULL);
//	GLOBAL_TRACE(("State4 Average = %f Hz; State5 Average = %f Hz\n",  lAvl1, lAvl2));
//#endif
}

void MULTL_CLKRecodeCLKOffsetToFile(void)
{
	GLOBAL_FD lFD;
	lFD = GLOBAL_FOPEN("/tmp/clk.txt", "a+");
	if (lFD)
	{
		
		CHAR_T plBuf[256];
		TIME_T lTimeT;
		GLOBAL_TIME(&lTimeT);
		CAL_StringU32TimeToStr(plBuf, lTimeT);
		GLOBAL_SPRINTF((&plBuf[GLOBAL_STRLEN(plBuf)], ", %d\n", s_pHandle->m_CurSumOffset));
		GLOBAL_TRACE(("%s\n",plBuf));
		GLOBAL_FWRITE(plBuf, GLOBAL_STRLEN(plBuf), 1, lFD);
		GLOBAL_FCLOSE(lFD);
	}
	else
	{
		GLOBAL_TRACE(("Open Failed!!!!!!\n"));
	}
}


#endif
/*EOF*/
