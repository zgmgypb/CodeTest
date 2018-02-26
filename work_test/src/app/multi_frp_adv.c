/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_private.h"
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_tsp.h"
#include "frp_menu_advance.h"
#include "frp_device.h"

#ifdef SUPPORT_NEW_FRP_MENU_MODULE

extern void HWL_DTMBTDParamToCHNParam(TD_DTMBParameter *pTDParam, MULT_SubModulatorInfo *pSubModuationInfo);
extern void HWL_DTMBCHNParamToTDParam(MULT_SubModulatorInfo *pSubModuationInfo, TD_DTMBParameter *pTDParam);

/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
enum
{
	MULT_FRP_EMNU_ADV_ID_IP_ADDR = FRP_MENU_ADV_ID_USER_START,
	MULT_FRP_EMNU_ADV_ID_IP_MASK,
	MULT_FRP_EMNU_ADV_ID_IP_GATE,
	MULT_FRP_EMNU_ADV_ID_IP_MAC,
	MULT_FRP_EMNU_ADV_ID_IP_SAVE,

	MULT_FRP_EMNU_ADV_ID_SN,
	MULT_FRP_EMNU_ADV_ID_RELEASE_DATE,
	MULT_FRP_EMNU_ADV_ID_SOFT_VERSION,
	MULT_FRP_EMNU_ADV_ID_FPGA_VERSION,
	MULT_FRP_EMNU_ADV_ID_HARD_VERSION,

	MULT_FRP_EMNU_ADV_ID_LANGUAGE,
	MULT_FRP_EMNU_ADV_ID_DEFAULT_PARAM,
	MULT_FRP_EMNU_ADV_ID_FACTORY_PRESET,
	MULT_FRP_EMNU_ADV_ID_REBOOT,

	MULT_FRP_EMNU_ADV_ID_RF_OUT_FREQ,
	MULT_FRP_EMNU_ADV_ID_RF_OUT_FREQ_ADJ,
	MULT_FRP_EMNU_ADV_ID_RF_OUT_GAIN,
	MULT_FRP_EMNU_ADV_ID_RF_OUT_GAIN_ADJ,
	MULT_FRP_EMNU_ADV_ID_RF_OUT_TONE,
	MULT_FRP_EMNU_ADV_ID_RF_ALC,
	MULT_FRP_EMNU_ADV_ID_RF_OUT_ENABLE,
	MULT_FRP_EMNU_ADV_ID_SPECT_INV,
	MULT_FRP_EMNU_ADV_ID_DTMB,
	MULT_FRP_EMNU_ADV_ID_RF_SAVE,


	MULT_FRP_EMNU_ADV_ID_SFN_ENABLE,
	MULT_FRP_EMNU_ADV_ID_SFN_INPUT_ASI_SELECT,
	MULT_FRP_EMNU_ADV_ID_ADD_DELAY,
	MULT_FRP_EMNU_ADV_ID_CLK_REFERENCE,
	MULT_FRP_EMNU_ADV_ID_1PPS_REFERENCE,
	MULT_FRP_EMNU_ADV_ID_CLK_REF_STAT,
	MULT_FRP_EMNU_ADV_ID_CLK_SYNC_STATUS,
	MULT_FRP_EMNU_ADV_ID_1PPS_STAT,
	MULT_FRP_EMNU_ADV_ID_SFN_SAVE,

	MULT_FRP_EMNU_ADV_ID_SIP_FORCE,
	MULT_FRP_EMNU_ADV_ID_SIP_DELETE,
	MULT_FRP_EMNU_ADV_ID_SAT_SIP_ENABLE,
	MULT_FRP_EMNU_ADV_ID_SAT_SIP_PID,
	MULT_FRP_EMNU_ADV_ID_SAT_SIP_CRC_CHECK,
	MULT_FRP_EMNU_ADV_ID_DEVICE_ADDR,
	MULT_FRP_EMNU_ADV_ID_ADDR_ENABLE,
	MULT_FRP_EMNU_ADV_ID_SIP_SAVE,

	MULT_FRP_EMNU_ADV_ID_EXT_DPD_ENABLE,

	MULT_FRP_EMNU_ADV_ID_DPD_ENABLE,
	MULT_FRP_EMNU_ADV_ID_DPD_RESET,
	MULT_FRP_EMNU_ADV_ID_DPD_FB,
	MULT_FRP_EMNU_ADV_ID_DPD_SAVE,

	MULT_FRP_EMNU_ADV_ID_DPD_STATUS,
	MULT_FRP_EMNU_ADV_ID_DPD_FB_STATUS,
	MULT_FRP_EMNU_ADV_ID_DPD_MODULE_TEMP,



	MULT_FRP_EMNU_ADV_ID_ALARM_START
};


/* Private Variables (static)-------------------------------------------------- */
static HANDLE32 s_FRPMenuADVHandle = NULL;
static U32 s_LastUserID = FRP_MENU_ADV_ID_USER_MAX;
static MULT_Config lTmpConfig;
static MULT_ChannelNode s_TmpChnNode;
static MULT_SubChannelNode s_TmpSubNode;
static MULT_SFN_Param s_TmpSFNParam;
#ifdef SUPPORT_NTS_DPD_BOARD
static DPD_CONTROL_DPD_PARAM s_TmpDPDParam;
#endif
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
BOOL MULT_FRPMenuADVNewCB(void* pUserParam, U32 UserID, FRP_MenuADVValue* pValue, BOOL bSet)
{
	BOOL lRet = FALSE;
	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;
	if (plHandle)
	{
		MULT_Config *plConfig;
		MULT_Information *plInfo;
		MULT_Monitor *plMonitor;
		S32 lLanguageInd;

		FRP_MenuADVLanguageIndProc(s_FRPMenuADVHandle, &lLanguageInd, TRUE); 

		plConfig = &plHandle->m_Configuration;
		plInfo = &plHandle->m_Information;
		plMonitor = &plHandle->m_Monitor;

		{
			/*设备型号名称*/
			if (UserID == FRP_MENU_ADV_ID_ROOT)
			{
				if (bSet == FALSE)
				{
					if (lLanguageInd == FRP_MENU_ADV_LANGUAGE_CHN)
					{
						CAL_StringCopyStringToLimitedBuf(pValue->m_pStringValue, plInfo->m_pLCDCHN, sizeof(pValue->m_pStringValue));
					}
					else
					{
						CAL_StringCopyStringToLimitedBuf(pValue->m_pStringValue, plInfo->m_pLCDENG, sizeof(pValue->m_pStringValue));
					}
				}
				lRet = TRUE;
			}
		}

		{
			/*报警*/
			if (UserID >= MULT_FRP_EMNU_ADV_ID_ALARM_START)
			{
				if (bSet)
				{
					CAL_LogClearLogInfo(plMonitor->m_LogHandle, UserID - MULT_FRP_EMNU_ADV_ID_ALARM_START);
				}
				else
				{
					pValue->m_S32Value = CAL_LogGetLogInfoCount(plMonitor->m_LogHandle, UserID - MULT_FRP_EMNU_ADV_ID_ALARM_START);
				}
				lRet = TRUE;
			}
			if (UserID == FRP_MENU_ADV_ID_ALARM_CLEAR_NODE)
			{
				/*清除报警！*/
				if (bSet)
				{
					MULTL_ResetAlarmCount(plHandle, GLOBAL_INVALID_INDEX);
				}
				lRet = TRUE;
			}
		}

		{
			/*网络参数*/
			if ((s_LastUserID < MULT_FRP_EMNU_ADV_ID_IP_ADDR) || (s_LastUserID > MULT_FRP_EMNU_ADV_ID_IP_SAVE))
			{
				if (UserID == MULT_FRP_EMNU_ADV_ID_IP_ADDR)
				{
					/*读取参数到临时缓存里面！*/
					GLOBAL_MEMCPY(&lTmpConfig, plConfig, sizeof(lTmpConfig));
				}
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_IP_ADDR)
			{
				if (bSet)
				{
					lTmpConfig.m_ManageIPv4Addr = pValue->m_U32Value;
				}
				else
				{
					pValue->m_U32Value = lTmpConfig.m_ManageIPv4Addr;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_IP_MASK)
			{
				if (bSet)
				{
					lTmpConfig.m_ManageIPv4Mask = pValue->m_U32Value;
				}
				else
				{
					pValue->m_U32Value = lTmpConfig.m_ManageIPv4Mask;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_IP_GATE)
			{
				if (bSet)
				{
					lTmpConfig.m_ManageIPv4Gate = pValue->m_U32Value;
				}
				else
				{
					pValue->m_U32Value = lTmpConfig.m_ManageIPv4Gate;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_IP_MAC)
			{
				if (bSet)
				{

				}
				else
				{
					CAL_StringBinToMAC(plConfig->m_pMAC, sizeof(plConfig->m_pMAC), pValue->m_pStringValue, sizeof(pValue->m_pStringValue));
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_IP_SAVE)
			{
				if (bSet)
				{
					plConfig->m_ManageIPv4Addr = lTmpConfig.m_ManageIPv4Addr;
					plConfig->m_ManageIPv4Mask = lTmpConfig.m_ManageIPv4Mask;
					plConfig->m_ManageIPv4Gate = lTmpConfig.m_ManageIPv4Gate;
					GLOBAL_MEMCPY(plConfig->m_pMAC, &lTmpConfig.m_pMAC, 6);
					MULTL_ManagePortConfig(plHandle);//设置IP地址
					GLOBAL_TRACE(("FRP Set IP\n"));
					MULTL_SaveConfigurationXML(plHandle);
					MULTL_SaveParamterToStorage(plHandle);
				}
				lRet = TRUE;
			}
		}

		{
			if (UserID == MULT_FRP_EMNU_ADV_ID_SN)
			{
				if (bSet)
				{

				}
				else
				{
					CAL_StringCopyStringToLimitedBuf(pValue->m_pStringValue, plInfo->m_pSNString, sizeof(pValue->m_pStringValue));
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RELEASE_DATE)
			{
				if (bSet)
				{

				}
				else
				{
					CAL_StringCopyStringToLimitedBuf(pValue->m_pStringValue, plInfo->m_pSoftRelease, sizeof(pValue->m_pStringValue));
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SOFT_VERSION)
			{
				if (bSet)
				{

				}
				else
				{
					CAL_StringCopyStringToLimitedBuf(pValue->m_pStringValue, plInfo->m_pSoftVersion, sizeof(pValue->m_pStringValue));
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_FPGA_VERSION)
			{
				if (bSet)
				{

				}
				else
				{
					CAL_StringCopyStringToLimitedBuf(pValue->m_pStringValue, plInfo->m_pFPGARelease, sizeof(pValue->m_pStringValue));
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_HARD_VERSION)
			{
				if (bSet)
				{

				}
				else
				{
					CAL_StringCopyStringToLimitedBuf(pValue->m_pStringValue, plInfo->m_pHardVersion, sizeof(pValue->m_pStringValue));
				}
				lRet = TRUE;
			}
		}

		{
			/*系统参数*/
			if (UserID == MULT_FRP_EMNU_ADV_ID_LANGUAGE)
			{
				if (bSet)
				{
					plConfig->m_FrpLanguage = pValue->m_S32Value;
					FRP_MenuADVLanguageIndProc(s_FRPMenuADVHandle, &pValue->m_S32Value, FALSE);
					MULTL_SaveConfigurationXML(plHandle);
					MULTL_SaveParamterToStorage(plHandle);
				}
				else
				{
					pValue->m_S32Value  = plConfig->m_FrpLanguage;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_DEFAULT_PARAM)
			{
				if (bSet)
				{
					GLOBAL_TRACE(("Parameter Default Set!!!\n"));
					MULTL_ParameterReset(plHandle);
					MULTL_RebootSequence(plHandle);
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_FACTORY_PRESET)
			{
				if (bSet)
				{
					GLOBAL_TRACE(("Factory Preset!!!\n"));
					MULTL_FactoryPreset(plHandle);
					MULTL_RebootSequence(plHandle);
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_REBOOT)
			{
				if (bSet)
				{
					GLOBAL_TRACE(("Reboot!!!\n"));
					MULTL_RebootSequence(plHandle);
				}
				lRet = TRUE;
			}
		}


		/*射频参数*/
		{

			MULT_ChannelNode *plChnNode;
			MULT_SubChannelNode *plSubNode;
			plChnNode = &plHandle->m_Parameter.m_pOutChannel[0];
			plSubNode = &plChnNode->m_pSubChannelNode[0];

			if ((s_LastUserID < MULT_FRP_EMNU_ADV_ID_RF_OUT_FREQ) || (s_LastUserID > MULT_FRP_EMNU_ADV_ID_RF_SAVE))
			{
				if (UserID == MULT_FRP_EMNU_ADV_ID_RF_OUT_FREQ)
				{
					/*读取参数到临时缓存里面！*/
					GLOBAL_MEMCPY(&s_TmpChnNode, plChnNode, sizeof(s_TmpChnNode));
					GLOBAL_MEMCPY(&s_TmpSubNode, plSubNode, sizeof(s_TmpSubNode));
				}
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RF_OUT_FREQ)
			{
				if (bSet)
				{
					s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_CenterFreq = (U32)(pValue->m_F64Value * 1000000);
					if (s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_CenterFreq > s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsHigh)
					{
						s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_CenterFreq = s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsHigh;
					}

					if (s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_CenterFreq < s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsLow)
					{
						s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_CenterFreq = s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsLow;
					}

				}
				else
				{
					pValue->m_F64Value  = ((F64)s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_CenterFreq) / 1000000;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RF_OUT_FREQ_ADJ)
			{
				if (bSet)
				{
					s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_FreqAdj = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value  = s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_FreqAdj;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RF_OUT_GAIN)
			{
				if (bSet)
				{
					s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_GainLevel = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value  = s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_GainLevel;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RF_OUT_GAIN_ADJ)
			{
				if (bSet)
				{
					s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value  = s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RF_OUT_TONE)
			{
				if (bSet)
				{
					s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_Modulation = !pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value =! s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_Modulation;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RF_ALC)
			{
				if (bSet)
				{
					s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_ALCMark = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value  = s_TmpChnNode.m_ChannelInfo.m_ModulatorInfo.m_ALCMark;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RF_OUT_ENABLE)
			{
				if (bSet)
				{
					s_TmpSubNode.m_ActiveMark = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value  = s_TmpSubNode.m_ActiveMark;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SPECT_INV)
			{
				if (bSet)
				{
					s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_SpectInv = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSubNode.m_SubInfo.m_SubModulatorInfo.m_SpectInv;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_DTMB)
			{
				if (bSet)
				{
					HWL_DTMBTDParamToCHNParam(&pValue->m_DTMBValue, &s_TmpSubNode.m_SubInfo.m_SubModulatorInfo);
				}
				else
				{
					HWL_DTMBCHNParamToTDParam(&s_TmpSubNode.m_SubInfo.m_SubModulatorInfo, &pValue->m_DTMBValue);
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_RF_SAVE)
			{
				if (bSet)
				{
					GLOBAL_MEMCPY(plChnNode, &s_TmpChnNode, sizeof(s_TmpChnNode));
					GLOBAL_MEMCPY(plSubNode, &s_TmpSubNode, sizeof(s_TmpSubNode));
					GLOBAL_TRACE(("FRP Set RF\n"));
					FRP_MenuADVShowWaitDisplay(s_FRPMenuADVHandle);
#ifdef SUPPORT_SFN_MODULATOR
					MULT_SFNApplyByQAMModule();
#endif
					MULTL_SaveParameterXML(plHandle);
					MULTL_SaveParamterToStorage(plHandle);
				}
				lRet = TRUE;
			}
		}


		/*SFN参数1*/
		{
			if ((s_LastUserID < MULT_FRP_EMNU_ADV_ID_SFN_ENABLE) || (s_LastUserID > MULT_FRP_EMNU_ADV_ID_SFN_SAVE))
			{
				if (UserID == MULT_FRP_EMNU_ADV_ID_SFN_ENABLE)
				{
					/*读取参数到临时缓存里面！*/
					MULT_SFNProcSFNParam(&s_TmpSFNParam, TRUE);
				}
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SFN_ENABLE)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_bUseSFN = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_bUseSFN;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SFN_INPUT_ASI_SELECT)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_SFNASIMode = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_SFNASIMode;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_ADD_DELAY)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_SFNAddDelay100ns = (S32)(pValue->m_F64Value * 10);
					if (s_TmpSFNParam.m_SFNAddDelay100ns > 9999999)
					{
						s_TmpSFNParam.m_SFNAddDelay100ns = 9999999;
					}
					if (s_TmpSFNParam.m_SFNAddDelay100ns < 0)
					{
						s_TmpSFNParam.m_SFNAddDelay100ns = 0;
					}
				}
				else
				{
					pValue->m_F64Value = ((F64)s_TmpSFNParam.m_SFNAddDelay100ns) / 10;
				}
				lRet = TRUE;
			}


			if (UserID == MULT_FRP_EMNU_ADV_ID_CLK_REFERENCE)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_bUse10MClkSynSrc = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_bUse10MClkSynSrc;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_1PPS_REFERENCE)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_bUseEx1PPS = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_bUseEx1PPS;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_CLK_REF_STAT)
			{
				MULT_SFN_Param lTmpSFNParam;
				MULT_SFNProcSFNParam(&lTmpSFNParam, TRUE);
				if (bSet)
				{
				}
				else
				{
					MULT_SFN_Status lStatus;
					MULT_SFNGetSFNStatus(&lStatus);
					if (lTmpSFNParam.m_bUse10MClkSynSrc == 0)
					{
						pValue->m_S32Value = lStatus.m_bInt10MStatus;
					}
					else if (lTmpSFNParam.m_bUse10MClkSynSrc == 1)
					{
						pValue->m_S32Value = lStatus.m_bExt10MStatus;
					}
					else
					{
						if (lTmpSFNParam.m_bUseEx1PPS)
						{
							pValue->m_S32Value = lStatus.m_bExt1PPSStatus;
						}
						else
						{
							pValue->m_S32Value = lStatus.m_bInt1PPSStatus;
						}
					}
				}
				lRet = TRUE;
			}
			if (UserID == MULT_FRP_EMNU_ADV_ID_1PPS_STAT)
			{
				MULT_SFN_Param lTmpSFNParam;
				MULT_SFNProcSFNParam(&lTmpSFNParam, TRUE);
				if (bSet)
				{
				}
				else
				{
					MULT_SFN_Status lStatus;
					MULT_SFNGetSFNStatus(&lStatus);
					if (lTmpSFNParam.m_bUseEx1PPS)
					{
						pValue->m_S32Value = lStatus.m_bExt1PPSStatus;
					}
					else
					{
						pValue->m_S32Value = lStatus.m_bInt1PPSStatus;
					}
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_CLK_SYNC_STATUS)
			{
				if (bSet)
				{
				}
				else
				{
					pValue->m_S32Value = MULT_CLKGet10MLockStatus();
				}
				lRet = TRUE;
			}


			if (UserID == MULT_FRP_EMNU_ADV_ID_SFN_SAVE)
			{
				if (bSet)
				{
#ifdef SUPPORT_SFN_MODULATOR
					MULT_SFNProcSFNParam(&s_TmpSFNParam, FALSE);
					MULT_SFNApplyByQAMModule();
#endif
					GLOBAL_TRACE(("FRP Set SFN 01!!!!\n"));
					FRP_MenuADVShowWaitDisplay(s_FRPMenuADVHandle);
					MULTL_SaveParameterXML(plHandle);
					MULTL_SaveParamterToStorage(plHandle);
				}
				lRet = TRUE;
			}
		}

		/*SFN参数2*/
		{
			if ((s_LastUserID < MULT_FRP_EMNU_ADV_ID_SIP_FORCE) || (s_LastUserID > MULT_FRP_EMNU_ADV_ID_SIP_SAVE))
			{
				if (UserID == MULT_FRP_EMNU_ADV_ID_SIP_FORCE)
				{
					/*读取参数到临时缓存里面！*/
					MULT_SFNProcSFNParam(&s_TmpSFNParam, TRUE);
				}
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SIP_FORCE)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_bUseCMNSIP = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_bUseCMNSIP;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SIP_DELETE)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_bDeleteSIP = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_bDeleteSIP;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SAT_SIP_ENABLE)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_bEnableSatSFN = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_bEnableSatSFN;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SAT_SIP_PID)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_SatSFNNullPacketPID = pValue->m_S32Value;
					if (s_TmpSFNParam.m_SatSFNNullPacketPID > MPEG2_TS_PACKET_NULL_PID)
					{
						s_TmpSFNParam.m_SatSFNNullPacketPID = MPEG2_TS_PACKET_NULL_PID;
					}

					if (s_TmpSFNParam.m_SatSFNNullPacketPID < MPEG2_PSI_PID_FIRST_USER_DEFINE)
					{
						s_TmpSFNParam.m_SatSFNNullPacketPID = MPEG2_PSI_PID_FIRST_USER_DEFINE;
					}

				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_SatSFNNullPacketPID;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SAT_SIP_CRC_CHECK)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_bSatSFNSIPCRC32Check = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_bSatSFNSIPCRC32Check;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_DEVICE_ADDR)
			{
				U8 plCovertBuf[4], *plTmpBuf;
				plTmpBuf = pValue->m_HEXValue;
				if (bSet)
				{
					GLOBAL_MSB16_D(plTmpBuf, s_TmpSFNParam.m_SFNAddrID)
				}
				else
				{
					GLOBAL_MSB16_E(plTmpBuf, s_TmpSFNParam.m_SFNAddrID)
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_ADDR_ENABLE)
			{
				if (bSet)
				{
					s_TmpSFNParam.m_bUseIndvSIP = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpSFNParam.m_bUseIndvSIP;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_SIP_SAVE)
			{
				if (bSet)
				{
					GLOBAL_TRACE(("FRP Set SFN 02!!!!\n"));
					FRP_MenuADVShowWaitDisplay(s_FRPMenuADVHandle);
#ifdef SUPPORT_SFN_MODULATOR
					MULT_SFNProcSFNParam(&s_TmpSFNParam, FALSE);
					MULT_SFNApplyByQAMModule();
#endif
					MULTL_SaveParameterXML(plHandle);
					MULTL_SaveParamterToStorage(plHandle);
				}
				lRet = TRUE;
			}
		}


#ifdef SUPPORT_NTS_DPD_BOARD
		{
			if ((s_LastUserID < MULT_FRP_EMNU_ADV_ID_DPD_ENABLE) || (s_LastUserID > MULT_FRP_EMNU_ADV_ID_DPD_SAVE))
			{
				if (UserID == MULT_FRP_EMNU_ADV_ID_DPD_ENABLE)
				{
					/*读取参数到临时缓存里面！*/
					MULT_NTSDPDProcParam(&s_TmpDPDParam, TRUE);
				}
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_DPD_ENABLE)
			{
				if (bSet)
				{
					s_TmpDPDParam.m_DPDMark = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpDPDParam.m_DPDMark;
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_DPD_RESET)
			{
				if (bSet)
				{
					DPD_CONTROL_DPD_PARAM lTmpDPDParam;
					MULT_NTSDPDProcParam(&lTmpDPDParam, TRUE);
					lTmpDPDParam.m_DPDTrackResetMark = TRUE;
					MULT_NTSDPDProcParam(&lTmpDPDParam, FALSE);
					MULT_NTSDPDApplyParameter();

					/*强制变动前面板到指定节点*/
					GLOBAL_TRACE(("DPD Reset!!!!!!!!!!!!!!!!!\n"));
					FRP_MenuADVSetNodeCurrent(s_FRPMenuADVHandle, FRP_MenuADVNodeFindByID(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_DPD_STATUS));
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_DPD_FB)
			{
				if (bSet)
				{
					s_TmpDPDParam.m_DPDFeedbackSelect = pValue->m_S32Value;
				}
				else
				{
					pValue->m_S32Value = s_TmpDPDParam.m_DPDFeedbackSelect;
				}
				lRet = TRUE;
			}


			if (UserID == MULT_FRP_EMNU_ADV_ID_DPD_SAVE)
			{
				if (bSet)
				{
					GLOBAL_TRACE(("FRP Set DPD!!!!\n"));
					FRP_MenuADVShowWaitDisplay(s_FRPMenuADVHandle);
					s_TmpDPDParam.m_DPDTrackResetMark = FALSE;
					MULT_NTSDPDProcParam(&s_TmpDPDParam, FALSE);
					MULT_NTSDPDApplyParameter();
					MULTL_SaveParameterXML(plHandle);
					MULTL_SaveParamterToStorage(plHandle);
				}
				lRet = TRUE;
			}
		}

		{
			if (UserID == MULT_FRP_EMNU_ADV_ID_DPD_STATUS)
			{
				if (bSet)
				{
				}
				else
				{
					DPD_CONTROL_STATUS lTmpStatus;
					MULT_NTSDPDProcStatus(&lTmpStatus);
					if (lTmpStatus.m_DPDStatus == 0)
					{
						if (lLanguageInd == FRP_MENU_ADV_LANGUAGE_CHN)
						{
							GLOBAL_SPRINTF((pValue->m_pStringValue, "    锁相环错误      "));
						}
						else
						{
							GLOBAL_SPRINTF((pValue->m_pStringValue, "   PLL LOCK FAILED  "));
						}
					}
					else
					{
						if (lTmpStatus.m_DPDTrackStatus == 0)
						{
							if (lLanguageInd == FRP_MENU_ADV_LANGUAGE_CHN)
							{
								GLOBAL_SPRINTF((pValue->m_pStringValue, "成功 %5.2f dB", ((F64)lTmpStatus.m_DPDSN) / 100));
							}
							else
							{
								GLOBAL_SPRINTF((pValue->m_pStringValue, "OK %5.2f dB", ((F64)lTmpStatus.m_DPDSN) / 100));
							}
						}
						else if (lTmpStatus.m_DPDTrackStatus == 1)
						{
							if (lLanguageInd == FRP_MENU_ADV_LANGUAGE_CHN)
							{
								GLOBAL_SPRINTF((pValue->m_pStringValue, "跟踪中 %5.2f dB", ((F64)lTmpStatus.m_DPDSN) / 100));
							}
							else
							{
								GLOBAL_SPRINTF((pValue->m_pStringValue, "TRACKING %5.2f dB", ((F64)lTmpStatus.m_DPDSN) / 100));
							}
							FRP_MenuADVLCDLockTimeoutCurrent(s_FRPMenuADVHandle, 30 * 1000);
						}
						else
						{
							if (lLanguageInd == FRP_MENU_ADV_LANGUAGE_CHN)
							{
								GLOBAL_SPRINTF((pValue->m_pStringValue, "失败 %5.2f dB", ((F64)lTmpStatus.m_DPDSN) / 100));
							}
							else
							{
								GLOBAL_SPRINTF((pValue->m_pStringValue, "FAILED %5.2f dB", ((F64)lTmpStatus.m_DPDSN) / 100));
							}
						}
					}
				}
				lRet = TRUE;
			}


			if (UserID == MULT_FRP_EMNU_ADV_ID_DPD_FB_STATUS)
			{
				DPD_CONTROL_STATUS lTmpStatus;
				MULT_NTSDPDProcStatus(&lTmpStatus);
				if (lTmpStatus.m_DPDTrackStatus == 0)
				{
					if (lLanguageInd == FRP_MENU_ADV_LANGUAGE_CHN)
					{
						GLOBAL_SPRINTF((pValue->m_pStringValue, "正常 % 3d dBuV", lTmpStatus.m_DPDFeedebacklevel));
					}
					else
					{
						GLOBAL_SPRINTF((pValue->m_pStringValue, "OK   % 3d dBuV", lTmpStatus.m_DPDFeedebacklevel));
					}
				}
				else if (lTmpStatus.m_DPDTrackStatus == 1)
				{
					if (lLanguageInd == FRP_MENU_ADV_LANGUAGE_CHN)
					{
						GLOBAL_SPRINTF((pValue->m_pStringValue, "过高 % 3d dBuV", lTmpStatus.m_DPDFeedebacklevel));
					}
					else
					{
						GLOBAL_SPRINTF((pValue->m_pStringValue, "HIGH % 3d dBuV", lTmpStatus.m_DPDFeedebacklevel));
					}
				}
				else
				{
					if (lLanguageInd == FRP_MENU_ADV_LANGUAGE_CHN)
					{
						GLOBAL_SPRINTF((pValue->m_pStringValue, "过低 % 3d dBuV", lTmpStatus.m_DPDFeedebacklevel));
					}
					else
					{
						GLOBAL_SPRINTF((pValue->m_pStringValue, "LOW  % 3d dBuV", lTmpStatus.m_DPDFeedebacklevel));
					}
				}
				lRet = TRUE;
			}

			if (UserID == MULT_FRP_EMNU_ADV_ID_DPD_MODULE_TEMP)
			{
				DPD_CONTROL_STATUS lTmpStatus;
				MULT_NTSDPDProcStatus(&lTmpStatus);
				if (bSet)
				{
				}
				else
				{
					GLOBAL_SPRINTF((pValue->m_pStringValue, "      % 2d C / % 3d F", lTmpStatus.m_DPDBoardTemp, lTmpStatus.m_DPDBoardTemp * 9 / 5 + 32));
				}
				lRet = TRUE;
			}

		}
#endif
		/*保存上一次的操作！*/
		s_LastUserID = UserID;
	}
	return lRet;
}

void MULT_FRPMenuADVInitiate(MULT_Handle *pHandle)
{
	FRP_MenuADVInitParam lParam;
	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));

	lParam.m_COMInd = 1;
	lParam.m_MaxLockTimeoutMS = 45 * 1000;
	lParam.m_MaxNodeNum = 256;
	lParam.m_pUserParam = pHandle;
	lParam.m_pUserKeyCB = NULL;
	lParam.m_pMenuCB = MULT_FRPMenuADVNewCB;

	s_FRPMenuADVHandle = FRP_MenuADVCreate(&lParam);
}

void MULT_FRPMenuADVStart(MULT_Handle *pHandle)
{
	S32 lOffst;
	S32 i, lTmpSlotInd, lTmpEnumInd, lTmpColParentInd, lTmpFirstInd;
	CHAR_T plStrBuf[1024];
	MULT_Config *plConfig;
	MULT_Information *plInfo;

	plConfig = &pHandle->m_Configuration;
	plInfo = &pHandle->m_Information;

	FRP_MenuADVNodeChar plTitle[FRP_MENU_ADV_LANGUAGE_NUM];
	FRP_MenuADVNodeChar plAlarm[FRP_MENU_ADV_LANGUAGE_NUM];
	FRP_MenuADVValue lValue;

	/*设置界面语言初值*/
	lTmpSlotInd = plConfig->m_FrpLanguage;
	FRP_MenuADVLanguageIndProc(s_FRPMenuADVHandle, &lTmpSlotInd, FALSE);


	/*计算设备型号名称居中设置*/
	lOffst = CAL_StringCenterOffset(plInfo->m_pModelName, 21);
	if ((lOffst < (21 - GLOBAL_STRLEN(plInfo->m_pModelName))) && (lOffst >= 0))
	{
		GLOBAL_MEMSET(plStrBuf, ' ', sizeof(plStrBuf));
		GLOBAL_STRCPY(&plStrBuf[lOffst], plInfo->m_pModelName);
	}

	/*添加设备型号和名称字符串*/
	lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, FRP_MENU_ADV_ID_ROOT);//注意这个必须是根节点
	FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, plStrBuf, plStrBuf);
	FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);


	/*添加报警菜单项目*/
	{
		FRP_MenuADVAlarmParam lAlarmParam;
		GLOBAL_ZEROMEM(&lAlarmParam, sizeof(lAlarmParam));
		lAlarmParam.m_AlarmUpdateIntervalMS = 10000;
		lAlarmParam.m_AlarmLEDBlinkIntervalMS = 10000;
		lAlarmParam.m_AlarmMaxNum = MULT_MONITOR_TYPE_NUM + 1;//预留一个空位
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "1.0 ALARMS", "1.0 报警");
		lTmpSlotInd = FRP_MenuADVAlarmBaseNodeSetup(s_FRPMenuADVHandle, &lAlarmParam, plTitle);
		lTmpColParentInd = lTmpSlotInd;//报警节点是当前列的父节点！
		lTmpFirstInd = lTmpSlotInd;//第一个节点是报警节点

		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "Temp Abnormal ", "温度异常      ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_TEMP, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "HW Module Err ", "硬件异常      ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_FPGA, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "PLL Lock Lost ", "锁相环失锁    ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_PLL, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "Input Error   ", "输入码率错误  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_CHANNEL_IN, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "Output Error  ", "输出码率错误  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_CHANNEL_OUT, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "NTP Failure   ", "时间同步失败  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_NTP, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "EMM Error     ", "EMM 错误      ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_SCS_EMM, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "ECM Error     ", "ECM 错误      ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_SCS_ECM, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "Buf Overflow  ", "缓存溢出      ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_BUFFER_STATUS_START, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "SAT Lock Lost ", "GPS 失锁      ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_GPS_LOCK_LOST, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "EXT 10M Lost  ", "外部10M 丢失  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_EXT_10M_LOST, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "EXT 1PPS Lost ", "外部1PPS丢失  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_EXT_1PPS_LOST, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "INT 10M Lost  ", "内部10M 丢失  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_INT_10M_LOST, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "INT 1PPS Lost ", "内部1PPS丢失  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_INT_1PPS_LOST, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "SAT Module ERR", "SAT 模块异常  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_GPS_ERROR, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "SFN SIP ERR   ", "SFN SIP 异常  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_SFN_SIP_ERROR, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "SIP CRC ERR   ", "SIP CRC 异常  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_SFN_SIP_CRC32_ERROR, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "SFN ERR       ", "SFN 异常      ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_SFN_ERROR, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "DPD PARAM ERR ", "DPD 参数异常  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_DPD_PARAMETER_SAVE_ERROR, plTitle);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "CLK SYNC ERR  ", "时钟同步异常  ");
		FRP_MenuADVAlarmAddAlarmOption(s_FRPMenuADVHandle, MULT_FRP_EMNU_ADV_ID_ALARM_START + MULT_MONITOR_TYPE_CLK_SYNC_ERROR, plTitle);

		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "1.1 CLEAN ALARMS", "1.1 清除报警");
		lTmpSlotInd = FRP_MenuADVAlarmCleanNodeSetup(s_FRPMenuADVHandle, plTitle);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);
	}

	{
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_IPADDR, MULT_FRP_EMNU_ADV_ID_IP_ADDR);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "2.0 IP ADDRESS", "2.0 网络地址");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpFirstInd, FRP_MENU_ADV_INSERT_TYPE_LEFT, GLOBAL_INVALID_INDEX);
		lTmpColParentInd = lTmpSlotInd;

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_IPADDR, MULT_FRP_EMNU_ADV_ID_IP_MASK);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "2.1 IP MASK", "2.1 子网掩码");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_IPADDR, MULT_FRP_EMNU_ADV_ID_IP_GATE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "2.2 IP GATE", "2.2 默认网关");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_IP_MAC);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "2.3 IP MAC", "2.3 物理地址");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeHEXSetup(s_FRPMenuADVHandle, lTmpSlotInd, 6);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_IP_SAVE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "2.4 SAVE & APPLY", "2.4 保存并应用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);
	}


	{
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_SN);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "3.0 SERIAL NUMBER", "3.0 序列号");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpFirstInd, FRP_MENU_ADV_INSERT_TYPE_LEFT, GLOBAL_INVALID_INDEX);
		lTmpColParentInd = lTmpSlotInd;

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_RELEASE_DATE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "3.1 RELEASE DATE", "3.1 发布日期");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_SOFT_VERSION);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "3.2 SOFT VERSION", "3.2 软件版本");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_FPGA_VERSION);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "3.3 FPGA VERSION", "3.3 FPGA版本");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_HARD_VERSION);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "3.4 HARD VERSION", "3.4 硬件版本");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

	}

	{
		GLOBAL_ZEROMEM(&lValue, sizeof(lValue));
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_LANGUAGE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "4.0 LANGUAGE  语言", "4.0 LANGUAGE  语言");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpFirstInd, FRP_MENU_ADV_INSERT_TYPE_LEFT, GLOBAL_INVALID_INDEX);
		lTmpColParentInd = lTmpSlotInd;

		FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 2);
		lValue.m_S32Value = FRP_MENU_ADV_LANGUAGE_ENG;
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "ENGLISH 英语", "ENGLISH 英语");
		lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
		lValue.m_S32Value = FRP_MENU_ADV_LANGUAGE_CHN;
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "CHINESE 中文", "CHINESE 中文");
		FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);


		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_DEFAULT_PARAM);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "4.1 DEFAULT PARAM", "4.1 参数重置");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_FACTORY_PRESET);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "4.2 FACTORY PRESET", "4.2 出厂设置");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_REBOOT);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "4.3 MANUAL REBOOT", "4.3 手动重启");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

	}

	{
		/*DTMB 单频网调制器菜单*/
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_F64, MULT_FRP_EMNU_ADV_ID_RF_OUT_FREQ);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X RF FREQUENCY", "X.X 输出频率");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeF64Setup(s_FRPMenuADVHandle, lTmpSlotInd, 6, " MHz");
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpFirstInd, FRP_MENU_ADV_INSERT_TYPE_LEFT, GLOBAL_INVALID_INDEX);
		lTmpColParentInd = lTmpSlotInd;

		{
			MULT_ChannelNode *plChnNode;
			plChnNode = &pHandle->m_Parameter.m_pOutChannel[0];

			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));
			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_RF_OUT_FREQ_ADJ);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X FREQ ADJ", "X.X 频率微调");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 128);
			for (i = 64; i >= -63; i--)
			{
				lValue.m_S32Value = i;
				GLOBAL_SPRINTF((plStrBuf, "%5.2f Hz", i * 0.25));
				FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, plStrBuf, plStrBuf);
				lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
			}
		}

		{
			MULT_ChannelNode *plChnNode;
			plChnNode = &pHandle->m_Parameter.m_pOutChannel[0];

			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));
			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_RF_OUT_GAIN);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X RF GAIN", "X.X 输出增益");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax + 1);
			for (i = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax; i >= 0; i--)
			{
				lValue.m_S32Value = i;
				GLOBAL_SPRINTF((plStrBuf, "%5.2f dB", i * plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainStepping));
				FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, plStrBuf, plStrBuf);
				lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
			}
		}

		{
			MULT_ChannelNode *plChnNode;
			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));
			plChnNode = &pHandle->m_Parameter.m_pOutChannel[0];
			if (plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttValidMark)
			{
				lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_RF_OUT_GAIN_ADJ);
				FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X GAIN ADJ", "X.X 增益微调");
				FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
				FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

				FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevelMax + 1);
				for (i = plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevelMax; i >= 0 ; i--)
				{
					lValue.m_S32Value = i;
					GLOBAL_SPRINTF((plStrBuf, "%5.2f dB", i * plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttStepping));
					FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, plStrBuf, plStrBuf);
					lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
				}
			}
		}
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_RF_ALC);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X ALC", "X.X ALC");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_RF_OUT_TONE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X TONE", "X.X 单音");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_RF_OUT_ENABLE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X RF OUT", "X.X 射频输出");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_DTMBPARAM, MULT_FRP_EMNU_ADV_ID_DTMB);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X DTMB PARAMETER", "X.X 国标地面参数");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_SPECT_INV);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X SPECT INV", "X.X 频谱翻转");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_RF_SAVE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X SAVE & APPLY", "X.X 保存并应用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);
	}



	{
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_SFN_ENABLE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X SFN ENABLE", "X.X 单频网启用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpFirstInd, FRP_MENU_ADV_INSERT_TYPE_LEFT, GLOBAL_INVALID_INDEX);
		lTmpColParentInd = lTmpSlotInd;

		{
			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));
			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_SFN_INPUT_ASI_SELECT);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X INPUT ASI", "X.X 输入ASI 选择");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 3);

			lValue.m_S32Value = 1;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "ASI 1", "ASI 1");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);

			lValue.m_S32Value = 2;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "ASI 2", "ASI 2");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);

			lValue.m_S32Value = 3;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "ASI AUTO", "ASI AUTO");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);

		}



		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_F64, MULT_FRP_EMNU_ADV_ID_ADD_DELAY);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X ADD DELAY", "X.X 附加延时");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeF64Setup(s_FRPMenuADVHandle, lTmpSlotInd, 1, " us");
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		{
			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));
			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_CLK_REFERENCE);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X CLK REFERENCE", "X.X 时钟参考源");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			if (MULT_GNSCheckHaveGNS())
			{
				FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 3);
			}
			else
			{
				FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 2);
			}
			lValue.m_S32Value = 0;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "INT CLK", "内部时钟");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);

			lValue.m_S32Value = 1;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "EXT CLK", "外部时钟");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);

			if (MULT_GNSCheckHaveGNS())
			{
				lValue.m_S32Value = 2;
				FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "1PPS SRC", "秒脉冲参考源");
				lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
			}
		}



		{
			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));
			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_1PPS_REFERENCE);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X 1PPS REFERENCE", "X.X 秒脉冲参考源");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			if (MULT_GNSCheckHaveGNS())
			{
				FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 2);
			}
			else
			{
				FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 1);
			}

			if (MULT_GNSCheckHaveGNS())
			{
				lValue.m_S32Value = 0;
				FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "INT 1PPS", "内部秒脉冲");
				lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
			}

			lValue.m_S32Value = 1;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "EXT 1PPS", "外部秒脉冲");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);

		}

		/*时钟，秒脉冲状态*/
		{
			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));

			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_CLK_REF_STAT);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X CLK REF STAT", "X.X 时钟参考状态");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			FRP_MenuADVNodeAutoUpdateSet(s_FRPMenuADVHandle, lTmpSlotInd, TRUE, 2000);

			FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 2);
			lValue.m_S32Value = TRUE;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "NORMAL", "正常");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
			lValue.m_S32Value = FALSE;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "ERROR", "异常");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
		}

		{
			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));

			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_CLK_SYNC_STATUS);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X CLK SYNC STAT", "X.X 时钟同步状态");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			FRP_MenuADVNodeAutoUpdateSet(s_FRPMenuADVHandle, lTmpSlotInd, TRUE, 2000);

			FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 2);
			lValue.m_S32Value = TRUE;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "NORMAL", "正常");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
			lValue.m_S32Value = FALSE;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "ERROR", "异常");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
		}

		{
			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));

			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_1PPS_STAT);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X 1PPS REF STAT", "X.X 秒脉冲状态");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			FRP_MenuADVNodeAutoUpdateSet(s_FRPMenuADVHandle, lTmpSlotInd, TRUE, 2000);

			FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 2);
			lValue.m_S32Value = TRUE;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "NORMAL", "正常");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
			lValue.m_S32Value = FALSE;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "ERROR", "异常");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
		}

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_SFN_SAVE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X SAVE & APPLY", "X.X 保存并应用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);
	}


	{
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_SIP_FORCE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "5.1 SIP FORCE USE", "X.X SIP 强制使用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpFirstInd, FRP_MENU_ADV_INSERT_TYPE_LEFT, GLOBAL_INVALID_INDEX);
		lTmpColParentInd = lTmpSlotInd;

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_SIP_DELETE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "5.2 SIP DELETE", "5.2 输出SIP 删除");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_SAT_SIP_ENABLE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "5.3 SAT SIP ENABLE", "5.3 卫星SIP 启用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_INT32, MULT_FRP_EMNU_ADV_ID_SAT_SIP_PID);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "5.4 SAT SIP PID", "5.4 卫星SIP PID");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_SAT_SIP_CRC_CHECK);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "5.5 SAT SIP CRC", "5.5 卫星SIP 校验");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_HEX, MULT_FRP_EMNU_ADV_ID_DEVICE_ADDR);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "6.4 DEVICE ADDR", "6.4 设备地址");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeHEXSetup(s_FRPMenuADVHandle, lTmpSlotInd, 2);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_ADDR_ENABLE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "6.5 ADDR ENABLE", "6.5 设备寻址启用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);


		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_SIP_SAVE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X SAVE & APPLY", "X.X 保存并应用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);
	}




#ifdef SUPPORT_NTS_DPD_BOARD
	{
		/*预失真参数*/
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_DPD_ENABLE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X DPD ENABLE", "X.X 数字预失真开关");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeEnumAddONOFF(s_FRPMenuADVHandle, lTmpSlotInd);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpFirstInd, FRP_MENU_ADV_INSERT_TYPE_LEFT, GLOBAL_INVALID_INDEX);
		lTmpColParentInd = lTmpSlotInd;

		{
			GLOBAL_ZEROMEM(&lValue, sizeof(lValue));
			lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_ENUM, MULT_FRP_EMNU_ADV_ID_DPD_FB);
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X FB CHANNEL", "X.X 反馈通道");
			FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
			FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

			FRP_MenuADVNodeEnumSetup(s_FRPMenuADVHandle, lTmpSlotInd, 2);
			lValue.m_S32Value = 0;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "CHANNEL A", "通道A");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
			lValue.m_S32Value = 1;
			FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "CHANNEL B", "通道B");
			lTmpEnumInd = FRP_MenuADVNodeEnumSELAdd(s_FRPMenuADVHandle, lTmpSlotInd, &lValue, plTitle);
		}

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_DPD_SAVE);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X SAVE & APPLY", "X.X 保存并应用");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		/*预失真复位和状态*/
		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_CONFIRM, MULT_FRP_EMNU_ADV_ID_DPD_RESET);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X DPD RESET", "X.X 预失真参数复位");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, TRUE);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpFirstInd, FRP_MENU_ADV_INSERT_TYPE_LEFT, GLOBAL_INVALID_INDEX);
		lTmpColParentInd = lTmpSlotInd;

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_DPD_STATUS);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X DPD STATUS", "X.X 预失真状态");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeAutoUpdateSet(s_FRPMenuADVHandle, lTmpSlotInd, TRUE, 2000);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_DPD_FB_STATUS);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X FB STATUS", "X.X 反馈状态");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeAutoUpdateSet(s_FRPMenuADVHandle, lTmpSlotInd, TRUE, 2000);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

		lTmpSlotInd = FRP_MenuADVNodeAdd(s_FRPMenuADVHandle, FRP_MENU_ADV_NODE_TYPE_STRING, MULT_FRP_EMNU_ADV_ID_DPD_MODULE_TEMP);
		FRP_MenuADVMultiLanguageStringsSetup(s_FRPMenuADVHandle, plTitle, "X.X DPD TEMP", "X.X DPD 模块温度");
		FRP_MenuADVNodeCMNInfoSet(s_FRPMenuADVHandle, lTmpSlotInd, plTitle, FALSE);
		FRP_MenuADVNodeAutoUpdateSet(s_FRPMenuADVHandle, lTmpSlotInd, TRUE, 2000);
		FRP_MenuADVNodeInsert(s_FRPMenuADVHandle, lTmpSlotInd, lTmpColParentInd, FRP_MENU_ADV_INSERT_TYPE_UP, lTmpColParentInd);

	}
#endif

	/*设置解锁节点*/
	FRP_MenuADVNodeUnlockNodeSetup(s_FRPMenuADVHandle, FRP_MENU_ADV_ID_ALARM_ROOT);

	/*自动设置序号*/
	FRP_MenuADVReformTitle(s_FRPMenuADVHandle, 2);

	FRP_MenuADVStart(s_FRPMenuADVHandle);
}


void MULT_FRPMenuADVShowInitateProgressString(CHAR_T *pString)
{
	FRP_MenuADVSetLCD(s_FRPMenuADVHandle, 0, pString);
	FRP_MenuADVSetLCD(s_FRPMenuADVHandle, 1, "PLEASE WAIT......   ");
	FRP_MenuADVSetLED(s_FRPMenuADVHandle, FRP_LED_BACK, FRP_LED_BACK_ON);
	FRP_MenuADVSetLED(s_FRPMenuADVHandle, FRP_LED_STATUS, FRP_LED_GREEN);
	FRP_MenuADVSetLED(s_FRPMenuADVHandle, FRP_LED_ALARM, FRP_LED_GREEN);
	FRP_MenuADVSetCursor(s_FRPMenuADVHandle, GLOBAL_INVALID_INDEX);
	FRP_MenuADVUpdateAll(s_FRPMenuADVHandle);
}


void MULT_FRPMenuADVShowRebootProgressString(void)
{
	FRP_MenuADVSetLCD(s_FRPMenuADVHandle, 0, "REBOOTING   ");
	FRP_MenuADVSetLCD(s_FRPMenuADVHandle, 1, "PLEASE WAIT......   ");
	FRP_MenuADVSetLED(s_FRPMenuADVHandle, FRP_LED_BACK, FRP_LED_BACK_ON);
	FRP_MenuADVSetCursor(s_FRPMenuADVHandle, GLOBAL_INVALID_INDEX);
	FRP_MenuADVUpdateAll(s_FRPMenuADVHandle);
}


void MULT_FRPMenuADVDestroy(void)
{
	FRP_MenuADVDestroy(s_FRPMenuADVHandle);
	s_FRPMenuADVHandle = NULL;
}

#endif

/*EOF*/
