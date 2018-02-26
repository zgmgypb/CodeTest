#include "gn_hwl_mfpga.h"
#include "gn_drv.h"

/* FPGA通信地址定义 */
/* PID映射模块 */
#define FPGA_PIDMAP_ENC_SUBBOARD_SEL		0x200
#define FPGA_PIDMAP_PID_CHANGED_FLAG		0x201
#define FPGA_PIDMAP_CH1_VID_OLD_PID_H		0x202
#define FPGA_PIDMAP_CH1_VID_OLD_PID_L		0x203
#define FPGA_PIDMAP_CH1_AUD_OLD_PID_H		0x204
#define FPGA_PIDMAP_CH1_AUD_OLD_PID_L		0x205
#define FPGA_PIDMAP_CH1_PCR_OLD_PID_H		0x206
#define FPGA_PIDMAP_CH1_PCR_OLD_PID_L		0x207
#define FPGA_PIDMAP_CH2_VID_OLD_PID_H		0x208
#define FPGA_PIDMAP_CH2_VID_OLD_PID_L		0x209
#define FPGA_PIDMAP_CH2_AUD_OLD_PID_H		0x20A
#define FPGA_PIDMAP_CH2_AUD_OLD_PID_L		0x20B
#define FPGA_PIDMAP_CH2_PCR_OLD_PID_H		0x20C
#define FPGA_PIDMAP_CH2_PCR_OLD_PID_L		0x20D
#define FPGA_PIDMAP_CH1_VID_NEW_PID_H		0x20E
#define FPGA_PIDMAP_CH1_VID_NEW_PID_L		0x20F
#define FPGA_PIDMAP_CH1_AUD_NEW_PID_H	0x210
#define FPGA_PIDMAP_CH1_AUD_NEW_PID_L		0x211
#define FPGA_PIDMAP_CH1_PCR_NEW_PID_H		0x212
#define FPGA_PIDMAP_CH1_PCR_NEW_PID_L		0x213
#define FPGA_PIDMAP_CH2_VID_NEW_PID_H		0x214
#define FPGA_PIDMAP_CH2_VID_NEW_PID_L		0x215
#define FPGA_PIDMAP_CH2_AUD_NEW_PID_H	0x216
#define FPGA_PIDMAP_CH2_AUD_NEW_PID_L		0x217
#define FPGA_PIDMAP_CH2_PCR_NEW_PID_H		0x218
#define FPGA_PIDMAP_CH2_PCR_NEW_PID_L		0x219
#define FPGA_PIDMAP_CH_ADD_STEP					(FPGA_PIDMAP_CH2_VID_OLD_PID_H - FPGA_PIDMAP_CH1_VID_OLD_PID_H)
/* PID过滤 */
#define FPGA_PIDFILTER_ENC_SUBBOARD_SEL	0x21A
#define FPGA_PIDFILTER_CH1_PCR_PID_H			0x21B
#define FPGA_PIDFILTER_CH1_PCR_PID_L			0x21C
#define FPGA_PIDFILTER_CH1_VID_PID_H			0x21D
#define FPGA_PIDFILTER_CH1_VID_PID_L			0x21E
#define FPGA_PIDFILTER_CH1_AUD_PID_H			0x21F
#define FPGA_PIDFILTER_CH1_AUD_PID_L			0x220
#define FPGA_PIDFILTER_CH2_PCR_PID_H			0x221
#define FPGA_PIDFILTER_CH2_PCR_PID_L			0x222
#define FPGA_PIDFILTER_CH2_VID_PID_H			0x223
#define FPGA_PIDFILTER_CH2_VID_PID_L			0x224
#define FPGA_PIDFILTER_CH2_AUD_PID_H			0x225
#define FPGA_PIDFILTER_CH2_AUD_PID_L			0x226
#define FPGA_PIDFILTER_CH_ADD_STEP					(FPGA_PIDFILTER_CH2_PCR_PID_H - FPGA_PIDFILTER_CH1_PCR_PID_H)
/* 写PSI/SI信息 */
#define FPGA_PSI_WRITE_START						0x227
#define FPGA_PSI_WRITE_END							0x228
#define FPGA_PSI_PACKET_NUM							0x229	
#define FPGA_PSI_SEND_INTERVAL					0x22A /* 发送时间检测，25～250 */	
#define FPGA_PSI_SDT_PACKET_NUM					0x289 
#define FPGA_PSI_SECOND_SDT_PACKET_POS	0x28A /* 如有2个SDT包，写入第二个SDT包在总PSI包中的位置（即第几个包） */
#define FPGA_PSI_BASE_ADDR							0x300
/* 码率控制 */
#define FPGA_SET_OUT_BITRATE_H					0x22B
#define FPGA_SET_OUT_BITRATE_L					0x22C
/* RS232通信模块控制 */
#define FPGA_RS232_RESET_ADD   					0x22D
#define FPGA_RS232_WFUL_ADD_1					0x22E
#define FPGA_RS232_RD_EN_1							0x22F
#define FPGA_RS232_WFUL_ADD_2					0x230
#define FPGA_RS232_RD_EN_2							0x231
#define FPGA_RS232_RD_DATA_1						0x232
#define FPGA_RS232_RD_DATA_2						0x233
#define FPGA_RS232_WFUL_ADD_3					0x27F
#define FPGA_RS232_RD_EN_3							0x280
#define FPGA_RS232_WFUL_ADD_4					0x281
#define FPGA_RS232_RD_EN_4							0x282
#define FPGA_RS232_RD_DATA_3						0x283
#define FPGA_RS232_RD_DATA_4						0x284
/* 其他控制信号的读取及控制 */
#define FPGA_IN_TS_CC_ERROR_FLAG				0x234
#define FPGA_IN_CH1_BITRATE_H						0x235
#define FPGA_IN_CH2_BITRATE_H						0x236
#define FPGA_IN_CH3_BITRATE_H						0x237
#define FPGA_IN_CH4_BITRATE_H						0x238
#define FPGA_IN_CH5_BITRATE_H						0x239
#define FPGA_IN_CH6_BITRATE_H						0x23A
#define FPGA_IN_CH7_BITRATE_H						0x23B
#define FPGA_IN_CH8_BITRATE_H						0x23C
#define FPGA_IN_BITRATE_L								0x23D
//注：
//023EH～0245H：读出的码率为0～13的数值,仅用于判断相应的通道是否有码流，不作为实时码率依据。
#define FPGA_IN_CH1_VID_AUD_PACKET_NUM	0x23E
#define FPGA_IN_CH2_VID_AUD_PACKET_NUM	0x23F
#define FPGA_IN_CH3_VID_AUD_PACKET_NUM	0x240
#define FPGA_IN_CH4_VID_AUD_PACKET_NUM	0x241
#define FPGA_IN_CH5_VID_AUD_PACKET_NUM	0x242
#define FPGA_IN_CH6_VID_AUD_PACKET_NUM	0x243
#define FPGA_IN_CH7_VID_AUD_PACKET_NUM	0x244
#define FPGA_IN_CH8_VID_AUD_PACKET_NUM	0x245

#define FPGA_CARD_STATUS_VERSION_H			0x246
#define FPGA_VERSION_L									0x247
#define FPGA_ID_H											0x248
#define FPGA_ID_L											0x249
#define FPGA_SN_0											0x24A
#define FPGA_SN_1											0x24B
#define FPGA_SN_2											0x24C
#define FPGA_SN_3											0x24D
#define FPGA_GET_OUT_BITRATE_L					0x24E
#define FPGA_GET_OUT_BITRATE_H					0x24F

#define FPGA_UART_SEL_1								0x285
#define FPGA_UART_SEL_2								0x286
#define FPGA_ENC_BOARD_RESET						0x287

static pthread_mutex_t main_fpga_mutex;

static U8 MFPGA_Read(U32 ADDRESS)
{
	U8 lData = 0;

	if (DRL_FpgaRead(GN_FPGA_INDEX_MAIN, ADDRESS, &lData, 1) < 0)
		return -1;

	return lData;
}

static void MFPGA_Write(U32 ADDRESS , S32 DATA)
{
	U8 lData = (U8)(DATA & 0xFF);
	DRL_FpgaWrite(GN_FPGA_INDEX_MAIN, ADDRESS, &lData, 1);
}

BOOL MFPGA_Init(void)
{
	pthread_mutex_init(&main_fpga_mutex,NULL);

	return TRUE;
}

BOOL MFPGA_Terminate(void)
{
	pthread_mutex_destroy(&main_fpga_mutex);

	return TRUE;
}

BOOL MFPGA_GetRelease(CHAR_T *pRelease)
{
	U8 lData[2];
	U16 lTmpVar;

	lData[0] = MFPGA_Read(FPGA_CARD_STATUS_VERSION_H) & 0x7F;
	lData[1] = MFPGA_Read(FPGA_VERSION_L) & 0xFF;

	//add by leonli for test
	GLOBAL_TRACE(("data1 = %d\n", lData[0]));
	GLOBAL_TRACE(("data2 = %d\n", lData[1]));

	lTmpVar = (lData[0] << 8) | lData[1];
	GLOBAL_SPRINTF((pRelease, "%02d.%02d.%02d", ((lTmpVar >> 9) & 0x3F) + 9, (lTmpVar >> 5) & 0x0F, lTmpVar & 0x1F));

	return TRUE;
}

U16 MFPGA_GetFpgaId(void)
{
	return ((MFPGA_Read(FPGA_ID_H) << 8) | MFPGA_Read(FPGA_ID_L));
}

/* 返回4位数据，每位表示一个编码子板的输入状态 */
U8 MFPGA_GetInputCCErrorFlag(void)
{
	return MFPGA_Read(FPGA_IN_TS_CC_ERROR_FLAG);
}

/* 返回加密卡ID */
U32 MFPGA_GetSecretCardId(void)
{
	U8 lData[4];

	DRL_FpgaRead(GN_FPGA_INDEX_MAIN, FPGA_SN_0, lData, 4);

	return (U32)((lData[3] << 24) | (lData[2] << 16) | (lData[1] << 8) | lData[0]);
}

/* 返回主板到输出板的实时有效数据速率, 返回单位kbps */
U32 MFPGA_GetOutValidBitrate(void)
{
	U8 lData[2];
	U32 lBitrate;

	lData[0] = MFPGA_Read(FPGA_GET_OUT_BITRATE_L);
	lData[1] = MFPGA_Read(FPGA_GET_OUT_BITRATE_H);

	lBitrate = ((lData[1] << 8) & 0xFF00) | (lData[0] & 0xFF);
	lBitrate = (lBitrate * 188 * 8) / 1024;

	return lBitrate;
}

BOOL MFPGA_SetEncBoardReset(U32 EncBoardIndex)
{
	

	return TRUE;
}

BOOL MFPGA_SetUartSelect(U32 EncBoardIndex)
{
	switch (EncBoardIndex)
	{
	case DXT8243_INDEX_1:
		MFPGA_Write(FPGA_UART_SEL_1, 0x01);
		break;
	case DXT8243_INDEX_2:
		MFPGA_Write(FPGA_UART_SEL_1, 0x02);
		break;
	case DXT8243_INDEX_3:
		MFPGA_Write(FPGA_UART_SEL_2, 0x01);
		break;
	case DXT8243_INDEX_4:
		MFPGA_Write(FPGA_UART_SEL_2, 0x02);
		break;
	default:
		break;
	}

	return TRUE;
}

U8 MFPGA_GetInputVideoPacketNum(U32 ChIndex)
{
	return ((READ_FPGA(FPGA_IN_CH1_VID_AUD_PACKET_NUM + ChIndex) >> 4) & 0x0F); /* 高4位是视频 */
}

U8 MFPGA_GetInputAudioPacketNum(U32 ChIndex)
{
	return (READ_FPGA(FPGA_IN_CH1_VID_AUD_PACKET_NUM + ChIndex) & 0x0F); /* 低4位是音频 */
}

/*
	返回值：TRUE 溢出   FALSE 不溢出
	传入值：OutBitrate 设置的输出编码码率，单位kbps
	说明：移植于GN1898，代码具体含义不懂，不能用于多线程
*/
BOOL MFPGA_GetBufferOverflowStatus(U32 OutBitrate)
{
	U32 lRatePercent[6]; /* 分别代表0%, 10%, 20%, 30%, 40%, 50% */
	static BOOL lOldStatus = FALSE;
	U32 lTmpData = 0;
	U32 lCurTotalBitrate = 0;
	U32 lChIndex = 0;
	S32 lN30P = 0, lM10P = 0;

	lTmpData = OutBitrate * 66;
	lTmpData /= 1000;
	lRatePercent[0] = 0;
	lRatePercent[1] = lTmpData;
	lRatePercent[2] = lRatePercent[1] << 1;
	lRatePercent[3] = (lRatePercent[1] * 3) / 2;
	lRatePercent[4] = lRatePercent[3] + lRatePercent[1];
	lRatePercent[5] = lRatePercent[4] + lRatePercent[1];
	lRatePercent[1] *= 8;
	lRatePercent[1] /= 10;

	for (lChIndex=0; lChIndex<GN_ENC_CH_NUM; lChIndex++)
	{
		lCurTotalBitrate = lCurTotalBitrate + (((READ_FPGA(FPGA_IN_CH1_BITRATE_H + lChIndex) << 8) & 0xFF00) | (READ_FPGA(FPGA_IN_BITRATE_L) & 0xFF));
	}

	if (lCurTotalBitrate >= lRatePercent[3])
	{
		if (lN30P < 10)
		{
			lN30P ++ ;
		}
		lM10P = 0 ;
	}
	else if (lCurTotalBitrate < lRatePercent[1])		
	{
		if (lM10P < 10)
		{
			lM10P ++ ;
		}
		lN30P = 0 ;
	}
	else
	{
		lM10P = 0 ;
		lN30P = 0 ;
	}

	if (lN30P > 7) /* 溢出 */
	{
		lOldStatus = TRUE;
	}
	else if ((lM10P > 7) && (lN30P < 3))
	{
		lOldStatus = FALSE;
	}

	return lOldStatus;
}

/* 移植于GN1898，具体含义不清楚 */
BOOL MFPGA_SetOutBitrate(U32 OutBitrate)
{
	U32 lOutPlus = 0, lLargeNumber = 0;

	lOutPlus = 432000 / OutBitrate;
	lLargeNumber = 432000 % OutBitrate;
	lLargeNumber *= 188;
	lLargeNumber /= OutBitrate ;  
	if (lLargeNumber > 185)
	{
		lLargeNumber = 0; 
		lOutPlus++ ;
	}

	MFPGA_Write(FPGA_SET_OUT_BITRATE_H, lOutPlus & 0xFF);
	MFPGA_Write(FPGA_SET_OUT_BITRATE_L, lLargeNumber & 0xFF);

	return TRUE;
}

BOOL MFPGA_RS232Reset(void)
{
	MFPGA_Write(FPGA_RS232_RESET_ADD, 1); /* 写1复位 */
	usleep(100);
	MFPGA_Write(FPGA_RS232_RESET_ADD, 0);

	return TRUE;
}

BOOL MFPGA_RS232Write()
{

}

BOOL MFPGA_RS232Read()
{

}

BOOL MFPGA_SetPidMap(U32 EncBoardIndex, MFPGA_PidMapParam *pPidMapPara)
{
	S32 i;

	pthread_mutex_lock(&main_fpga_mutex);
	MFPGA_Write(FPGA_PIDMAP_ENC_SUBBOARD_SEL, 0x01 << EncBoardIndex);
	MFPGA_Write(FPGA_PIDMAP_PID_CHANGED_FLAG, 0x03);

	for (i=0; i<GN_CH_NUM_PER_ENC_BOARD; i++)
	{
		MFPGA_Write(FPGA_PIDMAP_CH1_VID_OLD_PID_H + FPGA_PIDMAP_CH_ADD_STEP * i, (pPidMapPara->m_OldPid[i].m_VideoPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDMAP_CH1_VID_OLD_PID_L + FPGA_PIDMAP_CH_ADD_STEP * i, pPidMapPara->m_OldPid[i].m_VideoPid & 0xFF);
		MFPGA_Write(FPGA_PIDMAP_CH1_AUD_OLD_PID_H + FPGA_PIDMAP_CH_ADD_STEP * i, (pPidMapPara->m_OldPid[i].m_AudioPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDMAP_CH1_AUD_OLD_PID_L + FPGA_PIDMAP_CH_ADD_STEP * i, pPidMapPara->m_OldPid[i].m_AudioPid & 0xFF);
		MFPGA_Write(FPGA_PIDMAP_CH1_PCR_OLD_PID_H + FPGA_PIDMAP_CH_ADD_STEP * i, (pPidMapPara->m_OldPid[i].m_PcrPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDMAP_CH1_PCR_OLD_PID_L + FPGA_PIDMAP_CH_ADD_STEP * i, pPidMapPara->m_OldPid[i].m_PcrPid & 0xFF);

		MFPGA_Write(FPGA_PIDMAP_CH1_VID_NEW_PID_H + FPGA_PIDMAP_CH_ADD_STEP * i, (pPidMapPara->m_NewPid[i].m_VideoPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDMAP_CH1_VID_NEW_PID_L + FPGA_PIDMAP_CH_ADD_STEP * i, pPidMapPara->m_NewPid[i].m_VideoPid & 0xFF);
		MFPGA_Write(FPGA_PIDMAP_CH1_AUD_NEW_PID_H + FPGA_PIDMAP_CH_ADD_STEP * i, (pPidMapPara->m_NewPid[i].m_AudioPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDMAP_CH1_AUD_NEW_PID_L + FPGA_PIDMAP_CH_ADD_STEP * i, pPidMapPara->m_NewPid[i].m_AudioPid & 0xFF);
		MFPGA_Write(FPGA_PIDMAP_CH1_PCR_NEW_PID_H + FPGA_PIDMAP_CH_ADD_STEP * i, (pPidMapPara->m_NewPid[i].m_PcrPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDMAP_CH1_PCR_NEW_PID_L + FPGA_PIDMAP_CH_ADD_STEP * i, pPidMapPara->m_NewPid[i].m_PcrPid & 0xFF);
	}
	MFPGA_Write(FPGA_PIDMAP_ENC_SUBBOARD_SEL, 0x00);
	pthread_mutex_unlock(&main_fpga_mutex);

	return TRUE;
}

BOOL MFPGA_SetPidFilter(U32 EncBoardIndex, MFPGA_PidFilterParam *pPidFilterPara)
{
	S32 i;

	pthread_mutex_lock(&main_fpga_mutex);
	MFPGA_Write(FPGA_PIDFILTER_ENC_SUBBOARD_SEL, 0x01 << EncBoardIndex);

	for (i=0; i<GN_CH_NUM_PER_ENC_BOARD; i++)
	{
		MFPGA_Write(FPGA_PIDFILTER_CH1_VID_PID_H + FPGA_PIDFILTER_CH_ADD_STEP * i, (pPidFilterPara->m_PidGroup[i].m_VideoPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDFILTER_CH1_VID_PID_L + FPGA_PIDFILTER_CH_ADD_STEP * i, pPidFilterPara->m_PidGroup[i].m_VideoPid & 0xFF);
		MFPGA_Write(FPGA_PIDFILTER_CH1_AUD_PID_H + FPGA_PIDFILTER_CH_ADD_STEP * i, (pPidFilterPara->m_PidGroup[i].m_AudioPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDFILTER_CH1_AUD_PID_L + FPGA_PIDFILTER_CH_ADD_STEP * i, pPidFilterPara->m_PidGroup[i].m_AudioPid & 0xFF);
		MFPGA_Write(FPGA_PIDFILTER_CH1_PCR_PID_H + FPGA_PIDFILTER_CH_ADD_STEP * i, (pPidFilterPara->m_PidGroup[i].m_PcrPid >> 8) & 0x1F);
		MFPGA_Write(FPGA_PIDFILTER_CH1_PCR_PID_L + FPGA_PIDFILTER_CH_ADD_STEP * i, pPidFilterPara->m_PidGroup[i].m_PcrPid & 0xFF);
	}
	MFPGA_Write(FPGA_PIDFILTER_ENC_SUBBOARD_SEL, 0x00);
	pthread_mutex_unlock(&main_fpga_mutex);

	return TRUE;
}

BOOL MFPGA_SetPsiInsert(MFPGA_PsiInsertParam *pPsiInsertPara)
{
	S32 i, j;
	S32 lFpgaAddr = FPGA_PSI_BASE_ADDR;

	pthread_mutex_lock(&main_fpga_mutex);
	MFPGA_Write(FPGA_PSI_WRITE_START, 0x0);
	for (i=0; i<pPsiInsertPara->m_PsiPacketNum; i++)
	{
		for (j=0; j<MPEG2_TS_PACKET_SIZE; j++)
		{
			//printf ("address: 0x%06x data: 0x%02x\n", lFpgaAddr, pPsiInsertPara->m_Data[i][j]);
			MFPGA_Write(lFpgaAddr++, pPsiInsertPara->m_Data[i][j]);
		}
	}
	MFPGA_Write(FPGA_PSI_WRITE_END, 0x0);
	MFPGA_Write(FPGA_PSI_PACKET_NUM, pPsiInsertPara->m_PsiPacketNum);
	MFPGA_Write(FPGA_PSI_SEND_INTERVAL, pPsiInsertPara->m_Interval / 2);

	MFPGA_Write(FPGA_PSI_SDT_PACKET_NUM, pPsiInsertPara->m_SdtPacketNum);
	if (pPsiInsertPara->m_SdtPacketNum > 1) 
	{ /* 有两个SDT包，则写入第二个SDT包的位置 */
		MFPGA_Write(FPGA_PSI_SECOND_SDT_PACKET_POS, pPsiInsertPara->m_SecondSdtPacketPosition);
	}
	pthread_mutex_unlock(&main_fpga_mutex);

	return TRUE;
}

#define MAX_DOWNLOAD_TIME 5 /* 最大下载次数 */
#define MAIN_FPGA_ID (0x5A88)
BOOL MFPGA_ConfigRbf(void)
{
	S32 lConfigCounter = 0;

	while (1)
	{
		if (lConfigCounter ++ >= MAX_DOWNLOAD_TIME)
		{
			GLOBAL_TRACE(("Config Main Fpga Failed!!\n"));
			return FALSE;
		}
		GLOBAL_PRINTF(("Config Main Fpga: %d\n", lConfigCounter));

		//if (DRL_MainFpgaConfig() == TRUE)
		//{
			//if (MFPGA_GetFpgaId() == MAIN_FPGA_ID)
			//{
				//GLOBAL_TRACE(("Config Main Fpga Success!!\n"));
				//return TRUE;
			//}
		//}
	}
}

//add by leonli for GQ serial equipment
#define FPGA_DATA_NULL			0x1FFF
#define DEFAULT_OUT_BITRATE (60000) /* 60M */
BOOL HWL_SetParaToMainFpga(HWL_MfpgaCfgParam *pCfgParam)
{
	S32 i, j;

	{ /* PID滤波和映射 */
		MFPGA_PidFilterParam lPidFilterPara;
		MFPGA_PidMapParam lPidMapPara;

		for (i=0; i<GN_ENC_BOARD_NUM; i++)
		{
			for (j=0; j<GN_CH_NUM_PER_ENC_BOARD; j++)
			{
				if (pCfgParam->m_ChParam[j + i * 2].m_WorkEn == TRUE)
				{
					/* 注意：处理流程是进入的数据映射，然后滤波出去，所以滤波的Pid是最终输出的PID */
					lPidFilterPara.m_PidGroup[j].m_AudioPid = pCfgParam->m_ChParam[j + i * 2].m_AudioPid;
					lPidFilterPara.m_PidGroup[j].m_PcrPid = pCfgParam->m_ChParam[j + i * 2].m_PcrPid;
					lPidFilterPara.m_PidGroup[j].m_VideoPid = pCfgParam->m_ChParam[j + i * 2].m_VideoPid;

					lPidMapPara.m_NewPid[j].m_AudioPid = pCfgParam->m_ChParam[j + i * 2].m_AudioPid;
					lPidMapPara.m_NewPid[j].m_PcrPid = pCfgParam->m_ChParam[j + i * 2].m_PcrPid;
					lPidMapPara.m_NewPid[j].m_VideoPid = pCfgParam->m_ChParam[j + i * 2].m_VideoPid;

					lPidMapPara.m_OldPid[j].m_AudioPid = HWL_AUD_PID_BASE + j + i * 2;
					if (pCfgParam->m_ChParam[j + i * 2].m_PcrPid == pCfgParam->m_ChParam[j + i * 2].m_VideoPid)
						lPidMapPara.m_OldPid[j].m_PcrPid = HWL_VID_PID_BASE + j + i * 2;
					else
						lPidMapPara.m_OldPid[j].m_PcrPid = HWL_PCR_PID_BASE + j + i * 2;
					lPidMapPara.m_OldPid[j].m_VideoPid = HWL_VID_PID_BASE + j + i * 2;
				}
				else
				{
					lPidFilterPara.m_PidGroup[j].m_AudioPid = FPGA_DATA_NULL;
					lPidFilterPara.m_PidGroup[j].m_PcrPid = FPGA_DATA_NULL;
					lPidFilterPara.m_PidGroup[j].m_VideoPid = FPGA_DATA_NULL;

					lPidMapPara.m_NewPid[j].m_AudioPid = FPGA_DATA_NULL;
					lPidMapPara.m_NewPid[j].m_PcrPid = FPGA_DATA_NULL;
					lPidMapPara.m_NewPid[j].m_VideoPid = FPGA_DATA_NULL;
					lPidMapPara.m_OldPid[j].m_AudioPid = FPGA_DATA_NULL;
					lPidMapPara.m_OldPid[j].m_PcrPid = FPGA_DATA_NULL;
					lPidMapPara.m_OldPid[j].m_VideoPid = FPGA_DATA_NULL;
				}
			}

			MFPGA_SetPidFilter(i, &lPidFilterPara);
			MFPGA_SetPidMap(i, &lPidMapPara);
		}
	}

	/* 设置码率 */
	MFPGA_SetOutBitrate(DEFAULT_OUT_BITRATE);

	/* PSI发表 */
	{
		PSI_CreateParam lPsiCreatePara;

		lPsiCreatePara.m_TsParam.m_TsId = pCfgParam->m_TsId;
		lPsiCreatePara.m_TsParam.m_OnId = pCfgParam->m_OnId;
		lPsiCreatePara.m_TsParam.m_Charset = pCfgParam->m_Charset;

		for (i=0; i<GN_ENC_CH_NUM; i++)
		{
			lPsiCreatePara.m_ProgParam[i].m_WorkEn = pCfgParam->m_ChParam[i].m_WorkEn;
			lPsiCreatePara.m_ProgParam[i].m_VidEncMode = pCfgParam->m_ChParam[i].m_VideoEncStandard;
			lPsiCreatePara.m_ProgParam[i].m_AudEncMode = pCfgParam->m_ChParam[i].m_AudioEncStandard;
			lPsiCreatePara.m_ProgParam[i].m_ServiceId = pCfgParam->m_ChParam[i].m_ServiceId;
			GLOBAL_STRCPY(lPsiCreatePara.m_ProgParam[i].m_pServiceName, pCfgParam->m_ChParam[i].m_pServiceName);
			lPsiCreatePara.m_ProgParam[i].m_VidPid = pCfgParam->m_ChParam[i].m_VideoPid;
			lPsiCreatePara.m_ProgParam[i].m_AudPid = pCfgParam->m_ChParam[i].m_AudioPid;
			lPsiCreatePara.m_ProgParam[i].m_PmtPid = pCfgParam->m_ChParam[i].m_PmtPid;
			lPsiCreatePara.m_ProgParam[i].m_PcrPid = pCfgParam->m_ChParam[i].m_PcrPid;
		}

		PSI_SetEncPsiParamToHw(&lPsiCreatePara);
	}

	return TRUE;
}

BOOL HWL_GetParaFromMainFpga(HWL_MfpgaStatusParam *pStatusParam)
{
	S32 i;

	for (i=0; i<GN_ENC_CH_NUM; i++)
	{
		if (MFPGA_GetInputVideoPacketNum(i) == 0)
		{
			pStatusParam->m_InputErrorFlag[i] = HWL_MFPGA_INPUT_ERROR_VIDEO;
		}
		else if (MFPGA_GetInputAudioPacketNum(i) == 0)
		{
			pStatusParam->m_InputErrorFlag[i] = HWL_MFPGA_INPUT_ERROR_AUDIO;
		}
		else if (MFPGA_GetInputCCErrorFlag() && (0x01 << (i / GN_CH_NUM_PER_ENC_BOARD))) /* 连续计数错误 */
		{
			pStatusParam->m_InputErrorFlag[i] = HWL_MFPGA_INPUT_ERROR_CC;
		}
		else
		{
			pStatusParam->m_InputErrorFlag[i] = HWL_MFPGA_INPUT_ERROR_NONE;
		}
	}

	if (MFPGA_GetOutValidBitrate() == 0)
	{
		pStatusParam->m_OutputErrorFlag = HWL_MFPGA_OUTPUT_ERROR_NOBITRATE;
	}
	else if (MFPGA_GetBufferOverflowStatus(DEFAULT_OUT_BITRATE) == TRUE)
	{
		pStatusParam->m_OutputErrorFlag = HWL_MFPGA_OUTPUT_ERROR_OVERFLOW;
	}
	else
	{
		pStatusParam->m_OutputErrorFlag = HWL_MFPGA_OUTPUT_ERROR_NONE;
	}

	return TRUE;
}



/* EOF */
