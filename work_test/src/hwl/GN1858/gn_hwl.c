#include "gn_hwl.h"
#include "main_fpga_op.h"

#define HWL_VID_PID_BASE	(0x80)
#define HWL_AUD_PID_BASE	(0x90)
#define HWL_PMT_PID_BASE	(0xA0)
#define HWL_PCR_PID_BASE	(0xB0)

static GN_HwInfo s_GN_HwInfo;

HWL_CvbsCfgParam s_CvbsCfgParam[GN_CVBS_CHANNEL_NUM];

HWL_VixsEncCfgParam s_VixsCfgParam[2];

/* 编码子板的上电，上电系统运行起来需要大概1分钟 */
static void HWL_VixsPowerUp(void)
{
	S32 lRebootTime; /* 重启延时 */
	S32 i;

	GLOBAL_PRINTF(("Up Vixs Power .....................\n"));

	/* VIXS端口输出使能，低电平使能 */
	for (i=0; i<GN_VIXS_SUBBOARD_NUM; i++)
	{
		MFPGA_SetVixsFlashConnect(i, VIXS_FLASH_CON_PRO90);
	}

	MFPGA_SetEncBoardPowerUp();

	GLOBAL_PRINTF(("Vixs System Starting......\n"));

	lRebootTime = 50;  
	while(lRebootTime--)
	{
		GLOBAL_PRINTF((">> : %d\n", lRebootTime));
		sleep(1);
	}

	GLOBAL_PRINTF(("Up Vixs Power End.....................\n"));
}

BOOL HWL_GN_Init()
{
	S32 i;

	//ds8b20 温控芯片
	Hwl_Ds18b20Init();

	GN_HwInfo *pHwInfo = &s_GN_HwInfo;
	/* 主板FPGA */
	{
		//MFPGA_Init();
		//if (MFPGA_ConfigRbf() == FALSE)
		//{
			//return FALSE;
		//}
		DRL_Initiate();
	}

	/* CVBS模块 */ 
	{
		for (i=0; i<GN_CVBS_SUBBOARD_NUM; i++)
		{
			CVBS_InitParam lInitParam;

			lInitParam.m_ChannelNum = GN_CVBS_CHANNEL_NUM;
			lInitParam.m_pI2cOpen = DRL_CvbsI2cOpen;
			lInitParam.m_pI2cClose = DRL_CvbsI2cClose;
			lInitParam.m_pI2cRead = DRL_CvbsI2cRead;
			lInitParam.m_pI2cWrite = DRL_CvbsI2cWrite;
			lInitParam.m_pReset = DRL_CvbsReset;
			lInitParam.m_pVolumeOpen = DRL_CvbsVolumeOpen;
			lInitParam.m_pVolumeClose = DRL_CvbsVolumeClose;
			lInitParam.m_pVolumeWrite = DRL_CvbsVolumeWrite;
			lInitParam.m_pUserParam = (void *)i;
			pHwInfo->m_CvbsHandle[i] = CVBS_Create(&lInitParam);

			if (pHwInfo->m_CvbsHandle[i] == NULL)
			{
				GLOBAL_TRACE(("CVBS_Create Failed!!\n"));
				continue;
			}

			if (CVBS_Detect(pHwInfo->m_CvbsHandle[i]) == TRUE)
			{
				pHwInfo->m_CvbsIsExist[i] = TRUE;
			}
			else
			{
				pHwInfo->m_CvbsIsExist[i] = FALSE;
				CVBS_Destroy(pHwInfo->m_CvbsHandle[i]);
				pHwInfo->m_CvbsHandle[i] = NULL;
				GLOBAL_TRACE(("CVBS SubBoard [i] Not Exist!!\n", i));
			}
		}
	}

	/* VIXS模块 */
	{
		HWL_VixsPowerUp();

		for (i=0; i<GN_VIXS_SUBBOARD_NUM; i++)
		{
			VIXS_InitParam lVixsInitParam;
			U8 lVersion[16] = {0};

			lVixsInitParam.m_pReset = DRL_VixsReset;
			lVixsInitParam.m_pUartClose = DRL_VixsUartClose;
			lVixsInitParam.m_pUartOpen = DRL_VixsUartOpen;
			lVixsInitParam.m_pUartRead = DRL_VixsUartRead;
			lVixsInitParam.m_pUartWrite = DRL_VixsUartWrite;
			lVixsInitParam.m_pUserParam = (void *)i;

			GLOBAL_PRINTF(("VIXS_Create Object-%d\n", i));
			pHwInfo->m_VixsHandle[i] = VIXS_Create(&lVixsInitParam);
			if (pHwInfo->m_VixsHandle[i] == NULL)
			{
				GLOBAL_TRACE(("VIXS_Create Failed!!\n"));
				continue;
			}

			/* 开启编码子板通信程序 */
			VIXS_StartUartCommunicationApp(pHwInfo->m_VixsHandle[i]);
			sleep(2);   

			if (VIXS_GetVersion(pHwInfo->m_VixsHandle[i], lVersion) == FALSE) /* 用获取版本号的方式判断是否存在 */
			{
				pHwInfo->m_VixsIsExist[i] = FALSE;
				VIXS_Destroy(pHwInfo->m_VixsHandle[i]);
				pHwInfo->m_VixsHandle[i] = NULL;
				GLOBAL_TRACE(("===================Vixs SubBoard [%d] Not Exist!!\n", i));
			}
			else
			{
				pHwInfo->m_VixsIsExist[i] = TRUE;
				GLOBAL_TRACE(("===================Vixs SubBoard [%d] Exist.\n", i));
			}
		}
	}

	return TRUE;
}

BOOL HWL_GN_Terminate()
{
	S32 i;

	//ds8b20 温控芯片
	Hwl_Ds18b20Close();

	GN_HwInfo *pHwInfo = &s_GN_HwInfo;
	//MFPGA_Terminate();

	if (pHwInfo)
	{
		for (i=0; i<GN_CVBS_SUBBOARD_NUM; i++)
		{
			if (pHwInfo->m_CvbsHandle[i])
			{
				CVBS_Destroy(pHwInfo->m_CvbsHandle[i]);
				pHwInfo->m_CvbsHandle[i] = NULL;
			}
		}
		
		for (i=0; i<GN_VIXS_SUBBOARD_NUM; i++)
		{
			if (pHwInfo->m_VixsHandle[i])
			{
				VIXS_Destroy(pHwInfo->m_VixsHandle[i]);
				pHwInfo->m_VixsHandle[i] = NULL;
			}
		}
	}

	return TRUE;
}

BOOL HWL_Check_Encoder_Board_Channel(S32 lChannelIndex)
{
	return s_GN_HwInfo.m_CvbsIsExist[lChannelIndex];
}

BOOL HWL_Check_Encoder_Board_SubChannel(S32 lSubChannelIndex)
{
	return s_GN_HwInfo.m_VixsIsExist[lSubChannelIndex];
}

BOOL HWL_SetEncParaToVixs(HANDLE32 lHandle, S32 lVixsIndex) /* 一张VIXS有两个通道，这个是恒定的 */
{
	VIXS_ConfigParam lConfigParam[2];
	S32 i;

	/* 参数处理 */
	for (i=0; i<2; i++)
	{
		lConfigParam[i].m_WorkEn = s_VixsCfgParam[i].m_WorkEn;
		lConfigParam[i].m_Src = VIXS_TSI_1 - i; /* 注意这里源的定义是反的 */
		lConfigParam[i].m_AudioSrc = VIXS_TSI_1 - i;
		lConfigParam[i].m_AspectRatio = s_VixsCfgParam[i].m_VideoAspectRatio;
		lConfigParam[i].m_Bitrate = s_VixsCfgParam[i].m_VideoBitrate * 1000; /* kbps转bps */

		switch(s_VixsCfgParam[i].m_VideoEncStandard)
		{
		case HWL_VID_ENC_H264:
			if (s_VixsCfgParam[i].m_BitrateMode == HWL_VID_CBR)
			{
				lConfigParam[i].m_VideoFormat = VIXS_H264_CBR;
			}
			else
			{
				lConfigParam[i].m_VideoFormat = VIXS_H264_VBR;
			}
			break;
		case HWL_VID_ENC_MPEG2:
			if (s_VixsCfgParam[i].m_BitrateMode == HWL_VID_CBR)
			{
				lConfigParam[i].m_VideoFormat = VIXS_MPEG2_CBR;
			}
			else
			{
				lConfigParam[i].m_VideoFormat = VIXS_MPEG2_VBR;
			}
			break;
		default:
			lConfigParam[i].m_VideoFormat = VIXS_H264_CBR;
			break;
		}
		
		switch(s_VixsCfgParam[i].m_AudioEncStandard)
		{
		case HWL_AUD_ENC_MPEG1_L2:
			lConfigParam[i].m_AudioFormat = VIXS_MPEG_1_L2;
			break;
		case HWL_AUD_ENC_MPEG2_AAC:
			lConfigParam[i].m_AudioFormat = VIXS_MPEG2_AAC;
			break;
		case HWL_AUD_ENC_MPEG4_AAC:
			lConfigParam[i].m_AudioFormat = VIXS_MPEG4_AAC;
			break;
		default:
			lConfigParam[i].m_AudioFormat = VIXS_MPEG_1_L2;
			break;
		}
		
		switch(s_VixsCfgParam[i].m_AudioSampleFreq)
		{
		case HWL_AUD_SAMP_32K:
			lConfigParam[i].m_OutAudioSampleRate = VIXS_AUDIO_32K;
			lConfigParam[i].m_InAudioSampleRate = VIXS_AUDIO_32K;
			break;
		case HWL_AUD_SAMP_441K:
			lConfigParam[i].m_OutAudioSampleRate = VIXS_AUDIO_44_1K;
			lConfigParam[i].m_InAudioSampleRate = VIXS_AUDIO_44_1K;
			break;
		case HWL_AUD_SAMP_48K:
			lConfigParam[i].m_OutAudioSampleRate = VIXS_AUDIO_48K;
			lConfigParam[i].m_InAudioSampleRate = VIXS_AUDIO_48K;
			break;
		default:
			lConfigParam[i].m_OutAudioSampleRate = VIXS_AUDIO_48K;
			lConfigParam[i].m_InAudioSampleRate = VIXS_AUDIO_48K;
			break;
		}

		if (s_VixsCfgParam[i].m_AudioEncStandard == HWL_AUD_ENC_MPEG1_L2)
		{
			switch(s_VixsCfgParam[i].m_AudioOutBitrate)
			{
			case HWL_AUDIO_BITRATE_192K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_192K;
				break;
			case HWL_AUDIO_BITRATE_224K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_224K;
				break;
			case HWL_AUDIO_BITRATE_256K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_256K;
				break;
			case HWL_AUDIO_BITRATE_320K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_320K;
				break;
			case HWL_AUDIO_BITRATE_384K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_384K;
				break;
			default:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_192K;
				break;
			}
		}
		else
		{
			switch(s_VixsCfgParam[i].m_AudioOutBitrate)
			{
			case HWL_AUDIO_BITRATE_96K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_96K;
				break;
			case HWL_AUDIO_BITRATE_112K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_112K;
				break;
			case HWL_AUDIO_BITRATE_128K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_128K;
				break;
			case HWL_AUDIO_BITRATE_160K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_160K;
				break;
			case HWL_AUDIO_BITRATE_192K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_192K;
				break;
			case HWL_AUDIO_BITRATE_224K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_224K;
				break;
			case HWL_AUDIO_BITRATE_256K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_256K;
				break;
			case HWL_AUDIO_BITRATE_320K:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_320K;
				break;
			default:
				lConfigParam[i].m_OutAudioBitRate = VIXS_AUDIO_BITRATE_256K;
				break;
			}
		}

		if (s_VixsCfgParam[i].m_VideoEncStandard == HWL_VID_ENC_MPEG2)
		{	
			switch(s_VixsCfgParam[i].m_VideoProfile)
			{
			case HWL_VIDEO_MPEG2_HD_MPHL:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_HP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_3");
				break;
			case HWL_VIDEO_MPEG2_SD_MPML:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_MP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_3");
				break;
			default:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_MP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_3");
				break;
			}
		}
		else if (s_VixsCfgParam[i].m_VideoEncStandard == HWL_VID_ENC_H264)
		{
			switch(s_VixsCfgParam[i].m_VideoProfile)
			{
			case HWL_VIDEO_H264_HPL41:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_HP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_41");
				break;
			case HWL_VIDEO_H264_MPL41:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_MP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_41");
				break;
			case HWL_VIDEO_H264_HPL40:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_HP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_4");
				break;
			case HWL_VIDEO_H264_MPL40:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_MP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_4");
				break;
			case HWL_VIDEO_H264_BP30:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_BP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_3");
				break;
			default:
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Profile, "MPEG4_AVC_HP");
				GLOBAL_STRCPY(lConfigParam[i].m_MPEG4Level, "MPEG4_AVC_LEVEL_41");
				break;
			}
		}

		switch(s_VixsCfgParam[i].m_VideoResolution)
		{
		case HWL_VR_1920_1080I:
			lConfigParam[i].m_HorizontalResolution = 1920;
			lConfigParam[i].m_VerticalResolution = 1080;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_1440_1080I:
			lConfigParam[i].m_HorizontalResolution = 1440;
			lConfigParam[i].m_VerticalResolution = 1080;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_1280_720P:
			lConfigParam[i].m_HorizontalResolution = 1280;
			lConfigParam[i].m_VerticalResolution = 720;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 2;
			break;
		case HWL_VR_720_480I:
			lConfigParam[i].m_HorizontalResolution = 720;
			lConfigParam[i].m_VerticalResolution = 480;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_704_480I:
			lConfigParam[i].m_HorizontalResolution = 704;
			lConfigParam[i].m_VerticalResolution = 480;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_640_480I:
			lConfigParam[i].m_HorizontalResolution = 640;
			lConfigParam[i].m_VerticalResolution = 480;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;	
		case HWL_VR_544_480I:
			lConfigParam[i].m_HorizontalResolution = 544;
			lConfigParam[i].m_VerticalResolution = 480;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;	
		case HWL_VR_480_480I:
			lConfigParam[i].m_HorizontalResolution = 480;
			lConfigParam[i].m_VerticalResolution = 480;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;	
		case HWL_VR_352_480I:
			lConfigParam[i].m_HorizontalResolution = 352;
			lConfigParam[i].m_VerticalResolution = 480;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;	
		case HWL_VR_352_240I:
			lConfigParam[i].m_HorizontalResolution = 352;
			lConfigParam[i].m_VerticalResolution = 240;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;	
		case HWL_VR_320_240I:
			lConfigParam[i].m_HorizontalResolution = 320;
			lConfigParam[i].m_VerticalResolution = 240;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;	
		case HWL_VR_720_576I:
			lConfigParam[i].m_HorizontalResolution = 720;
			lConfigParam[i].m_VerticalResolution = 576;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_704_576I:
			lConfigParam[i].m_HorizontalResolution = 704;
			lConfigParam[i].m_VerticalResolution = 576;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_640_576I:
			lConfigParam[i].m_HorizontalResolution = 640;
			lConfigParam[i].m_VerticalResolution = 576;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_544_576I:
			lConfigParam[i].m_HorizontalResolution = 544;
			lConfigParam[i].m_VerticalResolution = 576;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_480_576I:
			lConfigParam[i].m_HorizontalResolution = 480;
			lConfigParam[i].m_VerticalResolution = 576;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_352_576I:
			lConfigParam[i].m_HorizontalResolution = 352;
			lConfigParam[i].m_VerticalResolution = 576;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_352_288I:
			lConfigParam[i].m_HorizontalResolution = 352;
			lConfigParam[i].m_VerticalResolution = 288;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		case HWL_VR_320_288I:
			lConfigParam[i].m_HorizontalResolution = 320;
			lConfigParam[i].m_VerticalResolution = 288;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 1;
			break;
		default:
			lConfigParam[i].m_HorizontalResolution = 0;
			lConfigParam[i].m_VerticalResolution = 0;
			lConfigParam[i].m_VideoOutInterlacedProgressive = 0;
			break;
		}

		switch(s_VixsCfgParam[i].m_VideoFrameRate)
		 {
			case HWL_FRAME_FREQ_25:
				 lConfigParam[i].m_FrameRateOutput = 25;
				 break;
			case HWL_FRAME_FREQ_2997:
				 lConfigParam[i].m_FrameRateOutput = 30;
				 break;
			case HWL_FRAME_FREQ_30:
				 lConfigParam[i].m_FrameRateOutput = 30;
				 break;
			 case HWL_FRAME_FREQ_50:
				 lConfigParam[i].m_FrameRateOutput = 50;
				 break;
			 case HWL_FRAME_FREQ_5994:
				 lConfigParam[i].m_FrameRateOutput = 60;
				 break;
			 case HWL_FRAME_FREQ_60:
				 lConfigParam[i].m_FrameRateOutput = 60;
				 break;
			 default :
				 lConfigParam[i].m_FrameRateOutput = 0;
				 break;
		 }

		if(s_VixsCfgParam[i].m_VideoInMode == HWL_VID_MODE_NTSC)
		{
			lConfigParam[i].m_InputHorizontal = 720;
			lConfigParam[i].m_InputVertical = 480;
			lConfigParam[i].m_FrameRateInput = 30;
		}
		else
		{
			lConfigParam[i].m_InputHorizontal = 720;
			lConfigParam[i].m_InputVertical = 576;
			lConfigParam[i].m_FrameRateInput = 25;
		}
				

		lConfigParam[i].m_StreamRemuxRate = s_VixsCfgParam[i].m_VideoBitrate * 1000 * 188 / 184 + 30 * 188 * 8 + 
			(500 + 6 + 40) * 1024 + s_VixsCfgParam[i].m_RemuxAdjuctBitrate * 1000;  

		if (lConfigParam[i].m_StreamRemuxRate <= 0)    
		{
			lConfigParam[i].m_StreamRemuxRate = s_VixsCfgParam[i].m_VideoBitrate * 1000 * 188 / 184 + 30 * 188 * 8 + (500 + 6 + 40) * 1024;
		}

		if (lConfigParam[i].m_StreamRemuxRate > HWL_STREAM_REMUX_BITRAT_MAX_CBR)    
		{
			lConfigParam[i].m_StreamRemuxRate = HWL_STREAM_REMUX_BITRAT_MAX_CBR;
		}
		if (s_VixsCfgParam[i].m_BitrateMode == HWL_VID_VBR) 
		{
			if(lConfigParam[i].m_StreamRemuxRate * 2 < HWL_STREAM_REMUX_BITRAT_MAX)
			{
				lConfigParam[i].m_StreamRemuxRate *= 2;
			}
			else
			{
				lConfigParam[i].m_StreamRemuxRate = HWL_STREAM_REMUX_BITRAT_MAX;
			}
		}

		lConfigParam[i].m_VideoOutPID = HWL_VID_PID_BASE + i + lVixsIndex * 2;
		lConfigParam[i].m_AudioOutPID = HWL_AUD_PID_BASE + i + lVixsIndex * 2;
		lConfigParam[i].m_AudioOutPID2 = 0;
		lConfigParam[i].m_OutPMTPID = HWL_PMT_PID_BASE + i + lVixsIndex * 2;
		if (s_VixsCfgParam[i].m_VideoPid == s_VixsCfgParam[i].m_PcrPid)
		{
			lConfigParam[i].m_PCROutPid = lConfigParam[i].m_VideoOutPID;
		}
		else
		{
			lConfigParam[i].m_PCROutPid = HWL_PCR_PID_BASE + i + lVixsIndex * 2;
		}

		/* 以下内容设置固定值 */
		lConfigParam[i].m_AudioInFormat = VIXS_AUDIO_BYPASS;
		lConfigParam[i].m_VideoInFormat = VIXS_MPEG2_CBR;
		lConfigParam[i].m_VideoInPID = 702;
		lConfigParam[i].m_AudioInPID = 703;
		lConfigParam[i].m_AudioInPID2 = 0;
		lConfigParam[i].m_PCRPid = 701;
		lConfigParam[i].m_ProgressiveVideoInput = 0;
		lConfigParam[i].m_InsertNullPackets = 1;
		lConfigParam[i].m_PcrOutputInterval = 38;
		lConfigParam[i].m_PureFrameRateInput = 0;
		GLOBAL_STRCPY(lConfigParam[i].m_AudioMode, "AUDIO_FLAG_MODE_STEREO");
		lConfigParam[i].m_MuxBitrate = 0;

		/* 扩展参数 */
		lConfigParam[i].m_GOP_B = s_VixsCfgParam[i].m_GOP_B;
		lConfigParam[i].m_GOP_P = s_VixsCfgParam[i].m_GOP_P;
		lConfigParam[i].m_IDR_EN = (s_VixsCfgParam[i].m_GOP_EN ? 1 : 0);
	}
	return VIXS_SetPara(lHandle, lConfigParam, VIXS_CFGTYPE_ENC);
}
 
//add by leonli
BOOL HWL_Encoder_Vixs_Add_Para( S32 SubIndex,
								BOOL	WorkEn,
								S32		VideoBitrate, 
								S32		VideoInMode, 
								S32		VideoResolution, 
								S32		VideoFrameRate,
								S32		VideoAspectRatio,
								S32		VideoProfile,

								S32		AudioOutBitrate,
								S32		AudioSampleFreq,

								S32		VideoPid,
								S32		AudioPid,
								S32		PcrPid,

								S32		BitrateMode,
								S32		VideoEncStandard,
								S32		AudioEncStandard)
{
	if(GLOBAL_CHECK_INDEX(SubIndex, 2))
	{
		s_VixsCfgParam[SubIndex].m_WorkEn = WorkEn;

		s_VixsCfgParam[SubIndex].m_VideoBitrate = VideoBitrate;
		s_VixsCfgParam[SubIndex].m_VideoInMode = VideoInMode;
		s_VixsCfgParam[SubIndex].m_VideoResolution = VideoResolution;
		s_VixsCfgParam[SubIndex].m_VideoFrameRate = VideoFrameRate;
		s_VixsCfgParam[SubIndex].m_VideoAspectRatio = VideoAspectRatio;
		s_VixsCfgParam[SubIndex].m_VideoProfile = VideoProfile;

		s_VixsCfgParam[SubIndex].m_AudioOutBitrate = AudioOutBitrate;
		s_VixsCfgParam[SubIndex].m_AudioSampleFreq = AudioSampleFreq;

		s_VixsCfgParam[SubIndex].m_VideoPid = VideoPid;
		s_VixsCfgParam[SubIndex].m_AudioPid = AudioPid;
		s_VixsCfgParam[SubIndex].m_PcrPid = PcrPid;

		s_VixsCfgParam[SubIndex].m_BitrateMode = BitrateMode;
		s_VixsCfgParam[SubIndex].m_VideoEncStandard = VideoEncStandard;
		s_VixsCfgParam[SubIndex].m_AudioEncStandard = AudioEncStandard;

/*
		GLOBAL_TRACE(("\n======================================== HWl VISX[%d]================\n" , SubIndex));
		GLOBAL_TRACE(("	m_VideoBitrate = %d\n" , s_VixsCfgParam[SubIndex].m_VideoBitrate));
		GLOBAL_TRACE(("	m_VideoInMode = %d\n" , s_VixsCfgParam[SubIndex].m_VideoInMode));
		GLOBAL_TRACE(("	m_VideoResolution = %d\n" , s_VixsCfgParam[SubIndex].m_VideoResolution));
		GLOBAL_TRACE(("	m_VideoFrameRate = %d\n" , s_VixsCfgParam[SubIndex].m_VideoFrameRate));
		GLOBAL_TRACE(("	m_VideoAspectRatio = %d\n" , s_VixsCfgParam[SubIndex].m_VideoAspectRatio));
		GLOBAL_TRACE(("	m_VideoProfile = %d\n" , s_VixsCfgParam[SubIndex].m_VideoProfile));

		GLOBAL_TRACE(("	m_AudioOutBitrate = %d\n" , s_VixsCfgParam[SubIndex].m_AudioOutBitrate));
		GLOBAL_TRACE(("	m_AudioSampleFreq = %d\n" , s_VixsCfgParam[SubIndex].m_AudioSampleFreq));

		GLOBAL_TRACE(("	m_VideoPid = %d\n" , s_VixsCfgParam[SubIndex].m_VideoPid));
		GLOBAL_TRACE(("	m_AudioPid = %d\n" , s_VixsCfgParam[SubIndex].m_AudioPid));
		GLOBAL_TRACE(("	m_PcrPid = %d\n" , s_VixsCfgParam[SubIndex].m_PcrPid));

		GLOBAL_TRACE(("	m_BitrateMode = %d\n" , s_VixsCfgParam[SubIndex].m_BitrateMode));
		GLOBAL_TRACE(("	m_VideoEncStandard = %d\n" , s_VixsCfgParam[SubIndex].m_VideoEncStandard));
		GLOBAL_TRACE(("	m_AudioEncStandard = %d\n" , s_VixsCfgParam[SubIndex].m_AudioEncStandard));
*/

		return TRUE;
	}
	else
	{
		GLOBAL_TRACE(("Encoder Vixs Add Para Error!\n"));
		return FALSE;
	}

}

BOOL HWL_Encoder_Vixs_Apply(S32 VixsIndex)
{

	if(GLOBAL_CHECK_INDEX(VixsIndex, GN_VIXS_SUBBOARD_NUM))
	{
		if(HWL_SetEncParaToVixs(s_GN_HwInfo.m_VixsHandle[VixsIndex], VixsIndex) == FALSE)
		{
			GLOBAL_TRACE(("Vixs Set Para Error!\n"));
			return FALSE;
		}
		else
		{	
			GLOBAL_TRACE(("Vixs Set Para Success!\n"));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL HWL_SetParaToCvbs(HANDLE32 lHandle)
{
	return CVBS_SetPara(lHandle, s_CvbsCfgParam);
}

//add by leonli
BOOL HWL_Encoder_Cvbs_Add_Para(S32 SubIndex,
							   S32	VideoStandard,
							   S32	VideoBrigh,
							   S32	VideoContrast,
							   S32	VideoHue,
							   S32	H_Location,
							   S32	VideoStaturation,
							   S16	VolumeVal)
{
	if(GLOBAL_CHECK_INDEX(SubIndex,GN_ENC_CH_NUM))
	{
		s_CvbsCfgParam[SubIndex].m_VideoStandard = VideoStandard;
		s_CvbsCfgParam[SubIndex].m_VideoBrigh = VideoBrigh;
		s_CvbsCfgParam[SubIndex].m_VideoContrast = VideoContrast;
		s_CvbsCfgParam[SubIndex].m_VideoHue = VideoHue;
		s_CvbsCfgParam[SubIndex].m_H_Location = H_Location;
		s_CvbsCfgParam[SubIndex].m_VideoStaturation = VideoStaturation;
		s_CvbsCfgParam[SubIndex].m_VolumeVal = VolumeVal;


		/*
			GLOBAL_TRACE(("\n======================================== HWl CVBS[%d]================\n" , SubIndex));
			GLOBAL_TRACE(("	m_VideoBrigh = %d\n" , s_CvbsCfgParam[SubIndex].m_VideoStandard));
			GLOBAL_TRACE(("	m_VideoBrigh = %d\n" , s_CvbsCfgParam[SubIndex].m_VideoBrigh));
			GLOBAL_TRACE(("	m_VideoBrigh = %d\n" , s_CvbsCfgParam[SubIndex].m_VideoContrast));
			GLOBAL_TRACE(("	m_VideoBrigh = %d\n" , s_CvbsCfgParam[SubIndex].m_VideoStaturation));
			GLOBAL_TRACE(("	m_VideoBrigh = %d\n" , s_CvbsCfgParam[SubIndex].m_VideoHue));
			GLOBAL_TRACE(("	m_VideoBrigh = %d\n" , s_CvbsCfgParam[SubIndex].m_H_Location));
			GLOBAL_TRACE(("	m_VideoBrigh = %d\n" , s_CvbsCfgParam[SubIndex].m_VolumeVal));
		*/

		return TRUE;
	}
	else
		return FALSE;
}


BOOL HWL_Encoder_Cvbs_Apply(S32 ChnIndex)
{
	if(GLOBAL_CHECK_INDEX(ChnIndex,GN_CVBS_SUBBOARD_NUM))
	{
		if(HWL_SetParaToCvbs(s_GN_HwInfo.m_CvbsHandle[ChnIndex]) == FALSE)
		{
			GLOBAL_TRACE(("Cvbs Set Para Error!\n"));
			return FALSE;
		}
		else
		{	
			GLOBAL_TRACE(("Cvbs Set Para Success!\n"));
			return TRUE;
		}
	}
	else
		return FALSE;
}

#define FPGA_DATA_NULL			0x1FFF
BOOL HWL_SetParaToMainFpga(HWL_MfpgaCfgParam *pCfgParam)
{
	MFPGA_PidMapParam lPidMapPara;
	S32 i;
	U8 lCbrStuff = 0;

	/* pid映射 */
	for (i=0; i<GN_ENC_CH_NUM; i++)
	{
		if (pCfgParam->m_ChParam[i].m_WorkEn == TRUE)
		{
			lPidMapPara.m_OldPidArray.m_Prog0VideoPid = HWL_VID_PID_BASE + i;
			lPidMapPara.m_OldPidArray.m_Prog1VideoPid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog0VideoPid = pCfgParam->m_ChParam[i].m_VideoPid;
			lPidMapPara.m_NewPidArray.m_Prog1VideoPid = FPGA_DATA_NULL;

			lPidMapPara.m_OldPidArray.m_Prog0Audio1Pid = HWL_AUD_PID_BASE + i;
			lPidMapPara.m_OldPidArray.m_Prog1Audio1Pid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog0Audio1Pid = pCfgParam->m_ChParam[i].m_AudioPid;
			lPidMapPara.m_NewPidArray.m_Prog1Audio1Pid = FPGA_DATA_NULL;

			if (pCfgParam->m_ChParam[i].m_PcrPid == pCfgParam->m_ChParam[i].m_VideoPid)
			{
				lPidMapPara.m_OldPidArray.m_Prog0PcrPid = HWL_VID_PID_BASE + i;
			}
			else
			{
				lPidMapPara.m_OldPidArray.m_Prog0PcrPid = HWL_PCR_PID_BASE + i;
			}
			lPidMapPara.m_OldPidArray.m_Prog1PcrPid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog0PcrPid = pCfgParam->m_ChParam[i].m_PcrPid;
			lPidMapPara.m_NewPidArray.m_Prog1PcrPid = FPGA_DATA_NULL;

			lPidMapPara.m_OldPidArray.m_Prog0Audio2Pid = FPGA_DATA_NULL;
			lPidMapPara.m_OldPidArray.m_Prog1Audio2Pid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog0Audio2Pid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog1Audio2Pid = FPGA_DATA_NULL;
		}
		else
		{
			/* 不工作的通道，清空PID映射 */
			lPidMapPara.m_OldPidArray.m_Prog0VideoPid = FPGA_DATA_NULL;
			lPidMapPara.m_OldPidArray.m_Prog1VideoPid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog0VideoPid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog1VideoPid = FPGA_DATA_NULL;
			lPidMapPara.m_OldPidArray.m_Prog0Audio1Pid = FPGA_DATA_NULL;
			lPidMapPara.m_OldPidArray.m_Prog1Audio1Pid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog0Audio1Pid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog1Audio1Pid = FPGA_DATA_NULL;
			lPidMapPara.m_OldPidArray.m_Prog0PcrPid = FPGA_DATA_NULL;
			lPidMapPara.m_OldPidArray.m_Prog1PcrPid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog0PcrPid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog1PcrPid = FPGA_DATA_NULL;
			lPidMapPara.m_OldPidArray.m_Prog0Audio2Pid = FPGA_DATA_NULL;
			lPidMapPara.m_OldPidArray.m_Prog1Audio2Pid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog0Audio2Pid = FPGA_DATA_NULL;
			lPidMapPara.m_NewPidArray.m_Prog1Audio2Pid = FPGA_DATA_NULL;
		}
		MFPGA_SetPidMap(i, &lPidMapPara);
	}

	/* CBRSTUFF配置 */
	for (i=0; i<GN_ENC_CH_NUM; i++)
	{
		if (pCfgParam->m_ChParam[i].m_CbrStuffEn == FALSE)
		{
			lCbrStuff = lCbrStuff | (0x01 << i);
		}
	}

	/* 设置自定义包溢出数目 */
	for (i=0; i<GN_ENC_CH_NUM; i++)
	{
		MFPGA_SetCustomOverFlowPacketNum(i, 25000);
	}

	/* 设置插入空包 */
	MFPGA_SetIsOutNullPacket(FALSE);

	/* 设置码率 */
	MFPGA_SetOutBitrate(100000);

	/* PCM1723控制 */
	for (i=0; i<GN_CVBS_SUBBOARD_NUM; i++)
	{
		HWL_Pcm1723Config(i, pCfgParam->m_AudioSampleFreq[i]);
	}

	/* PSI发表 */
	{
		PSI_CreateParam lPsiCreatePara;

		lPsiCreatePara.m_TsParam.m_TsId = pCfgParam->m_TsId;
		lPsiCreatePara.m_TsParam.m_OnId = pCfgParam->m_OnId;
		lPsiCreatePara.m_TsParam.m_Charset = pCfgParam->m_Charset;

		for (i=0; i<GN_ENC_CH_NUM; i++)
		{
			lPsiCreatePara.m_ProgParam[i].m_WorkEn = pCfgParam->m_ChParam[i].m_WorkEn;
			lPsiCreatePara.m_ProgParam[i].m_VidEncMode = pCfgParam->m_ChParam[i].m_VideoEncStandard;
			lPsiCreatePara.m_ProgParam[i].m_AudEncMode = pCfgParam->m_ChParam[i].m_AudioEncStandard;
			lPsiCreatePara.m_ProgParam[i].m_ServiceId = pCfgParam->m_ChParam[i].m_ServiceId;
			GLOBAL_STRCPY(lPsiCreatePara.m_ProgParam[i].m_pServiceName, pCfgParam->m_ChParam[i].m_pServiceName);
			lPsiCreatePara.m_ProgParam[i].m_VidPid = pCfgParam->m_ChParam[i].m_VideoPid;
			lPsiCreatePara.m_ProgParam[i].m_AudPid = pCfgParam->m_ChParam[i].m_AudioPid;
			lPsiCreatePara.m_ProgParam[i].m_PmtPid = pCfgParam->m_ChParam[i].m_PmtPid;
			lPsiCreatePara.m_ProgParam[i].m_PcrPid = pCfgParam->m_ChParam[i].m_PcrPid;
		}

		PSI_SetEncPsiParamToHw(&lPsiCreatePara);
	}

	/* PCR发送周期 */
	MFPGA_SetSendPcrPeriod(35);

	return TRUE;
}

BOOL HWL_GetParaFromCvbs(HANDLE32 Handle, HWL_CvbsStatusParam StatusPara[GN_CVBS_CHANNEL_NUM]) 
{
	S32 i;

	for (i=0; i<GN_CVBS_CHANNEL_NUM; i++)
	{
		CVBS_GetPara(Handle, &StatusPara[i], i); 
	}

	return TRUE;
}

//add by leonli 
BOOL HWL_GetEncoderCvbsLockStatus(S32 ChnIndex , HWL_CvbsStatusParam EncoderCvbsLockStatusPara[GN_CVBS_CHANNEL_NUM])
{
	if(GLOBAL_CHECK_INDEX(ChnIndex,GN_CVBS_SUBBOARD_NUM))
	{
		if(HWL_GetParaFromCvbs(s_GN_HwInfo.m_CvbsHandle[ChnIndex], EncoderCvbsLockStatusPara) == FALSE)
		{
			GLOBAL_TRACE(("Cvbs Get Para Error!\n"));
			return FALSE;
		}
		else
		{	
			//GLOBAL_TRACE(("Cvbs Get Para Success!\n"));
			return TRUE;
		}
	}
	else
		return FALSE;	
}

BOOL HWL_Encoder_Vixs_VersionGet(S32 VixsIndex, CHAR_T *pVersion)
{
	if (VIXS_GetVersion(s_GN_HwInfo.m_VixsHandle[VixsIndex], pVersion) == FALSE)
	{
		GLOBAL_TRACE(("Vixs SubBoard [%d] Not Exist!!\n", VixsIndex));
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL HWL_GetParaFromMainFpga(HWL_MfpgaStatusParam *pStatusParam)
{
	S32 i;

	for (i=0; i<GN_ENC_CH_NUM; i++)
	{
		if (MFPGA_GetBitrateStatistics(i, MFPGA_STATISTIC_VIDEO) == 0)
		{
			pStatusParam->m_InputErrorFlag[i] = HWL_MFPGA_INPUT_ERROR_VIDEO;
		}
		else if (MFPGA_GetBitrateStatistics(i, MFPGA_STATISTIC_AUDIO1) == 0)
		{
			pStatusParam->m_InputErrorFlag[i] = HWL_MFPGA_INPUT_ERROR_AUDIO1;
		}
		else if (MFPGA_GetInputCCErrorFlag() && (0x01 << i)) /* 连续计数错误 */
		{
			pStatusParam->m_InputErrorFlag[i] = HWL_MFPGA_INPUT_ERROR_CC;
		}
		else
		{
			pStatusParam->m_InputErrorFlag[i] = HWL_MFPGA_INPUT_ERROR_NONE;
		}
	}

	if (MFPGA_GetOutValidBitrate() == 0)
	{
		pStatusParam->m_OutputErrorFlag = HWL_MFPGA_OUTPUT_ERROR_NOBITRATE;
	}
	else if (MFPGA_GetIsOverFlow() == TRUE)
	{
		pStatusParam->m_OutputErrorFlag = HWL_MFPGA_OUTPUT_ERROR_OVERFLOW;
	}
	else
	{
		pStatusParam->m_OutputErrorFlag = HWL_MFPGA_OUTPUT_ERROR_NONE;
	}

	return TRUE;
}
