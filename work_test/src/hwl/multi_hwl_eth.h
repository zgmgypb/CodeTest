#ifndef _MULTI_HWL_ETH_H_
#define _MULTI_HWL_ETH_H_
/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
/* Macro ---------------------------------------------------------------------- */
/* Types Define (struct typedef)----------------------------------------------- */
/* Functions prototypes ------------------------------------------------------- */
void HWL_ETHSetChnParameter(S16 ChnIndex, U32 IPAddr, U8 pMAC[GLOBAL_MAC_BUF_SIZE], BOOL bInput, U32 MaxBitrate);
void HWL_ETHLoopSelect(S16 ChnIndex,U8 ScrambleMark, U8 PCRActive);
void HWL_ETHClear(BOOL bInput);
void HWL_ETHAdd(S32 ChnIndex, S32 SubIndex, U8 Protocol, U32 DesAddr, U16 DesPort, BOOL8 Active, BOOL bInput, U32 MaxBitrate);
void HWL_ETHApply(S32 ChnIndex, BOOL bInput);

#endif
/*EOF*/
