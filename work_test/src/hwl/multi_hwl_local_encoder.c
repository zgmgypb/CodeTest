//这个是有自己的线程和进程的的模块

/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "mpeg2_micro.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_hwl_local_encoder.h"
#include "multi_hwl_igmp.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "multi_hwl_monitor.h"
#include "multi_hwl_tags.h"
#include "ts.h"
#ifdef GN1846
#include "multi_main_internal.h"
#include "encoder_3531A.h"
#endif

/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define MODULETASK_STATCK_SIZE					(1024*1024)

/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */

typedef struct
{
	HWL_LENCODER_InitParam	m_InitParam;
	U8						m_bTaskMark;
	HANDLE32				m_ThreadHandle;

	HWL_LENCODER_ChnParam	*m_pChn;
#ifdef GN1846
	HANDLE32				*m_pChnHandle;
#endif
}HWL_LENCODER_Handle;


/* Private Variables (static)-------------------------------------------------- */
static HWL_LENCODER_Handle *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

void HWL_LENCODER_ModuleTaskFn(void *pUserParam)
{
	HWL_LENCODER_Handle *plHandle = (HWL_LENCODER_Handle*) pUserParam;
	if (plHandle)
	{
		GLOBAL_TRACE(("Task Started!\n"));
		plHandle->m_bTaskMark = TRUE;
		while(plHandle->m_bTaskMark == TRUE)
		{
			PFC_TaskSleep(1000);
		}
		GLOBAL_TRACE(("Task Stoped!\n"));
	}
}

BOOL HWL_LENCODER_Initiate(HWL_LENCODER_InitParam *pParam)
{
	BOOL lRet = FALSE;

	if (s_pHandle == NULL)
	{
		HWL_LENCODER_Handle *plHandle = (HWL_LENCODER_Handle*) GLOBAL_MALLOC(sizeof(HWL_LENCODER_Handle));

		if (plHandle)
		{
			GLOBAL_ZEROMEM(plHandle, sizeof(HWL_LENCODER_Handle));
			GLOBAL_MEMCPY(&plHandle->m_InitParam, pParam, sizeof(HWL_LENCODER_InitParam));
#ifdef GN1846
			plHandle->m_pChnHandle = (HANDLE32 *)GLOBAL_ZMALLOC(plHandle->m_InitParam.m_ChnNum * sizeof(HANDLE32));
			if (plHandle->m_pChnHandle) {
				S32 i, j;
				
				lRet = TRUE;
				for (i = 0; i < plHandle->m_InitParam.m_ChnNum; i++) {
					ENC_3531AInitParam lInitParam;

					lInitParam.m_ChnNum = plHandle->m_InitParam.m_SubNumPerCHN;
					lInitParam.m_GetEthSetFlagCB = MULTL_GetEthSetFlag;
					plHandle->m_pChnHandle[i] = ENC_3531ACreate(&lInitParam);
					if (!plHandle->m_pChnHandle[i]) {
						GLOBAL_TRACE(("ENC_3531ACreate Failed!\n"));
						lRet = FALSE;
						break;
					}
				}
				GLOBAL_TRACE(("Initiate Complete!\n"));
			}
			else {
				GLOBAL_SAFEFREE(plHandle);
				GLOBAL_TRACE(("\n"));
			}
#else
			plHandle->m_pChn = (HWL_LENCODER_ChnParam *)GLOBAL_ZMALLOC(plHandle->m_InitParam.m_ChnNum * sizeof(HWL_LENCODER_ChnParam));
			if (plHandle->m_pChn)
			{
				S32 i;
				for (i = 0; i < plHandle->m_InitParam.m_ChnNum; i++)
				{
					plHandle->m_pChn[i].m_pSubCHN = (HWL_LENCODER_SubParam *)GLOBAL_ZMALLOC(plHandle->m_InitParam.m_SubNumPerCHN * sizeof(HWL_LENCODER_SubParam));
				}
				GLOBAL_TRACE(("Initiate Complete!\n"));

				PFC_TaskCreate((CHAR_T*)__FUNCTION__, MODULETASK_STATCK_SIZE, HWL_LENCODER_ModuleTaskFn, 0, plHandle);

				lRet = TRUE;
			}
			else
			{
				GLOBAL_SAFEFREE(plHandle);
				GLOBAL_TRACE(("\n"));
			}
#endif
		}
		s_pHandle = plHandle;
	}

	return lRet;
}


void HWL_LENCODER_SetChnParam(S32 ChnInd, HWL_LENCODER_ChnParam *pParam)
{
	HWL_LENCODER_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		if (GLOBAL_CHECK_INDEX(ChnInd, plHandle->m_InitParam.m_ChnNum))
		{
		}
		else
		{
			GLOBAL_TRACE(("Overlimit !!!!\n"));
		}
	}
}

void HWL_LENCODER_SetSubParam(S32 ChnInd, S32 SubInd, HWL_LENCODER_SubParam *pParam)
{
	HWL_LENCODER_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		if (GLOBAL_CHECK_INDEX(ChnInd, plHandle->m_InitParam.m_ChnNum))
		{
#ifdef GN1846
			ENC_3531ASetParam(s_pHandle->m_pChnHandle[ChnInd], SubInd, &pParam->m_EncParam);
#endif
		}
		else
		{
			GLOBAL_TRACE(("Overlimit !!!!\n"));
		}
	}
}

BOOL HWL_LENCODER_SetTsPacket(S32 TsInd, HWL_LENCODER_SubParam *pParam)
{
	HWL_LENCODER_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		/*注意这里自行对应Chn和TS*/
		if (GLOBAL_CHECK_INDEX(TsInd, plHandle->m_InitParam.m_ChnNum))
		{
		}
		else
		{
			GLOBAL_TRACE(("Overlimit !!!!\n"));
		}
	}
}

S32 HWL_LENCODER_GetTsBitrate(S32 ChnInd, S32 TsInd)
{
	HWL_LENCODER_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		/*注意这里自行对应Chn和TS*/
		if (GLOBAL_CHECK_INDEX(ChnInd, plHandle->m_InitParam.m_ChnNum))
		{
#ifdef GN1846
			HWL_LENCODER_Status lStatus;
			ENC_3531AGetStatus(s_pHandle->m_pChnHandle[ChnInd], TsInd, &lStatus.m_Status);

			return lStatus.m_Status.m_TotalTsCount * MPEG2_TS_PACKET_SIZE * 8;
#endif
		}
		else
		{
			GLOBAL_TRACE(("Overlimit !!!!\n"));
		}
	}
}

#ifdef GN1846
BOOL HWL_LENCODER_GetStatus(S32 ChnInd, S32 SubInd, HWL_LENCODER_Status *pStatus)
{
	HWL_LENCODER_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		/*注意这里自行对应Chn和TS*/
		if (GLOBAL_CHECK_INDEX(ChnInd, plHandle->m_InitParam.m_ChnNum))
		{
			return ENC_3531AGetStatus(s_pHandle->m_pChnHandle[ChnInd], SubInd, &pStatus->m_Status);
		}
		else
		{
			GLOBAL_TRACE(("Overlimit !!!!\n"));
		}
	}
}

BOOL HWL_LENCODER_GetAlarmInfo(S32 ChnInd, S32 SubInd, HWL_LENCODER_AlarmInfo *pAlarmInfo)
{
	HWL_LENCODER_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		/*注意这里自行对应Chn和TS*/
		if (GLOBAL_CHECK_INDEX(ChnInd, plHandle->m_InitParam.m_ChnNum))
		{
			return ENC_3531AGetAlarmInfo(s_pHandle->m_pChnHandle[ChnInd], SubInd, &pAlarmInfo->m_AlarmInfo);
		}
		else
		{
			GLOBAL_TRACE(("Overlimit !!!!\n"));
		}
	}
}

BOOL HWL_LENCODER_ResetAlarmInfo(S32 ChnInd, S32 SubInd, HWL_LENCODER_AlarmInfo *pAlarmInfo)
{
	HWL_LENCODER_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		/*注意这里自行对应Chn和TS*/
		if (GLOBAL_CHECK_INDEX(ChnInd, plHandle->m_InitParam.m_ChnNum))
		{
			return ENC_3531AResetAlarmInfo(s_pHandle->m_pChnHandle[ChnInd], SubInd, &pAlarmInfo->m_AlarmInfo);
		}
		else
		{
			GLOBAL_TRACE(("Overlimit !!!!\n"));
		}
	}
}
#endif

void HWL_LENCODER_Terminate(void)
{
	HWL_LENCODER_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		S32 i;
		s_pHandle->m_bTaskMark = FALSE;
		PFC_TaskWait(s_pHandle->m_ThreadHandle, GLOBAL_INVALID_INDEX);
		for (i = 0; i < plHandle->m_InitParam.m_ChnNum; i++)
		{
#ifdef GN1846
			ENC_3531ADestroy(plHandle->m_pChnHandle[i]);
#else
			GLOBAL_SAFEFREE(plHandle->m_pChn[i].m_pSubCHN);
#endif
		}
#ifdef GN1846
		GLOBAL_SAFEFREE(s_pHandle->m_pChnHandle);
#else
		GLOBAL_SAFEFREE(s_pHandle->m_pChn);
#endif
		GLOBAL_SAFEFREE(s_pHandle);
		GLOBAL_TRACE(("Terminate Complete!\n"));
	}
}

/*EOF*/
