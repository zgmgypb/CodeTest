#ifndef _MUL_TSP_H_
#define _MUL_TSP_H_

/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
#include "multi_hwl.h"
#include "mpeg2.h"
/* Macro ---------------------------------------------------------------------- */
#define MULT_TSP_INTERNAL_PID_MAP_SRC	(0xFE)
/* Types Define (struct typedef)----------------------------------------------- */

typedef void (*HD_PIDRequestAddCB)(S32 SlotIndex, S32 TsIndex, U16 PID);
typedef void (*HD_PIDRequestRemoveCB)(S32 SlotIndex);

typedef void (*HD_ISRDataOutCB)(S32 TsIndex, U8 *pTsPacket);
typedef void (*HD_ISRRequestAddCB)(S32 SlotIndex, S32 TsIndex, U8 *pData, S32 DataSize, S32 Interval);
typedef void (*HD_ISRRequestRemoveCB)(U32 SlotIndex);


typedef struct  
{
	S16		m_InTsIndex;
	S16		m_OutTsIndex;
	U16		m_InPID;
	U16		m_OutPID;
	BOOL	m_bScramble;
	S32		m_CwGroupIndex;
}TSP_PIDMapInfo;


/* Functions prototypes ------------------------------------------------------- */
BOOL TSP_Initiate(void);
void TSP_SetTsPacket(S32 SlotIndex, U8* pData);
void TSP_SetCurrentWorkTsIndex(S32 TsIndex);
U32 TSP_PESRequeset(U32 EsPID, MPEG2_DataFilterArriveCB pCB, void *pUserParam, BOOL bBlock);
BOOL TSP_PESRequesetCancel(U32 FilterID, BOOL bBlock);
U32 TSP_SectionRequestAdd(MPEG2_DataFilter* pFilter);
BOOL TSP_SectionRequestRemove( U32 FilterID);
BOOL TSP_PSIBufferGetArrayPtr( U32 FilterID, MPEG2_SectionArray *pArray);

U32 TSP_PSIInserterAdd(S16 TsIndex, U8 *pData, S32 DataSize, S32 Interval);
BOOL TSP_PSIInserterRemove(U32 InserterID);
void TSP_PSIInserterClear(void);//强制清除所有内容，同时清理硬件插入器
//void TSP_ApplyHWInserter(void);

void TSP_AddPIDMap(S16 InTsIndex, U16 InPID, S16 OutTsIndex, U16 OutPID, BOOL bScramble, S32 CwGroupIndex);
void TSP_ClearPIDMap(void);//强制清除所有内容，同时清理硬件复用器
void TSP_ApplyPIDMap(void);
S32 TSP_GetPIDMapCount(void);
BOOL TSP_GetPIDMapInfo(S32 Index, TSP_PIDMapInfo *plInfo);
// S32 TSP_GetInternalBitrate(void);
void TSP_Access(S32 Duration);
void TSP_Terminate(void);
void TSP_InsertTsPacket(S16 TsIndex, U8* pTsPacket);



U32 TSP_TsFilterAdd(U16 CallerID, S32 TsInd, U16 PID, HWL_TsFilterCB pDataCB, S32 Count);
BOOL TSP_TsFilterRemove(U32 FilterID);

#endif
