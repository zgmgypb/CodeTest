#ifndef GN_GLOBAL_H
#define GN_GLOBAL_H

#include "global_def.h"
#include "global_micros.h"
#include "linuxplatform.h"
#include "libc_assist.h"
#include "mpeg2.h"
#include "mpeg2_micro.h"

/* CVBS板参数 */
#define GN_CVBS_SUBBOARD_NUM (2)
#define GN_CVBS_CHANNEL_NUM (4) /* 每张子板的通道数 */
#define GN_CVBS_CH_NUM (GN_CVBS_SUBBOARD_NUM * GN_CVBS_CHANNEL_NUM) /* 总通道数 */

/* 编码板参数 */
#define GN_VIXS_SUBBOARD_NUM (4)
#define GN_ENC_CH_NUM (8)

#define Encoder_Error_Count_Max 40

#define MPTS 0
#define SPTS 1

enum
{
	ENC_WORK_OFF = 0,
	ENC_WORK_ON
};

enum
{
	VIXS_INDEX_1 = 0,
	VIXS_INDEX_2,
	VIXS_INDEX_3,
	VIXS_INDEX_4,
	VIXS_NUM
};


#endif /* GN_GLOBAL_H */
/* EOF */
