#ifndef _HWL_VRS232_H_
#define _HWL_VRS232_H_
/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
/* Macro ---------------------------------------------------------------------- */
/* Types Define (struct typedef)----------------------------------------------- */
typedef struct  
{
	S32 m_TimeOutMS;
}HWL_VRS232_Param;

/* Functions prototypes ------------------------------------------------------- */
void HWL_VRS232Initiate(void);
void HWL_VRS232Open(S32 Slot, HWL_VRS232_Param *pParam);
void HWL_VRS232Close(S32 Slot);
S32 HWL_VRS232Write(S32 Slot, U8 *pData, S32 DataLen);
S32 HWL_VRS232Read(S32 Slot, U8 *pBuf, S32 BufLen);
S32 HWL_VRS232Flush(S32 Slot);
void HWL_VRS232Terminate(void);
void HWL_VRS232WriteProtocol(S32 Slot, U8 *pData, S32 DataSize);
void HWL_VRS232ReadProtocol(U8 *pData, S32 DataSize);

#endif
/*EOF*/
