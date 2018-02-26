#ifndef GN_HWL_CVBS_H
#define GN_HWL_CVBS_H

#include "cvbs.h"
#include "gn_global.h"

/* CVBS板参数 */
#define GN_CVBS_SUBBOARD_NUM (2)
#define GN_CVBS_CHANNEL_NUM (4) /* 每张子板的通道数 */
#define GN_CVBS_CH_NUM (GN_CVBS_SUBBOARD_NUM * GN_CVBS_CHANNEL_NUM) /* 总通道数 */


#define HWL_CvbsCfgParam CVBS_ConfigParam
#define HWL_CvbsStatusParam CVBS_StatusPara

typedef struct
{
	HANDLE32 m_CvbsHandle[GN_CVBS_SUBBOARD_NUM];
	BOOL m_CvbsIsExist[GN_CVBS_SUBBOARD_NUM];
}GN_CvbsHwInfo; 

BOOL HWL_SetEncParaToCvbs(HANDLE32 Handle, HWL_CvbsCfgParam CvbsCfgParam[GN_CVBS_CHANNEL_NUM]);

BOOL HWL_GetEncoderCvbsLockStatus(HANDLE32 lHandle, HWL_CvbsStatusParam EncoderCvbsLockStatusPara[GN_CVBS_CHANNEL_NUM]);

























#endif /* GN_HWL_H */
/* EOF */
