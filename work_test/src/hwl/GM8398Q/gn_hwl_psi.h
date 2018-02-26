#ifndef GN_HWL_PSI_H
#define GN_HWL_PSI_H

#include "gn_global.h"
#include "gn_hwl_dxt8243.h"

#define MFPGA_MAX_PSI_NUM 32 

enum
{
	HWL_VID_ENC_H264 = 0,
	HWL_VID_ENC_MPEG2,
	HWL_VID_ENC_AVS,
	HWL_VID_ENC_AVS_PLUS /* AVS+ */
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

	U32	m_SdtPacketNum; /* 写入SDT包的个数 */
	U32	m_SecondSdtPacketPosition; /* 第二个SDT包在总包数中的位置 */
}PSI_PacketInfo;

typedef struct 
{
	U16	m_TsId;			/* transport_stream_id */
	U16	m_OnId;			/* original_network_id */

	U32	m_Charset;
}PSI_TsParam;

typedef struct 
{
	BOOL	m_WorkEn;			/* 工作模式 */

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

#endif /* GN_HWL_H */
/* EOF */
