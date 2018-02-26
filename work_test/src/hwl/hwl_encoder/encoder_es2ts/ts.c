/******************************************************************************

                  版权所有 (C), 2005-2017, GOSPELL有限公司

 ******************************************************************************
  文 件 名   : ts.c
  版 本 号   : 1.1
  作    者   : 许斌
  生成日期   : 2017年7月14日
  最近修改   :
  功能描述   : 对编码器输出audio and video的ES流打包TS输出SPTS
  函数列表   :
  修改历史   :

   1.日    期          : 2017年7月14日
      作    者          : 许斌
      修改内容   : 创建文件

   2.日    期          : 2017年11月03日
      作    者          : 许斌
      修改内容   : PCR采用计算的方式输出高精度PCR 的TS流
                                PTS和PCR同步的方式:
                                  PTS同步OS的系统时间；
                                  PCR同步OS的系统时间，这样就可以保证
                                  PCR同步PTS。

******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "ts.h"
#include "ts_psi.h"

#ifdef MPEG_TS_DEBUG_ENABLE
int currentDebugLevel = MSG_ERR;
#endif

/* Local function */
static void mpegts_get_pcr(TS_OPTIONS_T *glopts, LONGLONG *p64_pcr);
static void mpegts_write_pmt(HWL_PSI_OPTION_T *glopts);
static void mpegts_write_pat(HWL_PSI_OPTION_T *glopts);
static void mpegts_insert_pcr_only(TS_OPTIONS_T *glopts, LONGLONG pcr);
static int write_pcr_bits(uint8_t *buf, LONGLONG pcr);
static uint32_t write_pts(uint8_t *q, int fourbits, long long pts);

static uint32_t write_ts_header(uint8_t *q, 
                                                    int is_start, 
                                                    int write_pcr,
                                                    LONGLONG pcr,
                                                    int discontinuity,
                                                    TS_TS_HEADER_T *p_ts_header);

static uint32_t write_pes_header(uint8_t *q,
                                                       TS_PES_T pes_header,
												               LONGLONG pts,
												               LONGLONG dts);

static uint32_t mpegts_adjust_pcr_base(TS_OPTIONS_T *glopts);

/*****************************************************************************
* FUNCTION:mpegts_get_cc_with_pid()
*
* DESCRIPTION:
*	    根据PID在TS包中查找CC
* INPUT:
*	  const uint8_t *data
*	  uint32_t pid
* OUTPUT:
*	cc
*
* RETURN:
*  cc
* NOTES:
*  
* HISTORY:
*	
*	Review: 
******************************************************************************/
int32_t  mpegts_get_cc_with_pid(uint8_t *data, uint32_t pid)
{
	uint8_t *p_buff = NULL;
	uint16_t  ts_pid;
	uint8_t tmp = 0;

	if (data == NULL)
	{
		return -1;
	}

	p_buff = data ;
	p_buff++;

	tmp = (*p_buff) & 0x1F ;
	ts_pid = tmp << 8;

	p_buff++;
	ts_pid |= (*p_buff );

	if (ts_pid == pid)
	{
		p_buff++;
		tmp = (*p_buff) & 0xF;
		return tmp;
	}

	return -1;
}

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
int mpegs_sync_pts(LONGLONG  *pUserParam)
{ 
	int ret = 0;
	long long base_system_time = 0;
	long long current_system_time = 0;
	long long diff_value;
	unsigned long long pts_time;

	if (pUserParam == NULL)
	{
		return (-1);
	}
	
	OS_TimeNow(&base_system_time);		
	ret = HI_MPI_SYS_SyncPts(base_system_time);	

	if (ret != 0)
	{
		DBG(MSG_ERR,"\r\n Syn Pts time fail!(Line:%d)", __LINE__);
		return (-1);
	}

	ret =  HI_MPI_SYS_GetCurPts(&pts_time);
	if (ret != 0)
	{
		DBG(MSG_ERR,"\r\n Syn Pts time fail!(Line:%d)",__LINE__);
		return (-1);
	}
	OS_TimeNow(&current_system_time);
	*pUserParam = pts_time;
	diff_value = (long long)(pts_time- current_system_time);

	if ((diff_value <= 50)&&(diff_value >= -50)) 
	{	
		//printf("\r\n Sync PTS success!(diff_value = %lld)", diff_value);
		return 0;
	}
	else
	{
		printf("\r\n Warning! OS time not syn PTS (%lld)\n", diff_value);
		//return (-1);
		return (0);
	}
	
}

/*****************************************************************************
* FUNCTION:mpegts_timer()
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
void mpegts_timer(TS_OPTIONS_T *glopts,  RNGBUF_t * pVidBuf, RNGBUF_t * pAudBuf, RNGBUF_t * pInsertBuf)
{
	int ret;
	long long	pcr = 0;
	long long 	current_system_time = 0;
	long long	tx_period = 0;
	int32_t		tx_ts_num = 0;
	int32_t   frame_len = 0;
	int32_t		read_ts_num = 0;
	int32_t		count_ts_num = 0;
	uint8_t 	buf[TS_PACKET_SIZE];
	long long current_time = 0;
	static int sync_count = 6;
	int32_t		cc = 0;
	
	if ((glopts == NULL)
		||(pVidBuf == NULL) || (pAudBuf == NULL) || (pInsertBuf == NULL))
	{
		return;
	}
	
	if (glopts->syn_flag != 1)
	{
		return;
	}

	if (glopts->first_pcr == 0)
	{
		/*给PCR赋初始值*/
		OS_TimeNow(&current_system_time);
		printf("\r\n Current PTS time(Init)[%d] = %lld \n", glopts->m_pUserParam, current_system_time);
		//glopts->first_pcr = (current_system_time - 500000)*27; 
		glopts->first_pcr = (current_system_time)*27; 

		printf("\r\n Init PCR (first_pcr)[%d] = %llu (os time = %llu)\n", glopts->m_pUserParam, glopts->first_pcr, current_system_time);
	}

	OS_TimeNow(&current_time);
	
	/* every 2s */
	if ((current_time - glopts->begin_calc_time) >= 2000000)
	{
		/* 调整PCR */
		if (glopts->syn_flag == 1)
		{
	   		 mpegts_adjust_pcr_base(glopts);
		}
		 
		glopts->begin_calc_time = current_time;
	}

	/*得到发送时间周期*/
	if (glopts->previous_system_time == 0)
	{
		tx_period = 10;
	}
	else
	{
		tx_period = current_time - glopts->previous_system_time;
		tx_period = (long long)((double)tx_period /1000.0 + 0.5);
	}

   glopts->previous_system_time = current_time;

	/*计算应发送的TS 包数*/
	tx_ts_num = mpegts_get_ts_num(tx_period, glopts->mux_rate, &glopts->real_rate);

	DBG(MSG_INFO,"\r\n----------------Begin-----------------------"); 

   /* 从buffer中去取帧*/
	if (pVidBuf != NULL && pAudBuf != NULL || pInsertBuf != NULL)
	{
			frame_len = RngNBytes(pVidBuf) + RngNBytes(pAudBuf) + RngNBytes(pInsertBuf);

			DBG(MSG_INFO, "\r\n frame_len = %d, tx_ts_num = %d (%d)", frame_len, tx_ts_num, (TS_PACKET_SIZE*tx_ts_num)); 
			
			if(frame_len >= (TS_PACKET_SIZE*tx_ts_num))
			{

				while(tx_ts_num > 0)
				{
					if (glopts->pcr_packet_count >= glopts->pcr_packet_period)
					{
						/* 插入PCR TS */
						glopts->pcr_packet_count = 0;

						mpegts_get_pcr(glopts, &pcr);


						mpegts_tx_pcr_only(glopts, pcr);

					   DBG(MSG_INFO, "\r\n insert pcr  = %lld (line:%d)", pcr, __LINE__); 
							
						count_ts_num++;
						glopts->ts_packet_count++;
						tx_ts_num--;
					}

					/* 发送TS */
					if (RngNBytes(pInsertBuf) >= TS_PACKET_SIZE) {
						RngBufGet(pInsertBuf, (char*)buf, TS_PACKET_SIZE);
					}
					else if (RngNBytes(pAudBuf) >= TS_PACKET_SIZE) {
						RngBufGet(pAudBuf, (char*)buf, TS_PACKET_SIZE);
					}
					else {
						RngBufGet(pVidBuf, (char*)buf, TS_PACKET_SIZE);
					}

					cc = mpegts_get_cc_with_pid(buf, glopts->ts_header.PID);
					if (cc != -1)
					{
						glopts->last_video_cc = cc;
					}
					
	 				if (glopts->send_ts_call != NULL)
	 				{
	      				glopts->send_ts_call(buf, TS_PACKET_SIZE, glopts->m_pUserParam);
	 				}

					DBG(MSG_INFO, "\r\n TS(Video)  (line:%d)", __LINE__); 
					
					count_ts_num++;
					glopts->pcr_packet_count++;
					glopts->ts_packet_count++;
					tx_ts_num--;
				}
			}
			else
			{

			    /*计算需要填充多少个NULL TS */
				 read_ts_num = frame_len/TS_PACKET_SIZE;

              while(tx_ts_num > 0)
				{
					if (glopts->pcr_packet_count >= glopts->pcr_packet_period)
					{
						/* 插入PCR TS */
						glopts->pcr_packet_count = 0;

						mpegts_get_pcr(glopts, &pcr);
			
						mpegts_tx_pcr_only(glopts, pcr);

						DBG(MSG_INFO, "\r\n insert pcr  = %lld (line:%d)", pcr, __LINE__); 
						
						count_ts_num++;
						glopts->ts_packet_count++;
						tx_ts_num--;
					}

					/* 发送TS */
					if (read_ts_num > 0)
					{
						if (RngNBytes(pInsertBuf) >= TS_PACKET_SIZE) {
							RngBufGet(pInsertBuf, (char*)buf, TS_PACKET_SIZE);
						}
						else if (RngNBytes(pAudBuf) >= TS_PACKET_SIZE) {
							RngBufGet(pAudBuf, (char*)buf, TS_PACKET_SIZE);
						}
						else {
							RngBufGet(pVidBuf, (char*)buf, TS_PACKET_SIZE);
						}

						cc = mpegts_get_cc_with_pid(buf, glopts->ts_header.PID);
						if (cc != -1)
						{
							glopts->last_video_cc = cc;
						}
						
						DBG(MSG_INFO, "\r\n TS(Video)  (line:%d)", __LINE__); 
						
						read_ts_num--;
						count_ts_num++;
					}
					else
					{
						/* 插入空包*/
						mpegts_insert_null_packet(buf, TS_PACKET_SIZE);

						DBG(MSG_INFO, "\r\n TS (NULL)  (line:%d)", __LINE__); 
						
					}

	 				if (glopts->send_ts_call != NULL)
	 				{
	      				glopts->send_ts_call(buf, TS_PACKET_SIZE, glopts->m_pUserParam);
	 				}

					glopts->pcr_packet_count++;
					glopts->ts_packet_count++;
					count_ts_num++;
					tx_ts_num--;
				}
			}
	}

	DBG(MSG_INFO, "\r\n-----------------End----------------------"); 
}

/*****************************************************************************
* FUNCTION:write_pts()
*
* DESCRIPTION:
*	    计算TS流的CRC
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
uint32_t TS_Crc32Calculate(uint8_t *pData, uint32_t Size)
{
#define CRC32_MAX_COEFFICIENTS	256
	static const uint32_t sc_Crc32Table[CRC32_MAX_COEFFICIENTS]=
	{
		0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
		0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
		0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
		0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
		0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
		0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
		0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
		0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
		0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
		0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
		0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
		0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
		0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
		0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
		0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
		0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
		0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
		0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
		0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
		0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
		0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
		0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
		0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
		0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
		0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
		0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
		0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
		0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
		0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
		0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
		0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
		0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
		0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
		0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
		0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
		0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
		0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
		0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
		0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
		0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
		0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
		0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
		0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
		0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
		0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
		0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
		0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
		0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
		0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
		0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
		0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
		0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
		0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
		0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
		0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
		0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
		0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
		0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
		0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
		0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
		0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
		0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
		0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
		0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
	};

	uint32_t lCrc32Val = 0xFFFFFFFF;
	uint8_t *pEnd = pData + Size;
	while(pData < pEnd)
	{
		lCrc32Val = (lCrc32Val<<8)^sc_Crc32Table[(lCrc32Val>>24)^(*pData)];
		pData++;
	}
	return lCrc32Val;
}

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
int32_t  mpegts_get_pcr_packet_period(int32_t pcr_period, int32_t mux_rate)
{
   int32_t pcr_packet_period = 0;
	 
   pcr_packet_period = (int32_t)((double)(mux_rate * pcr_period) /(double)(TS_PACKET_SIZE * 8 * 1000));

	return pcr_packet_period;
}

/*****************************************************************************
* FUNCTION:mpegts_get_ts_num()
*
* DESCRIPTION:
*	    根据输出间隔，和TS码流计算应发送多少TS包。
* INPUT:
*	  tx_period //发送TS的间隔时间，单位ms
*    mux_rate//TS发送的码流
* OUTPUT:
*	  在tx_period间隔时间内，按照mux_rate 的TS码流应该
*  发送多少TS包。
*
* RETURN:
*
* NOTES:

* HISTORY:
*	
*	Review: 
******************************************************************************/
int32_t  mpegts_get_ts_num(LONGLONG  tx_period, int32_t mux_rate, int32_t *real_mux_rate)
{
   int32_t tx_ts_num = 0;

	tx_ts_num = (int32_t)((double)tx_period* (double)mux_rate/(double)(TS_PACKET_SIZE*8*1000) + 0.5);

	*real_mux_rate = (int32_t)((double)tx_ts_num * (double)(TS_PACKET_SIZE*8*1000)/(double)tx_period);
	
	return tx_ts_num;
}

/*****************************************************************************
* FUNCTION:retransmit_si_info()
*
* DESCRIPTION:
*	    TS PSI
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
static void retransmit_si_info(TS_OPTIONS_T *glopts, HWL_PSI_OPTION_T *g_psi_opts, int force_pat)
{
	if (force_pat)
	{
			glopts->psi_tx_count--;
			if (glopts->psi_tx_count == 0)
			{
        		mpegts_write_psi(g_psi_opts);
				glopts->psi_tx_count = PSI_TX_INTERVAL_NUM;
			}
	}
}

/*****************************************************************************
* FUNCTION:mpegts_write_pmt()
*
* DESCRIPTION:
*	    Send PMT
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
static void mpegts_write_pmt(HWL_PSI_OPTION_T *glopts)
{
    HWL_EncPsiInfo PsiTableInfo; /* 创建的PSI表 */
	uint32_t i;

	 /* 初始化表 */
	for (i = 0; i < glopts->pmt_num; i++) {
		PsiTableInfo.m_PsiPacketCounter = 0;
		ts_psi_CreatePmt(&(glopts->pmt_Info[i]), &(glopts->pmt_continuity_counter[i]), &PsiTableInfo);

		if (glopts->tx_psi_call != NULL)
		{
			glopts->tx_psi_call(PsiTableInfo.m_pPsiPacket[0], MPEG2_TS_PACKET_SIZE, glopts->m_pUserParam);
		}
	}
}

/*****************************************************************************
* FUNCTION:mpegts_write_pat()
*
* DESCRIPTION:
*	    Send PAT
* INPUT:
*	    HWL_PSI_OPTION_T *glopts
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
static void mpegts_write_pat(HWL_PSI_OPTION_T *glopts)
{
       HWL_EncPsiInfo PsiTableInfo; /* 创建的PSI表 */

		PsiTableInfo.m_PsiPacketCounter = 0;  /* 初始化表 */
		ts_psi_CreatePat(&(glopts->pat_Info), (&glopts->pat_continuity_counter), &PsiTableInfo);
	
		if (glopts->tx_psi_call != NULL)
		{
		    glopts->tx_psi_call(PsiTableInfo.m_pPsiPacket[0], MPEG2_TS_PACKET_SIZE, glopts->m_pUserParam);
		}
}

static void mpegts_write_sdt(HWL_PSI_OPTION_T *glopts)
{
	HWL_EncPsiInfo PsiTableInfo; /* 创建的PSI表 */

	PsiTableInfo.m_PsiPacketCounter = 0;  /* 初始化表 */
	ts_psi_CreateSdt(&(glopts->sdt_Info), (&glopts->sdt_continuity_counter), &PsiTableInfo);

	if (glopts->tx_psi_call != NULL)
	{
		glopts->tx_psi_call(PsiTableInfo.m_pPsiPacket[0], MPEG2_TS_PACKET_SIZE, glopts->m_pUserParam);
	}
}

static void mpegts_write_cat(HWL_PSI_OPTION_T *glopts)
{
	HWL_EncPsiInfo PsiTableInfo; /* 创建的PSI表 */

	PsiTableInfo.m_PsiPacketCounter = 0;  /* 初始化表 */
	ts_psi_CreateNullCat((&glopts->cat_continuity_counter), PsiTableInfo.m_pPsiPacket[0]);

	if (glopts->tx_psi_call != NULL)
	{
		glopts->tx_psi_call(PsiTableInfo.m_pPsiPacket[0], MPEG2_TS_PACKET_SIZE, glopts->m_pUserParam);
	}
}

/*****************************************************************************
* FUNCTION:mpegts_insert_pcr_only()
*
* DESCRIPTION:
*	    Write a single transport stream packet with a PCR and no payload
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
static void mpegts_insert_pcr_only(TS_OPTIONS_T *glopts, LONGLONG pcr)
{
    uint8_t *q = NULL;
    uint8_t buf[TS_PACKET_SIZE];
    uint32_t  cc =0;
    q = buf;

    /* TS Header */
    *q++ = 0x47;

    *q++ = glopts->pcr_ts_opt.pcr_pid >> 8;
    *q++ = glopts->pcr_ts_opt.pcr_pid;

	if (glopts->ts_header.PID == glopts->pcr_ts_opt.pcr_pid)
	{
		cc =  glopts->last_video_cc;
	}
	 
	/* Continuity Count field does not increment (see 13818-1 section 2.4.3.3) */
   // *q++ = 0x20 | glopts->ts_header.continuity_counter;   /* adaptation_field only, no payload */
    *q++ = 0x20 | cc;   /* adaptation_field only, no payload */
		
    /* Adaptation field */
    *q++ = TS_PACKET_SIZE - 5; /* Adaptation Field Length */
		
    *q++ = 0x10;               /* Adaptation flags: PCR present */

	if (glopts->pcr_ts_opt.discontinuity) 
	{
        q[-1] |= 0x80;
    }

    /** PCR coded into 6 bytes */
    q += write_pcr_bits(q, pcr);

    /* stuffing bytes */
    memset(q, 0xFF, TS_PACKET_SIZE - (q - buf));


	 /* 发送PCR packet */
	 if (glopts->send_ts_call != NULL)
	 {
	      glopts->send_ts_call(buf, TS_PACKET_SIZE,  glopts->m_pUserParam);
	 }
}

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
void mpegts_insert_null_packet(uint8_t *out_buf, uint32_t out_len)
{
    uint8_t *q = NULL;

	if (out_len != TS_PACKET_SIZE)
	{
	    return;
	}

	if (out_buf == NULL)
	{
	   return;
	}
	
    q    = out_buf;
    *q++ = 0x47;
    *q++ = 0x00 | 0x1f;
    *q++ = 0xff;
    *q++ = 0x10;
		
    memset(q, 0x0FF, TS_PACKET_SIZE - (q - out_buf));
}

/*****************************************************************************
* FUNCTION:write_pcr_bits()
*
* DESCRIPTION:
*	    写PCR 字段(Total:48 bit(6 bytes))
*          program_clock_reference_base                   bit 33
*          reserved                                                 bit 6
*          program_clock_reference_extension            bit 9
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
static int write_pcr_bits(uint8_t *buf, LONGLONG pcr)
{
    int len = 0;
    LONGLONG pcr_low = pcr % 300;
	 LONGLONG pcr_high = pcr / 300;

    /* Fill program_clock_reference_base */
    *buf++ = pcr_high >> 25;
    *buf++ = pcr_high >> 17;
    *buf++ = pcr_high >>  9;
    *buf++ = pcr_high >>  1;

	 /*Fill program_clock_reference_extension */
    *buf++ = pcr_high <<  7 | pcr_low >> 8 | 0x7e;
    *buf++ = pcr_low;

	 len = 6;

    return len;
}

/*****************************************************************************
* FUNCTION:write_pts()
*
* DESCRIPTION:
*	    写PTS字段
* INPUT:
*	
* OUTPUT:
*	None.
*
* RETURN:
*
* NOTES:
*   PTS:
*         0010                    bit 4   0010 填充字段，表示只含有PTS，
*                                             不含有DTS 当含有DTS时，这个填
*                                             充字段为0011(如该值为0001 填充字段，
*                                             表示接下来为DTS。 )

*         PTS[32…30]       bit 3
*         marker_bit            bit 1
*         PTS[29…15]       bit 15
*         marker_bit            bit 1
*         PTS[14…0]         bit 15
*         marker_bit            bit 1
*
*  [TODO]: DTS the same as PTS.

* HISTORY:
*	
*	Review: 
******************************************************************************/
static uint32_t write_pts(uint8_t *q, int fourbits, LONGLONG pts)
{
    int val;
	int len = 0;
	uint8_t *p_buff;

	p_buff = q;
    val  = fourbits << 4 | (((pts >> 30) & 0x07) << 1) | 1;
    *p_buff++ = val;
	 len++;
	 
    val  = (((pts >> 15) & 0x7fff) << 1) | 1;
    *p_buff++ = val >> 8;
	 len++;
	 
    *p_buff++ = val;
	 len++;
	 
    val  = (((pts) & 0x7fff) << 1) | 1;
    *p_buff++ = val >> 8;
	 len++;
	 
    *p_buff++ = val;
	 len++;
		
	 return len;
}

/*****************************************************************************
* FUNCTION:write_ts_header()
*
* DESCRIPTION:
*	    写TS头
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
static uint32_t write_ts_header(uint8_t *q, 
                                                    int is_start, 
                                                    int write_pcr,
                                                    LONGLONG pcr,
                                                    int discontinuity,
                                                    TS_TS_HEADER_T *p_ts_header)
{
    int val = 0;
    int len = 0;
	 int tmp_len = 0;
	 uint8_t *p_buff;

	 p_buff = q;

    DBG(MSG_INFO, "\r\n write TS header  (cc = %d)", p_ts_header->continuity_counter);
			
	 /* Write 'sync_byte' of TS frame (Fixed 0x47)*/
    *p_buff++ = 0x47;
	 len++;
	 
    val  =  p_ts_header->PID >> 8;
    if (is_start)
    {
       /*Set 'payload_unit_start_indicator', This is first TS frame. */
        val |= 0x40;
    }

	 *p_buff++    = val;
	 len++;
	 
    *p_buff++    = p_ts_header->PID;
    len++;

	 if (write_pcr == 1)
	 {
	      if (is_start)
	      {
		      /* include adaptation_field and payload */
				*p_buff++      = 0x30 | (p_ts_header->continuity_counter & 0xf); 
				 len++;

				/* Adaptation field */
				*p_buff++ = 1 + 6; /* Adaptation Field Length */
				 len++;

				*p_buff++ = 0x10;               /* Adaptation flags: PCR present */
				 len++;

		      if (discontinuity) 
		     {
	            p_buff[-1] |= 0x80;
	         }

	         /** PCR coded into 6 bytes */
	         tmp_len = write_pcr_bits(p_buff, pcr);
			  p_buff += tmp_len;
			  len += tmp_len;
	      }
			else
			{
			     /* no adaptation_field, only payload */
              *p_buff++      = 0x10 | (p_ts_header->continuity_counter & 0xf); // payload indicator + CC
               len++;

			}
			
	 }
	 else
	 {
	     /* no adaptation_field, only payload */
         *p_buff++      = 0x10 | (p_ts_header->continuity_counter & 0xf); // payload indicator + CC
          len++;
	 }

    p_ts_header->continuity_counter++;
	 if (p_ts_header->continuity_counter > 0xF)
	 {
	    p_ts_header->continuity_counter = 0;
	 }
	return len;
}

/*****************************************************************************
* FUNCTION:write_pes_header()
*
* DESCRIPTION:
*	    写PES Header字段，封装为PES帧。
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
static uint32_t write_pes_header(uint8_t *q,
                                                       TS_PES_T pes_header,
								                            LONGLONG pts,
								                            LONGLONG dts)
{
     int val = 0;
	 int flags = 0;
	 int header_len = 0;
	 uint32_t  len = 0;
	 uint8_t *p_buff;
		 
	 p_buff = q;

	 /* Write packet_start_code_prefix */
       *p_buff++ = 0x00;
       *p_buff++ = 0x00;
       *p_buff++ = 0x01;

	*p_buff++ = pes_header.stream_id;

	*p_buff++ = pes_header.PES_packet_length >> 8;
	*p_buff++ = pes_header.PES_packet_length;


	val  = 0x80;

       /* data alignment indicator is required for subtitle and data streams */
	if (pes_header.data_alignment_indicator == 1)
	{
	    val |= 0x04;
	}
	*p_buff++ = val;

	flags = 0;
	
	switch (pes_header.PTS_DTS_flag)
	{
	    case PES_NOT_INCLUDE_PTS_DTS:
			flags = 0;
			break;

		case  PES_INCLUDE_ONLY_PTS:
			flags |= 0x80;
			header_len += 5;
			break;

		case PES_INCLUDE_PTS_DTS:
			flags |= 0xC0;
			header_len += 5;
			header_len += 5;
			break;

		default:
			flags = 0;
			break;
	}

    if (pes_header.PES_extension_flag == 1)
   	{
   	    flags |= 0x01;
	    header_len += 3;
   	}

	*p_buff++ = flags;

	/* Write PES_header_data_length */
	*p_buff++ = header_len;
	 
	/* Write PTS */
	if (pes_header.PTS_DTS_flag == PES_INCLUDE_ONLY_PTS)
	{

	     write_pts(p_buff, flags >> 6, pts);
         p_buff += 5;
	}
	else if (pes_header.PTS_DTS_flag == PES_INCLUDE_PTS_DTS)
	{	
	     write_pts(p_buff, flags >> 6, pts);
         p_buff += 5;

	     /* Write DTS */
	     write_pts(p_buff, flags >> 6, dts);
        p_buff += 5;	
	}

	/* Base PES header(include PTS) length is 9 bytes + 5 bytes*/
	len = p_buff - q;

	return len;
}

/*****************************************************************************
* FUNCTION:mpegts_get_pts()
*
* DESCRIPTION:
*	    计算得到当前PTS
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
void mpegts_get_pts(LONGLONG pts, int32_t video_frame_rate, LONGLONG  *p64_current_pts)
{
    LONGLONG  current_pts;
		
    /* 用一秒钟除以帧率,得到每一帧应该耗时是多少，单位是 timescale单位*/
    uint32_t video_pts_increment = 90000 / video_frame_rate;   

   current_pts = pts;
	current_pts += video_pts_increment;

	*p64_current_pts = current_pts;
}

/*****************************************************************************
* FUNCTION:mpegts_adjust_pcr_base()
*
* DESCRIPTION:
*	    同步PTS和PCR
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
static uint32_t mpegts_adjust_pcr_base(TS_OPTIONS_T *glopts)
{
	int i;
	LONGLONG		pcr;
	LONGLONG		pcr_us = 0;
	LONGLONG		os_us = 0;
	unsigned long long		pts_us = 0;
	LONGLONG		current_interval_us = 0;
	LONGLONG		current_interval_s = 0;
	LONGLONG		average_value_1s = 0;
	LONGLONG		average_value_pcr_pts = 0;

	if (glopts->adjust_count > 0)
	{
		glopts->adjust_count--;
	}
	else
	{
		glopts->adjust_count = 0;
	}

	mpegts_get_pcr(glopts, &pcr);
	pcr_us = (LONGLONG)(((double)pcr*37.037037037)/(double)1000);

	DBG(MSG_DEBUG, "\r\n adjust_count[%d] = %d", glopts->m_pUserParam, glopts->adjust_count);
	
    //OS_TimeNow(&os_us);
    HI_MPI_SYS_GetCurPts(&pts_us);

    //DBG(MSG_DEBUG, "\r\n pts_us[%d] = %llu, pcr_us[%d] = %llu",glopts->m_pUserParam, pts_us, glopts->m_pUserParam, pcr_us);
	 
	current_interval_us = pts_us - pcr_us;
	//DBG(MSG_DEBUG, "\r\n first_pts[%d] = %lld (us)", glopts->m_pUserParam, glopts->first_pts);
	DBG(MSG_DEBUG, "\r\n current pts and pcr interval[%d] = %lld (us)",glopts->m_pUserParam, current_interval_us);
	//DBG(MSG_DEBUG, "\r\n current all pts time[%d] = %lld (us)", glopts->m_pUserParam, (pts_us - glopts->first_pts));
	
	current_interval_s = (LONGLONG)((double)current_interval_us*(double)1000000/(double)(pts_us - glopts->first_pts));
	
	
	DBG(MSG_DEBUG, "\r\n current_interval_s[%d] = %lld \n", glopts->m_pUserParam, current_interval_s);
	
	if (glopts->adjust_count == 0)
	{	
		glopts->pcr_pts_interval[glopts->delay_value_count] = current_interval_us;
		glopts->delay_value_1s[glopts->delay_value_count] = current_interval_s;
		
		glopts->delay_value_count++;

		if (glopts->delay_value_count >= AVERAGE_CALC_NUMBER)
		{
		   DBG(MSG_DEBUG, "\r\n ------------------------------------------");
			for (i = 0; i < AVERAGE_CALC_NUMBER; i++)
			{
				average_value_1s += glopts->delay_value_1s[i];
				average_value_pcr_pts += glopts->pcr_pts_interval[i];
				DBG(MSG_DEBUG, "\r\n ==%d==delay_value_1s[%d]= %lld", glopts->m_pUserParam, i, glopts->delay_value_1s[i]);
				DBG(MSG_DEBUG, "\r\n ==%d==pcr_pts_interval[%d]= %lld", glopts->m_pUserParam, i, glopts->pcr_pts_interval[i]);
			}

			average_value_1s = average_value_1s/AVERAGE_CALC_NUMBER;
			average_value_pcr_pts = average_value_pcr_pts/AVERAGE_CALC_NUMBER;
			
			DBG(MSG_DEBUG, "\r\n average_value_1s[%d] = %lld \n", glopts->m_pUserParam, average_value_1s);
			DBG(MSG_DEBUG, "\r\n average_value_pcr_pts[%d] = %lld \n", glopts->m_pUserParam, average_value_pcr_pts);
			
			if (((average_value_pcr_pts < glopts->min_pts_pcr_interval) && (average_value_pcr_pts > PTS_PCR_MIN_OVERTURN))
			 	|| ((average_value_pcr_pts > glopts->max_pts_pcr_interval) && (average_value_pcr_pts < PTS_PCR_MAX_OVERTURN)))
			{
				average_value_1s = glopts->base_time +(LONGLONG)((double)(average_value_1s*1000)/37.037037037);
			
				DBG(MSG_DEBUG, "\r\n Adjust base time[%d] = %lld", glopts->m_pUserParam, average_value_1s);
				DBG(MSG_DEBUG, "\r\n Adjust number[%d] = %d", glopts->m_pUserParam, glopts->adjust_num);

				LONGLONG  sys_time_us;
				OS_TimeNow(&sys_time_us);
			
         		DBG(MSG_DEBUG, "\r\n Prior Adjust number[%d] = %lld", glopts->m_pUserParam, glopts->adjust_begin_time);
		   		DBG(MSG_DEBUG, "\r\n Current Adjust number[%d] = %lld\n", glopts->m_pUserParam, sys_time_us);
			 
				glopts->adjust_begin_time = sys_time_us;
				glopts->base_time = average_value_1s;


				glopts->delay_value_count = 0;
				glopts->adjust_num++;
				glopts->adjust_count = 4;

				DBG(MSG_DEBUG, "\r\n Adjust base time[%d] = %lld",glopts->m_pUserParam, glopts->base_time);
				DBG(MSG_DEBUG, "\r\n ------------------------------------------");

				return 1;
			}
			else
			{
			    glopts->adjust_count = 4;
				 glopts->delay_value_count = 0;
				 
				 DBG(MSG_DEBUG, "\r\n It not need adjust[%d]!!!", glopts->m_pUserParam);
				 DBG(MSG_DEBUG, "\r\n ------------------------------------------");
				 return 0;
			}
			
		}
	}
	
	/*DBG(MSG_DEBUG, "\r\n glopts->prior_interval_1s = %lld\n",glopts->prior_interval_1s);*/
	
  return 0;
}

/*****************************************************************************
* FUNCTION:mpegts_get_pcr()
*
* DESCRIPTION:
*	    计算得到当前PCR
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
static void mpegts_get_pcr(TS_OPTIONS_T *glopts, LONGLONG *p64_pcr)
{
	LONGLONG		pcr;
   int32_t				ts_rate;

	if (glopts->real_rate == 0)
	{
		ts_rate = glopts->mux_rate;
	}
	else
	{
		ts_rate = glopts->real_rate;
	}
	ts_rate = glopts->mux_rate;
	pcr =(LONGLONG)(glopts->base_time*((((188*(double)glopts->ts_packet_count)+11)*8)/(double)ts_rate)) + glopts->first_pcr;

#if 0

	LONGLONG base_system_time_tmp;

	OS_TimeNow(&base_system_time_tmp);
	printf("\r\n  Calc PCR:OS time = %lld \n", base_system_time_tmp);

	LONGLONG pts_time_tmp;
	int result;
	result =  HI_MPI_SYS_GetCurPts(&pts_time_tmp);
	if (result == 0)
	{
		printf("\r\n Calc PCR:first_pcr= %lld, current pts = %lld \n", 
			glopts->first_pcr,
			pts_time_tmp);
	}


#endif
	*p64_pcr = pcr;
}

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
							 							LONGLONG dts)
{
    int  is_start = 0;
	 int  stuffing_len = 0;
	 int afc_len = 0;
	 uint8_t *q = NULL;
	 uint32_t len = 0;
	 uint32_t header_len = 0;
    uint8_t buf[TS_PACKET_SIZE];
	 int force_pat = 1; 
	 int write_pcr = 0;
	
     /*is_start用于判断TS包封装PES或PSI的第一帧报文。*/
	 is_start = 1;
		
    q = buf;

	 DBG(MSG_INFO, "\r\n ------------Begin write pes pkt-------------------\n");
	 DBG(MSG_INFO, "\r\n payload_size:%d \n", payload_size);

    /* 根据ES是视频还是音频给出PES长度*/
    if ((glopts->pes_header.stream_id == PES_AUDIO_STREAM_ID) || (glopts->pes_header.stream_id ==PES_AC3_STREAM_ID))
    {
         glopts->pes_header.PES_packet_length =   payload_size + 5 + 3;
    }
	else
	{
	    /* [NOTE]: Video PES_packet_length must be 0. */
	    glopts->pes_header.PES_packet_length = 0;
	}

    while (payload_size > 0)
    {
		  /* 每一个ES 开始都发送一次PSI */
         //retransmit_si_info(glopts, g_psi_opts, force_pat);	
         force_pat = 0;
			
         /* 准备TS的包头*/
         len =write_ts_header(q, 
                                          is_start, 
                                          write_pcr,
                                          pcr,
                                          glopts->discontinuity,
                                          &(glopts->ts_header));

	      q += len;
    
         /* 填写第一帧TS包，第一帧TS包封装PES头*/
         if (is_start)
         {
             /* 写PES头*/
             len = write_pes_header(q, glopts->pes_header, pts, dts);
		      q += len;

             is_start = 0;
				write_pcr = 0;
         } 

         /* 计算头(包括TS头以及可能的PES头)的长度*/
         header_len = q - buf;
			 
          /* 计算TS剩余填充的有效数据*/
         len = TS_PACKET_SIZE - header_len;

         if (len > payload_size)
         {
             len = payload_size;
         }

         /* 计算需要写入填充字节的长度*/
         stuffing_len = TS_PACKET_SIZE - header_len - len;

         if (stuffing_len > 0)
         {
             /* add stuffing with AFC */
             if (buf[3] & 0x20) 
			   {							
                 /* stuffing already present: increase its size */
                 afc_len = buf[4] + 1;

                 memmove(buf + 4 + afc_len + stuffing_len,
                                buf + 4 + afc_len,
                                header_len - (4 + afc_len));
								 
                buf[4] += stuffing_len;
                memset(buf + 4 + afc_len, 0xff, stuffing_len);
            } 
			  else 
			  {
				   /* add stuffing */
                memmove(buf + 4 + stuffing_len, buf + 4, header_len - 4);
                buf[3] |= 0x20;
                buf[4]  = stuffing_len - 1;
								
                if (stuffing_len >= 2) 
				  {
                    buf[5] = 0x00;
                    memset(buf + 6, 0xff, stuffing_len - 2);
                }
            }
        } 

		  
         /* 写入有效数据到TS包*/
         memcpy(buf + TS_PACKET_SIZE - len, payload, len);

         payload      += len;
         payload_size -= len;
         q = buf;

		 /* 发送数据*/
        if (glopts->tx_ts_to_buffer_call != NULL)
        {
            glopts->tx_ts_to_buffer_call(buf, TS_PACKET_SIZE, glopts->m_pUserParam);
        }			
  } 
 
   DBG(MSG_INFO, "\r\n ===========End write pes pkt =================\n");
}

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
void mpegts_write_psi(HWL_PSI_OPTION_T *glopts)
{
    if (glopts == NULL)
    {
        return;
    }

    /* PAT */
	mpegts_write_pat(glopts);

	/* PMT */
	mpegts_write_pmt(glopts);

	mpegts_write_sdt(glopts);

	mpegts_write_cat(glopts);
}

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
void mpegts_tx_pcr_only(TS_OPTIONS_T *glopts, LONGLONG pcr)
{
    if (glopts == NULL)
    {
         return;
    }
	
    mpegts_insert_pcr_only(glopts, pcr);
}

/*****************************************************************************
* FUNCTION:Init_pes_header()
*
* DESCRIPTION:
*	    初始化pes Header
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
static void init_pes_header(TS_PES_T *p_pes_header)
{
	 p_pes_header->packet_start_code_prefix = PES_START_CODE_PREFIX;
	 p_pes_header->stream_id = PES_VIDEO_STREAM_ID;
	 p_pes_header->PES_packet_length =  0;
	 p_pes_header->reserved = 2;
	 p_pes_header->PES_scrambling_control = 0;
	 p_pes_header->PES_priority = 0;
	 p_pes_header->data_alignment_indicator = 0;
	 p_pes_header->copyright = 0;
	 p_pes_header->original_or_copy = 0;
	 p_pes_header->PTS_DTS_flag =  PES_INCLUDE_PTS_DTS;
	 p_pes_header->ESCR_flag = 0;
	 p_pes_header->ES_rate_flag = 0;
    p_pes_header->DSM_trick_mode_flag = 0;
	 p_pes_header->additional_copy_info_flag = 0;
	 p_pes_header->PES_CRC_flag = 0;
	 p_pes_header->PES_header_data_length = 0x5;
    
}

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
void mpegts_destory(TS_OPTIONS_T *glopts)
{
	if (glopts != NULL)
	{
	    free(glopts);
	}
}

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
void mpegts_psi_destory(HWL_PSI_OPTION_T *glopts)
{
	if (glopts != NULL)
	{
	    free(glopts);
	}
}

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
HWL_PSI_OPTION_T * mpegts_psi_init(void)
{

  	HWL_PSI_OPTION_T * psi_options = NULL;
		
	if ((psi_options = (HWL_PSI_OPTION_T *) malloc( sizeof(HWL_PSI_OPTION_T))) == NULL) 
	{
        fprintf(stderr, "PSI options malloc failed\n");
        return NULL;
    }
	
    memset(psi_options, 0x0, sizeof(HWL_PSI_OPTION_T));

	/* Init PAT info*/		 
   psi_options->pat_Info.m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
	psi_options->pat_Info.m_TsId = 0; 
	psi_options->pat_Info.m_Version = 0;
	psi_options->pat_Info.m_ProgramInfo[0].m_PmtPid = PROGRAM_MAP_PID;
	psi_options->pat_Info.m_ProgramInfo[0].m_ProgramNum = 1; // It only one program.
	psi_options->pat_Info.m_ProgramLen = 1;/* 节目数*/

   /* Init PMT info */			 
	psi_options->pmt_Info[0].m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
	psi_options->pmt_Info[0].m_Version = 0;
	psi_options->pmt_Info[0].m_VidEncMode = 3; 
	psi_options->pmt_Info[0].m_VidPid = TS_VIDEO_PID;
	psi_options->pmt_Info[0].m_AudEncMode = PSI_AUD_ENC_MODE_MPEG1_L2;
	psi_options->pmt_Info[0].m_AudPid = TS_AUDIO_PID;
	psi_options->pmt_Info[0].m_PcrPid = TS_PCR_PID; 
	psi_options->pmt_Info[0].m_PmtPid = PROGRAM_MAP_PID;
	psi_options->pmt_Info[0].m_ProgramNum = 1;

	psi_options->pat_continuity_counter = 0;
	psi_options->pmt_continuity_counter[0] = 0;
		
	return psi_options;
}


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
void mpegts_set_pid(TS_OPTIONS_T *glopts, uint32_t   pid)
{
     if (glopts == NULL)
     {
         return;
     }

	  glopts->ts_header.PID = pid;
}

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
void mpegts_set_ts_rate(TS_OPTIONS_T *glopts, int32_t   ts_rate, int32_t   pcr_period)
{
	  double  other_rate;
		
     if (glopts == NULL)
     {
         return;
     }
		
	  glopts->mux_rate = ts_rate;
	 /* glopts->mux_rate = 30 * 1000 *1000;*/
	
	  glopts->pcr_period = pcr_period;

	  /* 计算得到PCR 包的发送间隔*/
	 glopts->pcr_packet_period  = mpegts_get_pcr_packet_period(glopts->pcr_period, glopts->mux_rate);
		 
     glopts->pat_packet_period      = (LONGLONG)glopts->mux_rate * PAT_RETRANS_TIME /
                                     (TS_PACKET_SIZE * 8 * 1000);

     glopts->pcr_packet_count = glopts->pcr_packet_period;
	  glopts->pat_packet_count      = glopts->pat_packet_period - 1;
		 
	 DBG(MSG_INFO, "\r\n pcr_packet_period = %d, pat_packet_period = %d", glopts->pcr_packet_period, glopts->pat_packet_period);
}

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
void mpegts_set_pts_dts_flag(TS_OPTIONS_T *glopts, uint32_t  pts_dts_flag)
{
     if (glopts == NULL)
     {
         return;
     }

     glopts->pes_header.PTS_DTS_flag = pts_dts_flag;
}

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
void mpegts_set_stream_id(TS_OPTIONS_T *glopts, uint16_t   stream_id)
{
     if (glopts == NULL)
     {
         return;
     }

     glopts->pes_header.stream_id = stream_id;
}

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
void mpegts_set_sync_parm(TS_OPTIONS_T *glopts, int32_t   delay_time, int32_t   max_interval, int32_t   min_interval)
{
     if (glopts == NULL)
     {
         return;
     }

	  glopts->pts_delay_time = delay_time;
	  glopts->max_pts_pcr_interval = max_interval;
	  glopts->min_pts_pcr_interval = min_interval;
}

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
TS_OPTIONS_T * mpegts_init(void)
{

  	TS_OPTIONS_T * ts_options = NULL;
		
	if ((ts_options = (TS_OPTIONS_T *) malloc( sizeof(TS_OPTIONS_T))) == NULL) 
	{
        fprintf(stderr, "Ts options malloc failed\n");
        return NULL;
    }
	
    memset(ts_options, 0x0, sizeof(TS_OPTIONS_T));

   ts_options->pcr_period = 20;
   ts_options->psi_tx_count = PSI_TX_INTERVAL_NUM;
	ts_options->base_time =    27000000;
	
   	ts_options->ts_header.continuity_counter = 0;
	ts_options->adjust_num = 0;
	ts_options->real_rate = 0;
	ts_options->adjust_count = 4;
	ts_options->pts_delay_time = DEFAULT_PTS_DELAY_TIME;
	ts_options->max_pts_pcr_interval = PTS_PCR_MAX_INTERVAL_1s;
	ts_options->min_pts_pcr_interval = PTS_PCR_MIN_INTERVAL_1s;

	init_pes_header(&(ts_options->pes_header));
	
	return ts_options;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

/* EOF */

