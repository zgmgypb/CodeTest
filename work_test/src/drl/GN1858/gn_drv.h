#ifndef GN_DRV_H
#define GN_DRV_H

#include "gn_global.h"
#include "../../drivers/fpga_ps/fpga_ps_driver.h"
/*
typedef enum
{
	GN_FPGA_INDEX_MAIN = 0,
	GN_FPGA_INDEX_IP_ASI_OUTPUT
}GN_FpgaIndex;
*/

BOOL DRL_Initiate(void);
void DRL_Terminate(void);

/* 主板FPGA驱动接口 */
//BOOL DRL_MainFpgaConfig(void);
//S32 DRL_MainFpgaRead(U32 RegAddr, CHAR_T *pData, U32 DataLen);
//S32 DRL_MainFpgaWrite(U32 RegAddr, CHAR_T *pData, U32 DataLen);

/* CVBS驱动接口 */
S32 DRL_CvbsI2cOpen(void *pUserParam);
S32 DRL_CvbsI2cClose(void *pUserParam, S32 Fd);
S32 DRL_CvbsI2cRead(void *pUserParam, U32 ChNum, S32 Fd, U32 RegAddr, U8 *pData, S32 DataSize);
S32 DRL_CvbsI2cWrite(void *pUserParam, U32 ChNum, S32 Fd, U32 RegAddr, U8 *pData, S32 DataSize);

S32 DRL_CvbsVolumeOpen(void *pUserParam);
S32 DRL_CvbsVolumeClose(void *pUserParam, S32 Fd);
S32 DRL_CvbsVolumeWrite(void *pUserParam, U32 ChNum, S32 Fd, S32 Data);

BOOL DRL_CvbsReset(void *pUserParam);

/* 编码板VIXS驱动接口 */
BOOL DRL_VixsReset(void *pUserParam);
S32 DRL_VixsUartOpen(void *pUserParam);
S32 DRL_VixsUartClose(void *pUserParam, S32 Fd);
S32 DRL_VixsUartRead(void *pUserParam, S32 Fd, U8 *pData);
S32 DRL_VixsUartWrite(void *pUserParam, S32 Fd, U8 *pData, S32 DataSize);
S32 DRL_UartOpen(U32 UartIndex);
S32 DRL_UartClose(S32 Fd);
S32 DRL_UartRead(S32 Fd, U8 *pData);
S32 DRL_UartWrite(S32 Fd, U8 *pData, S32 DataSize);

/* FPGA操作接口 */
/*
U8 READ_FPGA(S32 B_TYPE, U32 ADDRESS);
void WRITE_FPGA(S32 B_TYPE, U32 ADDRESS, U32 DATA);
*/
#endif /* GN_DRV_H */
/* EOF */
