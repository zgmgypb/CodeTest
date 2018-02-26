
/* Includes-------------------------------------------------------------------- */
#include "multi_tsp.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_private.h"
#include "mpeg2.h"

#include "multi_hwl.h"
/* Global Variables (extern)--------------------------------------------------- */

/* Macro (local)--------------------------------------------------------------- */
#define TSP_TASK_STATCK_SIZE					(1024*1024)
#define TSP_TS_FILTER_SLOT_NUMBER				(1)
#define TSP_PSI_FILTER_SLOT_NUMBER				(1)
#define TSP_PSI_SUB_FILTER_SLOT_NUMBER			(8)
#define TSP_PSI_BUFFER_SLOT_NUMBER				(256)
#ifdef ENCODER_CARD_PLATFORM
#define TSP_INSERTER_SLOT_NUMBER				(4096 * 2)
#else
#define TSP_INSERTER_SLOT_NUMBER				4096
#endif
#define TSP_HARDWARE_INSERTER_SLOT_NUMBER		MULT_HARDWARE_INSERTER_SLOT_NUMBER
#define TSP_HARDWARE_INSERTER_SLOT_TS_CAPACITY	32 
#define TSP_LOCK_TIMEOUT_MS						1000
#define TSP_INSERTER_TIMEOUT_MS					100
#define TSP_FILTER_TIMEOUT_MS					500
#define TSP_PID_MAP_NODE_NUMBER					(4096)
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */

typedef struct  
{
	TSP_PIDMapInfo	m_pMapNode[TSP_PID_MAP_NODE_NUMBER];
	S32				m_CurNodeCount;
}TSP_PIDMapArray;

#define TSP_OUT_TS_MAX_NUM						32

typedef struct
{
	//TSP_Param	m_Param;
	HANDLE32		m_TsFilterHandle;
	HANDLE32		m_PSIFilterHandle;
	HANDLE32		m_PSIBufferHandle;
	HANDLE32		m_PSIInserterHandle;

	HANDLE32		m_InserterLockSemaphore;
	HANDLE32		m_InserterTimerTaskHandle;


	HANDLE32		m_FilterLockSemaphore;
	HANDLE32		m_FilterTimerTaskHandle;

	U32				m_pHWTsFilterID[TSP_TS_FILTER_SLOT_NUMBER];
	BOOL8			m_pHWTsInserterID[TSP_HARDWARE_INSERTER_SLOT_NUMBER];

	U8				m_pTsCCReset[TSP_OUT_TS_MAX_NUM][MPEG2_TS_PACKET_MAX_PID_NUM];

	TSP_PIDMapArray	m_PIDMapArray;

	S32				m_CurWorkTsIndex;

// 	S32				m_InternalPacketCount;
// 	S32				m_IntarnalBitrate;
}TSP_Handle;


/* Private Variables (static)-------------------------------------------------- */
static TSP_Handle *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
void TSPL_HWLTsCB(S32 SlotIndex, U8 *pTsPacket)
{
	TSP_Handle *plHandle = (TSP_Handle*)s_pHandle;
	if (plHandle)
	{
		if (GLOBAL_CHECK_INDEX(SlotIndex, TSP_TS_FILTER_SLOT_NUMBER))
		{
			//GLOBAL_TRACE(("DATA Slot = %d---------------------------------\n", SlotIndex));
#if 1
			MPEG2_TsFilterSetTsPacket(plHandle->m_TsFilterHandle, 0, pTsPacket, MPEG2_TS_PACKET_SIZE);
#else
			MPEG2_TsFilterSetTsPacket(plHandle->m_TsFilterHandle, plHandle->m_pHWTsFilterID[SlotIndex], pTsPacket, MPEG2_TS_PACKET_SIZE);
#endif
		}
	}
}


U32 TSPL_DataFilterAdd(void *pUserParam, MPEG2_DataFilter* pFilter)
{
#ifdef GN1846
	return 0;
#else
	U32 lFilterID = 0;
	TSP_Handle *plHandle = (TSP_Handle*)pUserParam;
	if (plHandle)
	{
		S32 i;
		for (i = 0; i < TSP_TS_FILTER_SLOT_NUMBER; i++)
		{
			if (plHandle->m_pHWTsFilterID[i] == 0)
			{
				//GLOBAL_TRACE(("Add Slot = %d, Ts = %d, PID = %d \n", i, plHandle->m_CurWorkTsIndex, pFilter->m_TsFilter.m_Pid));
				HWL_AddTsPacketsRequest(i, plHandle->m_CurWorkTsIndex, pFilter->m_TsFilter.m_Pid, TSPL_HWLTsCB, 0x00);
				lFilterID = i + 1;
				plHandle->m_pHWTsFilterID[i] = pFilter->m_CallerID;
				break;
			}
		}
	}
	return lFilterID;
#endif
}


BOOL TSPL_DataFilterRemove(void *pUserParam, U32 FilterID)
{
#ifdef GN1846
	return TRUE;
#else
	BOOL lRet = FALSE;
	TSP_Handle *plHandle = (TSP_Handle*)pUserParam;
	if (plHandle)
	{
		S32 lSlotIndex;
		lSlotIndex = FilterID -1;
		if (GLOBAL_CHECK_INDEX(lSlotIndex, TSP_TS_FILTER_SLOT_NUMBER))
		{
 			//GLOBAL_TRACE(("Remove Slot = %d~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", lSlotIndex));
			HWL_RemoveTsPacketsRequest(lSlotIndex);
			plHandle->m_pHWTsFilterID[lSlotIndex] = 0;
			lRet = TRUE;
		}
	}
	return lRet;
#endif
}




void TSPL_DataInserterDataOutput(void* pUserParam, S16 TsIndex, U8* pTsPacket)
{
#ifdef GN1846
#else
	TSP_Handle *plHandle = (TSP_Handle*)pUserParam;
	if (plHandle)
	{
		/*校正连续计数器*/
		if (GLOBAL_CHECK_INDEX(TsIndex, TSP_OUT_TS_MAX_NUM))
		{
			MPEG2_TsResetCCSimple(pTsPacket, plHandle->m_pTsCCReset[TsIndex]);
			HWL_SetDirectOutTsPacket(TsIndex, pTsPacket, MPEG2_TS_PACKET_SIZE);
		}
	}
#endif
}

void TSP_InsertTsPacket(S16 TsIndex, U8* pTsPacket)
{
	TSP_Handle* plHandle = s_pHandle;
	if ( plHandle )
	{
		/*校正连续计数器*/
		if (GLOBAL_CHECK_INDEX(TsIndex, TSP_OUT_TS_MAX_NUM))
		{
			MPEG2_TsResetCCSimple( pTsPacket, plHandle->m_pTsCCReset[ TsIndex ] );
			HWL_SetDirectOutTsPacket(TsIndex, pTsPacket, MPEG2_TS_PACKET_SIZE);
		}
	}
}

/* 插入器回调接口 */
U32 TSPL_DataInserterAdd(HANDLE32 Handle, S16 TsIndex, U8 *pData, S32 DataSize, S32 Interval)
{
#ifdef GN1846
	return 0;
#else
	U32 lInserterID = 0;
	TSP_Handle *plHandle = (TSP_Handle*)Handle;
	if (plHandle)
	{
		if (DataSize >= MPEG2_TS_PACKET_SIZE)
		{
			S32 i;
			for (i = 0; i < TSP_HARDWARE_INSERTER_SLOT_NUMBER; i++)
			{
				if (plHandle->m_pHWTsInserterID[i] == 0)
				{
					break;
				}
			}

			if (i != TSP_HARDWARE_INSERTER_SLOT_NUMBER)
			{

 				//GLOBAL_TRACE(("Set HW Inserter Slot %d, Interval = %d, PID = %d, TsIndex = %d\n", i, Interval, MPEG2_TsGetPID(pData), TsIndex));

				if (Interval <= 0)
				{
					GLOBAL_TRACE(("Error!!! Interval = %d\n", Interval));
					Interval = 1;
				}

				if (Interval >= 1000)
				{
					GLOBAL_TRACE(("Error!!! Interval = %d\n", Interval));
					Interval = 999;
				}

				if (DataSize >= MPEG2_TS_PACKET_SIZE)
				{
						//GLOBAL_TRACE(("Add HW TsInserter Slot = %d, Ts = %d, PID = %d\n", i, TsIndex, MPEG2_TsGetPID(pData)));
#ifdef GM2730S
					if (MULT_GetOutChannelState(TsIndex))
					{
						HWL_HWInserterSet(i, TsIndex, pData, DataSize, Interval);;
					}
#else
					HWL_HWInserterSet(i, TsIndex, pData, DataSize, Interval);;
#endif


				}
				else
				{
					GLOBAL_TRACE(("Error!!! DataSize = %d\n", DataSize));
				}


				/*HD TS插入器添加支持*/
				lInserterID = i + 1;
				plHandle->m_pHWTsInserterID[i] = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("No More HD Inserters\n"));
			}
		}
		else
		{
			GLOBAL_TRACE(("Data To Small = %d\n", DataSize));
		}

	}
	return lInserterID;
#endif
}


BOOL TSPL_DataInserterRemove(HANDLE32 Handle, U32 InserterID)
{
#ifdef GN1846
	return TRUE;
#else
	BOOL lRet = FALSE;
	TSP_Handle *plHandle = (TSP_Handle*)Handle;
	if (plHandle)
	{
		S32 lSlotIndex = InserterID - 1;
		if (GLOBAL_CHECK_INDEX(lSlotIndex, TSP_HARDWARE_INSERTER_SLOT_NUMBER))
		{
			if (plHandle->m_pHWTsInserterID[lSlotIndex] == TRUE)
			{
				/*HD TS插入器删除支持*/
 				//GLOBAL_TRACE(("Clear HW Inserter Slot %d\n", lSlotIndex));

				HWL_HWInserterClear(lSlotIndex);

				plHandle->m_pHWTsInserterID[lSlotIndex] = FALSE;
				lRet = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("SlotIndex [%d] by InserterID[%d] Status Error!\n", lSlotIndex, InserterID));
			}
		}
		else
		{
			GLOBAL_TRACE(("SlotIndex [%d] by InserterID[%d] Error!\n", lSlotIndex, InserterID));
		}
	}
	return lRet;
#endif
}


U32 TSPL_InserterTaskFn(void *pUserParam, S32 Duration)
{
#ifdef GN1846
	return 0;
#else
	TSP_Handle *plHandle = (TSP_Handle*) pUserParam;
	if (plHandle)
	{
		if (PFC_SemaphoreWait(plHandle->m_InserterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			MPEG2_TSIAccess(plHandle->m_PSIInserterHandle, Duration);
			PFC_SemaphoreSignal(plHandle->m_InserterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	return 0;
#endif
}


U32 TSPL_FilterTimerTaskFn(void *pUserParam, S32 Duration)
{
#ifdef GN1846
	return 0;
#else
	TSP_Handle *plHandle = (TSP_Handle*) pUserParam;
	if (plHandle)
	{
		if (PFC_SemaphoreWait(plHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			MPEG2_PSIBufferAccess(plHandle->m_PSIBufferHandle, Duration);
			PFC_SemaphoreSignal(plHandle->m_FilterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	return 0;
#endif
}

/* TSP模块入口 */

BOOL TSP_Initiate(void)
{
	BOOL lRet = FALSE;
	if (s_pHandle == NULL)
	{
		TSP_Handle *plHandle = (TSP_Handle*)GLOBAL_ZMALLOC(sizeof(TSP_Handle));
		if (plHandle)
		{
			{
				MPEG2_TsFilterInitParam lTsFilterInitParameter;
				MPEG2_PSIFilterInitParam lPSIFilterInitParameter;
				MPEG2_PSIBufferInitParam lPSIBufferInitParameter;

				lTsFilterInitParameter.m_TsSlotNum = 0;
#ifdef ENCODER_CARD_PLATFORM
				lTsFilterInitParameter.m_PESSlotNum = 1;
#else
				lTsFilterInitParameter.m_PESSlotNum = 0;
#endif
				lTsFilterInitParameter.m_PESBufSize = 2 * 1024 * 1024;
				lTsFilterInitParameter.m_SecSlotNum = TSP_TS_FILTER_SLOT_NUMBER;
				lTsFilterInitParameter.m_pRequestCB = TSPL_DataFilterAdd;
				lTsFilterInitParameter.m_pCancelCB = TSPL_DataFilterRemove;
				lTsFilterInitParameter.m_pDataArriveCB = NULL;//表示在申请时采用内部回调函数填充对应指针
				lTsFilterInitParameter.m_pUserParam = plHandle;
				plHandle->m_TsFilterHandle = MPEG2_TsFilterCreate(&lTsFilterInitParameter);

				lPSIFilterInitParameter.m_SlotNum = TSP_PSI_FILTER_SLOT_NUMBER;
				lPSIFilterInitParameter.m_SubFilterNum = TSP_PSI_SUB_FILTER_SLOT_NUMBER;
				lPSIFilterInitParameter.m_pRequestCB = MPEG2_TsFilterSectionRequest;
				lPSIFilterInitParameter.m_pCancelCB = MPEG2_TsFilterSectionCancel;
				lPSIFilterInitParameter.m_pDataArriveCB = NULL;//表示在申请时采用内部回调函数填充对应指针
				lPSIFilterInitParameter.m_pUserParam = plHandle->m_TsFilterHandle;
				plHandle->m_PSIFilterHandle = MPEG2_PSIFilterCreate(&lPSIFilterInitParameter);


				lPSIBufferInitParameter.m_SlotNum = TSP_PSI_BUFFER_SLOT_NUMBER;
				lPSIBufferInitParameter.m_pRequestCB = MPEG2_PSIFilterRequest;
				lPSIBufferInitParameter.m_pCancelCB = MPEG2_PSIFilterCancel;
				lPSIBufferInitParameter.m_pDataArriveCB = NULL;//表示在申请时采用内部回调函数填充对应指针
				lPSIBufferInitParameter.m_pUserParam = plHandle->m_PSIFilterHandle;
				plHandle->m_PSIBufferHandle = MPEG2_PSIBufferCreate(&lPSIBufferInitParameter);

				plHandle->m_FilterLockSemaphore = PFC_SemaphoreCreate("TSP Filter Lock", 1);
				PFC_SemaphoreSignal(plHandle->m_FilterLockSemaphore);

				/*打开过滤器定时器任务*/
				plHandle->m_FilterTimerTaskHandle = PAL_TIMThreadCreate("TSP Filter Task", TSP_TASK_STATCK_SIZE, TSP_FILTER_TIMEOUT_MS, TSPL_FilterTimerTaskFn, plHandle, FALSE);
			}


			{
				MPEG2_TSIInitParam	lInserterInitParameter;

				lInserterInitParameter.m_HardwareSlotNum = TSP_HARDWARE_INSERTER_SLOT_NUMBER;
				lInserterInitParameter.m_HardwareSlotMaxDataSize =  MPEG2_TS_PACKET_SIZE/*MPEG2_TS_PACKET_SIZE * 32*/;
				lInserterInitParameter.m_HardwareSlotMaxInterval =  1000;
				lInserterInitParameter.m_TotalSlotNum = TSP_INSERTER_SLOT_NUMBER;
				lInserterInitParameter.m_pDataOutputCB = TSPL_DataInserterDataOutput;
				lInserterInitParameter.m_DataOutputHandle = plHandle;
				lInserterInitParameter.m_pHWInserterAddCB = TSPL_DataInserterAdd;
				lInserterInitParameter.m_pHWInserterRemoveCB = TSPL_DataInserterRemove;
				lInserterInitParameter.m_HWInserterHandle = plHandle;

				plHandle->m_PSIInserterHandle = MPEG2_TSICreate(&lInserterInitParameter);

				plHandle->m_InserterLockSemaphore = PFC_SemaphoreCreate("TSP Inserter Lock", 1);
				PFC_SemaphoreSignal(plHandle->m_InserterLockSemaphore);

				/*开启插入器输出用定时器*/
				plHandle->m_InserterTimerTaskHandle =  PAL_TIMThreadCreate("TSP Inserter Task", TSP_TASK_STATCK_SIZE, TSP_INSERTER_TIMEOUT_MS, TSPL_InserterTaskFn, plHandle, TRUE);
			}
#ifdef INSERTER_PID_TEST
			GLOBAL_ZEROMEM(s_PIDCount, sizeof(s_PIDCount));
#endif
			GLOBAL_TRACE(("Initiate Complete!\n"));

			lRet = TRUE;
		}
		s_pHandle = plHandle;
	}
	return lRet;
}


void TSP_SetTsPacket(S32 SlotIndex, U8* pData)
{
	if (s_pHandle)
	{
		U32 lFilterID;
		if (GLOBAL_CHECK_INDEX(SlotIndex, TSP_TS_FILTER_SLOT_NUMBER))
		{
			lFilterID = s_pHandle->m_pHWTsFilterID[SlotIndex];
			if (PFC_SemaphoreWait(s_pHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
			{
				MPEG2_TsFilterSetTsPacket(s_pHandle->m_TsFilterHandle, lFilterID, pData, MPEG2_TS_PACKET_SIZE);
				PFC_SemaphoreSignal(s_pHandle->m_FilterLockSemaphore);
			}
			else
			{
				GLOBAL_TRACE(("Semaphore Timeout\n"));
			}
		}
		else
		{
			GLOBAL_TRACE(("SlotIndex[%d] OverRange[%d]\n", SlotIndex, TSP_TS_FILTER_SLOT_NUMBER));
		}
	}
}


void TSP_SetCurrentWorkTsIndex(S32 TsIndex)
{
	if (s_pHandle)
	{
		s_pHandle->m_CurWorkTsIndex = TsIndex;
	}
}

U32 TSP_PESRequeset(U32 EsPID, MPEG2_DataFilterArriveCB pCB, void *pUserParam, BOOL bBlock)
{
	U32 lFilterID = 0;
	MPEG2_DataFilter lDataFilter;
	GLOBAL_ZEROMEM(&lDataFilter, sizeof(lDataFilter));
	lDataFilter.m_pDataArriveCB = pCB;
	lDataFilter.m_pUserParam = pUserParam;
	lDataFilter.m_TimeOut = 0;
	lDataFilter.m_TsFilter.m_Pid = EsPID;
	lDataFilter.m_TsFilter.m_FilterDepth = 0;

	if (bBlock)
	{
		if (PFC_SemaphoreWait(s_pHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			lFilterID = MPEG2_TsFilterPESRequest(s_pHandle->m_TsFilterHandle, &lDataFilter);
			PFC_SemaphoreSignal(s_pHandle->m_FilterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	else
	{
		lFilterID = MPEG2_TsFilterPESRequest(s_pHandle->m_TsFilterHandle, &lDataFilter);
	}
	return lFilterID;
}


BOOL TSP_PESRequesetCancel(U32 FilterID, BOOL bBlock)
{
	BOOL lRet = FALSE;
	if (s_pHandle)
	{
		if (bBlock)
		{
			if (PFC_SemaphoreWait(s_pHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
			{
				lRet = MPEG2_TsFilterPESCancel(s_pHandle->m_TsFilterHandle, FilterID);
				PFC_SemaphoreSignal(s_pHandle->m_FilterLockSemaphore);
			}
			else
			{
				GLOBAL_TRACE(("Semaphore Timeout\n"));
			}
		}
		else
		{
			lRet = MPEG2_TsFilterPESCancel(s_pHandle->m_TsFilterHandle, FilterID);
		}
	}
	return lRet;
}

U32 TSP_SectionRequestAdd(MPEG2_DataFilter* pFilter)
{
	U32 lFilterID = 0;
	if (s_pHandle)
	{
		if (PFC_SemaphoreWait(s_pHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			lFilterID = MPEG2_PSIBufferRequest(s_pHandle->m_PSIBufferHandle, pFilter);
			PFC_SemaphoreSignal(s_pHandle->m_FilterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	return lFilterID;
}


BOOL TSP_SectionRequestRemove( U32 FilterID)
{
	BOOL lRet = FALSE;
	if (s_pHandle)
	{
		if (PFC_SemaphoreWait(s_pHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			lRet = MPEG2_PSIBufferCancel(s_pHandle->m_PSIBufferHandle, FilterID);
			PFC_SemaphoreSignal(s_pHandle->m_FilterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	return lRet;
}


BOOL TSP_PSIBufferGetArrayPtr(U32 FilterID, MPEG2_SectionArray *pArray)
{
	BOOL lRet = FALSE;
	if (s_pHandle)
	{
		if (PFC_SemaphoreWait(s_pHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			lRet = MPEG2_PSIBufferGetArrayPtr(s_pHandle->m_PSIBufferHandle, FilterID, pArray);
			PFC_SemaphoreSignal(s_pHandle->m_FilterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	return lRet;

}

/*插入器接口*/
U32 TSP_PSIInserterAdd(S16 TsIndex, U8 *pData, S32 DataSize, S32 Interval)
{
	U32 lInserterID = 0;
	if (s_pHandle)
	{
#ifdef MULT_TS_BACKUP_SUPPORT
		if (MULT_BPGetTsIsUsedByBackup(TsIndex, FALSE))
		{
			/*设置为备用的TS不输出PID映射信息*/
			GLOBAL_TRACE(("Output PSI SI TsInd = %d, OutPID = %d, Blocked By Ts Backup System\n", TsIndex, MPEG2_TsGetPID(pData)));
			return 1;
		}

		if (PFC_SemaphoreWait(s_pHandle->m_InserterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			lInserterID = MPEG2_TSIAddInserter(s_pHandle->m_PSIInserterHandle, MULT_BPGetTsBackupTs(TsIndex, FALSE), pData, DataSize, Interval);
			PFC_SemaphoreSignal(s_pHandle->m_InserterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
#else
		if (PFC_SemaphoreWait(s_pHandle->m_InserterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			lInserterID = MPEG2_TSIAddInserter(s_pHandle->m_PSIInserterHandle, TsIndex, pData, DataSize, Interval);
			PFC_SemaphoreSignal(s_pHandle->m_InserterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
#endif
	}
	return lInserterID;
}


BOOL TSP_PSIInserterRemove(U32 InserterID)
{
	BOOL lRet = FALSE;
	if (s_pHandle)
	{
		if (PFC_SemaphoreWait(s_pHandle->m_InserterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			lRet = MPEG2_TSIRemoveInserter(s_pHandle->m_PSIInserterHandle, InserterID);
			PFC_SemaphoreSignal(s_pHandle->m_InserterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	return lRet;
}


void TSP_PSIInserterClear(void)
{
	BOOL lRet = FALSE;
	if (s_pHandle)
	{
		if (PFC_SemaphoreWait(s_pHandle->m_InserterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			lRet = MPEG2_TSIClearInserter(s_pHandle->m_PSIInserterHandle);
			PFC_SemaphoreSignal(s_pHandle->m_InserterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}

		HWL_HWInserterClear(0xFFFF);//特殊命令，全清硬件插入器（确保即使上面没有完全清除）
		PFC_TaskSleep(10);//等待清空！
	}
}


//void TSP_ApplyHWInserter(void)
//{
//	if (s_pHandle)
//	{
//		if (PFC_SemaphoreWait(s_pHandle->m_InserterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
//		{
//			MPEG2_TSIApplyHWInserter(s_pHandle->m_PSIInserterHandle);
//			PFC_SemaphoreSignal(s_pHandle->m_InserterLockSemaphore);
//		}
//		else
//		{
//			GLOBAL_TRACE(("Semaphore Timeout\n"));
//		}
//	}
//}


/*PID映射接口*/
void TSP_AddPIDMap(S16 InTsIndex, U16 InPID, S16 OutTsIndex, U16 OutPID, BOOL bScramble, S32 CwGroupIndex)
{
	GLOBAL_ASSERT(s_pHandle);
	if (s_pHandle->m_PIDMapArray.m_CurNodeCount < TSP_PID_MAP_NODE_NUMBER)
	{
		TSP_PIDMapInfo *plNode;
#ifdef GN2000
		if (InTsIndex == 16)
		{
			if (InPID != 888 && InPID != 889)
			{
				return;
			}
		}
#endif

#ifdef MULT_TS_BACKUP_SUPPORT
		if (MULT_BPGetTsIsUsedByBackup(InTsIndex, TRUE) || MULT_BPGetTsIsUsedByBackup(OutTsIndex, FALSE))
		{
			/*设置为备用的TS不输出PID映射信息*/
			GLOBAL_TRACE(("PID Map InTsInd %d, InPID %d, OutTsInd = %d, OutPID = %d, Blocked By Ts Backup System\n", InTsIndex, InPID, OutTsIndex, OutPID));
			return;
		}
#endif
		if (InPID != 8191)
		{
			plNode = &s_pHandle->m_PIDMapArray.m_pMapNode[s_pHandle->m_PIDMapArray.m_CurNodeCount];

			plNode->m_InPID = InPID;
#ifdef MULT_TS_BACKUP_SUPPORT
			if (plNode->m_InTsIndex != MULT_TSP_INTERNAL_PID_MAP_SRC)//回避EMM插入映射
			{
				plNode->m_InTsIndex = MULT_BPGetTsBackupTs(InTsIndex, TRUE);
			}
#else
			plNode->m_InTsIndex = InTsIndex;
#endif
			plNode->m_OutPID = OutPID;
#ifdef MULT_TS_BACKUP_SUPPORT
			plNode->m_OutTsIndex = MULT_BPGetTsBackupTs(OutTsIndex, FALSE);
#else
			plNode->m_OutTsIndex = OutTsIndex;
#endif
			plNode->m_bScramble = bScramble;
			plNode->m_CwGroupIndex = CwGroupIndex;
			s_pHandle->m_PIDMapArray.m_CurNodeCount++;
			//GLOBAL_TRACE(("Map From Ts[%d] PID[%.4d] To Ts[%d] PID[%.4d]  bScramble = %d, GroupIndex = %d\n", plNode->m_InTsIndex, plNode->m_InPID, plNode->m_OutTsIndex, plNode->m_OutPID, plNode->m_bScramble, plNode->m_CwGroupIndex));
		}
	}
	else
	{
		GLOBAL_TRACE(("Too Many PIDs In Map[%d]\n", TSP_PID_MAP_NODE_NUMBER));
	}
}

void TSP_ClearPIDMap(void)
{
#if 0//手动清除命令！
	S32 i;
	TSP_PIDMapInfo *plNode;
	GLOBAL_TRACE(("Last PID Map Count = %d\n", s_pHandle->m_PIDMapArray.m_CurNodeCount));
	for (i = 0; i < s_pHandle->m_PIDMapArray.m_CurNodeCount; i++)
	{
		plNode = &s_pHandle->m_PIDMapArray.m_pMapNode[i];
		GLOBAL_TRACE(("Clean From Ts[%d] PID[%.4d] To Ts[%d] PID[%.4d]  bScramble = %d, GroupIndex = %d\n", plNode->m_InTsIndex, plNode->m_InPID, plNode->m_OutTsIndex, plNode->m_OutPID, plNode->m_bScramble, plNode->m_CwGroupIndex));
		HWL_AddPIDMap(plNode->m_InTsIndex, plNode->m_InPID, plNode->m_OutTsIndex, MPEG2_TS_PACKET_NULL_PID, FALSE, 0);
	}
#endif
	GLOBAL_ASSERT(s_pHandle);
	s_pHandle->m_PIDMapArray.m_CurNodeCount = 0;
}

void TSP_ApplyPIDMap(void)
{
#ifdef GN1846
#else
	S32 i;
	U8 lCwGroupMark;
	TSP_PIDMapInfo *plNode;
	GLOBAL_ASSERT(s_pHandle);
	/*清除HWL模块中的映射表*/
	HWL_ClearPIDMap();
	HWL_AddPIDMap(0, MPEG2_TS_PACKET_NULL_PID, 0, MPEG2_TS_PACKET_NULL_PID, FALSE, 0x80);
#ifdef DEBUG_SHOW_PIDMAP
	GLOBAL_TRACE(("Total PID Map Num = %d\n", s_pHandle->m_PIDMapArray.m_CurNodeCount));
#endif
	for (i = 0; i < s_pHandle->m_PIDMapArray.m_CurNodeCount; i++)
	{
		plNode = &s_pHandle->m_PIDMapArray.m_pMapNode[i];
#ifdef DEBUG_SHOW_PIDMAP
		GLOBAL_TRACE(("Map From Ts[%d] PID[%.4d] To Ts[%d] PID[%.4d]  Scr = %d, GIndex = %d\n", plNode->m_InTsIndex, plNode->m_InPID, plNode->m_OutTsIndex, plNode->m_OutPID, plNode->m_bScramble, plNode->m_CwGroupIndex));
#endif
 		HWL_AddPIDMap(plNode->m_InTsIndex, plNode->m_InPID, plNode->m_OutTsIndex, plNode->m_OutPID, plNode->m_bScramble, plNode->m_CwGroupIndex);
	}

	/*清除FPGA全部映射，并将当前HWL模块中的新的映射表排序后应用给FPGA*/
	HWL_PerformPIDMap();
#endif
}

S32 TSP_GetPIDMapCount(void)
{
	GLOBAL_ASSERT(s_pHandle);
	return s_pHandle->m_PIDMapArray.m_CurNodeCount;
}

BOOL TSP_GetPIDMapInfo(S32 Index, TSP_PIDMapInfo *plInfo)
{
	TSP_PIDMapInfo *plNode;
	BOOL lRet = FALSE;
	GLOBAL_ASSERT(s_pHandle);
	GLOBAL_ASSERT(plInfo);
	if (GLOBAL_CHECK_INDEX(Index , s_pHandle->m_PIDMapArray.m_CurNodeCount))
	{
		plNode = &s_pHandle->m_PIDMapArray.m_pMapNode[Index];
		GLOBAL_MEMCPY(plInfo, plNode, sizeof(TSP_PIDMapInfo));
	}
	return lRet;
}

void TSP_Access(S32 Duration)
{
	if (s_pHandle)
	{
// 		s_pHandle->m_IntarnalBitrate = s_pHandle->m_InternalPacketCount * (MPEG2_TS_PACKET_SIZE * 8 * 1000 / Duration);
 		//GLOBAL_TRACE(("Packet Count = %d, Duration = %d, Bitrate = %d\n", s_pHandle->m_InternalPacketCount, Duration, s_pHandle->m_IntarnalBitrate));
// 		s_pHandle->m_InternalPacketCount = 0;
	}
}


/*外部可以访问的TS过滤器*/
U32 TSP_TsFilterAdd(U16 CallerID, S32 TsInd, U16 PID, HWL_TsFilterCB pDataCB, S32 Count)
{
	U32 lFilterID = 0;
	TSP_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		if (PFC_SemaphoreWait(plHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			S32 i;
			for (i = 0; i < TSP_TS_FILTER_SLOT_NUMBER; i++)
			{
				if (plHandle->m_pHWTsFilterID[i] == 0)
				{
					GLOBAL_TRACE(("Add Slot = %d, Ts = %d, PID = %d \n", i, TsInd, PID));
					HWL_AddTsPacketsRequest(i, TsInd, PID, pDataCB, Count);
					plHandle->m_pHWTsFilterID[i] = CallerID;
					lFilterID = i + 1;
					break;
				}
			}
			PFC_SemaphoreSignal(plHandle->m_FilterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	return lFilterID;
}

BOOL TSP_TsFilterRemove(U32 FilterID)
{
	BOOL lRet = FALSE;
	TSP_Handle *plHandle = s_pHandle;
	if (plHandle)
	{
		S32 lSlotIndex;
		lSlotIndex = FilterID -1;
		if (PFC_SemaphoreWait(plHandle->m_FilterLockSemaphore, TSP_LOCK_TIMEOUT_MS))
		{
			if (GLOBAL_CHECK_INDEX(lSlotIndex, TSP_TS_FILTER_SLOT_NUMBER))
			{
				//GLOBAL_TRACE(("Remove Slot = %d\n", lSlotIndex));
				HWL_RemoveTsPacketsRequest(lSlotIndex);
				plHandle->m_pHWTsFilterID[lSlotIndex] = 0;
				lRet = TRUE;
			}
			PFC_SemaphoreSignal(plHandle->m_FilterLockSemaphore);
		}
		else
		{
			GLOBAL_TRACE(("Semaphore Timeout\n"));
		}
	}
	return lRet;
}



void TSP_Terminate(void)
{
	if (s_pHandle)
	{
		if (s_pHandle->m_FilterTimerTaskHandle)
		{
			PAL_TIMThreadDestroy(s_pHandle->m_FilterTimerTaskHandle);
			s_pHandle->m_FilterTimerTaskHandle = NULL;
		}

		if (s_pHandle->m_InserterTimerTaskHandle)
		{
			PAL_TIMThreadDestroy(s_pHandle->m_InserterTimerTaskHandle);
			s_pHandle->m_InserterTimerTaskHandle = NULL;
		}

		PFC_SemaphoreDestroy(s_pHandle->m_FilterLockSemaphore);
		PFC_SemaphoreDestroy(s_pHandle->m_InserterLockSemaphore);

		MPEG2_TSIDestroy(s_pHandle->m_PSIInserterHandle);
		MPEG2_PSIBufferDestroy(s_pHandle->m_PSIBufferHandle);
		MPEG2_PSIFilterDestroy(s_pHandle->m_PSIFilterHandle);
		MPEG2_TsFilterDestroy(s_pHandle->m_TsFilterHandle);


		GLOBAL_FREE(s_pHandle);
		s_pHandle = NULL;

		GLOBAL_TRACE(("Terminate Complete!\n"));
	}
}

//void TSP_PreloadDataToMem(void)
//{
//	S32 i;
//	U32 lTimeTick;
//	S32 lDuration;
//	U8 *plTmpBuf, plTsData[MPEG2_TS_PACKET_SIZE];
//	plTmpBuf = plTsData;
//	plTmpBuf[0] = 0x47;
//	plTmpBuf[1] = (8191 >> 8) & 0x1F;
//	plTmpBuf[2] = 8191 & 0xFF;
//	plTmpBuf[3] = 0x10 | (i & 0x0F);
//
//	lTimeTick = PFC_GetTickCount();
//	
//	for (i = 0; i < 27000; i++)
//	{
//		HWL_SetDirectOutTsPacket(0, plTsData, MPEG2_TS_PACKET_SIZE);
//	}
//
//	lDuration = CAL_TimeDurationMS(&lTimeTick, PFC_GetTickCount(), 1);
//
//	GLOBAL_TRACE(("Load Use Time = %d ms\n", lDuration));
//
//}
