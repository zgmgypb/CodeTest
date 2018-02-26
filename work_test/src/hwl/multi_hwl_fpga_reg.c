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
#include "fpga_reg.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
//#define PROTOCOL_DEBUG
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	HANDLE32	m_FPGARegHandle;
}HWL_FPGARegHandle;

/* Private Variables (static)-------------------------------------------------- */
static HWL_FPGARegHandle *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
static BOOL HWLL_FPGARegICPSend(void *pUserParam, U8 *pData, S32 DataSize);
/* Functions ------------------------------------------------------------------ */

void HWL_FPGARegInitate(void)
{
	HWL_FPGARegHandle *plHandle;
	plHandle = (HWL_FPGARegHandle *)GLOBAL_ZMALLOC(sizeof(HWL_FPGARegHandle));
	if (plHandle)
	{
		FREG_InitParam lInitParam;

		lInitParam.m_NodeNum = 1;
		lInitParam.m_pICPSendCB = HWLL_FPGARegICPSend;
		lInitParam.m_pUserParam = plHandle;

		plHandle->m_FPGARegHandle = FREG_Create(&lInitParam);

		s_pHandle = plHandle;
	}
	else
	{
		GLOBAL_TRACE(("NULL PTR\n"));
	}

}

BOOL HWL_FPGARegProcess(S32 SlotInd, U16 ChipID, U32 Address, U32 *pData, S32 Size, BOOL bRead, S32 TimeoutMS)
{
	BOOL lRet = FALSE;
	HWL_FPGARegHandle *plHandle = s_pHandle;
	if (plHandle)
	{
		lRet = FREG_Process(plHandle->m_FPGARegHandle, SlotInd, ChipID, Address, pData, Size, bRead, TimeoutMS);
	}
	else
	{
		GLOBAL_TRACE(("NULL PTR\n"));
	}
	return lRet;
}

BOOL HWLL_FPGARegICPSend(void *pUserParam, U8 *pData, S32 DataSize)
{
	return (HWL_FPGAWrite(pData, DataSize) == 0);
}

BOOL HWL_FPGARegICPRecv(U8 *pData, S32 DataSize)
{
	HWL_FPGARegHandle *plHandle = s_pHandle;
	if (plHandle)
	{
		return FREG_ICPRecv(plHandle->m_FPGARegHandle, pData, DataSize);
	}
	else
	{
		GLOBAL_TRACE(("NULL PTR\n"));
	}
	return FALSE;
}

void HWL_FPGARegTerminate(void)
{
	HWL_FPGARegHandle *plHandle = s_pHandle;
	if (plHandle)
	{
		FREG_Destroy(plHandle->m_FPGARegHandle);

		GLOBAL_FREE(plHandle);

		s_pHandle = NULL;
	}
	else
	{
		GLOBAL_TRACE(("NULL PTR\n"));
	}
}


/*调用上面模块的子应用模块*/


#define HWL_FPGA_GETH_NUM	2

static U8 s_pPhyAddr[HWL_FPGA_GETH_NUM];

void HWL_FPGARegPHYInitate(void)
{
	S32 i;
	for (i = 0; i < HWL_FPGA_GETH_NUM; i++)
	{
		s_pPhyAddr[i] = 0xFF;
	}
}

BOOL HWL_FPGARegPHYDetectOK(S32 Slot)
{
	if (GLOBAL_CHECK_INDEX(Slot, HWL_FPGA_GETH_NUM))
	{
		return (s_pPhyAddr[Slot] != 0xFF);
	}
	return FALSE;
}

void HWL_FPGARegPHYDetectAddr(S32 Slot)
{
	S32 i, lRetryCount;
	U32 lData, lAddr;

	lRetryCount = 4;
	while((s_pPhyAddr[Slot] == 0xFF) && (lRetryCount > 0))
	{
		for (i = 0; i < 16; i++)
		{
			lAddr = (i + 0x08) & 0x0F;
			//GLOBAL_TRACE(("i = %d, Phy Addr = %d\n", i, lAddr));
			if (HWL_FPGARegProcess(0, Slot + 0x01, 0x0F, &lAddr, 1, FALSE, 1000))
			{
				if (HWL_FPGARegProcess(0, Slot + 0x01, 0x82, &lData, 1, TRUE, 1000))
				{
					if (lData == 0x0141)
					{
						s_pPhyAddr[Slot] = lAddr;
						break;
					}
				}
			}
			else
			{
				GLOBAL_TRACE(("Access Error !!!!!! RetryCount = %d\n", lRetryCount));
				break;
			}
			PFC_TaskSleep(10);
		}

		lRetryCount--;
	}


	if (s_pPhyAddr[Slot] != 0xFF)
	{
		GLOBAL_TRACE(("Slot = %d, Phy = %d, OK!\n", Slot, s_pPhyAddr[Slot]));

		lData = 0x4140;
		if (HWL_FPGARegProcess(0, Slot + 0x01, 0x98, &lData, 1, FALSE, 1000) != TRUE)
		{
			GLOBAL_TRACE(("Set PHY LED Reg Failed!!!!!!!\n"));
		}
	}
	else
	{
		GLOBAL_TRACE(("Slot = %d, Phy = %d, Failed!\n", Slot, s_pPhyAddr[Slot]));

	}
}

#define GLOBAL_32GET_NBIT(value, bit_h, bit_l)	 (((value) >> (bit_l)) & CAL_GetBitMASK32(bit_h - bit_l + 1))

S32 HWL_FPGARegPHYGetLinkStatus(S32 Slot)
{
	S32 lRet = 0;
	
	U32 lData;
	U32 lSpeed;
	lData = 0x0;

	if (s_pPhyAddr[Slot] != 0xFF)
	{
		if (HWL_FPGARegProcess(0, Slot + 0x01, 0x91, &lData, 1, TRUE, 1000) != TRUE)
		{
			GLOBAL_TRACE(("Get PHY Reg Failed!!!!!!!\n"));
		}
		else
		{
#ifdef PROTOCOL_DEBUG
			GLOBAL_TRACE(("Reg Data = 0x%04X\n", lData));
#endif
		}


		if (GLOBAL_32GET_NBIT(lData, 10, 10) > 0)
		{
			lSpeed = GLOBAL_32GET_NBIT(lData, 15, 14);
			lRet = lSpeed + 1;

			if (lRet >0 && lRet <= 3)
			{
#ifdef PROTOCOL_DEBUG
				GLOBAL_TRACE(("Slot %d, Link Speed = %dM\n", Slot, (S32)pow(10, lRet)));
#endif
			}
		}
		else
		{
#ifdef PROTOCOL_DEBUG
			GLOBAL_TRACE(("Slot %d, Link Down\n", Slot));
#endif
		}
	}

	return lRet;
}
/*EOF*/
