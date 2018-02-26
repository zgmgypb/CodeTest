#ifndef GN_HWL_MFPGA_H
#define GN_HWL_MFPGA_H

#include "gn_global.h"
#include "gn_hwl_psi.h"
#include "gn_hwl_dxt8243.h"

enum
{
	HWL_MFPGA_INPUT_ERROR_NONE = 0, /* 无错误 */
	HWL_MFPGA_INPUT_ERROR_CC, /* 连续计数错误，表示有流输入，但TS流有问题 */
	HWL_MFPGA_INPUT_ERROR_VIDEO, /* 视频数据错误，表示没有视频数据输入 */
	HWL_MFPGA_INPUT_ERROR_AUDIO, /* 音频数据错误，表示没有音频数据输入 */
};

enum
{
	HWL_MFPGA_OUTPUT_ERROR_NONE = 0,
	HWL_MFPGA_OUTPUT_ERROR_OVERFLOW, /* 码流溢出 */
	HWL_MFPGA_OUTPUT_ERROR_NOBITRATE /* 无码率输出 */
};

typedef struct
{
	U16	m_VideoPid;
	U16	m_AudioPid;
	U16	m_PcrPid;
}MFPGA_PidGroup;

typedef struct
{
	MFPGA_PidGroup	m_OldPid[GN_CH_NUM_PER_ENC_BOARD];
	MFPGA_PidGroup	m_NewPid[GN_CH_NUM_PER_ENC_BOARD];
}MFPGA_PidMapParam;

typedef struct
{
	MFPGA_PidGroup	m_PidGroup[GN_CH_NUM_PER_ENC_BOARD];
}MFPGA_PidFilterParam;

/* PLD说明书说支持100个到地址0x4DFF，这需要地址线A0~A15，
但A15地址线没接，所以这里我们取一个能满足8路节目的最大的PSI表数目的值即可 */
typedef struct  
{
	S32	m_Interval; /* 插入间隔ms */
	U8		m_Data[MFPGA_MAX_PSI_NUM][MPEG2_TS_PACKET_SIZE];
	U32	m_PsiPacketNum; /* 要插入包个数 */

	U32	m_SdtPacketNum; /* 写入SDT包的个数 */
	U32	m_SecondSdtPacketPosition; /* 第二个SDT包在总包数中的位置 */
}MFPGA_PsiInsertParam;	/* PSI插入参数 */

typedef struct  
{
	struct
	{
		BOOL		m_WorkEn;

		U32	m_VideoPid;
		U32	m_AudioPid;
		U32	m_PcrPid;
		U32	m_PmtPid;

		S32	m_VideoEncStandard;		/* 视频编码模式 */
		S32	m_AudioEncStandard;		/* 音频编码模式 */

		CHAR_T m_pServiceName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN]; /* 节目名 */
		U32	m_ServiceId; /* 节目号 */
	}m_ChParam[GN_ENC_CH_NUM];
	U16	m_TsId;			/* transport_stream_id */
	U16	m_OnId;			/* original_network_id */
	U32	m_Charset;		/* 字符集 */
}HWL_MfpgaCfgParam;

typedef struct
{
	U32	m_InputErrorFlag[GN_ENC_CH_NUM];
	U32	m_OutputErrorFlag;
}HWL_MfpgaStatusParam;

BOOL MFPGA_Init(void);
BOOL MFPGA_Terminate(void);
BOOL MFPGA_ConfigRbf(void);
BOOL MFPGA_GetRelease(CHAR_T *pRelease);
U16 MFPGA_GetFpgaId(void);
U32 MFPGA_GetSecretCardId(void);
U8 MFPGA_GetInputVideoPacketNum(U32 ChIndex);
U8 MFPGA_GetInputAudioPacketNum(U32 ChIndex);
U8 MFPGA_GetInputCCErrorFlag(void);
BOOL MFPGA_GetBufferOverflowStatus(U32 OutBitrate); /* 单位kbps */
/* 返回主板到输出板的实时有效数据速率, 返回单位kbps */
U32 MFPGA_GetOutValidBitrate(void);
BOOL MFPGA_SetOutBitrate(U32 OutBitrate); /* 单位kbps */
BOOL MFPGA_SetPidMap(U32 EncBoardIndex, MFPGA_PidMapParam *pPidMapPara);
BOOL MFPGA_SetPidFilter(U32 EncBoardIndex, MFPGA_PidFilterParam *pPidFilterPara);
BOOL MFPGA_SetPsiInsert(MFPGA_PsiInsertParam *pPsiInsertPara);
BOOL MFPGA_SetUartSelect(U32 EncBoardIndex);
BOOL HWL_SetParaToMainFpga(HWL_MfpgaCfgParam *pCfgParam);
BOOL HWL_GetParaFromMainFpga(HWL_MfpgaStatusParam *pStatusParam);

#endif 

