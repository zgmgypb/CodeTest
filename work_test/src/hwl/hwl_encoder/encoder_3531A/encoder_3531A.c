#include <stdio.h>
#include <unistd.h>
#include "platform_def.h"
#include "global_def.h"
#include "sample_comm.h"
#include "linuxplatform.h"
#include "platform_conf.h"
#include "ts.h"
#include "encoder_3531A.h"
#include "audio_enc.h"
#include "mpeg2.h"
#include "platform_assist.h"
#include "multi_main_internal.h"

#define AUD_TS_BUFF_SIZE (MPEG2_TS_PACKET_SIZE * 2000)
#define INSERT_TS_BUFF_SIZE (MPEG2_TS_PACKET_SIZE * 500)
/* 由于音频编码的速度比视频快，所以然音频后进入，这样起到让音频与视频同步的情况，250 表示 6s 的延时 */
#define AUD_DELAY_BUFF_SIZE (SAMPLE_AUDIO_PTNUMPERFRM * 4 * 250) 
#define AC3_BUFF_SIZE (1920 * 2 * 20)
typedef enum {
	AUD_PROC_INIT = 0,
	AUD_PROC_START,
	AUD_PROC_STOP,
	AUD_PROC_REINIT
} ENC_3531AAudProcCmd;

typedef struct {
	BOOL		m_Ac3ValidFlag;
	BOOL		m_Ac3StartFlag;
	HI_S32		m_Ac3Counter;
	U8			m_BufNow[SAMPLE_AUDIO_PTNUMPERFRM * 4];
	U8			m_BufBefor[SAMPLE_AUDIO_PTNUMPERFRM * 4];
	HI_S32		m_BufNowSize;
	HI_S32		m_BufBeforSize;
	HI_S32		m_Ac3FrameSize;
	HI_S32		m_Ac3FrameSizeBefor;
	RNGBUF_t	*m_pAC3RngBuf;
	
} ENC_3531AAC3Param;


typedef struct {
	VI_MODE_E		m_Vimode;
	VIDEO_NORM_E	m_ViNorm;
	PIC_SIZE_E		m_PicSize;
	HI_U32			m_FrmRate;
	VPSS_DIE_MODE_E	m_DieMode;
} ENC_3531AViInfo;

typedef struct {
	VIDEO_NORM_E	m_ViNorm;
	PIC_SIZE_E		m_PicSize;
	HI_U32			m_FrmRate;
} ENC_3531AVoInfo;

typedef enum {
	TS_TYPE_VID = 0,
	TS_TYPE_AUD,
	TS_TYPE_INSERT,
	TS_TYPE_NUM
} ENC_3531ASendTsType;
typedef struct {
	S32						m_ChnIndex;
	ENC_3531ASendTsType		m_TsType;
	RNGBUF_t				*m_pRngBuf;
} ENC_3531ASendTsToFifoParam;

#define RTP_PACKET_LEN (12)
#define UDP_MAX_TS_PACKET_SIZE (7)
typedef struct {
	HANDLE32		m_SocketHandle;

	U8		m_pBuf[RTP_PACKET_LEN + MPEG2_TS_PACKET_SIZE * UDP_MAX_TS_PACKET_SIZE];
	S32		m_BufCount; 
	U16		m_RtpSn; /* RTP 序列号 */
	
	/* 设置参数 */
	S32		m_IpProto; /* 发送协议 */
	U32		m_DestIp;
	U16		m_DestPort;
	S32		m_SendTsNum; /* 发送 Ts 包数，每个 sendto 函数发送包含的 TS 包个数，范围 1 ~ 7 */

	HANDLE32	m_Lock; /* 网络发包相关参数更改 */
} ENC_3531AEthOutParam;

typedef struct {
	HANDLE32	m_AudProThreadHandle;
	TS_OPTIONS_T	*m_pEs2TsObj;
	HANDLE32	m_AudEncHandle; 
	ENC_3531AAudProcCmd	m_AudProcCmd;

	RNGBUF_t 		*m_pAacRngBuf;
	HANDLE32		m_ParamSetLock; /* 参数设置和流数据获取是互斥的 */

	ENC_3531ASendTsToFifoParam m_SendToFifoParam;

	/* 设置的参数 */
	ENC_3531AAudEncMode	m_AudEncMode;
	ENC_3531AAudBitrate	m_AudBitrate;
	ENC_3531AAudSample	m_AudSample;
	S32					m_AudVolume;
	S32		m_AudPtsRelativeDelayTime; /* 音频相对 Pts 延时，单位：us */
	S32		m_PtsDelayTime; /* 单位：us */
	S32		m_AudDelayFrameNum; /* 用于音频延时的帧数目 */
	BOOL	m_SignalIsLocked;
} ENC_3531AAudProcParam;

typedef struct {
	HANDLE32	m_VidProThreadHandle; /* 每一路视频对应一个线程处理，否则处理不过来 */

	TS_OPTIONS_T	*m_pEs2TsObj;
	ENC_3531ASendTsToFifoParam m_SendToFifoParam;

	/* 设置的参数 */
	BOOL		m_ActiveMark; 
	S32			m_PtsDelayTime;
	BOOL		m_SignalIsLocked;

	HANDLE32	m_TsProcLock; /* TS 发包处理锁，由于控制码流模块与视频 ES 转 TS 模块在一起，所以这里需要单独锁 */
	HANDLE32	m_ParamSetLock; /* 参数设置和流数据获取是互斥的 */
} ENC_3531AVidProcParam;

typedef struct {
	U8		m_pPidCc[MPEG2_TS_PACKET_MAX_PID_NUM];
	U32		m_pPidCcCount[MPEG2_TS_PACKET_MAX_PID_NUM];
	U8		m_pPidCcRstFlag[MPEG2_TS_PACKET_MAX_PID_NUM];

	ENC_3531AStatus	m_RecordTsInfo; /* 1s 刷一次 */
	ENC_3531AStatus	m_RtTsInfo; /* 实时计数 */

	ENC_3531AAlarmInfo	m_AlarmInfo;

	HANDLE32	m_Lock;
} ENC_3531AMonitorParam;

typedef struct {
	RNGBUF_t	*m_pVidTsRngBuf;
	RNGBUF_t	*m_pAudTsRngBuf; 
	RNGBUF_t	*m_pInsertTsRngBuf; /* 插入 TS 包环形 buffer，包括了 PSI 插入 */
} ENC_3531ATsProcParam;

typedef struct {
	HWL_PSI_OPTION_T	*m_pPsiObj;

	ENC_3531ASendTsToFifoParam m_SendToFifoParam;

	/* PSI 相关传入参数 */
	BOOL				m_ActiveMark;
	U8					m_pProgName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN];
	MPEG2_PSICharSet	m_PsiCharSet;
	U16					m_ProgNumber;
	U16					m_VidPid;
	U16					m_PcrPid;
	U16					m_AudPid;
	U16					m_PmtPid;
	ENC_3531AAudEncMode	m_AudEncMode;

	HANDLE32			m_Lock;
} ENC_3531APsiProcParam;

typedef struct {
	ENC_3531AInitParam		m_InitParam;

	ENC_3531AAudProcParam	*m_pAudProcParam; /* 音频处理参数 */

	ENC_3531AVidProcParam	*m_pVidProcParam; /* 视频处理参数 */

	HANDLE32				m_PsiSendThread; /* PSI 发包 */
	ENC_3531APsiProcParam	*m_pPsiProcParam; /* PSI 处理参数 */

	HANDLE32				m_TsProcThread;
	ENC_3531ATsProcParam	*m_pTsProcParam;

	HANDLE32				m_MonitorTimer;
	ENC_3531AMonitorParam	*m_pMonitorParam; /* 监控参数 */

	BOOL					m_TaskRunFlg; /* 控制所有线程运行状态，是否结束 */

	ENC_3531AIpOutType		m_IpOutputType;
	ENC_3531AEthOutParam	*m_pEthOutParam;

	HANDLE32				m_SysLock; /* 系统锁，用于如 PTS 同步能设置的线程互锁 */
} ENC_3531AHandle;

static ENC_3531AHandle *s_pHandle = NULL;

typedef struct {
	U64	m_Pcr; 
	U64	m_Pts;
	U64 m_Dts;
} ENC_3531ATimeStamp;
static void ENC_3531AGetTimeStamp(U64 OldPts, S32 PtsDelayTime, ENC_3531ATimeStamp *pTimeStamp)
{
	pTimeStamp->m_Dts = pTimeStamp->m_Pts = ((OldPts + PtsDelayTime) * 27) / 300;
	pTimeStamp->m_Pcr = 0;
}

static HI_U32 ENC_3531AGetProfile(ENC_3531AProfile Profile)
{
	return Profile;
}

static HI_U32 ENC_3531AGetAudEncMode(ENC_3531AAudEncMode AudBitrate)
{
	switch (AudBitrate) {
		case AUD_ENC_MODE_MPEG1_L2:
			return PSI_AUD_ENC_MODE_MPEG1_L2;
		case AUD_ENC_MODE_LC_AAC:
			return PSI_AUD_ENC_MODE_AAC;
		case AUD_ENC_MODE_AC3:
			return PSI_AUD_ENC_MODE_AC3;
		case AUD_ENC_MODE_EAC3:
			return PSI_AUD_ENC_MODE_EAC3;
		default:
			return PSI_AUD_ENC_MODE_MPEG1_L2;
	}
}

/* 单位 Kbps */
static HI_U32 ENC_3531AGetAudBitrate(ENC_3531AAudBitrate AudBitrate)
{
	switch (AudBitrate) {
		case AUD_BR_16K:
			return 16;
		case AUD_BR_64K:
			return 64;
		case AUD_BR_96K:
			return 96;
		case AUD_BR_112K:
			return 112;
		case AUD_BR_128K:
			return 128;
		case AUD_BR_160K:
			return 160;
		case AUD_BR_192K:
			return 192;
		case AUD_BR_224K:
			return 224;
		case AUD_BR_256K:
			return 256;
		case AUD_BR_320K:
			return 320;
		case AUD_BR_384K:
			return 384;
		default:
			return 192;
	}

	return 192;
}

/* 单位 Hz */
static AUDIO_SAMPLE_RATE_E ENC_3531AGetAudSample(ENC_3531AAudSample AudSample)
{
	switch (AudSample) {
		case AUD_SAMP_32K:
			return AUDIO_SAMPLE_RATE_32000;
		case AUD_SAMP_44_1K:
			return AUDIO_SAMPLE_RATE_44100;
		case AUD_SAMP_48K:
			return AUDIO_SAMPLE_RATE_48000;
		default:
			return AUDIO_SAMPLE_RATE_48000;
	}

	return AUDIO_SAMPLE_RATE_48000;
}

static void ENC_3531AGetViInfo(ENC_3531AViMode ViMode, ENC_3531AViInfo *pViInfo)
{
	switch (ViMode)
	{
	case VI_MODE_PAL:
		pViInfo->m_Vimode = VI_MODE_D1_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_PAL;
		pViInfo->m_PicSize = PIC_D1;
		pViInfo->m_FrmRate = 25;
		pViInfo->m_DieMode = VPSS_DIE_MODE_DIE;
		break;
	case VI_MODE_NTSC:
		pViInfo->m_Vimode = VI_MODE_D1_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_NTSC;
		pViInfo->m_PicSize = PIC_D1;
		pViInfo->m_FrmRate = 2997;
		pViInfo->m_DieMode = VPSS_DIE_MODE_DIE;
		break;
	case VI_MODE_720P_50:
		pViInfo->m_Vimode = VI_MODE_720P_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD720;
		pViInfo->m_FrmRate = 50;
		pViInfo->m_DieMode = VPSS_DIE_MODE_NODIE;
		break;
	case VI_MODE_720P_5994:
		pViInfo->m_Vimode = VI_MODE_720P_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD720;
		pViInfo->m_FrmRate = 5994;
		pViInfo->m_DieMode = VPSS_DIE_MODE_NODIE;
		break;
	case VI_MODE_720P_60:
		pViInfo->m_Vimode = VI_MODE_720P_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD720;
		pViInfo->m_FrmRate = 60;
		pViInfo->m_DieMode = VPSS_DIE_MODE_NODIE;
		break;
	case VI_MODE_1080I_50:
		pViInfo->m_Vimode = VI_MODE_1080I_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD1080;
		pViInfo->m_FrmRate = 25;
		pViInfo->m_DieMode = VPSS_DIE_MODE_DIE;
		break;
	case VI_MODE_1080I_5994:
		pViInfo->m_Vimode = VI_MODE_1080I_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD1080;
		pViInfo->m_FrmRate = 2997;
		pViInfo->m_DieMode = VPSS_DIE_MODE_DIE;
		break;
	case VI_MODE_1080I_60:
		pViInfo->m_Vimode = VI_MODE_1080I_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD1080;
		pViInfo->m_FrmRate = 30;
		pViInfo->m_DieMode = VPSS_DIE_MODE_DIE;
		break;
	case VI_MODE_1080P_50:
		pViInfo->m_Vimode = VI_MODE_1080P_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD1080;
		pViInfo->m_FrmRate = 50;
		pViInfo->m_DieMode = VPSS_DIE_MODE_NODIE;
		break;
	case VI_MODE_1080P_5994:
		pViInfo->m_Vimode = VI_MODE_1080P_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD1080;
		pViInfo->m_FrmRate = 5994;
		pViInfo->m_DieMode = VPSS_DIE_MODE_NODIE;
		break;
	case VI_MODE_1080P_60:
		pViInfo->m_Vimode = VI_MODE_1080P_BT1120_1MUX_SINGLE;
		pViInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pViInfo->m_PicSize = PIC_HD1080;
		pViInfo->m_FrmRate = 60;
		pViInfo->m_DieMode = VPSS_DIE_MODE_NODIE;
		break;
	default:
		break;
	}
}

static void ENC_3531AGetVoInfo(ENC_3531AVoMode VoMode, ENC_3531AVoInfo *pVoInfo, ENC_3531AViInfo *pViInfo)
{
	switch (VoMode)
	{
	case VO_MODE_576P_25:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_PAL;
		pVoInfo->m_PicSize = PIC_D1;
		pVoInfo->m_FrmRate = 25;
		break;
	case VO_MODE_480P_2997:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_NTSC;
		pVoInfo->m_PicSize = PIC_D1;
		pVoInfo->m_FrmRate = 2997;
		break;
	case VO_MODE_720P_25:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD720;
		pVoInfo->m_FrmRate = 25;
		break;
	case VO_MODE_720P_2997:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD720;
		pVoInfo->m_FrmRate = 2997;
		break;
	case VO_MODE_720P_30:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD720;
		pVoInfo->m_FrmRate = 30;
		break;
	case VO_MODE_720P_50:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD720;
		pVoInfo->m_FrmRate = 50;
		break;
	case VO_MODE_720P_5994:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD720;
		pVoInfo->m_FrmRate = 5994;
		break;
	case VO_MODE_720P_60:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD720;
		pVoInfo->m_FrmRate = 60;
		break;
	case VO_MODE_1080P_25:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD1080;
		pVoInfo->m_FrmRate = 25;
		break;
	case VO_MODE_1080P_2997:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD1080;
		pVoInfo->m_FrmRate = 2997;
		break;
	case VO_MODE_1080P_30:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD1080;
		pVoInfo->m_FrmRate = 30;
		break;
	case VO_MODE_1080P_50:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD1080;
		pVoInfo->m_FrmRate = 50;
		break;
	case VO_MODE_1080P_5994:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD1080;
		pVoInfo->m_FrmRate = 5994;
		break;
	case VO_MODE_1080P_60:
		pVoInfo->m_ViNorm = VIDEO_ENCODING_MODE_AUTO;
		pVoInfo->m_PicSize = PIC_HD1080;
		pVoInfo->m_FrmRate = 60;
		break;
	case VO_MODE_AUTO: /* 自动输出模式与输入的分辨率和帧率完全一样 */
	default:
		pVoInfo->m_ViNorm = pViInfo->m_ViNorm;
		pVoInfo->m_PicSize = pViInfo->m_PicSize;
		pVoInfo->m_FrmRate = pViInfo->m_FrmRate;
		break;
	}
}

static void ENC_3531ACcValidClean(ENC_3531AHandle *pHandle, U32 ChnIndex)
{
	U32 i;

	for (i = 0; i < MPEG2_TS_PACKET_MAX_PID_NUM; i++) {
		pHandle->m_pMonitorParam[ChnIndex].m_pPidCc[i] = 0;
		pHandle->m_pMonitorParam[ChnIndex].m_pPidCcRstFlag[i] = HI_TRUE;
		pHandle->m_pMonitorParam[ChnIndex].m_pPidCcCount[i] = 0;
	}
}

static void ENC_3531ACcValid(ENC_3531AMonitorParam *pMonitor, S32 ChnIndex, U8 *pTs)
{
	MPEG2_TsHead lTsHeader;

	if (pTs[0] != MPEG2_TS_PACKET_SYN_BYTE) {
		GLOBAL_TRACE((ANSI_COLOR_RED"SYNC Byte ERR!!!! %#x\n"ANSI_COLOR_NONE, pTs[0]));
		return;
	}

	MPEG2_TsHeadParser(&lTsHeader, pTs);
	if (lTsHeader.pid == MPEG2_TS_PACKET_NULL_PID || lTsHeader.adaptation_field_control == 0x02) { /* 空包和调整字段包没有连续计数判断 */
		return;
	}

	if(pMonitor->m_pPidCcRstFlag[lTsHeader.pid]) { /* 初值 */
		pMonitor->m_pPidCc[lTsHeader.pid] = lTsHeader.continuity_counter;
		pMonitor->m_pPidCcRstFlag[lTsHeader.pid] = HI_FALSE;
		return;
	}

	if (++pMonitor->m_pPidCc[lTsHeader.pid] == 0x10) {
		pMonitor->m_pPidCc[lTsHeader.pid] = 0;
	}
	if (pMonitor->m_pPidCc[lTsHeader.pid] != lTsHeader.continuity_counter) {
		pMonitor->m_pPidCcCount[lTsHeader.pid]++;
		GLOBAL_TRACE((ANSI_COLOR_YELLOW"CHN[%d] PID[%d] CC Error[%d]! Expect %#x, real %#x\n"ANSI_COLOR_NONE, 
			ChnIndex, lTsHeader.pid, pMonitor->m_pPidCcCount[lTsHeader.pid], 
			pMonitor->m_pPidCc[lTsHeader.pid], lTsHeader.continuity_counter));
		pMonitor->m_pPidCc[lTsHeader.pid] = lTsHeader.continuity_counter;
	}
}

static void ENC_3531AConstructRtp(U8 *pDataBuf, U16 RtpSn)
{
	U8 plRtp[RTP_PACKET_LEN] = {0x80, 0xa1, 0x00, 0x00, 0x0a, 0xa2, 0xe9, 0x0e,
		0x54, 0x48, 0x00, 0x10};

	memcpy(pDataBuf, plRtp, RTP_PACKET_LEN);
	pDataBuf[2] = (RtpSn >> 8) & 0xFF;
	pDataBuf[3] = RtpSn & 0xFF;
}

static BOOL ENC_3531AEthOutSendData(HANDLE32 lSocketHandle, U32 DestIp, U16 DestPort, U8 *pData, S32 DataLen)
{
	S32 lActLen;

	if (s_pHandle->m_InitParam.m_GetEthSetFlagCB()) { /* 设置网络参数的时候，需要将数据丢弃，否则会导致出现编码溢出和码流过大情况 */
		return FALSE;
	}
	else {
		return PFC_SocketSendTo(lSocketHandle, pData, DataLen, &lActLen, DestIp, DestPort) && (lActLen == DataLen);
	}
}

static void ENC_3531AVolSet(HI_S16 *pData, HI_S32 DataLen, HI_S32 Db)
{
	HI_S32 i;
	HI_S32 lPcmVal;
	F32 lMultiplier = pow(10.0, (F32)Db/20.0);

	for (i = 0; i < DataLen; i++) {
		lPcmVal = *(pData + i) * lMultiplier;
		if (lPcmVal < 32767 && lPcmVal > -32768) {
			*(pData + i) = lPcmVal;
		} 
		else if (lPcmVal > 32767) {
			*(pData + i) = 32767;
		} 
		else if (lPcmVal < -32768) {
			*(pData + i) = -32768;
		}
	}
}

/* Ai 输入数据处理，将左右声道的数据组合起来 */
static void ENC_3531AAiDataProc(HANDLE32 Handle, HI_U16 *Ai_stereo, HI_U16 *Ai_left, HI_U16 *Ai_right, HI_S32 *pCount)
{
	ENC_3531AAudProcParam *plAudProcParam = (ENC_3531AAudProcParam  *)Handle;
	HI_S32 lCountBytes = 0;/* 单位：字节 */
	HI_S32 i = 0, j = 0;

	memset(Ai_stereo, 0, sizeof(HI_U16) * SAMPLE_AUDIO_PTNUMPERFRM * 2);
	if ((plAudProcParam->m_AudEncMode == AUD_ENC_MODE_AC3) ||
		(plAudProcParam->m_AudEncMode == AUD_ENC_MODE_EAC3)) {	
		while(i < SAMPLE_AUDIO_PTNUMPERFRM) {
			if ((0x5AA5 == Ai_left[i]) && (0x5AA5 == Ai_right[i])) { /* 去除无效数据 */
				lCountBytes += 4; /* 左右声道个两个字节 */
			}
			else { 	
				Ai_stereo[j ] = Ai_left[i];
				Ai_stereo[j + 1] = Ai_right[i];
				j += 2;
			} 
			i++;
		}            
	} 
	else {
		while(i < SAMPLE_AUDIO_PTNUMPERFRM) {				
			Ai_stereo[j] = Ai_left[i];
			Ai_stereo[j + 1] = Ai_right[i];
			j += 2;	
			i++;
		}
	}

	*pCount = lCountBytes; /* 无效数据长度 */
}

/* AC3 数据处理，AC3 必须一帧一帧地传，所以要寻找满一帧后再传送 
   注意：由于 AC3 的数据速率慢于 I2s 48K 的采样率，所以不会出现在一个接收的 AI 帧中存在两个 AC3 帧的情况，程序中也不进行该类处理。
*/
static BOOL ENC_3531AProcAc3Data(ENC_3531AAudProcParam *plAudProcParam, ENC_3531AAC3Param *pAc3Param, HI_U16 *Ai_stereo, S32 BufSize)
{
	U8 lTmpBuf[SAMPLE_AUDIO_PTNUMPERFRM * 4];
	HI_S32 lTmpBufSize = 0;
	HI_S32 lcount = 0;
	HI_S32 x = 0;
	HI_S32 i = 0;

	if (0 == BufSize) {
		return FALSE;
	}

	GLOBAL_MEMSET(pAc3Param->m_BufBefor, 0, SAMPLE_AUDIO_PTNUMPERFRM * 4);
	GLOBAL_MEMCPY(pAc3Param->m_BufBefor, pAc3Param->m_BufNow, pAc3Param->m_BufNowSize);
	pAc3Param->m_BufBeforSize = pAc3Param->m_BufNowSize;
	GLOBAL_MEMSET(pAc3Param->m_BufNow, 0, SAMPLE_AUDIO_PTNUMPERFRM * 4);
	GLOBAL_MEMCPY(pAc3Param->m_BufNow, (U8 *)Ai_stereo, BufSize);
	pAc3Param->m_BufNowSize = BufSize;
	pAc3Param->m_Ac3FrameSizeBefor = pAc3Param->m_Ac3FrameSize;

	if ((pAc3Param->m_BufNow[0] == 0x0B) && (pAc3Param->m_BufNow[1] == 0x77)) { /* 恰好 AC3 的头在头两个字节 */
		if ((0 == pAc3Param->m_Ac3Counter ) && (pAc3Param->m_Ac3StartFlag != TRUE)) { /* 上一帧 AC3 数据已经取完，这一帧还未开始 */
			if (( pAc3Param->m_BufBeforSize > 1)) { /* AC3 帧前面有两个字节用于指定 AC3 帧的长度 */
				pAc3Param->m_Ac3FrameSize = ((pAc3Param->m_BufBefor[ pAc3Param->m_BufBeforSize - 2] << 8 & 0XFF00) 
											| (pAc3Param->m_BufBefor[ pAc3Param->m_BufBeforSize - 1] & 0X00FF));
				
				/* 帧长校验 */
				if (pAc3Param->m_Ac3FrameSize > 1920 * 2) { /* 大于最大长度，无效 */
					GLOBAL_TRACE(("Ac3FrameSize Error!! Ac3FrameSize: %d\n", pAc3Param->m_Ac3FrameSize));
					plAudProcParam->m_AudProcCmd = AUD_PROC_REINIT;
					return FALSE;
				}
				if (pAc3Param->m_Ac3FrameSize != pAc3Param->m_Ac3FrameSizeBefor) { /* 帧长发生变化，清空buffer，重新初始化 */
					GLOBAL_TRACE(("Ac3FrameSize Change!! Old Frame Size: %d, New Frame Size: %d\n", pAc3Param->m_Ac3FrameSizeBefor, pAc3Param->m_Ac3FrameSize));
					plAudProcParam->m_AudProcCmd = AUD_PROC_REINIT;
					return FALSE;
				}

				pAc3Param->m_Ac3Counter = pAc3Param->m_Ac3FrameSize;
				pAc3Param->m_Ac3ValidFlag = TRUE;
				lcount = 0;
				GLOBAL_NAMED_TRACE(pAc3Param->m_Ac3Counter, d);
			} 
			else {
				GLOBAL_TRACE(("lBufBefor Erro!! pAc3Param->m_BufBeforSize: %d!!\n",pAc3Param->m_BufBeforSize));
			}	
		}	
	}

	/* 在 buffer 中间搜索 AC3 的头 */
	for (x = 2; x < pAc3Param->m_BufNowSize - 1; x++) {
		if ((pAc3Param->m_BufNow[x] == 0x0B) && (pAc3Param->m_BufNow[x + 1] == 0x77)) {			
			if (((x-2) == pAc3Param->m_Ac3Counter ) || (pAc3Param->m_Ac3StartFlag == 1)) {	
				if (!pAc3Param->m_Ac3StartFlag) {
					lTmpBufSize = 0;
					memset(lTmpBuf, 0, sizeof(U8) * SAMPLE_AUDIO_PTNUMPERFRM * 4);		
					while (pAc3Param->m_Ac3Counter > 0) { /* AC3 头之前的数据全是上一帧 AC3 的有效数据 */
						lTmpBuf[lTmpBufSize] = pAc3Param->m_BufNow[lTmpBufSize];
						lTmpBufSize ++;
						pAc3Param->m_Ac3Counter --;
					}													
					RngBufPut(pAc3Param->m_pAC3RngBuf, lTmpBuf, lTmpBufSize);
				}
				else {
					GLOBAL_TRACE(("pAc3Param->m_Ac3Counter: %d!!X: %d!!\n",pAc3Param->m_Ac3Counter, x));
				}
				lTmpBufSize = 0;
				memset(lTmpBuf, 0, sizeof(U8) * SAMPLE_AUDIO_PTNUMPERFRM * 4);
				pAc3Param->m_Ac3FrameSize = ((pAc3Param->m_BufNow[x - 2] << 8 & 0xFF00) 
											| (pAc3Param->m_BufNow[x - 1] & 0xFF));
				if (pAc3Param->m_Ac3FrameSize > 1920 * 2) { /* 大于最大长度，无效 */
					GLOBAL_TRACE(("Ac3FrameSize Error!! Ac3FrameSize: %d\n", pAc3Param->m_Ac3FrameSize));
					plAudProcParam->m_AudProcCmd = AUD_PROC_REINIT;
					return FALSE;
				}
				if (!pAc3Param->m_Ac3StartFlag) {
					if (pAc3Param->m_Ac3FrameSize != pAc3Param->m_Ac3FrameSizeBefor) { /* 帧长发生变化，清空buffer，重新初始化 */
						GLOBAL_TRACE(("Ac3FrameSize Change!! Old Frame Size: %d, New Frame Size: %d\n", pAc3Param->m_Ac3FrameSizeBefor, pAc3Param->m_Ac3FrameSize));
						plAudProcParam->m_AudProcCmd = AUD_PROC_REINIT;
						return FALSE;
					}
				}
				pAc3Param->m_Ac3Counter = pAc3Param->m_Ac3FrameSize;
				pAc3Param->m_Ac3ValidFlag = TRUE;
				pAc3Param->m_Ac3StartFlag = FALSE;
				lcount = x;
				break;
			}
			else {
				//GLOBAL_TRACE(("pAc3Param->m_Ac3Counter: %d!!X: %d!!\n",pAc3Param->m_Ac3Counter, x));
			}
		}
	}

	if(pAc3Param->m_Ac3ValidFlag) {
		lTmpBufSize =  0;
		memset(lTmpBuf, 0, sizeof(U8) * SAMPLE_AUDIO_PTNUMPERFRM * 4);
		for (i = 0; i + lcount < pAc3Param->m_BufNowSize; i ++) {
			lTmpBuf[i] = pAc3Param->m_BufNow[i + lcount];
			lTmpBufSize ++;
			pAc3Param->m_Ac3Counter --;										
			if(pAc3Param->m_Ac3Counter < 1) {
				pAc3Param->m_Ac3ValidFlag = FALSE;
				break;
			}
		}
		RngBufPut(pAc3Param->m_pAC3RngBuf, lTmpBuf, lTmpBufSize);
	}

	 return TRUE;
}

/* 音频处理线程 */
static void ENC_3531AAudioProcThread(void *pParam)
{
	ENC_3531AHandle *plHandle = s_pHandle;
	AI_CHN lAiChn = (int)pParam;
	ENC_3531AAudProcParam *plAudProcParam = &plHandle->m_pAudProcParam[lAiChn];
	AI_CHN lAiChn2 = lAiChn; 
	
	HI_S32 i = 0;
	HI_S32 s32Ret = 0;
	AUDIO_DEV lAiDev = SAMPLE_AUDIO_AI_DEV;
	HI_S32 lCountBytes = 0;/*单位：字节*/
	HI_S32 lAiFd;
	fd_set lReadFds;
	AUDIO_FRAME_S stFrame; 
	struct timeval TimeoutVal;
	HI_U16 Ai_left[SAMPLE_AUDIO_PTNUMPERFRM];
	HI_U16 Ai_right[SAMPLE_AUDIO_PTNUMPERFRM];
	HI_U16 Ai_stereo[SAMPLE_AUDIO_PTNUMPERFRM * 2 ];
	HI_U64 lPtsTimeStamp;/* MPEG模式时PTS时间戳 */
	RNGBUF_t *plAudDelayRngBuf;
	ENC_3531AAC3Param lAc3Param;

	lAc3Param.m_Ac3Counter = 0;
	lAc3Param.m_Ac3FrameSize = 0;
	lAc3Param.m_Ac3FrameSizeBefor = 0;
	lAc3Param.m_Ac3StartFlag = TRUE;
	lAc3Param.m_Ac3ValidFlag = FALSE;

	if (!GLOBAL_CHECK_INDEX(lAiChn, plHandle->m_InitParam.m_ChnNum)) {
		GLOBAL_TRACE((ANSI_COLOR_RED"Index ERROR!!\n"ANSI_COLOR_NONE));
		return;
	}
	plAudDelayRngBuf = RngCreate(AUD_DELAY_BUFF_SIZE);
	if (!plAudDelayRngBuf) {
		GLOBAL_TRACE(("RngCreate Failed!\n"));
		return;
	}
	lAc3Param.m_pAC3RngBuf = RngCreate(AC3_BUFF_SIZE);
	if (!lAc3Param.m_pAC3RngBuf) {
		GLOBAL_TRACE(("AC3RngBuf Failed!\n"));
		return;
	}

	while (plHandle->m_TaskRunFlg) {
		if ((plAudProcParam->m_AudEncMode == AUD_ENC_MODE_AC3)
			|| (plAudProcParam->m_AudEncMode == AUD_ENC_MODE_EAC3)) { /* 选择通道 */
			lAiChn2 = lAiChn + 4; /* AC3 的数据在后 4 个通道 */
		}
		else {
			lAiChn2 = lAiChn;
		}
		switch (plAudProcParam->m_AudProcCmd) {
			case AUD_PROC_REINIT:
				{
					AUD_EncInitParam lInitParam;
					if (plAudProcParam->m_AudEncHandle) {
						AUD_EncDestroy(plAudProcParam->m_AudEncHandle);
						plAudProcParam->m_AudEncHandle = NULL;
					}
					lInitParam.m_AudEncMode = plAudProcParam->m_AudEncMode;
					lInitParam.m_AudBitrate = ENC_3531AGetAudBitrate(plAudProcParam->m_AudBitrate);
					lInitParam.m_AudSample = ENC_3531AGetAudSample(plAudProcParam->m_AudSample);
					if ((plAudProcParam->m_AudEncHandle = AUD_EncCreate(&lInitParam)) == NULL) {
						GLOBAL_TRACE((ANSI_COLOR_RED"AUD_EncCreate Failed!\n"ANSI_COLOR_NONE));
						break;
					}
					RngFlush(plAudDelayRngBuf);
					RngFlush(lAc3Param.m_pAC3RngBuf);
					lAc3Param.m_Ac3Counter = 0;
					lAc3Param.m_Ac3StartFlag = TRUE;
					lAc3Param.m_Ac3ValidFlag = FALSE;
					
					plAudProcParam->m_AudProcCmd = AUD_PROC_START;
				}
				break;
			case AUD_PROC_START:  
				{
					U8 *plRetBuf;
					S32 lRetBufSize;

					if (!plAudProcParam->m_SignalIsLocked) {
						PL_TaskSleep(10);
						continue;
					}

					PFC_SemaphoreWait(plAudProcParam->m_ParamSetLock, -1);
					TimeoutVal.tv_sec = 1;
					TimeoutVal.tv_usec = 0;

					FD_ZERO(&lReadFds);
					lAiFd = HI_MPI_AI_GetFd(lAiDev, lAiChn2);
					FD_SET(lAiFd,&lReadFds);
					PFC_SemaphoreSignal(plAudProcParam->m_ParamSetLock);
					s32Ret = select(lAiFd + 1, &lReadFds, NULL, NULL, &TimeoutVal);
					PFC_SemaphoreWait(plAudProcParam->m_ParamSetLock, -1);
					if (s32Ret < 0) {
						GLOBAL_TRACE(("select failed:%d!\n",lAiChn));
					}
					else if (0 == s32Ret) {
						GLOBAL_TRACE(("get ai frame select time out:%d!\n",lAiChn));
						PFC_SemaphoreSignal(plAudProcParam->m_ParamSetLock);
						continue;
					}        

					if (FD_ISSET(lAiFd, &lReadFds)) {
						BOOL lRetAudEncStatus = FALSE;
						ENC_3531ATimeStamp lTimeStamp; 

						/* get frame from ai chn */
						s32Ret = HI_MPI_AI_GetFrame(lAiDev, lAiChn2, &stFrame, NULL, HI_FALSE); /* 非阻塞模式，没有音频输入则报错 */
						if (HI_SUCCESS != s32Ret) {
							printf("%s: HI_MPI_AI_GetFrame(%d, %d), failed with %#x!\n",\
								__FUNCTION__, lAiDev, lAiChn, s32Ret);  
							PFC_SemaphoreSignal(plAudProcParam->m_ParamSetLock);
							continue ;
						}

						if (stFrame.u32Len != SAMPLE_AUDIO_PTNUMPERFRM * 2) { /* 获取到的数据错误 */
							GLOBAL_TRACE((ANSI_COLOR_RED"Audio Expect Read Datas: %d, real value: %d\n"ANSI_COLOR_NONE, 
								SAMPLE_AUDIO_PTNUMPERFRM * 2, stFrame.u32Len));
							s32Ret = HI_MPI_AI_ReleaseFrame(lAiDev, lAiChn2, &stFrame, NULL);
							if (HI_SUCCESS != s32Ret) {
								printf("%s: HI_MPI_AI_ReleaseFrame(%d, %d), failed with %#x!\n",\
									__FUNCTION__, lAiDev, lAiChn, s32Ret);
							}
							PFC_SemaphoreSignal(plAudProcParam->m_ParamSetLock);
							continue ;
						}

						/* CAUTION: 这里采样数据是固定的单声道 SAMPLE_AUDIO_PTNUMPERFRM 个点，即 SAMPLE_AUDIO_PTNUMPERFRM * 2 个字节 */
						memcpy(Ai_left, stFrame.pVirAddr[0], sizeof(HI_U16) * SAMPLE_AUDIO_PTNUMPERFRM);
						memcpy(Ai_right, stFrame.pVirAddr[1], sizeof(HI_U16) * SAMPLE_AUDIO_PTNUMPERFRM);
						ENC_3531AAiDataProc(plAudProcParam, Ai_stereo, Ai_left, Ai_right, &lCountBytes);

						if((plAudProcParam->m_AudEncMode == AUD_ENC_MODE_AC3) ||
							(plAudProcParam->m_AudEncMode == AUD_ENC_MODE_EAC3)) {
							BOOL lDataStatus;

							plRetBuf = (U8 *)Ai_stereo;
							lRetBufSize = SAMPLE_AUDIO_PTNUMPERFRM * 4 - lCountBytes;

							lDataStatus = ENC_3531AProcAc3Data(plAudProcParam, &lAc3Param, Ai_stereo, lRetBufSize);

							if (RngNBytes(lAc3Param.m_pAC3RngBuf) >= lAc3Param.m_Ac3FrameSize) {
								RngBufGet(lAc3Param.m_pAC3RngBuf, plRetBuf, lAc3Param.m_Ac3FrameSize);
								lRetBufSize = lAc3Param.m_Ac3FrameSize;
								ENC_3531AGetTimeStamp(stFrame.u64TimeStamp  + plAudProcParam->m_AudPtsRelativeDelayTime, 
									plAudProcParam->m_PtsDelayTime, &lTimeStamp);
								lRetAudEncStatus = lDataStatus;
							}

						}
						else {
							S32 lInputNeedBytes;
							
							if (plAudProcParam->m_AudEncMode == AUD_ENC_MODE_LC_AAC) {
								/* 针对 AAC，传入的数据必须是输入采样点要求的数据长度（4096Byte） */
								lInputNeedBytes = AUD_EncAacGetInputBufSize(plAudProcParam->m_AudEncHandle);
							}
							else if (plAudProcParam->m_AudEncMode == AUD_ENC_MODE_MPEG1_L2) {
								/* 针对 MPEG1，传入的数据是单个通道输入采样点的数据长度（2048Byte） */
								lInputNeedBytes = SAMPLE_AUDIO_PTNUMPERFRM * 2;
							}

							ENC_3531AVolSet((HI_S16 *)Ai_stereo, SAMPLE_AUDIO_PTNUMPERFRM * 2, plAudProcParam->m_AudVolume);
							RngBufPut(plAudDelayRngBuf, (char*)Ai_stereo, stFrame.u32Len * 2);
							plAudProcParam->m_AudDelayFrameNum = GLOBAL_MAX(plAudProcParam->m_AudDelayFrameNum, 0);
							plAudProcParam->m_AudDelayFrameNum = GLOBAL_MIN(plAudProcParam->m_AudDelayFrameNum, 250);
							if (RngNBytes(plAudDelayRngBuf) > plAudProcParam->m_AudDelayFrameNum * SAMPLE_AUDIO_PTNUMPERFRM * 4) {
								RngBufGet(plAudDelayRngBuf, (char*)Ai_stereo, SAMPLE_AUDIO_PTNUMPERFRM * 4);
								lRetAudEncStatus = AUD_EncProcess(plAudProcParam->m_AudEncHandle, (U8 *)Ai_stereo, lInputNeedBytes, &plRetBuf, &lRetBufSize);
								if (plAudProcParam->m_AudEncMode == AUD_ENC_MODE_MPEG1_L2) {
									/* 采样点为1024，MPEG编码器按照1152送数据，每隔8帧数据就会有一个输出长度为0的点（1024*9=1152*8）*/
									if(lRetBufSize == 0){
										/* 此时取系统的PTS */
										lPtsTimeStamp = stFrame.u64TimeStamp + plAudProcParam->m_AudPtsRelativeDelayTime;	
										lRetAudEncStatus = FALSE;
									}
									else{	
										lPtsTimeStamp = lPtsTimeStamp + 24000; /* 固定PTS间隔为24ms, 24ms 是在 48KHz 采样率获取到 1152 个采样点的时间 */
									}
									ENC_3531AGetTimeStamp(lPtsTimeStamp, 
										plAudProcParam->m_PtsDelayTime, &lTimeStamp);	
								}
								else { /* AAC */
									ENC_3531AGetTimeStamp(stFrame.u64TimeStamp + plAudProcParam->m_AudPtsRelativeDelayTime, 
										plAudProcParam->m_PtsDelayTime, &lTimeStamp);
								}
							}
						}
						
						if (lRetAudEncStatus) {	
							mpegts_write_packet(plAudProcParam->m_pEs2TsObj,
								plRetBuf,
								lRetBufSize,
								lTimeStamp.m_Pcr,
								lTimeStamp.m_Pts,
								lTimeStamp.m_Dts
								);
						}
						/* finally you must release the stream */
						s32Ret = HI_MPI_AI_ReleaseFrame(lAiDev, lAiChn2, &stFrame, NULL);
						if (HI_SUCCESS != s32Ret) {
							printf("%s: HI_MPI_AI_ReleaseFrame(%d, %d), failed with %#x!\n",\
								__FUNCTION__, lAiDev, lAiChn, s32Ret);
							PFC_SemaphoreSignal(plAudProcParam->m_ParamSetLock);
							continue;
						}
					}
					PFC_SemaphoreSignal(plAudProcParam->m_ParamSetLock);
				}
				break;
			case AUD_PROC_STOP:
			default:
				PL_TaskSleep(100); 
		}
	}

	if (plAudProcParam->m_AudEncHandle) {
		AUD_EncDestroy(plAudProcParam->m_AudEncHandle);
		plAudProcParam->m_AudEncHandle = NULL;
	}
	RngDestroy(plAudDelayRngBuf);
	RngDestroy(lAc3Param.m_pAC3RngBuf);
	
}

static HI_S32 ENC_3531ASendStream(ENC_3531AHandle *pHandle, VENC_STREAM_S *pstStream, VENC_CHN VencChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_S32 i;

	for (i = 0; i < pstStream->u32PackCount; i++) {
		ENC_3531ATimeStamp lTimeStamp; 

		ENC_3531AGetTimeStamp(pstStream->pstPack[i].u64PTS, pHandle->m_pVidProcParam[VencChn].m_PtsDelayTime, &lTimeStamp);
		mpegts_write_packet(pHandle->m_pVidProcParam[VencChn].m_pEs2TsObj,
			pstStream->pstPack[i].pu8Addr + pstStream->pstPack[i].u32Offset,
			pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset, 
			lTimeStamp.m_Pcr,
			lTimeStamp.m_Pts, 
			lTimeStamp.m_Dts);
	}

	return s32Ret;
}

/* 视频处理线程 */
static void ENC_3531AVideoProcThread(void *pParam)
{
	ENC_3531AHandle *plHandle = s_pHandle;
	S32 lChn = (int)pParam;
	ENC_3531AVidProcParam *plVid = &plHandle->m_pVidProcParam[lChn];
	HI_S32 lVencFd;
	fd_set lReadFds;
	struct timeval lTimeoutVal;
	HI_S32 s32Ret;
	VENC_STREAM_S stStream;
	VENC_CHN_STAT_S stStat;
	HI_S32 lVencChn = lChn;

	if (!GLOBAL_CHECK_INDEX(lChn, plHandle->m_InitParam.m_ChnNum)) {
		GLOBAL_TRACE((ANSI_COLOR_RED"Index ERROR!!\n"ANSI_COLOR_NONE));
		return;
	}

	while (plHandle->m_TaskRunFlg) {
		PFC_SemaphoreWait(plVid->m_ParamSetLock, -1);
		FD_ZERO(&lReadFds);
		lVencFd = HI_MPI_VENC_GetFd(lVencChn);
		if (lVencFd < 0) {
			GLOBAL_TRACE(("HI_MPI_VENC_GetFd Failed!\n"));
			PFC_SemaphoreSignal(plVid->m_ParamSetLock);
			continue;
		}
		FD_SET(lVencFd, &lReadFds);

		lTimeoutVal.tv_sec  = 1;
		lTimeoutVal.tv_usec = 0;
		PFC_SemaphoreSignal(plVid->m_ParamSetLock);
		s32Ret = select(lVencFd + 1, &lReadFds, NULL, NULL, &lTimeoutVal); 
		PFC_SemaphoreWait(plVid->m_ParamSetLock, -1);
		if (plVid->m_ActiveMark) {
			if (s32Ret < 0) {
				GLOBAL_TRACE(("select failed!\n"));
			}
			else if (s32Ret == 0) {
				GLOBAL_TRACE((ANSI_COLOR_YELLOW"get venc[%d] stream time out!\n"ANSI_COLOR_NONE, lVencChn));
			}
			else {
				if (FD_ISSET(lVencFd, &lReadFds)) {
					/*******************************************************
					 step 2.1 : query how many packs in one-frame stream.
					*******************************************************/
					memset(&stStream, 0, sizeof(stStream));
					s32Ret = HI_MPI_VENC_Query(lVencChn, &stStat);
					if (HI_SUCCESS != s32Ret) {
						PFC_SemaphoreSignal(plVid->m_ParamSetLock);
						PL_TaskSleep(1);
						continue; /* Venc 还未开始，select 也会存在数据 */
					}
					/*******************************************************
						step 2.2 : suggest to check both u32CurPacks and u32LeftStreamFrames at the same time, for example:
					   if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
						{
							  SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
							  continue;
						}
					*******************************************************/
					if(0 == stStat.u32CurPacks) {
					  GLOBAL_TRACE(("NOTE: Current  frame is NULL!\n"));
					  PFC_SemaphoreSignal(plVid->m_ParamSetLock);
					  continue;
					}

					/*******************************************************
					 step 2.3 : malloc corresponding number of pack nodes.
					*******************************************************/
					stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
					if (NULL == stStream.pstPack) {
						GLOBAL_TRACE(("malloc stream pack failed!\n"));
						PFC_SemaphoreSignal(plVid->m_ParamSetLock);
						break;
					}
	                
					/*******************************************************
					 step 2.4 : call mpi to get one-frame stream
					*******************************************************/
					stStream.u32PackCount = stStat.u32CurPacks;
					
					s32Ret = HI_MPI_VENC_GetStream(lVencChn, &stStream, HI_TRUE);
					if (HI_SUCCESS != s32Ret) {
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						GLOBAL_TRACE(("HI_MPI_VENC_GetStream failed with %#x!\n", \
							   s32Ret));
						PFC_SemaphoreSignal(plVid->m_ParamSetLock);
						continue;
					}
					
					if (plVid->m_SignalIsLocked) {
						s32Ret = ENC_3531ASendStream(plHandle, &stStream, lVencChn);
						if (HI_SUCCESS != s32Ret) {
							free(stStream.pstPack);
							stStream.pstPack = NULL;
							GLOBAL_TRACE(("process stream failed!\n"));
							PFC_SemaphoreSignal(plVid->m_ParamSetLock);
							continue;
						}
					}
					else {
						PL_TaskSleep(10);
					}

					/*******************************************************
					 step 2.6 : release stream
					*******************************************************/
					s32Ret = HI_MPI_VENC_ReleaseStream(lVencChn, &stStream);
					if (HI_SUCCESS != s32Ret) {
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						PFC_SemaphoreSignal(plVid->m_ParamSetLock);
						continue;
					}
					/*******************************************************
					 step 2.7 : free pack nodes
					*******************************************************/
					free(stStream.pstPack);
					stStream.pstPack = NULL;
				} 
			}
			PFC_SemaphoreSignal(plVid->m_ParamSetLock);
		}
		else {
			PFC_SemaphoreSignal(plVid->m_ParamSetLock);
			PL_TaskSleep(100);
		}
	}
}

/* TS 流发送线程 */
static void ENC_3531ATsProcThread(void *pParam)
{
	ENC_3531AHandle *plHandle = (ENC_3531AHandle *)pParam;
	U32 i;

	while(plHandle->m_TaskRunFlg) { 
		if (IP_OUT_TYPE_SPTS == plHandle->m_IpOutputType) {
			for (i = 0; i < plHandle->m_InitParam.m_ChnNum; i++) {
				ENC_3531ATsProcParam *plTs = &plHandle->m_pTsProcParam[i];
				ENC_3531AVidProcParam *plVid = &plHandle->m_pVidProcParam[i];

				PFC_SemaphoreWait(plVid->m_TsProcLock, -1); /* 确保在修改 plVid->m_pEs2TsObj 的参数时，不能处于 mpegts_timer 中 */
				if (plVid->m_ActiveMark) {	
					mpegts_timer(plVid->m_pEs2TsObj, plTs->m_pVidTsRngBuf, plTs->m_pAudTsRngBuf, plTs->m_pInsertTsRngBuf); 
				}
				PFC_SemaphoreSignal(plVid->m_TsProcLock);
			}
		}
		else { /* MPTS */
			ENC_3531ATsProcParam *plTs;
			ENC_3531AVidProcParam *plVid;
			BOOL lTsOutputFlag = FALSE; /* 是否输出 TS 流 */

			for (i = 0; i < plHandle->m_InitParam.m_ChnNum; i++) {
				plVid = &plHandle->m_pVidProcParam[i];
				if (plVid->m_ActiveMark) {
					lTsOutputFlag = TRUE;
					break;
				}
			}
			if (lTsOutputFlag) {
				plVid = &plHandle->m_pVidProcParam[0];
				PFC_SemaphoreWait(plVid->m_TsProcLock, -1); /* 确保在修改 plVid->m_pEs2TsObj 的参数时，不能处于 mpegts_timer 中 */
				mpegts_timer(plVid->m_pEs2TsObj, plTs->m_pVidTsRngBuf, plTs->m_pAudTsRngBuf, plTs->m_pInsertTsRngBuf); 
				PFC_SemaphoreSignal(plVid->m_TsProcLock);
			}
			for (i = 1; i < plHandle->m_InitParam.m_ChnNum; i++) {
				plVid = &plHandle->m_pVidProcParam[i];
				if (plVid->m_ActiveMark) {
					PFC_SemaphoreWait(plVid->m_TsProcLock, -1); /* 确保在修改 plVid->m_pEs2TsObj 的参数时，不能处于 mpegts_timer 中 */
					if (plVid->m_ActiveMark) {	
						mpegts_timer(plVid->m_pEs2TsObj, plTs->m_pVidTsRngBuf, plTs->m_pAudTsRngBuf, plTs->m_pInsertTsRngBuf); 
					}
					PFC_SemaphoreSignal(plVid->m_TsProcLock);
				}
			}
		}

		PL_TaskSleep(10);
	}
}

/* PSI 间隔插入线程 */
static void ENC_3531APsiInsertThread(void *pParam)
{
	ENC_3531AHandle *plHandle = (ENC_3531AHandle *)pParam;
	S32 i;

	while (plHandle->m_TaskRunFlg) {
		if (IP_OUT_TYPE_SPTS == plHandle->m_IpOutputType) {
			for (i = 0; i < plHandle->m_InitParam.m_ChnNum; i++) {
				ENC_3531APsiProcParam *plPsiParam = &plHandle->m_pPsiProcParam[i];
				HWL_PSI_OPTION_T *plPsiObj = plPsiParam->m_pPsiObj;

				PFC_SemaphoreWait(plPsiParam->m_Lock, -1); /* 修改 PSI 参数的线程与该线程互斥 */
				if (plPsiParam->m_ActiveMark) {
					plPsiObj->pat_Info.m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
					plPsiObj->pat_Info.m_Version = 0x01;
					plPsiObj->pat_Info.m_TsId = 0x01;
					plPsiObj->pat_Info.m_ProgramLen = 1;
					plPsiObj->pat_Info.m_ProgramInfo[0].m_PmtPid = plPsiParam->m_PmtPid;
					plPsiObj->pat_Info.m_ProgramInfo[0].m_ProgramNum = plPsiParam->m_ProgNumber;

					plPsiObj->pmt_num = 1;
					plPsiObj->pmt_Info[0].m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
					plPsiObj->pmt_Info[0].m_Version = 0x01;
					plPsiObj->pmt_Info[0].m_VidEncMode = 3;
					plPsiObj->pmt_Info[0].m_VidPid = plPsiParam->m_VidPid;
					plPsiObj->pmt_Info[0].m_AudEncMode = ENC_3531AGetAudEncMode(plPsiParam->m_AudEncMode);
					plPsiObj->pmt_Info[0].m_AudPid = plPsiParam->m_AudPid;
					plPsiObj->pmt_Info[0].m_PcrPid = plPsiParam->m_PcrPid;
					plPsiObj->pmt_Info[0].m_PmtPid = plPsiParam->m_PmtPid;
					plPsiObj->pmt_Info[0].m_ProgramNum = plPsiParam->m_ProgNumber;
					GLOBAL_STRCPY(plPsiObj->pmt_Info[0].m_pProgramName, plPsiParam->m_pProgName);

					plPsiObj->sdt_Info.m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
					plPsiObj->sdt_Info.m_TsId = 0x01;
					plPsiObj->sdt_Info.m_OnId = 0x01;
					plPsiObj->sdt_Info.m_Version = 0x01;
					plPsiObj->sdt_Info.m_Charset = plPsiParam->m_PsiCharSet;
					plPsiObj->sdt_Info.m_CharsetMark = FALSE;
					plPsiObj->sdt_Info.m_ProgramLen = 1;
					plPsiObj->sdt_Info.m_ProgramInfo[0].m_PmtPid = plPsiParam->m_PmtPid;
					plPsiObj->sdt_Info.m_ProgramInfo[0].m_ProgramNum = plPsiParam->m_ProgNumber;
					GLOBAL_STRCPY(plPsiObj->sdt_Info.m_ProgramInfo[0].m_pProgramName, plPsiParam->m_pProgName);

					mpegts_write_psi(plPsiObj);
				}
				PFC_SemaphoreSignal(plPsiParam->m_Lock);
			}
		}
		else { /* MPTS 只会在第一个通道发送 PSI 信息 */
			ENC_3531APsiProcParam *plPsiParam = &plHandle->m_pPsiProcParam[0];
			HWL_PSI_OPTION_T *plPsiObj = plPsiParam->m_pPsiObj;
			S32 lProgLen = 0; /* MPTS 中包含的节目数 */
			S32 lProgCnt = 0;
			
			for (i = 0; i < plHandle->m_InitParam.m_ChnNum; i++) {
				plPsiParam = &plHandle->m_pPsiProcParam[i];
				if (plPsiParam->m_ActiveMark) {
					lProgLen ++;
				}
			}
			if (lProgLen > 0) {
				PFC_SemaphoreWait(plPsiParam->m_Lock, -1); /* 修改 PSI 参数的线程与该线程互斥 */
				plPsiObj->pat_Info.m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
				plPsiObj->pat_Info.m_Version = 0x01;
				plPsiObj->pat_Info.m_TsId = 0x01;
				plPsiObj->pat_Info.m_ProgramLen = lProgLen;

				plPsiObj->sdt_Info.m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
				plPsiObj->sdt_Info.m_TsId = 0x01;
				plPsiObj->sdt_Info.m_OnId = 0x01;
				plPsiObj->sdt_Info.m_Version = 0x01;
				plPsiObj->sdt_Info.m_Charset = plPsiParam->m_PsiCharSet;
				plPsiObj->sdt_Info.m_CharsetMark = FALSE;
				plPsiObj->sdt_Info.m_ProgramLen = lProgLen;

				plPsiObj->pmt_num = lProgLen;
				for (i = 0; i < plHandle->m_InitParam.m_ChnNum; i++) {
					plPsiParam = &plHandle->m_pPsiProcParam[i];
					if (plPsiParam->m_ActiveMark) {
						plPsiObj->pat_Info.m_ProgramInfo[lProgCnt].m_PmtPid = plPsiParam->m_PmtPid;
						plPsiObj->pat_Info.m_ProgramInfo[lProgCnt].m_ProgramNum = plPsiParam->m_ProgNumber;

						plPsiObj->pmt_Info[lProgCnt].m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
						plPsiObj->pmt_Info[lProgCnt].m_Version = 0x01;
						plPsiObj->pmt_Info[lProgCnt].m_VidEncMode = 3;
						plPsiObj->pmt_Info[lProgCnt].m_VidPid = plPsiParam->m_VidPid;
						plPsiObj->pmt_Info[lProgCnt].m_AudEncMode = ENC_3531AGetAudEncMode(plPsiParam->m_AudEncMode);
						plPsiObj->pmt_Info[lProgCnt].m_AudPid = plPsiParam->m_AudPid;
						plPsiObj->pmt_Info[lProgCnt].m_PcrPid = plPsiParam->m_PcrPid;
						plPsiObj->pmt_Info[lProgCnt].m_PmtPid = plPsiParam->m_PmtPid;
						plPsiObj->pmt_Info[lProgCnt].m_ProgramNum = plPsiParam->m_ProgNumber;
						GLOBAL_STRCPY(plPsiObj->pmt_Info[lProgCnt].m_pProgramName, plPsiParam->m_pProgName);

						plPsiObj->sdt_Info.m_ProgramInfo[lProgCnt].m_PmtPid = plPsiParam->m_PmtPid;
						plPsiObj->sdt_Info.m_ProgramInfo[lProgCnt].m_ProgramNum = plPsiParam->m_ProgNumber;
						GLOBAL_STRCPY(plPsiObj->sdt_Info.m_ProgramInfo[lProgCnt].m_pProgramName, plPsiParam->m_pProgName);

						lProgCnt ++;
					}
				}
				mpegts_write_psi(plPsiObj);
				PFC_SemaphoreSignal(plPsiParam->m_Lock);
			}
		}

		PL_TaskSleep(100);
	}
}

/* UDP 发送 TS 包 */
static void ENC_3531AUdpSendTs(U8 *pDataBuf, U32 Len, void *pParam)
{
	ENC_3531ASendTsToFifoParam *plSendTsToFifoParam = (ENC_3531ASendTsToFifoParam *)pParam;
	S32 lIndex = plSendTsToFifoParam->m_ChnIndex;
	ENC_3531AHandle *plHandle = s_pHandle;
	ENC_3531AEthOutParam *plEth = &plHandle->m_pEthOutParam[lIndex];
	ENC_3531AMonitorParam *plMonitor = &plHandle->m_pMonitorParam[lIndex];

	if (!GLOBAL_CHECK_INDEX(lIndex, plHandle->m_InitParam.m_ChnNum)) {
		GLOBAL_TRACE((ANSI_COLOR_RED"Index ERROR!!\n"ANSI_COLOR_NONE));
		return;
	}

	if (Len != MPEG2_TS_PACKET_SIZE) {
		GLOBAL_TRACE((ANSI_COLOR_RED"Len ERROR!! expect: 188 real: %d\n"ANSI_COLOR_NONE, Len));
		return;
	}

	if (IP_OUT_TYPE_MPTS == plHandle->m_IpOutputType) { /* MPTS 只会使用通道 1 对应的 IP 地址作为目的地址 */
		plEth = &plHandle->m_pEthOutParam[0];
		plMonitor = &plHandle->m_pMonitorParam[0];
	}

	// debug
	//if (IP_OUT_TYPE_MPTS == plHandle->m_IpOutputType) {
	//	GLOBAL_NAMED_TRACE(lIndex, d);
	//	GLOBAL_NAMED_TRACE(plMonitor, p);
	//}

	/* 计数器刷新 */
	PFC_SemaphoreWait(plMonitor->m_Lock, -1);
	plMonitor->m_RtTsInfo.m_TotalTsCount ++;
	plMonitor->m_RtTsInfo.m_PidTsCount[MPEG2_TsGetPID(pDataBuf)] ++;
	ENC_3531ACcValid(plMonitor, lIndex, pDataBuf); /* 连续计数校验 */
	PFC_SemaphoreSignal(plMonitor->m_Lock);

	//if (IP_OUT_TYPE_MPTS == plHandle->m_IpOutputType) {
	//	GLOBAL_NAMED_TRACE(lIndex, d);
	//	GLOBAL_NAMED_TRACE(plMonitor, p);
	//}

	PFC_SemaphoreWait(plEth->m_Lock, -1);
	memcpy(&plEth->m_pBuf[RTP_PACKET_LEN + plEth->m_BufCount * MPEG2_TS_PACKET_SIZE], pDataBuf, MPEG2_TS_PACKET_SIZE);
	plEth->m_BufCount += 1; 

	if (plEth->m_SendTsNum > 7) {
		GLOBAL_TRACE(("========ERR!!! m_SendTsNum = %d\n", plEth->m_SendTsNum));
		PFC_SemaphoreSignal(plEth->m_Lock);
		return;
	}
	if(plEth->m_BufCount == plEth->m_SendTsNum) {
		if (plEth->m_IpProto == GS_ETH_PROTOCOL_RTP) {
			ENC_3531AConstructRtp(plEth->m_pBuf, plEth->m_RtpSn ++); /* 构建  RTP 头 */
			ENC_3531AEthOutSendData(plEth->m_SocketHandle, plEth->m_DestIp, plEth->m_DestPort,
				plEth->m_pBuf, RTP_PACKET_LEN + plEth->m_BufCount * TS_PACKET_SIZE);
		}
		else {
			ENC_3531AEthOutSendData(plEth->m_SocketHandle, plEth->m_DestIp, plEth->m_DestPort,
				&plEth->m_pBuf[RTP_PACKET_LEN], plEth->m_BufCount * TS_PACKET_SIZE);
		}
		plEth->m_BufCount = 0;	
	}
	PFC_SemaphoreSignal(plEth->m_Lock);
}

static BOOL ENC_3531ASetEthParam(ENC_3531AHandle *pHandle, ENC_3531AParam *pParam, S32 ChnIndex)
{
	BOOL lRet = TRUE;
	ENC_3531AEthOutParam *plEth;
	S32 i;
	
	/* 配置编码参数的时候是 4 个通道依次配置，而且所有通道的网络输出模式是一致的，
		要么都是 SPTS，要么都是 MPTS，所以只通过第一个通道的 SPTS/MPTS 来标识网络输出
		流的情况。
	*/
	if (0 == ChnIndex) {
		if (pHandle->m_IpOutputType != pParam->m_IpOutputType) { /* MPTS/SPTS 切换，需要清空所有 buffer */
			pHandle->m_IpOutputType = pParam->m_IpOutputType;
			for (i = 0; i < pHandle->m_InitParam.m_ChnNum; i++) {
				plEth = &pHandle->m_pEthOutParam[i];
				PFC_SemaphoreWait(plEth->m_Lock, -1);
				memset(plEth->m_pBuf, 0, sizeof(plEth->m_pBuf));
				plEth->m_BufCount = 0;
				PFC_SemaphoreSignal(plEth->m_Lock);
			}
		}
	}

	plEth = &pHandle->m_pEthOutParam[ChnIndex];

	if (IP_OUT_TYPE_MPTS == pHandle->m_IpOutputType) { 
		if (0 == ChnIndex) { /* 复用使用的是第一个通道的网络参数 */
			if ((plEth->m_SendTsNum != pParam->m_SendTsNum) || 
				(plEth->m_IpProto != pParam->m_IpProto) || 
				(plEth->m_DestIp != pParam->m_DestIp) ||
				(plEth->m_DestPort != pParam->m_DestPort)) { /* 当网络相关参数变化，需要重置发包相关信息 */
					PFC_SemaphoreWait(plEth->m_Lock, -1);
					plEth->m_SendTsNum = pParam->m_SendTsNum;
					plEth->m_IpProto = pParam->m_IpProto;
					plEth->m_DestIp = pParam->m_DestIp;
					plEth->m_DestPort = pParam->m_DestPort;
					memset(plEth->m_pBuf, 0, sizeof(plEth->m_pBuf));
					plEth->m_BufCount = 0;
					PFC_SemaphoreSignal(plEth->m_Lock);
			}
		}
	}
	else { /* SPTS */
		if ((plEth->m_SendTsNum != pParam->m_SendTsNum) || 
			(plEth->m_IpProto != pParam->m_IpProto) || 
			(plEth->m_DestIp != pParam->m_DestIp) ||
			(plEth->m_DestPort != pParam->m_DestPort)) { /* 当网络相关参数变化，需要重置发包相关信息 */
				PFC_SemaphoreWait(plEth->m_Lock, -1);
				plEth->m_SendTsNum = pParam->m_SendTsNum;
				plEth->m_IpProto = pParam->m_IpProto;
				plEth->m_DestIp = pParam->m_DestIp;
				plEth->m_DestPort = pParam->m_DestPort;
				memset(plEth->m_pBuf, 0, sizeof(plEth->m_pBuf));
				plEth->m_BufCount = 0;
				PFC_SemaphoreSignal(plEth->m_Lock);
		}

		if (!pParam->m_ActiveMark) { /* 通道关闭时，清空发送 buffer */
			PFC_SemaphoreWait(plEth->m_Lock, -1);
			memset(plEth->m_pBuf, 0, sizeof(plEth->m_pBuf));
			plEth->m_BufCount = 0;
			PFC_SemaphoreSignal(plEth->m_Lock);
		}
	}

	return lRet;
}

static void ENC_3531ASyncPts(ENC_3531AVidProcParam *pVid, S32 ChnIndex)
{
	S32 ret = -1;
	U64 pts_time;

	PFC_SemaphoreWait(s_pHandle->m_SysLock, -1);
	ret = mpegs_sync_pts(&pts_time);
	PFC_SemaphoreSignal(s_pHandle->m_SysLock);
	if (ret != 0) {
		GLOBAL_TRACE((ANSI_COLOR_GREEN"CHN[%d] ret = %d\n"ANSI_COLOR_NONE"\n", ChnIndex, ret));
	}
	else {
		pVid->m_pEs2TsObj->first_pts = pts_time;
		pVid->m_pEs2TsObj->ts_packet_count = 0;
		pVid->m_pEs2TsObj->first_pcr = 0;
		pVid->m_pEs2TsObj->previous_system_time = 0;
		GLOBAL_TRACE(("glopts->first_pts[%d] = %lld\n", ChnIndex, pts_time));
		pVid->m_pEs2TsObj->syn_flag = 1;
	} 
}

static BOOL ENC_3531ASetPsiParam(ENC_3531APsiProcParam *pPsi, S32 ChnIndex, ENC_3531AParam *pParam)
{
	PFC_SemaphoreWait(pPsi->m_Lock, -1);
	pPsi->m_VidPid = pParam->m_VidPid;
	pPsi->m_AudPid = pParam->m_AudPid;
	pPsi->m_PcrPid = pParam->m_PcrPid;
	pPsi->m_PmtPid = pParam->m_PmtPid;
	pPsi->m_ProgNumber = pParam->m_ProgNumber;
	GLOBAL_STRCPY(pPsi->m_pProgName, pParam->m_pProgName);
	pPsi->m_PsiCharSet = pParam->m_PsiCharSet;
	pPsi->m_AudEncMode = pParam->m_AudEncMode;
	pPsi->m_ActiveMark = pParam->m_ActiveMark;
	if (!pParam->m_ActiveMark) {
		PFC_SemaphoreWait(s_pHandle->m_pVidProcParam[ChnIndex].m_TsProcLock, -1);
		RngFlush(s_pHandle->m_pTsProcParam[ChnIndex].m_pInsertTsRngBuf); /* 通道停止时清除对应的 Buffer */
		PFC_SemaphoreSignal(s_pHandle->m_pVidProcParam[ChnIndex].m_TsProcLock);
	}
	PFC_SemaphoreSignal(pPsi->m_Lock);

	return TRUE;
}

static BOOL ENC_3531ASetVidParam(ENC_3531AVidProcParam *pVid, S32 ChnIndex, ENC_3531AParam *pParam)
{
	HI_S32 lHiRet = HI_SUCCESS;
	ENC_3531AViInfo lViInfo;
	VPSS_GRP_ATTR_S lGrpAttr;
	SIZE_S lViSize;
	ENC_3531AVoInfo lVoInfo;
	SIZE_S lVoSize;
	VENC_PARAM_S lVencParam;
	HI_S32 lVpssChnCnt = 1;
	VENC_CHN lVencChn;
	VPSS_GRP lVpssGrp;
	VI_DEV lViDev;
	VI_CHN lViChn;
	HI_S32 lViDevInterval = 2;
	HI_S32 lViChnInterval = 8;
	HI_U32 lTmpSetSrcFrameRate, lTmpSetDstFrameRate;

	PFC_SemaphoreWait(pVid->m_ParamSetLock, -1);
	ENC_3531AGetViInfo(pParam->m_ViMode, &lViInfo);
	lVencChn = ChnIndex;
	lVpssGrp = ChnIndex;
	lViChn = ChnIndex * lViChnInterval;
	lViDev = ChnIndex * lViDevInterval;

	PFC_SemaphoreWait(pVid->m_TsProcLock, -1);
	/* 设置配置的参数 */
	pVid->m_ActiveMark = pParam->m_ActiveMark;
	pVid->m_PtsDelayTime = pParam->m_PtsDelayTime;
	pVid->m_SignalIsLocked = pParam->m_SignalIsLocked;

	/* ES2TS 参数设置 */
	mpegts_set_pid(pVid->m_pEs2TsObj, pParam->m_VidPid);
	mpegts_set_stream_id(pVid->m_pEs2TsObj, PES_VIDEO_STREAM_ID); /* Set stream id */
	//mpegts_set_pts_dts_flag(pVid->m_pEs2TsObj, PES_INCLUDE_PTS_DTS); /* Set pts/des flag */
	mpegts_set_pts_dts_flag(pVid->m_pEs2TsObj, PES_INCLUDE_ONLY_PTS); /* Set pts/des flag */
	pVid->m_pEs2TsObj->pcr_ts_opt.pcr_pid = pParam->m_PcrPid;
	GLOBAL_TRACE(("==CHN[%d] audPtsDelayTime: %d ptsDelaytime:%d maxptspcrInterval: %d minptspcrInterval: %d auddelayframenum:%d\n", ChnIndex, 
		pParam->m_AudPtsRelativeDelayTime, pParam->m_PtsDelayTime, pParam->m_MaxPtsPcrInterval, pParam->m_MinPtsPcrInterval, pParam->m_AudDelayFrameNum));
	mpegts_set_sync_parm(pVid->m_pEs2TsObj, pParam->m_PtsDelayTime, pParam->m_MaxPtsPcrInterval, pParam->m_MinPtsPcrInterval);
	mpegts_set_ts_rate(pVid->m_pEs2TsObj, pParam->m_ProgBitrate * 1000, 30);
	pVid->m_pEs2TsObj->write_pcr = 0;
	pVid->m_pEs2TsObj->discontinuity = 0;
	ENC_3531ASyncPts(pVid, ChnIndex); /* 同步 PTS */
	if (!pParam->m_ActiveMark) {
		RngFlush(s_pHandle->m_pTsProcParam[ChnIndex].m_pVidTsRngBuf); /* 通道停止时清除对应的 Buffer */
	}
	PFC_SemaphoreSignal(pVid->m_TsProcLock);

	/* 停止 VENC */
	lHiRet = VENC_UnBindVpss(lVencChn, lVpssGrp, 0);
	if (HI_SUCCESS != lHiRet) {
		GLOBAL_TRACE(("UnBind Venc and Vpss failed!\n"));
	}

	lHiRet = VENC_Stop(lVencChn);
	if (HI_SUCCESS != lHiRet) {
		GLOBAL_TRACE(("Stop Venc failed!\n"));
	}

	/* 停止 VI */
	lHiRet = VI_UnBindVpss(lViChn, lVpssGrp);
	if (HI_SUCCESS != lHiRet) {
		GLOBAL_TRACE(("UnBind Vi and Vpss failed!\n"));
	}
	/* 停止 VPSS */
	lHiRet = VPSS_UnDisplayColorBar(lVpssGrp, 0);
	if (HI_SUCCESS != lHiRet) {
		GLOBAL_TRACE(("UnDisplayColorBar failed!\n"));
	}

	lHiRet = VPSS_Stop(lVpssGrp, lVpssChnCnt);
	if (HI_SUCCESS != lHiRet) {
		GLOBAL_TRACE(("Stop Vpss failed!\n"));
	}
	lHiRet = VI_StopChn(lViChn);
	if (HI_SUCCESS != lHiRet) {
		GLOBAL_TRACE(("Stop ViChn failed!\n"));
	}

	lHiRet = VI_StopDev(lViDev);
	if (HI_SUCCESS != lHiRet) {
		GLOBAL_TRACE(("Stop ViDev failed!\n"));
	}

	if (pParam->m_ActiveMark) {		
		/* 启动 VI */
		lHiRet = VI_StartDev(lViDev, lViInfo.m_Vimode);
		if (HI_SUCCESS != lHiRet) {
			GLOBAL_TRACE(("Start ViDev failed!\n"));
			goto ret;
		}

		lHiRet = VI_StartChn(lViChn, lViInfo.m_Vimode, lViInfo.m_ViNorm, VI_CHN_SET_NORMAL);
		if (HI_SUCCESS != lHiRet) {
			GLOBAL_TRACE(("Start ViChn failed!\n"));
			goto ret;
		}

		/* 启动 VPSS */
		lHiRet = SAMPLE_COMM_SYS_GetPicSize(lViInfo.m_ViNorm, lViInfo.m_PicSize, &lViSize);
		if (HI_SUCCESS != lHiRet) {
			GLOBAL_TRACE(("SAMPLE_COMM_SYS_GetPicSize failed!\n"));
			goto ret;
		}

		lGrpAttr.u32MaxW = lViSize.u32Width;
		lGrpAttr.u32MaxH = lViSize.u32Height;
		lGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
		lGrpAttr.bIeEn = HI_FALSE;
		lGrpAttr.bNrEn = HI_TRUE;
		lGrpAttr.bDciEn = HI_FALSE;
		lGrpAttr.bHistEn = HI_FALSE;
		lGrpAttr.bEsEn = HI_FALSE;
		lGrpAttr.enDieMode = lViInfo.m_DieMode;
		lHiRet = VPSS_Start(lVpssGrp, &lViSize, lVpssChnCnt, &lGrpAttr);
		if (HI_SUCCESS != lHiRet) {
			GLOBAL_TRACE(("Start Vpss failed!\n"));
			goto ret;
		}

		if (!pParam->m_SignalIsLocked) { /* 信号未锁定的情况下，认为信号丢失，即显示标准彩条 */
			lHiRet = VPSS_DisplayColorBar(lVpssGrp, 0, lViSize.u32Width, lViSize.u32Height);
			if (HI_SUCCESS != lHiRet) {
				SAMPLE_PRT("VPSS_DisplayColorBar failed!\n");
				goto ret;
			}
		}

		lHiRet = VI_BindVpss(lViChn, lVpssGrp);
		if (HI_SUCCESS != lHiRet) {
			GLOBAL_TRACE(("Vi bind Vpss failed!\n"));
			goto ret;
		}

		/* 设置单包模式 */
		{
			VENC_PARAM_MOD_S pstModParam;

			pstModParam.enVencModType = MODTYPE_H264E;
			HI_MPI_VENC_GetModParam(&pstModParam);
			pstModParam.stH264eModParam.u32OneStreamBuffer=1;
			HI_MPI_VENC_SetModParam(&pstModParam);
		}

		/* 启动 VENC */
		ENC_3531AGetVoInfo(pParam->m_VoMode, &lVoInfo, &lViInfo);
		lHiRet = SAMPLE_COMM_SYS_GetPicSize(lVoInfo.m_ViNorm, lVoInfo.m_PicSize, &lVoSize);
		if (HI_SUCCESS != lHiRet) {
			GLOBAL_TRACE(("SAMPLE_COMM_SYS_GetPicSize failed!\n"));
			goto ret;
		}

		memset(&lVencParam, 0, sizeof(lVencParam));
		lVencParam.m_VencChn = lVencChn;

		lVencParam.m_VencChnAttr.stVeAttr.enType = PT_H264;
		lVencParam.m_VencChnAttr.stVeAttr.stAttrH264e.bByFrame = HI_TRUE;
		lVencParam.m_VencChnAttr.stVeAttr.stAttrH264e.u32BufSize = lVoSize.u32Height * lVoSize.u32Width * 2;
		lVencParam.m_VencChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = lVoSize.u32Height;
		lVencParam.m_VencChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = lVoSize.u32Width;
		lVencParam.m_VencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = lVoSize.u32Height;
		lVencParam.m_VencChnAttr.stVeAttr.stAttrH264e.u32PicWidth = lVoSize.u32Width;
		lVencParam.m_VencChnAttr.stVeAttr.stAttrH264e.u32Profile = ENC_3531AGetProfile(pParam->m_Profile);

		/* 转换设置参数，由于 Hi3531A 不支持输入设置小数，所以对于 29.97/59.94 相应设置为 30/60 */
		if (2997 == lViInfo.m_FrmRate) {
			lTmpSetSrcFrameRate = 30;
		}
		else if (5994 == lViInfo.m_FrmRate) {
			lTmpSetSrcFrameRate = 60;
		}
		else {
			lTmpSetSrcFrameRate = lViInfo.m_FrmRate;
		}
		if (2997 == lVoInfo.m_FrmRate) {
			lTmpSetDstFrameRate = 30;
		}
		else if (5994 == lVoInfo.m_FrmRate) {
			lTmpSetDstFrameRate = 60;
		}
		else {
			lTmpSetDstFrameRate = lVoInfo.m_FrmRate;
		}

		switch (pParam->m_RcMode)
		{
		case RC_MODE_CBR:
			lVencParam.m_VencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 1;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate = lTmpSetSrcFrameRate;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate = lTmpSetDstFrameRate;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = pParam->m_RcParam.m_CbrParam.m_Gop;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = pParam->m_RcParam.m_CbrParam.m_BitRate;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0; // 不使用
			break;
		case RC_MODE_VBR:
			lVencParam.m_VencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Vbr.u32StatTime = 1;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate = lTmpSetSrcFrameRate;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate = lTmpSetDstFrameRate;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Vbr.u32Gop = pParam->m_RcParam.m_VbrParam.m_Gop;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp = pParam->m_RcParam.m_VbrParam.m_MinQp;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp = pParam->m_RcParam.m_VbrParam.m_MaxQp;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = pParam->m_RcParam.m_VbrParam.m_MaxBitRate;
			break;
		case RC_MODE_FIXQP:
			lVencParam.m_VencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRate = lTmpSetSrcFrameRate;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264FixQp.fr32DstFrmRate = lTmpSetDstFrameRate;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = pParam->m_RcParam.m_FixqpParam.m_Gop;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = pParam->m_RcParam.m_FixqpParam.m_IQp;
			lVencParam.m_VencChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = pParam->m_RcParam.m_FixqpParam.m_PQp;
			break;
		default: 
			break;
		}

		lHiRet = VENC_Start(&lVencParam, pParam->m_SignalIsLocked, lVoInfo.m_FrmRate);
		if (HI_SUCCESS != lHiRet) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto ret;
		}

		lHiRet = VENC_BindVpss(lVencParam.m_VencChn, lVpssGrp, 0);
		if (HI_SUCCESS != lHiRet) {
			SAMPLE_PRT("Venc Bind Vpss failed!\n");
			goto ret;
		}
	}

ret:
	PFC_SemaphoreSignal(pVid->m_ParamSetLock);

	return (lHiRet == HI_SUCCESS);
}

static BOOL ENC_3531ASetAudParam(ENC_3531AAudProcParam *pAud, S32 ChnIndex, ENC_3531AParam *pParam)
{
	HI_S32 lHiRet = HI_SUCCESS;
	AUDIO_DEV lAiDev = 0;
	HI_S32 lAiChn = 0;
	
	if ((pParam->m_AudEncMode == AUD_ENC_MODE_AC3) || (pParam->m_AudEncMode == AUD_ENC_MODE_EAC3))
	{
		lAiChn = ChnIndex + 4;/*AC3为4-7通道*/
	}
	else
	{
		lAiChn = ChnIndex;

	}

	PFC_SemaphoreWait(pAud->m_ParamSetLock, -1);
	/* 设置配置的参数 */
	pAud->m_AudEncMode = pParam->m_AudEncMode;
	pAud->m_AudBitrate = pParam->m_AudBitrate;
	pAud->m_AudSample = pParam->m_AudSample;
	pAud->m_AudVolume = pParam->m_AudVolume;
	pAud->m_AudPtsRelativeDelayTime = pParam->m_AudPtsRelativeDelayTime; 
	pAud->m_PtsDelayTime = pParam->m_PtsDelayTime; 
	pAud->m_AudDelayFrameNum = pParam->m_AudDelayFrameNum;
	pAud->m_SignalIsLocked = pParam->m_SignalIsLocked;

	/* 设置 ES2TS 参数 */
	mpegts_set_pid(pAud->m_pEs2TsObj, pParam->m_AudPid);
	if ((pParam->m_AudEncMode == AUD_ENC_MODE_AC3) || (pParam->m_AudEncMode == AUD_ENC_MODE_EAC3))
	{
		mpegts_set_stream_id(pAud->m_pEs2TsObj, PES_AC3_STREAM_ID);
	} 
	else
	{
		mpegts_set_stream_id(pAud->m_pEs2TsObj, PES_AUDIO_STREAM_ID);
	}
	
	
	mpegts_set_pts_dts_flag(pAud->m_pEs2TsObj, PES_INCLUDE_ONLY_PTS);
	if (!pParam->m_ActiveMark) {
		PFC_SemaphoreWait(s_pHandle->m_pVidProcParam[ChnIndex].m_TsProcLock, -1);
		RngFlush(s_pHandle->m_pTsProcParam[ChnIndex].m_pAudTsRngBuf); /* 通道停止时清除对应的 Buffer */
		PFC_SemaphoreSignal(s_pHandle->m_pVidProcParam[ChnIndex].m_TsProcLock);
	}

	/* 停止通道 */
	AI_StopChn(lAiDev, lAiChn);
	pAud->m_AudProcCmd = AUD_PROC_STOP;

	/* 启动通道 */
	if (pParam->m_ActiveMark) {
		lHiRet = AI_StartChn(lAiDev, lAiChn);
		if (lHiRet != HI_SUCCESS) {
			GLOBAL_TRACE(("AI_StartChn Failed!\n"));
		}
		pAud->m_AudProcCmd = AUD_PROC_REINIT; /* 触发音频编码重新配置 */
		GLOBAL_TRACE(("AI Start Channel OK!\n"));
	}
	PFC_SemaphoreSignal(pAud->m_ParamSetLock);

	return (lHiRet == HI_SUCCESS);
}

static void ENC_3531ASendTsToFifo(U8 *pBuf, U32 BufLen, void *pParam)
{
	HI_S32 s32Ret = HI_SUCCESS;
	ENC_3531ASendTsToFifoParam *plSendTsToFifoParam = (ENC_3531ASendTsToFifoParam *)pParam;
	HI_S32 lIndex = plSendTsToFifoParam->m_ChnIndex;
	RNGBUF_t *plRngBuf = plSendTsToFifoParam->m_pRngBuf;
	ENC_3531AHandle *plHandle = s_pHandle;
	ENC_3531AMonitorParam *plMonitor = &plHandle->m_pMonitorParam[lIndex];
	U8 *plStr[TS_TYPE_NUM] = {"Video", "Audio", "Inserter"};

	if (!GLOBAL_CHECK_INDEX(lIndex, plHandle->m_InitParam.m_ChnNum)) {
		GLOBAL_TRACE((ANSI_COLOR_RED"Index ERROR!!\n"ANSI_COLOR_NONE));
		return;
	}

	if (BufLen != MPEG2_TS_PACKET_SIZE) {
		GLOBAL_TRACE(("expect %d, but BufLen = %d\n", MPEG2_TS_PACKET_SIZE, BufLen));
		return;
	}

	s32Ret = RngIsFull(plRngBuf);
	if (s32Ret) {
		GLOBAL_TRACE((ANSI_COLOR_RED"%s RngBuf Is Full:%d!"ANSI_COLOR_NONE"\n", plStr[plSendTsToFifoParam->m_TsType], lIndex));
		RngFlush(plRngBuf); /* Buffer 满后直接清掉 */
		PFC_SemaphoreWait(plMonitor->m_Lock, -1);
		plMonitor->m_AlarmInfo.m_BufferOverFlowCount ++;
		PFC_SemaphoreSignal(plMonitor->m_Lock);
	}
	else if(RngFreeBytes(plRngBuf) < MPEG2_TS_PACKET_SIZE) {
		GLOBAL_TRACE(("%s RngFreeBytes[%d] < %d!\n", plStr[plSendTsToFifoParam->m_TsType], lIndex, MPEG2_TS_PACKET_SIZE));
	}
	else {
		RngBufPut(plRngBuf, (char*)pBuf, MPEG2_TS_PACKET_SIZE);
	}
}

static void ENC_3531AMonitorTimer(void *pUserParam)
{
	ENC_3531AHandle *plHandle = (ENC_3531AHandle *)pUserParam;
	S32 lIndex;
	S64 lPtsTime;

	for (lIndex = 0; lIndex < plHandle->m_InitParam.m_ChnNum; lIndex++) {
		PFC_SemaphoreWait(plHandle->m_pMonitorParam[lIndex].m_Lock, -1);
		GLOBAL_MEMCPY(&plHandle->m_pMonitorParam[lIndex].m_RecordTsInfo, &plHandle->m_pMonitorParam[lIndex].m_RtTsInfo, sizeof(ENC_3531AStatus));
		GLOBAL_ZEROSTRUCT(plHandle->m_pMonitorParam[lIndex].m_RtTsInfo);
		PFC_SemaphoreSignal(plHandle->m_pMonitorParam[lIndex].m_Lock);
	}

	PFC_SemaphoreWait(plHandle->m_SysLock, -1);
	mpegs_sync_pts(&lPtsTime); /* 定时系统时间同步到海思模块 */
	PFC_SemaphoreSignal(plHandle->m_SysLock);
}

static void ENC_3531ATerminate(ENC_3531AHandle *pHandle)
{
	S32 i;

	/* 销毁线程 */
	pHandle->m_TaskRunFlg = FALSE;
	PL_TaskSleep(3000); /* 延时等待线程结束 */

	/* 销毁定时器 */
	if (pHandle->m_MonitorTimer) {
		PFC_TimerDestroy(pHandle->m_MonitorTimer);
		pHandle->m_MonitorTimer = NULL;
	}
	
	/* 销毁参数 */
	for (i = 0; i < pHandle->m_InitParam.m_ChnNum; i++) {
		if (pHandle->m_pTsProcParam[i].m_pVidTsRngBuf) {
			RngDestroy(pHandle->m_pTsProcParam[i].m_pVidTsRngBuf);
			pHandle->m_pTsProcParam[i].m_pVidTsRngBuf = NULL;
		}

		if (pHandle->m_pTsProcParam[i].m_pAudTsRngBuf) {
			RngDestroy(pHandle->m_pTsProcParam[i].m_pAudTsRngBuf);
			pHandle->m_pTsProcParam[i].m_pAudTsRngBuf = NULL;
		}

		if (pHandle->m_pTsProcParam[i].m_pInsertTsRngBuf) {
			RngDestroy(pHandle->m_pTsProcParam[i].m_pInsertTsRngBuf);
			pHandle->m_pTsProcParam[i].m_pInsertTsRngBuf = NULL;
		}

		if (pHandle->m_pPsiProcParam[i].m_pPsiObj) {
			mpegts_psi_destory(pHandle->m_pPsiProcParam[i].m_pPsiObj);
			pHandle->m_pPsiProcParam[i].m_pPsiObj = NULL;
		}

		if (pHandle->m_pEthOutParam[i].m_SocketHandle) {
			PFC_SocketClose(pHandle->m_pEthOutParam[i].m_SocketHandle);
			pHandle->m_pEthOutParam[i].m_SocketHandle = NULL;
		}

		if (pHandle->m_pAudProcParam[i].m_ParamSetLock) {
			PFC_SemaphoreDestroy(pHandle->m_pAudProcParam[i].m_ParamSetLock);
			pHandle->m_pAudProcParam[i].m_ParamSetLock = NULL;
		}
		if (pHandle->m_pAudProcParam[i].m_pEs2TsObj) {
			mpegts_destory(pHandle->m_pAudProcParam[i].m_pEs2TsObj);
			pHandle->m_pAudProcParam[i].m_pEs2TsObj = NULL;
		}

		if (pHandle->m_pVidProcParam[i].m_ParamSetLock) {
			PFC_SemaphoreDestroy(pHandle->m_pVidProcParam[i].m_ParamSetLock);
			pHandle->m_pVidProcParam[i].m_ParamSetLock = NULL;
		}
		if (pHandle->m_pVidProcParam[i].m_TsProcLock) {
			PFC_SemaphoreDestroy(pHandle->m_pVidProcParam[i].m_TsProcLock);
			pHandle->m_pVidProcParam[i].m_TsProcLock = NULL;
		}
		if (pHandle->m_pVidProcParam[i].m_pEs2TsObj) {
			mpegts_destory(pHandle->m_pVidProcParam[i].m_pEs2TsObj);
			pHandle->m_pVidProcParam[i].m_pEs2TsObj = NULL;
		}
	}

	if (pHandle->m_SysLock) {
		PFC_SemaphoreDestroy(pHandle->m_SysLock);
		pHandle->m_SysLock = NULL;
	}
	if (pHandle->m_pAudProcParam) {
		GLOBAL_SAFEFREE(pHandle->m_pAudProcParam);
	}
	if (pHandle->m_pVidProcParam) {
		GLOBAL_SAFEFREE(pHandle->m_pVidProcParam);
	}
	if (pHandle->m_pTsProcParam) {
		GLOBAL_SAFEFREE(pHandle->m_pTsProcParam);
	}
	if (pHandle->m_pPsiProcParam) {
		GLOBAL_SAFEFREE(pHandle->m_pPsiProcParam);
	}
	if (pHandle->m_pMonitorParam) {
		GLOBAL_SAFEFREE(pHandle->m_pMonitorParam);
	}
	if (pHandle->m_pEthOutParam) {
		GLOBAL_SAFEFREE(pHandle->m_pEthOutParam);
	}

	AI_StopDev(SAMPLE_AUDIO_AI_DEV);
	SAMPLE_COMM_SYS_Exit();
}

static BOOL ENC_3531AInit(ENC_3531AHandle *pHandle)
{
	ENC_3531AInitParam *plInitParam = &pHandle->m_InitParam;
	S32 lChnNum = plInitParam->m_ChnNum;	
	VB_CONF_S stVbConf;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	S32 i;
	S64 lSysTime = 0;

	pHandle->m_IpOutputType = IP_OUT_TYPE_SPTS;

	/* 用系统时间戳初始化 MPI 时间戳 */
	OS_TimeNow(&lSysTime);
	s32Ret = HI_MPI_SYS_InitPtsBase(lSysTime);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_SYS_InitPtsBase failed with %d!\n", s32Ret);
		goto ret_end;
	}

	/******************************************
     step  1: init variable 
    ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_AUTO,\
		PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH, COMPRESS_MODE_SEG); /* 按照 1080 的图像分配块大小，针对 1080/720 都能满足 */
	stVbConf.u32MaxPoolCnt = 128; /* 整个系统可容纳的缓存池数目 */

	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize; /* 公共缓存池块大小 */
	stVbConf.astCommPool[0].u32BlkCnt = lChnNum * 12; /* 公共缓存池块数目 */
	memset(stVbConf.astCommPool[0].acMmzName, 0, sizeof(stVbConf.astCommPool[0].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto ret_end;
    }

	/* Audio Device Init. */
	{
		AIO_ATTR_S  lAioAttr; 
		HI_S32 lAiChnCnt = 8; /* 每个 AI 设备有 8 个通道 */

		/* 停止通道 */
		for (i = 0; i < lAiChnCnt; i++) {
			AI_StopChn(SAMPLE_AUDIO_AI_DEV, i);
		}

		/* 停止设备 */
		AI_StopDev(SAMPLE_AUDIO_AI_DEV);

		/* 启动设备 */
		lAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_48000; /* 固定 48K 采样 */
		lAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
		lAioAttr.enWorkmode     = AIO_MODE_I2S_SLAVE;
		lAioAttr.enSoundmode    = AUDIO_SOUND_MODE_STEREO;
		lAioAttr.u32EXFlag      = 1;
		lAioAttr.u32FrmNum      = 30; /* 缓存帧数目 */
		lAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM; /* 采样点个数 */
		lAioAttr.u32ChnCnt      = lAiChnCnt * 2; /* 立体声有2个声道 */
		lAioAttr.u32ClkChnCnt   = lAiChnCnt * 2;
		lAioAttr.u32ClkSel      = 0;
		s32Ret = AI_StartDev(SAMPLE_AUDIO_AI_DEV, &lAioAttr);
		if (s32Ret != HI_SUCCESS) {
			GLOBAL_TRACE(("AI_StartDev Failed!\n"));
			goto ret_end;
		}
	}

	pHandle->m_pAudProcParam = (ENC_3531AAudProcParam *)GLOBAL_ZMALLOC(pHandle->m_InitParam.m_ChnNum * sizeof(ENC_3531AAudProcParam));
	if (!pHandle->m_pAudProcParam) {
		GLOBAL_TRACE(("malloc error!\n"));
		goto ret_end;
	}

	pHandle->m_pVidProcParam = (ENC_3531AVidProcParam *)GLOBAL_ZMALLOC(pHandle->m_InitParam.m_ChnNum * sizeof(ENC_3531AVidProcParam));
	if (!pHandle->m_pVidProcParam) {
		GLOBAL_TRACE(("malloc error!\n"));
		goto ret_end;
	}

	pHandle->m_pTsProcParam = (ENC_3531ATsProcParam *)GLOBAL_ZMALLOC(pHandle->m_InitParam.m_ChnNum * sizeof(ENC_3531ATsProcParam));
	if (!pHandle->m_pTsProcParam) {
		GLOBAL_TRACE(("malloc error!\n"));
		goto ret_end;
	}

	pHandle->m_pPsiProcParam = (ENC_3531APsiProcParam *)GLOBAL_ZMALLOC(pHandle->m_InitParam.m_ChnNum * sizeof(ENC_3531APsiProcParam));
	if (!pHandle->m_pPsiProcParam) {
		GLOBAL_TRACE(("malloc error!\n"));
		goto ret_end;
	}

	pHandle->m_pMonitorParam = (ENC_3531AMonitorParam *)GLOBAL_ZMALLOC(pHandle->m_InitParam.m_ChnNum * sizeof(ENC_3531AMonitorParam));
	if (!pHandle->m_pMonitorParam) {
		GLOBAL_TRACE(("malloc error!\n"));
		goto ret_end;
	}

	pHandle->m_pEthOutParam = (ENC_3531AEthOutParam*)GLOBAL_ZMALLOC(pHandle->m_InitParam.m_ChnNum * sizeof(ENC_3531AEthOutParam));
	if (!pHandle->m_pEthOutParam) {
		GLOBAL_TRACE(("malloc error!\n"));
		goto ret_end;
	}

	pHandle->m_SysLock = PFC_SemaphoreCreate("Sys Set Lock", 1);
	if (!pHandle->m_SysLock) {
		GLOBAL_TRACE(("PFC_SemaphoreCreate Failed!\n"));
		goto ret_end;
	}
	PFC_SemaphoreSignal(pHandle->m_SysLock);

	pHandle->m_TaskRunFlg = TRUE;

	for (i = 0; i < pHandle->m_InitParam.m_ChnNum; i++) {
		S32 lSocketVal = 1;

		/* TS */
		pHandle->m_pTsProcParam[i].m_pVidTsRngBuf = RngCreate(VIDEO_SIZFEOF);
		if (!pHandle->m_pTsProcParam[i].m_pVidTsRngBuf) {
			GLOBAL_TRACE(("RngCreate Failed!\n"));
			goto ret_end;
		}
		pHandle->m_pTsProcParam[i].m_pAudTsRngBuf = RngCreate(AUD_TS_BUFF_SIZE);
		if (!pHandle->m_pTsProcParam[i].m_pAudTsRngBuf) {
			GLOBAL_TRACE(("RngCreate Failed!\n"));
			goto ret_end;
		}
		pHandle->m_pTsProcParam[i].m_pInsertTsRngBuf = RngCreate(INSERT_TS_BUFF_SIZE);
		if (!pHandle->m_pTsProcParam[i].m_pInsertTsRngBuf) {
			GLOBAL_TRACE(("RngCreate Failed!\n"));
			goto ret_end;
		}

		/* PSI */
		pHandle->m_pPsiProcParam[i].m_pPsiObj = mpegts_psi_init();
		if (!pHandle->m_pPsiProcParam[i].m_pPsiObj) {
			GLOBAL_TRACE(("mpegts_psi_init Failed!\n"));
			goto ret_end;
		}
		pHandle->m_pPsiProcParam[i].m_SendToFifoParam.m_ChnIndex = i;
		pHandle->m_pPsiProcParam[i].m_SendToFifoParam.m_TsType = TS_TYPE_INSERT;
		pHandle->m_pPsiProcParam[i].m_SendToFifoParam.m_pRngBuf = pHandle->m_pTsProcParam[i].m_pInsertTsRngBuf;
		pHandle->m_pPsiProcParam[i].m_pPsiObj->m_pUserParam = &pHandle->m_pPsiProcParam[i].m_SendToFifoParam;
		pHandle->m_pPsiProcParam[i].m_pPsiObj->tx_psi_call = ENC_3531ASendTsToFifo;
		pHandle->m_pPsiProcParam[i].m_pPsiObj->pat_continuity_counter = 0;
		pHandle->m_pPsiProcParam[i].m_pPsiObj->pmt_continuity_counter[0] = 0;
		pHandle->m_pPsiProcParam[i].m_pPsiObj->sdt_continuity_counter = 0;
		pHandle->m_pPsiProcParam[i].m_pPsiObj->cat_continuity_counter = 0;

		/* 网络发包 */
		pHandle->m_pEthOutParam[i].m_BufCount = 0;
		pHandle->m_pEthOutParam[i].m_SocketHandle = PFC_SocketCreate(PFC_SOCKET_TYPE_UDP);
		if (!pHandle->m_pEthOutParam[i].m_SocketHandle) {
			GLOBAL_TRACE(("PFC_SocketCreate Failed!\n"));
			goto ret_end;
		}
		/* 设置允许该 Socket 进行组播 */
		PFC_SocketOption(pHandle->m_pEthOutParam[i].m_SocketHandle, PFC_SOCKET_OPTION_TYPE_BROADCAST, &lSocketVal, sizeof(S32));
		PFC_SocketOption(pHandle->m_pEthOutParam[i].m_SocketHandle, PFC_SOCKET_OPTION_TYPE_ADDRREUSE, &lSocketVal, sizeof(S32));

		/* 监控 */
		pHandle->m_pMonitorParam[i].m_Lock = PFC_SemaphoreCreate("Monitor Set Lock", 1);
		if (!pHandle->m_pMonitorParam[i].m_Lock) {
			GLOBAL_TRACE(("PFC_SemaphoreCreate Failed!\n"));
			goto ret_end;
		}
		PFC_SemaphoreSignal(pHandle->m_pMonitorParam[i].m_Lock);
		ENC_3531ACcValidClean(pHandle, i);

		/* 音频 */
		pHandle->m_pAudProcParam[i].m_pEs2TsObj = mpegts_init();
		if (!pHandle->m_pAudProcParam[i].m_pEs2TsObj) {
			GLOBAL_TRACE(("mpegts_init Failed!\n"));
			goto ret_end;
		}
		pHandle->m_pAudProcParam[i].m_SendToFifoParam.m_ChnIndex = i;
		pHandle->m_pAudProcParam[i].m_SendToFifoParam.m_TsType = TS_TYPE_AUD;
		pHandle->m_pAudProcParam[i].m_SendToFifoParam.m_pRngBuf = pHandle->m_pTsProcParam[i].m_pAudTsRngBuf;
		pHandle->m_pAudProcParam[i].m_pEs2TsObj->m_pUserParam = &pHandle->m_pAudProcParam[i].m_SendToFifoParam;
		pHandle->m_pAudProcParam[i].m_pEs2TsObj->tx_ts_to_buffer_call = ENC_3531ASendTsToFifo;
		pHandle->m_pAudProcParam[i].m_pEs2TsObj->send_ts_call = NULL; 
		pHandle->m_pAudProcParam[i].m_ParamSetLock = PFC_SemaphoreCreate("Aud Param Set Lock", 1);
		if (!pHandle->m_pAudProcParam[i].m_ParamSetLock) {
			GLOBAL_TRACE(("PFC_SemaphoreCreate Failed!\n"));
			goto ret_end;
		}
		PFC_SemaphoreSignal(pHandle->m_pAudProcParam[i].m_ParamSetLock);
		pHandle->m_pAudProcParam[i].m_AudProcCmd = AUD_PROC_STOP;
		pHandle->m_pAudProcParam[i].m_AudProThreadHandle = PFC_TaskCreate("Hi3531 Audio Process", 1024 * 1024, ENC_3531AAudioProcThread, 1, (void *)i);
		if (!pHandle->m_pAudProcParam[i].m_AudProThreadHandle) {
			GLOBAL_TRACE(("PFC_TaskCreate Failed!\n"));
			goto ret_end;
		}

		/* 视频 */
		pHandle->m_pVidProcParam[i].m_pEs2TsObj = mpegts_init();
		if (!pHandle->m_pVidProcParam[i].m_pEs2TsObj) {
			GLOBAL_TRACE(("mpegts_init Failed!\n"));
			goto ret_end;
		}
		pHandle->m_pVidProcParam[i].m_SendToFifoParam.m_ChnIndex = i;
		pHandle->m_pVidProcParam[i].m_SendToFifoParam.m_TsType = TS_TYPE_VID;
		pHandle->m_pVidProcParam[i].m_SendToFifoParam.m_pRngBuf = pHandle->m_pTsProcParam[i].m_pVidTsRngBuf;
		pHandle->m_pVidProcParam[i].m_pEs2TsObj->m_pUserParam = &pHandle->m_pVidProcParam[i].m_SendToFifoParam;
		pHandle->m_pVidProcParam[i].m_pEs2TsObj->tx_ts_to_buffer_call = ENC_3531ASendTsToFifo;
		pHandle->m_pVidProcParam[i].m_pEs2TsObj->send_ts_call = ENC_3531AUdpSendTs; 
		pHandle->m_pVidProcParam[i].m_ParamSetLock = PFC_SemaphoreCreate("Vid Param Set Lock", 1);
		if (!pHandle->m_pVidProcParam[i].m_ParamSetLock) {
			GLOBAL_TRACE(("PFC_SemaphoreCreate Failed!\n"));
			goto ret_end;
		}
		PFC_SemaphoreSignal(pHandle->m_pVidProcParam[i].m_ParamSetLock);
		pHandle->m_pVidProcParam[i].m_TsProcLock = PFC_SemaphoreCreate("TS Set Lock", 1);
		if (!pHandle->m_pVidProcParam[i].m_TsProcLock) {
			GLOBAL_TRACE(("PFC_SemaphoreCreate Failed!\n"));
			goto ret_end;
		}
		PFC_SemaphoreSignal(pHandle->m_pVidProcParam[i].m_TsProcLock);
		pHandle->m_pVidProcParam[i].m_VidProThreadHandle = PFC_TaskCreate("Hi3531 Stream Process", 1024 * 1024, ENC_3531AVideoProcThread, 1, (void *)i);
		if (!pHandle->m_pVidProcParam[i].m_VidProThreadHandle) {
			GLOBAL_TRACE(("PFC_TaskCreate Failed!\n"));
			goto ret_end;
		}
	}

	pHandle->m_PsiSendThread = PFC_TaskCreate("Hi3531 PSI Send", 1024 * 1024, ENC_3531APsiInsertThread, 1, pHandle);
	if (!pHandle->m_PsiSendThread) {
		GLOBAL_TRACE(("PFC_TaskCreate Failed!\n"));
		goto ret_end;
	}

	pHandle->m_TsProcThread = PFC_TaskCreate("Hi3531 TS Process", 1024 * 1024 * 2, ENC_3531ATsProcThread, 1, pHandle);
	if (!pHandle->m_TsProcThread) {
		GLOBAL_TRACE(("PFC_TaskCreate Failed!\n"));
		goto ret_end;
	}		

	pHandle->m_MonitorTimer = PFC_TimerCreate(1000, ENC_3531AMonitorTimer, pHandle);
	if (!pHandle->m_MonitorTimer) {
		GLOBAL_TRACE(("PFC_TimerCreate Failed!\n"));
		goto ret_end;
	}

	return TRUE;
ret_end:
	ENC_3531ATerminate(pHandle);
	return FALSE;
}

HANDLE ENC_3531ACreate(ENC_3531AInitParam *pInitParam)
{
	if (!s_pHandle) {
		s_pHandle = (ENC_3531AHandle *)malloc(sizeof(ENC_3531AHandle));
		if (!s_pHandle) {
			GLOBAL_TRACE(("malloc error!\n"));
		}
		else {
			memcpy(&s_pHandle->m_InitParam, pInitParam, sizeof(ENC_3531AInitParam));
			if (ENC_3531AInit(s_pHandle) == FALSE) {
				GLOBAL_SAFEFREE(s_pHandle);
			}
		}
	}

	return s_pHandle;
}

void ENC_3531ADestroy(HANDLE Handle)
{
	ENC_3531AHandle *plHandle = (ENC_3531AHandle *)Handle;

	if (plHandle) {
		ENC_3531ATerminate(plHandle);

		GLOBAL_SAFEFREE(s_pHandle);
	}
}

BOOL ENC_3531ASetParam(HANDLE Handle, S32 ChnIndex, ENC_3531AParam *pParam)
{
	ENC_3531AHandle *plHandle = (ENC_3531AHandle *)Handle;

	if (plHandle) {
		if (!GLOBAL_CHECK_INDEX(ChnIndex, s_pHandle->m_InitParam.m_ChnNum)) {
			GLOBAL_TRACE((ANSI_COLOR_RED"Index ERROR!!\n"ANSI_COLOR_NONE));
			return FALSE;
		}
	
		/* 由于在后面的几个初始化会使用网络发送数据，所以网络参数初始化在最前面 */
		if (!ENC_3531ASetEthParam(plHandle, pParam, ChnIndex)) { 
			GLOBAL_TRACE(("ENC_3531ASetEthParam Error!\n"));
			return FALSE;
		}

		if (!ENC_3531ASetPsiParam(&plHandle->m_pPsiProcParam[ChnIndex], ChnIndex, pParam)) {
			GLOBAL_TRACE(("ENC_3531ASetPsiParam Error!\n"));
			return FALSE;
		}
		
		if (!ENC_3531ASetAudParam(&plHandle->m_pAudProcParam[ChnIndex], ChnIndex, pParam)) {
			GLOBAL_TRACE(("ENC_3531ASetAudParam Error!\n"));
			return FALSE;
		}

		if (!ENC_3531ASetVidParam(&plHandle->m_pVidProcParam[ChnIndex], ChnIndex, pParam)) {
			GLOBAL_TRACE(("ENC_3531ASetVidParam Error!\n"));
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

BOOL ENC_3531AGetStatus(HANDLE Handle, S32 ChnIndex, ENC_3531AStatus *pStatus)
{
	ENC_3531AHandle *plHandle = (ENC_3531AHandle *)Handle;

	if (plHandle && pStatus) {
		if (!GLOBAL_CHECK_INDEX(ChnIndex, s_pHandle->m_InitParam.m_ChnNum)) {
			GLOBAL_TRACE((ANSI_COLOR_RED"Index ERROR!!\n"ANSI_COLOR_NONE));
			return FALSE;
		}

		GLOBAL_MEMCPY(pStatus, &plHandle->m_pMonitorParam[ChnIndex].m_RecordTsInfo, sizeof(ENC_3531AStatus));
		return TRUE;
	}

	return FALSE;
}

BOOL ENC_3531AGetAlarmInfo(HANDLE Handle, S32 ChnIndex, ENC_3531AAlarmInfo *pAlarmInfo)
{
	ENC_3531AHandle *plHandle = (ENC_3531AHandle *)Handle;

	if (plHandle && pAlarmInfo) {
		if (!GLOBAL_CHECK_INDEX(ChnIndex, s_pHandle->m_InitParam.m_ChnNum)) {
			GLOBAL_TRACE((ANSI_COLOR_RED"Index ERROR!!\n"ANSI_COLOR_NONE));
			return FALSE;
		}

		GLOBAL_MEMCPY(pAlarmInfo, &plHandle->m_pMonitorParam[ChnIndex].m_AlarmInfo, sizeof(ENC_3531AAlarmInfo));
		return TRUE;
	}

	return FALSE;
}

/* 重置报警信息（pAlarmInfo 为 NULL，全部报警计数重置，否则用改值去复位） */
BOOL ENC_3531AResetAlarmInfo(HANDLE Handle, S32 ChnIndex, ENC_3531AAlarmInfo *pAlarmInfo)
{
	ENC_3531AHandle *plHandle = (ENC_3531AHandle *)Handle;

	if (plHandle) {
		if (!GLOBAL_CHECK_INDEX(ChnIndex, s_pHandle->m_InitParam.m_ChnNum)) {
			GLOBAL_TRACE((ANSI_COLOR_RED"Index ERROR!!\n"ANSI_COLOR_NONE));
			return FALSE;
		}

		if (pAlarmInfo) {
			PFC_SemaphoreWait(plHandle->m_pMonitorParam[ChnIndex].m_Lock, -1);
			GLOBAL_MEMCPY(&plHandle->m_pMonitorParam[ChnIndex].m_AlarmInfo, pAlarmInfo, sizeof(ENC_3531AAlarmInfo));
			PFC_SemaphoreSignal(plHandle->m_pMonitorParam[ChnIndex].m_Lock);
		}
		else {
			PFC_SemaphoreWait(plHandle->m_pMonitorParam[ChnIndex].m_Lock, -1);
			GLOBAL_MEMSET(&plHandle->m_pMonitorParam[ChnIndex].m_AlarmInfo, 0, sizeof(ENC_3531AAlarmInfo));
			PFC_SemaphoreSignal(plHandle->m_pMonitorParam[ChnIndex].m_Lock);
		}
		return TRUE;
	}

	return FALSE;
}

/* EOF */
