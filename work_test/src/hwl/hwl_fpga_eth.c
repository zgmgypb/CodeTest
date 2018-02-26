/* Includes-------------------------------------------------------------------- */
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "mpeg2_micro.h"
#include "multi_private.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
static HWL_FPGAEthTSCB s_pFPGATSCB;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

/* FPGA ETH Ïà¹Øº¯Êý -------------------------------------------------------------------------------------------------------------------------------*/
void HWL_FPGAEthSetCB(HWL_FPGAEthTSCB pCB)
{
	s_pFPGATSCB = pCB;
}

void HWL_FPGAETHSendToFPGA(S32 Index, U8 *pTsData)
{
	S32 i, lLen;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf;

	lLen = 0;
	plTmpBuf = plCMDBuf;
	GLOBAL_MSB8_EC(plTmpBuf, ICPL_TAGID_FPGA_ETH_TS, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x30, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	GLOBAL_BYTES_SC(plTmpBuf, 3, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, Index, lLen);
	GLOBAL_BYTES_EC(plTmpBuf, pTsData, MPEG2_TS_PACKET_SIZE, lLen);

#ifdef DEBUG_IP_TO_TS_PROTOCOL
	CAL_PrintDataBlock(__FUNCTION__, plCMDBuf, lLen);
#endif

	HWL_FPGAWrite(plCMDBuf, lLen);
}


void HWL_FPGAEthRedvFromFPGA(U8 *pData, S32 DataLen)
{
#ifdef DEBUG_IP_TO_TS_PROTOCOL
	CAL_PrintDataBlock(__FUNCTION__, pData, DataLen);
#endif
	if (s_pFPGATSCB)
	{
		s_pFPGATSCB(ICPL_TAGID_FPGA_ETH_TS, pData + 8);
	}
}

/*EOF*/
