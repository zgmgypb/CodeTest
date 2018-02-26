/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"

#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "remote_control.h"
#include "platform_assist.h"
#include "multi_tsp.h"
#include "multi_hwl_internal.h"
#include "hwl_vrs232.h"


#ifdef SUPPORT_NTS_DPD_BOARD
#include "hwl_dpd_control.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
//#define DPD_DEBUG

#define DPD_VRS232_IND						(1)
#define DPD_VRS232_RECV_BUF_SIZE			(1024)
#define DPD_VRS232_SEND_BUF_SIZE			(1024)

#define DPD_CONTROL_IO_TIMEOUT				(1000)

#define DPD_CONTROL_MSG_ID_DATA_SEND		(0x0001)

#define DPD_CONTROL_MSG_ID_DATA_RECV		(0x0002)
#define DPD_CONTROL_MSG_ID_BOARD_INFO		(0x0003)
#define DPD_CONTROL_MSG_ID_RF_PARAM			(0x0004)
#define DPD_CONTROL_MSG_ID_DPD_SET			(0x0005)
#define DPD_CONTROL_MSG_ID_DPD_STATUS		(0x0006)

/*数据收发类型*/
#define DPD_CONTROL_DATA_TYPE_SOLID			(0x01)		
#define DPD_CONTROL_DATA_TYPE_DPD_TABLE		(0x02)		
#define DPD_CONTROL_DATA_TYPE_DPD_LOG		(0x03)
#define DPD_CONTROL_DATA_FRAME_MAX_SIZE		(996 - 8)	
#define DPD_CONTROL_MAX_RECV_FILE_SIZE		(4 * 1024 * 1024)


/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	HANDLE32		m_COMHandle;
	BOOL			m_ThreadMark;

	REMOTE_MSG		m_MSG;
	S32				m_LastLevel;

	HANDLE32		m_RecvBufHandle;

	U8				m_pRecvMsgBuf[DPD_VRS232_SEND_BUF_SIZE];
	S32				m_RecvCurLen;


	U8				m_pSendMsgBuf[DPD_VRS232_SEND_BUF_SIZE];

	S32				m_SendSize;

	HANDLE32		m_AccessLock;

	BOOL			m_CMDLock;
}DPD_CONTROL_HANDLE;

/* Private Variables (static)-------------------------------------------------- */
static DPD_CONTROL_HANDLE *s_pHandle = NULL;
/* Private Function prototypes ------------------------------------------------ */
static BOOL DPDL_ControlSendAndRecvMsgSYNC(S32 WaitTimeMS);
static void DPDL_ControlCOMOpen(void);
static void DPDL_ControlCOMFlush(void);
static S32 DPDL_ControlCOMWrite(U8 *pData, S32 DataLen);
static S32 DPDL_ControlCOMRead(U8 *pBuf, S32 BufLen);
static void DPDL_ControlCOMClose(void);
static void DPDL_Lock(void);
static void DPDL_UnLock(void);

/* Functions ------------------------------------------------------------------ */
void DPD_ControlInitiate(void)
{
	DPD_CONTROL_HANDLE *plHandle;
	plHandle = (DPD_CONTROL_HANDLE*)GLOBAL_ZMALLOC(sizeof(DPD_CONTROL_HANDLE));
	if (plHandle)
	{
		plHandle->m_AccessLock = PFC_SemaphoreCreate(__FUNCTION__, 1);
		PFC_SemaphoreSignal(plHandle->m_AccessLock);

		s_pHandle = plHandle;
	}

	DPDL_ControlCOMOpen();

	GLOBAL_TRACE(("Complete!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"))
}


BOOL DPD_ControlSendData(CHAR_T *pPathName, S32 Type)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	DPDL_Lock();
	if (plHandle && pPathName)
	{
		S32 lFileSize;
		U32 lFileCRC32;
		U8 *plFileBuf;
		plFileBuf = NULL;
		GLOBAL_FD lFileFD;
		lFileFD = GLOBAL_FOPEN(pPathName, "rb");
		if (lFileFD)
		{
			lFileSize = CAL_FileSize(lFileFD);
			plFileBuf = (U8 *)GLOBAL_ZMALLOC(lFileSize);
			if (plFileBuf)
			{
				GLOBAL_FREAD(plFileBuf, 1, lFileSize, lFileFD);
			}
			else
			{
				GLOBAL_TRACE(("Malloc FW File Buf Failed!!!!!!!!!\n"));
			}
			GLOBAL_FCLOSE(lFileFD);
			lFileFD = NULL;
		}

		if (plFileBuf && (lFileSize > 0))
		{
			REMOTE_MSG *plMsg;
			S32 i, lFrameNum, lFrameLeftSize;
			U8 *plTmpBuf;
			U32 lTimeTick, lReturnCRC32;

			plMsg = &plHandle->m_MSG;

			lFrameNum = lFileSize / DPD_CONTROL_DATA_FRAME_MAX_SIZE;
			if ((lFileSize % DPD_CONTROL_DATA_FRAME_MAX_SIZE) > 0)
			{
				lFrameNum++;
			}

			lFileCRC32 = CRYPTO_CRC32(GLOBAL_U32_MAX, plFileBuf, lFileSize);

			GLOBAL_TRACE(("Send File %s, Size = %d, Total Frame Num = %d, Total CRC32 = %.8X, !!!!!!!!!!!!!!!!!\n", pPathName, lFileSize, lFrameNum, lFileCRC32));

			plMsg->m_ID = DPD_CONTROL_MSG_ID_DATA_SEND;

			GLOBAL_PRINTF(("Loading in Porgress >> 00"));

			lTimeTick = PFC_GetTickCount();

			/*每次发送新命令之前清空串口缓冲区*/
			DPDL_ControlCOMFlush();


			for (i = 0; i < lFrameNum; i++)
			{
				plMsg->m_Len = 0;
				plTmpBuf = plMsg->m_pDataBuf;
				GLOBAL_MSB8_EC(plTmpBuf, Type, plMsg->m_Len);
				GLOBAL_BYTES_SC(plTmpBuf, 3, plMsg->m_Len);
				GLOBAL_MSB32_EC(plTmpBuf, i, plMsg->m_Len);
				GLOBAL_MSB32_EC(plTmpBuf, lFrameNum, plMsg->m_Len);

				lFrameLeftSize = lFileSize - i * DPD_CONTROL_DATA_FRAME_MAX_SIZE;
				if (lFrameLeftSize > DPD_CONTROL_DATA_FRAME_MAX_SIZE)
				{
					lFrameLeftSize = DPD_CONTROL_DATA_FRAME_MAX_SIZE;
				}
				GLOBAL_BYTES_EC(plTmpBuf, &plFileBuf[i * DPD_CONTROL_DATA_FRAME_MAX_SIZE], lFrameLeftSize, plMsg->m_Len);

				if (DPDL_ControlSendMsg() == FALSE)
				{
					GLOBAL_TRACE(("Send Data To DPD Board Failed At Ind %d/%d, Size = %d\n", i, lFrameNum, lFileSize));
					break;
				}

				if ((i % 3 == 1) || (i == lFrameNum - 1))
				{
					if (lFrameNum - 1 <= 0)
					{
						GLOBAL_PRINTF(("\b\b%.2d", 100));
					}
					else
					{
						GLOBAL_PRINTF(("\b\b%.2d", i * 100 / (lFrameNum - 1)));
					}
					GLOBAL_FFUSH(stdout);
					PFC_TaskSleep(1);
				}
			}

			GLOBAL_PRINTF(("\n"));

			GLOBAL_TRACE(("Send Data Done!!! Time = %d ms\n", PAL_TimeDuration(&lTimeTick, 1)));


			if (i == lFrameNum)
			{
				S32 lVoidErrMsgCount = 5;
				while(lVoidErrMsgCount > 0)
				{
					if (DPDL_ControlRecvMsg(5000))
					{
						if (plMsg->m_ID == DPD_CONTROL_MSG_ID_DATA_SEND)
						{
							plTmpBuf = plMsg->m_pDataBuf;
							GLOBAL_MSB32_D(plTmpBuf, lReturnCRC32);
							if (lReturnCRC32 == lFileCRC32)
							{
								GLOBAL_TRACE(("Donwload Successful!!!!!!!!!!!!!!!!!!!!!!!\n"));
								lRet = TRUE;
							}
							else
							{
								GLOBAL_TRACE(("Download CRC Error!!!!!!!!!!!!!!\n"));
							}
						}
						else
						{
							GLOBAL_TRACE(("Error Msg ID = 0x%.2X\n", plMsg->m_ID));
						}
					}
					else
					{
						GLOBAL_TRACE(("Recv Replay Failed!!!!!!!!!!!!!!!!!!!!!!!\n"));
					}

					if (lRet == TRUE)
					{
						break;
					}

					lVoidErrMsgCount--;
				}
			}

			GLOBAL_FREE(plFileBuf);
		}
		else
		{
			GLOBAL_TRACE(("Open File %s Failed!!!!!!!!!!!!!!!!!!!!!!\n", pPathName));
		}
	}
	else
	{
		GLOBAL_TRACE(("Handle or Ptr Error!!!\n"));
	}
	DPDL_UnLock();
	return lRet;
}


BOOL DPD_ControlRecvData(CHAR_T *pPathName, S32 Type)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	DPDL_Lock();
	if (plHandle)
	{
		U8 *plTmpBuf;
		S32 i, lFileSize, lFrameInd, lFrameNum, lLastFrameInd, lFrameDataSize;
		GLOBAL_FD lFileFD;
		U32 lRecvFileCRC32, lCMDFileCRC32;
		U8 *plFileBuf, *plTmpFileBuf;

		plFileBuf = (U8 *)GLOBAL_MALLOC(DPD_CONTROL_MAX_RECV_FILE_SIZE);

		plHandle->m_CMDLock = TRUE;

		/*每次发送新命令之前清空串口缓冲区*/
		DPDL_ControlCOMFlush();

		if (plFileBuf)
		{
			REMOTE_MSG *plMsg;

			plMsg = &plHandle->m_MSG;

			/*打包启动命令*/
			plMsg->m_ID = DPD_CONTROL_MSG_ID_DATA_RECV;
			plMsg->m_Len = 0;
			plTmpBuf = plMsg->m_pDataBuf;
			GLOBAL_MSB8_EC(plTmpBuf, Type, plMsg->m_Len);
			GLOBAL_BYTES_SC(plTmpBuf, 3, plMsg->m_Len);
			if (DPDL_ControlSendMsg())
			{
				/*开始循环接收数据*/
				lLastFrameInd = GLOBAL_INVALID_INDEX;
				plTmpFileBuf = plFileBuf;
				lFileSize = 0;
				while(TRUE)
				{
					if (DPDL_ControlRecvMsg(200))
					{
						if (plMsg->m_ID == DPD_CONTROL_MSG_ID_DATA_RECV)
						{
							plTmpBuf = plMsg->m_pDataBuf;
							GLOBAL_MSB32_D(plTmpBuf, lFrameInd);
							GLOBAL_MSB32_D(plTmpBuf, lFrameNum);
							GLOBAL_MSB32_D(plTmpBuf, lCMDFileCRC32);

							if (lFrameInd == 0)
							{
								GLOBAL_TRACE(("Total Frame Num = %d\n", lFrameNum));
							}

							/*接收帧序号错误判断*/
							if (lLastFrameInd == GLOBAL_INVALID_INDEX)
							{
								if (lFrameInd != 0)
								{
									GLOBAL_TRACE(("First Frame = %d, Stop Recv!!!!\n", lFrameInd));
									break;
								}
							}
							else 
							{
								if (lLastFrameInd + 1 != lFrameInd)
								{
									GLOBAL_TRACE(("Ind Error Last = %d, Cur = %d, Stop Recv!!!\n", lLastFrameInd, lFrameInd));
									break;
								}
							}
							lLastFrameInd = lFrameInd;

							lFrameDataSize = (plMsg->m_Len - 12);

							/*越界检测*/
							if (lFileSize + lFrameDataSize > DPD_CONTROL_MAX_RECV_FILE_SIZE)
							{
								GLOBAL_TRACE(("Data Over Limits, %d/%d\n", lFileSize + lFrameDataSize, DPD_CONTROL_MAX_RECV_FILE_SIZE));
								break;
							}

							GLOBAL_MEMCPY(plTmpFileBuf, plTmpBuf, lFrameDataSize);
							plTmpFileBuf += lFrameDataSize;
							lFileSize += lFrameDataSize;


							if (lFrameInd == lFrameNum - 1)
							{
								//接收完成，计算CRC32
								lRecvFileCRC32 = CRYPTO_CRC32(GLOBAL_U32_MAX, plFileBuf, lFileSize);
								if (lCMDFileCRC32 == lRecvFileCRC32)
								{
									GLOBAL_TRACE(("File Check Successful!, Total File Size = %d\n", lFileSize));
									lRet = TRUE;
								}
								else
								{
									GLOBAL_TRACE(("File CRC Failed!, Total File Size = %d, CMD CRC32 = %.8X, Actual Data CRC32 = %.8X\n", lFileSize, lCMDFileCRC32, lRecvFileCRC32));
								}
								break;
							}
						}
						else
						{
							GLOBAL_TRACE(("Error Msg ID = %d\n", plMsg->m_ID));
							break;
						}
					}
					else
					{
						GLOBAL_TRACE(("Recv Error!\n"));
						break;
					}
				}

				if (lRet == TRUE)
				{
					lFileFD = GLOBAL_FOPEN(pPathName, "wb");
					if (lFileFD)
					{
						GLOBAL_FWRITE(plFileBuf, 1, lFileSize, lFileFD);
						GLOBAL_FCLOSE(lFileFD);
						lFileFD = NULL;
						GLOBAL_TRACE(("Save [%s]File Successful!\n", pPathName));
					}
					else
					{
						lRet = FALSE;
						GLOBAL_TRACE(("File [%s] Open Failed\n", pPathName));
					}
				}
				else
				{
					GLOBAL_TRACE(("Operation Failed!!!!!!!!!!!!!\n"));
				}
			}
			GLOBAL_FREE(plFileBuf);
			plFileBuf = NULL;
		}
		else
		{
			GLOBAL_TRACE(("Malloc Failed!\n"));
		}

		plHandle->m_CMDLock = FALSE;
	}
	DPDL_UnLock();
	return lRet;
}


BOOL DPD_ControlGetBoardInfoInfo(DPD_CONTROL_BOARD_INFO *pBoardInfo)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	DPDL_Lock();
	if (plHandle && pBoardInfo)
	{
		U8 *plTmpBuf;
		REMOTE_MSG *plMsg;
		plMsg = &plHandle->m_MSG;

		plMsg->m_ID = DPD_CONTROL_MSG_ID_BOARD_INFO;
		plMsg->m_Len = 0;

		/*每次发送新命令之前清空串口缓冲区*/
		DPDL_ControlCOMFlush();

		if (DPDL_ControlSendAndRecvMsgSYNC(DPD_CONTROL_IO_TIMEOUT) == TRUE)
		{
			if (plMsg->m_ID == DPD_CONTROL_MSG_ID_BOARD_INFO && plMsg->m_Len >= 25)
			{
				plTmpBuf = plMsg->m_pDataBuf;
				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_PackageYear);
				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_PackageMonth);
				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_PackageDay);

				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_FPGAYear);
				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_FPGAMonth);
				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_FPGADay);

				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_HardwareYear);
				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_HardwareMonth);
				GLOBAL_MSB8_D(plTmpBuf, pBoardInfo->m_HardwareDay);

				GLOBAL_BYTES_S(plTmpBuf, 3);

				GLOBAL_BYTES_D(plTmpBuf, pBoardInfo->m_pBoardID, sizeof(pBoardInfo->m_pBoardID));


				pBoardInfo->m_PackageYear += 2000;
				pBoardInfo->m_FPGAYear += 2000;
				pBoardInfo->m_HardwareYear += 2000;

				GLOBAL_TRACE(("Package = %.4d-%.2d-%.2d\n", pBoardInfo->m_PackageYear, pBoardInfo->m_PackageMonth, pBoardInfo->m_PackageDay));
				GLOBAL_TRACE(("FPGA = %.4d-%.2d-%.2d\n", pBoardInfo->m_FPGAYear, pBoardInfo->m_FPGAMonth, pBoardInfo->m_FPGADay));
				GLOBAL_TRACE(("Hardware = %.4d-%.2d-%.2d\n", pBoardInfo->m_HardwareYear, pBoardInfo->m_HardwareMonth, pBoardInfo->m_HardwareDay));
				CAL_PrintDataBlockWithASCII("Board ID", pBoardInfo->m_pBoardID, sizeof(pBoardInfo->m_pBoardID), 4);

				lRet = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("MSG Error ID = %d, Len = %d\n", plMsg->m_ID, plMsg->m_Len));
			}
		}
	}
	else
	{
		GLOBAL_TRACE(("Handle or Param Error Handle = %.8X, ParamPtr = %.8X\n", plHandle, pBoardInfo));
	}
	DPDL_UnLock();
	return lRet;
}

static S32 s_LastLevelValue = GLOBAL_INVALID_INDEX;

BOOL DPD_ControlSetRFParam(DPD_CONTROL_RF_PARAM *pRFParam)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	DPDL_Lock();
	if (plHandle && pRFParam)
	{
		U8 *plTmpBuf;
		REMOTE_MSG *plMsg;
		plMsg = &plHandle->m_MSG;

		if (pRFParam->m_RFDisableMark == TRUE)
		{
			pRFParam->m_GradualChangeMark = FALSE;
		}
		else
		{
			pRFParam->m_GradualChangeMark = TRUE;
		}

#ifdef DPD_DEBUG
		GLOBAL_TRACE(("Freq = %ld, FreqAdj = %ld, AttLevel = %d, ALCLevel = %d, ALCMark = %d, Tone = %d, Gradu = %d, SpecInv = %d, RFOut = %d\n", 
			pRFParam->m_CenterFreqHz, 
			pRFParam->m_FreqAdj, 
			pRFParam->m_AttenuatorLevel, 
			pRFParam->m_ALCLevel,
			pRFParam->m_ALCMark, 
			pRFParam->m_ToneMark, 
			pRFParam->m_GradualChangeMark, 
			pRFParam->m_SpecInv, 
			pRFParam->m_RFDisableMark));
#endif

		plMsg->m_ID = DPD_CONTROL_MSG_ID_RF_PARAM;
		plMsg->m_Len = 0;
		plTmpBuf = plMsg->m_pDataBuf;
		GLOBAL_MSB32_EC(plTmpBuf, pRFParam->m_CenterFreqHz, plMsg->m_Len);
		GLOBAL_MSB32_EC(plTmpBuf, pRFParam->m_FreqAdj, plMsg->m_Len);
		GLOBAL_MSB16_EC(plTmpBuf, pRFParam->m_AttenuatorLevel, plMsg->m_Len);
		GLOBAL_MSB16_EC(plTmpBuf, pRFParam->m_ALCLevel, plMsg->m_Len);
		GLOBAL_MSB8_EC(plTmpBuf, pRFParam->m_ALCMark, plMsg->m_Len);
		GLOBAL_MSB8_EC(plTmpBuf, pRFParam->m_ToneMark, plMsg->m_Len);
		GLOBAL_MSB8_EC(plTmpBuf, pRFParam->m_GradualChangeMark, plMsg->m_Len);
		GLOBAL_MSB8_EC(plTmpBuf, pRFParam->m_RFDisableMark, plMsg->m_Len);
		GLOBAL_MSB8_EC(plTmpBuf, pRFParam->m_SpecInv, plMsg->m_Len);
		GLOBAL_MSB24_EC(plTmpBuf, pRFParam->m_Reserved, plMsg->m_Len);

		/*每次发送新命令之前清空串口缓冲区*/
		DPDL_ControlCOMFlush();

		if (DPDL_ControlSendAndRecvMsgSYNC(DPD_CONTROL_IO_TIMEOUT) == TRUE)
		{
			if (plMsg->m_ID == DPD_CONTROL_MSG_ID_RF_PARAM && plMsg->m_Len >= 4)
			{
				//U8 lRetValue;
				//plTmpBuf = plMsg->m_pDataBuf;
				//GLOBAL_MSB8_D(plTmpBuf, lRetValue);
				//if (lRetValue == 1)
				//{
					lRet = TRUE;
				//}
				//else
				//{
				//	GLOBAL_TRACE(("DPD Module Return Failed!!!!!!!!!!!!!\n"));
				//}
			}
			else
			{
				GLOBAL_TRACE(("MSG Error ID = %d, Len = %d\n", plMsg->m_ID, plMsg->m_Len));
			}
		}
	}
	else
	{
		GLOBAL_TRACE(("Handle or Param Error Handle = %.8X, ParamPtr = %.8X\n", plHandle, pRFParam));
	}
	DPDL_UnLock();

	if (pRFParam->m_GradualChangeMark == TRUE)
	{
		S32 lTimeout;

		lTimeout = 15 * 1000;
		if (s_LastLevelValue != GLOBAL_INVALID_INDEX)
		{
			lTimeout = lTimeout * (fabs(s_LastLevelValue - pRFParam->m_AttenuatorLevel) / 300);
		}

		plHandle->m_CMDLock = TRUE;
		while(lTimeout > 0)
		{
			lTimeout -= 1000;
			PFC_TaskSleep(1000);
			GLOBAL_TRACE(("Progress Time Left = %ds\n", lTimeout / 1000));
		}
		plHandle->m_CMDLock = FALSE;

	}
	if (pRFParam->m_RFDisableMark == TRUE)
	{
		s_LastLevelValue = 0;
	}
	else
	{
		s_LastLevelValue = pRFParam->m_AttenuatorLevel;
	}

	return lRet;
}

BOOL DPD_ControlSetDPDParam(DPD_CONTROL_DPD_PARAM *pDPDParam)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	DPDL_Lock();
	if (plHandle && pDPDParam)
	{
		U8 *plTmpBuf;
		REMOTE_MSG *plMsg;
		plMsg = &plHandle->m_MSG;

//#ifdef DPD_DEBUG
		GLOBAL_TRACE(("DPD Mark = %d, DPDTrackReset = %d, DPDFeedSelect = %d, Time = %.8X\n", 
			pDPDParam->m_DPDMark,
			pDPDParam->m_DPDTrackResetMark,
			pDPDParam->m_DPDFeedbackSelect,
			pDPDParam->m_TimeTick));
//#endif

		plMsg->m_ID = DPD_CONTROL_MSG_ID_DPD_SET;
		plMsg->m_Len = 0;
		plTmpBuf = plMsg->m_pDataBuf;
		GLOBAL_MSB8_EC(plTmpBuf, pDPDParam->m_DPDMark, plMsg->m_Len);
		GLOBAL_MSB8_EC(plTmpBuf, pDPDParam->m_DPDTrackResetMark, plMsg->m_Len);
		GLOBAL_MSB8_EC(plTmpBuf, pDPDParam->m_DPDFeedbackSelect, plMsg->m_Len);
		GLOBAL_MSB8_EC(plTmpBuf, pDPDParam->m_Reserved, plMsg->m_Len);
		GLOBAL_MSB32_EC(plTmpBuf, pDPDParam->m_TimeTick, plMsg->m_Len);

		/*每次发送新命令之前清空串口缓冲区*/
		DPDL_ControlCOMFlush();

		if (DPDL_ControlSendAndRecvMsgSYNC(DPD_CONTROL_IO_TIMEOUT) == TRUE)
		{
			if (plMsg->m_ID == DPD_CONTROL_MSG_ID_DPD_SET && plMsg->m_Len >= 4)
			{
				U8 lRetValue;
				plTmpBuf = plMsg->m_pDataBuf;
				GLOBAL_MSB8_D(plTmpBuf, lRetValue);
				if (lRetValue == 1)
				{
					lRet = TRUE;
				}
				else
				{
					GLOBAL_TRACE(("DPD Module Return Failed!!!!!!!!!!!!!\n"));
				}
			}
			else
			{
				GLOBAL_TRACE(("MSG Error ID = %d, Len = %d\n", plMsg->m_ID, plMsg->m_Len));
			}
		}
	}
	else
	{
		GLOBAL_TRACE(("Handle or Param Error Handle = %.8X, ParamPtr = %.8X\n", plHandle, pDPDParam));
	}
	DPDL_UnLock();
	return lRet;
}

BOOL DPD_ControlGetDPDStatus(DPD_CONTROL_STATUS *pStatus)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	DPDL_Lock();
	if (plHandle && pStatus)
	{
		U8 *plTmpBuf;
		REMOTE_MSG *plMsg;
		plMsg = &plHandle->m_MSG;

		plMsg->m_ID = DPD_CONTROL_MSG_ID_DPD_STATUS;
		plMsg->m_Len = 0;

		/*每次发送新命令之前清空串口缓冲区*/
		DPDL_ControlCOMFlush();

		if (DPDL_ControlSendAndRecvMsgSYNC(DPD_CONTROL_IO_TIMEOUT) == TRUE)
		{
			if (plMsg->m_ID == DPD_CONTROL_MSG_ID_DPD_STATUS && plMsg->m_Len >= 20)
			{
				plTmpBuf = plMsg->m_pDataBuf;
				GLOBAL_MSB8_D(plTmpBuf, pStatus->m_DPDStatus);
				GLOBAL_MSB8_D(plTmpBuf, pStatus->m_DPDTrackStatus);
				GLOBAL_MSB8_D(plTmpBuf, pStatus->m_DPDFeedebackStatus);
				GLOBAL_BYTES_S(plTmpBuf, 1);

				GLOBAL_MSB16_D(plTmpBuf, pStatus->m_DPDFeedebacklevel);
				GLOBAL_MSB16_D(plTmpBuf, pStatus->m_DPDBoardTemp);
				GLOBAL_MSB8_D(plTmpBuf, pStatus->m_DPDClkStatus);
				GLOBAL_MSB8_D(plTmpBuf, pStatus->m_DPDIOStatus);
				GLOBAL_MSB16_D(plTmpBuf, pStatus->m_DPDTxPower);

				GLOBAL_MSB8_D(plTmpBuf, pStatus->m_DPDRunFlag);
				GLOBAL_BYTES_S(plTmpBuf, 3);

				GLOBAL_MSB32_D(plTmpBuf, pStatus->m_DPDSN);
				GLOBAL_BYTES_D(plTmpBuf, pStatus->m_pReserved, sizeof(pStatus->m_pReserved));

#ifdef DPD_DEBUG
				GLOBAL_TRACE(("DPDStatus = %d, TrackStatus = %d, FeedbackStatus = %d, FeedLevel = %d, Temp = %d, CLKStatus = %d, IOStatus = %d, TxPower = %d, RunFlag = %d, DSN = %d\n", 
					pStatus->m_DPDStatus,
					pStatus->m_DPDTrackStatus,
					pStatus->m_DPDFeedebackStatus,
					pStatus->m_DPDFeedebacklevel,
					pStatus->m_DPDBoardTemp,
					pStatus->m_DPDClkStatus,
					pStatus->m_DPDIOStatus,
					pStatus->m_DPDTxPower,
					pStatus->m_DPDRunFlag,
					pStatus->m_DPDSN));

#endif
				lRet = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("MSG Error ID = %d, Len = %d\n", plMsg->m_ID, plMsg->m_Len));
			}
		}
	}
	else
	{
		GLOBAL_TRACE(("Handle or Param Error Handle = %.8X, ParamPtr = %.8X\n", plHandle, pStatus));
	}
	DPDL_UnLock();
	return lRet;
}


void DPD_ControlTerminate(void)
{
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		DPDL_ControlCOMClose();
		PFC_SemaphoreDestroy(plHandle->m_AccessLock);
	}
}

/*避免状态获取消息和数据发送接收消息重叠*/
BOOL DPDL_ControlCheckCMDLock(void)
{
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		return plHandle->m_CMDLock;
	}
	return FALSE;
}

/*消息发送和应答读取同步函数*/
//BOOL DPDL_ControlSendAndRecvMsgSYNC(S32 WaitTimeMS)
//{
//	BOOL lRet = FALSE;
//	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
//	if (plHandle)
//	{
//		plHandle->m_SendSize = REMOTE_ProtocolPacker(plHandle->m_pSendMsgBuf, sizeof(plHandle->m_pSendMsgBuf), &plHandle->m_MSG);
//		if (plHandle->m_SendSize > 0)
//		{
//			/*调试用打印*/
//			CAL_PrintDataBlockWithASCII(__FUNCTION__, plHandle->m_pSendMsgBuf, plHandle->m_SendSize, 4);
//
//			if (DPDL_ControlCOMWrite(plHandle->m_pSendMsgBuf, plHandle->m_SendSize) == plHandle->m_SendSize)
//			{
//				S32 lActRead;
//				U32 lTick;
//				lTick = PFC_GetTickCount();
//				while(TRUE)
//				{
//					lActRead = DPDL_ControlCOMRead(&plHandle->m_pRecvMsgBuf[plHandle->m_RecvCurLen], sizeof(plHandle->m_pRecvMsgBuf) - plHandle->m_RecvCurLen);
//					if (lActRead > 0)
//					{
//						plHandle->m_RecvCurLen += lActRead;
//
//						if (plHandle->m_MSG.m_ID != 0x0001)
//						{
//							CAL_PrintDataBlockWithASCII("DPDL_ControlSendAndRecvMsgSYNC Recv", plHandle->m_pRecvMsgBuf, plHandle->m_RecvCurLen, 4);
//						}
//
//						if (REMOTE_ProtocolParser(plHandle->m_pRecvMsgBuf, plHandle->m_RecvCurLen, &plHandle->m_MSG, &lActRead))
//						{
//							lRet = TRUE;
//							break;
//						}
//					}
//					else
//					{
//						if (PFC_GetTickCount() - lTick > WaitTimeMS)
//						{
//							GLOBAL_TRACE(("Error!!! RecvTimeOut!!!!! WaitTime = %d\n", WaitTimeMS));
//							break;
//						}
//						else
//						{
//							PFC_TaskSleep(1);
//						}
//					}
//				}
//			}
//		}
//	}
//
//	return lRet;
//}
//
//
BOOL DPDL_ControlSendMsg(void)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		plHandle->m_SendSize = REMOTE_ProtocolPacker(plHandle->m_pSendMsgBuf, sizeof(plHandle->m_pSendMsgBuf), &plHandle->m_MSG);
		if (plHandle->m_SendSize > 0)
		{
			/*打印函数！*/
#ifdef DPD_DEBUG
			if (plHandle->m_MSG.m_ID != 0x0001)
			{
				CAL_PrintDataBlockWithASCII(__FUNCTION__, plHandle->m_pSendMsgBuf, plHandle->m_SendSize, 4);
			}
#endif

			if (DPDL_ControlCOMWrite(plHandle->m_pSendMsgBuf, plHandle->m_SendSize) == plHandle->m_SendSize)
			{
				lRet = TRUE;
			}
		}

		/*发送消息之后，清空BUFFER*/
		plHandle->m_RecvCurLen = 0;
	}
	return lRet;
}

BOOL DPDL_ControlRecvMsg(S32 WaitTimeMS)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		S32 lActRead;
		U32 lTick;
		lTick = PFC_GetTickCount();
		while(TRUE)
		{
			lActRead = DPDL_ControlCOMRead(&plHandle->m_pRecvMsgBuf[plHandle->m_RecvCurLen], sizeof(plHandle->m_pRecvMsgBuf) - plHandle->m_RecvCurLen);
			if (lActRead > 0)
			{
				plHandle->m_RecvCurLen += lActRead;

#ifdef DPD_DEBUG
				/*打印函数！*/
				if (plHandle->m_MSG.m_ID != 0x0002)
				{
					CAL_PrintDataBlockWithASCII(__FUNCTION__, plHandle->m_pRecvMsgBuf, plHandle->m_RecvCurLen, 4);
				}
#endif

				if (REMOTE_ProtocolParser(plHandle->m_pRecvMsgBuf, plHandle->m_RecvCurLen, &plHandle->m_MSG, &lActRead))
				{
					plHandle->m_RecvCurLen -= lActRead;
					lRet = TRUE;
					break;
				}
			}
			else
			{
				if (PFC_GetTickCount() - lTick > WaitTimeMS)
				{
					GLOBAL_TRACE(("Error!!! RecvTimeOut!!!!! WaitTime = %d\n", WaitTimeMS));
					break;
				}
				else
				{
					PFC_TaskSleep(1);
				}
			}
		}
	}

	return lRet;
}


BOOL DPDL_ControlSendAndRecvMsgSYNC(S32 WaitTimeMS)
{
	BOOL lRet = FALSE;
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		if (DPDL_ControlSendMsg())
		{
			if (DPDL_ControlRecvMsg(WaitTimeMS))
			{
				lRet = TRUE;
			}
		}
	}

	return lRet;
}

/*接口多线程保护锁定函数*/
void DPDL_Lock(void)
{
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		PFC_SemaphoreWait(plHandle->m_AccessLock, -1);
	}
}

void DPDL_UnLock(void)
{
	DPD_CONTROL_HANDLE *plHandle = s_pHandle;
	if (plHandle)
	{
		PFC_SemaphoreSignal(plHandle->m_AccessLock);
	}
}

/*串口封装函数*/
void DPDL_ControlCOMOpen(void)
{
	HWL_VRS232_Param lVRS232Param;
	GLOBAL_ZEROMEM(&lVRS232Param, sizeof(HWL_VRS232_Param));
	lVRS232Param.m_TimeOutMS = 1000;
	HWL_VRS232Open(DPD_VRS232_IND, &lVRS232Param);
}

void DPDL_ControlCOMFlush(void)
{
	HWL_VRS232Flush(DPD_VRS232_IND);
}


S32 DPDL_ControlCOMWrite(U8 *pData, S32 DataLen)
{
	return HWL_VRS232Write(DPD_VRS232_IND, pData, DataLen);
}

S32 DPDL_ControlCOMRead(U8 *pBuf, S32 BufLen)
{
	return HWL_VRS232Read(DPD_VRS232_IND, (U8*)pBuf, BufLen);
}

void DPDL_ControlCOMClose(void)
{
	HWL_VRS232Close(DPD_VRS232_IND);
}

#endif

/*EOF*/
