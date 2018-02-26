#ifndef  __MB86391_H__
#define __MB86391_H__

#include "global_def.h"

#define         VIDEO_BITRATE_MIN                         (1000) //k
#define         VIDEO_BITRATE_MAX                         (7000)


/*
在第一次上电时应当对TVP5146/MB86391进行硬件复位，在以后的参数配置过程中也可以
在每次配置TVP5146或者MB86391时对对应模块进行硬件复位
*/

typedef enum
{
	VIDEO_FORMAT_NTSC = 0,
	VIDEO_FORMAT_PAL,
	VIDEO_FORMAT_MAX,
}VideoInputFormatType;

typedef enum
{
	RESOLUTION_PAL_720_576_NTSC_720_480 = 0,
	RESOLUTION_PAL_544_576_NTSC_544_480,
	RESOLUTION_PAL_480_576_NTSC_480_480,
	RESOLUTION_PAL_352_576_NTSC_352_480,
	RESOLUTION_PAL_352_288_NTSC_352_240
}VideoResolutionType;

typedef enum
{
	AUDIO_SAMPLE_FREQ_32K = 32000,
	AUDIO_SAMPLE_FREQ_44_1K = 44100,
	AUDIO_SAMPLE_FREQ_48K = 48000,
}AudioSampleFreqType;

typedef enum
{
	AUDIO_ES_MODE_STEREO = 0,
	AUDIO_ES_MODE_JOINT_STEREO,
	AUDIO_ES_MODE_DUAL_CHANNEL,
	AUDIO_ES_MODE_SINGLE_CHANNLE
}AudioEsModeType;

typedef enum
{
	AUDIO_BITRATE_32 = 32,
	AUDIO_BITRATE_64 = 64,
	AUDIO_BITRATE_128 = 128,
	AUDIO_BITRATE_192 = 192,
	AUDIO_BITRATE_256 = 256,
	AUDIO_BITRATE_384 = 384
}AudioBitRateType;

typedef enum
{
	GOP_STRUCTURE_IIIIIIIII = 0,
	GOP_STRUCTURE_IPPPPPPPP,
	GOP_STRUCTURE_IBIPBPBPB,
	GOP_STRUCTURE_IBBPBBPBB
}GopStructureType;

typedef enum
{
	AUDIO_IF_TYPE_SLAVE = 0,
	AUDIO_IF_TYPE_MASTER
}AudioIfType;


typedef struct
{
	VideoInputFormatType m_VideoInputFormat;
	VideoResolutionType m_VideoResolution;
	AudioSampleFreqType m_AudioSampleFreq;
	AudioEsModeType m_AudioEsMode;
	AudioBitRateType m_AudioBitRate;
	S32 m_VideoBitrate;
	S32 m_VideoPid;
	S32 m_AudioPid;
	S32 m_PcrPid;
	S32 m_PmtPid;
	GopStructureType m_GopStructure;
	S32 m_GopSize;
	S32 m_Hlocation;
}Mb86391EncodeParameter;

typedef void (*Mb86391_DelayMSCB)(S32 MS);
typedef S32 (*Mb86391_UartSendFn)( U8 *pSendBuf, S32 len );
typedef S32 (*Mb86391_UartReceiveFn)( U8 *pReceiveBuf, S32 len );

BOOL8 InitMb86391(Mb86391_UartSendFn pUartSend, Mb86391_UartReceiveFn pUartReceive, Mb86391_DelayMSCB pDelay, S8 bWaitMb86391AnwserFlag);
BOOL8 ConfigMb86391AllParameterToDefault(Mb86391EncodeParameter *pEncodeParameter);
BOOL8 ConfigMb86391AllParameter(Mb86391EncodeParameter *pEncodeParameter);

void ED_MB86391SetHandle(void* CurrentHandle);

#endif

