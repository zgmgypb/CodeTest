#include "multi_main_internal.h"
#include "gn_hwl_dxt8243.h"

void HWL_DXT8243Reset(GN_Dx8243HwInfo *pHwInfo)
{
	S32 i;
	GLOBAL_PRINTF(("Reset Dxt8243 Board .....................\n"));

	/* ½ûÖ¹Éý¼¶¹¦ÄÜ */
	DRL_Dxt8243UpgradeEnable(FALSE);
	for (i=0; i<GN_ENC_BOARD_NUM; i++)
	{
		if (pHwInfo->m_Dxt8243Handle[i])
			DXT8243_Reset(pHwInfo->m_Dxt8243Handle[i]);
	}

	int lRebootTime = 30;  
	while(lRebootTime--)
	{
		GLOBAL_PRINTF((">> : %d\n", lRebootTime));
		sleep(1);
	}
	//GN_CountDown("Dxt8243 Resetting......", 30, sleep);

	GLOBAL_PRINTF(("Dxt8243 Reset End.....................\n"));
}

BOOL HWL_SetEncParaToDxt8243(HANDLE32 Handle, S32 SubBoardIndex, HWL_Dxt8243EncCfgParam CfgParam[GN_CH_NUM_PER_ENC_BOARD]) 
{
	DXT8243_ConfigParam lConfigPara[GN_CH_NUM_PER_ENC_BOARD];
	S32 i;

	for (i=0; i<GN_CH_NUM_PER_ENC_BOARD; i++)
	{
#if 0

		lConfigPara[i].m_WorkEn = CfgParam[i].m_WorkEn;
		lConfigPara[i].m_AudioBitrate = CfgParam[i].m_AudioBitrate;
		lConfigPara[i].m_AudioEncMode = CfgParam[i].m_AudioEncMode;
		lConfigPara[i].m_AudioSampleRate = CfgParam[i].m_AudioSampleRate;
		lConfigPara[i].m_VideoAspectRatio = CfgParam[i].m_VideoAspectRatio;
		lConfigPara[i].m_VideoBitrate = CfgParam[i].m_VideoBitrate;
		lConfigPara[i].m_VideoBitrateMode = CfgParam[i].m_VideoBitrateMode;
		lConfigPara[i].m_VideoEncMode = CfgParam[i].m_VideoEncMode;
		lConfigPara[i].m_VideoProfile = CfgParam[i].m_VideoProfile;
		lConfigPara[i].m_VideoResolution = CfgParam[i].m_VideoResolution;
		lConfigPara[i].m_VideoStandard = CfgParam[i].m_VideoStandard;

		lConfigPara[i].m_VideoPID = CfgParam[i].m_VideoPID;
		lConfigPara[i].m_AudioPID = CfgParam[i].m_AudioPID;
		lConfigPara[i].m_PmtPID = CfgParam[i].m_PmtPID;
		if (CfgParam[i].m_VideoPID == CfgParam[i].m_PcrPID)
		{
			lConfigPara[i].m_PcrPID = lConfigPara[i].m_VideoPID;
		}
		else
		{
			lConfigPara[i].m_PcrPID = CfgParam[i].m_PcrPID;
		}

#else

		//lConfigPara[i].m_WorkEn = CfgParam[i].m_WorkEn;
		switch(CfgParam[i].m_WorkEn)
		{
			case ENCODER_WORK_MODE_FREE:
				lConfigPara[i].m_WorkEn = FALSE;
				break;

			case ENCODER_WORK_MODE_ENCODER:
				lConfigPara[i].m_WorkEn = TRUE;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Work Mode Default!\n"));
				lConfigPara[i].m_WorkEn = TRUE;
				break;
		}

		//lConfigPara[i].m_AudioBitrate = CfgParam[i].m_AudioBitrate;
		switch(CfgParam[i].m_AudioBitrate)
		{
			case ENCODER_AudioBitRate_96:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_96K;
				break;

			case ENCODER_AudioBitRate_112:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_112K;
				break;

			case ENCODER_AudioBitRate_128:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_128K;
				break;

			case ENCODER_AudioBitRate_160:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_160K;
				break;

			case ENCODER_AudioBitRate_192:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_192K;
				break;

			case ENCODER_AudioBitRate_224:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_224K;
				break;

			case ENCODER_AudioBitRate_256:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_256K;
				break;

			case ENCODER_AudioBitRate_320:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_320K;
				break;

			case ENCODER_AudioBitRate_384:
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_384K;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Audio Bitrate Default!\n"));
				lConfigPara[i].m_AudioBitrate = DXT8243_AUD_BITRATE_96K;
				break;
		}

		//lConfigPara[i].m_AudioEncMode = CfgParam[i].m_AudioEncMode;
		switch(CfgParam[i].m_AudioEncMode)
		{
			case ENCODER_AudioEncodeMode1://MPEG-1 L2
				lConfigPara[i].m_AudioEncMode = DXT8243_AUD_ENC_MODE_MPEG1;
				break;

			case ENCODER_AudioEncodeMode2://LC-AAC
				lConfigPara[i].m_AudioEncMode = DXT8243_AUD_ENC_MODE_AAC;
				break;

			case ENCODER_AudioEncodeMode3://HE-ACC V1
				lConfigPara[i].m_AudioEncMode = DXT8243_AUD_ENC_MODE_AAC_V1;
				break;

			case ENCODER_AudioEncodeMode4://HE-ACC V2
				lConfigPara[i].m_AudioEncMode = DXT8243_AUD_ENC_MODE_AAC_V2;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Audio Encode Mode Default!\n"));
				lConfigPara[i].m_AudioEncMode = DXT8243_AUD_ENC_MODE_MPEG1;
				break;
		}

		//lConfigPara[i].m_AudioSampleRate = CfgParam[i].m_AudioSampleRate;
		switch(CfgParam[i].m_AudioSampleRate)
		{
			case ENCODER_AudioSampleRate1://32K
				lConfigPara[i].m_AudioSampleRate = DXT8243_AUD_SAMPL_32K;
				break;

			case ENCODER_AudioSampleRate2://44.1K
				lConfigPara[i].m_AudioSampleRate = DXT8243_AUD_SAMPL_44_1K;
				break;

			case ENCODER_AudioSampleRate3://48K
				lConfigPara[i].m_AudioSampleRate = DXT8243_AUD_SAMPL_48K;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Audio Sample Rate Default!\n"));
				lConfigPara[i].m_AudioSampleRate = DXT8243_AUD_SAMPL_32K;
				break;
		}

		//lConfigPara[i].m_VideoAspectRatio = CfgParam[i].m_VideoAspectRatio;
		switch(CfgParam[i].m_AudioSampleRate)
		{
			case ENCODER_VideoAspect1://16:9
				lConfigPara[i].m_VideoAspectRatio = DXT8243_VID_ASPECT_16X9;
				break;

			case ENCODER_VideoAspect2://1:1
				lConfigPara[i].m_VideoAspectRatio = DXT8243_VID_ASPECT_1X1;
				break;

			case ENCODER_VideoAspect3://4:3
				lConfigPara[i].m_VideoAspectRatio = DXT8243_VID_ASPECT_4X3;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Video Aspect Default!\n"));
				lConfigPara[i].m_VideoAspectRatio = DXT8243_AUD_SAMPL_32K;
				break;
		}

		lConfigPara[i].m_VideoBitrate = CfgParam[i].m_VideoBitrate;

		//lConfigPara[i].m_VideoBitrateMode = CfgParam[i].m_VideoBitrateMode;
		switch(CfgParam[i].m_VideoBitrateMode)
		{
			case ENCODER_EncoderOutBitRrateMode_VBR:
				lConfigPara[i].m_VideoBitrateMode = DXT8243_VID_BITRATE_MODE_VBR;
				break;

			case ENCODER_EncoderOutBitRrateMode_CBR:
				lConfigPara[i].m_VideoBitrateMode = DXT8243_VID_BITRATE_MODE_CBR;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Video Bitrate Mode Default!\n"));
				lConfigPara[i].m_VideoBitrateMode = ENCODER_EncoderOutBitRrateMode_CBR;
				break;
		}

		//lConfigPara[i].m_VideoEncMode = CfgParam[i].m_VideoEncMode;
		switch(CfgParam[i].m_VideoEncMode)
		{
			case ENCODER_VideoEncodeMode_H264:
				lConfigPara[i].m_VideoEncMode = DXT8243_VID_ENC_MODE_H264;
				break;

			case ENCODER_VideoEncodeMode_MPEG2:
				lConfigPara[i].m_VideoEncMode = DXT8243_VID_ENC_MODE_MPEG2;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Video Encoder Mode Default!\n"));
				lConfigPara[i].m_VideoEncMode = DXT8243_VID_ENC_MODE_MPEG2;
				break;
		}

		//lConfigPara[i].m_VideoProfile = CfgParam[i].m_VideoProfile;
		switch(CfgParam[i].m_VideoProfile)
		{
			case ENCODER_VideoProfileMode_High:
				lConfigPara[i].m_VideoProfile = DXT8243_VID_PROFILE_HIGH;
				break;

			case ENCODER_VideoProfileMode_Main:
				lConfigPara[i].m_VideoProfile = DXT8243_VID_PROFILE_MAIN;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Video Profile Default!\n"));
				lConfigPara[i].m_VideoProfile = DXT8243_VID_ENC_MODE_MPEG2;
				break;
		}

		//lConfigPara[i].m_VideoResolution = CfgParam[i].m_VideoResolution;
		switch(CfgParam[i].m_VideoResolution)
		{
			case ENCODER_VIDEO_RESOLUTION_352_576:
				lConfigPara[i].m_VideoResolution = DXT8243_PAL_VR_352_576;
				break;

			case ENCODER_VIDEO_RESOLUTION_544_576:
				lConfigPara[i].m_VideoResolution = DXT8243_PAL_VR_544_576;
				break;

			case ENCODER_VIDEO_RESOLUTION_640_576:
				lConfigPara[i].m_VideoResolution = DXT8243_PAL_VR_640_576;
				break;

			case ENCODER_VIDEO_RESOLUTION_704_576:
				lConfigPara[i].m_VideoResolution = DXT8243_PAL_VR_704_576;
				break;

			case ENCODER_VIDEO_RESOLUTION_720_576:
				lConfigPara[i].m_VideoResolution = DXT8243_PAL_VR_720_576;
				break;

			case ENCODER_VIDEO_RESOLUTION_544_480:
				lConfigPara[i].m_VideoResolution = DXT8243_NTSC_VR_544_480;
				break;
			case ENCODER_VIDEO_RESOLUTION_640_480:
				lConfigPara[i].m_VideoResolution = DXT8243_NTSC_VR_640_480;
				break;

			case ENCODER_VIDEO_RESOLUTION_704_480:
				lConfigPara[i].m_VideoResolution = DXT8243_NTSC_VR_704_480;
				break;

			case ENCODER_VIDEO_RESOLUTION_720_480:
				lConfigPara[i].m_VideoResolution = DXT8243_NTSC_VR_720_480;
				break;

			case ENCODER_VIDEO_RESOLUTION_352_480:
				lConfigPara[i].m_VideoResolution = DXT8243_NTSC_VR_352_480;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Video Resolution Default!\n"));
				lConfigPara[i].m_VideoResolution = DXT8243_NTSC_VR_720_480;
				break;
		}


		//lConfigPara[i].m_VideoStandard = CfgParam[i].m_VideoStandard;
		switch(CfgParam[i].m_VideoStandard)
		{
			case ENCODER_VIDEO_FORMAT_NTSC1://NTSC(M/J)
				lConfigPara[i].m_VideoStandard = DXT8243_VID_STANDARD_NTSC;
				break;

			case ENCODER_VIDEO_FORMAT_PAL1://PAL(D/K/B/G/H/I/N)
				lConfigPara[i].m_VideoStandard = DXT8243_VID_STANDARD_PAL;
				break;

			case ENCODER_VIDEO_FORMAT_PAL2://PAL(M)
				lConfigPara[i].m_VideoStandard = DXT8243_VID_STANDARD_NTSC;
				break;

			case ENCODER_VIDEO_FORMAT_PAL3://PAL(COMBINATION-N)
				lConfigPara[i].m_VideoStandard = DXT8243_VID_STANDARD_PAL;
				break;

			case ENCODER_VIDEO_FORMAT_NTSC2://NTSC4.43
				lConfigPara[i].m_VideoStandard = DXT8243_VID_STANDARD_NTSC;
				break;

			case ENCODER_VIDEO_FORMAT_SECAM://SECAM
				lConfigPara[i].m_VideoStandard = DXT8243_VID_STANDARD_PAL;
				break;

			case ENCODER_VIDEO_FORMAT_PAL4://PAL60
				lConfigPara[i].m_VideoStandard = DXT8243_VID_STANDARD_PAL;
				break;

			default:
				GLOBAL_TRACE(("Set Dxt8243 Pata At Video Standard Default!\n"));
				lConfigPara[i].m_VideoStandard = DXT8243_VID_STANDARD_NTSC;
				break;
		}

		lConfigPara[i].m_VideoPID = HWL_VID_PID_BASE + i + SubBoardIndex * 2;
		lConfigPara[i].m_AudioPID = HWL_AUD_PID_BASE + i + SubBoardIndex * 2;
		lConfigPara[i].m_PmtPID = HWL_PMT_PID_BASE + i + SubBoardIndex * 2;
		if (CfgParam[i].m_VideoPID == CfgParam[i].m_PcrPID)
		{
			lConfigPara[i].m_PcrPID = lConfigPara[i].m_VideoPID;
		}
		else
		{
			lConfigPara[i].m_PcrPID = HWL_PCR_PID_BASE + i + SubBoardIndex * 2;
		}

#endif
/*
		GLOBAL_TRACE(("\n========================================Encoder Chip===============\n"));
		GLOBAL_TRACE(("	m_WorkMod = %d\n" , lConfigPara[i].m_WorkEn));
		GLOBAL_TRACE(("	m_VideoBitRate = %d\n" , lConfigPara[i].m_VideoBitrate));
		GLOBAL_TRACE(("	m_VideoFormat = %d\n" , lConfigPara[i].m_VideoEncMode));

		GLOBAL_TRACE(("	m_VideoResolution = %d\n" , lConfigPara[i].m_VideoResolution));
		GLOBAL_TRACE(("	m_VideoAspectRatio = %d\n" , lConfigPara[i].m_VideoAspectRatio));
		GLOBAL_TRACE(("	m_VideoProfile = %d\n" , lConfigPara[i].m_VideoProfile));
		GLOBAL_TRACE(("	m_AudioBitrate = %d\n" , lConfigPara[i].m_AudioBitrate));
		GLOBAL_TRACE(("	m_AudioSampleRate = %d\n" , lConfigPara[i].m_AudioSampleRate));

		GLOBAL_TRACE(("	m_VideoPID = %d\n" , lConfigPara[i].m_VideoPID));
		GLOBAL_TRACE(("	m_AudioPID = %d\n" , lConfigPara[i].m_AudioPID));
		GLOBAL_TRACE(("	m_PcrPID = %d\n" , lConfigPara[i].m_PcrPID));

		GLOBAL_TRACE(("	m_VideoBitrateMode = %d\n" , lConfigPara[i].m_VideoBitrateMode));
		GLOBAL_TRACE(("	m_VideoStandard = %d\n" , lConfigPara[i].m_VideoStandard));
		GLOBAL_TRACE(("	m_AudioEncMode = %d\n" , lConfigPara[i].m_AudioEncMode));
*/
	}

	return DXT8243_SetPara(Handle, lConfigPara);
}

BOOL HWL_GetParaFromDxt8243(HANDLE32 Handle, HWL_Dxt8243StatusParam StatusPara[GN_CH_NUM_PER_ENC_BOARD]) 
{
	return DXT8243_GetPara(Handle, StatusPara); 
}
