/* Includes-------------------------------------------------------------------- */
#include "multi_main_internal.h"

#ifdef ENCODER_CARD_PLATFORM

#ifdef CARD_TEST


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


//#define CARD_TEST_RS232//串口回环测试！
//#define CARD_TEST_RS232_DELAY//串口回环延时测试！
//#define CARD_TEST_TEMP
//#define CARD_TEST_GPIO
//#define CARD_TEST_SPI

#define CARD_TEST_PID_DELAY
//#define CARD_TOTAL_TEST

#ifdef CARD_PRO100_TRANS
//#define CARD_PRO100_TRANS
#define CARD_PRO100_TRANS_AUD_BYPASS
#define CARD_PRO100_TRANS_SINGLE_SERV_TEST //这个时候可以将TS从转码板的ASI接口转出，从而可以直接查看转码输出的TS流是否正常！
//#define CARD_PRO100_TRANS_AUD_MPEG4_AAC
#define USE_TRANSENC_LIB
//#define RESET_PRO100
#endif

#ifdef CARD_TOTAL_TEST
#define CARD_TEST_RS232//串口回环测试！
#define CARD_TEST_RS232_DELAY//串口回环延时测试！
#define CARD_TEST_TEMP
#define CARD_TEST_GPIO
#define CARD_TEST_PID_DELAY
#define CARD_TEST_SPI
#endif

#ifdef CARD_TEST
/* Global Variables (extern)--------------------------------------------------- */
extern MULT_Handle *g_TestHandle;
/* Macro (local)--------------------------------------------------------------- */
#define CARD_TESTTASK_STATCK_SIZE				(1024*1024)
#define CARD_TESTRS232_NUM						(3)
#define CARD_TESTIO_BUF_SIZE					(64 * 1024)
#define CARD_TESTPID_DELAY_SLOT_NUM				(12)
#define CARDTESET_PRO100_CHN_NUM				(2)
#define CARDTESET_PRO100_MAX_CHN_NUM			(4)
#define CARDTESET_PRO100_CHN_PID_RESERVED		(100)
#define CARDTESET_PRO100_CHN_VID_PID_RESERVED	(10)
#ifdef CARD_PRO100_TRANS_SINGLE_SERV_TEST
#define CARDTESET_PRO100_NUM					(1)
#else
#define CARDTESET_PRO100_NUM					(3)
#endif
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef void (*MULT_CARDTRANS_CB)(S32 SlotIndex, S32 TsIndex, U16 PID);

typedef struct
{
	BOOL			m_bEncoder;
	MULT_CARDTRANS_CB	m_SampleCB;


	S32				m_ModuleSlot;
}CARD_TESTInitParam;

typedef struct
{
	S32				m_Slot;
	HANDLE32		m_UARTHandle;
}CARD_TESTPro100Node;

typedef struct
{
	CARD_TESTInitParam		m_InitParam;
	BOOL					m_TaskMark;
	HANDLE32				m_TaskHandle;

	U8						m_pSendBuf[CARD_TESTIO_BUF_SIZE];
	U8						m_pRecvBuf[CARD_TESTIO_BUF_SIZE];

	S32						m_RS232Error;
	U32						m_ReadTick;
	U32						m_WriteTick;
	HANDLE32				m_UARTHandle;

	HANDLE32				m_TemperatureHandle;

	HANDLE32				m_FMISCHandle;

	HANDLE32				m_GPIOHandle;

	FSPIF_Param				m_SPIFlashParam;
	FSPIF_FlashStructure	m_SPIFlashStruct;
	HANDLE32				m_SPIHandle;

	HANDLE32				m_VIXSPro100Handle[CARDTESET_PRO100_NUM];
	CARD_TESTPro100Node		m_pVIXSPro100Node[CARDTESET_PRO100_NUM];
	CHAR_T					m_pFirmwareVersion[128];

	BOOL					m_bReseted;
}CARD_TESTHandle;


/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

/* 本地函数 --------------------------------------------------------------------------------------------------------------------------------------- */

/*编码芯片接口函数*/
BOOL CARD_TESTResetCB(void *pUserParam)
{
	return TRUE;
}

S32 CARD_TESTUARTReadCB(void *pUserParam, U8 *pData, S32 DataSize)
{
	CARD_TESTPro100Node *plNode = (CARD_TESTPro100Node*)pUserParam;
	return FUART_Read(plNode->m_UARTHandle, plNode->m_Slot, pData, DataSize);
}

S32 CARD_TESTUARTWriteCB(void *pUserParam, U8 *pData, S32 DataSize)
{
	CARD_TESTPro100Node *plNode = (CARD_TESTPro100Node*)pUserParam;
	return FUART_Write(plNode->m_UARTHandle, plNode->m_Slot, pData, DataSize);
}


/* 线程函数 */
void CARD_TESTTaskFn(void *pUserParam)
{
	CARD_TESTHandle *plHandle = (CARD_TESTHandle*) pUserParam;

	GLOBAL_TRACE(("Module %d, Thread Enter At %08X !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", plHandle->m_InitParam.m_ModuleSlot, PFC_GetTickCount()));

	if (plHandle)
	{
		U32 lTick;
		S32 lDuration;

#ifdef CARD_TEST_SPI
		S32 lSlotInd = 2;
		U32 lTempTick = 0;
		FSPIF_Identfication lID;
		FSPIF_StatusReg lStatus;
		FSPIF_FlagStatusReg lFStatus;
#endif

#ifdef CARD_TEST_GPIO
		S32 lGPIOTimeout = 2000;
		U32	lGPIOGetTick;
		S32	lPINValue = 0;
#endif
#ifdef CARD_TEST_PID_DELAY
		U32	lDelayTick;
#endif

#ifdef CARD_TEST_TEMP
		S32 lTemperatureTimeout = 2000;
		U32	lTempGetTick;
#endif

#ifdef CARD_TEST_RS232
		S32 lActRead;
		S32 lActWrite;
		S32 lTotalByte;
		S32 lTotalCount = 0;
		S32 lOKCount = 0;
		S32 lRS232Timeout = 4000;
		lTotalByte = 0;
		plHandle->m_RS232Error = 0;
		FUART_Flush(plHandle->m_UARTHandle, 0);
#endif

		plHandle->m_TaskMark = TRUE;
		lTick = PFC_GetTickCount();


#ifdef CARD_TEST_GPIO
		{
			S32 i;
			for (i = 0; i < 6; i ++)
			{
				FGPIOV2_IOMaskSetPIN(plHandle->m_GPIOHandle, 0, i, FALSE);
			}
		}
#endif

#ifdef CARD_PRO100_TRANS
		{
			S32 i, k, lSerInd, lEsLoopCount;
			S32 lMainInTsInd, lSubInTsInd;
			HANDLE32 lDBHandle;
			MPEG2_DBServiceInInfo lServInInfo;
			MPEG2_DBServiceOutInfo lServOutInfo;
			MPEG2_DBEsInInfo lEsInInfo;
			MPEG2_DBDescriptorInfo lDescInfo;
			MPEG2_DBEsOutInfo lEsOutInfo;
			U32 lServIDs, lEsIDs, lDelayTick;
			VIXSPRO100_ConfigParam plCfgPara[2];


			/*设置直通模块，暂时不使用，因为每个芯片均能够转码两个音频PID，暂时也没有其它PID需要BYPASS！*/
			FMISC_DelayInfo lDelayInfo;

			for (i = 0; i < 12; i++)
			{
				GLOBAL_ZEROSTRUCT(lDelayInfo);

				//if (i == 0 || i == 1)
				//{
				//	lDelayInfo.m_bEnable[0] = TRUE;
				//	lDelayInfo.m_bModifyPTS[0] = FALSE;
				//	lDelayInfo.m_PID[0] = 111 + i * CARDTESET_PRO100_CHN_PID_RESERVED;
				//	GLOBAL_TRACE(("CHN %d Bypass PID = %d\n", i, lDelayInfo.m_PID[0]));
				//}

				lDelayTick  = PFC_GetTickCount();
				if (FMISC_DataProcess(plHandle->m_FMISCHandle, i, 500, FMISC_TYPE_TS_DELAY_SET, &lDelayInfo, sizeof(lDelayInfo)))
				{
					GLOBAL_TRACE(("Set PID Delay OK!!!! CHN = %d, Duraion = %d\n", i, PFC_GetTickCount() - lDelayTick));
				}
				else
				{
					GLOBAL_TRACE(("Set PID Delay Failed!!!!  CHN = %d, DDuraion = %d\n", i, PFC_GetTickCount() - lDelayTick));
				}

			}

			lDBHandle = CARD_SubModuleRemuxGet();


			/*自动设置网口输入节目信息和输出到转码板的信息！，注意输入TS限定！！！*/
			for (k = 0; k < 3; k++)
			{
				lMainInTsInd = plHandle->m_InitParam.m_ModuleSlot * 3 + k;
				/*清除输入TS参数*/
				MPEG2_DBClearInTs(lDBHandle, lMainInTsInd);
				for (i = 0; i < CARDTESET_PRO100_CHN_NUM; i++)
				{
					lSerInd = (i + k * CARDTESET_PRO100_CHN_NUM);

					lServIDs = MPEG2_DBAddService(lDBHandle, TRUE, lMainInTsInd);

					/*设置虚拟节目的Service参数以及PCR映射*/
					MPEG2_DBGetServiceInInfo(lDBHandle, lServIDs, &lServInInfo);
					GLOBAL_SPRINTF((lServInInfo.m_ServiceName, "Ts [%d], Sub [%d]", lMainInTsInd + 1, i + 1));
					lServInInfo.m_PMTPID = 176 + i;
					lServInInfo.m_PCRPID = 160 + i;
					lServInInfo.m_PMTVersion = 0;
					lServInInfo.m_ServiceID = 1 + i;
					lServInInfo.m_ServiceType = MPEG2_SERVICE_TYPE_DTV;
					MPEG2_DBSetServiceInInfo(lDBHandle, lServIDs, &lServInInfo);

					/*建立虚拟节目的视频PID映射*/
					lEsIDs = MPEG2_DBAddEs(lDBHandle, lServIDs);

					lEsInInfo.m_EsPID = 128 + i;
					lEsInInfo.m_EsType = MPEG2_STREAM_TYPE_MPEG2_VIDEO;
					MPEG2_DBSetEsInInfo(lDBHandle, lEsIDs, &lEsInInfo);

					/*建立虚拟节目的音频PID映射*/
					lEsIDs = MPEG2_DBAddEs(lDBHandle, lServIDs);
					lEsInInfo.m_EsPID = 144 + i;
					lEsInInfo.m_EsType = MPEG2_STREAM_TYPE_MPEG1_AUDIO;
					MPEG2_DBSetEsInInfo(lDBHandle, lEsIDs, &lEsInInfo);

					/*复用对应转码板输入TS上（子板的0~11输入TS，6个节目按照 1 2， 5 6， 9 10进行设置）！！*/
					MPEG2_DBTsServiceRoute(lDBHandle, lServIDs, 512 + plHandle->m_InitParam.m_ModuleSlot * 256 + k * CARDTESET_PRO100_MAX_CHN_NUM + i);


					MPEG2_DBGetServiceOutInfo(lDBHandle, lServIDs, &lServOutInfo);
					lServOutInfo.m_PMTPID = 100 + lSerInd * CARDTESET_PRO100_CHN_PID_RESERVED;
					lServOutInfo.m_PCRPID = 2001 + lSerInd;
					MPEG2_DBSetServiceOutInfo(lDBHandle, lServIDs, &lServOutInfo);

					lEsIDs = 0;
					lEsLoopCount = 0;
					while((lEsIDs = MPEG2_DBGetEsNextNode(lDBHandle, lEsIDs, lServIDs)) != 0)
					{
						MPEG2_DBGetEsOutInfo(lDBHandle, lEsIDs, &lEsOutInfo);

						if (lEsOutInfo.m_EsType == MPEG2_STREAM_TYPE_MPEG2_VIDEO)
						{
							lEsOutInfo.m_EsPID = lServOutInfo.m_PMTPID + 1;
						}
						else if (lEsOutInfo.m_EsType == MPEG2_STREAM_TYPE_MPEG1_AUDIO)
						{
							lEsOutInfo.m_EsPID = lServOutInfo.m_PMTPID + 2;
						}

						MPEG2_DBSetEsOutInfo(lDBHandle, lEsIDs, &lEsOutInfo);
					}
				}
			}




			/*自动设置转码板输出的TS，并将其复用到输出TS0~5!*/
			lSubInTsInd = 512 + plHandle->m_InitParam.m_ModuleSlot * 256;

			/*清除输入TS参数*/
			MPEG2_DBClearInTs(lDBHandle, lSubInTsInd);

			for (k = 0; k < CARDTESET_PRO100_NUM; k++)
			{
				for (i = 0; i < CARDTESET_PRO100_CHN_NUM; i++)
				{
					lSerInd = (i + k * CARDTESET_PRO100_CHN_NUM);
					/*在子板第一输入TS上生成虚拟节目*/
					lServIDs = MPEG2_DBAddService(lDBHandle, TRUE, lSubInTsInd);

					/*设置虚拟节目的Service参数以及PCR映射*/
					MPEG2_DBGetServiceInInfo(lDBHandle, lServIDs, &lServInInfo);
					GLOBAL_SPRINTF((lServInInfo.m_ServiceName, "Card[%d],Chip[%d],Slot[%d]", plHandle->m_InitParam.m_ModuleSlot, k, i));
					lServInInfo.m_PMTPID = 100 + lSerInd * CARDTESET_PRO100_CHN_PID_RESERVED;
					lServInInfo.m_PCRPID = lServInInfo.m_PMTPID + 1;
					lServInInfo.m_PMTVersion = 0;
					lServInInfo.m_ServiceID = 1;
					lServInInfo.m_ServiceType = MPEG2_SERVICE_TYPE_DTV;
					MPEG2_DBSetServiceInInfo(lDBHandle, lServIDs, &lServInInfo);

					/*建立虚拟节目的视频PID映射*/
					lEsIDs = MPEG2_DBAddEs(lDBHandle, lServIDs);

					lEsInInfo.m_EsPID = lServInInfo.m_PCRPID;
					//lEsInInfo.m_EsType = MPEG2_STREAM_TYPE_MPEG2_VIDEO;
					lEsInInfo.m_EsType = MPEG2_STREAM_TYPE_H264_VIDEO;
					MPEG2_DBSetEsInInfo(lDBHandle, lEsIDs, &lEsInInfo);

					/*建立虚拟节目的音频PID映射*/
					lEsIDs = MPEG2_DBAddEs(lDBHandle, lServIDs);
					lEsInInfo.m_EsPID = lServInInfo.m_PCRPID + 1/*CARDTESET_PRO100_CHN_VID_PID_RESERVED*/;
#ifdef CARD_PRO100_TRANS_AUD_BYPASS
					lEsInInfo.m_EsType = MPEG2_STREAM_TYPE_MPEG1_AUDIO;
#else
#ifdef CARD_PRO100_TRANS_AUD_MPEG4_AAC
					lEsInInfo.m_EsType = MPEG2_STREAM_TYPE_AUDIO_WITH_ADTS;
					lDescInfo.m_OutputMark = TRUE;
					lDescInfo.m_pDescriptorData[0] = 0x52;
					lDescInfo.m_pDescriptorData[1] = 0x01;
					lDescInfo.m_pDescriptorData[2] = 0x10;
					lDescInfo.m_DescriptorDataSize = 3;
					MPEG2_DBAddDescriptor(lDBHandle, lEsIDs, MPEG2_DESCRIPTOR_PMT_ES_INFO, &lDescInfo);
#else
					lEsInInfo.m_EsType = MPEG2_STREAM_TYPE_MPEG1_AUDIO;
#endif
#endif
					MPEG2_DBSetEsInInfo(lDBHandle, lEsIDs, &lEsInInfo);


					/*复用到输出TS0上！！*/
					//MPEG2_DBTsServiceRoute(lDBHandle, lServIDs, plHandle->m_InitParam.m_ModuleSlot);
					MPEG2_DBTsServiceRoute(lDBHandle, lServIDs, 0);


#ifdef USE_TRANSENC_LIB
					{
						TRANSENC_Param lTransEncParam;
						GLOBAL_ZEROSTRUCT(lTransEncParam);

						if (TRANSENC_ParamProcess(plHandle->m_VIXSPro100Handle[k], i, &lTransEncParam, TRUE))
						{
							lTransEncParam.m_EncoderParam.m_VIDBitrate = 2500 * 1000;

							lTransEncParam.m_EncoderParam.m_PMTPID = lServInInfo.m_PMTPID;
							lTransEncParam.m_EncoderParam.m_PCRPID = lServInInfo.m_PCRPID;
							lTransEncParam.m_EncoderParam.m_VIDPID = lServInInfo.m_PCRPID;
							lTransEncParam.m_EncoderParam.m_AUDPID = lServInInfo.m_PCRPID + 1/*CARDTESET_PRO100_CHN_VID_PID_RESERVED*/;

							lTransEncParam.m_DecoderParam.m_VIDPID = lTransEncParam.m_EncoderParam.m_VIDPID;
							lTransEncParam.m_DecoderParam.m_AUDPID = lTransEncParam.m_EncoderParam.m_AUDPID;
							lTransEncParam.m_DecoderParam.m_PCRPID =  2001 + lSerInd;

							TRANSENC_ParamProcess(plHandle->m_VIXSPro100Handle[k], i, &lTransEncParam, FALSE);
						}
						else
						{
							GLOBAL_TRACE(("TRANSENC_ParamProcess [%d] Failed!\n", i));
						}
					}

#else
					{

						plCfgPara[i].m_WorkEn = TRUE;
						plCfgPara[i].m_TSIIndex = i;
#ifdef CARD_PRO100_TRANS_SINGLE_SERV_TEST
						if (i > 0)
						{
							plCfgPara[i].m_WorkEn = FALSE;
						}
#endif

						//plCfgPara[i].m_VideoFormat = VIXSPRO100_VID_FMT_MPEG2;
						plCfgPara[i].m_VideoFormat = VIXSPRO100_VID_FMT_H264;
						plCfgPara[i].m_BitrateMode = VIXSPRO100_VID_CBR; /* VBR/CBR */
#ifdef CARD_PRO100_TRANS_AUD_BYPASS
						plCfgPara[i].m_AudioFormat = VIXSPRO100_AUD_BYPASS;
#else
#ifdef CARD_PRO100_TRANS_AUD_MPEG4_AAC
						plCfgPara[i].m_AudioFormat = VIXSPRO100_AUD_MPEG4_AAC;
#else
						plCfgPara[i].m_AudioFormat = VIXSPRO100_AUD_MPEG1_L2;
#endif
#endif
						plCfgPara[i].m_VideoBitrate = 3500;
						plCfgPara[i].m_VideoResolutionFrameRate = VIXSPRO100_VID_720_576I50;
						plCfgPara[i].m_VideoAspectRatio = VIXSPRO100_VID_ASPECT_4X3;
						plCfgPara[i].m_VideoProfileLevel = VIXSPRO100_H264_HP_L30;

						plCfgPara[i].m_PmtPid = lServInInfo.m_PMTPID;
						plCfgPara[i].m_PcrPid = lServInInfo.m_PCRPID;
						plCfgPara[i].m_VideoPid = lServInInfo.m_PCRPID;
						plCfgPara[i].m_AudioPid = lServInInfo.m_PCRPID + 1/*CARDTESET_PRO100_CHN_VID_PID_RESERVED*/;

						/* 转码输入参数参数 */
						plCfgPara[i].m_VideoInResolutionFrameRate = VIXSPRO100_VID_720_576I50; 
						plCfgPara[i].m_VideoInFormat = VIXSPRO100_VID_FMT_MPEG2;
#ifdef CARD_PRO100_TRANS_AUD_BYPASS
						plCfgPara[i].m_AudioInFormat = VIXSPRO100_AUD_BYPASS;
#else
						plCfgPara[i].m_AudioInFormat = VIXSPRO100_AUD_MPEG1_L2;
#endif
						plCfgPara[i].m_VideoInPid = plCfgPara[i].m_VideoPid;
						plCfgPara[i].m_AudioInPid = plCfgPara[i].m_AudioPid;
						plCfgPara[i].m_PcrInPid = 2001 + lSerInd;

						GLOBAL_TRACE(("CHIP Ind = %d\n", k));
						GLOBAL_TRACE(("CHN [%d] IN TS = %d\n", i, plCfgPara[i].m_TSIIndex));
						GLOBAL_TRACE(("CHN [%d] IN VID PID = %d\n", i, plCfgPara[i].m_VideoInPid));
						GLOBAL_TRACE(("CHN [%d] IN AUD PID = %d\n", i, plCfgPara[i].m_AudioInPid));
						GLOBAL_TRACE(("CHN [%d] IN PCR PID = %d\n", i, plCfgPara[i].m_PcrInPid));

						GLOBAL_TRACE(("CHN [%d] Out PMT PID = %d\n", i, plCfgPara[i].m_PmtPid));
						GLOBAL_TRACE(("CHN [%d] Out PCR PID = %d\n", i, plCfgPara[i].m_PcrPid));
						GLOBAL_TRACE(("CHN [%d] Out VID PID = %d\n", i, plCfgPara[i].m_VideoPid));
						GLOBAL_TRACE(("CHN [%d] Out AUD PID = %d\n", i, plCfgPara[i].m_AudioPid));
					}
#endif
				}

				lDelayTick  = PFC_GetTickCount();
#ifdef USE_TRANSENC_LIB
				if (TRANSENC_ParamApply(plHandle->m_VIXSPro100Handle[k]))
				{
					GLOBAL_TRACE(("TRANSENC_ParamApply OK!!!! Duraion = %d\n", PFC_GetTickCount() - lDelayTick));
				}
				else
				{
					GLOBAL_TRACE(("TRANSENC_ParamApply Failed!!!! Duraion = %d\n", PFC_GetTickCount() - lDelayTick));
				}
#else
				if (VIXSPRO100_SetPara(plHandle->m_VIXSPro100Handle[k], plCfgPara, VIXSPRO100_CFGTYPE_TRANS))
				{
					GLOBAL_TRACE(("VIXSPRO100_SetPara OK!!!! Duraion = %d\n", PFC_GetTickCount() - lDelayTick));
				}
				else
				{
					GLOBAL_TRACE(("VIXSPRO100_SetPara Failed!!!! Duraion = %d\n", PFC_GetTickCount() - lDelayTick));
				}
#endif
			}
			/*生成新的XML供测试查看！*/
			MULTL_SetRemuxApplyMark(g_TestHandle, FALSE);
			MULTL_SetRemuxApplyMark(g_TestHandle, TRUE);

			return;
		}
#endif

		while(plHandle->m_TaskMark)
		{
			lDuration = PFC_GetTickCount() - lTick;
			lTick = PFC_GetTickCount();

			if (lDuration <= 0)
			{
				lDuration = 1;
			}


#ifdef CARD_TEST_PID_DELAY
			{
				S32 i, k;

				FMISC_DelayInfo lDelayInfo;
				for (k = 0; k < 12; k++)
				{
					GLOBAL_ZEROSTRUCT(lDelayInfo);

					if (k == 0)
					{
						i = 0;
						lDelayInfo.m_bEnable[i] = TRUE;
						lDelayInfo.m_bModifyPTS[i] = FALSE;
						lDelayInfo.m_PID[i] = 0;

						i ++;
						lDelayInfo.m_bEnable[i] = TRUE;
						lDelayInfo.m_bModifyPTS[i] = FALSE;
						lDelayInfo.m_PID[i] = 1;

						i ++;
						lDelayInfo.m_bEnable[i] = TRUE;
						lDelayInfo.m_bModifyPTS[i] = FALSE;
						lDelayInfo.m_PID[i] = 17;

						i ++;
						lDelayInfo.m_bEnable[i] = TRUE;
						lDelayInfo.m_bModifyPTS[i] = FALSE;
						lDelayInfo.m_PID[i] = 256;

						i ++;
						lDelayInfo.m_bEnable[i] = TRUE;
						lDelayInfo.m_bModifyPTS[i] = FALSE;
						lDelayInfo.m_PID[i] = 500;

						lDelayInfo.m_DelayMS = 500;
						lDelayInfo.m_PCRPID = 500;
					}

					lDelayTick  = PFC_GetTickCount();
					if (FMISC_DataProcess(plHandle->m_FMISCHandle, k, 500, FMISC_TYPE_TS_DELAY_SET, &lDelayInfo, sizeof(lDelayInfo)))
					{
						GLOBAL_TRACE(("Set PID Delay OK!!!!CHN = %d Duraion = %d\n", k, PFC_GetTickCount() - lDelayTick));
					}
					else
					{
						GLOBAL_TRACE(("Set PID Delay Failed!!!!CHN = %d  Duraion = %d\n", k, PFC_GetTickCount() - lDelayTick));
					}
				}

				PFC_TaskSleep(3000);
			}
#endif



#ifdef CARD_TEST_SPI
			{

				/*选择并复位芯片*/
				GLOBAL_TRACE(("Slot [%d] Select And Reset ON\n", lSlotInd));
				FSPI_ChipConfig(plHandle->m_SPIHandle, lSlotInd, TRUE, TRUE);
				PFC_TaskSleep(100);
				GLOBAL_TRACE(("Slot [%d] Select And Reset OFF\n", lSlotInd));
				FSPI_ChipConfig(plHandle->m_SPIHandle, lSlotInd, FALSE, TRUE);
				PFC_TaskSleep(1000);

				/*读取芯片ID号*/
				GLOBAL_TRACE(("Slot [%d] Read ID\n", lSlotInd));
				if (FSPIF_ReadIdentification(&plHandle->m_SPIFlashParam, lSlotInd, &lID))
				{
					GLOBAL_TRACE(("Slot = %d, Manufacturer = %02X, Type = %02X, Capacity = %02X\n", lSlotInd, lID.m_Manufacturer_ID, lID.m_DeviceType, lID.m_DeviceCapacity));
					//CAL_PrintDataBlock("UNI ID", lID.m_pUniqueID, sizeof(lID.m_pUniqueID));
					/*读数据操作*/
					{
						S32 lTotalSize;
						S32 lCurSize;
						S32 lReadSize;
						U32 lStartAddr;
						S32 lPerCur, lPerLast;

						PFC_System("rm -r /tmp/0.bin");

						lStartAddr = 0;
						lTotalSize = 32 * 1024 * 1024;

						lTempTick = PFC_GetTickCount();
						GLOBAL_TRACE(("Start Read Slot = %d ->> 00", lSlotInd));
						lCurSize = 0;

						FSPIF_SetWriteEnable(&plHandle->m_SPIFlashParam, lSlotInd, TRUE);
						FSPIF_WriteExtendedAddressRegisterEnable(&plHandle->m_SPIFlashParam, lSlotInd, FALSE);

						lPerLast = 0;
						while(lCurSize < lTotalSize)
						{
							if (lCurSize == 16 *  1024 * 1024)
							{
								FSPIF_SetWriteEnable(&plHandle->m_SPIFlashParam, lSlotInd, TRUE);
								FSPIF_WriteExtendedAddressRegisterEnable(&plHandle->m_SPIFlashParam, lSlotInd, TRUE);
							}
							lReadSize = 512;
							if (FSPIF_Read(&plHandle->m_SPIFlashParam, lSlotInd, lStartAddr, plHandle->m_pRecvBuf, lReadSize))
							{
								//CAL_PrintDataBlockWithASCIIEX(plHandle->m_pRecvBuf, lReadSize, 16, "Read Data At Slot = %d", lSlotInd);
								CAL_FileDataSave("/tmp/0.bin", TRUE, plHandle->m_pRecvBuf, lReadSize);

								lPerCur = (S32)((F64)lCurSize * 100  / lTotalSize);
								if (lPerCur != lPerLast)
								{
									GLOBAL_PRINTF(("\b\b%02d", lPerCur));
									GLOBAL_FFUSH(GLOBAL_STDOUT);
									lPerLast = lPerCur;
								}

								lCurSize += lReadSize;
								lStartAddr += lReadSize;

							}
							else
							{
								GLOBAL_TRACE(("Slot = %d, Read Data Failed!!\n", lSlotInd));
								break;
							}

						}
						GLOBAL_PRINTF(("\b\b100\n"));
						GLOBAL_TRACE(("Read Done!!! Total Bytes = %d Bitrate = %f Mbps\n", lCurSize, ((lCurSize * 8.0) / CAL_TimeDurationMS(&lTempTick, PFC_GetTickCount(), 1)) / 1000));
					}
				}
				else
				{
					GLOBAL_TRACE(("Slot = %d, Read ID Register Failed!!\n", lSlotInd));
				}

				GLOBAL_TRACE(("Slot [%d] Deselect And Reset OFF\n", lSlotInd));
				FSPI_ChipConfig(plHandle->m_SPIHandle, lSlotInd, FALSE, FALSE);

				PFC_TaskSleep(3000);
			}
#endif


#ifdef CARD_TEST_GPIO
			//if(CAL_TimeoutCheck(&lGPIOTimeout, lDuration))
			{
				//lGPIOGetTick  = PFC_GetTickCount();

				S32 i;

				lPINValue = 0;
				for (i = 3; i < 6; i ++)
				{
					FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, i, lPINValue);
				}

				if (FGPIOV2_Write(plHandle->m_GPIOHandle, 0))
				{
					GLOBAL_TRACE(("Write GPIO OK! Value = %d\n", lPINValue));
				}
				else
				{
					GLOBAL_TRACE(("Write GPIO Failed!\n"));
				}
				PFC_TaskSleep(1000);

				lPINValue = 0;
				for (i = 0; i < 3; i ++)
				{
					FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, i, lPINValue);
				}

				if (FGPIOV2_Write(plHandle->m_GPIOHandle, 0))
				{
					GLOBAL_TRACE(("Write GPIO OK! Value = %d\n", lPINValue));
				}
				else
				{
					GLOBAL_TRACE(("Write GPIO Failed!\n"));
				}

				PFC_TaskSleep(500);

				//lPINValue = 1;
				//for (i = 0; i < 3; i ++)
				//{
				//	FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, i, lPINValue);
				//}

				//if (FGPIOV2_Write(plHandle->m_GPIOHandle, 0))
				//{
				//	GLOBAL_TRACE(("Write GPIO OK! Value = %d\n", lPINValue));
				//}
				//else
				//{
				//	GLOBAL_TRACE(("Write GPIO Failed!\n"));
				//}

				//lPINValue = !lPINValue;
				//lGPIOTimeout = 2000;

				PFC_TaskSleep(5000);
			}
			//else
			//{
			//	PFC_TaskSleep(100);
			//}
#endif

#ifdef CARD_TEST_TEMP
			{
				FMISC_TempInfo lTempInfo;

				lTempGetTick  = PFC_GetTickCount();
				if (FMISC_DataProcess(plHandle->m_TemperatureHandle, 0, 1000, FMISC_TYPE_TEMPERATURE_GET, &lTempInfo, sizeof(lTempInfo)))
				{
					GLOBAL_TRACE(("Get Temperature = %f C\n", lTempInfo.m_TempValueC));
				}
				else
				{
					GLOBAL_TRACE(("Get Temperature Failed!!!! Duraion = %d\n", PFC_GetTickCount() - lTempGetTick));
				}
				lTemperatureTimeout = 1000;
				PFC_TaskSleep(2000);
			}
#endif

#ifdef CARD_TEST_RS232
			{
				lActWrite = 32;
				CAL_RandRandomFill(plHandle->m_pSendBuf, lActWrite);

				plHandle->m_WriteTick = PFC_GetTickCount();

				lTotalCount ++;
				if (FUART_Write(plHandle->m_UARTHandle, 0, plHandle->m_pSendBuf, lActWrite) == lActWrite)
				{

#ifdef CARD_TEST_RS232_DELAY
					GLOBAL_TRACE(("Write Complete! Len = %d, Delay = %d ms\n", lActWrite, PFC_GetTickCount() - plHandle->m_WriteTick));
#endif
					plHandle->m_ReadTick = PFC_GetTickCount();
					lActRead = FUART_Read(plHandle->m_UARTHandle, 0, plHandle->m_pRecvBuf, sizeof(plHandle->m_pRecvBuf));
					if (lActRead > 0)
					{
#ifdef CARD_TEST_RS232_DELAY
						GLOBAL_TRACE(("Read Complete! Len = %d, Delay = %dms\n", lActRead, PFC_GetTickCount() - plHandle->m_ReadTick));
#endif
						if (GLOBAL_MEMCMP(plHandle->m_pSendBuf, plHandle->m_pRecvBuf, lActRead) != 0)
						{
							CAL_PrintDataBlock("Write Data ", plHandle->m_pSendBuf, lActWrite);
							CAL_PrintDataBlock("Read Data ", plHandle->m_pRecvBuf, lActRead);
							plHandle->m_RS232Error++;
						}
						else
						{
#ifdef CARD_TEST_RS232_DELAY
							GLOBAL_TRACE(("RS232 Test Recv OK! Error Count = %d, OK/Total = %d/%d\n", lTotalByte * 8 * 1000 / CAL_TimeDurationMS(&lTick, PFC_GetTickCount(), 0), plHandle->m_RS232Error, lOKCount, lTotalCount));
#endif
						}
						lTotalByte += lActRead;
						lOKCount++;
					}
					else
					{
						GLOBAL_TRACE(("Read Timeout! Duraion = %d\n", PFC_GetTickCount() - plHandle->m_ReadTick));
					}
#ifdef CARD_TEST_RS232_DELAY
					PFC_TaskSleep(1000);
#endif
				}
				else
				{
					GLOBAL_TRACE(("Write Error! Duraion = %d\n", PFC_GetTickCount() - plHandle->m_WriteTick));
				}
				/*使用串口收发数据！*/

#ifndef CARD_TEST_RS232_DELAY
				if(CAL_TimeoutCheck(&lRS232Timeout, lDuration))
				{
					GLOBAL_TRACE(("RS232 Test Bitrate = %d bps, Recv Error Count = %d, OK/Total = %d/%d\n", lTotalByte * 8 * 1000 / CAL_TimeDurationMS(&lTick, PFC_GetTickCount(), 0), plHandle->m_RS232Error, lOKCount, lTotalCount));
					lTotalByte = 0;
					lRS232Timeout = 4000;
				}
#endif

			}
#endif

#ifndef CARD_TOTAL_TEST
			break;
#endif
			PFC_TaskSleep(10);
		}
	}
	GLOBAL_TRACE(("Thread Leave At %08X !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", PFC_GetTickCount()));
}


/* 创建句柄 */
HANDLE32 CARD_TESTCreate(CARD_TESTInitParam *pParam)
{
	CARD_TESTHandle *plHandle = (CARD_TESTHandle*) GLOBAL_ZMALLOC(sizeof(CARD_TESTHandle));
	if (plHandle)
	{
		GLOBAL_MEMCPY(&plHandle->m_InitParam, pParam, sizeof(CARD_TESTInitParam));
#if defined(CARD_TEST_RS232) || defined(CARD_PRO100_TRANS)
		{
			S32 i;
			FUART_InitParam lFRS232Init;
			FUART_PortParam lPortParam;
			GLOBAL_ZEROSTRUCT(lPortParam);
			GLOBAL_ZEROSTRUCT(lFRS232Init);

			GLOBAL_SNPRINTF((lFRS232Init.m_pHandleName, sizeof(lFRS232Init.m_pHandleName), "RS232 M[%d] ", pParam->m_ModuleSlot));
			lFRS232Init.m_pICPSendCB = CARD_SubModuleICPSend;
			lFRS232Init.m_pUserParam = plHandle;
			lFRS232Init.m_SlotNum = CARD_TESTRS232_NUM;

			plHandle->m_UARTHandle = FUART_Create(&lFRS232Init);
			lPortParam.m_ReadTimeOutMS = 1000;
			lPortParam.m_WriteTimeOutMS = 1000;
			for (i = 0; i < CARDTESET_PRO100_NUM; i ++)
			{
				FUART_Open(plHandle->m_UARTHandle, i, &lPortParam);
			}
		}
#endif

#if defined(CARD_TEST_TEMP)
		{
			FMISC_InitParam lFMISCInitParam;
			GLOBAL_ZEROSTRUCT(lFMISCInitParam);
			GLOBAL_SNPRINTF((lFMISCInitParam.m_pHandleName, sizeof(lFMISCInitParam.m_pHandleName), "FMISC TEMP M[%d] ", pParam->m_ModuleSlot));
			lFMISCInitParam.m_pUserParam = plHandle;
			lFMISCInitParam.m_pICPSendCB = CARD_SubModuleICPSend;
			lFMISCInitParam.m_NodeNum = 1;
			plHandle->m_TemperatureHandle = FMISC_Create(&lFMISCInitParam);
		}
#endif

#if defined(CARD_TEST_PID_DELAY) || defined(CARD_PRO100_TRANS)
		{
			FMISC_InitParam lFMISCInitParam;
			GLOBAL_ZEROSTRUCT(lFMISCInitParam);
			GLOBAL_SNPRINTF((lFMISCInitParam.m_pHandleName, sizeof(lFMISCInitParam.m_pHandleName), "FMISC PID Delay M[%d] ", pParam->m_ModuleSlot));
			lFMISCInitParam.m_pUserParam = plHandle;
			lFMISCInitParam.m_pICPSendCB = CARD_SubModuleICPSend;
			lFMISCInitParam.m_NodeNum = 12;
			plHandle->m_FMISCHandle = FMISC_Create(&lFMISCInitParam);
		}
#endif

#if defined(CARD_TEST_GPIO) || defined(CARD_PRO100_TRANS)
		{
			FGPIOV2_InitParam lGPIOInitParam;
			GLOBAL_ZEROSTRUCT(lGPIOInitParam);
			GLOBAL_SNPRINTF((lGPIOInitParam.m_pHandleName, sizeof(lGPIOInitParam.m_pHandleName), "GPIO M[%d] ", pParam->m_ModuleSlot));
			lGPIOInitParam.m_SlotNum = 4;
			lGPIOInitParam.m_pUserParam = plHandle;
			lGPIOInitParam.m_pICPSendCB = CARD_SubModuleICPSend;
			lGPIOInitParam.m_TimeoutMS = 1000;
			plHandle->m_GPIOHandle = FGPIOV2_Create(&lGPIOInitParam);

		}
#endif

#if defined(CARD_TEST_SPI) || defined(CARD_PRO100_TRANS)
		{
			FSPI_InitParam lInitParam;
			GLOBAL_ZEROSTRUCT(lInitParam);
			GLOBAL_SNPRINTF((lInitParam.m_pHandleName, sizeof(lInitParam.m_pHandleName), "SPI M[%d] ", pParam->m_ModuleSlot));
			lInitParam.m_SlotNum = 3;
			lInitParam.m_pUserParam = plHandle;
			lInitParam.m_pICPSendCB = CARD_SubModuleICPSend;
			lInitParam.m_TimeoutMS = 1000;
			plHandle->m_SPIHandle = FSPI_Create(&lInitParam);
		}
		{
			FSPIF_Param lParam;
			FSPIF_FlashStructure lFlash;

			GLOBAL_ZEROSTRUCT(lParam);

			lParam.m_MaxOnceReadSize = 1000;
			lParam.m_MaxOnceWriteSize = 1000;
			lParam.m_pSPICB = FSPI_WriteAndRead;
			lParam.m_pUserParam = plHandle->m_SPIHandle;

			lParam.m_bUseFastRead = TRUE;
			lParam.m_FastReadDummyClock = 8;

			GLOBAL_MEMCPY(&plHandle->m_SPIFlashParam, &lParam, sizeof(FSPIF_Param));

			lFlash.m_PageSize = 256;
			lFlash.m_SectorSize = 64 * 1024;
			lFlash.m_TotalBytes = 32 * 1024 *1024;
			lFlash.m_EraseOperationTimeout = 1000;
			lFlash.m_ProgramOperationTimeout = 100;

			GLOBAL_MEMCPY(&plHandle->m_SPIFlashStruct, &lFlash, sizeof(FSPIF_FlashStructure));
		}
#endif

#ifdef CARD_PRO100_TRANS
		{
			S32 i;
#ifdef USE_TRANSENC_LIB
			TRANSENC_InitParameter lTRANSENCInitPara;

			GLOBAL_ZEROSTRUCT(lTRANSENCInitPara);

			lTRANSENCInitPara.m_SystemType = TRANSENC_SYSTEM_TYPE_TRANSCODER;
			lTRANSENCInitPara.m_DecoderType = TRANSENC_DECODER_TYPE_PRO100;
			lTRANSENCInitPara.m_EncoderType = TRANSENC_DECODER_TYPE_PRO100;
			lTRANSENCInitPara.m_pResetCB = CARD_TESTResetCB;
			lTRANSENCInitPara.m_pUartReadCB = CARD_TESTUARTReadCB;
			lTRANSENCInitPara.m_pUartWriteCB = CARD_TESTUARTWriteCB;

			for (i = 0; i < CARDTESET_PRO100_NUM; i++)
			{
				plHandle->m_pVIXSPro100Node[i].m_UARTHandle = plHandle->m_UARTHandle;
				plHandle->m_pVIXSPro100Node[i].m_Slot = i;
				lTRANSENCInitPara.m_pUserParam = &plHandle->m_pVIXSPro100Node[i];
				plHandle->m_VIXSPro100Handle[i] = TRANSENC_Create(&lTRANSENCInitPara);
			}
#else
			VIXSPRO100_InitParam lInitPara;

			/*建立转码控制句柄！*/
			for (i = 0; i < CARDTESET_PRO100_NUM; i++)
			{
				lInitPara.m_pReset = CARD_TESTResetCB;
				lInitPara.m_pUartRead = CARD_TESTUARTReadCB;
				lInitPara.m_pUartWrite = CARD_TESTUARTWriteCB;
				lInitPara.m_ChNum = 2;
				plHandle->m_pVIXSPro100Node[i].m_UARTHandle = plHandle->m_UARTHandle;
				plHandle->m_pVIXSPro100Node[i].m_Slot = i;
				lInitPara.m_pUserParam = &plHandle->m_pVIXSPro100Node[i];
				plHandle->m_VIXSPro100Handle[i] = VIXSPRO100_Create(&lInitPara);
			}
#endif
		}
#endif
		GLOBAL_TRACE(("Module %d, TestModule Create Succesfull!\n", pParam->m_ModuleSlot));
	}
	return plHandle;
}

/* 启动线程 */
void CARD_TESTStart(HANDLE32 Handle)
{
	CARD_TESTHandle *plHandle = (CARD_TESTHandle*)Handle;
	if (plHandle)
	{
		plHandle->m_TaskHandle = PFC_TaskCreate("Module Task", CARD_TESTTASK_STATCK_SIZE, CARD_TESTTaskFn, 0, plHandle);
	}
}

/* 关闭线程 */
void CARD_TESTStop(HANDLE32 Handle)
{
	CARD_TESTHandle *plHandle = (CARD_TESTHandle*)Handle;
	if (plHandle)
	{
		if (plHandle->m_TaskHandle)
		{
			plHandle->m_TaskMark = FALSE;
			if (PFC_TaskWait(plHandle->m_TaskHandle, GLOBAL_INVALID_INDEX))
			{
				GLOBAL_TRACE(("Task Closed\n"));
			}
			plHandle->m_TaskHandle = NULL;
		}
	}
}


/* 模块销毁 */
void CARD_TESTDestroy(HANDLE32 Handle)
{
	CARD_TESTHandle *plHandle = (CARD_TESTHandle*)Handle;
	if (plHandle)
	{
		//CARD_TESTStop(Handle);
#if defined(CARD_TEST_TEMP) || defined(CARD_PRO100_TRANS)
		FMISC_Destroy(plHandle->m_TemperatureHandle);
#endif

#if defined(CARD_TEST_PID_DELAY) || defined(CARD_PRO100_TRANS)
		FMISC_Destroy(plHandle->m_FMISCHandle);
#endif

#if defined(CARD_TEST_RS232) || defined(CARD_PRO100_TRANS)
		FUART_Destroy(plHandle->m_UARTHandle);
#endif

#if defined(CARD_TEST_GPIO) || defined(CARD_PRO100_TRANS)
		FGPIOV2_Destroy(plHandle->m_GPIOHandle);
#endif
		GLOBAL_FREE(plHandle);

#if defined(CARD_TEST_SPI) || defined(CARD_PRO100_TRANS)
		FSPI_Destroy(plHandle->m_SPIHandle);
#endif

#if defined(CARD_PRO100_TRANS)
		{
			S32 i;
			for (i = 0; i < CARDTESET_PRO100_NUM; i++)
			{
#ifdef USE_TRANSENC_LIB
				TRANSENC_Destroy(plHandle->m_VIXSPro100Handle[i]);
#else
				VIXSPRO100_Destroy(plHandle->m_VIXSPro100Handle[i]);
#endif
			}
		}
#endif
		plHandle = NULL;
	}
}



/*CARD_SYSTEM 接口*/
BOOL CARD_TESTCMDProcessCB(void *pUserParam, S32 CMDOPType, void *pOPParam, S32 OPParamSize)
{
	BOOL lRet = FALSE;

	GLOBAL_TRACE(("CMD Type = %d\n", CMDOPType));

	if (CMDOPType == CARD_CMD_OP_TYPE_CREATE)
	{
		HANDLE32 lHandle;

		CARD_TESTInitParam lParam;
		GLOBAL_ZEROSTRUCT(lParam);

		lParam.m_bEncoder = TRUE;
		lParam.m_ModuleSlot = *((S32*)pOPParam);
		lHandle = CARD_TESTCreate(&lParam);

		(*(void **)pUserParam) = lHandle;

		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_PREPARE)
	{
#ifdef CARD_PRO100_TRANS
		CARD_TESTHandle *plHandle = (CARD_TESTHandle*)pUserParam;
		if (plHandle)
		{
			S32 i;
			BOOL blOK;
			CHAR_T plVersion[128];

			while(TRUE)
			{
				GLOBAL_TRACE(("Module %d, Deselect And Reset OFF\n", plHandle->m_InitParam.m_ModuleSlot));
				blOK = TRUE;
				for (i = 0; i < CARDTESET_PRO100_NUM; i++)
				{
					if (FSPI_ChipConfig(plHandle->m_SPIHandle, i, FALSE, FALSE) == FALSE)
					{
						blOK = FALSE;
						break;
					}
				}

				if (blOK == TRUE)
				{
					break;
				}
			}

			PFC_TaskSleep(100);

			for (i = 0; i < 6; i ++)
			{
				FGPIOV2_IOMaskSetPIN(plHandle->m_GPIOHandle, 0, i, FALSE);
			}

			for (i = 0; i < CARDTESET_PRO100_NUM; i++)
			{
#ifdef USE_TRANSENC_LIB
				if (TRANSENC_FirmwareVersionGet(plHandle->m_VIXSPro100Handle[i], plVersion) == FALSE)
#else
				if (VIXSPRO100_GetVersion(plHandle->m_VIXSPro100Handle[i], plVersion) == FALSE)
#endif
				{
					GLOBAL_TRACE(("Module %d, Get Version Failed At Chip[%d]\n", plHandle->m_InitParam.m_ModuleSlot, i));
					break;
				}
			}

			if (i < CARDTESET_PRO100_NUM)
			{
				GLOBAL_TRACE(("Module %d, Enable 3.3V\n", plHandle->m_InitParam.m_ModuleSlot));
				/*控制GPIO口，对芯片进行复位操作*/
				/*打开3.3V（高有效）*/
				FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, 3, 1);
				FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, 4, 1);
				FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, 5, 1);
				FGPIOV2_Write(plHandle->m_GPIOHandle, 0);
				PFC_TaskSleep(1000);

				GLOBAL_TRACE(("Module %d, Reset Enable\n", plHandle->m_InitParam.m_ModuleSlot));
				/*开始复位（低有效）*/
				FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, 0, 0);
				FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, 1, 0);
				FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, 2, 0);
				FGPIOV2_Write(plHandle->m_GPIOHandle, 0);
				PFC_TaskSleep(500);

				GLOBAL_TRACE(("Module %d, Reset Disable\n", plHandle->m_InitParam.m_ModuleSlot));
				/*结束复位复位（低有效）*/
				for (i = 0; i < CARDTESET_PRO100_NUM; i++)
				{
					FGPIOV2_ValueSetPIN(plHandle->m_GPIOHandle, 0, i, 1);
				}
				FGPIOV2_Write(plHandle->m_GPIOHandle, 0);

				plHandle->m_bReseted = TRUE;
			}
			//else
			//{
			//	plHandle->m_bReseted = FALSE;
			//}
		}
#endif

		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_CHECK_OK)
	{
		/*调用接口函数，确定子模块已经在正常工作，并可以接收参数设置！*/
#ifdef CARD_PRO100_TRANS
		{
			S32 i;
			CARD_TESTHandle *plHandle = (CARD_TESTHandle*)pUserParam;
			for (i = 0; i < CARDTESET_PRO100_NUM; i++)
			{
#ifdef USE_TRANSENC_LIB
				if (TRANSENC_FirmwareVersionGet(plHandle->m_VIXSPro100Handle[i], plHandle->m_pFirmwareVersion) == FALSE)
#else
				if (VIXSPRO100_GetVersion(plHandle->m_VIXSPro100Handle[i], plHandle->m_pFirmwareVersion) == FALSE)
#endif
				{
					GLOBAL_TRACE(("Module %d, Slot = %d, Get Version Failed\n", plHandle->m_InitParam.m_ModuleSlot, i));
					break;
				}
				else
				{
					GLOBAL_TRACE(("Module %d, Slot = %d, PRO100 Firmvare Version = [%s]\n", plHandle->m_InitParam.m_ModuleSlot,i,  plHandle->m_pFirmwareVersion));
				}
			}

			if (i < CARDTESET_PRO100_NUM)
			{
				(*(S32*)pOPParam) = 0;
			}
			else
			{
				(*(S32*)pOPParam) = 1;
			}
		}
#else
		(*(S32*)pOPParam) = 1000;
#endif
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_GET_FIRMWARE_VERSION)
	{
		CARD_TESTHandle *plHandle = (CARD_TESTHandle*)pUserParam;
		if (plHandle)
		{
			GLOBAL_STRNCPY((CHAR_T*)pOPParam, plHandle->m_pFirmwareVersion, OPParamSize - 1);
			lRet = TRUE;
		}
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_APPLY)
	{
		//CARD_TESTStop(pUserParam);
		//CARD_TESTStart(pUserParam);

		CARD_TESTTaskFn(pUserParam);
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_DESTROY)
	{
		CARD_TESTDestroy(pUserParam);
		lRet = TRUE;
	}
	else if (CMDOPType == CARD_CMD_OP_TYPE_GET_MODULE_INIT_DELAY_MS)
	{
#ifdef CARD_PRO100_TRANS
		CARD_TESTHandle *plHandle = (CARD_TESTHandle*)pUserParam;
		if (plHandle->m_bReseted == TRUE)
		{
			(*(S32*)pOPParam) = 60 * 1000;
		}
		else
		{
			(*(S32*)pOPParam) = 1000;
		}
#else
		(*(S32*)pOPParam) = 10;
#endif
		lRet = TRUE;
	}
	else
	{

	}
	return lRet;
}

BOOL CARD_TESTXMLProcessCB(void *pUserParam, HANDLE32 XMLLoad, HANDLE32 MXMLHandleSave, S32 XMLOPType)
{
	BOOL lRet = FALSE;
	CARD_TESTHandle *plHandle = (CARD_TESTHandle*)pUserParam;
	if (plHandle)
	{

	}
	return lRet;
}

BOOL CARD_TESTICPProcessCB(void *pUserParam, U8 *pData, S32 DataSize)
{
	BOOL lRet = FALSE;
	CARD_TESTHandle *plHandle = (CARD_TESTHandle*)pUserParam;
	if (plHandle)
	{
#if defined(CARD_TEST_RS232) || defined(CARD_PRO100_TRANS)
		lRet = FUART_ICPRecv(plHandle->m_UARTHandle, pData, DataSize);
		if (lRet == FALSE)
		{
			/*其它接口处理*/
		}
#endif
#if defined(CARD_TEST_TEMP) || defined(CARD_PRO100_TRANS)
		lRet = FMISC_ICPRecv(plHandle->m_TemperatureHandle, pData, DataSize);
		if (lRet == FALSE)
		{
			/*其它接口处理*/

		}
#endif
#if defined(CARD_TEST_GPIO) || defined(CARD_PRO100_TRANS)
		lRet = FGPIOV2_ICPRecv(plHandle->m_GPIOHandle, pData, DataSize);
		if (lRet == FALSE)
		{
			/*其它接口处理*/

		}
#endif
#if defined(CARD_TEST_PID_DELAY) || defined(CARD_PRO100_TRANS)
		lRet = FMISC_ICPRecv(plHandle->m_FMISCHandle, pData, DataSize);
		if (lRet == FALSE)
		{
			/*其它接口处理*/

		}
#endif

#if defined(CARD_TEST_SPI) || defined(CARD_PRO100_TRANS)
		lRet = FSPI_ICPRecv(plHandle->m_SPIHandle, pData, DataSize);
		if (lRet == FALSE)
		{
			/*其它接口处理*/

		}
#endif


	}
	return lRet;
}
#endif

/* API函数 ---------------------------------------------------------------------------------------------------------------------------------------- */

void CARD_TESTRegister(void)
{
#ifdef CARD_TEST
	S32 i;
	CARD_ModuleParam lParam;
	GLOBAL_ZEROSTRUCT(lParam);

	lParam.m_ModuleType = CARD_MODULE_TYPE_TRANSCODER_VIXS_PRO100;
	GLOBAL_STRCPY(lParam.m_pModuleControlVersion, "01.00");
	lParam.m_pCMDCB = CARD_TESTCMDProcessCB;
	lParam.m_pXMLCB = CARD_TESTXMLProcessCB;

	lParam.m_pICPRecvCB = CARD_TESTICPProcessCB;
	GLOBAL_STRCPY(lParam.m_pFirstFPGAName, "module_0001");
	lParam.m_FPGANum = 1;
	GLOBAL_STRCPY(lParam.m_pModuleTag, "transcoder_sd_type_a");

	lParam.m_ChnInfo.m_ChnNum = 3;
	for (i = 0; i < lParam.m_ChnInfo.m_ChnNum; i++)
	{
		GLOBAL_STRNCPY(lParam.m_ChnInfo.m_pChnInfo[i].m_pCHNTypeTag, "transcoder_sd_type_a", sizeof(lParam.m_ChnInfo.m_pChnInfo[i].m_pCHNTypeTag));
		lParam.m_ChnInfo.m_pChnInfo[i].m_MToSTsCount = 12;
		lParam.m_ChnInfo.m_pChnInfo[i].m_SToMTsCount = 1;
	}

	CARD_ModuleRegister(&lParam);
#endif
}
#endif

#endif

/*EOF*/
