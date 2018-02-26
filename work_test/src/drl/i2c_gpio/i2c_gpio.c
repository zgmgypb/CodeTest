#include "i2c_gpio.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"

typedef struct {
	void (*m_SdaSetValue)(S32 Value, void *pUserParam);
	void (*m_SclSetValue)(S32 Value, void *pUserParam);
	void (*m_SdaSetup)(BOOL IsInput, BOOL IsPullUp, void *pUserParam);
	void (*m_SclSetup)(BOOL IsInput, BOOL IsPullUp, void *pUserParam);
	int (*m_SdaGetValue)(void *pUserParam);

	void *m_pUserParam;
}I2C_GpioHandle;

/*  注意：写的时候一定不能产生二义性，一定是 sda先为低，然后操作 scl。
*  在操作的时候时钟信号变化后都要修改为低，避免产生起始或结束条件。 */
static void I2C_GpioStart(I2C_GpioHandle *pHandle)
{
	pHandle->m_SdaSetup(FALSE, TRUE, pHandle->m_pUserParam);
	pHandle->m_SclSetup(FALSE, TRUE, pHandle->m_pUserParam);
	pHandle->m_SclSetValue(1, pHandle->m_pUserParam);
	pHandle->m_SdaSetValue(1, pHandle->m_pUserParam);
	pHandle->m_SdaSetValue(0, pHandle->m_pUserParam);
	pHandle->m_SclSetValue(0, pHandle->m_pUserParam);
}

static void I2C_GpioStop(I2C_GpioHandle *pHandle)
{
	pHandle->m_SdaSetValue(0, pHandle->m_pUserParam);
	pHandle->m_SclSetValue(1, pHandle->m_pUserParam);
	pHandle->m_SdaSetValue(1, pHandle->m_pUserParam);
}

static void I2C_GpioSendAck(I2C_GpioHandle *pHandle)
{
	pHandle->m_SdaSetValue(0, pHandle->m_pUserParam);
	pHandle->m_SclSetValue(1, pHandle->m_pUserParam);
	pHandle->m_SclSetValue(0, pHandle->m_pUserParam);
}

static void I2C_GpioSendNoAck(I2C_GpioHandle *pHandle)
{
	pHandle->m_SdaSetValue(1, pHandle->m_pUserParam);
	pHandle->m_SclSetValue(1, pHandle->m_pUserParam);
	pHandle->m_SclSetValue(0, pHandle->m_pUserParam);
}

static int I2C_GpioWaitAck(I2C_GpioHandle *pHandle)
{
	int lTimeOut = 100;
	unsigned char lCh;

	pHandle->m_SdaSetup(TRUE, TRUE, pHandle->m_pUserParam);
	while(lTimeOut--)
	{
		lCh = pHandle->m_SdaGetValue(pHandle->m_pUserParam) & 0x01;
		if(0 == lCh)
			break;
		PL_TaskSleep(1);
	}
	pHandle->m_SclSetValue(1, pHandle->m_pUserParam); // 第九个时钟
	pHandle->m_SclSetValue(0, pHandle->m_pUserParam);
	pHandle->m_SdaSetup(FALSE, TRUE, pHandle->m_pUserParam);

	if(lTimeOut > 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static int I2C_GpioSend8Bit(I2C_GpioHandle *pHandle, const char Val)
{
	unsigned char lTmp = 0x80;
	int i;

	for(i=0; i<8; i++)
	{
		pHandle->m_SclSetValue(0, pHandle->m_pUserParam);
		if(lTmp & Val)       //MSB first
		{
			pHandle->m_SdaSetValue(1, pHandle->m_pUserParam);
		}
		else
		{
			pHandle->m_SdaSetValue(0, pHandle->m_pUserParam);
		}
		pHandle->m_SclSetValue(1, pHandle->m_pUserParam);
		lTmp >>= 1;
	}
	pHandle->m_SclSetValue(0, pHandle->m_pUserParam);
	pHandle->m_SdaSetValue(1, pHandle->m_pUserParam);
	if (!I2C_GpioWaitAck(pHandle))
	{
		GLOBAL_TRACE ((ANSI_COLOR_RED"wait ack error -1!\n"ANSI_COLOR_NONE));
		return FALSE;
	}

	return TRUE;
}

static void I2C_GpioRead8Bit(I2C_GpioHandle *pHandle, char *pCh)
{
	unsigned char lTmp, lReadChar = 0;
	int i = 8;

	pHandle->m_SdaSetup(TRUE, TRUE, pHandle->m_pUserParam);
	for(i=0; i<8; i++)
	{
		pHandle->m_SclSetValue(1, pHandle->m_pUserParam);
		lTmp = pHandle->m_SdaGetValue(pHandle->m_pUserParam) & 0x01;
		lReadChar <<= 1;
		lReadChar |= lTmp;
		pHandle->m_SclSetValue(0, pHandle->m_pUserParam);
	}
	pHandle->m_SdaSetup(FALSE, TRUE, pHandle->m_pUserParam);
	*pCh = lReadChar;
}

/* 连续读*/
static int I2C_GpioReadBytes(I2C_GpioHandle *pHandle, int DevAddr, char *pSubAddr, int SubAddrLen, char *pData, int DataLen)
{
	int i;

	/* 设备地址 */
	if (!I2C_GpioSend8Bit(pHandle, DevAddr & 0xFF))
	{
		GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
		return -1;
	}

	/* 寄存器地址 */
	for (i=0; i<SubAddrLen; i++)
	{
		if (!I2C_GpioSend8Bit(pHandle, pSubAddr[i]))
		{
			GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
			return -1;
		}
	}

	I2C_GpioStart(pHandle);
	if (!I2C_GpioSend8Bit(pHandle, (DevAddr & 0xFF) | 0x01))
	{
		GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
		return -1;
	}
	for(i=0; i<DataLen-1; i++)
	{
		I2C_GpioRead8Bit(pHandle, &pData[i]);
		I2C_GpioSendAck(pHandle);
	}
	I2C_GpioRead8Bit(pHandle, &pData[i]);
	I2C_GpioSendNoAck(pHandle);

	return DataLen;
}

/* 连续写*/
static int I2C_GpioWriteBytes(I2C_GpioHandle *pHandle, int DevAddr, char *pSubAddr, int SubAddrLen, char *pData, int DataLen)
{
	int i;

	/* 设备地址 */
	if (!I2C_GpioSend8Bit(pHandle, DevAddr & 0xFF))
	{
		GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
		return -1;
	}

	/* 寄存器地址 */
	for (i=0; i<SubAddrLen; i++)
	{
		if (!I2C_GpioSend8Bit(pHandle, pSubAddr[i]))
		{
			GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
			return -1;
		}
	}

	for(i=0; i<DataLen; i++)
	{
		if (!I2C_GpioSend8Bit(pHandle, pData[i]))
		{
			GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
			return -1;
		}
	}

	return DataLen;
}

HANDLE32 I2C_GpioCreate(I2C_GpioInitParam *pInitPara)
{
	I2C_GpioHandle *plHandle;

	if (!pInitPara)
	{
		GLOBAL_TRACE(("pInitPara is NULL!\n"));
		return NULL;
	}

	if (!pInitPara->m_SclSetup || !pInitPara->m_SclSetValue
		|| !pInitPara->m_SdaSetup || !pInitPara->m_SdaSetValue)
	{
		GLOBAL_TRACE(("pInitPara Param Error!\n"));
		return NULL;
	}

	plHandle = (I2C_GpioHandle *)GLOBAL_MALLOC(sizeof(I2C_GpioHandle));
	if (!plHandle)
	{
		GLOBAL_TRACE(("malloc error!\n"));
		return NULL;
	}

	plHandle->m_pUserParam = pInitPara->m_pUserParam;
	plHandle->m_SclSetup = pInitPara->m_SclSetup;
	plHandle->m_SclSetValue = pInitPara->m_SclSetValue;
	plHandle->m_SdaSetup = pInitPara->m_SdaSetup;
	plHandle->m_SdaSetValue = pInitPara->m_SdaSetValue;
	plHandle->m_SdaGetValue = pInitPara->m_SdaGetValue;

	return plHandle;
}

int I2C_GpioRead(HANDLE32 Handle, U8 DevAddr, U8 *pRegAddr, U32 RegAddrLen, char *pBuf, U32 BufSize)
{
	I2C_GpioHandle *plHandle = (I2C_GpioHandle *)Handle;
	int lRet = -1;

	if (plHandle)
	{
		I2C_GpioStart(plHandle);
		lRet = I2C_GpioReadBytes(plHandle, DevAddr, pRegAddr, RegAddrLen, pBuf, BufSize);
		I2C_GpioStop(plHandle);
	}

	return lRet;
}

int I2C_GpioWrite(HANDLE32 Handle, U8 DevAddr, U8 *pRegAddr, U32 RegAddrLen, char *pBuf, U32 BufSize, BOOL IsPageAlign, int PageSize)
{
	int lRet = -1;
	I2C_GpioHandle *plHandle = (I2C_GpioHandle *)Handle;

	if (RegAddrLen > 4)
	{
		GLOBAL_TRACE(("RegAddrLen is too big!\n"));
		return -1;
	}

	if (plHandle)
	{
		if (!IsPageAlign)
		{
			I2C_GpioStart(plHandle);
			lRet = I2C_GpioWriteBytes(plHandle, DevAddr, pRegAddr, RegAddrLen, pBuf, BufSize);
			I2C_GpioStop(plHandle);
		}
		else /* 针对使用页对齐方式的 eeprom */
		{
			int i, j;
			int lCurPageRem = 0;
			int lPageNum = 0;
			int lRemPageNum = 0;
			int lPageSize = PageSize;
			int lRegAddr = 0, lTmpAddr = 0;
			char plRegAddr[4];

			for (i=RegAddrLen-1; i>=0; i--)
			{
				lRegAddr = pRegAddr[i] & (lRegAddr << 8);
			}
			lCurPageRem = lPageSize - (lRegAddr % lPageSize);
			lPageNum = (BufSize - lCurPageRem) / lPageSize;
			lRemPageNum = BufSize - lCurPageRem - lPageSize * lPageNum;

			I2C_GpioStart(plHandle);
			lRet = I2C_GpioWriteBytes(plHandle, DevAddr, pRegAddr, RegAddrLen, pBuf, lCurPageRem);
			I2C_GpioStop(plHandle);
			for (i=0; i<lPageNum; i++)
			{
				I2C_GpioStart(plHandle);
				lTmpAddr = lRegAddr + lCurPageRem + i * lPageSize;
				for (j=0; j<RegAddrLen; j++)
				{
					plRegAddr[j] = lTmpAddr & 0xFF;
					lTmpAddr >>= 8;
				}
				lRet = I2C_GpioWriteBytes(plHandle, DevAddr, plRegAddr, RegAddrLen, pBuf + lCurPageRem + i * lPageSize, lPageSize);
				I2C_GpioStop(plHandle);
			}
			I2C_GpioStart(plHandle);
			lTmpAddr = lRegAddr + lCurPageRem + lPageNum * lPageSize;
			for (j=0; j<RegAddrLen; j++)
			{
				plRegAddr[j] = lTmpAddr & 0xFF;
				lTmpAddr >>= 8;
			}
			lRet = I2C_GpioWriteBytes(plHandle, DevAddr, plRegAddr, RegAddrLen, pBuf + lCurPageRem + lPageNum * lPageSize, lRemPageNum);
			I2C_GpioStop(plHandle);
		}
		lRet = BufSize;
	}

	return lRet;
}

/* QN8007 芯片的 I2C 的读比较特别，没有专门写寄存器地址，是和读连起来的 */
int I2C_Gpio8007Read(HANDLE32 Handle, U8 DevAddr, U8 RegAddr, char *pBuf)
{
	I2C_GpioHandle *plHandle = (I2C_GpioHandle *)Handle;
	int i;

	I2C_GpioStart(plHandle);
	if (!I2C_GpioSend8Bit(plHandle, (DevAddr & 0xFF) | 0x01))
	{
		GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
		I2C_GpioStop(plHandle);
		return -1;
	}
	/* 寄存器地址 */
	if (!I2C_GpioSend8Bit(plHandle, RegAddr))
	{
		GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
		I2C_GpioStop(plHandle);
		return -1;
	}

	I2C_GpioRead8Bit(plHandle, pBuf);
	I2C_GpioSendNoAck(plHandle);
	I2C_GpioStop(plHandle);

	return 1;
}

int I2C_GpioTwoStepRead(HANDLE32 Handle, U8 DevAddr, char *pBuf, U32 BufSize)
{
	I2C_GpioHandle *plHandle = (I2C_GpioHandle *)Handle;
	int i;

	if (plHandle)
	{
		I2C_GpioStart(plHandle);
		if (!I2C_GpioSend8Bit(plHandle, (DevAddr & 0xFF) | 0x01))
		{
			GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
			I2C_GpioStop(plHandle);
			return -1;
		}
		for(i=0; i<BufSize-1; i++)
		{
			I2C_GpioRead8Bit(plHandle, &pBuf[i]);
			I2C_GpioSendAck(plHandle);
		}
		I2C_GpioRead8Bit(plHandle, &pBuf[i]);
		I2C_GpioSendNoAck(plHandle);
		I2C_GpioStop(plHandle);

		return BufSize;
	}

	return -1;
}

int I2C_GpioTwoStepWrite(HANDLE32 Handle, U8 DevAddr, char *pBuf, U32 BufSize)
{
	I2C_GpioHandle *plHandle = (I2C_GpioHandle *)Handle;
	int i;

	if (plHandle)
	{
		I2C_GpioStart(plHandle);
		if (!I2C_GpioSend8Bit(plHandle, DevAddr))
		{
			GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
			I2C_GpioStop(plHandle);
			return -1;
		}
		for(i=0; i<BufSize; i++)
		{
			if (!I2C_GpioSend8Bit(plHandle, pBuf[i]))
			{
				GLOBAL_TRACE(("I2C Error!!! DevAddr:0x%x\n", DevAddr));
				I2C_GpioStop(plHandle);
				return -1;
			}
		}
		I2C_GpioStop(plHandle);

		return BufSize;
	}

	return -1;
}

void I2C_GpioDestroy(HANDLE32 Handle)
{
	I2C_GpioHandle *plHandle = (I2C_GpioHandle *)Handle;

	if (plHandle)
	{
		free(plHandle);
		plHandle = NULL;
	}
}
