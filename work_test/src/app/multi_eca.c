
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

/*¼ÓÃÜº¯Êý*/
void MULTL_ECAEncryptionCB(void* pUserParam, U8 pKey[16], void* pData, S32 DataLen)
{

}

void MULTL_XMLLoadECA(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bPost)
{
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLHolder;
	MPEG2_DBEncryptedCAParam lECA;

	MPEG2_DBEncryptedCAProc(pHandle->m_DBSHandle, &lECA, TRUE);

	if (bPost)
	{
		plXMLHolder = pXMLRoot;
	}
	else
	{
		plXMLHolder = mxmlFindElement(pXMLRoot, pXMLRoot, "eca_setting", NULL, NULL, MXML_DESCEND_FIRST);
	}

	if (plXMLHolder)
	{
		lECA.m_ActiveMark = MULTL_XMLGetNodeMark(plXMLHolder, "eca_active_mark");
		lECA.m_EncryptMark = MULTL_XMLGetNodeMark(plXMLHolder, "eca_encrypt_mark");
		lECA.m_DescTag = MULTL_XMLGetNodeUINT(plXMLHolder, "eca_desc_tag", 10);
		lECA.m_LocationID = MULTL_XMLGetNodeUINT(plXMLHolder, "local_id", 16);

		MPEG2_DBEncryptedCAProc(pHandle->m_DBSHandle, &lECA, FALSE);
	}
	else
	{
		GLOBAL_TRACE(("Find eca_setting Failed!\n"));
	}
}


void MULTL_XMLSaveECA(MULT_Handle *pHandle, mxml_node_t *pXMLRoot)
{
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLHolder;
	MPEG2_DBEncryptedCAParam lECA;

	MPEG2_DBEncryptedCAProc(pHandle->m_DBSHandle, &lECA, TRUE);

	plXMLHolder = mxmlNewElement(pXMLRoot, "eca_setting");
	if (plXMLHolder)
	{
		MULTL_XMLAddNodeMark(plXMLHolder, "eca_active_mark", lECA.m_ActiveMark);
		MULTL_XMLAddNodeMark(plXMLHolder, "eca_encrypt_mark", lECA.m_EncryptMark);
		MULTL_XMLAddNodeUINT(plXMLHolder, "eca_desc_tag", lECA.m_DescTag);
		MULTL_XMLAddNodeHEX(plXMLHolder, "local_id", lECA.m_LocationID);
	}
}
#endif
/*EOF*/
