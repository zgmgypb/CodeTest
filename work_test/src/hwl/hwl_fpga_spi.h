#ifndef HWL_FPGA_SPI_H
#define HWL_FPGA_SPI_H

#include "global_def.h"
#include "platform_assist.h"
#include "libc_assist.h" 
#include "spi_gpio.h"
#include "hisi_gpio.h"

typedef enum {
	FPGA_OP_MODE_DEV_ID = 0x01,
	FPGA_OP_MODE_HW_INFO,
	FPGA_OP_MODE_SET_VOL,
	FPGA_OP_MODE_SET_EX_IO,
	FPGA_OP_MODE_GET_EX_IO,
	FPGA_OP_MODE_CLOSE = 0xFF
} FSPI_FpgaOpMode;

typedef enum {
	FSPI_EX_IO_0 = 0,
	FSPI_EX_IO_1,
	FSPI_EX_IO_2,
	FSPI_EX_IO_3,
	FSPI_EX_IO_4,
	FSPI_EX_IO_5,
	FSPI_EX_IO_6,
	FSPI_EX_IO_7,
	FSPI_EX_IO_8,
	FSPI_EX_IO_9,
	FSPI_EX_IO_10,
	FSPI_EX_IO_11,
	FSPI_EX_IO_NUM
} FSPI_EX_IO;

typedef enum {
	FSPI_CFG_VOL = 0,
	FSPI_CFG_EX_IO,
	FSPI_CFG_NUM
} FSPI_CFG_TAG;

typedef struct {
	S32 m_IoIndex;
	U8	m_IoDir; /* 0:out 1:in */
	U8	m_IoLvl; /* 0:low 1:high */
} FSPI_ExIo;

typedef struct {
	U8 m_Chn;
	U32 m_Vol;
} FSPI_VolInfo;

typedef struct {
	FSPI_CFG_TAG	m_CfgTag;
	union {
		FSPI_VolInfo m_Vol; /* 音量 */
		FSPI_ExIo m_ExIo;
	} m_CfgParam;
} FSPI_ConfigParam;

typedef enum {
	FSPI_STAT_DEV_ID = 0,
	FSPI_STAT_CARD_ID,
	FSPI_STAT_VER,
	FSPI_STAT_RELEASE,
	FSPI_STAT_TEMP,
	FSPI_STAT_EX_IO,
	FSPI_STAT_NUM
} FSPI_STAT_TAG;

typedef struct {
	FSPI_STAT_TAG	m_StatTag;
	union {
		U8 m_DevId;
		U32 m_CardId;
		U8 m_pVer[6]; /* 格式：1.0 */
		U8 m_pRelease[16]; /* 格式：2017.10.10 */
		F32 m_Temp; /* 温度 */
		FSPI_ExIo m_ExIo;
	} m_StatParam;
} FSPI_StatusParam;

HANDLE32 FSPI_Create(void);
HANDLE32 FSPI_GetHandle(void); /* 获取 SPI 通信的对象 */
void FSPI_Destroy(HANDLE32 FspiHandle);
BOOL FSPI_SetParam(HANDLE32 FspiHandle, FSPI_ConfigParam *pCfgParam);
BOOL FSPI_GetStatus(HANDLE32 FspiHandle, FSPI_StatusParam *pStatus);
U8 FSPI_GetFpgaHwVer(void);
BOOL FSPI_Write(S32 OpMode, U8 *pData, S32 DataLen);
BOOL FSPI_Read(S32 OpMode, U8 *pData, S32 DataLen);

#endif /* HWL_FPGA_SPI_H */
