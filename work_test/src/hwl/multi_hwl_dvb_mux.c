/*
 插卡式Route模块，注意用输入选择输出
*/
/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_hwl_igmp.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "multi_hwl_monitor.h"
#include "multi_hwl_tags.h"
#include "mpeg2_micro.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
//#define PROTOCOL_DEBUG
#define HWL_DVB_MUX_ROUTE_MAX_NODE_NUM_ONE_CHN				(256)
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	S32				m_OutTsInd;
	BOOL			m_bRoute;
	BOOL			m_bRouteWithNullPacket;
	BOOL			m_EnablePCRCorrection;
}HWL_DVB_MUX_ROUTE_NODE;


/* Private Variables (static)-------------------------------------------------- */
static HWL_DVB_MUX_ROUTE_NODE s_pRouteNodeArray[HWL_DVB_MUX_ROUTE_MAX_NODE_NUM_ONE_CHN];
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
void HWL_DVBMuxRouteSet(S32 InTsInd, S32 OutTsInd, BOOL bRoute, BOOL bRouteWithNullPacket, BOOL bEnablePCRCorrection)
{
	if (InTsInd < HWL_DVB_MUX_ROUTE_MAX_NODE_NUM_ONE_CHN)
	{
		HWL_DVB_MUX_ROUTE_NODE *plNode = &s_pRouteNodeArray[InTsInd];

#ifdef PROTOCOL_DEBUG
		GLOBAL_TRACE(("In Ts = %d, Out Ts = %d, bRoute = %d, bNull = %d, bPCR = %d\n", InTsInd, OutTsInd, bRoute, bRouteWithNullPacket, bEnablePCRCorrection));
#endif
		plNode->m_OutTsInd = OutTsInd;
		plNode->m_bRoute = bRoute;
		plNode->m_bRouteWithNullPacket = bRouteWithNullPacket;
		plNode->m_EnablePCRCorrection = bEnablePCRCorrection;

	}
	else
	{
		GLOBAL_TRACE(("Route Number Over Limits = %d\n", HWL_DVB_MUX_ROUTE_MAX_NODE_NUM_ONE_CHN));
	}

}

void HWL_DVBMuxRouteApply(S32 InCHN, S32 MaxSubNum)
{
	S32 i, lLen;
	U16 lTmpValue;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	HWL_DVB_MUX_ROUTE_NODE *plNode;

	lLen = 0;
	plTmpBuf = plCMDBuf;
	GLOBAL_MSB8_EC(plTmpBuf, HWL_DIRECT_ROUTE_MODE_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, (MaxSubNum / 2), lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, InCHN, lLen);

#ifdef PROTOCOL_DEBUG
	GLOBAL_TRACE(("CHN  = %d\n", InCHN));
#endif

	for (i = 0; i < MaxSubNum; i++)
	{
		plNode = &s_pRouteNodeArray[i];
		lTmpValue = 0;
		if (plNode->m_bRoute)
		{
			lTmpValue |=  (plNode->m_OutTsInd & 0x0FFF);
			lTmpValue |=  (((plNode->m_bRouteWithNullPacket?1:0) << 14) & (1 << 14));
			lTmpValue |=  (((plNode->m_EnablePCRCorrection?0:1) << 15) & (1 << 15));
		}
		else
		{
			lTmpValue = 0xFFFF;
		}
		GLOBAL_MSB16_EC(plTmpBuf, lTmpValue, lLen);
	}


#ifdef PROTOCOL_DEBUG
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);

	/*清空设置内容*/
	GLOBAL_ZEROSTRUCT(s_pRouteNodeArray);
}



/*EOF*/
