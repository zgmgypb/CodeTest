/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
/*主回调函数*/
void MULTL_SCSManagerCBFn(SCS_Evnet *pEvent,void* pUserParam)
{
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;
	if (plHandle)
	{
		S32 i, k, n, lSCSIndex, lPacketNum;
		MULT_SCS *plMULTSCS;
		MULT_SCSSlot *plSCSSlot;
		plMULTSCS = &plHandle->m_MultSCS;
		if (pEvent->m_Event == SCS_EVENT_EMM_TS_DATA)
		{
			SCS_EmmData *plEmmData;
			MULT_SCSEMMSlot *plMultEMM;
			plEmmData = (SCS_EmmData *)pEvent->m_vParam;

			for (lSCSIndex = 0; lSCSIndex < plMULTSCS->m_SCSCount; lSCSIndex++)
			{
				plSCSSlot = &plMULTSCS->m_SCSSlot[lSCSIndex];
				if (plSCSSlot->m_bActiveMark == TRUE)
				{
					if (plSCSSlot->m_SuperCASID == plEmmData->m_SuperCasId)
					{
#ifdef MULT_EMM_REPLICATE_MODE
						//复制模式时仅仅发送EMM到第一个可用端口（符合PID映射）
						for (n = 0; n < plMULTSCS->m_EMMMaxNum; n++)
						{
							plMultEMM = &plMULTSCS->m_pEMMSlotArray[n];
							if (plMultEMM->m_pInfo[lSCSIndex].m_OutputMark)
							{
								/*按照目的PID修改对应ECM的PID*/
								lPacketNum = plEmmData->m_TsData.m_TsLen / MPEG2_TS_PACKET_SIZE;
								for (k = 0; k < lPacketNum; k++)
								{
									MPEG2_TsSetPID(&plEmmData->m_TsData.m_Ts[k * MPEG2_TS_PACKET_SIZE], plMultEMM->m_pInfo[lSCSIndex].m_PID);
									TSP_InsertTsPacket(plMultEMM->m_OutTsIndex, &plEmmData->m_TsData.m_Ts[k * MPEG2_TS_PACKET_SIZE]);
									//HWL_SetDirectOutTsPacket(plMultEMM->m_OutTsIndex, &plEmmData->m_TsData.m_Ts[k * MPEG2_TS_PACKET_SIZE], MPEG2_TS_PACKET_SIZE);
								}
								break;
							}


						}
#else
						for (n = 0; n < plMULTSCS->m_EMMMaxNum; n++)
						{
							plMultEMM = &plMULTSCS->m_pEMMSlotArray[n];
							if (plMultEMM->m_pInfo[lSCSIndex].m_OutputMark)
							{
								/*按照目的PID修改对应ECM的PID*/
								lPacketNum = plEmmData->m_TsData.m_TsLen / MPEG2_TS_PACKET_SIZE;
								for (k = 0; k < lPacketNum; k++)
								{
									MPEG2_TsSetPID(&plEmmData->m_TsData.m_Ts[k * MPEG2_TS_PACKET_SIZE], plMultEMM->m_pInfo[lSCSIndex].m_PID);
									HWL_SetDirectOutTsPacket(plMultEMM->m_OutTsIndex, &plEmmData->m_TsData.m_Ts[k * MPEG2_TS_PACKET_SIZE], MPEG2_TS_PACKET_SIZE);
								}
							}
						}
#endif
						break;
					}
				}
			}
		}
		else if (pEvent->m_Event == SCS_EVENT_ECM_COMBINE_DATA)
		{
			MULT_SCSECMSlot *plECMSlot;
			SCS_EcmData *plEcmData, *plEcmDataArray;
			SCS_SectionTsData *plTsData;

			plEcmDataArray = (SCS_EcmData*)pEvent->m_vParam;
			if (plEcmDataArray)
			{
				for (i = 0; i < pEvent->m_iParam; i++)
				{
					plEcmData = &plEcmDataArray[i];
					plECMSlot = (MULT_SCSECMSlot *)plEcmData->m_pUserParam;
					if (plECMSlot->m_CryptoStreamID == plEcmData->m_CryptoId)
					{
						for (lSCSIndex = 0 ; lSCSIndex < SCS_MAX_CAS_SUPPORT_NUM ; lSCSIndex++)
						{
							plSCSSlot = &plMULTSCS->m_SCSSlot[lSCSIndex];
							if (plSCSSlot->m_bActiveMark)//SCS要开启
							{
								if (plECMSlot->m_pInfo[lSCSIndex].m_OutputMark)//允许ECM输出
								{
									plTsData = &plEcmData->m_pTsData[lSCSIndex];
									lPacketNum = plTsData->m_TsLen / MPEG2_TS_PACKET_SIZE;
									for (k = 0; k < lPacketNum; k++)
									{
										MPEG2_TsSetPID(&plTsData->m_Ts[k * MPEG2_TS_PACKET_SIZE], plECMSlot->m_pInfo[lSCSIndex].m_PID);
									}

									if (plECMSlot->m_pInfo[lSCSIndex].m_ISRID)
									{
										TSP_PSIInserterRemove(plECMSlot->m_pInfo[lSCSIndex].m_ISRID);
										plECMSlot->m_pInfo[lSCSIndex].m_ISRID = 0;
									}

									plECMSlot->m_pInfo[lSCSIndex].m_ISRID = TSP_PSIInserterAdd(plECMSlot->m_OutTsIndex, plTsData->m_Ts, plTsData->m_TsLen, plEcmData->m_RepPeriod);
								}
							}
						}
					}
					else
					{
						GLOBAL_ASSERT(0);
					}
				}
			}
		}
		else if (pEvent->m_Event == SCS_EVENT_ECM_CW_UPDATE)
		{
			/*CW UPDATE*/
			S32 i, lCWGroup;
			HWL_ControlWord_t lHWLCW;
			SCS_CwGroupInfo* pCwGroup = (SCS_CwGroupInfo*)pEvent->m_vParam;
			lCWGroup = pEvent->m_iParam;
			if (pCwGroup && (lCWGroup > 0))
			{
				// 				GLOBAL_TRACE(("Update CW Group for CP = %d\n", pCwGroup[0].m_CpNumber))
				/*更换奇偶控制字*/
#ifdef GM2700S //2700仅支持30对控制字组
				if (lCWGroup > 30)
				{
					lCWGroup = 30;
				}
#endif
				PFC_TaskSleep(1500);

				for (i = 0; i < lCWGroup; i++)
				{
					lHWLCW.cwGroup = (U8)(pCwGroup[i].m_CwGroupIndex & 0x7F);
					lHWLCW.evenOrOdd = pCwGroup[i].m_CpNumber % 2;
					//GLOBAL_TRACE(("Value = %d\n", lHWLCW.evenOrOdd));
					GLOBAL_MEMCPY(lHWLCW.words, pCwGroup[i].m_pCW, SCS_CW_LEN);
					HWL_ControlWordSet(&lHWLCW);
				}
			}
			else
			{
				GLOBAL_TRACE(("CW Update  ERROR!!!\n"));
			}
		}
		else if (pEvent->m_Event == SCS_EVENT_CW_SWITCH)
		{
			plMULTSCS->m_CurrentCPNumber = (S32)pEvent->m_vParam;
			//GLOBAL_TRACE(("Value = %d\n", plMULTSCS->m_CurrentCPNumber % 2));
			HWL_ControlWordSwitchSet(plMULTSCS->m_CurrentCPNumber % 2);
			// 			GLOBAL_TRACE(("Switch CW Group for CP = %d, to %d\n", plMULTSCS->m_CurrentCPNumber, plMULTSCS->m_CurrentCPNumber % 2));
		}
		else if (pEvent->m_Event == SCS_EVENT_ECM_CP_DURATION)
		{
			plMULTSCS->m_CurrentCPDuration = pEvent->m_iParam;
		}
		else if (pEvent->m_Event == SCS_EVENT_ECM_COMPUTE_TIME)
		{
			plMULTSCS->m_CurrentComputeTime = pEvent->m_iParam;
		}
		else if (pEvent->m_Event == SCS_EVENT_EMM_BAND_REQUEST)
		{
			SCS_EmmBandRequestCancel *plRC;
			plRC = (SCS_EmmBandRequestCancel*)pEvent->m_vParam;
			for (lSCSIndex = 0; lSCSIndex < plMULTSCS->m_SCSCount; lSCSIndex++)
			{
				plSCSSlot = &plMULTSCS->m_SCSSlot[lSCSIndex];
				if (plSCSSlot->m_SuperCASID == plRC->m_SuperCasId)
				{
					plSCSSlot->m_EMMActualBandwidth = plRC->m_BandRequest * 1024;//记录下实际的EMM带宽
					break;
				}
			}
			GLOBAL_ASSERT (lSCSIndex < plMULTSCS->m_SCSCount);
		}
	}
}


void MULTL_SCSThreadFn(void* pUserParam)
{
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;
	if (plHandle)
	{
		MULT_SCS *plMultSCS;
		plMultSCS = &plHandle->m_MultSCS;

		/*进入线程LOOP*/
		plMultSCS->m_ThreadMark = MULT_TASK_MARK_RUN;
		while(plMultSCS->m_ThreadMark == MULT_TASK_MARK_RUN)
		{
			SCS_Access(plMultSCS->m_SCSHandle);
		}
		plMultSCS->m_ThreadMark = MULT_TASK_MARK_STOP;
	}
}

void BSS_EncrypLibGetCW(unsigned char pSW[BSS_SESSION_WORD_LENGTH], unsigned char pUserID[BSS_USERID_LENGTH], unsigned char pCW[BSS_CONTROL_WORD_LENGTH])
{
	CRYPTO_AESContext lCT;
	U8 plTmpCW[BSS_SESSION_WORD_LENGTH];
	CRYPTO_AESSetKey(&lCT, pUserID, BSS_USERID_LENGTH * 8);
	CRYPTO_AESDecrypt(&lCT, pSW, plTmpCW);
	GLOBAL_MEMCPY(pCW, plTmpCW, 8);
}


/* API函数 ---------------------------------------------------------------------------------------------------------------------------------------- */
void MULTL_SCSManagerInitiate(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	S32 i;
	MULT_SCS *plMultSCS;
	plMultSCS = &pHandle->m_MultSCS;
#ifdef SUPPORT_NEW_HWL_MODULE
	for (i = 0; i < pHWInfo->m_OutChnNum; i++)
	{
		if ((pHWInfo->m_pOutChn[i].m_Type != HWL_CHANNEL_TYPE_IP_LOOP) && (pHWInfo->m_pOutChn[i].m_Type != HWL_CHANNEL_TYPE_IP_LOOP_DEP))
		{
			plMultSCS->m_EMMMaxNum += pHWInfo->m_pOutChn[i].m_CurSubSupport;
		}
	}
#else
	for (i = 0; i < pHWInfo->m_ChannelNum; i++)
	{
		if (pHWInfo->m_pInfoList[i].m_Direction == HWL_CHANNEL_DIRECTION_OUT)
		{
			if ((pHWInfo->m_pInfoList[i].m_Type != HWL_CHANNEL_TYPE_IP_LOOP) && (pHWInfo->m_pInfoList[i].m_Type != HWL_CHANNEL_TYPE_IP_LOOP_DEP))
			{
				plMultSCS->m_EMMMaxNum += pHWInfo->m_pInfoList[i].m_CurSubSupport;
			}
		}
	}
#endif
	plMultSCS->m_ECMMaxNum = plMultSCS->m_EMMMaxNum * MULT_SCS_MAX_SERVICE_NUM_PER_TS;
	plMultSCS->m_pECMSlotArray = (MULT_SCSECMSlot *)GLOBAL_ZMALLOC(sizeof(MULT_SCSECMSlot) * plMultSCS->m_ECMMaxNum);
	plMultSCS->m_pEMMSlotArray = (MULT_SCSEMMSlot *)GLOBAL_ZMALLOC(sizeof(MULT_SCSEMMSlot) * plMultSCS->m_EMMMaxNum);
}

void MULTL_SCSManagerApply(MULT_Handle *pHandle, BOOL bClose)
{
	MULT_SCS *plMultSCS;
	//MULT_BISS *lBSSystemInfo;
	if (pHandle)
	{
		S32 i, k;

		/*不支持CA的模块就不要应用CA参数了*/
		if (pHandle->m_Information.m_LicenseSCSNum == 0)
		{
			return;
		}

		plMultSCS = &pHandle->m_MultSCS;
		/*如果当前SCS模块正在运行，则首先关闭线程，销毁模块*/
		if (plMultSCS->m_ThreadHandle && (plMultSCS->m_ThreadMark == MULT_TASK_MARK_RUN))
		{
			plMultSCS->m_ThreadMark = MULT_TASK_MARK_WAIT_STOP;
			//GLOBAL_TRACE(("Wait Thread 0x%.8X Stop---------------------------\n", plMultSCS->m_ThreadHandle));
			PFC_TaskWait(plMultSCS->m_ThreadHandle, GLOBAL_INVALID_INDEX);
			//GLOBAL_TRACE(("Wait Thread 0x%.8X Complete-----------------------\n", plMultSCS->m_ThreadHandle));
			plMultSCS->m_ThreadHandle = NULL;
		}

		if (plMultSCS->m_SCSHandle)
		{
			SCS_Terminal(plMultSCS->m_SCSHandle);
			plMultSCS->m_SCSHandle = NULL;
		}

		for (i = 0; i < plMultSCS->m_ECMCurNum; i++)
		{
			for (k = 0; k < plMultSCS->m_SCSCount; k++)
			{
				if (plMultSCS->m_pECMSlotArray[i].m_pInfo[k].m_ISRID)
				{
					TSP_PSIInserterRemove(plMultSCS->m_pECMSlotArray[i].m_pInfo[k].m_ISRID);
					plMultSCS->m_pECMSlotArray[i].m_pInfo[k].m_ISRID = 0;
				}
			}
		}


		if (bClose == FALSE)
		{

			if (FALSE == pHandle->m_BSSystemInfo.m_ActiveMark)
			{

				S32 i, k, lSCSNum, lCurECMNum;
				S16 lOutTsIndex;
				U16 lEmmPID;
				U32 lSCSIDs, lServiceIDs, lSCSCAIDs, lTsIDs;
				MULT_SCSECMSlot *plSlot;
				MPEG2_DBSCSInfo lDBSCSInfo;
				MPEG2_DBServiceOutInfo lDBServInfo;
				MPEG2_DBSCSCAInfo lDBSCSCAInfo;
				MPEG2_DBSCSACInfo lDBSCSACInfo;
				SCS_CAInfo lSCSCAInfo;
				SCS_EcmCryptoStreamInfo lECMStreamInfo;
				MPEG2_DBSCSSystemInfo lDBSCSSystemInfo;
				MPEG2_DBTsRouteInfo lRouteInfo;
				MULT_SCSEMMSlot *plEMMSlot;
				MULT_SCSEMMInfo *plEMMInfo;

				/*然后新建模块*/
				SCS_Initiate(&plMultSCS->m_SCSHandle, MULTL_SCSManagerCBFn, pHandle);

				/*读取SCS模块系统设置信息*/
				MPEG2_DBProcSCSSystemInfo(pHandle->m_DBSHandle, &lDBSCSSystemInfo, TRUE);

				SCS_SetCpDuration(plMultSCS->m_SCSHandle, lDBSCSSystemInfo.m_DefaultCPDuration);//设置默认最小加扰周期

				SCS_SetWorstNetworkDelay(plMultSCS->m_SCSHandle, lDBSCSSystemInfo.m_NetworkDelay);//设置网络最大延时

				SCS_SetFixedCW(plMultSCS->m_SCSHandle, lDBSCSSystemInfo.m_pFxiedCW);//设置默认控制字

				SCS_SetUSEFixedCW(plMultSCS->m_SCSHandle, lDBSCSSystemInfo.m_bUserFixedCW);//设置默认控制字开关

				//for(i=0; i<MPEG2_DB_SCS_CW_LEN; i++)
				//{
				//GLOBAL_TRACE(("lDBSCSSystemInfo.m_pFxiedCW------------------0x%x\n",lDBSCSSystemInfo.m_pFxiedCW[i]));
				//}


				/*配置SCS信息*/
				lSCSNum = MPEG2_DBGetSCSCount(pHandle->m_DBSHandle);
				for (i = 0; i < lSCSNum; i++)
				{
					lSCSIDs = MPEG2_DBGetSCSIDs(pHandle->m_DBSHandle, i);
					MPEG2_DBProcSCSInfo(pHandle->m_DBSHandle, lSCSIDs, &lDBSCSInfo, TRUE);
					lSCSCAInfo.m_bEmmUseUDP = FALSE;
					lSCSCAInfo.m_EcmgIP = lDBSCSInfo.m_ECMIPv4;
					lSCSCAInfo.m_EcmPort = lDBSCSInfo.m_ECMPort;
					lSCSCAInfo.m_EmmPort = lDBSCSInfo.m_EMMPort;
					lSCSCAInfo.m_SuperCASID = lDBSCSInfo.m_CASystemID << 16 | lDBSCSInfo.m_CASubSystemID;
					lSCSCAInfo.m_MaxEMMBitrate = MULT_SCS_MAX_EMM_BITRATE;
					SCS_SetCasInfo(plMultSCS->m_SCSHandle, &lSCSCAInfo, i);

					/*设置私有SCSChannelSetup消息标志*/
					//GLOBAL_TRACE(("SCS %d Set PrivateSCSMark = %d\n", i, pHandle->m_MaintSetting.m_PrivateChannelSetupMark));
					SCS_SetPrivateChannelSetupSCSMark(plMultSCS->m_SCSHandle, i, pHandle->m_MaintSetting.m_PrivateChannelSetupMark);

					if (i < pHandle->m_Information.m_LicenseSCSNum)//加扰授权控制
					{
#ifdef REQUEST_2014_0110_ONLY_GOS_CA
						if (lDBSCSInfo.m_CASystemID != 0x5448 /*本公司CA*/)
						{
							GLOBAL_TRACE(("CAS Restricted %d\n", i));
							SCS_SetCasStatus(plMultSCS->m_SCSHandle, i, FALSE);
						}
						else
						{
							SCS_SetCasStatus(plMultSCS->m_SCSHandle, i, lDBSCSInfo.m_ActiveMark);
						}
#else
						SCS_SetCasStatus(plMultSCS->m_SCSHandle, i, lDBSCSInfo.m_ActiveMark);
#endif
					}
					else
					{
						SCS_SetCasStatus(plMultSCS->m_SCSHandle, i, FALSE);
					}



					plMultSCS->m_SCSSlot[i].m_SuperCASID = lSCSCAInfo.m_SuperCASID;
					plMultSCS->m_SCSSlot[i].m_bActiveMark = lDBSCSInfo.m_ActiveMark;
				}
				plMultSCS->m_SCSCount = lSCSNum;

				/*配置EMM信息*/
				for (k = 0; k < plMultSCS->m_EMMMaxNum; k++)
				{
					lTsIDs = MPEG2_DBGetTsIDs(pHandle->m_DBSHandle, FALSE, k);
					MPEG2_DBGetTsRouteInfo(pHandle->m_DBSHandle, lTsIDs, FALSE, &lRouteInfo);
					lSCSCAIDs = 0;
					lSCSCAIDs = MPEG2_DBGetSCSCANextNode(pHandle->m_DBSHandle, lSCSCAIDs, MPEG2_DB_CA_TYPE_TS, lTsIDs);//每个TS仅一个项目
					if (lSCSCAIDs != 0)
					{
						if (MPEG2_DBGetSCSCAInfo(pHandle->m_DBSHandle, lSCSCAIDs, &lDBSCSCAInfo))
						{
							plMultSCS->m_pEMMSlotArray[k].m_OutTsIndex = k;
							for (i = 0; i < lSCSNum; i++)
							{
								lSCSIDs = MPEG2_DBGetSCSIDs(pHandle->m_DBSHandle, i);
								MPEG2_DBProcSCSInfo(pHandle->m_DBSHandle, lSCSIDs, &lDBSCSInfo, TRUE);
								/*EMM PID和EMM 开关*/
								plMultSCS->m_pEMMSlotArray[k].m_pInfo[i].m_PID = lDBSCSCAInfo.m_pOutputCaInfo[i].m_CaPID;
								if (lRouteInfo.m_ActiveMark == TRUE || lDBSCSInfo.m_ActiveMark == FALSE)
								{
									plMultSCS->m_pEMMSlotArray[k].m_pInfo[i].m_OutputMark = FALSE;//直通模式下，不允许传输ECM
								}
								else
								{
									plMultSCS->m_pEMMSlotArray[k].m_pInfo[i].m_OutputMark = lDBSCSCAInfo.m_pOutputCaInfo[i].m_OutputMark;
								}
							}
						}
						else
						{
							GLOBAL_TRACE(("Error! Get SCSCA Info Failed\n"));
						}
					}
					else
					{
						GLOBAL_TRACE(("Error!!! Ts %d No SCS Data!!!!!!!!!!\n", k));
					}
				}

#ifdef MULT_EMM_REPLICATE_MODE

				/*设定内部复制路径*/
				for (i = 0; i < lSCSNum; i++)
				{
					lEmmPID = MPEG2_TS_PACKET_NULL_PID;
					for (k = 0; k < plMultSCS->m_EMMMaxNum; k++)
					{
						plEMMSlot = &plMultSCS->m_pEMMSlotArray[k];
						plEMMInfo = &plEMMSlot->m_pInfo[i];
						if (plEMMInfo->m_OutputMark == TRUE)
						{
							if (lEmmPID == MPEG2_TS_PACKET_NULL_PID)
							{
								lEmmPID = plEMMInfo->m_PID;//使用第一个为复制目标
							}
							//GLOBAL_TRACE(("EMM Map %d To %d, TsIndex %d\n", lEmmPID, plEMMInfo->m_PID, plEMMSlot->m_OutTsIndex));
							TSP_AddPIDMap(MULT_TSP_INTERNAL_PID_MAP_SRC, lEmmPID, plEMMSlot->m_OutTsIndex, plEMMInfo->m_PID, FALSE, 0);
						}
					}
				}
#endif



				/*配置加扰节目信息*/
				lCurECMNum = 0;
				lServiceIDs = 0;
				while((lServiceIDs = MPEG2_DBGetServiceNextNode(pHandle->m_DBSHandle, lServiceIDs)) != 0)
				{
					if (MPEG2_DBGetServiceOutInfo(pHandle->m_DBSHandle, lServiceIDs, &lDBServInfo))
					{
						lOutTsIndex = MPEG2_DBGetServicTsIndex(pHandle->m_DBSHandle, lServiceIDs, FALSE);

						if (GLOBAL_CHECK_INDEX(lOutTsIndex, pHandle->m_Parameter.m_MaxOutTsNumber))//表示节目是要输出的
						{
							if (MPEG2_DB_SERVICE_MARK_IS_SCRAMBLE(lDBServInfo.m_ServiceMark))
							{
								lSCSCAIDs = 0;

								lTsIDs = MPEG2_DBGetTsIDs(pHandle->m_DBSHandle, FALSE, lOutTsIndex);
								MPEG2_DBGetTsRouteInfo(pHandle->m_DBSHandle, lTsIDs, FALSE, &lRouteInfo);

								if (lRouteInfo.m_ActiveMark == TRUE)
								{
									GLOBAL_TRACE(("Error!!!!!! Service IDs = 0x%.8X At Out Ts %d, Can not Be Scrambled In Route Mode!!!!!!!!\n", lServiceIDs, lOutTsIndex));
								}
								else
								{
									while ((lSCSCAIDs = MPEG2_DBGetSCSCANextNode(pHandle->m_DBSHandle, lSCSCAIDs, MPEG2_DB_CA_TYPE_SERVICE, lServiceIDs)) != 0)
									{
										if (MPEG2_DBGetSCSCAInfo(pHandle->m_DBSHandle, lSCSCAIDs, &lDBSCSCAInfo))
										{
											if (GLOBAL_CHECK_INDEX(lCurECMNum, plMultSCS->m_ECMMaxNum))
											{
												plSlot = &plMultSCS->m_pECMSlotArray[lCurECMNum];

												GLOBAL_ASSERT(plSlot->m_CryptoStreamID);
#ifdef GM2700S//2700仅支持30对控制字组
												lDBSCSCAInfo.m_ScrambleData = CAL_RandGetRange(0, 29);
#else
												lDBSCSCAInfo.m_ScrambleData = CAL_RandGetRange(0, GLOBAL_U8_MAX) & 0x7F;
#endif
												lECMStreamInfo.m_CwGroupIndex = (S32)lDBSCSCAInfo.m_ScrambleData;

												SCS_AddCryptoStream(plMultSCS->m_SCSHandle, &plSlot->m_CryptoStreamID, plSlot);

												for (i = 0; i < lSCSNum; i++)
												{
													/*保存好对应ECM PID和ECM开关*/
													plSlot->m_OutTsIndex = lOutTsIndex;
													plSlot->m_pInfo[i].m_PID = lDBSCSCAInfo.m_pOutputCaInfo[i].m_CaPID;
													plSlot->m_pInfo[i].m_OutputMark = lDBSCSCAInfo.m_pOutputCaInfo[i].m_OutputMark;

													/*将AC信息读出并传递给SCS模块*/
													if (plMultSCS->m_SCSSlot[i].m_bActiveMark)
													{
														if (MPEG2_DBProcACInfo(pHandle->m_DBSHandle, lDBSCSCAInfo.m_pOutputCaInfo[i].m_SPInfo.m_ACIDs, &lDBSCSACInfo, TRUE) == FALSE)
														{
															lECMStreamInfo.m_AcInfoLen = 0;
														}
														else
														{
															lECMStreamInfo.m_AcInfoLen = lDBSCSACInfo.m_ACDataSize;
															if (lDBSCSACInfo.m_ACDataSize <= SCS_MAX_ACCESS_CRITERIA_LEN)
															{
																GLOBAL_MEMCPY(lECMStreamInfo.m_pAcInfo, lDBSCSACInfo.m_pAccessData, lECMStreamInfo.m_AcInfoLen);
															}
															else
															{
																GLOBAL_ASSERT(0);
															}
														}

														SCS_ModifyCryptoStreamInfo(plMultSCS->m_SCSHandle, &lECMStreamInfo, plSlot->m_CryptoStreamID, i);
													}
												}

												if (i == lSCSNum)
												{
													lCurECMNum++;
												}
												else
												{
													GLOBAL_TRACE(("AC Infor Error, ECM Not Added!!!!!!!!!!\n"));
												}


											}
											MPEG2_DBSetSCSCAInfo(pHandle->m_DBSHandle, lSCSCAIDs, &lDBSCSCAInfo);//将修改了的加扰数据重新保存进系统，以供后面PIDmap时使用！！注意此时每次更新复用数据时SCS模块必须先与复用模块更新！！！！！！另外此修改不保存在参数文件当中！！！
										}
										break;
									}

								}

							}
						}
					}
					else
					{
						GLOBAL_ASSERT(0);
					}
				}
				plMultSCS->m_ECMCurNum = lCurECMNum;

				/*打开SCS线程*/
				plMultSCS->m_ThreadHandle = PFC_TaskCreate("SCS", 1024*1024, MULTL_SCSThreadFn, 0, pHandle);
				//GLOBAL_TRACE(("New SCS Thread ID = 0x%.8X\n", plMultSCS->m_ThreadHandle));
			}
			else
			{
				/*------当BSS开关打开时，  参数设置---------------*/

				S32 i, k, lSCSNum, lCurECMNum;
				S16 lOutTsIndex;
				U16 lEmmPID;
				U32 lSCSIDs, lServiceIDs, lSCSCAIDs, lTsIDs;
				U8 plControlWord[MPEG2_DB_SCS_CW_LEN];
				MULT_SCSECMSlot *plSlot;
				MPEG2_DBSCSInfo lDBSCSInfo;
				MPEG2_DBServiceOutInfo lDBServInfo;
				MPEG2_DBSCSCAInfo lDBSCSCAInfo;
				MPEG2_DBSCSACInfo lDBSCSACInfo;
				SCS_CAInfo lSCSCAInfo;
				SCS_EcmCryptoStreamInfo lECMStreamInfo;
				MPEG2_DBSCSSystemInfo lDBSCSSystemInfo;
				MPEG2_DBTsRouteInfo lRouteInfo;
				MULT_SCSEMMSlot *plEMMSlot;
				MULT_SCSEMMInfo *plEMMInfo;
				HWL_ControlWord_t lControlWord;

				/*--固定控制字开关打开时，默认使用该控制字--*/
				MPEG2_DBProcSCSSystemInfo(pHandle->m_DBSHandle, &lDBSCSSystemInfo, TRUE);

				if(lDBSCSSystemInfo.m_bUserFixedCW == TRUE)
				{
					lControlWord.cwGroup = 0;
					for(i=0; i<MPEG2_DB_SCS_CW_LEN; i++)
					{
						lControlWord.words[i] =  lDBSCSSystemInfo.m_pFxiedCW[i];
						//GLOBAL_TRACE(("sw.words[i]= --------------0x%x\n", lControlWord.words[i]));
					}
					lControlWord.evenOrOdd = 1;
					HWL_ControlWordSet(&lControlWord);
					lControlWord.evenOrOdd = 0;
					HWL_ControlWordSet(&lControlWord);
				}
				else
				{
					/* ----由key和SW获得CW 并设置-------*/
					BSS_EncrypLibGetCW(pHandle->m_BSSystemInfo.m_pSW, pHandle->m_BSSystemInfo.m_pKey,plControlWord);
					lControlWord.cwGroup = 0;
					for(i=0; i<MPEG2_DB_SCS_CW_LEN; i++)
					{
						lControlWord.words[i] = plControlWord[i];
						//GLOBAL_TRACE(("sw.words[i]= ----------------0x%x\n", lControlWord.words[i]));

					}
					lControlWord.evenOrOdd = 1;
					HWL_ControlWordSet(&lControlWord);
					lControlWord.evenOrOdd = 0;
					HWL_ControlWordSet(&lControlWord);

				}
				HWL_ControlWordSwitchSet(0);

				MPEG2_DBBSSSetParameters(pHandle->m_DBSHandle, pHandle->m_BSSystemInfo.m_SuperCASID, pHandle->m_BSSystemInfo.m_ActiveMark);



				if (pHandle->m_BSSystemInfo.m_ActiveMark)
				{
					HWL_ScrambleEnable(TRUE);
					plMultSCS->m_ScrambleStatus = TRUE;
				}
				else
				{
					HWL_ScrambleEnable(FALSE);
					plMultSCS->m_ScrambleStatus = FALSE;
				}


				/*配置加扰节目信息*/

				lCurECMNum = 0;
				lServiceIDs = 0;
				while((lServiceIDs = MPEG2_DBGetServiceNextNode(pHandle->m_DBSHandle, lServiceIDs)) != 0)
				{
					if (MPEG2_DBGetServiceOutInfo(pHandle->m_DBSHandle, lServiceIDs, &lDBServInfo))
					{
						lOutTsIndex = MPEG2_DBGetServicTsIndex(pHandle->m_DBSHandle, lServiceIDs, FALSE);

						if (GLOBAL_CHECK_INDEX(lOutTsIndex, pHandle->m_Parameter.m_MaxOutTsNumber))//表示节目是要输出的
						{
							if (MPEG2_DB_SERVICE_MARK_IS_SCRAMBLE(lDBServInfo.m_ServiceMark))
							{
								lSCSCAIDs = 0;

								lTsIDs = MPEG2_DBGetTsIDs(pHandle->m_DBSHandle, FALSE, lOutTsIndex);
								MPEG2_DBGetTsRouteInfo(pHandle->m_DBSHandle, lTsIDs, FALSE, &lRouteInfo);

								if (lRouteInfo.m_ActiveMark == TRUE)
								{
									GLOBAL_TRACE(("Error!!!!!! Service IDs = 0x%.8X At Out Ts %d, Can not Be Scrambled In Route Mode!!!!!!!!\n", lServiceIDs, lOutTsIndex));
								}
								else
								{
									while ((lSCSCAIDs = MPEG2_DBGetSCSCANextNode(pHandle->m_DBSHandle, lSCSCAIDs, MPEG2_DB_CA_TYPE_SERVICE, lServiceIDs)) != 0)
									{
										if (MPEG2_DBGetSCSCAInfo(pHandle->m_DBSHandle, lSCSCAIDs, &lDBSCSCAInfo))
										{
											if (GLOBAL_CHECK_INDEX(lCurECMNum, plMultSCS->m_ECMMaxNum))
											{
												plSlot = &plMultSCS->m_pECMSlotArray[lCurECMNum];

												GLOBAL_ASSERT(plSlot->m_CryptoStreamID);
												/*---使用第0组------*/
												lDBSCSCAInfo.m_ScrambleData = 0;      //CAL_RandGetRange(0, GLOBAL_U8_MAX) & 0x7F;
												lECMStreamInfo.m_CwGroupIndex = (S32)lDBSCSCAInfo.m_ScrambleData;
											}
											MPEG2_DBSetSCSCAInfo(pHandle->m_DBSHandle, lSCSCAIDs, &lDBSCSCAInfo);//将修改了的加扰数据重新保存进系统，以供后面PIDmap时使用！！注意此时每次更新复用数据时SCS模块必须先与复用模块更新！！！！！！另外此修改不保存在参数文件当中！！！
										}
										break;
									}

								}

							}
						}
					}
					else
					{
						GLOBAL_ASSERT(0);
					}
				}

			}

		}
	}
}

void MULTL_SCSManagerDestroy(MULT_Handle *pHandle)
{
	MULT_SCS *plMultSCS;
	if (pHandle)
	{
		plMultSCS = &pHandle->m_MultSCS;
		GLOBAL_SAFEFREE(plMultSCS->m_pECMSlotArray);
		GLOBAL_SAFEFREE(plMultSCS->m_pEMMSlotArray);
	}
}

/*EOF*/
