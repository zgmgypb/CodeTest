/*
*需要设置的转码参数：
*输入：
*视频格式
*视频帧率和分辨率
*音频格式
*编码：
*视频格式
*视频码率
*视频帧率和分辨率
*视频Profile和Level
*音频格式
*码率模式
*
**/

/* Includes-------------------------------------------------------------------- */
#include "multi_main_internal.h"

#ifdef ENCODER_CARD_PLATFORM

#include "global_micros.h"
#include "platform_conf.h"
#include "libc_assist.h"
#include "platform_assist.h"
#include "card_app.h"
#include "fpga_rs232_v2.h"
#include "fpga_gpio_v2.h"
#include "fpga_misc.h"
#include "fpga_spi.h"
#include "fpga_spi_flash.h"
#include "transenc.h"
#include "multi_card_pro100.h"
//#include "vixs_pro100.h"//Pro100的串口驱动程序


/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define CARDTRANS_VIXSPRO100_TASK_STATCK_SIZE				(1024*1024)
#define CARDTRANS_VIXSPRO100_CHIP_MAX_NUM					(3)
#define CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM				(2)//当前固件支持的个数
#define CARDTRANS_VIXSPRO100_ENCODER_PARAM(A, B)			(((A << 8) & 0xFF00) | (B & 0xFF))
#define CARDTRANS_VIXSPRO100_DECODER_PARAM(X, A, B)			A = (X >> 8) & 0xFF; B = X & 0xFF;
#define CARDTRANS_VIXSPRO100_PID_START						(1000)
#define CARDTRANS_VIXSPRO100_PID_INTERVAL					(100)

#define CARDTRANS_VIXSPRO100_START_PID(chip, sub)			(CARDTRANS_VIXSPRO100_PID_START + CARDTRANS_VIXSPRO100_PID_INTERVAL * (chip * CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM + sub))
#define CARDTRANS_CRITICAL_TEMP_DEFAULT						(70)
#define CARDTRANS_VIXSPRO100_CHIP_INIT_DELAY				(75 * 1000)

#define CARDTRANS_VIXSPRO100_OUTPUT_ERROR_TOLLERANCE		(8)
#define CARDTRANS_VIXSPRO100_REAPPLY_TOLLERANCE				(0)
#define CARDTRANS_VIXSPRO100_COM_ERROR_TOLLERANCE			(4)
#define CARDTRANS_VIXSPRO100_INIT_RETRY_MAX_COUNT			(1)
//#define CARDTRANS_VIXSPRO100_ALLOW_PARTIAL_PRO100_INIT

#define CARDTRANS_VIXSPRO100_CHIP_APPLY_MONITOR_DELAY		(8 * 1000)
#define CARDTRANS_VIXSPRO100_ERROR_RECOVER_FUNC

/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef enum
{
	CARDTRANS_VIXSPRO100_ALARM_NO_OUTPUT = 0,//转码模块在设置工作的情况下没有输出
	CARDTRANS_VIXSPRO100_ALARM_NO_INPUT,//转码模块在设置工作的情况下没有给输入流
	CARDTRANS_VIXSPRO100_ALARM_CHIP_COM_ERROR,//PRO100串口通讯异常
	CARDTRANS_VIXSPRO100_TEMPERATURE,//温度报警
	CARDTRANS_VIXSPRO100_ALARM_NUM
}CARDTRANS_VIXSPRO100_ALARM;


typedef void (*MULT_CARDTRANS_CB)(S32 SlotIndex, S32 TsIndex, U16 PID);

typedef struct
{
	BOOL			m_bEncoder;
	S32				m_ModuleSlot;
}MULT_CardVixsPro100InitParam;

typedef struct
{
	S32				m_FUARTSubSlot;//因为一个模块支持多个串口，这个是子串口的序号
	HANDLE32		m_FUARTHandle;//虚拟串口句柄，注意这里的句柄是复制的，请不要在这里销毁
	HANDLE32		m_TransENCHandle;
	BOOL			m_PRO100Disabled;//模块检测如果失败，则这个置TRUE，且不对对应芯片进行参数配置，但是让其它芯片继续工作！
	CHAR_T			m_pFirmwareVersion[128];
	U32				m_DelayMS[CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM];//其余PID的延迟时间（毫秒）
	U32				m_pSerUniqueID[CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM];
	U32				m_pServOLDUniqueID[CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM];
	//TRANSENC_Param	m_Param[CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM];

	/*2017-04-14 增加PCR/VID合并参数*/
	BOOL			m_bPCRVIDCombine[CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM];

	S32				m_pSubInError[CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM];
	S32				m_pSubOutError[CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM];

	S32				m_TransENCChipOutputErrorCount;
	S32				m_TransENCChipCOMErrorCount;
	S32				m_TransENCChipReApplyCount;

}MULT_CardVixsPro100Node;

typedef struct
{
	S32				m_CriticalTemp;
	BOOL			m_bGeneralAlarmMark;
}MULT_CardVixsPro100Monitor;

typedef struct
{
	MULT_CardVixsPro100InitParam	m_InitParam;

	MULT_CardVixsPro100Monitor		m_Monitor;

	S32							m_InTsStart;
	S32							m_InTsNum;
	S32							m_OutTsStart;
	S32							m_OutTsNum;

	/*参数应用线程*/
	BOOL						m_TaskMark;
	HANDLE32					m_TaskHandle;

	FMISC_StatusInfo			m_PRO100Status;
	S32							m_StatusGetTimeout;

	BOOL						m_ParamApplyMark;
	BOOL						m_ParamApplyMarkRecover;//修正应用复用参数导致的短暂断流会连锁导致其它子板PRO100异常的问题
	S32							m_ParamApplySlotInd;

	HANDLE32					m_FUARTHandle;//虚拟串口句柄
	HANDLE32					m_FMISCHandle;//虚拟多协议接口，获取温度，设置PID延迟等简单功能
	HANDLE32					m_FGPIOHandle;

	MULT_CardVixsPro100Node			m_pPRO100Node[CARDTRANS_VIXSPRO100_CHIP_MAX_NUM];

	BOOL						m_bReseted;

	HANDLE32					m_DBHandle;//注意这个是从主模块获取的句柄，不能销毁
	HANDLE32					m_AlarmHandle;//注意这个是从主模块获取的句柄，不能销毁

	BOOL			m_bRecoverMode;
	BOOL			m_RecoverSlot;
	S32				m_RecoverModeWaitTimeout;

}MULT_CardVixsPro100Handle;



/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

/* 本地函数 --------------------------------------------------------------------------------------------------------------------------------------- */

/*编码芯片接口函数*/
BOOL MULTL_CardVixsPro100ResetCB(void *pUserParam)
{
	return TRUE;
}

S32 MULTL_CardVixsPro100UARTReadCB(void *pUserParam, U8 *pData, S32 DataSize)
{
	MULT_CardVixsPro100Node *plNode = (MULT_CardVixsPro100Node*)pUserParam;
	return FUART_Read(plNode->m_FUARTHandle, plNode->m_FUARTSubSlot, pData, DataSize);
}

S32 MULTL_CardVixsPro100UARTWriteCB(void *pUserParam, U8 *pData, S32 DataSize)
{
	MULT_CardVixsPro100Node *plNode = (MULT_CardVixsPro100Node*)pUserParam;
	return FUART_Write(plNode->m_FUARTHandle, plNode->m_FUARTSubSlot, pData, DataSize);
}

/*ES TYPE修改和修复函数*/
BOOL MULTL_CardVixsPro100ESTypeModify(HANDLE32 Handle, U32 ServIDs, S32 NewEsType, S32 MPEG2EsCategory, BOOL bRestore)
{
	BOOL lRet = FALSE;
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)Handle;
	if (plHandle)
	{
		U32 lEsIDs;
		HANDLE32 lDBHandle;
		MPEG2_DBEsInInfo lEsInInfo;
		MPEG2_DBEsOutInfo lEsOutInfo;


		lDBHandle = plHandle->m_DBHandle;

		lEsIDs = 0;
		while((lEsIDs = MPEG2_DBGetEsNextNode(lDBHandle, lEsIDs, ServIDs)) != 0)
		{
			//GLOBAL_TRACE(("Module %d, ES ID = 0x%08X!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot, lEsIDs));
			if (MPEG2_DBCheckEsTypeCategory(lDBHandle, lEsIDs, MPEG2EsCategory, TRUE))
			{
				if (MPEG2_DBGetEsOutInfo(lDBHandle, lEsIDs, &lEsOutInfo))
				{
					if (bRestore)
					{
						MPEG2_DBGetEsInInfo(lDBHandle, lEsIDs, &lEsInInfo);

						lEsOutInfo.m_EsType = lEsInInfo.m_EsType;
						//GLOBAL_TRACE(("Restored EsType = %d\n", lEsOutInfo.m_EsType));
					}
					else
					{
						lEsOutInfo.m_EsType = NewEsType;
						//GLOBAL_TRACE(("Set EsType = %d\n", lEsOutInfo.m_EsType));
					}
					MPEG2_DBSetEsOutInfo(lDBHandle, lEsIDs, &lEsOutInfo);
					lRet = TRUE;
				}
				else
				{
					GLOBAL_TRACE(("Module %d, Get Failed!!!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot));
				}

				break;
			}
			else
			{
				GLOBAL_TRACE(("Module %d, Check ES Failed\n", plHandle->m_InitParam.m_ModuleSlot));
			}
		}
	}
	return lRet;
}

/* 创建句柄 */
HANDLE32 MULT_CardVixsPro100Create(MULT_CardVixsPro100InitParam *pParam)
{
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*) GLOBAL_ZMALLOC(sizeof(MULT_CardVixsPro100Handle));
	if (plHandle)
	{
		S32 i;

		plHandle->m_InTsStart = CARD_SubModuleGetTsInd(pParam->m_ModuleSlot, 0, TRUE);
		plHandle->m_OutTsStart = CARD_SubModuleGetTsInd(pParam->m_ModuleSlot, 0, FALSE);
		plHandle->m_InTsNum = CARD_SubModuleGetTsNum(pParam->m_ModuleSlot, TRUE);
		plHandle->m_OutTsNum = CARD_SubModuleGetTsNum(pParam->m_ModuleSlot, FALSE);


		plHandle->m_Monitor.m_bGeneralAlarmMark = TRUE;
		plHandle->m_Monitor.m_CriticalTemp = CARDTRANS_CRITICAL_TEMP_DEFAULT;


		/*虚拟串口初始化*/
		{
			FUART_InitParam lFRS232Init;
			FUART_PortParam lPortParam;
			GLOBAL_ZEROSTRUCT(lPortParam);
			GLOBAL_MEMCPY(&plHandle->m_InitParam, pParam , sizeof(MULT_CardVixsPro100InitParam));

			GLOBAL_ZEROSTRUCT(lFRS232Init);
			GLOBAL_SNPRINTF((lFRS232Init.m_pHandleName, sizeof(lFRS232Init.m_pHandleName), "PRO100 FUART M[%d]", pParam->m_ModuleSlot));
			lFRS232Init.m_pICPSendCB = CARD_SubModuleICPSend;
			lFRS232Init.m_pUserParam = plHandle;
			lFRS232Init.m_SlotNum = CARDTRANS_VIXSPRO100_CHIP_MAX_NUM;

			plHandle->m_FUARTHandle = FUART_Create(&lFRS232Init);
			lPortParam.m_ReadTimeOutMS = 1000;
			lPortParam.m_WriteTimeOutMS = 1000;

			for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
			{
				FUART_Open(plHandle->m_FUARTHandle, i, &lPortParam);
			}

		}

		/*虚拟多协议接口初始化*/
		{
			FMISC_InitParam lFMISCInitParam;
			GLOBAL_ZEROSTRUCT(lFMISCInitParam);
			lFMISCInitParam.m_pUserParam = plHandle;
			lFMISCInitParam.m_pICPSendCB = CARD_SubModuleICPSend;
			GLOBAL_SNPRINTF((lFMISCInitParam.m_pHandleName, sizeof(lFMISCInitParam.m_pHandleName), "PID Bypass M%.2d", pParam->m_ModuleSlot));
			lFMISCInitParam.m_NodeNum = 12;
			plHandle->m_FMISCHandle = FMISC_Create(&lFMISCInitParam);
		}

		/*虚拟GPIO接口初始化*/
		{
			FGPIOV2_InitParam lGPIOInitParam;
			GLOBAL_ZEROSTRUCT(lGPIOInitParam);
			GLOBAL_SNPRINTF((lGPIOInitParam.m_pHandleName, sizeof(lGPIOInitParam.m_pHandleName), "GPIO M[%d]", pParam->m_ModuleSlot));
			lGPIOInitParam.m_SlotNum = 1;
			lGPIOInitParam.m_pUserParam = plHandle;
			lGPIOInitParam.m_pICPSendCB = CARD_SubModuleICPSend;
			lGPIOInitParam.m_TimeoutMS = 1000;
			plHandle->m_FGPIOHandle = FGPIOV2_Create(&lGPIOInitParam);
		}

		{
			S32 k;
			TRANSENC_InitParameter lTRANSENCInitPara;
			MULT_CardVixsPro100Node *plNode;

			GLOBAL_ZEROSTRUCT(lTRANSENCInitPara);

			lTRANSENCInitPara.m_SystemType = TRANSENC_SYSTEM_TYPE_TRANSCODER;
			lTRANSENCInitPara.m_DecoderType = TRANSENC_DECODER_TYPE_PRO100;
			lTRANSENCInitPara.m_EncoderType = TRANSENC_DECODER_TYPE_PRO100;
			lTRANSENCInitPara.m_pResetCB = MULTL_CardVixsPro100ResetCB;
			lTRANSENCInitPara.m_pUartReadCB = MULTL_CardVixsPro100UARTReadCB;
			lTRANSENCInitPara.m_pUartWriteCB = MULTL_CardVixsPro100UARTWriteCB;

			for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
			{
				plNode = &plHandle->m_pPRO100Node[i];
				plNode->m_FUARTHandle = plHandle->m_FUARTHandle;
				plNode->m_FUARTSubSlot = i;
				lTRANSENCInitPara.m_pUserParam = &plHandle->m_pPRO100Node[i];
				plNode->m_TransENCHandle = TRANSENC_Create(&lTRANSENCInitPara);
				/*读取默认参数*/
				if (plNode->m_TransENCHandle)
				{
					for (k = 0; k < CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM; k++)
					{
						plNode->m_DelayMS[k] = 1000;
						/*2017-04-14 增加PCR/VID合并参数*/
						plNode->m_bPCRVIDCombine[k] = FALSE;
					}
				}
				else
				{
					plNode->m_PRO100Disabled = TRUE;
					GLOBAL_TRACE(("Module %d, Slot %d, TRANSENC Create Failed\n", pParam->m_ModuleSlot, i));
				}
			}
		}
	}
	return plHandle;
}

/*参数处理*/
void MULT_CardVixsPro100XMLParamProcess(HANDLE32 Handle, HANDLE32 XMLLoad, HANDLE32 XMLSave, BOOL bRead)
{
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)Handle;
	if (plHandle)
	{
		S32 i, k;
		TRANSENC_Param lTransParam;
		HANDLE32 lHolder, lChnHolder, lSubHolder;
		if (bRead == FALSE)
		{
			/*写入该芯片支持的各种格式等参数！*/
			/*子模块参数*/
			lHolder = XML_WarpAddNode(XMLSave, "parameter");
			for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
			{
				lChnHolder = XML_WarpAddNode(lHolder, "chn");
				XML_WarpAddNodeS32(lChnHolder, "chn_ind", i);
				XML_WarpAddNodeBOOL(lChnHolder, "disabled", plHandle->m_pPRO100Node[i].m_PRO100Disabled);			
				for (k = 0; k < CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM; k++)
				{
					lSubHolder = XML_WarpAddNode(lChnHolder, "sub");
					XML_WarpAddNodeS32(lSubHolder, "sub_ind", k);
					if (TRANSENC_ParamProcess(plHandle->m_pPRO100Node[i].m_TransENCHandle, k, &lTransParam, TRUE))
					{
						XML_WarpAddNodeBOOL(lSubHolder, "enable", lTransParam.m_bEnable);
						/*输入参数*/
						XML_WarpAddNodeHEX(lSubHolder, "uni_id", plHandle->m_pPRO100Node[i].m_pSerUniqueID[k], TRUE);
						XML_WarpAddNodeS32(lSubHolder, "vid_in_fmt", lTransParam.m_DecoderParam.m_VIDFMT);
						XML_WarpAddNodeHEX(lSubHolder, "vid_in_res_fr", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lTransParam.m_DecoderParam.m_VIDRES, lTransParam.m_DecoderParam.m_VIDFR), TRUE);
						XML_WarpAddNodeS32(lSubHolder, "aud_in_fmt", lTransParam.m_DecoderParam.m_AUDFMT);
						XML_WarpAddNodeS32(lSubHolder, "pid_delay", plHandle->m_pPRO100Node[i].m_DelayMS[k]);

						/*输出参数*/
						XML_WarpAddNodeS32(lSubHolder, "vid_out_fmt", lTransParam.m_EncoderParam.m_VIDFMT);
						XML_WarpAddNodeHEX(lSubHolder, "vid_out_res_fr", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lTransParam.m_EncoderParam.m_VIDRES, lTransParam.m_EncoderParam.m_VIDFR), TRUE);
						XML_WarpAddNodeHEX(lSubHolder, "vid_out_profile_level", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lTransParam.m_EncoderParam.m_VIDProfile, lTransParam.m_EncoderParam.m_VIDLevel), TRUE);
						XML_WarpAddNodeS32(lSubHolder, "vid_out_rate", lTransParam.m_EncoderParam.m_VIDBitrate);
						XML_WarpAddNodeS32(lSubHolder, "vid_out_xbr", lTransParam.m_EncoderParam.m_VIDXBR);
						XML_WarpAddNodeS32(lSubHolder, "aud_out_fmt", lTransParam.m_EncoderParam.m_AUDFMT);
						/*输出音频码率和采样率固定不能设置，所以这里也不需要参数保存*/

						/*2017-04-14 增加PCR/VID合并参数*/
						XML_WarpAddNodeONOFF(lSubHolder, "pcr_combine", plHandle->m_pPRO100Node[i].m_bPCRVIDCombine[k]);
					}
					else
					{
						GLOBAL_TRACE(("Module %d, Get TRANSENC Param Failed!!!!!! CHN %d, SUB = %d\n", plHandle->m_InitParam.m_ModuleSlot, i, k));
					}

				}


			}

		}
		else
		{
			S32 lChnInd, lSubInd, lChnCount;

			lChnCount = 0;
			lHolder = XML_WarpFoundFirstNode(XMLLoad, "parameter");
			lChnHolder = XML_WarpFoundFirstNode(lHolder, "chn");
			while(lChnHolder)
			{
				lChnInd = XML_WarpGetNodeS32(lChnHolder, "chn_ind", 10, GLOBAL_INVALID_INDEX);
				GLOBAL_TRACE(("ChnInd = %d\n", lChnInd));
				lChnCount ++;
				if (GLOBAL_CHECK_INDEX(lChnInd, CARDTRANS_VIXSPRO100_CHIP_MAX_NUM))
				{
					lSubHolder = XML_WarpFoundFirstNode(lChnHolder, "sub");
					while(lSubHolder)
					{
						lSubInd = XML_WarpGetNodeS32(lSubHolder, "sub_ind", 10, GLOBAL_INVALID_INDEX);
						if (GLOBAL_CHECK_INDEX(lSubInd, CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM))
						{
							if (TRANSENC_ParamProcess(plHandle->m_pPRO100Node[lChnInd].m_TransENCHandle, lSubInd, &lTransParam, TRUE))
							{
								U32 lTmpValue;
								/*输入参数*/

								/*参数设置，更换节目时，需要清除旧的节目的Transcoder复用属性！！*/
								plHandle->m_pPRO100Node[lChnInd].m_pServOLDUniqueID[lSubInd] = plHandle->m_pPRO100Node[lChnInd].m_pSerUniqueID[lSubInd];
								plHandle->m_pPRO100Node[lChnInd].m_pSerUniqueID[lSubInd] = XML_WarpGetNodeHEX(lSubHolder, "uni_id", 0);
								//GLOBAL_TRACE(("UniqueID Old = 0x%08X, New = 0x%08X\n", plHandle->m_pPRO100Node[lChnInd].m_pServOLDUniqueID[lSubInd], plHandle->m_pPRO100Node[lChnInd].m_pSerUniqueID[lSubInd]));
								lTransParam.m_bEnable = XML_WarpGetNodeBOOL(lSubHolder, "enable", FALSE);
								GLOBAL_TRACE(("Module %d, Chip %d Sub %d bEnable = %d\n", plHandle->m_InitParam.m_ModuleSlot, lChnInd, lSubInd, lTransParam.m_bEnable));

								lTransParam.m_DecoderParam.m_VIDFMT = XML_WarpGetNodeS32(lSubHolder, "vid_in_fmt", 10, GLOBAL_INVALID_INDEX);

								lTmpValue = XML_WarpGetNodeHEX(lSubHolder, "vid_in_res_fr", GLOBAL_INVALID_INDEX);
								CARDTRANS_VIXSPRO100_DECODER_PARAM(lTmpValue, lTransParam.m_DecoderParam.m_VIDRES, lTransParam.m_DecoderParam.m_VIDFR);
								lTransParam.m_DecoderParam.m_AUDFMT = XML_WarpGetNodeS32(lSubHolder, "aud_in_fmt", 10, GLOBAL_INVALID_INDEX);
								plHandle->m_pPRO100Node[lChnInd].m_DelayMS[lSubInd] = XML_WarpGetNodeS32(lSubHolder, "pid_delay", 10, GLOBAL_INVALID_INDEX);

								/*2017-04-14 增加PCR/VID合并参数*/
								plHandle->m_pPRO100Node[lChnInd].m_bPCRVIDCombine[lSubInd] = XML_WarpGetNodeONOFF(lSubHolder, "pcr_combine", FALSE);

								/*输出参数*/
								lTransParam.m_EncoderParam.m_VIDFMT = XML_WarpGetNodeS32(lSubHolder, "vid_out_fmt", 10, GLOBAL_INVALID_INDEX);
								lTmpValue = XML_WarpGetNodeHEX(lSubHolder, "vid_out_res_fr", GLOBAL_INVALID_INDEX);
								CARDTRANS_VIXSPRO100_DECODER_PARAM(lTmpValue, lTransParam.m_EncoderParam.m_VIDRES, lTransParam.m_EncoderParam.m_VIDFR);
								lTmpValue = XML_WarpGetNodeHEX(lSubHolder, "vid_out_profile_level", GLOBAL_INVALID_INDEX);
								CARDTRANS_VIXSPRO100_DECODER_PARAM(lTmpValue, lTransParam.m_EncoderParam.m_VIDProfile, lTransParam.m_EncoderParam.m_VIDLevel);
								lTransParam.m_EncoderParam.m_VIDBitrate = XML_WarpGetNodeS32(lSubHolder, "vid_out_rate", 10, GLOBAL_INVALID_INDEX);
								lTransParam.m_EncoderParam.m_VIDXBR = XML_WarpGetNodeS32(lSubHolder, "vid_out_xbr", 10, GLOBAL_INVALID_INDEX);
								lTransParam.m_EncoderParam.m_AUDFMT = XML_WarpGetNodeS32(lSubHolder, "aud_out_fmt", 10, GLOBAL_INVALID_INDEX);
								/*输出音频码率和采样率固定不能设置，所以这里也不需要参数读取*/
								TRANSENC_ParamProcess(plHandle->m_pPRO100Node[lChnInd].m_TransENCHandle, lSubInd, &lTransParam, FALSE);
							}
							else
							{
								GLOBAL_TRACE(("Module %d, Get TRANSENC Param Failed!!!!!! CHN %d, SUB = %d\n", plHandle->m_InitParam.m_ModuleSlot, lChnInd, lSubInd));
								break;
							}
						}
						else
						{
							GLOBAL_TRACE(("Module %d, SubInd = %d , OverNum %d\n", plHandle->m_InitParam.m_ModuleSlot, lSubInd, CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM));
							break;
						}

						lSubHolder = XML_WarpFoundNextNode(lChnHolder, lSubHolder, "sub");
					}

				}
				else
				{
					GLOBAL_TRACE(("Module %d,ChnInd = %d , OverNum %d\n", plHandle->m_InitParam.m_ModuleSlot, lChnInd, CARDTRANS_VIXSPRO100_CHIP_MAX_NUM));
					break;
				}

				lChnHolder = XML_WarpFoundNextNode(XMLLoad, lChnHolder, "chn");
			}

			/*这个时候只有一个模块的参数更新，仅仅重新设置这一个PRO100的参数，以节省时间*/
			if (lChnCount == 1)
			{
				plHandle->m_ParamApplySlotInd = lChnInd;
			}
		}
	}
}

/*监控参数*/
void MULT_CardVixsPro100XMLMonitorProcess(HANDLE32 Handle, HANDLE32 XMLLoad, HANDLE32 XMLSave, BOOL bRead)
{
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)Handle;
	if (plHandle)
	{
		HANDLE32 lHolder;
		MULT_CardVixsPro100Monitor *plMonitor;

		plMonitor = &plHandle->m_Monitor;
		if (bRead == FALSE)
		{
			lHolder = XML_WarpAddNode(XMLSave, "monitor");
			XML_WarpAddNodeBOOL(lHolder, "global_mark", plMonitor->m_bGeneralAlarmMark);
			XML_WarpAddNodeS32(lHolder, "critical_temp", plMonitor->m_CriticalTemp);
		}
		else
		{
			lHolder = XML_WarpFoundFirstNode(XMLLoad, "monitor");
			if (lHolder)
			{
				plMonitor->m_bGeneralAlarmMark = XML_WarpGetNodeBOOL(lHolder, "global_mark", FALSE);
				plMonitor->m_CriticalTemp = XML_WarpGetNodeS32(lHolder, "critical_temp", 10, CARDTRANS_CRITICAL_TEMP_DEFAULT);
			}
		}
	}
}

/*复位PRO100*/
void MULT_CardVixsPro100ChipReset(MULT_CardVixsPro100Handle *pHandle, S32 ChipInd)
{
	MULT_CardVixsPro100Handle *plHandle = pHandle;
	if (plHandle)
	{
		S32 i;

		GLOBAL_TRACE(("Module %d, Chip = ALL, Reset Enable\n", plHandle->m_InitParam.m_ModuleSlot));
		/*开始复位（低有效）*/
		for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
		{
			if ((ChipInd == GLOBAL_INVALID_INDEX) || (ChipInd == i))
			{
				FGPIOV2_ValueSetPIN(plHandle->m_FGPIOHandle, 0, 0 + i, 0);
			}
		}
		FGPIOV2_Write(plHandle->m_FGPIOHandle, 0);
		PFC_TaskSleep(500);


		GLOBAL_TRACE(("Module %d, Chip = ALL, Disable 3.3V\n", plHandle->m_InitParam.m_ModuleSlot));
		/*关闭3.3V（高有效）*/
		for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
		{
			if ((ChipInd == GLOBAL_INVALID_INDEX) || (ChipInd == i))
			{
				FGPIOV2_ValueSetPIN(plHandle->m_FGPIOHandle, 0, 3 + i, 0);
			}
		}
		FGPIOV2_Write(plHandle->m_FGPIOHandle, 0);
		PFC_TaskSleep(1000);

		GLOBAL_TRACE(("Module %d, Chip = All, Enable 3.3V\n", plHandle->m_InitParam.m_ModuleSlot));
		/*打开3.3V（高有效）*/
		for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
		{
			if ((ChipInd == GLOBAL_INVALID_INDEX) || (ChipInd == i))
			{
				FGPIOV2_ValueSetPIN(plHandle->m_FGPIOHandle, 0, 3 + i, 1);
			}
		}
		FGPIOV2_Write(plHandle->m_FGPIOHandle, 0);
		PFC_TaskSleep(1000);



		GLOBAL_TRACE(("Module %d, Chip = ALL, Reset Disable\n", plHandle->m_InitParam.m_ModuleSlot));
		/*结束复位复位（低有效）*/
		for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
		{
			if ((ChipInd == GLOBAL_INVALID_INDEX) || (ChipInd == i))
			{
				FGPIOV2_ValueSetPIN(plHandle->m_FGPIOHandle, 0, 0 + i, 1);
			}
		}
		FGPIOV2_Write(plHandle->m_FGPIOHandle, 0);
	}
}

/*初始化PRO100通讯接口，并判断是否正常*/
BOOL MULT_CardVixsPro100ChipInit(MULT_CardVixsPro100Handle *pHandle, S32 ChipInd, S32 RetryCount)
{
	BOOL lRet = FALSE;
	MULT_CardVixsPro100Handle *plHandle = pHandle;
	if (plHandle)
	{
		if (RetryCount <= 0)
		{
			RetryCount = 1;
		}

		if (GLOBAL_CHECK_INDEX(ChipInd, CARDTRANS_VIXSPRO100_CHIP_MAX_NUM))
		{
			MULT_CardVixsPro100Node *plNode;
			plNode = &plHandle->m_pPRO100Node[ChipInd];

			while(RetryCount > 0)
			{
				lRet = TRANSENC_FirmwareInitiate(plNode->m_TransENCHandle);
				if (lRet == TRUE)
				{
					break;
				}
				else
				{
					GLOBAL_TRACE(("Module %d, Chip %d, Init Failed! Retry = %d\n", plHandle->m_InitParam.m_ModuleSlot, ChipInd, RetryCount));
					PFC_TaskSleep(50);
				}
				RetryCount--;
			}
		}
		else
		{
			GLOBAL_TRACE(("Module %d, Chip %d Error!\n", plHandle->m_InitParam.m_ModuleSlot, ChipInd));
		}
	}
	return lRet;
}

/*应用复用参数*/
void MULT_CardVixsPro100ApplyRemux(MULT_CardVixsPro100Handle *pHandle, BOOL bRemove)
{
	MULT_CardVixsPro100Handle *plHandle = pHandle;
	if (plHandle)
	{
		S32 i, k;
		U32 lServIDs;
		HANDLE32 lDBHandle;
		MPEG2_DBServiceTransInfo lTransInfo;
		MPEG2_DBServiceInInfo lServInInfo;
		MPEG2_DBServiceOutInfo lServOutInfo;
		TRANSENC_Param lTRANSENCParam;
		MULT_CardVixsPro100Node *plNode;

		lDBHandle = plHandle->m_DBHandle;

		GLOBAL_TRACE(("Module %d, Start! Remove = %d!\n", plHandle->m_InitParam.m_ModuleSlot, bRemove));

		for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
		{
			//GLOBAL_TRACE(("plHandle->m_ParamApplySlotInd = %d, i = %d\n", plHandle->m_ParamApplySlotInd, i));
			if (plHandle->m_ParamApplySlotInd == i || plHandle->m_ParamApplySlotInd == GLOBAL_INVALID_INDEX)
			{
				plNode = &plHandle->m_pPRO100Node[i];
				if (plNode->m_PRO100Disabled == FALSE)
				{
					for (k = 0; k < CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM; k ++)
					{
						/*清除旧的Transcoder和修复旧的ES类型*/
						if (plNode->m_pServOLDUniqueID[k] != 0)
						{
							lServIDs = MPEG2_DBGetServiceIDsByUniqueID(lDBHandle, plNode->m_pServOLDUniqueID[k]);
							if (lServIDs != 0)
							{
								/*检测节目的输入和输出ES TYPE是否一致，不一致则修改成一致的*/
								//MULTL_CardVixsPro100ESTypeModify(plHandle, lServIDs, 0, MPEG2_ES_TYPE_CATEGORY_VIDEO, TRUE);

								MPEG2_DBExTransformServiceProc(lDBHandle, lServIDs, NULL, FALSE);
							}
							else
							{
								GLOBAL_TRACE(("Module %d, Error!!!!!!! UniqueID = 0x%08X, ServIDs = 0x%08X\n", plHandle->m_InitParam.m_ModuleSlot, plNode->m_pServOLDUniqueID[k], lServIDs));
							}
						}

						if (bRemove == FALSE)
						{
							/*设置新的Transcoder，和设置新的ES类型*/
							if (plNode->m_pSerUniqueID[k] != 0)
							{
								lServIDs = MPEG2_DBGetServiceIDsByUniqueID(lDBHandle, plNode->m_pSerUniqueID[k]);
								if (lServIDs != 0)
								{
									S32 lNewEsType;

									lNewEsType = 0;

									if (TRANSENC_ParamProcess(plNode->m_TransENCHandle, k, &lTRANSENCParam, TRUE))
									{
										/*修改节目输出的ES类型和当前转码参数一致*/
										if (lTRANSENCParam.m_EncoderParam.m_VIDFMT == MPEG2_PES_VID_FMT_MPEG2)
										{
											lNewEsType = MPEG2_STREAM_TYPE_MPEG2_VIDEO;
										}
										else if (lTRANSENCParam.m_EncoderParam.m_VIDFMT == MPEG2_PES_VID_FMT_MPEG4_AVC_H264)
										{
											lNewEsType = MPEG2_STREAM_TYPE_MPEG4_AVC_H264_VIDEO;
										}
										else
										{
											GLOBAL_TRACE(("Module %d, Unsupported TRANSENC VID FMT = %d\n", plHandle->m_InitParam.m_ModuleSlot, lTRANSENCParam.m_EncoderParam.m_VIDFMT));
											lNewEsType = MPEG2_STREAM_TYPE_MPEG2_VIDEO;
										}

										//MULTL_CardVixsPro100ESTypeModify(plHandle, lServIDs, lNewEsType, MPEG2_ES_TYPE_CATEGORY_VIDEO, FALSE);
									}
									else
									{
										lNewEsType = MPEG2_STREAM_TYPE_MPEG2_VIDEO;
										GLOBAL_TRACE(("Module %d, TRANSENC Param Get Failed!!!!!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot));
									}

									MPEG2_DBGetServiceInInfo(lDBHandle, lServIDs, &lServInInfo);

									if (MPEG2_DBExTransformServiceProc(lDBHandle, lServIDs, &lTransInfo, TRUE))
									{
										lTransInfo.m_bEnable = TRUE;
										lTransInfo.m_TransInTsInd = plHandle->m_InTsStart;
										lTransInfo.m_TransOutTsInd = plHandle->m_OutTsStart + i * 4/*设定每个芯片最大能处理4个节目，但是目前仅仅能处理2个，但是还是留够了空间的*/ + k;
										lTransInfo.m_TransPIDStart = CARDTRANS_VIXSPRO100_START_PID(i, k);
										lTransInfo.m_TransOutVIDESType = lNewEsType;
										/*2017-04-14 增加PCR/VID合并参数*/
										lTransInfo.m_bPCRVIDCombine = plNode->m_bPCRVIDCombine[k];
										if (MPEG2_DBExTransformServiceProc(lDBHandle, lServIDs, &lTransInfo, FALSE))
										{
											GLOBAL_TRACE(("Module %d, Set UniqueID = 0x%08X, ServID = 0x%08X, [%s], Chip %d, Sub %d OK\n", plHandle->m_InitParam.m_ModuleSlot, plNode->m_pSerUniqueID[k], lServIDs, lServInInfo.m_ServiceName, i, k));
										}
										else
										{
											GLOBAL_TRACE(("Module %d, Set UniqueID = 0x%08X, ServID = 0x%08X, [%s], Chip %d, Sub %d Failed\n", plHandle->m_InitParam.m_ModuleSlot, plNode->m_pSerUniqueID[k], lServIDs, lServInInfo.m_ServiceName, i, k));
										}
									}
									else
									{
										GLOBAL_TRACE(("Module %d, Get UniqueID = 0x%08X, ServID = 0x%08X, [%s], Chip %d, Sub %d Failed\n", plHandle->m_InitParam.m_ModuleSlot, plNode->m_pSerUniqueID[k], lServIDs, lServInInfo.m_ServiceName, i, k));
									}
								}
								else
								{
									GLOBAL_TRACE(("Module %d, ServIDs = 0\n", plHandle->m_InitParam.m_ModuleSlot));
								}
							}
						}
					}
				}
			}
		}

		/*设置参数，并收集信息*/
		CARD_SubModuleRemuxApply();


		GLOBAL_TRACE(("Module %d, Done!\n", plHandle->m_InitParam.m_ModuleSlot));
	}
}


/* 线程函数 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
void MULT_CardVixsPro100TaskFn(void *pUserParam)
{
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*) pUserParam;
	if (plHandle)
	{
		S32 i, k, lDuration;
		U32 lTick;
		U32 lServIDs;
		HANDLE32 lDBHandle;
		MULT_CardVixsPro100Node *plNode;
		TRANSENC_Param lTRANSENCParam;

		lDBHandle = plHandle->m_DBHandle;

		lTick = PFC_GetTickCount();

		GLOBAL_TRACE(("Module %d, Thread Enter!!!!!!!!!!! At Tick = %08X, DBSHandle = 0x%08X\n", plHandle->m_InitParam.m_ModuleSlot, lTick, lDBHandle));

		plHandle->m_TaskMark = TRUE;
		plHandle->m_StatusGetTimeout = 8000;
		plHandle->m_ParamApplyMark = TRUE;
		plHandle->m_ParamApplySlotInd = GLOBAL_INVALID_INDEX;
		while(plHandle->m_TaskMark)
		{
			lDuration = PAL_TimeDuration(&lTick, 10);

			/*处理定时器*/
			CAL_TimeoutProcess(&plHandle->m_StatusGetTimeout, lDuration);
			CAL_TimeoutProcess(&plHandle->m_RecoverModeWaitTimeout, lDuration);

			if (plHandle->m_ParamApplyMark == TRUE)
			{
				GLOBAL_TRACE(("Module %d, Prepare Enter Single Access Lock!!!\n", plHandle->m_InitParam.m_ModuleSlot));
				if (CARD_SubModuleSingleAccessLock())
				{
					GLOBAL_TRACE(("Module %d, Apply Enter, Slot %d, bRecover %d, Tick 0x%08X\n", plHandle->m_InitParam.m_ModuleSlot, plHandle->m_ParamApplySlotInd, plHandle->m_ParamApplyMarkRecover, PFC_GetTickCount()));

					if (lDBHandle)
					{
						if (plHandle->m_ParamApplyMarkRecover == FALSE)
						{
							MULT_CardVixsPro100ApplyRemux(plHandle, FALSE);
						}

						/*直通模块设置*/
						{
							S32 lPIDInd;
							FMISC_DelayInfo lDelayInfo;
							for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
							{
								if ((plHandle->m_ParamApplySlotInd == i) || (plHandle->m_ParamApplySlotInd == GLOBAL_INVALID_INDEX))
								{
									plNode = &plHandle->m_pPRO100Node[i];

									for (k = 0; k < CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM; k ++)
									{
										for (lPIDInd = 0; lPIDInd < FPGA_TS_DELAY_INFO_NUM; lPIDInd++)
										{
											if (plNode->m_PRO100Disabled == FALSE)
											{
												lDelayInfo.m_bEnable[lPIDInd] = TRUE;
											}
											else
											{
												lDelayInfo.m_bEnable[lPIDInd] = FALSE;
											}
											lDelayInfo.m_bModifyPTS[lPIDInd] = FALSE;
											lDelayInfo.m_PID[lPIDInd] = CARDTRANS_VIXSPRO100_START_PID(i, k) + 3 + lPIDInd;
											lDelayInfo.m_PCRPID = CARDTRANS_VIXSPRO100_START_PID(i, k);
											lDelayInfo.m_DelayMS = plHandle->m_pPRO100Node[i].m_DelayMS[k];
										}

										if (FMISC_DataProcess(plHandle->m_FMISCHandle, i * 4 + k, 500, FMISC_TYPE_TS_DELAY_SET, &lDelayInfo, sizeof(lDelayInfo)))
										{
											GLOBAL_TRACE(("Module %d, Chip %d, Sub %d, Slot %d, Set PID Delay OK!!!! \n", plHandle->m_InitParam.m_ModuleSlot, i, k, i * 4 + k));
										}
										else
										{
											GLOBAL_TRACE(("Module %d, Chip %d, Sub %d, Slot %d, Set PID Delay Failed!!!!\n", plHandle->m_InitParam.m_ModuleSlot, i, k, i * 4 + k));
										}
									}
								}
							}
						}




#if 0
						FMISC_DelayInfo lDelayInfo;
						for (i = 0; i < 12; i++)
						{
							GLOBAL_ZEROSTRUCT(lDelayInfo);

							lTick  = PFC_GetTickCount();
							lDelayInfo.m_bEnable[k] = TRUE;

							/*用于测试将PID=500转发*/
							for (k = 0; k < FPGA_TS_DELAY_INFO_NUM; k++)
							{
								lDelayInfo.m_bEnable[k] = FALSE;
								//if (k == 0)
								//{
								//	lDelayInfo.m_bEnable[k] = TRUE;
								//	lDelayInfo.m_PID[k] = 500;
								//}
							}

							if (FMISC_DataProcess(plHandle->m_FMISCHandle, i , 500, FMISC_TYPE_TS_DELAY_SET, &lDelayInfo, sizeof(lDelayInfo)))
							{
								//GLOBAL_TRACE(("Module %d, Set PID Delay OK!!!! CHN %d, Duraion = %d\n", plHandle->m_InitParam.m_ModuleSlot, i));
							}
							else
							{
								GLOBAL_TRACE(("Module %d, Set PID Delay Failed!!!!  CHN %d, Duraion = %d\n", plHandle->m_InitParam.m_ModuleSlot, i));
							}
						}
#endif


						GLOBAL_TRACE(("Start Set PRO100!!!!!!!!\n"));

						/*设置转码芯片*/
						for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
						{
							//GLOBAL_TRACE(("plHandle->m_ParamApplySlotInd = %d, i = %d\n", plHandle->m_ParamApplySlotInd, i));
							if ((plHandle->m_ParamApplySlotInd == i) || (plHandle->m_ParamApplySlotInd == GLOBAL_INVALID_INDEX))
							{
								plNode = &plHandle->m_pPRO100Node[i];
								if (plNode->m_PRO100Disabled == FALSE)
								{
									lTick  = PFC_GetTickCount();
									for (k = 0; k < CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM; k ++)
									{
										if (TRANSENC_ParamProcess(plNode->m_TransENCHandle, k, &lTRANSENCParam, TRUE))
										{
											lServIDs = MPEG2_DBGetServiceIDsByUniqueID(lDBHandle, plNode->m_pSerUniqueID[k]);
											if (plNode->m_pSerUniqueID[k] != 0)
											{
												lServIDs = MPEG2_DBGetServiceIDsByUniqueID(lDBHandle, plNode->m_pSerUniqueID[k]);
												if (lServIDs != 0)
												{
													lTRANSENCParam.m_DecoderParam.m_VIDPID = CARDTRANS_VIXSPRO100_START_PID(i, k) + 1;
													lTRANSENCParam.m_DecoderParam.m_AUDPID = CARDTRANS_VIXSPRO100_START_PID(i, k) + 2;
													lTRANSENCParam.m_EncoderParam.m_VIDPID = CARDTRANS_VIXSPRO100_START_PID(i, k) + 1;
													lTRANSENCParam.m_EncoderParam.m_AUDPID = CARDTRANS_VIXSPRO100_START_PID(i, k) + 2;
													if (MPEG2_DBGetServiceESPCRPIDISShared(lDBHandle, lServIDs) == TRUE)
													{
														lTRANSENCParam.m_DecoderParam.m_PCRPID = lTRANSENCParam.m_DecoderParam.m_VIDPID;
													}
													else
													{
														lTRANSENCParam.m_DecoderParam.m_PCRPID = CARDTRANS_VIXSPRO100_START_PID(i, k);
													}

													/*转码时，如果输出PCR PID和视频一致,则忽视PCR PID，并将转码模块的参数中的PCR PID赋值为VID PID*/
													if (plNode->m_bPCRVIDCombine[k] == TRUE)
													{
														lTRANSENCParam.m_EncoderParam.m_PCRPID = lTRANSENCParam.m_EncoderParam.m_VIDPID;
													}
													else
													{
														lTRANSENCParam.m_EncoderParam.m_PCRPID = CARDTRANS_VIXSPRO100_START_PID(i, k);
													}

												}
												else
												{
													GLOBAL_TRACE(("Module %d, Chip %d, Serv %d, Service IDs == 0, Possiable Service Removed!!\n", plHandle->m_InitParam.m_ModuleSlot, i, k));
												}
											}
											TRANSENC_ParamProcess(plNode->m_TransENCHandle, k, &lTRANSENCParam, FALSE);
										}
										else
										{
											GLOBAL_TRACE(("Module %d, TRANSENC Param Get Failed!!!!!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot));
										}
									}

									if (TRANSENC_ParamApply(plNode->m_TransENCHandle))
									{
										GLOBAL_TRACE(("Module %d, TRANSENC_ParamApply [%d] OK!!!! Duraion = %d\n", plHandle->m_InitParam.m_ModuleSlot, i, PFC_GetTickCount() - lTick));
									}
									else
									{
										GLOBAL_TRACE(("Module %d, TRANSENC_ParamApply [%d] Failed!!!! Duraion = %d\n", plHandle->m_InitParam.m_ModuleSlot, i, PFC_GetTickCount() - lTick));
									}
								}
							}
						}
					}
					else
					{
						GLOBAL_TRACE(("Module %d, DB Handle NULL!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot));
					}

					GLOBAL_TRACE(("Module %d, Apply Done, Tick = 0x%08X\n", plHandle->m_InitParam.m_ModuleSlot, PFC_GetTickCount()));

					CARD_SubModuleSingleAccessUnlock();
				}

				plHandle->m_ParamApplyMark = FALSE;
				plHandle->m_ParamApplyMarkRecover = FALSE;

				plHandle->m_StatusGetTimeout = CARDTRANS_VIXSPRO100_CHIP_APPLY_MONITOR_DELAY;

				/*不计算设置参数的时间*/
				PAL_TimeDuration(&lTick, 10);
			}
			else
			{
				if (CAL_TimeoutCheck2(&plHandle->m_StatusGetTimeout))
				{
					if (CARD_SubModuleStatusGet(plHandle->m_InitParam.m_ModuleSlot, &plHandle->m_PRO100Status))
					{
						S32 k;
						S32 lBitValue;
						for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
						{
							plNode = &plHandle->m_pPRO100Node[i];
							for (k = 0; k < CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM; k++)
							{
								if (plNode->m_PRO100Disabled == FALSE)
								{
									if (TRANSENC_ParamProcess(plNode->m_TransENCHandle, k, &lTRANSENCParam, TRUE))
									{
										if (lTRANSENCParam.m_bEnable == TRUE)
										{
											lBitValue = 1 << (i * 4 + k);//这里每个芯片输输入有4个TS！！！一个TS一个节目，目前只使用了2个
											if ((plHandle->m_PRO100Status.m_Status[0] & lBitValue) == 0)
											{
												plNode->m_pSubInError[k]++;
											}
											else
											{
												plNode->m_pSubInError[k] = 0;
											}

											lBitValue = 1 << (i * 2 + k);//这里每个芯片输出只有2个TS！！！
											if ((plHandle->m_PRO100Status.m_Status[1] & lBitValue) == 0)
											{
												plNode->m_pSubOutError[k]++;
											}
											else
											{
												plNode->m_pSubOutError[k] = 0;
											}
										}
										else
										{
											plNode->m_pSubInError[k] = 0;
											plNode->m_pSubOutError[k] = 0;
										}

										/*报警容错*/
										if (plNode->m_pSubInError[k] > 1)
										{
											if (plHandle->m_Monitor.m_bGeneralAlarmMark)
											{
												CAL_LogAddLog(plHandle->m_AlarmHandle, CARDTRANS_VIXSPRO100_ALARM_NO_INPUT, i << 4 | k, PFC_GetTickCount(), FALSE);
											}
										}
									}
								}
							}
						}


						//GLOBAL_TRACE(("[0x%08X] [0x%08X]\n", plHandle->m_PRO100Status.m_Status[0], plHandle->m_PRO100Status.m_Status[1]));
					}

					{
						FMISC_TempInfo lTempInfo;
						CARD_SubModuleTempGet(plHandle->m_InitParam.m_ModuleSlot, &lTempInfo);
						if (plHandle->m_Monitor.m_CriticalTemp < lTempInfo.m_TempValueC)
						{
							if (plHandle->m_Monitor.m_bGeneralAlarmMark)
							{
								CAL_LogAddLog(plHandle->m_AlarmHandle, CARDTRANS_VIXSPRO100_TEMPERATURE, 0, PFC_GetTickCount(), FALSE);
							}
						}
					}


					/*检测指定转码芯片在有输入的情况下没有输出，此时复位指定芯片*/
					if (plHandle->m_bRecoverMode == FALSE)
					{
						for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
						{
							plNode = &plHandle->m_pPRO100Node[i];
							if (plNode->m_PRO100Disabled == FALSE)
							{
								BOOL blChipHaveChannelError = FALSE;
								/*TransENC 通讯检测*/
								if (TRANSENC_FirmwareVersionGet(plNode->m_TransENCHandle, plNode->m_pFirmwareVersion) == FALSE)
								{
									plNode->m_TransENCChipCOMErrorCount++;
									GLOBAL_TRACE(("Module %d, Chip %d, COM Error, Count = %d!\n", plHandle->m_InitParam.m_ModuleSlot, i, plNode->m_TransENCChipCOMErrorCount));
								}
								else
								{
									plNode->m_TransENCChipCOMErrorCount = 0;
								}

								/*报警容错*/
								if (plNode->m_TransENCChipCOMErrorCount > 1)
								{
									if (plHandle->m_Monitor.m_bGeneralAlarmMark)
									{
										CAL_LogAddLog(plHandle->m_AlarmHandle, CARDTRANS_VIXSPRO100_ALARM_CHIP_COM_ERROR, i, PFC_GetTickCount(), FALSE);
									}
								}


								for (k = 0; k < CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM; k++)
								{

									if (TRANSENC_ParamProcess(plNode->m_TransENCHandle, k, &lTRANSENCParam, TRUE))
									{
										if (lTRANSENCParam.m_bEnable == TRUE)
										{
											/*报警容错*/

											if ((plNode->m_pSubInError[k] == 0) && (plNode->m_pSubOutError[k] > 0))
											{
												/*输出错误！*/
												blChipHaveChannelError = TRUE;
												GLOBAL_TRACE(("Module %d, Chip %d, Sub %d, Output Error\n", plHandle->m_InitParam.m_ModuleSlot, i, k));
												if (plNode->m_pSubOutError[k] > 1)//连续两次才输出报警
												{
													if (plHandle->m_Monitor.m_bGeneralAlarmMark)
													{
														CAL_LogAddLog(plHandle->m_AlarmHandle, CARDTRANS_VIXSPRO100_ALARM_NO_OUTPUT, i << 4 | k, PFC_GetTickCount(), FALSE);
													}
												}
											}
										}
									}
								}

								if (blChipHaveChannelError)
								{
									plNode->m_TransENCChipOutputErrorCount++;
									GLOBAL_TRACE(("Module %d, Chip %d, Output Error, Count %d, Limit %d\n", plHandle->m_InitParam.m_ModuleSlot, i, plNode->m_TransENCChipOutputErrorCount, CARDTRANS_VIXSPRO100_OUTPUT_ERROR_TOLLERANCE));
								}
								else
								{
									plNode->m_TransENCChipOutputErrorCount = 0;
									plNode->m_TransENCChipReApplyCount = 0;
								}

#ifndef DEBUG_MODE_FPGA_CONFIG_ONCE
#ifdef CARDTRANS_VIXSPRO100_ERROR_RECOVER_FUNC
								{
									BOOL blNeedReset = FALSE;

									if (plNode->m_TransENCChipCOMErrorCount > CARDTRANS_VIXSPRO100_COM_ERROR_TOLLERANCE)
									{
										plNode->m_TransENCChipCOMErrorCount = 0;
										blNeedReset = TRUE;
										GLOBAL_TRACE(("Module %d, Chip %d COM Error Over Tollerance! Need Reset!\n", plHandle->m_InitParam.m_ModuleSlot, i));
									}
									else
									{
										if (plNode->m_TransENCChipOutputErrorCount > CARDTRANS_VIXSPRO100_OUTPUT_ERROR_TOLLERANCE)
										{
											if (plNode->m_TransENCChipReApplyCount > CARDTRANS_VIXSPRO100_REAPPLY_TOLLERANCE)
											{
												blNeedReset = TRUE;
												plNode->m_TransENCChipReApplyCount = 0;
												GLOBAL_TRACE(("Module %d, Chip %d, ReApply Over Tollerance! Need Reset!\n", plHandle->m_InitParam.m_ModuleSlot, i));
											}
											else
											{
												GLOBAL_TRACE(("Module %d, Chip %d, Output Error! ReApply Param! Count = %d\n", plHandle->m_InitParam.m_ModuleSlot, i, plNode->m_TransENCChipReApplyCount));
												plHandle->m_ParamApplySlotInd = i;
												plHandle->m_ParamApplyMark = TRUE;
												plHandle->m_ParamApplyMarkRecover = TRUE;
												plNode->m_TransENCChipReApplyCount ++;
											}

											plNode->m_TransENCChipOutputErrorCount = 0;
										}

									}

									if (blNeedReset)
									{
										plHandle->m_RecoverSlot = i;
										GLOBAL_TRACE(("Module %d, Chip %d, Fatal Error! Start Reset!\n", plHandle->m_InitParam.m_ModuleSlot, plHandle->m_RecoverSlot));
										TRANSENC_FirmwareReset(plNode->m_TransENCHandle);
										GLOBAL_TRACE(("After Firmware Reset!\n"));
										MULT_CardVixsPro100ChipReset(plHandle, plHandle->m_RecoverSlot);
										GLOBAL_TRACE(("After Chip Reset!\n"));
										plHandle->m_bRecoverMode = TRUE;
										plHandle->m_RecoverModeWaitTimeout = 75 * 1000;
									}
								}
#endif
#endif

							}
						}
					}
					else
					{
						if (CAL_TimeoutCheck2(&plHandle->m_RecoverModeWaitTimeout))
						{
							GLOBAL_TRACE(("Module %d, Chip %d, Recover From Reset Prepare Init PRO100!!!!\n", plHandle->m_InitParam.m_ModuleSlot, plHandle->m_RecoverSlot));
							plHandle->m_ParamApplySlotInd = plHandle->m_RecoverSlot;
							if (MULT_CardVixsPro100ChipInit(plHandle, plHandle->m_ParamApplySlotInd, CARDTRANS_VIXSPRO100_INIT_RETRY_MAX_COUNT) == TRUE)
							{
								GLOBAL_TRACE(("Module %d, Chip %d, Init PRO100 Complete!!!!!, Prepare Apply Parameter\n", plHandle->m_InitParam.m_ModuleSlot, plHandle->m_RecoverSlot));
								plHandle->m_ParamApplyMark = TRUE;
								plHandle->m_ParamApplyMarkRecover = TRUE;
								plHandle->m_bRecoverMode = FALSE;
							}
							else
							{
								GLOBAL_TRACE(("Module %d, Chip %d, Init PRO100 Failed!!!!!, Reset Again!\n", plHandle->m_InitParam.m_ModuleSlot, plHandle->m_RecoverSlot));

								TRANSENC_FirmwareReset(plNode->m_TransENCHandle);
								MULT_CardVixsPro100ChipReset(plHandle, plHandle->m_RecoverSlot);
								plHandle->m_bRecoverMode = TRUE;
								plHandle->m_RecoverModeWaitTimeout = CARDTRANS_VIXSPRO100_CHIP_INIT_DELAY;
							}
						}
						else
						{
							GLOBAL_TRACE(("Module %d, Chip %d, Reset Wait Left = %d S\n", plHandle->m_InitParam.m_ModuleSlot, plHandle->m_RecoverSlot, plHandle->m_RecoverModeWaitTimeout / 1000));
						}

					}

					plHandle->m_StatusGetTimeout = 2000;
				}
			}
			PFC_TaskSleep(100);
		}
	}
}

/* 启动线程 */
void MULT_CardVixsPro100Start(HANDLE32 Handle)
{
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)Handle;
	if (plHandle)
	{
		plHandle->m_TaskHandle = PFC_TaskCreate("Module Task", CARDTRANS_VIXSPRO100_TASK_STATCK_SIZE, MULT_CardVixsPro100TaskFn, 0, plHandle);
	}
}

/* 关闭线程 */
void MULT_CardVixsPro100Stop(HANDLE32 Handle)
{
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)Handle;
	if (plHandle)
	{
		if (plHandle->m_TaskMark)
		{
			plHandle->m_TaskMark = FALSE;
			if (plHandle->m_TaskHandle)
			{
				if (PFC_TaskWait(plHandle->m_TaskHandle, GLOBAL_INVALID_INDEX))
				{
					GLOBAL_TRACE(("Module %d, Task Closed\n", plHandle->m_InitParam.m_ModuleSlot));
				}
				plHandle->m_TaskHandle = NULL;
			}
		}
	}
}

/* 模块销毁 */
void MULT_CardVixsPro100Destroy(HANDLE32 Handle)
{
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)Handle;
	if (plHandle)
	{
		S32 i;

		GLOBAL_TRACE(("Module %d, Module Start To Close!\n", plHandle->m_InitParam.m_ModuleSlot));

		MULT_CardVixsPro100Stop(Handle);

		/*清除复用参数！*/
		MULT_CardVixsPro100ApplyRemux(plHandle, TRUE);

		if (plHandle->m_FUARTHandle)
		{
			FUART_Destroy(plHandle->m_FUARTHandle);
			plHandle->m_FUARTHandle = NULL;
		}

		if (plHandle->m_FMISCHandle)
		{
			FMISC_Destroy(plHandle->m_FMISCHandle);
			plHandle->m_FMISCHandle = NULL;
		}

		if (plHandle->m_FGPIOHandle)
		{
			FGPIOV2_Destroy(plHandle->m_FGPIOHandle);
			plHandle->m_FGPIOHandle = NULL;
		}

		for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
		{
			if (plHandle->m_pPRO100Node[i].m_TransENCHandle)
			{
				TRANSENC_Destroy(plHandle->m_pPRO100Node[i].m_TransENCHandle);
				plHandle->m_pPRO100Node[i].m_TransENCHandle = NULL;
			}
		}

		GLOBAL_TRACE(("Module %d, Module Close Done!\n", plHandle->m_InitParam.m_ModuleSlot));

		GLOBAL_FREE(plHandle);
		plHandle = NULL;
	}
}

/*CARD_SYSTEM 接口*/

/*命令接口*/
BOOL MULT_CardVixsPro100CMDProcessCB(void *pUserParam, S32 CMDOPType, void *pOPParam, S32 OPParamSize)
{
	BOOL lRet = FALSE;

	if (CMDOPType != CARD_CMD_OP_TYPE_CHECK_BUSY)
	{
		GLOBAL_TRACE(("CMD Type = %d\n", CMDOPType));
	}

	if (CMDOPType == CARD_CMD_OP_TYPE_CREATE)
	{
		HANDLE32 lHandle;

		MULT_CardVixsPro100InitParam lParam;
		GLOBAL_ZEROSTRUCT(lParam);

		lParam.m_bEncoder = TRUE;
		lParam.m_ModuleSlot = *((S32*)pOPParam);
		lHandle = MULT_CardVixsPro100Create(&lParam);

		(*(void **)pUserParam) = lHandle;

		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_PREPARE)
	{
		MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)pUserParam;
		if (plHandle)
		{
			S32 i;
			CHAR_T plVersion[128];
			//BOOL blAllOK;

			/*获取报警模块句柄，并初始化*/
			plHandle->m_AlarmHandle = CARD_SubModuleAlarmHandleGet(plHandle->m_InitParam.m_ModuleSlot);
			if (plHandle->m_AlarmHandle)
			{
				for (i = 0; i < CARDTRANS_VIXSPRO100_ALARM_NUM; i++)
				{
					CAL_LogSetUsedMark(plHandle->m_AlarmHandle, i, TRUE);
				}
			}
			else
			{
				GLOBAL_TRACE(("Module %d, Alarm Handle Get NULL\n", plHandle->m_InitParam.m_ModuleSlot));
			}

			/*获取数据库句柄，并初始化*/
			{
				plHandle->m_DBHandle = CARD_SubModuleRemuxGet();
				/*关闭输出到这个通道的TS的PSI信息，因为不需要！*/
				for (i = 0; i < plHandle->m_OutTsNum; i++)
				{
					MPEG2_DBSetOutputTsPSIBlock(plHandle->m_DBHandle, i + plHandle->m_OutTsStart, TRUE);
				}
			}

			/*设置SPI接口为PRO100控制*/
			plHandle->m_bReseted = FALSE;

			//blAllOK = TRUE;
			//for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
			//{
			//	if (FSPI_ChipConfig(plHandle->m_FSPIHandle, i, FALSE, FALSE) == FALSE)
			//	{
			//		blAllOK = FALSE;
			//		break;
			//	}
			//}

			//if (blAllOK == TRUE)
			//{
			PFC_TaskSleep(100);

			/*设置FGPIO均为输出，均初始化为1*/
			for (i = 0; i < 6; i ++)
			{
				FGPIOV2_IOMaskSetPIN(plHandle->m_FGPIOHandle, 0, i, FALSE);
			}

			for (i = 0; i < 6; i ++)
			{
				FGPIOV2_ValueSetPIN(plHandle->m_FGPIOHandle, 0, i, 1);
			}

#ifdef DEBUG_MODE_FPGA_CONFIG_ONCE
			/*试图取得版本信息*/
			for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
			{
				TRANSENC_FirmwareInitiate(plHandle->m_pPRO100Node[i].m_TransENCHandle);
				if (TRANSENC_FirmwareVersionGet(plHandle->m_pPRO100Node[i].m_TransENCHandle, plVersion) == FALSE)
				{
					GLOBAL_TRACE(("Module %d, Get Version Failed At Chip[%d]\n", plHandle->m_InitParam.m_ModuleSlot, i));
					break;
				}
				else
				{
					GLOBAL_TRACE(("Module %d, Slot %d, PRO100 Firmvare Version = [%s]\n", plHandle->m_InitParam.m_ModuleSlot,i,  plVersion));
				}
			}
#else
			i = 0;
#endif

			/*复位所有的PRO100*/
			if (i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM)
			{
				MULT_CardVixsPro100ChipReset(plHandle, GLOBAL_INVALID_INDEX);
				plHandle->m_bReseted = TRUE;
			}
			lRet = TRUE;
			//}
			//else
			//{
			//	GLOBAL_TRACE(("Module %d, SPI Direction Set To Pro100 Failed!!\n", plHandle->m_InitParam.m_ModuleSlot));
			//}
		}
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_GET_MODULE_INIT_DELAY_MS)
	{
		MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)pUserParam;
		if (plHandle)
		{
			if (plHandle->m_bReseted)
			{
				(*(S32*)pOPParam) = CARDTRANS_VIXSPRO100_CHIP_INIT_DELAY;
			}
			else
			{
				(*(S32*)pOPParam) = 10;
			}
		}
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_CHECK_OK)
	{
		MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)pUserParam;
		if (plHandle)
		{
			S32 i, lOKCount;
			S32 *plTmpValue;
			MULT_CardVixsPro100Node *plNode;
			/*调用接口函数，确定子模块已经在正常工作，并可以接收参数设置！*/

			plTmpValue = (S32 *)pOPParam;

			GLOBAL_TRACE(("Module %d, Start Init!, Tollerance = %d\n", plHandle->m_InitParam.m_ModuleSlot, (*plTmpValue)));
			lOKCount = 0;
			for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
			{
				plNode = &plHandle->m_pPRO100Node[i];
				if (MULT_CardVixsPro100ChipInit(plHandle, i, CARDTRANS_VIXSPRO100_INIT_RETRY_MAX_COUNT))
				{
					plNode->m_PRO100Disabled = FALSE;
					if (TRANSENC_FirmwareVersionGet(plNode->m_TransENCHandle, plNode->m_pFirmwareVersion) == FALSE)
					{
						GLOBAL_TRACE(("Module %d, Get Version Failed At Chip %d, \n", plHandle->m_InitParam.m_ModuleSlot, i));
					}
					else
					{
						lOKCount++;
						GLOBAL_TRACE(("Module %d, Slot %d, PRO100 Firmvare Version = [%s]\n", plHandle->m_InitParam.m_ModuleSlot, i, plNode->m_pFirmwareVersion));
					}
				}
				else
				{
					GLOBAL_TRACE(("Module %d, Chip %d Init Error! Need Reset!\n", plHandle->m_InitParam.m_ModuleSlot, i));
					plNode->m_PRO100Disabled = TRUE;
					//MULT_CardVixsPro100ChipReset(plHandle, i);
				}
			}


#ifdef CARDTRANS_VIXSPRO100_ALLOW_PARTIAL_PRO100_INIT
			if (lOKCount <= 0)
			{
				(*(S32*)pOPParam) = 0;

			}
			else
			{
				(*(S32*)pOPParam) = 1;
			}
#else
			if ((*plTmpValue) > 0)
			{
				if (lOKCount != CARDTRANS_VIXSPRO100_CHIP_MAX_NUM)
				{
					(*(S32*)pOPParam) = 0;

				}
				else
				{
					(*(S32*)pOPParam) = 1;
				}
			}
			else
			{
				if (lOKCount <= 0)
				{
					(*(S32*)pOPParam) = 0;

				}
				else
				{
					(*(S32*)pOPParam) = 1;
				}
			}
#endif
		}
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_GET_FIRMWARE_VERSION)
	{
		MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)pUserParam;
		if (plHandle)
		{
			S32 i;
			CHAR_T *plVersion;
			MULT_CardVixsPro100Node *plNode;

			plVersion = (CHAR_T*)pOPParam;
			plVersion[0] = 0;
			/*找到第一个可以正常工作的PRO100，获取其固件版本*/
			for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
			{
				plNode = &plHandle->m_pPRO100Node[i];
				if (plNode->m_PRO100Disabled == FALSE)
				{
					GLOBAL_STRCAT(plVersion, plNode->m_pFirmwareVersion);
				}
				else
				{
					GLOBAL_STRCAT(plVersion, "--.--");
				}
				if (i != CARDTRANS_VIXSPRO100_CHIP_MAX_NUM - 1)
				{
					GLOBAL_STRCAT(plVersion, ", ");
				}
			}
		}
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_APPLY)
	{
		MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)pUserParam;
		if (plHandle)
		{
			if (plHandle->m_TaskHandle == NULL)
			{
				MULT_CardVixsPro100Start(pUserParam);
			}
		}
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_CHECK_BUSY)
	{
		MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)pUserParam;
		if (plHandle)
		{
			if (plHandle->m_ParamApplyMark == TRUE && plHandle->m_ParamApplyMarkRecover == FALSE)
			{
				(*(S32*)pOPParam) = TRUE;
			}
			else
			{
				(*(S32*)pOPParam) = FALSE;
			}
		}
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_DESTROY)
	{
		MULT_CardVixsPro100Destroy(pUserParam);
		lRet = TRUE;
	}
	else
	{

	}
	return lRet;
}

/*XML接口*/
BOOL MULT_CardVixsPro100XMLProcessCB(void *pUserParam, HANDLE32 XMLLoad, HANDLE32 XMLSave, S32 XMLOPType)
{
	BOOL lRet = FALSE;
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)pUserParam;
	if (plHandle)
	{
		//GLOBAL_TRACE(("Module %d, XML OP Type = %d!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot, XMLOPType));
		if (XMLOPType == CARD_XML_OP_TYPE_SAVE)
		{
			GLOBAL_TRACE(("Module %d, Save Parameter!\n", plHandle->m_InitParam.m_ModuleSlot));
			if (XMLSave)
			{
				MULT_CardVixsPro100XMLParamProcess(plHandle, NULL, XMLSave, FALSE);
				MULT_CardVixsPro100XMLMonitorProcess(plHandle, NULL, XMLSave, FALSE);
				lRet = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("NULL XMLSave\n"));
			}
		}
		else if (XMLOPType == CARD_XML_OP_TYPE_LOAD)
		{
			GLOBAL_TRACE(("Module %d, Load Parameter!\n", plHandle->m_InitParam.m_ModuleSlot));
			if (XMLLoad)
			{
				MULT_CardVixsPro100XMLParamProcess(plHandle, XMLLoad, NULL, TRUE);
				MULT_CardVixsPro100XMLMonitorProcess(plHandle, XMLLoad, NULL, TRUE);

				lRet = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("NULL XMLLoad\n"));
			}
		}
		else if (XMLOPType == CARD_XML_OP_TYPE_DYNAMIC)
		{
			if (XMLLoad)
			{
				HANDLE32 lParamHolder, lSubHolder, lServHolder;
				CHAR_T *plSubModuleInfo;
				S32 i, lParamA, lParamB;

				plSubModuleInfo = XML_WarpGetNodeText(XMLLoad, CARD_XML_SUB_MODULE_INFO_TAG, NULL);
				if (GLOBAL_STRCMP(plSubModuleInfo, "param_set") == 0)
				{
					GLOBAL_TRACE(("Module %d, Set Parameter\n", plHandle->m_InitParam.m_ModuleSlot));
					MULT_CardVixsPro100XMLParamProcess(plHandle, XMLLoad, NULL, TRUE);
					CARD_SubModuleParameterSave(plHandle->m_InitParam.m_ModuleSlot, FALSE);//回调函数中不锁定
					CARD_SubModuleSetSaveMark();
					plHandle->m_ParamApplyMark = TRUE;
					lRet = TRUE;
				}
				else if (GLOBAL_STRCMP(plSubModuleInfo, "monitor_set") == 0)
				{
					GLOBAL_TRACE(("Module %d, Set Monitor\n", plHandle->m_InitParam.m_ModuleSlot));
					MULT_CardVixsPro100XMLMonitorProcess(plHandle, XMLLoad, NULL, TRUE);
					CARD_SubModuleParameterSave(plHandle->m_InitParam.m_ModuleSlot, FALSE);//回调函数中不锁定
					CARD_SubModuleSetSaveMark();
					lRet = TRUE;
				}
				else if (GLOBAL_STRCMP(plSubModuleInfo, "monitor_get") == 0)
				{
					S32 k;
					CHAR_T plTmpBuf[32];
					FMISC_TempInfo lTempInfo;
					FMISC_StatusInfo lStatusInfo;
					MULT_CardVixsPro100Node *plNode;
					//GLOBAL_TRACE(("Module %d, Get Monitor\n", plHandle->m_InitParam.m_ModuleSlot));

					/*获取温度，输入输出有效、总码率，错误状态*/
					lParamHolder = XML_WarpAddNode(XMLSave, "monitor_status");

					CARD_SubModuleTempGet(plHandle->m_InitParam.m_ModuleSlot, &lTempInfo);
					CARD_SubModuleStatusGet(plHandle->m_InitParam.m_ModuleSlot, &lStatusInfo);

					XML_WarpAddNodeF64(lParamHolder, "temperature", lTempInfo.m_TempValueC);
					XML_WarpAddNodeF64(lParamHolder, "in_v_rate", lStatusInfo.m_ValidInputTsBitrate);
					XML_WarpAddNodeF64(lParamHolder, "out_v_rate", lStatusInfo.m_ValidOutputTsBitrate);
					XML_WarpAddNodeF64(lParamHolder, "in_t_rate", lStatusInfo.m_TotalInputTsBitrate);
					XML_WarpAddNodeF64(lParamHolder, "out_t_rate", lStatusInfo.m_TotalOutputTsBitrate);


					for (i = 0; i < CARDTRANS_VIXSPRO100_CHIP_MAX_NUM; i++)
					{
						plNode = &plHandle->m_pPRO100Node[i];
						lSubHolder = XML_WarpAddNode(lParamHolder, "trans_chn");
						for (k = 0; k < CARDTRANS_VIXSPRO100_CHIP_SERV_CUR_NUM; k++)
						{
							lServHolder = XML_WarpAddNode(lSubHolder, "sub_chn");
							XML_WarpAddNodeBOOL(lServHolder, "enable", (plNode->m_pSerUniqueID[k] != 0)?TRUE:FALSE);
							if (plNode->m_pSerUniqueID[k] != 0)
							{
								XML_WarpAddNodeBOOL(lServHolder, "in_err", plNode->m_pSubInError[k]);
								XML_WarpAddNodeBOOL(lServHolder, "out_err", plNode->m_pSubOutError[k]);
							}
						}
					}

				}
				else if (GLOBAL_STRCMP(plSubModuleInfo, "param_support") == 0)
				{
					//GLOBAL_TRACE(("Module %d, Get Limit\n", plHandle->m_InitParam.m_ModuleSlot));
					lParamHolder = XML_WarpAddNode(XMLSave, "param_support");

					/*输入参数*/
					/*视频编码格式*/
					lParamA = MPEG2_PES_VID_FMT_MPEG2;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_in_fmt");
					XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
					XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_VIDFMTEnumToStr(lParamA));

					lParamA = MPEG2_PES_VID_FMT_MPEG4_AVC_H264;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_in_fmt");
					XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
					XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_VIDFMTEnumToStr(lParamA));

					/*视频分辨率和帧率*/
					for (i = MPEG2_PES_VID_RES_320_288I; i <= MPEG2_PES_VID_RES_720_576I; i++ )
					{
						lParamA = i;
						lParamB = MPEG2_PES_VID_FRAME_RATE_25;
						lSubHolder = XML_WarpAddNode(lParamHolder, "vid_in_res_fr");
						XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
						XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_VIDRESEnumToStr(lParamA), MPEG2_PES_VIDFREnumToStr(lParamB));
					}

					for (i = MPEG2_PES_VID_RES_320_240I; i <= MPEG2_PES_VID_RES_720_480I; i++ )
					{
						lParamA = i;
						lParamB = MPEG2_PES_VID_FRAME_RATE_2997;
						lSubHolder = XML_WarpAddNode(lParamHolder, "vid_in_res_fr");
						XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
						XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_VIDRESEnumToStr(lParamA), MPEG2_PES_VIDFREnumToStr(lParamB));
					}

					/*音频编码格式*/
					for (i = MPEG2_PES_AUD_FMT_MPEG1_L2; i < MPEG2_PES_AUD_FMT_BYPASS; i++ )
					{
						lParamA = i;
						lSubHolder = XML_WarpAddNode(lParamHolder, "aud_in_fmt");
						XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
						XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_AUDFMTEnumToStr(lParamA));
					}


					/*视频编码格式*/
					lParamA = MPEG2_PES_VID_FMT_MPEG2;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_fmt");
					XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
					XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_VIDFMTEnumToStr(lParamA));

					lParamA = MPEG2_PES_VID_FMT_MPEG4_AVC_H264;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_fmt");
					XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
					XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_VIDFMTEnumToStr(lParamA));

					/*输出参数*/
					/*视频分辨率和帧率*/
					for (i = MPEG2_PES_VID_RES_320_288I; i <= MPEG2_PES_VID_RES_720_576I; i++ )
					{
						lParamA = i;
						lParamB = MPEG2_PES_VID_FRAME_RATE_25;
						lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_res_fr");
						XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
						XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_VIDRESEnumToStr(lParamA), MPEG2_PES_VIDFREnumToStr(lParamB));
					}

					for (i = MPEG2_PES_VID_RES_320_240I; i <= MPEG2_PES_VID_RES_720_480I; i++ )
					{
						lParamA = i;
						lParamB = MPEG2_PES_VID_FRAME_RATE_2997;
						lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_res_fr");
						XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
						XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_VIDRESEnumToStr(lParamA), MPEG2_PES_VIDFREnumToStr(lParamB));
					}

					lParamA = MPEG2_PES_MPEG4_AVC_H264_PROFILE_MP;
					lParamB = MPEG2_PES_MPEG4_AVC_H264_LEVEL_3;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_h264_profile_level");
					XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
					XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_MPEG4AVCH264ProfileEnumToStr(lParamA), MPEG2_PES_MPEG4AVCH264LevelEnumToStr(lParamB));

					lParamA = MPEG2_PES_MPEG4_AVC_H264_PROFILE_MP;
					lParamB = MPEG2_PES_MPEG4_AVC_H264_LEVEL_4;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_h264_profile_level");
					XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
					XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_MPEG4AVCH264ProfileEnumToStr(lParamA), MPEG2_PES_MPEG4AVCH264LevelEnumToStr(lParamB));

					lParamA = MPEG2_PES_MPEG4_AVC_H264_PROFILE_MP;
					lParamB = MPEG2_PES_MPEG4_AVC_H264_LEVEL_41;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_h264_profile_level");
					XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
					XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_MPEG4AVCH264ProfileEnumToStr(lParamA), MPEG2_PES_MPEG4AVCH264LevelEnumToStr(lParamB));


					lParamA = MPEG2_PES_MPEG4_AVC_H264_PROFILE_HP;
					lParamB = MPEG2_PES_MPEG4_AVC_H264_LEVEL_3;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_h264_profile_level");
					XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
					XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_MPEG4AVCH264ProfileEnumToStr(lParamA), MPEG2_PES_MPEG4AVCH264LevelEnumToStr(lParamB));

					lParamA = MPEG2_PES_MPEG4_AVC_H264_PROFILE_HP;
					lParamB = MPEG2_PES_MPEG4_AVC_H264_LEVEL_4;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_h264_profile_level");
					XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
					XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_MPEG4AVCH264ProfileEnumToStr(lParamA), MPEG2_PES_MPEG4AVCH264LevelEnumToStr(lParamB));

					lParamA = MPEG2_PES_MPEG4_AVC_H264_PROFILE_HP;
					lParamB = MPEG2_PES_MPEG4_AVC_H264_LEVEL_41;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_h264_profile_level");
					XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
					XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_MPEG4AVCH264ProfileEnumToStr(lParamA), MPEG2_PES_MPEG4AVCH264LevelEnumToStr(lParamB));


					lParamA = MPEG2_PES_MPEG2_PROFILE_MP;
					lParamB = MPEG2_PES_MPEG2_LEVEL_MAIN;
					lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_mpeg2_profile_level");
					XML_WarpAddNodeHEX(lSubHolder, "value", CARDTRANS_VIXSPRO100_ENCODER_PARAM(lParamA, lParamB), TRUE);
					XML_WarpAddNodeTextF(lSubHolder, "name", "%s@%s", MPEG2_PES_MPEG2ProfileEnumToStr(lParamA), MPEG2_PES_MPEG2LevelEnumToStr(lParamB));

					XML_WarpAddNodeS32(lParamHolder, "vid_out_rate_low", 0.5 * 1000 * 1000);
					XML_WarpAddNodeS32(lParamHolder, "vid_out_rate_max", 5.5 * 1000 * 1000);

					for (i = MPEG2_PES_VID_VBR; i <= MPEG2_PES_VID_CBR; i++ )
					{
						lParamA = i;
						lSubHolder = XML_WarpAddNode(lParamHolder, "vid_out_xbr");
						XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
						XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_VIDXBREnumToStr(lParamA));
					}

					for (i = MPEG2_PES_AUD_FMT_MPEG1_L2; i <= MPEG2_PES_AUD_FMT_BYPASS; i++ )
					{
						lParamA = i;
						lSubHolder = XML_WarpAddNode(lParamHolder, "aud_out_fmt");
						XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
						XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_AUDFMTEnumToStr(lParamA));
					}

					lParamA = MPEG2_PES_AUD_BITRATE_192K;
					lSubHolder = XML_WarpAddNode(lParamHolder, "aud_out_rate");
					XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
					XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_AUDBitrateEnumToStr(lParamA));

					lParamA = MPEG2_PES_AUD_SAMPLE_RATE_48K;
					lSubHolder = XML_WarpAddNode(lParamHolder, "aud_out_sampl");
					XML_WarpAddNodeS32(lSubHolder, "value", lParamA);
					XML_WarpAddNodeText(lSubHolder, "name", MPEG2_PES_AUDSAMPLEEnumToStr(lParamA));
					lRet = TRUE;
				}
			}
			else
			{
				GLOBAL_TRACE(("NULL XMLLoad\n"));
			}
		}

	}
	return lRet;
}

/*ICP接口*/
BOOL MULT_CardVixsPro100ICPProcessCB(void *pUserParam, U8 *pData, S32 DataSize)
{
	BOOL lRet = FALSE;
	MULT_CardVixsPro100Handle *plHandle = (MULT_CardVixsPro100Handle*)pUserParam;
	if (plHandle)
	{
		lRet = FUART_ICPRecv(plHandle->m_FUARTHandle, pData, DataSize);
		if (lRet == FALSE)
		{
			lRet = FGPIOV2_ICPRecv(plHandle->m_FGPIOHandle, pData, DataSize);
			if (lRet == FALSE)
			{
				lRet = FMISC_ICPRecv(plHandle->m_FMISCHandle, pData, DataSize);
				if (lRet == FALSE)
				{
					/*其它接口处理，没有则丢弃*/
				}
			}
		}
	}
	return lRet;
}


/* API函数 ---------------------------------------------------------------------------------------------------------------------------------------- */
/*注册函数*/
void MULT_CardVixsPro100Register(void)
{
	S32 i;
	CARD_ModuleParam lParam;
	GLOBAL_ZEROSTRUCT(lParam);

	lParam.m_ModuleType = CARD_MODULE_TYPE_TRANSCODER_VIXS_PRO100;
	GLOBAL_STRCPY(lParam.m_pModuleControlVersion, "01.00");
	lParam.m_pCMDCB = MULT_CardVixsPro100CMDProcessCB;
	lParam.m_pXMLCB = MULT_CardVixsPro100XMLProcessCB;

	lParam.m_pICPRecvCB = MULT_CardVixsPro100ICPProcessCB;
	GLOBAL_STRCPY(lParam.m_pFirstFPGAName, "module_0001");
	lParam.m_FPGANum = 1;
	GLOBAL_STRCPY(lParam.m_pModuleTag, "transcoder_sd_type_a");

	lParam.m_AlarmNum = CARDTRANS_VIXSPRO100_ALARM_NUM;

	lParam.m_ChnInfo.m_ChnNum = 3;
	for (i = 0; i < lParam.m_ChnInfo.m_ChnNum; i++)
	{
		GLOBAL_STRNCPY(lParam.m_ChnInfo.m_pChnInfo[i].m_pCHNTypeTag, "transcoder_sd_type_a_ch", sizeof(lParam.m_ChnInfo.m_pChnInfo[i].m_pCHNTypeTag));
		lParam.m_ChnInfo.m_pChnInfo[i].m_MToSTsCount = 0;
		lParam.m_ChnInfo.m_pChnInfo[i].m_SToMTsCount = 0;
	}

	CARD_ModuleRegister(&lParam);
}
#endif

/*EOF*/
