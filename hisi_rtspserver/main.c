
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>

#include "rtspservice.h"
#include "rtputils.h"
#include "ringfifo.h"
//#include "sample_comm.h"
#include "hi_mpi_nextchip_process.h"
//static SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara;
static pthread_t gs_VencPid;



extern int g_s32Quit ;
//VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;

#define TRI_VENC 0

/**************************************************************************************************
**
**
**
**************************************************************************************************/
#if 0
void * SAMPLE_VENC_4D1_H264(void *arg)
{
	PAYLOAD_TYPE_E enPayLoad[3] = { PT_H264, PT_H264, PT_H264 };
	PIC_SIZE_E enSize[3] = { PIC_HD720, PIC_VGA, PIC_QVGA };
	
	VB_CONF_S stVbConf;
	SAMPLE_VI_CONFIG_S stViConfig;
	
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr;
	VPSS_CHN_MODE_S stVpssChnMode;
	VPSS_EXT_CHN_ATTR_S stVpssExtChnAttr;
	
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR; //SAMPLE_RC_CBR SAMPLE_RC_VBR SAMPLE_RC_FIXQP
    #if TRI_VENC
		HI_S32 s32ChnNum = 3;
    #else
		HI_S32 s32ChnNum = 1;
    #endif
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	SIZE_S stSize;
	
	/******************************************
		 step  1: init sys variable 
	******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));
	stVbConf.u32MaxPoolCnt = 128;
	
	/*video buffer*/
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,
													  enSize[0],
													  SAMPLE_PIXEL_FORMAT,
													  SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = 10;
#if TRI_VENC
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
													  enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt = 6;
	
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
													  enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[2].u32BlkCnt = 6;
#endif
	/* hist buf*/
	stVbConf.astCommPool[3].u32BlkSize = (196 * 4);
	stVbConf.astCommPool[3].u32BlkCnt = 6;
	
	/******************************************
	 step 2: mpp system init. 
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_VENC_720P_CLASSIC_0;
	}
	
	/******************************************
	 step 3: start vi dev & chn to capture
	******************************************/
	stViConfig.enViMode   = SENSOR_TYPE;
	stViConfig.enRotate   = ROTATE_NONE;
	stViConfig.enNorm	  = VIDEO_ENCODING_MODE_AUTO;
	stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
	s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_VENC_720P_CLASSIC_1;
	}
	
	/******************************************
      step 4: start vpss and vi bind vpss
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_720P_CLASSIC_1;
	}
	
	VpssGrp = 0;
	stVpssGrpAttr.u32MaxW = stSize.u32Width;
	stVpssGrpAttr.u32MaxH = stSize.u32Height;
	stVpssGrpAttr.bDrEn = HI_FALSE;
	stVpssGrpAttr.bDbEn = HI_FALSE;
	stVpssGrpAttr.bIeEn = HI_TRUE;
	stVpssGrpAttr.bNrEn = HI_TRUE;
	stVpssGrpAttr.bHistEn = HI_TRUE;
	stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
	s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_VENC_720P_CLASSIC_2;
	}
	
	s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_VENC_720P_CLASSIC_3;
	}
	
	VpssChn = 0;
	memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
	stVpssChnAttr.bFrameEn = HI_FALSE;
	stVpssChnAttr.bSpEn    = HI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, HI_NULL, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
		goto END_VENC_720P_CLASSIC_4;
	}
	
#if TRI_VENC
	VpssChn = 1;
	stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
	stVpssChnMode.bDouble		= HI_FALSE;
	stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
	stVpssChnMode.u32Width		= 640;
	stVpssChnMode.u32Height 	= 480;
	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
		goto END_VENC_720P_CLASSIC_4;
	}
	
	VpssChn = 3;
	stVpssExtChnAttr.s32BindChn = 1;
	stVpssExtChnAttr.s32SrcFrameRate = 25;
	stVpssExtChnAttr.s32DstFrameRate = 25;
	stVpssExtChnAttr.enPixelFormat	 = SAMPLE_PIXEL_FORMAT;
	stVpssExtChnAttr.u32Width		 = 320;
	stVpssExtChnAttr.u32Height		 = 240;
	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, HI_NULL, HI_NULL, &stVpssExtChnAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
		goto END_VENC_720P_CLASSIC_4;
	}
#endif
	
	/******************************************
	 step 5: start stream venc
	******************************************/
	/*** HD720P **/
	VpssGrp = 0;
	VpssChn = 0;
	VencGrp = 0;
	VencChn = 0;
	s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],\
											gs_enNorm, enSize[0], enRcMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_720P_CLASSIC_5;
	}
	
	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_720P_CLASSIC_5;
	}
	
#if TRI_VENC
	/*** vga **/
	VpssChn = 1;
	VencGrp = 1;
	VencChn = 1;
	s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[1],\
										gs_enNorm, enSize[1], enRcMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_720P_CLASSIC_5;
	}
	
	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_720P_CLASSIC_5;
	}
	
	/*** vga **/
	VpssChn = 3;
	VencGrp = 2;
	VencChn = 2;
	s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[2],\
										gs_enNorm, enSize[2], enRcMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_720P_CLASSIC_5;
	}
	
	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_720P_CLASSIC_5;
	}
	
#endif
	
	/******************************************
	 step 6: stream venc process -- get stream, then save it to file. 
	******************************************/
#if 0
		s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
#else
	gs_stPara.bThreadStart = HI_TRUE;
	gs_stPara.s32Cnt = s32ChnNum;
	struct sched_param schedvenc;
	schedvenc.sched_priority = 10;
	int res2;
	res2=pthread_create(&gs_VencPid, 0, SAMPLE_COMM_VENC_GetVencStreamProc, (HI_VOID *)&gs_stPara);
	if(res2!=0)
	{
	  perror("error!");
	}
	pthread_setschedparam(gs_VencPid, SCHED_RR, &schedvenc);
#endif
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_720P_CLASSIC_5;
	}
	printf("please press twice ENTER to exit this sample\n");
	getchar();
	getchar();
	
	/******************************************
		step 7: exit process
	******************************************/
	SAMPLE_COMM_VENC_StopGetStream();
	
	END_VENC_720P_CLASSIC_5:
	VpssGrp = 0;
	VpssChn = 0;
	VencGrp = 0;
	VencChn = 0;
	SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
	SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
	
#if TRI_VENC
	VpssChn = 1;
	VencGrp = 1;
	VencChn = 1;
	SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
	SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
	
	VpssChn = 3;
	VencGrp = 2;
	VencChn = 2;
	SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
	SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
#endif
	
	SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
	END_VENC_720P_CLASSIC_4:	//vpss stop
	VpssGrp = 0;
	VpssChn = 3;
	SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	VpssChn = 0;
	SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	VpssChn = 1;
	SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	END_VENC_720P_CLASSIC_3:	//vpss stop
	SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
	END_VENC_720P_CLASSIC_2:	//vpss stop
	SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
	END_VENC_720P_CLASSIC_1:	//vi stop
	SAMPLE_COMM_VI_StopVi(&stViConfig);
	END_VENC_720P_CLASSIC_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}
#endif

/*void tcplisten(unsigned short pport)
{

  s32MainFd=tcp_accept(pport);

}*/
/**************************************************************************************************
**
**
**
**************************************************************************************************/
int main(void)
{
	int stemp,s32MainFd;
	struct timespec ts = { 2, 0 };
	pthread_t id;
	ringmalloc(720*576);
	printf("RTSP server START\n");
	PrefsInit(); // 端口和主机名
	printf("listen for client connecting...\n");
	signal(SIGINT, IntHandl);
	s32MainFd=tcp_listen( SERVER_RTSP_PORT_DEFAULT);

	if (ScheduleInit() == ERR_FATAL)
	{
		fprintf(stderr,"Fatal: Can't start scheduler %s, %i \nServer is aborting.\n", __FILE__, __LINE__);
		return 0;
	}
	RTP_port_pool_init(RTP_DEFAULT_PORT);
	//pthread_create(&id,NULL,SAMPLE_VENC_4D1_H264,NULL);
	while (!g_s32Quit)
	{
		nanosleep(&ts, NULL);
		EventLoop(s32MainFd);
	}
	sleep(2);
	ringfree();
	printf("The Server quit!\n");

	return 0;
}

