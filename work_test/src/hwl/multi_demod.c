
/*---tuner---------*/

#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "global_micros.h"

#define TUNER_CMD_LENGTH   8
static U8 g_Reset;
static U8 g_ChannelNo;

HWL_TunerParam_t s_TUNERParam[HWL_TUNER_MAX_NUM];
HWL_TunerParam_t s_ParamCMP[HWL_TUNER_MAX_NUM];

/*-------------------------------------------------------------------------------------*/
void HWL_TunerSetParameter(S16 TsIndex,HWL_TunerParam_t *lParam)
{
	if (GLOBAL_CHECK_INDEX(TsIndex, HWL_TUNER_MAX_NUM))
	{	
		GLOBAL_MEMCPY(&s_TUNERParam[TsIndex], lParam, sizeof(HWL_TunerParam_t));
	}
}

//void HWL_TunerReset(S32 ChnNo)
//{
//	U8 TmpBuff[TUNER_CMD_LENGTH];
//	GLOBAL_MEMSET(TmpBuff, 0, sizeof(TmpBuff));
//
//	TmpBuff[0] = 0x11;	
//	TmpBuff[1] = TUNER_CMD_LENGTH/4 - 1;
//	TmpBuff[2] = 0x00;	
//	TmpBuff[3] = 0x00;
//	TmpBuff[4] = 0x22;
//	TmpBuff[5] = 0x00;
//	TmpBuff[6] = 0x00;
//
//	TmpBuff[7] = 1<< 6;	
//	HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);
//#endif
//
///*----口线切换------*/
//	TmpBuff[6] = 0x01;
//	TmpBuff[7] = ChnNo;
//	HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);
//
//
//}

void HWL_TunerApplyParameter(S16 Tunercount, S16 ChannelType)
{
	
	U8  TmpBuff[TUNER_CMD_LENGTH];	
	S16 i;
	GLOBAL_MEMSET(TmpBuff, 0, sizeof(TmpBuff));
	U8   FunctionControl;
	TunerPara plTmpTunerParam[HWL_TUNER_MAX_NUM];

	TmpBuff[0] = 0x11;	
	TmpBuff[1] = TUNER_CMD_LENGTH/4 - 1;
	TmpBuff[2] = 0;	
	TmpBuff[3] = 0;
	TmpBuff[4] = 0x22;
	TmpBuff[5] = 0x00;		

	for(i = 0; i< Tunercount; i++)			
	{
#if 0
		FunctionControl = 0;	
		if( GS_TUNER_PLOAR_NONE == s_TUNERParam[i].PolarMethod)
		{
			FunctionControl |= (GS_TUNER_PLOAR_NONE<<4);
		}
		else if ( GS_TUNER_PLOAR_HOR == s_TUNERParam[i].PolarMethod)
		{
			FunctionControl |= (GS_TUNER_PLOAR_HOR<<4);
		}
		else
		{
			FunctionControl |= (GS_TUNER_PLOAR_VER<<4);
		}


		TmpBuff[6] = 0x00;			
		TmpBuff[7] = ( (1<<6) | ((s_TUNERParam[i].PolarMethod << 4) & 0x30) | (i & 0x0F));	
		HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);	

		PFC_TaskSleep(100);

		TmpBuff[7] = ( (0<<6) | ((s_TUNERParam[i].PolarMethod << 4) & 0x30) | (i & 0x0F));	
		HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);	

		if((0 == g_Reset) && (i == g_ChannelNo))
		{
			sleep(0.01);
			TmpBuff[7] |= (1<<6);
			HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);
		}
		else
		{
			TmpBuff[7] |= (1<<6);
			HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);
		}
#endif

#if 0//为什么要在这里切换口线
		TmpBuff[6] = 0x01;			
		TmpBuff[7] = i;			
		HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);	
#endif


		plTmpTunerParam[i].bandwith = 8;
		plTmpTunerParam[i].Frequency = s_TUNERParam[i].Frequency;
		plTmpTunerParam[i].IQInvert = s_TUNERParam[i].PolarMethod;
		plTmpTunerParam[i].lnb22K = s_TUNERParam[i].Switch_22K;
		plTmpTunerParam[i].lnbPower = 0;
		plTmpTunerParam[i].lnbSymbol = s_TUNERParam[i].Symbol;
		plTmpTunerParam[i].LocalFreq = s_TUNERParam[i].LocalFreq;
		plTmpTunerParam[i].modulation = s_TUNERParam[i].Modulation;
		plTmpTunerParam[i].TuneReqType = s_TUNERParam[i].TuneReqType;
	}

	{
		for(i = 0; i< Tunercount; i++)			
		{
			if (HWL_CHANNEL_TYPE_TUNER_S == ChannelType)
			{
				HWL_TDApplyParam(i, abs(s_TUNERParam[i].Frequency - s_TUNERParam[i].LocalFreq) * 1000 * 10, s_TUNERParam[i].Symbol * 1000, 0, !s_TUNERParam[i].SpectInv, s_TUNERParam[i].Switch_22K);
			}
			else
			{
				HWL_TDApplyParam(i, s_TUNERParam[i].Frequency * 1000 * 10, s_TUNERParam[i].Symbol * 1000, 0, s_TUNERParam[i].SpectInv, !s_TUNERParam[i].Switch_22K);
			}
		}
	}
}

void HWL_SetI2cAndTunerReset(U8 ChannelNo, U8 Reset,U8 LNBPower)
{                                                                                                                                                          
	U8 TmpBuff[TUNER_CMD_LENGTH];
	GLOBAL_MEMSET(TmpBuff, 0, sizeof(TmpBuff));
	g_Reset = Reset;

	g_ChannelNo = ChannelNo;
	TmpBuff[0] = 0x11;	
	TmpBuff[1] = TUNER_CMD_LENGTH/4 - 1;
	TmpBuff[2] = 0x00;	
	TmpBuff[3] = 0x00;

	TmpBuff[4] = 0x22;
	TmpBuff[5] = 0x00;
	TmpBuff[6] = 0x00;

//为什么要在这里切换供电？
	if (ChannelNo < HWL_TUNER_MAX_NUM)
	{
		LNBPower = s_TUNERParam[ChannelNo].PolarMethod;
	}
	TmpBuff[7] = ( (Reset<<6) | ((LNBPower << 4) & 0x30) | (ChannelNo & 0x0F));	
	HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);

//口线切换
	TmpBuff[6] = 0x01;
	TmpBuff[7] = ChannelNo;
	HWL_FPGAWrite(&TmpBuff[0], TUNER_CMD_LENGTH);

}

/*EOF*/	
