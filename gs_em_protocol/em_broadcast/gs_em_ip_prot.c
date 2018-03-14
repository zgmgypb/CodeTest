#include "gs_em_ip_prot.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

GS_DEBUG_SET_LEVEL(GS_DBG_LVL_DBG); 

/* 十进制转 BCD 码，表示 0 - 99 */
static GS_U8 GS_EM_IP_PROT_CharToBCD(GS_U8 Dec)
{
	GS_U8 lBcd = 0;

	lBcd = ((Dec / 10) * 16 + Dec % 10) & 0xFF;

	return lBcd;
}

/* BCD 码转十进制，表示 0 - 99 */
static GS_U8 GS_EM_IP_PROT_BCDToChar(GS_U8 Bcd)
{
	GS_U8 lDec = 0;

	lDec = (((Bcd >> 4) & 0x0F) * 10) | (Bcd & 0x0F);

	return lDec;
}

static GS_VOID GS_EM_IP_PROT_HeaderPack(GS_EM_IP_PROT_HEADER_S *pHeader, GS_U32 SessionId, GS_U8 DataType, GS_U16 DataLen)
{
	pHeader->m_Tag = GS_EM_IP_PROT_TAG;
	pHeader->m_Ver = GS_EM_IP_PROT_VER;
	pHeader->m_SessionId = SessionId;
	pHeader->m_DataType = DataType;
	pHeader->m_SignTag = 0;
	pHeader->m_DataLen = DataLen + 12; /* 数据头长度为 12 */
}

/* 返回长度 */
static GS_S32 GS_EM_IP_PROT_ContentPack(GS_EM_IP_PROT_CONTENT_S *pContent, GS_VOID *pCmdInfo, GS_U8 DataType, GS_U8 CmdType)
{
	GS_S32 lCount = 0;
	
	pContent->m_DataSrcCompBCDCode.m_ResTypeCode = 0;
	pContent->m_DataSrcCompBCDCode.m_ResSubTypeCode = 0; 
	memset(pContent->m_DataSrcCompBCDCode.m_pZoneCode, 0, 6); 
	pContent->m_DataSrcCompBCDCode.m_ExCode = 0;
	lCount += 9;
	pContent->m_DataDstNum = 0;
	lCount += 2;
	pContent->m_CmdType = CmdType; /* 业务类型 */
	lCount += 1;

	if (GS_EM_IP_PROT_DATA_TYPE_RESPONSE == DataType) { /* 应答封包 */
		GS_EM_IP_PROT_RESPONSE_S *plResponseInfo = (GS_EM_IP_PROT_RESPONSE_S *)pCmdInfo;

		pContent->m_CmdLen = 1 + 2 + plResponseInfo->m_DescLen; /* 结果代码[1] + 结果描述长度[2] + 结果描述[结果描述长度] */
		lCount += 2;
		memcpy(&pContent->m_CmdContent.m_ResponseInfo, plResponseInfo, sizeof(GS_EM_IP_PROT_RESPONSE_S));
	}
	else {
		switch (CmdType)
		{
		case IP_PROT_CMD_DEV_HEARTBEAT:
			{
				GS_EM_IP_PROT_DEV_HEARTBEAT_S *plDevHeartBeatInfo = (GS_EM_IP_PROT_DEV_HEARTBEAT_S *)pCmdInfo;

				pContent->m_CmdLen = 1 + 1 + 1 + plDevHeartBeatInfo->m_PhyAddrCodeLen; /* 终端工作状态[1] + 首次注册标识[1] + 地址编码长度[1] + 地址编码[地址编码长度] */
				lCount += 2;
				memcpy(&pContent->m_CmdContent.m_HeartBeatInfo, plDevHeartBeatInfo, sizeof(GS_EM_IP_PROT_DEV_HEARTBEAT_S));
			}
			break;
		case IP_PROT_CMD_START_BC:
			{
				GS_S32 lTmpCount = 0, i;
				GS_EM_IP_PROT_START_BC_S *plStartBcInfo = (GS_EM_IP_PROT_START_BC_S *)pCmdInfo;
				
				lTmpCount += 15;
				lTmpCount += 1;
				lTmpCount += 1;
				lTmpCount += 5;
				lTmpCount += 1;
				lTmpCount += 4;
				lTmpCount += 4;
				lTmpCount += 1;
				for (i = 0; i < plStartBcInfo->m_AssistDataNum; i++) {
					lTmpCount += 1;
					lTmpCount += 2;
					lTmpCount += plStartBcInfo->m_pAssistData[i].m_Len;
				}
				pContent->m_CmdLen = lTmpCount;
				lCount += 2;

				memcpy(&pContent->m_CmdContent.m_StartBCInfo, plStartBcInfo, sizeof(GS_EM_IP_PROT_START_BC_S));
			}
			break;
		case IP_PROT_CMD_STOP_BC:
			{
				GS_EM_IP_PROT_STOP_BC_S *plStopBcInfo = (GS_EM_IP_PROT_STOP_BC_S *)pCmdInfo;

				pContent->m_CmdLen = 15;
				lCount += 2;

				memcpy(&pContent->m_CmdContent.m_StopBCInfo, plStopBcInfo, sizeof(GS_EM_IP_PROT_STOP_BC_S));
			}
			break;
		case IP_PROT_CMD_STAT_QUERY:
			break;
		case IP_PROT_CMD_DEV_PARAM_SET:
			{
				GS_S32 lTmpCount = 0, i;
				GS_EM_IP_PROT_DEV_PARAM_SET_S *plDevParamSetInfo = (GS_EM_IP_PROT_DEV_PARAM_SET_S *)pCmdInfo;

				lTmpCount += 1;
				for (i = 0; i < plDevParamSetInfo->m_ParamSetNum; i++) {
					lTmpCount += 1;
					lTmpCount += 1;
					lTmpCount += plDevParamSetInfo->m_pParamData[i].m_ParamLen;
				}

				pContent->m_CmdLen = lTmpCount;
				lCount += 2;

				memcpy(&pContent->m_CmdContent.m_ParamSetInfo, plDevParamSetInfo, sizeof(GS_EM_IP_PROT_DEV_PARAM_SET_S));
			}
			break;
		case IP_PROT_CMD_FAULT_RECOVERY:
			break;
		case IP_PROT_CMD_TASK_SWITCH:
			break;
		case IP_PROT_CMD_BC_REPONSE:
			break;
		case IP_PROT_CMD_BC_RECORD_QUERY:
			break;
		case IP_PROT_CMD_AUT:
			break;
		case IP_PROT_CMD_AUD_TRANS_START:
			{
				GS_S32 lTmpCount = 0, i;
				GS_EM_IP_PROT_AUD_TRANS_START_S *plAudTransStartInfo = (GS_EM_IP_PROT_AUD_TRANS_START_S *)pCmdInfo;

				lTmpCount += 4;
				lTmpCount += 2;
				for (i = 0; i < plAudTransStartInfo->m_SpotsObjNum; i++) {
					lTmpCount += 9;
				}

				lTmpCount += 1;
				lTmpCount += 1;
				lTmpCount += 1;
				lTmpCount += 2;
				lTmpCount += plAudTransStartInfo->m_BCAddrLen;

				pContent->m_CmdLen = lTmpCount;
				lCount += 2;

				memcpy(&pContent->m_CmdContent.m_AudTransStartInfo, plAudTransStartInfo, sizeof(GS_EM_IP_PROT_AUD_TRANS_START_S));
			}
			break;
		case IP_PROT_CMD_AUD_TRANS_END:
			{
				GS_S32 lTmpCount = 0, i;
				GS_EM_IP_PROT_AUD_TRANS_END_S *plAudTransEndInfo = (GS_EM_IP_PROT_AUD_TRANS_END_S *)pCmdInfo;

				lTmpCount += 4;
				lTmpCount += 2;
				for (i = 0; i < plAudTransEndInfo->m_SpotsObjNum; i++) {
					lTmpCount += 9;
				}

				pContent->m_CmdLen = lTmpCount;
				lCount += 2;

				memcpy(&pContent->m_CmdContent.m_AudTransEndInfo, plAudTransEndInfo, sizeof(GS_EM_IP_PROT_AUD_TRANS_END_S));
			}
			break;
		default:
			break;
		}
	}

	lCount += pContent->m_CmdLen;

	return lCount;
}

static GS_S32 GS_EM_IP_PROT_ResCodeParse(GS_U8 *pBuf, GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S *pResCode, GS_HANDLE BufHandle)
{
	pResCode->m_ResTypeCode = pBuf[0];
	pResCode->m_ResSubTypeCode = pBuf[1];
	memcpy(pResCode->m_pZoneCode, &pBuf[2], 6);
	pResCode->m_ExCode = pBuf[8];

	return GS_SUCCESS;
}

static GS_S32 GS_EM_IP_PROT_BCMsgIdParse(GS_U8 *pBuf, GS_EM_IP_PROT_EM_MSG_ID_S *pBCMsgId, GS_HANDLE BufHandle)
{
	GS_EM_IP_PROT_ResCodeParse(pBuf, &pBCMsgId->m_EmPlatformId, BufHandle);
	pBCMsgId->m_Year = (GS_EM_IP_PROT_BCDToChar(pBuf[9]) << 8) | GS_EM_IP_PROT_BCDToChar(pBuf[10]); /* 高位在前 */
	pBCMsgId->m_Month = GS_EM_IP_PROT_BCDToChar(pBuf[11]);
	pBCMsgId->m_Day = GS_EM_IP_PROT_BCDToChar(pBuf[12]);
	GS_MSB16_D(&pBuf[13], pBCMsgId->m_SeqCode);

	return GS_SUCCESS;
}

static GS_S32 GS_EM_IP_PROT_NetAddrConstruct(GS_U8 *pData, GS_EM_IP_PROT_NET_ADDR_S *pAddr, GS_S32 AddrLen) 
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;

	pData[lCount ++] = pAddr->m_Type;
	if (GS_EM_IP_PROT_HOSTNAME_TYPE_IP == pAddr->m_Type) {
		GS_MSB32_E(&pData[lCount], pAddr->m_HostName.m_IpAddr);
		lCount += 4;
	}
	else if (GS_EM_IP_PROT_HOSTNAME_TYPE_DOMAIN == pAddr->m_Type) {
		if (AddrLen - 3 > 64) {
			GS_COMM_ErrMsg("Domain Too Long!");
			return GS_FAILURE;
		}
		memcpy(&pData[lCount], pAddr->m_HostName.m_pDomain, AddrLen - 3);
		lCount += (AddrLen - 3);
	}
	else {
		GS_COMM_ErrMsg("IP Addr Type Error!");
		return GS_FAILURE;
	}
	GS_MSB16_E(&pData[lCount], pAddr->m_Port);
	lCount += 2;

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_StartBCConstruct(GS_U8 *pData, GS_EM_IP_PROT_START_BC_S *pCmdContent, GS_S32 CmdLen)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;

	memcpy(&pData[lCount], &pCmdContent->m_EmMsgId, 15);
	lCount += 15;
	pData[lCount ++] = pCmdContent->m_BCType;
	pData[lCount ++] = pCmdContent->m_EmEventLvl;
	memcpy(&pData[lCount], pCmdContent->m_pEmEventType, 5);
	lCount += 5;
	pData[lCount ++] = pCmdContent->m_Vol;
	GS_MSB32_E(&pData[lCount], pCmdContent->m_StartTime);
	lCount += 4;
	GS_MSB32_E(&pData[lCount], pCmdContent->m_DurationTime);
	lCount += 4;
	pData[lCount ++] = pCmdContent->m_AssistDataNum;
	for (i = 0; i < pCmdContent->m_AssistDataNum; i++) {
		GS_EM_IP_PROT_ASSIST_DATA_S *plData = &pCmdContent->m_pAssistData[i];
		GS_S32 lDataLen = plData->m_Len;

		pData[lCount ++] = plData->m_Type;
		GS_MSB16_E(&pData[lCount], lDataLen);
		lCount += 2;
		switch (plData->m_Type)
		{
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_TEXT:
			memcpy(&pData[lCount], plData->m_Data.m_pText, lDataLen);
			lCount += lDataLen;
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_AUD:
			pData[lCount ++] = plData->m_Data.m_AudInfo.m_AudTransProt;
			pData[lCount ++] = plData->m_Data.m_AudInfo.m_AudEncFmt;
			if (plData->m_Data.m_AudInfo.m_AudTransProt != GS_EM_IP_PROT_AUD_TRANS_PROT_RTP) { /* 针对 RTP 单播不会传送网络地址 */ 
				lRet = GS_EM_IP_PROT_NetAddrConstruct(&pData[lCount], &plData->m_Data.m_AudInfo.m_Addr, lDataLen - 2);
				if (lRet != GS_SUCCESS) {
					return lRet;
				}
				lCount += (lDataLen - 2);
			}
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_PIC:
			lRet = GS_EM_IP_PROT_NetAddrConstruct(&pData[lCount], &plData->m_Data.m_PicInfo.m_Addr, lDataLen);
			if (lRet != GS_SUCCESS) {
				return lRet;
			}
			lCount += lDataLen;
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_VID:
			lRet = GS_EM_IP_PROT_NetAddrConstruct(&pData[lCount], &plData->m_Data.m_VidInfo.m_Addr, lDataLen);
			if (lRet != GS_SUCCESS) {
				return lRet;
			}
			lCount += lDataLen;
			break;
		default:
			GS_COMM_ErrMsg("Assist Data Type Error!");
			return GS_FAILURE;
		}
	}

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_StopBCConstruct(GS_U8 *pData, GS_EM_IP_PROT_STOP_BC_S *pCmdContent, GS_S32 CmdLen)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;

	memcpy(&pData[lCount], &pCmdContent->m_EmMsgId, 15);
	lCount += 15;

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_DevHeartBeatConstruct(GS_U8 *pData, GS_EM_IP_PROT_DEV_HEARTBEAT_S *pCmdContent, GS_S32 CmdLen)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;

	pData[lCount ++] = pCmdContent->m_DevWorkStat;
	pData[lCount ++] = pCmdContent->m_FirstRegister;
	pData[lCount ++] = pCmdContent->m_PhyAddrCodeLen;
	memcpy(&pData[lCount], pCmdContent->m_pPhyAddrCode, pCmdContent->m_PhyAddrCodeLen);
	lCount += pCmdContent->m_PhyAddrCodeLen;

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_DevParamSetConstruct(GS_U8 *pData, GS_EM_IP_PROT_DEV_PARAM_SET_S *pCmdContent, GS_S32 CmdLen)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;

	pData[lCount ++] = pCmdContent->m_ParamSetNum;
	for (i = 0; i < pCmdContent->m_ParamSetNum; i++) {
		pData[lCount ++] = pCmdContent->m_pParamData[i].m_Tag;
		pData[lCount ++] = pCmdContent->m_pParamData[i].m_ParamLen;
		switch (pCmdContent->m_pParamData[i].m_Tag)
		{
		case IP_PROT_DEV_PARAM_TAG_VOL:
			pData[lCount ++] = pCmdContent->m_pParamData[i].m_Param.m_Vol;
			break;
		case IP_PROT_DEV_PARAM_TAG_IPV4:
			GS_MSB32_E(&pData[lCount], pCmdContent->m_pParamData[i].m_Param.m_LocalIp.m_IpAddr);
			lCount += 4;
			GS_MSB32_E(&pData[lCount], pCmdContent->m_pParamData[i].m_Param.m_LocalIp.m_IpMask);
			lCount += 4;
			GS_MSB32_E(&pData[lCount], pCmdContent->m_pParamData[i].m_Param.m_LocalIp.m_IpGate);
			lCount += 4;
			break;
		case IP_PROT_DEV_PARAM_TAG_RTP_POSTBACK_ADDR:
			lRet = GS_EM_IP_PROT_NetAddrConstruct(&pData[lCount], &pCmdContent->m_pParamData[i].m_Param.m_PostBackAddrInfo, pCmdContent->m_pParamData[i].m_ParamLen);
			if (lRet != GS_SUCCESS) {
				return lRet;
			}
			lCount += pCmdContent->m_pParamData[i].m_ParamLen;
			break;
		case IP_PROT_DEV_PARAM_TAG_SERVER_TCP_ADDR:
			lRet = GS_EM_IP_PROT_NetAddrConstruct(&pData[lCount], &pCmdContent->m_pParamData[i].m_Param.m_ServerTcpAddr, pCmdContent->m_pParamData[i].m_ParamLen);
			if (lRet != GS_SUCCESS) {
				return lRet;
			}
			lCount += pCmdContent->m_pParamData[i].m_ParamLen;
			break;
		case IP_PROT_DEV_PARAM_TAG_DEV_RES_CODE:
			memcpy(&pData[lCount], &pCmdContent->m_pParamData[i].m_Param.m_DevResCode, 9);
			lCount += 9;
			break;
		default:
			break;
		}
	}

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_AudTransStartConstruct(GS_U8 *pData, GS_EM_IP_PROT_AUD_TRANS_START_S *pCmdContent, GS_S32 CmdLen)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;

	GS_MSB32_E(&pData[lCount], pCmdContent->m_CertID);
	lCount += 4;
	GS_MSB16_E(&pData[lCount], pCmdContent->m_SpotsObjNum);
	lCount += 2;
	for (i = 0; i < pCmdContent->m_SpotsObjNum; i++) {
		memcpy (&pData[lCount], &pCmdContent->m_pSpotsObj[i], 9);
		lCount += 9;
	}
	pData[lCount ++] = pCmdContent->m_BCType;
	pData[lCount ++] = pCmdContent->m_AudTransProt;
	pData[lCount ++] = pCmdContent->m_AudEncFmt;
	GS_MSB16_E(&pData[lCount], pCmdContent->m_BCAddrLen);
	lCount += 2;
	lRet = GS_EM_IP_PROT_NetAddrConstruct(&pData[lCount], &pCmdContent->m_BCAddr, pCmdContent->m_BCAddrLen);
	if (lRet != GS_SUCCESS) {
		return lRet;
	}
	lCount += pCmdContent->m_BCAddrLen;

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_AudTransEndConstruct(GS_U8 *pData, GS_EM_IP_PROT_AUD_TRANS_END_S *pCmdContent, GS_S32 CmdLen)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;

	GS_MSB32_E(&pData[lCount], pCmdContent->m_CertID);
	lCount += 4;
	GS_MSB16_E(&pData[lCount], pCmdContent->m_SpotsObjNum);
	lCount += 2;
	for (i = 0; i < pCmdContent->m_SpotsObjNum; i++) {
		memcpy (&pData[lCount], &pCmdContent->m_pSpotsObj[i], 9);
		lCount += 9;
	}

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_ResponseMsgConstruct(GS_U8 *pData, GS_EM_IP_PROT_RESPONSE_S *pCmdContent, GS_S32 CmdLen)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;

	pData[lCount ++] = pCmdContent->m_RetCode;
	GS_MSB16_E(&pData[lCount], pCmdContent->m_DescLen);
	lCount += 2;
	memcpy(&pData[lCount], pCmdContent->m_pDesc, CmdLen - 3);
	lCount += (CmdLen - 3);

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_NetAddrParse(GS_U8 *pData, GS_EM_IP_PROT_NET_ADDR_S *pAddr, GS_S32 AddrLen, GS_HANDLE BufHandle) 
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;

	pAddr->m_Type = pData[lCount ++];
	if (GS_EM_IP_PROT_HOSTNAME_TYPE_IP == pAddr->m_Type) {
		GS_MSB32_D(&pData[lCount], pAddr->m_HostName.m_IpAddr);
		lCount += 4;
	}
	else if (GS_EM_IP_PROT_HOSTNAME_TYPE_DOMAIN == pAddr->m_Type) {
		if (AddrLen - 3 > 64) {
			GS_COMM_ErrMsg("Domain Too Long!");
			return GS_FAILURE;
		}
		memcpy(pAddr->m_HostName.m_pDomain, &pData[lCount], AddrLen - 3);
		lCount += (AddrLen - 3);
	}
	else {
		GS_COMM_ErrMsg("IP Addr Type Error!");
		return GS_FAILURE;
	}
	GS_MSB16_D(&pData[lCount], pAddr->m_Port);
	lCount += 2;

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_StartBCParse(GS_U8 *pData, GS_EM_IP_PROT_START_BC_S *pCmdContent, GS_S32 CmdLen, GS_HANDLE BufHandle)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;

	memcpy(&pCmdContent->m_EmMsgId, &pData[lCount], 15);
	lCount += 15;
	pCmdContent->m_BCType = pData[lCount ++];
	pCmdContent->m_EmEventLvl = pData[lCount ++];
	memcpy(pCmdContent->m_pEmEventType, &pData[lCount], 5);
	lCount += 5;
	pCmdContent->m_Vol = pData[lCount ++];
	GS_MSB32_D(&pData[lCount], pCmdContent->m_StartTime);
	lCount += 4;
	GS_MSB32_D(&pData[lCount], pCmdContent->m_DurationTime);
	lCount += 4;
	pCmdContent->m_AssistDataNum = pData[lCount ++];
	for (i = 0; i < pCmdContent->m_AssistDataNum; i++) {
		GS_EM_IP_PROT_ASSIST_DATA_S *plData = &pCmdContent->m_pAssistData[i];
		GS_S32 lDataLen;

		plData->m_Type = pData[lCount ++];
		GS_MSB16_D(&pData[lCount], plData->m_Len);
		lCount += 2;
		lDataLen = plData->m_Len;
		switch (plData->m_Type)
		{
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_TEXT:
			if (lDataLen > 0) {
				plData->m_Data.m_pText = (GS_CHAR *)GS_COMM_UserBufMalloc(BufHandle, lDataLen);
				if (!plData->m_Data.m_pText) {
					GS_COMM_ErrMsg("GS_COMM_UserBufMalloc Error!");
					return GS_FAILURE;
				}
				memcpy(plData->m_Data.m_pText, &pData[lCount], lDataLen);
				lCount += lDataLen;
			}
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_AUD:
			plData->m_Data.m_AudInfo.m_AudTransProt = pData[lCount ++];
			plData->m_Data.m_AudInfo.m_AudEncFmt = pData[lCount ++];
			if (plData->m_Data.m_AudInfo.m_AudTransProt != GS_EM_IP_PROT_AUD_TRANS_PROT_RTP) { /* 针对 RTP 单播不会传送网络地址 */ 
				lRet = GS_EM_IP_PROT_NetAddrParse(&pData[lCount], &plData->m_Data.m_AudInfo.m_Addr, lDataLen - 2, BufHandle);
				if (lRet != GS_SUCCESS) {
					return lRet;
				}
				lCount += (lDataLen - 2);
			}
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_PIC:
			lRet = GS_EM_IP_PROT_NetAddrParse(&pData[lCount], &plData->m_Data.m_PicInfo.m_Addr, lDataLen, BufHandle);
			if (lRet != GS_SUCCESS) {
				return lRet;
			}
			lCount += lDataLen;
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_VID:
			lRet = GS_EM_IP_PROT_NetAddrParse(&pData[lCount], &plData->m_Data.m_VidInfo.m_Addr, lDataLen, BufHandle);
			if (lRet != GS_SUCCESS) {
				return lRet;
			}
			lCount += lDataLen;
			break;
		default:
			GS_COMM_ErrMsg("Assist Data Type Error!");
			return GS_FAILURE;
		}
	}

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_StopBCParse(GS_U8 *pData, GS_EM_IP_PROT_STOP_BC_S *pCmdContent, GS_S32 CmdLen, GS_HANDLE BufHandle)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;

	memcpy(&pCmdContent->m_EmMsgId, &pData[lCount], 15);
	lCount += 15;

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_DevHeartBeatParse(GS_U8 *pData, GS_EM_IP_PROT_DEV_HEARTBEAT_S *pCmdContent, GS_S32 CmdLen, GS_HANDLE BufHandle)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;

	pCmdContent->m_DevWorkStat = pData[lCount ++];
	pCmdContent->m_FirstRegister = pData[lCount ++];
	pCmdContent->m_PhyAddrCodeLen = pData[lCount ++];
	if (pCmdContent->m_PhyAddrCodeLen > 0) {
		pCmdContent->m_pPhyAddrCode = GS_COMM_UserBufMalloc(BufHandle, pCmdContent->m_PhyAddrCodeLen);
		if (!pCmdContent->m_pPhyAddrCode) {
			GS_COMM_ErrMsg("GS_COMM_UserBufMalloc Error!");
			return GS_FAILURE;
		}
		memcpy(pCmdContent->m_pPhyAddrCode, &pData[lCount], pCmdContent->m_PhyAddrCodeLen);
		lCount += pCmdContent->m_PhyAddrCodeLen;
	}

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_DevParamSetParse(GS_U8 *pData, GS_EM_IP_PROT_DEV_PARAM_SET_S *pCmdContent, GS_S32 CmdLen, GS_HANDLE BufHandle)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;

	pCmdContent->m_ParamSetNum = pData[lCount ++];
	if (pCmdContent->m_ParamSetNum > 0) {
		pCmdContent->m_pParamData = (GS_EM_IP_PROT_DEV_PARAM_S *)GS_COMM_UserBufMalloc(BufHandle, pCmdContent->m_ParamSetNum * sizeof(GS_EM_IP_PROT_DEV_PARAM_S));
		if (!pCmdContent->m_pParamData) {
			GS_COMM_ErrMsg("GS_COMM_UserBufMalloc Error!");
			return GS_FAILURE;
		}
		for (i = 0; i < pCmdContent->m_ParamSetNum; i++) {
			pCmdContent->m_pParamData[i].m_Tag = pData[lCount ++];
			pCmdContent->m_pParamData[i].m_ParamLen = pData[lCount ++];
			switch (pCmdContent->m_pParamData[i].m_Tag)
			{
			case IP_PROT_DEV_PARAM_TAG_VOL:
				pCmdContent->m_pParamData[i].m_Param.m_Vol = pData[lCount ++];
				break;
			case IP_PROT_DEV_PARAM_TAG_IPV4:
				GS_MSB32_D(&pData[lCount], pCmdContent->m_pParamData[i].m_Param.m_LocalIp.m_IpAddr);
				lCount += 4;
				GS_MSB32_D(&pData[lCount], pCmdContent->m_pParamData[i].m_Param.m_LocalIp.m_IpMask);
				lCount += 4;
				GS_MSB32_D(&pData[lCount], pCmdContent->m_pParamData[i].m_Param.m_LocalIp.m_IpGate);
				lCount += 4;
				break;
			case IP_PROT_DEV_PARAM_TAG_RTP_POSTBACK_ADDR:
				lRet = GS_EM_IP_PROT_NetAddrParse(&pData[lCount], &pCmdContent->m_pParamData[i].m_Param.m_PostBackAddrInfo, pCmdContent->m_pParamData[i].m_ParamLen, BufHandle);
				if (lRet != GS_SUCCESS) {
					return lRet;
				}
				lCount += pCmdContent->m_pParamData[i].m_ParamLen;
				break;
			case IP_PROT_DEV_PARAM_TAG_SERVER_TCP_ADDR:
				lRet = GS_EM_IP_PROT_NetAddrParse(&pData[lCount], &pCmdContent->m_pParamData[i].m_Param.m_ServerTcpAddr, pCmdContent->m_pParamData[i].m_ParamLen, BufHandle);
				if (lRet != GS_SUCCESS) {
					return lRet;
				}
				lCount += pCmdContent->m_pParamData[i].m_ParamLen;
				break;
			case IP_PROT_DEV_PARAM_TAG_DEV_RES_CODE:
				memcpy(&pCmdContent->m_pParamData[i].m_Param.m_DevResCode, &pData[lCount], 9);
				lCount += 9;
				break;
			default:
				break;
			}
		}
	}

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_AudTransStartParse(GS_U8 *pData, GS_EM_IP_PROT_AUD_TRANS_START_S *pCmdContent, GS_S32 CmdLen, GS_HANDLE BufHandle)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;

	GS_MSB32_D(&pData[lCount], pCmdContent->m_CertID);
	lCount += 4;
	GS_MSB16_D(&pData[lCount], pCmdContent->m_SpotsObjNum);
	lCount += 2;
	if (pCmdContent->m_SpotsObjNum > 0) {
		pCmdContent->m_pSpotsObj = (GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S *)GS_COMM_UserBufMalloc(BufHandle, 9 * pCmdContent->m_SpotsObjNum);
		if (!pCmdContent->m_pSpotsObj) {
			GS_COMM_ErrMsg("GS_COMM_UserBufMalloc Error!");
			return GS_FAILURE;
		}
		for (i = 0; i < pCmdContent->m_SpotsObjNum; i++) {
			memcpy (&pCmdContent->m_pSpotsObj[i], &pData[lCount], 9);
			lCount += 9;
		}
	}
	pCmdContent->m_BCType = pData[lCount ++];
	pCmdContent->m_AudTransProt = pData[lCount ++];
	pCmdContent->m_AudEncFmt = pData[lCount ++];
	GS_MSB16_D(&pData[lCount], pCmdContent->m_BCAddrLen);
	lCount += 2;
	lRet = GS_EM_IP_PROT_NetAddrParse(&pData[lCount], &pCmdContent->m_BCAddr, pCmdContent->m_BCAddrLen, BufHandle);
	if (lRet != GS_SUCCESS) {
		return lRet;
	}
	lCount += pCmdContent->m_BCAddrLen;

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_AudTransEndParse(GS_U8 *pData, GS_EM_IP_PROT_AUD_TRANS_END_S *pCmdContent, GS_S32 CmdLen, GS_HANDLE BufHandle)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;

	GS_MSB32_D(&pData[lCount], pCmdContent->m_CertID);
	lCount += 4;
	GS_MSB16_D(&pData[lCount], pCmdContent->m_SpotsObjNum);
	lCount += 2;

	if (pCmdContent->m_SpotsObjNum > 0) {
		pCmdContent->m_pSpotsObj = (GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S *)GS_COMM_UserBufMalloc(BufHandle, 9 * pCmdContent->m_SpotsObjNum);
		if (!pCmdContent->m_pSpotsObj) {
			GS_COMM_ErrMsg("GS_COMM_UserBufMalloc Error!");
			return GS_FAILURE;
		}
		for (i = 0; i < pCmdContent->m_SpotsObjNum; i++) {
			memcpy (&pCmdContent->m_pSpotsObj[i], &pData[lCount], 9);
			lCount += 9;
		}
	}

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error! lCount=%d CmdLen=%d\n", lCount, CmdLen);
		lRet = GS_FAILURE;
	}

	return lRet;
}

static GS_S32 GS_EM_IP_PROT_ResponseMsgParse(GS_U8 *pData, GS_EM_IP_PROT_RESPONSE_S *pCmdContent, GS_S32 CmdLen, GS_HANDLE BufHandle)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;

	pCmdContent->m_RetCode = pData[lCount ++];
	GS_MSB16_D(&pData[lCount], pCmdContent->m_DescLen);
	lCount += 2;
	if (CmdLen - 3 > 0) {
		pCmdContent->m_pDesc = (GS_CHAR *)GS_COMM_UserBufMalloc(BufHandle, CmdLen - 3);
		if (!pCmdContent->m_pDesc) {
			GS_COMM_ErrMsg("GS_COMM_UserBufMalloc Error!");
			return GS_FAILURE;
		}
		memcpy(pCmdContent->m_pDesc, &pData[lCount], CmdLen - 3);
		lCount += (CmdLen - 3);
	}

	if (lCount != CmdLen) {
		GS_DBGERR("Command Length Check Error!\n");
		lRet = GS_FAILURE;
	}

	return lRet;
}

/* 应急数据包构建 */
GS_S32	GS_EM_IP_PROT_Construct(GS_U8 *pData, GS_S32 *pLen, GS_EM_IP_PROT_S *pIpProt)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0, i;
	GS_U32 lCrc32;

	if (!pData || !pIpProt) {
		GS_COMM_ErrMsg("Param is NULL!");
		return GS_FAILURE;
	}

	*pLen = pIpProt->m_Header.m_DataLen;

	/* 包头转换 */
	GS_MSB16_E(&pData[lCount], pIpProt->m_Header.m_Tag);
	lCount += 2;
	GS_MSB16_E(&pData[lCount], pIpProt->m_Header.m_Ver);
	lCount += 2;
	GS_MSB32_E(&pData[lCount], pIpProt->m_Header.m_SessionId);
	lCount += 4;
	pData[lCount ++] = pIpProt->m_Header.m_DataType;
	pData[lCount ++] = pIpProt->m_Header.m_SignTag;
	GS_MSB16_E(&pData[lCount], pIpProt->m_Header.m_DataLen);
	lCount += 2;

	/* 包体转换 */
	memcpy(&pData[lCount], &pIpProt->m_Content.m_DataSrcCompBCDCode, 9);
	lCount += 9;
	GS_MSB16_E(&pData[lCount], pIpProt->m_Content.m_DataDstNum);
	lCount += 2;
	for (i = 0; i < pIpProt->m_Content.m_DataDstNum; i++) {
		memcpy(&pData[lCount], &pIpProt->m_Content.m_pDataDstCompBCDCode[i], 9);
		lCount += 9;
	}
	pData[lCount ++] = pIpProt->m_Content.m_CmdType;
	GS_MSB16_E(&pData[lCount], pIpProt->m_Content.m_CmdLen);
	lCount += 2;

	if (pIpProt->m_Header.m_DataType == 0x01) { /* 请求数据包 */
		switch (pIpProt->m_Content.m_CmdType)
		{
		case IP_PROT_CMD_START_BC:
			lRet = GS_EM_IP_PROT_StartBCConstruct(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_StartBCInfo, pIpProt->m_Content.m_CmdLen);
			break;
		case IP_PROT_CMD_STOP_BC:
			lRet = GS_EM_IP_PROT_StopBCConstruct(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_StopBCInfo, pIpProt->m_Content.m_CmdLen);
			break;
		case IP_PROT_CMD_DEV_HEARTBEAT:
			lRet = GS_EM_IP_PROT_DevHeartBeatConstruct(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo, pIpProt->m_Content.m_CmdLen);
			break;
		case IP_PROT_CMD_STAT_QUERY:
			break;
		case IP_PROT_CMD_DEV_PARAM_SET:
			lRet = GS_EM_IP_PROT_DevParamSetConstruct(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_ParamSetInfo, pIpProt->m_Content.m_CmdLen);
			break;
		case IP_PROT_CMD_FAULT_RECOVERY:
			break;
		case IP_PROT_CMD_TASK_SWITCH:
			break;
		case IP_PROT_CMD_BC_REPONSE:
			break;
		case IP_PROT_CMD_BC_RECORD_QUERY:
			break;
		case IP_PROT_CMD_AUT:
			break;
		case IP_PROT_CMD_AUD_TRANS_START:
			lRet = GS_EM_IP_PROT_AudTransStartConstruct(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo, pIpProt->m_Content.m_CmdLen);
			break;
		case IP_PROT_CMD_AUD_TRANS_END:
			lRet = GS_EM_IP_PROT_AudTransEndConstruct(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo, pIpProt->m_Content.m_CmdLen);
			break;
		default:
			GS_COMM_ErrMsg("Command Type is Not Support!");
			lRet = GS_FAILURE;
			break;
		}
	}
	else if (pIpProt->m_Header.m_DataType == 0x02) { /* 应答数据包 */
		lRet = GS_EM_IP_PROT_ResponseMsgConstruct(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_ResponseInfo, pIpProt->m_Content.m_CmdLen);
	}
	else {
		GS_COMM_ErrMsg("Data Packet Type is Not Support!");
		lRet = GS_FAILURE;
	}

	if (lRet == GS_SUCCESS) {
		lCount += pIpProt->m_Content.m_CmdLen;

		lCrc32 = GS_COMM_Crc32Calc(pData, lCount);
		GS_MSB32_E(&pData[lCount], lCrc32);
		lCount += 4;
	}

	if (lCount != *pLen) {
		GS_COMM_ErrQuit("Packet Length Check Failure!");
	}

	return lRet;
}

GS_VOID GS_EM_IP_PROT_ResponseInfoMake(GS_EM_IP_PROT_RESPONSE_S *pResponseInfo, GS_EM_IP_PROT_S *pIpProt, GS_U8 RetCode)
{
	pResponseInfo->m_RetCode = RetCode;
	if (RetCode == IP_PROT_SUCCESS) {
		switch (pIpProt->m_Content.m_CmdType)
		{
		case IP_PROT_CMD_START_BC:
			pResponseInfo->m_DescLen = 15;
			pResponseInfo->m_pDesc = (GS_CHAR *)&pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId;
			break;
		case IP_PROT_CMD_STOP_BC:
			pResponseInfo->m_DescLen = 15;
			pResponseInfo->m_pDesc = (GS_CHAR *)&pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId;
			break;
		case IP_PROT_CMD_DEV_HEARTBEAT:
			pResponseInfo->m_DescLen = pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_PhyAddrCodeLen;
			pResponseInfo->m_pDesc = (GS_CHAR *)pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_pPhyAddrCode;
			break;
		case IP_PROT_CMD_STAT_QUERY:
			break;
		case IP_PROT_CMD_DEV_PARAM_SET:
			pResponseInfo->m_DescLen = 0;
			pResponseInfo->m_pDesc = NULL;
			break;
		case IP_PROT_CMD_FAULT_RECOVERY:
			break;
		case IP_PROT_CMD_TASK_SWITCH:
			break;
		case IP_PROT_CMD_BC_REPONSE:
			break;
		case IP_PROT_CMD_BC_RECORD_QUERY:
			break;
		case IP_PROT_CMD_AUT:
			break;
		case IP_PROT_CMD_AUD_TRANS_START:
			pResponseInfo->m_DescLen = 0;
			pResponseInfo->m_pDesc = NULL;
			break;
		case IP_PROT_CMD_AUD_TRANS_END:
			pResponseInfo->m_DescLen = 0;
			pResponseInfo->m_pDesc = NULL;
			break;
		default:
			break;
		}
	}
	else { /* 失败的应答，没有返回描述，为空 */
		pResponseInfo->m_DescLen = 0;
		pResponseInfo->m_pDesc = NULL;
	}
}

/* 应答消息封包 */
GS_S32 GS_EM_IP_PROT_ResponseMsgPack(GS_U8 *pData, GS_S32 *pLen, GS_EM_IP_PROT_RESPONSE_S *pResponseInfo, GS_U32 SessionId, GS_U32 CmdType)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_EM_IP_PROT_S lIpProt;
	GS_S32 lCount = 0; /* 数据长度计数器 */
	
	memset(&lIpProt, 0, sizeof(GS_EM_IP_PROT_S));
	
	lCount += GS_EM_IP_PROT_ContentPack(&lIpProt.m_Content, pResponseInfo, GS_EM_IP_PROT_DATA_TYPE_RESPONSE, CmdType);
	lCount += 4; /* 4 个字节的 CRC32 */
	GS_EM_IP_PROT_HeaderPack(&lIpProt.m_Header, SessionId, GS_EM_IP_PROT_DATA_TYPE_RESPONSE, lCount);

	lRet = GS_EM_IP_PROT_Construct(pData, pLen, &lIpProt);

	return lRet;
}

/* 业务数据封包 */
GS_S32 GS_EM_IP_PROT_CmdInfoPack(GS_U8 *pData, GS_S32 *pLen, GS_VOID *pCmdInfo, GS_U8 CmdType, GS_U32 SessionId)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_EM_IP_PROT_S lIpProt;
	GS_S32 lCount = 0; /* 数据长度计数器 */

	memset(&lIpProt, 0, sizeof(GS_EM_IP_PROT_S));

	lCount += GS_EM_IP_PROT_ContentPack(&lIpProt.m_Content, pCmdInfo, GS_EM_IP_PROT_DATA_TYPE_REQUEST, CmdType);
	lCount += 4; /* 4 个字节的 CRC32 */
	GS_EM_IP_PROT_HeaderPack(&lIpProt.m_Header, SessionId, GS_EM_IP_PROT_DATA_TYPE_REQUEST, lCount);

	lRet = GS_EM_IP_PROT_Construct(pData, pLen, &lIpProt);

	return lRet;
}

/* 
	应急数据包解析
	注意!
		1) 该函数使用后，需要使用 GS_EM_IP_PROT_ParseClean() 销毁使用分配的内存；
*/
GS_S32	GS_EM_IP_PROT_Parse(GS_U8 *pData, GS_S32 Len, GS_EM_IP_PROT_S *pIpProt, GS_HANDLE *pBufHandle)
{
	GS_S32 lRet = GS_SUCCESS;
	GS_S32 lCount = 0;
	GS_S32 lCrc32 = 0;

	*pBufHandle = GS_COMM_UserBufCreate(GS_EM_IP_PROT_MAX_LEN);
	if (!(*pBufHandle)) {
		GS_COMM_ErrMsg("GS_COMM_UserBufCreate Error!");
		return GS_FAILURE;
	}

	/* 包头解析 */
	GS_MSB16_D(&pData[lCount], pIpProt->m_Header.m_Tag);
	lCount += 2;
	GS_MSB16_D(&pData[lCount], pIpProt->m_Header.m_Ver);
	lCount += 2;
	GS_MSB32_D(&pData[lCount], pIpProt->m_Header.m_SessionId);
	lCount += 4;
	pIpProt->m_Header.m_DataType = pData[lCount ++];
	pIpProt->m_Header.m_SignTag = pData[lCount ++];
	GS_MSB16_D(&pData[lCount], pIpProt->m_Header.m_DataLen);
	lCount += 2;

	/* 消息体解析 */
	memcpy(&pIpProt->m_Content.m_DataSrcCompBCDCode, &pData[lCount], 9);
	lCount += 9;
	GS_MSB16_D(&pData[lCount], pIpProt->m_Content.m_DataDstNum);
	lCount += 2;
	if (pIpProt->m_Content.m_DataDstNum > 0) {
		pIpProt->m_Content.m_pDataDstCompBCDCode = (GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S	*)GS_COMM_UserBufMalloc(*pBufHandle, pIpProt->m_Content.m_DataDstNum * 9);
		if (!pIpProt->m_Content.m_pDataDstCompBCDCode) {
			GS_COMM_ErrMsg("GS_COMM_UserBufMalloc Error!");
			return GS_FAILURE;
		}
		memcpy(pIpProt->m_Content.m_pDataDstCompBCDCode, &pData[lCount], pIpProt->m_Content.m_DataDstNum * 9);
		lCount += (pIpProt->m_Content.m_DataDstNum * 9);
	}
	pIpProt->m_Content.m_CmdType = pData[lCount ++];
	GS_MSB16_D(&pData[lCount], pIpProt->m_Content.m_CmdLen);
	lCount += 2;
	if (pIpProt->m_Header.m_DataType == GS_EM_IP_PROT_DATA_TYPE_REQUEST) { /* 请求数据包 */
		switch (pIpProt->m_Content.m_CmdType)
		{
		case IP_PROT_CMD_START_BC:
			lRet = GS_EM_IP_PROT_StartBCParse(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_StartBCInfo, pIpProt->m_Content.m_CmdLen, *pBufHandle);
			break;
		case IP_PROT_CMD_STOP_BC:
			lRet = GS_EM_IP_PROT_StopBCParse(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_StopBCInfo, pIpProt->m_Content.m_CmdLen, *pBufHandle);
			break;
		case IP_PROT_CMD_DEV_HEARTBEAT:
			lRet = GS_EM_IP_PROT_DevHeartBeatParse(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo, pIpProt->m_Content.m_CmdLen, *pBufHandle);
			break;
		case IP_PROT_CMD_STAT_QUERY:
			break;
		case IP_PROT_CMD_DEV_PARAM_SET:
			lRet = GS_EM_IP_PROT_DevParamSetParse(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_ParamSetInfo, pIpProt->m_Content.m_CmdLen, *pBufHandle);
			break;
		case IP_PROT_CMD_FAULT_RECOVERY:
			break;
		case IP_PROT_CMD_TASK_SWITCH:
			break;
		case IP_PROT_CMD_BC_REPONSE:
			break;
		case IP_PROT_CMD_BC_RECORD_QUERY:
			break;
		case IP_PROT_CMD_AUT:
			break;
		case IP_PROT_CMD_AUD_TRANS_START:
			lRet = GS_EM_IP_PROT_AudTransStartParse(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo, pIpProt->m_Content.m_CmdLen, *pBufHandle);
			break;
		case IP_PROT_CMD_AUD_TRANS_END:
			lRet = GS_EM_IP_PROT_AudTransEndParse(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo, pIpProt->m_Content.m_CmdLen, *pBufHandle);
			break;
		default:
			GS_COMM_ErrMsg("Command Type is Not Support!");
			lRet = GS_FAILURE;
			break;
		}
	}
	else if (pIpProt->m_Header.m_DataType == GS_EM_IP_PROT_DATA_TYPE_RESPONSE) { /* 应答数据包 */
		lRet = GS_EM_IP_PROT_ResponseMsgParse(&pData[lCount], &pIpProt->m_Content.m_CmdContent.m_ResponseInfo, pIpProt->m_Content.m_CmdLen, *pBufHandle);
	}
	else {
		GS_COMM_ErrMsg("Data Packet Type is Not Support!");
		lRet = GS_FAILURE;
	}

	if (lRet == GS_SUCCESS) {
		lCount += pIpProt->m_Content.m_CmdLen;

		lCrc32 = GS_COMM_Crc32Calc(pData, lCount);
		GS_MSB32_D(&pData[lCount], pIpProt->m_CheckData.m_Crc32);
		lCount += 4;
		if (lCrc32 != pIpProt->m_CheckData.m_Crc32) {
			GS_DBGWRN("lcrc32=0x%x pIpProt->m_CheckData.m_Crc32=0x%x\n", lCrc32, pIpProt->m_CheckData.m_Crc32);
			GS_COMM_ErrMsg("CRC32 Check Failure!");
			return IP_PROT_ERR_CRC;
		}
	}

	if (lCount != Len) {
		GS_COMM_ErrQuit("Packet Length Check Failure!");
	}

	return lRet;
}

GS_VOID GS_EM_IP_PROT_ParseClean(GS_HANDLE BufHandle)
{
	if (BufHandle) {
		GS_COMM_UserBufDestroy(BufHandle);
		BufHandle = NULL;
	}
}

static GS_VOID GS_EM_IP_PROT_ResCodePrint(GS_CHAR *pTitle, GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S *pResCode)
{
	GS_COMM_Printf("%s: ", pTitle);
	GS_COMM_Printf("ResTypeCode[%02X] ResSubTypeCode[%02x] ZoneCode[%02x%02x%02x%02x%02x%02x] ExCode[%02x]\n",
		pResCode->m_ResTypeCode, pResCode->m_ResSubTypeCode, pResCode->m_pZoneCode[0], pResCode->m_pZoneCode[1],
		pResCode->m_pZoneCode[2], pResCode->m_pZoneCode[3], pResCode->m_pZoneCode[4], pResCode->m_pZoneCode[5],
		pResCode->m_ExCode);
}

static GS_VOID GS_EM_IP_PROT_NetAddrPrint(GS_CHAR *pTitle, GS_EM_IP_PROT_NET_ADDR_S *pAddr)
{
	GS_COMM_Printf("%s: ", pTitle);
	GS_COMM_Printf("Type[%d] ", pAddr->m_Type);
	if (pAddr->m_Type == 0x01) {
		GS_COMM_Printf("IP[0x%x] ", pAddr->m_HostName.m_IpAddr);
	}
	else {
		GS_COMM_Printf("Domain[%s] ", pAddr->m_HostName.m_pDomain);
	}
	GS_COMM_Printf("Port[%d]\n", pAddr->m_Port);
}

GS_VOID GS_EM_IP_PROT_Print(GS_EM_IP_PROT_S *pIpProt)
{
	GS_S32 i;
	GS_CHAR plStr[128];

	if (pIpProt) {
		GS_COMM_Printf("\n===================== IP_PROT Packet ======================\n");
		GS_COMM_Printf("pIpProt->m_Header.m_Tag: 0x%x\n", pIpProt->m_Header.m_Tag);
		GS_COMM_Printf("pIpProt->m_Header.m_Ver: 0x%x\n", pIpProt->m_Header.m_Ver);
		GS_COMM_Printf("pIpProt->m_Header.m_SessionId: 0x%x\n", pIpProt->m_Header.m_SessionId);
		GS_COMM_Printf("pIpProt->m_Header.m_DataType: 0x%x\n", pIpProt->m_Header.m_DataType);
		GS_COMM_Printf("pIpProt->m_Header.m_SignTag: 0x%x\n", pIpProt->m_Header.m_SignTag);
		GS_COMM_Printf("pIpProt->m_Header.m_DataLen: 0x%x\n", pIpProt->m_Header.m_DataLen);
		GS_EM_IP_PROT_ResCodePrint("pIpProt->m_Content.m_DataSrcCompBCDCode", &pIpProt->m_Content.m_DataSrcCompBCDCode);
		GS_COMM_Printf("pIpProt->m_Content.m_DataDstNum: 0x%x\n", pIpProt->m_Content.m_DataDstNum);
		for (i = 0; i < pIpProt->m_Content.m_DataDstNum; i++) {
			snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_pDataDstCompBCDCode[%d]", i);
			GS_EM_IP_PROT_ResCodePrint("", &pIpProt->m_Content.m_pDataDstCompBCDCode[i]);
		}
		GS_COMM_Printf("pIpProt->m_Content.m_CmdType: 0x%02x\n", pIpProt->m_Content.m_CmdType);
		GS_COMM_Printf("pIpProt->m_Content.m_CmdLen: 0x%02x\n", pIpProt->m_Content.m_CmdLen);
		if (pIpProt->m_Header.m_DataType == GS_EM_IP_PROT_DATA_TYPE_REQUEST) { /* 请求数据包 */
			switch (pIpProt->m_Content.m_CmdType)
			{
			case IP_PROT_CMD_START_BC:
				GS_EM_IP_PROT_ResCodePrint("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId.m_EmPlatformId", 
					&pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId.m_EmPlatformId);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId.m_Date: %04d-%04d-%04d\n", 
					pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId.m_Year, pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId.m_Month,
					pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId.m_Day);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId.m_SeqCode: 0x%x\n", pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmMsgId.m_SeqCode);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_BCType: %d\n", pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_BCType);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmEventLvl: %d\n", pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_EmEventLvl);
				GS_COMM_PrintDataBlock("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pEmEventType", pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pEmEventType, 5);	
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_Vol: %d\n", pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_Vol);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_StartTime: %d\n", pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_StartTime);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_DurationTime: %d\n", pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_DurationTime);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_AssistDataNum: %d\n", pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_AssistDataNum);
				for (i = 0; i < pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_AssistDataNum; i++) {
					snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[%d]", i);
					GS_COMM_Printf("%s.m_Type: %d\n", plStr, pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Type);
					GS_COMM_Printf("%s.m_Len: %d\n", plStr, pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Len);
					switch (pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Type)
					{
					case GS_EM_IP_PROT_ASSIST_DATA_TYPE_TEXT:
						GS_COMM_Printf("%s.m_Data.m_pText: %s\n", plStr, pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_pText);
						break;
					case GS_EM_IP_PROT_ASSIST_DATA_TYPE_AUD:
						GS_COMM_Printf("%s.m_Data.m_AudInfo.m_AudTransProt: %s\n", plStr, pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudTransProt);
						GS_COMM_Printf("%s.m_Data.m_AudInfo.m_AudEncFmt: %s\n", plStr, pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudEncFmt);
						if (pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudTransProt != GS_EM_IP_PROT_AUD_TRANS_PROT_RTP) {
							snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[%d].m_Data.m_AudInfo.m_RtpAddr", i);
							GS_EM_IP_PROT_NetAddrPrint(plStr, &pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_Addr);
						}
						break;
					case GS_EM_IP_PROT_ASSIST_DATA_TYPE_PIC:
						snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[%d].m_Data.m_PicInfo.m_RtpAddr", i);
						GS_EM_IP_PROT_NetAddrPrint(plStr, &pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_PicInfo.m_Addr);
						break;
					case GS_EM_IP_PROT_ASSIST_DATA_TYPE_VID:
						snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[%d].m_Data.m_VidInfo.m_RtpAddr", i);
						GS_EM_IP_PROT_NetAddrPrint(plStr, &pIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_VidInfo.m_Addr);
						break;
					default:
						break;
					}
				}
				break;
			case IP_PROT_CMD_STOP_BC:
				GS_EM_IP_PROT_ResCodePrint("pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId.m_EmPlatformId", 
					&pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId.m_EmPlatformId);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId.m_Date: %04d-%04d-%04d\n", 
					pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId.m_Year, pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId.m_Month,
					pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId.m_Day);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId.m_SeqCode: 0x%x\n", pIpProt->m_Content.m_CmdContent.m_StopBCInfo.m_EmMsgId.m_SeqCode);
				break;
			case IP_PROT_CMD_DEV_HEARTBEAT:
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_DevWorkStat: %d\n", pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_DevWorkStat);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_FirstRegister: %d\n", pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_FirstRegister);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_PhyAddrCodeLen: %d\n", pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_PhyAddrCodeLen);
				GS_COMM_PrintDataBlock("pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_pPhyAddrCode", pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_pPhyAddrCode, 
					pIpProt->m_Content.m_CmdContent.m_HeartBeatInfo.m_PhyAddrCodeLen);
				break;
			case IP_PROT_CMD_STAT_QUERY:
				break;
			case IP_PROT_CMD_DEV_PARAM_SET:
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_ParamSetNum: %d\n", pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_ParamSetNum);
				for (i = 0; i < pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_ParamSetNum; i++) {
					snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[%d]", i);
					GS_COMM_Printf("%s.m_Tag: %d\n", pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Tag);
					GS_COMM_Printf("%s.m_ParamLen: %d\n", pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_ParamLen);
					switch (pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Tag) 
					{
					case IP_PROT_DEV_PARAM_TAG_VOL:
						GS_COMM_Printf("%s.m_Param.m_Vol: %d\n", pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_Vol);
						break;
					case IP_PROT_DEV_PARAM_TAG_IPV4:
						GS_COMM_Printf("%s.m_Param.m_LocalIp: IP[0x%x] MASK[0x%x] GW[0x%x]\n", 
							pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpAddr, 
							pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpMask, 
							pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpGate);
						break;
					case IP_PROT_DEV_PARAM_TAG_RTP_POSTBACK_ADDR:
						snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[%d].m_Param.m_PostBackAddrInfo", i);
						GS_EM_IP_PROT_NetAddrPrint(plStr, &pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_PostBackAddrInfo);
						break;
					case IP_PROT_DEV_PARAM_TAG_SERVER_TCP_ADDR:
						snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[%d].m_Param.m_ServerTcpAddr", i);
						GS_EM_IP_PROT_NetAddrPrint(plStr, &pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_ServerTcpAddr);
						break;
					case IP_PROT_DEV_PARAM_TAG_DEV_RES_CODE:
						snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[%d].m_Param.m_DevResCode", i);
						GS_EM_IP_PROT_ResCodePrint(plStr, &pIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_DevResCode);
						break;
					default:
						break;
					}
				}
				break;
			case IP_PROT_CMD_FAULT_RECOVERY:
				break;
			case IP_PROT_CMD_TASK_SWITCH:
				break;
			case IP_PROT_CMD_BC_REPONSE:
				break;
			case IP_PROT_CMD_BC_RECORD_QUERY:
				break;
			case IP_PROT_CMD_AUT:
				break;
			case IP_PROT_CMD_AUD_TRANS_START:
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_CertID: 0x%x\n", pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_CertID);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_SpotsObjNum: %d\n", pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_SpotsObjNum);
				for (i = 0; i < pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_SpotsObjNum; i++)
				{
					snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_pSpotsObj[%d]", i);
					GS_EM_IP_PROT_ResCodePrint(plStr, &pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_pSpotsObj[i]);
				}
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_BCType: %d\n", pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_BCType);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_AudTransProt: %d\n", pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_AudTransProt);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_AudEncFmt: %d\n", pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_AudEncFmt);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_BCAddrLen: %d\n", pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_BCAddrLen);
				GS_EM_IP_PROT_NetAddrPrint("pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_BCAddr", &pIpProt->m_Content.m_CmdContent.m_AudTransStartInfo.m_BCAddr);
				break;
			case IP_PROT_CMD_AUD_TRANS_END:
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo.m_CertID: 0x%x\n", pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo.m_CertID);
				GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo.m_SpotsObjNum: %d\n", pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo.m_SpotsObjNum);
				for (i = 0; i < pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo.m_SpotsObjNum; i++)
				{
					snprintf (plStr, sizeof(plStr), "pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo.m_pSpotsObj[%d]", i);
					GS_EM_IP_PROT_ResCodePrint(plStr, &pIpProt->m_Content.m_CmdContent.m_AudTransEndInfo.m_pSpotsObj[i]);
				}
				break;
			default:
				break;
			}
		}
		else if (pIpProt->m_Header.m_DataType == GS_EM_IP_PROT_DATA_TYPE_RESPONSE) { /* 应答数据包 */
			GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_ResponseInfo.m_RetCode: 0x%02x\n", pIpProt->m_Content.m_CmdContent.m_ResponseInfo.m_RetCode);
			GS_COMM_Printf("pIpProt->m_Content.m_CmdContent.m_ResponseInfo.m_DescLen: 0x%02x\n", pIpProt->m_Content.m_CmdContent.m_ResponseInfo.m_DescLen);
			GS_COMM_PrintDataBlock("pIpProt->m_Content.m_CmdContent.m_ResponseInfo.m_pDesc", (GS_U8 *)pIpProt->m_Content.m_CmdContent.m_ResponseInfo.m_pDesc, pIpProt->m_Content.m_CmdContent.m_ResponseInfo.m_DescLen);			
		}
		GS_COMM_Printf("pIpProt->m_CheckData.m_Crc32: 0x%x", pIpProt->m_CheckData.m_Crc32);
		GS_COMM_Printf("\n=================== IP_PROT Packet End =====================\n");
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
