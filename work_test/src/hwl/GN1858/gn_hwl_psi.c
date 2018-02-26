#include "gn_hwl.h"

/* CRC表，用于计算PSI表的CRC计算 */
#define CRC32_MAX_COEFFICIENTS	256
static const U32 sc_Crc32Table[CRC32_MAX_COEFFICIENTS]=
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

/* CRC32校验计算 */
static U32 ENC_Crc32Calculate(U8 *pData, U32 Size)
{
	U32 lCrc32Val = 0xFFFFFFFF;	
	U32 i;

	for (i=0; i<Size; i++)	
	{
		lCrc32Val = (lCrc32Val << 8 ) ^ sc_Crc32Table[((lCrc32Val >> 24) ^ *(pData++)) & 0xFF]; 
	}

	return lCrc32Val;
}

static S32 MPEG2L_DBOutputCharsetCovert2(U16 CharSet, CHAR_T *pDes, S32 DesSize, CHAR_T *pSrc, S32 SrcSize, BOOL bSetTextMark)
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

	return lActualLen;
}

static void HWL_EncPsiDelRightSpace(CHAR_T *pStr)
{
	S32 lLen = GLOBAL_STRLEN(pStr);

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

void HWL_EncPsiCreatePat(PSI_PatInfo *pPatInfo, PSI_PacketInfo *pPsiInfo)
{
	U32 lIndex = 0;
	U32 lSectionLength;
	U32 i;
	U32 lCrc32Val;
	U8	 lPatTable[MPEG2_PSI_MAX_PSI_SINGLE_SECTION_TS_BUF_SIZE];

	if (pPatInfo->m_ProgramLen == 0) /* 没有节目不会创建PAT表 */
		return;

	GLOBAL_MEMSET(lPatTable, 0xFF, sizeof(lPatTable));

	/* 创建TS头 */
	lPatTable[lIndex++] = pPatInfo->m_SyncByteReplaceChar;
	lPatTable[lIndex++] = ((MPEG2_PSI_PAT_PID >> 8) & 0x1F) | 0x40;
	lPatTable[lIndex++] = MPEG2_PSI_PAT_PID & 0xFF;
	lPatTable[lIndex++] = 0x10; /* 仅有有效负载 */

	lPatTable[lIndex++] = 0x00; /* Pointer_field */

	/* 创建PSI表头 */
	lPatTable[lIndex++] = MPEG2_PSI_PAT_TABLE_ID; /* PAT table ID */
	lSectionLength = 4 + 5 + 4 * pPatInfo->m_ProgramLen; /* CRC占4个字节，TS_ID、VersionNum、section_number、last_section_num共5个字节，每个节目对应4个字节 */
	lPatTable[lIndex++] = 0xB0 | ((lSectionLength >> 8) & 0x0F);
	lPatTable[lIndex++] = lSectionLength & 0xFF;

	/* 创建PSI表内容 */
	lPatTable[lIndex++] = (pPatInfo->m_TsId >> 8) & 0xFF; /* TS_ID */
	lPatTable[lIndex++] = pPatInfo->m_TsId & 0xFF;
	lPatTable[lIndex++] = 0xC1 | (pPatInfo->m_Version << 1); /* Version */
	lPatTable[lIndex++] = 0x00; /* section_number, 只有一个段，所以当前段号和最后段号都为0 */
	lPatTable[lIndex++] = 0x00; /* last_section_number */

	for( i=0; i<pPatInfo->m_ProgramLen; i++ ) 								
	{
		lPatTable[lIndex++] = (pPatInfo->m_ProgramInfo[i].m_ProgramNum >> 8) & 0xFF; /* 节目号 */
		lPatTable[lIndex++] = pPatInfo->m_ProgramInfo[i].m_ProgramNum & 0xFF;

		lPatTable[lIndex++] = ((pPatInfo->m_ProgramInfo[i].m_PmtPid >> 8) & 0x1F) | 0xE0; /* PMT PID */
		lPatTable[lIndex++] = pPatInfo->m_ProgramInfo[i].m_PmtPid & 0xFF;
	}

	lCrc32Val = ENC_Crc32Calculate(&lPatTable[5], lIndex - 5); /* 从Table_ID开始计算校验值 */
	lPatTable[lIndex++] = (lCrc32Val >> 24) & 0xFF;
	lPatTable[lIndex++] = (lCrc32Val >> 16) & 0xFF;
	lPatTable[lIndex++] = (lCrc32Val >> 8) & 0xFF;
	lPatTable[lIndex++] = lCrc32Val & 0xFF;

	GLOBAL_MEMCPY(pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter], lPatTable, MPEG2_TS_PACKET_SIZE);
	pPsiInfo->m_PsiPacketCounter ++;

	/* 当pat表的大小大于一个TS包时，存入下一个TS包中 */
	if (lIndex > MPEG2_TS_PACKET_SIZE)
	{
		/* 重新建一个表头 */
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][0] = pPatInfo->m_SyncByteReplaceChar;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][1] = (MPEG2_PSI_PAT_PID >> 8) & 0x1F;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][2] = MPEG2_PSI_PAT_PID & 0xFF;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][3] = 0x10; /* 仅有有效负载 */

		GLOBAL_MEMCPY(&pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][4], &lPatTable[MPEG2_TS_PACKET_SIZE], MPEG2_TS_PACKET_SECTION_OTHER_PAYLOAD_SIZE);
		pPsiInfo->m_PsiPacketCounter ++;
	}
}

void HWL_EncPsiCreateSdt(PSI_SdtInfo *pSdtInfo, PSI_PacketInfo *pPsiInfo)
{
	U8	 lSdtTable[MPEG2_PSI_MAX_PSI_SINGLE_SECTION_TS_BUF_SIZE]; 
	U32 lIndex = 0, lDescLoopLenIndex, lDescLenIndex, lServiceNameLenIndex, lSectionLenIndex;
	U32 i, j;
	U32 lServiceNameLen;
	CHAR_T lProgramName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN];
	U32 lCrc32Val;

	if (pSdtInfo->m_ProgramLen == 0) /* 没有节目不会创建SDT表 */
		return;

	GLOBAL_MEMSET(lSdtTable, 0xFF, sizeof(lSdtTable)); /* 对于节目信息表，没有意义的位填充0xFF */

	/* 创建TS头 */
	lSdtTable[lIndex++] = pSdtInfo->m_SyncByteReplaceChar;
	lSdtTable[lIndex++] = ((MPEG2_PSI_SDT_BAT_PID >> 8) & 0x1F) | 0x40;
	lSdtTable[lIndex++] = MPEG2_PSI_SDT_BAT_PID & 0xFF;
	lSdtTable[lIndex++] = 0x10; /* 仅有有效负载 */

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

		GLOBAL_MEMSET(lProgramName, 0, sizeof(lProgramName));
		MPEG2L_DBOutputCharsetCovert2(pSdtInfo->m_Charset, lProgramName, sizeof(lProgramName), 
			pSdtInfo->m_ProgramInfo[i].m_pProgramName, GLOBAL_STRLEN(pSdtInfo->m_ProgramInfo[i].m_pProgramName) + 1, FALSE);

		HWL_EncPsiDelRightSpace(lProgramName);
		lServiceNameLen = GLOBAL_STRLEN(lProgramName);
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

	lCrc32Val = ENC_Crc32Calculate(&lSdtTable[5], lIndex - 5); /* 从Table_ID开始计算校验值 */
	lSdtTable[lIndex++] = (lCrc32Val >> 24) & 0xFF;
	lSdtTable[lIndex++] = (lCrc32Val >> 16) & 0xFF;
	lSdtTable[lIndex++] = (lCrc32Val >> 8) & 0xFF;
	lSdtTable[lIndex++] = lCrc32Val & 0xFF;

	GLOBAL_MEMCPY(pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter], lSdtTable, MPEG2_TS_PACKET_SIZE);
	pPsiInfo->m_PsiPacketCounter ++;

	for (i=MPEG2_TS_PACKET_SIZE; i<lIndex; i+=MPEG2_TS_PACKET_SECTION_OTHER_PAYLOAD_SIZE)
	{
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][0] = pSdtInfo->m_SyncByteReplaceChar;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][1] = (MPEG2_PSI_SDT_BAT_PID >> 8) & 0x0F;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][2] = MPEG2_PSI_SDT_BAT_PID & 0xFF;
		pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][3] = 0x10; /* 仅有有效负载 */
		GLOBAL_MEMCPY(&pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter][4], &lSdtTable[i], MPEG2_TS_PACKET_SECTION_OTHER_PAYLOAD_SIZE);
		pPsiInfo->m_PsiPacketCounter ++; 
	}
}

/* 创建一个CAT表，但这里创建的是一个空的CAT表 */
void HWL_EncPsiCreateNullCat(U8 pPsiPacket[MPEG2_TS_PACKET_SIZE])
{
	U32 lIndex = 0;

	GLOBAL_ASSERT(pPsiPacket);

	GLOBAL_MEMSET(pPsiPacket, 0xFF, MPEG2_TS_PACKET_SIZE);

	pPsiPacket[lIndex++] = MPEG2_TS_PACKET_SYN_BYTE;
	pPsiPacket[lIndex++] = 0x40 | ((MPEG2_PSI_CAT_PID >> 8) & 0xFF); /* 表示带有point_field标志 */
	pPsiPacket[lIndex++] = MPEG2_PSI_CAT_PID & 0xFF; /* PID为0x01表示负载为CAT表 */
	pPsiPacket[lIndex++] = 0x10; /* 仅含有效负载 */

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

void HWL_EncPsiCreatePmt(PSI_PmtInfo *pPmtInfo, PSI_PacketInfo *pPsiInfo)
{
	CHAR_T lPmtTable[MPEG2_TS_PACKET_SIZE];
	S32	lIndex;
	S32 lSectionLength;
	U32 lCrc32Val;

	lIndex = 0;
	GLOBAL_MEMSET(lPmtTable, 0xFF, sizeof(lPmtTable));

	/* 创建TS头 */
	lPmtTable[lIndex++] = pPmtInfo->m_SyncByteReplaceChar;
	lPmtTable[lIndex++] = ((pPmtInfo->m_PmtPid >> 8) & 0x1F) | 0x40;
	lPmtTable[lIndex++] = pPmtInfo->m_PmtPid & 0xFF;
	lPmtTable[lIndex++] = 0x10; /* 仅有有效负载 */

	lPmtTable[lIndex++] = 0x00; /* Pointer_field */

	lPmtTable[lIndex++] = MPEG2_PSI_PMT_TABLE_ID; /* PMT table ID */

	switch (pPmtInfo->m_VidEncMode)
	{
	case HWL_VID_ENC_MPEG2:
	case HWL_VID_ENC_H264:
		lSectionLength = 0x00;
		break;

	case HWL_VID_ENC_AVS:
	case HWL_VID_ENC_AVS_PLUS:
		lSectionLength = 0x06;
		break;

	default: 
		lSectionLength = 0x00;
		break;
	}

	switch (pPmtInfo->m_AudEncMode)
	{
	case HWL_AUD_ENC_MPEG1_L2:
		lSectionLength += 0x17;
		break;
	case HWL_AUD_ENC_MPEG4_AAC:
	case HWL_AUD_ENC_MPEG2_AAC:
	case HWL_AUD_ENC_AC3:
	case HWL_AUD_ENC_EAC3:
		lSectionLength += 0x1A;
		break;
	case HWL_AUD_ENC_DRA_2_0:
	case HWL_AUD_ENC_DRA_5_1:
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

	switch (pPmtInfo->m_VidEncMode)
	{
	case HWL_VID_ENC_MPEG2:
		lPmtTable[lIndex++] = MPEG2_STREAM_TYPE_MPEG2_VIDEO; /* Stream_type: For MPEG-2 Video */
		break;
	case HWL_VID_ENC_H264:
		lPmtTable[lIndex++] = MPEG2_STREAM_TYPE_H264_VIDEO; /* Stream_type: For AVV/H.264 Video */
		break;
	case HWL_VID_ENC_AVS:
	case HWL_VID_ENC_AVS_PLUS:
		lPmtTable[lIndex++] = MPEG2_STREAM_TYPE_AVS_VIDEO; /* Stream_type: For AVS/AVS+ Video */
		break;
	default: 
		lPmtTable[lIndex++] = 0x00; /* undefine */
		break;
	}

	lPmtTable[lIndex++] = ((pPmtInfo->m_VidPid >> 8) & 0x1F) | 0xE0; /* Res(3),Elementary_PID [12:8] */
	lPmtTable[lIndex++] = pPmtInfo->m_VidPid & 0xFF; /* Elementary_PID [7:0] */
	lPmtTable[lIndex++] = 0xF0; /* Res (4), ES_info_length [11:8] */

	switch (pPmtInfo->m_VidEncMode)
	{
	case HWL_VID_ENC_MPEG2:
	case HWL_VID_ENC_H264:
		lPmtTable[lIndex++] = 0x00; /* Tag length */
		break;
	case HWL_VID_ENC_AVS:
	case HWL_VID_ENC_AVS_PLUS:
		lPmtTable[lIndex++] = 0x06; /* Tag length */

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

	switch (pPmtInfo->m_AudEncMode)
	{
	case HWL_AUD_ENC_MPEG1_L2:
		lPmtTable[lIndex++] = 0x03;
		break;
	case HWL_AUD_ENC_MPEG2_AAC:
	case HWL_AUD_ENC_MPEG4_AAC:
		lPmtTable[lIndex++] = 0x0F;
		break;
	case HWL_AUD_ENC_AC3:
	case HWL_AUD_ENC_EAC3:
		lPmtTable[lIndex++] = 0x06;
		break;
	case HWL_AUD_ENC_DRA_2_0:
	case HWL_AUD_ENC_DRA_5_1:
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
	case HWL_AUD_ENC_MPEG1_L2:
		lPmtTable[lIndex++] = 0x0; /* ES_info_length [7:0] */
		break;
	case HWL_AUD_ENC_MPEG2_AAC:
	case HWL_AUD_ENC_MPEG4_AAC:
		lPmtTable[lIndex++] = 0x03; /* ES_info_length [7:0] */
		lPmtTable[lIndex++] = 0x52;
		lPmtTable[lIndex++] = 0x01;
		lPmtTable[lIndex++] = 0x10;
		break;
	case HWL_AUD_ENC_AC3:
		lPmtTable[lIndex++] = 0x03;	/* ES_info_length [7:0] */	
		lPmtTable[lIndex++] = 0x6A;		
		lPmtTable[lIndex++] = 0x01;
		lPmtTable[lIndex++] = 0x00;
		break;
	case HWL_AUD_ENC_EAC3:
		lPmtTable[lIndex++] = 0x03;	/* ES_info_length [7:0] */	
		lPmtTable[lIndex++] = 0x7A;		
		lPmtTable[lIndex++] = 0x01;
		lPmtTable[lIndex++] = 0x00;
		break;
	case HWL_AUD_ENC_DRA_2_0:
	case HWL_AUD_ENC_DRA_5_1:
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

	lCrc32Val = ENC_Crc32Calculate(&lPmtTable[5], (lIndex - 5));
	lPmtTable[lIndex++] = (lCrc32Val >> 24) & 0xFF;	
	lPmtTable[lIndex++] = (lCrc32Val >> 16) & 0xFF;	
	lPmtTable[lIndex++] = (lCrc32Val >> 8) & 0xFF;	
	lPmtTable[lIndex++] = lCrc32Val & 0xFF;		

	GLOBAL_MEMCPY(pPsiInfo->m_pPsiPacket[pPsiInfo->m_PsiPacketCounter++], lPmtTable, MPEG2_TS_PACKET_SIZE);
}

static void PSI_FillNullPacket(MFPGA_PsiInsertParam *pPsiInsertPara, U32 StartNum)
{
	S32 i, j;

	for (i=StartNum; i<MFPGA_MAX_PSI_NUM; i++)
	{
		pPsiInsertPara->m_Data[i][0] = MPEG2_TS_PACKET_SYN_BYTE;
		pPsiInsertPara->m_Data[i][1] = 0x1F;
		pPsiInsertPara->m_Data[i][2] = 0xFF;
		pPsiInsertPara->m_Data[i][3] = 0x10;

		for (j=4; j<MPEG2_TS_PACKET_SIZE; j++)
		{
			pPsiInsertPara->m_Data[i][j] = 0xFF; 
		}
	}
}

/* 设置编码PSI表到主板 */
BOOL PSI_SetEncPsiParamToHw(PSI_CreateParam *pPsiParam) 
{
	PSI_TableCreatePara lPsiCreatePara;
	PSI_PacketInfo *plPsiTableInfo; /* 创建的PSI表 */
	PSI_TsParam *plEncOutInfo;
	PSI_ChProgParam *plEncPara;
	int j;
	static U8 lPatVersion = 0, lSdtVersion = 0, lPmtVersion = 0;  

	lPatVersion = (lPatVersion > 31 ? 0 : (lPatVersion + 1)); /* 当同一个输出端口的流发生PSI表变化时，其版本号变化 */
	lSdtVersion = (lSdtVersion > 31 ? 0 : (lSdtVersion + 1));
	lPmtVersion = (lPmtVersion > 31 ? 0 : (lPmtVersion + 1));

	plEncOutInfo = &pPsiParam->m_TsParam;

	plPsiTableInfo = (PSI_PacketInfo *)GLOBAL_MALLOC(sizeof(PSI_PacketInfo));
	if (!plPsiTableInfo)
	{
		GLOBAL_TRACE(("malloc err!\n"));
		return FALSE;
	}
	plPsiTableInfo->m_PsiPacketCounter = 0; /* 初始化表 */

	/* MPTS模式下建表，MPTS建一个PAT表，建一个SDT表，建多个PMT表（恒定MPTS模式） */
	{
		lPsiCreatePara.m_PatInfo.m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
		lPsiCreatePara.m_PatInfo.m_TsId = plEncOutInfo->m_TsId;
		lPsiCreatePara.m_PatInfo.m_Version = lPatVersion;
		lPsiCreatePara.m_PatInfo.m_ProgramLen = 0;
		for (j=0; j<GN_ENC_CH_NUM; j++)
		{
			plEncPara = &pPsiParam->m_ProgParam[j];
			if (plEncPara->m_WorkEn == TRUE)
			{
				lPsiCreatePara.m_PatInfo.m_ProgramInfo[lPsiCreatePara.m_PatInfo.m_ProgramLen].m_PmtPid = plEncPara->m_PmtPid;
				lPsiCreatePara.m_PatInfo.m_ProgramInfo[lPsiCreatePara.m_PatInfo.m_ProgramLen].m_ProgramNum = plEncPara->m_ServiceId;
				lPsiCreatePara.m_PatInfo.m_ProgramLen ++;
			}
		}
		HWL_EncPsiCreatePat(&lPsiCreatePara.m_PatInfo, plPsiTableInfo);

		lPsiCreatePara.m_SdtInfo.m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
		lPsiCreatePara.m_SdtInfo.m_TsId = plEncOutInfo->m_TsId;
		lPsiCreatePara.m_SdtInfo.m_OnId = plEncOutInfo->m_OnId;
		lPsiCreatePara.m_SdtInfo.m_Version = lSdtVersion;
		lPsiCreatePara.m_SdtInfo.m_Charset = plEncOutInfo->m_Charset;
		lPsiCreatePara.m_SdtInfo.m_ProgramLen = 0;
		for (j=0; j<GN_ENC_CH_NUM; j++)
		{
			plEncPara = &pPsiParam->m_ProgParam[j];
			if (plEncPara->m_WorkEn == TRUE)
			{
				lPsiCreatePara.m_SdtInfo.m_ProgramInfo[lPsiCreatePara.m_SdtInfo.m_ProgramLen].m_PmtPid = plEncPara->m_PmtPid;
				lPsiCreatePara.m_SdtInfo.m_ProgramInfo[lPsiCreatePara.m_SdtInfo.m_ProgramLen].m_ProgramNum = plEncPara->m_ServiceId;
				GLOBAL_STRCPY(lPsiCreatePara.m_SdtInfo.m_ProgramInfo[lPsiCreatePara.m_SdtInfo.m_ProgramLen].m_pProgramName, plEncPara->m_pServiceName);
				lPsiCreatePara.m_SdtInfo.m_ProgramLen ++;
			}
		}
		HWL_EncPsiCreateSdt(&lPsiCreatePara.m_SdtInfo, plPsiTableInfo);

		for (j=0; j<GN_ENC_CH_NUM; j++)
		{
			plEncPara = &pPsiParam->m_ProgParam[j];
			if (plEncPara->m_WorkEn == TRUE)
			{
				lPsiCreatePara.m_PmtInfo.m_SyncByteReplaceChar = MPEG2_TS_PACKET_SYN_BYTE;
				lPsiCreatePara.m_PmtInfo.m_Version = lPmtVersion;
				lPsiCreatePara.m_PmtInfo.m_VidEncMode = plEncPara->m_VidEncMode;
				lPsiCreatePara.m_PmtInfo.m_VidPid = plEncPara->m_VidPid;
				lPsiCreatePara.m_PmtInfo.m_AudEncMode = plEncPara->m_AudEncMode;
				lPsiCreatePara.m_PmtInfo.m_AudPid = plEncPara->m_AudPid;
				lPsiCreatePara.m_PmtInfo.m_PcrPid = plEncPara->m_PcrPid; 
				lPsiCreatePara.m_PmtInfo.m_PmtPid = plEncPara->m_PmtPid;
				lPsiCreatePara.m_PmtInfo.m_ProgramNum = plEncPara->m_ServiceId;
				GLOBAL_STRCPY(lPsiCreatePara.m_PmtInfo.m_pProgramName, plEncPara->m_pServiceName);
				HWL_EncPsiCreatePmt(&lPsiCreatePara.m_PmtInfo, plPsiTableInfo);
			}
		}
	}

#if (0)
	{ /* 打印Psi表 */
		CHAR_T plTitle[256];

		for (j=0; j<plPsiTableInfo->m_PsiPacketCounter; j++)
		{
			GLOBAL_SPRINTF((plTitle, "PSI Table [%d]", j + 1));
			CAL_PrintDataBlock(plTitle, plPsiTableInfo->m_pPsiPacket[j], MPEG2_TS_PACKET_SIZE);
		}
	}
#endif

	/* 发送PSI包 */
	{
		MFPGA_PsiInsertParam lOutPsiInsertPara;

		lOutPsiInsertPara.m_PsiPacketNum = MFPGA_MAX_PSI_NUM;
		lOutPsiInsertPara.m_OutTsMode = MPTS; /* 注意这里设置到fpga的值和web设置值要一致 */
		lOutPsiInsertPara.m_Interval = 100;
		GLOBAL_MEMCPY(lOutPsiInsertPara.m_Data, plPsiTableInfo->m_pPsiPacket, plPsiTableInfo->m_PsiPacketCounter * MPEG2_TS_PACKET_SIZE);
		PSI_FillNullPacket(&lOutPsiInsertPara, plPsiTableInfo->m_PsiPacketCounter); /* 用空包填充剩余 */
		if (MFPGA_SetPsiInsert(&lOutPsiInsertPara) == FALSE)
		{
			GLOBAL_SAFEFREE(plPsiTableInfo);
			return FALSE;
		}
	}

	GLOBAL_SAFEFREE(plPsiTableInfo);

	return TRUE;
}

/* EOF */
