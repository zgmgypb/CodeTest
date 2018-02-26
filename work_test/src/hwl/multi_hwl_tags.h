#ifndef _MULTI_HWL_TAGS_H_
#define _MULTI_HWL_TAGS_H_
/* Includes-------------------------------------------------------------------- */
/* Macro ---------------------------------------------------------------------- */
#define HWL_UNKNOW_TAG				0x00
#define HWL_HEART_BEAT_TAG			0x01
#define HWL_CHN_BITRATE_TAG			0x02
#define HWL_IN_ETH_SUB_PARAM_TAG	0x03
#define HWL_OUT_ETH_SUB_PARAM_TAG	0x04
#define HWL_PID_MAP_TAG				0x05
#define HWL_CW_SET_TAG				0x06
#define HWL_CW_SWITCH_TAG			0x07
#define HWL_STATISTICS_TAG			0x08

#define HWL_TS_PACKET_SEND_TAG		0x0A
#define HWL_ETH_CHN_PARAMETER_TAG	0x0B	
#define HWL_HW_INFO_TAG				0x0C	

#define HWL_PID_BLOCK_TAG			0x0E	

#define HWL_PID_REPLICATE_SRC_TAG	0x0F	
#define HWL_PID_REPLICATE_DES_TAG	0x10	

#define HWL_SUB_CHIP_OP_TAG			0x11	
#define HWL_SUB_CHIP_5862_TAG		0x1B	

#define HWL_PSISI_DETECT_TAG		0x1D	

#define HWL_IGMP_TAG				0x1E
#define HWL_SECRET_CHIP_IO_TAG		0x20	
#define HWL_DIRECT_ROUTE_MODE_TAG	0x21	
#define HWL_TS_INSERTER_TAG			0x23	
#define HWL_MODULE_RESET_TAG		0x24

#define HWL_FPGA_REMOTE_CONF_TAG	0x25
#define HWL_MAIN_INFO_TAG			0x26	

#define HWL_MULTI_DEVICE_TAG		0x45	

#define HWL_VOD_PORT_MAP_TAG		0x13
#define HWL_VOD_FILTER_PID_TAG		0x14
#define HWL_VOD_PID_MAP_TAG			0x15

/* Types Define (struct typedef)----------------------------------------------- */
typedef struct  
{
	U8		m_Tag;
	U8		m_MsgLen;
	U8		m_Flag1;
	U8		m_Flag2;

	U8		*m_pPayload;
	S32		m_PayloadSize;
}HWL_MsgHead;

/* Functions prototypes ------------------------------------------------------- */
#endif
/*EOF*/
