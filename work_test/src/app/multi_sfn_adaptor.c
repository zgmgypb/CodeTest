/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_tsp.h"
#include "TunerDemod.h"

#ifdef SUPPORT_SFN_ADAPTER
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
//#define SFNA_DEBUG
#define SFN_SIP_PACKET_PID			(0x15)
#define SFN_SAT_NULL_PACKET_PID		(8190)
#define SFN_STATUS_TAG				(0x33)
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */


typedef struct  
{
	/*设置参数*/
	MULT_SFNA_Param			m_SFNParam;

	/*单频网适配器控制参数和状态*/
	MULT_SFNA_Status		m_SFNStatus;

	CHAR_T					m_pSFNTmpInfo[1024];

	/*SIP包过滤*/
	U32						m_FilterID;

	MULT_Handle				*m_pMainHandle;

}MULT_SFNA_HANDLE;

/* Private Variables (static)-------------------------------------------------- */
static MULT_SFNA_HANDLE s_SFNHandle;
/* Private Function prototypes ------------------------------------------------ */
static void MULTL_SFNAProtocolParser(MULT_SFNA_Status *pStatus, U8 *pData, S32 DataSize);
static void MULTL_SFNAProtocolPacker(MULT_SFNA_Param *pParam, F64 CurBitrate);
static void MULTL_SFNAProtocolSIPPacker(S32 CHNInd, S32 SlotInd, U32 MaxDelay, TD_DTMBParameter *pDTMBParam, MULT_SFNA_INDV_SIP *pINDVInfo, S32 CCInd, BOOL bSATMode);
static S32 MULTL_SFNASIPPacker(U8 *pTsBuf, U32 MaxDelay, MULT_SFNA_CMN_SIP *pSIPInfo, MULT_SFNA_INDV_SIP *pINDVInfo, S32 CCInd, BOOL bCRC32);
static void MULTL_SFNAPrinter(TD_DTMBParameter *pTDDTMBParam, MULT_SFNA_INDV_SIP *pSFNINDVSIP);
static void MULTL_SFNASIPParamChange(TD_DTMBParameter *pTDDTMBParam, MULT_SFNA_CMN_SIP *pSFNSISIP, BOOL bToSFNParam);
/* Functions ------------------------------------------------------------------ */
/* SFNA API函数 ------------------------------------------------------------------------------------------------------------------------------------*/
void MULT_SFNAInitiate(MULT_Handle *pHandle)
{
	GLOBAL_ZEROMEM(&s_SFNHandle, sizeof(MULT_SFNA_HANDLE));

	s_SFNHandle.m_pMainHandle = pHandle;

	s_SFNHandle.m_SFNParam.m_bEnableSatSFN = FALSE;
	s_SFNHandle.m_SFNParam.m_SatSFNNullPacketPID = SFN_SAT_NULL_PACKET_PID;

	/*默认参数设置为常用模式7*/
	s_SFNHandle.m_SFNParam.m_DTMBParam.m_CarrierMode = TD_DTMB_CARRIER_MODE_SINGLE;
	s_SFNHandle.m_SFNParam.m_DTMBParam.m_CodeRate = TD_DTMB_CODE_RATE_08;
	s_SFNHandle.m_SFNParam.m_DTMBParam.m_QAMMode = TD_DTMB_QAM_MAP_32QAM;
	s_SFNHandle.m_SFNParam.m_DTMBParam.m_PNMode = TD_DTMB_PN_MODE_595;
	s_SFNHandle.m_SFNParam.m_DTMBParam.m_TIMode = TD_DTMB_TI_MODE_720;
	s_SFNHandle.m_SFNParam.m_DTMBParam.m_bDoublePilot = FALSE;
}

/*参数读写，包括网页提交*/
void MULT_SFNAXMLLoad(mxml_node_t *pXMLRoot, BOOL bPost)
{
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLHolder;
	mxml_node_t *plXMLNodeHolder;

	if (bPost)
	{
		plXMLHolder = pXMLRoot;
	}
	else
	{
		plXMLHolder = mxmlFindElement(pXMLRoot, pXMLRoot, "sfn_adaptor_setting", NULL, NULL, MXML_DESCEND_FIRST);
	}

	if (plXMLHolder)
	{
		s_SFNHandle.m_SFNParam.m_bUse10MClkSynSrc = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "sfn_10m_sel", 10, 0);
		s_SFNHandle.m_SFNParam.m_bUseEx1PPS = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_ex1pps_mark", FALSE);


		s_SFNHandle.m_SFNParam.m_SFNASIMode = MULTL_XMLGetNodeINT(plXMLHolder, "sfn_input_asi_mod", 10);
		s_SFNHandle.m_SFNParam.m_SFNMaxDelay100NS = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "sfn_max_delay_100ns", 10, 0);

		if (s_SFNHandle.m_SFNParam.m_SFNASIMode <= 0 || s_SFNHandle.m_SFNParam.m_SFNASIMode > 3)
		{
			s_SFNHandle.m_SFNParam.m_SFNASIMode = 3;
		}

		s_SFNHandle.m_SFNParam.m_DTMBParam.m_CarrierMode = TD_DTMBParamCarrierModeFromStr(MULTL_XMLGetNodeText(plXMLHolder, "sfn_carrier_mode"));
		s_SFNHandle.m_SFNParam.m_DTMBParam.m_CodeRate = TD_DTMBParamCodeRateFromStr(MULTL_XMLGetNodeText(plXMLHolder, "sfn_code_rate"));
		s_SFNHandle.m_SFNParam.m_DTMBParam.m_QAMMode = TD_DTMBParamConstellationFromStr(MULTL_XMLGetNodeText(plXMLHolder, "sfn_constellation"));
		s_SFNHandle.m_SFNParam.m_DTMBParam.m_PNMode = TD_DTMBParamPNFromStr(MULTL_XMLGetNodeText(plXMLHolder, "sfn_pn_mode"));
		s_SFNHandle.m_SFNParam.m_DTMBParam.m_TIMode = TD_DTMBParamTIModeFromStr(MULTL_XMLGetNodeText(plXMLHolder, "sfn_ti_mode"));
		s_SFNHandle.m_SFNParam.m_DTMBParam.m_bDoublePilot = TD_DTMBParamDPModeFromStr(MULTL_XMLGetNodeText(plXMLHolder, "sfn_dp"));

		s_SFNHandle.m_SFNParam.m_bEnableSatSFN = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_sat_mark", FALSE);
		s_SFNHandle.m_SFNParam.m_SatSFNNullPacketPID = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "sfn_sat_null_pid", 10, 8190);


		{
			S32 lIndex = 0;
			plXMLNodeHolder = mxmlFindElement(plXMLHolder, plXMLHolder, "sfn_indv_node", NULL, NULL, MXML_DESCEND_FIRST);
			while (plXMLNodeHolder)
			{
				if (lIndex < SFN_MAX_INDV_NUM)
				{
					s_SFNHandle.m_SFNParam.m_pINDVArrap[lIndex].m_Info.m_SFNAddr = MULTL_XMLGetNodeUINTDefault(plXMLNodeHolder, "indv_addr", 10, 0);
					s_SFNHandle.m_SFNParam.m_pINDVArrap[lIndex].m_Info.m_SFNSIPIndvFreqOffsetHz = MULTL_XMLGetNodeINTDefault(plXMLNodeHolder, "indv_freq_offset_hz", 10, 0);
					s_SFNHandle.m_SFNParam.m_pINDVArrap[lIndex].m_Info.m_SFNSIPIndviDelay = MULTL_XMLGetNodeUINTDefault(plXMLNodeHolder, "indv_delay", 10, 0);
					s_SFNHandle.m_SFNParam.m_pINDVArrap[lIndex].m_Info.m_SFNSIPIndvPower = MULTL_XMLGetNodeUINTDefault(plXMLNodeHolder, "indv_power", 10, 0);
					s_SFNHandle.m_SFNParam.m_pINDVArrap[lIndex].m_Info.m_SFNSIPIndvPowerMark = MULTL_XMLGetNodeMark(plXMLNodeHolder, "indv_power_mark");
					s_SFNHandle.m_SFNParam.m_pINDVArrap[lIndex].m_ActiveMark = MULTL_XMLGetNodeMark(plXMLNodeHolder, "active_mark");
					lIndex++;
				}

				plXMLNodeHolder = mxmlFindElement(plXMLNodeHolder, plXMLHolder, "sfn_indv_node", NULL, NULL, MXML_NO_DESCEND);
			}
		}
	}
}

void MULT_SFNAXMLSave(mxml_node_t *pXMLRoot, BOOL bStat)
{
	CHAR_T plTmpStr[512];
	S32 lTmpStrLen;
	mxml_node_t *plXMLHolder;
	mxml_node_t *plXMLNodeHolder;

	if (bStat)
	{
		MULT_SFNA_Status *plStatus;
		plStatus = &s_SFNHandle.m_SFNStatus;

		plXMLHolder = pXMLRoot;
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_int1pps_status", plStatus->m_bInt1PPSStatus);
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_ext1pps_status", plStatus->m_bExt1PPSStatus);
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_int10m_status", plStatus->m_bInt10MStatus);
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_ext10m_status", plStatus->m_bExt10MStatus);

#ifdef SUPPORT_CLK_ADJ_MODULE
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_10m_lock_status", MULT_CLKGet10MLockStatus());
		MULTL_XMLAddNodeINT(plXMLHolder, "sfn_10m_symbol_offset", MULT_CLKGetCurSumOffset());
#endif

		MULTL_XMLAddNodeMark(plXMLHolder, "asi0_status", plStatus->m_bTS0Status);
		MULTL_XMLAddNodeMark(plXMLHolder, "asi1_status", plStatus->m_bTS1Status);

	}
	else
	{
		plXMLHolder = mxmlNewElement(pXMLRoot, "sfn_adaptor_setting");
		if (plXMLHolder)
		{
			MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_10m_sel", s_SFNHandle.m_SFNParam.m_bUse10MClkSynSrc);
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_ex1pps_mark", s_SFNHandle.m_SFNParam.m_bUseEx1PPS);

			MULTL_XMLAddNodeINT(plXMLHolder, "sfn_input_asi_mod", s_SFNHandle.m_SFNParam.m_SFNASIMode);
			MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_max_delay_100ns", s_SFNHandle.m_SFNParam.m_SFNMaxDelay100NS);
			MULTL_XMLAddNodeText(plXMLHolder, "sfn_carrier_mode", TD_DTMBParamCarrierModeStr(s_SFNHandle.m_SFNParam.m_DTMBParam.m_CarrierMode));
			MULTL_XMLAddNodeText(plXMLHolder, "sfn_code_rate", TD_DTMBParamCodeRateStr(s_SFNHandle.m_SFNParam.m_DTMBParam.m_CodeRate));
			MULTL_XMLAddNodeText(plXMLHolder, "sfn_constellation", TD_DTMBParamConstellationStr(s_SFNHandle.m_SFNParam.m_DTMBParam.m_QAMMode));
			MULTL_XMLAddNodeText(plXMLHolder, "sfn_pn_mode", TD_DTMBParamPNStr(s_SFNHandle.m_SFNParam.m_DTMBParam.m_PNMode));
			MULTL_XMLAddNodeText(plXMLHolder, "sfn_ti_mode", TD_DTMBParamTIModeStr(s_SFNHandle.m_SFNParam.m_DTMBParam.m_TIMode));
			MULTL_XMLAddNodeText(plXMLHolder, "sfn_dp", TD_DTMBParamDPModeStr(s_SFNHandle.m_SFNParam.m_DTMBParam.m_bDoublePilot));
			MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_max_bitrate", TD_DTMBParamGetBitrate(&s_SFNHandle.m_SFNParam.m_DTMBParam));


			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_sat_mark", s_SFNHandle.m_SFNParam.m_bEnableSatSFN);
			MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_sat_null_pid", s_SFNHandle.m_SFNParam.m_SatSFNNullPacketPID);



			{
				S32 i;
				for (i = 0; i < SFN_MAX_INDV_NUM; i++)
				{
					plXMLNodeHolder = mxmlNewElement(plXMLHolder, "sfn_indv_node");
					if (plXMLNodeHolder)
					{
						MULTL_XMLAddNodeMark(plXMLNodeHolder, "active_mark", s_SFNHandle.m_SFNParam.m_pINDVArrap[i].m_ActiveMark);

						MULTL_XMLAddNodeUINT(plXMLNodeHolder, "indv_addr", s_SFNHandle.m_SFNParam.m_pINDVArrap[i].m_Info.m_SFNAddr);
						MULTL_XMLAddNodeINT(plXMLNodeHolder, "indv_freq_offset_hz", s_SFNHandle.m_SFNParam.m_pINDVArrap[i].m_Info.m_SFNSIPIndvFreqOffsetHz);
						MULTL_XMLAddNodeUINT(plXMLNodeHolder, "indv_delay", s_SFNHandle.m_SFNParam.m_pINDVArrap[i].m_Info.m_SFNSIPIndviDelay);
						MULTL_XMLAddNodeUINT(plXMLNodeHolder, "indv_power", s_SFNHandle.m_SFNParam.m_pINDVArrap[i].m_Info.m_SFNSIPIndvPower);
						MULTL_XMLAddNodeMark(plXMLNodeHolder, "indv_power_mark", s_SFNHandle.m_SFNParam.m_pINDVArrap[i].m_Info.m_SFNSIPIndvPowerMark);
					}
				}
			}

		}
	}
}

/*SFN参数设置函数*/
void MULT_SFNAApplyParameter(void)
{
	MULT_SFNA_Param *plParam;
	plParam = &s_SFNHandle.m_SFNParam;

#ifdef SUPPORT_GNS_MODULE
	if (MULT_GNSCheckHaveGNS() == FALSE)
	{
		GLOBAL_TRACE(("Force Ext 1PPS!!!!!!!!!!!\n"));
		plParam->m_bUseEx1PPS = TRUE;
	}
#else
	plParam->m_bUseEx1PPS = TRUE;
#endif

	MULTL_SFNAProtocolPacker(plParam, TD_DTMBParamGetBitrate(&plParam->m_DTMBParam));

	/*插入SIP包到调制器当中*/
	{
		S32 i;
		MULT_SFNA_INDV_SIP lDefaultSIP;
		GLOBAL_ZEROMEM(&lDefaultSIP, sizeof(lDefaultSIP));
		for (i = 0; i < SFN_MAX_INDV_NUM; i++)
		{
			if (s_SFNHandle.m_SFNParam.m_pINDVArrap[i].m_ActiveMark)
			{
				MULTL_SFNAProtocolSIPPacker(0, i, plParam->m_SFNMaxDelay100NS, &plParam->m_DTMBParam, &s_SFNHandle.m_SFNParam.m_pINDVArrap[i].m_Info, i, s_SFNHandle.m_SFNParam.m_bEnableSatSFN);
			}
			else
			{
				MULTL_SFNAProtocolSIPPacker(0, i, plParam->m_SFNMaxDelay100NS, &plParam->m_DTMBParam, &lDefaultSIP, i, s_SFNHandle.m_SFNParam.m_bEnableSatSFN);
			}
		}
	}


	/*获取当前复用参数，得到输入TS IND*/
	{
		HANDLE32 lDBHandle;
		MPEG2_DBTsRouteInfo	lRouteInfo;

		lDBHandle = s_SFNHandle.m_pMainHandle->m_DBSHandle;
		MPEG2_DBGetTsRouteInfo(lDBHandle, MPEG2_DBGetTsIDs(lDBHandle, FALSE, 0), FALSE, &lRouteInfo);

		s_SFNHandle.m_SFNStatus.m_CurrentUseTsInd = lRouteInfo.m_TsIndex;
	}


//#ifdef SUPPORT_CLK_ADJ_MODULE
//	/*时钟模块必须重置*/
//	MULT_CLKResetByModulator();
//#endif


}




/*监控线程*/
void MULT_SFNAMonitorProcess(S32 Duration)
{

}

/*返回SFN模式下是否接收到SIP包*/
BOOL MULT_SFNACheckSFNError(S32 ErrorType, BOOL bClear)
{
	BOOL lRet = FALSE;
	switch (ErrorType)
	{
	case SFNA_ERROR_INT_1PPS_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmInt1PPSLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmInt1PPSLostCount = 0;
			}
		}
		break;
	case SFNA_ERROR_EXT_1PPS_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmExt1PPSLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmExt1PPSLostCount = 0;
			}
		}
		break;
	case SFNA_ERROR_INT_10M_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmInt10MLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmInt10MLostCount = 0;
			}
		}
		break;
	case SFNA_ERROR_EXT_10M_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmExt10MLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmExt10MLostCount = 0;
			}
		}
		break;
	case SFNA_ERROR_ASI_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmTSXLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmTSXLostCount = 0;
			}
		}
		break;
	default:
		break;;
	}

	return lRet;
}

void MULT_SFNASetInternalCOMData(U8 *pData, S32 DataSize)
{
	MULT_SFNA_Status *plStatus;
	MULT_SFNA_Param	*plParam;

	plStatus = &s_SFNHandle.m_SFNStatus;
	plParam = &s_SFNHandle.m_SFNParam;

	MULTL_SFNAProtocolParser(plStatus, pData, DataSize);

	if (plStatus->m_bInt10MStatus == FALSE)
	{
		plStatus->m_AlarmInt10MLostCount++;
	}

	if (plStatus->m_bExt10MStatus == FALSE)
	{
		if (s_SFNHandle.m_SFNParam.m_bUse10MClkSynSrc == 1)
		{
			plStatus->m_AlarmExt10MLostCount++;
		}
	}

	if (plStatus->m_bInt1PPSStatus == FALSE)
	{
		if (s_SFNHandle.m_SFNParam.m_bUseEx1PPS == FALSE)
		{
			plStatus->m_AlarmInt1PPSLostCount++;
		}
	}


	if (plStatus->m_bExt1PPSStatus == FALSE)
	{
		if (s_SFNHandle.m_SFNParam.m_bUseEx1PPS == TRUE)
		{
			plStatus->m_AlarmExt1PPSLostCount++;
		}
	}


	if (((plStatus->m_bTS0Status == FALSE) && (s_SFNHandle.m_SFNParam.m_SFNASIMode == 0x01)) || ((plStatus->m_bTS1Status == FALSE) && (s_SFNHandle.m_SFNParam.m_SFNASIMode == 0x02)) || ((plStatus->m_bTS0Status == FALSE) && (plStatus->m_bTS1Status == FALSE) && (s_SFNHandle.m_SFNParam.m_SFNASIMode == 0x03)))
	{
		//GLOBAL_TRACE(("TS0 Status = %d, TS1 Status = %d, TS Mode = %.2X\n", plStatus->m_bTS0Status, plStatus->m_bTS1Status, s_SFNHandle.m_SFNParam.m_SFNASIMode));
		if (s_SFNHandle.m_SFNStatus.m_CurrentUseTsInd == 0)
		{
			GLOBAL_TRACE(("Input TS Lost!!\n"));
			s_SFNHandle.m_SFNStatus.m_AlarmTSXLostCount++;
		}
	}

}
/*销毁当前模块*/
void MULT_SFNATerminate(void)
{

}

/* SFNA 内部函数 -----------------------------------------------------------------------------------------------------------------------------------*/

/*SFN内部通讯协议*/
void MULTL_SFNAProtocolParser(MULT_SFNA_Status *pStatus, U8 *pData, S32 DataSize)
{
	if (pData)
	{
		if ((pData[0] == SFN_STATUS_TAG) && (DataSize >= 12))
		{
			U8 *plTmpData;
			U32 lTmpValue;

			plTmpData = pData;

			if (pData[1] == 0x03)//SFN状态
			{
				GLOBAL_BYTES_S(plTmpData, 4);
				GLOBAL_MSB8_D(plTmpData, lTmpValue);

				pStatus->m_bInt1PPSStatus = (lTmpValue >> 7) & 0x01;
				pStatus->m_bExt1PPSStatus = (lTmpValue >> 6) & 0x01;
				pStatus->m_bInt10MStatus = (lTmpValue >> 5) & 0x01;
				pStatus->m_bExt10MStatus = (lTmpValue >> 4) & 0x01;
				pStatus->m_bTS1Status = (lTmpValue >> 2) & 0x03;
				pStatus->m_bTS0Status = (lTmpValue >> 0) & 0x03;

#ifdef SFNA_DEBUG
				GLOBAL_TRACE(("lTmpValue = %.8X\n", lTmpValue));
				GLOBAL_TRACE(("In1PPS = %d, Ext1PPS = %d\n", pStatus->m_bInt1PPSStatus, pStatus->m_bExt1PPSStatus));
				GLOBAL_TRACE(("In10M = %d, Ext10M = %d\n", pStatus->m_bInt10MStatus, pStatus->m_bExt10MStatus));
				GLOBAL_TRACE(("Ts0 = %d, Ts1 = %d\n", pStatus->m_bTS0Status, pStatus->m_bTS1Status));
#endif

				if (s_SFNHandle.m_SFNStatus.m_bTS0Status == 0x01 || s_SFNHandle.m_SFNStatus.m_bTS0Status == 0x02)
				{
					s_SFNHandle.m_SFNStatus.m_bTS0Status = TRUE;
				}
				else
				{
					s_SFNHandle.m_SFNStatus.m_bTS0Status = FALSE;
				}
				if (s_SFNHandle.m_SFNStatus.m_bTS1Status == 0x01 || s_SFNHandle.m_SFNStatus.m_bTS1Status == 0x02)
				{
					s_SFNHandle.m_SFNStatus.m_bTS1Status = TRUE;
				}
				else
				{
					s_SFNHandle.m_SFNStatus.m_bTS1Status = FALSE;
				}

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
}


void MULTL_SFNAProtocolPacker(MULT_SFNA_Param *pParam, F64 CurBitrate)
{
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	S32 lLen;
	S32 lTmpAddDelay100ns;
	U32 lTmpValue;
	F64 lCur1sTSPacketNum, lCur1PacketPulseNum, lCurDelayPacketNum, lCurDelayPacketNumFloat;
	U32 lCurDelayPacketNumInt;
	U32	lCurDelayPacketNumFLoatPulseNum;
	S16 lPhyIndex;


	GLOBAL_TRACE(("Current DTMB Bitrate = %f\n", CurBitrate));
	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	GLOBAL_MSB8_EC(plTmpBuf, 0x33/*SFN参数设置*/, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x05, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	lTmpValue = ((0?1:0)<< 7) | (pParam->m_bUse10MClkSynSrc<< 5)  | ((pParam->m_bUseEx1PPS?1:0)<< 4) | ((0?1:0)<< 3) | ((pParam->m_bEnableSatSFN?1:0)<< 2) | (pParam->m_SFNASIMode);

	GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);
	/*因为FPGA设置0值工作不正常，这里+1*/
	lTmpAddDelay100ns = 0;
	if (lTmpAddDelay100ns == 0)
	{
		lTmpAddDelay100ns = 1;
	}

	GLOBAL_MSB24_EC(plTmpBuf, lTmpAddDelay100ns, lLen);
	GLOBAL_MSB16_EC(plTmpBuf, pParam->m_SatSFNNullPacketPID, lLen);
	GLOBAL_BYTES_SC(plTmpBuf, 2, lLen);



	lCur1sTSPacketNum = CurBitrate / 8 / 188;//得到当前码率1S的TS包的个数

	lCur1PacketPulseNum = (MULT_FPGA_MAIN_CLK_SFN * 1000000) / lCur1sTSPacketNum;//得到当前码率下1个包的FPGA的时钟个数

	lCurDelayPacketNum = (lTmpAddDelay100ns * lCur1sTSPacketNum) / (10000000);//得到当前延时对应的TS包的个数

	lCurDelayPacketNumInt = (U32)lCurDelayPacketNum;

	lCurDelayPacketNumFloat = lCurDelayPacketNum - lCurDelayPacketNumInt;

	lCurDelayPacketNumFLoatPulseNum = (U32)((MULT_FPGA_MAIN_CLK_SFN * 1000000) * lCurDelayPacketNumFloat / lCur1sTSPacketNum);

#ifdef SFNA_DEBUG
	GLOBAL_TRACE(("CurMode 1S PacketNum = %f, 1Packet Pulse Num = %f\n", lCur1sTSPacketNum, lCur1PacketPulseNum));

	GLOBAL_TRACE(("Delay100nsValue = %d PacketNum = %f, INT Part = %d, FloatPart = %f, PulseNum = %d\n", lTmpAddDelay100ns, lCurDelayPacketNum, lCurDelayPacketNumInt, lCurDelayPacketNumFloat, lCurDelayPacketNumFLoatPulseNum));
#endif

	lTmpValue = (U32)lCur1sTSPacketNum;
	GLOBAL_MSB32_EC(plTmpBuf, lTmpValue, lLen);//
	lTmpValue = (U32)lCur1PacketPulseNum;
	GLOBAL_MSB32_EC(plTmpBuf, lTmpValue, lLen);//
	GLOBAL_MSB16_EC(plTmpBuf, lCurDelayPacketNumInt, lLen);//
	GLOBAL_MSB16_EC(plTmpBuf, lCurDelayPacketNumFLoatPulseNum, lLen);//

#ifdef SFNA_DEBUG
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);



	/*为了保证输出码率抖动最小，这里新的协议传输信息*/
	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	plTmpBuf = plCMDBuf;
	lLen = 0;

	GLOBAL_MSB8_EC(plTmpBuf, 0x3A/*SFN ASI 参数设置*/, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	lTmpValue = CurBitrate ;
	GLOBAL_MSB32_EC(plTmpBuf, lTmpValue, lLen);

	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#ifdef SFNA_DEBUG
#endif
	HWL_FPGAWrite(plCMDBuf, lLen);

}


void MULTL_SFNAProtocolSIPPacker(S32 CHNInd, S32 SlotInd, U32 MaxDelay, TD_DTMBParameter *pDTMBParam, MULT_SFNA_INDV_SIP *pINDVInfo, S32 CCInd, BOOL bSATMode)
{
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	S32 lLen;
	MULT_SFNA_CMN_SIP lCMNSIP;
	U32 lTmpValue;
	S16 lPhyIndex;

	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	lLen = 0;
	plTmpBuf = plCMDBuf;

#ifdef SFNA_DEBUG
	GLOBAL_TRACE(("-- Info For SIP Insert CHN %d, Slot %d--\n", CHNInd, SlotInd));
	MULTL_SFNAPrinter(pDTMBParam , pINDVInfo);
	GLOBAL_TRACE(("-- End--\n"));
#endif


	GLOBAL_MSB8_EC(plTmpBuf, 0x34/*SIP设置*/, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x30, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	GLOBAL_MSB16_EC(plTmpBuf, CHNInd, lLen);
	GLOBAL_MSB16_EC(plTmpBuf, SlotInd, lLen);

	MULTL_SFNASIPParamChange(pDTMBParam, &lCMNSIP, TRUE);

	lLen += MULTL_SFNASIPPacker(plTmpBuf, MaxDelay, &lCMNSIP, pINDVInfo, CCInd, bSATMode);

#ifdef SFNA_DEBUG
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);

}



S32 MULTL_SFNASIPPacker(U8 *pTsBuf, U32 MaxDelay, MULT_SFNA_CMN_SIP *pSIPInfo, MULT_SFNA_INDV_SIP *pINDVInfo, S32 CCInd, BOOL bCRC32)
{
	if (pTsBuf && pSIPInfo && pINDVInfo)
	{
		U8 *plTmpData;
		U32 lTmpValue;

		plTmpData = pTsBuf;

		GLOBAL_MSB8_E(plTmpData, MPEG2_TS_PACKET_SYN_BYTE);
		GLOBAL_MSB8_E(plTmpData, 0x40);
		GLOBAL_MSB8_E(plTmpData, 0x15);
		GLOBAL_MSB8_E(plTmpData, (0x10 | ( CCInd & 0x0F)));

		lTmpValue = 0;
		lTmpValue |= ((pSIPInfo->m_SFNSIPPN & 0x03) << 14);
		lTmpValue |= ((pSIPInfo->m_SFNSIPCarrier & 0x01) << 13);
		lTmpValue |= ((pSIPInfo->m_SFNSIPConstellation & 0x07) << 10);
		lTmpValue |= ((pSIPInfo->m_SFNSIPCodeRate & 0x03) << 8);
		lTmpValue |= ((pSIPInfo->m_SFNSIPTI & 0x01) << 7);
		lTmpValue |= ((pSIPInfo->m_SFNSIPDoublePilot & 0x01) << 6);
		lTmpValue |= ((pSIPInfo->m_SFNSIPPN_Shift & 0x01) << 5);


		GLOBAL_MSB16_E(plTmpData, lTmpValue);
		GLOBAL_MSB24_E(plTmpData, MaxDelay);
		GLOBAL_MSB16_E(plTmpData, pINDVInfo->m_SFNAddr);
		GLOBAL_MSB24_E(plTmpData, pINDVInfo->m_SFNSIPIndviDelay);
		GLOBAL_MSB24_E(plTmpData, pINDVInfo->m_SFNSIPIndvFreqOffsetHz);

		lTmpValue = 0;
		lTmpValue |= pINDVInfo->m_SFNSIPIndvPower & 0x7FFF;
		lTmpValue |= (((pINDVInfo->m_SFNSIPIndvPowerMark == TRUE)?1:0) << 15) & 0x8000;

		GLOBAL_MSB16_E(plTmpData, lTmpValue);

		GLOBAL_MEMSET(plTmpData, 0xFF, 169);

		if (bCRC32)
		{
			CRYPTO_CRC32GenerateWithInitValue(GLOBAL_U32_MAX, pTsBuf, MPEG2_TS_PACKET_SIZE);
		}

	}
	else
	{
		GLOBAL_TRACE(("NULL Ptr!!!!!!!!!!\n"));
	}

	return MPEG2_TS_PACKET_SIZE;
}


/*工具函数*/
void MULTL_SFNAPrinter(TD_DTMBParameter *pTDDTMBParam, MULT_SFNA_INDV_SIP *pSFNINDVSIP)
{
	if (pTDDTMBParam)
	{
		TD_DTMBParamStr(pTDDTMBParam, s_SFNHandle.m_pSFNTmpInfo, sizeof(s_SFNHandle.m_pSFNTmpInfo));
		GLOBAL_TRACE((" SI Info: [%s]\n", s_SFNHandle.m_pSFNTmpInfo));
	}
	if (pSFNINDVSIP)
	{
		GLOBAL_TRACE(("Addr = %.4X, IndvDelay = %d, FreqOffset = %d, IndvPower/Mark = %d/%d\n", pSFNINDVSIP->m_SFNAddr, pSFNINDVSIP->m_SFNSIPIndviDelay, pSFNINDVSIP->m_SFNSIPIndvFreqOffsetHz, pSFNINDVSIP->m_SFNSIPIndvPower, pSFNINDVSIP->m_SFNSIPIndvPowerMark));
	}
}



void MULTL_SFNASIPParamChange(TD_DTMBParameter *pTDDTMBParam, MULT_SFNA_CMN_SIP *pSFNSISIP, BOOL bToSFNParam)
{
	if (pTDDTMBParam && pSFNSISIP)
	{
		if (bToSFNParam)
		{
			switch (pTDDTMBParam->m_CarrierMode)
			{
			case TD_DTMB_CARRIER_MODE_SINGLE:
				pSFNSISIP->m_SFNSIPCarrier = 0;
				break;
			case TD_DTMB_CARRIER_MODE_3780:
				pSFNSISIP->m_SFNSIPCarrier = 1;
				break;
			default:
				pSFNSISIP->m_SFNSIPCarrier = 0;
				break;
			}

			switch (pTDDTMBParam->m_CodeRate)
			{
			case TD_DTMB_CODE_RATE_04:
				pSFNSISIP->m_SFNSIPCodeRate = 0;
				break;
			case TD_DTMB_CODE_RATE_06:
				pSFNSISIP->m_SFNSIPCodeRate = 1;
				break;
			case TD_DTMB_CODE_RATE_08:
				pSFNSISIP->m_SFNSIPCodeRate = 2;
				break;
			default:
				pSFNSISIP->m_SFNSIPCodeRate = 0;
				break;
			}

			switch (pTDDTMBParam->m_QAMMode)
			{
			case TD_DTMB_QAM_MAP_4QAMNR:
				pSFNSISIP->m_SFNSIPConstellation = 0;
				break;
			case TD_DTMB_QAM_MAP_4QAM:
				pSFNSISIP->m_SFNSIPConstellation = 1;
				break;
			case TD_DTMB_QAM_MAP_16QAM:
				pSFNSISIP->m_SFNSIPConstellation = 2;
				break;
			case TD_DTMB_QAM_MAP_32QAM:
				pSFNSISIP->m_SFNSIPConstellation = 3;
				break;
			case TD_DTMB_QAM_MAP_64QAM:
				pSFNSISIP->m_SFNSIPConstellation = 4;
				break;
			default:
				pSFNSISIP->m_SFNSIPConstellation = 0;
				break;
			}

			pSFNSISIP->m_SFNSIPPN_Shift = 0;
			switch (pTDDTMBParam->m_PNMode)
			{
			case TD_DTMB_PN_MODE_420:
				pSFNSISIP->m_SFNSIPPN_Shift = 1;
			case TD_DTMB_PN_MODE_420C:
				{
					pSFNSISIP->m_SFNSIPPN = 0;
				}
				break;
			case TD_DTMB_PN_MODE_595:
				{
					pSFNSISIP->m_SFNSIPPN = 1;
				}
				break;
			case TD_DTMB_PN_MODE_945:
				pSFNSISIP->m_SFNSIPPN_Shift = 1;
			case TD_DTMB_PN_MODE_945C:
				{
					pSFNSISIP->m_SFNSIPPN = 2;
				}
				break;
			default:
				pSFNSISIP->m_SFNSIPPN = 0;
				break;
			}


			switch (pTDDTMBParam->m_TIMode)
			{
			case TD_DTMB_TI_MODE_240:
				pSFNSISIP->m_SFNSIPTI = 0;
				break;
			case TD_DTMB_TI_MODE_720:
				pSFNSISIP->m_SFNSIPTI = 1;
				break;
			default:
				pSFNSISIP->m_SFNSIPTI = 0;
				break;
			}

			switch (pTDDTMBParam->m_bDoublePilot)
			{
			case TRUE:
				pSFNSISIP->m_SFNSIPDoublePilot = 1;
				break;
			case FALSE:
				pSFNSISIP->m_SFNSIPDoublePilot = 0;
				break;
			default:
				pSFNSISIP->m_SFNSIPDoublePilot = 0;
				break;
			}

		}
		else /*反向转换 ----------------------------------------------------------  */
		{
			switch (pSFNSISIP->m_SFNSIPCarrier)
			{
			case 0:
				pTDDTMBParam->m_CarrierMode = TD_DTMB_CARRIER_MODE_SINGLE;
				break;
			case 1:
				pTDDTMBParam->m_CarrierMode = TD_DTMB_CARRIER_MODE_3780;
				break;
			default:
				pTDDTMBParam->m_CarrierMode = TD_DTMB_CARRIER_MODE_SINGLE;
				break;
			}

			switch (pSFNSISIP->m_SFNSIPCodeRate)
			{
			case 0:
				pTDDTMBParam->m_CodeRate = TD_DTMB_CODE_RATE_04;
				break;
			case 1:
				pTDDTMBParam->m_CodeRate = TD_DTMB_CODE_RATE_06;
				break;
			case 2:
				pTDDTMBParam->m_CodeRate = TD_DTMB_CODE_RATE_08;
				break;
			default:
				pTDDTMBParam->m_CodeRate = TD_DTMB_CODE_RATE_04;
				break;
			}

			switch (pSFNSISIP->m_SFNSIPConstellation)
			{
			case 0:
				pTDDTMBParam->m_QAMMode = TD_DTMB_QAM_MAP_4QAMNR;
				break;
			case 1:
				pTDDTMBParam->m_QAMMode = TD_DTMB_QAM_MAP_4QAM;
				break;
			case 2:
				pTDDTMBParam->m_QAMMode = TD_DTMB_QAM_MAP_16QAM;
				break;
			case 3:
				pTDDTMBParam->m_QAMMode = TD_DTMB_QAM_MAP_32QAM;
				break;
			case 4:
				pTDDTMBParam->m_QAMMode = TD_DTMB_QAM_MAP_64QAM;
				break;
			default:
				pTDDTMBParam->m_QAMMode = TD_DTMB_QAM_MAP_4QAMNR;
				break;
			}


			switch (pSFNSISIP->m_SFNSIPPN)
			{
			case 0:
				pTDDTMBParam->m_PNMode = TD_DTMB_PN_MODE_420C;
				if (pSFNSISIP->m_SFNSIPPN_Shift == 1)
				{
					pTDDTMBParam->m_PNMode = TD_DTMB_PN_MODE_420;
				}
				break;
			case 1:
				pTDDTMBParam->m_PNMode = TD_DTMB_PN_MODE_595;
				break;
			case 2:
				pTDDTMBParam->m_PNMode = TD_DTMB_PN_MODE_945C;
				if (pSFNSISIP->m_SFNSIPPN_Shift == 1)
				{
					pTDDTMBParam->m_PNMode = TD_DTMB_PN_MODE_945;
				}
				break;
			default:
				pTDDTMBParam->m_PNMode = TD_DTMB_PN_MODE_420;
				break;
			}

			switch (pSFNSISIP->m_SFNSIPTI)
			{
			case 0:
				pTDDTMBParam->m_TIMode = TD_DTMB_TI_MODE_240;
				break;
			case 1:
				pTDDTMBParam->m_TIMode = TD_DTMB_TI_MODE_720;
				break;
			default:
				pTDDTMBParam->m_TIMode = TD_DTMB_TI_MODE_240;
				break;
			}


			switch (pSFNSISIP->m_SFNSIPDoublePilot)
			{
			case 0:
				pTDDTMBParam->m_bDoublePilot = FALSE;
				break;
			case 1:
				pTDDTMBParam->m_bDoublePilot = TRUE;
				break;
			default:
				pTDDTMBParam->m_bDoublePilot = FALSE;
				break;
			}

		}
	}
}

/*外部参数，状态，获取设置*/
void MULT_SFNAProcSFNParam(MULT_SFNA_Param *pParam, BOOL bRead)
{
	if (pParam)
	{
		if (bRead)
		{
			GLOBAL_MEMCPY(pParam, &s_SFNHandle.m_SFNParam, sizeof(MULT_SFNA_Param));
		}
		else
		{
			GLOBAL_MEMCPY(&s_SFNHandle.m_SFNParam, pParam, sizeof(MULT_SFNA_Param));
		}
	}
}

void MULT_SFNAGetSFNStatus(MULT_SFNA_Status *pStatus)
{
	if (pStatus)
	{
		GLOBAL_MEMCPY(pStatus, &s_SFNHandle.m_SFNStatus, sizeof(MULT_SFNA_Status));
	}
}


#endif

/*EOF*/
