/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "multi_private.h"

#include "hwl_vrs232.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
//#define PROTOCOL_DEBUG

#define HWL_VSRS232_MAX_SLOT			(2)
#define HWL_VSRS232_IO_BUF_SIZE			(1024)
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */


typedef struct  
{
	HWL_VRS232_Param		m_VRS232Param;
	HANDLE32				m_FIFOHandle;
	BOOL					m_bOpen;
}HWL_VRS232_DEV;


typedef struct  
{
	/*设置参数*/

	HWL_VRS232_DEV				m_pVRS232DEV[HWL_VSRS232_MAX_SLOT];


}HWL_VRS232_HANDLE;

/* Private Variables (static)-------------------------------------------------- */
static HWL_VRS232_HANDLE s_VRS232Handle;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

void HWL_VRS232Initiate(void)
{
	S32 i;
	GLOBAL_ZEROMEM(&s_VRS232Handle, sizeof(HWL_VRS232_HANDLE));

	for (i = 0; i < HWL_VSRS232_MAX_SLOT; i++)
	{
		s_VRS232Handle.m_pVRS232DEV[i].m_FIFOHandle = (HANDLE32)PAL_DataQueueCreate(64 * HWL_VSRS232_IO_BUF_SIZE);
	}

}

void HWL_VRS232Open(S32 Slot, HWL_VRS232_Param *pParam)
{
	if (GLOBAL_CHECK_INDEX(Slot, HWL_VSRS232_MAX_SLOT))
	{
		GLOBAL_MEMCPY(&s_VRS232Handle.m_pVRS232DEV[Slot].m_VRS232Param, pParam, sizeof(HWL_VRS232_Param));
		if (s_VRS232Handle.m_pVRS232DEV[Slot].m_VRS232Param.m_TimeOutMS <= 0)
		{
			s_VRS232Handle.m_pVRS232DEV[Slot].m_VRS232Param.m_TimeOutMS = 1;
		}
		s_VRS232Handle.m_pVRS232DEV[Slot].m_bOpen = TRUE;
		GLOBAL_TRACE(("Slot %d Open Successful!!!\n", Slot));
	}
}

void HWL_VRS232Close(S32 Slot)
{
	if (GLOBAL_CHECK_INDEX(Slot, HWL_VSRS232_MAX_SLOT))
	{
		s_VRS232Handle.m_pVRS232DEV[Slot].m_bOpen = FALSE;
		GLOBAL_TRACE(("Slot %d Close Successful!!!\n", Slot));
	}
}

S32 HWL_VRS232Write(S32 Slot, U8 *pData, S32 DataLen)
{
	S32 lActSend = 0;

	if (GLOBAL_CHECK_INDEX(Slot, HWL_VSRS232_MAX_SLOT))
	{
		HWL_VRS232_DEV *plDEV = &s_VRS232Handle.m_pVRS232DEV[Slot];
		if (plDEV->m_bOpen == TRUE)
		{
			HWL_VRS232WriteProtocol(Slot, pData, DataLen);
			
			lActSend = DataLen;
		}
	}

	return lActSend;
}


S32 HWL_VRS232Read(S32 Slot, U8 *pBuf, S32 BufLen)
{
	S32 lActRecv = 0;

	if (GLOBAL_CHECK_INDEX(Slot, HWL_VSRS232_MAX_SLOT))
	{
		HWL_VRS232_DEV *plDEV = &s_VRS232Handle.m_pVRS232DEV[Slot];
		if (plDEV->m_bOpen == TRUE)
		{
			lActRecv = PAL_DataQueueRead(plDEV->m_FIFOHandle, pBuf, BufLen, plDEV->m_VRS232Param.m_TimeOutMS);
		}
	}
	return lActRecv;
}

S32 HWL_VRS232Flush(S32 Slot)
{
	S32 lActRecv = 0;

	if (GLOBAL_CHECK_INDEX(Slot, HWL_VSRS232_MAX_SLOT))
	{
		HWL_VRS232_DEV *plDEV = &s_VRS232Handle.m_pVRS232DEV[Slot];
		if (plDEV->m_bOpen)
		{
			PAL_DataQueueFlush(plDEV->m_FIFOHandle);
		}
	}
	return lActRecv;
}

void HWL_VRS232Terminate(void)
{
	S32 i;
	for (i = 0; i < HWL_VSRS232_MAX_SLOT; i++)
	{
		if (s_VRS232Handle.m_pVRS232DEV[i].m_FIFOHandle)
		{
			PAL_DataQueueDestroy(s_VRS232Handle.m_pVRS232DEV[i].m_FIFOHandle);
		}
		s_VRS232Handle.m_pVRS232DEV[i].m_FIFOHandle = NULL;
	}
}




void HWL_VRS232WriteProtocol(S32 Slot, U8 *pData, S32 DataSize)
{
	S32 lLen;
	U32 lTmpValue;
	U8 plCMDBuf[1024], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, 0x32/*串口协议*/, lLen);

	lTmpValue = DataSize / 4 + (((DataSize%4) > 0)?1:0) + 1;

	GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, Slot, lLen);

	GLOBAL_MSB16_EC(plTmpBuf, DataSize, lLen);
	GLOBAL_BYTES_SC(plTmpBuf, 2, lLen);//保留

	GLOBAL_BYTES_EC(plTmpBuf, pData, DataSize, lLen);

	if ( (DataSize%4) > 0)
	{
		lLen += (4 - (DataSize%4));
	}

#ifdef PROTOCOL_DEBUG
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);
}


void HWL_VRS232ReadProtocol(U8 *pData, S32 DataSize)
{
	U8 *plTmpBuf;

#ifdef PROTOCOL_DEBUG
	CAL_PrintDataBlock(__FUNCTION__, pData, DataSize);
#endif

	plTmpBuf = pData;

	{
		S32 lSlotInd, lTagLen, lByteLen;

		GLOBAL_BYTES_S(plTmpBuf, 1);//TAG
		GLOBAL_MSB8_D(plTmpBuf, lTagLen);
		GLOBAL_BYTES_S(plTmpBuf, 1);//0x00
		GLOBAL_MSB8_D(plTmpBuf, lSlotInd);

		if (GLOBAL_CHECK_INDEX(lSlotInd, HWL_VSRS232_MAX_SLOT) && (lTagLen > 1))
		{
			HWL_VRS232_DEV *plDEV = &s_VRS232Handle.m_pVRS232DEV[lSlotInd];
			if (plDEV->m_bOpen)
			{
				GLOBAL_MSB16_D(plTmpBuf, lByteLen);
				GLOBAL_BYTES_S(plTmpBuf, 2);//保留

#ifdef SUPPORT_NTS_DPD_BOARD
				{
					if (lSlotInd == 1)//在这里不判断协议长度！
					{
						PAL_DataQueueWrite(plDEV->m_FIFOHandle, plTmpBuf, (lTagLen - 1) * 4);
					}
					else
					{
						if (lByteLen <= (lTagLen - 1) * 4)
						{
							PAL_DataQueueWrite(plDEV->m_FIFOHandle, plTmpBuf, lByteLen);
						}
						else
						{
							GLOBAL_TRACE(("Tag Len = %d, ByteLen = %d\n", lTagLen, lByteLen));
						}
					}
				}
#else
				if (lByteLen <= (lTagLen - 1) * 4)
				{
					PAL_DataQueueWrite(plDEV->m_FIFOHandle, plTmpBuf, lByteLen);
				}
				else
				{
					GLOBAL_TRACE(("Tag Len = %d, ByteLen = %d\n", lTagLen, lByteLen));
				}
#endif


			}
		}
		
	}
}



/*EOF*/
