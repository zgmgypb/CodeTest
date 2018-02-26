/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "multi_remote_control.h"
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_tsp.h"
#include "remote_control.h"

#ifdef SUPPORT_REMOTE_COM_CONTROL
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define MULTI_REMOTE_RECV_TIMEOUT			(1500)
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	MULT_Handle *m_pMultiHandle;
	HANDLE32	m_COMHandle;
	HANDLE32	m_SocketHandle;
	BOOL		m_ThreadMark;
	HANDLE32	m_ThreadHandle;
	U8			m_pSendBuf[REMOTE_MAX_CMD_BUFFER_SIZE];
	S32			m_CurSendSize;
	U8			m_pRecvBuf[REMOTE_MAX_CMD_BUFFER_SIZE];
	S32			m_CurRecvSize;
	S32			m_LastTick;
	S32			m_Timeout;

	HANDLE32	m_BUFFHandle;

	REMOTE_MSG	m_MSG;
	REMOTE_CMN_DEVICE_INFO	m_DeviceInfo;
	REMOTE_CMN_BITRATE	m_Bitrate;

	REMOTE_SFN_A_PARAM	m_SFNAParam;//调制器参数
	REMOTE_SFN_M_PARAM	m_SFNMParam;//激励器参数
	BOOL				m_ExtDPDMark;
	REMOTE_SFN_STATUS	m_SFNStatus;
	REMOTE_SFN_INDV_SIP	m_SFNSIP;
	REMOTE_TD_PARAM		m_TDParam;
	REMOTE_TD_STATUS	m_TDStatus;


}MULT_REMOTE_HANDLE;

/* Private Variables (static)-------------------------------------------------- */
static MULT_REMOTE_HANDLE *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
void MULT_RemoteInitiate(MULT_Handle *pHandle)
{
	MULT_REMOTE_HANDLE *plHandle;
	plHandle = (MULT_REMOTE_HANDLE*)GLOBAL_ZMALLOC(sizeof(MULT_REMOTE_HANDLE));
	if (plHandle)
	{
		plHandle->m_pMultiHandle = pHandle;
		s_pHandle = plHandle;
		MULT_RemoteStart();
	}
	GLOBAL_TRACE(("Complete!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"))
}

void MULTL_RemoteThreadFunction(void *pUserParam)
{
	MULT_REMOTE_HANDLE *plHandle = (MULT_REMOTE_HANDLE*)pUserParam;
	if (plHandle)
	{
		S32 lActUsed;
		S32 lErrorCode;
		U32 MSG_ID;
		MULT_Handle *plMultiHandle;

		plMultiHandle = plHandle->m_pMultiHandle;

		plHandle->m_ThreadMark = TRUE;
		while (plHandle->m_ThreadMark)
		{
			lActUsed = PFC_ComDeviceRead(plHandle->m_COMHandle, &plHandle->m_pRecvBuf[plHandle->m_CurRecvSize], sizeof(plHandle->m_pRecvBuf) - plHandle->m_CurRecvSize);
			if (lActUsed > 0)
			{
				plHandle->m_CurRecvSize += lActUsed;

				CAL_PrintDataBlockWithASCII("Recv", plHandle->m_pRecvBuf, plHandle->m_CurRecvSize, 4);

				while(plHandle->m_CurRecvSize > 0)
				{
					lActUsed = 0;
					if (REMOTE_ProtocolParser(plHandle->m_pRecvBuf, plHandle->m_CurRecvSize, &plHandle->m_MSG, &lActUsed))
					{
						MSG_ID = plHandle->m_MSG.m_ID;

						GLOBAL_TRACE(("Got MsgID = %.4X, MsgLen = %d\n", plHandle->m_MSG.m_ID, plHandle->m_MSG.m_Len));
						/*处理消息*/
						lErrorCode = REMOTE_MSG_ERR_CODE_OK;
						if (plHandle->m_MSG.m_ID == REMOTE_TAG_CMN_GET_DEVICE_INFO_R)
						{

							if (REMOTE_DecodeMsgDeviceInfo(&plHandle->m_MSG, &plHandle->m_DeviceInfo))
							{

								GLOBAL_ZEROMEM(&plHandle->m_DeviceInfo, sizeof(plHandle->m_DeviceInfo));
								plHandle->m_DeviceInfo.m_DeviceType = (MULT_DEVICE_TYPE << 8) | MULT_DEVICE_SUB_TYPE;
								GLOBAL_MEMCPY(plHandle->m_DeviceInfo.m_pDeviceName, plMultiHandle->m_Information.m_pWEBENG, sizeof(plHandle->m_DeviceInfo.m_pDeviceName) - 1);
								GLOBAL_MEMCPY(plHandle->m_DeviceInfo.m_pDeviceTypeName, plMultiHandle->m_Information.m_pModelName, sizeof(plHandle->m_DeviceInfo.m_pDeviceTypeName) - 1);
								GLOBAL_MEMCPY(plHandle->m_DeviceInfo.m_pFPGARelease, plMultiHandle->m_Information.m_pFPGARelease, sizeof(plHandle->m_DeviceInfo.m_pFPGARelease) - 1);
								GLOBAL_MEMCPY(plHandle->m_DeviceInfo.m_pHardVersion, plMultiHandle->m_Information.m_pHardVersion, sizeof(plHandle->m_DeviceInfo.m_pHardVersion) - 1);
								GLOBAL_MEMCPY(plHandle->m_DeviceInfo.m_pSN, plMultiHandle->m_Information.m_pSNString, sizeof(plHandle->m_DeviceInfo.m_pSN) - 1);
								GLOBAL_MEMCPY(plHandle->m_DeviceInfo.m_pSoftRelease, plMultiHandle->m_Information.m_pSoftRelease, sizeof(plHandle->m_DeviceInfo.m_pSoftRelease) - 1);
								GLOBAL_MEMCPY(plHandle->m_DeviceInfo.m_pSoftVersion, plMultiHandle->m_Information.m_pSoftVersion, sizeof(plHandle->m_DeviceInfo.m_pSoftVersion) - 1);
								REMOTE_EncodeMsgDeviceInfo(&plHandle->m_MSG, &plHandle->m_DeviceInfo, FALSE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}

						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_CMN_GET_BITRATE_R)
						{
							if (REMOTE_DecodeMsgBitrate(&plHandle->m_MSG, &plHandle->m_Bitrate))
							{
								if (plHandle->m_Bitrate.m_bInput)
								{
									plHandle->m_Bitrate.m_Bitrate = plMultiHandle->m_Monitor.m_TotalInBitrate;
								}
								else
								{
									plHandle->m_Bitrate.m_Bitrate = plMultiHandle->m_Monitor.m_TotalOutBitrate;
								}
								REMOTE_EncodeMsgBitrate(&plHandle->m_MSG, &plHandle->m_Bitrate, FALSE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_SFN_A_PARAM_R)
						{
							if (REMOTE_DecodeMsgSFNParam(&plHandle->m_MSG, &plHandle->m_SFNAParam))
							{
#ifdef SUPPORT_SFN_ADAPTER
								{
									MULT_SFNA_Param lSFNParam;
									MULT_SFNAProcSFNParam(&lSFNParam, TRUE);

									plHandle->m_SFNAParam.m_10MHz = lSFNParam.m_bUse10MClkSynSrc;
									plHandle->m_SFNAParam.m_1PPS = (lSFNParam.m_bUseEx1PPS?1:0);
									plHandle->m_SFNAParam.m_ASIMode = lSFNParam.m_SFNASIMode - 1;
									plHandle->m_SFNAParam.m_bSATSFN = lSFNParam.m_bEnableSatSFN;
									GLOBAL_MEMCPY(&plHandle->m_SFNAParam.m_DTMBParam, &lSFNParam.m_DTMBParam, sizeof(TD_DTMBParameter));
									plHandle->m_SFNAParam.m_SATNullPID = lSFNParam.m_SatSFNNullPacketPID;
									plHandle->m_SFNAParam.m_MaxDelay100ns = lSFNParam.m_SFNMaxDelay100NS;
									GLOBAL_TRACE(("Max Delay = %d\n", lSFNParam.m_SFNMaxDelay100NS));
								}
#endif
								REMOTE_EncodeMsgSFNParam(&plHandle->m_MSG, &plHandle->m_SFNAParam, FALSE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_SFN_A_PARAM)
						{
							if (REMOTE_DecodeMsgSFNParam(&plHandle->m_MSG, &plHandle->m_SFNAParam))
							{
#ifdef SUPPORT_SFN_ADAPTER
								{
									MULT_SFNA_Param lSFNParam;

									MULT_SFNAProcSFNParam(&lSFNParam, TRUE);

									lSFNParam.m_bEnableSatSFN = plHandle->m_SFNAParam.m_bSATSFN;
									lSFNParam.m_bUse10MClkSynSrc = plHandle->m_SFNAParam.m_10MHz;
									lSFNParam.m_bUseEx1PPS = plHandle->m_SFNAParam.m_1PPS;
									GLOBAL_MEMCPY(&lSFNParam.m_DTMBParam, &plHandle->m_SFNAParam.m_DTMBParam, sizeof(TD_DTMBParameter));
									lSFNParam.m_SFNASIMode = plHandle->m_SFNAParam.m_ASIMode + 1;
									lSFNParam.m_SatSFNNullPacketPID = plHandle->m_SFNAParam.m_SATNullPID;
									lSFNParam.m_SFNMaxDelay100NS = plHandle->m_SFNAParam.m_MaxDelay100ns;


									MULT_SFNAProcSFNParam(&lSFNParam, FALSE);
								}

								{
									MPEG2_DBTsRouteInfo lTsRouteInfo;
									/*设置直通模式*/
									GLOBAL_ZEROMEM(&lTsRouteInfo, sizeof(MPEG2_DBTsRouteInfo));

									lTsRouteInfo.m_TsIndex = 0;
									lTsRouteInfo.m_ActiveMark = TRUE;

									MPEG2_DBSetTsRouteInfo(plMultiHandle->m_DBSHandle, MPEG2_DBGetTsIDs(plMultiHandle->m_DBSHandle, FALSE, 0), &lTsRouteInfo);
								}
#endif

								REMOTE_EncodeMsgReply(&plHandle->m_MSG, REMOTE_MSG_ERR_CODE_OK, plHandle->m_MSG.m_ID);

								MULTL_SaveParameterXML(plMultiHandle); 

								MULTL_SaveParamterToStorage(plMultiHandle);

								MULTL_SetSaveMark(plMultiHandle, TRUE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_SFN_M_PARAM_R)
						{
							if (REMOTE_DecodeMsgSFNMParam(&plHandle->m_MSG, &plHandle->m_SFNMParam))
							{
#ifdef SUPPORT_SFN_MODULATOR
								{
									MULT_SFN_Param lSFNParam;
									MULT_ChannelNode *plChnNode;
									MULT_SubChannelNode *plSubNode;
									TD_DTMBParameter lDTMBParam;

									MULT_SFNProcSFNParam(&lSFNParam, TRUE);
									plChnNode = &plMultiHandle->m_Parameter.m_pOutChannel[0];
									plSubNode = &plChnNode->m_pSubChannelNode[0];

									/*RF部分参数*/
									plHandle->m_SFNMParam.m_FreqHz = plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq;
									plHandle->m_SFNMParam.m_FreqAdj = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_FreqAdj;
									plHandle->m_SFNMParam.m_bSpecInv = plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv;
									plHandle->m_SFNMParam.m_bToneMode = !plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation;
									plHandle->m_SFNMParam.m_Gain = plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel;
									plHandle->m_SFNMParam.m_bRF = plSubNode->m_ActiveMark;


									/*单频网SIP部分参数*/
									plHandle->m_SFNMParam.m_bUseSFN = lSFNParam.m_bUseSFN;
									plHandle->m_SFNMParam.m_SFNAddDelay100ns = lSFNParam.m_SFNAddDelay100ns;
									plHandle->m_SFNMParam.m_SFNAddrID = lSFNParam.m_SFNAddrID;
									plHandle->m_SFNMParam.m_bUseIndvSIP = lSFNParam.m_bUseIndvSIP;
									plHandle->m_SFNMParam.m_bUseCMNSIP = lSFNParam.m_bUseCMNSIP;
									plHandle->m_SFNMParam.m_bDeleteSIP = lSFNParam.m_bDeleteSIP;


									/*单频网激励器部分参数*/
									lDTMBParam.m_bDoublePilot = plSubNode->m_SubInfo.m_SubModulatorInfo.m_DoublePilot;
									if (plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode >= GS_MODULATOR_GUARD_INTERVAL_PN_420C && plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode <= GS_MODULATOR_GUARD_INTERVAL_PN_945C)
									{
										lDTMBParam.m_bPNPhaseVariable = FALSE;
									}
									else
									{
										lDTMBParam.m_bPNPhaseVariable = TRUE;
									}

									switch (plSubNode->m_SubInfo.m_SubModulatorInfo.m_CarrierMode)
									{
									case GS_MODULATOR_CARRIER_MODE_1:
										lDTMBParam.m_CarrierMode = TD_DTMB_CARRIER_MODE_SINGLE;
										break;
									case GS_MODULATOR_CARRIER_MODE_3780:
										lDTMBParam.m_CarrierMode = TD_DTMB_CARRIER_MODE_3780;
										break;
									}

									switch (plSubNode->m_SubInfo.m_SubModulatorInfo.m_CodeRate)
									{
									case GS_MODULATOR_CODE_RATE_0_4:
										lDTMBParam.m_CodeRate = TD_DTMB_CODE_RATE_04;
										break;
									case GS_MODULATOR_CODE_RATE_0_6:
										lDTMBParam.m_CodeRate = TD_DTMB_CODE_RATE_06;
										break;
									case GS_MODULATOR_CODE_RATE_0_8:
										lDTMBParam.m_CodeRate = TD_DTMB_CODE_RATE_08;
										break;
									}

									switch (plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode)
									{
									case GS_MODULATOR_GUARD_INTERVAL_PN_420C:
										lDTMBParam.m_PNMode = TD_DTMB_PN_MODE_420C;
										break;
									case GS_MODULATOR_GUARD_INTERVAL_PN_420F:
										lDTMBParam.m_PNMode = TD_DTMB_PN_MODE_420;
										break;
									case GS_MODULATOR_GUARD_INTERVAL_PN_595:
										lDTMBParam.m_PNMode = TD_DTMB_PN_MODE_595;
										break;
									case GS_MODULATOR_GUARD_INTERVAL_PN_945C:
										lDTMBParam.m_PNMode = TD_DTMB_PN_MODE_945C;
										break;
									case GS_MODULATOR_GUARD_INTERVAL_PN_945F:
										lDTMBParam.m_PNMode = TD_DTMB_PN_MODE_945;
										break;
									}


									switch (plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode)
									{
									case GS_MODULATOR_QAM_4NR:
										lDTMBParam.m_QAMMode = TD_DTMB_QAM_MAP_4QAMNR;
										break;
									case GS_MODULATOR_QAM_4:
										lDTMBParam.m_QAMMode = TD_DTMB_QAM_MAP_4QAM;
										break;
									case GS_MODULATOR_QAM_16:
										lDTMBParam.m_QAMMode = TD_DTMB_QAM_MAP_16QAM;
										break;
									case GS_MODULATOR_QAM_32:
										lDTMBParam.m_QAMMode = TD_DTMB_QAM_MAP_32QAM;
										break;
									case GS_MODULATOR_QAM_64:
										lDTMBParam.m_QAMMode = TD_DTMB_QAM_MAP_64QAM;
										break;
									}

									switch (plSubNode->m_SubInfo.m_SubModulatorInfo.m_TimeInterleave)
									{
									case GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240:
										lDTMBParam.m_TIMode = TD_DTMB_TI_MODE_240;
										break;
									case GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_720:
										lDTMBParam.m_TIMode = TD_DTMB_TI_MODE_720;
										break;
									}

									GLOBAL_MEMCPY(&plHandle->m_SFNMParam.m_DTMBParam, &lDTMBParam, sizeof(TD_DTMBParameter));
									plHandle->m_SFNMParam.m_ASIMode = lSFNParam.m_SFNASIMode - 1;
									plHandle->m_SFNMParam.m_10MHz = lSFNParam.m_bUse10MClkSynSrc;
									plHandle->m_SFNMParam.m_1PPS = (lSFNParam.m_bUseEx1PPS?1:0);


									/*卫星SFN参数*/
									plHandle->m_SFNMParam.m_bSATSFN = lSFNParam.m_bEnableSatSFN;
									plHandle->m_SFNMParam.m_SATNullPID = lSFNParam.m_SatSFNNullPacketPID;
									plHandle->m_SFNMParam.m_bSatSFNSIPCRC32Check = lSFNParam.m_bSatSFNSIPCRC32Check;
								}
#endif
								REMOTE_EncodeMsgSFNMParam(&plHandle->m_MSG, &plHandle->m_SFNMParam, FALSE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_SFN_M_PARAM)
						{
							if (REMOTE_DecodeMsgSFNMParam(&plHandle->m_MSG, &plHandle->m_SFNMParam))
							{
#ifdef SUPPORT_SFN_MODULATOR
								{
									MULT_SFN_Param lSFNParam;
									MULT_ChannelNode *plChnNode;
									MULT_SubChannelNode *plSubNode;
									TD_DTMBParameter lDTMBParam;

									MULT_SFNProcSFNParam(&lSFNParam, TRUE);
									plChnNode = &plMultiHandle->m_Parameter.m_pOutChannel[0];
									plSubNode = &plChnNode->m_pSubChannelNode[0];


									/*RF部分参数*/
									plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq = plHandle->m_SFNMParam.m_FreqHz;
									plChnNode->m_ChannelInfo.m_ModulatorInfo.m_FreqAdj = plHandle->m_SFNMParam.m_FreqAdj;
									plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv = plHandle->m_SFNMParam.m_bSpecInv;
									plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation = !plHandle->m_SFNMParam.m_bToneMode;
									plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel = plHandle->m_SFNMParam.m_Gain;
									plSubNode->m_ActiveMark = plHandle->m_SFNMParam.m_bRF;


									/*单频网SIP部分参数*/
									lSFNParam.m_bUseSFN = plHandle->m_SFNMParam.m_bUseSFN;
									lSFNParam.m_SFNAddDelay100ns = plHandle->m_SFNMParam.m_SFNAddDelay100ns;
									lSFNParam.m_SFNAddrID = plHandle->m_SFNMParam.m_SFNAddrID;
									lSFNParam.m_bUseIndvSIP = plHandle->m_SFNMParam.m_bUseIndvSIP;
									lSFNParam.m_bUseCMNSIP = plHandle->m_SFNMParam.m_bUseCMNSIP;
									lSFNParam.m_bDeleteSIP = plHandle->m_SFNMParam.m_bDeleteSIP;


									/*单频网激励器部分参数*/
									GLOBAL_MEMCPY(&lDTMBParam, &plHandle->m_SFNMParam.m_DTMBParam, sizeof(TD_DTMBParameter));
									plSubNode->m_SubInfo.m_SubModulatorInfo.m_DoublePilot = lDTMBParam.m_bDoublePilot;

									switch (lDTMBParam.m_CarrierMode)
									{
									case TD_DTMB_CARRIER_MODE_SINGLE:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_CarrierMode = GS_MODULATOR_CARRIER_MODE_1;
										break;
									case TD_DTMB_CARRIER_MODE_3780:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_CarrierMode = GS_MODULATOR_CARRIER_MODE_3780;
										break;
									}

									switch (lDTMBParam.m_CodeRate)
									{
									case TD_DTMB_CODE_RATE_04:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_CodeRate = GS_MODULATOR_CODE_RATE_0_4;
										break;
									case TD_DTMB_CODE_RATE_06:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_CodeRate = GS_MODULATOR_CODE_RATE_0_6;
										break;
									case TD_DTMB_CODE_RATE_08:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_CodeRate = GS_MODULATOR_CODE_RATE_0_8;
										break;
									}

									switch (lDTMBParam.m_PNMode)
									{
									case TD_DTMB_PN_MODE_420:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_420F;
										break;
									case TD_DTMB_PN_MODE_420C:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_420C;
										break;
									case TD_DTMB_PN_MODE_595:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_595;
										break;
									case TD_DTMB_PN_MODE_945:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_945F;
										break;
									case TD_DTMB_PN_MODE_945C:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_945C;
										break;
									}


									switch (lDTMBParam.m_QAMMode)
									{
									case TD_DTMB_QAM_MAP_4QAMNR:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode = GS_MODULATOR_QAM_4NR;
										break;
									case TD_DTMB_QAM_MAP_4QAM:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode = GS_MODULATOR_QAM_4;
										break;
									case TD_DTMB_QAM_MAP_16QAM:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode = GS_MODULATOR_QAM_16;
										break;
									case TD_DTMB_QAM_MAP_32QAM:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode = GS_MODULATOR_QAM_32;
										break;
									case TD_DTMB_QAM_MAP_64QAM:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode = GS_MODULATOR_QAM_64;
										break;
									}

									switch (lDTMBParam.m_TIMode)
									{
									case TD_DTMB_TI_MODE_240:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_TimeInterleave = GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240;
										break;
									case TD_DTMB_TI_MODE_720:
										plSubNode->m_SubInfo.m_SubModulatorInfo.m_TimeInterleave = GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_720;
										break;
									}

									lSFNParam.m_SFNASIMode = plHandle->m_SFNMParam.m_ASIMode + 1;
									lSFNParam.m_bUse10MClkSynSrc = plHandle->m_SFNMParam.m_10MHz;
									lSFNParam.m_bUseEx1PPS = plHandle->m_SFNMParam.m_1PPS;


									/*卫星SFN参数*/
									lSFNParam.m_bEnableSatSFN = plHandle->m_SFNMParam.m_bSATSFN;
									lSFNParam.m_SatSFNNullPacketPID = plHandle->m_SFNMParam.m_SATNullPID;
									lSFNParam.m_bSatSFNSIPCRC32Check = plHandle->m_SFNMParam.m_bSatSFNSIPCRC32Check;


									MULT_SFNProcSFNParam(&lSFNParam, FALSE);
								}

								{
									MPEG2_DBTsRouteInfo lTsRouteInfo;
									/*设置直通模式*/
									GLOBAL_ZEROMEM(&lTsRouteInfo, sizeof(MPEG2_DBTsRouteInfo));

									lTsRouteInfo.m_TsIndex = 0;
									lTsRouteInfo.m_ActiveMark = TRUE;

									MPEG2_DBSetTsRouteInfo(plMultiHandle->m_DBSHandle, MPEG2_DBGetTsIDs(plMultiHandle->m_DBSHandle, FALSE, 0), &lTsRouteInfo);
								}
#endif

								REMOTE_EncodeMsgReply(&plHandle->m_MSG, REMOTE_MSG_ERR_CODE_OK, plHandle->m_MSG.m_ID);

								MULTL_SaveParameterXML(plMultiHandle); 

								MULTL_SaveParamterToStorage(plMultiHandle);

								MULTL_SetSaveMark(plMultiHandle, TRUE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_EXT_DPD_PARAM_R)
						{
							if (REMOTE_DecodeMsgExtDPDParam(&plHandle->m_MSG, &plHandle->m_ExtDPDMark))
							{
								MULT_ChannelNode *plChnNode;

								plChnNode = &plMultiHandle->m_Parameter.m_pOutChannel[0];

								plHandle->m_ExtDPDMark = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_DPDMark;

								GLOBAL_TRACE(("Current Ext DPD Mark = %d\n", plHandle->m_ExtDPDMark));

								REMOTE_EncodeMsgExtDPDParam(&plHandle->m_MSG, plHandle->m_ExtDPDMark, FALSE);
							}

						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_EXT_DPD_PARAM)
						{
							if (REMOTE_DecodeMsgExtDPDParam(&plHandle->m_MSG, &plHandle->m_ExtDPDMark))
							{
								MULT_ChannelNode *plChnNode;

								plChnNode = &plMultiHandle->m_Parameter.m_pOutChannel[0];

								plChnNode->m_ChannelInfo.m_ModulatorInfo.m_DPDMark = plHandle->m_ExtDPDMark;

								GLOBAL_TRACE(("Set Ext DPD Mark = %d\n", plHandle->m_ExtDPDMark));

								REMOTE_EncodeMsgReply(&plHandle->m_MSG, REMOTE_MSG_ERR_CODE_OK, plHandle->m_MSG.m_ID);

								MULTL_SaveParameterXML(plMultiHandle); 

								MULTL_SaveParamterToStorage(plMultiHandle);

								MULTL_SetSaveMark(plMultiHandle, TRUE);
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_SFN_SIP_R)
						{
							S32 lInd;
							if (REMOTE_DecodeMsgSFNSIP(&plHandle->m_MSG, &lInd, &plHandle->m_SFNSIP))
							{
#ifdef SUPPORT_SFN_ADAPTER
								MULT_SFNA_Param lSFNParam;
								MULT_SFNAProcSFNParam(&lSFNParam, TRUE);

								GLOBAL_TRACE(("Ind = %d\n", lInd));

								if (GLOBAL_CHECK_INDEX(lInd, SFN_MAX_INDV_NUM))
								{
									plHandle->m_SFNSIP.m_ActiveMark = lSFNParam.m_pINDVArrap[lInd].m_ActiveMark;
									plHandle->m_SFNSIP.m_Info.m_SFNAddr = lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNAddr;
									plHandle->m_SFNSIP.m_Info.m_SFNSIPIndvFreqOffsetHz = lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNSIPIndvFreqOffsetHz;
									plHandle->m_SFNSIP.m_Info.m_SFNSIPIndviDelay = lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNSIPIndviDelay;
									plHandle->m_SFNSIP.m_Info.m_SFNSIPIndvPower = lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNSIPIndvPower;
									plHandle->m_SFNSIP.m_Info.m_SFNSIPIndvPowerMark = lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNSIPIndvPowerMark;
								}
#endif
								REMOTE_EncodeMsgSFNSIP(&plHandle->m_MSG, lInd, &plHandle->m_SFNSIP, FALSE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_SFN_SIP)
						{
							S32 lInd;
							if (REMOTE_DecodeMsgSFNSIP(&plHandle->m_MSG, &lInd, &plHandle->m_SFNSIP))
							{
#ifdef SUPPORT_SFN_ADAPTER
								MULT_SFNA_Param lSFNParam;
								MULT_SFNAProcSFNParam(&lSFNParam, TRUE);
								if (GLOBAL_CHECK_INDEX(lInd, SFN_MAX_INDV_NUM))
								{
									lSFNParam.m_pINDVArrap[lInd].m_ActiveMark = plHandle->m_SFNSIP.m_ActiveMark;
									lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNAddr = plHandle->m_SFNSIP.m_Info.m_SFNAddr;
									lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNSIPIndvFreqOffsetHz = plHandle->m_SFNSIP.m_Info.m_SFNSIPIndvFreqOffsetHz;
									lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNSIPIndviDelay = plHandle->m_SFNSIP.m_Info.m_SFNSIPIndviDelay;
									lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNSIPIndvPower = plHandle->m_SFNSIP.m_Info.m_SFNSIPIndvPower;
									lSFNParam.m_pINDVArrap[lInd].m_Info.m_SFNSIPIndvPowerMark = plHandle->m_SFNSIP.m_Info.m_SFNSIPIndvPowerMark;
								}
								MULT_SFNAProcSFNParam(&lSFNParam, FALSE);
#endif
								REMOTE_EncodeMsgReply(&plHandle->m_MSG, REMOTE_MSG_ERR_CODE_OK, plHandle->m_MSG.m_ID);


								MULTL_SaveParameterXML(plMultiHandle); 

								MULTL_SaveParamterToStorage(plMultiHandle);

								MULTL_SetSaveMark(plMultiHandle, TRUE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_SFN_STATUS_R)
						{
							if (REMOTE_DecodeMsgSFNStatus(&plHandle->m_MSG, &plHandle->m_SFNStatus))
							{
#ifdef SUPPORT_SFN_ADAPTER
								{
									MULT_SFNA_Status lSFNStatus;
									MULT_SFNAGetSFNStatus(&lSFNStatus);

									plHandle->m_SFNStatus.m_ASI1 = lSFNStatus.m_bTS0Status;
									plHandle->m_SFNStatus.m_ASI2 = lSFNStatus.m_bTS1Status;
									plHandle->m_SFNStatus.m_bExt10MHz = lSFNStatus.m_bExt10MStatus;
									plHandle->m_SFNStatus.m_bExt1PPS = lSFNStatus.m_bExt1PPSStatus;
									plHandle->m_SFNStatus.m_bInt10MHz = lSFNStatus.m_bInt10MStatus;
									plHandle->m_SFNStatus.m_bInt1PPS = lSFNStatus.m_bInt1PPSStatus;
								}
#endif

#ifdef SUPPORT_SFN_MODULATOR
								{
									MULT_SFN_Status lSFNStatus;
									MULT_SFNGetSFNStatus(&lSFNStatus);

									plHandle->m_SFNStatus.m_ASI1 = lSFNStatus.m_bTS0Status;
									plHandle->m_SFNStatus.m_ASI2 = lSFNStatus.m_bTS1Status;
									plHandle->m_SFNStatus.m_bExt10MHz = lSFNStatus.m_bExt10MStatus;
									plHandle->m_SFNStatus.m_bExt1PPS = lSFNStatus.m_bExt1PPSStatus;
									plHandle->m_SFNStatus.m_bInt10MHz = lSFNStatus.m_bInt10MStatus;
									plHandle->m_SFNStatus.m_bInt1PPS = lSFNStatus.m_bInt1PPSStatus;
								}
#endif

#ifdef SUPPORT_CLK_ADJ_MODULE
								{
									plHandle->m_SFNStatus.m_b10MSync = MULT_CLKGet10MLockStatus();
								}
#endif						
								REMOTE_EncodeMsgSFNStatus(&plHandle->m_MSG, &plHandle->m_SFNStatus, FALSE);
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_TD_PARAM)//TD设置参数
						{
							if (REMOTE_DecodeMsgTDParam(&plHandle->m_MSG, &plHandle->m_TDParam))
							{
								MULT_ChannelNode *plChnNode = NULL;
								MULT_SubChannelNode *plSubNode = NULL;
#ifdef GQ3768
								plChnNode = &plMultiHandle->m_Parameter.m_pInChannel[1];
#endif
								if (plChnNode)
								{
									if (GLOBAL_CHECK_INDEX(plHandle->m_TDParam.m_Slot, plChnNode->m_SubChannelNumber))
									{
										plSubNode = &plChnNode->m_pSubChannelNode[plHandle->m_TDParam.m_Slot];
										plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq = plHandle->m_TDParam.m_FreqHz / 1000;
										plSubNode->m_SubInfo.m_SubTUNERInfo.m_SymbolRate = plHandle->m_TDParam.m_SymbolRate / 1000;
										plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv = plHandle->m_TDParam.m_SpecInv;

										MULTL_SaveParameterXML(plMultiHandle); 

										MULTL_SaveParamterToStorage(plMultiHandle);

										MULTL_SetSaveMark(plMultiHandle, TRUE);
									}
									else
									{
										lErrorCode = REMOTE_MSG_ERR_CODE_MSG_UNSUPPORTED_PARAM;
									}
								}
								else
								{
									lErrorCode = REMOTE_MSG_ERR_CODE_MSG_UNSUPPORTED_PARAM;
								}
							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_TD_PARAM_R)//TD获取参数
						{
							if (REMOTE_DecodeMsgTDParam(&plHandle->m_MSG, &plHandle->m_TDParam))
							{
								TD_DTMBParameter lDTMBParam;
								MULT_ChannelNode *plChnNode = NULL;
								MULT_SubChannelNode *plSubNode = NULL;
#ifdef GQ3768
								plChnNode = &plMultiHandle->m_Parameter.m_pInChannel[1];
#endif
								if (plChnNode)
								{
									if (GLOBAL_CHECK_INDEX(plHandle->m_TDParam.m_Slot, plChnNode->m_SubChannelNumber))
									{
										plSubNode = &plChnNode->m_pSubChannelNode[plHandle->m_TDParam.m_Slot];
										plHandle->m_TDParam.m_FreqHz = plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq * 1000;
										plHandle->m_TDParam.m_SymbolRate = plSubNode->m_SubInfo.m_SubTUNERInfo.m_SymbolRate * 1000;
										plHandle->m_TDParam.m_SpecInv = plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv;

										REMOTE_EncodeMsgTDParam(&plHandle->m_MSG, &plHandle->m_TDParam, FALSE);
									}
									else
									{
										lErrorCode = REMOTE_MSG_ERR_CODE_MSG_UNSUPPORTED_PARAM;
									}
								}
								else
								{
									lErrorCode = REMOTE_MSG_ERR_CODE_MSG_UNSUPPORTED_PARAM;
								}

							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else if (plHandle->m_MSG.m_ID == REMOTE_TAG_TD_STATUS_R)//TD获取状态
						{
							if (REMOTE_DecodeMsgTDStatus(&plHandle->m_MSG, &plHandle->m_TDStatus))
							{
								TD_DTMBParameter lDTMBParam;
								MULT_ChannelNode *plChnNode = NULL;
								MULT_SubChannelNode *plSubNode = NULL;
#ifdef GQ3768
								plChnNode = &plMultiHandle->m_Parameter.m_pInChannel[1];
#endif
								if (plChnNode)
								{
									if (GLOBAL_CHECK_INDEX(plHandle->m_TDStatus.m_Slot, plChnNode->m_SubChannelNumber))
									{
										BOOL bLock;
										F64 lPower, lSN, lBER;
										plSubNode = &plChnNode->m_pSubChannelNode[plHandle->m_TDParam.m_Slot];
										HWL_TDGetSignalFloat(plHandle->m_TDParam.m_Slot, &bLock, &lPower, &lSN, &lBER);

										plHandle->m_TDStatus.m_bLock = bLock;
										plHandle->m_TDStatus.m_Power = (S32)lPower;
										plHandle->m_TDStatus.m_SN = (S32)lSN;

										REMOTE_EncodeMsgTDStatus(&plHandle->m_MSG, &plHandle->m_TDStatus, FALSE);
									}
									else
									{
										lErrorCode = REMOTE_MSG_ERR_CODE_MSG_UNSUPPORTED_PARAM;
									}
								}
								else
								{
									lErrorCode = REMOTE_MSG_ERR_CODE_MSG_UNSUPPORTED_PARAM;
								}

							}
							else
							{
								lErrorCode = REMOTE_MSG_ERR_CODE_MSG_DECODE_ERROR;
							}
						}
						else
						{
							lErrorCode = REMOTE_MSG_ERR_CODE_MSG_UNSUPPORTED_ID;
						}


						if (lErrorCode != REMOTE_MSG_ERR_CODE_OK)
						{
							GLOBAL_TRACE(("MSG Process Error! Code = %d\n", lErrorCode));
							REMOTE_EncodeMsgReply(&plHandle->m_MSG, lErrorCode, plHandle->m_MSG.m_ID);
						}

						plHandle->m_CurSendSize = REMOTE_ProtocolPacker(plHandle->m_pSendBuf, sizeof(plHandle->m_pSendBuf), &plHandle->m_MSG);
						CAL_PrintDataBlockWithASCII("Send", plHandle->m_pSendBuf, plHandle->m_CurSendSize, 4);
						PFC_ComDeviceWrite(plHandle->m_COMHandle, plHandle->m_pSendBuf, plHandle->m_CurSendSize);

						/*应答之后再应用参数*/
						if (MSG_ID == REMOTE_TAG_SFN_M_PARAM)
						{
#ifdef SUPPORT_SFN_MODULATOR
							MULT_SFNApplyByQAMModule();
#endif
						}

						if (MSG_ID == REMOTE_TAG_TD_PARAM)
						{
							/*应用TD 参数！*/
							MULTL_ApplyTunerParameter(plMultiHandle, 1);
						}

						if (MSG_ID == REMOTE_TAG_SFN_A_PARAM)
						{
#ifdef SUPPORT_SFN_ADAPTER 
							MULT_SFNAApplyParameter();
#endif
						}

						if (MSG_ID == REMOTE_TAG_SFN_SIP)
						{
#ifdef SUPPORT_SFN_ADAPTER
							MULT_SFNAApplyParameter();
#endif
						}
					}

					if (lActUsed > 0)
					{
						plHandle->m_CurRecvSize -= lActUsed;

						if (plHandle->m_CurRecvSize > 0)
						{
							GLOBAL_MEMMOVE(plHandle->m_pRecvBuf, &plHandle->m_pRecvBuf[lActUsed], plHandle->m_CurRecvSize);
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				plHandle->m_CurRecvSize = 0;
			}
		}
	}

}

void MULT_RemoteStart(void)
{
	MULT_REMOTE_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		plHandle->m_COMHandle = PFC_ComDeviceOpen(2, TRUE);
		if (plHandle->m_COMHandle)
		{
			PFC_ComDeviceSetState(plHandle->m_COMHandle, 9600, 8, 'N', 1);
			PFC_ComDeviceSetOption(plHandle->m_COMHandle, sizeof(plHandle->m_pRecvBuf), sizeof(plHandle->m_pSendBuf), 200, 200);
			plHandle->m_ThreadHandle = PFC_TaskCreate(__FUNCTION__, 40 * 1024, MULTL_RemoteThreadFunction, 0, plHandle);
		}
		else
		{
			GLOBAL_TRACE(("Open COM2 Failed!!!\n"));
		}
	}
}

void MULT_RemoteStop(void)
{
	MULT_REMOTE_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		if (plHandle->m_ThreadMark || plHandle->m_ThreadHandle)
		{
			plHandle->m_ThreadMark = FALSE;
			PFC_TaskWait(plHandle->m_ThreadHandle, 2000);
			plHandle->m_ThreadHandle = NULL;

			if (plHandle->m_COMHandle)
			{
				PFC_ComDeviceClose(plHandle->m_COMHandle);
				plHandle->m_COMHandle = NULL;
			}
		}
	}
}


void MULT_RemoteTerminate(void)
{
	MULT_REMOTE_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		MULT_RemoteStop();
	}
}

#endif

/*EOF*/
