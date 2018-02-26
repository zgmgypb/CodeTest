#ifndef _REMOTE_CONTROL_H_
#define _REMOTE_CONTROL_H_
/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
#include "multi_main_internal.h"
/* Macro ---------------------------------------------------------------------- */
/* Types Define (struct typedef)----------------------------------------------- */
/* Functions prototypes ------------------------------------------------------- */
void MULT_RemoteInitiate(MULT_Handle *pHandle);
void MULT_RemoteStart(void);
void MULT_RemoteStop(void);
void MULT_RemoteTerminate(void);
#endif
/*EOF*/
