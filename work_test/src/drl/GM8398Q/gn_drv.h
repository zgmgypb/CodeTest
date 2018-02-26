#ifndef GN_DRV_H
#define GN_DRV_H

#include "gn_global.h"
#include "../../drivers/fpga_ps_GM8398Q/fpga_ps_driver.h"

BOOL DRL_Initiate(void);
void DRL_Terminate(void);

/* DXT8243操作接口 */
BOOL DRL_Dxt8243Reset(void *pUserParam);
void DRL_Dxt8243UpgradeEnable(BOOL Enable); /* 是否使能8243升级，正常操作时禁止，升级时需要开启使能 */
S32 DRL_Dxt8243Read(void *pUserParam, U8 *pData, S32 DataSize);
S32 DRL_Dxt8243Write(void *pUserParam, U8 *pData, S32 DataSize);

/* CVBS驱动接口*/
S32 DRL_CvbsI2cOpen(void *pUserParam);
S32 DRL_CvbsI2cClose(void *pUserParam, S32 Fd);
S32 DRL_CvbsI2cRead(void *pUserParam, U32 ChNum, S32 Fd, U32 RegAddr, U8 *pData, S32 DataSize);
S32 DRL_CvbsI2cWrite(void *pUserParam, U32 ChNum, S32 Fd, U32 RegAddr, U8 *pData, S32 DataSize);

S32 DRL_CvbsVolumeOpen(void *pUserParam);
S32 DRL_CvbsVolumeClose(void *pUserParam, S32 Fd);
S32 DRL_CvbsVolumeWrite(void *pUserParam, U32 ChNum, S32 Fd, S32 Data);

BOOL DRL_CvbsReset(void *pUserParam);

#endif /* GN_DRV_H */
/* EOF */
