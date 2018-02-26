#include "hdmi_rx_inter.h"

#define Array_Max_Size (256)
static U8 s_DataNone[Array_Max_Size] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static U8 s_DataAC3Bypass[Array_Max_Size] = {
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x05, 0xCC, 0x90, 0x21, 0x01, 0x00, 0x00, 0x00, 
	0x15, 0x0F, 0x01, 0x03, 0x80, 0x5D, 0x34, 0x96, 0x0A, 0xF3, 0x30, 0xA7, 0x54, 0x42, 0xAA, 0x26, 
	0x0F, 0x48, 0x4C, 0xFF, 0xFE, 0x80, 0x4C, 0xCF, 0x31, 0x5E, 0x45, 0x5E, 0x61, 0x59, 0x71, 0x40, 
	0x71, 0x4F, 0x81, 0x40, 0x81, 0x80, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 
	0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 
	0x10, 0x3E, 0x96, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x41, 
	0x4E, 0x58, 0x39, 0x30, 0x32, 0x31, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD, 
	0x00, 0x32, 0x55, 0x1F, 0x45, 0x0B, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x02, 
	0x02, 0x03, 0x22, 0x72, 0x26, 0x10, 0x7F, 0x10, 0x0F, 0x7F, 0x01, 0x49, 0x84, 0x03, 0x05, 0x13, 
	0x12, 0x01, 0x14, 0x10, 0x1F, 0x83, 0x01, 0x00, 0x00, 0x68, 0x03, 0x0C, 0x00, 0x10, 0x00, 0x38, 
	0x32, 0x00, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58, 0x2C, 0x25, 0x00, 0xC4, 0x8E, 
	0x21, 0x00, 0x00, 0x9E, 0xD6, 0x09, 0x80, 0xA0, 0x20, 0xE0, 0x2D, 0x10, 0x08, 0x60, 0x22, 0x00, 
	0x12, 0x8E, 0x21, 0x08, 0x08, 0x18, 0x01, 0x1D, 0x00, 0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8, 0x28, 
	0x55, 0x40, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 
	0x0C, 0x40, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6B
};

static U8 s_DataDownMix[Array_Max_Size] = {
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x05, 0xCC, 0x87, 0x75, 0x01, 0x00, 0x00, 0x00, 
	0x18, 0x11, 0x01, 0x03, 0x80, 0x47, 0x28, 0x96, 0x0A, 0xCA, 0xB0, 0xA7, 0x54, 0x42, 0xAA, 0x26, 
	0x0F, 0x48, 0x4C, 0xFF, 0xFF, 0x80, 0x4C, 0xC0, 0x31, 0x5E, 0x45, 0x5E, 0x61, 0x59, 0x71, 0x40, 
	0x71, 0x4F, 0x81, 0x40, 0x81, 0x80, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 
	0x45, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x02, 0x3A, 0x80, 0xD0, 0x72, 0x38, 0x2D, 0x40, 
	0x10, 0x2C, 0x45, 0x80, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x41, 
	0x4E, 0x58, 0x38, 0x37, 0x37, 0x35, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD, 
	0x00, 0x31, 0x3D, 0x0F, 0x50, 0x17, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x1C, 
	0x02, 0x03, 0x23, 0xF1, 0x4B, 0x05, 0x1F, 0x90, 0x14, 0x06, 0x04, 0x13, 0x02, 0x11, 0x15, 0x00, 
	0x23, 0x09, 0x07, 0x07, 0x6A, 0x03, 0x0C, 0x00, 0x10, 0x00, 0xA8, 0x2D, 0x00, 0x00, 0x00, 0x83, 
	0x01, 0x00, 0x00, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58, 0x2C, 0x25, 0x00, 0xC4, 
	0x8E, 0x21, 0x00, 0x00, 0x9E, 0x01, 0x1D, 0x80, 0xD0, 0x72, 0x1C, 0x16, 0x20, 0x10, 0x2C, 0x25, 
	0x80, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E, 
	0x28, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 
	0x10, 0x10, 0x3E, 0x96, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B
};

static void HDMI_RxAdv7612I2cDelay(void)
{
	//PL_TaskSleepUS(1);
}

static void HDMI_RxAdv7612SdaSetValue(S32 Value, void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	plInitParam->m_GpioSetValueCB(&plInitParam->m_Adv7612I2cSdaPin, Value, plInitParam->m_pUserParam);
	HDMI_RxAdv7612I2cDelay();
}

static void HDMI_RxAdv7612SclSetValue(S32 Value, void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	plInitParam->m_GpioSetValueCB(&plInitParam->m_Adv7612I2cSclPin, Value, plInitParam->m_pUserParam);
	HDMI_RxAdv7612I2cDelay();
}

static void HDMI_RxAdv7612SdaSetup(BOOL IsInput, BOOL IsPullUp, void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	plInitParam->m_GpioSetupCB(&plInitParam->m_Adv7612I2cSdaPin, IsInput, IsPullUp, plInitParam->m_pUserParam);
	HDMI_RxAdv7612I2cDelay();
}

static void HDMI_RxAdv7612SclSetup(BOOL IsInput, BOOL IsPullUp, void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	plInitParam->m_GpioSetupCB(&plInitParam->m_Adv7612I2cSclPin, IsInput, IsPullUp, plInitParam->m_pUserParam);
	HDMI_RxAdv7612I2cDelay();
}

static S32 HDMI_RxAdv7612SdaGetValue(void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	return plInitParam->m_GpioGetValueCB(&plInitParam->m_Adv7612I2cSdaPin, plInitParam->m_pUserParam);
}

static void HDMI_RxEepromI2cDelay(S32 Count)
{
	S32 i;

	while (Count--) {
		i = 10000;
		while (i--);
	}
}

static void HDMI_RxEepromSdaSetValue(S32 Value, void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromI2cSdaPin, Value, plInitParam->m_pUserParam);
	HDMI_RxEepromI2cDelay(2);
}

static void HDMI_RxEepromSclSetValue(S32 Value, void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	plInitParam->m_GpioSetValueCB(&plInitParam->m_EepromI2cSclPin, Value, plInitParam->m_pUserParam);
	HDMI_RxEepromI2cDelay(2);
}

static void HDMI_RxEepromSdaSetup(BOOL IsInput, BOOL IsPullUp, void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSdaPin, IsInput, IsPullUp, plInitParam->m_pUserParam);
	HDMI_RxEepromI2cDelay(80);
}

static void HDMI_RxEepromSclSetup(BOOL IsInput, BOOL IsPullUp, void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	plInitParam->m_GpioSetupCB(&plInitParam->m_EepromI2cSclPin, IsInput, IsPullUp, plInitParam->m_pUserParam);
	HDMI_RxEepromI2cDelay(80);
}

static S32 HDMI_RxEepromSdaGetValue(void *pUserParam)
{
	HDMI_RxInitParam *plInitParam = (HDMI_RxInitParam *)pUserParam;

	return plInitParam->m_GpioGetValueCB(&plInitParam->m_EepromI2cSdaPin, plInitParam->m_pUserParam);
}

typedef struct {
	int fd;                       /*设备描述符*/
	U8 ChipAddr;                  /*芯片地址*/
	U8 RegAddr;                   /*寄存器地址*/
	U8 *DataBuffer;               /*读写数据指针*/
	U32 len;                      /*读写U8数据个数*/
} DeviceHndl;

static int HDMI_RxAdv7612Read(device_handle Handle, void *pUserParam)
{
	HDMI_RxHandle *plHandle = (HDMI_RxHandle *)pUserParam;
	HANDLE32 lI2cHandle = plHandle->m_Adv7612I2cHandle;
	DeviceHndl *plDevHandle = (DeviceHndl *)Handle;
	U8 lRegAddr;
	S32 i;

	for (i=0; i<plDevHandle->len; i++) {
		lRegAddr = plDevHandle->RegAddr + i;
		if (I2C_GpioRead(lI2cHandle, plDevHandle->ChipAddr, &lRegAddr, 1, plDevHandle->DataBuffer + i, 1) == -1) {
			plHandle->m_HwErrMark = TRUE;
		}
		else {
			plHandle->m_HwErrMark = FALSE;
		}
	}

	return 0;
}

static int HDMI_RxAdv7612Write(device_handle Handle, void *pUserParam)
{
	HDMI_RxHandle *plHandle = (HDMI_RxHandle *)pUserParam;
	HANDLE32 lI2cHandle = plHandle->m_Adv7612I2cHandle;
	DeviceHndl *plDevHandle = (DeviceHndl *)Handle;
	U8 lRegAddr;
	S32 i;

	//CAL_PrintDataBlock("I2c Write Data", plDevHandle->DataBuffer, plDevHandle->len);
	for (i=0; i<plDevHandle->len; i++) {
		lRegAddr = plDevHandle->RegAddr + i;
		if (I2C_GpioWrite(lI2cHandle, plDevHandle->ChipAddr, &lRegAddr, 1, plDevHandle->DataBuffer + i, 1, FALSE, 0) == -1) {
			plHandle->m_HwErrMark = TRUE;
		}
		else {
			plHandle->m_HwErrMark = FALSE;
		}
	}

	return 0;
}

#define ADV7612_IIC_DEV0                0x98
#define ADV7612_IIC_CP                  0x44
#define ADV7612_IIC_HDMI                0x68
#define ADV7612_IIC_EDID                0x6c
#define ADV7612_IIC_KSV                 0x64
#define ADV7612_IIC_DPLL                0x4c
#define ADV7612_IIC_INFOFRAME           0x7c
#define ADV7612_IIC_CEC                 0x80
/* 初始化 */
static BOOL HDMI_RxInit(HDMI_RxHandle *pHandle)
{
	HDMI_RxInitParam *plInitParam = &pHandle->m_InitParam;

	/* I2C 对象创建 */
	I2C_GpioInitParam lI2cInitParam;
	
	lI2cInitParam.m_pUserParam = &pHandle->m_InitParam;
	lI2cInitParam.m_SclSetup = HDMI_RxAdv7612SclSetup;
	lI2cInitParam.m_SclSetValue = HDMI_RxAdv7612SclSetValue;
	lI2cInitParam.m_SdaSetup = HDMI_RxAdv7612SdaSetup;
	lI2cInitParam.m_SdaGetValue = HDMI_RxAdv7612SdaGetValue;
	lI2cInitParam.m_SdaSetValue = HDMI_RxAdv7612SdaSetValue;
	pHandle->m_Adv7612I2cHandle = I2C_GpioCreate(&lI2cInitParam);
	if (!pHandle->m_Adv7612I2cHandle) {
		GLOBAL_TRACE(("I2c Create Failed!\n"));
		return FALSE;
	}

	lI2cInitParam.m_pUserParam = &pHandle->m_InitParam;
	lI2cInitParam.m_SclSetup = HDMI_RxEepromSclSetup;
	lI2cInitParam.m_SclSetValue = HDMI_RxEepromSclSetValue;
	lI2cInitParam.m_SdaSetup = HDMI_RxEepromSdaSetup;
	lI2cInitParam.m_SdaGetValue = HDMI_RxEepromSdaGetValue;
	lI2cInitParam.m_SdaSetValue = HDMI_RxEepromSdaSetValue;
	pHandle->m_EdidI2cHandle = I2C_GpioCreate(&lI2cInitParam);
	if (!pHandle->m_EdidI2cHandle) {
		I2C_GpioDestroy(pHandle->m_Adv7612I2cHandle);
		GLOBAL_TRACE(("I2c Create Failed!\n"));
		return FALSE;
	}

	/* 创建 ADV7612 对象 */
	adv7612_create_params lAdv7612CreateParam;
	ADV7612_DEV_ADDR *plAdv7612DevAddr = &lAdv7612CreateParam.device_addr;

	plAdv7612DevAddr->Io = ADV7612_IIC_DEV0;
	plAdv7612DevAddr->Cp = ADV7612_IIC_CP;
	plAdv7612DevAddr->Hdmi = ADV7612_IIC_HDMI;
	plAdv7612DevAddr->Cec = ADV7612_IIC_CEC;
	plAdv7612DevAddr->Edid = ADV7612_IIC_EDID;
	plAdv7612DevAddr->Infoframe = ADV7612_IIC_INFOFRAME;
	plAdv7612DevAddr->Rep = ADV7612_IIC_KSV;

	lAdv7612CreateParam.device_callbacks.device_read = HDMI_RxAdv7612Read;
	lAdv7612CreateParam.device_callbacks.device_write = HDMI_RxAdv7612Write;
	lAdv7612CreateParam.device_callbacks.m_pUserParam = (void *)pHandle;

	lAdv7612CreateParam.device_fd = 0;
	lAdv7612CreateParam.edid_type = 0; /* PCM */

	pHandle->m_Adv7612Handle = adv7612_create(&lAdv7612CreateParam);
	if (!pHandle->m_Adv7612Handle) {
		I2C_GpioDestroy(pHandle->m_Adv7612I2cHandle);
		I2C_GpioDestroy(pHandle->m_EdidI2cHandle);
		GLOBAL_TRACE(("adv76112 create failed!\n"));
		return FALSE;
	}

	/* EDID 芯片初始化 */
	EEPROM_Init(pHandle);

	return TRUE;
}

/* 反初始化 */
static void HDMI_RxTerminate(HDMI_RxHandle *pHandle)
{
	if (pHandle) {
		if (pHandle->m_Adv7612Handle) {
			adv7612_destroy(pHandle->m_Adv7612Handle);
			pHandle->m_Adv7612Handle = NULL;
		}
		
		if (pHandle->m_Adv7612I2cHandle) {
			I2C_GpioDestroy(pHandle->m_Adv7612I2cHandle);
			pHandle->m_Adv7612I2cHandle = NULL;
		}

		if (pHandle->m_EdidI2cHandle) {
			I2C_GpioDestroy(pHandle->m_EdidI2cHandle);
			pHandle->m_EdidI2cHandle = NULL;
		}
	}
}

static BOOL HDMI_RxReset(HANDLE32 Handle)
{
	HDMI_RxHandle *plHandle = (HDMI_RxHandle *)Handle;
	HDMI_RxInitParam *plInitParam = &plHandle->m_InitParam;

	if (!plHandle) {
		GLOBAL_TRACE(("Handle is NULL!\n"));
		return FALSE;
	}

	plInitParam->m_GpioSetValueCB(&plInitParam->m_Adv7612RstPin, LEVEL_LOW, plInitParam->m_pUserParam);
	PL_TaskSleepUS(10000);
	plInitParam->m_GpioSetValueCB(&plInitParam->m_Adv7612RstPin, LEVEL_HIGH, plInitParam->m_pUserParam);
	PL_TaskSleepUS(1000);

	return TRUE;
}

HANDLE32 HDMI_RxCreate(HDMI_RxInitParam *pInitParam)
{
	HDMI_RxHandle *plHandle = NULL;

	if (!pInitParam) {
		GLOBAL_TRACE(("pInitParam is NULL!\n"));
		return NULL;
	}

	/* 初始化参数检测 */
	if (!pInitParam->m_GpioSetValueCB || !pInitParam->m_GpioSetupCB || !pInitParam->m_GpioGetValueCB) {
		GLOBAL_TRACE(("Init Param Error!\n"));
		return NULL;
	}

	plHandle = (HDMI_RxHandle *)malloc(sizeof(HDMI_RxHandle));
	if (!plHandle) {
		GLOBAL_TRACE(("malloc failed!\n"));
		return NULL;
	}
	memset(plHandle, 0, sizeof(HDMI_RxHandle));
	memcpy(&plHandle->m_InitParam, pInitParam, sizeof(HDMI_RxInitParam));

	/* GPIO 引脚初始化 */
	pInitParam->m_GpioSetupCB(&pInitParam->m_Adv7612I2cSclPin, FALSE, TRUE, pInitParam->m_pUserParam);
	pInitParam->m_GpioSetupCB(&pInitParam->m_Adv7612I2cSdaPin, FALSE, TRUE, pInitParam->m_pUserParam);
	pInitParam->m_GpioSetupCB(&pInitParam->m_Adv7612RstPin, FALSE, TRUE, pInitParam->m_pUserParam);
	/* EEPROM 的 I2C 与 HDMI 接口的 I2C 挂在一组 I2C，所以默认设置为输入模式，不影响外部读取 EDID */
	pInitParam->m_GpioSetupCB(&pInitParam->m_EepromI2cSclPin, TRUE, TRUE, pInitParam->m_pUserParam); 
	pInitParam->m_GpioSetupCB(&pInitParam->m_EepromI2cSdaPin, TRUE, TRUE, pInitParam->m_pUserParam);
	pInitParam->m_GpioSetupCB(&pInitParam->m_EepromWpPin, FALSE, TRUE, pInitParam->m_pUserParam);
	pInitParam->m_GpioSetupCB(&pInitParam->m_HdmiHpdPin, FALSE, TRUE, pInitParam->m_pUserParam);

	/* 默认 HPD 输出为低（初始化完参数再拉高），WP 为低保护状态 */
	pInitParam->m_GpioSetValueCB(&pInitParam->m_HdmiHpdPin, LEVEL_LOW, pInitParam->m_pUserParam); 
	pInitParam->m_GpioSetValueCB(&pInitParam->m_EepromWpPin, LEVEL_LOW, pInitParam->m_pUserParam);

	/* ADV7612 引脚复位 */
	HDMI_RxReset(plHandle);

	/* 初始化 ADV7612 等硬件 */
	if (HDMI_RxInit(plHandle) == FALSE) {
		GLOBAL_TRACE(("HDMI Rx Module Init Failed!\n"));
		GLOBAL_SAFEFREE(plHandle);
		return NULL;
	}

	pInitParam->m_GpioSetValueCB(&pInitParam->m_HdmiHpdPin, LEVEL_HIGH, pInitParam->m_pUserParam); 

	return plHandle;
}

void HDMI_RxDestroy(HANDLE32 Handle)
{
	HDMI_RxHandle *plHandle = (HDMI_RxHandle *)Handle;

	if (plHandle) {
		HDMI_RxTerminate(plHandle);
		GLOBAL_SAFEFREE(plHandle);
	}
}

BOOL HDMI_RxSetParam(HANDLE32 Handle, HDMI_RxCfgParam *pCfgParam)
{
	HDMI_RxHandle *plHandle = (HDMI_RxHandle *)Handle;

	if (!plHandle) {
		GLOBAL_TRACE(("Handle is NULL!\n"));
		return FALSE;
	}

	plHandle->m_InitParam.m_GpioSetValueCB(&plHandle->m_InitParam.m_HdmiHpdPin, LEVEL_LOW, plHandle->m_InitParam.m_pUserParam); /* 设置参数触发一次热拔插 */

	/* 开启时设置亮度、色度、对比度、饱和度才生效 */
 	//adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_ADJUST_ENABLE, ADV7612_ADJUST_OPEN);
 	//adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_BRIGHTNESS, (unsigned long)pCfgParam->m_Brightness);
 	//adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_HUE, (unsigned long)pCfgParam->m_Hue);
 	//adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_CONTRAST, (unsigned long)pCfgParam->m_Contrast);
 	//adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_SATURATION, (unsigned long)pCfgParam->m_Saturation);

	/* 同步模式内同步 */
	//adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_SYNC_EXTERNAL_INTERNAL, (unsigned long)ADV7612_AVCODE_INTERNEL);
	if (pCfgParam->m_IsAc3Bypass) {
		adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_EDID_CTL, (unsigned long)ADV7612_EDID_AC3);
	}
	else {
		adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_EDID_CTL, (unsigned long)ADV7612_EDID_PCM);
	}

	/* 使用外部EDID，内同步 */
	adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_EDID_EXTERNAL_INTERNAL, (unsigned long)ADV7612_EDID_EXTERNEL);

	// 吴忠卫测试，1846 和 1866 设备上开启标清无输出，所以关闭
	// adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_LLC_DLL, 0x82); /* 该值是由王清龙测试确定的 */ 

	/* 使能均衡功能 */
	{
		int regs[3];
		regs[0] = 0x68; /* dev_addr */
		regs[1] = 0x96; /* reg_addr */
		regs[2] = 0x01; /* data */
		adv7612_set_params(plHandle->m_Adv7612Handle, ADV7612_SET_REG, (S32)regs);
	}

	plHandle->m_InitParam.m_GpioSetValueCB(&plHandle->m_InitParam.m_HdmiHpdPin, LEVEL_HIGH, plHandle->m_InitParam.m_pUserParam);

	return TRUE;
}

BOOL HDMI_RxGetStatus(HANDLE32 Handle, HDMI_RxStatusParam *pStatusParam)
{
	HDMI_RxHandle *plHandle = (HDMI_RxHandle *)Handle;
	S32 lClkLocked = 0, lHdmiConnected = 0;

	if (!plHandle) {
		GLOBAL_TRACE(("Handle is NULL!\n"));
		return FALSE;
	}

	adv7612_get_params(plHandle->m_Adv7612Handle, ADV7612_HPA_STATUS, (S32)&lHdmiConnected);
	adv7612_get_params(plHandle->m_Adv7612Handle, ADV7612_VID_STD, (S32)&pStatusParam->m_VideoStandard);
	adv7612_get_params(plHandle->m_Adv7612Handle, ADV7612_AUD_SAMPLE, (S32)&pStatusParam->m_AudioSample);
	adv7612_get_params(plHandle->m_Adv7612Handle, ADV7612_GET_CLK_LOCKED, (S32)&lClkLocked);
	pStatusParam->m_SignalIsLocked = ((lClkLocked == 0) ? FALSE : TRUE);
	pStatusParam->m_HdmiIsConnected = ((lHdmiConnected == 0) ? FALSE : TRUE);

	return TRUE;
}

BOOL HDMI_RxDownloadEdid(HANDLE32 Handle, HDMI_RxEdidType EdidType)
{
	HDMI_RxHandle *plHandle = (HDMI_RxHandle *)Handle;
	HDMI_RxInitParam *plInitParam = &plHandle->m_InitParam;
	BOOL lRet = TRUE;
	S32 i;
	U8 *plBuffer;

	if (!plHandle) {
		GLOBAL_TRACE(("Handle is NULL!\n"));
		return FALSE;
	}

	if (EdidType == HDMI_RX_EDID_PCM) {
		plBuffer = s_DataDownMix;
	}
	else if (EdidType == HDMI_RX_EDID_AC3) {
		plBuffer = s_DataAC3Bypass;
	}
	else {
		plBuffer = s_DataNone;
	}

	plInitParam->m_GpioSetValueCB(&plInitParam->m_HdmiHpdPin, LEVEL_LOW, plInitParam->m_pUserParam); /* 在操作EEPROM的时候禁止HDMI的操作 */
	lRet = EEPROM_DownLoadEdid(plHandle, plBuffer, Array_Max_Size);
	plInitParam->m_GpioSetValueCB(&plInitParam->m_HdmiHpdPin, LEVEL_HIGH, plInitParam->m_pUserParam);
	if (lRet) {
		GLOBAL_PRINTF(("download EDID OK!\n"));
	}

	return lRet;
}

BOOL HDMI_RxGetHwIsOk(HANDLE32 Handle)
{
	HDMI_RxHandle *plHandle = (HDMI_RxHandle *)Handle;

	return !plHandle->m_HwErrMark;
}
