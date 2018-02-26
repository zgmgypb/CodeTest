#include "fpga_ps_gpio.h"

#define GN_RBF_SIZE_LIMIT	(1024*1024*1024)

#define LEVEL_HIGH 1
#define LEVEL_LOW 0

static void FPS_GpioInit(FPS_GpioCfgParam *pCfgParam)
{
	pCfgParam->m_GpioSetupCB(&pCfgParam->m_NStatusPinAddr, TRUE, TRUE, pCfgParam->m_pUserParam);
	if (pCfgParam->m_IsUseConfigDone)
		pCfgParam->m_GpioSetupCB(&pCfgParam->m_ConfigDonePinAddr, TRUE, TRUE, pCfgParam->m_pUserParam);
	pCfgParam->m_GpioSetupCB(&pCfgParam->m_DclkPinAddr, FALSE, TRUE, pCfgParam->m_pUserParam);
	pCfgParam->m_GpioSetupCB(&pCfgParam->m_DataPinAddr, FALSE, TRUE, pCfgParam->m_pUserParam);
	pCfgParam->m_GpioSetupCB(&pCfgParam->m_NConfigPinAddr, FALSE, TRUE, pCfgParam->m_pUserParam);
}

#define NOOP
static BOOL FPS_WriteData(FPS_GpioCfgParam *pCfgParam, U8 *pData, U32 DataSize)
{   
	BOOL lRet = TRUE;
	S32 i = 0, j;
	U8 lCh;

	if(DataSize > 0) {   
		while(i < DataSize) {			 
			lCh = pData[i];  
			for(j = 0; j < 8; j++) { /* 标准PS下载模式 */	
				if((lCh >> j) & 0x01) {
					pCfgParam->m_GpioSetValueCB(&pCfgParam->m_DataPinAddr, LEVEL_HIGH, pCfgParam->m_pUserParam);
				}						
				else {
					pCfgParam->m_GpioSetValueCB(&pCfgParam->m_DataPinAddr, LEVEL_LOW, pCfgParam->m_pUserParam);
				}
				NOOP; /* 时序图这里要求延时，当实际这里Arm的语句执行时间已经超过这个时间，所以用一个空语句代替 */
				pCfgParam->m_GpioSetValueCB(&pCfgParam->m_DclkPinAddr, LEVEL_HIGH, pCfgParam->m_pUserParam);
				NOOP;
				pCfgParam->m_GpioSetValueCB(&pCfgParam->m_DclkPinAddr, LEVEL_LOW, pCfgParam->m_pUserParam);
			}   
			i++;
		}
	}

	return lRet;
}

/* PS下载rbf前的初始化 */
static BOOL FPS_PinInit(FPS_GpioCfgParam *pCfgParam)
{
	int lTimeOutCounter = 0xffff; /* 超时计数 */
	int lData = 0;

	pCfgParam->m_GpioSetValueCB(&pCfgParam->m_DclkPinAddr, 0, pCfgParam->m_pUserParam);
	pCfgParam->m_GpioSetValueCB(&pCfgParam->m_NConfigPinAddr, 1, pCfgParam->m_pUserParam);

	PFC_TaskSleep(10); /* PS下载文档要求此处延时10ms */

	pCfgParam->m_GpioSetValueCB(&pCfgParam->m_NConfigPinAddr, 0, pCfgParam->m_pUserParam);

	PFC_TaskSleep(10); /* PS下载文档要求此处延时10ms */

	//lTimeOutCounter = 0xffff;
	lTimeOutCounter = 200;
	do {
		lData = pCfgParam->m_GpioGetValueCB(&pCfgParam->m_NStatusPinAddr, pCfgParam->m_pUserParam);
		PFC_TaskSleepUS(1);
	}while (lData == LEVEL_HIGH && lTimeOutCounter-- > 0);
	if (lTimeOutCounter <= 0) {
		GLOBAL_TRACE(("read nstatus err! expected 0, but readed 1\n"));
		return FALSE;
	}

	if (pCfgParam->m_IsUseConfigDone) {
		//lTimeOutCounter = 0xffff;
		lTimeOutCounter = 200;
		do {
			lData = pCfgParam->m_GpioGetValueCB(&pCfgParam->m_ConfigDonePinAddr, pCfgParam->m_pUserParam);
			PFC_TaskSleepUS(1);
		}while (lData == LEVEL_HIGH && lTimeOutCounter-- > 0);
		if (lTimeOutCounter <= 0) {
			GLOBAL_TRACE(("read configdone err! expected 0, but readed 1\n"));
			return FALSE;
		}
	}

	pCfgParam->m_GpioSetValueCB(&pCfgParam->m_NConfigPinAddr, LEVEL_HIGH, pCfgParam->m_pUserParam);

	//lTimeOutCounter = 0xffff;
	lTimeOutCounter = 200;
	do {
		lData = pCfgParam->m_GpioGetValueCB(&pCfgParam->m_NStatusPinAddr, pCfgParam->m_pUserParam);
		PFC_TaskSleepUS(1);
	}while (lData == LEVEL_LOW && lTimeOutCounter-- > 0);
	if (lTimeOutCounter <= 0) {
		GLOBAL_TRACE(("read nstatus err! expected 1, but readed 0\n"));
		return FALSE;
	}

	return TRUE;
}

static BOOL FPS_IsEnd(FPS_GpioCfgParam *pCfgParam)
{
	int lTimeOutCounter = 0xffff;
	int lData = 0;

	if (pCfgParam->m_IsUseConfigDone) {
		lTimeOutCounter = 0xffff;
		do {
			lData = pCfgParam->m_GpioGetValueCB(&pCfgParam->m_ConfigDonePinAddr, pCfgParam->m_pUserParam);
			PFC_TaskSleepUS(1);
		}while (lData == LEVEL_LOW && lTimeOutCounter-- > 0);
		if (lTimeOutCounter <= 0) {
			GLOBAL_TRACE(("read configdone err! expected 1, but readed 0\n"));
			return FALSE;
		}
	}

	return TRUE;
}

/* 在FPGA的nconfig脚使用一个低脉冲停止rbf的程序运行，这样后必须重新下载rbf程序 */
void FPS_CleanProg(FPS_GpioCfgParam *pCfgParam)
{
	pCfgParam->m_GpioSetValueCB(&pCfgParam->m_NConfigPinAddr, LEVEL_LOW, pCfgParam->m_pUserParam);
	PFC_TaskSleep(10); 
	pCfgParam->m_GpioSetValueCB(&pCfgParam->m_NConfigPinAddr, LEVEL_HIGH, pCfgParam->m_pUserParam);
}

BOOL FPS_GpioConfig(CHAR_T *pRbfFile, FPS_GpioCfgParam *pCfgParam)
{
	FILE *plRbfFileFd;
	BOOL lRet = TRUE; 
	char *plDataBuf = NULL;
	int lFlen = 0;

	plRbfFileFd = GLOBAL_FOPEN(pRbfFile , "rb") ;
	if (plRbfFileFd == NULL) {
		GLOBAL_TRACE(("GLOBAL_FOPEN File %s Failed\n", pRbfFile));
		return FALSE;
	}

	GLOBAL_FSEEK(plRbfFileFd, 0, SEEK_END);
	lFlen = GLOBAL_FTELL(plRbfFileFd);
	GLOBAL_FSEEK(plRbfFileFd, 0, SEEK_SET);
	if (lFlen > GN_RBF_SIZE_LIMIT) {
		GLOBAL_TRACE(("Rbf feof error!!\n"));
		GLOBAL_FCLOSE(plRbfFileFd);
		return FALSE;
	}

	if ((plDataBuf = (char*) GLOBAL_MALLOC(lFlen)) == NULL) {
		GLOBAL_TRACE(("GLOBAL_MALLOC Error\n"));
		GLOBAL_FCLOSE(plRbfFileFd);
		return FALSE;
	}

	FPS_GpioInit(pCfgParam);

	/* PS开始的时序操作 */
	if (FPS_PinInit(pCfgParam) == FALSE) {
		GLOBAL_TRACE(("FPGA_PS_PIN_INIT Failed"));
		lRet = FALSE;
		goto ret;
	}

	/* 写入配置文件数据 */
	if (lFlen > 0) {
		if (lFlen != GLOBAL_FREAD(plDataBuf, 1, lFlen, plRbfFileFd)) {
			GLOBAL_TRACE(("fread error\n"));
			lRet = FALSE;
			goto ret;
		}
		if (FPS_WriteData(pCfgParam, plDataBuf, lFlen) == FALSE) {
			GLOBAL_TRACE(("write error\n"));
			lRet = FALSE;
			goto ret;
		}
	}	

	/* PS结束判断 */
	if (FPS_IsEnd(pCfgParam) == FALSE) {
		lRet = FALSE;
		goto ret;
	}

ret:
	GLOBAL_FREE(plDataBuf);
	GLOBAL_FCLOSE(plRbfFileFd);

	return lRet;
}
/* EOF */

