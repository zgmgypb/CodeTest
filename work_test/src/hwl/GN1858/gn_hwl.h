#ifndef GN_HWL_H
#define GN_HWL_H

#include "gn_drv.h"
#include "cvbs.h"
#include "vixs_pro90.h"
#include "main_fpga_op.h"
#include "hwl_ds8b20.h"

#define HWL_CvbsCfgParam CVBS_ConfigParam
#define HWL_CvbsStatusParam CVBS_StatusPara

enum
{
	HWL_VID_ENC_H264 = 0,
	HWL_VID_ENC_MPEG2,
	HWL_VID_ENC_AVS,
	HWL_VID_ENC_AVS_PLUS /* AVS+ */
};

enum
{
	HWL_VID_VBR = 0,
	HWL_VID_CBR
};

enum
{
	HWL_AUD_ENC_MPEG1_L2 = 0,
	HWL_AUD_ENC_MPEG2_AAC,
	HWL_AUD_ENC_MPEG4_AAC,
	HWL_AUD_ENC_DRA_2_0,
	HWL_AUD_ENC_DRA_5_1,
	HWL_AUD_ENC_AC3,
	HWL_AUD_ENC_EAC3
};

enum
{
	HWL_AUD_SAMP_32K	 = 0,
	HWL_AUD_SAMP_441K, /* 44.1K */
	HWL_AUD_SAMP_48K
};

enum
{
	HWL_AUDIO_BITRATE_16K = 0,
	HWL_AUDIO_BITRATE_32K,
	HWL_AUDIO_BITRATE_64K	,
	HWL_AUDIO_BITRATE_96K,
	HWL_AUDIO_BITRATE_112K,
	HWL_AUDIO_BITRATE_128K,
	HWL_AUDIO_BITRATE_160K,
	HWL_AUDIO_BITRATE_192K,
	HWL_AUDIO_BITRATE_224K,
	HWL_AUDIO_BITRATE_256K,
	HWL_AUDIO_BITRATE_320K,
	HWL_AUDIO_BITRATE_384K
};

enum
{
	HWL_VIDEO_H264_HPL41 = 0,
	HWL_VIDEO_H264_MPL41,
	HWL_VIDEO_H264_HPL40,
	HWL_VIDEO_H264_MPL40,	
	HWL_VIDEO_H264_BP30
};

enum
{
	HWL_VIDEO_MPEG2_HD_MPHL = 0,
	HWL_VIDEO_MPEG2_SD_MPML
};

/* 视频分辨率 */
enum
{	
	HWL_VR_UNRECOGNIZED,

	HWL_VR_320_288I,
	HWL_VR_352_288I,
	HWL_VR_352_576I,
	HWL_VR_480_576I,
	HWL_VR_544_576I,
	HWL_VR_640_576I,
	HWL_VR_704_576I,
	HWL_VR_720_576I,

	HWL_VR_320_240I,
	HWL_VR_352_240I,
	HWL_VR_352_480I,
	HWL_VR_480_480I,
	HWL_VR_544_480I,
	HWL_VR_640_480I,
	HWL_VR_704_480I,
	HWL_VR_720_480I,

	HWL_VR_1280_720P,

	HWL_VR_1440_1080I,

	HWL_VR_1920_1080I
};

/* 视频帧频 */
enum
{
	HWL_FRAME_FREQ_2398 = 0,
	HWL_FRAME_FREQ_24	,
	HWL_FRAME_FREQ_25,
	HWL_FRAME_FREQ_2997,
	HWL_FRAME_FREQ_30,
	HWL_FRAME_FREQ_50	,
	HWL_FRAME_FREQ_5994,
	HWL_FRAME_FREQ_60	
};

enum
{
	HWL_VID_MODE_PAL = 0,
	HWL_VID_MODE_NTSC
};

#define HWL_STREAM_REMUX_BITRAT_MAX         27000000 /* 最大复用码流 */
#define HWL_STREAM_REMUX_BITRAT_MAX_CBR     25000000

typedef struct 
{
	BOOL	m_WorkEn;//工作模式

	S32		m_VideoBitrate;//视频输出码率
	S32		m_VideoInMode;//视频制式 
	S32		m_VideoResolution; //视频解像度
	S32		m_VideoFrameRate;//帧频
	S32		m_VideoAspectRatio;//宽高比
	S32		m_VideoProfile;//视频类型

	S32		m_AudioOutBitrate;//音频输出码流
	S32		m_AudioSampleFreq;//音频采样率

	S32		m_VideoPid;
	S32		m_AudioPid;
	S32		m_PcrPid;

	S32		m_BitrateMode; //Video Bitrate Mode
	S32		m_VideoEncStandard;//Video Encode Mode 
	S32		m_AudioEncStandard;//Audio Encode Mode 

	/* 扩展参数 */
	S32		m_RemuxAdjuctBitrate; /* 单位：kbps */
	S32		m_GOP_B;
	S32		m_GOP_P;
	BOOL		m_GOP_EN;
}HWL_VixsEncCfgParam;

typedef struct  
{
	struct
	{
		BOOL		m_WorkEn;

		U32	m_VideoPid;
		U32	m_AudioPid;
		U32	m_PcrPid;
		U32	m_PmtPid;

		S32	m_VideoEncStandard;	//Video Encode Mode 	/* 视频编码模式 */
		S32	m_AudioEncStandard;	//Audio Encode Mode 	/* 音频编码模式 */

		CHAR_T m_pServiceName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN]; /* 节目名 */
		U32	m_ServiceId; /* 节目号 */

		BOOL		m_CbrStuffEn;//扩展参数 0
	}m_ChParam[GN_ENC_CH_NUM];
	U16	m_TsId;			/* transport_stream_id */
	U16	m_OnId;			/* original_network_id */
	U32	m_Charset;		/* 字符集 */

	S32		m_AudioSampleFreq[GN_CVBS_SUBBOARD_NUM]; /* 跟CVBS的子板数目有关，每张板子四个通道的设置值为一样的 */
}HWL_MfpgaCfgParam;

enum
{
	HWL_MFPGA_INPUT_ERROR_NONE = 0, /* 无错误 */
	HWL_MFPGA_INPUT_ERROR_CC, /* 连续计数错误，表示有流输入，但TS流有问题 */
	HWL_MFPGA_INPUT_ERROR_VIDEO, /* 视频数据错误，表示没有视频数据输入 */
	HWL_MFPGA_INPUT_ERROR_AUDIO1, /* 音频1数据错误，表示没有音频数据输入 */
	HWL_MFPGA_INPUT_ERROR_AUDIO2 /* 音频2数据错误 */
};

enum
{
	HWL_MFPGA_OUTPUT_ERROR_NONE = 0,
	HWL_MFPGA_OUTPUT_ERROR_OVERFLOW, /* 码流溢出 */
	HWL_MFPGA_OUTPUT_ERROR_NOBITRATE /* 无码率输出 */
};

typedef struct
{
	U32	m_InputErrorFlag[GN_ENC_CH_NUM];
	U32	m_OutputErrorFlag;
}HWL_MfpgaStatusParam;

typedef struct
{
	HANDLE32 m_CvbsHandle[GN_CVBS_SUBBOARD_NUM];
	BOOL m_CvbsIsExist[GN_CVBS_SUBBOARD_NUM];

	HANDLE32 m_VixsHandle[GN_VIXS_SUBBOARD_NUM];
	BOOL m_VixsIsExist[GN_VIXS_SUBBOARD_NUM];
}GN_HwInfo; /* 硬件信息 */

/* 编码建表接口 */
typedef enum
{
	PSI_AUD_ENC_MODE_MPEG1_L2 = 0,
	PSI_AUD_ENC_MODE_DRA_2_0,
	PSI_AUD_ENC_MODE_DRA_5_1,
	PSI_AUD_ENC_MODE_AAC,
	PSI_AUD_ENC_MODE_AC3,
	PSI_AUD_ENC_MODE_EAC3
}PSI_AudEncMode; 

typedef struct  
{
	U32	m_TsId;
	U8		m_Version;
	U8		m_SyncByteReplaceChar; /* 同步字节指定，指定同步字节的原因在于当是SPTS模式时，FPGA通过同步字节确定表放出的IP端口 */

	struct {
		U32	m_ProgramNum; /* 节目号 */
		U32	m_PmtPid; /* PMT的PID */
	}m_ProgramInfo[GN_ENC_CH_NUM];
	U32	m_ProgramLen; /* 节目数目 */
}PSI_PatInfo;

typedef struct  
{
	U32	m_TsId;
	U32	m_OnId; /* original_network_id */
	U8		m_Version;
	U8		m_SyncByteReplaceChar; /* 同步字节指定，指定同步字节的原因在于当是SPTS模式时，FPGA通过同步字节确定表放出的IP端口 */

	S32		m_Charset;
	BOOL		m_CharsetMark;

	struct {
		U32	m_ProgramNum; /* 节目号 */
		CHAR_T	m_pProgramName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN]; /* 节目名 */
		U32	m_PmtPid; /* PMT的PID */
	}m_ProgramInfo[GN_ENC_CH_NUM];
	U32	m_ProgramLen; /* 节目数目 */
}PSI_SdtInfo;

typedef struct
{
	U8		m_SyncByteReplaceChar; /* 同步字节指定，指定同步字节的原因在于当是SPTS模式时，FPGA通过同步字节确定表放出的IP端口 */
	U8		m_Version;

	U32	m_ProgramNum; /* 节目号 */
	CHAR_T	m_pProgramName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN]; /* 节目名 */
	U32	m_PmtPid; /* PMT的PID */

	U32	m_PcrPid;

	U32	m_VidPid;
	S32	m_VidEncMode; /* 视频编码模式 */

	U32	m_AudPid;
	S32	m_AudEncMode; /* 音频编码模式 */
}PSI_PmtInfo;

typedef struct 
{
	PSI_PatInfo		m_PatInfo;
	PSI_PmtInfo		m_PmtInfo;
	PSI_SdtInfo		m_SdtInfo;
}PSI_TableCreatePara;

typedef struct  
{
	U8		m_pPsiPacket[MFPGA_MAX_PSI_NUM][MPEG2_TS_PACKET_SIZE];
	U32	m_PsiPacketCounter; /* PSI包计数 */	
}PSI_PacketInfo;

typedef struct 
{
	U16	m_TsId;			/* transport_stream_id */
	U16	m_OnId;			/* original_network_id */

	U32	m_Charset;
}PSI_TsParam;

typedef struct 
{
	S32	m_WorkEn;			/* 工作模式 */

	U32	m_VidPid;
	U32	m_AudPid;
	U32	m_PcrPid;
	U32	m_PmtPid;

	S32	m_VidEncMode;			/* 视频编码模式 */
	S32	m_AudEncMode;		/* 音频编码模式 */

	CHAR_T m_pServiceName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN]; /* 节目名 */
	U32	m_ServiceId; /* 节目号 */
}PSI_ChProgParam;

typedef struct
{
	PSI_TsParam m_TsParam; /* TS流参数 */
	PSI_ChProgParam m_ProgParam[GN_ENC_CH_NUM]; /* 节目参数 */
}PSI_CreateParam;

BOOL PSI_SetEncPsiParamToHw(PSI_CreateParam *pPsiParam);

void HWL_Pcm1723Config(U32 Pcm1723Index, S32	AudioSampleFreq);

BOOL HWL_GN_Init();
BOOL HWL_GN_Terminate();

BOOL HWL_Check_Encoder_Board_Channel(S32 lChannelIndex);
BOOL HWL_Check_Encoder_Board_SubChannel(S32 lSubChannelIndex);

BOOL HWL_SetEncParaToVixs(HANDLE32 lHandle, S32 lVixsIndex);

BOOL HWL_Encoder_Cvbs_Add_Para(S32 SubIndex, S32 VideoStandard, S32 VideoBrigh, S32 VideoContrast, S32 VideoHue, S32 H_Location, S32 VideoStaturation, S16 VolumeVal);
BOOL HWL_Encoder_Cvbs_Apply(S32 ChnIndex);

BOOL HWL_Encoder_Vixs_Add_Para(S32 SubIndex, BOOL WorkEn, S32 VideoBitrate, S32 VideoInMode, S32 VideoResolution, S32 VideoFrameRate, S32 VideoAspectRatio, S32 VideoProfile, S32 AudioOutBitrate, S32 AudioSampleFreq, S32 VideoPid, S32 AudioPid, S32 PcrPid, S32 BitrateMode, S32 VideoEncStandard, S32 AudioEncStandard);
BOOL HWL_Encoder_Vixs_Apply(S32 VixsIndex);

BOOL HWL_SetParaToMainFpga(HWL_MfpgaCfgParam *pCfgParam);
/* 获取状态参数 */
BOOL HWL_GetParaFromCvbs(HANDLE32 Handle, HWL_CvbsStatusParam StatusPara[GN_CVBS_CHANNEL_NUM]);
BOOL HWL_GetParaFromMainFpga(HWL_MfpgaStatusParam *pStatusParam);

//add by leonli
BOOL HWL_GetEncoderCvbsLockStatus(S32 ChnIndex , HWL_CvbsStatusParam EncoderCvbsLockStatusPara[GN_CVBS_CHANNEL_NUM]);

BOOL HWL_Encoder_Vixs_VersionGet(S32 VixsIndex, CHAR_T *pVersion);

#endif /* GN_HWL_H */
/* EOF */
