#ifndef _DPD_CONTROL_H_
#define _DPD_CONTROL_H_
/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
#include "multi_main_internal.h"
/* Macro ---------------------------------------------------------------------- */
/* Types Define (struct typedef)----------------------------------------------- */
typedef struct  
{
	S32		m_PackageYear;
	S32		m_PackageMonth;
	S32		m_PackageDay;

	S32		m_FPGAYear;
	S32		m_FPGAMonth;
	S32		m_FPGADay;

	S32		m_HardwareYear;
	S32		m_HardwareMonth;
	S32		m_HardwareDay;

	U8		m_pBoardID[16];

}DPD_CONTROL_BOARD_INFO;


typedef struct  
{
	U32		m_CenterFreqHz;
	S32		m_FreqAdj;
	U16		m_AttenuatorLevel;
	U16		m_ALCLevel;
	U8		m_ALCMark;
	U8		m_ToneMark;
	U8		m_GradualChangeMark;
	U8		m_RFDisableMark;
	U8		m_SpecInv;
	U32		m_Reserved;
}DPD_CONTROL_RF_PARAM;




/* Functions prototypes ------------------------------------------------------- */
void DPD_ControlInitiate(void);
BOOL DPD_ControlSendData(CHAR_T *pPathName, S32 Type);
BOOL DPD_ControlRecvData(CHAR_T *pPathName, S32 Type);
BOOL DPD_ControlGetBoardInfoInfo(DPD_CONTROL_BOARD_INFO *pBoardInfo);
BOOL DPD_ControlSetRFParam(DPD_CONTROL_RF_PARAM *pRFParam);
BOOL DPD_ControlSetDPDParam(DPD_CONTROL_DPD_PARAM *pDPDParam);
BOOL DPD_ControlGetDPDStatus(DPD_CONTROL_STATUS *pStatus);
BOOL DPDL_ControlCheckCMDLock(void);
void DPD_ControlTerminate(void);
#endif
/*EOF*/
