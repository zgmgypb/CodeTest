/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_hwl_igmp.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define HWL_IGMP_ARRAY_SIZE				(500)
#define HWL_IGMP_MAX_MEMBER_IN_FRAME	125
#define HWL_IGMP_TAG					0x1E	
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	S32	m_ChnIndex;
	U32 m_DesAddr;
	U32	m_SrcAddr;
	U16	m_SrcPort;
}HWL_IGMPNode;

typedef struct  
{
	HWL_IGMPNode	m_pNode[HWL_IGMP_ARRAY_SIZE];
	S32				m_NodeNum;
	S16				m_RepeatTime;/*秒，10~100？<10则关闭*/
	S16				m_IGMPVersion;
}HWL_IGMPArray;
/* Private Variables (static)-------------------------------------------------- */
static HWL_IGMPArray s_pIGMPArray;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
void HWLL_IGMPPrintArray(CHAR_T *pTitle, HWL_IGMPArray *pArray)
{
	S32 i;
	HWL_IGMPNode *plNode;
	GLOBAL_TRACE(("------ %s Size = %d ---------------------------\n", pTitle, pArray->m_NodeNum));
	for (i = 0; i < pArray->m_NodeNum; i++)
	{
		plNode = &pArray->m_pNode[i];
		GLOBAL_TRACE(("DES Addr = 0x%.8X, SRC Addr = 0x%.8X\n", plNode->m_DesAddr, plNode->m_SrcAddr));
	}
	GLOBAL_TRACE(("------------------------------------------------\n", pArray->m_NodeNum));
}
/* API Functions ------------------------------------------------------------------ */
void HWL_IGMPSetting(BOOL bRepeateMark, S16 RepeateTime, S16 IGMPVersion)
{
	HWL_IGMPArray *plArray;

	plArray = &s_pIGMPArray;
	if (RepeateTime > 255)
	{
		RepeateTime = 255;
	}

	if (RepeateTime < 20)
	{
		RepeateTime = 0;
	}

	if (bRepeateMark == FALSE)
	{
		RepeateTime = 0;
	}

	if ((IGMPVersion) < 2 || (IGMPVersion > 3))
	{
		IGMPVersion = 2;
	}
	plArray->m_RepeatTime = RepeateTime;
	plArray->m_IGMPVersion = IGMPVersion;
}

void HWL_IGMPTAdd(S32 ChnIndex, U32 IPAddr, U32 SrcAddr, U16 SrcPort)
{
	S32 i;
	HWL_IGMPArray *plArray;
	HWL_IGMPNode *plIGMPNode;

	if (GLOBAL_IN_MUTICAST(IPAddr))//必须要是组播才添加！
	{
		plArray = &s_pIGMPArray;
		for (i = 0; i < plArray->m_NodeNum; i++)
		{
			plIGMPNode = &plArray->m_pNode[i];
			if ((plIGMPNode->m_ChnIndex == ChnIndex) && (plIGMPNode->m_DesAddr == IPAddr))
			{
				break;//排除重复项目，源地址限定部分暂不使用！
			}
		}

		if (i >= plArray->m_NodeNum)
		{
			if (plArray->m_NodeNum < HWL_IGMP_ARRAY_SIZE)
			{
				plIGMPNode = &plArray->m_pNode[plArray->m_NodeNum];
				plIGMPNode->m_ChnIndex = ChnIndex;
				plIGMPNode->m_DesAddr = IPAddr;
				plIGMPNode->m_SrcAddr = SrcAddr;
				plIGMPNode->m_SrcPort = SrcPort;
				plArray->m_NodeNum++;
			}
		}
	}
}


void HWL_IGMPClear(void)
{
	s_pIGMPArray.m_NodeNum = 0;
}


void HWL_IGMPApply(S32 ChnIndex, BOOL bJoin)
{
	S32 i, lLen, lMemberNum, lMsgLen, lFrameCount;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	HWL_IGMPArray *plArray;
	HWL_IGMPNode *plNode;
	S32 lPhyIndex;

	plArray = &s_pIGMPArray;

#ifdef PRINTF_IGMP_ARRAY
	HWLL_IGMPPrintArray("IGMP Array Apply", plArray);
#endif

#if defined(GM2730H)
	if (ChnIndex == 1)
	{
		lPhyIndex = 0;
	}
	else if (ChnIndex == 2)
	{
		lPhyIndex = 1;
	}
	else
	{
		lPhyIndex = 0;
	}
#elif defined(SUPPORT_NEW_HWL_MODULE)
	if (ChnIndex == 0)
	{
		lPhyIndex = 0;
	}
	else if (ChnIndex == 1)
	{
		lPhyIndex = 1;
	}
	else
	{
		lPhyIndex = 0;
	}
#else
	lPhyIndex = 0;
#endif

	lFrameCount = 0;
	i = 0;
	while(TRUE)
	{
		lLen = 0;
		lMsgLen = 0;
		lMemberNum = 0;
		plTmpBuf = plCMDBuf;

		GLOBAL_MSB8_EC(plTmpBuf, HWL_IGMP_TAG, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, lMsgLen, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, (bJoin > 0)?0:1, lLen);

		GLOBAL_MSB8_EC(plTmpBuf, lPhyIndex, lLen);

		GLOBAL_MSB8_EC(plTmpBuf, lFrameCount, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, plArray->m_IGMPVersion, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, plArray->m_RepeatTime, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

		for (; i < plArray->m_NodeNum; i++)
		{
			plNode = &plArray->m_pNode[i];
			if (plNode->m_ChnIndex == ChnIndex)
			{
				if (lMemberNum < HWL_IGMP_MAX_MEMBER_IN_FRAME)
				{
					GLOBAL_MSB32_EC(plTmpBuf, plNode->m_DesAddr, lLen);
					GLOBAL_MSB32_EC(plTmpBuf, plNode->m_SrcAddr, lLen);
				}
				else
				{
					break;
				}
				lMemberNum++;
			}
		}

		lMsgLen = lMemberNum * 2 + 1;
		plCMDBuf[1] = lMsgLen;

		//CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
		HWL_FPGAWrite(plCMDBuf, lLen);

		if (i >= plArray->m_NodeNum)
		{
			break;
		}
		else
		{
			lFrameCount++;
		}
	}
}












/*EOF*/
