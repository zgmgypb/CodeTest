/******************************************************************************

                  版权所有 (C), 2005-2017, GOSPELL有限公司

 ******************************************************************************
  文 件 名   : ts_psi.c
  版 本 号   : 初稿
  作    者   : 许斌
  生成日期   : 2017年7月14日
  最近修改   :
  功能描述   : 对编码器输出audio and video的ES流打包TS输出SPTS
  函数列表   :
  修改历史   :

   1.日    期          : 2017年7月14日
      作    者          : 许斌
      修改内容   : 创建文件

******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "ts_psi.h"
#include "ts.h"
#include "mpeg2.h"
#include "libc_assist.h"

static S32 HWL_EncPsiOutputCharsetCovert(MPEG2_PSICharSet CharSet, CHAR_T *pDes, S32 DesSize, CHAR_T *pSrc, S32 SrcSize, BOOL bSetTextMark)
{
	S32 lActualLen, lDesSize;
	CHAR_T *plDes;

	plDes = pDes;
	lDesSize = DesSize;
	lActualLen = 0;

	if (bSetTextMark) {
		lActualLen = MPEG2_PSIDescriptorTextDataCharsetPacker(plDes, lDesSize, CharSet);
		plDes += lActualLen;
		lDesSize -= lActualLen;
	}

	if (CharSet == MPEG2_PSI_CHAR_SET_UTF8) {
		if (lDesSize >= SrcSize) {
			GLOBAL_MEMCPY(plDes, pSrc, SrcSize);
			lActualLen += SrcSize;
		}
	}
	else if (CharSet >= MPEG2_PSI_CHAR_SET_ISO_8859_0 && CharSet <= MPEG2_PSI_CHAR_SET_ISO_8859_F) {
		lActualLen += CAL_CovertUTF8ToISO8859(CAL_COVERT_8859_0 + (CharSet - MPEG2_PSI_CHAR_SET_ISO_8859_0), (U8*)pSrc, SrcSize, (U8*)plDes, lDesSize);
	}
	else if (CharSet == MPEG2_PSI_CHAR_SET_GB2312) {
		lActualLen += CAL_CovertUTF8ToGB2312((U8*)pSrc, SrcSize, (U8*)plDes, lDesSize);
	}
	else if (CharSet == MPEG2_PSI_CHAR_SET_UCS_BIG) {
		lActualLen += CAL_CovertUTF8ToUCS((U8*)pSrc, SrcSize, (U8*)plDes, lDesSize, TRUE);
	}
	else if (CharSet == MPEG2_PSI_CHAR_SET_UCS_LITTLE) {
		lActualLen += CAL_CovertUTF8ToUCS((U8*)pSrc, SrcSize, (U8*)plDes, lDesSize, FALSE);
	}
	else {
		lActualLen = SrcSize;
	}

	return lActualLen;
}

static void HWL_EncPsiDelRightSpace(char *pStr)
{
	int32_t  lLen = strlen(pStr);

	while (lLen --)
	{
		if (*(pStr + lLen) != ' ')
		{
			return;
		}
		else
		{
			*(pStr + lLen) = '\0'; /* 结束符前移 */
		}
	}
}

/*****************************************************************************
* FUNCTION:ts_psi_CreatePat()
*
* DESCRIPTION:
*	    
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
void ts_psi_CreatePat(HWL_EncPatInfo *pat_Info, uint8_t	 *pat_continuity_counter, HWL_EncPsiInfo *pPsiInfo)
{
	uint32_t lIndex = 0;
	uint32_t lSectionLength;
	uint32_t i;
	uint32_t lCrc32Val;
	uint8_t	 lPatTable[MPEG2_PSI_MAX_PSI_SINGLE_SECTION_TS_BUF_SIZE];

	if (pat_Info->m_ProgramLen == 0)
		return;

	memset(lPatTable, 0xFF, sizeof(lPatTable));

	/* 创建TS头 */
	lPatTable[lIndex++] = pat_Info->m_SyncByteReplaceChar;
	lPatTable[lIndex++] = ((MPEG2_PSI_PAT_PID >> 8) & 0x1F) | 0x40;
	lPatTable[lIndex++] = MPEG2_PSI_PAT_PID & 0xFF;

	lPatTable[lIndex++] = 0x10 | ((*pat_continuity_counter) & 0xF); /* 仅有有效负载 */

#if 0
printf("\r\n PAT CC = %d", (*pat_continuity_counter));
#endif

   (*pat_continuity_counter)++;

	if ((*pat_continuity_counter) > 0xF)
	{
	    (*pat_continuity_counter) = 0;
	}
	
	lPatTable[lIndex++] = 0x00; /* Pointer_field */

	/* 创建PSI表头 */
	lPatTable[lIndex++] = MPEG2_PSI_PAT_TABLE_ID; /* PAT table ID */
	lSectionLength = 4 + 5 + 4 * pat_Info->m_ProgramLen; /* CRC占4个字节，TS_ID、VersionNum、section_number、last_section_num共5个字节，每个节目对应4个字节 */
	lPatTable[lIndex++] = 0xB0 | ((lSectionLength >> 8) & 0x0F);
	lPatTable[lIndex++] = lSectionLength & 0xFF;

	/* 创建PSI表内容 */
	lPatTable[lIndex++] = (pat_Info->m_TsId >> 8) & 0xFF; /* TS_ID */
	lPatTable[lIndex++] = pat_Info->m_TsId & 0xFF;
	lPatTable[lIndex++] = 0xC1 | (pat_Info->m_Version << 1); /* Version */
	lPatTable[lIndex++] = 0x00; /* section_number, 只有一个段，所以当前段号和最后段号都为0 */
	lPatTable[lIndex++] = 0x00; /* last_section_number */

	for( i=0; i<pat_Info->m_ProgramLen; i++ ) 								
	{
		lPatTable[lIndex++] = (pat_Info->m_ProgramInfo[i].m_ProgramNum >> 8) & 0xFF; /* 节目号 */
		lPatTable[lIndex++] = pat_Info->m_ProgramInfo[i].m_ProgramNum & 0xFF;
		
		lPatTable[lIndex++] = ((pat_Info->m_ProgramInfo[i].m_PmtPid >> 8) & 0x1F) | 0xE0; /* PMT PID */
		lPatTable[lIndex++] = pat_Info->m_ProgramInfo[i].m_PmtPid & 0xFF;
	}

	lCrc32Val = TS_Crc32Calculate(&lPatTable[5], lIndex - 5); /* 从Table_ID开始计算校验值 */
	lPatTable[lIndex++] = (lCrc32Val >> 24) & 0xFF;
	lPatTable[lIndex++] = (lCrc32Val >> 16) & 0xFF;
	lPatTable[lIndex++] = (lCrc32Val >> 8) & 0xFF;
	lPatTable[lIndex++] = lCrc32Val & 0xFF;

	memcpy(pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter], lPatTable, MPEG2_TS_PACKET_SIZE);
	pPsiInfo->m_PsiPacketCounter ++;

	/* 当pat表的大小大于一个TS包时，存入下一个TS包中 */
	if (lIndex > MPEG2_TS_PACKET_SIZE)
	{
		/* 重新建一个表头 */
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][0] = pat_Info->m_SyncByteReplaceChar;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][1] = (MPEG2_PSI_PAT_PID >> 8) & 0x1F;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][2] = MPEG2_PSI_PAT_PID & 0xFF;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][3] = 0x10; /* 仅有有效负载 */

		memcpy(&pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][4], &lPatTable[MPEG2_TS_PACKET_SIZE], MPEG2_TS_PACKET_SECTION_OTHER_PAYLOAD_SIZE);
		pPsiInfo->m_PsiPacketCounter ++;
	}
}

/*****************************************************************************
* FUNCTION:ts_psi_CreatePmt()
*
* DESCRIPTION:
*	    
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
void ts_psi_CreatePmt(HWL_EncPmtInfo *pPmtInfo, uint8_t  *pmt_continuity_counter, HWL_EncPsiInfo *pPsiInfo)
{
	uint8_t lPmtTable[MPEG2_TS_PACKET_SIZE];
	int32_t 	lIndex;
	int32_t   lSectionLength;
	uint32_t lCrc32Val;

	lIndex = 0;
	memset(lPmtTable, 0xFF, sizeof(lPmtTable));

	/* 创建TS头 */
	lPmtTable[lIndex++] = pPmtInfo->m_SyncByteReplaceChar;
	lPmtTable[lIndex++] = ((pPmtInfo->m_PmtPid >> 8) & 0x1F) | 0x40;
	lPmtTable[lIndex++] = pPmtInfo->m_PmtPid & 0xFF;
	lPmtTable[lIndex++] = 0x10 | ((*pmt_continuity_counter) & 0xF);  /* 仅有有效负载 */

#if 0
printf("\r\n PMT CC = %d", (*pmt_continuity_counter));
#endif

       (*pmt_continuity_counter)++;

	if ((*pmt_continuity_counter) > 0xF)
	{
	    *pmt_continuity_counter = 0;
	}
	
	lPmtTable[lIndex++] = 0x00; /* Pointer_field */

	lPmtTable[lIndex++] = MPEG2_PSI_PMT_TABLE_ID; /* PMT table ID */

	switch (pPmtInfo->m_VidEncMode)
	{
	case PSI_VID_ENC_MODE_AVS:
	case PSI_VID_ENC_MODE_AVS_PLUS:
		lSectionLength = 0x06;
		break;
	default: 
		lSectionLength = 0x00;
		break;
	}

	switch (pPmtInfo->m_AudEncMode)
	{
	case PSI_AUD_ENC_MODE_MPEG1_L2:
#if 1 //xubin closed [ps]: It only support Audio.
		lSectionLength += 0x17;
#else
      lSectionLength += (0x17 - 5);
#endif
		break;

	case PSI_AUD_ENC_MODE_AAC:
		lSectionLength += 0x1A;
		break;
	case PSI_AUD_ENC_MODE_AC3:
	case PSI_AUD_ENC_MODE_EAC3:
		lSectionLength += 0x1A;
		break;
	case PSI_AUD_ENC_MODE_DRA_2_0:
	case PSI_AUD_ENC_MODE_DRA_5_1:
		lSectionLength += 0x21;
		break;
	default: 
		break;
	}

	lPmtTable[lIndex++] = ((lSectionLength >> 8) & 0x0F) | 0xB0; /* 段长度 */
	lPmtTable[lIndex++] = lSectionLength & 0xFF;

	lPmtTable[lIndex++] = (pPmtInfo->m_ProgramNum >> 8) & 0xFF; /* program_number */
	lPmtTable[lIndex++] = pPmtInfo->m_ProgramNum & 0xFF;

	lPmtTable[lIndex++] = 0xC1 | (pPmtInfo->m_Version << 1);	 /* Res., version, current_next */
	lPmtTable[lIndex++] = 0x00; /* Section number */
	lPmtTable[lIndex++] = 0x00; /* Last section number */

	lPmtTable[lIndex++] = ((pPmtInfo->m_PcrPid >> 8) & 0x1F) | 0xE0; /* PCR_PID */
	lPmtTable[lIndex++] = pPmtInfo->m_PcrPid & 0xFF;

	lPmtTable[lIndex++] = 0xF0; /* Res (4), Program_info_length [11:8] */
	lPmtTable[lIndex++] = 0x00; /* Program_info_length [7:0] */

#if 1 //xubin closed [ps]: It only support Audio.
	lPmtTable[lIndex++] = MPEG2_STREAM_TYPE_H264_VIDEO; /* Stream_type: HEVC */
	lPmtTable[lIndex++] = ((pPmtInfo->m_VidPid >> 8) & 0x1F) | 0xE0; /* Res(3),Elementary_PID [12:8] */
	lPmtTable[lIndex++] = pPmtInfo->m_VidPid & 0xFF; /* Elementary_PID [7:0] */
	lPmtTable[lIndex++] = 0xF0; /* Res (4), ES_info_length [11:8] */


	switch (pPmtInfo->m_VidEncMode)
	{
	case PSI_VID_ENC_MODE_AVS:
	case PSI_VID_ENC_MODE_AVS_PLUS:
		lPmtTable[lIndex++] = 0x06; /* ES_info_length [7:0] */

		lPmtTable[lIndex++] = 0x3F; /* descriptive tag; */ 
		lPmtTable[lIndex++] = 0x04;	 /* descriptive length */

		lPmtTable[lIndex++] = 0x48; /* content */
		lPmtTable[lIndex++] = 0x00;
		lPmtTable[lIndex++] = 0x19;
		lPmtTable[lIndex++] = 0x20;
		break;
	default: 
		lPmtTable[lIndex++] = 0x00; /* ES_info_length [7:0] */
		break;
	}
#endif

	switch (pPmtInfo->m_AudEncMode)
	{
	case PSI_AUD_ENC_MODE_MPEG1_L2:
		lPmtTable[lIndex++] = 0x03;
		break;
	case PSI_AUD_ENC_MODE_AAC:
		lPmtTable[lIndex++] = 0x0F;
		break;
	case PSI_AUD_ENC_MODE_AC3:
	case PSI_AUD_ENC_MODE_EAC3:
		lPmtTable[lIndex++] = 0x06;
		break;
	case PSI_AUD_ENC_MODE_DRA_2_0:
	case PSI_AUD_ENC_MODE_DRA_5_1:
		lPmtTable[lIndex++] = 0x06;
		break;
	default: 
		break;
	}
	lPmtTable[lIndex++] = ((pPmtInfo->m_AudPid >> 8) & 0x1F) | 0xE0; /* Res(3),Elementary_PID [12:8] */
	lPmtTable[lIndex++] = pPmtInfo->m_AudPid & 0xFF; /* Elementary_PID [7:0] */
	lPmtTable[lIndex++] = 0xF0; /* Res (4), ES_info_length [11:8] */

	switch (pPmtInfo->m_AudEncMode)
	{
	case PSI_AUD_ENC_MODE_MPEG1_L2:
		lPmtTable[lIndex++] = 0x0; /* ES_info_length [7:0] */
		break;
	case PSI_AUD_ENC_MODE_AAC:
		lPmtTable[lIndex++] = 0x03; /* ES_info_length [7:0] */
		lPmtTable[lIndex++] = 0x52;
		lPmtTable[lIndex++] = 0x01;
		lPmtTable[lIndex++] = 0x10;
		break;
	case PSI_AUD_ENC_MODE_AC3:
		lPmtTable[lIndex++] = 0x03;	/* ES_info_length [7:0] */	
		lPmtTable[lIndex++] = 0x6A;		
		lPmtTable[lIndex++] = 0x01;
		lPmtTable[lIndex++] = 0x00;
		break;
	case PSI_AUD_ENC_MODE_EAC3:
		lPmtTable[lIndex++] = 0x03;    /* ES_info_length [7:0] */   
		lPmtTable[lIndex++] = 0x7A;        
		lPmtTable[lIndex++] = 0x01;
		lPmtTable[lIndex++] = 0x00;
		break;

	case PSI_AUD_ENC_MODE_DRA_2_0:
	case PSI_AUD_ENC_MODE_DRA_5_1:
		lPmtTable[lIndex++] = 0x0A;	/* ES_info_length [7:0] */

		lPmtTable[lIndex++] = 0x05; /* tag */
		lPmtTable[lIndex++] = 0x04;	/* descriptive length */
		lPmtTable[lIndex++] = 0x44;	/* descriptive content */
		lPmtTable[lIndex++] = 0x52;		
		lPmtTable[lIndex++] = 0x41;		
		lPmtTable[lIndex++] = 0x31;		

		lPmtTable[lIndex++] = 0xA0;	/* tag */
		lPmtTable[lIndex++] = 0x02;	/* descriptive length */
		lPmtTable[lIndex++] = 0x81;	/* descriptive content */
		lPmtTable[lIndex++] = 0x40;	
		break;
	default: 
		break;
	}

	lCrc32Val = TS_Crc32Calculate(&lPmtTable[5], (lIndex - 5));
	lPmtTable[lIndex++] = (lCrc32Val >> 24) & 0xFF;	
	lPmtTable[lIndex++] = (lCrc32Val >> 16) & 0xFF;	
	lPmtTable[lIndex++] = (lCrc32Val >> 8) & 0xFF;	
	lPmtTable[lIndex++] = lCrc32Val & 0xFF;		

	memcpy(pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter++], lPmtTable, MPEG2_TS_PACKET_SIZE);
}

/*****************************************************************************
* FUNCTION:ts_psi_CreateSdt()
*
* DESCRIPTION:
*	    
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
void ts_psi_CreateSdt(HWL_EncSdtInfo *pSdtInfo, uint8_t *sdt_continuity_counter, HWL_EncPsiInfo *pPsiInfo)
{
	uint8_t	 lSdtTable[MPEG2_PSI_MAX_PSI_SINGLE_SECTION_TS_BUF_SIZE]; 
	uint32_t lIndex = 0, lDescLoopLenIndex, lDescLenIndex, lServiceNameLenIndex, lSectionLenIndex;
	uint32_t i, j;
	uint32_t lServiceNameLen;
	char lProgramName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN];
	uint32_t lCrc32Val;

	if (pSdtInfo->m_ProgramLen == 0)
		return;

	memset(lSdtTable, 0xFF, sizeof(lSdtTable)); /* 对于节目信息表，没有意义的位填充0xFF */

	/* 创建TS头 */
	lSdtTable[lIndex++] = pSdtInfo->m_SyncByteReplaceChar;
	lSdtTable[lIndex++] = ((MPEG2_PSI_SDT_BAT_PID >> 8) & 0x1F) | 0x40;
	lSdtTable[lIndex++] = MPEG2_PSI_SDT_BAT_PID & 0xFF;
	lSdtTable[lIndex++] = 0x10 | ((*sdt_continuity_counter) & 0xF);  /* 仅有有效负载 */
	(*sdt_continuity_counter)++;
	if ((*sdt_continuity_counter) > 0xF)
	{
		*sdt_continuity_counter = 0;
	}

	lSdtTable[lIndex++] = 0x00; /* Pointer_field */

	lSdtTable[lIndex++] = MPEG2_PSI_SDT_ACUTUAL_TABLE_ID; /* table_id */
	lSectionLenIndex = lIndex;
	lSdtTable[lIndex++] = 0; /* bit7:section syntax indictor ; bit6-bit4:reseve; bit3-bit0:section length */
	lSdtTable[lIndex++] = 0; /* section length */
	lSdtTable[lIndex++] = (pSdtInfo->m_TsId >> 8) & 0xFF;
	lSdtTable[lIndex++] = pSdtInfo->m_TsId & 0xFF;
	lSdtTable[lIndex++] = 0xC1 | (pSdtInfo->m_Version << 1); /* bit7-6:reseve;bit5-bit1:version number = 1  ,bit0: current_next_indicator = 1 */
	lSdtTable[lIndex++] = 0x00; /* section_number */
	lSdtTable[lIndex++] = 0x00; /* last_section_number */
	lSdtTable[lIndex++] = (pSdtInfo->m_OnId >> 8) & 0xFF; /* original_network_id */
	lSdtTable[lIndex++] = pSdtInfo->m_OnId & 0xFF;
	lSdtTable[lIndex++] = 0xFF; /* reserved_future_use */

	for (i=0; i<pSdtInfo->m_ProgramLen; i++)
	{
		lSdtTable[lIndex++] = (pSdtInfo->m_ProgramInfo[i].m_ProgramNum >> 8) & 0xFF; /* 节目号 */
		lSdtTable[lIndex++] = pSdtInfo->m_ProgramInfo[i].m_ProgramNum & 0xFF;

		lSdtTable[lIndex++] = 0xFD; /* bit7-2:reseve:bit1:EIT_schedult_flag = 0 ,bit0: EIT_present_following_flag = 1 */
		lSdtTable[lIndex++] = 0x80; /* bit7-5:running_status = 100b ,bit4: free_CA_mode = 0 ,bit3-bit0: descriptors_loop_length_high = 0 */
		lDescLoopLenIndex = lIndex;
		lSdtTable[lIndex++] = 0x00; /* descriptors_loop_length */
		lSdtTable[lIndex++] = MPEG2_PSI_SERVICE_DESCRIPTOR_TAG; /* descriptor_tag:service_descriptor 0x48 */
		lDescLenIndex = lIndex;
		lSdtTable[lIndex++] = 0x00; /* descriptor_length */
		lSdtTable[lIndex++] = MPEG2_SERVICE_TYPE_DTV; /* service_type: 数字电视业务 0x01 */
		lSdtTable[lIndex++] = 0x00; /* service_provider_name_length */
		lServiceNameLenIndex = lIndex;
		lSdtTable[lIndex++] = 0x00; /* service_name_length */

		memset(lProgramName, 0, sizeof(lProgramName));
		HWL_EncPsiOutputCharsetCovert(pSdtInfo->m_Charset, lProgramName, sizeof(lProgramName), 
			pSdtInfo->m_ProgramInfo[i].m_pProgramName, strlen(pSdtInfo->m_ProgramInfo[i].m_pProgramName) + 1, FALSE);
	
		HWL_EncPsiDelRightSpace(lProgramName);
		lServiceNameLen = strlen(lProgramName);
		lServiceNameLen ++; /* 结束符 */
		lServiceNameLen = (lServiceNameLen > MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN ? MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN : lServiceNameLen);
		for (j=0; j<lServiceNameLen; j++)
		{
			lSdtTable[lIndex++] = lProgramName[j];
		}
		lSdtTable[lDescLoopLenIndex] = lServiceNameLen + 5; /* descriptors_loop_length */
		lSdtTable[lDescLenIndex] = lServiceNameLen + 3; /* descriptor_length */
		lSdtTable[lServiceNameLenIndex] = lServiceNameLen; /* service_name_length */
	}
	/* 段长度 = 索引值 - TS头(4字节) - Pointer_Field(1字节) - Tabel_ID(1字节) - 段长度(2字节) + CRC32(4字节) */
	lSdtTable[lSectionLenIndex] = ((lIndex - MPEG2_TS_PACKET_HEAD_LEN - 3 - 1 + 4) >> 8) | 0xB0; /* bit7:section syntax indictor=1; bit6-bit4:reseve 为1; bit3-bit0:section length */
	lSdtTable[lSectionLenIndex + 1] = (lIndex - MPEG2_TS_PACKET_HEAD_LEN - 3 - 1 + 4) & 0xFF; 

	lCrc32Val = TS_Crc32Calculate(&lSdtTable[5], lIndex - 5); /* 从Table_ID开始计算校验值 */
	lSdtTable[lIndex++] = (lCrc32Val >> 24) & 0xFF;
	lSdtTable[lIndex++] = (lCrc32Val >> 16) & 0xFF;
	lSdtTable[lIndex++] = (lCrc32Val >> 8) & 0xFF;
	lSdtTable[lIndex++] = lCrc32Val & 0xFF;

	memcpy(pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter], lSdtTable, MPEG2_TS_PACKET_SIZE);
	pPsiInfo->m_PsiPacketCounter ++;

	for (i=MPEG2_TS_PACKET_SIZE; i<lIndex; i+=MPEG2_TS_PACKET_SECTION_OTHER_PAYLOAD_SIZE)
	{
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][0] = pSdtInfo->m_SyncByteReplaceChar;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][1] = (MPEG2_PSI_SDT_BAT_PID >> 8) & 0x0F;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][2] = MPEG2_PSI_SDT_BAT_PID & 0xFF;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][3] = 0x10; /* 仅有有效负载 */
		memcpy(&pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][4], &lSdtTable[i], MPEG2_TS_PACKET_SECTION_OTHER_PAYLOAD_SIZE);
		pPsiInfo->m_PsiPacketCounter ++; 
	}
}

/*****************************************************************************
* FUNCTION:ts_psi_CreateNullCat()
*
* DESCRIPTION:
*	    
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
void ts_psi_CreateNullCat(uint8_t	 *cat_continuity_counter, uint8_t *pPsiPacket)
{
	uint32_t lIndex = 0;
	
	//GLOBAL_ASSERT(pPsiPacket);
	
	memset(pPsiPacket, 0xFF, MPEG2_TS_PACKET_SIZE);

	pPsiPacket[lIndex++] = MPEG2_TS_PACKET_SYN_BYTE;
	pPsiPacket[lIndex++] = 0x40 | ((MPEG2_PSI_CAT_PID >> 8) & 0xFF); /* 表示带有point_field标志 */
	pPsiPacket[lIndex++] = MPEG2_PSI_CAT_PID & 0xFF; /* PID为0x01表示负载为CAT表 */
	pPsiPacket[lIndex++] = 0x10 |  ((*cat_continuity_counter) & 0xF); /* 仅含有效负载 */
	if (++(*cat_continuity_counter) > 0x0f) {
		*cat_continuity_counter = 0;
	}

	pPsiPacket[lIndex++] = 0x00; /* point_field标志 */
	pPsiPacket[lIndex++] = MPEG2_PSI_CAT_TABLE_ID; /* Tabel ID为0x01表示CAT */
	pPsiPacket[lIndex++] = 0x80;
	pPsiPacket[lIndex++] = 0x09;
	pPsiPacket[lIndex++] = 0x00;
	pPsiPacket[lIndex++] = 0x01; /* TS_ID: 0x01 */
	pPsiPacket[lIndex++] = 0x01;
	pPsiPacket[lIndex++] = 0x00; /* CAT为空表，没有数据内容 */
	pPsiPacket[lIndex++] = 0x00;

	pPsiPacket[lIndex++] = 0x00; /* CRC, 4字节 */
	pPsiPacket[lIndex++] = 0xA8;
	pPsiPacket[lIndex++] = 0xD9;
	pPsiPacket[lIndex++] = 0x7E;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

/* Eof */
