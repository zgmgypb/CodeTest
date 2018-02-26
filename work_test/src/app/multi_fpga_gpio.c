//这个是有自己的线程和进程的的模块

/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_tsp.h"

#ifdef SUPPORT_FGPIO_MODULE
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */

//#define FPGA_GPIO_PROTOCOL_DEBUG
#define FPGA_GPIO_MAX_PORT_SUPPORST					(1)
#define FPGA_GPIO_PROTOCOL_TAG_ID					(0x36)
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */

typedef struct
{
	U32			m_ReadValue;
	U32			m_OutValue;
	U32			m_PortIOMask;
	BOOL		m_bUpdated;
}FGPIO_Port;


typedef struct
{
	FGPIO_Port	m_pPort[FPGA_GPIO_MAX_PORT_SUPPORST];
}FGPIO_Handle;


/* Private Variables (static)-------------------------------------------------- */
static FGPIO_Handle *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
static void FPGIOL_ProtocolPacker(S32 PortInd, U32 GPIOValue, U32 GPIOMask, BOOL bRead);
/* Functions ------------------------------------------------------------------ */
BOOL FGPIO_Initiate(void)
{
	BOOL lRet = FALSE;
	if (s_pHandle == NULL)
	{
		FGPIO_Handle *plHandle = (FGPIO_Handle*) GLOBAL_ZMALLOC(sizeof(FGPIO_Handle));
		if (plHandle)
		{
			GLOBAL_TRACE(("Init Done!\n"));
			lRet = TRUE;
		}
		s_pHandle = plHandle;
	}
	return lRet;
}

/*消息解析函数，解析FPGA给出的值，并对DA进行调整*/
BOOL FGPIO_ProtocolParser(U8 *pData, S32 DataSize)
{
	BOOL lRet = FALSE;

	if (s_pHandle)
	{
		if (pData)
		{
#ifdef FPGA_GPIO_PROTOCOL_DEBUG
			CAL_PrintDataBlock(__FUNCTION__, pData, DataSize);
#endif
			if ((pData[0] == FPGA_GPIO_PROTOCOL_TAG_ID) && (DataSize >= 8))
			{
				U8 *plTmpData;
				S32 lPortInd;

				plTmpData = pData;

				GLOBAL_BYTES_S(plTmpData, 3);
				GLOBAL_MSB8_D(plTmpData, lPortInd);

				if (GLOBAL_CHECK_INDEX(lPortInd, FPGA_GPIO_MAX_PORT_SUPPORST))
				{
					FGPIO_Port *plPort;
					plPort = &s_pHandle->m_pPort[lPortInd];

					GLOBAL_MSB32_D(plTmpData, plPort->m_ReadValue);
				}
				else
				{
					GLOBAL_TRACE(("Port Ind [%d] Error!\n", lPortInd));
				}
			}
		}
		else
		{
			GLOBAL_TRACE(("NULL Ptr!!!!!!!!!!\n"));
		}

	}
	else
	{
		GLOBAL_TRACE(("Module Not Initiated!\n"));
	}
	return lRet;
}

void FGPIO_ValueSet(S32 PortInd, U32 GPIOMask, U8 bHigh)
{
	if (s_pHandle)
	{
		if (GLOBAL_CHECK_INDEX(PortInd, FPGA_GPIO_MAX_PORT_SUPPORST))
		{
			FGPIO_Port *plPort;

			plPort = &s_pHandle->m_pPort[PortInd];
			if (bHigh)
			{
				plPort->m_OutValue |= GPIOMask;
			}
			else
			{
				plPort->m_OutValue &= ~GPIOMask;
			}
		}
	}
}

void FGPIO_IOMaskSet(S32 PortInd, U32 GPIOMask, U8 bInput)
{
	if (s_pHandle)
	{
		if (GLOBAL_CHECK_INDEX(PortInd, FPGA_GPIO_MAX_PORT_SUPPORST))
		{
			FGPIO_Port *plPort;

			plPort = &s_pHandle->m_pPort[PortInd];
			if (bInput)
			{
				plPort->m_PortIOMask &= ~GPIOMask;
			}
			else
			{
				plPort->m_PortIOMask |= GPIOMask;
			}
		}
	}
}


void FGPIO_Write(S32 PortInd)
{
	if (s_pHandle)
	{
		if (GLOBAL_CHECK_INDEX(PortInd, FPGA_GPIO_MAX_PORT_SUPPORST))
		{
			FGPIO_Port *plPort;
			plPort = &s_pHandle->m_pPort[PortInd];
			FPGIOL_ProtocolPacker(PortInd, plPort->m_OutValue, plPort->m_PortIOMask, FALSE);
		}
	}
}


BOOL FGPIO_Read(S32 PortInd, U32 *pValue, S32 Timeout)
{
	BOOL lRet = FALSE;
	if (s_pHandle)
	{
		if (GLOBAL_CHECK_INDEX(PortInd, FPGA_GPIO_MAX_PORT_SUPPORST))
		{
			FGPIO_Port *plPort;

			plPort = &s_pHandle->m_pPort[PortInd];
			plPort->m_bUpdated = FALSE;
			FPGIOL_ProtocolPacker(PortInd, 0, 0, TRUE);
			while(Timeout > 0)
			{
				if (plPort->m_bUpdated == TRUE)
				{
					break;
				}
				PFC_TaskSleep(10);
				Timeout -= 10;
			}

			if (plPort->m_bUpdated == TRUE)
			{
				if (pValue)
				{
					(*pValue) = plPort->m_ReadValue;
				}
				lRet = TRUE;
			}
		}
	}
	return lRet;
}


/*模块销毁函数*/
void FGPIO_Terminate(void)
{
	if (s_pHandle)
	{
		GLOBAL_FREE(s_pHandle);
		s_pHandle = NULL;

		GLOBAL_TRACE(("MODULE Terminate Complete!\n"));
	}
}


/* 本地函数 --------------------------------------------------------------------------------------------------------------------------------------- */

void FPGIOL_ProtocolPacker(S32 PortInd, U32 GPIOValue, U32 GPIOMask, BOOL bRead)
{
	S32 lLen;
	U8 plCMDBuf[1024], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;


	{
		GLOBAL_MSB8_EC(plTmpBuf, FPGA_GPIO_PROTOCOL_TAG_ID, lLen);
		if (bRead)
		{
			GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
			GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
		}
		else
		{
			GLOBAL_MSB8_EC(plTmpBuf, 0x02, lLen);
			GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
		}
		GLOBAL_MSB8_EC(plTmpBuf, PortInd, lLen);

		if (bRead == FALSE)
		{
			GLOBAL_MSB32_EC(plTmpBuf, GPIOValue, lLen);
			GLOBAL_MSB32_EC(plTmpBuf, GPIOMask, lLen);
		}
	}

#ifdef FPGA_GPIO_PROTOCOL_DEBUG
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);
}

#endif










/*EOF*/
