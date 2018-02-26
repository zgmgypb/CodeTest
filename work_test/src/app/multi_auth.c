/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
#ifdef MULT_DEVICE_NOT_SUPPORT_ENCRYPT_CHIP
#define MULTL_TEST_AUTH_STORAGE_FILE	"/mnt/mtd/auth.bin"
void MULTL_AuthFixStorageCB(void* pUserParam, U32 StartAddress, U8 *pData, S32 DataSize, BOOL bRead)
{
	GLOBAL_FD lFD;
	S32 lFileSize;
	lFD = GLOBAL_FOPEN(MULTL_TEST_AUTH_STORAGE_FILE, "rb+");
	if (!lFD)
	{
		lFD = GLOBAL_FOPEN(MULTL_TEST_AUTH_STORAGE_FILE, "wb+");
		if (lFD)
		{
			U8 lChar = 0xFF;
			GLOBAL_FWRITE(&lChar, 1, 80, lFD);
		}
	}

	if (lFD)
	{
		GLOBAL_TRACE(("bRead = %d, StartAddr = %d\n", bRead, StartAddress));
		GLOBAL_FSEEK(lFD, StartAddress * 10, SEEK_SET);
		if (bRead)
		{
			GLOBAL_FREAD(pData, 1, 10, lFD);
		}
		else
		{
			GLOBAL_FWRITE(pData, 1, 10, lFD);
		}
		GLOBAL_FCLOSE(lFD);
	}
}

#else
void MULTL_AuthFixStorageCB(void* pUserParam, U32 StartAddress, U8 *pData, S32 DataSize, BOOL bRead)
{
#ifdef GN1846
	return;
#endif

	MULT_Handle *plHandle = (MULT_Handle*)pUserParam;
	if (plHandle)
	{
#ifdef GM8358Q
		HWL_EncoderAuthFixStorageCB(pUserParam, StartAddress, pData, DataSize, bRead);
#endif
		if (bRead)
		{
			HWL_EncryptChipRead(pData, GLOBAL_MIN(DataSize, 10), StartAddress, 2000);
		}
		else
		{
			HWL_EncryptChipWrite(pData, GLOBAL_MIN(DataSize, 10), StartAddress);
		}
	}
}
#endif

/*读取授权文件*/
BOOL MULTL_LoadLicense(MULT_Handle *pHandle, CHAR_T *pPathname, BOOL bCheckValid, CHAR_T* pInformation, S32 BufSize)
{
	BOOL bValid = FALSE;
	GLOBAL_FD lFD;
	U8 *plData, plTmpData[MULT_APPLICATION_CODE_BUF_SIZE], *plTmpBuf;
	S32 lFileSize, lTmpDataSize;
	MULT_Information *plInfo;
	plInfo = &pHandle->m_Information;
	lFD = GLOBAL_FOPEN(pPathname, "rb");
	if (lFD)
	{
		lFileSize = CAL_FileSize(lFD);
		if (lFileSize)
		{
			plData = (U8*)GLOBAL_MALLOC(lFileSize);
			if (plData)
			{
				GLOBAL_FREAD(plData, 1, lFileSize, lFD);

				if (pInformation && BufSize > 0)
				{
					pInformation[0] = 0;
				}

				if (bCheckValid)
				{
					if (AUTH_CheckLicense(pHandle->m_AUTHHandle, plData, lFileSize))
					{
						GLOBAL_TRACE(("Check License OK!\n"));
						bValid = TRUE;
					}
					else
					{
						GLOBAL_TRACE(("Check License Failed!\n"));
					}
				}
				else
				{
					plInfo->m_LicenseValid = FALSE;
					if ((lTmpDataSize = AUTH_GetLicense(pHandle->m_AUTHHandle, plData, lFileSize, plTmpData, lFileSize)) > 0)
					{
						plTmpBuf = plTmpData;
						GLOBAL_MSB32_D(plTmpBuf, plInfo->m_LicenseMode);
						plTmpBuf = plTmpData +4;
						GLOBAL_MSB32_D( plTmpBuf, plInfo->m_LicenseInASINum);

#if defined(GQ3655) || defined(GQ3760A) || defined(GQ3760)|| defined(GM8358Q)
						plTmpBuf = plTmpData + 8;
						GLOBAL_MSB32_D( plTmpBuf, plInfo->m_LicenseInIPNum);
						plTmpBuf = plTmpData + 12;
						GLOBAL_MSB32_D( plTmpBuf, plInfo->m_LicenseSCSNum);
						plTmpBuf = plTmpData + 16;
						GLOBAL_MSB32_D( plTmpBuf, plInfo->m_LicenseDescrambleNum);
						plTmpBuf = plTmpData + 20;
						GLOBAL_MSB32_D( plTmpBuf, plInfo->m_LicenseOutIPNum);
#endif
						GLOBAL_TRACE(("Load License OK! Mode = %d, InASI = %d, InIP = %d, SCS = %d, Descramble = %d, OutIP = %d\n", plInfo->m_LicenseMode, plInfo->m_LicenseInASINum, plInfo->m_LicenseInIPNum, plInfo->m_LicenseSCSNum, plInfo->m_LicenseDescrambleNum, plInfo->m_LicenseOutIPNum));
						AUTH_GetIssueDate(pHandle->m_AUTHHandle, plData, lFileSize, plInfo->m_LicenseDate, sizeof(plInfo->m_LicenseDate));
						AUTH_GetExpiredDate(pHandle->m_AUTHHandle, plData, lFileSize, plInfo->m_ExpireDate, sizeof(plInfo->m_ExpireDate));
						plInfo->m_LicenseValid = TRUE;
						bValid = TRUE;
					}
					else
					{
						GLOBAL_TRACE(("Load License Failed!\n"));
					}
				}
				GLOBAL_FREE(plData);
				plData = NULL;
			}
		}
		GLOBAL_FCLOSE(lFD);
	}
	return bValid;
}

/*授权测试*/
void MULTL_AuthCHIPTest(void)
{
	S32 lErrorCount, lDataErrorCount, lTotalCount;
	U8 plData[10], plRead[10];

	CAL_RandRandomFill(plData, sizeof(plData));

	CRYPTO_CRC32Generate(plData, sizeof(plData));

	HWL_EncryptChipWrite(plData, 10, 0);

	lDataErrorCount = 0;
	lErrorCount = 0;
	lTotalCount = 0;
	while(TRUE)
	{
		GLOBAL_ZEROMEM(plRead, sizeof(plRead));

		if (HWL_EncryptChipRead(plRead, 10, 0, 2000) == 10)
		{
			if (GLOBAL_MEMCMP(plData, plRead, 10) != 0)
			{
				CAL_PrintDataBlock("Ori", plData, 10);
				CAL_PrintDataBlock("Read", plRead, 10);
				lErrorCount++;
			}
		}
		else
		{
			GLOBAL_TRACE(("Read Error!\n"));
			lErrorCount++;
		}
		lTotalCount++;
		GLOBAL_TRACE(("Read Error = %d, Data Error = %d, Total = %d, Rate = %f %%\n", lErrorCount, lDataErrorCount, lTotalCount, 100.0 - (lErrorCount + lDataErrorCount) * 100.0 / lTotalCount));
	}
}


/*生成授权--------------------------------------------------------------------------------------------------------------------------------*/
void MULTL_GenerateAuth(MULT_Handle *pHandle)
{
	MULT_Information *plInfo;
	AUTH_Param lParam;
	U8 plTmpData[MULT_APPLICATION_CODE_BUF_SIZE];
	S32 lTmpDataSize;
	CHAR_T plTmpCMD[1024];

	plInfo = &pHandle->m_Information;

/*加密芯片访问控制！*/
#if 0
	MULTL_AuthCHIPTest();
#endif

#ifdef MULT_LICENSE_DEBUG//用于调试过程中
	plInfo->m_LicenseValid = MULT_LICENSE_DEBUG_MARK_VALID;
	plInfo->m_LicenseMode = MULT_LICENSE_DEBUG_MODE;
	plInfo->m_LicenseOutTsNum = MULT_LICENSE_DEBUG_OUT_TS_NUM;
	plInfo->m_LicenseInASINum = MULT_LICENSE_DEBUG_ASI_TS_NUM;
	plInfo->m_LicenseSCSNum = MULT_LICENSE_DEBUG_SCS_NUM;
#else /*MULT_LICENSE_DEBUG*/

	GLOBAL_STRCPY(lParam.m_pSN, pHandle->m_Information.m_pSNString);
	lParam.m_TotalFreeTimeAllowed = MULT_AUTH_TOTAL_TRAIL_TIME;
	lParam.m_CountDownCycle = MULT_AUTH_COUNT_DOWN_CYCLE;
#ifdef GM8358Q
	lParam.m_pFixCB = HWL_EncoderAuthFixStorageCB;
#else
	lParam.m_pFixCB = MULTL_AuthFixStorageCB;
#endif
	lParam.m_pUserParam = pHandle;
	pHandle->m_AUTHHandle = AUTH_Create(&lParam);

	/*配置默认参数*/
	plInfo->m_LicenseValid = FALSE;
	plInfo->m_LicenseMode = 0;
	GLOBAL_STRCPY(plInfo->m_LicenseDate, "------------");
	GLOBAL_STRCPY(plInfo->m_ExpireDate, "------------");
	plInfo->m_TrailTime = AUTH_GetTrailWorkTime(pHandle->m_AUTHHandle);
	lTmpDataSize = AUTH_GetRequestCode(pHandle->m_AUTHHandle, plTmpData, sizeof(plTmpData));
	lTmpDataSize = CAL_StringBinToHex(plTmpData, lTmpDataSize, plInfo->m_ApplicationCode, sizeof(plInfo->m_ApplicationCode), TRUE);
	plInfo->m_ApplicationCode[lTmpDataSize] = 0;

	/*读取授权文件*/
	GLOBAL_SPRINTF((plTmpCMD, "%s%s", MULT_STORAGE_BASE_DIR, MULT_LICENSE_FILE_PATHNAME));
	if (MULTL_LoadLicense(pHandle, plTmpCMD, FALSE, NULL, 0))
	{
		GLOBAL_TRACE(("Load License Successful!\n"));

#ifdef GQ3710A

		switch (plInfo->m_LicenseMode)
		{
		case 0:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 0;
			}
			break;
		case 1:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 1;
			}
			break;
		case 2:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 2;
			}
			break;
		case 3:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 3;
			}
			break;
		case 4:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 4;
			}
			break;
		default:
			plInfo->m_LicenseOutTsNum = 1;
			plInfo->m_LicenseSCSNum = 0;
			break;
		}

		if ((plInfo->m_LicenseInASINum <= 0) || (plInfo->m_LicenseInASINum > 8) )
		{
			plInfo->m_LicenseInASINum = 1;
		}
#endif	

//#if defined(GQ3710B)
//
//		switch (plInfo->m_LicenseMode)
//		{
//		case 0:
//			{
//				plInfo->m_LicenseSCSNum = 0;
//			}
//			break;
//		case 1:
//			{
//				plInfo->m_LicenseSCSNum = 1;
//			}
//			break;
//		case 2:
//			{
//				plInfo->m_LicenseSCSNum = 2;
//			}
//			break;
//		case 3:
//			{
//				plInfo->m_LicenseSCSNum = 3;
//			}
//			break;
//		case 4:
//			{
//				plInfo->m_LicenseSCSNum = 4;
//			}
//			break;
//		default:
//			plInfo->m_LicenseSCSNum = 0;
//			break;
//		}
//
//		if ((plInfo->m_LicenseInASINum <= 0) || (plInfo->m_LicenseInASINum > 8) )
//		{
//			plInfo->m_LicenseInASINum = 1;
//		}
//		plInfo->m_LicenseOutTsNum = 1;
//		plInfo->m_LicenseInIPNum = 16;
//#endif	

#if defined(GQ3760A) || defined(GQ3760)

		plInfo->m_LicenseOutTsNum = 1;
		plInfo->m_LicenseSCSNum = 0;

		if ((plInfo->m_LicenseInASINum <= 0) || (plInfo->m_LicenseInASINum > 2) )
		{
			plInfo->m_LicenseInASINum = 1;
		}

		if ((plInfo->m_LicenseInIPNum > 64) || (plInfo->m_LicenseInIPNum < 0))
		{
			plInfo->m_LicenseInIPNum = 16;
		}
#endif	

#ifdef GQ3650DR

		switch (plInfo->m_LicenseMode)
		{
		case 0:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 0;
			}
			break;
		case 1:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 4;
			}
			break;
		case 2:
			{
				plInfo->m_LicenseOutTsNum = 2;
				plInfo->m_LicenseSCSNum = 0;
			}
			break;
		case 3:
			{
				plInfo->m_LicenseOutTsNum = 2;
				plInfo->m_LicenseSCSNum = 4;
			}
			break;
		case 4:
			{
				plInfo->m_LicenseOutTsNum = 4;
				plInfo->m_LicenseSCSNum = 0;
			}
			break;
		case 5:
			{
				plInfo->m_LicenseOutTsNum = 4;
				plInfo->m_LicenseSCSNum = 4;
			}
			break;
		default:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 0;
			}
			break;
		}

		if ((plInfo->m_LicenseInASINum <= 0) || (plInfo->m_LicenseInASINum > 8) )
		{
			plInfo->m_LicenseInASINum = 8;
		}	

#endif	

#ifdef GQ3650DS
		switch (plInfo->m_LicenseMode)
		{
		case 0:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 0;
			}
			break;
		case 1:
			{
				plInfo->m_LicenseOutTsNum = 1;
				plInfo->m_LicenseSCSNum = 4;
			}
			break;
		case 2:
			{
				plInfo->m_LicenseOutTsNum = 2;
				plInfo->m_LicenseSCSNum = 0;
			}
			break;
		case 3:
			{
				plInfo->m_LicenseOutTsNum = 2;
				plInfo->m_LicenseSCSNum = 4;
			}
			break;
		case 4:
			{
				plInfo->m_LicenseOutTsNum = 4;
				plInfo->m_LicenseSCSNum = 0;
			}
			break;
		case 5:
			{
				plInfo->m_LicenseOutTsNum = 4;
				plInfo->m_LicenseSCSNum = 4;
			}
			break;
		default:
			plInfo->m_LicenseOutTsNum = 1;
			plInfo->m_LicenseSCSNum = 0;
			break;
		}
		plInfo->m_LicenseInASINum = 8;
#endif

#ifdef GQ3655
		switch (plInfo->m_LicenseMode)
		{
		case 0:
			{
				plInfo->m_LicenseOutTsNum = 1;
			}
			break;
		case 1:
			{
				plInfo->m_LicenseOutTsNum = 2;
			}
			break;
		case 2:
			{
				plInfo->m_LicenseOutTsNum = 3;
			}
			break;
		case 3:
			{
				plInfo->m_LicenseOutTsNum = 4;
			}
			break;
		default:
			plInfo->m_LicenseOutTsNum = 1;
			break;
		}

		if ((plInfo->m_LicenseInASINum > 12) || (plInfo->m_LicenseInASINum < 0))
		{
			plInfo->m_LicenseInASINum = 0;
		}
		if ((plInfo->m_LicenseInIPNum > 64) || (plInfo->m_LicenseInIPNum < 0))
		{
			plInfo->m_LicenseInIPNum = 16;
		}
		if ((plInfo->m_LicenseSCSNum > 4) || (plInfo->m_LicenseSCSNum < 0))
		{
			plInfo->m_LicenseSCSNum = 0;
		}
#endif

#ifdef GM8358Q
		switch (plInfo->m_LicenseMode)
		{
		case 0:
			{
				plInfo->m_LicenseOutTsNum = 1;
			}
			break;
		case 1:
			{
				plInfo->m_LicenseOutTsNum = 2;
			}
			break;
		case 2:
			{
				plInfo->m_LicenseOutTsNum = 3;
			}
			break;
		case 3:
			{
				plInfo->m_LicenseOutTsNum = 4;
			}
			break;
		default:
			plInfo->m_LicenseOutTsNum = 1;
			break;
		}

		plInfo->m_LicenseInASINum = 1;
		plInfo->m_LicenseInIPNum = 0;
		plInfo->m_LicenseOutIPNum = 0;
		plInfo->m_LicenseDescrambleNum = 0;

		if ((plInfo->m_LicenseSCSNum > 4) || (plInfo->m_LicenseSCSNum < 0))
		{
			plInfo->m_LicenseSCSNum = 0;
		}
#endif

	}
	else
	{
		GLOBAL_TRACE(("Trail Time = %d\n", plInfo->m_TrailTime));
		if (plInfo->m_TrailTime > 0)
		{
#ifdef GQ3650DS
			plInfo->m_LicenseMode = 5;
			plInfo->m_LicenseOutTsNum = 4;
			plInfo->m_LicenseInASINum = 8;
			plInfo->m_LicenseSCSNum = 4;
#endif
#ifdef GQ3650DR
			plInfo->m_LicenseMode = 5;
			plInfo->m_LicenseOutTsNum = 4;

			plInfo->m_LicenseInASINum = 8;
			plInfo->m_LicenseSCSNum = 4;
#endif
#ifdef GQ3655
			plInfo->m_LicenseMode = 3;
			plInfo->m_LicenseInASINum = 8;
			plInfo->m_LicenseInIPNum = 64;
			plInfo->m_LicenseOutTsNum = 4;
			plInfo->m_LicenseSCSNum = 4;
#endif
#ifdef GM8358Q
			plInfo->m_LicenseMode = 3;
			plInfo->m_LicenseOutTsNum = 4;
			plInfo->m_LicenseInASINum = 1;
			plInfo->m_LicenseInIPNum = 0;
			plInfo->m_LicenseOutIPNum = 0;
			plInfo->m_LicenseDescrambleNum = 0;

			plInfo->m_LicenseSCSNum = 4;
#endif
#ifdef GQ3710A
			plInfo->m_LicenseMode = 4;
			plInfo->m_LicenseOutTsNum = 1;

			plInfo->m_LicenseInASINum = 8;
			plInfo->m_LicenseSCSNum = 4;
#endif
//#if defined(GQ3710B)
//			plInfo->m_LicenseMode = 4;
//			plInfo->m_LicenseOutTsNum = 1;
//
//			plInfo->m_LicenseInIPNum = 16;
//			plInfo->m_LicenseInASINum = 8;
//			plInfo->m_LicenseSCSNum = 4;
//#endif
#if defined(GQ3760A)
			plInfo->m_LicenseMode = 0;
			plInfo->m_LicenseOutTsNum = 1;
			plInfo->m_LicenseInIPNum = 64;
			plInfo->m_LicenseInASINum = 2;
			plInfo->m_LicenseSCSNum = 0;
			plInfo->m_LicenseOutIPNum = 1;
#endif
		}
		else
		{
			plInfo->m_LicenseMode = 0;
			plInfo->m_LicenseOutTsNum = 1;
			plInfo->m_LicenseInASINum = 8;
			plInfo->m_LicenseSCSNum = 4;
#ifdef GQ3710A
			plInfo->m_LicenseInASINum = 1;
			plInfo->m_LicenseSCSNum = 0;
#endif
//#if defined(GQ3710B) 
//			plInfo->m_LicenseInASINum = 1;
//			plInfo->m_LicenseSCSNum = 0;
//#endif
#if defined(GQ3760A)
			plInfo->m_LicenseInASINum = 2;
			plInfo->m_LicenseInIPNum = 0;
			plInfo->m_LicenseSCSNum = 0;
#endif
#ifdef GQ3655
			plInfo->m_LicenseInASINum = 0;
			plInfo->m_LicenseInIPNum = 16;
			plInfo->m_LicenseSCSNum = 0;
#endif	
#ifdef GM8358Q
			plInfo->m_LicenseOutTsNum = 1;
			plInfo->m_LicenseInASINum = 1;
			plInfo->m_LicenseInIPNum = 0;
			plInfo->m_LicenseOutIPNum = 0;
			plInfo->m_LicenseDescrambleNum = 0;

			plInfo->m_LicenseSCSNum = 0;
#endif	
		}
	}
#endif /*MULT_LICENSE_DEBUG*/

#ifdef GA2620B
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 4;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 1;

#endif	

# if defined(GM2700B) || defined(GM2700S)
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 4;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 2;
#ifdef GM2700S

#else

#endif
#endif	


#ifdef GM2730X
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 4;
#ifdef GM2762
	plInfo->m_LicenseSCSNum = 0;
#endif
	plInfo->m_LicenseInIPNum = 64;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 8;
#endif	

#ifdef GM2730S
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInIPNum = 8;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 500;
#endif	

#ifdef GC1804C
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInIPNum = 0;
	plInfo->m_LicenseInASINum = 0;
	plInfo->m_LicenseOutTsNum = 4;

#endif	

#ifdef GC1815B
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInIPNum = 8;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 8;

#endif	

#ifdef GM2730H
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInIPNum = 48;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 4;
#endif	

#ifdef GM2730F
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 1;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseInIPNum = 8;
	plInfo->m_LicenseOutTsNum = 8;

#endif	

#ifdef GN2000
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInIPNum = 8;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 8;

#endif	


#ifdef GM4500
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInIPNum = 16;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 32;

#endif	

#ifdef LR1800S
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInIPNum = 16;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseOutTsNum = 32;

#endif

#ifdef GM7000
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseInIPNum = 128;
	plInfo->m_LicenseOutTsNum = 4;
	plInfo->m_LicenseSCSNum = 4;
	plInfo->m_LicenseOutIPNum = 4;
#endif	

#if defined(GQ3760B) || defined(GQ3763) || defined(GQ3768) || defined(GQ3765)
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseOutTsNum = 1;
	plInfo->m_LicenseInIPNum = 1;
	plInfo->m_LicenseInASINum = 1;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseOutIPNum = 1;
#endif

#ifdef GM2750
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseInASINum = 1;
	plInfo->m_LicenseInIPNum = 1;
	plInfo->m_LicenseOutIPNum = 1;
	plInfo->m_LicenseOutTsNum = 64;
	plInfo->m_LicenseSCSNum = 0;
#endif	

#if defined(GQ3760)
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseOutTsNum = 1;
	plInfo->m_LicenseInIPNum = 64;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseOutIPNum = 1;
#endif

#if defined(GQ3765)
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseOutTsNum = 1;
	plInfo->m_LicenseInIPNum = 64;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseOutIPNum = 1;
#endif

#if defined(GN1772)
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseOutTsNum = 4096;
	plInfo->m_LicenseInIPNum = 4096;
	plInfo->m_LicenseOutIPNum = 4096;
	plInfo->m_LicenseInASINum = 4096;
	plInfo->m_LicenseSCSNum = 0;
#endif

#ifdef GC1804B
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInASINum = 8;
	plInfo->m_LicenseInIPNum = 8;
	plInfo->m_LicenseOutTsNum = 4;
#endif	

#ifdef GN1866
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInASINum = 1;
	plInfo->m_LicenseInIPNum = 1;
	plInfo->m_LicenseOutTsNum = 2;
#endif	

#ifdef GN1846
	plInfo->m_LicenseValid = TRUE;
	plInfo->m_LicenseMode = 0;
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInASINum = 1;
	plInfo->m_LicenseInIPNum = 1;
	plInfo->m_LicenseOutTsNum = 2;
#endif	

#if defined(GQ3710B)
	plInfo->m_LicenseSCSNum = 0;
	plInfo->m_LicenseInASINum = 2;
	plInfo->m_LicenseInIPNum = 4;
	plInfo->m_LicenseOutTsNum = 1;
#endif	

	GLOBAL_TRACE(("License Mark = %d, Mode = %d, ASI = %d, IPIN = %d, IPOut = %d, Desc = %d, SCS = %d, OutTs = %d\n", plInfo->m_LicenseValid, plInfo->m_LicenseMode, plInfo->m_LicenseInASINum, plInfo->m_LicenseInIPNum, plInfo->m_LicenseOutIPNum, plInfo->m_LicenseDescrambleNum, plInfo->m_LicenseSCSNum, plInfo->m_LicenseOutTsNum));
}


/*EOF*/
