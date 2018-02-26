#include "spi_gpio.h"

#define LEVEL_HIGH 1
#define LEVEL_LOW 0

typedef struct {
	SPI_GpioInitParam m_InitParam;
} SPI_GpioHandle;

static void SPI_GpioInit(SPI_GpioHandle *pHandle)
{
	SPI_GpioInitParam *plInitParam = &pHandle->m_InitParam;

	plInitParam->m_GpioSetupCB(&plInitParam->m_SdaOutPin, TRUE, TRUE, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetupCB(&plInitParam->m_CsPin, FALSE, TRUE, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetupCB(&plInitParam->m_SdaInPin, FALSE, TRUE, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetupCB(&plInitParam->m_SckPin, FALSE, TRUE, plInitParam->m_pUserParam);
}

S32 SPI_GpioRead(HANDLE32 Handle, U8 *pRegAddr, U32 RegAddrLen, char *pBuf, U32 BufSize)
{
	SPI_GpioHandle *plHandle = (SPI_GpioHandle *)Handle;
	SPI_GpioInitParam *pInitParam = &plHandle->m_InitParam;
	S32 i, j;
	U8 lCh;

	pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_HIGH, pInitParam->m_pUserParam);
	pInitParam->m_GpioSetValueCB(&pInitParam->m_CsPin, LEVEL_LOW, pInitParam->m_pUserParam);

	for (i = 0; i < RegAddrLen; i++) {
		lCh = pRegAddr[i];
		for(j = 0; j < 8; j++) { 	
			if( (lCh << j) & 0x80) {
				pInitParam->m_GpioSetValueCB(&pInitParam->m_SdaInPin, LEVEL_HIGH, pInitParam->m_pUserParam);
			}							
			else {
				pInitParam->m_GpioSetValueCB(&pInitParam->m_SdaInPin, LEVEL_LOW, pInitParam->m_pUserParam);
			}
			pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_LOW, pInitParam->m_pUserParam);
			pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_HIGH, pInitParam->m_pUserParam);
		}
	}

	for (i = 0; i < BufSize; i++) {
		lCh = 0;
		for (j = 0; j < 8; j++) {
			pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_LOW, pInitParam->m_pUserParam);
			pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_HIGH, pInitParam->m_pUserParam);
			lCh = (lCh << 1) | (0x01 & pInitParam->m_GpioGetValueCB(&pInitParam->m_SdaOutPin, pInitParam->m_pUserParam));
		}
		pBuf[i] = lCh;
	}

	pInitParam->m_GpioSetValueCB(&pInitParam->m_CsPin, LEVEL_HIGH, pInitParam->m_pUserParam);

	return BufSize;
}

S32 SPI_GpioWrite(HANDLE32 Handle, U8 *pRegAddr, U32 RegAddrLen, char *pBuf, U32 BufSize)
{
	SPI_GpioHandle *plHandle = (SPI_GpioHandle *)Handle;
	SPI_GpioInitParam *pInitParam = &plHandle->m_InitParam;
	S32 i, j;
	U8 lCh;

	pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_LOW, pInitParam->m_pUserParam);
	pInitParam->m_GpioSetValueCB(&pInitParam->m_CsPin, LEVEL_LOW, pInitParam->m_pUserParam);

	for (i = 0; i < RegAddrLen; i++) {
		lCh = pRegAddr[i];
		for(j = 0; j < 8; j++) { 	
			if( (lCh << j) & 0x80) {
				pInitParam->m_GpioSetValueCB(&pInitParam->m_SdaInPin, LEVEL_HIGH, pInitParam->m_pUserParam);
			}							
			else {
				pInitParam->m_GpioSetValueCB(&pInitParam->m_SdaInPin, LEVEL_LOW, pInitParam->m_pUserParam);	
			}

			pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_HIGH, pInitParam->m_pUserParam);
			pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_LOW, pInitParam->m_pUserParam);
		}
	}

	for (i = 0; i < BufSize; i++) {
		lCh = pBuf[i];
		for(j = 0; j < 8; j++) { 	
			if( (lCh << j) & 0x80) {
				pInitParam->m_GpioSetValueCB(&pInitParam->m_SdaInPin, LEVEL_HIGH, pInitParam->m_pUserParam);	
			}							
			else {
				pInitParam->m_GpioSetValueCB(&pInitParam->m_SdaInPin, LEVEL_LOW, pInitParam->m_pUserParam);	
			}

			pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_HIGH, pInitParam->m_pUserParam);	
			pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_LOW, pInitParam->m_pUserParam);	
		}
	}

	pInitParam->m_GpioSetValueCB(&pInitParam->m_CsPin, LEVEL_HIGH, pInitParam->m_pUserParam);	
	pInitParam->m_GpioSetValueCB(&pInitParam->m_SckPin, LEVEL_HIGH, pInitParam->m_pUserParam);	

	return BufSize;
}

HANDLE32 SPI_GpioCreate(SPI_GpioInitParam *pInitPara)
{
	SPI_GpioHandle *plHandle;

	if (!pInitPara) {
		GLOBAL_TRACE(("pInitPara is NULL!\n"));
		return NULL;
	}

	if (!pInitPara->m_GpioGetValueCB || !pInitPara->m_GpioSetupCB
		|| !pInitPara->m_GpioSetValueCB) {
		GLOBAL_TRACE(("pInitPara Param Error!\n"));
		return NULL;
	}

	plHandle = (SPI_GpioHandle *)GLOBAL_MALLOC(sizeof(SPI_GpioHandle));
	if (!plHandle) {
		GLOBAL_TRACE(("malloc error!\n"));
		return NULL;
	}

	GLOBAL_MEMCPY(&plHandle->m_InitParam, pInitPara, sizeof(SPI_GpioInitParam));

	SPI_GpioInit(plHandle);

	return plHandle;
}

void SPI_GpioDestroy(HANDLE32 Handle)
{
	SPI_GpioHandle *plHandle = (SPI_GpioHandle *)Handle;

	if (plHandle) {
		free(plHandle);
		plHandle = NULL;
	}
}

/* EOF */
