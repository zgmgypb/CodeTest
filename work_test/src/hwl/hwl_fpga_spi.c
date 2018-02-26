#include "hwl_fpga_spi.h"
#include "multi_drv.h"

#ifdef GN1846

#define FPGA_OP_MODE_ADDR (0x4FF) /* FPGA 操作模式地址 */
#define FPGA_DATA_BASE_ADDR (0x400)

typedef struct {
	U8	m_Version;
	U8	m_pRelease[2];
	U8	m_pCardId[4];
	U8	m_pTemp[2];
} FSPI_FpgaHwInfo;

static U8 FSPI_GetDevId(HANDLE32 SpiHandle)
{
	U8 lValue;
	U8 lRetVal;

	lValue = FPGA_OP_MODE_DEV_ID;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
	DRL_FspiRead(FPGA_DATA_BASE_ADDR, &lRetVal, 1);
	lValue = FPGA_OP_MODE_CLOSE;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);

	return lRetVal;
}

static void FSPI_GetHwInfo(FSPI_FpgaHwInfo *pHwInfo)
{
	U8 lValue;

	lValue = FPGA_OP_MODE_HW_INFO;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
	DRL_FspiRead(FPGA_DATA_BASE_ADDR, &pHwInfo->m_Version, 1);
	DRL_FspiRead(FPGA_DATA_BASE_ADDR + 1, pHwInfo->m_pRelease, 2);
	DRL_FspiRead(FPGA_DATA_BASE_ADDR + 3, pHwInfo->m_pCardId, 4);
	DRL_FspiRead(FPGA_DATA_BASE_ADDR + 7, pHwInfo->m_pTemp, 2);
	lValue = FPGA_OP_MODE_CLOSE;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
}

static void FSPI_SetVol(HANDLE32 SpiHandle, U8 Chn, U32 Vol)
{
	U8 lValue;
	U8 plSetValue[5];

	lValue = FPGA_OP_MODE_SET_VOL;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
	plSetValue[0] = Chn;
	plSetValue[1] = (Vol >> 24) & 0xFF;
	plSetValue[2] = (Vol >> 16) & 0xFF;
	plSetValue[3] = (Vol >> 8) & 0xFF;
	plSetValue[4] = Vol & 0xFF;
	DRL_FspiWrite(FPGA_DATA_BASE_ADDR, plSetValue, 5);
	lValue = FPGA_OP_MODE_CLOSE;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
}

static void FSPI_SetExIo(HANDLE32 SpiHandle, U16 Dir, U16 Data)
{
	U8 lValue;
	U8 plSetValue[4];

	lValue = FPGA_OP_MODE_SET_EX_IO;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
	plSetValue[0] = (Dir >> 8) & 0xFF;
	plSetValue[1] = Dir & 0xFF;
	plSetValue[2] = (Data >> 8) & 0xFF;
	plSetValue[3] = Data & 0xFF;
	DRL_FspiWrite(FPGA_DATA_BASE_ADDR, plSetValue, 4);
	lValue = FPGA_OP_MODE_CLOSE;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
}

static void FSPI_GetExIo(HANDLE32 SpiHandle, U16 *pDir, U16 *pData)
{
	U8 lValue;
	U8 plGetValue[4];

	lValue = FPGA_OP_MODE_GET_EX_IO;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
	DRL_FspiRead(FPGA_DATA_BASE_ADDR, plGetValue, 4);
	*pDir = (plGetValue[0] << 8) | plGetValue[1];
	*pData = (plGetValue[2] << 8) | plGetValue[3];
	lValue = FPGA_OP_MODE_CLOSE;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
}

static void FSPI_PinDelay(void) 
{

}

static void FSPI_SetValue(SPI_GpioPin *pPinAddr, S32 Value, void *pUserParam)
{
	HI_GpioSetValue(pPinAddr->m_GpioIndex, pPinAddr->m_Pin, Value);
	FSPI_PinDelay();
}

static S32 FSPI_GetValue(SPI_GpioPin *pPinAddr, void *pUserParam)
{	
	return HI_GpioGetValue(pPinAddr->m_GpioIndex, pPinAddr->m_Pin);
}

static void FSPI_Setup(SPI_GpioPin *pPinAddr, BOOL IsInput, BOOL IsPullUp, void *pUserParam)
{
	HI_GpioSetup(pPinAddr->m_GpioIndex, pPinAddr->m_Pin, IsInput, IsPullUp);
	FSPI_PinDelay();
}


static HANDLE32 s_FspiHandle;
HANDLE32 FSPI_Create(void)
{
	if (!s_FspiHandle) {
		SPI_GpioInitParam lInitParam;

		lInitParam.m_CsPin.m_GpioIndex = HI_GPIO0;
		lInitParam.m_CsPin.m_Pin = HI_PIN4;
		lInitParam.m_SckPin.m_GpioIndex = HI_GPIO0;
		lInitParam.m_SckPin.m_Pin = HI_PIN1;
		lInitParam.m_SdaInPin.m_GpioIndex = HI_GPIO0;
		lInitParam.m_SdaInPin.m_Pin = HI_PIN3;
		lInitParam.m_SdaOutPin.m_GpioIndex = HI_GPIO0;
		lInitParam.m_SdaOutPin.m_Pin = HI_PIN2;
		lInitParam.m_GpioGetValueCB = FSPI_GetValue;
		lInitParam.m_GpioSetValueCB = FSPI_SetValue;
		lInitParam.m_GpioSetupCB = FSPI_Setup;
		lInitParam.m_pUserParam = NULL;
		if ((s_FspiHandle = SPI_GpioCreate(&lInitParam)) == NULL) {
			GLOBAL_TRACE(("SPI_GpioCreate Failed!\n"));
			return NULL;
		}
	}

	return s_FspiHandle;
}

HANDLE32 FSPI_GetHandle(void)
{
	return s_FspiHandle;
}

void FSPI_Destroy(HANDLE32 FspiHandle) 
{
	SPI_GpioDestroy(FspiHandle);
	s_FspiHandle = NULL;
}

BOOL FSPI_SetParam(HANDLE32 FspiHandle, FSPI_ConfigParam *pCfgParam)
{
	switch (pCfgParam->m_CfgTag) {
		case FSPI_CFG_VOL:
			FSPI_SetVol(FspiHandle, pCfgParam->m_CfgParam.m_Vol.m_Chn, pCfgParam->m_CfgParam.m_Vol.m_Vol);
			break;
		case FSPI_CFG_EX_IO:
			{
				U16 lDir = 0, lData = 0;
				
				FSPI_GetExIo(FspiHandle, &lDir, &lData);
				lDir = (lDir & ~(1 << pCfgParam->m_CfgParam.m_ExIo.m_IoIndex)) | 
					(pCfgParam->m_CfgParam.m_ExIo.m_IoDir << pCfgParam->m_CfgParam.m_ExIo.m_IoIndex);
				if (pCfgParam->m_CfgParam.m_ExIo.m_IoDir == 0) { /* 输出才设置 IO 口的数据 */
					lData = (lData & ~(1 << pCfgParam->m_CfgParam.m_ExIo.m_IoIndex)) | 
						(pCfgParam->m_CfgParam.m_ExIo.m_IoLvl << pCfgParam->m_CfgParam.m_ExIo.m_IoIndex);
				}
				// GLOBAL_TRACE(("ldir:0x%x ldata:0x%x\n", lDir, lData));
				FSPI_SetExIo(FspiHandle, lDir, lData);
			}
			break;
		default: break;
	}

	return TRUE;
}

BOOL FSPI_GetStatus(HANDLE32 FspiHandle, FSPI_StatusParam *pStatus)
{
	FSPI_FpgaHwInfo lHwInfo;
	U8 *p;

	switch (pStatus->m_StatTag) {
		case FSPI_STAT_DEV_ID:
			pStatus->m_StatParam.m_DevId = FSPI_GetDevId(FspiHandle);
			break;
		case FSPI_STAT_CARD_ID:
			FSPI_GetHwInfo(&lHwInfo);
			p = lHwInfo.m_pCardId;
			GLOBAL_MSB32_D(p, pStatus->m_StatParam.m_CardId);
			break;
		case FSPI_STAT_VER:
			FSPI_GetHwInfo(&lHwInfo);
			snprintf(pStatus->m_StatParam.m_pVer, 6, "V%d.%d", (lHwInfo.m_Version >> 4) & 0x0F, lHwInfo.m_Version & 0x0F);
			break;
		case FSPI_STAT_RELEASE:
			FSPI_GetHwInfo(&lHwInfo);
			snprintf(pStatus->m_StatParam.m_pRelease, 16, "%04d.%02d.%02d", ((lHwInfo.m_pRelease[0] >> 4) & 0x0F) + 2008,
				lHwInfo.m_pRelease[0] & 0x0F, lHwInfo.m_pRelease[1] & 0x1F);
			break;
		case FSPI_STAT_TEMP:
			FSPI_GetHwInfo(&lHwInfo);
			pStatus->m_StatParam.m_Temp = ((lHwInfo.m_pTemp[0] << 5) | ((lHwInfo.m_pTemp[1] >> 3) & 0xFF)) * 0.5;
			break;
		case FSPI_STAT_EX_IO:
			{
				U16 lDir = 0, lData = 0;

				FSPI_GetExIo(FspiHandle, &lDir, &lData);
				pStatus->m_StatParam.m_ExIo.m_IoLvl = (lData & (1 << pStatus->m_StatParam.m_ExIo.m_IoIndex) ? 1 : 0);
			}
			break;
		default:break;
	}

	return TRUE;
}

U8 FSPI_GetFpgaHwVer(void)
{
	FSPI_FpgaHwInfo lHwInfo;

	FSPI_GetHwInfo(&lHwInfo);
	return lHwInfo.m_Version;
}

BOOL FSPI_Write(S32 OpMode, U8 *pData, S32 DataLen)
{
	U8 lValue;

	lValue = OpMode;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
	if (DRL_FspiWrite(FPGA_DATA_BASE_ADDR, pData, DataLen) != DataLen) {
		return FALSE;
	}
	lValue = FPGA_OP_MODE_CLOSE;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);

	return TRUE;
}

BOOL FSPI_Read(S32 OpMode, U8 *pData, S32 DataLen)
{
	U8 lValue;

	lValue = OpMode;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);
	if (DRL_FspiRead(FPGA_DATA_BASE_ADDR, pData, DataLen) != DataLen) {
		return FALSE;
	}
	lValue = FPGA_OP_MODE_CLOSE;
	DRL_FspiWrite(FPGA_OP_MODE_ADDR, &lValue, 1);

	return TRUE;
}

#endif
