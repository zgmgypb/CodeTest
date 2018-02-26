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
#define HWL_ETH_ARRAY_SIZE							(1024)
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	S32				m_ChnIndex;
	S32				m_SubIndex;
	U8				m_Protocol;
	U32				m_DesAddr;
	U16				m_DesPort;
	U32				m_SrcAddr;
	U16				m_SrcPort;
	BOOL8			m_ActiveMark;
	U32				m_MaxBitrate;
}HWL_ETHNode;

typedef struct  
{
	HWL_ETHNode		m_pNode[HWL_ETH_ARRAY_SIZE];
	S32				m_NodeNum;
}HWL_ETHArray;

/* Private Variables (static)-------------------------------------------------- */
static HWL_ETHArray	s_IPArray;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
S32 HWLL_ETHQSortCompare(const void *pElement1, const void *pElement2)
{
	HWL_ETHNode *plNode1, *plNode2;

	plNode1 = (HWL_ETHNode *)pElement1;
	plNode2 = (HWL_ETHNode *)pElement2;

	if (plNode1->m_ChnIndex > plNode2->m_ChnIndex)
	{
		return 1;
	}
	else if (plNode1->m_ChnIndex == plNode2->m_ChnIndex)
	{
		if (plNode1->m_DesAddr > plNode2->m_DesAddr)
		{
			return 1;
		}
		else if (plNode1->m_DesAddr == plNode2->m_DesAddr)
		{
			if (plNode1->m_DesPort > plNode2->m_DesPort)
			{
				return 1;
			}
			else if (plNode1->m_DesPort == plNode2->m_DesPort)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

	return 0;
}

/* API Functions ------------------------------------------------------------------ */
void HWL_ETHSetChnParameter(S16 ChnIndex, U32 IPAddr, U8 pMAC[GLOBAL_MAC_BUF_SIZE], BOOL bInput, U32 MaxBitrate)
{
	S32 lLen, lTmpValue;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	S16 lPhyIndex;

	lLen = 0;
	plTmpBuf = plCMDBuf;

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


#if defined(MULT_SUPPORT_FPGA_ETH)
	if (ChnIndex == 0xFE)
	{
		lPhyIndex = 0xFE;
	}
#endif

	GLOBAL_TRACE(("ChnInd = %d, PhyInd = %d, IP = 0x%.8X\n", ChnIndex, lPhyIndex, IPAddr));
#if HWL_DEBUG_PRINT_ETH_APPLY
#endif

	GLOBAL_MSB8_EC(plTmpBuf, ICPL_TAGID_ETH_CHN_PARAM, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x06, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, lPhyIndex, lLen);

	GLOBAL_MSB16_EC(plTmpBuf, 0x0000, lLen);

	GLOBAL_BYTES_EC(plTmpBuf, pMAC, GLOBAL_MAC_BUF_SIZE, lLen);

	//if (bInput)
	//{
	//	GLOBAL_BYTES_SC(plTmpBuf, 3, lLen);
	//}
	//else
	{
		lTmpValue = MULT_FPGA_MAIN_CLK * 1504.0 / (((F64)MaxBitrate) / 1000000);
#if HWL_DEBUG_PRINT_ETH_APPLY
		GLOBAL_TRACE(("ChnInd = %d, MaxBitrate = %d, 0x%.4X\n", ChnIndex, MaxBitrate, lTmpValue));
#endif
		GLOBAL_MSB16_EC(plTmpBuf, lTmpValue, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	}
	GLOBAL_MSB8_EC(plTmpBuf, 0x04, lLen);//IP Version
	GLOBAL_MSB32_EC(plTmpBuf, IPAddr, lLen);//IP Addr
	GLOBAL_BYTES_SC(plTmpBuf, 8, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}


void HWL_ETHLoopSelect(S16 ChnIndex,U8 ScrambleMark, U8 PCRActive)
{
	S32 lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	U8 lMark;
	U8 lTmpValue;

	S32 lPhyIndex = ChnIndex;

	lTmpValue = 0x00;
	lTmpValue = (ScrambleMark + PCRActive)<<4;
	lTmpValue += (lPhyIndex & 0x0f);
	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, 0x04/*HWL_OUT_ETH_SUB_PARAM_TAG*/, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);
	DRL_FpgaWriteLock(plCMDBuf, lLen);
}


void HWL_ETHAdd(S32 ChnIndex, S32 SubIndex, U8 Protocol, U32 DesAddr, U16 DesPort, BOOL8 Active, BOOL bInput, U32 MaxBitrate)
{
	S32 i;
	HWL_ETHArray *plArray;
	HWL_ETHNode *plNode;

	plArray = &s_IPArray;

	for (i = 0; i < plArray->m_NodeNum; i++)
	{
		plNode = &plArray->m_pNode[i];
		if (plNode->m_ChnIndex == ChnIndex && plNode->m_DesAddr == DesAddr && plNode->m_DesPort == DesPort)
		{
			break;//排除重复项目，源地址限定部分暂不使用！
		}
	}

	if (i == plArray->m_NodeNum)
	{
		if (plArray->m_NodeNum < HWL_ETH_ARRAY_SIZE)
		{
			plNode = &plArray->m_pNode[plArray->m_NodeNum];
			plNode->m_ChnIndex 	= ChnIndex;
			plNode->m_SubIndex 	= SubIndex;
			plNode->m_Protocol 	= Protocol;
			plNode->m_DesAddr 	= DesAddr;
			plNode->m_DesPort 	= DesPort;
			plNode->m_ActiveMark 	= Active;
			plNode->m_MaxBitrate = MaxBitrate;
			plArray->m_NodeNum++;
		}
	}
}


void HWL_ETHClear(BOOL bInput)
{
	HWL_ETHArray *plArray;
	plArray = &s_IPArray;
	plArray->m_NodeNum = 0;

}

void HWL_ETHApply(S32 ChnIndex, BOOL bInput)
{
#ifdef GN1846
#else
	U8 tmp;
	S32 i, lLen, lMemberNum, lTmpValue;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf, lTagID;
	S32 lPhyIndex;
	HWL_ETHArray *plArray;
	HWL_ETHNode *plNode;	

	if (bInput)
	{
		plArray = &s_IPArray;
		lTagID = ICPL_TAGID_INPUT_ETH_SUB_PARAM;


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

#ifdef GM2730S
		if (bInput == TRUE)
		{
			if (ChnIndex == 1)
			{
				lPhyIndex = 1;
			}
		}
#endif


	}
	else
	{
		plArray = &s_IPArray;
		lTagID = ICPL_TAGID_OUTPUT_ETH_SUB_PARAM;
		lPhyIndex = 0;
#if defined(SUPPORT_NEW_HWL_MODULE)
		lPhyIndex = ChnIndex;
#endif
	}

	/*输入才排序！*/
	GLOBAL_QSORT(plArray->m_pNode, plArray->m_NodeNum, sizeof(HWL_ETHNode), HWLL_ETHQSortCompare);

	GLOBAL_TRACE(("ChnInd = %d, PhyIndex = %d, Input = %d, NodeNum = %d\n", ChnIndex, lPhyIndex, bInput, plArray->m_NodeNum));

	/*应用*/
	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	lMemberNum = 0;
	for (i = 0; i < plArray->m_NodeNum; i++)
	{
		tmp = 0;
		plNode = &plArray->m_pNode[i];
		//if (lMemberNum <= 450)
		{
			if (plNode->m_ChnIndex == ChnIndex)
			{
				lLen = 0;
				plTmpBuf = plCMDBuf;

				GLOBAL_MSB8_EC(plTmpBuf, lTagID, lLen);
				GLOBAL_MSB8_EC(plTmpBuf, 0x05, lLen);
				GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
				GLOBAL_MSB8_EC(plTmpBuf, lPhyIndex, lLen);

				GLOBAL_MSB16_EC(plTmpBuf, plNode->m_SubIndex, lLen);
				GLOBAL_MSB16_EC(plTmpBuf, plNode->m_DesPort, lLen);
#ifdef GM2730S
				GLOBAL_MSB16_EC(plTmpBuf, i, lLen);
				tmp |= plNode->m_Protocol;
				tmp |= ( !plNode->m_ActiveMark ) <<7;
				GLOBAL_MSB8_EC(plTmpBuf, tmp, lLen);
#else
				if (bInput)
				{
					GLOBAL_MSB16_EC(plTmpBuf, i, lLen);
				}
				else
				{
					lTmpValue = MULT_FPGA_MAIN_CLK * 1504.0 / (((F64)plNode->m_MaxBitrate) / 1000000);
					//GLOBAL_TRACE(("SubInd = %d, MaxBitrate = %d, 0x%.4X\n", plNode->m_SubIndex, plNode->m_MaxBitrate, lTmpValue));
					GLOBAL_MSB16_EC(plTmpBuf, lTmpValue, lLen);
				}
				tmp |= plNode->m_Protocol;
				tmp |= ( !plNode->m_ActiveMark ) << 4;
				tmp |= ( !plNode->m_ActiveMark ) << 7;//BIT4/BIT7都是开关，需要兼容协议
				GLOBAL_MSB8_EC(plTmpBuf, tmp, lLen);
#endif
				GLOBAL_MSB8_EC(plTmpBuf, 0x04, lLen);

				GLOBAL_MSB32_EC(plTmpBuf, plNode->m_DesAddr, lLen);
				GLOBAL_MSB32_EC(plTmpBuf, plNode->m_SrcAddr, lLen);
				GLOBAL_BYTES_SC(plTmpBuf, 4, lLen);//保留

				//CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);

				HWL_FPGAWrite(plCMDBuf, lLen);

				//GLOBAL_TRACE(("Set SubChn %d, Addr = 0x%.8X:%d, Arrange = %d\n", plNode->m_SubIndex, plNode->m_DesAddr, plNode->m_DesPort, i));

				lMemberNum++;
			}
		}
	}
#endif
}


/*EOF*/
