#ifndef _MULTI_HWL_IGMP_H_
#define _MULTI_HWL_IGMP_H_
/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
/* Macro ---------------------------------------------------------------------- */
/* Types Define (struct typedef)----------------------------------------------- */
/* Functions prototypes ------------------------------------------------------- */
void HWL_IGMPClear(void);
void HWL_IGMPSetting(BOOL bRepeateMark, S16 RepeateTime, S16 IGMPVersion);
void HWL_IGMPApply(S32 ChnIndex, BOOL bJoin);
void HWL_IGMPTAdd(S32 ChnIndex, U32 IPAddr, U32 SrcAddr, U16 SrcPort);

#endif
/*EOF*/
