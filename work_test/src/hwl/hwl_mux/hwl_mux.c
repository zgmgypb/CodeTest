#if 0
#include "hwl_mux.h"
#include "mpeg2_micro.h"

typedef struct {
	U16		m_pPidMap[MPEG2_TS_PACKET_MAX_PID_NUM];
	BOOL	m_pPidFilter[MPEG2_TS_PACKET_MAX_PID_NUM];
} HWL_MuxHandle;

static void HWLL_MuxPidMap(HWL_MuxHandle *pHandle, U8 *pTs, U32 TsNum)
{
	S32 i;
	U16 lPid;

	for (i = 0; i < TsNum; i++) {
		lPid = MPEG2_TS_PACKET_PID13_G(&pTs[1]);
		pTs[1] = (pHandle->m_pPidMap[lPid] >> 8) & 0x1FF;
		pTs[2] = pHandle->m_pPidMap[lPid] & 0xFF;
		pTs += MPEG2_TS_PACKET_SIZE;
	}
}

static BOOL HWLL_MuxPidFilter(HWL_MuxHandle *pHandle, U8 *pTs)
{
	U16 lPid;

	lPid = MPEG2_TS_PACKET_PID13_G(&pTs[1]);

	return pHandle->m_pPidFilter[lPid];
}

static S32 HWLL_MuxInThread()
{
	
}

HANDLE32 HWL_MuxCreate(HWL_MuxInitParam *pInitParam)
{
	
}

void HWL_MuxDestroy(HANDLE32 MuxHandle)
{

}
#endif