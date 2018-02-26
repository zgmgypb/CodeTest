#include "multi_drv.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "global_micros.h"
#include "mpeg2_micro.h"
#include "platform_assist.h"
#include "multi_private.h"

#ifdef GN2000


/*
马赛克系统的特殊操作部分
1、将复用到输出TS-01上的所有节目都复用到输出TS-16上，
2、在输出TS-01上固定一个节目，节目固定一个视频ES，类型符合编码器的输出（调用删除命令时，清楚上述复用，并重新生成固定节目）
3、调用DB函数之后将复用到TS-02上的音频找到，并让其转向复用到输出TS-01上，加入固定的输出TS-01上的节目01当中
4、将输出到TS-02上的多个节目，按照设定的顺序（1-6）安排PMT PID（0x101 ~ 0x106）
5、根据复用的结果在输出TS-01的固定节目的SDT描述符中，添加Servic描述符，用于描述编码生成的6个节目的名称！
*/

#include "mb86391.h"
#include "tvp5146.h"


static HANDLE32 s_UARTHandle = NULL;

void HWL_Mb86391_DelayMSCB(S32 MS)
{
	PFC_TaskSleep(MS);
}

S32 HWL_Mb86391_UartSendFn(U8 *pBuf, S32 len)
{
	//CAL_PrintDataBlock(__FUNCTION__, pBuf, len);
	return PFC_ComDeviceWrite(s_UARTHandle, pBuf, len);
}

S32 HWL_Mb86391_UartRecvFn(U8 *pBuf, S32 len)
{
	S32 lLen = 0;
	lLen = PFC_ComDeviceRead(s_UARTHandle, pBuf, len);
	//CAL_PrintDataBlock(__FUNCTION__, pBuf, lLen);
	return lLen;
}


BOOL8 HWL_TVP5146_SetRegisterFn(U8 RegiserId, U8 RegisterValue)
{
	S32 lRet;
	//GLOBAL_TRACE(("RegiserID = %.2X, RegisterValue = %.2X\n", RegiserId, RegisterValue));
	if ( (lRet = TUNER_Write(0xBA, RegiserId, &RegisterValue, 1, TRUE)) == NORMAL)
	{
		return TRUE;
	}
	else
	{
		GLOBAL_TRACE(("I2C Read Error = %d\n", lRet));
	}
	return FALSE;
}

BOOL8 HWL_TVP5146_GetRegisterFn(U8 RegiserId, U8 *pBuf)
{
	S32 lRet;
	if ((lRet = TUNER_Write(0xBA ,RegiserId, pBuf, 0, 0)) == NORMAL)
	{
		if ((lRet = TUNER_Read(0xBA, pBuf, 1)) == NORMAL)
		{
			return TRUE;
		}
		else
		{
			GLOBAL_TRACE(("I2C Read Error = %d\n", lRet));
		}
	}
	//GLOBAL_TRACE(("RegiserID = %.2X, RegisterValue = %.2X\n", RegiserId, (*pBuf)));
	return FALSE;
}

Tuner_Notify HWL_EncoderI2CCB(U8 ChannelNo, U8 Reset,U8 LNBPower)
{                                                                                                                                                          
	PFC_TaskSleep(1000000000);
}

void HWL_EncoderReset(BOOL bEnable)
{
	U8 TmpBuff[16];
	GLOBAL_MEMSET(TmpBuff, 0, sizeof(TmpBuff));
	TmpBuff[0] = 0x11;	
	TmpBuff[1] = 0x01;
	TmpBuff[2] = 0x00;	
	TmpBuff[3] = 0x00;

	TmpBuff[4] = 0x22;
	TmpBuff[5] = 0x00;
	TmpBuff[6] = 0x00;
	TmpBuff[7] = (bEnable<<6);	

	HWL_FPGAWrite(&TmpBuff[0], 8);
}

void HWL_EncoderInit(void)
{
#if 0
	{
		s_UARTHandle = PFC_ComDeviceOpen(2, TRUE);
		if (s_UARTHandle)
		{
			U8 plTmpData[100];
			PFC_ComDeviceSetState(s_UARTHandle, 9600, 8, 'E', 1);//固定！！

			PFC_ComDeviceSetOption(s_UARTHandle, 0, 16, 1000, 1000);

			while(1)
			{
				plTmpData[0] = 0x55;
				PFC_ComDeviceWrite(s_UARTHandle, plTmpData, 1);
				GLOBAL_TRACE(("Write Data To S3 = %.2X\n", plTmpData[0]));
				plTmpData[0] = 0;
				if (PFC_ComDeviceRead(s_UARTHandle, plTmpData, 1) > 0)
				{
					GLOBAL_TRACE(("Read Data To S3 = %.2X\n", plTmpData[0]));
				}
				else
				{
					GLOBAL_TRACE(("Read Failed!\n"));
				}
				PFC_TaskSleep(500);
			}
		}
	}
#endif

	{
		//initTunerDriver(1, DVB_C,(Tuner_Notify)HWL_EncoderI2CCB);
		InitTvp5146(HWL_TVP5146_SetRegisterFn, HWL_TVP5146_GetRegisterFn);
	}

	{
		s_UARTHandle = PFC_ComDeviceOpen(2, TRUE);
		if (s_UARTHandle)
		{
			PFC_ComDeviceSetState(s_UARTHandle, 9600, 8, 'E', 1);//固定！！

			PFC_ComDeviceSetOption(s_UARTHandle, 0, 16, 1000, 1000);

		}
		else
		{
			GLOBAL_TRACE(("Init COM 1 Failed!\n"));
		}

		InitMb86391(HWL_Mb86391_UartSendFn, HWL_Mb86391_UartRecvFn, HWL_Mb86391_DelayMSCB, TRUE);
	}
	GLOBAL_TRACE(("Init Encoder Module\n"));
}

void HWL_EncoderApply(void)
{
	Tvp5146Parameter	lTV5146Param;
	Mb86391EncodeParameter lNB86391Param;

	GLOBAL_TRACE(("Apply Encoder Module In!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));

	HWL_EncoderReset(TRUE);

	PFC_TaskSleep(1000);

	HWL_EncoderReset(FALSE);

	GLOBAL_TRACE(("Apply Encoder Reset Complete!\n"));

	ConfigTvp5146AllParameterToDefault(&lTV5146Param);

	ConfigTvp5146AllParameter(&lTV5146Param);

	ConfigMb86391AllParameterToDefault(&lNB86391Param);
	lNB86391Param.m_VideoBitrate = 5200; 
	lNB86391Param.m_PcrPid = 888;
	lNB86391Param.m_VideoPid = 889;

	ConfigMb86391AllParameter(&lNB86391Param);

	GLOBAL_TRACE(("Apply Encoder Module Out!\n"));

#if 0//测试用
	{
		while(1)
		{
			GLOBAL_TRACE(("Lock Status = %d\n", TVP5146_CheckLockStatus()));
			PFC_TaskSleep(1000);
		}
	}
#endif
}

void HWL_EncoderTerm(void)
{
	if (s_UARTHandle)
	{
		PFC_ComDeviceClose(s_UARTHandle);
		s_UARTHandle = NULL;
	}
}

void HWL_EncoderSetAudioPIDDelay(HWL_AudioOffsetNode *pArray, S32 PIDNum)
{
	S32 i, lFramInd, lPow, lTmmOffset;
	S32 lLen, lTmpValue;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

	if (PIDNum >= HWL_MAX_AUDIO_OFFSET_PID_NUM)
	{
		PIDNum = HWL_MAX_AUDIO_OFFSET_PID_NUM;
	}

	for (lFramInd = 0; lFramInd <  2; lFramInd++)
	{
		lLen = 0;
		plTmpBuf = plCMDBuf;

		GLOBAL_MSB8_EC(plTmpBuf, 0x30, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x07, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
		GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

		if ((PIDNum > 0) && (lFramInd == 0))
		{
			lTmpValue = lFramInd | 0x00;
		}
		else if ((PIDNum > 6) && (lFramInd == 1))
		{
			lTmpValue = lFramInd | 0x00;
		}
		else
		{
			lTmpValue = lFramInd | 0x80;
			pArray[0].m_TsInd = 0;
		}
		GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);//Frame Index

		lTmpValue = pArray[0].m_TsInd;
		GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);

		lTmpValue = pArray[0].m_PCRPID;
		GLOBAL_MSB16_EC(plTmpBuf, lTmpValue, lLen);

		for (i = (lFramInd * 6) ;i < ((lFramInd + 1) * 6); i++)
		{

			if (i < PIDNum)
			{
				lTmmOffset = abs(pArray[i].m_Offset);

				lPow = 0;
				//while(lTmmOffset > 728)
				//{
				//	lTmmOffset = lTmmOffset / 2;
				//	lPow++;
				//}

				GLOBAL_TRACE(("Pow = %d\n", lPow));

				lTmpValue = pArray[i].m_PID | ((lPow & 0x03) << 14);
				GLOBAL_MSB16_EC(plTmpBuf, lTmpValue, lLen);

				//lTmpValue = lTmmOffset * 45;
				lTmpValue = lTmmOffset;
				GLOBAL_MSB16_EC(plTmpBuf, lTmpValue, lLen);
			}
			else
			{
				lTmpValue = MPEG2_TS_PACKET_NULL_PID;
				GLOBAL_MSB16_EC(plTmpBuf, lTmpValue, lLen);

				lTmpValue = 0;
				GLOBAL_MSB16_EC(plTmpBuf, lTmpValue, lLen);
			}
		}

		HWL_FPGAWrite(plCMDBuf, lLen);
	}
}





#endif


