/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_tsp.h"


#include "multi_hwl_local_encoder.h"

/* Global Variables (extern)--------------------------------------------------- */

/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */

typedef struct  
{
	MULT_Handle				*m_pMainHandle;

	U32						m_Reserved;
}MULTI_LOCALENCODERParam;

typedef struct  
{
	MULT_Handle				*m_pMainHandle;

	U32						m_Reserved;
}MULTI_LOCALENCODERHandle;

/* Private Variables (static)-------------------------------------------------- */
static MULTI_LOCALENCODERHandle s_MultiLocalEncoderHandle;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

void MULT_LOCALENCODERInitiate(MULT_Handle *pHandle)
{
	U32 lTickCount;
	GLOBAL_ZEROMEM(&s_MultiLocalEncoderHandle, sizeof(MULTI_LOCALENCODERHandle));

	s_MultiLocalEncoderHandle.m_pMainHandle = pHandle;
}



/*参数读写，包括网页提交*/
void MULT_LOCALENCODERXMLLoad(HANDLE32 pXMLRoot, BOOL bPost)
{
	MULTI_LOCALENCODERHandle *plHandle = &s_MultiLocalEncoderHandle;
	if (plHandle)
	{
		CHAR_T *plTmpStr;
		HANDLE32 plXMLHolder;
		HANDLE32 plXMLNodeHolder;

		if (bPost)
		{
			plXMLHolder = pXMLRoot;
		}
		else
		{
			plXMLHolder = mxmlFindElement(pXMLRoot, pXMLRoot, "local_endcoder", NULL, NULL, MXML_DESCEND_FIRST);
		}

		if (plXMLHolder)
		{
		}
	}
}

void MULT_LOCALENCODERXMLSave(mxml_node_t *pXMLRoot, BOOL bStat)
{
	MULTI_LOCALENCODERHandle *plHandle = &s_MultiLocalEncoderHandle;
	if (plHandle)
	{
		CHAR_T plTmpStr[512];
		S32 lTmpStrLen;
		mxml_node_t *plXMLHolder;
		mxml_node_t *plXMLNodeHolder;

		if (bStat)
		{
		}
		else
		{
		}
	}
}

/*SFN参数设置函数*/
void MULT_LOCALENCODERApplyParameter(void)
{
	MULTI_LOCALENCODERHandle *plHandle = &s_MultiLocalEncoderHandle;
	if (plHandle)
	{
	}
}


/*销毁当前模块*/
void MULT_LOCALENCODERTerminate(void)
{
	MULTI_LOCALENCODERHandle *plHandle = &s_MultiLocalEncoderHandle;
	if (plHandle)
	{
	}
}

BOOL MULT_LOCALENCODERProcParam(MULTI_LOCALENCODERParam *pParam, BOOL bRead)
{
	BOOL lRet = FALSE;
	MULTI_LOCALENCODERHandle *plHandle = &s_MultiLocalEncoderHandle;
	if (plHandle)
	{
		lRet = TRUE;
	}
	return lRet;
}


/*EOF*/
