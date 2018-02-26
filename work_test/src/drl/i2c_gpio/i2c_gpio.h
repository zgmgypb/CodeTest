#ifndef I2C_GPIO_H
#define I2C_GPIO_H

#include "global_def.h"
#include "platform_assist.h"

typedef struct {
	void (*m_SdaSetValue)(S32 Value, void *pUserParam);
	void (*m_SclSetValue)(S32 Value, void *pUserParam);
	void (*m_SdaSetup)(BOOL IsInput, BOOL IsPullUp, void *pUserParam);
	void (*m_SclSetup)(BOOL IsInput, BOOL IsPullUp, void *pUserParam);
	int (*m_SdaGetValue)(void *pUserParam);

	void *m_pUserParam;
}I2C_GpioInitParam;

HANDLE32 I2C_GpioCreate(I2C_GpioInitParam *pInitPara);
int I2C_GpioRead(HANDLE32 Handle, U8 DevAddr, U8 *pRegAddr, U32 RegAddrLen, char *pBuf, U32 BufSize);
int I2C_GpioWrite(HANDLE32 Handle, U8 DevAddr, U8 *pRegAddr, U32 RegAddrLen, char *pBuf, U32 BufSize, BOOL IsPageAlign, int PageSize);
int I2C_GpioTwoStepRead(HANDLE32 Handle, U8 DevAddr, char *pBuf, U32 BufSize); /* 分两步读/写操作是参考 ATBM8869 SDK 的 pdf 说明 */
int I2C_GpioTwoStepWrite(HANDLE32 Handle, U8 DevAddr, char *pBuf, U32 BufSize);
/* QN8007 芯片的 I2C 的读比较特别，没有专门写寄存器地址，是和读连起来的 */
int I2C_Gpio8007Read(HANDLE32 Handle, U8 DevAddr, U8 RegAddr, char *pBuf);
void I2C_GpioDestroy(HANDLE32 Handle);

#endif /* I2C_GPIO_H */