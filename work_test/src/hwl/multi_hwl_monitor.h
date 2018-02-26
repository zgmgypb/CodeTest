#ifndef _MULTI_HWL_MONITOR_H_
#define _MULTI_HWL_MONITOR_H_
/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
#include "multi_hwl.h"
/* Macro ---------------------------------------------------------------------- */
#define HWL_PID_STATISTICS_MAX_NUM			250
#define HWL_IP_STATISTICS_MAX_NUM			250
/* Types Define (struct typedef)----------------------------------------------- */
typedef struct  
{
	U16	m_PID;
	U32	m_Bitrate;//bps
}HWL_MonitorPIDStatNode;

typedef struct  
{
	HWL_MonitorPIDStatNode		m_pNode[HWL_PID_STATISTICS_MAX_NUM];
	S32							m_Number;
	S32							m_Bitrate;
}HWL_MonitorPIDStatArray;


typedef struct  
{
	U32 m_IPAddr;
	U16 m_Port;
	U8	m_Protocol;
	S32	m_Bitrate;
}HWL_MonitorIPStatNode;

typedef struct  
{
	HWL_MonitorIPStatNode	m_pNode[HWL_IP_STATISTICS_MAX_NUM];
	S32				m_Number;
	S32				m_Bitrate;//Total bps
}HWL_MonitorIPStatArray;

/* Functions prototypes ------------------------------------------------------- */

/*模块初始化函数！*/
void HWL_MonitorInitate(void);
/*轮询函数，预计1S轮询一次*/
void HWL_MonitorAccess(S32 Duration);
/*返回信息获取函数，返回TRUE时表示该数据已经被处理！*/
BOOL HWL_MonitorParser(U8 *pData, S32 DataLen);

/*PID码率查询配置*/
void HWL_MonitorPIDStatisticConfig(S16 ChnIndex, S16 SubIndex, BOOL bInput);
/*PID码率数据申请*/
void HWL_MonitorPIDStatisticResultReq(void);
/*清空PID码率信息*/
void HWL_MonitorPIDStatisticResultClean(void);
/*获取PID码率信息*/
void HWL_MonitorPIDStatisticResultGet(HWL_MonitorPIDStatArray *pArray);


/*设置需要统计的输入端口号，发出该命令的同时以前的统计结果会被自动清空*/
void HWL_MonitorIPStatisticConfig(S16 ChnIndex, BOOL bInput);
/*查询数据申请*/
void HWL_MonitorIPStatisticResultReq(void);
/*清空IP码率信息*/
void HWL_MonitorIPStatisticResultClean(void);
/*获取IP码率信息*/
void HWL_MonitorIPStatisticResultGet(HWL_MonitorIPStatArray *pArray);

void HWL_MonitorHeartBeateSend(void);

/*获取心跳错误*/
BOOL HWL_MonitorGetHeartBeatError(void);
/*硬件信息获取接口*/
void HWL_MonitorHWInfoGet(HWL_HWInfo *pHWInfo);
/*获取以太网的格式*/
S32 HWL_MonitorHWInfoETHChnNum(HWL_HWInfo *pHWInfo);
/*获取通道/子通道码率信息，单位bps；ChnIndex == -1时为当前子板的总码率，SubIndex == -1时为当前通道总码率*/
U32 HWL_MonitorChnBitrateInfoGet(S16 ChnIndex, S16 SubIndex, BOOL bInput);

/*ISR统计*/
void HWL_MonitorPlusInserterPacketNum(S32 Num);
/*获取插入器码率*/
U32 HWL_MonitorInserterBitrateGet(void);

/*内部通讯统计*/
void HWL_MonitorPlusICPByteNum(S32 ByteNum, BOOL bFPGAToCPU);
/*获取内部通讯码率*/
S32 HWL_MonitorInternalCOMBitrateGet(BOOL bFPGAToCPU);

/*获取以太网连接状态，旧版本！*/
BOOL HWL_MonitorGetETHLinkStatus(S32 Slot);
/*获取DDR错误状态*/
U32 HWL_MonitorGetMainFPGAModuleStatus(void);
/*模块复位状态清空*/
void HWL_MonitorModuleResetSend(U32 ModuleResetMark);










void HWL_DVBMuxRouteApply(S32 InCHN, S32 MaxSubNum);
void HWL_DVBMuxRouteSet(S32 InTsInd, S32 OutTsInd, BOOL bRoute, BOOL RouteWithNullPacket, BOOL EnablePCRCorrection);

#endif
/*EOF*/
