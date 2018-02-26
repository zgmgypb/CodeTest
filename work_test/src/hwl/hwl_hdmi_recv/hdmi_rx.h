#ifndef HDMI_RX_H
#define HDMI_RX_H

#include "global_def.h"
#include "platform_assist.h"
#include "libc_assist.h" 

typedef enum {
	HDMI_RX_EDID_PCM = 0,              
	HDMI_RX_EDID_AC3,   
	HDMI_RX_EDID_NONE,
	HDMI_RX_EDID_NUM
} HDMI_RxEdidType;

typedef enum {
	HDMI_RX_GPIO_CPU = 0,
	HDMI_RX_GPIO_FPGA_EXT,
	HDMI_RX_GPIO_NUM
} HDMI_RxGpioType;

typedef struct {
	HDMI_RxGpioType m_GpioType; /* GPIO 类型，CPU 自带还是扩展 */
	U32 m_GpioIndex;
	U32 m_Pin;
} HDMI_RxGpioPin;

typedef struct {
	void (*m_GpioSetValueCB)(HDMI_RxGpioPin *pPinAddr, S32 Value, void *pUserParam);
	S32 (*m_GpioGetValueCB)(HDMI_RxGpioPin *pPinAddr, void *pUserParam);
	void (*m_GpioSetupCB)(HDMI_RxGpioPin *pPinAddr, BOOL IsInput, BOOL IsPullUp, void *pUserParam);

	void *m_pUserParam;

	/* 引脚信息 */
	HDMI_RxGpioPin m_HdmiHpdPin; /* 热拔插引脚 */
	HDMI_RxGpioPin m_Adv7612I2cSdaPin;
	HDMI_RxGpioPin m_Adv7612I2cSclPin;
	HDMI_RxGpioPin m_Adv7612RstPin; /* HDMI 复位引脚 */
	HDMI_RxGpioPin m_EepromI2cSdaPin;
	HDMI_RxGpioPin m_EepromI2cSclPin;
	HDMI_RxGpioPin m_EepromWpPin;
} HDMI_RxInitParam;

typedef struct {
	S32	m_Brightness;	/* 亮度 */
	S32	m_Contrast;		/* 对比度 */
	S32	m_Saturation;	/* 饱和度 */
	S32	m_Hue;			/* 色度 */

	BOOL m_IsAc3Bypass;
} HDMI_RxCfgParam;

typedef struct {
	BOOL m_HdmiIsConnected;	/* HDMI 是否连接，连接将检测到 5V 的电压 */
	BOOL m_SignalIsLocked;	/* 信号是否锁定 TRUE: 锁定 FALSE: 未锁定 */
	S32 m_VideoStandard;	/* 视频制式 */
	S32 m_AudioSample;		/* 音频采样率 */
} HDMI_RxStatusParam;

HANDLE32 HDMI_RxCreate(HDMI_RxInitParam *pInitParam); /* HDMI 接收模块创建 */
void HDMI_RxDestroy(HANDLE32 Handle); /* HDMI 接收模块销毁 */
BOOL HDMI_RxSetParam(HANDLE32 Handle, HDMI_RxCfgParam *pCfgParam); /* 设置 HDMI 接收模块的参数 */
BOOL HDMI_RxGetStatus(HANDLE32 Handle, HDMI_RxStatusParam *pStatusParam); /* 获取 HDMI 接收模块的状态值 */
BOOL HDMI_RxDownloadEdid(HANDLE32 Handle, HDMI_RxEdidType EdidType); /* 下载 EDID 到 EEPROM 芯片 */
BOOL HDMI_RxGetHwIsOk(HANDLE32 Handle); /* 巡检硬件通信是否出错 */

#endif /* HDMI_RX_H */
