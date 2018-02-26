#ifndef _MUL_MULT_H_
#define _MUL_MULT_H_

/* Includes-------------------------------------------------------------------- */
#include "global_def.h"
/* Macro ---------------------------------------------------------------------- */
/* Types Define (struct typedef)----------------------------------------------- */

/* Functions prototypes ------------------------------------------------------- */
#ifdef __cplusplus 
extern "C" { 
#endif 

U32 MULT_Enter(void);
void MULT_ForceTerminate(void);
BOOL MULT_IsRunning(void);
void MULT_TestInputFunction(U8 MsgParam);

#ifdef __cplusplus 
} 
#endif 


#endif
