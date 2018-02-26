#include <stdio.h>
#include <unistd.h>
#include <global_micros.h>
#include <pthread.h>
#include "main_fpga_op.h"

/* 主板FPGA通信的宏定义 
直接寻址1		0200H~02FFH	为加密芯片保留
直接寻址2		0300H~03FFH	新定义的直接寻址
一级寻址		0400H~04FFH	需先直接寻址04FFH后，才能使用该寻址范围
*/
/* 以下是直接寻址 */
#define FPGA_VERSION_INFO_H	0x300
#define FPGA_VERSION_INFO_L		0x301
#define FPGA_ID_H						0x302
#define FPGA_ID_L						0x303
#define FPGA_INPUT_CC_ERR_FLAG		0x305
#define FPGA_SECRET_CARD_ID_1		0x308
#define FPGA_SECRET_CARD_ID_2		0x309
#define FPGA_SECRET_CARD_ID_3		0x30A
#define FPGA_SECRET_CARD_ID_4		0x30B
#define FPGA_OUT_VALID_BITRATE_L		0x30C
#define FPGA_OUT_VALID_BITRATE_H	0x30D
#define	FPGA_TS_OVER_FLOW_STATUS	0x030E	
#define FPGA_OVER_FLOW_SET_CH		0x30F
#define FPGA_OVER_FLOW_SET_PACKET_L		0x310
#define FPGA_OVER_FLOW_SET_PACKET_H		0x311
#define FPGA_IS_INSERT_NULL_PACKET			0x314
#define FPGA_CBR_STUFF_CTRL				0x315
#define FPGA_PCM1723_CTRL_0				0x032E
#define FPGA_PCM1723_CTRL_1				0x032F
#define FPGA_OUT_BITRATE_H				0x330
#define FPGA_OUT_BITRATE_L				0x331
#define FPGA_SEND_PCR_PERIOD			0x332
#define FPGA_VIXS_POWER_UP_CTRL		0x334
#define FPGA_VIXS_UART_SEL_1			0x335
#define FPGA_VIXS_UART_SEL_2			0x336
#define FPGA_VIXS_NET_RESET				0x337
#define FPGA_VIXS_RESET					0x338
#define FPGA_VIXS_FLASH_PORT_CTRL_1	0x340 /* 低电平使能 */
#define FPGA_VIXS_FLASH_PORT_CTRL_2	0x341 
#define FPGA_VIXS_FLASH_PORT_CTRL_3	0x342 
#define FPGA_VIXS_FLASH_PORT_CTRL_4	0x343 
#define FPGA_OUT_TS_SEL					0x350
#define FPGA_VIXS_SUB_CH1_VIDEO_COUNT		0x360
#define FPGA_VIXS_SUB_CH1_AUDIO1_COUNT	0x361
#define FPGA_VIXS_SUB_CH1_AUDIO2_COUNT	0x362
#define VIXS_SUB_ADDR_OFFSET					0x10
/* 以下是间接寻址 */
#define FPGA_MODE_SEL			0x4FF
#define FPGA_SEL_PSI_INSERT	0x005
#define FPGA_SEL_PID_MAP_1	0x080
#define FPGA_SEL_PID_MAP_2	0x081
#define FPGA_SEL_PID_MAP_3	0x082
#define FPGA_SEL_PID_MAP_4	0x083
#define FPGA_SEL_PID_MAP_5	0x084
#define FPGA_SEL_PID_MAP_6	0x085
#define FPGA_SEL_PID_MAP_7	0x086
#define FPGA_SEL_PID_MAP_8	0x087
#define FPGA_MAP_OLD_PROG0_VIDEO_PID_L				0x0400
#define FPGA_MAP_OLD_PROG0_VIDEO_PID_H				0x0401
#define FPGA_MAP_OLD_PROG1_VIDEO_PID_L				0x0402
#define FPGA_MAP_OLD_PROG1_VIDEO_PID_H				0x0403
#define FPGA_MAP_OLD_PROG0_AUDIO1_PID_L				0x0404
#define FPGA_MAP_OLD_PROG0_AUDIO1_PID_H				0x0405
#define FPGA_MAP_OLD_PROG1_AUDIO1_PID_L				0x0406
#define FPGA_MAP_OLD_PROG1_AUDIO1_PID_H				0x0407
#define FPGA_MAP_OLD_PROG0_PCR_PID_L				0x0408
#define FPGA_MAP_OLD_PROG0_PCR_PID_H				0x0409
#define FPGA_MAP_OLD_PROG1_PCR_PID_L				0x040A
#define FPGA_MAP_OLD_PROG1_PCR_PID_H				0x040B
#define FPGA_MAP_OLD_PROG0_AUDIO2_PID_L			0x040C
#define FPGA_MAP_OLD_PROG0_AUDIO2_PID_H			0x040D
#define FPGA_MAP_OLD_PROG1_AUDIO2_PID_L			0x040E
#define FPGA_MAP_OLD_PROG1_AUDIO2_PID_H			0x040F
#define FPGA_MAP_NEW_PROG0_VIDEO_PID_L				0x0480
#define FPGA_MAP_NEW_PROG0_VIDEO_PID_H				0x0481
#define FPGA_MAP_NEW_PROG1_VIDEO_PID_L				0x0482
#define FPGA_MAP_NEW_PROG1_VIDEO_PID_H				0x0483
#define FPGA_MAP_NEW_PROG0_AUDIO1_PID_L				0x0484
#define FPGA_MAP_NEW_PROG0_AUDIO1_PID_H			0x0485
#define FPGA_MAP_NEW_PROG1_AUDIO1_PID_L				0x0486
#define FPGA_MAP_NEW_PROG1_AUDIO1_PID_H			0x0487
#define FPGA_MAP_NEW_PROG0_PCR_PID_L				0x0488
#define FPGA_MAP_NEW_PROG0_PCR_PID_H				0x0489
#define FPGA_MAP_NEW_PROG1_PCR_PID_L				0x048A
#define FPGA_MAP_NEW_PROG1_PCR_PID_H				0x048B
#define FPGA_MAP_NEW_PROG0_AUDIO2_PID_L			0x048C
#define FPGA_MAP_NEW_PROG0_AUDIO2_PID_H		0x048D
#define FPGA_MAP_NEW_PROG1_AUDIO2_PID_L			0x048E
#define FPGA_MAP_NEW_PROG1_AUDIO2_PID_H		0x048F
#define FPGA_PSI_INSERT_INTERVAL							0x04C0
#define FPGA_PSI_INSERT_DRAM_PACKET_COUNTER	0x04C1
#define FPGA_PSI_INSERT_DRAM_STATUS					0x04C2
#define FPGA_PSI_INSERT_DRAM_POINTER				0x04C3
#define FPGA_PSI_INSERT_DATA_ADDR						0x04C4
#define FPGA_PSI_INSERT_DATA_VALUE					0x04C5
#define FPGA_PSI_INSERT_DRAM_WRITE_TRIG			0x04C6
#define FPGA_PSI_INSERT_CRAM_POINTER			0x04D0
#define FPGA_PSI_INSERT_PID_L							0x04D1
#define FPGA_PSI_INSERT_PID_H							0x04D2
#define FPGA_PSI_INSERT_TS_PACKET_NUM			0x04D3
#define FPGA_PSI_INSERT_CRAM_WRITE_TRIG		0x04D4
#define FPGA_PSI_INSERT_CRAM_STATUS				0x04D5
#define FPGA_PSI_INSERT_TS_OUT_MODE				0x04E0

static pthread_mutex_t main_fpga_mutex;

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

BOOL MFPGA_GetRelease(CHAR_T *pRelease)//获取FPGA主版本
{
	U8 lFpgaRelease[2];
	U32 lTmpVal;

	if (!pRelease)
		return FALSE;

	lFpgaRelease[0] = READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VERSION_INFO_H);
	lFpgaRelease[1] = READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VERSION_INFO_L);
	lTmpVal = (lFpgaRelease[0] << 8) | lFpgaRelease[1];
	GLOBAL_SPRINTF((pRelease, "%.4d.%.2d.%.2d", ((lTmpVal >> 9) & 0x7F) + 2009, (lTmpVal >> 5) & 0x0F, lTmpVal & 0x1F));

	return TRUE;
}

U16 MFPGA_GetFpgaId(void)
{
	U8 lId[2];

	lId[0] = READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_ID_H);
	lId[1] = READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_ID_L);

	return (U16)((lId[0] << 8) | lId[1]);
}

/* 返回8位数据，每位表示一个输入通道的状态 */
U8 MFPGA_GetInputCCErrorFlag(void)
{
	return READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_INPUT_CC_ERR_FLAG);
}

/* 返回加密卡ID */
U32 MFPGA_GetSecretCardId(void)
{
	U8 lData[4];
	U32 i;

	for (i=0; i<4; i++)
	{
		lData[i] = READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_SECRET_CARD_ID_1 + i);
	}

	return (U32)(((lData[3]<<24) & 0xFF000000)  |  ((lData[2]<<16) &0xFF0000) | ((lData[1]<<8) &0xFF00) | (lData[0] & 0xFF));
}

/* 返回主板到输出板的实时有效数据速率, 返回单位kbps */
U32 MFPGA_GetOutValidBitrate(void)
{
	U8 lData[2];
	U32 lBitrate;

	lData[0] = READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_OUT_VALID_BITRATE_L);
	lData[1] = READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_OUT_VALID_BITRATE_H);

	lBitrate = ((lData[1]<<8)& 0xFF00)| (lData[0] & 0xFF);
	lBitrate = (lBitrate * 188 * 8) / 1024;

	return lBitrate;
}

/* 返回是否溢出，返回TRUE表示溢出，否则表示不溢出 */
BOOL MFPGA_GetIsOverFlow(void)
{
	if (READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_TS_OVER_FLOW_STATUS) == 0)
		return FALSE;
	else
		return TRUE;
}

/* 选择自定义存储块, 设置自定义溢出包个数 */
BOOL MFPGA_SetCustomOverFlowPacketNum(U8 ChNum, U16 PacketNum)
{
	pthread_mutex_lock(&main_fpga_mutex);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_OVER_FLOW_SET_CH, ChNum);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_OVER_FLOW_SET_PACKET_L, PacketNum & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_OVER_FLOW_SET_PACKET_H, (PacketNum >> 8) & 0xFF);
	pthread_mutex_unlock(&main_fpga_mutex);

	return TRUE;
}

/* 设置是否输出空包 */
BOOL MFPGA_SetIsOutNullPacket(BOOL IsOutNullPacket)
{
	if (IsOutNullPacket)
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_IS_INSERT_NULL_PACKET, 0x00); /* 插入空包 */
	else
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_IS_INSERT_NULL_PACKET, 0x01); /* 不插入空包 */

	return TRUE;
}

/* 传入的参数按位表示插入空包填充为视频包 */
BOOL MFPGA_SetCbrStuff(U8 CbrStuffCtrl)
{
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_CBR_STUFF_CTRL, CbrStuffCtrl);

	return TRUE;
}

BOOL MFPGA_SetPcm1723(U32 ChNum, U8 lData)
{
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PCM1723_CTRL_0 + ChNum, lData);

	return TRUE;
}

BOOL MFPGA_SetOutBitrate(U32 Bitrate)
{
	U32 lData = Bitrate / 8;

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_OUT_BITRATE_H, (lData >> 8) & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_OUT_BITRATE_L, lData & 0xFF);

	return TRUE;
}

BOOL MFPGA_SetSendPcrPeriod(U8 Period)
{
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_SEND_PCR_PERIOD, Period);

	return TRUE;
}

BOOL MFPGA_SetEncBoardPowerUp(void)
{
	pthread_mutex_lock(&main_fpga_mutex);

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_RESET, 0x00);	/* 复位 */
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_NET_RESET, 0x00); /* 编码板以太网复位，硬件已经去掉 */
	sleep(1);

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_POWER_UP_CTRL, 0x01) ; /* open 3.3v */
	usleep(50000);	

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_POWER_UP_CTRL, 0x03) ; /* open 1v */
	usleep(500000);	

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_NET_RESET, 0x01);

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_RESET, 0x0F);	

	pthread_mutex_unlock(&main_fpga_mutex);

	return TRUE;
}

BOOL MFPGA_SetUartSelect(U32 SubBoardIndex)
{
	switch (SubBoardIndex)
	{
	case VIXS_INDEX_1:
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_UART_SEL_1, 0x01);
		break;
	case VIXS_INDEX_2:
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_UART_SEL_1, 0x02);
		break;
	case VIXS_INDEX_3:
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_UART_SEL_2, 0x03);
		break;
	case VIXS_INDEX_4:
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_UART_SEL_2, 0x04);
		break;
	default:
		break;
	}

	return TRUE;
}

BOOL MFPGA_SetEncBoardReset(U32 SubBoardIndex)
{
	pthread_mutex_lock(&main_fpga_mutex);

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_RESET, ~(0x0001 << SubBoardIndex));	
	usleep(200000);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_RESET, 0x000f);
	sleep(1);

	pthread_mutex_unlock(&main_fpga_mutex);

	return TRUE;
}

BOOL MFPGA_SetVixsFlashConnect(U32 SubBoardIndex, U32 ConEn)
{
	switch(ConEn)
	{
	case VIXS_FLASH_CON_FPGA:
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_FLASH_PORT_CTRL_1 + SubBoardIndex, 0x01);	
		break;
	case VIXS_FLASH_CON_PRO90:
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_FLASH_PORT_CTRL_1 + SubBoardIndex, 0x02);
		break;
	case VIXS_FLASH_CON_DIS:
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_FLASH_PORT_CTRL_1 + SubBoardIndex, 0x03);
		break;
	default:
		break;
	}

	return TRUE;
}

BOOL MFPGA_SetOutSrcSel(U32 OutSrc)
{
	U8 lData = 0;

	switch (OutSrc)
	{
	case MFPGA_OUT_SRC_MUX:
		lData = 0;
		break;
	case MFPGA_OUT_SRC_SUBBOARD1_TS0:
		lData = 1;
		break;
	case MFPGA_OUT_SRC_SUBBOARD1_TS1:
		lData = 2;
		break;
	case MFPGA_OUT_SRC_SUBBOARD2_TS0:
		lData = 3;
		break;
	case MFPGA_OUT_SRC_SUBBOARD2_TS1:
		lData = 4;
		break;
	case MFPGA_OUT_SRC_SUBBOARD3_TS0:
		lData = 5;
		break;
	case MFPGA_OUT_SRC_SUBBOARD3_TS1:
		lData = 6;
		break;
	case MFPGA_OUT_SRC_SUBBOARD4_TS0:
		lData = 7;
		break;
	case MFPGA_OUT_SRC_SUBBOARD4_TS1:
		lData = 8;
		break;
	case MFPGA_OUT_SRC_INPUT_BOARD:
		lData = 9;
		break;
	case MFPGA_OUT_SRC_MUX_MULTI_AUDIO:
		lData = 0xA;
		break;
	default:
		break;
	}
	
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_OUT_TS_SEL, lData);

	return TRUE;
}

U8 MFPGA_GetBitrateStatistics(U32 InputChNum, U32 StatisticType)
{
	switch (StatisticType)
	{
	case MFPGA_STATISTIC_VIDEO:
		return READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_SUB_CH1_VIDEO_COUNT + 0x10 * InputChNum) & 0xFF;
	case MFPGA_STATISTIC_AUDIO1:
		return READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_SUB_CH1_AUDIO1_COUNT + 0x10 * InputChNum) & 0xFF;
	case MFPGA_STATISTIC_AUDIO2:
		return READ_FPGA(GN_FPGA_INDEX_MAIN, FPGA_VIXS_SUB_CH1_AUDIO2_COUNT + 0x10 * InputChNum) & 0xFF;
	default:
		return 0;
	}
}

BOOL MFPGA_SetPidMap(U32 InputChNum, MFPGA_PidMapParam *pPidMapPara)
{
	pthread_mutex_lock(&main_fpga_mutex);

	/* 选择模式 */
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MODE_SEL, FPGA_SEL_PID_MAP_1 + InputChNum);
	
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG0_VIDEO_PID_L, pPidMapPara->m_OldPidArray.m_Prog0VideoPid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG0_VIDEO_PID_H, (pPidMapPara->m_OldPidArray.m_Prog0VideoPid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG1_VIDEO_PID_L, pPidMapPara->m_OldPidArray.m_Prog1VideoPid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG1_VIDEO_PID_H, (pPidMapPara->m_OldPidArray.m_Prog1VideoPid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG0_VIDEO_PID_L, pPidMapPara->m_NewPidArray.m_Prog0VideoPid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG0_VIDEO_PID_H, (pPidMapPara->m_NewPidArray.m_Prog0VideoPid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG1_VIDEO_PID_L, pPidMapPara->m_NewPidArray.m_Prog1VideoPid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG1_VIDEO_PID_H, (pPidMapPara->m_NewPidArray.m_Prog1VideoPid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG0_AUDIO1_PID_L, pPidMapPara->m_OldPidArray.m_Prog0Audio1Pid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG0_AUDIO1_PID_H, (pPidMapPara->m_OldPidArray.m_Prog0Audio1Pid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG1_AUDIO1_PID_L, pPidMapPara->m_OldPidArray.m_Prog1Audio1Pid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG1_AUDIO1_PID_H, (pPidMapPara->m_OldPidArray.m_Prog1Audio1Pid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG0_AUDIO1_PID_L, pPidMapPara->m_NewPidArray.m_Prog0Audio1Pid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG0_AUDIO1_PID_H, (pPidMapPara->m_NewPidArray.m_Prog0Audio1Pid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG1_AUDIO1_PID_L, pPidMapPara->m_NewPidArray.m_Prog1Audio1Pid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG1_AUDIO1_PID_H, (pPidMapPara->m_NewPidArray.m_Prog1Audio1Pid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG0_AUDIO2_PID_L, pPidMapPara->m_OldPidArray.m_Prog0Audio2Pid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG0_AUDIO2_PID_H, (pPidMapPara->m_OldPidArray.m_Prog0Audio2Pid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG1_AUDIO2_PID_L, pPidMapPara->m_OldPidArray.m_Prog1Audio2Pid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG1_AUDIO2_PID_H, (pPidMapPara->m_OldPidArray.m_Prog1Audio2Pid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG0_AUDIO2_PID_L, pPidMapPara->m_NewPidArray.m_Prog0Audio2Pid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG0_AUDIO2_PID_H, (pPidMapPara->m_NewPidArray.m_Prog0Audio2Pid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG1_AUDIO2_PID_L, pPidMapPara->m_NewPidArray.m_Prog1Audio2Pid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG1_AUDIO2_PID_H, (pPidMapPara->m_NewPidArray.m_Prog1Audio2Pid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG0_PCR_PID_L, pPidMapPara->m_OldPidArray.m_Prog0PcrPid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG0_PCR_PID_H, (pPidMapPara->m_OldPidArray.m_Prog0PcrPid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG1_PCR_PID_L, pPidMapPara->m_OldPidArray.m_Prog1PcrPid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_OLD_PROG1_PCR_PID_H, (pPidMapPara->m_OldPidArray.m_Prog1PcrPid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG0_PCR_PID_L, pPidMapPara->m_NewPidArray.m_Prog0PcrPid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG0_PCR_PID_H, (pPidMapPara->m_NewPidArray.m_Prog0PcrPid >> 8) & 0x1F);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG1_PCR_PID_L, pPidMapPara->m_NewPidArray.m_Prog1PcrPid & 0xFF);
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MAP_NEW_PROG1_PCR_PID_H, (pPidMapPara->m_NewPidArray.m_Prog1PcrPid >> 8) & 0x1F);
	
	/* 释放模式 */
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MODE_SEL, 0x00);

	pthread_mutex_unlock(&main_fpga_mutex);

	return TRUE;
}

BOOL MFPGA_SetPsiInsert(MFPGA_PsiInsertParam *pPsiInsertPara)
{
	S32 i, j;

	pthread_mutex_lock(&main_fpga_mutex);

	/* 选择模式 */
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MODE_SEL, FPGA_SEL_PSI_INSERT);

	if (pPsiInsertPara->m_OutTsMode == SPTS)
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_TS_OUT_MODE, 0xFF);	
	else
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_TS_OUT_MODE, 0x00);

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_DRAM_STATUS, 0x00);					
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_CRAM_STATUS, 0x00);					
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_INTERVAL, pPsiInsertPara->m_Interval / 2);								
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_DRAM_PACKET_COUNTER, pPsiInsertPara->m_PsiPacketNum);

	for(i = 0; i < pPsiInsertPara->m_PsiPacketNum; i++)
	{
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_DRAM_POINTER, i);

		for(j = 0; j < MPEG2_TS_PACKET_SIZE; j++)
		{
			WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_DATA_ADDR, j);
			WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_DATA_VALUE, pPsiInsertPara->m_Data[i][j]);
			WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_DRAM_WRITE_TRIG, 0x00);
		}
	}

	for (i = 0; i < pPsiInsertPara->m_PsiPacketNum; i++)
	{
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_CRAM_POINTER, i);
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_PID_L, pPsiInsertPara->m_Data[i][2] & 0xFF);					
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_PID_H, pPsiInsertPara->m_Data[i][1] & 0x1F);		
		WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_TS_PACKET_NUM, 0x00);					
		WRITE_FPGA(GN_FPGA_INDEX_MAIN,  FPGA_PSI_INSERT_CRAM_WRITE_TRIG, 0x00);							
	}

	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_DRAM_STATUS, 0xFF);					
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_PSI_INSERT_CRAM_STATUS, 0xFF);		
	/* 释放模式 */
	WRITE_FPGA(GN_FPGA_INDEX_MAIN, FPGA_MODE_SEL, 0x00);

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

/* EOF */
