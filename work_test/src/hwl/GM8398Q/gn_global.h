#ifndef GN_GLOBAL_H
#define GN_GLOBAL_H

#include "global_def.h"
#include "global_micros.h"
#include "linuxplatform.h"
#include "libc_assist.h"
#include "mpeg2.h"
#include "mpeg2_micro.h"

#define HWL_VID_PID_BASE	(0x80)
#define HWL_AUD_PID_BASE	(0x90)
#define HWL_PMT_PID_BASE	(0xA0)
#define HWL_PCR_PID_BASE	(0xB0)

typedef unsigned int (*tDelayFunc)(unsigned int);
//void GN_CountDown(char *pTitle, long Counter, tDelayFunc DelayFunc);

#endif /* GN_GLOBAL_H */
/* EOF */
