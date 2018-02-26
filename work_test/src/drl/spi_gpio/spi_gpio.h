#ifndef SPI_GPIO_H
#define SPI_GPIO_H

#include "global_def.h"
#include "platform_assist.h"

typedef struct {
	U32 m_GpioIndex;
	U32 m_Pin;
} SPI_GpioPin;

typedef struct {
	void (*m_GpioSetValueCB)(SPI_GpioPin *pPinAddr, S32 Value, void *pUserParam);
	S32 (*m_GpioGetValueCB)(SPI_GpioPin *pPinAddr, void *pUserParam);
	void (*m_GpioSetupCB)(SPI_GpioPin *pPinAddr, BOOL IsInput, BOOL IsPullUp, void *pUserParam);

	SPI_GpioPin m_SdaOutPin;
	SPI_GpioPin m_CsPin;
	SPI_GpioPin m_SdaInPin;
	SPI_GpioPin m_SckPin;

	void *m_pUserParam;
} SPI_GpioInitParam;

HANDLE32 SPI_GpioCreate(SPI_GpioInitParam *pInitPara);
S32 SPI_GpioRead(HANDLE32 Handle, U8 *pRegAddr, U32 RegAddrLen, char *pBuf, U32 BufSize);
S32 SPI_GpioWrite(HANDLE32 Handle, U8 *pRegAddr, U32 RegAddrLen, char *pBuf, U32 BufSize);
void SPI_GpioDestroy(HANDLE32 Handle);

#endif /* FPGA_SPI_GPIO_H */
