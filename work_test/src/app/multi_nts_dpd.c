/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_tsp.h"

//#define AVIOD_RESET_NTS_DPD_MODULE

#ifdef SUPPORT_NTS_DPD_BOARD
#include "hwl_dpd_control.h"
/* Global Variables (extern)--------------------------------------------------- */
#define NTS_DPD_RUNTIME_UPGRADE_FIRMWARE
#define NTS_DPD_RUNTIME_UPGRADE_DPD_TABLE

#define NTS_DPD_TABLE_FILE				("/mnt/mtd/nts_dpd.bin")
/* Macro (local)--------------------------------------------------------------- */
#define NTS_DPD_STATUS_TIMEOUT			(5000)
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */


typedef struct  
{
	MULT_Handle				*m_pMainHandle;

	DPD_CONTROL_BOARD_INFO	m_BoardInfo;
	DPD_CONTROL_DPD_PARAM	m_Param;
	DPD_CONTROL_STATUS		m_Status;

	BOOL					m_bBoardDetected;
	S32						m_StatusTimeout;

	HANDLE32				m_StatusLock;

	BOOL					m_ThreadMark;
	HANDLE32				m_ThreadHandle;


	BOOL					m_bNeedSaveDPDParam;
	U8						m_LastDPDTrackStatus;



	S32						m_DPDTableSaveErrorCount;
}MULT_DPD_HANDLE;

/* Private Variables (static)-------------------------------------------------- */
static MULT_DPD_HANDLE s_NTSDPDHandle;
/* Private Function prototypes ------------------------------------------------ */
static void MULTL_NTSDPDMonitorThread(void *pUserParam);
/* Functions ------------------------------------------------------------------ */

void MULT_NTSDPDInitiate(MULT_Handle *pHandle)
{
	U32 lTickCount;
	GLOBAL_ZEROMEM(&s_NTSDPDHandle, sizeof(MULT_DPD_HANDLE));

	s_NTSDPDHandle.m_pMainHandle = pHandle;

	/*设置初始化参数*/
	s_NTSDPDHandle.m_Param.m_DPDMark = FALSE;
	s_NTSDPDHandle.m_Param.m_DPDTrackResetMark =  FALSE;
	s_NTSDPDHandle.m_Param.m_DPDFeedbackSelect = 0;

	s_NTSDPDHandle.m_StatusLock = PFC_SemaphoreCreate(__FUNCTION__, 1);
	PFC_SemaphoreSignal(s_NTSDPDHandle.m_StatusLock);
#ifdef SUPPORT_NEW_FRP_MENU_MODULE
	MULT_FRPMenuADVShowInitateProgressString("INIT EX MODULE S2");
#endif
	GLOBAL_TRACE(("Delay For DPD Board Bootup Time 1 !!!\n"));
	PFC_TaskSleep(5000);

	lTickCount = PFC_GetTickCount() + 60000;
	while(PFC_GetTickCount() < lTickCount)
	{
		if (DPD_ControlGetBoardInfoInfo(&s_NTSDPDHandle.m_BoardInfo) == TRUE)
		{
			break;
		}
		else
		{
			GLOBAL_TRACE(("Get BoardInfo Failed, Time Left = %d S\n", (lTickCount - PFC_GetTickCount()) / 1000));
		}
	}

#ifndef AVIOD_RESET_NTS_DPD_MODULE

#ifdef SUPPORT_NEW_FRP_MENU_MODULE
	MULT_FRPMenuADVShowInitateProgressString("INIT EX MODULE S3");
#endif

#ifdef NTS_DPD_RUNTIME_UPGRADE_FIRMWARE
	DPD_ControlSendData("/tmp/nts.tgz", 0x01);

#ifdef SUPPORT_NEW_FRP_MENU_MODULE
	MULT_FRPMenuADVShowInitateProgressString("INIT EX MODULE S4");
#endif

	GLOBAL_TRACE(("Delay For DPD Board Bootup Time 2 !!!\n"));
	PFC_TaskSleep(20000);

	lTickCount = PFC_GetTickCount() + 60000;
	while(PFC_GetTickCount() < lTickCount)
	{
		if (DPD_ControlGetBoardInfoInfo(&s_NTSDPDHandle.m_BoardInfo) == TRUE)
		{
			break;
		}
		else
		{
			GLOBAL_TRACE(("Get BoardInfo Failed, TimeLeft = %d S\n", (lTickCount - PFC_GetTickCount()) / 1000));
		}
	}
#endif

#endif

#ifdef NTS_DPD_RUNTIME_UPGRADE_DPD_TABLE
	DPD_ControlSendData(NTS_DPD_TABLE_FILE, 0x02);
#endif
		
	s_NTSDPDHandle.m_bBoardDetected = TRUE;

	s_NTSDPDHandle.m_ThreadHandle = PFC_TaskCreate(__FUNCTION__, 4096, MULTL_NTSDPDMonitorThread, 1, NULL);
}

/*参数读写，包括网页提交*/
void MULT_NTSDPDXMLLoad(mxml_node_t *pXMLRoot, BOOL bPost)
{
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLHolder;
	mxml_node_t *plXMLNodeHolder;

	if (bPost)
	{
		plXMLHolder = pXMLRoot;
	}
	else
	{
		plXMLHolder = mxmlFindElement(pXMLRoot, pXMLRoot, "dpd_setting", NULL, NULL, MXML_DESCEND_FIRST);
	}

	if (plXMLHolder)
	{
		DPD_CONTROL_DPD_PARAM *plDPDParam = &s_NTSDPDHandle.m_Param;
		plDPDParam->m_DPDMark = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "dpd_mark", FALSE);
		plDPDParam->m_DPDTrackResetMark = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "track_reset_mark", FALSE);
		plDPDParam->m_DPDFeedbackSelect = MULTL_XMLGetNodeINTDefault(plXMLHolder, "feedback_select", 10, 0);
	}
}

void MULT_NTSDPDXMLSave(mxml_node_t *pXMLRoot, BOOL bStat)
{
	CHAR_T plTmpStr[512];
	S32 lTmpStrLen;
	mxml_node_t *plXMLHolder;
	mxml_node_t *plXMLNodeHolder;

	if (bStat)
	{
		DPD_CONTROL_STATUS *plStatus = &s_NTSDPDHandle.m_Status;
		DPD_CONTROL_BOARD_INFO *plBoard = &s_NTSDPDHandle.m_BoardInfo;
		plXMLHolder = pXMLRoot;

		if (PFC_SemaphoreWait(s_NTSDPDHandle.m_StatusLock, 100))
		{
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_status", plStatus->m_DPDStatus);
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_track_status", plStatus->m_DPDTrackStatus);
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_feedback_status", plStatus->m_DPDFeedebackStatus);
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_feedback_level", plStatus->m_DPDFeedebacklevel);
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_board_temp", plStatus->m_DPDBoardTemp);
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_clk_status", plStatus->m_DPDClkStatus);
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_io_status", plStatus->m_DPDIOStatus);
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_tx_power", plStatus->m_DPDTxPower);
			MULTL_XMLAddNodeFLOAT(plXMLHolder, "dpd_feedback_dsn", ((F64)plStatus->m_DPDSN / 100));
			MULTL_XMLAddNodeUINT(plXMLHolder, "dpd_run_flag", plStatus->m_DPDRunFlag);

			PFC_SemaphoreSignal(s_NTSDPDHandle.m_StatusLock);
		}

		GLOBAL_SPRINTF((plTmpStr, "%d-%.2d-%.2d", plBoard->m_PackageYear, plBoard->m_PackageMonth, plBoard->m_PackageDay));
		MULTL_XMLAddNodeText(plXMLHolder, "package_release", plTmpStr);
		GLOBAL_SPRINTF((plTmpStr, "%d-%.2d-%.2d", plBoard->m_FPGAYear, plBoard->m_FPGAMonth, plBoard->m_FPGADay));
		MULTL_XMLAddNodeText(plXMLHolder, "fpga_release", plTmpStr);
		GLOBAL_SPRINTF((plTmpStr, "%d-%.2d-%.2d", plBoard->m_HardwareYear, plBoard->m_HardwareMonth, plBoard->m_HardwareDay));
		MULTL_XMLAddNodeText(plXMLHolder, "hardware_release", plTmpStr);
		lTmpStrLen = CAL_StringBinToHex(plBoard->m_pBoardID, sizeof(plBoard->m_pBoardID), plTmpStr, sizeof(plTmpStr), TRUE);
		plTmpStr[lTmpStrLen] = 0;
		MULTL_XMLAddNodeText(plXMLHolder, "board_id", plTmpStr);
	}
	else
	{
		if (s_NTSDPDHandle.m_bBoardDetected)
		{
			DPD_CONTROL_DPD_PARAM *plDPDParam = &s_NTSDPDHandle.m_Param;
			plXMLHolder = mxmlNewElement(pXMLRoot, "dpd_setting");
			if (plXMLHolder)
			{
				MULTL_XMLAddNodeMark(plXMLHolder, "dpd_mark", plDPDParam->m_DPDMark);
				MULTL_XMLAddNodeUINT(plXMLHolder, "feedback_select", plDPDParam->m_DPDFeedbackSelect);
			}
		}
	}
}

/*SFN参数设置函数*/
void MULT_NTSDPDApplyParameter(void)
{
	TIME_T lTimeT;
	GLOBAL_TIME(&lTimeT);

	s_NTSDPDHandle.m_Param.m_TimeTick = lTimeT;

	DPD_ControlSetDPDParam(&s_NTSDPDHandle.m_Param);

	if (s_NTSDPDHandle.m_Param.m_DPDTrackResetMark == TRUE)
	{
		/*清除参数*/
		PFC_System("rm %s", NTS_DPD_TABLE_FILE);
		s_NTSDPDHandle.m_bNeedSaveDPDParam = TRUE;
	}

	/*自动恢复*/
	s_NTSDPDHandle.m_Param.m_DPDTrackResetMark = FALSE;
}

void MULT_NTSDPDGenerateDPDLogFile(void)
{
	PFC_System("rm %s", NTS_DPD_LOG_FILE_PATH_NAME);
	DPD_ControlRecvData(NTS_DPD_LOG_FILE_PATH_NAME, 0x03);
}

void MULT_NTSDPDSaveDPDTable(void)
{
#ifdef NTS_DPD_RUNTIME_UPGRADE_DPD_TABLE

	S32 lCount = 5;
	while(lCount > 0)
	{
		if (DPD_ControlRecvData(NTS_DPD_TABLE_FILE, 0x02))
		{
			GLOBAL_TRACE(("Save DPD Table File Successful!!!!!!!!!!!!!!!\n"));
			break;
		}
		lCount --;
	}

	if (lCount == 0)
	{
		s_NTSDPDHandle.m_DPDTableSaveErrorCount++;
		GLOBAL_TRACE(("Save DPD Table Error!!\n"));
	}
#endif
}

BOOL MULT_NTSDPDGetError(S32 ErrorType, BOOL bClear)
{
	BOOL lRet = FALSE; 
	if (s_NTSDPDHandle.m_DPDTableSaveErrorCount > 0)
	{
		lRet = TRUE;
	}

	if (bClear)
	{
		s_NTSDPDHandle.m_DPDTableSaveErrorCount = 0;
	}

	return lRet;
}

/*销毁当前模块*/
void MULT_NTSDPDTerminate(void)
{
	s_NTSDPDHandle.m_bBoardDetected = FALSE;

	s_NTSDPDHandle.m_ThreadMark = FALSE;

	if (PFC_TaskWait(s_NTSDPDHandle.m_ThreadHandle, 2000) == FALSE)
	{
		GLOBAL_TRACE(("Task Wait Failed!!!!!!!!\n"));
	}
	

	if (s_NTSDPDHandle.m_StatusLock)
	{
		PFC_SemaphoreDestroy(s_NTSDPDHandle.m_StatusLock);
		s_NTSDPDHandle.m_StatusLock = NULL;
	}
}

BOOL MULT_NTSDPDGetPLLError(void)
{
	return !s_NTSDPDHandle.m_Status.m_DPDStatus;
}

/*监控线程*/
void MULTL_NTSDPDMonitorThread(void *pUserParam)
{
	U32 lLastTick;
	DPD_CONTROL_STATUS lStatus;
	s_NTSDPDHandle.m_ThreadMark = TRUE;

	if (s_NTSDPDHandle.m_bBoardDetected == TRUE)
	{
		lLastTick = PFC_GetTickCount();
		while(s_NTSDPDHandle.m_ThreadMark)
		{
			s_NTSDPDHandle.m_StatusTimeout -= PAL_TimeDuration(&lLastTick, 1);
			if (s_NTSDPDHandle.m_StatusTimeout <= 0 || s_NTSDPDHandle.m_StatusTimeout > NTS_DPD_STATUS_TIMEOUT)
			{

				if (DPDL_ControlCheckCMDLock() == FALSE)
				{
					if (DPD_ControlGetDPDStatus(&lStatus) == FALSE)
					{
						GLOBAL_TRACE(("Get Status Failed!!!\n"));
					}
					else
					{
						if (PFC_SemaphoreWait(s_NTSDPDHandle.m_StatusLock, 1500))
						{
							GLOBAL_MEMCPY(&s_NTSDPDHandle.m_Status, &lStatus, sizeof(lStatus));
							PFC_SemaphoreSignal(s_NTSDPDHandle.m_StatusLock);
						}
					}

					//GLOBAL_TRACE(("L = %d, C = %d, SaveMark = %d\n", s_NTSDPDHandle.m_LastDPDTrackStatus, s_NTSDPDHandle.m_Status.m_DPDTrackStatus, s_NTSDPDHandle.m_bNeedSaveDPDParam));
					if ((s_NTSDPDHandle.m_LastDPDTrackStatus == 1) && (s_NTSDPDHandle.m_Status.m_DPDTrackStatus == 0))
					{
						if (s_NTSDPDHandle.m_bNeedSaveDPDParam == TRUE)
						{
							MULT_NTSDPDSaveDPDTable();
							s_NTSDPDHandle.m_bNeedSaveDPDParam = FALSE;
						}
					}
					s_NTSDPDHandle.m_LastDPDTrackStatus = s_NTSDPDHandle.m_Status.m_DPDTrackStatus;




				}

				s_NTSDPDHandle.m_StatusTimeout = NTS_DPD_STATUS_TIMEOUT;
			}
			PFC_TaskSleep(100);
		}
	}

}

BOOL MULT_NTSDPDProcParam(DPD_CONTROL_DPD_PARAM *pParam, BOOL bRead)
{
	BOOL lRet = FALSE;
	MULT_DPD_HANDLE *plHandle = &s_NTSDPDHandle;
	if (plHandle && pParam)
	{
		if (bRead)
		{
			GLOBAL_MEMCPY(pParam, &plHandle->m_Param, sizeof(DPD_CONTROL_DPD_PARAM));
		}
		else
		{
			GLOBAL_MEMCPY(&plHandle->m_Param, pParam, sizeof(DPD_CONTROL_DPD_PARAM));
		}
		lRet = TRUE;
	}
	return lRet;
}

BOOL MULT_NTSDPDProcStatus(DPD_CONTROL_STATUS *pStatus)
{
	BOOL lRet = FALSE;
	MULT_DPD_HANDLE *plHandle = &s_NTSDPDHandle;
	if (plHandle && pStatus)
	{
		GLOBAL_MEMCPY(pStatus, &plHandle->m_Status, sizeof(DPD_CONTROL_STATUS));
		lRet = TRUE;
	}
	return lRet;
}

#endif

/*EOF*/
