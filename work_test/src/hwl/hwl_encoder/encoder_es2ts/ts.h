/******************************************************************************

                  版权所有 (C), 2005-2017, GOSPELL有限公司

 ******************************************************************************
  文 件 名   : ts.h
  版 本 号   : 初稿
  作    者   : 许斌
  生成日期   : 2017年7月14日
  最近修改   :
  功能描述   : ts.c 的头文件
  函数列表   :
  修改历史   :

   1.日    期          : 2017年7月14日
      作    者          : 许斌
      修改内容   : 创建文件

******************************************************************************/
#ifndef __TS_H__
#define __TS_H__

#include <stdio.h>
#include <stdlib.h>		/* calloc realloc abs div*/
#include <string.h>		/* memcpy memcmp */
#include <stdint.h>		/* uint8_t ... */
#include <stdbool.h>	/* bool true false */
#include <ctype.h>		/* toupper isxdigit isspace */
#include <math.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>		/* time_t clock_t tm */

#include "ts_psi.h"
#include "mpi_sys.h"
#include "os_assist.h"

#define MPEG_TS_DEBUG_ENABLE
#define MSG_ERR			0 /*Used to report error conditions */
#define MSG_DEBUG	1 /*Used for debugging messages */
#define MSG_INFO	2 /*Used for information messages */


#ifdef MPEG_TS_DEBUG_ENABLE
	extern int currentDebugLevel;

	#define DBG(level,...) \
	do \
	{ \
		if (level <= currentDebugLevel) \
		{ \
			printf(__VA_ARGS__); \
		}\
	} while (0)
	
#else
	#define DBG(level,fmt,...)	;
#endif

#define TS_PACKET_SIZE 							188
#define PAT_RETRANS_TIME 						40

#define PES_START_CODE_PREFIX     		0X000001
#define PES_AUDIO_STREAM_ID     			0XC0
#define PES_VIDEO_STREAM_ID    	 			0XE0
#define PES_AC3_STREAM_ID     			0XBD

#define PES_INCLUDE_ONLY_PTS            	2
#define PES_INCLUDE_PTS_DTS              	3
#define PES_NOT_INCLUDE_PTS_DTS      	0

/* table ids */
#define PAT_TID   			0x00
#define PMT_TID   			176
#define SDT_TID   			0x42

#define PROGRAM_MAP_PID   49

#define TS_AUDIO_PID  			144
#define TS_VIDEO_PID  				128
#define TS_PCR_PID  160

#define  DEFAULT_PTS_DELAY_TIME            (500000) //500ms
#define  PTS_PCR_MAX_INTERVAL_1s			(50000) //50ms
#define  PTS_PCR_MIN_INTERVAL_1s				(-50000)//-50ms

#define  PTS_PCR_MAX_OVERTURN 			   (60000000) //9000ms
#define  PTS_PCR_MIN_OVERTURN					(-60000000)//-9000ms

#define PSI_TX_INTERVAL_NUM    10
#define AVERAGE_CALC_NUMBER  5

#ifndef LONGLONG
#define LONGLONG long long
#endif

typedef int int32_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;

/* PES Header (Fixed Length  = 9 bytes)*/
typedef struct TS_PES
{
    uint32_t  packet_start_code_prefix;         /* Bit 24, 开始码字为0X00 00 01 */
    uint16_t  stream_id;                                 /*Bit 8, 原始流的类型和数目，取值从1011 1100到1111 1111之间*/
    uint16_t  PES_packet_length;                   /*Bit 16, 表示从此字节之后PES包长（单位字节）。*/
	                                                               /* 0表示PES包长不限制，且只能用于视频PES*/
    uint32_t  reserved;								          /* Bit 2 fixed '10' */
																																		
    uint32_t  PES_scrambling_control;            /* Bit 2, PES有效负载的加密模式。*/
                                                                  /* 00表示不加密，其余表示用户自定义*/
																																		 
	uint32_t  PES_priority;                              /* Bit 1, PES数据包的优先级。类似于TS的此字段。*/
																																		 
	uint32_t  data_alignment_indicator;           /* Bit 1,  为1时，表明此分组头部之后*/
	                                                               /* 紧跟着 数据流描述子中定义的访问单元类型。*/
																																	 
	uint32_t  copyright;                                  /*  Bit 1, 版权，1表示有版权，*/
	                                                              /* 具体见版权描述子13818-1 1-2-6-24。0表示没有。*/
																																	 
	uint32_t  original_or_copy;                        /* Bit 1, 表示原始数据，0表示备份*/
																																	 
	uint32_t  PTS_DTS_flag;								  /*  Bit 2, 10 表示含有PTS字段，*/
	                                                              /* 11 表示含有PTS和DTS字段，*/
	                                                              /* 00 表示不含有PTS和DTS，01无定义。*/
	uint32_t  ESCR_flag;                             /* Bit 1, 1 表示ESCR在PES首部出现，0表示不出现*/
																																	 
	uint32_t  ES_rate_flag;                          /* Bit 1, 1 表示PES分组含有ES_rate字段。0表示不含有。*/

	uint32_t DSM_trick_mode_flag;              /* Bit 1, 1 表示有8位的trick_mode_flag字段，*/
	                                                              /* 0  表示不出现此字段。只对DSM有效。在广播中不用*/

	uint32_t additional_copy_info_flag;        /* Bit 1, 1 表示有copy_info_flag字段，0 表示不出现此字段。*/

   uint32_t PES_CRC_flag;                         /* Bit 1, 1 表示PES分组中有CRC字段，0 表示不出现此字段。*/

   uint32_t PES_extension_flag;                 /* Bit 1, 1 表示扩展字段在PES包头存在，0 表示扩展字段不存在*/

	uint8_t PES_header_data_length;             /* Bit 8, 表示可选字段和填充字段所占的字节数*/
	
}TS_PES_T;


/* TS Header (Fixed length 4 bytes)  */
typedef struct TS_TS_HEADER
{
    uint32_t    sync_byte;     								/*  Bit 8, 同步头,  其值固定为0x47*/
	 uint32_t   transport_error_indicator; 			/* Bit 1 */
	 
	 uint32_t   payload_unit_start_indicator; 		/*Bit 1, 1 表示负载PES或PSI开始传送第一个字节*/
	 
	 uint32_t   transport_priority;   						/*Bit 1*/
	 uint32_t   PID;   												/* Bit 13 */
	 uint32_t   transport_scrambling_control; 	/* Bit 2*/
	 
	 uint32_t   adaptation_field_control; 			/* Bit 2, 指示本TS首部是否跟随有调整字段*/
	                                                               /* 或有效负载*/
																			   /* '10'  仅含调整字段，无有效负载*/
																				/* '11' 调整字段后为有效负载*/
	 
	 uint32_t   continuity_counter; 						/* Bit 4, 随着每一个具有相同PID的TS流 而增加，*/
																			   /* 达到最大值又回复到0  */
																				/* 如果adaptation_field_control 为'00'或'10' 则continuity_counter不增加*/
}TS_TS_HEADER_T;

/* PCR TS  options */
typedef struct TS_ONLY_PCR_IN_TS_OPTIONS
{
    uint32_t               pcr_pid;
	 uint32_t               cc;
	 uint32_t               discontinuity;
}TS_ONLY_PCR_IN_TS_OPTIONS_T;

/* TS Packets options */
typedef struct TS_OPTIONS
{
	 TS_TS_HEADER_T  ts_header;
	 TS_PES_T             pes_header;
	 void					*m_pUserParam;
	 TS_ONLY_PCR_IN_TS_OPTIONS_T  pcr_ts_opt;
	 void (*tx_ts_to_buffer_call)(uint8_t *data, uint32_t len, void *pUserParam);
	 void (*send_ts_call)(uint8_t *data, uint32_t len, void *pUserParam);
	 
	 LONGLONG     	first_pcr;   /* PCR 的初始值*/
	 LONGLONG     	first_pts;   /* PTS 的初始值*/
	 LONGLONG       ts_packet_count; /* TS 包个数统计*/
	 
	 LONGLONG     	adjust_interval; /* 每隔多少个PCR计算和同步一次*/
	 LONGLONG			base_time;

	 LONGLONG			prior_pcr_pts_interval; /* 先前的PCR和PTS的间隔值*/
	 LONGLONG     	adjust_pcr_value; /* PCR调整的累加值*/

	 LONGLONG     	adjust_base_time; /* 调整的base time 部分*/
	 LONGLONG			prior_interval_1s;

	 LONGLONG			adjust_begin_time;	
	 LONGLONG 		begin_calc_time;
	 LONGLONG 		previous_system_time; 
     LONGLONG 		delay_value_1s[AVERAGE_CALC_NUMBER];
	 LONGLONG 		pcr_pts_interval[AVERAGE_CALC_NUMBER];
	 
	 uint32_t           write_pcr; /* 0 is not write PCR to video TS, otherwise 1 is write video TS */
	 uint32_t           discontinuity;
	 
	 int32_t          	mux_rate; /* 设置的TS发送的码率(1kbps -> 1000bps)*/
	 int32_t				real_rate;  /*实际的TS发送码率(1kbps -> 1000bps)*/	 
	 int32_t          	pcr_packet_count; /* PCR 包计数*/
    int32_t          	pcr_packet_period; /* PCR 发送周期,每间隔多少TS包发送一次PCR*/
	 int32_t          	pcr_period; /* PCR间隔时间,间隔多少ms发送一次PCR*/

	 int32_t            	pat_packet_count; /* PSI报文计数*/
    int32_t           	pat_packet_period; /* PSI报文周期,每隔多少个TS发送一个PSI */
	 uint32_t				psi_tx_count;

	 int32_t				adjust_num; /*调整次数*/
	 int32_t				syn_flag; /*同步状态*/

	 int32_t 				adjust_count;
	 int32_t 				delay_value_count;

	 uint32_t				last_video_cc;
	 int32_t				pts_delay_time; 
	 int32_t				max_pts_pcr_interval;
	 int32_t				min_pts_pcr_interval;
}TS_OPTIONS_T;

uint32_t TS_Crc32Calculate(uint8_t *pData, uint32_t Size);


/*****************************************************************************
* FUNCTION:mpegs_sync_pts()
*
* DESCRIPTION:
*	    定时器的回调函数
* INPUT:
*	  TS_OPTIONS_T *glopts
*	  RNGBUF_t * pBuf
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*   每个10ms调用一次
* HISTORY:
*	
*	Review: 
******************************************************************************/
int mpegs_sync_pts(LONGLONG *pUserParam);

/*****************************************************************************
* FUNCTION:mpegts_timer()
*
* DESCRIPTION:
*	    定时器的回调函数
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*   每个10ms调用一次
* HISTORY:
*	
*	Review: 
******************************************************************************/
void mpegts_timer(TS_OPTIONS_T *glopts,  RNGBUF_t * pVidBuf, RNGBUF_t * pAudBuf, RNGBUF_t * pInsertBuf);

/*****************************************************************************
* FUNCTION:mpegts_get_pcr_packet_period()
*
* DESCRIPTION:
*	    计算PCR插入的间隔。
* INPUT:
*	 int32_t pcr_period //PCR发送周期，单位ms
*   int32_t mux_rate //TS输出码流bps
* OUTPUT:
*	 间隔多少个TS发送一次PCR。
*
* RETURN:
*
* NOTES:

* HISTORY:
*	
*	Review: 
******************************************************************************/
int32_t  mpegts_get_pcr_packet_period(int32_t pcr_period, int32_t mux_rate);

/*****************************************************************************
* FUNCTION:mpegts_get_ts_num()
*
* DESCRIPTION:
*	    根据输出间隔，和TS码流计算应发送多少TS包。
* INPUT:
*	  HWL_PSI_OPTION_T *glopts
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:

* HISTORY:
*	
*	Review: 
******************************************************************************/
int32_t  mpegts_get_ts_num(LONGLONG  tx_period, int32_t mux_rate, int32_t *real_mux_rate);

/*****************************************************************************
* FUNCTION:mpegts_insert_null_packet()
*
* DESCRIPTION:
*	    Write a single null transport stream packet
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:

* HISTORY:
*	
*	Review: 
******************************************************************************/
void mpegts_insert_null_packet(uint8_t *out_buf, uint32_t out_len);

/*****************************************************************************
* FUNCTION:mpegts_write_packet()
*
* DESCRIPTION:
*	   首先给payload前增加PES header，并将PES分包成多个TS packets
*   最后的TS packet 增加填充段。每个TS packet的长度固定188 bytes
*
* INPUT:
*	   uint8_t *payload;  编码器输出的音视频ES流
*                               Video :完整的一帧视频
*                               Audio:采样一帧或多帧音频编码。
*                             
*     int payload_size; ES流的长度
*
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
******************************************************************************/
void mpegts_write_packet(TS_OPTIONS_T *glopts,
                                              const uint8_t *payload, 
							 							uint32_t payload_size, 
                         	 						LONGLONG pcr,
							 							LONGLONG pts, 
							 							LONGLONG dts);
							 							
/*****************************************************************************
* FUNCTION:mpegts_write_psi()
*
* DESCRIPTION:
*	   Send PAT and PMT.
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
******************************************************************************/
void mpegts_write_psi(HWL_PSI_OPTION_T *glopts);

/*****************************************************************************
* FUNCTION:mpegts_tx_pcr_only()
*
* DESCRIPTION:
*	   Only send PCR in ts.
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
******************************************************************************/
void mpegts_tx_pcr_only(TS_OPTIONS_T *glopts, LONGLONG pcr);

/*****************************************************************************
* FUNCTION:mpegts_destory()
*
* DESCRIPTION:
*	   释放TS
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
******************************************************************************/
void mpegts_destory(TS_OPTIONS_T *glopts);

/*****************************************************************************
* FUNCTION:mpegts_psi_destory()
*
* DESCRIPTION:
*	   释放PSI 资源
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
******************************************************************************/
void mpegts_psi_destory(HWL_PSI_OPTION_T *glopts);

/*****************************************************************************
* FUNCTION:mpegts_psi_init()
*
* DESCRIPTION:
*	    初始化SPTS PSI 
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
****************************************************************************/
HWL_PSI_OPTION_T * mpegts_psi_init(void);

/*****************************************************************************
* FUNCTION:mpegts_set_ts_rate()
*
* DESCRIPTION:
*	    配置TS发送码率
* INPUT:
*	    TS_OPTIONS_T *glopts//Ts handle
*		 int32_t   rate //Ts rate
*		 int32_t   pcr_period //PCR period.
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
****************************************************************************/
void mpegts_set_ts_rate(TS_OPTIONS_T *glopts, int32_t   ts_rate, int32_t   pcr_period);

/*****************************************************************************
* FUNCTION:mpegts_set_pts_dts_flag
*
* DESCRIPTION:
*	    配置PES header的pts/dts flag
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
****************************************************************************/
void mpegts_set_pts_dts_flag(TS_OPTIONS_T *glopts, uint32_t  pts_dts_flag);

/*****************************************************************************
* FUNCTION:mpegts_set_pid()
*
* DESCRIPTION:
*	    配置该TS流的PID
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
****************************************************************************/
void mpegts_set_pid(TS_OPTIONS_T *glopts, uint32_t   pid);

/*****************************************************************************
* FUNCTION:mpegts_set_stream_id
*
* DESCRIPTION:
*	    配置PES header的stream id
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
****************************************************************************/
void mpegts_set_stream_id(TS_OPTIONS_T *glopts, uint16_t   stream_id);

/*****************************************************************************
* FUNCTION:mpegts_set_sync_parm()
*
* DESCRIPTION:
*	  设置PTS同步参数
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
****************************************************************************/
void mpegts_set_sync_parm(TS_OPTIONS_T *glopts, int32_t   delay_time, int32_t   max_interval, int32_t   min_interval);

/*****************************************************************************
* FUNCTION:mpegts_init()
*
* DESCRIPTION:
*	    初始化TS
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*
* HISTORY:
*	
*	Review: 
****************************************************************************/
TS_OPTIONS_T * mpegts_init(void);

#define VIDEO_SIZFEOF  8000*7*188

#endif /* __TS_H__ */

/* EOF */

