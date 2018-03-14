#include "gs_em_ip_prot.h"
#include "gs_dev_if.h"
#include "gs_tcp.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

GS_DEBUG_SET_LEVEL(GS_DBG_LVL_DBG); 
#define DEV_HEARTBEAT_TIMEOUT (30)
#define DEV_HEARTBEAT_INTERVAL (10)
#define DEV_TCP_CONN_CHECK_INTERVAL (10)
#define DEV_AUD_RTP_DEFAULT_RECV_PORT (8888) /* 音频 RTP 播发默认的接收端口 */

typedef struct {
	GS_EM_DEV_IF_INIT_PARAM_S	m_InitParam;

	GS_BOOL						m_IsRun;
	pthread_t 					m_ThreadID;
	pthread_t					m_ApplyParamThreadID;
	pthread_t					m_PostBackThreadID;

	GS_HANDLE					m_TcpSndMsgQ; /* 发送消息队列 */
	GS_HANDLE					m_TcpRecvMsgQ; /* 接收消息队列 */

	GS_HANDLE					m_TcpHandle; /* TCP 处理对象 */

	GS_U32						m_SessionId; /* 会话 ID，作为发送只维护一个 */
} GS_EM_DEV_IF_S;

static GS_EM_DEV_IF_S *s_pDevIfHandle = NULL;

/* ---转换函数部分，用于用户接口与协议接口的转换--- */
static GS_EM_DEV_AUD_TRANS_PROT_TYPE_E GS_DEV_GetDevAudTransProtFromIpProt(GS_EM_IP_PROT_AUD_TRANS_PROT_TYPE_E AudTransProtType)
{
	switch (AudTransProtType) {
		case  GS_EM_IP_PROT_AUD_TRANS_PROT_RTSP:
			return AUD_TRANS_PROT_RTSP;
		case  GS_EM_IP_PROT_AUD_TRANS_PROT_RTMP:
			return AUD_TRANS_PROT_RTMP;
		case  GS_EM_IP_PROT_AUD_TRANS_PROT_RTP:
			return AUD_TRANS_PROT_RTP;
		default:
			return AUD_TRANS_PROT_BUTT;
	}
}

static GS_EM_DEV_AUD_ENC_FMT_E GS_DEV_GetDevAudEncFmtFromIpProt(GS_EM_IP_PROT_AUD_ENC_FMT_E AudEncFmt)
{
	switch (AudEncFmt) {
		case  GS_EM_IP_PROT_AUD_ENC_FMT_MPEG1_L2:
			return AUD_ENC_FMT_MPEG1_L2;
		case  GS_EM_IP_PROT_AUD_ENC_FMT_AAC:
			return AUD_ENC_FMT_AAC;
		case  GS_EM_IP_PROT_AUD_ENC_FMT_MP3:
			return AUD_ENC_FMT_MP3;
		default:
			return AUD_ENC_FMT_BUTT;
	}
}
/* ---转换函数部分结束--- */

#define GS_DEV_REGISTER_FILE "register_info.cfg"
/* 是否第一次注册，第一次注册返回 TRUE，否则返回 FALSE */
static GS_BOOL GS_DEV_IsFirstRegister(GS_EM_DEV_NET_ADDR_S *pServAddr)
{
	FILE *plFile;
	GS_CHAR plTmpBuf[256];
	GS_CHAR plTmpStr[256];
	GS_CHAR plPath[256];
	GS_BOOL lRet = GS_TRUE;

	s_pDevIfHandle->m_InitParam.m_GS_EM_DevGetStorePath(plPath);
	sprintf (plTmpStr, "%s/%s", plPath, GS_DEV_REGISTER_FILE);
	if ((plFile = fopen(plTmpStr, "r"))) {
		if (fgets(plTmpBuf, sizeof(plTmpBuf), plFile)) {
			if (pServAddr->m_Type == 0x01) { /* ip */
				sprintf (plTmpStr, "0x%x", pServAddr->m_HostName.m_IpAddr);
				if (strstr(plTmpBuf, plTmpStr)) {
					sprintf (plTmpStr, "%d", pServAddr->m_Port);
					if (strstr(plTmpBuf, plTmpStr)) {
						GS_DBGWRN("Has Register!\n");
						lRet = GS_FALSE;
					}
				}
			}
			else {
				if (strstr(plTmpBuf, pServAddr->m_HostName.m_pDomain)) {
					sprintf (plTmpStr, "%d", pServAddr->m_Port);
					if (strstr(plTmpBuf, plTmpStr)) {
						lRet = GS_FALSE;
					}
				}
			}
		}

		fclose(plFile);
		return lRet;
	}

	GS_DBGWRN("First Register!\n");
	return lRet;
}

static GS_VOID GS_DEV_SetFirstRegister(GS_EM_DEV_NET_ADDR_S *pServAddr, GS_BOOL IsFirst)
{
	GS_CHAR plTmpStr[256];
	GS_CHAR plPath[256];

	s_pDevIfHandle->m_InitParam.m_GS_EM_DevGetStorePath(plPath);
	if (IsFirst) { /* 设置首次注册，即检测是没有注册，所以没有注册文件 */
		sprintf(plTmpStr, "rm -f %s/%s", plPath, GS_DEV_REGISTER_FILE);
		system(plTmpStr);
	}
	else { /* 写注册文件 */
		if (pServAddr->m_Type == 0x01)
			sprintf(plTmpStr, "echo \"0x%x, %d\" > %s/%s", pServAddr->m_HostName.m_IpAddr, pServAddr->m_Port, plPath, GS_DEV_REGISTER_FILE);
		else
			sprintf(plTmpStr, "echo \"%s, %d\" > %s/%s", pServAddr->m_HostName.m_pDomain, pServAddr->m_Port, plPath, GS_DEV_REGISTER_FILE);

		system(plTmpStr);
	}
}

static GS_VOID *GS_PostBackThread(GS_VOID *pParam)
{
	GS_EM_DEV_IF_S *plHandle = (GS_EM_DEV_IF_S *)pParam;
	GS_EM_DEV_POSTBACK_SERV_S lPostBackInfo;

	while (1) {
		pthread_testcancel();
	
		if (plHandle->m_InitParam.m_GS_EM_DevWaitPostBackService(&lPostBackInfo) == GS_SUCCESS) {
			// 发送消息到发送线程 
			if (GS_TCP_GetConnStat(s_pDevIfHandle->m_TcpHandle) == GS_TRUE) {
				GS_S32 lLen;
				GS_MSGBUF_S lMsgQBuf;
				GS_EM_DEV_SYS_STAT_S lSysStat;

				lMsgQBuf.m_MsgText.m_pMsgData = (GS_U8 *)malloc(GS_EM_IP_PROT_MAX_LEN);
				if (!lMsgQBuf.m_MsgText.m_pMsgData)
					GS_COMM_ErrSys("malloc error");

				if (plHandle->m_InitParam.m_GS_EM_DevStatGet(DEV_MODULE_SYS, &lSysStat) == GS_FAILURE) {
					GS_COMM_ErrMsg("GS_EM_DevStatGet Error!");
					return NULL;
				}
				switch (lPostBackInfo.m_ServTag) {
					case DEV_POSTBACK_SERV_AUD_TRANS_START:
						{
							GS_EM_IP_PROT_AUD_TRANS_START_S lAudTransStartInfo;
							GS_S32 lRet;

							lAudTransStartInfo.m_CertID = GS_COMM_Crc32Calc(lSysStat.m_pPhyId, 18);
							lAudTransStartInfo.m_SpotsObjNum = 0;
							lAudTransStartInfo.m_BCType = IP_PROT_BC_NORM;
							lAudTransStartInfo.m_AudTransProt = lPostBackInfo.m_ServParam.m_AudTransStart.m_TransProt;
							lAudTransStartInfo.m_AudEncFmt = lPostBackInfo.m_ServParam.m_AudTransStart.m_EncFmt;
							lAudTransStartInfo.m_BCAddr.m_Type = lPostBackInfo.m_ServParam.m_AudTransStart.m_PostBackAddr.m_Type;
							if (lAudTransStartInfo.m_BCAddr.m_Type == GS_EM_IP_PROT_HOSTNAME_TYPE_IP) {
								lRet = snprintf (lAudTransStartInfo.m_BCAddr.m_HostName.m_pDomain, 64, "%s", lPostBackInfo.m_ServParam.m_AudTransStart.m_PostBackAddr.m_HostName.m_pDomain);
								lAudTransStartInfo.m_BCAddrLen = 3 + lRet;
							}
							else {
								lAudTransStartInfo.m_BCAddr.m_HostName.m_IpAddr = lPostBackInfo.m_ServParam.m_AudTransStart.m_PostBackAddr.m_HostName.m_IpAddr;
								lAudTransStartInfo.m_BCAddrLen = 7;
							}					
							lAudTransStartInfo.m_BCAddr.m_Port = lPostBackInfo.m_ServParam.m_AudTransStart.m_PostBackAddr.m_Port;
							GS_EM_IP_PROT_CmdInfoPack(lMsgQBuf.m_MsgText.m_pMsgData, &lLen, &lAudTransStartInfo, IP_PROT_CMD_AUD_TRANS_START, s_pDevIfHandle->m_SessionId++);
						}
						break;
					case DEV_POSTBACK_SERV_AUD_TRANS_END:
						{
							GS_EM_IP_PROT_AUD_TRANS_END_S lAudTransEndInfo;

							lAudTransEndInfo.m_CertID = GS_COMM_Crc32Calc(lSysStat.m_pPhyId, 18);
							lAudTransEndInfo.m_SpotsObjNum = 0;
							GS_EM_IP_PROT_CmdInfoPack(lMsgQBuf.m_MsgText.m_pMsgData, &lLen, &lAudTransEndInfo, IP_PROT_CMD_AUD_TRANS_END, s_pDevIfHandle->m_SessionId++);
						}
						break;
					default:
						break;
				}

				lMsgQBuf.m_MsgType = TCP_SEND_MSG_TYPE;
				lMsgQBuf.m_MsgText.m_MsgTag = TCP_SEND_MSG_TAG_POSTBACK;
				lMsgQBuf.m_MsgText.m_MsgParam1 = 0; 
				lMsgQBuf.m_MsgText.m_MsgParam2 = 0; 
				lMsgQBuf.m_MsgText.m_MsgDataLen = lLen;

				if (GS_COMM_MsgQueueSend(s_pDevIfHandle->m_TcpSndMsgQ, &lMsgQBuf, GS_FALSE) == GS_FAILURE) { /* 消息队列满 */ 
					GS_DBGWRN("MsgQueue OverFlow!!!\n");
					free (lMsgQBuf.m_MsgText.m_pMsgData);
				}
			}
		}
	}

	return NULL;
}

/* 设备处理线程 */
static GS_VOID *GS_ApplyParamThread(GS_VOID *pParam)
{
	GS_EM_DEV_IF_S *plHandle = (GS_EM_DEV_IF_S *)pParam;
	GS_MSGBUF_S lMsgQBuf;
	GS_S32 i;

	while (1) 
	{
		pthread_testcancel();
		if (GS_COMM_MsgQueueRecv(plHandle->m_TcpRecvMsgQ, &lMsgQBuf, TCP_RECV_MSG_TYPE_DEV_DATA, GS_TRUE) == GS_SUCCESS) {
			GS_EM_IP_PROT_S *plIpProt = (GS_EM_IP_PROT_S *)lMsgQBuf.m_MsgText.m_pMsgData;

			switch (plIpProt->m_Content.m_CmdType)
			{
			case IP_PROT_CMD_START_BC:
				{ // 设置音量
					GS_EM_DEV_SYS_PARAM_S lSysParam;

					lSysParam.m_Param.m_Vol = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_Vol;
					lSysParam.m_Tag = SYS_PARAM_VOL;
					plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_SYS, &lSysParam);
				}
				for (i = 0; i < plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_AssistDataNum; i++) {					
					switch (plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Type)
					{
					case GS_EM_IP_PROT_ASSIST_DATA_TYPE_TEXT:
						{ // 文本处理
							GS_EM_DEV_TTS_PARAM_S lTtsParam;

							lTtsParam.m_Index = 0;
							lTtsParam.m_Speed = 5;
							lTtsParam.m_Tune = 5;
							lTtsParam.m_Volume = 10;
							lTtsParam.m_TextEncFmt = TTS_TEXT_ENC_FMT_GB2312;
							lTtsParam.m_TextLen = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Len;
							memcpy (lTtsParam.m_pText, plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_pText, lTtsParam.m_TextLen);
							GS_COMM_PrintDataBlock("Text Command Data", (GS_U8 *)lTtsParam.m_pText, lTtsParam.m_TextLen);
							plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_TTS, &lTtsParam);
						}
						break;
					case GS_EM_IP_PROT_ASSIST_DATA_TYPE_AUD:
						// 音频处理
						if (plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudTransProt == GS_EM_IP_PROT_AUD_TRANS_PROT_RTP) {
							GS_DBGINFO("Audio Command: TransFmt[%d] EncFmt[%d]\n", plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudTransProt, 
								plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudEncFmt);
						}
						else {
							GS_DBGINFO("Audio Command: TransFmt[%d] EncFmt[%d] IP[0x%X] PORT[%d]\n", plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudTransProt, 
								plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudEncFmt, 
								plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_Addr.m_HostName.m_IpAddr, 
								plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_Addr.m_Port);
						}
						{
							GS_EM_DEV_PSI_INFO_S lPsiInfo;
							GS_EM_DEV_BC_PARAM_S lBcParam;
							GS_EM_DEV_DECODER_PARAM_S lDecoderParam;
							GS_EM_DEV_SYS_STAT_S lSysStat;

							plHandle->m_InitParam.m_GS_EM_DevStatGet(DEV_MODULE_SYS, &lSysStat);
							lBcParam.m_Type = BC_PARAM_TYPE_AUD;
							lBcParam.m_Len = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Len;
							lBcParam.m_Data.m_AudInfo.m_AudTransProt = GS_DEV_GetDevAudTransProtFromIpProt(plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudTransProt);
							lBcParam.m_Data.m_AudInfo.m_AudEncFmt = GS_DEV_GetDevAudEncFmtFromIpProt(plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_AudEncFmt);
							if (lBcParam.m_Data.m_AudInfo.m_AudTransProt == AUD_TRANS_PROT_RTP) { /* RTP 单播模式不会传送地址，所以这里的地址使用本地 IP + 默认端口 */
								lBcParam.m_Data.m_AudInfo.m_Addr.m_Type = 0x01; /* IP + PORT */
								lBcParam.m_Data.m_AudInfo.m_Addr.m_HostName.m_IpAddr = lSysStat.m_LocalIp.m_IpAddr;
								lBcParam.m_Data.m_AudInfo.m_Addr.m_Port = DEV_AUD_RTP_DEFAULT_RECV_PORT;
							}
							else {
								lBcParam.m_Data.m_AudInfo.m_Addr.m_Type = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_Addr.m_Type;
								lBcParam.m_Data.m_AudInfo.m_Addr.m_HostName.m_IpAddr = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_Addr.m_HostName.m_IpAddr;
								lBcParam.m_Data.m_AudInfo.m_Addr.m_Port = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_AudInfo.m_Addr.m_Port;
							}
							plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_BC_PARAM, &lBcParam);
							if (plHandle->m_InitParam.m_GS_EM_DevGetPsiInfo(DEV_UDP_RECV_CHAN_IP0, &lPsiInfo) == GS_SUCCESS)
							{
								lDecoderParam.m_Index = 0;
								lDecoderParam.m_DecoderIndex = 0;
								lDecoderParam.m_AudioVolume = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_Vol / 10;
								lDecoderParam.m_SoundChannel = DECODER_SOUNDCHN_STEREO;
								lDecoderParam.m_VideoOutputMode = lPsiInfo.m_VideoMode;
								lDecoderParam.m_VideoOutputStandard = lPsiInfo.m_VideoStandard;
								lDecoderParam.m_AudioPid = lPsiInfo.m_AudPid;
								lDecoderParam.m_PmtPid = lPsiInfo.m_PmtPid;
								lDecoderParam.m_ServiceId = lPsiInfo.m_ServiceId;

								plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_DECODER, &lDecoderParam);
							}
						}
						break;
					case GS_EM_IP_PROT_ASSIST_DATA_TYPE_PIC:
						// 图片处理
						GS_DBGINFO("Pic Command: IP[0x%X] PORT[%d]\n", 
							plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_PicInfo.m_Addr.m_HostName.m_IpAddr, 
							plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_PicInfo.m_Addr.m_Port);
						break;
					case GS_EM_IP_PROT_ASSIST_DATA_TYPE_VID:
						// 视频处理
						GS_DBGINFO("Video Command: IP[0x%X] PORT[%d]\n", 
							plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_VidInfo.m_Addr.m_HostName.m_IpAddr, 
							plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_VidInfo.m_Addr.m_Port);
						{
							GS_EM_DEV_PSI_INFO_S lPsiInfo;
							GS_EM_DEV_BC_PARAM_S lBcParam;
							GS_EM_DEV_DECODER_PARAM_S lDecoderParam;

							lBcParam.m_Type = BC_PARAM_TYPE_VID;
							lBcParam.m_Len = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Len;
							lBcParam.m_Data.m_VidInfo.m_Type = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_VidInfo.m_Addr.m_Type;
							lBcParam.m_Data.m_VidInfo.m_HostName.m_IpAddr = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_VidInfo.m_Addr.m_HostName.m_IpAddr;
							lBcParam.m_Data.m_VidInfo.m_Port = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_pAssistData[i].m_Data.m_VidInfo.m_Addr.m_Port;
							plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_BC_PARAM, &lBcParam);
							if (plHandle->m_InitParam.m_GS_EM_DevGetPsiInfo(DEV_UDP_RECV_CHAN_IP0, &lPsiInfo) == GS_SUCCESS)
							{
								lDecoderParam.m_Index = 0;
								lDecoderParam.m_DecoderIndex = 0;
								lDecoderParam.m_AudioVolume = plIpProt->m_Content.m_CmdContent.m_StartBCInfo.m_Vol / 10;
								lDecoderParam.m_SoundChannel = DECODER_SOUNDCHN_STEREO;
								lDecoderParam.m_VideoOutputMode = lPsiInfo.m_VideoMode;
								lDecoderParam.m_VideoOutputStandard = lPsiInfo.m_VideoStandard;
								lDecoderParam.m_AudioPid = lPsiInfo.m_AudPid;
								lDecoderParam.m_PmtPid = lPsiInfo.m_PmtPid;
								lDecoderParam.m_ServiceId = lPsiInfo.m_ServiceId;

								plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_DECODER, &lDecoderParam);
							}
						}
						break;
					default:
						break;
					}
				}
				break;
			case IP_PROT_CMD_STOP_BC:
				// 停止播发
				GS_DBGINFO("Stop BC!!\n");
				break;
			case IP_PROT_CMD_STAT_QUERY:
				break;
			case IP_PROT_CMD_DEV_PARAM_SET:
				for (i = 0; i < plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_ParamSetNum; i++) {
					switch (plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Tag) 
					{
					case IP_PROT_DEV_PARAM_TAG_VOL:
						// 音量设置
						GS_DBGINFO("Param Set: Vol[%d]!!\n", plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_Vol);
						{ 
							GS_EM_DEV_SYS_PARAM_S lSysParam;

							lSysParam.m_Param.m_Vol = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_Vol;
							lSysParam.m_Tag = SYS_PARAM_VOL;
							plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_SYS, &lSysParam);
						}
						break;
					case IP_PROT_DEV_PARAM_TAG_IPV4:
						// 本地 IP 设置
						GS_DBGINFO("Param Set: LocalIp[0x%x] Mask[0x%x] Gate[0x%x]!!\n", 
							plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpAddr, 
							plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpMask,
							plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpGate);
						{ 
							GS_EM_DEV_SYS_PARAM_S lSysParam;

							lSysParam.m_Param.m_LocalIp.m_IpAddr = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpAddr;
							lSysParam.m_Param.m_LocalIp.m_IpMask = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpMask;
							lSysParam.m_Param.m_LocalIp.m_IpGate = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_LocalIp.m_IpGate;
							lSysParam.m_Tag = SYS_PARAM_LOCALIP;
							plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_SYS, &lSysParam);
						}
						break;
					case IP_PROT_DEV_PARAM_TAG_RTP_POSTBACK_ADDR:
						// RTP 回传设置
						GS_DBGINFO("Param Set: PostBackAddr[0x%x - %d]!!\n", 
							plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_PostBackAddrInfo.m_HostName.m_IpAddr, 
							plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_PostBackAddrInfo.m_Port);
						{ 
							GS_EM_DEV_SYS_PARAM_S lSysParam;

							lSysParam.m_Param.m_AudPostBack.m_Type = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_PostBackAddrInfo.m_Type;
							lSysParam.m_Param.m_AudPostBack.m_HostName.m_IpAddr = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_PostBackAddrInfo.m_HostName.m_IpAddr;
							lSysParam.m_Param.m_AudPostBack.m_Port = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_PostBackAddrInfo.m_Port;
							lSysParam.m_Tag = SYS_PARAM_AUD_POSTBACK;
							plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_SYS, &lSysParam);
						}
						break;
					case IP_PROT_DEV_PARAM_TAG_SERVER_TCP_ADDR:
						// 服务器地址
						GS_DBGINFO("Param Set: ServerTcpAddr[0x%x - %d]!!\n", 
							plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_ServerTcpAddr.m_HostName.m_IpAddr, 
							plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_ServerTcpAddr.m_Port);
						{ 
							GS_EM_DEV_SYS_PARAM_S lSysParam;

							lSysParam.m_Param.m_ServerTcpAddr.m_Type = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_ServerTcpAddr.m_Type;
							lSysParam.m_Param.m_ServerTcpAddr.m_HostName.m_IpAddr = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_ServerTcpAddr.m_HostName.m_IpAddr;
							lSysParam.m_Param.m_ServerTcpAddr.m_Port = plIpProt->m_Content.m_CmdContent.m_ParamSetInfo.m_pParamData[i].m_Param.m_ServerTcpAddr.m_Port;
							lSysParam.m_Tag = SYS_PARAM_SERVER_TCP_ADDR;
							plHandle->m_InitParam.m_GS_EM_DevParamSet(DEV_MODULE_SYS, &lSysParam);
						}
						break;
					case IP_PROT_DEV_PARAM_TAG_DEV_RES_CODE:
						// 设备资源编码设置
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
				break;
			case IP_PROT_CMD_AUD_TRANS_END:
				break;
			default:
				break;
			}

			free(lMsgQBuf.m_MsgText.m_pMsgData); /* 释放小时数据内存 */
			GS_EM_IP_PROT_ParseClean((GS_HANDLE)lMsgQBuf.m_MsgText.m_MsgParam1); /* 清除解析时使用的内存 */
		}
	}

	return GS_NULL;
}

GS_VOID *GS_ProtProcThread(void *pParam)
{
	GS_EM_DEV_IF_S *plHandle = (GS_EM_DEV_IF_S *)pParam;
	GS_TCP_INIT_PARAM_S lInitParam;
	GS_EM_DEV_SYS_STAT_S lSysStat;
	GS_VOID *plTRet; /* 线程返回值 */
	GS_S32 lErrNo;

	if (plHandle->m_InitParam.m_GS_EM_DevStatGet(DEV_MODULE_SYS, &lSysStat) == GS_FAILURE) {
		GS_COMM_ErrMsg("GS_EM_DevStatGet Error!");
		return NULL;
	}
	
	if ((s_pDevIfHandle->m_TcpSndMsgQ = GS_COMM_MsgQueueCreate(16 * sizeof(GS_MSGBUF_S))) == NULL) {
		GS_COMM_ErrQuit("GS_COMM_MsgQueueCreate error");
	}

	if ((s_pDevIfHandle->m_TcpRecvMsgQ = GS_COMM_MsgQueueCreate(16 * sizeof(GS_MSGBUF_S))) == NULL) {
		GS_COMM_ErrQuit("GS_COMM_MsgQueueCreate error");
	}
	
	lInitParam.m_ServAddr.m_Type = lSysStat.m_ServerAddr.m_Type;
	if (lSysStat.m_ServerAddr.m_Type == GS_EM_IP_PROT_HOSTNAME_TYPE_IP)
		lInitParam.m_ServAddr.m_HostName.m_IpAddr = lSysStat.m_ServerAddr.m_HostName.m_IpAddr;
	else
		memcpy (lInitParam.m_ServAddr.m_HostName.m_pDomain, lSysStat.m_ServerAddr.m_HostName.m_pDomain, sizeof(lSysStat.m_ServerAddr.m_HostName.m_pDomain));
	lInitParam.m_ServAddr.m_Port = lSysStat.m_ServerAddr.m_Port;
	lInitParam.m_TcpRecvMsgQ = s_pDevIfHandle->m_TcpRecvMsgQ;
	lInitParam.m_TcpSndMsgQ = s_pDevIfHandle->m_TcpSndMsgQ;
	if ((s_pDevIfHandle->m_TcpHandle = GS_TCP_Create(&lInitParam)) == NULL) {
		GS_COMM_MsgQueueDestroy(s_pDevIfHandle->m_TcpSndMsgQ);
		GS_COMM_MsgQueueDestroy(s_pDevIfHandle->m_TcpRecvMsgQ);
		GS_COMM_ErrQuit("GS_TCP_Create error");
	}

	if ((lErrNo = pthread_create(&s_pDevIfHandle->m_ApplyParamThreadID, NULL, GS_ApplyParamThread, plHandle)) != 0) {
		GS_COMM_MsgQueueDestroy(s_pDevIfHandle->m_TcpSndMsgQ);
		GS_COMM_MsgQueueDestroy(s_pDevIfHandle->m_TcpRecvMsgQ);
		GS_COMM_ErrExit(lErrNo, "pthread_create error!");
	}

	if ((lErrNo = pthread_create(&s_pDevIfHandle->m_PostBackThreadID, NULL, GS_PostBackThread, plHandle)) != 0) {
		GS_COMM_MsgQueueDestroy(s_pDevIfHandle->m_TcpSndMsgQ);
		GS_COMM_MsgQueueDestroy(s_pDevIfHandle->m_TcpRecvMsgQ);
		GS_COMM_ErrExit(lErrNo, "pthread_create error!");
	}
	
	while (s_pDevIfHandle->m_IsRun) {
		pthread_testcancel();
		if (GS_TCP_GetConnStat(s_pDevIfHandle->m_TcpHandle) == GS_TRUE) {
			GS_S32 lLen;
			GS_EM_IP_PROT_DEV_HEARTBEAT_S lDevHeartBeatInfo;
			GS_MSGBUF_S lMsgQBuf;
			GS_S32 lTimeOut = DEV_HEARTBEAT_TIMEOUT;

			lMsgQBuf.m_MsgText.m_pMsgData = (GS_U8 *)malloc(GS_EM_IP_PROT_MAX_LEN);
			if (!lMsgQBuf.m_MsgText.m_pMsgData)
				GS_COMM_ErrSys("malloc error");

			plHandle->m_InitParam.m_GS_EM_DevStatGet(DEV_MODULE_SYS, &lSysStat);
			if (lSysStat.m_WorkStat == DEV_WORK_STAT_FREE) {
				lDevHeartBeatInfo.m_DevWorkStat = IP_PROT_DEV_WORK_STAT_FREE;
			}
			else if (lSysStat.m_WorkStat == DEV_WORK_STAT_BUSY) {
				lDevHeartBeatInfo.m_DevWorkStat = IP_PROT_DEV_WORK_STAT_RUNNING;
			}
			else {
				lDevHeartBeatInfo.m_DevWorkStat = IP_PROT_DEV_WORK_STAT_FAULT;
			}
			lDevHeartBeatInfo.m_FirstRegister = ((GS_DEV_IsFirstRegister(&lSysStat.m_ServerAddr) == GS_TRUE) ? 0x01 : 0x02);
			lDevHeartBeatInfo.m_PhyAddrCodeLen = 18;
			lDevHeartBeatInfo.m_pPhyAddrCode = lSysStat.m_pPhyId;
			GS_EM_IP_PROT_CmdInfoPack(lMsgQBuf.m_MsgText.m_pMsgData, &lLen, &lDevHeartBeatInfo, IP_PROT_CMD_DEV_HEARTBEAT, s_pDevIfHandle->m_SessionId++);

			lMsgQBuf.m_MsgType = TCP_SEND_MSG_TYPE;
			lMsgQBuf.m_MsgText.m_MsgTag = TCP_SEND_MSG_TAG_HEARTBEAT;
			lMsgQBuf.m_MsgText.m_MsgParam1 = 0; 
			lMsgQBuf.m_MsgText.m_MsgParam2 = 0; 
			lMsgQBuf.m_MsgText.m_MsgDataLen = lLen;

			if (GS_COMM_MsgQueueSend(s_pDevIfHandle->m_TcpSndMsgQ, &lMsgQBuf, GS_FALSE) == GS_FAILURE) { /* 消息队列满 */ 
				GS_DBGERR("MsgQueue OverFlow!!!\n");
				free (lMsgQBuf.m_MsgText.m_pMsgData);
				sleep(DEV_HEARTBEAT_INTERVAL);
				continue;
			}

			while (lTimeOut --) {
				if (GS_COMM_MsgQueueRecv(s_pDevIfHandle->m_TcpRecvMsgQ, &lMsgQBuf, TCP_RECV_MSG_TYPE_HEARTBEAT_RESPONSE, GS_FALSE) == GS_SUCCESS) {
					GS_DBGINFO("HeartBeat Response OK!\n");
					if (GS_DEV_IsFirstRegister(&lSysStat.m_ServerAddr) == GS_TRUE) 
						GS_DEV_SetFirstRegister(&lSysStat.m_ServerAddr, GS_FALSE);
					GS_EM_IP_PROT_ParseClean((GS_HANDLE)lMsgQBuf.m_MsgText.m_MsgParam1); /* 清除解析时使用的内存 */
					free (lMsgQBuf.m_MsgText.m_pMsgData);
					break;
				}
				sleep(1);	
			}

			if (lTimeOut <= 0) { /* 心跳应答超时 */
				GS_DBGWRN("HeartBeat Response TimeOut!\n");
				GS_TCP_SetConnStat(plHandle->m_TcpHandle, GS_FALSE);
				continue;
			}

			sleep(DEV_HEARTBEAT_INTERVAL);
		}
		else {
			sleep(DEV_TCP_CONN_CHECK_INTERVAL);	
		}
	}

	if ((lErrNo = pthread_cancel(plHandle->m_ApplyParamThreadID)) != 0)
		GS_COMM_ErrExit(lErrNo, "pthread_cancel error");
	if ((lErrNo = pthread_cancel(plHandle->m_PostBackThreadID)) != 0)
		GS_COMM_ErrExit(lErrNo, "pthread_cancel error");

	/* 等待线程结束 */
	if ((lErrNo = pthread_join(plHandle->m_ApplyParamThreadID, &plTRet)) != 0)
		GS_COMM_ErrExit(lErrNo, "pthread_join error");

	if ((lErrNo = pthread_join(plHandle->m_PostBackThreadID, &plTRet)) != 0)
		GS_COMM_ErrExit(lErrNo, "pthread_join error");

	GS_TCP_Destroy(s_pDevIfHandle->m_TcpHandle);
	GS_COMM_MsgQueueDestroy(s_pDevIfHandle->m_TcpSndMsgQ);
	GS_COMM_MsgQueueDestroy(s_pDevIfHandle->m_TcpRecvMsgQ);
	
	return 0;
}

GS_S32 GS_EM_ProtProcTaskCreate(GS_EM_DEV_IF_INIT_PARAM_S *pInitParam) 
{
	GS_S32 lErrNo;

	if (!pInitParam)
		return GS_FAILURE;

	if (s_pDevIfHandle) {
		GS_COMM_ErrMsg("GS_EM_ProtProcTaskCreate Only Create Once!");
		return GS_FAILURE;
	}
	if ((s_pDevIfHandle = (GS_EM_DEV_IF_S *)malloc(sizeof(GS_EM_DEV_IF_S))) == NULL) {
		GS_COMM_ErrSys("malloc error!\n");
	}
	bzero(s_pDevIfHandle, sizeof(GS_EM_DEV_IF_S));

	memcpy (&s_pDevIfHandle->m_InitParam, pInitParam, sizeof(GS_EM_DEV_IF_INIT_PARAM_S));
	s_pDevIfHandle->m_IsRun = GS_TRUE;

	if ((lErrNo = pthread_create(&s_pDevIfHandle->m_ThreadID, NULL, GS_ProtProcThread, s_pDevIfHandle)) != 0)
		GS_COMM_ErrExit(lErrNo, "pthread_create error!");
	
	return GS_SUCCESS;
}

GS_VOID GS_EM_ProtProcTaskDestroy(GS_VOID)
{
	GS_S32 lErrNo;
	GS_VOID *plTRet; /* 线程返回值 */

	if (s_pDevIfHandle) {
		s_pDevIfHandle->m_IsRun = GS_FALSE; /* 退出线程 */

		/* 等待线程结束 */
		if ((lErrNo = pthread_join(s_pDevIfHandle->m_ThreadID, &plTRet)) != 0)
			GS_COMM_ErrExit(lErrNo, "pthread_join error");

		free(s_pDevIfHandle);
		s_pDevIfHandle = NULL;
	}
}

GS_VOID GS_EM_RemoteReConnect(GS_VOID)
{
	if (s_pDevIfHandle) {
		GS_EM_DEV_SYS_STAT_S lSysStat;
		GS_EM_IP_PROT_NET_ADDR_S lServerAddr;

		if (s_pDevIfHandle->m_InitParam.m_GS_EM_DevStatGet(DEV_MODULE_SYS, &lSysStat) == GS_FAILURE) {
			GS_COMM_ErrMsg("GS_EM_DevStatGet Error!");
			return;
		}

		lServerAddr.m_Type = lSysStat.m_ServerAddr.m_Type;
		if (lSysStat.m_ServerAddr.m_Type == GS_EM_IP_PROT_HOSTNAME_TYPE_IP)
			lServerAddr.m_HostName.m_IpAddr = lSysStat.m_ServerAddr.m_HostName.m_IpAddr;
		else
			memcpy (lServerAddr.m_HostName.m_pDomain, lSysStat.m_ServerAddr.m_HostName.m_pDomain, sizeof(lSysStat.m_ServerAddr.m_HostName.m_pDomain));
		lServerAddr.m_Port = lSysStat.m_ServerAddr.m_Port;
		GS_TCP_ReconfigServerAddr(s_pDevIfHandle->m_TcpHandle, &lServerAddr);

		GS_TCP_SetConnStat(s_pDevIfHandle->m_TcpHandle, GS_FALSE); /* 触发重连 */
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

