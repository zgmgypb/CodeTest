#include "multi_private.h"
#include "multi_drv.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "global_micros.h"
#include "mpeg2_micro.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "liu_iic.h"
/*
*3650DS/3655的板子
SCL			PA3		PIN45
SDA			PB10	PIN90
Direction	PC10	PIN11	对于ARM来说高是输入

1815/1818的板子
SDA			--ARM PIN67
SCL			--ARM pin66  是直接连接的
*/

#include "TunerDemod.h"


HANDLE32 s_TDHandle = NULL;
HANDLE32 s_TDLock = NULL;

//BOOL TDL_CMNI2CWrite(HANDLE32 Handle, U8 *pWriteData, S32 WriteLen)
//{
//	BOOL lRet;
//	if ((lRet = TUNER_Write(pWriteData[0] & 0xFE ,pWriteData[1], &pWriteData[2], WriteLen - 2, 1)) == NORMAL)
//	{
//		return TRUE;
//	}
//	else
//	{
//		GLOBAL_TRACE(("I2C Write Error = %d, ChipAddr = %.2X\n", lRet, pWriteData[0]));
//	}
//	return FALSE;
//}
//
//BOOL TDL_CMNI2CRead(HANDLE32 Handle, U8 *pWriteData, S32 WriteLen, U8 *pReadData, S32 ReadLen)
//{
//	BOOL lRet;
//	//if ((lRet = TUNER_Write(pWriteData[0] & 0xFE ,pWriteData[1], &pWriteData[2], WriteLen - 2, 0)) == NORMAL)
//	//{
//		if ((lRet = TUNER_Read(pWriteData[0] & 0xFE, pReadData, ReadLen)) == NORMAL)
//		{
//			return TRUE;
//		}
//		else
//		{
//			GLOBAL_TRACE(("I2C Read Error = %d, ChipAddr = %.2X\n", lRet, pWriteData[0]));
//		}
//	//}
//	//else
//	//{
//	//	GLOBAL_TRACE(("Write Error!!!\n"));
//	//}
//	return FALSE;
//}

BOOL TDL_CMNI2CWriteNEW(HANDLE32 Handle, U8 ChipAddr8Bit, U8 *pWriteData, S32 WriteLen, BOOL bSendStopConditon)
{
	if (TUNER_Write(ChipAddr8Bit ,pWriteData[0], &pWriteData[1], WriteLen - 1, bSendStopConditon?1:0) == NORMAL)
	{
		return TRUE;
	}
	else
	{
		GLOBAL_TRACE(("I2C Write Error, ChipAddr = %.2X\n", ChipAddr8Bit));
	}
	return FALSE;
}

BOOL TDL_CMNI2CReadNEW(HANDLE32 Handle, U8 ChipAddr8Bit, U8 *pReadData, S32 ReadLen, BOOL bSendStopConditon)
{
	if (TUNER_Read(ChipAddr8Bit, pReadData, ReadLen) == NORMAL)
	{
		return TRUE;
	}
	else
	{
		GLOBAL_TRACE(("I2C Read Error, ChipAddr = %.2X\n", ChipAddr8Bit));
	}
	return FALSE;
}

void TDL_CMNDelay(HANDLE32 Handle, S32 MS)
{
	PFC_TaskSleep(MS);
}

void TDL_CMNReset(HANDLE32 Handle, S32 MS)
{

}

/*Demod 检测*/
S32 HWL_TDDemodCheck(S32 *pType)
{
	S32 i, lType, lFirstType, lCount = 0;
	TD_InitParameter lParam;
	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));

	lParam.m_pDelay = TDL_CMNDelay;
	//lParam.m_pReadFunc = TDL_CMNI2CRead;
	//lParam.m_pWriteFunc = TDL_CMNI2CWrite;
	lParam.m_pReadFuncNEW = TDL_CMNI2CReadNEW;
	lParam.m_pWriteFuncNEW = TDL_CMNI2CWriteNEW;
	lParam.m_pReset = TDL_CMNReset;
	lParam.m_pUserParam = NULL;

	lFirstType = TD_DEMOD_TYPE_UNKNOW;
	for (i = 0; i< HWL_TUNER_MAX_NUM; i++)
	{
		HWL_SetI2cAndTunerReset(i,0,0);
		PFC_TaskSleep(100);
		HWL_SetI2cAndTunerReset(i,1,0);


		lType = TD_DetectDemodType(&lParam);
		if (lType != TD_DEMOD_TYPE_UNKNOW)
		{
			lCount++;
			GLOBAL_TRACE(("Detect Demod OK At Slot = %d, Type = %d\n", i, lType));
		}
		else
		{
			GLOBAL_TRACE(("Detect Demod Failed At Slot = %d\n", i));
			break;
		}

		if (i == 0)
		{
			lFirstType = lType;
		}
	}

	GLOBAL_TRACE((" ---------------------------- Demod Count = %d, Type = %d ------------------- \n", lCount, lFirstType));

	if (pType)
	{
		(*pType) = lFirstType;
	}

	PFC_TaskSleep(1500);

	return lCount;
}

void HWL_TDInitiateTDModule(S32 HWLSubChnType)
{
	TD_InitParameter lParam;
	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));
	if (HWLSubChnType == HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869)
	{

		GLOBAL_TRACE(("Init DTMB ATMB8869(S)\n"));
		lParam.m_SystemType = TD_SYS_TYPE_DTMB;
		lParam.m_TunerType = TD_TUNER_TYPE_MXL603;
		lParam.m_DemodType = TD_DEMOD_TYPE_ATBM8869;
		lParam.m_bUseDemodPassI2C = TRUE;
		lParam.m_bDemodTsGPIO0 = FALSE;
		lParam.m_bDemodTsBitOrderMSB = TRUE;
		lParam.m_bDemodTsRisingEdge = FALSE;
		lParam.m_bDemodTsSerial = TRUE;

	}
	else
	{
		/*DVB-S2 Tuner初始化的方法*/
		GLOBAL_TRACE(("Init DVB-SX AVL6211\n"));

		lParam.m_SystemType = TD_SYS_TYPE_DVBSX;
		lParam.m_TunerType = TD_TUNER_TYPE_AV2011_2012;
		lParam.m_DemodType = TD_DEMOD_TYPE_AVL6211;
		lParam.m_bUseDemodPassI2C = TRUE;
		lParam.m_bDemodTsGPIO0 = TRUE;
		lParam.m_bDemodTsBitOrderMSB = TRUE;
		lParam.m_bDemodTsRisingEdge = FALSE;
		lParam.m_bDemodTsSerial = TRUE;
	}

	lParam.m_pDelay = TDL_CMNDelay;
	//lParam.m_pReadFunc = TDL_CMNI2CRead;
	//lParam.m_pWriteFunc = TDL_CMNI2CWrite;
	lParam.m_pReadFuncNEW = TDL_CMNI2CReadNEW;
	lParam.m_pWriteFuncNEW = TDL_CMNI2CWriteNEW;
	lParam.m_pReset = TDL_CMNReset;
	lParam.m_pUserParam = NULL;

	s_TDHandle = TD_Create(&lParam);
	s_TDLock = PFC_SemaphoreCreate("", 1);
	PFC_SemaphoreSignal(s_TDLock);
}


void HWL_TDApplyParam(S32 Slot, S32 FrequencyHz, S32 SymboRate, S32 Bandwidth, BOOL SpecInv, BOOL bEnable22K)
{
	if (s_TDHandle)
	{
		if (PFC_SemaphoreWait(s_TDLock, 1000))
		{
			HWL_SetI2cAndTunerReset(Slot, 0, 0);
			PFC_TaskSleep(100);
			HWL_SetI2cAndTunerReset(Slot, 1, 0);

			GLOBAL_TRACE(("Slot = %d, Freq = %d, Symbol = %d, bSpectINV = %d, b22k = %d\n", Slot, FrequencyHz, SymboRate, SpecInv, bEnable22K));

			TD_SetFrequncy(s_TDHandle, FrequencyHz);
			TD_SetCommonParam(s_TDHandle, Bandwidth, ((SpecInv>0)?1:0));
			TD_DVBCSetParam(s_TDHandle, 0, SymboRate);
			TD_DVBSXSetParam(s_TDHandle, 0, SymboRate, bEnable22K);
			TD_ApplyParam(s_TDHandle);

			PFC_SemaphoreSignal(s_TDLock);
		}
	}
}


void HWL_TDGetSignalFloat(S32 Slot, BOOL *pbLock, F64 *pStrenth, F64 *pQuanlity, F64 *pBER)
{
	if (s_TDHandle != NULL)
	{
		TD_DemodSignal lSignal;
		BOOL blLock;

		if (PFC_SemaphoreWait(s_TDLock, 1000))
		{
			HWL_SetI2cAndTunerReset(Slot, 1, 0);

			blLock = TD_GetDemodLock(s_TDHandle);;


			TD_GetDemodSignalStatus(s_TDHandle, &lSignal);

			if (blLock == FALSE)
			{
				lSignal.m_Quanlity = 0;
				lSignal.m_BER = 1.0;
			}
			PFC_SemaphoreSignal(s_TDLock);
		}

		//GLOBAL_TRACE(("Slot = %d, Lock = %d, Signal %.3f, Quanlity = %.3f, BER = %.3e, BERpre = %e\n", Slot, TD_GetDemodLock(s_TDHandle), lSignal.m_Strength, lSignal.m_Quanlity, lSignal.m_BER, lSignal.m_PER));

		if (pStrenth)
		{
			(*pStrenth) = lSignal.m_Strength;
		}
		if (pQuanlity)
		{
			(*pQuanlity) = lSignal.m_Quanlity;
		}
		if (pBER)
		{
			(*pBER) = lSignal.m_BER;
		}
		if (pbLock)
		{
			(*pbLock) = blLock;
		}
	}
}

S32 HWL_TDGetDVBCConStellation(S32 Slot)
{
	S32 lRetValue = -1;
	if (s_TDHandle != NULL)
	{
		TD_DVBCCHNlInfo lDVBCParam;

		if (PFC_SemaphoreWait(s_TDLock, 1000))
		{
			HWL_SetI2cAndTunerReset(Slot, 1, 0);

			TD_DVBCGetCHNInfo(s_TDHandle, &lDVBCParam);

			lRetValue = lDVBCParam.m_QAMMode;

			PFC_SemaphoreSignal(s_TDLock);
		}

	}
	return lRetValue;
}

BOOL HWL_TDGetDVBSChannelParam(S32 Slot, TD_DVBSXCHNlInfo *pDVBSXParam)
{
	BOOL lRetValue = FALSE;
	if (s_TDHandle != NULL)
	{
		if (PFC_SemaphoreWait(s_TDLock, 1000))
		{
			HWL_SetI2cAndTunerReset(Slot, 1, 0);

			lRetValue = TD_DVBSXGetCHNInfo(s_TDHandle, pDVBSXParam);

			PFC_SemaphoreSignal(s_TDLock);
		}

	}
	return lRetValue;
}

void HWL_DTMBTDParamToCHNParam(TD_DTMBParameter *pTDParam, MULT_SubModulatorInfo *pSubModuationInfo)
{
	pSubModuationInfo->m_DoublePilot = pTDParam->m_bDoublePilot;

	switch (pTDParam->m_CarrierMode)
	{
	case TD_DTMB_CARRIER_MODE_SINGLE:
		pSubModuationInfo->m_CarrierMode = GS_MODULATOR_CARRIER_MODE_1;
		break;
	case TD_DTMB_CARRIER_MODE_3780:
		pSubModuationInfo->m_CarrierMode = GS_MODULATOR_CARRIER_MODE_3780;
		break;
	}

	switch (pTDParam->m_CodeRate)
	{
	case TD_DTMB_CODE_RATE_04:
		pSubModuationInfo->m_CodeRate = GS_MODULATOR_CODE_RATE_0_4;
		break;
	case TD_DTMB_CODE_RATE_06:
		pSubModuationInfo->m_CodeRate = GS_MODULATOR_CODE_RATE_0_6;
		break;
	case TD_DTMB_CODE_RATE_08:
		pSubModuationInfo->m_CodeRate = GS_MODULATOR_CODE_RATE_0_8;
		break;
	}

	switch (pTDParam->m_PNMode)
	{
	case TD_DTMB_PN_MODE_420:
		pSubModuationInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_420F;
		break;
	case TD_DTMB_PN_MODE_420C:
		pSubModuationInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_420C;
		break;
	case TD_DTMB_PN_MODE_595:
		pSubModuationInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_595;
		break;
	case TD_DTMB_PN_MODE_945:
		pSubModuationInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_945F;
		break;
	case TD_DTMB_PN_MODE_945C:
		pSubModuationInfo->m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_945C;
		break;
	}


	switch (pTDParam->m_QAMMode)
	{
	case TD_DTMB_QAM_MAP_4QAMNR:
		pSubModuationInfo->m_Mode = GS_MODULATOR_QAM_4NR;
		break;
	case TD_DTMB_QAM_MAP_4QAM:
		pSubModuationInfo->m_Mode = GS_MODULATOR_QAM_4;
		break;
	case TD_DTMB_QAM_MAP_16QAM:
		pSubModuationInfo->m_Mode = GS_MODULATOR_QAM_16;
		break;
	case TD_DTMB_QAM_MAP_32QAM:
		pSubModuationInfo->m_Mode = GS_MODULATOR_QAM_32;
		break;
	case TD_DTMB_QAM_MAP_64QAM:
		pSubModuationInfo->m_Mode = GS_MODULATOR_QAM_64;
		break;
	}

	switch (pTDParam->m_TIMode)
	{
	case TD_DTMB_TI_MODE_240:
		pSubModuationInfo->m_TimeInterleave = GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240;
		break;
	case TD_DTMB_TI_MODE_720:
		pSubModuationInfo->m_TimeInterleave = GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_720;
		break;
	}

}

void HWL_DTMBCHNParamToTDParam(MULT_SubModulatorInfo *pSubModuationInfo, TD_DTMBParameter *pTDParam)
{
	pTDParam->m_bDoublePilot = pSubModuationInfo->m_DoublePilot;

	switch (pSubModuationInfo->m_CarrierMode)
	{
	case GS_MODULATOR_CARRIER_MODE_1:
		pTDParam->m_CarrierMode = TD_DTMB_CARRIER_MODE_SINGLE;
		break;
	case GS_MODULATOR_CARRIER_MODE_3780:
		pTDParam->m_CarrierMode = TD_DTMB_CARRIER_MODE_3780;
		break;
	}

	switch (pSubModuationInfo->m_CodeRate)
	{
	case GS_MODULATOR_CODE_RATE_0_4:
		pTDParam->m_CodeRate = TD_DTMB_CODE_RATE_04;
		break;
	case GS_MODULATOR_CODE_RATE_0_6:
		pTDParam->m_CodeRate = TD_DTMB_CODE_RATE_06;
		break;
	case GS_MODULATOR_CODE_RATE_0_8:
		pTDParam->m_CodeRate = TD_DTMB_CODE_RATE_08;
		break;
	}

	switch (pSubModuationInfo->m_PNMode)
	{
	case GS_MODULATOR_GUARD_INTERVAL_PN_420C:
		pTDParam->m_PNMode = TD_DTMB_PN_MODE_420C;
		break;
	case GS_MODULATOR_GUARD_INTERVAL_PN_420F:
		pTDParam->m_PNMode = TD_DTMB_PN_MODE_420;
		break;
	case GS_MODULATOR_GUARD_INTERVAL_PN_595:
		pTDParam->m_PNMode = TD_DTMB_PN_MODE_595;
		break;
	case GS_MODULATOR_GUARD_INTERVAL_PN_945C:
		pTDParam->m_PNMode = TD_DTMB_PN_MODE_945C;
		break;
	case GS_MODULATOR_GUARD_INTERVAL_PN_945F:
		pTDParam->m_PNMode = TD_DTMB_PN_MODE_945;
		break;
	}


	switch (pSubModuationInfo->m_Mode)
	{
	case GS_MODULATOR_QAM_4NR:
		pTDParam->m_QAMMode = TD_DTMB_QAM_MAP_4QAMNR;
		break;
	case GS_MODULATOR_QAM_4:
		pTDParam->m_QAMMode = TD_DTMB_QAM_MAP_4QAM;
		break;
	case GS_MODULATOR_QAM_16:
		pTDParam->m_QAMMode = TD_DTMB_QAM_MAP_16QAM;
		break;
	case GS_MODULATOR_QAM_32:
		pTDParam->m_QAMMode = TD_DTMB_QAM_MAP_32QAM;
		break;
	case GS_MODULATOR_QAM_64:
		pTDParam->m_QAMMode = TD_DTMB_QAM_MAP_64QAM;
		break;
	}

	switch (pSubModuationInfo->m_TimeInterleave)
	{
	case GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240:
		pTDParam->m_TIMode = TD_DTMB_TI_MODE_240;
		break;
	case GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_720:
		pTDParam->m_TIMode = TD_DTMB_TI_MODE_720;
		break;
	}
}


/*测试程序~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~！！！*/

void HWL_TDDemodTest(void)
{
	S32 lType;
	BOOL blLock;
	TD_DemodSignal lSignal;
	TD_InitParameter lParam;
	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));

	{
		PFC_GPIOSetupPIN(PFCGPIO_PA(25), FALSE, TRUE);

		PFC_GPIOSetPIN(PFCGPIO_PA(25), 0);
		PFC_TaskSleep(100);
		PFC_GPIOSetPIN(PFCGPIO_PA(25), 1);
		PFC_TaskSleep(10);
	}



	lParam.m_pDelay = TDL_CMNDelay;
	//lParam.m_pReadFunc = TDL_CMNI2CRead;
	//lParam.m_pWriteFunc = TDL_CMNI2CWrite;
	lParam.m_pReadFuncNEW = TDL_CMNI2CReadNEW;
	lParam.m_pWriteFuncNEW = TDL_CMNI2CWriteNEW;
	lParam.m_pReset = TDL_CMNReset;
	lParam.m_pUserParam = NULL;

	//{
	//	/*测试寄存器*/
	//	U8 lRegAddr;
	//	U8 lRegValue;

	//	lRegAddr = 0x02;
	//	lRegValue = 0x4C;
	//	while(TRUE)
	//	{
	//		//U8 plTmpBuf[2];

	//		//if (lRegAddr <= 4)
	//		//{
	//		//	plTmpBuf[0] = lRegAddr;
	//		//	plTmpBuf[1] = lRegValue;

	//		//	TD_WriteRegs(&lParam, 0xA0, plTmpBuf, 2, NULL, 0);
	//		//	PFC_TaskSleep(10);
	//		//}

	//		TUNER_Write(0xA0, lRegAddr, &lRegValue, 1, 1);
	//		TD_ReadRegs(&lParam, 0xA0, &lRegAddr, 1, &lRegValue, 1);
	//		GLOBAL_TRACE(("Reg Addr = 0x%02X, Read = 0x%02X\n", lRegAddr, lRegValue));
	//		lRegAddr++;

	//		lRegAddr = 0x04;
	//		lRegValue = 0x04;
	//		TD_WriteRegs(&lParam, 0x18, &lRegAddr, 1, &lRegValue, 1);

	//		PFC_TaskSleep(1000);
	//	}
	//}

	lType = TD_DetectDemodType(&lParam);

	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));


	GLOBAL_TRACE(("Init ABS-S\n"));

	lParam.m_SystemType = TD_SYS_TYPE_ABS;
	lParam.m_TunerType = TD_TUNER_TYPE_RDA5815M;
	lParam.m_DemodType = TD_DEMOD_TYPE_HI3123E;
	lParam.m_bUseDemodPassI2C = TRUE;
	lParam.m_bDemodTsGPIO0 = FALSE;
	lParam.m_bDemodTsBitOrderMSB = TRUE;
	lParam.m_bDemodTsRisingEdge = FALSE;
	lParam.m_bDemodTsSerial = TRUE;

	lParam.m_pDelay = TDL_CMNDelay;
	//lParam.m_pReadFunc = TDL_CMNI2CRead;
	//lParam.m_pWriteFunc = TDL_CMNI2CWrite;
	lParam.m_pReadFuncNEW = TDL_CMNI2CReadNEW;
	lParam.m_pWriteFuncNEW = TDL_CMNI2CWriteNEW;
	lParam.m_pReset = TDL_CMNReset;
	lParam.m_pUserParam = NULL;


	s_TDHandle = TD_Create(&lParam);

	TD_SetFrequncy(s_TDHandle, 1270 * 1000000);
	TD_SetCommonParam(s_TDHandle, 8000, TRUE);
	TD_DVBSXSetParam(s_TDHandle, 0, 28.8 * 1000000, FALSE);
	TD_ApplyParam(s_TDHandle);

	PFC_TaskSleep(2000);

	while(TRUE)
	{

		blLock = TD_GetDemodLock(s_TDHandle);;

		TD_GetDemodSignalStatus(s_TDHandle, &lSignal);

		GLOBAL_TRACE(("Lock = %d, Signal %f, Quanlity = %f, BER = %e, BERPre = %e\n", blLock, lSignal.m_Strength, lSignal.m_Quanlity, lSignal.m_BER, lSignal.m_PER));

		PFC_TaskSleep(3000);
	}

}



//EOF
