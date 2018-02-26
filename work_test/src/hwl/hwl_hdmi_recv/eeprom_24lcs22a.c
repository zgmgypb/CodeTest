/*
EEPROM 24LCS22A读写文件，用于存储编码格式文件EDID，通过IIC操作
1 操作24LCS22A时，每次为了确保写入成功，需读出来进行对比

2014-08-22
1 调试24LCS22A，在配置正常的情况下，写操作没有问题；在读的时候经测试驱动不足，原因目前不详，取掉CM2031芯片通信正常。
2 关于IIC时序时间的控制问题，由于操作24LCS22A的频率低，需要保证数据，所以延时尽量长以保证读写正确。
3 写页写和字节写两种方式，一般采用页写方式。
4 空闲时候将scl和sda线置高，原因是这里有上拉，置低耗电，协议也规定置高。
5 读数据方式采用随机连续读方式。
*/

#include "hdmi_rx_inter.h"
#define DEV_ADDR_24LCS22A	0xA0

#define EEPROM_24LCS22A_PAGE_SIZE 8
	
/* 将模式切换到双向模式，原因是上电默认模式是transtion模式，参见24LC22A芯片的datasheet */
static void EEPROM_ChangeModeToBidirection(HDMI_RxHandle *pHandle)
{
	S32 i;
	HDMI_RxInitParam *plInitParam = &pHandle->m_InitParam;
	
	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSclPin, FALSE, TRUE, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSdaPin, FALSE, TRUE, plInitParam->m_pUserParam);

	plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromWpPin, LEVEL_HIGH, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromI2cSclPin, LEVEL_LOW, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromWpPin, LEVEL_LOW, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromI2cSclPin, LEVEL_HIGH, plInitParam->m_pUserParam);
	for (i=0; i<2; i++) {
		plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromWpPin, LEVEL_HIGH, plInitParam->m_pUserParam);
		plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromWpPin, LEVEL_LOW, plInitParam->m_pUserParam);
	}

	/* 变为永久Bidirection模式 */
	if (I2C_GpioWrite(pHandle->m_EdidI2cHandle, DEV_ADDR_24LCS22A, NULL, 0, NULL, 0, FALSE, 0) == -1) {
		pHandle->m_HwErrMark = TRUE;
	}
	else {
		pHandle->m_HwErrMark = FALSE;
	}
	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSclPin, TRUE, TRUE, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSdaPin, TRUE, TRUE, plInitParam->m_pUserParam);
}

BOOL EEPROM_DownLoadEdid(HDMI_RxHandle *pHandle, U8 *pBuffer, S32 BufLen)
{
	S32 i;
	U8 plRecvBuffer[256];
	HDMI_RxInitParam *plInitParam = &pHandle->m_InitParam;
	U8 lRegAddr = 0;
	BOOL lRet = TRUE;

	plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromWpPin, LEVEL_HIGH, plInitParam->m_pUserParam); /* 设置写保护释放 */
	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSclPin, FALSE, TRUE, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSdaPin, FALSE, TRUE, plInitParam->m_pUserParam);

	if (I2C_GpioWrite(pHandle->m_EdidI2cHandle, DEV_ADDR_24LCS22A, &lRegAddr, 1, pBuffer, BufLen, TRUE, EEPROM_24LCS22A_PAGE_SIZE) == -1) {
		pHandle->m_HwErrMark = TRUE;
		return FALSE;
	}
	else {
		pHandle->m_HwErrMark = FALSE;
	}
	if (I2C_GpioRead(pHandle->m_EdidI2cHandle, DEV_ADDR_24LCS22A, &lRegAddr, 1, plRecvBuffer, BufLen) == -1) {
		pHandle->m_HwErrMark = TRUE;
		return FALSE;
	}
	else {
		pHandle->m_HwErrMark = FALSE; 
	}
	if (memcmp(plRecvBuffer, pBuffer, BufLen) != 0) {
		GLOBAL_TRACE (("download EDID error!\n"));
		CAL_PrintDataBlock("Expect Data", pBuffer, BufLen);
		CAL_PrintDataBlock("Real Data", plRecvBuffer, BufLen);
		lRet = FALSE;
	}

	plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromWpPin, LEVEL_LOW, plInitParam->m_pUserParam);
	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSclPin, TRUE, TRUE, plInitParam->m_pUserParam); /* I2C操作完成后设置为高阻，不影响HDMI的读写 */
	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSdaPin, TRUE, TRUE, plInitParam->m_pUserParam);

	return lRet;
}

void EEPROM_Init(HDMI_RxHandle *pHandle)
{
	EEPROM_ChangeModeToBidirection(pHandle);
}
/* EOF */
