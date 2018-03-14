#include "gs_dev_if.h"
#include <signal.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

GS_DEBUG_SET_LEVEL(GS_DBG_LVL_DBG); 

/* 服务器地址和端口 */
//#define SERVER_IP (0xC0A803F9) 
//#define SERVER_IP (0x33A803F9) 
#define SERVER_IP (0xC0A803F1) 
//#define SERVER_IP (0xC0A8030F) 
#define SERVER_PORT (9001)
static GS_CHAR *s_pPhyId = "0123456789ABCDEFGH";

static void SigHandler(GS_S32 SigNum)
{
	GS_EM_ProtProcTaskDestroy();
	exit(-1);
}

GS_S32 GS_EM_DevParamSet(GS_EM_DEV_MODULE_TAG_E ModuleTag, GS_VOID *pParamInfo) /* 设备参数设置 */
{
	GS_S32 lRet = GS_SUCCESS;

	switch (ModuleTag) {
		case DEV_MODULE_TTS:
			{
				GS_EM_DEV_TTS_PARAM_S *plTtsParam = (GS_EM_DEV_TTS_PARAM_S *)pParamInfo; 
			}
			break;
		case DEV_MODULE_TUNER:
			{
				GS_EM_DEV_TUNER_PARAM_S *plTunerParam = (GS_EM_DEV_TUNER_PARAM_S *)pParamInfo; 
			}
			break;
		case DEV_MODULE_FMT:
			{
				GS_EM_DEV_FMT_PARAM_S *plFmtParam = (GS_EM_DEV_FMT_PARAM_S *)pParamInfo; 
			}
			break;
		case DEV_MODULE_FMR:
			{
				GS_EM_DEV_FMR_PARAM_S *plFmrParam = (GS_EM_DEV_FMR_PARAM_S *)pParamInfo; 
			}
			break;
		case DEV_MODULE_DECODER:
			{
				GS_EM_DEV_DECODER_PARAM_S *plDecoderParam = (GS_EM_DEV_DECODER_PARAM_S *)pParamInfo; 
			}
			break;
		case DEV_MODULE_BC_PARAM:
			{
				GS_EM_DEV_BC_PARAM_S *plBCParam = (GS_EM_DEV_BC_PARAM_S *)pParamInfo; 
			}
			break;
		case DEV_MODULE_SYS:
			{
				GS_EM_DEV_SYS_PARAM_S *plSysParam = (GS_EM_DEV_SYS_PARAM_S *)pParamInfo; 
			}
			break;
		default:
			GS_DBGERR("Module Tag Error!\n");
			break;
	}

	return lRet;
}

GS_S32 GS_EM_DevStatGet(GS_EM_DEV_MODULE_TAG_E ModuleTag, GS_VOID *pStatInfo) /* 设备状态获取 */
{
	GS_S32 lRet = GS_SUCCESS;

	switch (ModuleTag) {
		case DEV_MODULE_TTS:
			break;
		case DEV_MODULE_TUNER:
			break;
		case DEV_MODULE_FMT:
			break;
		case DEV_MODULE_FMR:
			break;
		case DEV_MODULE_DECODER:
			break;
		case DEV_MODULE_BC_PARAM:
			break;
		case DEV_MODULE_SYS:
			{
				GS_EM_DEV_SYS_STAT_S *plSysStat = (GS_EM_DEV_SYS_STAT_S *)pStatInfo; 

				plSysStat->m_Vol = 100;
				plSysStat->m_LocalIp.m_IpAddr = 0x78787878;
				plSysStat->m_LocalIp.m_IpMask = 0xFF000000;
				plSysStat->m_LocalIp.m_IpGate = 0x78787801;
				memcpy(plSysStat->m_pPhyId, s_pPhyId, 18);
				plSysStat->m_ServerAddr.m_Type = 0x01;
				plSysStat->m_ServerAddr.m_HostName.m_IpAddr = SERVER_IP;
				plSysStat->m_ServerAddr.m_Port = SERVER_PORT;
				plSysStat->m_WorkStat = DEV_WORK_STAT_BUSY;
				plSysStat->m_AudPostBackAddr.m_Type = 0x01;
				plSysStat->m_AudPostBackAddr.m_HostName.m_IpAddr = 0x78787877;
				plSysStat->m_AudPostBackAddr.m_Port = 1001;
			}
			break;
		default:
			GS_DBGERR("Module Tag Error!\n");
			break;
	}

	return lRet;
}

GS_S32 GS_EM_DevGetPsiInfo(GS_S32 ChannelIndex, GS_EM_DEV_PSI_INFO_S *pPsiInfo)
{
	GS_S32 lRet = GS_SUCCESS;

	sleep(10);

	return lRet;
}

GS_S32 GS_EM_DevWaitPostBackService(GS_EM_DEV_POSTBACK_SERV_S *pPostBackServInfo) /* 阻塞等待回传业务 */
{
	GS_S32 lRet = GS_SUCCESS;

	while (1) sleep(100);

	return lRet;
}

GS_VOID GS_EM_DevGetStorePath(GS_CHAR *pPath)
{
	sprintf (pPath, ".");
}

GS_S32 main(GS_S32 argc, GS_CHAR *argv[])
{
	GS_EM_DEV_IF_INIT_PARAM_S lInitParam;

#if 0
	{ /* 测试环形 Buffer 的使用 */
		GS_HANDLE lRingBuffHandle = GS_COMM_RingBufferCreate(20);
		GS_U8 plWriteData[10] = {1,2,3,4,5,6,7,8,9,10};
		GS_U8 plReadData[10];

		GS_COMM_RingBufferWrite(lRingBuffHandle, plWriteData, 10);
		GS_COMM_RingBufferInfoPrint(lRingBuffHandle);
		GS_COMM_RingBufferWrite(lRingBuffHandle, plWriteData, 10);
		GS_COMM_RingBufferInfoPrint(lRingBuffHandle);
		while (1) {
			GS_COMM_RingBufferRead(lRingBuffHandle, plReadData, 3);
			GS_COMM_PrintDataBlock("read data", plReadData, 3);
			sleep(1);
		}
		
		while (1) sleep(10);
	}
#endif

	signal(SIGINT, SigHandler);
	signal(SIGQUIT, SigHandler);

	lInitParam.m_GS_EM_DevGetPsiInfo = GS_EM_DevGetPsiInfo;
	lInitParam.m_GS_EM_DevParamSet = GS_EM_DevParamSet;
	lInitParam.m_GS_EM_DevStatGet = GS_EM_DevStatGet;
	lInitParam.m_GS_EM_DevWaitPostBackService = GS_EM_DevWaitPostBackService;
	lInitParam.m_GS_EM_DevGetStorePath = GS_EM_DevGetStorePath;
	if (GS_EM_ProtProcTaskCreate(&lInitParam) != GS_SUCCESS) {
		GS_COMM_ErrQuit("GS_EM_ProtProcTaskCreate Error!");
	}
	
	while (1) {
		sleep(10);
	}

	GS_EM_ProtProcTaskDestroy();
	
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

