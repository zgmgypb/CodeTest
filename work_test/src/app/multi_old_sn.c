
#include "multi_main_internal.h"

#ifdef MULT_DEVICE_NOT_SUPPORT_ENCRYPT_CHIP
static S32 s_EquipType;
static S32 s_EquipSubType;
static U32 s_EquipSN;
static S32 s_EquipYear;
static S32 s_EquipMonth;
static S32 s_EquipDay;


U32 MULT_OLDSNGetSN(void)
{
	return(s_EquipSN) ;
}

BOOL MULT_OLDSNLoadSN(CHAR_T *pPathName)
{
	BOOL lRet = FALSE;

	S32 i, j, k;
	GLOBAL_FD lTmpFD;
	CHAR_T lTmpBuf[256];
	CHAR_T lTmpVar;

	s_EquipType = 0x00;
	s_EquipSubType = 0x00;
	s_EquipSN = 10;
	s_EquipYear = 10;
	s_EquipMonth = 0;
	s_EquipDay = 1;

	lTmpFD = GLOBAL_FOPEN(pPathName , "rb");
	if (lTmpFD != NULL)
	{
		k = 0;
		while(TRUE)
		{
			if (k >= 11)
			{
				break;
			}

			switch(k)
			{
			case 0 :
				i = GLOBAL_FREAD(lTmpBuf , 1 , 1 , lTmpFD);
				if (i > 30) k = 50 ;
				break ;
			case 1 :
				i = lTmpBuf[0] ;
				j = GLOBAL_FREAD(lTmpBuf , 1 , i , lTmpFD);
				if (j != i) k = 50 ; 
				break ;
			case 2 :
				j = GLOBAL_FREAD(&lTmpBuf[0] , 1 , 20 , lTmpFD);
				if (j != 20) k = 50  ;
				break ;
			case 3 :
				j = GLOBAL_FREAD(&lTmpBuf[128+8] , 1 , 2 , lTmpFD);
				if (j != 2) k = 50  ;
				break ;
			case 4 :
				j = GLOBAL_FREAD(&lTmpBuf[20] , 1 , 30 , lTmpFD);
				if (j != 30) k = 50  ;
				break ;
			case 5 :
				j = GLOBAL_FREAD(&lTmpBuf[128+8+2] , 1 , 2 , lTmpFD) ;
				if (j != 2) k = 50  ;
				break ;
			case 6 :
				j = GLOBAL_FREAD(&lTmpBuf[50] , 1 , 20 , lTmpFD) ;
				if (j != 20) k = 50  ;
				break ;
			case 7 :
				j = GLOBAL_FREAD(&lTmpBuf[128] , 1 , 4 , lTmpFD) ;
				if (j != 4) k = 50  ;
				break ;
			case 8 :
				j = GLOBAL_FREAD(&lTmpBuf[70] , 1 , 30 , lTmpFD) ;
				if (j != 30) k = 50  ;
				break ;
			case 9 :
				j = GLOBAL_FREAD(&lTmpBuf[128+4] , 1 , 4 , lTmpFD) ;
				if (j != 4) k = 50  ;
				break ;
			case 10 :
				j = GLOBAL_FREAD(&lTmpBuf[100] , 1 , 28 , lTmpFD) ;
				if (j != 28) k = 50  ;
				break ;
			case 11 :
				break ;
			default :
				k = 50  ;
				break ;
			}
			k++ ;
		}
		GLOBAL_FCLOSE(lTmpFD) ;

		/*
		outFile.Write(&oneChar[0] , 20) ;
		outFile.Write(&oneChar[128+8] , 2) ;  //crc32 bit31..16
		outFile.Write(&oneChar[20] , 30) ;
		outFile.Write(&oneChar[128+8+2] , 2) ; //crc32 bit15..0
		outFile.Write(&oneChar[50] , 20) ;
		outFile.Write(&oneChar[128] , 4) ;    //serial
		outFile.Write(&oneChar[70] , 30) ;
		outFile.Write(&oneChar[128 + 4] , 4) ;  //year month data
		outFile.Write(&oneChar[100] , 28) ;

		*/

		if (k <= 11)
		{
			U32 lCRC32;
			lCRC32 = CRYPTO_CRC32(0xFFFFFFFF, lTmpBuf, 128 + 8 + 4);
			if (lCRC32 == 0)
			{
				for(j = 0 ; j < 8 ; j++)
				{
					lTmpVar = lTmpBuf[128+j] ;
					for(k = j * 8 ; k < (j * 8) + (lTmpBuf[j] & 0x0f) ; k++)
					{
						lTmpVar ^= lTmpBuf[k] ;
					}
					lTmpBuf[128+j] = lTmpVar ;
				}

				/*
				j = 128 ;
				oneChar[j++] = m_typeByte ;
				oneChar[j++] = m_subType ;
				oneChar[j++] = ((i + m_startSerial) >> 8) & 0xff ;
				oneChar[j++] = (i + m_startSerial) & 0xff ;
				oneChar[j++] = rand() ;
				seconds = time((time_t *) NULL) ;
				pTM = gmtime(&seconds) ;
				oneChar[j++] = pTM->tm_year - 100 ;
				oneChar[j++] = pTM->tm_mon ;
				oneChar[j++] = pTM->tm_mday ;
				*/
				j = 128 ;
				s_EquipType = lTmpBuf[j++] ;
				s_EquipSubType = lTmpBuf[j++] ;
				s_EquipSN = lTmpBuf[j++] ;
				s_EquipSN <<= 8 ;
				s_EquipSN |= lTmpBuf[j++] ;
				s_EquipSN |= ((lTmpBuf[100] & 0xff) << 24);
				s_EquipSN |= ((lTmpBuf[105] & 0xff) << 16);
				j++ ;
				s_EquipYear =  lTmpBuf[j++] ;
				s_EquipMonth =  lTmpBuf[j++] ;
				s_EquipDay =  lTmpBuf[j++] ;

				GLOBAL_TRACE(("SN = 0x%.8X\n", s_EquipSN));
				lRet = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("SN File CRC  = 0x%.8X, Error!\n", lCRC32));
			}
		}
		else
		{
			GLOBAL_TRACE(("\n"));
		}

	}
	else
	{
		GLOBAL_TRACE(("SN File Not Exist!!!\n"));
	}

	return lRet;
}
#endif
