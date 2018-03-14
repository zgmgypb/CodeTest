#include <pthread.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/wait.h>
#include "../prot_proc/gs_tcp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

GS_DEBUG_SET_LEVEL(GS_DBG_LVL_DBG); 

/* ·þÎñÆ÷µØÖ·ºÍ¶Ë¿Ú */
#define SERVER_IP (0xC0A803F9) 
#define SERVER_PORT (9001)
#define SERVER_MAX_LISTENQ (16)
#define TCP_MTU (1500)

typedef struct {
	GS_S32 m_SockFd;
	struct sockaddr_in *m_pClientAddr;
} GS_TCP_SERVER_HANDLE_T;

static GS_VOID GS_TCP_ConstructAudRequestEndData(GS_U8 *pData, GS_S32 *pLen, GS_S32 DataType, GS_TCP_SERVER_HANDLE_T *pHandle)
{
	GS_EM_IP_PROT_AUD_TRANS_END_S lAudTransEndInfo;

	GS_DBGWHERE();
	bzero(&lAudTransEndInfo, sizeof(GS_EM_IP_PROT_AUD_TRANS_END_S));
	lAudTransEndInfo.m_CertID = 0;
	lAudTransEndInfo.m_SpotsObjNum = 0;

	GS_EM_IP_PROT_CmdInfoPack(pData, pLen, &lAudTransEndInfo, IP_PROT_CMD_AUD_TRANS_END, 0);
}

static GS_VOID GS_TCP_ConstructAudRequestStartData(GS_U8 *pData, GS_S32 *pLen, GS_S32 DataType, GS_TCP_SERVER_HANDLE_T *pHandle)
{
	GS_EM_IP_PROT_AUD_TRANS_START_S lAudTransStartInfo;

	GS_DBGWHERE();
	bzero(&lAudTransStartInfo, sizeof(GS_EM_IP_PROT_AUD_TRANS_START_S));
	lAudTransStartInfo.m_CertID = 0;
	lAudTransStartInfo.m_SpotsObjNum = 0;
	lAudTransStartInfo.m_BCType = IP_PROT_BC_NORM;
	lAudTransStartInfo.m_AudTransProt = GS_EM_IP_PROT_AUD_TRANS_PROT_RTP;
	lAudTransStartInfo.m_AudEncFmt = GS_EM_IP_PROT_AUD_ENC_FMT_MP3;
	lAudTransStartInfo.m_BCAddrLen = 7;
	lAudTransStartInfo.m_BCAddr.m_Type = GS_EM_IP_PROT_HOSTNAME_TYPE_IP;
	lAudTransStartInfo.m_BCAddr.m_HostName.m_IpAddr = 0x78787877;
	lAudTransStartInfo.m_BCAddr.m_Port = 9001;

	GS_EM_IP_PROT_CmdInfoPack(pData, pLen, &lAudTransStartInfo, IP_PROT_CMD_AUD_TRANS_START, 0);
}

static GS_VOID GS_TCP_ConstructParamSetData(GS_U8 *pData, GS_S32 *pLen, GS_S32 DataType, GS_TCP_SERVER_HANDLE_T *pHandle)
{
	GS_EM_IP_PROT_DEV_PARAM_SET_S lDevParamSetInfo;

	GS_DBGWHERE();
	bzero(&lDevParamSetInfo, sizeof(GS_EM_IP_PROT_DEV_PARAM_SET_S));
	lDevParamSetInfo.m_ParamSetNum = 4;
	lDevParamSetInfo.m_pParamData = (GS_EM_IP_PROT_DEV_PARAM_S *)malloc(lDevParamSetInfo.m_ParamSetNum * sizeof(GS_EM_IP_PROT_DEV_PARAM_S));
	lDevParamSetInfo.m_pParamData[0].m_Tag = IP_PROT_DEV_PARAM_TAG_VOL;
	lDevParamSetInfo.m_pParamData[0].m_ParamLen = 1;
	lDevParamSetInfo.m_pParamData[0].m_Param.m_Vol = 0x64;
	lDevParamSetInfo.m_pParamData[1].m_Tag = IP_PROT_DEV_PARAM_TAG_IPV4;
	lDevParamSetInfo.m_pParamData[1].m_ParamLen = 12;
	lDevParamSetInfo.m_pParamData[1].m_Param.m_LocalIp.m_IpAddr = ntohl(pHandle->m_pClientAddr->sin_addr.s_addr);
	lDevParamSetInfo.m_pParamData[1].m_Param.m_LocalIp.m_IpMask = 0xFF000000;
	lDevParamSetInfo.m_pParamData[1].m_Param.m_LocalIp.m_IpGate = (ntohl(pHandle->m_pClientAddr->sin_addr.s_addr) & 0xFFFFFF00) | 0x01;
	lDevParamSetInfo.m_pParamData[2].m_Tag = IP_PROT_DEV_PARAM_TAG_RTP_POSTBACK_ADDR;
	lDevParamSetInfo.m_pParamData[2].m_ParamLen = 7;
	lDevParamSetInfo.m_pParamData[2].m_Param.m_PostBackAddrInfo.m_Type = GS_EM_IP_PROT_HOSTNAME_TYPE_IP;
	lDevParamSetInfo.m_pParamData[2].m_Param.m_PostBackAddrInfo.m_HostName.m_IpAddr = SERVER_IP;
	lDevParamSetInfo.m_pParamData[2].m_Param.m_PostBackAddrInfo.m_Port = 5000 + (ntohl(pHandle->m_pClientAddr->sin_addr.s_addr) & 0xFF);
	lDevParamSetInfo.m_pParamData[3].m_Tag = IP_PROT_DEV_PARAM_TAG_SERVER_TCP_ADDR;
	lDevParamSetInfo.m_pParamData[3].m_ParamLen = 7;
	lDevParamSetInfo.m_pParamData[3].m_Param.m_PostBackAddrInfo.m_Type = GS_EM_IP_PROT_HOSTNAME_TYPE_IP;
	lDevParamSetInfo.m_pParamData[3].m_Param.m_PostBackAddrInfo.m_HostName.m_IpAddr = SERVER_IP;
	lDevParamSetInfo.m_pParamData[3].m_Param.m_PostBackAddrInfo.m_Port = SERVER_PORT;
	
	GS_EM_IP_PROT_CmdInfoPack(pData, pLen, &lDevParamSetInfo, IP_PROT_CMD_DEV_PARAM_SET, 0);
	free(lDevParamSetInfo.m_pParamData);
}

static GS_VOID GS_TCP_ConstructStopBcData(GS_U8 *pData, GS_S32 *pLen, GS_S32 DataType, GS_TCP_SERVER_HANDLE_T *pHandle)
{
	GS_EM_IP_PROT_STOP_BC_S lStopBcInfo;
	struct tm *plTm;
	time_t lTime = time(NULL);

	GS_DBGWHERE();
	if ((plTm = localtime(&lTime)) == NULL) {
		GS_COMM_ErrSys("localtime error");
	} 

	bzero(&lStopBcInfo, sizeof(GS_EM_IP_PROT_STOP_BC_S));
	lStopBcInfo.m_EmMsgId.m_Year = plTm->tm_year;
	lStopBcInfo.m_EmMsgId.m_Month = plTm->tm_mon;
	lStopBcInfo.m_EmMsgId.m_Day = plTm->tm_mday;

	GS_EM_IP_PROT_CmdInfoPack(pData, pLen, &lStopBcInfo, IP_PROT_CMD_STOP_BC, 0);
}

static GS_VOID GS_TCP_ConstructStartBcData(GS_U8 *pData, GS_S32 *pLen, GS_S32 DataType, GS_TCP_SERVER_HANDLE_T *pHandle)
{
	GS_EM_IP_PROT_START_BC_S lStartBcInfo;
	struct tm *plTm;
	GS_CHAR *plStr = "³É¶¼¸ßË¹±´¶ûÊýÂëÓÐÏÞ¹«Ë¾Ó¦¼±¹ã²¥ÑÐ·¢²âÊÔÖÐ";
	time_t lTime = time(NULL);

	GS_DBGWHERE();
	if ((plTm = localtime(&lTime)) == NULL) {
		GS_COMM_ErrSys("localtime error");
	} 
	
	bzero(&lStartBcInfo, sizeof(GS_EM_IP_PROT_START_BC_S));
	lStartBcInfo.m_EmMsgId.m_Year = plTm->tm_year;
	lStartBcInfo.m_EmMsgId.m_Month = plTm->tm_mon;
	lStartBcInfo.m_EmMsgId.m_Day = plTm->tm_mday;
	lStartBcInfo.m_BCType = IP_PROT_BC_EM_DRILL_SIMU;
	lStartBcInfo.m_EmEventLvl = IP_PROT_EM_LVL_IV;
	memset(&lStartBcInfo.m_pEmEventType, 1, sizeof(lStartBcInfo.m_pEmEventType)); // "11111"
	lStartBcInfo.m_Vol = 0x64;
	lStartBcInfo.m_StartTime = time(NULL);
	lStartBcInfo.m_DurationTime = 1000;
	lStartBcInfo.m_AssistDataNum = 1;
	lStartBcInfo.m_pAssistData[0].m_Type = DataType;
	switch (DataType) {
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_TEXT:
			{
				lStartBcInfo.m_pAssistData[0].m_Len = strlen(plStr);
				lStartBcInfo.m_pAssistData[0].m_Data.m_pText = plStr;
			}
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_AUD:
			{
				//lStartBcInfo.m_pAssistData[0].m_Len = 9;
				lStartBcInfo.m_pAssistData[0].m_Len = 2;
				lStartBcInfo.m_pAssistData[0].m_Data.m_AudInfo.m_AudTransProt = GS_EM_IP_PROT_AUD_TRANS_PROT_RTP;
				lStartBcInfo.m_pAssistData[0].m_Data.m_AudInfo.m_AudEncFmt = GS_EM_IP_PROT_AUD_ENC_FMT_MPEG1_L2;
				//lStartBcInfo.m_pAssistData[0].m_Data.m_AudInfo.m_Addr.m_Type = GS_EM_IP_PROT_HOSTNAME_TYPE_IP;
				//lStartBcInfo.m_pAssistData[0].m_Data.m_AudInfo.m_Addr.m_HostName.m_IpAddr = ntohl(pHandle->m_pClientAddr->sin_addr.s_addr);
				//lStartBcInfo.m_pAssistData[0].m_Data.m_AudInfo.m_Addr.m_Port = 10001;
			}
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_PIC:
			lStartBcInfo.m_pAssistData[0].m_Len = 7;
			lStartBcInfo.m_pAssistData[0].m_Data.m_PicInfo.m_Addr.m_Type = GS_EM_IP_PROT_HOSTNAME_TYPE_IP;
			lStartBcInfo.m_pAssistData[0].m_Data.m_PicInfo.m_Addr.m_HostName.m_IpAddr = 0x78787878;
			lStartBcInfo.m_pAssistData[0].m_Data.m_PicInfo.m_Addr.m_Port = 10001;
			break;
		case GS_EM_IP_PROT_ASSIST_DATA_TYPE_VID:
			lStartBcInfo.m_pAssistData[0].m_Len = 7;
			lStartBcInfo.m_pAssistData[0].m_Data.m_VidInfo.m_Addr.m_Type = GS_EM_IP_PROT_HOSTNAME_TYPE_IP;
			lStartBcInfo.m_pAssistData[0].m_Data.m_VidInfo.m_Addr.m_HostName.m_IpAddr = 0x78787878;
			lStartBcInfo.m_pAssistData[0].m_Data.m_VidInfo.m_Addr.m_Port = 10001;
			break;
		default:
			break;
	}

	GS_EM_IP_PROT_CmdInfoPack(pData, pLen, &lStartBcInfo, IP_PROT_CMD_START_BC, 0);
}

/* Ìî³äÓÐÐ§°ü£¬Èç¹ûÌî³äÍê³É·µ»Ø TRUE£¬ÐèÒª¼ÌÐøÌî³ä·µ»Ø FALSE */
static GS_BOOL GS_TCP_FullPacket(GS_U16	DstLen, GS_U16 *pCurLen, GS_U8 *pDst, GS_U8 *pSrc, GS_S32 SrcSize)
{
	GS_U16 lRemLen = DstLen - *pCurLen; /* Ê£ÓàÐèÒªÌî³äµÄÊý¾Ý³¤¶È */

	memcpy(pDst, pSrc, GS_MIN(lRemLen, SrcSize));
	*pCurLen += GS_MIN(lRemLen, SrcSize);
	if (DstLen == *pCurLen) {
		return GS_TRUE;
	}

	return GS_FALSE;
}

pthread_mutex_t s_Mutex;
static GS_VOID GS_TCP_ValidPacketProc(GS_S32 SockFd, GS_U8 *pData, GS_S32 DataLen)
{
	GS_S32 lCrc32, lReadCrc32;
	GS_EM_IP_PROT_S *pIpProt;
	GS_S32 lLen;
	GS_EM_IP_PROT_RESPONSE_S lResponseInfo;
	GS_U32 lSessionId;
	GS_S32 lRet;
	GS_HANDLE lBufHandle;
	GS_U8 *plBuf;

	if ((plBuf = (GS_U8 *)malloc(GS_EM_IP_PROT_MAX_LEN)) == GS_NULL) 
		GS_COMM_ErrSys("malloc error");

	if ((pIpProt = (GS_EM_IP_PROT_S *)malloc(sizeof(GS_EM_IP_PROT_S))) == GS_NULL) 
		GS_COMM_ErrSys("malloc error");

	/* Ð¶Ï CRC32 */
	lCrc32 = GS_COMM_Crc32Calc(pData, DataLen - 4);
	GS_MSB32_D(&pData[DataLen - 4], lReadCrc32);
	if (lCrc32 != lReadCrc32) {
		GS_DBGERR("CRC32 Error! CalcCRC = 0x%x ReadCrc = 0x%x\n", lCrc32, lReadCrc32);

		GS_MSB32_D(&pData[4], lSessionId); /* sessionId */
		/* ·¢ËÍÐ£ÑéÊ§°ÜµÄÏûÏ¢¸ø send Ïß³Ì */
		GS_EM_IP_PROT_ResponseInfoMake(&lResponseInfo, pIpProt, IP_PROT_ERR_CRC); 
		GS_EM_IP_PROT_ResponseMsgPack(plBuf, &lLen, &lResponseInfo, lSessionId, IP_PROT_CMD_BUTT);
		
		pthread_mutex_lock(&s_Mutex);
		if (GS_ISDEBUG())
			GS_COMM_PrintDataBlock("Server Send Data", plBuf, lLen);
		if (send(SockFd, plBuf, lLen, 0) < 0) {
			GS_COMM_ErrSys("send error");
		}
		pthread_mutex_unlock(&s_Mutex);

		free(plBuf);
		free(pIpProt);

		return;
	}

	if ((lRet = GS_EM_IP_PROT_Parse(pData, DataLen, pIpProt, &lBufHandle)) == GS_SUCCESS) {
		//GS_EM_IP_PROT_Print(pIpProt);
		if (pIpProt->m_Header.m_DataType == GS_EM_IP_PROT_DATA_TYPE_REQUEST) { /* Ö»ÓÐÇëÇóÐÅÏ¢»á·¢ËÍÏìÓ¦ÏûÏ¢ */
			GS_EM_IP_PROT_ResponseInfoMake(&lResponseInfo, pIpProt, IP_PROT_SUCCESS); 
			GS_EM_IP_PROT_ResponseMsgPack(plBuf, &lLen, &lResponseInfo, pIpProt->m_Header.m_SessionId, pIpProt->m_Content.m_CmdType);

			pthread_mutex_lock(&s_Mutex);
			if (GS_ISDEBUG())
				GS_COMM_PrintDataBlock("Server Send Data", plBuf, lLen);
			if (send(SockFd, plBuf, lLen, 0) < 0) {
				GS_COMM_ErrSys("send error");
			}
			pthread_mutex_unlock(&s_Mutex);
		}
		free(plBuf);
		free(pIpProt);
		GS_EM_IP_PROT_ParseClean(lBufHandle);
	}
	else {
		free(plBuf);
		free(pIpProt);
		GS_EM_IP_PROT_ParseClean(lBufHandle);
	}
}

GS_VOID *GS_TCP_SERVSendCmd(GS_VOID *pParam)
{
	GS_TCP_SERVER_HANDLE_T *plHandle = (GS_TCP_SERVER_HANDLE_T *)pParam;
	GS_S32 lSockFd = plHandle->m_SockFd;
	GS_U8 plData[GS_EM_IP_PROT_MAX_LEN];
	GS_S32 lDataLen = 0;
	GS_S32 lDataType = GS_EM_IP_PROT_ASSIST_DATA_TYPE_TEXT, lFuncCount = 0;
	typedef GS_VOID(*CALLBACK_T)(GS_U8 *pData, GS_S32 *pLen, GS_S32 DataType, GS_TCP_SERVER_HANDLE_T *pHandle);
	CALLBACK_T plFunc[] = {GS_TCP_ConstructStartBcData, GS_TCP_ConstructStopBcData, 
							GS_TCP_ConstructParamSetData/*, GS_TCP_ConstructAudRequestStartData,
							GS_TCP_ConstructAudRequestEndData*/};

	while (1) {
		if (lDataType >= GS_EM_IP_PROT_ASSIST_DATA_TYPE_BUTT) {
			lDataType = GS_EM_IP_PROT_ASSIST_DATA_TYPE_TEXT;
		}

		if (lFuncCount >= (sizeof(plFunc) / sizeof(CALLBACK_T))) {
			lFuncCount = 0;
		}
		if (lFuncCount == 0)
			plFunc[lFuncCount++](plData, &lDataLen, lDataType++, plHandle);
		else
			plFunc[lFuncCount++](plData, &lDataLen, 0, plHandle);

		pthread_mutex_lock(&s_Mutex);
		if (GS_ISDEBUG())
			GS_COMM_PrintDataBlock("Server Send Data", plData, lDataLen);
		if (send(lSockFd, plData, lDataLen, 0) < 0) {
			GS_COMM_ErrSys("send error");
		}
		pthread_mutex_unlock(&s_Mutex);
		//sleep(30);
		sleep(1);
	}

	return NULL;
}

GS_VOID *GS_TCP_SERVMonitorThread(GS_VOID *pParam)
{
	GS_BOOL *plConnStat = (GS_BOOL *)pParam;

	while (1) {
		if (*plConnStat) {
			*plConnStat = GS_FALSE;
		}
		else {
			exit(EXIT_FAILURE);
		}

		sleep(100); /* 100s Ã»ÓÐ¼à²âµ½ÐÄÌø°ü£¬ÈÏÎª¿Í»§¶ËÍË³ö£¬Ö±½ÓÍË³öµ±Ç°Á¬½Ó½ø³Ì */
	}
}

GS_VOID GS_TCP_SERV_ClientComm(GS_S32 SockFd, struct sockaddr_in *pClientAddr)
{
	GS_U8 plBuf[TCP_MTU] = {0};
	GS_S32 lReadLen = 0;
	GS_U16 lValidPackLen = 0, lContentLen, lCount;
	GS_U8 plValidData[TCP_MTU] = {0};
	GS_S32 lErrNo;
	pthread_t lPthreadId;
	GS_BOOL lConnStat = GS_TRUE;
	GS_TCP_SERVER_HANDLE_T lHandle;

	lHandle.m_pClientAddr = pClientAddr;
	lHandle.m_SockFd = SockFd;

	if ((lErrNo = pthread_create(&lPthreadId, NULL, GS_TCP_SERVSendCmd, (GS_VOID *)&lHandle)) != 0)
		GS_COMM_ErrExit(lErrNo, "pthread_create error!");

	if ((lErrNo = pthread_create(&lPthreadId, NULL, GS_TCP_SERVMonitorThread, (GS_VOID *)&lConnStat)) != 0)
		GS_COMM_ErrExit(lErrNo, "pthread_create error!");

	pthread_mutex_init(&s_Mutex, NULL);

	while (1) {
		if ((lReadLen = recv(SockFd, plBuf, sizeof(plBuf), 0)) > 0) {
			GS_S32 i;
			GS_U16 lTag, lVer;

			lConnStat = GS_TRUE;

			if (GS_ISDEBUG())
				GS_COMM_PrintDataBlock("Server Recv Data", plBuf, lReadLen);
			/* ½øÐÐÓÐÐ§°üÍ·¼ì²â */
			if (lReadLen >= 12) { /* ÕâÖÖ´¦Àí·½Ê½ÒªÇó£ºTCP ·¢ËÍµÄÍ·Êý¾ÝÒ»¶¨ÔÚÒ»¸ö TCP °üÄÚ£¬·ñÔò»á¶ªÆú */
				for (i = 0; i <= lReadLen - 12; i++) {
					GS_MSB16_D(&plBuf[i], lTag);
					GS_MSB16_D(&plBuf[i + 2], lVer);
					if (GS_EM_IP_PROT_TAG == lTag && GS_EM_IP_PROT_VER == lVer) {
						GS_MSB16_D(&plBuf[i + 10], lContentLen);

						lValidPackLen = lContentLen;
						lCount = 0;

						if (GS_TCP_FullPacket(lValidPackLen, &lCount, &plValidData[lCount], &plBuf[i], lReadLen - i))
						{
							GS_TCP_ValidPacketProc(SockFd, plValidData, lValidPackLen);
						}
						break;
					}
				}
			}
		}
		else {
			break; // ½áÊøÁ¬½Ó
		}
	}
}

static GS_S32 s_SocketFd;
static void SigHandler(GS_S32 SigNum)
{
	shutdown(s_SocketFd, SHUT_RDWR);
	close(s_SocketFd);
	exit(-1);
}

static void SigCldHandler(GS_S32 SigNum)
{
	pid_t lPid;
	GS_S32 lStatus;

	lPid = wait(&lStatus);
	if (lPid == -1) 
		GS_COMM_ErrSys("wait error");

	GS_DBGINFO("Child Process[%d] Exit! Exit Status:%d \n", lPid, lStatus);
}

GS_S32 main(GS_S32 argc, GS_CHAR *argv[])
{
	struct sockaddr_in lServerAddr;
	struct sockaddr_in lCliAddr;
	GS_S32 lReUse = 1; // ¿ÉÖØÓÃµØÖ·

	signal(SIGINT, SigHandler);
	signal(SIGQUIT, SigHandler);
	signal(SIGABRT, SigHandler);
	signal(SIGCLD, SigCldHandler);

	if ((s_SocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		GS_COMM_ErrSys("socket error");
	}
	if (setsockopt(s_SocketFd, SOL_SOCKET, SO_REUSEADDR, &lReUse, sizeof(GS_S32)) < 0) { // ÎªÁË¿ÉÒÔÁ¢¼´ÖØÆôÊ¹ÓÃ¶Ë¿Ú
		close(s_SocketFd);
		GS_COMM_ErrSys("setsockopt error");
	}

	bzero(&lServerAddr, sizeof(lServerAddr));
	lServerAddr.sin_family = AF_INET;
	lServerAddr.sin_addr.s_addr = htonl(SERVER_IP);
	lServerAddr.sin_port = htons(SERVER_PORT);
	if (bind(s_SocketFd, (struct sockaddr *)&lServerAddr, sizeof(struct sockaddr_in)) < 0) {
		GS_COMM_ErrSys("bind error");
	}

	if (listen(s_SocketFd, SERVER_MAX_LISTENQ) < 0) {
		GS_COMM_ErrSys("listen error");
	}

	while (1) {
		GS_S32 lConnFd;
		socklen_t lAlen = sizeof(lCliAddr);

		lConnFd = accept(s_SocketFd, (struct sockaddr *)&lCliAddr, &lAlen);
		if (lConnFd < 0) {
			GS_COMM_ErrSys("accept error");
		}
		else {
			pid_t lPid;

			GS_DBGINFO("Connect Client is: %s-%d\n", inet_ntoa(lCliAddr.sin_addr), ntohs(lCliAddr.sin_port));
			lPid = fork();
			if (lPid < 0) {
				GS_COMM_ErrSys("fork error");
			}
			else if (lPid == 0) {
				close(s_SocketFd);
				GS_TCP_SERV_ClientComm(lConnFd, &lCliAddr);
				exit(EXIT_FAILURE);
			}
			else {
				GS_DBGINFO("New Process Is: PID[%d]\n", lPid);
				close(lConnFd);
			}
		}
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
