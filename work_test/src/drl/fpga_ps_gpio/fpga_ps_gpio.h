#ifndef FPGA_PS_GPIO_H
#define FPGA_PS_GPIO_H

#include "global_def.h"
#include "platform_assist.h"

typedef struct {
	U32 m_GpioIndex;
	U32 m_Pin;
} FPS_GpioPin;

typedef struct {
	void (*m_GpioSetValueCB)(FPS_GpioPin *pPinAddr, S32 Value, void *pUserParam);
	S32 (*m_GpioGetValueCB)(FPS_GpioPin *pPinAddr, void *pUserParam);
	void (*m_GpioSetupCB)(FPS_GpioPin *pPinAddr, BOOL IsInput, BOOL IsPullUp, void *pUserParam);

	void *m_pUserParam;

	FPS_GpioPin m_DclkPinAddr;
	FPS_GpioPin m_DataPinAddr;
	FPS_GpioPin m_NStatusPinAddr;
	FPS_GpioPin m_NConfigPinAddr;
	FPS_GpioPin m_ConfigDonePinAddr;

	BOOL m_IsUseConfigDone;
}FPS_GpioCfgParam;

BOOL FPS_GpioConfig(CHAR_T *pRbfFile, FPS_GpioCfgParam *pCfgParam); /* 使用 GPIO 模拟 PS 时序配置 FPGA */
void FPS_CleanProg(FPS_GpioCfgParam *pCfgParam); /* 清除 FPGA 中的内容 */

#endif /* FPGA_PS_GPIO_H */
