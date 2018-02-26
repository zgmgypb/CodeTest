/* Includes-------------------------------------------------------------------- */
#include "multi_main_internal.h"

#ifdef SUPPORT_REMOTE_COM_CONTROL
#include "multi_remote_control.h"
#endif
#ifdef SUPPORT_NTS_DPD_BOARD
#include "hwl_dpd_control.h"
#endif

#include "multi_hwl_eth.h"
#include "multi_hwl_igmp.h"
#include "multi_hwl_monitor.h"
#include "frp_card.h"
#ifdef GN1846
#include "multi_hwl_local_encoder.h"
#endif
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
static MULT_Handle *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

S16 MULTL_DBActionCB(void* pUserParam, S32 DBActionType, U8* pBuf,S16 Datalen)
{
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;

	GLOBAL_ASSERT(plHandle);

	if (DBActionType == MPEG2_DB_ACTION_IO_TYPE_MSG_SEARCH_END)
	{
		MULT_MainMsg lTmpMsg;
		lTmpMsg.m_MsgType = MULTI_MAIN_MSG_TYPE_INIPUT_ANALYZE_COMPLETE;
		lTmpMsg.m_MsgParam = 0;
		lTmpMsg.m_MsglValue = 0;
		lTmpMsg.m_MsgwValue = 0;
		if (PAL_MsgQueueSend(plHandle->m_MainMsgHandle, &lTmpMsg, sizeof(lTmpMsg)) == FALSE)
		{
			GLOBAL_TRACE(("Send DB Action CB Failed\n"));
		}
	}
	return 0;
}


U32 MULTL_DBDataFilterAddCB(void *pUserParam, MPEG2_DataFilter* pFilter)
{
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;

	GLOBAL_ASSERT(plHandle);

	return TSP_SectionRequestAdd(pFilter);
}

BOOL MULTL_DBDataFilterRemoveCB(void *pUserParam, U32 FilterID)
{
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;
	GLOBAL_ASSERT(plHandle);
	return TSP_SectionRequestRemove(FilterID);
}


void MULTL_DBFilterDataArriveCB(void* pUserParam, U32 CallerID, void* pData, S32 DataCount)
{
	MULT_MainMsg lTmpMsg;
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;

	GLOBAL_ASSERT(plHandle);
	lTmpMsg.m_MsgType = MULTI_MAIN_MSG_TYPE_PSI_ARRIVE;
	lTmpMsg.m_MsgParam = 1;
	lTmpMsg.m_MsglValue = DataCount;
	lTmpMsg.m_MsgwValue = CallerID;
	if (PAL_MsgQueueSend(plHandle->m_MainMsgHandle, &lTmpMsg, sizeof(lTmpMsg)) == FALSE)
	{
		GLOBAL_TRACE(("Send DB Action CB Failed\n"));
	}
	//GLOBAL_TRACE(("Data Arrive CallerID = 0x%.8X\n", CallerID));
}

CHAR_T* MULTL_WEBDataCB(void *pUserParam, S32 WEBCBType, CHAR_T *pString)
{
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;
	GLOBAL_ASSERT(plHandle);
	/*解析XML或其它信息*/
	{
		MULT_MainMsg lTmpMsg;
		switch (WEBCBType)
		{
		case WEB_CB_XML_POST:
			{
				/*将控制命令信息备份后发送给主处理线程*/
				CHAR_T *plNewBuf;
				S32 lStrLen = GLOBAL_STRLEN(pString);
				GLOBAL_ASSERT(lStrLen < MULT_MAX_XML_STRING_SIZE);
				plNewBuf = (CHAR_T*) GLOBAL_MALLOC(lStrLen + 1);
				GLOBAL_STRCPY(plNewBuf, pString);

				lTmpMsg.m_MsgType = MULTI_MAIN_MSG_TYPE_WEB_XML_CMD;
				lTmpMsg.m_MsgParam = 0;
				lTmpMsg.m_MsglValue = lStrLen + 1;
				lTmpMsg.m_MsgwValue = (U32)plNewBuf;

				if (PAL_MsgQueueSend(plHandle->m_MainMsgHandle, &lTmpMsg, sizeof(lTmpMsg)) == FALSE)
				{
					GLOBAL_TRACE(("Send WEB Msg Failed!!!!!!!!\n"));
				}
				// 				GLOBAL_TRACE(("Got Post String = %s", pString));
			}
			break;
		case WEB_CB_XML_GET:
			{
				/*直接在本地处理，动态数据的传递！*/
				// 				GLOBAL_TRACE(("Got Get String = %s \n", pString));
				MULTL_WEBXMLGetProcess(plHandle, pString, GLOBAL_STRLEN(pString) + 1); 
			}
			break;
		case WEB_CB_DOWNLOAD_REQUEST:
			{
				/*下载不涉及到多线程问题，自行处理*/
				if (GLOBAL_STRCMP(pString, "parameter") == 0)
				{
					GLOBAL_TRACE(("Got Parameter Export CMD!!!!!!!!!!!\n"));
					MULTL_GenerateParamters(plHandle, CGIC_UPLOAD_FILE_PATHNAME, NULL);
					GLOBAL_STRCPY(plHandle->m_pReplayXMLBuf, CGIC_UPLOAD_FILE_PATHNAME);
				}
				else if (GLOBAL_STRCMP(pString, "software") == 0)
				{
					GLOBAL_TRACE(("Got Software Export CMD!!!!!!!!!!!\n"));
					// 					GLOBAL_STRCPY(plHandle->m_pReplayXMLBuf, "test_t.bin");
				}
				else if (GLOBAL_STRCMP(pString, "license") == 0)
				{
					GLOBAL_TRACE(("Got License Export CMD!!!!!!!!!!!\n"));
					GLOBAL_SPRINTF((plHandle->m_pReplayXMLBuf, "%s%s", MULT_STORAGE_BASE_DIR, MULT_LICENSE_FILE_PATHNAME));
				}
				//else if (GLOBAL_STRCMP(pString, "epg_xml") == 0)
				//{
				//	GLOBAL_TRACE(("Got EPG XML Export CMD!!!!!!!!!!!\n"));
				//	// 					GLOBAL_STRCPY(plHandle->m_pReplayXMLBuf, "test_t.bin");
				//}
				else if (GLOBAL_STRCMP(pString, "service_xml") == 0)
				{
					GLOBAL_TRACE(("Got Service XML Export CMD!!!!!!!!!!!\n"));
					MULTL_GenerateServiceXML(plHandle, CGIC_UPLOAD_FILE_PATHNAME);
					GLOBAL_TRACE(("Service XML Export File Generated OK!\n"));
					GLOBAL_STRCPY(plHandle->m_pReplayXMLBuf, CGIC_UPLOAD_FILE_PATHNAME);
				}
#ifdef SUPPORT_NTS_DPD_BOARD
				else if (GLOBAL_STRCMP(pString, "dpd_log") == 0)
				{
					GLOBAL_TRACE(("Got DPD LOG Export CMD!!!!!!!!!!!!!!!\n"));
					MULT_NTSDPDGenerateDPDLogFile();
					GLOBAL_STRCPY(plHandle->m_pReplayXMLBuf, NTS_DPD_LOG_FILE_PATH_NAME);
				}
#endif
			}
			break;
		default:
			GLOBAL_ASSERT(0);
			break;
		}
	}

	return plHandle->m_pReplayXMLBuf;
}


U32 MULTL_DataPIDMapCB(HANDLE32 Handle, S16 InTsIndex, U16 InPID, S16 OutTsIndex, U16 OutPID, BOOL bScramble, U32 Mark)
{
	MULT_Handle *plHandle = (MULT_Handle*)Handle;
	GLOBAL_ASSERT(plHandle);
	if (InTsIndex == GLOBAL_INVALID_INDEX && InPID == MPEG2_TS_PACKET_NULL_PID)
	{
		// 		TSP_ClearPIDMap();
	}
	else
	{
		TSP_AddPIDMap(InTsIndex, InPID, OutTsIndex, OutPID, bScramble, Mark);
	}
	return 0;
}


U32 MULTL_DataInserterAddCB(HANDLE32 Handle, S16 TsIndex, U8 *pData, S32 DataSize, S32 Interval)
{
	U32 lTmpID;
	MULT_Handle *plHandle = (MULT_Handle*)Handle;
	GLOBAL_ASSERT(plHandle);
	lTmpID = TSP_PSIInserterAdd(TsIndex, pData, DataSize, Interval);
	// GLOBAL_PRINTF(("Add Inserter Ts = %d, DataSize = %d, Interval = %d, Return ID = 0x%x\n", TsIndex, DataSize, Interval, lTmpID));
	return lTmpID;
}

BOOL MULTL_DataInserterRemoveCB(HANDLE32 Handle, U32 InserterID)
{
	// 	GLOBAL_PRINTF(("Remove Inserter ID = 0x%x\n", InserterID));
	return TSP_PSIInserterRemove(InserterID);
}


void MULTL_MonitorTimerCB(HANDLE32 Handle, U32 CallerID)
{
	MULT_MainMsg lTmpMsg;
	MULT_Handle *plHandle = (MULT_Handle*)Handle;

	GLOBAL_ASSERT(plHandle);

	lTmpMsg.m_MsgType = MULTI_MAIN_MSG_TYPE_MONITOR_TIMER;
	lTmpMsg.m_MsgParam = 1;
	lTmpMsg.m_MsglValue = 0;
	lTmpMsg.m_MsgwValue = CallerID;
	if (PAL_MsgQueueSend(plHandle->m_MainMsgHandle, &lTmpMsg, sizeof(lTmpMsg)) == FALSE)
	{
		GLOBAL_TRACE(("Send Monitor Timer CB Failed\n"));
	}
}



void MULTL_ForceSNTPSync(MULT_Handle *pHandle)
{
	pHandle->m_Monitor.m_NTPSyncTimeout = 0;
}

void MULTL_GenerateBasic(MULT_Handle *pHandle, HWL_HWInfo *pHWInfo)
{
	MULT_Information *plInfo;
	STRUCT_TM lTM;
	S32 lTmpLen;
	U8 lDevicename[20];
	plInfo = &pHandle->m_Information;
	/*系统信息*/
	/* SoftVersion */
	GLOBAL_STRCPY(plInfo->m_pSoftVersion, MULT_SOFT_VERSION);
#ifdef REQUEST_2014_0110_ONLY_GOS_CA
	GLOBAL_STRCAT(plInfo->m_pSoftVersion, "Pakistan");
#endif


	/* HardVersion */
	GLOBAL_STRCPY(plInfo->m_pHardVersion, MULT_HARD_VERSION);

	if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
	{
		GLOBAL_STRCPY(plInfo->m_pHardVersion, MULT_HARD_VERSION);
	}
	else
	{
		GLOBAL_STRCPY(plInfo->m_pHardVersion, "02.00");
	}


#ifdef GQ3760B
	switch(pHWInfo->m_HwType)
	{
	case 0:
		GLOBAL_STRCPY(plInfo->m_pHardVersion, "2.0");
		break;
	case 3:
		GLOBAL_STRCPY(plInfo->m_pHardVersion, "3.0");
		break;
	default:
		GLOBAL_TRACE(("Unknown HwType:%d \n", pHWInfo->m_HwType));
		break;
	}
#endif


#ifdef SUPPORT_2730_IP_BOARD_4CE55_SUPPORT
	if (DRL_GetMainBoardIS4CE55() == TRUE)
	{
		GLOBAL_STRCPY(plInfo->m_pHardVersion, MULT_HARD_VERSION_4CE55);
	}
	else
	{
		GLOBAL_STRCPY(plInfo->m_pHardVersion, MULT_HARD_VERSION);
	}
#endif


/* Release */

#ifdef SUPPORT_NEW_HWL_MODULE
	GLOBAL_SPRINTF((plInfo->m_pSoftRelease, "%s %s", __DATE__, __TIME__));
	CAL_StringTimeDateCovert2(plInfo->m_pSoftRelease, &lTM);
	CAL_StringDateToStr(plInfo->m_pSoftRelease, &lTM);
#else
	CAL_StringDateCovert(__DATE__, &lTM);
	GLOBAL_SPRINTF((plInfo->m_pSoftRelease, "%.4d-%.2d-%.2d", lTM.tm_year + 1900, lTM.tm_mon + 1, lTM.tm_mday));
#endif

#ifdef GM8358Q
	//add by leonli
	CHAR_T lMainFpgaRelease[16];
	MFPGA_GetRelease(lMainFpgaRelease);
	GLOBAL_STRCPY(plInfo->m_pFPGARelease, lMainFpgaRelease);
	GLOBAL_STRCPY(plInfo->m_pMuxFPGARelease, pHWInfo->m_pFPGARelease);
#else
	GLOBAL_STRCPY(plInfo->m_pFPGARelease, pHWInfo->m_pFPGARelease);
#endif

	MULTL_GenerateSN(MULT_DEVICE_TYPE, MULT_DEVICE_SUB_TYPE, pHWInfo->m_ChipID, plInfo->m_pSNString, sizeof(plInfo->m_pSNString));

#ifdef ENCODER_CARD_PLATFORM
	GLOBAL_STRCPY(plInfo->m_pModelName,MULT_DEFAULT_DEVICE_MODEL);
#else
	GLOBAL_STRCPY(lDevicename, MULT_DEFAULT_DEVICE_MODEL);

#ifdef SUPPORT_NEW_HWL_MODULE
	GLOBAL_STRCPY(plInfo->m_pModelName,MULT_DEFAULT_DEVICE_MODEL);
#else
	if(HWL_CHANNEL_TYPE_TUNER_S == pHWInfo->m_pInfoList[1].m_Type)
	{
		GLOBAL_STRCPY(plInfo->m_pModelName, GLOBAL_STRCAT(lDevicename, "-S"));

	}
	else if(HWL_CHANNEL_TYPE_TUNER_C == pHWInfo->m_pInfoList[1].m_Type)
	{
		GLOBAL_STRCPY(plInfo->m_pModelName,GLOBAL_STRCAT(lDevicename, "-C"));

	}
	else if(HWL_CHANNEL_TYPE_TUNER_DTMB == pHWInfo->m_pInfoList[1].m_Type)
	{
		GLOBAL_STRCPY(plInfo->m_pModelName, GLOBAL_STRCAT(lDevicename, "-D"));
	}
	else
	{
		GLOBAL_STRCPY(plInfo->m_pModelName,MULT_DEFAULT_DEVICE_MODEL);
	}


#ifdef GC1804B
	GLOBAL_STRCPY(plInfo->m_pModelName, lDevicename);
#endif

#ifdef GQ3768
	GLOBAL_STRCPY(plInfo->m_pModelName, lDevicename);
#endif

#endif
#endif

	//GLOBAL_STRCPY(plInfo->m_pModelName,MULT_DEFAULT_DEVICE_MODEL);

	GLOBAL_STRCPY(plInfo->m_pWEBENG, MULT_DEFAULT_DEVICE_NAME_ENG);
	GLOBAL_STRCPY(plInfo->m_pWEBCHN, MULT_DEFAULT_DEVICE_NAME_ENG);
	lTmpLen = CAL_CovertGB2312ToUTF8(MULT_DEFAULT_DEVICE_NAME_CHN, GLOBAL_STRLEN(MULT_DEFAULT_DEVICE_NAME_CHN), plInfo->m_pWEBCHN, sizeof(plInfo->m_pWEBCHN));
	plInfo->m_pWEBCHN[lTmpLen] = 0;
	GLOBAL_STRCPY(plInfo->m_pLCDENG, MULT_DEFAULT_DEVICE_LCD_ENG);
	GLOBAL_STRCPY(plInfo->m_pLCDCHN, MULT_DEFAULT_DEVICE_LCD_CHN);


}

void MULTL_GenerateOEM(MULT_Handle *pHandle)
{
	CHAR_T plTmpCMD[1024];
	MULT_Information *plInfo;
	plInfo = &pHandle->m_Information;

	plInfo->m_bHaveManual = FALSE;

	GLOBAL_SPRINTF((plTmpCMD, "%s%s", MULT_STORAGE_BASE_DIR, MULT_OEM_FILE_PATHNAME));
	MULTL_LoadOEM(pHandle, plTmpCMD, FALSE, NULL, 0);
}

/*SCS系统----------------------------------------------------------------------------------------------------------------------------------*/



void MULTL_TSCB(void *pUserParam, S32 TsIndex, U8 *pTsData, S32 DataLen)
{
	S32 i;
	for (i = 0; i < DataLen; i += MPEG2_TS_PACKET_SIZE)
	{
		HWL_SetDirectOutTsPacket(TsIndex, &pTsData[i], MPEG2_TS_PACKET_SIZE);
	}
}

#ifdef GM2730S
BOOL MULT_GetOutChannelState(S16 TsIndex)
{
	return s_pHandle->m_Parameter.m_pOutChannel[0].m_pSubChannelNode[TsIndex].m_ActiveMark;

}
#endif


/*应用直通参数*/
void MULTL_ApplyDirectRouteMode(MULT_Handle *pHandle)
{
	S32 i, lIOTsNum;
	U32 lTsIDs;
	MPEG2_DBTsRouteInfo	lRouteInfo;

#ifdef SUPPORT_NEW_HWL_MODULE
	{
		S32 k;
		MULT_ChannelNode *plCHNNode;
		MULT_SubChannelNode *plSubNode;
		for (i = 0; i < pHandle->m_Parameter.m_InChannelNumber; i++)
		{
			plCHNNode = &pHandle->m_Parameter.m_pInChannel[i];
			for (k = 0; k < plCHNNode->m_SubChannelNumber; k++)
			{
				plSubNode = &plCHNNode->m_pSubChannelNode[k];
				if (MPEG2_DBGetTsValid(pHandle->m_DBSHandle, plSubNode->m_CorrespondTsIndex, TRUE))
				{
					lTsIDs = MPEG2_DBGetTsIDs(pHandle->m_DBSHandle, TRUE, plSubNode->m_CorrespondTsIndex);
					MPEG2_DBGetTsRouteInfo(pHandle->m_DBSHandle, lTsIDs, TRUE, &lRouteInfo);
					HWL_DVBMuxRouteSet(k, lRouteInfo.m_TsIndex, lRouteInfo.m_ActiveMark, lRouteInfo.m_bRouteWithNullPacket, lRouteInfo.m_bRouteWithPCRCorrection);
				}

			}
			HWL_DVBMuxRouteApply(i, 250);
		}
	}
#else
	HWL_PassMode_t lMode;
#ifdef MULT_TS_BACKUP_SUPPORT
	lIOTsNum = MPEG2_DBGetTsCount(pHandle->m_DBSHandle, FALSE);

	/*关闭所有的直通*/
	for (i = 0; i < lIOTsNum; i++)
	{
		lMode.inputChannelId = 0;
		lMode.outputChanneldId = i;
		lMode.passMode = HWL_CONST_PASS_MODE_PIDMAP;
		HWL_PassModeSend(&lMode);
	}

	/*打开直通*/
	for (i = 0; i < lIOTsNum; i++)
	{
		lTsIDs = MPEG2_DBGetTsIDs(pHandle->m_DBSHandle, FALSE, i);
		MPEG2_DBGetTsRouteInfo(pHandle->m_DBSHandle, lTsIDs, FALSE, &lRouteInfo);
		if (lRouteInfo.m_ActiveMark == TRUE)
		{
			lMode.inputChannelId = MULT_BPGetTsBackupTs(lRouteInfo.m_TsIndex, TRUE);
			lMode.outputChanneldId = MULT_BPGetTsBackupTs(i, FALSE);
			GLOBAL_TRACE(("Backup  Ori/Actual Input %d/%d, Output %d/%d\n", lRouteInfo.m_TsIndex, lMode.inputChannelId, i, lMode.outputChanneldId));
			lMode.passMode = HWL_CONST_PASS_MODE_THROUGH;
			HWL_PassModeSend(&lMode);
		}
	}
#else
	lIOTsNum = MPEG2_DBGetTsCount(pHandle->m_DBSHandle, FALSE);
	for (i = 0; i < lIOTsNum; i++)
	{
		lTsIDs = MPEG2_DBGetTsIDs(pHandle->m_DBSHandle, FALSE, i);
		MPEG2_DBGetTsRouteInfo(pHandle->m_DBSHandle, lTsIDs, FALSE, &lRouteInfo);
#if defined(GM2730S)//注意这里修改是因为2730S在操作过程中，后面的通道参数会覆盖前面的直通参数
		if (i < 16)
#endif
		{
			if (lRouteInfo.m_ActiveMark == TRUE)
			{
				lMode.inputChannelId = lRouteInfo.m_TsIndex;
				lMode.outputChanneldId = i;
				lMode.passMode = HWL_CONST_PASS_MODE_THROUGH;
				GLOBAL_TRACE(("Route Mode From Ts %d to Ts %d\n", lRouteInfo.m_TsIndex, i));
			}
			else
			{
				lMode.inputChannelId = lRouteInfo.m_TsIndex;
				lMode.outputChanneldId = i;
				lMode.passMode = HWL_CONST_PASS_MODE_PIDMAP;
				//GLOBAL_TRACE(("PIDMap Mode Ts %d\n", i));
			}
			HWL_PassModeSend(&lMode);
		}
	}
#endif
#endif
}

/*应用调制参数*/

void MULTL_ApplyQAMParameter(MULT_Handle *pHandle, S32 Index)
{
	S32 i;
	MULT_ChannelNode *plChnNode;
	MULT_SubChannelNode *plSubNode;
	HWL_ModulatorParam_t lParam;
#ifdef GQ3768
	BOOL lDPDMark = FALSE;
#endif
	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));

	GLOBAL_TRACE(("Apply Modulator Parameters\n"));


	HWL_QAMFDACOffsetSet(pHandle->m_MaintSetting.m_PLLFreqOffset);

	if (pHandle->m_Parameter.m_OutChannelNumber > 0)
	{
#ifdef GN2000
		plChnNode = &pHandle->m_Parameter.m_pOutChannel[1];
#else
		plChnNode = &pHandle->m_Parameter.m_pOutChannel[0];
#endif
		if (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_DVB_C_MODULATOR)
		{
			GLOBAL_TRACE(("Set DVB-C Parameter, Max Count = %d\n", plChnNode->m_SubChannelNumber));
			for (i = 0; i < plChnNode->m_SubChannelNumber; i++)
			{
				plSubNode = &plChnNode->m_pSubChannelNode[i];
				lParam.AnalogBand = plSubNode->m_SubInfo.m_SubModulatorInfo.m_AnalogBand;
				lParam.ModulateFreq = plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq / 1000;
				lParam.Gain = plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel;
				lParam.ModulateStandard = plSubNode->m_SubInfo.m_SubModulatorInfo.m_ITUCoding;
				lParam.QamMode = plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode;
				lParam.QamSwitch = plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation;
				lParam.SpectrumInvert = plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv;
				lParam.SymRate = plSubNode->m_SubInfo.m_SubModulatorInfo.m_SymbolRate / 1000;
				lParam.Attenuator = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel;
#ifdef GN2000
				if (i > 0)
				{
					lParam.RFSwitch = FALSE;
				}
				else
				{
					lParam.RFSwitch = plSubNode->m_ActiveMark;
				}
#else
				if (i >= pHandle->m_Information.m_LicenseOutTsNum)//20121031 薛晶 修改BUG：避免恢复出厂设置后。被限制的通道仍然会有射频输出的问题！
				{
					lParam.RFSwitch = FALSE;
				}
				else
				{
					lParam.RFSwitch = plSubNode->m_ActiveMark;
				}
#endif
				HWL_QAMParameterSet(i, &lParam);
			}
		}
		else if (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_DVB_S_MODULATOR)
		{
			GLOBAL_TRACE(("Set DVB-S Parameter, Max Count = %d\n", plChnNode->m_SubChannelNumber));
			for (i = 0; i < plChnNode->m_SubChannelNumber; i++)
			{
				plSubNode = &plChnNode->m_pSubChannelNode[i];
				lParam.AnalogBand = plSubNode->m_SubInfo.m_SubModulatorInfo.m_AnalogBand;
				lParam.ModulateFreq = plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq / 1000;
				lParam.Gain = plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel;
				lParam.ModulateStandard = plSubNode->m_SubInfo.m_SubModulatorInfo.m_ITUCoding;
				lParam.QamMode = plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode;
				lParam.QamSwitch = plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation;
				lParam.SpectrumInvert = plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv;
				lParam.SymRate = plSubNode->m_SubInfo.m_SubModulatorInfo.m_SymbolRate / 1000;
				lParam.QPSK_FEC = plSubNode->m_SubInfo.m_SubModulatorInfo.m_FecEncode;
				lParam.Attenuator = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel;

#ifdef GN2000
				lParam.ModulateFreq = 474000 + 8000 * i;
				lParam.QamMode = HWL_CONST_MODULATOR_QAM_256;
				lParam.SymRate = 6952;
#endif

				if (i >= pHandle->m_Information.m_LicenseOutTsNum)//20121031 薛晶 修改BUG：避免恢复出厂设置后。被限制的通道仍然会有射频输出的问题！
				{
					lParam.RFSwitch = FALSE;
				}
				else
				{
					lParam.RFSwitch = plSubNode->m_ActiveMark;
				}
#ifdef GN2000
				if (i == 0)
				{
					lParam.RFSwitch = TRUE;
				}
				else
				{
					lParam.RFSwitch = FALSE;
				}
#endif
				HWL_QAMParameterSet(i, &lParam);
			}
		}
		else if (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_DTMB_MODULATOR)
		{
			for (i = 0; i < plChnNode->m_SubChannelNumber; i++)
			{
				plSubNode = &plChnNode->m_pSubChannelNode[i];
				lParam.AnalogBand = plSubNode->m_SubInfo.m_SubModulatorInfo.m_AnalogBand;
#ifdef USE_NEW_QAM_SETTING_FUNCTIONS
				lParam.ModulateFreq = plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq;
#else
				lParam.ModulateFreq = plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq / 1000;
#endif
#ifdef USE_NEW_QAM_SETTING_FUNCTIONS
				lParam.SymRate = 7560000;
#else
				lParam.SymRate = 7560;
#endif
				lParam.Gain = plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel;
				lParam.QamMode = plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode;
				lParam.QamSwitch = plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation;
				lParam.SpectrumInvert = plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv;

				lParam.DTMB_Carrier = plSubNode->m_SubInfo.m_SubModulatorInfo.m_CarrierMode;
				lParam.DTMB_PN = plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode;
				lParam.DTMB_CodeRate = plSubNode->m_SubInfo.m_SubModulatorInfo.m_CodeRate;
				lParam.DTMB_TI = plSubNode->m_SubInfo.m_SubModulatorInfo.m_TimeInterleave;
				lParam.DTMB_DoublePilot = plSubNode->m_SubInfo.m_SubModulatorInfo.m_DoublePilot;

				lParam.Attenuator = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel;
				lParam.FreqAdj = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_FreqAdj;
#ifdef GQ3768
				{
					U32 lValue;
					///*使用环出或者输出IP地址来设置作弊滤波器的设置开关和其它控制开关*/
					//HWL_ETHClear(TRUE);
					//if (plChnNode->m_ChannelInfo.m_ModulatorInfo.m_DPDMark)
					//{
					//	if (lParam.QamMode == HWL_CONST_MODULATOR_QAM_64)
					//	{
					//		lValue = 0x005028FF;
					//	}
					//	else
					//	{
					//		lValue = 0x005024FF;
					//	}
					//}
					//else
					//{
					//	lValue = 0x00502880;
					//}

					if (lParam.QamMode == HWL_CONST_MODULATOR_QAM_64)
					{
						lValue = 0x005028FF;
					}
					else
					{
						lValue = 0x005024FF;
					}
					GLOBAL_TRACE(("Mode SP Value = %.8X\n", lValue));
					HWL_ETHAdd(0, 0, 0, lValue, 5000, TRUE, TRUE, 1000000);
					HWL_ETHApply(0, TRUE);
				}
#endif


				if (i >= pHandle->m_Information.m_LicenseOutTsNum)//20121031 薛晶 修改BUG：避免恢复出厂设置后。被限制的通道仍然会有射频输出的问题！
				{
					lParam.RFSwitch = FALSE;
				}
				else
				{
					lParam.RFSwitch = plSubNode->m_ActiveMark;
				}

#ifndef USE_NEW_QAM_SETTING_FUNCTIONS
				HWL_QAMParameterSet(i, &lParam);
#else
				HWL_QAMParameterSetNew(i, &lParam, NULL);
#endif
			}
		}
		else
		{
			GLOBAL_TRACE(("Unknow Type = %d\n", plChnNode->m_ChannelType));
		}
	}

#ifdef SUPPORT_SFN_MODULATOR

#ifdef SUPPORT_FIR_II_RELOAD
	FIRII_Reload("/tmp/fir.txt");
#endif

	if (MULT_SFNCheckEnabled())
	{
		MULT_SFNPauseSIPRecv();
	}

	{
		//BOOL lSIPRecved;

		//lSIPRecved = MULT_SFNCheckSIPRecved();

		plChnNode = &pHandle->m_Parameter.m_pOutChannel[0];
		plSubNode = &plChnNode->m_pSubChannelNode[0];

#ifdef SUPPORT_NTS_DPD_BOARD
		{

			DPD_CONTROL_RF_PARAM lRFParam;
			lRFParam.m_CenterFreqHz = plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq;
			lRFParam.m_FreqAdj = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_FreqAdj;
			lRFParam.m_AttenuatorLevel = plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel * 5 + plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel;
			lRFParam.m_ALCLevel = lRFParam.m_AttenuatorLevel;
			lRFParam.m_ALCMark = FALSE;
			lRFParam.m_ToneMark = !plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation;
			lRFParam.m_SpecInv = plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv;
			lRFParam.m_GradualChangeMark = FALSE;
			lRFParam.m_RFDisableMark = !plSubNode->m_ActiveMark;


			/*如果不是LEVE ONLY 则 需要先关闭RF，*/
			if (HWL_QAMParameterCheckNeedResetRF() == TRUE && (plSubNode->m_ActiveMark == TRUE))
			{
				lRFParam.m_RFDisableMark = 1;//1是关闭RF！！！！
				lRFParam.m_GradualChangeMark = FALSE;
				DPD_ControlSetRFParam(&lRFParam);	

				/*复位调制模块*/
				HWL_QAMDTMBApplyFPGAParameters(lRFParam.m_ToneMark);

				/*复位单频网延时模块*/
				MULT_SFNApplyParameter();


			}

			lRFParam.m_GradualChangeMark = TRUE;//默认除掉关闭RF以外其余操作均渐变

			/*RF开关控制*/
			lRFParam.m_RFDisableMark = !plSubNode->m_ActiveMark;

			//if (MULT_SFNCheckEnabled())
			//{
			//	if (lSIPRecved == FALSE)
			//	{
			//		GLOBAL_TRACE(("SIP Not Recved!!!\n"));
			//		lRFParam.m_RFDisableMark = TRUE;
			//	}
			//}

			if (plSubNode->m_SubInfo.m_SubModulatorInfo.m_RFMute == TRUE)
			{
				lRFParam.m_RFDisableMark = TRUE;
			}

			if (lRFParam.m_RFDisableMark == TRUE)
			{
				lRFParam.m_GradualChangeMark = FALSE;
			}

			GLOBAL_TRACE(("Set Output RF Mark = %d, Gradual = %d!!!!!!!!!!!!!\n", !lRFParam.m_RFDisableMark, lRFParam.m_GradualChangeMark));

			DPD_ControlSetRFParam(&lRFParam);	


		}
#else
		{
			/*如果不是LEVE ONLY 则 需要先关闭RF，*/
			if ((HWL_QAMParameterCheckNeedResetRF() == TRUE) && (plSubNode->m_ActiveMark == TRUE))
			{
				/*关闭RF*/
				GLOBAL_TRACE(("Close RF For Reset!!!!!!!!!!!!!\n"));
				lParam.RFSwitch = FALSE;
				HWL_QAMParameterSetNew(0, &lParam, NULL);
				HWL_QAMParameterApply(0);

				/*复位DTMB模块*/
				HWL_QAMDTMBApplyFPGAParameters(!lParam.QamSwitch);

				PFC_TaskSleep(1);

				/*复位单频网延时模块*/
				MULT_SFNApplyParameter();


			}

			/*RF开关控制*/
			lParam.RFSwitch = plSubNode->m_ActiveMark;

			//if (MULT_SFNCheckEnabled())
			//{
			//	if (lSIPRecved == FALSE)
			//	{
			//		lParam.RFSwitch = FALSE;
			//	}
			//}

			if (plSubNode->m_SubInfo.m_SubModulatorInfo.m_RFMute == TRUE)
			{
				lParam.RFSwitch = FALSE;
			}

			GLOBAL_TRACE(("RF Switch = %d\n", lParam.RFSwitch));

			HWL_QAMParameterSetNew(0, &lParam, NULL);
			HWL_QAMParameterApply(0);
		}
#endif
	}

	if (MULT_SFNCheckEnabled())
	{
		MULT_SFNResumeSIPRecv();
	}
#else
	HWL_QAMParameterApply(0);
#endif



}

/*应用E3DS3参数*/
void MULTL_ApplyInE3DS3Parameter(MULT_Handle *pHandle, S32 ChnIndex)
{
	S32 i;
	GLOBAL_TRACE(("Apply InE3DS3 Parameters Index = %d\n", ChnIndex));
	if (GLOBAL_CHECK_INDEX(ChnIndex, pHandle->m_Parameter.m_InChannelNumber))
	{
		HWL_E3DS3Param_t lParam;
		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;
		GLOBAL_ZEROMEM(&lParam, sizeof(lParam));
		if (pHandle->m_Parameter.m_InChannelNumber > 0)  
		{
			plChnNode = &pHandle->m_Parameter.m_pInChannel[ ChnIndex ];
			if (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_E3DS3)
			{
				for (i = 0; i < plChnNode->m_SubChannelNumber; i++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[i];
					lParam.E3DS3Select = plSubNode->m_SubInfo.m_SubESDS3Info.m_E3DS3;				
					lParam.BitOrder = plSubNode->m_SubInfo.m_SubESDS3Info.m_Bitorder;
					lParam.FrameForm = plSubNode->m_SubInfo.m_SubESDS3Info.m_Frameform;
					lParam.InOutPacketLength = plSubNode->m_SubInfo.m_SubESDS3Info.m_Packetlength;
					lParam.ScrambleChanger = plSubNode->m_SubInfo.m_SubESDS3Info.m_Scramble;
					lParam.RSCoding = plSubNode->m_SubInfo.m_SubESDS3Info.m_RSCoding;
					lParam.InterweaveCoding = plSubNode->m_SubInfo.m_SubESDS3Info.m_InterweaveCoding;
					lParam.CodeRateRecover = plSubNode->m_SubInfo.m_SubESDS3Info.m_RateRecover;
					HWL_E3DS3SetParameter(i, &lParam);
				}
			}
		}
		HWL_E3DS3ApplyParameter(0);
	}
	else
	{
		for (i = 0; i < pHandle->m_Parameter.m_InChannelNumber; i++)
		{
			MULTL_ApplyInE3DS3Parameter(pHandle, i);
		}
	}

}

#ifdef GM8358Q
//#if 1
void MULTL_ApplyEncoderParameter(MULT_Handle *pHandle, S32 ChnIndex)
{
	MULT_Maintenace	 *plMaintSetting;
	plMaintSetting = &pHandle->m_MaintSetting;

	S32 i,j,k;
	S8 tempCount =0;
	if (GLOBAL_CHECK_INDEX(ChnIndex, pHandle->m_Parameter.m_InChannelNumber))
	{
		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;
		plChnNode = &pHandle->m_Parameter.m_pInChannel[ChnIndex];
		if(plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_ENCODER)
		{
			if(plChnNode->m_SubType == HWL_CHANNEL_SUBTYPE_ENCODER_CVBS)
			{
				for (i = 0; i < plChnNode->m_SubChannelNumber; i++)
				{

					plSubNode = &plChnNode->m_pSubChannelNode[i];

					for (j = 0; j < EncoderCvbsBoardSupportEntryNum; j++)
					{		
						//add by leonli for test
						/*
						GLOBAL_TRACE(("\n===================================Cvbs Channel %d Sub Encoder Entry %d================\n" , ChnIndex , j));
						GLOBAL_TRACE(("	m_VideoFormat = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat));
						GLOBAL_TRACE(("	m_Brightness = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Brightness));
						GLOBAL_TRACE(("	m_Contrast = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Contrast));
						GLOBAL_TRACE(("	m_Hue = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Hue));
						GLOBAL_TRACE(("	m_ImageHorizontalOffset = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_ImageHorizontalOffset));
						GLOBAL_TRACE(("	m_Saturation = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Saturation));
						GLOBAL_TRACE(("	m_Volume = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Volume));
						*/


						//GLOBAL_TRACE(("\n========================================Channel %d Sub Encoder Entry %d================\n" , ChnIndex , j));
						//GLOBAL_TRACE(("	==================m_Brightness = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Brightness));

						//if(FirstApply < 2)
						{
							HWL_Encoder_Cvbs_Add_Para(j,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Brightness,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Contrast,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Hue,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_ImageHorizontalOffset,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Saturation,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Volume
								);
						}


						/*
						GLOBAL_TRACE(("\n========================================visx ================\n"));
						GLOBAL_TRACE(("	m_WorkMod = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod));
						GLOBAL_TRACE(("	m_VideoBitRate = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoBitRate));
						GLOBAL_TRACE(("	m_VideoFormat = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat));
						GLOBAL_TRACE(("	m_Resolution = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Resolution));
						GLOBAL_TRACE(("	m_FrameRate = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_FrameRate));
						GLOBAL_TRACE(("	m_VideoAspect = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoAspect));
						GLOBAL_TRACE(("	m_VideoProfile = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoProfile));
						GLOBAL_TRACE(("	m_AudioBitRate = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioBitRate));
						GLOBAL_TRACE(("	m_AudioSampleRate = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioSampleRate));
						GLOBAL_TRACE(("	m_OutBitRate = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_OutBitRate));
						GLOBAL_TRACE(("	m_VideoEncodeMode = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoEncodeMode));
						GLOBAL_TRACE(("	m_AudioEncodeMode = %d\n" , plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEncodeMode));
						*/

						//if(FirstApply < 2)
						{
							HWL_Encoder_Vixs_Add_Para(tempCount,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoBitRate,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Resolution,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_FrameRate,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoAspect,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoProfile,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioBitRate,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioSampleRate,

								ChnIndex*4+j+0x80,
								ChnIndex*4+j+0x90,
								ChnIndex*4+j+0xB0,

								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_OutBitRate,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoEncodeMode,
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEncodeMode);
						}
						tempCount++;
						if(tempCount > 1)
						{
							tempCount = 0;
							GLOBAL_TRACE(("\n=====================================Apply Vixs Channel %d VixsIndex %d================\n" , ChnIndex , (ChnIndex*4+j)/2));
							HWL_Encoder_Vixs_Apply((ChnIndex*4+j)/2);//ChnIndex:0,1 / i:0.1.2.3

							//巡检参数 调试页面使用
							if (GLOBAL_CHECK_INDEX((ChnIndex*4+j)/2, GN_VIXS_SUBBOARD_NUM))
							{
								plMaintSetting->m_EncoderParaApplyCount[(ChnIndex*4+j)/2]++;
							}
						}
					}

				}

				HWL_Encoder_Cvbs_Apply(ChnIndex);

				{
					HWL_MfpgaCfgParam lMfpgaCfgPara;
					CHAR_T lProgName[64];

					lMfpgaCfgPara.m_AudioSampleFreq[0] = plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[0].m_AudioSampleRate;//edit
					lMfpgaCfgPara.m_AudioSampleFreq[1] = plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[0].m_AudioSampleRate;//edit
					lMfpgaCfgPara.m_TsId = 1;
					lMfpgaCfgPara.m_OnId = 1;
					lMfpgaCfgPara.m_Charset = MPEG2_PSI_CHAR_SET_GB2312;

					for(j = 0; j < GN_CVBS_SUBBOARD_NUM; j++ )
					{
						plChnNode = &pHandle->m_Parameter.m_pInChannel[j];
						plSubNode = &plChnNode->m_pSubChannelNode[0];
						for (i=0; i<GN_CVBS_CHANNEL_NUM; i++)
						{
							k = j*GN_CVBS_CHANNEL_NUM+i;
							lMfpgaCfgPara.m_ChParam[k].m_WorkEn = plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_WorkMod;//edit
							lMfpgaCfgPara.m_ChParam[k].m_VideoEncStandard = plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_VideoEncodeMode;//edit
							lMfpgaCfgPara.m_ChParam[k].m_AudioEncStandard = plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_AudioEncodeMode;//edit
							lMfpgaCfgPara.m_ChParam[k].m_ServiceId = k + 0x10;
							GLOBAL_SPRINTF((lProgName, "ProgName%d", k + 1));
							GLOBAL_STRCPY(lMfpgaCfgPara.m_ChParam[k].m_pServiceName, lProgName);

							lMfpgaCfgPara.m_ChParam[k].m_VideoPid =  k+0x80;
							lMfpgaCfgPara.m_ChParam[k].m_AudioPid =  k+0x90;

							//for HTML
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_VideoPid = k+0x80;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_AudioPid = k+0x90;

							if(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_PidEqualSwitch == ENCODER_PID_EQUAL_SWITCH_ON)
							{
								lMfpgaCfgPara.m_ChParam[k].m_PcrPid =  k+0x80;
								//for HTML
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_PcrPid = k+0x80;
							}
							else
							{
								lMfpgaCfgPara.m_ChParam[k].m_PcrPid =  k+0xB0;
								//for HTML
								plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_PcrPid = k+0xB0;
							}

							lMfpgaCfgPara.m_ChParam[k].m_PmtPid =  k+0xA0;
							//for HTML
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[i].m_PmtPid = k+0xA0;

							//GLOBAL_TRACE(("lMfpgaCfgPara  m_VideoPid = %d\n" , lMfpgaCfgPara.m_ChParam[k].m_VideoPid));
							//GLOBAL_TRACE(("lMfpgaCfgPara  m_AudioPid = %d\n" , lMfpgaCfgPara.m_ChParam[k].m_AudioPid));
							//GLOBAL_TRACE(("lMfpgaCfgPara  m_PcrPid = %d\n" , lMfpgaCfgPara.m_ChParam[k].m_PcrPid));

							lMfpgaCfgPara.m_ChParam[k].m_CbrStuffEn = FALSE;

						}
					}

					HWL_SetParaToMainFpga(&lMfpgaCfgPara);
					//MFPGA_SetOutSrcSel(MFPGA_OUT_SRC_SUBBOARD1_TS0);
				}
				//FirstApply++;

				GLOBAL_TRACE(("Apply Encoder Parameters Done!\n"));
			}

		}
		else
		{
			GLOBAL_TRACE(("Apply Encoder,ERROR  CHN TYPE=(%d)\n", plChnNode->m_ChannelType));
		}

	}
	else
	{
		GLOBAL_TRACE(("Error Chn Num = %d, Now Index = %d\n", pHandle->m_Parameter.m_InChannelNumber, ChnIndex));
	}

}
#endif

#if defined(GN1846) || defined(GN1866)
ENC_3531AViMode MULTL_Adv7612ViMode2LEncoderViMode(VideoStd_Type lVidStd)
{
	switch (lVidStd) {
		case VIDEO_STD_720_480_I:
			return VI_MODE_NTSC;
		case VIDEO_STD_720_576_I:
			return VI_MODE_PAL;
		case VIDEO_STD_1280_720P_50:
			return VI_MODE_720P_50;
		case VIDEO_STD_1280_720P_59_94:
			return VI_MODE_720P_5994;
		case VIDEO_STD_1280_720P_60:
			return VI_MODE_720P_60;
		case VIDEO_STD_1920_1080I_50:
			return VI_MODE_1080I_50;
		case VIDEO_STD_1920_1080I_59_94:
			return VI_MODE_1080I_5994;
		case VIDEO_STD_1920_1080I_60:
			return VI_MODE_1080I_60;
		case VIDEO_STD_1920_1080P_50:
			return VI_MODE_1080P_50;
		case VIDEO_STD_1920_1080P_59_94:
			return VI_MODE_1080P_5994;
		case VIDEO_STD_1920_1080P_60:
			return VI_MODE_1080P_60;
		default:
			return VI_MODE_1080P_60;
	}
}

ENC_3531AAudSample MULTL_Adv7612AudSample2LEncoderAudSample(samplerate_items lAudSample)
{
	switch (lAudSample) {
		case AUDIO_SAMPLE_32K:
			return AUD_SAMP_32K;
		case AUDIO_SAMPLE_44_1K:
			return AUD_SAMP_44_1K;
		case AUDIO_SAMPLE_48K:
			return AUD_SAMP_48K;
		default:
			return AUD_SAMP_48K;
	}
}

void MULTL_ApplyLEncoderParameter(MULT_Handle *pHandle, S32 ChnIndex)
{
	S32 i,j,k;
	S8 tempCount =0;

	if (GLOBAL_CHECK_INDEX(ChnIndex, pHandle->m_Parameter.m_InChannelNumber))
	{
		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;
		MULT_Maintenace *plMaintenance = &pHandle->m_MaintSetting;

		plChnNode = &pHandle->m_Parameter.m_pInChannel[ChnIndex];
		if(plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_ENCODER)
		{
			if(plChnNode->m_SubType == HWL_CHANNEL_SUBTYPE_ENCODER_HI3531A)
			{
				for (i = 0; i < plChnNode->m_SubChannelNumber; i++)
				{
					MULT_SubENCODERInfo *plEncInfo;
					HWL_LENCODER_SubParam lLEncoderSubParam;

					plSubNode = &plChnNode->m_pSubChannelNode[i];
					plEncInfo = &plSubNode->m_SubInfo.m_SubENCODERInfo;
				
					lLEncoderSubParam.m_bEnable = plEncInfo->m_ActiveMark;
					lLEncoderSubParam.m_UDPAddr = plEncInfo->m_IPv4Addr;
					lLEncoderSubParam.m_UDPPort = plEncInfo->m_IPv4Port;
					lLEncoderSubParam.m_EncParam.m_IpOutputType = pHandle->m_Configuration.m_IpOutputType; /* SPTS/MPTS */
					lLEncoderSubParam.m_EncParam.m_ActiveMark = lLEncoderSubParam.m_bEnable;
					lLEncoderSubParam.m_EncParam.m_SignalIsLocked = pHandle->m_Monitor.m_HdmiRxStatus[i].m_SignalIsLocked;

					lLEncoderSubParam.m_EncParam.m_DestIp = lLEncoderSubParam.m_UDPAddr;
					lLEncoderSubParam.m_EncParam.m_DestPort = lLEncoderSubParam.m_UDPPort;
					lLEncoderSubParam.m_EncParam.m_IpProto = plEncInfo->m_Protocol;
					lLEncoderSubParam.m_EncParam.m_SendTsNum = 7;
					lLEncoderSubParam.m_EncParam.m_ProgBitrate = plEncInfo->m_ProgBitrate;
					lLEncoderSubParam.m_EncParam.m_ProgNumber = plEncInfo->m_ProgNumber;
					GLOBAL_STRCPY(lLEncoderSubParam.m_EncParam.m_pProgName, plEncInfo->m_pProgName);
					lLEncoderSubParam.m_EncParam.m_PsiCharSet = pHandle->m_Configuration.m_OutputCharset;

					lLEncoderSubParam.m_EncParam.m_VidPid = plEncInfo->m_VidPid;
					lLEncoderSubParam.m_EncParam.m_PcrPid = plEncInfo->m_PcrPid;
					lLEncoderSubParam.m_EncParam.m_AudPid = plEncInfo->m_AudPid;
					lLEncoderSubParam.m_EncParam.m_PmtPid = plEncInfo->m_PmtPid;
					/* 注意下面这些值是否需要映射转换！！ */
					if (plEncInfo->m_ViMode == ENC_VI_MODE_AUTO) {
						lLEncoderSubParam.m_EncParam.m_ViMode = MULTL_Adv7612ViMode2LEncoderViMode(pHandle->m_Monitor.m_HdmiRxStatus[i].m_VideoStandard);
					}
					else {
						lLEncoderSubParam.m_EncParam.m_ViMode = plEncInfo->m_ViMode;
					}
					lLEncoderSubParam.m_EncParam.m_VoMode = plEncInfo->m_VoMode;
					lLEncoderSubParam.m_EncParam.m_Profile = plEncInfo->m_Profile;
					lLEncoderSubParam.m_EncParam.m_RcMode = plEncInfo->m_BrMode;
					if (lLEncoderSubParam.m_EncParam.m_RcMode == RC_MODE_CBR) {
						lLEncoderSubParam.m_EncParam.m_RcParam.m_CbrParam.m_BitRate = plEncInfo->m_Bitrate;
						lLEncoderSubParam.m_EncParam.m_RcParam.m_CbrParam.m_Gop = plEncInfo->m_Gop;
					}
					else if (lLEncoderSubParam.m_EncParam.m_RcMode == RC_MODE_VBR) {
						lLEncoderSubParam.m_EncParam.m_RcParam.m_VbrParam.m_MaxBitRate = plEncInfo->m_Bitrate;
						lLEncoderSubParam.m_EncParam.m_RcParam.m_VbrParam.m_Gop = plEncInfo->m_Gop;
						lLEncoderSubParam.m_EncParam.m_RcParam.m_VbrParam.m_MaxQp = 51;
						lLEncoderSubParam.m_EncParam.m_RcParam.m_VbrParam.m_MinQp = 16;
					}

					/* 音频参数 */
					lLEncoderSubParam.m_EncParam.m_AudEncMode = plEncInfo->m_AudEncMode;
					lLEncoderSubParam.m_EncParam.m_AudBitrate = plEncInfo->m_AudBitrate;
					if (plEncInfo->m_AudSample == ENC_AUD_SAMP_AUTO) {
						lLEncoderSubParam.m_EncParam.m_AudSample = MULTL_Adv7612AudSample2LEncoderAudSample(pHandle->m_Monitor.m_HdmiRxStatus[i].m_AudioSample);
					}
					else {
						lLEncoderSubParam.m_EncParam.m_AudSample = plEncInfo->m_AudSample;
					}
					lLEncoderSubParam.m_EncParam.m_AudVolume = plEncInfo->m_Volume;

					/* 调整参数 */
					lLEncoderSubParam.m_EncParam.m_AudPtsRelativeDelayTime = plMaintenance->m_AudPtsRelativeDelayTime[i];
					lLEncoderSubParam.m_EncParam.m_PtsDelayTime = plMaintenance->m_PtsDelayTime[i];
					lLEncoderSubParam.m_EncParam.m_MaxPtsPcrInterval = plMaintenance->m_MaxPtsPcrInterval[i];
					lLEncoderSubParam.m_EncParam.m_MinPtsPcrInterval = plMaintenance->m_MinPtsPcrInterval[i];
					lLEncoderSubParam.m_EncParam.m_AudDelayFrameNum = plMaintenance->m_AudDelayFrameNum[i];

					{ /* 对 HDMI 设置参数 */
						HDMI_RxCfgParam lCfgParam;

						lCfgParam.m_Brightness = 50;
						lCfgParam.m_Contrast = 50;
						lCfgParam.m_Hue = 50;
						lCfgParam.m_Saturation = 50;
						lCfgParam.m_IsAc3Bypass = ((lLEncoderSubParam.m_EncParam.m_AudEncMode == AUD_ENC_MODE_AC3 ? TRUE : FALSE)
							|| (lLEncoderSubParam.m_EncParam.m_AudEncMode == AUD_ENC_MODE_EAC3 ? TRUE : FALSE));
						
						HWL_HdmiRxSetParam(i, &lCfgParam);
					}

					HWL_LENCODER_SetSubParam(ChnIndex, i, &lLEncoderSubParam);
				}

				GLOBAL_TRACE(("Apply Encoder Parameters Done!\n"));
			}
		}
		else
		{
			GLOBAL_TRACE(("Apply Encoder,ERROR  CHN TYPE=(%d)\n", plChnNode->m_ChannelType));
		}
	}
	else
	{
		GLOBAL_TRACE(("Error Chn Num = %d, Now Index = %d\n", pHandle->m_Parameter.m_InChannelNumber, ChnIndex));
	}
}
#endif

/*应用Tuner参数*/
void MULTL_ApplyTunerParameter(MULT_Handle *pHandle, S32 ChnIndex)
{
	S32 i;
	if (GLOBAL_CHECK_INDEX(ChnIndex, pHandle->m_Parameter.m_InChannelNumber))
	{
		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;
		HWL_TunerParam_t lParam;
		GLOBAL_ZEROMEM(&lParam, sizeof(lParam));

		plChnNode = &pHandle->m_Parameter.m_pInChannel[ ChnIndex ];
		if((plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_TUNER_S) || (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_TUNER_C) || (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_TUNER_DTMB))
		{
			for (i = 0; i < plChnNode->m_SubChannelNumber; i++)
			{
				plSubNode = &plChnNode->m_pSubChannelNode[i];			
				lParam.Frequency = plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq / 10;				
				lParam.Symbol = plSubNode->m_SubInfo.m_SubTUNERInfo.m_SymbolRate / 1000;
				lParam.LocalFreq = plSubNode->m_SubInfo.m_SubTUNERInfo.m_LocalFreq / 10;
				lParam.Modulation = plSubNode->m_SubInfo.m_SubTUNERInfo.m_Modulation;
				if(GS_TUNER_ANNEX_C == plSubNode->m_SubInfo.m_SubTUNERInfo.m_Reqtype)
				{
					lParam.TuneReqType = GS_TUNER_ANNEX_A;
				}
				else
				{
					lParam.TuneReqType = plSubNode->m_SubInfo.m_SubTUNERInfo.m_Reqtype;
				}

				lParam.PolarMethod = plSubNode->m_SubInfo.m_SubTUNERInfo.m_PolarMethod;
				if (GLOBAL_CHECK_INDEX(i, 4))
				{
					pHandle->m_TunerSPolar[i] = lParam.PolarMethod;
				}
				lParam.Switch_22K = (~(plSubNode->m_SubInfo.m_SubTUNERInfo.m_22kSwitch)) & 0x01;
				lParam.SpectInv = plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv;
				HWL_TunerSetParameter(i, &lParam);
			}
		}
		HWL_TunerApplyParameter(plChnNode->m_SubChannelNumber,plChnNode->m_ChannelType);
	}
	else
	{
		for (i = 0; i < pHandle->m_Parameter.m_InChannelNumber; i++)
		{
			MULTL_ApplyTunerParameter(pHandle, i);
		}
	}
}

/*应用ASI输出参数*/
void MULTL_ApplyOutASIParameter(MULT_Handle *pHandle, S32 ChnIndex)
{
	S32 i;
	GLOBAL_TRACE(("Apply OUT ASI Parameters Index = %d\n", ChnIndex));

	if (GLOBAL_CHECK_INDEX(ChnIndex, pHandle->m_Parameter.m_OutChannelNumber))
	{
		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;


		plChnNode = &pHandle->m_Parameter.m_pOutChannel[ ChnIndex ];
		if (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_ASI)
		{
			HWL_ASIClear(0, FALSE);
			for(i = 0; i < plChnNode->m_SubChannelNumber; i++)
			{
				plSubNode = &plChnNode->m_pSubChannelNode[ i ];
				HWL_ASIAdd(0, plChnNode->m_OriginalIndex, i, plSubNode->m_Bitrate , FALSE, FALSE);
			}
			HWL_ASIApply(0, plChnNode->m_OriginalIndex, FALSE);	
		}
	}
	else
	{
		for (i = 0; i < pHandle->m_Parameter.m_OutChannelNumber; i++)
		{
			MULTL_ApplyOutASIParameter(pHandle, i);
		}
	}

}

/*应用ETH参数*/
void MULTL_ApplyInETHParameter(MULT_Handle *pHandle, S32 ChnIndex)
{
	S32 i, k;
	if (GLOBAL_CHECK_INDEX(ChnIndex, pHandle->m_Parameter.m_InChannelNumber))
	{
		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;

		plChnNode = &pHandle->m_Parameter.m_pInChannel[ ChnIndex ];
		if (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_IP)
		{
			HWL_IGMPClear();
			HWL_ETHClear(TRUE);
			HWL_IGMPSetting(pHandle->m_Parameter.m_IGMP.m_IGMPRepeateMark, pHandle->m_Parameter.m_IGMP.m_IGMPRepeateTime, pHandle->m_Parameter.m_IGMP.m_IGMPVersion);

			//GLOBAL_TRACE(("Set Chn %d InETH Param IP = %.8X\n", ChnIndex, plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress));
			HWL_ETHSetChnParameter(ChnIndex, plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress, plChnNode->m_ChannelInfo.m_IPInfo.m_MAC, TRUE, plChnNode->m_ChannelInfo.m_IPInfo.m_Bitrate);

			for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
			{
				plSubNode = &plChnNode->m_pSubChannelNode[k];
				if(plSubNode->m_ActiveMark == TRUE)
				{				
					HWL_ETHAdd(ChnIndex, k, plSubNode->m_SubInfo.m_SubIPInfo.m_Protocol, plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr, plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Port, plSubNode->m_ActiveMark, TRUE, plSubNode->m_Bitrate);
					HWL_IGMPTAdd(ChnIndex, plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr, (plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4LimitMark?plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4LimitAddr:0), 0);
				}
				else
				{
					HWL_ETHAdd(ChnIndex, k, plSubNode->m_SubInfo.m_SubIPInfo.m_Protocol, 0x7F000001, 61000 + k, TRUE, TRUE, plSubNode->m_Bitrate);//将关闭的通道的IP设置成非法地址以确保通道码率检测的正常

				}
			}
			HWL_ETHApply(ChnIndex, TRUE);
			HWL_IGMPApply(ChnIndex, TRUE);
		}
		else
		{
			GLOBAL_TRACE(("Type = %d\n", plChnNode->m_ChannelType));
		}
	}
	else
	{
		for (i = 0; i < pHandle->m_Parameter.m_InChannelNumber; i++)
		{
			MULTL_ApplyInETHParameter(pHandle, i);
		}
	}

}

void MULTL_ApplyOutETHParameter(MULT_Handle *pHandle, S32 ChnIndex)
{
	S32 i, k;
	S32 lEthPhyChn;
	GLOBAL_TRACE(("Apply Out ETH Parameters Index = %d\n", ChnIndex));

	if (GLOBAL_CHECK_INDEX(ChnIndex, pHandle->m_Parameter.m_OutChannelNumber))
	{
		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;
		plChnNode = &pHandle->m_Parameter.m_pOutChannel[ChnIndex];

		lEthPhyChn = 1;
#ifdef GM2730S
		lEthPhyChn = 0;
#endif

#ifdef SUPPORT_NEW_HWL_MODULE
		lEthPhyChn = ChnIndex;
#endif

		if (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_IP || plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_IP_LOOP)
		{
			/*设置输出通道*/
#ifndef GN1846 /* 1846 数据口与控制网口相同 */
			HWL_ETHSetChnParameter(lEthPhyChn, plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress, plChnNode->m_ChannelInfo.m_IPInfo.m_MAC, FALSE, plChnNode->m_ChannelInfo.m_IPInfo.m_Bitrate);
#endif
		}
		else if (plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_IP_DEP || plChnNode->m_ChannelType == HWL_CHANNEL_TYPE_IP_LOOP_DEP)
		{
			/*没有参数可以设置*/
		}
		else
		{
			GLOBAL_TRACE(("Unknow Channel Type = %d\n" ,plChnNode->m_ChannelType));
		}

		HWL_ETHClear(FALSE);
		for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
		{
			plSubNode = &plChnNode->m_pSubChannelNode[k];
			HWL_ETHAdd(lEthPhyChn, k, plSubNode->m_SubInfo.m_SubIPInfo.m_Protocol, plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr, plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Port, plSubNode->m_ActiveMark, FALSE, plSubNode->m_Bitrate);
		}
		HWL_ETHApply(lEthPhyChn, FALSE);
	}
	else
	{
		for (i = 0; i < pHandle->m_Parameter.m_OutChannelNumber; i++)
		{
			MULTL_ApplyOutETHParameter(pHandle, i);
		}
	}
}

/*应用复用参数*/
void MULTL_ApplyRemuxParameter(MULT_Handle *pHandle)
{

	if (PAL_CriticalEnter(pHandle->m_DBSLock, GLOBAL_INVALID_INDEX))
	{

#ifdef GN2000
		MULTI_MOSIACAdjustReMuxParameter(pHandle);
#endif
#ifndef DISABLE_SCS_MODULE
		/*关闭SCS模块*/
		MULTL_SCSManagerApply(pHandle, TRUE);
#endif
		/*清空数据库中的过滤器Slot*/
		MPEG2_DBClearInserter(pHandle->m_DBSHandle);

		/*以上操作将清空所有的过滤器，并停止对过滤器的访问，这里清空插入器模块中的过滤器Slot(本函数操作过程中因为TDT更新线程也是本线程，所以不存在同步错误)*/
		TSP_PSIInserterClear();
		/*清空PID映射表*/
		TSP_ClearPIDMap();
#ifdef MULT_UNUSED_TS_DISABLE_SUPPORT
		{
			S32 i, lTsNum;
			BOOL lTmpValue;
			lTsNum = MPEG2_DBGetTsCount(pHandle->m_DBSHandle, TRUE);
			for (i = 0; i < lTsNum; i++)
			{
				lTmpValue = MULTL_GetTsMark(pHandle, i, TRUE);
				MPEG2_DBSetTsValid(pHandle->m_DBSHandle, i, lTmpValue, TRUE);
				//GLOBAL_TRACE(("Set Input Ts %d Ts Valid = %d\n", i, lTmpValue));

#ifdef MULT_TS_BACKUP_SUPPORT
				if (lTmpValue == TRUE)
				{
					if (MULT_BPGetTsIsUsedByBackup(i, TRUE))
					{
						MPEG2_DBSetTsValid(pHandle->m_DBSHandle, i, FALSE, TRUE);
						GLOBAL_TRACE(("---Set Input Ts %d Ts Valid = %d By Backup Disable\n", i, FALSE));
					}
				}
#endif
			}

			lTsNum = MPEG2_DBGetTsCount(pHandle->m_DBSHandle, FALSE);
			for (i = 0; i < lTsNum; i++)
			{
				lTmpValue = MULTL_GetTsMark(pHandle, i, FALSE);
				MPEG2_DBSetTsValid(pHandle->m_DBSHandle, i,lTmpValue, FALSE);
				//GLOBAL_TRACE(("Set Output Ts %d Ts Valid = %d\n", i, lTmpValue));

#ifdef MULT_TS_BACKUP_SUPPORT
				if (lTmpValue == TRUE)
				{
					if (MULT_BPGetTsIsUsedByBackup(i, FALSE))
					{
						MPEG2_DBSetTsValid(pHandle->m_DBSHandle, i, FALSE, FALSE);
						GLOBAL_TRACE(("---Set Output Ts %d Ts Valid = %d By Backup Disable\n", i, FALSE));
					}
				}
#endif
			}

		}
#endif


#ifdef SUPPORT_SFN_MODULATOR
		//单频网启动状态 复用参数不能设置
		if (MULT_SFNCheckEnabled() == TRUE)
		{
			return;
		}
#endif

#ifdef MULT_SYSTEM_HAVE_PCR_CORRECT_ADJUST_FUNCTION
		/*应用PCR校验参数*/
		{
			MULT_PCRCorrect *plPCRCorrect = &pHandle->m_Parameter.m_PCRCorrect;
			HWL_SetChnPCRCorrectMode(0, plPCRCorrect->m_PCRCPos, plPCRCorrect->m_PCRCNeg, plPCRCorrect->m_PCRCMark);
		}
#endif

#ifdef SUPPORT_SFN_ADAPTER
		/*单频网适配器不支持复用！*/
		return;
#endif

#ifndef DISABLE_SCS_MODULE
		/*重启动SCS模块并应用参数*/
		MULTL_SCSManagerApply(pHandle, FALSE);//该函数执行必须在MPEG2_DBUpdatePIDMap()之前，确保加扰信息被正确输入
#endif
		/*将相同输入TS中，相同输入的PID，在输出的时候也修改成相同的，这样来避免多余的PID复制！！！，放置到这里修复第一次复用的时候PID并没有被修改的问题。最终是先复用再设置参数的*/
		MPEG2_DBRebuildOutTsServicePIDShareMark(pHandle->m_DBSHandle);
		/*应用PID复用参数*/
		MPEG2_DBUpdatePIDMap(pHandle->m_DBSHandle);
		/*应用插入器参数*/
		MPEG2_DBUpdatePSISIInserter(pHandle->m_DBSHandle);
		/*应用用户手工插入TS*/
		MPEG2_DBUpdateManualInserter(pHandle->m_DBSHandle);
		/*应用私有节目表系统*/
#ifdef MULT_ENABLE_ECA_AND_SERVICE_LIST
		MPEG2_DBUpdateServListInserter(pHandle->m_DBSHandle);
#endif
		/*应用TDTTOT插入参数*/
		MPEG2_DBUpdateTDTTOTParameter(pHandle->m_DBSHandle);
		/*将PID发送到HWL模块*/
		TSP_ApplyPIDMap();//先清除HWL模块内部存储的内容！然后添加新的内容
		MULTL_ApplyDirectRouteMode(pHandle);

		PAL_CriticalLeave(pHandle->m_DBSLock);
	}
	else
	{
		GLOBAL_TRACE(("Failed!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
	}
}


void MULTL_PauseMonitors(MULT_Handle *pHandle, S32 Timeout)
{
	pHandle->m_MonitorPauseTimeout = Timeout;
}


/*应用所有参数*/
void MULTL_ApplyAllParamter(MULT_Handle *pHandle)
{
	S32 i;
	MULT_ChannelNode *plNode = NULL;
	/*复用参数*/
	MULTL_ApplyRemuxParameter(s_pHandle);

	/*直通参数配置*/
	MULTL_ApplyDirectRouteMode(s_pHandle);

	/*接口参数*/
#ifdef SUPPORT_IP_O_TS_MODULE//此模块接管调制和复用控制
	HWL_TDInitiateTDModule(HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869);

	MULT_IPoTSApply();
#else

	GLOBAL_TRACE(("In CHN = %d\n", s_pHandle->m_Parameter.m_InChannelNumber));
	for(i = 0; i < s_pHandle->m_Parameter.m_InChannelNumber; i++)
	{
		plNode = &s_pHandle->m_Parameter.m_pInChannel[i];
		if(plNode->m_ChannelType == HWL_CHANNEL_TYPE_E3DS3)
		{
			MULTL_ApplyInE3DS3Parameter(s_pHandle, i); 
		}
		else if(plNode->m_ChannelType == HWL_CHANNEL_TYPE_ASI)
		{
			//没有参数可以设置！
		}

#ifdef GM8358Q
		else if(plNode->m_ChannelType == HWL_CHANNEL_TYPE_ENCODER)
		{
			MULTL_ApplyEncoderParameter(s_pHandle, i);
		}

#endif
#ifdef GN1846
		else if(plNode->m_ChannelType == HWL_CHANNEL_TYPE_ENCODER)
		{
			MULTL_ApplyLEncoderParameter(s_pHandle, i);
		}
#endif

		else if((plNode->m_ChannelType == HWL_CHANNEL_TYPE_TUNER_C) || (plNode->m_ChannelType == HWL_CHANNEL_TYPE_TUNER_S) || (plNode->m_ChannelType == HWL_CHANNEL_TYPE_TUNER_DTMB))
		{
			GLOBAL_TRACE(("Init Demod Type = %d, Number = %d\n", plNode->m_ChannelType, plNode->m_SubChannelNumber));
			HWL_TDInitiateTDModule(plNode->m_SubType);
			MULTL_ApplyTunerParameter(s_pHandle, i);
		}
		else if(plNode->m_ChannelType == HWL_CHANNEL_TYPE_IP)
		{
			MULTL_ApplyInETHParameter(s_pHandle, i);
		}
		else
		{
			GLOBAL_TRACE(("Error In Channel Type = %d!\n", plNode->m_ChannelType));
		}
	}

	GLOBAL_TRACE(("Out CHN = %d\n", s_pHandle->m_Parameter.m_OutChannelNumber));
	for(i = 0; i < s_pHandle->m_Parameter.m_OutChannelNumber; i++)
	{
		plNode = &s_pHandle->m_Parameter.m_pOutChannel[i];
		if(plNode->m_ChannelType == HWL_CHANNEL_TYPE_DVB_C_MODULATOR || plNode->m_ChannelType == HWL_CHANNEL_TYPE_DVB_S_MODULATOR || plNode->m_ChannelType == HWL_CHANNEL_TYPE_DTMB_MODULATOR)
		{
			MULTL_ApplyQAMParameter(s_pHandle, i);
		}
		else if(plNode->m_ChannelType == HWL_CHANNEL_TYPE_ASI)
		{
			MULTL_ApplyOutASIParameter(s_pHandle, i);
		}
		else if(plNode->m_ChannelType == HWL_CHANNEL_TYPE_IP || plNode->m_ChannelType == HWL_CHANNEL_TYPE_IP_LOOP || plNode->m_ChannelType == HWL_CHANNEL_TYPE_IP_DEP || plNode->m_ChannelType == HWL_CHANNEL_TYPE_IP_LOOP_DEP)
		{
#ifdef GN1846
			/* 1846 使用的数据网口可控制网口是一样的 */
#else
			MULTL_ApplyOutETHParameter(s_pHandle, i);
#endif
		}
		else
		{
			GLOBAL_TRACE(("Error Out Channel Type!\n"));
		}
	}

#endif


#ifdef GN2000
	HWL_EncoderApply();
#endif


#ifdef SUPPORT_SFN_ADAPTER
	MULT_SFNAApplyParameter();
#endif

#ifdef SUPPORT_GNS_MODULE
	MULT_GNSApplyParameter();
#endif

#ifdef SUPPORT_NTS_DPD_BOARD
	MULT_NTSDPDApplyParameter();
#endif
}

/*主程序入口*/
U32 MULT_Enter(void)
{
	U32 lReturnCode = 0;
	U32 lLastTick, lCurTick, lFilterID;
	S32 lDuration;
	MULT_MainMsg lTmpMsg;
	MPEG2_SectionArray lTmpSectionArray;

	/*初始化！平台配置模块*/
	PFC_Initiate();
	
#ifdef USE_UNISPI_DRIVER
	PFC_System("rmmod universal_spi_driver");
	PFC_System("insmod /tmp/universal_spi_driver.ko");
#endif

#ifdef GN1846
	PFC_System("mknod /dev/ttyPS1 c 204 65"); /* ttyPS1 前面板使用 */
	PL_TaskSleep(10);
#endif

	PAL_TimeStart(&lCurTick);

	/*初始化通用定时器模块*/
	PAL_TimerInitiate(16, 20);

#if defined(USE_FRP_PRE_CONSOLE)//Zynq板的UBoot没有修改成可以输出启动字符串的功能
	MULT_FRPPreConsoleIntiate();
#endif

	//测试新面板的版本获取接口！
#if 0
	{
		S32 lOKCount, lFailCount;
		lOKCount = 0;
		lFailCount = 0;
		while(TRUE)
		{
			BOOL lRetX;
			HANDLE32 lHandle;
			lHandle = FRP_DeviceCreate(1);
			lRetX = FRP_DeviceDetectFirmwareVersion(lHandle, NULL, NULL);
			FRP_DeviceDestroy(lHandle);
			if (lRetX <= 0)
			{
				lFailCount++;
			}
			else
			{
				lOKCount++;
				PFC_TaskSleep(100);
			}
			GLOBAL_TRACE(("OK = %d, Failed = %d!!!!!\n", lOKCount, lFailCount));
		}

		GLOBAL_PAUSE();
	}
#endif

#ifdef USE_CARD_FRP
	FRP_CardInitiate();
#ifdef GN1846
	FRP_CardSetInit(NULL, TRUE);
#else
	FRP_CardSetInit(TRUE);
#endif
#endif

#ifdef KERNEL_AUTO_UPDATE
	MULT_KernelAutoUpdate();
#endif

	s_pHandle = (MULT_Handle*) GLOBAL_ZMALLOC(sizeof(MULT_Handle));
	if (s_pHandle)
	{
		S32 i;
		HWL_HWInfo lHWInfo;
		GLOBAL_MEMSET(&lHWInfo,0, sizeof(HWL_HWInfo));


		GLOBAL_TRACE((" ---------------------------------- Initiate FRP ---------------------------------- \n"));

#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVInitiate(s_pHandle);
#endif



#ifdef GA2620B//2620B 串口和FPGA 配置
		/**面板各模块初始化.必须先调用.*/
		FRP_MenuInitiate(1, NULL);	
#endif

#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVShowInitateProgressString("INIT FPGA MODULE");
#endif



		GLOBAL_TRACE((" ---------------------------------- Initiate FPGA ---------------------------------- \n"));

		PFC_TaskSleep(100);

		/*配置FPGA！！！！！！！！*/
		HWL_FpgaConfig();

#ifdef GN1846 // for debug 
#if 0
		HANDLE32 lUartHandle;
		U8 plBuf[128] = "this is a test text";

		lUartHandle = PFC_ComDeviceOpenNEW("ttyAMA1", FALSE);
		if (lUartHandle) {
			PFC_ComDeviceSetState(lUartHandle, 115200, 8, 'N', 1);
			while (1) {
				//PFC_ComDeviceRead(lUartHandle, plBuf, 128);
				GLOBAL_TRACE(("read uart:%s!\n", plBuf));
				PFC_ComDeviceWrite(lUartHandle, plBuf, sizeof(plBuf));
				sleep(1);
			}
		} 
		else {
			GLOBAL_TRACE(("PFC_ComDeviceOpenNEW Error!\n"));
		}
#endif
#if 0
		HWL_RunStatusDebug();
		sleep(1);
		//HWL_HdmiRxDebug();
		HWL_FpgaSpiDebug();
#endif
#endif

#ifdef DEBUG_SPI_FLASH_INTERFACE//测试SPI接口
		{
#include "./GN1858/spi_flash.h"
			Test_FSPIF();
		}
#endif




#ifdef GN2000
		HWL_EncoderInit();
		MULTI_MOSIACInitiate();
#endif

		GLOBAL_TRACE((" ---------------------------------- Initiate HWL ---------------------------------- \n"));
#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVShowInitateProgressString("INIT HWL MODULE");
#endif
		lHWInfo.m_DeviceType = MULT_DEVICE_SIMU_TYPE;
		HWL_Initiate(lHWInfo.m_DeviceType);

		/*取得设备硬件信息，包括自动获取当前安装的Tuner的个数*/
#ifdef SUPPORT_NEW_HWL_MODULE
		HWL_MonitorHWInfoGet(&lHWInfo);
#else
		HWL_GetHWInfo(&lHWInfo);
#endif

		{
			GLOBAL_TRACE((" ---------------------------------- HWL Info ---------------------------------- \n"));
#ifdef SUPPORT_NEW_HWL_MODULE
			GLOBAL_TRACE(("Total In Num = %d\n", lHWInfo.m_InChnNum));
			for (i = 0; i < lHWInfo.m_InChnNum; i++)
			{
				GLOBAL_TRACE(("InCHN %d Type = %d, Sub = %d, MaxTs = %d\n", i, lHWInfo.m_pInChn[i].m_Type, lHWInfo.m_pInChn[i].m_SubType, lHWInfo.m_pInChn[i].m_CurSubSupport));
			}
			GLOBAL_TRACE(("Total Out Num = %d\n", lHWInfo.m_OutChnNum));
			for (i = 0; i < lHWInfo.m_OutChnNum; i++)
			{
				GLOBAL_TRACE(("InCHN %d Type = %d, Sub = %d, MaxTs = %d\n", i, lHWInfo.m_pOutChn[i].m_Type, lHWInfo.m_pOutChn[i].m_SubType, lHWInfo.m_pOutChn[i].m_CurSubSupport));
			}
			GLOBAL_TRACE(("Max In Ts = %d\n", lHWInfo.m_InTsMax));
			GLOBAL_TRACE(("Max Out Ts = %d\n", lHWInfo.m_OutTsMax));

			GLOBAL_TRACE(("Chip ID = 0x%.8X\n", lHWInfo.m_ChipID));
			GLOBAL_TRACE(("Fpga Release = %s\n", lHWInfo.m_pFPGARelease));
#else
			GLOBAL_TRACE(("Total Channel Num = %d\n", lHWInfo.m_ChannelNum));
			for (i = 0; i < lHWInfo.m_ChannelNum; i++)
			{
				GLOBAL_TRACE(("CHN %d Type = %d, Sub = %d, Direction = %d, MaxTs = %d\n", i, lHWInfo.m_pInfoList[i].m_Type, lHWInfo.m_pInfoList[i].m_SubType, lHWInfo.m_pInfoList[i].m_Direction, lHWInfo.m_pInfoList[i].m_CurSubSupport));
			}
			GLOBAL_TRACE(("Max In Ts = %d\n", lHWInfo.m_InTsMax));
			GLOBAL_TRACE(("Max Out Ts = %d\n", lHWInfo.m_OutTsMax));

			GLOBAL_TRACE(("Chip ID = 0x%.8X\n", lHWInfo.m_ChipID));
			GLOBAL_TRACE(("Fpga Release = %s\n", lHWInfo.m_pFPGARelease));
			GLOBAL_TRACE(("Tuner Count = %d\n", lHWInfo.m_TunerCount));
#endif

		}

#if 0
		/*测试内置存储区域读写接口(清空代码！！！)*/
		{
			U8 lData[10];
			S32 i;
			for (i = 0; i < 8; i++)
			{
				// 				GLOBAL_MEMSET(lData, 0xFF, 10);
				// 				HWL_EncryptChipWrite(lData, 10, i);
				// 				PFC_TaskSleep(500);
				GLOBAL_MEMSET(lData, 0, 10);
				HWL_EncryptChipRead(lData, 10, i, 2000);
				CAL_PrintDataBlock(__FUNCTION__, lData, 10);
			}

			PFC_TaskSleep(10000000000);
		}
#endif


		GLOBAL_TRACE((" ---------------------------------- Generate Info ---------------------------------- \n"));
#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVShowInitateProgressString("INIT CORE MODULE");
#endif
		/*初始化设备基本信息*/
		MULTL_GenerateBasic(s_pHandle, &lHWInfo); 
		/*初始化OEM*/
		MULTL_GenerateOEM(s_pHandle);
		/*初始化授权*/
		MULTL_GenerateAuth(s_pHandle);

#ifdef MULT_SUPPORT_FPGA_ETH
		GLOBAL_TRACE((" ---------------------------------- Initate FPGA Eth ---------------------------------- \n"));
		MULT_FPGAEthInitiate(lHWInfo.m_ChipID);
#endif


		/*下面是主模块的自模块系统的初始化，所有复用加扰信息均保存在数据库模块内部*/
		GLOBAL_TRACE((" ---------------------------------- Initiate MPEG2 DB ---------------------------------- \n"));
		{

			//数据库初始化，并设置参数！
			MPEG2_DBInitParam lDBInitParam;
			GLOBAL_ZEROMEM(&lDBInitParam, sizeof(MPEG2_DBInitParam));
			lDBInitParam.m_pRequestCB = MULTL_DBDataFilterAddCB;
			lDBInitParam.m_pCancelCB = MULTL_DBDataFilterRemoveCB;
			lDBInitParam.m_pDataArriveCB = MULTL_DBFilterDataArriveCB;
			lDBInitParam.m_pUserParam = s_pHandle; 

			lDBInitParam.m_pPIDMapCB = MULTL_DataPIDMapCB;
			lDBInitParam.m_pPIDMapUserParam = s_pHandle;

			lDBInitParam.m_pISRAddCB = MULTL_DataInserterAddCB;
			lDBInitParam.m_pISRRemoveCB = MULTL_DataInserterRemoveCB;
			lDBInitParam.m_pISRUserParam = s_pHandle;

			lDBInitParam.m_InputTsNumber = lHWInfo.m_InTsMax;
			lDBInitParam.m_OutputTsNumber = lHWInfo.m_OutTsMax;
			lDBInitParam.m_ServiceNumber = MULT_SERVICE_MAX_NUM;
#ifdef ENCODER_CARD_PLATFORM
			lDBInitParam.m_ManualTsNum = 32;
			lDBInitParam.m_ManualTsMaxBitrate = 8 * 1000 * 1000;
			lDBInitParam.m_ManualTsMaxSize = 1 * 1024 * 1024;
#else
			lDBInitParam.m_ManualTsNum = 32;
			lDBInitParam.m_ManualTsMaxBitrate = 4 * 1000 * 1000;
			lDBInitParam.m_ManualTsMaxSize = 1 * 1024 * 1024;
#endif
			lDBInitParam.m_EsNumber = lDBInitParam.m_ServiceNumber * 4;
			lDBInitParam.m_DescriptorNumber = lDBInitParam.m_EsNumber;
			lDBInitParam.m_MuxCANum = lDBInitParam.m_EsNumber * 4;
			lDBInitParam.m_SCSCANum = lDBInitParam.m_ServiceNumber + lDBInitParam.m_OutputTsNumber;
			lDBInitParam.m_AccessCriteriaNum = 256 * 4;
			lDBInitParam.m_PIDMapNum = lDBInitParam.m_ServiceNumber;
			lDBInitParam.m_NITTsNum = 128;

			s_pHandle->m_DBSHandle = MPEG2_DBCreate(&lDBInitParam);

			s_pHandle->m_DBSLock = PAL_CriticalInitiate();

			GLOBAL_TRACE(("MPEG2 DB Create Handle = 0x%08X Lock = 0x%08X !!!!!!!!!!!!!\n", s_pHandle->m_DBSHandle, s_pHandle->m_DBSLock));

#if defined(GM2730S)//SPTS模式专用
			{
				MPEG2_DBSPTSParam lSPTSParam;
				lSPTSParam.m_SPTSMark = TRUE;
				lSPTSParam.m_UseECM = FALSE;
				lSPTSParam.m_UseSDT = FALSE;
				MPEG2_DBProcSPTSMode(s_pHandle->m_DBSHandle, &lSPTSParam, FALSE);
			}
#endif


#if defined(LR1800S) || defined(GM7000)//禁止DB模块输出PSI和PIDMAP信息
			{
				/*关闭多余的TS通道的PSI输出*/
				S32 i;
				for (i = 0; i < lHWInfo.m_OutTsMax; i++)
				{
					MPEG2_DBSetTsValid(s_pHandle->m_DBSHandle, i, FALSE, FALSE);
				}
			}

#endif

#if defined(GN2000)

			{
				/*关闭多余的TS通道的PSI输出*/
				S32 i;
				for (i = 0; i < lHWInfo.m_OutTsMax; i++)
				{
					if (i != 0 && i != 16)
					{
						MPEG2_DBSetOutputTsPSIBlock(s_pHandle->m_DBSHandle, i, TRUE);
					}
				}
			}
#endif

#ifdef TRANS_PCR_PID_FUNCTION
			MPEG2_DBSetTransServiceFunction(s_pHandle->m_DBSHandle, TRUE);
#endif

#if defined(ENCODER_CARD_PLATFORM)
			{
				S32 k;
				S32 lCount;
				HWL_ChannelInfo *plTmpChnInfo;


				for (i = 0; i < lHWInfo.m_InTsMax; i++)
				{
					MPEG2_DBSetTsValid(s_pHandle->m_DBSHandle, i, FALSE, TRUE);
				}
				for (i = 0; i < lHWInfo.m_OutTsMax; i++)
				{
					MPEG2_DBSetTsValid(s_pHandle->m_DBSHandle, i, FALSE, FALSE);
				}

#ifdef GN1772
				/*仅仅使能输入输出通道*/
				for (i = 0; i < lHWInfo.m_InChnNum; i++)
				{
					plTmpChnInfo = &lHWInfo.m_pInChn[i];
					k = plTmpChnInfo->m_StartTsIndex + plTmpChnInfo->m_SubChnNum;
					for (k = 0; k < plTmpChnInfo->m_SubChnNum; k++)
					{
						MPEG2_DBSetTsValid(s_pHandle->m_DBSHandle, k + plTmpChnInfo->m_StartTsIndex, TRUE, TRUE);
					}
				}

				for (i = 0; i < lHWInfo.m_OutChnNum; i++)
				{
					plTmpChnInfo = &lHWInfo.m_pOutChn[i];
					k = plTmpChnInfo->m_StartTsIndex + plTmpChnInfo->m_SubChnNum;
					for (k = 0; k < plTmpChnInfo->m_SubChnNum; k++)
					{
						MPEG2_DBSetTsValid(s_pHandle->m_DBSHandle, k + plTmpChnInfo->m_StartTsIndex, TRUE, FALSE);
					}
				}

				/*使能子板通道每个子板输入输出各16个*/
				for (i = 0; i < 6; i++)
				{
					for (k = 0; k < 16; k++)
					{
						MPEG2_DBSetTsValid(s_pHandle->m_DBSHandle, k + 256 * (i + 2), TRUE, TRUE);
						MPEG2_DBSetTsValid(s_pHandle->m_DBSHandle, k + 256 * (i + 2), TRUE, FALSE);
					}
				}

				for (i = 0; i < 6; i++)
				{
					for (k = 0; k < 16; k++)
					{
						MPEG2_DBSetOutputTsPSIBlock(s_pHandle->m_DBSHandle, k + 256 * (i + 2), TRUE);
					}
				}
			
#endif

			}
#endif
		}

#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVShowInitateProgressString("INIT EX MODULE S1");
#endif

#ifdef MULT_TS_BACKUP_SUPPORT
		{
			MULT_BPInitiate(s_pHandle, lHWInfo.m_InTsMax, lHWInfo.m_OutTsMax);
		}
#endif

#ifdef SUPPORT_IP_O_TS_MODULE
		{
			MULT_IPoTSInitiate();
		}
#endif

#ifdef SUPPORT_GNS_MODULE
		{
			MULT_GNSInitiate();
		}
#endif

#ifdef SUPPORT_SFN_MODULATOR
		{
			MULT_SFNInitiate(s_pHandle);
		}
#endif

#ifdef SUPPORT_SFN_ADAPTER
		{
			MULT_SFNAInitiate(s_pHandle);
		}
#endif


#ifdef SUPPORT_FGPIO_MODULE
		FGPIO_Initiate();
#endif

#ifdef SUPPORT_NTS_DPD_BOARD
		MULT_NTSDPDInitiate(s_pHandle);
#endif

#ifdef SUPPORT_CLK_ADJ_MODULE
		{
			MULT_CLKInitiate(s_pHandle);
		}
#endif


		/*读取保存的参数，如果参数错误或者不存在则生成默认参数*/
		GLOBAL_TRACE((" ---------------------------------- Load Parameter ---------------------------------- \n"));
#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVShowInitateProgressString("LOAD PARAMETERS");
#endif
		{
			CHAR_T plCMD[1024];
			GLOBAL_SPRINTF((plCMD, "%s%s", MULT_STORAGE_BASE_DIR, MULT_PARAMETER_FILE_PATHNAME));
			/*展开FLASH的参数到TMPFS*/
			MULTL_LoadParamterFromStorage(s_pHandle, plCMD);
			/*维护参数*/
			MULTL_LoadMaintenaceXML(s_pHandle, &lHWInfo);
			/*设备设置参数XML(device_parameter.xml)*/
			MULTL_LoadConfigurationXML(s_pHandle, &lHWInfo);
			/*参数XML(system_parameter.xml)*/
			MULTL_LoadParameterXML(s_pHandle, &lHWInfo);//该函数会操作数据库模块以保存参数
			/*监控参数XML(monitor_setting.xml)*/
			MULTL_LoadMonitorXML(s_pHandle, &lHWInfo);
#ifdef GN1846
			GLOBAL_SPRINTF((plCMD, "cp -f %s%s %s", MULT_STORAGE_BASE_DIR, MULT_EDID_SETTINGS_XML, MULT_XML_BASE_DIR));
			PFC_System(plCMD);
			MULTL_LoadEdidXML(s_pHandle, &lHWInfo);
#endif
			/*生成信息XML*/
			MULTL_GenerateInfoXML(s_pHandle);
		}


		/*需要先关闭SYSLOG*/
#ifdef SUPPORT_SYSLOG_MODULE
		GLOBAL_TRACE((" ---------------------------------- Initiate SYSLOG ---------------------------------- \n"));
		MULT_SyslogApply(s_pHandle, FALSE);
#endif

		GLOBAL_TRACE((" ---------------------------------- Initiate TSP ---------------------------------- \n"));
		/*初始化流处理模块*/
		TSP_Initiate();
		TSP_SetCurrentWorkTsIndex(0);//设置默认工作输入TS序号

#ifndef DISABLE_SCS_MODULE
		GLOBAL_TRACE((" ---------------------------------- Initiate SCS ---------------------------------- \n"));
		/*SCS模块初始化！*/
		MULTL_SCSManagerInitiate(s_pHandle, &lHWInfo);
#endif


		GLOBAL_TRACE((" ---------------------------------- Apply Paramater ---------------------------------- \n"));

#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVShowInitateProgressString("APPLY PARAMETERS");
#endif

#ifdef SUPPORT_CLK_ADJ_MODULE
		MULT_CLKSetDefaultDAValue(s_pHandle->m_MaintSetting.m_10MPLLDAValue);
		MULT_CLKSetTranningValue(s_pHandle->m_MaintSetting.m_10MTrainningValue);
#endif
		MULTL_ApplyAllParamter(s_pHandle);

		MULTL_SaveParameterXML(s_pHandle); //参数保存在应用之后

		/*初始化子系统！~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifdef ENCODER_CARD_PLATFORM
		{

#ifdef GN1772
			/*不使用的直通命令必须提前关闭*/
			{
				S32 i, k;
				for (i = 2; i < 8; i++)
				{
					for (k = 0; k < 250; k++)
					{
						HWL_DVBMuxRouteSet(k, 0, FALSE, FALSE, FALSE);
					}
					HWL_DVBMuxRouteApply(i, 250);
				}
			}
#endif


			GLOBAL_TRACE((" ---------------------------------- Initiate Sub Module System ---------------------------------- \n"));
			/*初始化子板控制系统*/
			MULT_CARDModuleInitiate(s_pHandle, FALSE);
		}
#endif


		/*全系统初始化完成！~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

		/*清除参数相关标志位*/
		MULTL_SetRemuxApplyMark(s_pHandle, TRUE);
		MULTL_SetSaveMark(s_pHandle, TRUE);


		GLOBAL_TRACE((" ---------------------------------- Initiate WEB ---------------------------------- \n"));
		/*打开Web服务！*/
#if 1

		{
			WEB_InitParam lWebInitParam;
			GLOBAL_ZEROMEM(&lWebInitParam, sizeof(WEB_InitParam));
			lWebInitParam.m_pUserParam = s_pHandle;
			lWebInitParam.m_WEBDataCB = MULTL_WEBDataCB;
			lWebInitParam.m_PortNum = 80;
			lWebInitParam.m_SocketTimeOut = 1000;
			GLOBAL_STRCPY(lWebInitParam.m_pDefaultHomePage, MULT_DEFAULT_HOME_PAGE);
			GLOBAL_STRCPY(lWebInitParam.m_pDefaultWebDir, MULT_WEB_BASE_DIR);
			GLOBAL_SPRINTF((lWebInitParam.m_pDefaultUserFileName, "%s%s", MULT_STORAGE_BASE_DIR, MULT_WEB_USER_MANAGE_FILE_PATHNAME));
			GLOBAL_STRCPY(lWebInitParam.m_pRealmName, s_pHandle->m_Information.m_pModelName);
			lWebInitParam.m_NameLimit = 16;
			lWebInitParam.m_PasswordLimit = 16;
			lWebInitParam.m_UserLimit = 8;
			WEB_InitiateServer(&lWebInitParam);
			WEB_StartServer();
		}
#endif

		HWL_FanEnable(TRUE);

		GLOBAL_TRACE((" ---------------------------------- Reset Manage Port ---------------------------------- \n"));
		/*设置网管IP地址*/
#ifndef DEBUG_MODE_FPGA_CONFIG_ONCE
		MULTL_ManagePortConfig(s_pHandle);
#endif

#if defined(MULT_SUPPORT_FPGA_ETH)
		GLOBAL_TRACE((" ---------------------------------- FPGA ETH Port ---------------------------------- \n"));
		/*设置FPGA网管IP地址*/
		MULT_FPGAEthApply(TRUE);
#ifdef GQ3760A
		MULTL_ApplyInETHParameter(s_pHandle, 1);	

#endif
#endif

		/*启动前面板*/
		GLOBAL_TRACE((" ---------------------------------- Start FRP ---------------------------------- \n"));
#ifndef DEBUG_DISABLE_FRP
#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVStart(s_pHandle);
#else
#ifdef USE_CARD_FRP
#ifdef GN1846
		/* 由于前面板不能单独设置自己的语言属性，前面板语言同 web 语言是一样的 */
		s_pHandle->m_Configuration.m_FrpLanguage = s_pHandle->m_Configuration.m_WebLanguage; 
		FRP_CardDisplayInfoUpdate(s_pHandle);
#endif
#else
		FRP_AgentInitiate(s_pHandle);
#endif
#endif
#endif

		/*启动SNMP*/
		GLOBAL_TRACE((" ---------------------------------- Start SNMP ---------------------------------- \n"));
		if (s_pHandle->m_Configuration.m_SNMPGlobalMark)
		{
			MULT_SNMPTerminate(s_pHandle);
			MULT_SNMPInitiate(s_pHandle);
		}

#ifdef SUPPORT_REMOTE_COM_CONTROL
		MULT_RemoteInitiate(s_pHandle);
#endif

#ifdef SUPPORT_PES_ANALYS_FUNCTION
		PES_Initiate(s_pHandle->m_DBSHandle);
#endif

		GLOBAL_TRACE((" ---------------------------------- Initiate Complete!! Time = %f s------------------------------------\n", ((F32)PAL_TimeEnd(&lCurTick)) / 1000));
		GLOBAL_SYSLOG((PL_SYSLOG_PRIORITY_NOTICE, "Initiate Complete!!\n"));

#ifdef USE_CARD_FRP
#ifdef GN1846
		FRP_CardSetInit(s_pHandle, FALSE);
#else
		FRP_CardSetInit(FALSE);
#endif
#endif

		MULTL_PauseMonitors(s_pHandle, 4000);//启动8秒内状态不正常，暂时不获取状态！
		/*打开消息循环*/
		s_pHandle->m_Monitor.m_CurDuration = 0;
		lLastTick = PFC_GetTickCount();
		s_pHandle->m_MainMsgHandle = PAL_MsgQueueCreate(128, sizeof(MULT_MainMsg));
		/*打开监控更新定时器*/
		s_pHandle->m_MonitoerTimerID = PAL_TimerAdd(2000, TRUE, MULTL_MonitorTimerCB, s_pHandle, 0);
		s_pHandle->m_TaskMark = MULT_TASK_MARK_RUN;
		while(s_pHandle->m_TaskMark == MULT_TASK_MARK_RUN)
		{
			if (PAL_MsgQueueRecv(s_pHandle->m_MainMsgHandle, &lTmpMsg, sizeof(lTmpMsg), MULT_MAIN_MSG_QUEUE_MAX_TIMEOUT))
			{
				switch (lTmpMsg.m_MsgType)
				{
				case MULTI_MAIN_MSG_TYPE_PSI_ARRIVE:
					{
						lFilterID = MPEG2_DBAnalyzeGetFilterIDByCallerID(s_pHandle->m_DBSHandle, lTmpMsg.m_MsgwValue);
						if (lFilterID != 0)
						{
							if (TSP_PSIBufferGetArrayPtr(lFilterID, &lTmpSectionArray))
							{
								/*Section输出传输给DVB DB模块进行解析！*/
								MPEG2_DBAnalyzeSetSectionArray(s_pHandle->m_DBSHandle, lTmpMsg.m_MsgwValue, &lTmpSectionArray);
							}
						}
					}
					break;
				case MULTI_MAIN_MSG_TYPE_INIPUT_ANALYZE_COMPLETE:
					{
						MULT_AnalyzeDone(s_pHandle);
					}
					break;
				case MULTI_MAIN_MSG_TYPE_WEB_XML_CMD:
					{
						/*WEB服务器来的XML设置相关命令(POST命令)*/
						if (lTmpMsg.m_MsglValue > 0)
						{
							MULTL_WEBXMLPostProcess(s_pHandle, (CHAR_T*)lTmpMsg.m_MsgwValue, lTmpMsg.m_MsglValue);
							GLOBAL_FREE((CHAR_T*)lTmpMsg.m_MsgwValue);
						}
					}
					break;
				case MULTI_MAIN_MSG_TYPE_MONITOR_TIMER:
					{
						/*处理监控信息变化*/
						lCurTick = PFC_GetTickCount();
						lDuration = lCurTick - lLastTick;
						lLastTick = lCurTick;
						if (lDuration <= 0)
						{
							lDuration = 1;
						}

						s_pHandle->m_Monitor.m_CurDuration += (F64)lDuration / 1000;

						/*DB模块的TDTTOT更新驱动*/
						if (PAL_CriticalEnter(s_pHandle->m_DBSLock, 500))
						{
							MPEG2_DBAccess(s_pHandle->m_DBSHandle, lDuration);
							PAL_CriticalLeave(s_pHandle->m_DBSLock);
						}
						else
						{
							GLOBAL_TRACE(("Lock DBSLock Failed Possiable Apply Parameter On Going\n"));
						}

						MULTL_MonitorProcess(s_pHandle, lDuration);

#ifdef SUPPORT_IP_O_TS_MODULE
						MULT_IPoTSMonitorProcess(lDuration);
#endif

#ifdef SUPPORT_SFN_MODULATOR
						MULT_SFNMonitorProcess(lDuration);
#endif

#ifdef SUPPORT_SFN_ADAPTER
						MULT_SFNAMonitorProcess(lDuration);
#endif

#ifdef SUPPORT_GNS_MODULE
						MULT_GNSMonitorProcess(lDuration);
#endif

#ifndef DEBUG_DISABLE_FRP
#ifdef SUPPORT_NEW_FRP_MENU_MODULE
#else
#ifdef USE_CARD_FRP
						FRP_CardAccess(s_pHandle, lDuration);
#else
						FRP_AgentAccess(s_pHandle, lDuration);
#endif
#endif
#endif
						HWL_Access(lDuration);
						TSP_Access(lDuration);

						MULT_SNMPAccess(s_pHandle, lDuration);

						/*授权的驱动与控制*/
						if (s_pHandle->m_Information.m_LicenseValid == FALSE)
						{
							S32 lLastTime = 0;
							AUTH_Access(s_pHandle->m_AUTHHandle, lDuration);
							lLastTime = AUTH_GetTrailWorkTime(s_pHandle->m_AUTHHandle);
							if ((lLastTime <= 0) && (s_pHandle->m_Information.m_TrailTime > 0))
							{
								/*重启*/
								MULTL_RebootSequence(s_pHandle);
							}
							else
							{
								if (s_pHandle->m_Information.m_TrailTime != lLastTime)
								{
									GLOBAL_TRACE(("Last Trail Time = %d s\n", lLastTime));
									s_pHandle->m_Information.m_TrailTime = lLastTime;
									MULTL_GenerateInfoXML(s_pHandle);
								}

							}
						}
					}
					break;
				default:
					{
						GLOBAL_TRACE(("Unknown Msg Type = %d\n", lTmpMsg.m_MsgType));
					}
					break;
				}
			}
			else
			{
				// 				MULT_MainMsg lTmpMsg;
				// 
				// 				lTmpMsg.m_MsgType = MULTI_MAIN_MSG_TYPE_MONITOR_TIMER;
				// 				lTmpMsg.m_MsgParam = 1;
				// 				lTmpMsg.m_MsglValue = 0;
				// 				lTmpMsg.m_MsgwValue = 0;
				// 				PAL_MsgQueueSend(s_pHandle->m_MainMsgHandle, &lTmpMsg, sizeof(lTmpMsg));

				GLOBAL_TRACE(("MSG Queue Time Out\n"));
			}
		}

		s_pHandle->m_TaskMark = MULT_TASK_MARK_STOP;

		lReturnCode = s_pHandle->m_RetCode;

#ifdef SUPPORT_PES_ANALYS_FUNCTION
		PES_Terminate();
#endif

#ifdef ENCODER_CARD_PLATFORM
		{
			/*初始化子板控制系统*/
			MULT_CARDModuleTerminate();
		}
#endif

#ifdef SUPPORT_REMOTE_COM_CONTROL
		MULT_RemoteTerminate();
#endif

#ifndef DEBUG_DISABLE_FRP
#ifdef SUPPORT_NEW_FRP_MENU_MODULE
		MULT_FRPMenuADVDestroy();
#else
#ifdef USE_CARD_FRP
		FRP_CardTerminate();
#else
		FRP_AgentTerminate();
#endif
#endif
#endif


#ifdef SUPPORT_FGPIO_MODULE
		FGPIO_Terminate();
#endif


#ifdef SUPPORT_CLK_ADJ_MODULE
		{
			MULT_CLKTerminate();
		}
#endif


#ifdef SUPPORT_SFN_MODULATOR
		{
			MULT_SFNTerminate();
		}
#endif

#ifdef SUPPORT_SFN_ADAPTER
		{
			MULT_SFNATerminate();
		}
#endif

#ifdef SUPPORT_GNS_MODULE
		{
			MULT_GNSTerminate();
		}
#endif

#ifdef SUPPORT_IP_O_TS_MODULE
		{
			MULT_IPoTSTerminate();
		}
#endif

#ifdef GN2000
		HWL_EncoderTerm();
		MULTI_MOSIACTerminate();
#endif

#ifdef MULT_TS_BACKUP_SUPPORT
		MULT_BPTerminate();
#endif

		/*关闭SNMP服务*/
		MULT_SNMPTerminate(s_pHandle);

#ifdef SUPPORT_SYSLOG_MODULE
		/*关闭SYSLOG*/
		MULT_SyslogApply(s_pHandle, TRUE);
#endif

		/*关闭WEB服务*/
		WEB_StopServer();

		/*销毁WEBServer*/
		WEB_TerminateServer();

		/*关闭SCS线程*/
		MULTL_SCSManagerApply(s_pHandle, TRUE);

#ifndef DISABLE_SCS_MODULE
		/*销毁SCS模块*/
		MULTL_SCSManagerDestroy(s_pHandle);
#endif		

		/*销毁授权模块*/
		AUTH_Destroy(s_pHandle->m_AUTHHandle);

		/*重新锁定前面板，发送重启动信息*/


#ifdef MULT_SUPPORT_FPGA_ETH
		/*关闭FPGA ETH*/
		MULT_FPGAEthTerminate();
#endif

		/*关闭定时器*/
		if (s_pHandle->m_MonitoerTimerID)
		{
			PAL_TimerRemove(s_pHandle->m_MonitoerTimerID);
			s_pHandle->m_MonitoerTimerID = 0;
		}

		/*关闭消息处理*/
		PAL_MsgQueueDestroy(s_pHandle->m_MainMsgHandle);
		s_pHandle->m_MainMsgHandle = NULL;


		/*关闭数据库*/
		MPEG2_DBDestroy(s_pHandle->m_DBSHandle);
		s_pHandle->m_DBSHandle = NULL;

		PAL_CriticalTerminate(s_pHandle->m_DBSLock);
		s_pHandle->m_DBSLock = NULL;

		/*释放参数内存*/
		MULTL_ReleaseParameter(s_pHandle);
		MULTL_ReleaseMonitor(s_pHandle);

		/*关闭流处理模块*/
		TSP_Terminate();

		/*关闭硬件抽象层*/
		HWL_Terminate();

		/*释放主模块资源*/
		GLOBAL_FREE(s_pHandle);
		s_pHandle = NULL;


		/*销毁前面板*/
	}

	/*关闭平台抽象辅助层*/
	PAL_TimerTerminate();

	//销毁！平台配置模块
	PFC_Terminate();

	return lReturnCode;
}


void MULT_ForceTerminate(void)
{
	if (s_pHandle)
	{
		s_pHandle->m_TaskMark = MULT_TASK_MARK_WAIT_STOP;
	}
}


BOOL MULT_IsRunning(void)
{
	if (s_pHandle)
	{
		if (s_pHandle->m_TaskMark != MULT_TASK_MARK_STOP)
		{
			return TRUE;
		}
	}
	return FALSE;
}




void MULT_AnalyzeStart(MULT_Handle *pHandle, S16 DevIndex, S16 TsIndex, S16 TimeOutForPSIMs, S32 AnalyzeCharset, BOOL bTimeoutRemoveService, S32 AutoMapMode)
{

#ifdef SUPPORT_SFN_MODULATOR
	MULT_SFNPauseSIPRecv();
#endif

	TSP_SetCurrentWorkTsIndex(TsIndex);
#if MULT_SYSTEM_ENABLE_AUTO_MAP_FUNCTION
	MPEG2_DBExSetAnalyzeInputExParameter(pHandle->m_DBSHandle, AutoMapMode);//测试自动复用功能
#else
	MPEG2_DBExSetAnalyzeInputExParameter(pHandle->m_DBSHandle, MPEG2_DB_EX_AUTO_MAP_NONE);
#endif
	MPEG2_DBAnalyzeStart(pHandle->m_DBSHandle, TsIndex, TimeOutForPSIMs, AnalyzeCharset, bTimeoutRemoveService, MULTL_DBActionCB, pHandle);

}
void MULT_AnalyzeDone(MULT_Handle *pHandle)
{	

	if (pHandle->m_bBatchAnalyzeActive)
	{
		S32 i;

		for (i = pHandle->m_BatchCurTsIndex + 1; i < pHandle->m_BatchMaxTsNumber; i++)
		{
			if (MULTL_GetTsMark(pHandle,i, TRUE))
			{
				pHandle->m_BatchCurTsIndex = i;
				MULT_AnalyzeStart(pHandle, 0, pHandle->m_BatchCurTsIndex, pHandle->m_BatchTimeout, pHandle->m_BatchCharset, pHandle->m_BatchRemoveService, MPEG2_DB_EX_AUTO_MAP_NONE);
				break;
			}
		}

		if (i == pHandle->m_BatchMaxTsNumber)
		{
			pHandle->m_bBatchAnalyzeActive = FALSE;
			MULT_AnalyzeDone(pHandle);
		}
	}
	else
	{
		pHandle->m_bBatchAnalyzeActive = FALSE;
#if MULT_SYSTEM_ENABLE_AUTO_MAP_FUNCTION
		MULTL_ApplyRemuxParameter(pHandle);
#endif

		MULTL_SaveParameterXML(pHandle);

#ifdef SUPPORT_SFN_MODULATOR
		MULT_SFNResumeSIPRecv();
#endif

	}
}



void MULT_AnalyzeBatchStart(MULT_Handle *pHandle, S16 DevIndex, S16 TimeOutForPSIMs, S32 AnalyzeCharset, BOOL bTimeoutRemoveService)
{
	pHandle->m_BatchCurTsIndex = -1;
	pHandle->m_BatchMaxTsNumber = pHandle->m_Parameter.m_MaxInTsNumber ;
	pHandle->m_BatchCharset = AnalyzeCharset;
	pHandle->m_BatchRemoveService = bTimeoutRemoveService;
	pHandle->m_BatchTimeout = TimeOutForPSIMs;	
	pHandle->m_bBatchAnalyzeActive = TRUE;
	MULT_AnalyzeDone(pHandle);
}


void MULT_SaveCLKADJModuleParameters(MULT_Handle *pHandle)
{
#ifdef SUPPORT_CLK_ADJ_MODULE
	{
		U32 lNewDAVAlue;
		F64 lNewTranningValue;

		lNewDAVAlue = MULT_CLKGetDefaultDAValue();
		lNewTranningValue = MULT_CLKGetTranningValue();
		GLOBAL_TRACE(("New DA Value To Save = 0x%.4X or New TranningValue To Save %f!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", lNewDAVAlue, lNewTranningValue));
		pHandle->m_MaintSetting.m_10MPLLDAValue = lNewDAVAlue;
		pHandle->m_MaintSetting.m_10MTrainningValue = lNewTranningValue;
		MULTL_SaveMaintenaceXML(pHandle);
	}
#endif
}

void MULT_ResetCLKADJModuleParameters(MULT_Handle *pHandle)
{
	pHandle->m_MaintSetting.m_10MPLLDAValue = 0;
	pHandle->m_MaintSetting.m_10MTrainningValue = 0;
	MULTL_SaveMaintenaceXML(pHandle);
}

/*EOF*/
