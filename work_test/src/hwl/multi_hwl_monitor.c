/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_hwl_igmp.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "multi_hwl_monitor.h"
#include "multi_hwl_tags.h"
#include "mpeg2_micro.h"

#ifdef SUPPORT_NEW_HWL_MODULE

/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
//#define HWL_DEBUG_PRINT_CHN_BITRATE
//#define HWL_DEBUG_PRINT_STATISTIC


#define	HWL_MAX_V_NUM													(0)
#define	HWL_MAX_ETH_NUM													(2)
#define	HWL_MONITOR_MAX_SUB_CHANNEL_NUM									(250)
#define HWL_HEART_BEAT_LOST_COUNT_MAX									(8)	
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	U32				m_Bitrate;
}HWL_SubMonitor;

typedef struct  
{
	U32				m_Bitrate;
	HWL_SubMonitor  m_pSubChn[HWL_MONITOR_MAX_SUB_CHANNEL_NUM];
}HWL_ChnMonitor;



typedef struct  
{
	S32				m_HearbeatLostCount;

	U32				m_TotalInput;
	U32				m_TotalOutput;

	HWL_ChnMonitor  m_pInChn[HWL_MAX_CHANNEL_NUM];
	HWL_ChnMonitor  m_pOutChn[HWL_MAX_CHANNEL_NUM];

	U8				m_pETHStatus[HWL_MAX_ETH_NUM];

	U32				m_ModuleStatus;/*模块异常复位指示：BIT0，DDR0；BIT1，DDR1。清除需要发送清除命令（0x24）*/
	U32				m_ModulatorBufferStatus;/*0~31bit分别对应32个通道*/
	F32				m_Temperature;

	F32				m_Volt[HWL_MAX_V_NUM];

	U32				m_ISRPacketNum;
	U32				m_ISRBitrate;

	U32				m_FPGAToCPUBytes;
	U32				m_FPGAToCPUBitrate;
	U32				m_CPUToFPGABytes;
	U32				m_CPUToFPGABitrate;

	S32				m_MonitorTimeout;

	HWL_MonitorPIDStatArray	m_PIDStatArray;
	HWL_MonitorIPStatArray	m_IPStatArray;

	HANDLE32		m_Lock;

	HWL_HWInfo		m_HWInfo;
}HWL_Monitor;

/* Private Variables (static)-------------------------------------------------- */
static HWL_Monitor	s_HWLMonitor;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */


/* 心跳信息 --------------------------------------------------------------------------------------------------------------------------------------- */
/*心跳发送*/
void HWL_MonitorHeartBeateSend(void)
{
	S32 lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, HWL_HEART_BEAT_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);


	HWL_FPGAWrite(plCMDBuf, lLen);
}

/*获取心跳错误*/
BOOL HWL_MonitorGetHeartBeatError(void)
{
	BOOL lRet = TRUE;
	HWL_Monitor *plMonitor;

	plMonitor = &s_HWLMonitor;

	if (plMonitor->m_HearbeatLostCount < HWL_HEART_BEAT_LOST_COUNT_MAX)
	{
		lRet = FALSE;
	}
	else
	{
		plMonitor->m_HearbeatLostCount = 0;
	}
	return lRet;
}


/*心跳分析*/
void HWLL_MonitorHeartBeatParser(HWL_MsgHead *pMsgHead, HWL_Monitor *pMonitor)
{
	U8 *plTmpBuf;
	if ((pMsgHead->m_Tag == HWL_HEART_BEAT_TAG) && (pMsgHead->m_MsgLen == 2))
	{
		U32 lTmpValue;
		plTmpBuf = pMsgHead->m_pPayload;
		GLOBAL_MSB32_D(plTmpBuf, lTmpValue);
		pMonitor->m_TotalInput = lTmpValue * MPEG2_TS_PACKET_SIZE * 8;
		GLOBAL_MSB32_D(plTmpBuf, lTmpValue);
		pMonitor->m_TotalOutput = lTmpValue * MPEG2_TS_PACKET_SIZE * 8;

#if HWL_DEBUG_PRINT_HEAR_BEAT
		GLOBAL_TRACE(("Total In = %d, Out = %d\n", pMonitor->m_TotalInput, pMonitor->m_TotalOutput));
#endif

		pMonitor->m_HearbeatLostCount = 0;//心跳接收到！

	}
	else
	{
		GLOBAL_TRACE(("Parameter Error! Tag = %d, Len = %d\n", pMsgHead->m_Tag, pMsgHead->m_MsgLen));
	}
}

/* 硬件信息 和 状态 ------------------------------------------------------------------------------------------------------------------------------- */


/*获取硬件信息（芯片ID和FPGA日期）*/
BOOL HWLL_MonitorHWInfoRequst(U8 Mark)
{
	S32 lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, HWL_HW_INFO_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, Mark, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);

	return TRUE;
}

/*硬件信息分析*/
void HWLL_MonitorHWInfoParser(HWL_MsgHead *pMsgHead, HWL_HWInfo *pHWInfo)
{
	U8 *plTmpBuf;
	plTmpBuf = pMsgHead->m_pPayload;
	if ((pMsgHead->m_Tag == HWL_HW_INFO_TAG) && (pMsgHead->m_MsgLen >= 2))
	{
		U8 lTmpValue;
		S32 lFpgaYear, lFpgaMonth, lFpgaDay;
		GLOBAL_MSB32_D(plTmpBuf, pHWInfo->m_ChipID);
#if HWL_DEBUG_PRINT_MAIN_DEV_HW_INFO
		GLOBAL_TRACE(("Chip ID = 0x%.8X\n", pHWInfo->m_ChipID));
#endif

		GLOBAL_MSB8_D(plTmpBuf, lTmpValue);
#if HWL_DEBUG_PRINT_MAIN_DEV_HW_INFO
		GLOBAL_TRACE(("MsgDevType =  0x%.8X, Local =  0x%.8X\n", lTmpValue, pHWInfo->m_DeviceType));
#endif
		GLOBAL_MSB8_D(plTmpBuf, lTmpValue);
//		GLOBAL_SPRINTF((pHWInfo->m_pHardVersion, "%.2d.%.2d", ((lTmpValue >> 4) & 0x0F), (lTmpValue & 0x0F)));
//
//#if HWL_DEBUG_PRINT_MAIN_DEV_HW_INFO
//		GLOBAL_TRACE(("String = [%s], Value = 0x%.2X\n", pHWInfo->m_pHardVersion, lTmpValue));
//#endif

		lFpgaYear = ((plTmpBuf[0] >> 1) & 0x0F) + 2008;
		lFpgaMonth = ((plTmpBuf[0] << 3) & 0x08) | ((plTmpBuf[1] >> 5) & 0x07);
		lFpgaDay = plTmpBuf[1] & 0x1F;
		GLOBAL_BYTES_S(plTmpBuf, 2);

		GLOBAL_SPRINTF((pHWInfo->m_pFPGARelease, "%d-%.2d-%.2d", lFpgaYear, lFpgaMonth, lFpgaDay));
#if HWL_DEBUG_PRINT_MAIN_DEV_HW_INFO
		GLOBAL_TRACE(("String = [%s]\n", pHWInfo->m_pFPGARelease));
#endif


	}
	else
	{
		GLOBAL_TRACE(("Error!\n"));
	}
}

/*硬件信息获取接口*/
void HWL_MonitorHWInfoGet(HWL_HWInfo *pHWInfo)
{
	S32 lWaitHWInfoCount = 15;
	while((s_HWLMonitor.m_HWInfo.m_ChipID == 0x00) && (lWaitHWInfoCount > 0))
	{
		GLOBAL_TRACE(("Wait CHIP ID\n"));
		HWLL_MonitorHWInfoRequst(0x00);
		PFC_TaskSleep(500);
		lWaitHWInfoCount--;
	}

	GLOBAL_TRACE(("CHIP ID = 0x%08X, Count Left = %d!!!!!!!!!!!!\n", s_HWLMonitor.m_HWInfo.m_ChipID, lWaitHWInfoCount));

	GLOBAL_MEMCPY(pHWInfo, &s_HWLMonitor.m_HWInfo, sizeof(HWL_HWInfo));
}

/*获取以太网通道的个数*/
S32 HWL_MonitorHWInfoETHChnNum(HWL_HWInfo *pHWInfo)
{
	S32 lRet = 0;
	S32 i;

	for (i = 0; i < pHWInfo->m_InChnNum; i++)
	{
		if (pHWInfo->m_pInChn[i].m_Type == HWL_CHANNEL_TYPE_IP)
		{
			lRet++;
		}
	}

	for (i = 0; i < pHWInfo->m_OutChnNum; i++)
	{
		if (pHWInfo->m_pOutChn[i].m_Type == HWL_CHANNEL_TYPE_IP)
		{
			lRet++;
		}
	}

	return lRet;
}

/*硬件状态*/
void HWLL_MonitorHWStatusParser(HWL_MsgHead *pMsgHead, HWL_Monitor *pMonitor)
{
	S32 i;
	U32 lTmpValue;
	U8 *plTmpBuf;
	plTmpBuf = pMsgHead->m_pPayload;
	GLOBAL_MSB8_D(plTmpBuf, lTmpValue);
	for (i = 0; i < HWL_MAX_ETH_NUM ;i++)
	{
		pMonitor->m_pETHStatus[i] = ((lTmpValue >> (i * 2)) & 0x02);
#if HWL_DEBUG_PRINT_SUB_DEV_HW_MONITOR
		GLOBAL_TRACE(("ETH %d Status %d, Value = 0x%02X\n", i, pMonitor->m_pETHStatus[i], lTmpValue));
#endif
	}
	GLOBAL_MSB8_D(plTmpBuf, lTmpValue);
	pMonitor->m_Temperature = CAL_DS1775DataToTemperature(lTmpValue & 0xFF, 0.5);
	GLOBAL_MSB16_D(plTmpBuf, lTmpValue);//保留
	GLOBAL_MSB32_D(plTmpBuf, pMonitor->m_ModuleStatus);

}

/*获取DDR错误状态*/
U32 HWL_MonitorGetMainFPGAModuleStatus(void)
{
	return s_HWLMonitor.m_ModuleStatus;
}

/*模块复位状态清空*/
void HWL_MonitorModuleResetSend(U32 ModuleResetMark)
{
	S32 lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, HWL_MODULE_RESET_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	GLOBAL_MSB32_EC(plTmpBuf, ModuleResetMark, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}


/*获取以太网连接状态*/
BOOL HWL_MonitorGetETHLinkStatus(S32 Slot)
{
	BOOL lRet = FALSE;
	HWL_Monitor *plMonitor;
	plMonitor = &s_HWLMonitor;
	if (GLOBAL_CHECK_INDEX(Slot, HWL_MAX_ETH_NUM))
	{
		lRet = (plMonitor->m_pETHStatus[Slot] == 0x02);
	}
	return lRet;
}

/*获取子板温度（DS1775），返回值为实际温度值的1000倍*/
F32 HWL_MonitorGetTemperature(void)
{
	F32 lRet = 0;
	HWL_Monitor *plMonitor;
	plMonitor = &s_HWLMonitor;
	lRet = plMonitor->m_Temperature;
	return lRet;
}

/* 通道码率数据 ------------------------------------------------------------------------------------------------------------------------------------ */

/*通道码率数据申请*/
void HWL_MonitorChnBitrateRequest(S16 ChnIndex, BOOL bInput)
{
	S32 lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	U8 lIOType;

	lIOType = bInput?0x01:0x02;
	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, HWL_CHN_BITRATE_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, lIOType, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, ChnIndex, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}

/*通道码率数据分析*/
void HWLL_MonitorChnBitrateParser(HWL_MsgHead *pMsgHead, HWL_Monitor *pMonitor)
{
	if ((pMsgHead->m_Tag == HWL_CHN_BITRATE_TAG) && (pMsgHead->m_MsgLen > 0) && (pMsgHead->m_MsgLen <= 251))
	{
		U8 *plTmpBuf;
		U16 lBitrateValue, lSubIndex;
		S16 lPhyIndex, lChnIndex;
		S32 i;
		BOOL lbInput;
		HWL_Monitor *plMonitor;
		HWL_HWInfo *plHWInfo;
		HWL_ChannelInfo *plChnInfo;
		HWL_ChnMonitor *plChnMonitor;

		plMonitor = pMonitor;
		plHWInfo = &pMonitor->m_HWInfo;



		{
			lbInput = (pMsgHead->m_Flag1 == 0x01)?TRUE:FALSE;//这里需要注意旧FPGA不一定完全按照协议来实现。
			lPhyIndex = pMsgHead->m_Flag2;

			/*这里ChnInd还需要调整！！！！*/
			lChnIndex = lPhyIndex;


			plChnMonitor = NULL;
			plChnInfo = NULL;
			if (lbInput)
			{
				if (GLOBAL_CHECK_INDEX(lChnIndex, plHWInfo->m_InChnNum))
				{
					plChnMonitor = &plMonitor->m_pInChn[lChnIndex];
					plChnInfo = &plHWInfo->m_pInChn[lChnIndex];
				}
			}
			else
			{
				if (GLOBAL_CHECK_INDEX(lChnIndex, plHWInfo->m_OutChnNum))
				{
					plChnMonitor = &plMonitor->m_pOutChn[lChnIndex];
					plChnInfo = &plHWInfo->m_pOutChn[lChnIndex];
				}
			}

#if defined(HWL_DEBUG_PRINT_CHN_BITRATE)
			GLOBAL_TRACE(("ChnInd = %d, InputMark = %d\n", lChnIndex, lbInput));
#endif

			if(plChnMonitor && plChnInfo)
			{
				plTmpBuf = pMsgHead->m_pPayload;

				for (i = 0; i < pMsgHead->m_MsgLen; i++)
				{
					GLOBAL_MSB16_D(plTmpBuf, lSubIndex);
					GLOBAL_MSB16_D(plTmpBuf, lBitrateValue);

					if (GLOBAL_CHECK_INDEX(lSubIndex, plChnInfo->m_SubChnNum))
					{
						plChnMonitor->m_pSubChn[lSubIndex].m_Bitrate = lBitrateValue * MPEG2_TS_PACKET_SIZE * 2 * 8;
#if defined(HWL_DEBUG_PRINT_CHN_BITRATE)
						GLOBAL_TRACE(("Sub Chn %d ,Bitrate = %.3f MBps\n", lSubIndex, ((F64)plChnMonitor->m_pSubChn[lSubIndex].m_Bitrate) / 1000000));
#endif
					}
					else
					{

						if (lSubIndex >= 250)
						{
							plChnMonitor->m_Bitrate = lBitrateValue * MPEG2_TS_PACKET_SIZE * 16 * 8;
#if defined(HWL_DEBUG_PRINT_CHN_BITRATE)
							GLOBAL_TRACE(("Total Chn [%d],Bitrate = %.3f MBps\n", lChnIndex, ((F64)plChnMonitor->m_Bitrate) / 1000000));
#endif
						}
					}

				}
			}
			else
			{
				GLOBAL_TRACE(("Chn Error! ChnInd = %d, bInput = %d, Phy = 0x%x, IOType = 0x%x\n", lChnIndex, lbInput, pMsgHead->m_Flag2, pMsgHead->m_Flag1));
			}
		}
	}
}

/*获取通道/子通道码率信息，单位bps；ChnIndex == -1时为当前子板的总码率，SubIndex == -1时为当前通道总码率*/
U32 HWL_MonitorChnBitrateInfoGet(S16 ChnIndex, S16 SubIndex, BOOL bInput)
{
	U32 lBitrate;
	HWL_Monitor *plMonitor;

	lBitrate = 0;
	plMonitor = &s_HWLMonitor;

	if (bInput)
	{
		if (GLOBAL_CHECK_INDEX(ChnIndex, plMonitor->m_HWInfo.m_InChnNum))
		{
			if (GLOBAL_CHECK_INDEX(SubIndex, plMonitor->m_HWInfo.m_pInChn[ChnIndex].m_SubChnNum))
			{
				lBitrate = plMonitor->m_pInChn[ChnIndex].m_pSubChn[SubIndex].m_Bitrate;
			}
			else
			{
				lBitrate = plMonitor->m_pInChn[ChnIndex].m_Bitrate;
			}
		}
		else
		{
			lBitrate = plMonitor->m_TotalInput;
		}
	}
	else
	{
		if (GLOBAL_CHECK_INDEX(ChnIndex, plMonitor->m_HWInfo.m_OutChnNum))
		{
			if (GLOBAL_CHECK_INDEX(SubIndex, plMonitor->m_HWInfo.m_pOutChn[ChnIndex].m_SubChnNum))
			{
				lBitrate = plMonitor->m_pOutChn[ChnIndex].m_pSubChn[SubIndex].m_Bitrate;
			}
			else
			{
				lBitrate = plMonitor->m_pOutChn[ChnIndex].m_Bitrate;
			}
		}
		else
		{
			lBitrate = plMonitor->m_TotalOutput;
		}
	}
	return lBitrate;
}

/* PID码率数据 ------------------------------------------------------------------------------------------------------------------------------------ */

/*PID码率查询配置*/
void HWL_MonitorPIDStatisticConfig(S16 ChnIndex, S16 SubIndex, BOOL bInput)
{
	S32 lLen;
	S32 lTmpValue;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

#ifdef HWL_DEBUG_PRINT_STATISTIC
	GLOBAL_TRACE(("CHN = %d, SUB = %d, bInput = %d\n", ChnIndex, SubIndex, bInput));
#endif

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, HWL_STATISTICS_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);//必须设置为1
	if (bInput)
	{
		GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	}
	else
	{
		GLOBAL_MSB8_EC(plTmpBuf, 0x02, lLen);
	}

	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
#ifdef ENCODER_CARD_PLATFORM
	lTmpValue = ChnIndex * 256 + SubIndex;
	GLOBAL_MSB24_EC(plTmpBuf, lTmpValue, lLen);
#else
	GLOBAL_MSB8_EC(plTmpBuf, ChnIndex, lLen);
	GLOBAL_MSB16_EC(plTmpBuf, SubIndex, lLen);
#endif


#ifdef HWL_DEBUG_PRINT_STATISTIC
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);
}


/*PID码率数据申请*/
void HWL_MonitorPIDStatisticResultReq(void)
{
	S32 lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, HWL_STATISTICS_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);//必须设置为1
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

#ifdef HWL_DEBUG_PRINT_STATISTIC
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);
}

/*PID码率数据分析*/
void HWLL_MonitorPIDStatisticResultParser(HWL_MsgHead *pMsgHead, HWL_MonitorPIDStatArray *pArray)
{
	if ((pMsgHead->m_Tag == HWL_STATISTICS_TAG) && (pMsgHead->m_MsgLen > 0) && (pMsgHead->m_MsgLen <= 250))
	{
		S32 i, lItemNum;
		U16 lPID, lBitrateValue;
		U8 *plTmpBuf;


		lItemNum = pMsgHead->m_MsgLen;
		plTmpBuf = pMsgHead->m_pPayload;

		if (PAL_CriticalEnter(s_HWLMonitor.m_Lock, GLOBAL_INVALID_INDEX))
		{
			pArray->m_Number = 0;

			for (i = 0; i < lItemNum; i++)
			{
				GLOBAL_MSB16_D(plTmpBuf, lPID);
				GLOBAL_MSB16_D(plTmpBuf, lBitrateValue);
				if (lPID == 0xFFFF)
				{
					//总码率。
					pArray->m_Bitrate = lBitrateValue * 2 * 188 * 8;
				}
				else
				{
					if (pArray->m_Number < HWL_PID_STATISTICS_MAX_NUM)
					{
						pArray->m_pNode[pArray->m_Number].m_PID = lPID;//FPGA过来的数据时按PID自然排序的！
						pArray->m_pNode[pArray->m_Number].m_Bitrate = lBitrateValue * 2 * 188 * 8;
						pArray->m_Number++;
					}
				}
			}
			PAL_CriticalLeave(s_HWLMonitor.m_Lock);
		}
		else
		{
			GLOBAL_TRACE(("Lock Failed!!!!!!!!!!!!!!\n"));
		}

#ifdef HWL_DEBUG_PRINT_STATISTIC
		for (i = 0; i < pArray->m_Number; i++)
		{
			GLOBAL_TRACE(("PID = %.4d, Bitrate = %d\n", pArray->m_pNode[i].m_PID, pArray->m_pNode[i].m_Bitrate));
		}
#endif

	}
	else
	{
		GLOBAL_TRACE(("Parameter Error!\n"));
	}
}


/*清空PID码率信息*/
void HWL_MonitorPIDStatisticResultClean(void)
{
	if (PAL_CriticalEnter(s_HWLMonitor.m_Lock, GLOBAL_INVALID_INDEX))
	{
		GLOBAL_ZEROMEM(&s_HWLMonitor.m_PIDStatArray, sizeof(HWL_MonitorPIDStatArray));
		PAL_CriticalLeave(s_HWLMonitor.m_Lock);
	}
}


/*获取PID码率信息*/
void HWL_MonitorPIDStatisticResultGet(HWL_MonitorPIDStatArray *pArray)
{
	if (PAL_CriticalEnter(s_HWLMonitor.m_Lock, GLOBAL_INVALID_INDEX))
	{
		GLOBAL_MEMCPY(pArray, &s_HWLMonitor.m_PIDStatArray, sizeof(HWL_MonitorPIDStatArray));
		PAL_CriticalLeave(s_HWLMonitor.m_Lock);
	}
}


/* IP码率数据 ------------------------------------------------------------------------------------------------------------------------------------ */

/*输入IP数据包码率*/

/*设置需要统计的输入端口号，发出该命令的同时以前的统计结果会被自动清空*/
void HWL_MonitorIPStatisticConfig(S16 ChnIndex, BOOL bInput)
{
	S32 lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, HWL_STATISTICS_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x10, lLen);//固定值，区分PID和IP统计功能
	GLOBAL_MSB8_EC(plTmpBuf, ChnIndex, lLen);

	GLOBAL_BYTES_SC(plTmpBuf, 4, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}

/*查询数据申请*/
void HWL_MonitorIPStatisticResultReq(void)
{
	S32 lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, HWL_STATISTICS_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x10, lLen);//固定值，区分PID和IP统计功能
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);//本应该是物理通道号，新协议忽略该值。一次仅允许查询一个通道！

	HWL_FPGAWrite(plCMDBuf, lLen);
}

/*查询数据分析*/
void HWLL_MonitorIPStatisticResultParser(HWL_MsgHead *pMsgHead, HWL_MonitorIPStatArray *pArray)
{
	if ((pMsgHead->m_Tag == HWL_STATISTICS_TAG) && (pMsgHead->m_MsgLen > 0) && (pMsgHead->m_MsgLen <= 250))
	{
		S32 i, lItemNum, lStartOffset;
		U32 lIPAddr;
		U16 lPort, lBitrateValue;
		U8 *plTmpBuf;


		lItemNum = (pMsgHead->m_MsgLen - 1) / 2;
		plTmpBuf = pMsgHead->m_pPayload;

		GLOBAL_BYTES_S(plTmpBuf, 2);
		GLOBAL_MSB16_D(plTmpBuf, lBitrateValue);//总码率

		pArray->m_Bitrate = lBitrateValue * 14 * 188 * 8;//总码率，这里可能包括没有被端口接收的数据包的码率。

		//HWLL_LockEnterData();

		lStartOffset = (pMsgHead->m_Flag1 - 0x10) * 124;//最大两帧数据，一帧124个。

		if (pArray->m_Number < lStartOffset + lItemNum)
		{
			pArray->m_Number = lStartOffset + lItemNum;
		}

		for (i = 0; i < lItemNum; i++)
		{
			GLOBAL_MSB32_D(plTmpBuf, lIPAddr);
			GLOBAL_MSB16_D(plTmpBuf, lPort);
			GLOBAL_MSB16_D(plTmpBuf, lBitrateValue);

			if (lStartOffset < HWL_IP_STATISTICS_MAX_NUM)
			{
				pArray->m_pNode[lStartOffset].m_IPAddr = lIPAddr;//FPGA过来的数据时按IP端口号自然排序的！
				pArray->m_pNode[lStartOffset].m_Port = lPort;
				pArray->m_pNode[lStartOffset].m_Bitrate = lBitrateValue * 14 * 188 * 8;
				lStartOffset++;
			}
		}

		//HWLL_LockExitData();

	}
	else
	{
		GLOBAL_TRACE(("Parameter Error!\n"));
	}
}


/*清空IP码率信息*/
void HWL_MonitorIPStatisticResultClean(void)
{
	GLOBAL_ZEROMEM(&s_HWLMonitor.m_IPStatArray, sizeof(HWL_MonitorIPStatArray));
}

/*获取IP码率信息*/
void HWL_MonitorIPStatisticResultGet(HWL_MonitorIPStatArray *pArray)
{
	GLOBAL_MEMCPY(pArray, &s_HWLMonitor.m_IPStatArray, sizeof(HWL_MonitorIPStatArray));
}



/*头部分析函数！----------------------------------------------------------------------------------------------------------------------------------*/
void HWLL_MsgHeadParser(HWL_MsgHead *pMsgHead, U8 *pDataBuf, S32 DataSize)
{
	GLOBAL_MSB8_D(pDataBuf, pMsgHead->m_Tag);
	GLOBAL_MSB8_D(pDataBuf, pMsgHead->m_MsgLen);
	GLOBAL_MSB8_D(pDataBuf, pMsgHead->m_Flag1);
	GLOBAL_MSB8_D(pDataBuf, pMsgHead->m_Flag2);
	pMsgHead->m_pPayload = pDataBuf;
	pMsgHead->m_PayloadSize = DataSize - 4;
}


/*ISR统计*/
void HWL_MonitorPlusInserterPacketNum(S32 Num)
{
	s_HWLMonitor.m_ISRPacketNum += Num;
}
/*获取插入器码率*/
U32 HWL_MonitorInserterBitrateGet(void)
{
	return s_HWLMonitor.m_ISRBitrate;
}


/*内部通讯统计*/
void HWL_MonitorPlusICPByteNum(S32 ByteNum, BOOL bFPGAToCPU)
{
	if (bFPGAToCPU)
	{
		s_HWLMonitor.m_FPGAToCPUBytes += ByteNum;
	}
	else
	{
		s_HWLMonitor.m_CPUToFPGABytes += ByteNum;
	}
}

/*获取内部通讯码率*/
S32 HWL_MonitorInternalCOMBitrateGet(BOOL bFPGAToCPU)
{
	if (bFPGAToCPU)
	{
		return s_HWLMonitor.m_FPGAToCPUBitrate;
	}
	else
	{
		return s_HWLMonitor.m_CPUToFPGABitrate;
	}
	return 0;
}

/*模块基本接口 ------------------------------------------------------------------------------------------------------------------------------------ */
/*模块初始化函数！*/
void HWL_MonitorInitate(void)
{
	GLOBAL_ZEROMEM(&s_HWLMonitor, sizeof(HWL_Monitor));

	HWL_PlatformInfoSet(&s_HWLMonitor.m_HWInfo);

	s_HWLMonitor.m_Lock = PAL_CriticalInitiate();
}

/*轮询函数，预计1S轮询一次*/
void HWL_MonitorAccess(S32 Duration)
{
	S32 i;
	HWL_ChannelInfo *plChnInfo;
	HWL_Monitor *plMonitor;

	plMonitor = &s_HWLMonitor;

	for (i = 0; i < s_HWLMonitor.m_HWInfo.m_InChnNum; i++)
	{
		plChnInfo = &s_HWLMonitor.m_HWInfo.m_pInChn[i];
		HWL_MonitorChnBitrateRequest(i, TRUE);
	}

	for (i = 0; i < s_HWLMonitor.m_HWInfo.m_OutChnNum; i++)
	{
		plChnInfo = &s_HWLMonitor.m_HWInfo.m_pOutChn[i];
		HWL_MonitorChnBitrateRequest(i, FALSE);
	}

	HWLL_MonitorHWInfoRequst(0x01);//主板硬件监控，获取以太网状态等参数！

	HWL_MonitorHeartBeateSend();//心跳
	s_HWLMonitor.m_HearbeatLostCount++;


	s_HWLMonitor.m_ISRBitrate = (s_HWLMonitor.m_ISRPacketNum * 8 * MPEG2_TS_PACKET_SIZE) * 1000 / Duration;
	s_HWLMonitor.m_ISRPacketNum = 0;

	s_HWLMonitor.m_FPGAToCPUBitrate = s_HWLMonitor.m_FPGAToCPUBytes * 8 * 1000 / Duration;
	s_HWLMonitor.m_FPGAToCPUBytes = 0;

	//GLOBAL_TRACE(("FPGA To CPU Total Bitrete = %d Bps, Total Bytes = %d\n", s_HWLHandle.m_FPGAToCPUBitrate, s_HWLHandle.m_FPGAToCPUTotalBytes));

	s_HWLMonitor.m_CPUToFPGABitrate = s_HWLMonitor.m_CPUToFPGABytes * 8 * 1000 / Duration;
	s_HWLMonitor.m_CPUToFPGABytes = 0;

}


/*返回信息获取函数，返回TRUE时表示该数据已经被处理！*/
BOOL HWL_MonitorParser(U8 *pData, S32 DataLen)
{
	BOOL lRet = FALSE;

	HWL_MsgHead lMsgHead;

	
	HWLL_MsgHeadParser(&lMsgHead, pData, DataLen);

	if (lMsgHead.m_Tag == HWL_HEART_BEAT_TAG)
	{
		//CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, pData, DataLen);
		HWLL_MonitorHeartBeatParser(&lMsgHead, &s_HWLMonitor);
		lRet = TRUE;
	}
	else if (lMsgHead.m_Tag == HWL_CHN_BITRATE_TAG)
	{
#ifdef HWL_DEBUG_PRINT_CHN_BITRATE
		CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, pData, DataLen);
#endif
		HWLL_MonitorChnBitrateParser(&lMsgHead, &s_HWLMonitor);
		lRet = TRUE;
	}
	else if (lMsgHead.m_Tag == HWL_STATISTICS_TAG)
	{
#ifdef HWL_DEBUG_PRINT_STATISTIC
		CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, pData, DataLen);
#endif
		if (lMsgHead.m_Flag1 == 0x01)
		{
			HWLL_MonitorPIDStatisticResultParser(&lMsgHead, &s_HWLMonitor.m_PIDStatArray);
		}
		else
		{
			HWLL_MonitorIPStatisticResultParser(&lMsgHead, &s_HWLMonitor.m_IPStatArray);
		}
		lRet = TRUE;
	}
	else if (lMsgHead.m_Tag == HWL_HW_INFO_TAG)
	{
		//CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, pData, DataLen);
		if (lMsgHead.m_Flag1 == 0x00)
		{
			HWLL_MonitorHWInfoParser(&lMsgHead, &s_HWLMonitor.m_HWInfo);
		}
		else
		{
			HWLL_MonitorHWStatusParser(&lMsgHead, &s_HWLMonitor);
		}
		lRet = TRUE;
	}
	return lRet;
}


#endif


/*EOF*/
