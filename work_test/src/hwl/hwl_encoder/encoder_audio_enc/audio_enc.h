#ifndef _AUDIO_ENC_H_
#define _AUDIO_ENC_H_
/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
#include "multi_private.h"
#include "hi_comm_aio.h"
#include "encoder_3531A.h"

/* Macro ---------------------------------------------------------------------- */


/* Types Define (struct typedef)----------------------------------------------- */
typedef struct {
	ENC_3531AAudEncMode	m_AudEncMode;
	AUDIO_SAMPLE_RATE_E	m_AudSample; /* 单位 Hz */
	U32					m_AudBitrate; /* 单位 Kbps */
} AUD_EncInitParam;

/* Functions prototypes ------------------------------------------------------- */
HANDLE32 AUD_EncCreate(AUD_EncInitParam *pInitParam);
BOOL AUD_EncProcess(HANDLE32 Handle, U8 *pData, S32 DataLen, U8 **pBuf, S32 *pActLen);
void AUD_EncDestroy(HANDLE32 Handle);
/* 由于 AAC 需要输入固定大小的数据buffer，需要获取需要的大小 */
S32 AUD_EncAacGetInputBufSize(HANDLE32 Handle);

#endif
/*EOF*/
