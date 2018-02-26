#ifndef _MULTI_HWL_LOCAL_ENCODER_H_
#define _MULTI_HWL_LOCAL_ENCODER_H_

/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
#ifdef GN1846
#include "encoder_3531A.h"
#include "mpeg2_micro.h"
#endif
/* Macro ---------------------------------------------------------------------- */
/* Types Define (struct typedef)----------------------------------------------- */

typedef void (*HWL_LENCODER_CB)(void *pUserParam);

#ifdef GN1846
#define LOCAL_IP_ADDRESS 0x7F000001 /* "127.0.0.1" */
#define LOCAL_IP_PORT 0x1000
#endif

typedef struct
{
	/*物理过滤器*/
	S32					m_ChnNum; /* 编码通道数目 */
	S32					m_SubNumPerCHN; /* 每个编码通道包含的子通道数 */
}HWL_LENCODER_InitParam;


typedef struct
{
	BOOL					m_bEnable;
	U32						m_UDPAddr; /* 每个子通道都有一个地址 */
	S32						m_UDPPort;

#ifdef GN1846
	ENC_3531AParam			m_EncParam;
#endif
}HWL_LENCODER_SubParam;

typedef struct
{
	BOOL					m_bEnable;
	U32						m_IPAddr;
	U32						m_Mask;
	U32						m_Gate;
	HWL_LENCODER_SubParam	*m_pSubCHN;
}HWL_LENCODER_ChnParam;

#ifdef GN1846
typedef struct {
	ENC_3531AStatus m_Status;
} HWL_LENCODER_Status;

typedef struct {
	ENC_3531AAlarmInfo m_AlarmInfo;
} HWL_LENCODER_AlarmInfo;
#endif

/* Functions prototypes ------------------------------------------------------- */
BOOL HWL_LENCODER_Initiate(HWL_LENCODER_InitParam *pParam);
void HWL_LENCODER_SetChnParam(S32 ChnInd, HWL_LENCODER_ChnParam *pParam);
void HWL_LENCODER_SetSubParam(S32 ChnInd, S32 SubInd, HWL_LENCODER_SubParam *pParam);
BOOL HWL_LENCODER_SetTsPacket(S32 TsInd, HWL_LENCODER_SubParam *pParam);
S32 HWL_LENCODER_GetTsBitrate(S32 ChnInd, S32 TsInd);
void HWL_LENCODER_Terminate(void);
#ifdef GN1846
BOOL HWL_LENCODER_GetStatus(S32 ChnInd, S32 SubInd, HWL_LENCODER_Status *pStatus);
BOOL HWL_LENCODER_GetAlarmInfo(S32 ChnInd, S32 SubInd, HWL_LENCODER_AlarmInfo *pAlarmInfo);
BOOL HWL_LENCODER_ResetAlarmInfo(S32 ChnInd, S32 SubInd, HWL_LENCODER_AlarmInfo *pAlarmInfo);
#endif

#endif
/*EOF*/
