/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_hwl_eth.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
void HWL_PlatformInfoSet(HWL_HWInfo * pHWInfo)
{
	S32 i, k, lChnInd;

	/*下面的设备自己的通道信息*/
#ifdef GN1772
	lChnInd = 0;
	pHWInfo->m_pInChn[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInChn[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInChn[lChnInd].m_SubChnNum = 250;
	pHWInfo->m_pInChn[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInChn[lChnInd].m_CurSubSupport = 250;

	lChnInd++;
	pHWInfo->m_pInChn[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInChn[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInChn[lChnInd].m_SubChnNum = 0;
	pHWInfo->m_pInChn[lChnInd].m_StartTsIndex = 256;
	pHWInfo->m_pInChn[lChnInd].m_CurSubSupport = 0;

#ifdef DEBUG_CARD_PLATFORM_CHN_IO_STAT
	for (k = 0; k < 6; k++)
	{
		lChnInd++;
		pHWInfo->m_pInChn[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInChn[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInChn[lChnInd].m_SubChnNum = 16;
		pHWInfo->m_pInChn[lChnInd].m_StartTsIndex = 512 + 256 * (lChnInd - 2);
		pHWInfo->m_pInChn[lChnInd].m_CurSubSupport = 16;
	}
#endif

	pHWInfo->m_InChnNum = lChnInd + 1;

	pHWInfo->m_InTsMax = 1808;

	lChnInd = 0;
	pHWInfo->m_pOutChn[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_DEP;
	pHWInfo->m_pOutChn[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pOutChn[lChnInd].m_SubChnNum = 64;
	pHWInfo->m_pOutChn[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pOutChn[lChnInd].m_CurSubSupport = pHWInfo->m_pOutChn[lChnInd].m_SubChnNum;

	lChnInd++;
	pHWInfo->m_pOutChn[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_DEP;
	pHWInfo->m_pOutChn[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pOutChn[lChnInd].m_SubChnNum = 64;
	pHWInfo->m_pOutChn[lChnInd].m_StartTsIndex = 256;
	pHWInfo->m_pOutChn[lChnInd].m_CurSubSupport = pHWInfo->m_pOutChn[lChnInd].m_SubChnNum;

#ifdef DEBUG_CARD_PLATFORM_CHN_IO_STAT
	for (k = 0; k < 6; k++)
	{
		lChnInd++;
		pHWInfo->m_pOutChn[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pOutChn[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pOutChn[lChnInd].m_SubChnNum = 16;
		pHWInfo->m_pOutChn[lChnInd].m_StartTsIndex = 512 + 256 * (lChnInd - 2);
		pHWInfo->m_pOutChn[lChnInd].m_CurSubSupport = 16;
	}
#endif

	pHWInfo->m_OutChnNum = lChnInd + 1;

	pHWInfo->m_OutTsMax = 1808;

#endif
}

/*EOF*/
