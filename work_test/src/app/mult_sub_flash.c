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
static CAL_FCAPHandle *s_pFCAPHandle = NULL;
static BOOL	s_bFileLoadOK = FALSE;
static MULT_SUB_FLASH_INFO	s_SubFlashInfo;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
BOOL MULT_SubFlashXMLProcess(MULT_Handle *pHandle, mxml_node_t* pBodyNode, CHAR_T *pParameter, S32 OPType)
{
	BOOL lRet = FALSE;

	if (GLOBAL_STRCMP(pParameter, "sub_flash") == 0)
	{
		if (OPType == MULT_SUB_FLASH_OP_TYPE_VALIDATION_GET)
		{
			BOOL bValid = FALSE;
			CHAR_T plInformation[1024], *plTmpInfo;
			GLOBAL_FD lFD;
			S32 lFileSize;

			s_bFileLoadOK = FALSE;

			if (s_pFCAPHandle != NULL)
			{
				CAL_FCAPDestroy(s_pFCAPHandle);
				s_pFCAPHandle = NULL;
			}


			GLOBAL_ZEROMEM(plInformation, sizeof(plInformation));
			lFD = GLOBAL_FOPEN(CGIC_UPLOAD_FILE_PATHNAME, "rb");
			if (lFD)
			{
				lFileSize = CAL_FileSize(lFD);

				s_pFCAPHandle = UPG_MODULEDecodeCreate(lFileSize);
				if (s_pFCAPHandle)
				{
					GLOBAL_FREAD(s_pFCAPHandle->m_pDataBuf, 1, lFileSize, lFD);
					if (UPG_MODULEValidation(s_pFCAPHandle, MULT_DEVICE_COMPLETE_TYPE) == TRUE)
					{
						bValid = TRUE;
						plTmpInfo = CAL_FCAPGetDescription(s_pFCAPHandle);
						if (GLOBAL_STRLEN(plTmpInfo) < sizeof(plInformation))
						{
							GLOBAL_STRCPY(plInformation, plTmpInfo);
						}

						UPG_MODULEGetModuleDevType(s_pFCAPHandle, &s_SubFlashInfo.m_UPGInfo);
						s_SubFlashInfo.m_pDataPtr = CAL_FCAPGetPayloadPtr(s_pFCAPHandle);
						s_SubFlashInfo.m_DataSize = CAL_FCAPGetPayloadSize(s_pFCAPHandle);
						s_SubFlashInfo.m_DataCRC32 = CRYPTO_CRC32(GLOBAL_U32_MAX, s_SubFlashInfo.m_pDataPtr, s_SubFlashInfo.m_DataSize);

						GLOBAL_TRACE(("Valid Info [%s], CRC32 [%08X]\n", plInformation, s_SubFlashInfo.m_DataCRC32));

						{
							S32 lStrLen;

							GLOBAL_STRCAT(plInformation, " CRC32[");

							lStrLen = GLOBAL_STRLEN(plInformation);

							GLOBAL_SPRINTF((&plInformation[lStrLen], "%08X", s_SubFlashInfo.m_DataCRC32));

							GLOBAL_STRCAT(plInformation, "]");
						}

					}
					else
					{
						GLOBAL_TRACE(("Valid %s Failed!\n", pParameter));
					}
				}
				GLOBAL_FCLOSE(lFD);
			}


			{
				mxml_node_t *plXML;
				mxml_node_t *plXMLRoot;
				plXML = mxmlNewXML("1.0");
				plXMLRoot = mxmlNewElement(plXML, "root");

				if (bValid == TRUE)
				{
					MULTL_XMLAddNodeINT(plXMLRoot, "validation", 1);
					MULTL_XMLAddNodeText(plXMLRoot, "information", plInformation);
				}
				else
				{
					MULTL_XMLAddNodeINT(plXMLRoot, "validation", 0);
					MULTL_XMLAddNodeText(plXMLRoot, "information", "---------");
				}

				mxmlSaveString(plXML, pHandle->m_pReplayXMLBuf, sizeof(pHandle->m_pReplayXMLBuf), NULL);
				mxmlDelete(plXML);
				plXML = NULL;

			}

			PFC_System("rm -rf %s", CGIC_UPLOAD_FILE_PATHNAME);
		}
		else if (OPType == MULT_SUB_FLASH_OP_TYPE_CONFIRM)
		{
			if (s_pFCAPHandle)
			{
				GLOBAL_TRACE(("Sub Flash Prepare OK! Type %d, Version %04X, Size %d, PTR 0x%08X\n", s_SubFlashInfo.m_UPGInfo.m_ModuleType, s_SubFlashInfo.m_UPGInfo.m_Version, s_SubFlashInfo.m_DataSize, s_SubFlashInfo.m_pDataPtr));
				s_bFileLoadOK = TRUE;

#ifdef DEBUG_MODE_INIT_BUNNER_MODE
#else
#ifdef ENCODER_CARD_PLATFORM
				GLOBAL_TRACE(("Initate Sub Flash Burner Mode!!!!!!!!!!!!!!! Start\n"));
				MULT_CARDModuleTerminate();
				MULT_CARDModuleInitiate(pHandle, TRUE);
				GLOBAL_TRACE(("Initate Sub Flash Burner Mode!!!!!!!!!!!!!!! Done\n"));
#endif
#endif
			}
			else
			{
				GLOBAL_TRACE(("No File Loaded!!!\n"));
			}
		}


		lRet = TRUE;
	}
	return lRet;
}

BOOL MULT_SubFlashGetFlashInfo(MULT_SUB_FLASH_INFO *pInfo)
{
	BOOL lRet = FALSE;
	if (s_pFCAPHandle)
	{
		if (s_bFileLoadOK == TRUE)
		{
			if (pInfo)
			{
				GLOBAL_MEMCPY(pInfo, &s_SubFlashInfo, sizeof(MULT_SUB_FLASH_INFO));
				lRet = TRUE;
			}
		}
	}
	return lRet;
}

#if 0
void MULT_SubFlashTestInit(void)
{
	BOOL bValid = FALSE;
	CHAR_T plInformation[1024], *plTmpInfo;
	GLOBAL_FD lFD;
	S32 lFileSize;

	s_bFileLoadOK = FALSE;

	if (s_pFCAPHandle != NULL)
	{
		CAL_FCAPDestroy(s_pFCAPHandle);
		s_pFCAPHandle = NULL;
	}

	GLOBAL_ZEROMEM(plInformation, sizeof(plInformation));
	lFD = GLOBAL_FOPEN("/tmp/pro100_sub_flash.bin", "rb");
	if (lFD)
	{
		lFileSize = CAL_FileSize(lFD);

		s_pFCAPHandle = UPG_MODULEDecodeCreate(lFileSize);
		if (s_pFCAPHandle)
		{
			GLOBAL_FREAD(s_pFCAPHandle->m_pDataBuf, 1, lFileSize, lFD);
			if (UPG_MODULEValidation(s_pFCAPHandle, MULT_DEVICE_COMPLETE_TYPE) == TRUE)
			{
				bValid = TRUE;
				plTmpInfo = CAL_FCAPGetDescription(s_pFCAPHandle);
				if (GLOBAL_STRLEN(plTmpInfo) < sizeof(plInformation))
				{
					GLOBAL_STRCPY(plInformation, plTmpInfo);
				}

				UPG_MODULEGetModuleDevType(s_pFCAPHandle, &s_SubFlashInfo.m_UPGInfo);
				s_SubFlashInfo.m_pDataPtr = CAL_FCAPGetPayloadPtr(s_pFCAPHandle);
				s_SubFlashInfo.m_DataSize = CAL_FCAPGetPayloadSize(s_pFCAPHandle);
				s_SubFlashInfo.m_DataCRC32 = CRYPTO_CRC32(GLOBAL_U32_MAX, s_SubFlashInfo.m_pDataPtr, s_SubFlashInfo.m_DataSize);

				GLOBAL_TRACE(("Valid Info [%s], CRC32 [%08X]\n", plInformation, s_SubFlashInfo.m_DataCRC32));

				GLOBAL_TRACE(("Sub Flash Prepare OK! Type %d, Version %04X, Size %d, PTR 0x%08X\n", s_SubFlashInfo.m_UPGInfo.m_ModuleType, s_SubFlashInfo.m_UPGInfo.m_Version, s_SubFlashInfo.m_DataSize, s_SubFlashInfo.m_pDataPtr));
				s_bFileLoadOK = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("Validation Failed!!!!!!!!!!\n"));
				GLOBAL_PAUSE();
			}
			GLOBAL_FCLOSE(lFD);
		}
		else
		{
			GLOBAL_TRACE(("Create Handle Error!!!!!!!!!!\n"));
			GLOBAL_PAUSE();
		}
	}
	else
	{
		GLOBAL_TRACE(("Error!!!!!!!!!!\n"));
		GLOBAL_PAUSE();
	}
}
#endif

/*EOF*/
