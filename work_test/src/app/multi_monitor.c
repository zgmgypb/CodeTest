/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#ifdef GN1846
#include "multi_hwl_local_encoder.h"
#include "multi_drv.h"
#endif 
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
/*监控入口*/
void MULTL_MonitorProcess(MULT_Handle *pHandle, S32 Duration)
{
	S32 i, k, lTsInd;
	// 	S32 lLogCount, k, lNewCount;
	TIME_T lTimeT;
	STRUCT_TM lTM;
	MULT_Monitor *plMonitor;
	SCS_RunningStatus lRunningStatus;
	MULT_SCS *plMultSCS;
	MULT_MonitorCHN *plMonitorChn;
	MULT_MonitorSUB *plMonitorSub;
	MULT_Parameter *plParameter;
	MULT_Config *plConfig;

	plMonitor = &pHandle->m_Monitor;
	plParameter = &pHandle->m_Parameter;
	plMultSCS = &pHandle->m_MultSCS;
	plConfig = &pHandle->m_Configuration;

	if (CAL_TimeoutCheck(&pHandle->m_MonitorPauseTimeout, Duration))
	{
		/*设备时间*/
		GLOBAL_TIME(&lTimeT);
		GLOBAL_GMTTIME_R(&lTM, &lTimeT);
		CAL_StringDateToStr(plMonitor->m_pTimeDataBuf, &lTM);
		// GLOBAL_TRACE(("Current Time = %s / 0x%.8X \n", plMonitor->m_pTimeDataBuf, lTimeT));

		/*温度*/
#ifdef MULT_DEVICE_NO_TEMP_SENSOR
		plMonitor->m_CurrentTemp == GLOBAL_U8_MAX;
#else
		plMonitor->m_CurrentTemp = HWL_Temperature();
		if (plMonitor->m_CurrentTemp <= 0)
		{
			plMonitor->m_CurrentTemp = 15;
		}

		if (plMonitor->m_CurrentTemp >= plMonitor->m_CriticalTemp)
		{
			GLOBAL_TRACE(("Temp = %d\n", plMonitor->m_CurrentTemp));
			if (plMonitor->m_GlobalMark)
			{
				if (plMonitor->m_TempErrorCount > MULTL_TEMP_ERROR_TORLARANCE)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, 0, (lTimeT & GLOBAL_U32_MAX), FALSE);
				}
				else
				{
					plMonitor->m_TempErrorCount++;
				}
			}
		}
		else
		{
			plMonitor->m_TempErrorCount = 0;
		}

#ifdef GN1846
		if (!HWL_FPGAGetMuxOK() || !DRL_GetFspiComIsOk()) { /* FPGA 下载失败或通信错误报硬件错误 */
			if (plMonitor->m_GlobalMark)
			{
				CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
			}
		}

#endif
#endif

#ifdef MULT_DEVICE_USE_QAM_MODULE

#ifdef SUPPORT_NTS_DPD_BOARD
		if (plMonitor->m_GlobalMark)
		{
			if (MULT_NTSDPDGetPLLError() > 0)
			{
				plMonitor->m_PLLErrorCount++;

				if (plMonitor->m_PLLErrorCount > MULTL_PLL_ERROR_TORLARANCE)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_PLL, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					plMonitor->m_PLLErrorCount = 0;
				}
				else
				{
					plMonitor->m_PLLErrorCount++;
				}
			}
		}
#else
		/*PLL*/
		plMonitor->m_CurrentPLLValue = HWL_PLLValue();
#if (!defined(GM8358Q)) && (!defined(GQ3710B))
		if (abs(plMonitor->m_CurrentPLLValue - MULTL_PLL_STAND_VALUE) > MULTL_PLL_OFFSET_LIMIT)
		{
			if (plMonitor->m_GlobalMark)
			{
				if (plMonitor->m_PLLErrorCount > MULTL_PLL_ERROR_TORLARANCE)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_PLL, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					plMonitor->m_PLLErrorCount = 0;
				}
				else
				{
					plMonitor->m_PLLErrorCount++;
				}
			}
		}
		else
		{
			plMonitor->m_PLLErrorCount = 0;
		}
#endif

#endif

		/*QAM缓冲区状态*/

		for (i = 0; i < MULT_DEVICE_MODUATION_NUM; i++)
		{
			if ((HWL_GetQAMBuffStatus(i) > 0))
			{
				GLOBAL_TRACE(("QAM %d Overflow\n", i));
				if (plMonitor->m_GlobalMark &&(plMonitor->m_FlowCount[i]>4))
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_BUFFER_STATUS, i, (lTimeT&GLOBAL_U32_MAX), FALSE);
					plMonitor->m_FlowCount[i] = 0;
				}
				plMonitor->m_FlowCount[i]++;
			}
			else
			{
				plMonitor->m_FlowCount[i] = 0;
			}
		}

#endif


		/* Tuner 短路 */
		if(pHandle->m_bHaveShortTest == HWL_CHANNEL_TYPE_TUNER_S)
		{
			//for (i = 0; i < HWL_TUNER_MAX_NUM; i++)
			for (i = 0; i < pHandle->m_TunerCount; i++)
			{
				if((HWL_GetTunerShortStatus(i) > 0) &&(GS_TUNER_PLOAR_NONE != pHandle->m_TunerSPolar[i]))
				{				

					if((plMonitor->m_GlobalMark) && (plMonitor->m_ShortCount[i] > 3)) 
					{
						GLOBAL_TRACE(("Tuner %d Short\n", i));
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TUNER_SHORT_STATUS, i, (lTimeT&GLOBAL_U32_MAX), FALSE);
						plMonitor->m_ShortCount[i] = 0;
					}
					else
					{
						plMonitor->m_ShortCount[i] = plMonitor->m_ShortCount[i] + 1; 

					}
				}
				else
				{
					plMonitor->m_ShortCount[i] = 0;

				}
			}

		}



		/*NTP*/
		if (plConfig->m_NTPSyncMark)
		{
			if (plMonitor->m_NTPWorkingStatus == FALSE)
			{
				if (plMonitor->m_NTPSyncTimeout <= 0)
				{
					plMonitor->m_NTPWorkingStatus = TRUE;
					PFC_TaskCreate("SNTP Task", 40 * 1024, MULTL_SNTPRequestThread, 1, pHandle);
				}
				else
				{
					plMonitor->m_NTPSyncTimeout -= Duration;
				}
			}
			else
			{
				/*操作进行中*/
			}
		}
		else
		{
			plMonitor->m_NTPSyncTimeout = 0;
		}

#if defined(LR1800S)
		/*
		*从FPGA的角度来看
		--input : 0 : IP in to ARM   RJ45进入访问ARM的数据
		--       1 : IP in to Tuner  RJ45进入调制输出的数据
		--			 2 : ARM in out to IP and tuner 访问ARM后ARM应答的数据，即ARM发送给FPGA的IP数据
		--        3 : tuner in to arm
		--        4 : tuner in to IP
		--        8..11 : tuner TS in
		--output  32 : out to ARM
		--        33 :  out to tuner 
		--        34
		--        35
		--        36 : out to IP
		--        
		*/
		plMonitor->m_TotalInBitrate = HWL_GetBitrate(0, 0) + HWL_GetBitrate(0, 1);
		plMonitor->m_TotalOutBitrate = HWL_GetBitrate(0, 36);
		plMonitor->m_InserterBitrate = HWL_GetBitrate(0, 32) + HWL_GetBitrate(0, 2);

		plMonitorChn = &plMonitor->m_pInChnArray[0];
		for (k = 0; k < plMonitorChn->m_SubNumber; k++)
		{
			plMonitorSub = &plMonitorChn->m_pSubArray[k];
			plMonitorSub->m_CurValue = HWL_GetBitrate(0, 8 + k);
		}

		plMonitorChn = &plMonitor->m_pOutChnArray[0];
		for (k = 0; k < plMonitorChn->m_SubNumber; k++)
		{
			plMonitorSub = &plMonitorChn->m_pSubArray[k];
			plMonitorSub->m_CurValue = HWL_GetBitrate(0, 33);
		}
#elif defined(GM7000)

		plMonitor->m_TotalInBitrate = HWL_GetBitrate(0, 0) + HWL_GetBitrate(0, 1);
		plMonitor->m_TotalOutBitrate = HWL_GetBitrate(0, 36);
		plMonitor->m_InserterBitrate = HWL_GetBitrate(0, 32) + HWL_GetBitrate(0, 2);

#elif defined(SUPPORT_NEW_HWL_MODULE)
		{
#ifdef ENCODER_CARD_PLATFORM//主模块给出的是整个模块接收到的码率，包括送给子板和从子板获取的，这里则希望只显示设备输入输出的码率，这个未来还需要添加子板输入输出接口的码率
			U32 lTotalIn, lTotalOut;
			lTotalIn = lTotalOut = 0;
#endif
			plMonitor->m_TotalInBitrate = HWL_MonitorChnBitrateInfoGet(GLOBAL_INVALID_INDEX, GLOBAL_INVALID_INDEX, TRUE);
			plMonitor->m_TotalOutBitrate = HWL_MonitorChnBitrateInfoGet(GLOBAL_INVALID_INDEX, GLOBAL_INVALID_INDEX, FALSE);
			plMonitor->m_InserterBitrate = HWL_MonitorInserterBitrateGet();


			/*TS码率*/
			for (i = 0; i < plMonitor->m_InChnNum; i++)
			{
				plMonitorChn = &plMonitor->m_pInChnArray[i];
				for (k = 0; k < plMonitorChn->m_SubNumber; k++)
				{
					plMonitorSub = &plMonitorChn->m_pSubArray[k];
					plMonitorSub->m_CurValue = HWL_MonitorChnBitrateInfoGet(i, k, TRUE);
#ifdef ENCODER_CARD_PLATFORM
					lTotalIn += plMonitorSub->m_CurValue;
#endif
					if (plParameter->m_pInChannel[i].m_pSubChannelNode[k].m_ActiveMark != FALSE)
					{
						if (plMonitor->m_GlobalMark)
						{
							if (plMonitorSub->m_LimitInfo.m_Mark != FALSE)
							{
								if ((plMonitorSub->m_CurValue > plMonitorSub->m_LimitInfo.m_HighLimit) || (plMonitorSub->m_CurValue < plMonitorSub->m_LimitInfo.m_LowLimit))
								{
									if (plMonitorSub->m_LimitInfo.m_AlarmCount  > 1)
									{
										CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, k + 1, (lTimeT&GLOBAL_U32_MAX), FALSE);
										plMonitorSub->m_LimitInfo.m_AlarmCount = 0;
									}
									else
									{
										plMonitorSub->m_LimitInfo.m_AlarmCount ++;
									}
								}
								else
								{
									plMonitorSub->m_LimitInfo.m_AlarmCount = 0;
								}
							}
						}
					}
				}
				plMonitorChn->m_CurValue = HWL_MonitorChnBitrateInfoGet(i, GLOBAL_INVALID_INDEX, TRUE);;
			}	

			for (i = 0; i < plMonitor->m_OutChnNum; i++)
			{
				plMonitorChn = &plMonitor->m_pOutChnArray[i];
				for (k = 0; k < plMonitorChn->m_SubNumber; k++)
				{
					plMonitorSub = &plMonitorChn->m_pSubArray[k];
					plMonitorSub->m_CurValue = HWL_MonitorChnBitrateInfoGet(i, k, FALSE);
#ifdef ENCODER_CARD_PLATFORM
					lTotalOut += plMonitorSub->m_CurValue;
#endif
					if (plParameter->m_pOutChannel[i].m_pSubChannelNode[k].m_ActiveMark != FALSE)
					{
						if (plMonitor->m_GlobalMark)
						{
							if (plMonitorSub->m_LimitInfo.m_Mark != FALSE)
							{
								if ((plMonitorSub->m_CurValue > plMonitorSub->m_LimitInfo.m_HighLimit) || (plMonitorSub->m_CurValue < plMonitorSub->m_LimitInfo.m_LowLimit))
								{
									CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, k + 1, (lTimeT&GLOBAL_U32_MAX), FALSE);
								}
							}
						}
					}
				}
				plMonitorChn->m_CurValue = HWL_MonitorChnBitrateInfoGet(i, GLOBAL_INVALID_INDEX, FALSE);;
			}

#ifdef ENCODER_CARD_PLATFORM
			plMonitor->m_TotalInBitrate = lTotalIn;
			plMonitor->m_TotalOutBitrate = lTotalOut;
#endif
		}




#else
		/*总码率*/
		{
			U32 lTmpChnTotal;

#ifdef GN1846
			plMonitor->m_TotalInBitrate = HWL_GetBitrate(pHandle, 0, -1);
			plMonitor->m_TotalOutBitrate = HWL_GetBitrate(pHandle, 1, -1);
			plMonitor->m_InserterBitrate = HWL_GetBitrate(pHandle, -1, -1);
#else
			plMonitor->m_TotalInBitrate = HWL_GetBitrate(0, -1);
			plMonitor->m_TotalOutBitrate = HWL_GetBitrate(1, -1);
			plMonitor->m_InserterBitrate = HWL_GetBitrate(-1, -1);
#endif

			/*TS码率*/
			for (i = 0; i < plMonitor->m_InChnNum; i++)
			{
				lTmpChnTotal = 0;
				plMonitorChn = &plMonitor->m_pInChnArray[i];
				for (k = 0; k < plMonitorChn->m_SubNumber; k++)
				{
					plMonitorSub = &plMonitorChn->m_pSubArray[k];
					if (plParameter->m_pInChannel[i].m_pSubChannelNode[k].m_ActiveMark != FALSE)
					{
						lTsInd = k + plMonitorChn->m_StartTsIndex;
#ifdef GN1846
						plMonitorSub->m_CurValue = HWL_GetBitrate(pHandle, 0, lTsInd);
#else
						plMonitorSub->m_CurValue = HWL_GetBitrate(0, lTsInd);
#endif
#ifdef MULT_TS_BACKUP_SUPPORT
						MULT_BPSetInputTsBitrate(lTsInd, plMonitorSub->m_CurValue);
#endif
						lTmpChnTotal += plMonitorSub->m_CurValue;
						if (plMonitor->m_GlobalMark)
						{
							if (plMonitorSub->m_LimitInfo.m_Mark != FALSE)
							{
								if ((plMonitorSub->m_CurValue > plMonitorSub->m_LimitInfo.m_HighLimit) || (plMonitorSub->m_CurValue < plMonitorSub->m_LimitInfo.m_LowLimit))
								{
									if (plMonitorSub->m_LimitInfo.m_AlarmCount  > 1)
									{
#ifdef GN1846
										if (pHandle->m_Configuration.m_IpOutputType == IP_OUTPUT_SPTS) { /* SPTS 下情况同输入 */
											CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, i, (lTimeT&GLOBAL_U32_MAX), FALSE);
										} 
#endif
										CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, i, (lTimeT&GLOBAL_U32_MAX), FALSE);
										plMonitorSub->m_LimitInfo.m_AlarmCount = 0;
									}
									else
									{
										plMonitorSub->m_LimitInfo.m_AlarmCount ++;
									}
								}
								else
								{
									plMonitorSub->m_LimitInfo.m_AlarmCount = 0;
								}
							}
						}
					}
				}
				plMonitorChn->m_CurValue = lTmpChnTotal;
			}	


			for (i = 0; i < plMonitor->m_OutChnNum; i++)
			{
				lTmpChnTotal = 0;
				plMonitorChn = &plMonitor->m_pOutChnArray[i];
				for (k = 0; k < plMonitorChn->m_SubNumber; k++)
				{
					plMonitorSub = &plMonitorChn->m_pSubArray[k];
					if (plParameter->m_pOutChannel[i].m_pSubChannelNode[k].m_ActiveMark != FALSE)
					{
						lTsInd = k + plMonitorChn->m_StartTsIndex;
#ifdef GM2730S
						if (lTsInd < 250)
						{
							plMonitorSub->m_CurValue = HWL_GetBitrate(1, lTsInd);

						}
						else
						{
							plMonitorSub->m_CurValue = HWL_GetBitrate(2, lTsInd - 250);

						}

#else
#ifdef GN1846
						plMonitorSub->m_CurValue = HWL_GetBitrate(pHandle, 1, lTsInd);
#else
						plMonitorSub->m_CurValue = HWL_GetBitrate(1, lTsInd);
#endif
#endif
						lTmpChnTotal += plMonitorSub->m_CurValue;
						if (plMonitor->m_GlobalMark)
						{
							if (plMonitorSub->m_LimitInfo.m_Mark != FALSE)
							{
								if ((plMonitorSub->m_CurValue > plMonitorSub->m_LimitInfo.m_HighLimit) || (plMonitorSub->m_CurValue < plMonitorSub->m_LimitInfo.m_LowLimit))
								{
#ifdef GN1846
									if (pHandle->m_Configuration.m_IpOutputType != IP_OUTPUT_SPTS) {
										CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, i, (lTimeT&GLOBAL_U32_MAX), FALSE);
									} 
#else
									CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, i, (lTimeT&GLOBAL_U32_MAX), FALSE);
#endif
								}
							}
						}
					}
				}
				plMonitorChn->m_CurValue = lTmpChnTotal;
			}
#ifdef GN1846 /* 编码通道状态监测 */
			for (i = 0; i < plMonitor->m_InChnNum; i++)
			{
				plMonitorChn = &plMonitor->m_pInChnArray[i];
				for (k = 0; k < plMonitorChn->m_SubNumber; k++)
				{
					plMonitorSub = &plMonitorChn->m_pSubArray[k];
					if (plParameter->m_pInChannel[i].m_pSubChannelNode[k].m_SubInfo.m_SubENCODERInfo.m_ActiveMark != FALSE)
					{
						if (plMonitor->m_GlobalMark)
						{
							{ /* 输入信号检测 */
								HDMI_RxStatusParam lHdmiRxStatus;

								HWL_HdmiRxGetStatus(k, &lHdmiRxStatus);
								if (lHdmiRxStatus.m_SignalIsLocked == FALSE) { /* 通道打开且信号没有锁定，报信号丢失 */
									if (plMonitorSub->m_HdmiSignalLostCount++ > 5) { /* 连续多次没有信号锁定就报警信号丢失 */
										plMonitorSub->m_HdmiSignalLostCount = 0;
										CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_HDMI_INPUT_LOST_ERROR, k, (lTimeT&GLOBAL_U32_MAX), FALSE);
									}
								}
								else {
									S32 lSetViMode = plParameter->m_pInChannel[i].m_pSubChannelNode[k].m_SubInfo.m_SubENCODERInfo.m_ViMode;

									plMonitorSub->m_HdmiSignalLostCount = 0;
									/* 非自动检测输入模式的情况，如果输入与设置不符要进行报警 */
									if ((lSetViMode != ENC_VI_MODE_AUTO) && (MULTL_Adv7612ViMode2LEncoderViMode(lHdmiRxStatus.m_VideoStandard) != lSetViMode)) {
										CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_HDMI_INPUT_FORMAT_ERROR, k, (lTimeT&GLOBAL_U32_MAX), FALSE);
									}
								}
							}

							{ /* 编码缓冲器溢出检测 */
								HWL_LENCODER_AlarmInfo lAlarmInfo;

								HWL_LENCODER_GetAlarmInfo(i, k, &lAlarmInfo);
								if (lAlarmInfo.m_AlarmInfo.m_BufferOverFlowCount > 0) {
									CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_BUFFER_OVERFLOW_ERROR, k, (lTimeT&GLOBAL_U32_MAX), FALSE);
									lAlarmInfo.m_AlarmInfo.m_BufferOverFlowCount = 0;
									HWL_LENCODER_ResetAlarmInfo(i, k, &lAlarmInfo); /* 报警后清除上一次的报警数据 */
								}
							}
						}
					}
					else {
						plMonitorSub->m_HdmiSignalLostCount = 0;
					}
				}
			}	
#endif
		}
#endif


#ifdef SUPPORT_NEW_HWL_MODULE
		{
			BOOL lEthRecovery = FALSE;
			/*以太网数据接口状态*/
			for (i = 0; i < plMonitor->m_ETHNumber; i++)
			{
#ifdef SUPPORT_HWL_FPGA_REG_MODULE
				plMonitor->m_EthStatus[i] = HWL_FPGARegPHYGetLinkStatus(i);
#else
				if (HWL_MonitorGetETHLinkStatus(i) > 0)
				{
					plMonitor->m_EthStatus[i] = 3;
				}
				else
				{
					plMonitor->m_EthStatus[i] = 0;
				}
#endif
				if (plMonitor->m_EthStatus[i] != 3)
				{
					S32 lChnInd;
					if (i == 0)
					{
						lChnInd = 0;
					}
					else
					{
						lChnInd = 1;
					}

					if (plParameter->m_pInChannel[lChnInd].m_ChannelInfo.m_IPInfo.m_IPMark)
					{
						if (plMonitor->m_GlobalMark)
						{
							GLOBAL_TRACE(("Eth %d Error! Value = %d\n", i, plMonitor->m_EthStatus[i]));
							CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ETH_LINK, plMonitor->m_EthStatus[i] * 10 + lChnInd, (lTimeT&GLOBAL_U32_MAX), FALSE);
						}
					}
				}


				if ((plMonitor->m_LastEthStatus[i] == 0) && (plMonitor->m_EthStatus[i] != 0))
				{
					GLOBAL_TRACE(("ETH %d Recover From Link Down!!!\n", i));
					lEthRecovery = TRUE;
				}

				plMonitor->m_LastEthStatus[i] = plMonitor->m_EthStatus[i];

			}

			/*连接恢复，为了提高E组播恢复的速度这里调用输入ETH应用函数！*/

			//MULTL_ApplyInETHParameter(pHandle, GLOBAL_INVALID_INDEX);

		}


		/*DDR状态获取*/
		{
			U32 lModuleStatus;

			lModuleStatus = HWL_MonitorGetMainFPGAModuleStatus();

			if (lModuleStatus != 0)
			{
				CAL_PrintU32(__FUNCTION__, lModuleStatus, 8);
				HWL_MonitorModuleResetSend(lModuleStatus);
				GLOBAL_TRACE(("Reset Sended!!!!!!!!!\n"));
				//CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
			}
		}
#endif

#ifndef DISABLE_SCS_MODULE
		/*SCS*/
		if(FALSE == pHandle->m_BSSystemInfo.m_ActiveMark)
		{
			if (pHandle->m_Information.m_LicenseSCSNum > 0)
			{
				SCS_GetRuningInfo(plMultSCS->m_SCSHandle, &lRunningStatus);
				for (i = 0; i < plMultSCS->m_SCSCount; i++)
				{
					plMultSCS->m_SCSSlot[i].m_EMMCurBitrate = lRunningStatus.m_EmmBitrate[i] * 1024;
					if (plMonitor->m_GlobalMark)//当码率大于指定码率时也报警
					{
						//if (plMultSCS->m_SCSSlot[i].m_EMMCurBitrate > (plMultSCS->m_SCSSlot[i].m_EMMActualBandwidth * 1.2))
						//{
						//	CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SCS_EMM, 0, 0, FALSE);
						//}
					}
					plMultSCS->m_SCSSlot[i].m_EMMLinkStatus = lRunningStatus.m_EmmStatus[i];
					plMultSCS->m_SCSSlot[i].m_ECMLinkStatus = ((((lRunningStatus.m_EcmChannelNum[i] == lRunningStatus.m_EcmNormalChnNum[i])) && (lRunningStatus.m_EcmChannelNum[i] != 0))?1:0);
				}

				if (plMonitor->m_GlobalMark)
				{
					if (lRunningStatus.m_EmmErrorCount > 0)
					{
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SCS_EMM, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					}
					if (lRunningStatus.m_EcmErrorCount > 0)
					{
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SCS_ECM, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					}
				}


				/*更新ECM时间*/
				GLOBAL_GMTTIME_R(&lTM, &lRunningStatus.m_LastCWUpdateTime);
				CAL_StringDateToStr(plMultSCS->m_pECMTimeDataBuf, &lTM);

				if (lRunningStatus.m_ValidEcmStatus == TRUE)
				{
					if (plMultSCS->m_ScrambleStatus == FALSE)
					{
						/*打开加扰全局开关*/
						if (pHandle->m_Information.m_LicenseSCSNum > 0)//加扰授权控制
						{
							GLOBAL_TRACE(("Start Scramble\n"));
							HWL_ScrambleEnable(TRUE);
							plMultSCS->m_ScrambleStatus = TRUE;
						}
					}
				}
				else
				{
					if (plMultSCS->m_ScrambleStatus == TRUE)
					{
						/*关闭加扰全局开关，同时报警提示SCS系统离线断开！*/
						GLOBAL_TRACE(("Stop Scramble\n"));
						HWL_ScrambleEnable(FALSE);
						plMultSCS->m_ScrambleStatus = FALSE;
					}
				}
			}
		}

#endif
#ifdef MULT_TS_BACKUP_SUPPORT
		if (MULT_BPCheckApplyParameters() == TRUE)
		{
			/*复用参数设置*/
			MULTL_ApplyRemuxParameter(pHandle);
		}
#endif

#ifdef SUPPORT_GNS_MODULE
		/*GPS状态*/
		{
			static S32 s_GPSCheckDelay = 3;
			if (s_GPSCheckDelay <= 0)
			{
				if (MULT_GNSCheckEnabled())
				{
					if (MULT_GNSCheckLocked() == FALSE)
					{
						if (plMonitor->m_GlobalMark)
						{
							CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_GPS_LOCK_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
						}
					}

					if (MULT_GNSCheckError() == TRUE)
					{
						if (plMonitor->m_GlobalMark)
						{
							CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_GPS_ERROR, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
						}
					}
				}
			}
			else
			{
				s_GPSCheckDelay--;
			}
		}
#endif

#ifdef SUPPORT_SFN_MODULATOR
		/*SFN状态*/
		{
			/*输入状态报警*/
			if (MULT_SFNCheckSFNError(SFN_ERROR_ASI_LOST, TRUE) == TRUE)
			{
				if (plMonitor->m_GlobalMark)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}

			/*输入码率大小报警*/
			if (MULT_SFNCheckSFNError(SFN_ERROR_BITRATE_ERROR, TRUE) == TRUE)
			{
				if (MULT_SFNCheckEnabled())
				{
					if (plMonitor->m_GlobalMark)
					{
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					}
				}
			}


			if (MULT_SFNCheckEnabled())
			{
				if (MULT_SFNCheckSFNError(SFN_ERROR_SIP_LOST, TRUE) == TRUE)
				{
					//GLOBAL_TRACE(("SFN SIP Error!\n"));
					if (plMonitor->m_GlobalMark)
					{
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SFN_SIP_ERROR, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SFN_ERROR, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					}
				}
				if (MULT_SFNCheckSFNError(SFN_ERROR_SIP_CRC32_ERR, TRUE) == TRUE)
				{
					//GLOBAL_TRACE(("SFN SIP CRC32 Error!\n"));
					if (plMonitor->m_GlobalMark)
					{
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SFN_SIP_CRC32_ERROR, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					}
				}
				if (MULT_SFNCheckSFNError(SFN_ERROR_SIP_CHANGE, TRUE) == TRUE)
				{
					GLOBAL_TRACE(("SFN SIP Changed!\n"));
					if (plMonitor->m_GlobalMark)
					{
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SFN_SIP_CHANGE, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					}
				}
			}

			if (MULT_SFNCheckSFNError(SFN_ERROR_INT_10M_LOST, TRUE) == TRUE)
			{
				if (plMonitor->m_GlobalMark)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_INT_10M_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}

#ifdef SUPPORT_GNS_MODULE
			if (MULT_GNSCheckLocked())
			{
				if (MULT_SFNCheckSFNError(SFN_ERROR_INT_1PPS_LOST, TRUE) == TRUE)
				{
					if (plMonitor->m_GlobalMark)
					{
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_INT_1PPS_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					}
				}
			}
#endif

			if (MULT_SFNCheckSFNError(SFN_ERROR_EXT_10M_LOST, TRUE) == TRUE)
			{
				if (plMonitor->m_GlobalMark)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_10M_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}

			if (MULT_SFNCheckSFNError(SFN_ERROR_EXT_1PPS_LOST, TRUE) == TRUE)
			{
				if (plMonitor->m_GlobalMark)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_1PPS_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}

		}
#endif

#ifdef SUPPORT_SFN_ADAPTER
		/*SFN状态*/
		{
			/*输入状态报警*/
			if (MULT_SFNACheckSFNError(SFNA_ERROR_ASI_LOST, TRUE) == TRUE)
			{
				if (plMonitor->m_GlobalMark)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}

			if (MULT_SFNACheckSFNError(SFNA_ERROR_INT_10M_LOST, TRUE) == TRUE)
			{
				if (plMonitor->m_GlobalMark)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_INT_10M_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}

#ifdef SUPPORT_GNS_MODULE
			if (MULT_GNSCheckLocked())
			{
				if (MULT_SFNACheckSFNError(SFNA_ERROR_INT_1PPS_LOST, TRUE) == TRUE)
				{
					if (plMonitor->m_GlobalMark)
					{
						CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_INT_1PPS_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
					}
				}
			}
#endif

			if (MULT_SFNACheckSFNError(SFNA_ERROR_EXT_10M_LOST, TRUE) == TRUE)
			{
				if (plMonitor->m_GlobalMark)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_10M_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}

			if (MULT_SFNACheckSFNError(SFNA_ERROR_EXT_1PPS_LOST, TRUE) == TRUE)
			{
				if (plMonitor->m_GlobalMark)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_1PPS_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}

		}
#endif

#ifdef SUPPORT_NTS_DPD_BOARD
		if (MULT_NTSDPDGetError(0, TRUE))
		{
			if (plMonitor->m_GlobalMark)
			{
				CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_1PPS_LOST, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
			}
		}
#endif

#ifdef SUPPORT_CLK_ADJ_MODULE
		if (MULT_CLKGetSyncError(TRUE))
		{
			if (plMonitor->m_GlobalMark)
			{
				CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CLK_SYNC_ERROR, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
			}
		}
#endif


#ifdef GM8358Q
		MULT_Maintenace	 *plMaintSetting;
		plMaintSetting = &pHandle->m_MaintSetting;

		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;
		S32 m;
		{
			HWL_CvbsStatusParam m_EncoderCvbsLockStatusPara[GN_CVBS_CHANNEL_NUM];

			for (i=0; i<GN_CVBS_SUBBOARD_NUM; ++i)
			{
				plChnNode = &pHandle->m_Parameter.m_pInChannel[i];

				if(HWL_GetEncoderCvbsLockStatus(i, m_EncoderCvbsLockStatusPara) == TRUE)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[0];

					for (k = 0; k < GN_CVBS_CHANNEL_NUM; ++k)
					{
						if (plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[k].m_WorkMod == ENCODER_WORK_MODE_ENCODER)
						{
							m = i*GN_CVBS_CHANNEL_NUM+k;
							if(m_EncoderCvbsLockStatusPara[k].m_LockStatus == CVBS_NOLOCKED)
							{	
								CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN, m, (lTimeT&GLOBAL_U32_MAX), FALSE);
							}
						}
					}
				}
				else
				{
					GLOBAL_TRACE(("Get Cvbs Lock Status Error!\n"));
				}
			}
		}

		{
			//BOOL HWL_GetParaFromMainFpga(HWL_MfpgaStatusParam *m_EncoderStatusParam)
			HWL_MfpgaStatusParam m_EncoderStatusParam;
			//static m_EncoderOutputOverFlowErrorCount = 0;
			//static m_EncoderOutputNoBitrateErrorCount = 0;
			static m_EncoderVideoErrorCount = 0;

			if(HWL_GetParaFromMainFpga(&m_EncoderStatusParam)  == TRUE)
			{
				for (i=0; i<GN_ENC_CH_NUM; i++)
				{
					k = i/GN_CVBS_CHANNEL_NUM;
					m = i%GN_CVBS_CHANNEL_NUM;

					plChnNode = &pHandle->m_Parameter.m_pInChannel[k];
					plSubNode = &plChnNode->m_pSubChannelNode[0];
					if (plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[m].m_WorkMod == ENCODER_WORK_MODE_ENCODER)
					{
						if(m_EncoderStatusParam.m_InputErrorFlag[i] == HWL_MFPGA_INPUT_ERROR_VIDEO)
						{	
							CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO, i, (lTimeT&GLOBAL_U32_MAX), FALSE);
							m_EncoderVideoErrorCount++;

							//巡检计数 调试页面使用
							if (GLOBAL_CHECK_INDEX(i, GN_ENC_CH_NUM))
								plMaintSetting->m_EncoderChannelTsStatusCount[i]++;

						}

						if(m_EncoderStatusParam.m_InputErrorFlag[i] == HWL_MFPGA_INPUT_ERROR_CC)
						{	
							//巡检计数 调试页面使用
							if (GLOBAL_CHECK_INDEX(i, GN_ENC_CH_NUM))
								plMaintSetting->m_EncoderConCount[i]++;

						}

						if(m_EncoderVideoErrorCount > Encoder_Error_Count_Max)
						{
							GLOBAL_TRACE(("ReInit Encoder Module\n"));
							m_EncoderVideoErrorCount = 0;

							MULT_EncoderUpgradeInfo* pEncoderUpgradeInfo = GetEncoderUpgradeInfo(); 
							if(pEncoderUpgradeInfo->s_EncoderUpgradeStatus == UPGRADE_STATUS_FINISH)
							{
								HWL_GN_Terminate();
								HWL_GN_Init();
							}
						}
					}
				}

				if (m_EncoderStatusParam.m_OutputErrorFlag == HWL_MFPGA_OUTPUT_ERROR_NOBITRATE)
				{

					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_NOBITRATE, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}

				if (m_EncoderStatusParam.m_OutputErrorFlag == HWL_MFPGA_OUTPUT_ERROR_OVERFLOW)
				{
					CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_OVERFLOW, 0, (lTimeT&GLOBAL_U32_MAX), FALSE);
				}
			}
			else
			{
				GLOBAL_TRACE(("Get Encoder Status Error!\n"));
			}
		}
#endif

#ifdef GN1846
		for (i = 0; i < MULT_MAX_CHN_NUM; i++) {
			HDMI_RxStatusParam lHdmiRxStatus;
			MULT_SubENCODERInfo *plEncInfo = &pHandle->m_Parameter.m_pInChannel[0].m_pSubChannelNode[i].m_SubInfo.m_SubENCODERInfo;
			BOOL lSetParamFlag = FALSE;

			/* 巡检中加入该调用，另一个作用是输入信号格式变化时，
				ADV7612 会自动配置一些寄存器，所以在巡检中还必须
				加入该函数调用，否则标清与高清信号切换时会出现问
				题。
			*/
			HWL_HdmiRxGetStatus(i, &lHdmiRxStatus); 
			//GLOBAL_TRACE(("HDMI[%d] Connected:%d Lock:%d VidStandard:%d AudSample:%d\n", i, plMonitor->m_HdmiRxStatus[i].m_HdmiIsConnected, 
			//	plMonitor->m_HdmiRxStatus[i].m_SignalIsLocked, plMonitor->m_HdmiRxStatus[i].m_VideoStandard, plMonitor->m_HdmiRxStatus[i].m_AudioSample));
			if (plEncInfo->m_ActiveMark) { /* 开启该通道，才会判断是否自动设置参数 */
				if (lHdmiRxStatus.m_SignalIsLocked) {
					if (plEncInfo->m_ViMode == ENC_VI_MODE_AUTO) {
						if (lHdmiRxStatus.m_VideoStandard != plMonitor->m_HdmiRxStatus[i].m_VideoStandard) {
							lSetParamFlag = TRUE;
						}
					}
					if (plEncInfo->m_AudSample == ENC_AUD_SAMP_AUTO) {
						if (lHdmiRxStatus.m_AudioSample != plMonitor->m_HdmiRxStatus[i].m_AudioSample) {
							lSetParamFlag = TRUE;
						}
					}
				}
				if (lHdmiRxStatus.m_SignalIsLocked != plMonitor->m_HdmiRxStatus[i].m_SignalIsLocked) { /* 信号有无变化会触发一次参数配置 */
					lSetParamFlag = TRUE;
				}
			}
			GLOBAL_MEMCPY(&plMonitor->m_HdmiRxStatus[i], &lHdmiRxStatus, sizeof(lHdmiRxStatus));
			if (lSetParamFlag) {
				HWL_LENCODER_SubParam lLEncoderSubParam;

				lLEncoderSubParam.m_bEnable = plEncInfo->m_ActiveMark;
				lLEncoderSubParam.m_UDPAddr = plEncInfo->m_IPv4Addr;
				lLEncoderSubParam.m_UDPPort = plEncInfo->m_IPv4Port;
				lLEncoderSubParam.m_EncParam.m_IpOutputType = pHandle->m_Configuration.m_IpOutputType;
				lLEncoderSubParam.m_EncParam.m_ActiveMark = lLEncoderSubParam.m_bEnable;
				lLEncoderSubParam.m_EncParam.m_SignalIsLocked = lHdmiRxStatus.m_SignalIsLocked;
				lLEncoderSubParam.m_EncParam.m_DestIp = lLEncoderSubParam.m_UDPAddr;
				lLEncoderSubParam.m_EncParam.m_DestPort = lLEncoderSubParam.m_UDPPort;
				lLEncoderSubParam.m_EncParam.m_IpProto = plEncInfo->m_Protocol;
				lLEncoderSubParam.m_EncParam.m_SendTsNum = 7;
				lLEncoderSubParam.m_EncParam.m_ProgNumber = plEncInfo->m_ProgNumber;
				lLEncoderSubParam.m_EncParam.m_ProgBitrate = plEncInfo->m_ProgBitrate;
				GLOBAL_STRCPY(lLEncoderSubParam.m_EncParam.m_pProgName, plEncInfo->m_pProgName);
				lLEncoderSubParam.m_EncParam.m_PsiCharSet = pHandle->m_Configuration.m_OutputCharset;

				lLEncoderSubParam.m_EncParam.m_VidPid = plEncInfo->m_VidPid;
				lLEncoderSubParam.m_EncParam.m_PcrPid = plEncInfo->m_PcrPid;
				lLEncoderSubParam.m_EncParam.m_AudPid = plEncInfo->m_AudPid;
				lLEncoderSubParam.m_EncParam.m_PmtPid = plEncInfo->m_PmtPid;
				/* 注意下面这些值是否需要映射转换！！ */
				if (plEncInfo->m_ViMode == ENC_VI_MODE_AUTO) {
					lLEncoderSubParam.m_EncParam.m_ViMode = MULTL_Adv7612ViMode2LEncoderViMode(lHdmiRxStatus.m_VideoStandard);
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
					lLEncoderSubParam.m_EncParam.m_AudSample = MULTL_Adv7612AudSample2LEncoderAudSample(lHdmiRxStatus.m_AudioSample);
				}
				else {
					lLEncoderSubParam.m_EncParam.m_AudSample = plEncInfo->m_AudSample;
				}
				lLEncoderSubParam.m_EncParam.m_AudVolume = plEncInfo->m_Volume;
				/* 调整参数 */
				lLEncoderSubParam.m_EncParam.m_AudPtsRelativeDelayTime = pHandle->m_MaintSetting.m_AudPtsRelativeDelayTime[i];
				lLEncoderSubParam.m_EncParam.m_PtsDelayTime = pHandle->m_MaintSetting.m_PtsDelayTime[i];
				lLEncoderSubParam.m_EncParam.m_MaxPtsPcrInterval = pHandle->m_MaintSetting.m_MaxPtsPcrInterval[i];
				lLEncoderSubParam.m_EncParam.m_MinPtsPcrInterval = pHandle->m_MaintSetting.m_MinPtsPcrInterval[i];
				lLEncoderSubParam.m_EncParam.m_AudDelayFrameNum = pHandle->m_MaintSetting.m_AudDelayFrameNum[i];

				HWL_LENCODER_SetSubParam(0, i, &lLEncoderSubParam);
			}
		}
#endif

		//CPU/MEM占用读取
		{
			S32 lTotal, lCurrent;
			plMonitor->m_CPUUsage = PL_CPUAVGUsageCAL();
			PL_CPUAVGUsageSet();

			PL_MEMUsageGet(&lTotal, &lCurrent);
			if (lTotal > 0)
			{
				plMonitor->m_MEMUsage = (S32)(((F32)(lCurrent) / lTotal) * 100); 
			}
		}
	}



	/*系统忙状态*/
#ifdef ENCODER_CARD_PLATFORM
	plMonitor->m_BusyMark = MPEG2_DBAnalyzeGetBusyStatus(pHandle->m_DBSHandle) || MULT_CARDModuleIsBusy();
#else
	plMonitor->m_BusyMark = MPEG2_DBAnalyzeGetBusyStatus(pHandle->m_DBSHandle);
#endif
}

/*EOF*/
