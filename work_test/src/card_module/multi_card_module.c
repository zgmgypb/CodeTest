/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "card_app.h"
#include "multi_card_pro100.h"
#include "multi_main_internal.h"
#include "multi_card_test.h"
#ifdef ENCODER_CARD_PLATFORM
/* Global Variables (extern)--------------------------------------------------- */
MULT_Handle *g_TestHandle = NULL;
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */

/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

/* 本地函数 --------------------------------------------------------------------------------------------------------------------------------------- */

BOOL MULTL_CARDModuleICPProsessSendCB(void *pUserParam, U8 *pData, S32 DataSize)
{
	return (HWL_FPGAWrite(pData, DataSize) == 0);
}


BOOL MULTL_CARDModuleMainCMDProcessCB(void *pUserParam, S32 MainCMDOPType, void *pOPParam, S32 OPParamSize)
{
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;
	if (plHandle)
	{
		switch (MainCMDOPType)
		{
		case CARD_MAIN_CMD_OP_TYPE_APPLY_REMUX:
			{
				MULTL_SetRemuxApplyMark(plHandle, FALSE);
			}
			break;
		case CARD_MAIN_CMD_OP_TYPE_SET_SAVE_MARK:
			{
				MULTL_SetSaveMark(plHandle, FALSE);
			}
			break;
		}
	}
}
/* API函数 ---------------------------------------------------------------------------------------------------------------------------------------- */
void MULT_CARDModuleInitiate(MULT_Handle *pHandle, BOOL bSubFlashUpgradeMode)
{
	CARD_InitParam lInitParam;
	GLOBAL_ZEROMEM(&lInitParam, sizeof(CARD_InitParam));
	lInitParam.m_PlatformType = CARD_PLATFORM_TYPE_GN1772_MAIN_BOARD;
	lInitParam.m_PlatformDevType = MULT_DEVICE_COMPLETE_TYPE;
	lInitParam.m_MaxModuleNum = MULT_CARD_SYSTEM_MAX_MODULE_NUM;
	lInitParam.m_MaxTypeNum = MULT_CARD_SYSTEM_MAX_TYPE_NUM;
	lInitParam.m_MPEG2DBHandle = pHandle->m_DBSHandle;
	GLOBAL_STRNCPY(lInitParam.m_pModuleFilePath, CARD_MODULE_FILE_STORAGE_PATHNAME, sizeof(lInitParam.m_pModuleFilePath));
	GLOBAL_STRNCPY(lInitParam.m_pModuleFileUnfoldPath, CARD_MODULE_FILE_UNFOLD_PATHNAME, sizeof(lInitParam.m_pModuleFileUnfoldPath));
	GLOBAL_STRNCPY(lInitParam.m_pModuleStorageMountPoint, CARD_MODULE_FILE_STORAGE_MOUNT_PORINT_PATHNAME, sizeof(lInitParam.m_pModuleStorageMountPoint));
	GLOBAL_SNPRINTF((lInitParam.m_pXMLParameterFilePathName, sizeof(lInitParam.m_pXMLParameterFilePathName), "%s%s", MULT_XML_BASE_DIR, CARD_MODULE_PARAMETER_XML_FILE));
	GLOBAL_SNPRINTF((lInitParam.m_pXMLInfoFilePathName, sizeof(lInitParam.m_pXMLInfoFilePathName), "%s%s", MULT_XML_BASE_DIR, CARD_MODULE_INFO_XML_FILE));
	lInitParam.m_pICPSendCB = MULTL_CARDModuleICPProsessSendCB;
	lInitParam.m_pMAINCMDSendCB = MULTL_CARDModuleMainCMDProcessCB;
	lInitParam.m_pUserParam = pHandle;

#ifndef DEBUG_MODE_FPGA_CONFIG_ONCE
	lInitParam.m_bForceFPGAConfig = TRUE;
#endif

	/*建立好目录*/
	PFC_System("mkdir -p %s", CARD_MODULE_FILE_STORAGE_PATHNAME);

	CARD_Initate(&lInitParam);


#ifdef CARD_TEST
	CARD_TESTRegister();
#else


#ifdef DEBUG_MODE_INIT_BUNNER_MODE
	CARD_SubFlashBurnerRegister();
#else
	if (bSubFlashUpgradeMode == FALSE)
	{
		MULT_CardVixsPro100Register();
	}
	else
	{
		CARD_SubFlashBurnerRegister();
	}
#endif

#endif

	//CARD_XMLProcess(NULL, NULL, CARD_XML_OP_TYPE_LOAD);

	CARD_Start();

	g_TestHandle = pHandle;
}


BOOL MULT_CARDModuleICPProsessRecv(U8 *pData, S32 DataSize)
{
	return CARD_ICPRecv(pData, DataSize);
}


void MULT_CARDModuleXMLGetProcess(MULT_Handle *pHandle, mxml_node_t* pXMLRoot)
{
	HANDLE32 lSaveHandle;

	lSaveHandle = (HANDLE32)XML_WarpNew(NULL);

	CARD_XMLProcess(pXMLRoot, XML_WarpGetRootHandle(lSaveHandle), CARD_XML_OP_TYPE_DYNAMIC);

	XML_WarpSaveToString(lSaveHandle, pHandle->m_pReplayXMLBuf, sizeof(pHandle->m_pReplayXMLBuf));

	XML_WarpFree(lSaveHandle);
}

void MULT_CARDModuleXMLPostProcess(MULT_Handle *pHandle, mxml_node_t* pXMLRoot)
{
	CARD_XMLProcess(pXMLRoot, NULL, CARD_XML_OP_TYPE_DYNAMIC);
}

BOOL MULT_CARDModuleIsBusy(void)
{
	return CARD_IsBuys();	
}

void MULT_CARDModuleTerminate(void)
{
	CARD_Stop();
	CARD_Terminate();
}
#endif


/*EOF*/
