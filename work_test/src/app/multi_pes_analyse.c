//这个是有自己的线程和进程的的模块

/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "mpeg2.h"
#include "transenc.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define MODULETASK_STATCK_SIZE					(1024*1024)

/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef enum
{
	PES_STATE_IDLE = 0,
	PES_STATE_START,
	PES_STATE_STOP,
	PES_STATE_WAITING,
	PES_STATE_NUM
}PES_STATE;

typedef struct
{
	BOOL					m_bTaskMark;
	HANDLE32				m_ThreadHandle;
	BOOL					m_CompleteMark;
	S32						m_State;

	U32						m_PESFilterID;
	MPEG2_PESSequenceHeader m_PESSequenceHeader;
	S32						m_Timeout;

	S32						m_ESType;
	U16						m_ESPID;
	S32						m_TsInd;

	MPEG2_LocalESInfo		m_TransESInfo;

	HANDLE32				m_DBSHandle;
}PES_Handle;


/* Private Variables (static)-------------------------------------------------- */
static PES_Handle *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
void PESL_PESFilterDataArriveCB(void* pUserParam, U32 CallerID, void* pData, S32 DataCount)
{
	PES_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		if (plHandle->m_State == PES_STATE_WAITING)
		{
			MPEG2_PESHead lPESHead;

			MPEG2_PESHeadParser((U8*)pData, DataCount, &lPESHead, FALSE);

			//CAL_PrintDataBlock(__FUNCTION__, lPESHead.m_pPESPayload, ((lPESHead.m_PayloadSize > 128)?128:lPESHead.m_PayloadSize));

			if (plHandle->m_ESType == MPEG2_STREAM_TYPE_MPEG2_VIDEO)
			{
				MPEG2_PES_MPEG2ESInfo lPESEsInfo;
				if (MPEG2_PESVideoEsInfoGet(lPESHead.m_pPESPayload, lPESHead.m_PayloadSize, &lPESEsInfo))
				{
					GLOBAL_TRACE(("Parse Successful!!!!!!!!!!!!!\n"));

					MPEG2_PES_MPEG2VIDESInfoToLocalESInfo(&lPESEsInfo, &plHandle->m_TransESInfo);

					plHandle->m_CompleteMark = TRUE;
					plHandle->m_State = PES_STATE_STOP;
				}
			}
			else if (plHandle->m_ESType == MPEG2_STREAM_TYPE_MPEG4_AVC_H264_VIDEO)
			{
				MPEG2_PES_AVCESInfo lPESAVCEsInfo;

				if (MPEG2_PESAVCEsInfoGet(lPESHead.m_pPESPayload, lPESHead.m_PayloadSize, &lPESAVCEsInfo))
				{

					MPEG2_PES_MPEG4AVCESInfoToLocalESInfo(&lPESAVCEsInfo, &plHandle->m_TransESInfo);

					plHandle->m_CompleteMark = TRUE;
					plHandle->m_State = PES_STATE_STOP;
				}
			}
		}
	}
}

void PESL_ModuleTaskFn(void *pUserParam)
{
	PES_Handle *plHandle = (PES_Handle*) pUserParam;
	if (plHandle)
	{
		S32 lDuration;
		U32 lTimeTick;
		lTimeTick = PFC_GetTickCount();
		plHandle->m_bTaskMark = TRUE;
		while(plHandle->m_bTaskMark == TRUE)
		{
			lDuration = CAL_TimeDurationMS(&lTimeTick, PFC_GetTickCount(), 10);

			switch (plHandle->m_State)
			{
			case PES_STATE_IDLE:
				{
					PFC_TaskSleep(200);
				}
				break;
			case PES_STATE_START:
				{
					plHandle->m_CompleteMark = FALSE;
					plHandle->m_Timeout = 5000;
					TSP_SetCurrentWorkTsIndex(plHandle->m_TsInd);

					plHandle->m_PESFilterID = TSP_PESRequeset(plHandle->m_ESPID, PESL_PESFilterDataArriveCB, plHandle, TRUE);
					if (plHandle->m_PESFilterID != 0)
					{
						GLOBAL_TRACE(("Analyse Started!!!\n"));
						plHandle->m_State = PES_STATE_WAITING;
					}
					else
					{
						GLOBAL_TRACE(("Request Failed!!!\n"));
						plHandle->m_State = PES_STATE_IDLE;//失败了！
					}

				}
				break;
			case PES_STATE_WAITING:
				{
					if (CAL_TimeoutCheck(&plHandle->m_Timeout, lDuration))
					{
						plHandle->m_State = PES_STATE_STOP;
						GLOBAL_TRACE(("Analyse Timeout!!!\n"));
					}
					else
					{
						PFC_TaskSleep(100);
					}
				}
				break;
			case PES_STATE_STOP:
				{
					if (plHandle->m_PESFilterID != 0)
					{
						TSP_PESRequesetCancel(plHandle->m_PESFilterID, TRUE);
						plHandle->m_PESFilterID = 0;
					}
					plHandle->m_State = PES_STATE_IDLE;
				}
				break;
			default:
				plHandle->m_State = PES_STATE_IDLE;
				break;
			}
		}
	}
}

/* API -------------------------------------------------------------------------------------------------------------------------------------------- */
BOOL PES_Initiate(HANDLE32 DBSHandle)
{
	BOOL lRet = FALSE;
	if (s_pHandle == NULL)
	{
		PES_Handle *plHandle = (PES_Handle*) GLOBAL_MALLOC(sizeof(PES_Handle));
		if (plHandle)
		{
			GLOBAL_ZEROMEM(plHandle, sizeof(PES_Handle));

			plHandle->m_DBSHandle = DBSHandle;
			plHandle->m_State = PES_STATE_IDLE;
			plHandle->m_CompleteMark = FALSE;


			plHandle->m_ThreadHandle = PFC_TaskCreate((CHAR_T*)__FUNCTION__, 40 * 1024, PESL_ModuleTaskFn, 0, plHandle);

			GLOBAL_TRACE(("%s Complete!!!!\n", __FUNCTION__));
			lRet = TRUE;
		}
		s_pHandle = plHandle;
	}
	return lRet;
}

void PES_StopAnalyse(void)
{
	PES_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		if (plHandle->m_State != PES_STATE_IDLE)
		{
			plHandle->m_State = PES_STATE_STOP;
			while(TRUE)
			{
				if (plHandle->m_State != PES_STATE_IDLE)
				{
					PFC_TaskSleep(100);
				}
			}
		}
	}
}


BOOL PES_StartAnalyse(S32 TsInd, U32 ESPID, S32 ESType)
{
	BOOL lRet = FALSE;
	PES_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		if (plHandle->m_State == PES_STATE_IDLE)
		{
			GLOBAL_TRACE(("Analyse PES Start TsInd = %d, ESType = %d(%s), ESPID = %d\n", TsInd, ESType, MPEG2_PES_VIDFMTEnumToStr(MPEG2_PES_MPEG2SYSAVFormatCodeToEnum(ESType)), ESPID));

			if (ESType == MPEG2_STREAM_TYPE_MPEG4_AVC_H264_VIDEO
				|| ESType == MPEG2_STREAM_TYPE_MPEG2_VIDEO)
			{

				plHandle->m_ESType = ESType;
				plHandle->m_TsInd = TsInd;
				plHandle->m_ESPID = ESPID;
				plHandle->m_CompleteMark = FALSE;

				plHandle->m_State = PES_STATE_START;

				lRet = TRUE;
			}
		}
		else
		{
			PES_StopAnalyse();
			PES_StartAnalyse(TsInd, ESPID, ESType);
		}
	}
	return lRet;
}

BOOL PES_GetDone(void)
{
	BOOL lRet = FALSE;
	PES_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		if (plHandle->m_State == PES_STATE_IDLE)
		{
			lRet = TRUE;
		}
	}
	return lRet;
}

BOOL PES_GetVIDEsInfoStr(CHAR_T *pBuf, S32 BufSize)
{
	BOOL lRet = FALSE;
	PES_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		if ((plHandle->m_State == PES_STATE_IDLE) && (plHandle->m_CompleteMark))
		{
			MPEG2_LocalESInfo *plEsInfo = &plHandle->m_TransESInfo;

			GLOBAL_SNPRINTF((pBuf, BufSize, "%s %dX%d%c@", MPEG2_PES_VIDFMTEnumToStr(MPEG2_PES_MPEG2SYSAVFormatCodeToEnum(plHandle->m_ESType)), \
				plEsInfo->m_HorizontalSize, plEsInfo->m_VerticalSize, \
				(plEsInfo->m_bProgressive == FALSE)?'I':'P'));

			if (plEsInfo->m_bUseFrameRateValue == TRUE)
			{
				if (plEsInfo->m_FrameRateValue != 0)
				{
					if (plEsInfo->m_FrameRateValue != 25 
						&& plEsInfo->m_FrameRateValue != 30
						&& plEsInfo->m_FrameRateValue != 50
						&& plEsInfo->m_FrameRateValue != 60
						)
					{
						GLOBAL_SNPRINTF((pBuf + GLOBAL_STRLEN(pBuf), BufSize - GLOBAL_STRLEN(pBuf), " %.2f", plEsInfo->m_FrameRateValue));
					}
					else
					{
						GLOBAL_SNPRINTF((pBuf + GLOBAL_STRLEN(pBuf), BufSize - GLOBAL_STRLEN(pBuf), " %.0f", plEsInfo->m_FrameRateValue));
					}
				}
				else
				{
					GLOBAL_SNPRINTF((pBuf + GLOBAL_STRLEN(pBuf), BufSize - GLOBAL_STRLEN(pBuf), " 00"));
				}
			}
			else if (plEsInfo->m_FrameRateEnum != 0)
			{
				GLOBAL_STRCAT(pBuf, MPEG2_PES_VIDFREnumToStr(plEsInfo->m_FrameRateEnum));
			}

			GLOBAL_STRCAT(pBuf, " [");
			if (plHandle->m_ESType == MPEG2_STREAM_TYPE_MPEG2_VIDEO)
			{
				GLOBAL_STRCAT(pBuf, MPEG2_PES_MPEG2ProfileEnumToStr(plEsInfo->m_ProfileEnum));
				GLOBAL_STRCAT(pBuf, " ");
				GLOBAL_STRCAT(pBuf, MPEG2_PES_MPEG2LevelEnumToStr(plEsInfo->m_LevelEnum));
			}
			else if (plHandle->m_ESType == MPEG2_STREAM_TYPE_MPEG4_AVC_H264_VIDEO)
			{
				GLOBAL_STRCAT(pBuf, MPEG2_PES_MPEG4AVCH264ProfileEnumToStr(plEsInfo->m_ProfileEnum));
				GLOBAL_STRCAT(pBuf, " ");
				GLOBAL_STRCAT(pBuf, MPEG2_PES_MPEG4AVCH264LevelEnumToStr(plEsInfo->m_LevelEnum));
			}
			GLOBAL_STRCAT(pBuf, "]");

			GLOBAL_STRCAT(pBuf, " [");
			GLOBAL_STRCAT(pBuf, MPEG2_PES_VIDChromaEnumToStr(plEsInfo->m_ChromaEnum));
			if (plEsInfo->m_bHaveDepth)
			{
				GLOBAL_STRCAT(pBuf, "/");
				GLOBAL_SNPRINTF((pBuf + GLOBAL_STRLEN(pBuf), BufSize - GLOBAL_STRLEN(pBuf), "%d", plEsInfo->m_ChromaDepth));
			}
			GLOBAL_STRCAT(pBuf, "]");

			lRet = TRUE;
		}
	}
	return lRet;
}


void PES_Terminate(void)
{
	PES_Handle *plHandle = s_pHandle;
	if (plHandle)
	{

		plHandle->m_bTaskMark = FALSE;
		PFC_TaskWait(plHandle->m_ThreadHandle, GLOBAL_INVALID_INDEX);

		GLOBAL_SAFEFREE(s_pHandle);
		GLOBAL_TRACE(("%s Complete!\n", __FUNCTION__));
	}
}

void PES_XMLAcess(HANDLE32 XMLLoad, HANDLE32 XMLSave)
{
	PES_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		CHAR_T *plCMDType;
		plCMDType = XML_WarpGetNodeText(XMLLoad, "cmd_type", " ");
		if (plCMDType != NULL)
		{
			if (GLOBAL_STRCMP(plCMDType, "analyse_serv_set") == 0)
			{
				/*设置需要搜索的ES的节目的名称！，当前仅仅分析视频信息*/
				BOOL lRetValue = FALSE;
				U32 lServUNIID, lServIDs, lEsIDs;
				S32 lServTsInd;

				lServUNIID = XML_WarpGetNodeU32(XMLLoad, "serv_uniid", 16, 0);

				//GLOBAL_TRACE(("UNIID = 0x%08X\n", lServUNIID));

				if (lServUNIID != 0)
				{
					lServIDs = MPEG2_DBGetServiceIDsByUniqueID(plHandle->m_DBSHandle, lServUNIID);
					//GLOBAL_TRACE(("ServIDS = 0x%08X\n", lServUNIID));
					if (lServIDs != 0)
					{
						lServTsInd = MPEG2_DBGetServicTsIndex(plHandle->m_DBSHandle, lServIDs, TRUE);
						lEsIDs = MPEG2_DBExGetServFirstVIDEsIDs(plHandle->m_DBSHandle, lServIDs);

						//GLOBAL_TRACE(("EsIDs = 0x%08X\n", lEsIDs));
						if (lEsIDs != 0)
						{
							MPEG2_DBEsInInfo lEsInfo;

							if (MPEG2_DBGetEsInInfo(plHandle->m_DBSHandle, lEsIDs, &lEsInfo) == TRUE)
							{
								lRetValue = PES_StartAnalyse(lServTsInd, lEsInfo.m_EsPID, lEsInfo.m_EsType);
							}
						}
					}
				}

				if (lRetValue == FALSE)
				{
					plHandle->m_CompleteMark = FALSE;
				}


			}
			else if (GLOBAL_STRCMP(plCMDType, "analyse_result_get") == 0)
			{
				CHAR_T plVIDEsInfoBuf[256];
				if (PES_GetDone() == TRUE)
				{
					if (PES_GetVIDEsInfoStr(plVIDEsInfoBuf, sizeof(plVIDEsInfoBuf)) == TRUE)
					{
						XML_WarpAddNodeText(XMLSave, "analyse_status", "ok");
						XML_WarpAddNodeText(XMLSave, "video_es_info", plVIDEsInfoBuf);
					}
					else
					{
						XML_WarpAddNodeText(XMLSave, "analyse_status", "error");
					}
				}
				else
				{
					XML_WarpAddNodeText(XMLSave, "analyse_status", "wait");
				}
			}
		}
	}
}
















/*EOF*/
