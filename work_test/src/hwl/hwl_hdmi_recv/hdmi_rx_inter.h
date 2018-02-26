#ifndef HDMI_RX_INTER_H
#define HDMI_RX_INTER_H

#include "hdmi_rx.h"
#include "adv7612.h"
#include "i2c_gpio.h"

#define LEVEL_HIGH	1
#define LEVEL_LOW	0

typedef struct {
	HDMI_RxInitParam m_InitParam;

	adv7612_handle m_Adv7612Handle;
	HANDLE32 m_Adv7612I2cHandle;
	HANDLE32 m_EdidI2cHandle;

	BOOL	m_HwErrMark; /* 通信不正常时报错 */
} HDMI_RxHandle;

void EEPROM_Init(HDMI_RxHandle *pHandle); 
BOOL EEPROM_DownLoadEdid(HDMI_RxHandle *pHandle, U8 *pBuffer, S32 BufLen);

#endif /* HDMI_RX_INTER_H */
