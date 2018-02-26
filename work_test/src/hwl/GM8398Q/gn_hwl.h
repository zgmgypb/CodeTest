#ifndef GN_HWL_H
#define GN_HWL_H

#include "gn_global.h"
#include "gn_hwl_dxt8243.h"
#include "gn_hwl_cvbs.h"
#include "gn_hwl_mfpga.h"
#include "hwl_ds8b20.h"

BOOL HWL_GN_Init();
BOOL HWL_GN_Terminate();

GN_Dx8243HwInfo* GetDx8243Handle(void);
GN_CvbsHwInfo* GetCvbsHandle(void);

BOOL HWL_CheckEncoderDx8243Exist(S32 lIndex);
BOOL HWL_CheckEncoderCvbsExist(S32 lIndex);

#endif /* GN_HWL_H */
/* EOF */
