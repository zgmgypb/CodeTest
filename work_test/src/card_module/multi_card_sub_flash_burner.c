/* Includes-------------------------------------------------------------------- */
#include "multi_main_internal.h"

#ifdef ENCODER_CARD_PLATFORM

#include "global_micros.h"
#include "platform_conf.h"
#include "libc_assist.h"
#include "platform_assist.h"
#include "card_app.h"
#include "multi_card_test.h"
#include "fpga_rs232_v2.h"
#include "fpga_gpio_v2.h"
#include "fpga_misc.h"
#include "fpga_spi.h"
#include "fpga_spi_flash.h"
#include "vixs_pro100.h"
#include "transenc.h"


/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define CARD_SUB_FLASH_BUNER_TASK_STATCK_SIZE				(1024*1024)
#define CARD_SUB_FLASH_BURNER_VERSION						("01.00")

#define CARD_SUB_FLASH_BURNER_PRO100_NUM					(3)
#define CARD_SUB_FLASH_BURNER_PRO100_FLASH_FILE_SIZE		(32 * 1024 * 1024)
#define CARD_SUB_FLASH_BURNER_RETRY_COUNT					(3)

/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef void (*MULT_CARDTRANS_CB)(S32 SlotIndex, S32 TsIndex, U16 PID);

typedef struct
{
	S32					m_ModuleType;
	MULT_CARDTRANS_CB	m_SampleCB;
	S32					m_ModuleSlot;
}CARD_SubFlashBurnerInitParam;

typedef enum
{
	CARD_SUBFLASH_BURNER_STATE_IDLE = 0,
	CARD_SUBFLASH_BURNER_STATE_BURN,
	CARD_SUBFLASH_BURNER_STATE_CHECK,
	CARD_SUBFLASH_BURNER_STATE_COMPLETE,
	CARD_SUBFLASH_BURNER_NUM
}CARD_SubFlashBurnerState;

typedef struct
{
	S32								m_SlotInd;
	HANDLE32						m_TaskHandle;
	BOOL							m_TaskMark;

	FSPIF_Param						m_SPIFlashParam;
	FSPIF_FlashStructure			m_SPIFlashStruct;
	HANDLE32						m_IOTaskHandle;
	HANDLE32						m_pParentPtr;
	BOOL							m_bBurnOK;//成功设置为TRUE，其它为FALSE；

	S32								m_CurBytes;
	S32								m_TotalBytes;

	U32								m_StartTick;;

	S32								m_State;//当前状态


	F64								m_AVGBandwidth;//Bytes/S
	F64								m_CurBandwidth;
	F64								m_Progress;//0.0~1.0
	F64								m_DuraionLeft;//Second
	F64								m_DurationTotal;//Second

	/*用于计算实时速度*/
	S32								m_LastTick;
	S32								m_LastBytes;


}CARD_SubFlashBurnerNode;

typedef struct
{
	CARD_SubFlashBurnerInitParam	m_InitParam;
	S32								m_SubNum;
	BOOL							m_TaskMark;
	HANDLE32						m_TaskHandle;

	HANDLE32						m_SPIHandle;
	CARD_SubFlashBurnerNode			*m_pBurnerNode;
	MULT_SUB_FLASH_INFO				m_SubFlashInfo;

}CARD_SubFlashBurnerHandle;

/* Private Variables (static)-------------------------------------------------- */
static U8 *s_pDataBuf = NULL;
static S32 s_FlashSize = 0;
static BOOL s_bFileLoadOK = FALSE;
static S32	s_CurWriteOPCount = 0;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

/* 本地函数 --------------------------------------------------------------------------------------------------------------------------------------- */

/*进度回调函数*/
BOOL CARD_SubFlashBurnerProgressCB(void *pUserParam, S32 CurBytes, S32 TotalBytes)
{
	BOOL lRet = FALSE;
	CARD_SubFlashBurnerNode *plNode = (CARD_SubFlashBurnerNode*) pUserParam;
	if (plNode)
	{
		plNode->m_CurBytes = CurBytes;
		plNode->m_TotalBytes = TotalBytes;

		if (plNode->m_TaskMark == FALSE)
		{
			lRet = FALSE;
		}
		else
		{
			lRet = TRUE;
		}
	}

	return lRet;
}

/*烧写任务*/
void CARD_SubFlashBurnerIOThread(void *pUserParam)
{
	CARD_SubFlashBurnerHandle *plHandle;
	CARD_SubFlashBurnerNode *plNode = (CARD_SubFlashBurnerNode*) pUserParam;

	if (plNode)
	{
		U32 lTick;
		S32 lDuration;
		S32 lModuleSlot;
		S32 lSlotInd;
		U32 lFinalCRC32;

		FSPIF_Identfication lID;
		FSPIF_StatusReg lStatus;
		FSPIF_FlagStatusReg lFStatus;

		plHandle = (CARD_SubFlashBurnerHandle*)plNode->m_pParentPtr;

		lModuleSlot = plHandle->m_InitParam.m_ModuleSlot;
		lSlotInd = plNode->m_SlotInd;

		PAL_TimeStart(&lTick);
		GLOBAL_TRACE(("Module %d, Chip [%d], IO Thread Enter At %08X !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", lModuleSlot, lSlotInd, lTick));

		/*选择并复位芯片*/
		//GLOBAL_TRACE(("Module %d, Chip [%d], Select And Reset ON\n", lModuleSlot, lSlotInd));
		FSPI_ChipConfig(plHandle->m_SPIHandle, lSlotInd, TRUE, TRUE);
		PFC_TaskSleep(100);
		//GLOBAL_TRACE(("Module %d, Chip [%d], Select And Reset OFF\n", lModuleSlot, lSlotInd));
		FSPI_ChipConfig(plHandle->m_SPIHandle, lSlotInd, FALSE, TRUE);
		PFC_TaskSleep(1000);
		if (FSPIF_ReadIdentification(&plNode->m_SPIFlashParam, lSlotInd, &lID))
		{
			GLOBAL_TRACE(("Module %d, Chip [%d], Manufacturer = %02X, Type = %02X, Capacity = %02X\n", lModuleSlot, lSlotInd, lID.m_Manufacturer_ID, lID.m_DeviceType, lID.m_DeviceCapacity));
			plNode->m_CurBytes = 0;
			plNode->m_LastBytes = 0;
			/*对FLASH进行编程！*/
			PAL_TimeStart(&plNode->m_StartTick);
			s_CurWriteOPCount++;
			if (FSPIF_FLASHProgram(&plNode->m_SPIFlashParam, &plNode->m_SPIFlashStruct, lSlotInd, 0x00000000, plHandle->m_SubFlashInfo.m_pDataPtr, plHandle->m_SubFlashInfo.m_DataSize, FALSE, NULL, CARD_SubFlashBurnerProgressCB, plNode, CARD_SUB_FLASH_BURNER_RETRY_COUNT) == TRUE)
			{
				s_CurWriteOPCount--;
				while(plNode->m_TaskMark)
				{
					if (s_CurWriteOPCount == 0)
					{
						GLOBAL_TRACE(("Module %d, Chip [%d], Start CheckSUM!\n", lModuleSlot, lSlotInd));
						plNode->m_CurBytes = 0;
						plNode->m_LastBytes = 0;
						plNode->m_State = CARD_SUBFLASH_BURNER_STATE_CHECK;
						PAL_TimeStart(&plNode->m_StartTick);
						if (FSPIF_FLASHCheckSum(&plNode->m_SPIFlashParam, &plNode->m_SPIFlashStruct, lSlotInd, 0x00000000, plHandle->m_SubFlashInfo.m_DataSize, &lFinalCRC32, NULL, CARD_SubFlashBurnerProgressCB, plNode, CARD_SUB_FLASH_BURNER_RETRY_COUNT))
						{
							if (lFinalCRC32 == plHandle->m_SubFlashInfo.m_DataCRC32)
							{
								plNode->m_bBurnOK = TRUE;
								GLOBAL_TRACE(("Module %d, Chip [%d], CheckSUM OK!!! Check/ORI 0x%08X/0x%08X\n", lModuleSlot, lSlotInd, lFinalCRC32, plHandle->m_SubFlashInfo.m_DataCRC32));
							}
							else
							{
								GLOBAL_TRACE(("Module %d, Chip [%d], CheckSUM Error!!! Check/ORI 0x%08X/0x%08X\n", lModuleSlot, lSlotInd, lFinalCRC32, plHandle->m_SubFlashInfo.m_DataCRC32));
							}
						}
						else
						{
							GLOBAL_TRACE(("Module %d, Chip [%d], CheckSUM Function Error!!! \n", lModuleSlot, lSlotInd));
						}

						break;
					}
					else
					{
						PFC_TaskSleep(1000);
					}
				}
			}
			else
			{
				GLOBAL_TRACE(("Module %d, Chip [%d], Program Function Error!!! \n", lModuleSlot, lSlotInd));
			}
		}
		else
		{
			GLOBAL_TRACE(("Module %d, Chip [%d], ReadID Function Error!!! \n", lModuleSlot, lSlotInd));
		}

		plNode->m_State = CARD_SUBFLASH_BURNER_STATE_COMPLETE;

		FSPI_ChipConfig(plHandle->m_SPIHandle, lSlotInd, FALSE, FALSE);
		//GLOBAL_TRACE(("Module %d, Chip [%d], DeSelected And Reset OFF\n", lModuleSlot, lSlotInd));

		GLOBAL_TRACE(("Module %d, Chip [%d], IO Thread Leave At %08X, Duration = %.1f S,\n", lModuleSlot, lSlotInd, PFC_GetTickCount(), ((PAL_TimeEnd(&lTick)) / 1000.0)));
	}
	else
	{
		GLOBAL_TRACE(("Module %d, Chip [%d], Error!!!!!!!!!!!!!!!!\n"));
	}
}

/* 线程函数 */
void CARD_SubFlashBurnerThread(void *pUserParam)
{
	CARD_SubFlashBurnerHandle *plHandle = (CARD_SubFlashBurnerHandle*) pUserParam;

	if (plHandle)
	{
		U32 lTick;
		CARD_SubFlashBurnerNode *plNode;

		GLOBAL_TRACE(("Module %d, Control Thread Enter At %08X !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot, PAL_TimeStart(&lTick)));

		plHandle->m_TaskMark = TRUE;
		while(plHandle->m_TaskMark)
		{		
			if (MULT_SubFlashGetFlashInfo(&plHandle->m_SubFlashInfo) == TRUE)
			{
				S32 i;

				for (i = 0; i < plHandle->m_SubNum; i++)
				{
					plNode = &plHandle->m_pBurnerNode[i];

					if (plNode->m_State == CARD_SUBFLASH_BURNER_STATE_IDLE)
					{
						/*打开烧写线程！*/
						plNode->m_TaskMark = TRUE;
						plNode->m_bBurnOK = FALSE;
						plNode->m_State = CARD_SUBFLASH_BURNER_STATE_BURN;
						plNode->m_TaskHandle = PFC_TaskCreate("Buner Task", CARD_SUB_FLASH_BUNER_TASK_STATCK_SIZE, CARD_SubFlashBurnerIOThread, 0, plNode);
					}
					else if ((plNode->m_State == CARD_SUBFLASH_BURNER_STATE_BURN) || (plNode->m_State == CARD_SUBFLASH_BURNER_STATE_CHECK))
					{
						F64 lCurDuration;

						plNode->m_DurationTotal = GLOBAL_NO_ZERO_VALUE(PAL_TimeEnd(&plNode->m_StartTick)) / 1000.0;
						plNode->m_AVGBandwidth = plNode->m_CurBytes / plNode->m_DurationTotal;
						plNode->m_Progress = plNode->m_CurBytes * 1.0 / plHandle->m_SubFlashInfo.m_DataSize;
						lCurDuration = PAL_TimeEnd(&plNode->m_LastTick) / 1000.0;
						PAL_TimeStart(&plNode->m_LastTick);
						plNode->m_CurBandwidth = (plNode->m_CurBytes - plNode->m_LastBytes) / lCurDuration;
						plNode->m_LastBytes = plNode->m_CurBytes;

						plNode->m_DuraionLeft = (plHandle->m_SubFlashInfo.m_DataSize - plNode->m_CurBytes) / GLOBAL_NO_ZERO_VALUE(plNode->m_CurBandwidth);

						if (plNode->m_State == CARD_SUBFLASH_BURNER_STATE_BURN)
						{
							GLOBAL_TRACE(("Module %d, Chip [%d], Burn , %5.1f %%, CurBand %5.3f MByte/S, Data %6.3f MByte, Time %3.0f/%4.0f S\n", plHandle->m_InitParam.m_ModuleSlot, i, plNode->m_Progress * 100, plNode->m_CurBandwidth / 1024 / 1024, plNode->m_CurBytes * 1.0 / 1024 / 1024, plNode->m_DurationTotal, plNode->m_DuraionLeft));
						}
						else
						{
							GLOBAL_TRACE(("Module %d, Chip [%d], Check, %5.1f %%, CurBand %5.3f MByte/S, Data %6.3f MByte, Time %3.0f/%4.0f S\n", plHandle->m_InitParam.m_ModuleSlot, i, plNode->m_Progress * 100, plNode->m_CurBandwidth / 1024 / 1024, plNode->m_CurBytes * 1.0 / 1024 / 1024, plNode->m_DurationTotal, plNode->m_DuraionLeft));
						}
					}
					else if (plNode->m_State == CARD_SUBFLASH_BURNER_STATE_COMPLETE)
					{
						if (plNode->m_TaskHandle)
						{
							plNode->m_Progress = 1;
							plNode->m_CurBandwidth = 0;
							plNode->m_DurationTotal = GLOBAL_NO_ZERO_VALUE(PAL_TimeEnd(&plNode->m_StartTick)) / 1000.0;
							plNode->m_DuraionLeft = 0;

							PFC_TaskWait(plNode->m_TaskHandle, GLOBAL_INVALID_INDEX);
							plNode->m_TaskHandle = NULL;
						}
					}
				}

				{
					S32 lCompleteCount, lBurnOKCount;

					lCompleteCount = 0;
					lBurnOKCount = 0;

					for (i = 0; i < plHandle->m_SubNum; i++)
					{
						plNode = &plHandle->m_pBurnerNode[i];
						if (plNode->m_State == CARD_SUBFLASH_BURNER_STATE_COMPLETE)
						{
							lCompleteCount++;
							if (plNode->m_bBurnOK == TRUE)
							{
								lBurnOKCount++;
							}

						}
					}

					if (lCompleteCount == plHandle->m_SubNum)
					{
						GLOBAL_TRACE(("Module %d, Burn Complete !!!!!!!!!!!!!!!!!! OK Count %d\n", plHandle->m_InitParam.m_ModuleSlot, lBurnOKCount));
						break;
					}
				}


			}
			PFC_TaskSleep(3000);
		}
		GLOBAL_TRACE(("Module %d, Control Thread Leave, Duration %.1f S\n", plHandle->m_InitParam.m_ModuleSlot, ((PAL_TimeEnd(&lTick)) / 1000.0)));
	}
	else
	{
		GLOBAL_TRACE(("Module %d, Error!!!!!!!!!!!!!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot));
	}
}


/* 创建句柄 */
HANDLE32 CARD_SubFlashBurnerCreate(CARD_SubFlashBurnerInitParam *pParam)
{
	CARD_SubFlashBurnerHandle *plHandle = (CARD_SubFlashBurnerHandle*) GLOBAL_ZMALLOC(sizeof(CARD_SubFlashBurnerHandle));
	if (plHandle)
	{
		S32 i;
		CARD_SubFlashBurnerNode *plNode;
		GLOBAL_MEMCPY(&plHandle->m_InitParam, pParam, sizeof(CARD_SubFlashBurnerInitParam));

		if (plHandle->m_InitParam.m_ModuleType == CARD_MODULE_TYPE_TRANSCODER_VIXS_PRO100)
		{
			plHandle->m_SubNum = CARD_SUB_FLASH_BURNER_PRO100_NUM;
		}
		else
		{
			plHandle->m_SubNum = 1;
		}

		plHandle->m_pBurnerNode = (CARD_SubFlashBurnerNode *)GLOBAL_ZMALLOC(sizeof(CARD_SubFlashBurnerNode) * plHandle->m_SubNum);


		{
			FSPI_InitParam lInitParam;
			GLOBAL_ZEROSTRUCT(lInitParam);
			GLOBAL_SNPRINTF((lInitParam.m_pHandleName, sizeof(lInitParam.m_pHandleName), "SPI M[%d]", plHandle->m_InitParam.m_ModuleSlot));
			lInitParam.m_SlotNum = plHandle->m_SubNum;
			lInitParam.m_pUserParam = plHandle;
			lInitParam.m_pICPSendCB = CARD_SubModuleICPSend;
			lInitParam.m_TimeoutMS = 1000;
			plHandle->m_SPIHandle = FSPI_Create(&lInitParam);
		}

		for (i = 0; i < plHandle->m_SubNum; i++)
		{
			plNode = &plHandle->m_pBurnerNode[i];
			plNode->m_SlotInd = i;
			plNode->m_pParentPtr = plHandle;
			plNode->m_State = CARD_SUBFLASH_BURNER_STATE_IDLE;

			{
				FSPIF_Param lParam;
				FSPIF_FlashStructure lFlash;

				GLOBAL_ZEROSTRUCT(lParam);

				lParam.m_MaxOnceReadSize = 512;
				lParam.m_MaxOnceWriteSize = 512;
				lParam.m_pSPICB = FSPI_WriteAndRead;
				lParam.m_pUserParam = plHandle->m_SPIHandle;

				lParam.m_bUseFastRead = TRUE;
				lParam.m_FastReadDummyClock = 8;

				GLOBAL_MEMCPY(&plNode->m_SPIFlashParam, &lParam, sizeof(FSPIF_Param));

				lFlash.m_PageSize = 256;
				lFlash.m_SectorSize = 64 * 1024;
				lFlash.m_TotalBytes = 32 * 1024 *1024;
				lFlash.m_EraseOperationTypeTimeMS = 700;
				lFlash.m_EraseOperationMaxTimeMS = 3000;
				lFlash.m_ProgramOperationTypeTimeMS = 1;
				lFlash.m_ProgramOperationMaxTimeMS = 100;

				GLOBAL_MEMCPY(&plNode->m_SPIFlashStruct, &lFlash, sizeof(FSPIF_FlashStructure));
			}
		}

		GLOBAL_TRACE(("Module %d, Burner Module For Type %d [SUB NUM %d]Create Succesfull!\n", plHandle->m_InitParam.m_ModuleSlot, plHandle->m_InitParam.m_ModuleType, plHandle->m_SubNum));
	}
	return plHandle;
}

/* 启动线程 */
void CARD_SubFlashBurnerStart(HANDLE32 Handle)
{
	CARD_SubFlashBurnerHandle *plHandle = (CARD_SubFlashBurnerHandle*)Handle;
	if (plHandle)
	{
		plHandle->m_TaskHandle = PFC_TaskCreate("Module Task", CARD_SUB_FLASH_BUNER_TASK_STATCK_SIZE, CARD_SubFlashBurnerThread, 0, plHandle);
	}
}

/* 关闭线程 */
void CARD_SubFlashBurnerStop(HANDLE32 Handle)
{
	CARD_SubFlashBurnerHandle *plHandle = (CARD_SubFlashBurnerHandle*)Handle;
	if (plHandle)
	{
		if (plHandle->m_TaskHandle)
		{
			S32 i;
			CARD_SubFlashBurnerNode *plNode;

			if (plHandle->m_TaskHandle)
			{
				plHandle->m_TaskMark = FALSE;
				if (PFC_TaskWait(plHandle->m_TaskHandle, GLOBAL_INVALID_INDEX))
				{
					GLOBAL_TRACE(("Module %d, Burner Control Task Stop!\n", plHandle->m_InitParam.m_ModuleSlot));
				}
				plHandle->m_TaskHandle = NULL;
			}

			for (i = 0; i < plHandle->m_SubNum; i++)
			{
				plNode = &plHandle->m_pBurnerNode[i];
				if (plNode->m_TaskHandle)
				{
					plNode->m_TaskMark = FALSE;
					if (PFC_TaskWait(plNode->m_TaskHandle, GLOBAL_INVALID_INDEX))
					{
						GLOBAL_TRACE(("Module %d, Chip[%d], Burner IO Task Stop!\n", plHandle->m_InitParam.m_ModuleSlot, plNode->m_SlotInd));
					}
					plNode->m_TaskHandle = NULL;
				}
			}
		}
	}
}


/* 模块销毁 */
void CARD_SubFlashBurnerDestroy(HANDLE32 Handle)
{
	CARD_SubFlashBurnerHandle *plHandle = (CARD_SubFlashBurnerHandle*)Handle;
	if (plHandle)
	{
		CARD_SubFlashBurnerStop(Handle);

		GLOBAL_SAFEFREE(plHandle->m_pBurnerNode);

		FSPI_Destroy(plHandle->m_SPIHandle);

		GLOBAL_SAFEFREE(plHandle);
	}
}


/*CARD_SYSTEM 接口*/
BOOL CARD_SubFlashBurnerCMDProcessCB(void *pUserParam, S32 CMDOPType, void *pOPParam, S32 OPParamSize)
{
	BOOL lRet = FALSE;

	if (CMDOPType != CARD_CMD_OP_TYPE_CHECK_BUSY)
	{
		GLOBAL_TRACE(("CMD Type = %d\n", CMDOPType));
	}

	if (CMDOPType == CARD_CMD_OP_TYPE_CREATE)
	{
		HANDLE32 lHandle;

		CARD_SubFlashBurnerInitParam lParam;
		GLOBAL_ZEROSTRUCT(lParam);

		lParam.m_ModuleSlot = *((S32*)pOPParam);
		CARD_SubModuleType(lParam.m_ModuleSlot, &lParam.m_ModuleType);
		lHandle = CARD_SubFlashBurnerCreate(&lParam);

		(*(void **)pUserParam) = lHandle;

		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_PREPARE)
	{
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_CHECK_OK)
	{
		(*(S32*)pOPParam) = 1;
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_GET_FIRMWARE_VERSION)
	{
		GLOBAL_STRNCPY((CHAR_T*)pOPParam, CARD_SUB_FLASH_BURNER_VERSION, OPParamSize - 1);
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_APPLY)
	{
		CARD_SubFlashBurnerStart(pUserParam);
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_DESTROY)
	{
		CARD_SubFlashBurnerDestroy(pUserParam);
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_GET_MODULE_INIT_DELAY_MS)
	{
		(*(S32*)pOPParam) = 10;
		lRet = TRUE;
	}
	else
	{

	}
	return lRet;
}

BOOL CARD_SubFlashBurnerXMLProcessCB(void *pUserParam, HANDLE32 XMLLoad, HANDLE32 XMLSave, S32 XMLOPType)
{
	BOOL lRet = FALSE;
	CARD_SubFlashBurnerHandle *plHandle = (CARD_SubFlashBurnerHandle*)pUserParam;
	if (plHandle)
	{
		if (XMLOPType == CARD_XML_OP_TYPE_DYNAMIC)
		{
			if (XMLLoad)
			{
				HANDLE32 lParamHolder, lSubHolder;
				CHAR_T *plSubModuleInfo;
				S32 i;

				plSubModuleInfo = XML_WarpGetNodeText(XMLLoad, CARD_XML_SUB_MODULE_INFO_TAG, NULL);
				if (GLOBAL_STRCMP(plSubModuleInfo, "burner_info") == 0)
				{
					XML_WarpAddNodeS32(XMLSave, "module_type", plHandle->m_InitParam.m_ModuleType);
					XML_WarpAddNodeS32(XMLSave, "sub_chip_num", plHandle->m_SubNum);
					XML_WarpAddNodeS32(XMLSave, "flash_file_size", plHandle->m_SubFlashInfo.m_DataSize);
					XML_WarpAddNodeHEX(XMLSave, "flash_file_crc32", plHandle->m_SubFlashInfo.m_DataCRC32, TRUE);

					lRet = TRUE;
				}
				else if (GLOBAL_STRCMP(plSubModuleInfo, "progress_info") == 0)
				{
					CARD_SubFlashBurnerNode *plNode;
					for (i = 0; i < plHandle->m_SubNum; i++)
					{
						plNode = &plHandle->m_pBurnerNode[i];
						lSubHolder = XML_WarpAddNode(XMLSave, "progress_chip_info");
						XML_WarpAddNodeS32(lSubHolder, "state", plNode->m_State);
						if (plNode->m_State != CARD_SUBFLASH_BURNER_STATE_IDLE)
						{
							XML_WarpAddNodeF64(lSubHolder, "progress", plNode->m_Progress);
							XML_WarpAddNodeF64(lSubHolder, "avg_bandwidth", plNode->m_AVGBandwidth);
							XML_WarpAddNodeF64(lSubHolder, "cur_bandwidth", plNode->m_CurBandwidth);
							XML_WarpAddNodeF64(lSubHolder, "duration", plNode->m_DurationTotal);
							XML_WarpAddNodeF64(lSubHolder, "left_duration", plNode->m_DuraionLeft);
							XML_WarpAddNodeBOOL(lSubHolder, "check_ok", plNode->m_bBurnOK);
						}
					}
					lRet = TRUE;
				}
			}
		}
	}
	return lRet;
}

BOOL CARD_SubFlashBurnerICPProcessCB(void *pUserParam, U8 *pData, S32 DataSize)
{
	BOOL lRet = FALSE;
	CARD_SubFlashBurnerHandle *plHandle = (CARD_SubFlashBurnerHandle*)pUserParam;
	if (plHandle)
	{
		lRet = FSPI_ICPRecv(plHandle->m_SPIHandle, pData, DataSize);
		if (lRet == FALSE)
		{
			/*其它接口处理*/
		}
	}
	return lRet;
}

/* API函数 ---------------------------------------------------------------------------------------------------------------------------------------- */

void CARD_SubFlashBurnerRegister(void)
{
	S32 i;
	CARD_ModuleParam lParam;
	GLOBAL_ZEROSTRUCT(lParam);

	lParam.m_ModuleType = CARD_MODULE_TYPE_TRANSCODER_VIXS_PRO100;
	GLOBAL_STRCPY(lParam.m_pModuleControlVersion, "01.00");
	lParam.m_pCMDCB = CARD_SubFlashBurnerCMDProcessCB;
	lParam.m_pXMLCB = CARD_SubFlashBurnerXMLProcessCB;

	lParam.m_pICPRecvCB = CARD_SubFlashBurnerICPProcessCB;
	GLOBAL_STRCPY(lParam.m_pFirstFPGAName, "module_0001");
	lParam.m_FPGANum = 1;
	GLOBAL_STRCPY(lParam.m_pModuleTag, "sub_flash_burner");

	lParam.m_ChnInfo.m_ChnNum = 0;
	CARD_ModuleRegister(&lParam);
}

#endif

/*EOF*/
