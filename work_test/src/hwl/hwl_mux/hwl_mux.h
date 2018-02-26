#ifndef HWL_MUX_H
#define HWL_MUX_H

/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
#include "platform_assist.h"
#include "libc_assist.h" 
#include "multi_main_internal.h"

/* Macro ---------------------------------------------------------------------- */


/* Types Define (struct typedef)----------------------------------------------- */
typedef struct {
	
} HWL_MuxInitParam;

/* Functions prototypes ------------------------------------------------------- */
HANDLE32 HWL_MuxCreate(HWL_MuxInitParam *pInitParam);
void HWL_MuxDestroy(HANDLE32 MuxHandle);

#endif /* HWL_MUX_H */
