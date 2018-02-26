/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#ifdef SUPPORT_IP_O_TS_MODULE
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_hwl.h"
#include "TunerDemod.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define MULT_IPoTS_MAX_CHN				4
#define BAND_USAGE						(0.99935)
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef enum
{
	MULT_IPoTS_BAND_6M,//DVB-C 5.214  rolloff 0.15
	MULT_IPoTS_BAND_7M,//DVB-C 6.083 rolloff 0.15
	MULT_IPoTS_BAND_8M,//DVB-C 6.952 rolloff 0.15,一下类推

	MULT_IPoTS_BAND_12M,//使用2个通道
	MULT_IPoTS_BAND_14M,
	MULT_IPoTS_BAND_16M,

	MULT_IPoTS_BAND_18M,//使用3个通道
	MULT_IPoTS_BAND_21M,
	MULT_IPoTS_BAND_24M,

	MULT_IPoTS_BAND_28M,//使用4个通道
	MULT_IPoTS_BAND_32M,

	MULT_IPoTS_BAND_NUM
}MULT_IPoTSBand;


typedef struct  
{
	S32		m_UpLinkCenterFreqHz;//输出中心频率（Hz）
	S32		m_UpLinkBandWidth;//输出带宽，最小6MHz，最大32MHz，见MULT_IPoTSBand
	S32		m_UpLinkGain;
	S32		m_UplinkConstellation;//调制星座

	S32		m_DownLinkCenterFreqHz;//输入中心频率（Hz）
	S32		m_DownLinkBandWidth;//输出带宽，最小6MHz，最大32MHz，见MULT_IPoTSBand
	BOOL	m_bDownLinkIQSwap;


	BOOL	m_bConstellationAutoAdapt;//星座自适应功能
	BOOL	m_bDebugMode;//调试模式，打开此模式后调制模块仅输出单载波用于频率矫正
}MULT_IPoTSParam;


typedef struct  
{
	/*通用状态*/
	S32		m_Constellation;

	/*输入状态*/
	BOOL	m_bLock;
	F64		m_BER;
	F64		m_Strength;
	F64		m_Quality;
	S32		m_CurrentBitrateBPS;

}MULT_IPoTSDownlinkStatistic;

typedef struct  
{
	MULT_IPoTSParam					m_Parameter;

	/*计算参数*/
	S32								m_UplinkChnNum;	
	S32								m_UplinkSingleBandHz;	
	S32								m_UplinkSingleSymbolRate;	
	S32								m_DownlinkChnNum;	
	S32								m_DownlinkSingleBandHz;	
	S32								m_DownlinkSingleSymbolRate;



	/*实时状态*/
	S32								m_UplinkConstellation;
	F64								m_UplinkBandwidthBPS;
	F64								m_UplinkCurrentBPS;

	S32								m_DownlinkConstellation;
	F64								m_DownlinkBandwidthBPS;	
	F64								m_DownlinkCurrentBPS;

	MULT_IPoTSDownlinkStatistic		m_pStatistics[MULT_IPoTS_MAX_CHN];
}MULT_IPoTSHandle;


/* Private Variables (static)-------------------------------------------------- */
static MULT_IPoTSHandle	s_IPoTsHandle;
/* Private Function prototypes ------------------------------------------------ */
static void MULTL_IPoTSCalcChannels(S32 Bandwidth, S32 *pChnNum, S32 *pSingleBand);
static void MULTL_IPoTSCalcParameters(void);
static F64 MULTL_IPoTSGetDVBCBandwidthBps(S32 SymbolRate, S32 Constellation);
void MULTL_IPoTSProtocolSetCHNMark(U32 Mark);
/* Functions ------------------------------------------------------------------ */
void MULT_IPoTSInitiate(void)
{
	GLOBAL_ZEROMEM(&s_IPoTsHandle, sizeof(MULT_IPoTSHandle));

	s_IPoTsHandle.m_Parameter.m_UpLinkCenterFreqHz = 474000000;
	s_IPoTsHandle.m_Parameter.m_UpLinkBandWidth = MULT_IPoTS_BAND_28M;
	s_IPoTsHandle.m_Parameter.m_UplinkConstellation = GS_MODULATOR_QAM_64;
	s_IPoTsHandle.m_Parameter.m_UpLinkGain = 30;

	s_IPoTsHandle.m_Parameter.m_DownLinkCenterFreqHz = 474000000;
	s_IPoTsHandle.m_Parameter.m_DownLinkBandWidth = MULT_IPoTS_BAND_28M;

	MULTL_IPoTSCalcParameters();
}

void MULT_IPoTSXMLLoad(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bPost)
{
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLHolder;
	MULT_IPoTSParam *plParam;

	plParam = &s_IPoTsHandle.m_Parameter;

	if (bPost)
	{
		plXMLHolder = pXMLRoot;
	}
	else
	{
		plXMLHolder = mxmlFindElement(pXMLRoot, pXMLRoot, "ipots_setting", NULL, NULL, MXML_DESCEND_FIRST);
	}


	if (plXMLHolder)
	{
		plParam->m_UpLinkCenterFreqHz = MULTL_XMLGetNodeUINT(plXMLHolder, "uplink_freq", 10);
		plParam->m_UpLinkBandWidth = MULTL_XMLGetNodeINT(plXMLHolder, "uplink_bandwidth", 10);
		plParam->m_UpLinkGain = MULTL_XMLGetNodeINT(plXMLHolder, "uplink_gain", 10);
		plParam->m_UplinkConstellation = MULTL_XMLQAMModeValueFromStr(MULTL_XMLGetNodeText(plXMLHolder, "uplink_constellation"));

		plParam->m_DownLinkCenterFreqHz = MULTL_XMLGetNodeUINT(plXMLHolder, "downlink_freq", 10);
		plParam->m_DownLinkBandWidth = MULTL_XMLGetNodeINT(plXMLHolder, "downlink_bandwidth", 10);
		plParam->m_bDownLinkIQSwap = MULTL_XMLGetNodeMark(plXMLHolder, "downlink_iq_swapt_mark");

		plParam->m_bConstellationAutoAdapt = MULTL_XMLGetNodeMark(plXMLHolder, "constellatio_adapt_mark");
		plParam->m_bDebugMode = MULTL_XMLGetNodeMark(plXMLHolder, "debug_mode_mark");

		MULTL_IPoTSCalcParameters();
	}
	else
	{
		GLOBAL_TRACE(("Find eca_setting Failed!\n"));
	}

}


void MULT_IPoTSXMLSave(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bStatistics)
{
	CHAR_T plTmpStr[512];
	S32 lTmpStrLen;
	mxml_node_t *plXMLHolder;

	if (bStatistics)
	{
		S32 i;
		MULT_IPoTSParam *plParam;
		MULT_IPoTSDownlinkStatistic *plDownStat;
		plParam = &s_IPoTsHandle.m_Parameter;

		plXMLHolder = pXMLRoot;

		MULTL_XMLAddNodeText(plXMLHolder, "uplink_constellation", MULTL_XMLQAMModeValueToStr(s_IPoTsHandle.m_UplinkConstellation));
		MULTL_XMLAddNodeFLOAT(plXMLHolder, "uplink_bandwidth_bps", s_IPoTsHandle.m_UplinkBandwidthBPS);
		MULTL_XMLAddNodeFLOAT(plXMLHolder, "uplink_cur_bps", s_IPoTsHandle.m_UplinkCurrentBPS);

		MULTL_XMLAddNodeText(plXMLHolder, "downlink_constellation", MULTL_XMLQAMModeValueToStr(s_IPoTsHandle.m_DownlinkConstellation));
		MULTL_XMLAddNodeFLOAT(plXMLHolder, "downlink_bandwidth_bps", s_IPoTsHandle.m_DownlinkBandwidthBPS);
		MULTL_XMLAddNodeFLOAT(plXMLHolder, "downlink_cur_bps", s_IPoTsHandle.m_DownlinkCurrentBPS);

		for (i = 0; i < s_IPoTsHandle.m_DownlinkChnNum; i++)
		{
			plXMLHolder = mxmlNewElement(pXMLRoot, "chn_status");
			if (plXMLHolder)
			{
				plDownStat = &s_IPoTsHandle.m_pStatistics[i];
				MULTL_XMLAddNodeUINT(plXMLHolder, "cur_bps", plDownStat->m_CurrentBitrateBPS);
				MULTL_XMLAddNodeText(plXMLHolder, "cur_constella", MULTL_XMLQAMModeValueToStr(plDownStat->m_Constellation));
				MULTL_XMLAddNodeFLOAT(plXMLHolder, "strength", plDownStat->m_Strength);
				MULTL_XMLAddNodeFLOAT(plXMLHolder, "quality", plDownStat->m_Quality);
				MULTL_XMLAddNodeFLOATE(plXMLHolder, "ber", plDownStat->m_BER);
				MULTL_XMLAddNodeMark(plXMLHolder, "lock", plDownStat->m_bLock);
			}
		}
	}
	else
	{
		MULT_IPoTSParam *plParam;
		plParam = &s_IPoTsHandle.m_Parameter;
		/*参数保存*/
		plXMLHolder = mxmlNewElement(pXMLRoot, "ipots_setting");
		if (plXMLHolder)
		{
			MULTL_XMLAddNodeUINT(plXMLHolder, "uplink_freq", plParam->m_UpLinkCenterFreqHz);
			MULTL_XMLAddNodeINT(plXMLHolder, "uplink_bandwidth", plParam->m_UpLinkBandWidth);
			MULTL_XMLAddNodeText(plXMLHolder, "uplink_constellation", MULTL_XMLQAMModeValueToStr(plParam->m_UplinkConstellation));
			MULTL_XMLAddNodeINT(plXMLHolder, "uplink_gain", plParam->m_UpLinkGain);

			MULTL_XMLAddNodeUINT(plXMLHolder, "downlink_freq", plParam->m_DownLinkCenterFreqHz);
			MULTL_XMLAddNodeINT(plXMLHolder, "downlink_bandwidth", plParam->m_DownLinkBandWidth);
			MULTL_XMLAddNodeMark(plXMLHolder, "downlink_iq_swapt_mark", plParam->m_bDownLinkIQSwap);

			MULTL_XMLAddNodeMark(plXMLHolder, "constellatio_adapt_mark", plParam->m_bConstellationAutoAdapt);
			MULTL_XMLAddNodeMark(plXMLHolder, "debug_mode_mark", plParam->m_bDebugMode);


			/*固定参数不读取*/
			MULTL_XMLAddNodeINT(plXMLHolder, "gain_level_max", 40);
			MULTL_XMLAddNodeFLOAT(plXMLHolder, "gain_stepping", 0.25);

			/*辅助网页进行显示的参数，不读取*/
			MULTL_XMLAddNodeINT(plXMLHolder, "uplink_chn_num", s_IPoTsHandle.m_UplinkChnNum);
			//MULTL_XMLAddNodeUINT(plXMLHolder, "uplink_symbol_bps", s_IPoTsHandle.m_UplinkSingleSymbolRate);
			MULTL_XMLAddNodeINT(plXMLHolder, "downlink_chn_num", s_IPoTsHandle.m_DownlinkChnNum);
			//MULTL_XMLAddNodeUINT(plXMLHolder, "downlink_symbol_bps", s_IPoTsHandle.m_DownlinkSingleSymbolRate);
		}

	}
}


void MULT_IPoTSMonitorProcess(S32 Duration)
{
	S32 i, lDownConstellation;
	MULT_IPoTSDownlinkStatistic *plDownStat;

	for (i = 0; i < s_IPoTsHandle.m_DownlinkChnNum; i++)
	{
		plDownStat = &s_IPoTsHandle.m_pStatistics[i];
		HWL_TDGetSignalFloat(i, &plDownStat->m_bLock, &plDownStat->m_Strength, &plDownStat->m_Quality, &plDownStat->m_BER);
		if (plDownStat->m_bLock)
		{
			plDownStat->m_Constellation = HWL_TDGetDVBCConStellation(i);
		}
		else
		{
			plDownStat->m_Constellation = GLOBAL_INVALID_INDEX;
		}
		plDownStat->m_CurrentBitrateBPS = HWL_GetBitrate(0, 8 + i);

		//GLOBAL_TRACE(("CHN %d, %s, Pow = %f, SN = %f, BER = %E, Bitrate = %d bps, %s\n", i, plDownStat->m_bLock?"Locked":"Not Locked", plDownStat->m_Strength, plDownStat->m_Quality, plDownStat->m_BER, plDownStat->m_CurrentBitrateBPS, MULTL_XMLQAMModeValueToStr(plDownStat->m_Constellation)));
	}


	lDownConstellation = GLOBAL_INVALID_INDEX;
	for (i = 0; i < s_IPoTsHandle.m_DownlinkChnNum; i++)
	{
		plDownStat = &s_IPoTsHandle.m_pStatistics[i];
		if (plDownStat->m_bLock == FALSE)
		{
			break;
		}
		else
		{
			if (lDownConstellation == GLOBAL_INVALID_INDEX)
			{
				lDownConstellation = plDownStat->m_Constellation;
			}
			else
			{
				if (lDownConstellation != plDownStat->m_Constellation)
				{
					lDownConstellation = GLOBAL_INVALID_INDEX;
					break;
				}
			}
		}
	}

	s_IPoTsHandle.m_DownlinkConstellation = lDownConstellation;

	s_IPoTsHandle.m_UplinkBandwidthBPS = MULTL_IPoTSGetDVBCBandwidthBps(s_IPoTsHandle.m_UplinkSingleSymbolRate, s_IPoTsHandle.m_UplinkConstellation) * s_IPoTsHandle.m_UplinkChnNum;
	s_IPoTsHandle.m_DownlinkBandwidthBPS = MULTL_IPoTSGetDVBCBandwidthBps(s_IPoTsHandle.m_DownlinkSingleSymbolRate, s_IPoTsHandle.m_DownlinkConstellation) * s_IPoTsHandle.m_DownlinkChnNum;
}


void MULT_IPoTSApply(void)
{
	S32 i, lDownBandwidth;
	U32 lFirstFreq, lChnMark;
	HWL_ModulatorParam_t lParam;
	MULT_IPoTSParam *plParam;


	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));

	plParam = &s_IPoTsHandle.m_Parameter;


	GLOBAL_TRACE(("Uplink Band = %d, CHN Num = %d, SingleBand = %d Hz, SingleSymbol = %d \n", plParam->m_UpLinkBandWidth, s_IPoTsHandle.m_UplinkChnNum, s_IPoTsHandle.m_UplinkSingleBandHz, s_IPoTsHandle.m_UplinkSingleSymbolRate));

	lFirstFreq = plParam->m_UpLinkCenterFreqHz - (s_IPoTsHandle.m_UplinkChnNum - 1) * (s_IPoTsHandle.m_UplinkSingleBandHz / 2);
	lParam.Gain = plParam->m_UpLinkGain;
	lParam.ModulateStandard = HWL_CONST_MODULATOR_STANDARD_ANNEX_A;
	lParam.QamMode = plParam->m_UplinkConstellation;//暂时指定为参数设置，不支持自动适应
	lParam.SymRate = s_IPoTsHandle.m_UplinkSingleSymbolRate;
	if (plParam->m_bDebugMode)
	{
		lParam.QamSwitch = FALSE;
	}
	else
	{
		lParam.QamSwitch = TRUE;
	}

	lParam.SpectrumInvert = FALSE;

	lChnMark = 0;
	for (i = 0; i < MULT_IPoTS_MAX_CHN; i++)
	{
		lParam.ModulateFreq = lFirstFreq + i * s_IPoTsHandle.m_UplinkSingleBandHz;
		if (i < s_IPoTsHandle.m_UplinkChnNum)
		{
			lParam.RFSwitch = TRUE;
			lChnMark |= (1 << i);
		}
		else
		{

			lParam.QamSwitch = FALSE;
			lParam.RFSwitch = FALSE;
		}
		GLOBAL_TRACE(("Uplink [%d] Freq = %d, Symbol = %d, QAM Mode = %d, Gain = %d, QAM Switch = %d, RF Switch = %d\n", i,  lParam.ModulateFreq, lParam.SymRate, lParam.QamMode, lParam.Gain, lParam.QamSwitch, lParam.RFSwitch));
		HWL_QAMParameterSetNew(i, &lParam, NULL);
	}
	HWL_QAMParameterApply(0);
	MULTL_IPoTSProtocolSetCHNMark(lChnMark);


	GLOBAL_TRACE(("Downlink Band = %d, CHN Num = %d, SingleBand = %d Hz, SingleSymbol = %d \n", plParam->m_DownLinkBandWidth, s_IPoTsHandle.m_DownlinkChnNum, s_IPoTsHandle.m_DownlinkSingleBandHz, s_IPoTsHandle.m_DownlinkSingleSymbolRate));

	lFirstFreq = plParam->m_DownLinkCenterFreqHz - (s_IPoTsHandle.m_DownlinkChnNum - 1) * (s_IPoTsHandle.m_DownlinkSingleBandHz / 2);
	lParam.SymRate = s_IPoTsHandle.m_DownlinkSingleSymbolRate;

	switch (s_IPoTsHandle.m_DownlinkSingleBandHz)
	{
	case 8000000:
		{
			lDownBandwidth = TD_CMN_BAND_8M;
		}
		break;
	case 7000000:
		{
			lDownBandwidth = TD_CMN_BAND_7M;
		}
		break;
	case 6000000:
		{
			lDownBandwidth = TD_CMN_BAND_6M;
		}
		break;
	default:
		{
			lDownBandwidth = TD_CMN_BAND_8M;
		}
	}

	for (i = 0; i < MULT_IPoTS_MAX_CHN; i++)
	{
		if (i < s_IPoTsHandle.m_DownlinkChnNum)
		{
			lParam.ModulateFreq = lFirstFreq + i * s_IPoTsHandle.m_DownlinkSingleBandHz;
		}
		else
		{
			lParam.ModulateFreq = 0;
		}
		GLOBAL_TRACE(("Downlink [%d] Freq = %d, SymbolRate = %d, Band = %d, IQSwapt = %d\n", i,  lParam.ModulateFreq, lParam.SymRate, lDownBandwidth, plParam->m_bDownLinkIQSwap));
		HWL_TDApplyParam(i, lParam.ModulateFreq, lParam.SymRate, lDownBandwidth, plParam->m_bDownLinkIQSwap, FALSE);
	}

}

void MULT_IPoTSTerminate(void)
{
	GLOBAL_ZEROMEM(&s_IPoTsHandle, sizeof(MULT_IPoTSHandle));
}

void MULTL_IPoTSCalcChannels(S32 Bandwidth, S32 *pChnNum, S32 *pSingleBand)
{
	S32 lChnNum;
	S32 lSingleBand;

	switch (Bandwidth)
	{
	case MULT_IPoTS_BAND_32M:
		{
			lChnNum = 4;
			lSingleBand = 8000000;
		}
		break;
	case MULT_IPoTS_BAND_28M:
		{
			lChnNum = 4;
			lSingleBand = 7000000;
		}
		break;
	case MULT_IPoTS_BAND_24M:
		{
			lChnNum = 3;
			lSingleBand = 8000000;
		}
		break;
	case MULT_IPoTS_BAND_21M:
		{
			lChnNum = 3;
			lSingleBand = 7000000;
		}
		break;
	case MULT_IPoTS_BAND_18M:
		{
			lChnNum = 3;
			lSingleBand = 6000000;
		}
		break;
	case MULT_IPoTS_BAND_16M:
		{
			lChnNum = 2;
			lSingleBand = 8000000;
		}
		break;
	case MULT_IPoTS_BAND_14M:
		{
			lChnNum = 2;
			lSingleBand = 7000000;
		}
		break;
	case MULT_IPoTS_BAND_12M:
		{
			lChnNum = 2;
			lSingleBand = 6000000;
		}
		break;
	case MULT_IPoTS_BAND_8M:
		{
			lChnNum = 1;
			lSingleBand = 8000000;
		}
		break;
	case MULT_IPoTS_BAND_7M:
		{
			lChnNum = 1;
			lSingleBand = 7000000;
		}
		break;
	case MULT_IPoTS_BAND_6M:
		{
			lChnNum = 1;
			lSingleBand = 6000000;
		}
		break;
	default:
		{
			lChnNum = 4;
			lChnNum = 8000000;
		}
	}

	if (pChnNum)
	{
		(*pChnNum) = lChnNum;
	}

	if (pSingleBand)
	{
		(*pSingleBand) = lSingleBand;
	}
}

void MULTL_IPoTSCalcParameters(void)
{
	MULT_IPoTSParam *plParam;
	plParam = &s_IPoTsHandle.m_Parameter;

	MULTL_IPoTSCalcChannels(plParam->m_UpLinkBandWidth, &s_IPoTsHandle.m_UplinkChnNum, &s_IPoTsHandle.m_UplinkSingleBandHz);
	MULTL_IPoTSCalcChannels(plParam->m_DownLinkBandWidth, &s_IPoTsHandle.m_DownlinkChnNum, &s_IPoTsHandle.m_DownlinkSingleBandHz);

	s_IPoTsHandle.m_UplinkSingleSymbolRate = (S32)(CAL_EXGetSymbolRateByBadwidth(s_IPoTsHandle.m_DownlinkSingleBandHz, 0.15) * BAND_USAGE);
	s_IPoTsHandle.m_DownlinkSingleSymbolRate = (S32)(CAL_EXGetSymbolRateByBadwidth(s_IPoTsHandle.m_DownlinkSingleBandHz, 0.15) * BAND_USAGE);

	s_IPoTsHandle.m_UplinkConstellation = plParam->m_UplinkConstellation;//暂时强制等于设置值


	GLOBAL_TRACE(("Uplink CHN = %d, Symbol = %d, SingBand = %d\n", s_IPoTsHandle.m_UplinkChnNum, s_IPoTsHandle.m_UplinkSingleSymbolRate, s_IPoTsHandle.m_UplinkSingleBandHz));
	GLOBAL_TRACE(("Downlink CHN = %d, Symbol = %d, SingBand = %d\n", s_IPoTsHandle.m_DownlinkChnNum, s_IPoTsHandle.m_DownlinkSingleSymbolRate, s_IPoTsHandle.m_DownlinkSingleBandHz));
}



F64 MULTL_IPoTSGetDVBCBandwidthBps(S32 SymbolRate, S32 Constellation)
{
	S32 lRate = 0;
	switch (Constellation)
	{
	case GS_MODULATOR_QAM_16:
		lRate = 4;
		break;
	case GS_MODULATOR_QAM_32:
		lRate = 5;
		break;
	case GS_MODULATOR_QAM_64:
		lRate = 6;
		break;
	case GS_MODULATOR_QAM_128:
		lRate = 7;
		break;
	case GS_MODULATOR_QAM_256:
		lRate = 8;
		break;
	default:
		lRate = 0;
		break;
	}

	//GLOBAL_TRACE(("Downlink Band = %f bps, SymbolRate = %d, Constellation = %d, Rate = %d\n", ((F64)SymbolRate) * lRate * 188 / 204, SymbolRate, Constellation, lRate));

	return ((F64)SymbolRate) * lRate * 188 / 204;
}


void MULTL_IPoTSProtocolSetCHNMark(U32 Mark)
{
	S32 lLen, lTmpValue;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;
	S16 lPhyIndex;

	lLen = 0;
	plTmpBuf = plCMDBuf;

	GLOBAL_MSB8_EC(plTmpBuf, 0x37/**/, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB32_EC(plTmpBuf, Mark, lLen);

	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}


#endif



/*EOF*/
