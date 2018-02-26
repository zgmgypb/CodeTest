/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_tsp.h"

#ifdef SUPPORT_SFN_MODULATOR
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
//#define SFN_DEBUG
#define SFN_SIP_PACKET_PID			(0x15)
#define SFN_SIP_RECV_TIMEOUT_MS		(2000)//SIP包接收超时（毫秒）
#define SFN_SAT_NULL_PACKET_PID		(8190)
#define SFN_STATUS_TAG				(0x33)
#define SFN_SIP_APPLY_DELAY			(3)
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	/*设置参数*/
	MULT_SFN_Param			m_SFNParam;

	/*单频网适配器控制参数和状态*/
	MULT_SFN_Status			m_SFNStatus;

	CHAR_T					m_pSFNTmpInfo[1024];

	/*SIP包过滤*/
	U32						m_FilterID;

	MULT_Handle				*m_pMainHandle;

	S32						m_SFNModeSIPRecvTimeout;//SIP接收超时
	S32						m_SFNSIPTolerance;

	S32						m_SFNSIPPauseed;

	BOOL					m_SFNSIPRecvedAfterApply;

	BOOL					m_FirstInitiateMark;

	BOOL					m_ForceUpdate;//块设置为1时会导致强制重新设置参数

}MULT_SFN_HANDLE;

/* Private Variables (static)-------------------------------------------------- */
static MULT_SFN_HANDLE s_SFNHandle;
/* Private Function prototypes ------------------------------------------------ */
static void MULTL_SFNProtocolSetParameter(MULT_SFN_Param *pParam, MULT_SFN_CMN_SIP *pModParameter);
static void MULTL_SFNProtocolSetSIP(S32 CHNInd, S32 SlotInd, MULT_SFN_CMN_SIP *pSIPInfo, MULT_SFN_INDV_SIP *pINDVInfo);
static void MULTL_SFNProtocolParser(U8 *pData, S32 DataSize);
static void MULTL_SFNTsFilterCB(S32 SlotInd, U8 *pTsPacket);
static void MULTL_SFNSIPStart(void);
static void MULTL_SFNSIPParser(U8 *pTsPacket);
static S32 MULTL_SFNSIPPacker(U8 *pTsBuf, MULT_SFN_CMN_SIP *pSIPInfo, MULT_SFN_INDV_SIP *pINDVInfo);
static void MULTL_SFNSIPStop(void);
static void MULTL_SFNMakeInfo(void);
//static void MULTL_SFNApplyQamParameter(void);
//static void MULTL_SFNApplyTsParameter(void);
static void MULTL_SFNSIPPrinter(MULT_SFN_CMN_SIP *pSFNSISIP, MULT_SFN_INDV_SIP *pSFNINDVSIP);
/* Functions ------------------------------------------------------------------ */

void MULT_SFNInitiate(MULT_Handle *pHandle)
{
	GLOBAL_ZEROMEM(&s_SFNHandle, sizeof(MULT_SFN_HANDLE));


	s_SFNHandle.m_pMainHandle = pHandle;

	s_SFNHandle.m_SFNParam.m_bEnableSatSFN = FALSE;
	s_SFNHandle.m_SFNParam.m_SatSFNNullPacketPID = SFN_SAT_NULL_PACKET_PID;
	s_SFNHandle.m_SFNParam.m_SFNASIMode = 3;

	s_SFNHandle.m_SFNParam.m_bUseCMNSIP = TRUE;

	s_SFNHandle.m_FirstInitiateMark = TRUE;

	/*默认参数设置为常用模式7*/
	s_SFNHandle.m_SFNStatus.m_SFNCMNSIP.m_SFNSIPCarrier = 0;//C1
	s_SFNHandle.m_SFNStatus.m_SFNCMNSIP.m_SFNSIPCodeRate = 2;//0.8
	s_SFNHandle.m_SFNStatus.m_SFNCMNSIP.m_SFNSIPConstellation = 3;//32QAM
	s_SFNHandle.m_SFNStatus.m_SFNCMNSIP.m_SFNSIPPN = 1;//595
	s_SFNHandle.m_SFNStatus.m_SFNCMNSIP.m_SFNSIPPN_Shift = 0;
	s_SFNHandle.m_SFNStatus.m_SFNCMNSIP.m_SFNSIPDoublePilot = 0;
	s_SFNHandle.m_SFNStatus.m_SFNCMNSIP.m_SFNSIPTI = 1;//720

	s_SFNHandle.m_SFNModeSIPRecvTimeout = SFN_SIP_RECV_TIMEOUT_MS * 2;

	s_SFNHandle.m_SFNParam.m_bTsLostMUTE = FALSE;
	s_SFNHandle.m_SFNParam.m_bREFLostMUTE = FALSE;
	s_SFNHandle.m_SFNParam.m_b1PPSLostMUTE = FALSE;
	s_SFNHandle.m_SFNParam.m_bSIPLostMUTE = FALSE;


	s_SFNHandle.m_SFNSIPPauseed = FALSE;
	s_SFNHandle.m_ForceUpdate = TRUE;

}

/*参数读写，包括网页提交*/
void MULT_SFNXMLLoad(mxml_node_t *pXMLRoot, BOOL bPost)
{
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLHolder;

	if (bPost)
	{
		plXMLHolder = pXMLRoot;
	}
	else
	{
		plXMLHolder = mxmlFindElement(pXMLRoot, pXMLRoot, "sfn_modulator_setting", NULL, NULL, MXML_DESCEND_FIRST);
	}

	if (plXMLHolder)
	{

		s_SFNHandle.m_SFNParam.m_Last10MCLKSyncSrc = s_SFNHandle.m_SFNParam.m_bUse10MClkSynSrc;

		s_SFNHandle.m_SFNParam.m_bUseSFN = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_mark", FALSE);
		s_SFNHandle.m_SFNParam.m_bUse10MClkSynSrc = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "sfn_10m_sel", 10, 0);
		s_SFNHandle.m_SFNParam.m_bUseEx1PPS = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_ex1pps_mark", FALSE);
		s_SFNHandle.m_SFNParam.m_SFNAddDelay100ns = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "sfn_add_delay_100ns", 10, 0);


		s_SFNHandle.m_SFNParam.m_SFNASIMode = MULTL_XMLGetNodeINT(plXMLHolder, "sfn_input_asi_mod", 10);

		if (s_SFNHandle.m_SFNParam.m_SFNASIMode <= 0 || s_SFNHandle.m_SFNParam.m_SFNASIMode > 3)
		{
			s_SFNHandle.m_SFNParam.m_SFNASIMode = 3;
		}

		s_SFNHandle.m_SFNParam.m_SFNAddrID = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "sfn_addr_id", 10, 0);
		s_SFNHandle.m_SFNParam.m_bUseIndvSIP = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_use_indv_mark", FALSE);
		s_SFNHandle.m_SFNParam.m_bUseCMNSIP = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_use_cmn_mark", FALSE);

		s_SFNHandle.m_SFNParam.m_bEnableSatSFN = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_sat_mark", FALSE);
		s_SFNHandle.m_SFNParam.m_SatSFNNullPacketPID = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "sfn_sat_null_pid", 10, 8190);
		s_SFNHandle.m_SFNParam.m_bSatSFNSIPCRC32Check = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_sat_crc_checkmark", FALSE);


		s_SFNHandle.m_SFNParam.m_bTsLostMUTE = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_ts_mute_mark", FALSE);
		s_SFNHandle.m_SFNParam.m_bREFLostMUTE = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_ref_mute_mark", FALSE);
		s_SFNHandle.m_SFNParam.m_b1PPSLostMUTE = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_1pps_mute_mark", FALSE);
		s_SFNHandle.m_SFNParam.m_bSIPLostMUTE = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_sip_mute_mark", FALSE);


		s_SFNHandle.m_SFNParam.m_bDeleteSIP = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "sfn_sip_del_mark", FALSE);

#ifdef SUPPORT_CLK_ADJ_MODULE
		MULT_CLKSet10MCLKSYNCSyc(s_SFNHandle.m_SFNParam.m_bUse10MClkSynSrc);
#endif

		s_SFNHandle.m_SFNStatus.m_SFNSIPINDVUpdated = FALSE;
	}
}

void MULT_SFNXMLSave(mxml_node_t *pXMLRoot, BOOL bStat)
{
	CHAR_T plTmpStr[512];
	S32 lTmpStrLen;
	MULT_SFN_Status *plStatus;
	MULT_SFN_CMN_SIP *plSFNSISIP;
	MULT_SFN_INDV_SIP *plSFNINDVSIP;
	mxml_node_t *plXMLHolder;

	plStatus = &s_SFNHandle.m_SFNStatus;
	plSFNSISIP = &plStatus->m_SFNCMNSIP;
	plSFNINDVSIP = &plStatus->m_SFNIndvSIP;


	plStatus = &s_SFNHandle.m_SFNStatus;
	if (bStat)
	{
		plXMLHolder = pXMLRoot;
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_int1pps_status", plStatus->m_bInt1PPSStatus);
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_ext1pps_status", plStatus->m_bExt1PPSStatus);
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_int10m_status", plStatus->m_bInt10MStatus);
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_ext10m_status", plStatus->m_bExt10MStatus);
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_sip_status", (s_SFNHandle.m_SFNSIPTolerance > 0)?0:1);

		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_rf_mute_status", plStatus->m_RFMuted);

#ifdef SUPPORT_CLK_ADJ_MODULE
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_10m_lock_status", MULT_CLKGet10MLockStatus());
		MULTL_XMLAddNodeINT(plXMLHolder, "sfn_10m_symbol_offset", MULT_CLKGetCurSumOffset());
#endif

		MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_max_delay_100ns", plSFNSISIP->m_SFNMaxDelay100NS);
		MULTL_SFNMakeInfo();
		MULTL_XMLAddNodeText(plXMLHolder, "sfn_si_sip_info", s_SFNHandle.m_pSFNTmpInfo);


		MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_cur_addr", plSFNINDVSIP->m_SFNAddr);
		MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_cur_addr_delay", plSFNINDVSIP->m_SFNSIPIndviDelay);
		MULTL_XMLAddNodeINT(plXMLHolder, "sfn_cur_freqoffset", plSFNINDVSIP->m_SFNSIPIndvFreqOffsetHz);
		MULTL_XMLAddNodeFLOAT(plXMLHolder, "sfn_cur_pow", (plSFNINDVSIP->m_SFNSIPIndvPower * 0.1));
		MULTL_XMLAddNodeMark(plXMLHolder, "sfn_cur_pow_mark", plSFNINDVSIP->m_SFNSIPIndvPowerMark);

		MULTL_XMLAddNodeMark(plXMLHolder, "asi0_status", plStatus->m_bTS0Status);
		MULTL_XMLAddNodeMark(plXMLHolder, "asi1_status", plStatus->m_bTS1Status);

		MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_net_delay_100ns", plStatus->m_NetDelay100ns);
	}
	else
	{
		plXMLHolder = mxmlNewElement(pXMLRoot, "sfn_modulator_setting");
		if (plXMLHolder)
		{
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_mark", s_SFNHandle.m_SFNParam.m_bUseSFN);
			MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_10m_sel", s_SFNHandle.m_SFNParam.m_bUse10MClkSynSrc);
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_ex1pps_mark", s_SFNHandle.m_SFNParam.m_bUseEx1PPS);
			MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_add_delay_100ns", s_SFNHandle.m_SFNParam.m_SFNAddDelay100ns);

			MULTL_XMLAddNodeINT(plXMLHolder, "sfn_input_asi_mod", s_SFNHandle.m_SFNParam.m_SFNASIMode);

			MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_addr_id", s_SFNHandle.m_SFNParam.m_SFNAddrID);
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_use_indv_mark", s_SFNHandle.m_SFNParam.m_bUseIndvSIP);
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_use_cmn_mark", s_SFNHandle.m_SFNParam.m_bUseCMNSIP);

			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_sip_del_mark", s_SFNHandle.m_SFNParam.m_bDeleteSIP);


			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_sat_mark", s_SFNHandle.m_SFNParam.m_bEnableSatSFN);
			MULTL_XMLAddNodeUINT(plXMLHolder, "sfn_sat_null_pid", s_SFNHandle.m_SFNParam.m_SatSFNNullPacketPID);
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_sat_crc_checkmark", s_SFNHandle.m_SFNParam.m_bSatSFNSIPCRC32Check);


			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_ts_mute_mark", s_SFNHandle.m_SFNParam.m_bTsLostMUTE);
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_ref_mute_mark", s_SFNHandle.m_SFNParam.m_bREFLostMUTE);
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_1pps_mute_mark", s_SFNHandle.m_SFNParam.m_b1PPSLostMUTE);
			MULTL_XMLAddNodeMark(plXMLHolder, "sfn_sip_mute_mark", s_SFNHandle.m_SFNParam.m_bSIPLostMUTE);


			MULTL_XMLAddNodeINT(plXMLHolder, "sfn_gain_max", 40);
			MULTL_XMLAddNodeFLOAT(plXMLHolder, "sfn_gain_step", 0.25);
		}

	}
}

/*SFN参数设置函数*/
void MULT_SFNApplyParameter(void)
{
	MULT_SFN_Param *plParam;
	MULT_SubModulatorInfo *plSubModulatorInfo;
	plParam = &s_SFNHandle.m_SFNParam;
	plSubModulatorInfo = &s_SFNHandle.m_pMainHandle->m_Parameter.m_pOutChannel[0].m_pSubChannelNode[0].m_SubInfo.m_SubModulatorInfo;

	MULTL_SFNSIPParamChange(plSubModulatorInfo, &s_SFNHandle.m_SFNStatus.m_SFNCMNSIP, TRUE);

#ifdef SUPPORT_GNS_MODULE
	if (MULT_GNSCheckHaveGNS() == FALSE)
	{
		GLOBAL_TRACE(("Force Ext 1PPS!!!!!!!!!!!\n"));
		plParam->m_bUseEx1PPS = TRUE;
	}
#else
	plParam->m_bUseEx1PPS = TRUE;
#endif

	MULTL_SFNProtocolSetParameter(plParam, &s_SFNHandle.m_SFNStatus.m_SFNCMNSIP);

	/*清空更新标志*/
	s_SFNHandle.m_SFNStatus.m_SFNSIPINDVUpdated = FALSE;
	s_SFNHandle.m_SFNModeSIPRecvTimeout = SFN_SIP_RECV_TIMEOUT_MS * 2;

	if (s_SFNHandle.m_SFNParam.m_bUseSFN)
	{
		if (s_SFNHandle.m_SFNSIPPauseed == FALSE)
		{
			MULTL_SFNSIPStart();
		}
	}
	else
	{
		MULTL_SFNSIPStop();
	}

	GLOBAL_TRACE(("SFN Reseted!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
}


void MULT_SFNApplySIP(MULT_SubModulatorInfo *pSubModInfo)
{
	MULT_SFN_INDV_SIP lINDVSIP;
	MULT_SFN_CMN_SIP lSFNSISIP;

	MULTL_SFNSIPParamChange(pSubModInfo, &lSFNSISIP, TRUE);

	lINDVSIP.m_SFNAddr = 0;
	lINDVSIP.m_SFNSIPIndvFreqOffsetHz = 0;
	lINDVSIP.m_SFNSIPIndviDelay = 0;
	lINDVSIP.m_SFNSIPIndvPower = 0;
	lINDVSIP.m_SFNSIPIndvPowerMark = 0;

	MULTL_SFNProtocolSetSIP(0, 0, &lSFNSISIP, &lINDVSIP);
}


/*监控线程*/
void MULT_SFNMonitorProcess(S32 Duration)
{
	MULT_SFN_Status *plStatus;
	MULT_SFN_Param	*plParam;

	if (Duration > 2500)
	{
		Duration = 100;
	}

	plStatus = &s_SFNHandle.m_SFNStatus;
	plParam = &s_SFNHandle.m_SFNParam;


	if (plParam->m_bUseSFN)
	{
		/*SIP Lost 错误检测和复位*/
		if (plStatus->m_InputLosted == FALSE)
		{

			/*输入码率检测*/
			{
				S32 lCurTsBitrate, lCurChnBitrate;
				/*获取当前复用参数，得到输入TS IND*/
				HANDLE32 lDBHandle;
				MPEG2_DBTsRouteInfo	lRouteInfo;


				lDBHandle = s_SFNHandle.m_pMainHandle->m_DBSHandle;
				MPEG2_DBGetTsRouteInfo(lDBHandle, MPEG2_DBGetTsIDs(lDBHandle, FALSE, 0), FALSE, &lRouteInfo);


				/*得到实时码率*/
				lCurTsBitrate = HWL_GetBitrate(0, lRouteInfo.m_TsIndex);

				lCurChnBitrate = MULTL_SFNGetDTMBBitrate(&s_SFNHandle.m_SFNStatus.m_SFNCMNSIP);

				/*得到当前通道码率*/
				//if (fabs(lCurChnBitrate - lCurTsBitrate) > 50 * 1000)
				//{
				//	plStatus->m_AlarmBitrateErrorCount++;
				//	GLOBAL_TRACE(("Bitrate Error = %d / %d, Offset / Limits = %d / %d\n",lCurTsBitrate, lCurChnBitrate, fabs(lCurChnBitrate - lCurTsBitrate), 50 * 1000));
				//}
			}

			if (s_SFNHandle.m_SFNModeSIPRecvTimeout >= 0)
			{
				if (s_SFNHandle.m_SFNSIPPauseed == FALSE)
				{
					s_SFNHandle.m_SFNModeSIPRecvTimeout -= Duration;
				}
			}
			else
			{
				s_SFNHandle.m_SFNModeSIPRecvTimeout = SFN_SIP_RECV_TIMEOUT_MS;
				plStatus->m_SFNSIPRecvCount = 0;
				plStatus->m_SIPLosted = TRUE;
				plStatus->m_AlarmSIPLostCount++;
				GLOBAL_TRACE(("SIP Recv Lost!!!!!!!\n"));

				s_SFNHandle.m_SFNSIPTolerance++;
			}

			if (plStatus->m_SIPLosted == TRUE)
			{
				if (s_SFNHandle.m_SFNSIPTolerance == 0)
				{
					GLOBAL_TRACE(("Recover From SIP Lost !!!!!!!!!\n"));
					s_SFNHandle.m_ForceUpdate = TRUE;
				}
			}
		}


		/*系统从错误中恢复检测*/
		if (plStatus->m_1PPSLosted == TRUE)
		{
			if (((plParam->m_bUseEx1PPS == TRUE) && (plStatus->m_bExt1PPSStatus == TRUE)) || ((plParam->m_bUseEx1PPS == FALSE) && (plStatus->m_bInt1PPSStatus == TRUE)))
			{
				GLOBAL_TRACE(("Recover From 1PPS Lost!!!\n"));
				s_SFNHandle.m_ForceUpdate = TRUE;
				plStatus->m_1PPSLosted = FALSE;
			}
		}

		if (plStatus->m_InputLosted == TRUE)
		{
			if (((plStatus->m_bTS0Status == TRUE) && (plParam->m_SFNASIMode == 0x01)) || ((plStatus->m_bTS1Status == TRUE) && (plParam->m_SFNASIMode == 0x02)) || (((plStatus->m_bTS0Status == TRUE) || (plStatus->m_bTS0Status == TRUE)) && (plParam->m_SFNASIMode == 0x03)))
			{
				if (plStatus->m_CurrentUseTsInd == 0)
				{
					GLOBAL_TRACE(("Recover From Input ASI Lost!!!\n"));
					s_SFNHandle.m_ForceUpdate = TRUE;
					plStatus->m_InputLosted  = FALSE;
				}
			}
		}


		if (s_SFNHandle.m_ForceUpdate == TRUE)
		{
#ifdef USE_NEW_QAM_SETTING_FUNCTIONS
			HWL_QAMForceNotLevelOnly();
#endif
			/*应用主模块参数*/
			MULTL_ApplyQAMParameter(s_SFNHandle.m_pMainHandle, 0);

			/*保存参数*/
			MULTL_SaveParameterXML(s_SFNHandle.m_pMainHandle);

			/*不在界面显示，当前参数是可以保存的！*/
			MULTL_SetSaveMark(s_SFNHandle.m_pMainHandle, TRUE);

			s_SFNHandle.m_SFNModeSIPRecvTimeout = SFN_SIP_RECV_TIMEOUT_MS * 2;

			s_SFNHandle.m_ForceUpdate = FALSE;
		}

		if (plStatus->m_SFNSIPINDVUpdated == TRUE)
		{
			if (plParam->m_bUseIndvSIP == TRUE)
			{
				/*应用参数*/
			}
			else
			{
				GLOBAL_TRACE(("USE INDV SIP Info Forbidden!!!!!!\n"));
			}
			plStatus->m_SFNSIPINDVUpdated = FALSE;
		}


		/*MUTE RF设置*/
		{
			BOOL lLastMuteStatus;

			lLastMuteStatus = plStatus->m_RFMuted;

			plStatus->m_RFMuted = FALSE;

			if (plStatus->m_InputLosted && plParam->m_bTsLostMUTE)
			{
				GLOBAL_TRACE(("Input Lost MUTE\n"));
				plStatus->m_RFMuted = TRUE;
			}

			if (((plStatus->m_bInt10MStatus == FALSE) && (plParam->m_bUse10MClkSynSrc == 0)) && plParam->m_bREFLostMUTE)
			{
				GLOBAL_TRACE(("REF 0 Lost MUTE\n"));
				plStatus->m_RFMuted = TRUE;
			}
			if (((plStatus->m_bExt10MStatus == FALSE) && (plParam->m_bUse10MClkSynSrc == 1)) && plParam->m_bREFLostMUTE)
			{
				GLOBAL_TRACE(("REF 1 Lost MUTE\n"));
				plStatus->m_RFMuted = TRUE;
			}
			if (((plStatus->m_1PPSLosted) && (plParam->m_bUse10MClkSynSrc == 2)) && plParam->m_bREFLostMUTE)
			{
				GLOBAL_TRACE(("REF 2 Lost MUTE\n"));
				plStatus->m_RFMuted = TRUE;
			}

			if (plStatus->m_1PPSLosted && plParam->m_b1PPSLostMUTE)
			{
				GLOBAL_TRACE(("1PPS Lost MUTE\n"));
				plStatus->m_RFMuted = TRUE;
			}

			if (plStatus->m_SIPLosted && plParam->m_bSIPLostMUTE)
			{
				GLOBAL_TRACE(("SIP Lost MUTE\n"));
				plStatus->m_RFMuted = TRUE;
			}

			if (lLastMuteStatus == FALSE)
			{
				if (plStatus->m_RFMuted == TRUE)
				{
					GLOBAL_TRACE(("RF Muted!!!!!\n"));
					s_SFNHandle.m_pMainHandle->m_Parameter.m_pOutChannel[0].m_pSubChannelNode[0].m_SubInfo.m_SubModulatorInfo.m_RFMute = TRUE;
					MULTL_ApplyQAMParameter(s_SFNHandle.m_pMainHandle, 0);
				}
			}
			else
			{
				if (plStatus->m_RFMuted == FALSE)
				{
					GLOBAL_TRACE(("RF UnMuted!!!!!\n"));
					s_SFNHandle.m_pMainHandle->m_Parameter.m_pOutChannel[0].m_pSubChannelNode[0].m_SubInfo.m_SubModulatorInfo.m_RFMute = FALSE;
					MULTL_ApplyQAMParameter(s_SFNHandle.m_pMainHandle, 0);
				}
			}
		}
	}
}

/*停止/恢复接收SIP，在搜索的时候*/
void MULT_SFNPauseSIPRecv(void)
{
	s_SFNHandle.m_SFNSIPPauseed = TRUE;
	if (s_SFNHandle.m_SFNParam.m_bUseSFN)
	{
		MULTL_SFNSIPStop();
	}
}

void MULT_SFNResumeSIPRecv(void)
{
	s_SFNHandle.m_SFNSIPPauseed = FALSE;
	if (s_SFNHandle.m_SFNParam.m_bUseSFN)
	{
		MULTL_SFNSIPStart();
	}
}

/*返回SFN模式是否开启的标志*/
BOOL MULT_SFNCheckEnabled(void)
{
	return s_SFNHandle.m_SFNParam.m_bUseSFN;
}

BOOL MULT_SFNCheckSIPRecved(void)
{
	return s_SFNHandle.m_SFNSIPRecvedAfterApply;
}

//单频网下设置之后如果收到新的SIP则调制模块会自动复位
void MULT_SFNForceCMDSIPUpdate(void)
{
	s_SFNHandle.m_ForceUpdate = TRUE;
	s_SFNHandle.m_SFNSIPRecvedAfterApply = FALSE;//此时认为SIP没有收到！
}

/*返回SFN模式下是否接收到SIP包*/
BOOL MULT_SFNCheckSFNError(S32 ErrorType, BOOL bClear)
{
	BOOL lRet = FALSE;
	switch (ErrorType)
	{
	case SFN_ERROR_SIP_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmSIPLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmSIPLostCount = 0;
			}
		}
		break;
	case SFN_ERROR_SIP_CHANGE:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmSIPChangeCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmSIPChangeCount = 0;
			}
		}
		break;
	case SFN_ERROR_SIP_CRC32_ERR:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmSIPCRC32ErrCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmSIPCRC32ErrCount = 0;
			}
		}
		break;
	case SFN_ERROR_INT_1PPS_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmInt1PPSLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmInt1PPSLostCount = 0;
			}
		}
		break;
	case SFN_ERROR_EXT_1PPS_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmExt1PPSLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmExt1PPSLostCount = 0;
			}
		}
		break;
	case SFN_ERROR_INT_10M_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmInt10MLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmInt10MLostCount = 0;
			}
		}
		break;
	case SFN_ERROR_EXT_10M_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmExt10MLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmExt10MLostCount = 0;
			}
		}
		break;
	case SFN_ERROR_ASI_LOST:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmTSXLostCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmTSXLostCount = 0;
			}
		}
		break;
	case SFN_ERROR_BITRATE_ERROR:
		{
			lRet = ((s_SFNHandle.m_SFNStatus.m_AlarmBitrateErrorCount > 0)?TRUE:FALSE);
			if (bClear)
			{
				s_SFNHandle.m_SFNStatus.m_AlarmBitrateErrorCount = 0;
			}
		}
		break;
	default:
		break;;
	}

	return lRet;
}

void MULT_SFNSetInternalCOMData(U8 *pData, S32 DataSize)
{
	MULT_SFN_Status *plStatus;
	MULT_SFN_Param	*plParam;

	plStatus = &s_SFNHandle.m_SFNStatus;
	plParam = &s_SFNHandle.m_SFNParam;

	MULTL_SFNProtocolParser(pData, DataSize);

	if (plStatus->m_bInt10MStatus == FALSE)
	{
		plStatus->m_AlarmInt10MLostCount++;
	}

	if (plStatus->m_bExt10MStatus == FALSE && s_SFNHandle.m_SFNParam.m_bUseSFN)
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
			plStatus->m_1PPSLosted  = TRUE;
		}
	}


	if (plStatus->m_bExt1PPSStatus == FALSE && s_SFNHandle.m_SFNParam.m_bUseSFN)
	{
		if (s_SFNHandle.m_SFNParam.m_bUseEx1PPS == TRUE)
		{
			plStatus->m_AlarmExt1PPSLostCount++;
			plStatus->m_1PPSLosted  = TRUE;
		}
	}




	if (((plStatus->m_bTS0Status == FALSE) && (s_SFNHandle.m_SFNParam.m_SFNASIMode == 0x01)) || ((plStatus->m_bTS1Status == FALSE) && (s_SFNHandle.m_SFNParam.m_SFNASIMode == 0x02)) || ((plStatus->m_bTS0Status == FALSE) && (plStatus->m_bTS1Status == FALSE) && (s_SFNHandle.m_SFNParam.m_SFNASIMode == 0x03)))
	{
		if (s_SFNHandle.m_SFNStatus.m_CurrentUseTsInd == 0)
		{
			s_SFNHandle.m_SFNStatus.m_AlarmTSXLostCount++;
			plStatus->m_InputLosted  = TRUE;
		}
	}

}

/*销毁当前模块*/
void MULT_SFNTerminate(void)
{
	MULTL_SFNSIPStop();
}

/*SFN内部通讯协议*/
void MULTL_SFNProtocolParser(U8 *pData, S32 DataSize)
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

				s_SFNHandle.m_SFNStatus.m_bInt1PPSStatus = (lTmpValue >> 7) & 0x01;
				s_SFNHandle.m_SFNStatus.m_bExt1PPSStatus = (lTmpValue >> 6) & 0x01;
				s_SFNHandle.m_SFNStatus.m_bInt10MStatus = (lTmpValue >> 5) & 0x01;
				s_SFNHandle.m_SFNStatus.m_bExt10MStatus = (lTmpValue >> 4) & 0x01;
				s_SFNHandle.m_SFNStatus.m_bTS1Status = (lTmpValue >> 2) & 0x03;
				s_SFNHandle.m_SFNStatus.m_bTS0Status = (lTmpValue >> 0) & 0x03;

#ifdef	SFN_DEBUG
				GLOBAL_TRACE(("lTmpValue = %.8X\n", lTmpValue));
				GLOBAL_TRACE(("In1PPS = %d, Ext1PPS = %d\n", s_SFNHandle.m_SFNStatus.m_bInt1PPSStatus, s_SFNHandle.m_SFNStatus.m_bExt1PPSStatus));
				GLOBAL_TRACE(("In10M = %d, Ext10M = %d\n", s_SFNHandle.m_SFNStatus.m_bInt10MStatus, s_SFNHandle.m_SFNStatus.m_bExt10MStatus));
				GLOBAL_TRACE(("Ts0 = %d, Ts1 = %d\n", s_SFNHandle.m_SFNStatus.m_bTS0Status, s_SFNHandle.m_SFNStatus.m_bTS1Status));
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


				GLOBAL_BYTES_S(plTmpData, 3);

				GLOBAL_MSB32_D(plTmpData, lTmpValue);

				//GLOBAL_TRACE(("Net Delay Count = %d ,Time = %f ms\n", lTmpValue, ((F64)lTmpValue * 100) / 1000000));

				s_SFNHandle.m_SFNStatus.m_NetDelay100ns = lTmpValue;


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


void MULTL_SFNProtocolSetParameter(MULT_SFN_Param *pParam, MULT_SFN_CMN_SIP *pModParameter)
{
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	S32 lLen;
	S32 lTmpAddDelay100ns;
	U32 lTmpValue;
	F64 lCurBitrate, lCur1sTSPacketNum, lCur1PacketPulseNum, lCurDelayPacketNum, lCurDelayPacketNumFloat;
	U32 lCurDelayPacketNumInt;
	U32	lCurDelayPacketNumFLoatPulseNum;
	S16 lPhyIndex;


	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	GLOBAL_MSB8_EC(plTmpBuf, 0x33/*SFN参数设置*/, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x05, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	lTmpValue = ((pParam->m_bUseSFN?1:0)<< 7) | (pParam->m_bUse10MClkSynSrc<< 5)  | ((pParam->m_bUseEx1PPS?1:0)<< 4) | ((pParam->m_bDeleteSIP?1:0)<< 3) | ((pParam->m_bEnableSatSFN?1:0)<< 2) | (pParam->m_SFNASIMode);

	GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);
	/*因为FPGA设置0值工作不正常，这里+1*/
	lTmpAddDelay100ns = pParam->m_SFNAddDelay100ns;
	if (lTmpAddDelay100ns == 0)
	{
		lTmpAddDelay100ns = 1;
	}

	GLOBAL_TRACE(("AddDelay = %d, NetDelay = %d, FPGA Delay Buffer Value = %f us\n", pParam->m_SFNAddDelay100ns, s_SFNHandle.m_SFNStatus.m_NetDelay100ns, ((F64)lTmpAddDelay100ns) / 10));

	GLOBAL_MSB24_EC(plTmpBuf, lTmpAddDelay100ns, lLen);
	GLOBAL_MSB16_EC(plTmpBuf, pParam->m_SatSFNNullPacketPID, lLen);//
	GLOBAL_BYTES_SC(plTmpBuf, 2, lLen);


	lCurBitrate = MULTL_SFNGetDTMBBitrate(pModParameter);//得到当前码率

	lCur1sTSPacketNum = lCurBitrate / 8 / 188;//得到当前码率1S的TS包的个数

	lCur1PacketPulseNum = (MULT_FPGA_MAIN_CLK_SFN * 1000000) / lCur1sTSPacketNum;//得到当前码率下1个包的FPGA的时钟个数

	lCurDelayPacketNum = (lTmpAddDelay100ns * lCur1sTSPacketNum) / (10000000);//得到当前延时对应的TS包的个数

	lCurDelayPacketNumInt = (U32)lCurDelayPacketNum;

	lCurDelayPacketNumFloat = lCurDelayPacketNum - lCurDelayPacketNumInt;

	lCurDelayPacketNumFLoatPulseNum = (U32)((MULT_FPGA_MAIN_CLK_SFN * 1000000) * lCurDelayPacketNumFloat / lCur1sTSPacketNum);

	GLOBAL_TRACE(("CurMode 1S PacketNum = %f, 1Packet Pulse Num = %f\n", lCur1sTSPacketNum, lCur1PacketPulseNum));

	GLOBAL_TRACE(("Delay100nsValue = %d PacketNum = %f, INT Part = %d, FloatPart = %f, PulseNum = %d\n", lTmpAddDelay100ns, lCurDelayPacketNum, lCurDelayPacketNumInt, lCurDelayPacketNumFloat, lCurDelayPacketNumFLoatPulseNum));

	lTmpValue = (U32)lCur1sTSPacketNum;
	GLOBAL_MSB32_EC(plTmpBuf, lTmpValue, lLen);//
	lTmpValue = (U32)lCur1PacketPulseNum;
	GLOBAL_MSB32_EC(plTmpBuf, lTmpValue, lLen);//
	GLOBAL_MSB16_EC(plTmpBuf, lCurDelayPacketNumInt, lLen);//
	GLOBAL_MSB16_EC(plTmpBuf, lCurDelayPacketNumFLoatPulseNum, lLen);//

	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}


void MULTL_SFNProtocolSetSIP(S32 CHNInd, S32 SlotInd, MULT_SFN_CMN_SIP *pSIPInfo, MULT_SFN_INDV_SIP *pINDVInfo)
{
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	S32 lLen;
	U32 lTmpValue;
	F64 lCurBitrate, lCur1sTSPacketNum, lCur1PacketPulseNum, lCurDelayPacketNum, lCurDelayPacketNumFloat;
	U32 lCurDelayPacketNumInt;
	U32	lCurDelayPacketNumFLoatPulseNum;
	S16 lPhyIndex;

	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	lLen = 0;
	plTmpBuf = plCMDBuf;

#ifdef SFN_DEBUG
	GLOBAL_TRACE(("-- Info For SIP Insert --\n"));
	MULTL_SFNSIPPrinter(pSIPInfo , pINDVInfo);
	GLOBAL_TRACE(("-- End--\n"));
#endif


	GLOBAL_MSB8_EC(plTmpBuf, 0x34/*SIP设置*/, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x30, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	GLOBAL_MSB16_EC(plTmpBuf, CHNInd, lLen);
	GLOBAL_MSB16_EC(plTmpBuf, SlotInd, lLen);

	lLen += MULTL_SFNSIPPacker(plTmpBuf, pSIPInfo, pINDVInfo);


	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}


/*SIP/MIP 搜索函数*/
void MULTL_SFNTsFilterCB(S32 SlotInd, U8 *pTsPacket)
{
	U16 lPID;
#ifdef SFN_DEBUG
	CAL_PrintDataBlock(__FUNCTION__, pTsPacket, 20);
#endif
	s_SFNHandle.m_SFNModeSIPRecvTimeout = SFN_SIP_RECV_TIMEOUT_MS;
	s_SFNHandle.m_SFNSIPTolerance = 0;
	s_SFNHandle.m_SFNStatus.m_SIPLosted = FALSE;


	if (s_SFNHandle.m_SFNParam.m_bEnableSatSFN)
	{
		if (s_SFNHandle.m_SFNParam.m_bSatSFNSIPCRC32Check)
		{
			U32 lCRCValue;
			lCRCValue = CRYPTO_CRC32(GLOBAL_U32_MAX, pTsPacket, MPEG2_TS_PACKET_SIZE);
			if (lCRCValue != 0)
			{
				//CAL_PrintDataBlock(__FUNCTION__, pTsPacket, 188);
				GLOBAL_TRACE(("SIP CRC32 Failed!!! CRCValue = %08X\n", lCRCValue));
				s_SFNHandle.m_SFNStatus.m_AlarmSIPCRC32ErrCount++;
				return;
			}
		}
	}

	lPID = MPEG2_TsGetPID(pTsPacket);
	if (lPID == SFN_SIP_PACKET_PID)
	{
		MULTL_SFNSIPParser(pTsPacket);
	}
	else
	{
		GLOBAL_TRACE(("TS Data Error! PID = %d\n", lPID));
	}
}


void MULTL_SFNSIPStart(void)
{
	MULTL_SFNSIPStop();

	GLOBAL_TRACE(("Start Recv SIP\n"));

	/*获取当前复用参数，得到输入TS IND*/
	{
		HANDLE32 lDBHandle;
		MPEG2_DBTsRouteInfo	lRouteInfo;


		lDBHandle = s_SFNHandle.m_pMainHandle->m_DBSHandle;
		MPEG2_DBGetTsRouteInfo(lDBHandle, MPEG2_DBGetTsIDs(lDBHandle, FALSE, 0), FALSE, &lRouteInfo);

		s_SFNHandle.m_SFNStatus.m_CurrentUseTsInd = lRouteInfo.m_TsIndex;
	}

	s_SFNHandle.m_FilterID = TSP_TsFilterAdd(0x10, s_SFNHandle.m_SFNStatus.m_CurrentUseTsInd, SFN_SIP_PACKET_PID, MULTL_SFNTsFilterCB, 0x0);
}

void MULTL_SFNSIPStop(void)
{
	if (s_SFNHandle.m_FilterID)
	{
		GLOBAL_TRACE(("Stop Recv SIP\n"));
		TSP_TsFilterRemove(s_SFNHandle.m_FilterID);
		s_SFNHandle.m_FilterID = 0;
	}
}

void MULTL_SFNSIPParser(U8 *pTsPacket)
{
	U8 *plTmpBuf;
	U16 lSISIP;
	MULT_SFN_Status *plStatus;
	MULT_SFN_CMN_SIP lTmpCMNSIP;
	MULT_SFN_CMN_SIP *plSFNSISIP;
	MULT_SFN_INDV_SIP lTmpSFNINDVSIP;
	MULT_SFN_INDV_SIP *plSFNINDVSIP;
	mxml_node_t *plXMLHolder;
	MULT_SubModulatorInfo *plSubModulatorInfo;

	plStatus = &s_SFNHandle.m_SFNStatus;
	plSFNSISIP = &plStatus->m_SFNCMNSIP;
	plSFNINDVSIP = &plStatus->m_SFNIndvSIP;


	/*时刻与实际使用的模式做比较！避免不一致的现象！*/
	{
		plSubModulatorInfo = &s_SFNHandle.m_pMainHandle->m_Parameter.m_pOutChannel[0].m_pSubChannelNode[0].m_SubInfo.m_SubModulatorInfo;
		MULTL_SFNSIPParamChange(plSubModulatorInfo, plSFNSISIP, TRUE);
	}

	if (s_SFNHandle.m_SFNSIPPauseed == FALSE)
	{
		plTmpBuf = &pTsPacket[4];//越过TS包头

		GLOBAL_MSB16_D(plTmpBuf, lSISIP);

		GLOBAL_ZEROMEM(&lTmpCMNSIP, sizeof(MULT_SFN_CMN_SIP));

		lTmpCMNSIP.m_SFNSIPPN = (lSISIP >> 14) & 0x03;
		lTmpCMNSIP.m_SFNSIPCarrier = (lSISIP >> 13) & 0x01;
		lTmpCMNSIP.m_SFNSIPConstellation = (lSISIP >> 10) & 0x07;
		lTmpCMNSIP.m_SFNSIPCodeRate = (lSISIP >> 8) & 0x03;
		lTmpCMNSIP.m_SFNSIPTI = (lSISIP >> 7) & 0x01;
		lTmpCMNSIP.m_SFNSIPDoublePilot = (lSISIP >> 6) & 0x01;
		lTmpCMNSIP.m_SFNSIPPN_Shift = (lSISIP >> 5) & 0x01;

		GLOBAL_MSB24_D(plTmpBuf, lTmpCMNSIP.m_SFNMaxDelay100NS);
		GLOBAL_MSB16_D(plTmpBuf, lTmpSFNINDVSIP.m_SFNAddr);
		GLOBAL_MSB24_D(plTmpBuf, lTmpSFNINDVSIP.m_SFNSIPIndviDelay);
		GLOBAL_MSB24_D(plTmpBuf, lTmpSFNINDVSIP.m_SFNSIPIndvFreqOffsetHz);
		GLOBAL_MSB16_D(plTmpBuf, lTmpSFNINDVSIP.m_SFNSIPIndvPower);
		lTmpSFNINDVSIP.m_SFNSIPIndvPowerMark = ((lTmpSFNINDVSIP.m_SFNSIPIndvPower & 0x8000) > 0)?TRUE:FALSE;
		lTmpSFNINDVSIP.m_SFNSIPIndvPower = lTmpSFNINDVSIP.m_SFNSIPIndvPower & 0x7FFF;



		if (GLOBAL_MEMCMP(plSFNSISIP, &lTmpCMNSIP, sizeof(MULT_SFN_CMN_SIP)) != 0)
		{
			//CAL_PrintDataBlock(__FUNCTION__, plSFNSISIP, sizeof(MULT_SFN_CMN_SIP));
			//CAL_PrintDataBlock(__FUNCTION__, &lTmpCMNSIP, sizeof(MULT_SFN_CMN_SIP));

			if (plStatus->m_SFNSIPRecvCount < 3)
			{
				plStatus->m_SFNSIPRecvCount++;
				GLOBAL_TRACE(("SIP Recv Count = %d\n", plStatus->m_SFNSIPRecvCount));

			}
			else
			{
				GLOBAL_MEMCPY(plSFNSISIP, &lTmpCMNSIP, sizeof(MULT_SFN_CMN_SIP));
				plStatus->m_SFNSIPRecvCount = 0;
				//CAL_PrintDataBlock(__FUNCTION__, pTsPacket, 24);
				if (s_SFNHandle.m_SFNParam.m_bUseCMNSIP)
				{
#ifdef SFN_DEBUG
					MULTL_SFNSIPPrinter(plSFNSISIP, NULL);
#endif
					/*SIP 已经更新*/
					s_SFNHandle.m_SFNSIPRecvedAfterApply = TRUE;

					GLOBAL_TRACE(("SIP Updated !!!!!!!!!!!!!!!!!!!\n"));
					plStatus->m_AlarmSIPChangeCount++;//SIP变化也作为报警项目

					/*改变主模块参数*/
					MULTL_SFNSIPParamChange(plSubModulatorInfo, plSFNSISIP, FALSE);


					s_SFNHandle.m_ForceUpdate = TRUE;
				}
			}
		}

		if (GLOBAL_MEMCMP(&lTmpSFNINDVSIP, plSFNINDVSIP, sizeof(MULT_SFN_INDV_SIP)) != 0)
		{
			if ((lTmpSFNINDVSIP.m_SFNAddr == s_SFNHandle.m_SFNParam.m_SFNAddrID) && (lTmpSFNINDVSIP.m_SFNAddr != 0))
			{
				s_SFNHandle.m_SFNStatus.m_SFNSIPINDVUpdated = TRUE;
			}
			GLOBAL_MEMCPY(plSFNINDVSIP, &lTmpSFNINDVSIP, sizeof(MULT_SFN_INDV_SIP));
		}

#ifdef SFN_DEBUG
		MULTL_SFNSIPPrinter(&lTmpCMNSIP, &lTmpSFNINDVSIP);;
#endif
	}


}

S32 MULTL_SFNSIPPacker(U8 *pTsBuf, MULT_SFN_CMN_SIP *pSIPInfo, MULT_SFN_INDV_SIP *pINDVInfo)
{
	if (pTsBuf && pSIPInfo && pINDVInfo)
	{
		U8 *plTmpData;
		U32 lTmpValue;

		plTmpData = pTsBuf;

		GLOBAL_MSB8_E(plTmpData, MPEG2_TS_PACKET_SYN_BYTE);
		GLOBAL_MSB8_E(plTmpData, 0x40);
		GLOBAL_MSB8_E(plTmpData, 0x15);
		GLOBAL_MSB8_E(plTmpData, 0x10);

		lTmpValue = 0;
		lTmpValue |= ((pSIPInfo->m_SFNSIPPN & 0x03) << 14);
		lTmpValue |= ((pSIPInfo->m_SFNSIPCarrier & 0x01) << 13);
		lTmpValue |= ((pSIPInfo->m_SFNSIPConstellation & 0x07) << 10);
		lTmpValue |= ((pSIPInfo->m_SFNSIPCodeRate & 0x03) << 8);
		lTmpValue |= ((pSIPInfo->m_SFNSIPTI & 0x01) << 7);
		lTmpValue |= ((pSIPInfo->m_SFNSIPDoublePilot & 0x01) << 6);
		lTmpValue |= ((pSIPInfo->m_SFNSIPPN_Shift & 0x01) << 5);


		GLOBAL_MSB16_E(plTmpData, lTmpValue);
		GLOBAL_MSB24_E(plTmpData, pSIPInfo->m_SFNMaxDelay100NS);
		GLOBAL_MSB16_E(plTmpData, pINDVInfo->m_SFNAddr);
		GLOBAL_MSB24_E(plTmpData, pINDVInfo->m_SFNSIPIndviDelay);
		GLOBAL_MSB24_E(plTmpData, pINDVInfo->m_SFNSIPIndvFreqOffsetHz);

		lTmpValue = 0;
		lTmpValue |= pINDVInfo->m_SFNSIPIndvPower & 0x7FFF;
		lTmpValue |= (((pINDVInfo->m_SFNSIPIndvPowerMark == TRUE)?1:0) << 15) & 0x8000;

		GLOBAL_MSB16_E(plTmpData, lTmpValue);

		GLOBAL_MEMSET(plTmpData, 0xFF, 169);

	}
	else
	{
		GLOBAL_TRACE(("NULL Ptr!!!!!!!!!!\n"));
	}

	return MPEG2_TS_PACKET_SIZE;
}



void MULTL_SFNSIPPrinter(MULT_SFN_CMN_SIP *pSFNSISIP, MULT_SFN_INDV_SIP *pSFNINDVSIP)
{
	if (pSFNSISIP)
	{
		MULTL_SFNMakeInfo();
		GLOBAL_TRACE(("~~~~~~~~~~~~~~~~~~~~~~~~~~~%s~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n",s_SFNHandle.m_pSFNTmpInfo));
		if (pSFNINDVSIP)
		{
			GLOBAL_TRACE(("MaxDelay = %d, Addr = %.4X, IndvDelay = %d, FreqOffset = %d, IndvPower/Mark = %d/%d\n", pSFNSISIP->m_SFNMaxDelay100NS, pSFNINDVSIP->m_SFNAddr, pSFNINDVSIP->m_SFNSIPIndviDelay, pSFNINDVSIP->m_SFNSIPIndvFreqOffsetHz, pSFNINDVSIP->m_SFNSIPIndvPower, pSFNINDVSIP->m_SFNSIPIndvPowerMark));
		}
	}
}


/*参数转换*/
void MULTL_SFNSIPParamChange(MULT_SubModulatorInfo *pSubModInfo, MULT_SFN_CMN_SIP *pSFNSISIP, BOOL bToSFNParam)
{
	if (pSubModInfo && pSFNSISIP)
	{
		if (bToSFNParam)
		{
			switch (pSubModInfo->m_PNMode)
			{
			case GS_MODULATOR_GUARD_INTERVAL_PN_420C:
				{
					pSFNSISIP->m_SFNSIPPN_Shift = 0;
					pSFNSISIP->m_SFNSIPPN = 0;
				}
				break;
			case GS_MODULATOR_GUARD_INTERVAL_PN_420F:
				{
					pSFNSISIP->m_SFNSIPPN_Shift = 1;
					pSFNSISIP->m_SFNSIPPN = 0;
				}
				break;
			case GS_MODULATOR_GUARD_INTERVAL_PN_595:
				{
					pSFNSISIP->m_SFNSIPPN_Shift = 0;
					pSFNSISIP->m_SFNSIPPN = 1;
				}
				break;
			case GS_MODULATOR_GUARD_INTERVAL_PN_945C:
				{
					pSFNSISIP->m_SFNSIPPN_Shift = 0;
					pSFNSISIP->m_SFNSIPPN = 2;
				}
				break;
			case GS_MODULATOR_GUARD_INTERVAL_PN_945F:
				{
					pSFNSISIP->m_SFNSIPPN_Shift = 1;
					pSFNSISIP->m_SFNSIPPN = 2;
				}
				break;
			default:
				pSFNSISIP->m_SFNSIPPN_Shift = 0;
				pSFNSISIP->m_SFNSIPPN = 1;
				break;
			}

			switch (pSubModInfo->m_CodeRate)
			{
			case GS_MODULATOR_CODE_RATE_0_4:
				pSFNSISIP->m_SFNSIPCodeRate = 0;
				break;
			case GS_MODULATOR_CODE_RATE_0_6:
				pSFNSISIP->m_SFNSIPCodeRate = 1;
				break;
			case GS_MODULATOR_CODE_RATE_0_8:
				pSFNSISIP->m_SFNSIPCodeRate = 2;
				break;
			default:
				pSFNSISIP->m_SFNSIPCodeRate = 2;
				break;
			}

			switch (pSubModInfo->m_CarrierMode)
			{
			case GS_MODULATOR_CARRIER_MODE_1:
				pSFNSISIP->m_SFNSIPCarrier = 0;
				break;
			case GS_MODULATOR_CARRIER_MODE_3780:
				pSFNSISIP->m_SFNSIPCarrier = 1;
				break;
			default:
				pSFNSISIP->m_SFNSIPCarrier = 0;
				break;
			}

			switch (pSubModInfo->m_Mode)
			{
			case GS_MODULATOR_QAM_4NR:
				pSFNSISIP->m_SFNSIPConstellation = 0;
				break;
			case GS_MODULATOR_QAM_4:
				pSFNSISIP->m_SFNSIPConstellation = 1;
				break;
			case GS_MODULATOR_QAM_16:
				pSFNSISIP->m_SFNSIPConstellation = 2;
				break;
			case GS_MODULATOR_QAM_32:
				pSFNSISIP->m_SFNSIPConstellation = 3;
				break;
			case GS_MODULATOR_QAM_64:
				pSFNSISIP->m_SFNSIPConstellation = 4;
				break;
			default:
				pSFNSISIP->m_SFNSIPConstellation = 3;
				break;
			}

			switch (pSubModInfo->m_TimeInterleave)
			{
			case GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240:
				pSFNSISIP->m_SFNSIPTI = 0;
				break;
			case GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_720:
				pSFNSISIP->m_SFNSIPTI = 1;
				break;
			default:
				pSFNSISIP->m_SFNSIPTI = 0;
				break;
			}

			switch (pSubModInfo->m_DoublePilot)
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
			switch (pSFNSISIP->m_SFNSIPPN)
			{
			case 0:
				if (pSFNSISIP->m_SFNSIPPN_Shift)
				{
					pSubModInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_420F;
				}
				else
				{
					pSubModInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_420C;
				}
				break;
			case 1:
				pSubModInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_595;
				break;
			case 2:
				if (pSFNSISIP->m_SFNSIPPN_Shift)
				{
					pSubModInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_945F;
				}
				else
				{
					pSubModInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_945C;
				}
				break;
			default:
				pSubModInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_420C;
				break;
			}

			switch (pSFNSISIP->m_SFNSIPCodeRate)
			{
			case 0:
				pSubModInfo->m_CodeRate = GS_MODULATOR_CODE_RATE_0_4;
				break;
			case 1:
				pSubModInfo->m_CodeRate = GS_MODULATOR_CODE_RATE_0_6;
				break;
			case 2:
				pSubModInfo->m_CodeRate = GS_MODULATOR_CODE_RATE_0_8;
				break;
			default:
				pSubModInfo->m_CodeRate = GS_MODULATOR_CODE_RATE_0_4;
				break;
			}

			switch (pSFNSISIP->m_SFNSIPCarrier)
			{
			case 0:
				pSubModInfo->m_CarrierMode = GS_MODULATOR_CARRIER_MODE_1;
				break;
			case 1:
				pSubModInfo->m_CarrierMode = GS_MODULATOR_CARRIER_MODE_3780;
				break;
			default:
				pSubModInfo->m_CarrierMode = GS_MODULATOR_CARRIER_MODE_1;
				break;
			}


			switch (pSFNSISIP->m_SFNSIPConstellation)
			{
			case 0:
				pSubModInfo->m_Mode = GS_MODULATOR_QAM_4NR;
				break;
			case 1:
				pSubModInfo->m_Mode = GS_MODULATOR_QAM_4;
				break;
			case 2:
				pSubModInfo->m_Mode = GS_MODULATOR_QAM_16;
				break;
			case 3:
				pSubModInfo->m_Mode = GS_MODULATOR_QAM_32;
				break;
			case 4:
				pSubModInfo->m_Mode = GS_MODULATOR_QAM_64;
				break;
			default:
				pSubModInfo->m_Mode = GS_MODULATOR_QAM_64;
				break;
			}


			switch (pSFNSISIP->m_SFNSIPTI)
			{
			case 0:
				pSubModInfo->m_TimeInterleave = GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240;
				break;
			case 1:
				pSubModInfo->m_TimeInterleave = GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_720;
				break;
			default:
				pSubModInfo->m_TimeInterleave = GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240;
				break;
			}


			switch (pSFNSISIP->m_SFNSIPDoublePilot)
			{
			case 0:
				pSubModInfo->m_DoublePilot = FALSE;
				break;
			case 1:
				pSubModInfo->m_DoublePilot = TRUE;
				break;
			default:
				pSubModInfo->m_DoublePilot = FALSE;
				break;
			}
		}
	}
}


/*生成信息字符串*/
void MULTL_SFNMakeInfo(void)
{
	CHAR_T *plTmpStr;
	MULT_SFN_CMN_SIP *plSFNSISIP;

	plSFNSISIP = &s_SFNHandle.m_SFNStatus.m_SFNCMNSIP;

	GLOBAL_ZEROMEM(s_SFNHandle.m_pSFNTmpInfo, sizeof(s_SFNHandle.m_pSFNTmpInfo));

	plTmpStr = s_SFNHandle.m_pSFNTmpInfo;

	if (plSFNSISIP->m_SFNSIPCarrier == 0x00)
	{
		GLOBAL_STRCAT(plTmpStr, "C1");
	}
	else if (plSFNSISIP->m_SFNSIPCarrier == 0x01)
	{
		GLOBAL_STRCAT(plTmpStr, "C3780");
	}

	GLOBAL_STRCAT(plTmpStr, " ");

	if (plSFNSISIP->m_SFNSIPConstellation == 0x00)
	{
		GLOBAL_STRCAT(plTmpStr, "4QAMNR");
	}
	else if (plSFNSISIP->m_SFNSIPConstellation == 0x01)
	{
		GLOBAL_STRCAT(plTmpStr, "4QAM");
	}
	else if (plSFNSISIP->m_SFNSIPConstellation == 0x02)
	{
		GLOBAL_STRCAT(plTmpStr, "16QAM");
	}
	else if (plSFNSISIP->m_SFNSIPConstellation == 0x03)
	{
		GLOBAL_STRCAT(plTmpStr, "32QAM");
	}
	else if (plSFNSISIP->m_SFNSIPConstellation == 0x04)
	{
		GLOBAL_STRCAT(plTmpStr, "64QAM");
	}

	GLOBAL_STRCAT(plTmpStr, "&");

	if (plSFNSISIP->m_SFNSIPCodeRate == 0x00)
	{
		GLOBAL_STRCAT(plTmpStr, "0.4");
	}
	else if (plSFNSISIP->m_SFNSIPCodeRate == 0x01)
	{
		GLOBAL_STRCAT(plTmpStr, "0.6");
	}
	else if (plSFNSISIP->m_SFNSIPCodeRate == 0x02)
	{
		GLOBAL_STRCAT(plTmpStr, "0.8");
	}

	GLOBAL_STRCAT(plTmpStr, " ");


	if (plSFNSISIP->m_SFNSIPPN == 0x00)
	{
		if (plSFNSISIP->m_SFNSIPPN_Shift == 0)
		{
			GLOBAL_STRCAT(plTmpStr, "PN420");
		}
		else
		{
			GLOBAL_STRCAT(plTmpStr, "PN420F");
		}
	}
	else if (plSFNSISIP->m_SFNSIPPN == 0x01)
	{
		if (plSFNSISIP->m_SFNSIPPN_Shift == 0)
		{
			GLOBAL_STRCAT(plTmpStr, "PN595");
		}
		else
		{
			GLOBAL_STRCAT(plTmpStr, "PN595F");
		}
	}
	else if (plSFNSISIP->m_SFNSIPPN == 0x02)
	{
		if (plSFNSISIP->m_SFNSIPPN_Shift == 0)
		{
			GLOBAL_STRCAT(plTmpStr, "PN945");
		}
		else
		{
			GLOBAL_STRCAT(plTmpStr, "PN945F");
		}
	}

	GLOBAL_STRCAT(plTmpStr, " ");

	if (plSFNSISIP->m_SFNSIPTI == 0x00)
	{
		GLOBAL_STRCAT(plTmpStr, "M240");
	}
	else if (plSFNSISIP->m_SFNSIPTI == 0x01)
	{
		GLOBAL_STRCAT(plTmpStr, "M720");
	}

	GLOBAL_STRCAT(plTmpStr, " ");

	if (plSFNSISIP->m_SFNSIPDoublePilot == 0x00)
	{
		GLOBAL_STRCAT(plTmpStr, "");
	}
	else if (plSFNSISIP->m_SFNSIPDoublePilot == 0x01)
	{
		GLOBAL_STRCAT(plTmpStr, "DP");
	}

}


F64 MULTL_SFNGetDTMBBitrate(MULT_SFN_CMN_SIP *pCMNSIP)
{
	F64 lRet;

	lRet = 1;

#ifdef SFN_DEBUG
	GLOBAL_TRACE(("-- Mode For Calculate Bitrate --\n"));
	MULTL_SFNSIPPrinter(pCMNSIP , NULL);
	GLOBAL_TRACE(("-- End--\n"));
#endif


	if (pCMNSIP)
	{
		S32 lPNValue;
		S32 lRMValue;
		F64 lICValue;

		switch (pCMNSIP->m_SFNSIPPN)
		{
		case 0:
			lPNValue = 420;
			break;
		case 1:
			lPNValue = 595;
			break;
		case 2:
			lPNValue = 945;
			break;
		default:
			lPNValue = 945;
			break;
		}

		switch (pCMNSIP->m_SFNSIPConstellation)
		{
		case 0:
			lRMValue = 1;
			break;
		case 1:
			lRMValue = 2;
			break;
		case 2:
			lRMValue = 4;
			break;
		case 3:
			lRMValue = 5;
			break;
		case 4:
			lRMValue = 6;
			break;
		default:
			lRMValue = 1;
			break;
		}

		switch (pCMNSIP->m_SFNSIPCodeRate)
		{
		case 0:
			lICValue = 0.4;
			break;
		case 1:
			lICValue = 0.6;
			break;
		case 2:
			lICValue = 0.8;
			break;
		default:
			lICValue = 0.4;
			break;
		}


		lRet = ((F64)(3744 + 16)) / (lPNValue + 3780) * lICValue * lRMValue * (7.56 * 1000000);
	}

	//GLOBAL_TRACE(("Current DTMB Bitrate = %f\n", lRet));

	return lRet;
}


/*外部参数，状态，获取设置*/
void MULT_SFNProcSFNParam(MULT_SFN_Param *pParam, BOOL bRead)
{
	if (pParam)
	{
		if (bRead)
		{
			GLOBAL_MEMCPY(pParam, &s_SFNHandle.m_SFNParam, sizeof(MULT_SFN_Param));
		}
		else
		{
			GLOBAL_MEMCPY(&s_SFNHandle.m_SFNParam, pParam, sizeof(MULT_SFN_Param));
#ifdef SUPPORT_CLK_ADJ_MODULE
			MULT_CLKSet10MCLKSYNCSyc(s_SFNHandle.m_SFNParam.m_bUse10MClkSynSrc);
#endif
		}
	}
}

void MULT_SFNGetSFNStatus(MULT_SFN_Status *pStatus)
{
	if (pStatus)
	{
		GLOBAL_MEMCPY(pStatus, &s_SFNHandle.m_SFNStatus, sizeof(MULT_SFN_Status));
	}
}



void MULT_SFNApplyByQAMModule(void)
{
	if (MULT_SFNCheckEnabled())
	{
		HWL_QAMForceNotLevelOnly();//确保下面函数中会调用MULT_SFNApplyParameter()
		//MULT_SFNForceCMDSIPUpdate();//单频网时，会保证RF被设置2次，第一次会关闭RF，第二次则会打开！
	}
	else
	{
		//MULT_SFNApplyParameter();
	}
	MULTL_ApplyQAMParameter(s_SFNHandle.m_pMainHandle, 0);
}
#endif

/*EOF*/
