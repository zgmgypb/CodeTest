/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "linuxplatform.h"
#include "multi_main_internal.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "multi_drv.h"


/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
//#define PROTOCOL_DEBUG
#define FIRII_MAX_COEFF_NUM				(1024)
#define MAX_BIT_NUMBER					(21)//系数精度
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	F64		m_pCoeefData[FIRII_MAX_COEFF_NUM];
	S32		m_CurrentNum;
}FIRII_RELOAD_HANDLE;

/* Private Variables (static)-------------------------------------------------- */
static FIRII_RELOAD_HANDLE s_FirIIReleadHandle;
/* Private Function prototypes ------------------------------------------------ */
static void FIRIIL_ReloadWriteReg(void);
/* Functions ------------------------------------------------------------------ */

void FIRII_Reload(CHAR_T *pMatlabCoeffFile)
{
	FIRII_RELOAD_HANDLE *plHandle = &s_FirIIReleadHandle;
	if (plHandle)
	{
		GLOBAL_FD lCoeffFileHandle;
		S32 i, k;
		U32 lRegValue;

		GLOBAL_TRACE(("Loading Coeff File!!! %s\n", pMatlabCoeffFile));

		plHandle->m_CurrentNum = 0;

		lCoeffFileHandle = GLOBAL_FOPEN(pMatlabCoeffFile, "rb");
		if (lCoeffFileHandle)
		{
			U8 *plFileData;
			U8 plLineData[1024];
			S32 lFileSize, lActRead;
			S32 lStarInd, lLineLen;

			lFileSize = CAL_FileSize(lCoeffFileHandle);

			if (lFileSize > 0)
			{
				plFileData = (U8 *)GLOBAL_ZMALLOC(lFileSize);
				if (plFileData)
				{
					lActRead = GLOBAL_FREAD(plFileData, 1, lFileSize, lCoeffFileHandle);

					/*去除所有非数字和点*/

					lStarInd = 0;
					for (i = 0; i < lFileSize; i++)
					{
						if (plFileData[i] == '\r' || plFileData[i] == '\n')
						{
							lLineLen = i - lStarInd;

							if (lLineLen > 0)
							{
								GLOBAL_MEMCPY(plLineData, &plFileData[lStarInd], lLineLen);
								plLineData[lLineLen] = 0;

								if (plHandle->m_CurrentNum < FIRII_MAX_COEFF_NUM)
								{
									plHandle->m_pCoeefData[plHandle->m_CurrentNum] = GLOBAL_STRTOD(plLineData, NULL);
									plHandle->m_CurrentNum++;
								}
							}

							lStarInd = i + 1;
						}

						if (plHandle->m_CurrentNum == 500)
						{
							k = 0;
						}
					}

					/*最后一帧*/
					lLineLen = i - lStarInd;
					if ( lLineLen > 0)
					{
						lLineLen = i - lStarInd;
						GLOBAL_MEMCPY(plLineData, &plFileData[lStarInd], lLineLen);
						plLineData[lLineLen] = 0;

						//CAL_PrintDataBlockWithASCII(__FUNCTION__, &plFileData[lStarInd], lLineLen, 4);

						for (k = 0; k < lLineLen; k++)
						{
							if (plLineData[k] != '.' && plLineData[k] < '0' && plLineData[k] > '9')
							{
								plLineData[k] = ' ';
							}
						}

						if (plHandle->m_CurrentNum < FIRII_MAX_COEFF_NUM)
						{
							plHandle->m_pCoeefData[plHandle->m_CurrentNum] = GLOBAL_STRTOD(plLineData, NULL);
							plHandle->m_CurrentNum++;
						}
					}

					GLOBAL_FREE(plFileData);
					plFileData = NULL;
				}
				else
				{
					GLOBAL_TRACE(("Malloc Failed!!!!\n"));
				}
			}
			else
			{
				GLOBAL_TRACE(("File Size = %d\n", lFileSize));
			}
			GLOBAL_FCLOSE(lCoeffFileHandle);

			//FIRIIL_ReloadReadReg();
			FIRIIL_ReloadWriteReg();
			//FIRIIL_ReloadReadReg();
		}
		else
		{
			GLOBAL_TRACE(("Open File Failed!!!!!!!!!\n"));
		}

	}
}

void FIRIIL_ReloadReadReg(void)
{
	FIRII_RELOAD_HANDLE *plHandle = &s_FirIIReleadHandle;
	if (plHandle)
	{
		S32 i, lLen, lApplyLen;
		U32 lTmpValue;
		F32 lTmpValueD;
		U8 plCMDBuf[1024], *plTmpBuf;

		for (i = 0; i < plHandle->m_CurrentNum; i++)
		{
			lLen = 0;
			plTmpBuf = plCMDBuf;

			GLOBAL_MSB8_EC(plTmpBuf, 0x3F, lLen);
			GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
			GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
			GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

			GLOBAL_MSB32_EC(plTmpBuf, i, lLen);

#ifdef PROTOCOL_DEBUG
			CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

			HWL_FPGAWrite(plCMDBuf, lLen);
		}



	}
}


void FIRIIL_ReloadWriteReg(void)
{
	FIRII_RELOAD_HANDLE *plHandle = &s_FirIIReleadHandle;
	if (plHandle)
	{
		S32 i, lLen, lApplyLen, lMaxValueFixed;
		U32 lTmpValue;
		F64 lTmpValueD, lMaxABSValue;
		U8 plCMDBuf[1024], *plTmpBuf;

		/*找到绝对值最大的数*/
		lMaxABSValue = 0.0;
		for (i = 0; i < plHandle->m_CurrentNum; i++)
		{
			if (lMaxABSValue < fabs(plHandle->m_pCoeefData[i]))
			{
				lMaxABSValue = fabs(plHandle->m_pCoeefData[i]);
			}
		}

		lMaxValueFixed = pow(2, MAX_BIT_NUMBER - 1) - 1;

		GLOBAL_TRACE(("MaxABSValue = %f, MaxFixedValue = %d / %d  0x%.8X / 0x%.8X\n", lMaxABSValue, lMaxValueFixed, - lMaxValueFixed, lMaxValueFixed, - lMaxValueFixed))

		for (i = 0; i < plHandle->m_CurrentNum; i++)
		{
			lLen = 0;
			plTmpBuf = plCMDBuf;

			GLOBAL_MSB8_EC(plTmpBuf, 0x3F, lLen);
			GLOBAL_MSB8_EC(plTmpBuf, 0x02, lLen);
			GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
			GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

			lTmpValueD = plHandle->m_pCoeefData[i] / lMaxABSValue;
			lTmpValue = lTmpValueD * lMaxValueFixed;
			GLOBAL_TRACE(("Slot %d, Coeff = %.24f, NormCoeff = %f, RegValue = 0x%.8X / %d\n", i, plHandle->m_pCoeefData[i], lTmpValueD, lTmpValue, lTmpValue));

			GLOBAL_MSB32_EC(plTmpBuf, i, lLen);
			GLOBAL_MSB32_EC(plTmpBuf, lTmpValue, lLen);

#ifdef PROTOCOL_DEBUG
			CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

			HWL_FPGAWrite(plCMDBuf, lLen);
		}



	}
}

