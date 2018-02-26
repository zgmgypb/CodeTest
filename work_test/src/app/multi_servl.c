
/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
#ifdef MULT_ENABLE_ECA_AND_SERVICE_LIST
void MULTL_XMLLoadSERVL(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bPost)
{
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLHolder;
	MPEG2_DBServListParam lSERVL;

	MPEG2_DBServListProc(pHandle->m_DBSHandle, &lSERVL, TRUE);

	if (bPost)
	{
		plXMLHolder = pXMLRoot;
	}
	else
	{
		plXMLHolder = mxmlFindElement(pXMLRoot, pXMLRoot, "servl_setting", NULL, NULL, MXML_DESCEND_FIRST);
	}

	if (plXMLHolder)
	{
		lSERVL.m_ActiveMark = MULTL_XMLGetNodeMark(plXMLHolder, "serv_active_mark");
		lSERVL.m_EncryptMark = MULTL_XMLGetNodeMark(plXMLHolder, "serv_encrypt_mark");
		lSERVL.m_ServListPID = MULTL_XMLGetNodeUINT(plXMLHolder, "servl_pid", 10);
		lSERVL.m_ServListTableID = MULTL_XMLGetNodeUINT(plXMLHolder, "servl_table_id", 10);
		lSERVL.m_DescTag = MULTL_XMLGetNodeUINT(plXMLHolder, "servl_desc_tag", 10);
		lSERVL.m_ServListBps = MULTL_XMLGetNodeUINT(plXMLHolder, "servl_bps", 10);
		lSERVL.m_LocationID = MULTL_XMLGetNodeUINT(plXMLHolder, "local_id", 16);

		MPEG2_DBServListProc(pHandle->m_DBSHandle, &lSERVL, FALSE);
	}
	else
	{
		GLOBAL_TRACE(("Find eca_setting Failed!\n"));
	}
}


void MULTL_XMLSaveSERVL(MULT_Handle *pHandle, mxml_node_t *pXMLRoot)
{
	CHAR_T plTmpStr[512];
	S32 lTmpStrLen;
	mxml_node_t *plXMLHolder;
	MPEG2_DBServListParam lSERVL;

	MPEG2_DBServListProc(pHandle->m_DBSHandle, &lSERVL, TRUE);

	plXMLHolder = mxmlNewElement(pXMLRoot, "servl_setting");
	if (plXMLHolder)
	{
		MULTL_XMLAddNodeMark(plXMLHolder, "serv_active_mark", lSERVL.m_ActiveMark);
		MULTL_XMLAddNodeMark(plXMLHolder, "serv_encrypt_mark", lSERVL.m_EncryptMark);
		MULTL_XMLAddNodeUINT(plXMLHolder, "servl_pid", lSERVL.m_ServListPID);
		MULTL_XMLAddNodeUINT(plXMLHolder, "servl_table_id", lSERVL.m_ServListTableID);
		MULTL_XMLAddNodeUINT(plXMLHolder, "servl_desc_tag", lSERVL.m_DescTag);

		lTmpStrLen = CAL_StringBinToHex(lSERVL.m_pDescContent, lSERVL.m_ContentLen, plTmpStr, sizeof(plTmpStr), TRUE);
		plTmpStr[lTmpStrLen] = 0;

		MULTL_XMLAddNodeCDData(plXMLHolder, "servl_desc_content", plTmpStr);
		MULTL_XMLAddNodeUINT(plXMLHolder, "servl_bps", lSERVL.m_ServListBps);
		MULTL_XMLAddNodeHEX(plXMLHolder, "local_id", lSERVL.m_LocationID);
	}
}
#endif

/*EOF*/
