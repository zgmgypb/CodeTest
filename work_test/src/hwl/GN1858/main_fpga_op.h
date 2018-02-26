#ifndef MAIN_FPGA_OP_H
#define MAIN_FPGA_OP_H

#include "gn_drv.h"

enum
{
	VIXS_FLASH_CON_FPGA = 0, /* flash连接到FPGA */
	VIXS_FLASH_CON_PRO90, /* flash连接到Pro90芯片 */
	VIXS_FLASH_CON_DIS /* 禁止flash连接到其他设备 */
};

enum
{
	MFPGA_OUT_SRC_MUX = 0,
	MFPGA_OUT_SRC_SUBBOARD1_TS0,
	MFPGA_OUT_SRC_SUBBOARD1_TS1,
	MFPGA_OUT_SRC_SUBBOARD2_TS0,
	MFPGA_OUT_SRC_SUBBOARD2_TS1,
	MFPGA_OUT_SRC_SUBBOARD3_TS0,
	MFPGA_OUT_SRC_SUBBOARD3_TS1,
	MFPGA_OUT_SRC_SUBBOARD4_TS0,
	MFPGA_OUT_SRC_SUBBOARD4_TS1,
	MFPGA_OUT_SRC_INPUT_BOARD,
	MFPGA_OUT_SRC_MUX_MULTI_AUDIO,
	MFPGA_OUT_SRC_NUM
};

enum
{
	MFPGA_STATISTIC_VIDEO = 0,
	MFPGA_STATISTIC_AUDIO1,
	MFPGA_STATISTIC_AUDIO2
};

typedef struct
{
	U16 m_Prog0VideoPid;
	U16 m_Prog1VideoPid;
	U16 m_Prog0Audio1Pid;
	U16 m_Prog1Audio1Pid;
	U16 m_Prog0PcrPid;
	U16 m_Prog1PcrPid;
	U16 m_Prog0Audio2Pid;
	U16 m_Prog1Audio2Pid;
}MFPGA_PidArray;

typedef struct
{
	MFPGA_PidArray m_OldPidArray;
	MFPGA_PidArray m_NewPidArray;
}MFPGA_PidMapParam;

#define MFPGA_MAX_PSI_NUM 32
typedef struct  
{
	int m_OutTsMode; /* SPTS/MPTS */
	int m_Interval; /* 插入间隔ms */
	U8 m_Data[MFPGA_MAX_PSI_NUM][MPEG2_TS_PACKET_SIZE];
	U32 m_PsiPacketNum; /* 要插入包个数 */
}MFPGA_PsiInsertParam;	/* PSI插入参数 */

BOOL MFPGA_GetRelease(CHAR_T *pRelease);
U16 MFPGA_GetFpgaId(void);
U8 MFPGA_GetInputCCErrorFlag(void);
U32 MFPGA_GetSecretCardId(void);
U32 MFPGA_GetOutValidBitrate(void);
BOOL MFPGA_GetIsOverFlow(void);
BOOL MFPGA_SetCustomOverFlowPacketNum(U8 ChNum, U16 PacketNum);
BOOL MFPGA_SetIsOutNullPacket(BOOL IsOutNullPacket);
BOOL MFPGA_SetCbrStuff(U8 CbrStuffCtrl);
BOOL MFPGA_SetPcm1723(U32 ChNum, U8 lData);
BOOL MFPGA_SetOutBitrate(U32 Bitrate);
BOOL MFPGA_SetSendPcrPeriod(U8 Period);
BOOL MFPGA_SetEncBoardPowerUp(void);
BOOL MFPGA_SetVixsFlashConnect(U32 SubBoardIndex, U32 ConEn);
BOOL MFPGA_SetUartSelect(U32 SubBoardIndex);
BOOL MFPGA_SetEncBoardReset(U32 SubBoardIndex);
BOOL MFPGA_SetOutSrcSel(U32 OutSrc);
U8 MFPGA_GetBitrateStatistics(U32 InputChNum, U32 StatisticType);
BOOL MFPGA_SetPidMap(U32 InputChNum, MFPGA_PidMapParam *pPidMapPara);
BOOL MFPGA_SetPsiInsert(MFPGA_PsiInsertParam *pPsiInsertPara);
BOOL MFPGA_Init(void);
BOOL MFPGA_Terminate(void);
BOOL MFPGA_ConfigRbf(void);

#endif /* MAIN_FPGA_OP_H */
/* EOF */
