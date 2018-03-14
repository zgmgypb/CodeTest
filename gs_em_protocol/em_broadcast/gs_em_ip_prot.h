#ifndef GS_EM_PROTOCOL_H
#define GS_EM_PROTOCOL_H

#include "gs_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define	GS_EM_IP_PROT_MAX_LEN (65535) /* 应急数据包长度 */ 
#define GS_EM_IP_PROT_TAG (0xFEFD)
#define GS_EM_IP_PROT_VER (0x0100) /* 例：0x0100 表示 V01.00 */
#define GS_EM_IP_PROT_DATA_TYPE_REQUEST (1)
#define GS_EM_IP_PROT_DATA_TYPE_RESPONSE (2)

typedef enum {
	IP_PROT_CMD_START_BC = 0x01, /* 开始播发 */
	IP_PROT_CMD_STOP_BC = 0x02,

	IP_PROT_CMD_DEV_HEARTBEAT = 0x10,
	IP_PROT_CMD_STAT_QUERY = 0x11,
	IP_PROT_CMD_DEV_PARAM_SET = 0x12,
	IP_PROT_CMD_FAULT_RECOVERY = 0x13,
	IP_PROT_CMD_TASK_SWITCH = 0x14,
	IP_PROT_CMD_BC_REPONSE = 0x15, /* 终端上报播发结果 */

	IP_PROT_CMD_BC_RECORD_QUERY = 0x20,
	IP_PROT_CMD_AUT = 0x30, /* 身份认证 */
	IP_PROT_CMD_AUD_TRANS_START = 0x40, /* 音频播发需求接入开始 */ 
	IP_PROT_CMD_AUD_TRANS_END = 0x41,

	IP_PROT_CMD_BUTT
} GS_EM_IP_PROT_CMD_TYPE_E;

typedef enum {
	IP_PROT_BC_EM_DRILL_PUBLISH_SYS = 0x01, /* 应急演练-发布系统演练 */
	IP_PROT_BC_EM_DRILL_SIMU, /* 应急演练-模拟演练 */
	IP_PROT_BC_EM_DRILL_ACTUAl, /* 应急演练-实际演练 */
	IP_PROT_BC_EM, /* 应急广播 */
	IP_PROT_BC_NORM, /* 常态广播 */
	IP_PROT_BC_BUTT
} GS_EM_IP_PROT_BC_TYPE_E; /* 广播类型 */

typedef enum {
	IP_PROT_EM_LVL_I = 0x01,
	IP_PROT_EM_LVL_II,
	IP_PROT_EM_LVL_III,
	IP_PROT_EM_LVL_IV,
	IP_PROT_EM_LVL_BUTT
} GS_EM_IP_PROT_EM_LVL_E; /* 应急事件级别 */

typedef enum {
	IP_PROT_DEV_WORK_STAT_FREE = 0x01,
	IP_PROT_DEV_WORK_STAT_RUNNING,
	IP_PROT_DEV_WORK_STAT_FAULT,
	IP_PROT_DEV_WORK_STAT_BUTT
} GS_EM_IP_PROT_DEV_WORK_STAT_E;

typedef enum {
	IP_PROT_DEV_PARAM_TAG_VOL = 0x01,
	IP_PROT_DEV_PARAM_TAG_IPV4,
	IP_PROT_DEV_PARAM_TAG_RTP_POSTBACK_ADDR,
	IP_PROT_DEV_PARAM_TAG_DEV_RES_CODE,
	IP_PROT_DEV_PARAM_TAG_SERVER_TCP_ADDR, /* 服务器 TCP 地址 */
	IP_PROT_DEV_PARAM_TAG_BUTT
} GS_EM_IP_PROT_DEV_PARAM_TAG_E;

typedef enum {
	GS_EM_IP_PROT_AUD_TRANS_PROT_RTSP = 0x01,
	GS_EM_IP_PROT_AUD_TRANS_PROT_RTMP,
	GS_EM_IP_PROT_AUD_TRANS_PROT_RTP, 
	GS_EM_IP_PROT_AUD_TRANS_PROT_BUTT
} GS_EM_IP_PROT_AUD_TRANS_PROT_TYPE_E; /* 音频传输协议类型 */

typedef enum {
	GS_EM_IP_PROT_AUD_ENC_FMT_MP3 = 0x01,
	GS_EM_IP_PROT_AUD_ENC_FMT_MPEG1_L2, 
	GS_EM_IP_PROT_AUD_ENC_FMT_AAC,
	GS_EM_IP_PROT_AUD_ENC_FMT_BUTT
} GS_EM_IP_PROT_AUD_ENC_FMT_E; /* 音频编码类型 */

typedef enum {
	GS_EM_IP_PROT_ASSIST_DATA_TYPE_TEXT = 0x01,
	GS_EM_IP_PROT_ASSIST_DATA_TYPE_AUD, 
	GS_EM_IP_PROT_ASSIST_DATA_TYPE_PIC,
	GS_EM_IP_PROT_ASSIST_DATA_TYPE_VID,
	GS_EM_IP_PROT_ASSIST_DATA_TYPE_BUTT
} GS_EM_IP_PROT_ASSIST_DATA_TYPE_E; /* 辅助数据类型 */

typedef enum {
	GS_EM_IP_PROT_HOSTNAME_TYPE_IP = 0x01,
	GS_EM_IP_PROT_HOSTNAME_TYPE_DOMAIN, 
	GS_EM_IP_PROT_HOSTNAME_TYPE_BUTT
} GS_EM_IP_PROT_HOSTNAME_TYPE_E; 

typedef enum {
	IP_PROT_SUCCESS = 0,

	IP_PROT_ERR_UNRECOG_PROT = 10, /* 未知通信协议类型错误 */
	IP_PROT_ERR_TIMEOUT, /* 请求超时，对方无应答 */
	IP_PROT_ERR_VER_MISMATCH, /* 协议版本不匹配 */
	IP_PROT_ERR_DATA_PARSE, /* 数据包解析错误 */
	IP_PROT_ERR_LESS_PARAM, /* 缺少必选参数 */
	IP_PROT_ERR_CRC, /* CRC 校验错误 */

	IP_PROT_ERR_UNRECOG_SYS_TYPE = 30, /* 未知系统类别错误 */
	IP_PROT_ERR_SYS_BUSY, /* 系统忙 */
	IP_PROT_ERR_NO_MEM_CARD, /* 无存储卡 */
	IP_PROT_ERR_READ_FILE, /* 读文件失败 */
	IP_PROT_ERR_WRITE_FILE, /* 写文件失败 */
	IP_PROT_ERR_UKEY_NO_INSERT, /* UKey 未插入 */
	IP_PROT_ERR_UKEY_ILLEGAL, /* UKey 非法 */

	IP_PROT_ERR_UNRECOG_DATA_VALID = 50, /* 未知数据验证类别 */
	IP_PROT_ERR_USERNAME_PWD, /* 用户名或密码错误 */
	IP_PROT_ERR_CERT_ILLEGAL, /* 数字证书非法 */
	IP_PROT_ERR_INPUT_TIMEOUT, /* 输入超时 */
	IP_PROT_ERR_PRAM_ILLEGAL, /* 参数非法 */
	IP_PROT_ERR_FUNC_NOSUPPORT, /* 功能不支持 */
	IP_PROT_ERR_SMG_ILLEGAL, /* 短信格式非法 */
	IP_PROT_ERR_NUMBER_INVALID, /* 号码无效 */
	IP_PROT_ERR_CONTENT_ILLEGAL, /* 内容非法 */
	IP_PROT_ERR_REG_CODE_INVALID, /* 资源编码无效 */
	
	IP_PROT_ERR_NORECOG_DEV_TYPE = 70, /* 未知终端类型 */
	IP_PROT_ERR_DEV_INVALID, /* 终端无效 */
	IP_PROT_ERR_DEV_LINKDOWN, /* 设备离线 */
	IP_PROT_DEV_BUSY, /* 终端忙 */

	IP_PROT_ERR_BUTT
} GS_EM_IP_PORT_ERR_CODE_E;

typedef struct {
	GS_U8	m_ResTypeCode; /* 资源类型码 */  
	GS_U8	m_ResSubTypeCode; /* 资源子类型码 */  
	GS_U8	m_pZoneCode[6]; /* 地区编码 */
	GS_U8	m_ExCode; /* 扩展码 */
} GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S;

typedef struct {
	GS_U32	m_IpAddr;
	GS_U32	m_IpMask;
	GS_U32	m_IpGate;
} GS_EM_IP_PROT_IPV4_S;

typedef struct {
	GS_EM_IP_PROT_HOSTNAME_TYPE_E	m_Type; /* 0x01: IP + Port; 0x02: 域名 + 端口 */
	union {
		GS_U32	m_IpAddr;
		GS_CHAR	m_pDomain[64]; /* 域名最大长度限制为63个字符 */
	} m_HostName; /* 回传 IP 地址或域名 */
	GS_U16	m_Port;
} GS_EM_IP_PROT_NET_ADDR_S;

typedef struct {
	GS_EM_IP_PROT_AUD_TRANS_PROT_TYPE_E	m_AudTransProt;
	GS_EM_IP_PROT_AUD_ENC_FMT_E			m_AudEncFmt;
	GS_EM_IP_PROT_NET_ADDR_S		m_Addr;
} GS_EM_IP_PROT_ASSIST_AUD_DATA_S;
	
typedef struct {
	GS_EM_IP_PROT_NET_ADDR_S		m_Addr;
} GS_EM_IP_PROT_ASSIST_PIC_DATA_S;
	
typedef struct {
	GS_EM_IP_PROT_NET_ADDR_S		m_Addr;
} GS_EM_IP_PROT_ASSIST_VID_DATA_S;

typedef struct {
	GS_EM_IP_PROT_ASSIST_DATA_TYPE_E	m_Type;
	GS_U16								m_Len;
	union {
		GS_CHAR							*m_pText;
		GS_EM_IP_PROT_ASSIST_AUD_DATA_S m_AudInfo;
		GS_EM_IP_PROT_ASSIST_PIC_DATA_S m_PicInfo;
		GS_EM_IP_PROT_ASSIST_VID_DATA_S m_VidInfo;
	} m_Data;
} GS_EM_IP_PROT_ASSIST_DATA_S;

typedef struct {
	GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S	m_EmPlatformId;
	GS_U16								m_Year;
	GS_U8								m_Month;
	GS_U8								m_Day;
	GS_U16								m_SeqCode; /* 顺序码 */
} GS_EM_IP_PROT_EM_MSG_ID_S;

typedef struct {
	GS_EM_IP_PROT_DEV_PARAM_TAG_E		m_Tag;
	GS_U8								m_ParamLen;
	union {
		GS_U8							m_Vol;
		GS_EM_IP_PROT_IPV4_S			m_LocalIp;
		GS_EM_IP_PROT_NET_ADDR_S		m_PostBackAddrInfo; /* 回传地址信息 */
		GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S	m_DevResCode; /* 设备资源编码 */
		GS_EM_IP_PROT_NET_ADDR_S		m_ServerTcpAddr; /* 服务器 TCP 地址 */
	} m_Param;
} GS_EM_IP_PROT_DEV_PARAM_S;

typedef struct {
	GS_EM_IP_PROT_EM_MSG_ID_S				m_EmMsgId;

	GS_EM_IP_PROT_BC_TYPE_E					m_BCType; /* 广播类型 */
	GS_EM_IP_PROT_EM_LVL_E					m_EmEventLvl; /* 应急事件等级 */
	GS_U8									m_pEmEventType[5]; /* 应急事件类型 */
	GS_U8									m_Vol; /* 音量，0x00 静音；0xff 开播，音量不变；0x01 ~ 0x64 对应音量 1% ～ 100% */
	GS_U32									m_StartTime;
	GS_U32									m_DurationTime; 
	GS_U8									m_AssistDataNum; /* 1 - 4 */
	GS_EM_IP_PROT_ASSIST_DATA_S				m_pAssistData[4];
} GS_EM_IP_PROT_START_BC_S;

typedef struct {
	GS_EM_IP_PROT_EM_MSG_ID_S	m_EmMsgId;
} GS_EM_IP_PROT_STOP_BC_S;

typedef struct {
	GS_EM_IP_PROT_DEV_WORK_STAT_E	m_DevWorkStat;
	GS_U8							m_FirstRegister; /* 是否第一次注册，1 首次注册，2 非首次注册 */
	GS_U8							m_PhyAddrCodeLen; 
	GS_U8							*m_pPhyAddrCode; /* 终端唯一标识，出厂时生成，固定不变 */
} GS_EM_IP_PROT_DEV_HEARTBEAT_S;

typedef struct {
	GS_U8							m_ParamSetNum;
	GS_EM_IP_PROT_DEV_PARAM_S		*m_pParamData;
} GS_EM_IP_PROT_DEV_PARAM_SET_S;

typedef struct {
	GS_U32									m_CertID; /* 认证 ID */
	GS_U16									m_SpotsObjNum; /* 需要插播目标的数量 */
	GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S		*m_pSpotsObj;
	GS_EM_IP_PROT_BC_TYPE_E					m_BCType; /* 只有应急和常态两种选择 */
	GS_EM_IP_PROT_AUD_TRANS_PROT_TYPE_E		m_AudTransProt;
	GS_EM_IP_PROT_AUD_ENC_FMT_E				m_AudEncFmt;
	GS_U16									m_BCAddrLen;
	GS_EM_IP_PROT_NET_ADDR_S			m_BCAddr;
} GS_EM_IP_PROT_AUD_TRANS_START_S;

typedef struct {
	GS_U32									m_CertID; /* 认证 ID */
	GS_U16									m_SpotsObjNum; /* 需要结束插播目标的数量 */
	GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S		*m_pSpotsObj;
} GS_EM_IP_PROT_AUD_TRANS_END_S;

typedef struct {
	GS_EM_IP_PORT_ERR_CODE_E			m_RetCode; /* 结果代码 */
	GS_U16								m_DescLen; /* 描述内容长度 */
	GS_CHAR								*m_pDesc; /* 描述内容 */
} GS_EM_IP_PROT_RESPONSE_S;

typedef struct {
	GS_U16	m_Tag;
	GS_U16	m_Ver;
	GS_U32	m_SessionId;
	GS_U8	m_DataType;
	GS_U8	m_SignTag; /* 0:不签名 */
	GS_U16	m_DataLen; /* 包含校验数据和数据包头 */
} GS_EM_IP_PROT_HEADER_S; 

typedef struct {
	GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S	m_DataSrcCompBCDCode; /* 数据源压缩 BCD 编码 */

	GS_U16								m_DataDstNum; /* 数据目标对象 */
	GS_EM_IP_PROT_RES_PACKED_BCD_CODE_S	*m_pDataDstCompBCDCode; /* 数据接收端的资源编码 */

	GS_EM_IP_PROT_CMD_TYPE_E			m_CmdType; /* 业务数据类型 */
	GS_U16								m_CmdLen; /* 业务数据长度 */
	union {
		GS_EM_IP_PROT_START_BC_S		m_StartBCInfo; /* 开始播发 */
		GS_EM_IP_PROT_STOP_BC_S			m_StopBCInfo; /* 停止播发 */
		GS_EM_IP_PROT_DEV_HEARTBEAT_S		m_HeartBeatInfo; /* 终端心跳 */
		GS_EM_IP_PROT_DEV_PARAM_SET_S		m_ParamSetInfo; /* 终端参数设置 */
		GS_EM_IP_PROT_AUD_TRANS_START_S		m_AudTransStartInfo; /* 音频播发需求接入开始信息 */
		GS_EM_IP_PROT_AUD_TRANS_END_S		m_AudTransEndInfo; /* 音频播发需求接入结束信息 */

		GS_EM_IP_PROT_RESPONSE_S			m_ResponseInfo; /* 终端应答 */
	} m_CmdContent;
} GS_EM_IP_PROT_CONTENT_S; 

typedef struct {
	GS_U32	m_Crc32;
} GS_EM_IP_PROT_CHECKDATA_S; 

typedef struct {
	GS_EM_IP_PROT_HEADER_S		m_Header;
	GS_EM_IP_PROT_CONTENT_S		m_Content;
	GS_EM_IP_PROT_CHECKDATA_S	m_CheckData;
} GS_EM_IP_PROT_S; /* 应急广播 IP 数据包格式 */

/* Global Functions Declare */
GS_S32	GS_EM_IP_PROT_Construct(GS_U8 *pData, GS_S32 *pLen, GS_EM_IP_PROT_S *pIpProt); /* 根据 IP 协议包结构生成传送数据 */
GS_S32 GS_EM_IP_PROT_ResponseMsgPack(GS_U8 *pData, GS_S32 *pLen, GS_EM_IP_PROT_RESPONSE_S *pResponseInfo, GS_U32 SessionId, GS_U32 CmdType); /* 应答消息封包 */
GS_S32 GS_EM_IP_PROT_CmdInfoPack(GS_U8 *pData, GS_S32 *pLen, GS_VOID *pCmdInfo, GS_U8 CmdType, GS_U32 SessionId); /* 业务数据封包 */
GS_S32	GS_EM_IP_PROT_Parse(GS_U8 *pData, GS_S32 Len, GS_EM_IP_PROT_S *pIpProt, GS_HANDLE *pBufHandle); /* 传入数据，解析出包结构; pBufHandle 返回使用的内存块句柄 */
GS_VOID GS_EM_IP_PROT_ParseClean(GS_HANDLE BufHandle); /* 解析出的包结构使用完成后，需要进行内存清理 */
GS_VOID GS_EM_IP_PROT_Print(GS_EM_IP_PROT_S *pIpProt);
GS_VOID GS_EM_IP_PROT_ResponseInfoMake(GS_EM_IP_PROT_RESPONSE_S *pResponseInfo, GS_EM_IP_PROT_S *pIpProt, GS_U8 RetCode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* GS_EM_PROTOCOL_H */
/* EOF */
