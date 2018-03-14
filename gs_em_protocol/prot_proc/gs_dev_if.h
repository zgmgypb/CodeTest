#ifndef GS_DEV_IF_H
#define GS_DEV_IF_H

#include "gs_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/* 设备参数转换 */
typedef enum {
	DEV_MODULE_TTS = 0, /* TTS 参数 */
	DEV_MODULE_TUNER, /* Tuner 参数 */
	DEV_MODULE_FMT, /* FM 发送模块参数 */
	DEV_MODULE_FMR, /* FM 接收模块参数 */
	DEV_MODULE_DECODER, /* 解码板参数 */
	DEV_MODULE_BC_PARAM, /* 广播数据参数 */
	DEV_MODULE_SYS, /* 设备系统参数 */
	DEV_MODULE_BUTT
} GS_EM_DEV_MODULE_TAG_E;

/* 文本编码格式 */
typedef enum {
	TTS_TEXT_ENC_FMT_GB2312 = 0,
	TTS_TEXT_ENC_FMT_GBK,
	TTS_TEXT_ENC_FMT_BIG5,
	TTS_TEXT_ENC_FMT_UNICODE,
	TTS_TEXT_ENC_FMT_BUTT
} GS_EM_DEV_TTS_ENC_FMT_E;

typedef struct{
	GS_S32						m_Index; /* 模块索引，一个设备可能有多个模块 */

	GS_U8						m_Speed; /* 语速 0-10 */
	GS_U8						m_Volume; /* 音量 0-10 */
	GS_U8						m_Tune; /* 语调 0-10 */

	GS_EM_DEV_TTS_ENC_FMT_E		m_TextEncFmt; /* 文本编码方式 */
	GS_S32						m_TextLen; /* 文本长度 */
	GS_CHAR						m_pText[65535]; /* 文本内容 */
} GS_EM_DEV_TTS_PARAM_S;

typedef enum
{
	TUNER_MODULATOR_QAM_16 = 0,
	TUNER_MODULATOR_QAM_32,
	TUNER_MODULATOR_QAM_64,
	TUNER_MODULATOR_QAM_128,
	TUNER_MODULATOR_QAM_256,
	TUNER_MODULATOR_QAM_512,
	TUNER_MODULATOR_QAM_1024,
	TUNER_MODULATOR_QAM_UNUSED,
	TUNER_MODULATOR_QAM_4,
	TUNER_MODULATOR_QAM_4NR,
	TUNER_MODULATOR_QAM_BUTT
} GS_EM_DEV_TUNER_MODULATOR_E;

typedef enum
{
	TUNER_TUNER_PLOAR_NONE = 0,
	TUNER_TUNER_PLOAR_VER,	
	TUNER_TUNER_PLOAR_HOR
} GS_EM_DEV_TUNER_PLOAR_E;

typedef enum
{
	TUNER_SPECINV_OFF = 0,
	TUNER_SPECINV_ON,
	TUNER_SPECINV_AUTO
} GS_EM_DEV_TUNER_SPECINV_E;

typedef struct{
	GS_S32						m_Index; /* 模块索引，一个设备可能有多个模块 */

	GS_U32						m_Frequency; /* 频率，单位：KHz */
	GS_U32						m_Symbol; /* 符号率，单位：KHz */
	GS_U32						m_LocalFreq; /* 本地频率，单位：KHz */
	GS_EM_DEV_TUNER_MODULATOR_E	m_Modulation; /* 信号调制模式 */
	GS_EM_DEV_TUNER_PLOAR_E		m_PolarMethod; /* 极性方式 */
	GS_BOOL						m_Switch_22K;  	
	GS_EM_DEV_TUNER_SPECINV_E	m_SpectInv; /* 频谱翻转 */
} GS_EM_DEV_TUNER_PARAM_S;

typedef enum {
	FMT_SOUNDCHN_MONO = 0,
	FMT_SOUNDCHN_STEREO,
	FMT_SOUNDCHN_BUTT
} GS_EM_DEV_FMT_SOUNDCHN_E;

typedef enum{
	FMT_DEEMPH_50us = 0,
	FMT_DEEMPH_75us,
	FMT_DEEMPH_BUTT
} GS_EM_DEV_FMT_DEEMPH_E;

typedef enum {
	FMT_I2S_SAMPLE_FREQ_32K = 0,
	FMT_I2S_SAMPLE_FREQ_40K,
	FMT_I2S_SAMPLE_FREQ_44_1K,
	FMT_I2S_SAMPLE_FREQ_48K
}GS_EM_DEV_FMT_I2S_SAMPLE_FREQ_E;

typedef struct{
	GS_S32						m_Index; /* 模块索引，一个设备可能有多个模块 */

	GS_U32						m_Freq; /* 频率, 单位：KHz 范围：76MHz ～ 108MHz */
	GS_BOOL						m_RdsSwitch; /* RDS 开关，TRUE 开启发送RDS，FALSE 关闭发送RDS */
	GS_EM_DEV_FMT_SOUNDCHN_E	m_SoundChnMode; /* 发送声道模式 */
	GS_U32						m_OutLvl; /* 输出电平，范围 0 - 100 */
	GS_BOOL						m_MuteSwitch; /* 静音开关，TRUE 静音，FALSE 正常音量发送 */
	GS_U32						m_TxFreqDev; /* 总的发送频率偏差，范围：0~17595Hz 单位：Hz */
	GS_U32						m_RdsFreqDev; /* RDS 频率偏差，范围：0~4445Hz 单位：Hz */
	GS_EM_DEV_FMT_DEEMPH_E		m_PreEmphasisConst; /* 预加重设置 */

	GS_EM_DEV_FMT_I2S_SAMPLE_FREQ_E		m_SampleFreq; /* 采样率，仅针对于 I2S 输入 */
} GS_EM_DEV_FMT_PARAM_S;

typedef enum {
	FMR_SOUNDCHN_MONO = 0,
	FMR_SOUNDCHN_AUTO,
	FMR_SOUNDCHN_BUTT
} GS_EM_DEV_FMR_SOUNDCHN_E;

typedef enum{
	FMR_DEEMPH_50us = 0,
	FMR_DEEMPH_75us,
	FMR_DEEMPH_BUTT
} GS_EM_DEV_FMR_DEEMPH_E;

typedef struct{
	GS_S32						m_Index; /* 模块索引，一个设备可能有多个模块 */

	GS_U32						m_Freq; /* 频率, 单位：KHz 范围：60MHz ～ 108MHz */
	GS_U8						m_Vol; /* 音量 0-100 */
	GS_BOOL						m_MuteSwitch; /* TRUE:静音 FALSE:不静音 */
	GS_BOOL						m_RdsSwitch; /* TRUE:开启Rds接收 FALSE:关闭Rds接收 */
	GS_EM_DEV_FMR_SOUNDCHN_E	m_SoundChnMode; /* AUTO:自动接收 MONO:强制接收Mono单声道 */
	GS_EM_DEV_FMR_DEEMPH_E		m_DeEmphasisConst; /* 去加重常量 */
} GS_EM_DEV_FMR_PARAM_S;

typedef enum {
	DECODER_VIDEO_MODE_YPbPr_CVBS = 0,
	DECODER_VIDEO_MODE_BUTT
} GS_EM_DEV_DECODER_VIDEO_MODE_E;

typedef enum {
	DECODER_TV_SYS_PAL = 0,
	DECODER_TV_SYS_NTSC = 1,
	DECODER_TV_SYS_BUTT
} GS_EM_DEV_DECODER_TV_SYS_E;

typedef enum {
	DECODER_SOUNDCHN_LEFT = 0,
	DECODER_SOUNDCHN_RIGHT,
	DECODER_SOUNDCHN_STEREO,
	DECODER_SOUNDCHN_MONO,
	DECODER_SOUNDCHN_BUTT
} GS_EM_DEV_DECODER_SOUNDCHN_E;

typedef struct{
	GS_S32							m_Index; /* 模块索引，一个设备可能有多个模块 */
	
	GS_U8							m_DecoderIndex; /* 解码芯片索引号，根据解码板具体设置 */
	GS_EM_DEV_DECODER_TV_SYS_E		m_VideoOutputStandard; /* 视频输出制式 PAL/NTSC... */
	GS_EM_DEV_DECODER_VIDEO_MODE_E	m_VideoOutputMode; /* 视频输出模式 CVBS... */ 
	GS_EM_DEV_DECODER_SOUNDCHN_E	m_SoundChannel; /* 声道 LEFT/RIGHT/STEREO... */
	GS_U8							m_AudioVolume; /* 音量 0 - 100 */

	GS_U16							m_ServiceId; /* 节目号 ID */
	GS_U16							m_PmtPid; 
	GS_U16							m_AudioPid;
} GS_EM_DEV_DECODER_PARAM_S;

typedef struct {
	GS_U8	m_Type; /* 0x01: IP + Port; 0x02: 域名 + 端口 */
	union {
		GS_U32	m_IpAddr;
		GS_CHAR	m_pDomain[64]; /* 域名最大长度限制为63个字符 */
	} m_HostName; /* 回传 IP 地址或域名 */
	GS_U16	m_Port;
} GS_EM_DEV_NET_ADDR_S;

typedef enum {
	AUD_TRANS_PROT_RTSP = 0x0,
	AUD_TRANS_PROT_RTMP,
	AUD_TRANS_PROT_RTP, 
	AUD_TRANS_PROT_BUTT
} GS_EM_DEV_AUD_TRANS_PROT_TYPE_E; /* 音频传输协议类型 */

typedef enum {
	AUD_ENC_FMT_MP3 = 0x0,
	AUD_ENC_FMT_MPEG1_L2, 
	AUD_ENC_FMT_AAC,
	AUD_ENC_FMT_BUTT
} GS_EM_DEV_AUD_ENC_FMT_E; /* 音频编码类型 */

typedef struct {
	GS_EM_DEV_AUD_TRANS_PROT_TYPE_E	m_AudTransProt;
	GS_EM_DEV_AUD_ENC_FMT_E			m_AudEncFmt;
	GS_EM_DEV_NET_ADDR_S			m_Addr;
} GS_EM_DEV_AUD_BC_INFO_S;

typedef enum {
	BC_PARAM_TYPE_AUD, 
	BC_PARAM_TYPE_PIC,
	BC_PARAM_TYPE_VID,
	BC_PARAM_TYPE_BUTT
} GS_EM_DEV_BC_PARAM_TYPE_E; /* 辅助数据类型 */

typedef struct{
	GS_EM_DEV_BC_PARAM_TYPE_E			m_Type;
	GS_U16								m_Len;
	union {
		GS_EM_DEV_AUD_BC_INFO_S			m_AudInfo;
		GS_EM_DEV_NET_ADDR_S			m_PicInfo;
		GS_EM_DEV_NET_ADDR_S			m_VidInfo;
	} m_Data;
} GS_EM_DEV_BC_PARAM_S;

typedef enum {
	SYS_PARAM_LOCALIP = 0, /* 本地 IP 地址设置 */
	SYS_PARAM_AUD_POSTBACK, /* 音频回调设置 */
	SYS_PARAM_VOL, /* 全局音量 */
	SYS_PARAM_SERVER_TCP_ADDR, /* 服务器 TCP 地址设置 */
	SYS_PARAM_BUTT
} GS_EM_DEV_SYS_PARAM_TAG_E;

typedef struct {
	GS_U32	m_IpAddr;
	GS_U32	m_IpMask;
	GS_U32	m_IpGate;
} GS_EM_DEV_LOCALIP_S;

typedef struct {
	GS_EM_DEV_SYS_PARAM_TAG_E	m_Tag;	

	union {
		GS_EM_DEV_LOCALIP_S			m_LocalIp;
		GS_EM_DEV_NET_ADDR_S		m_AudPostBack;
		GS_EM_DEV_NET_ADDR_S		m_ServerTcpAddr;
		GS_U8						m_Vol; /* 全局音量 0 - 100 */
	} m_Param;
} GS_EM_DEV_SYS_PARAM_S;

typedef enum {
	DEV_WORK_STAT_FREE = 0,
	DEV_WORK_STAT_BUSY,
	DEV_WORK_STAT_FAULT,
	DEV_WORK_STAT_BUTT
} GS_EM_DEV_WORK_STAT_E;

typedef struct {
	GS_EM_DEV_WORK_STAT_E		m_WorkStat;
	GS_U8						m_pPhyId[18]; /* 设备物理 ID */
	GS_EM_DEV_LOCALIP_S			m_LocalIp;
	GS_EM_DEV_NET_ADDR_S		m_AudPostBackAddr;
	GS_EM_DEV_NET_ADDR_S		m_ServerAddr;
	GS_U8						m_Vol; /* 全局音量 0 - 100 */
} GS_EM_DEV_SYS_STAT_S; /* 设备状态 */

typedef struct {
	GS_U16	m_ServiceId; /* 节目号 ID */

	GS_U16	m_PmtPid;
	GS_U16	m_VidPid;
	GS_U16	m_AudPid;
	GS_U16	m_PcrPid;

	GS_EM_DEV_DECODER_TV_SYS_E		m_VideoStandard; /* 视频输出制式 PAL/NTSC... */
	GS_EM_DEV_DECODER_VIDEO_MODE_E	m_VideoMode; /* 视频输出模式 CVBS... */ 
} GS_EM_DEV_PSI_INFO_S;

typedef enum {
	DEV_POSTBACK_SERV_AUD_TRANS_START,
	DEV_POSTBACK_SERV_AUD_TRANS_END,
	DEV_POSTBACK_SERV_BUTT
} GS_EM_DEV_POSTBACK_SERV_TAG_E;

typedef struct {
	GS_EM_DEV_AUD_TRANS_PROT_TYPE_E	m_TransProt;
	GS_EM_DEV_AUD_ENC_FMT_E			m_EncFmt;
	GS_EM_DEV_NET_ADDR_S			m_PostBackAddr;
} GS_EM_DEV_POSTBACK_SERV_AUD_TRANS_START_S;

typedef struct {

} GS_EM_DEV_POSTBACK_SERV_AUD_TRANS_End_S;

typedef struct {
	GS_EM_DEV_POSTBACK_SERV_TAG_E	m_ServTag;
	union {
		GS_EM_DEV_POSTBACK_SERV_AUD_TRANS_START_S	m_AudTransStart;
		GS_EM_DEV_POSTBACK_SERV_AUD_TRANS_End_S		m_AudTransEnd;
	} m_ServParam;
} GS_EM_DEV_POSTBACK_SERV_S; /* 回调业务 */

typedef enum {
	DEV_UDP_RECV_CHAN_IP0 = 0, /* IP 预留 16 个通道，0 - 15 */
	DEV_UDP_RECV_CHAN_DTMB0 = 16, /* Tuner 各自预留 6 个 */
	DEV_UDP_RECV_CHAN_DVB_S0 = 24,
	DEV_UDP_RECV_CHAN_DVB_C0 = 32
} GS_EM_DEV_UDP_RECV_CHANNEL_E; /* UDP 接收通道编号 */

typedef struct {
	/*
		m_ModuleTag:	设备内模块类型
		pParamInfo：	设备内相应模块的参数
	*/
	GS_S32 (*m_GS_EM_DevParamSet)(GS_EM_DEV_MODULE_TAG_E ModuleTag, GS_VOID *pParamInfo); /* 设备参数设置 */
	GS_S32 (*m_GS_EM_DevStatGet)(GS_EM_DEV_MODULE_TAG_E ModuleTag, GS_VOID *pStatInfo); /* 设备状态获取 */
	GS_S32 (*m_GS_EM_DevGetPsiInfo)(GS_S32 ChannelIndex, GS_EM_DEV_PSI_INFO_S *pPsiInfo); /* ChannelIndex: 通道编号，搜索指定的哪路通道，不同设备不一样 */  
	GS_S32 (*m_GS_EM_DevWaitPostBackService)(GS_EM_DEV_POSTBACK_SERV_S *pPostBackServInfo); /* 阻塞等待回传业务 */

	GS_VOID (*m_GS_EM_DevGetStorePath)(GS_CHAR *pPath); /* 获取存储路径，返回值放到 pPath，如：/mnt/mtd */
} GS_EM_DEV_IF_INIT_PARAM_S;

GS_S32 GS_EM_ProtProcTaskCreate(GS_EM_DEV_IF_INIT_PARAM_S *pInitParam); 
GS_VOID GS_EM_ProtProcTaskDestroy(GS_VOID);
GS_VOID GS_EM_RemoteReConnect(GS_VOID); /* 远端通信重连，当修改了设备/远端连接地址时调用触发 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* GS_DEV_IF_H */
/* EOF */
