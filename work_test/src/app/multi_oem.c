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
BOOL MULTL_LoadOEM(MULT_Handle *pHandle, CHAR_T *pPathname, BOOL bCheckValid, CHAR_T* pInformation, S32 BufSize)
{
	BOOL lRet = FALSE;
	GLOBAL_FD lFD;
	CHAR_T plUTF8Buf[512], *plTmpStr;
	S32 lFileSize, lUTF8Len;
	CAL_FCAPHandle *plUPGHandle;
	MULT_Information *plInfo;
	plInfo = &pHandle->m_Information;
	plUPGHandle = NULL;
	lFD = GLOBAL_FOPEN(pPathname, "rb");
	if (lFD)
	{
		lFileSize = CAL_FileSize(lFD);
		plUPGHandle = UPG_OEMDecodeCreate(lFileSize);
		if (plUPGHandle)
		{
			GLOBAL_FREAD(plUPGHandle->m_pDataBuf, 1, lFileSize, lFD);
			if (UPG_OEMValidation(plUPGHandle, MULT_DEVICE_COMPLETE_TYPE) == TRUE)
			{
				if (bCheckValid)
				{
					if (pInformation && BufSize > 0)
					{
						plTmpStr = CAL_FCAPGetDescription(plUPGHandle);
						if (GLOBAL_STRLEN(plTmpStr) < BufSize)
						{
							GLOBAL_STRCPY(pInformation, plTmpStr);
						}
					}
					lRet = TRUE;
				}
				else
				{
					/*复制各种文件到目录当中*/
					GLOBAL_FD lNewFD;
					U8 *plNewBuf;
					S32 lNewSize;
					plNewBuf = UPG_OEMGetData(plUPGHandle, UPG_DATA_TYPE_LOGO, &lNewSize);
					if (plNewBuf && (lNewSize > 0))
					{
						lNewFD = GLOBAL_FOPEN(MULT_LOGO_FILE_WEB_PATH, "wb");
						if (lNewFD)
						{
							GLOBAL_FWRITE(plNewBuf, 1, lNewSize, lNewFD);
							GLOBAL_FCLOSE(lNewFD);
							lNewFD = NULL;
						}
					}

					plNewBuf = UPG_OEMGetData(plUPGHandle, UPG_DATA_TYPE_INTRO, &lNewSize);
					if (plNewBuf && (lNewSize > 0))
					{
						lNewFD = GLOBAL_FOPEN(MULT_INTRO_FILE_WEB_PATH, "wb");
						if (lNewFD)
						{
							GLOBAL_FWRITE(plNewBuf, 1, lNewSize, lNewFD);
							GLOBAL_FCLOSE(lNewFD);
							lNewFD = NULL;
						}
					}

					plNewBuf = UPG_OEMGetData(plUPGHandle, UPG_DATA_TYPE_MANUAL, &lNewSize);
					if (plNewBuf && (lNewSize > 0))
					{
						lNewFD = GLOBAL_FOPEN(MULT_MANUAL_FILE_WEB_PATH, "wb");
						if (lNewFD)
						{
							plInfo->m_bHaveManual = TRUE;
							GLOBAL_FWRITE(plNewBuf, 1, lNewSize, lNewFD);
							GLOBAL_FCLOSE(lNewFD);
							lNewFD = NULL;
						}
					}

					/*20130709增加OEM文件覆盖功能*/
					plNewBuf = UPG_OEMGetData(plUPGHandle, UPG_DATA_TYPE_OVERRIDE, &lNewSize);
					if (plNewBuf && (lNewSize > 0))
					{
						lNewFD = GLOBAL_FOPEN(MULT_OVERRIDE_FILE_WEB_PATH, "wb");
						if (lNewFD)
						{
							GLOBAL_FWRITE(plNewBuf, 1, lNewSize, lNewFD);
							GLOBAL_FCLOSE(lNewFD);
							lNewFD = NULL;

							GLOBAL_TRACE(("Override File Size = %d\n", lNewSize));

							/*展开压缩文件，直接覆盖本目录的网页或图片或脚本！*/
							PFC_System("tar -xzf %s", MULT_OVERRIDE_FILE_WEB_PATH);
							PFC_System("rm %s", MULT_OVERRIDE_FILE_WEB_PATH);
						}
					}
					/*END*/

					GLOBAL_STRCPY(plInfo->m_pModelName, UPG_OEMGetModel(plUPGHandle));
					GLOBAL_STRCPY(plInfo->m_pWEBENG, UPG_OEMGetWebNameENG(plUPGHandle));
					plTmpStr = UPG_OEMGetWebNameCHN(plUPGHandle);
					lUTF8Len = CAL_CovertGB2312ToUTF8(plTmpStr, GLOBAL_STRLEN(plTmpStr), plUTF8Buf, sizeof(plUTF8Buf));
					plUTF8Buf[lUTF8Len] = 0;
					GLOBAL_STRCPY(plInfo->m_pWEBCHN, plUTF8Buf);
					GLOBAL_STRCPY(plInfo->m_pLCDENG, UPG_OEMGetLCDNameENG(plUPGHandle));
					GLOBAL_STRCPY(plInfo->m_pLCDCHN, UPG_OEMGetLCDNameCHN(plUPGHandle));
					GLOBAL_STRCPY(plInfo->m_pManufacter, UPG_OEMGetManufacturyName(plUPGHandle));
					GLOBAL_STRCPY(plInfo->m_pManufacterWEBADDR, UPG_OEMGetManufacturyWebAddr(plUPGHandle));

					lRet = TRUE;
				}
			}
			else
			{
				GLOBAL_TRACE(("Validation OEM Filed!!\n"));
			}
			CAL_FCAPDestroy(plUPGHandle);
			plUPGHandle = NULL;
		}
		else {
			GLOBAL_TRACE(("UPG_OEMDecodeCreate Failed!\n"));
		}
		GLOBAL_FCLOSE(lFD);
	}
	else
	{
		GLOBAL_TRACE(("No OEM File!\n"));
	}
	return lRet;
}


/*EOF*/
