#include "gn_hwl.h"
#include "gn_drv.h"

///////////////////////////////////////////////回调函数HANDLE START///////////////////////////////////////////////
static GN_Dx8243HwInfo s_GN_Dx8243HwInfo;
static GN_CvbsHwInfo s_GN_CvbsHwInfo;

GN_Dx8243HwInfo* GetDx8243Handle(void)
{
	return &s_GN_Dx8243HwInfo;
}

GN_CvbsHwInfo* GetCvbsHandle(void)
{
	return &s_GN_CvbsHwInfo;
}

BOOL HWL_CheckEncoderDx8243Exist(S32 lIndex)
{
	return s_GN_Dx8243HwInfo.m_Dxt8243IsExist[lIndex];
}

BOOL HWL_CheckEncoderCvbsExist(S32 lIndex)
{
	return s_GN_CvbsHwInfo.m_CvbsIsExist[lIndex];
}

///////////////////////////////////////////////回调函数HANDLE START///////////////////////////////////////////////


BOOL HWL_GN_Init()
{
	S32 i;

	//ds8b20 温控芯片
	Hwl_Ds18b20Init();

	//mfpga mutex init
	MFPGA_Init();

	DRL_Initiate();

///////////////////////////////////////////////DXT8243模块初始化 START///////////////////////////////////////////////
	{
		GN_Dx8243HwInfo *pDx8243HwInfo = &s_GN_Dx8243HwInfo;

		for (i=0; i<GN_ENC_BOARD_NUM; i++)
		{	
			DXT8243_InitParam lInitParam;

			lInitParam.m_pReset = DRL_Dxt8243Reset;
			lInitParam.m_pUartRead = DRL_Dxt8243Read;
			lInitParam.m_pUartWrite = DRL_Dxt8243Write;
			lInitParam.m_pUserParam = (void *)i;

			GLOBAL_PRINTF(("Dxt8243_Create Object-%d\n", i));
			pDx8243HwInfo->m_Dxt8243Handle[i] = DXT8243_Create(&lInitParam);
			if (pDx8243HwInfo->m_Dxt8243Handle[i] == NULL)
			{
				GLOBAL_TRACE(("Dxt8243_Create Failed!!\n"));
				continue;
			}
		}

		HWL_DXT8243Reset(pDx8243HwInfo);

		for (i=0; i<GN_ENC_BOARD_NUM; i++)
		{	
			DXT8243_DeviceInfo lDevInfo;

			if (DXT8243_GetDeviceInfo(pDx8243HwInfo->m_Dxt8243Handle[i], &lDevInfo) == FALSE) /* 用获取设备信息的方式判断是否存在 */
			{
				pDx8243HwInfo->m_Dxt8243IsExist[i] = FALSE;
				DXT8243_Destroy(pDx8243HwInfo->m_Dxt8243Handle[i]);
				pDx8243HwInfo->m_Dxt8243Handle[i] = NULL;
				GLOBAL_TRACE(("DXT8243 SubBoard [%d] Not Exist!!\n", i));
			}
			else
			{
				pDx8243HwInfo->m_Dxt8243IsExist[i] = TRUE;

				GLOBAL_PRINTF(("--------Encode Board[%d] Device Info --------\n", i));
				GLOBAL_PRINTF(("ProductID: %X\n", lDevInfo.m_ProductID));
				GLOBAL_PRINTF(("Major Version: %d\n", lDevInfo.m_MajorVer));
				GLOBAL_PRINTF(("Minor Version: %d\n", lDevInfo.m_MinorVer));
				GLOBAL_PRINTF(("Build Version: %d\n", lDevInfo.m_BuildVersion));
				GLOBAL_PRINTF(("Description: %s\n", lDevInfo.m_Description));
				GLOBAL_PRINTF(("Build Date: %s\n", lDevInfo.m_BuildDate));
				GLOBAL_PRINTF(("Build Time: %s\n", lDevInfo.m_BuildTime));
				GLOBAL_PRINTF(("--------------------------------------------------\n"));
			}
		}
	}
///////////////////////////////////////////////DXT8243模块初始化 END///////////////////////////////////////////////

///////////////////////////////////////////////CVBS模块初始化 START///////////////////////////////////////////////
	{
		GN_CvbsHwInfo *pCvbsHwInfo = &s_GN_CvbsHwInfo;

		for (i=0; i<GN_CVBS_SUBBOARD_NUM; i++)
		{
			CVBS_InitParam lInitParam;

			lInitParam.m_ChannelNum = GN_CVBS_CHANNEL_NUM;
			lInitParam.m_pI2cOpen = DRL_CvbsI2cOpen;
			lInitParam.m_pI2cClose = DRL_CvbsI2cClose;
			lInitParam.m_pI2cRead = DRL_CvbsI2cRead;
			lInitParam.m_pI2cWrite = DRL_CvbsI2cWrite;
			lInitParam.m_pReset = DRL_CvbsReset;
			lInitParam.m_pVolumeOpen = DRL_CvbsVolumeOpen;
			lInitParam.m_pVolumeClose = DRL_CvbsVolumeClose;
			lInitParam.m_pVolumeWrite = DRL_CvbsVolumeWrite;
			lInitParam.m_pUserParam = (void *)i;
			pCvbsHwInfo->m_CvbsHandle[i] = CVBS_Create(&lInitParam);

			if (pCvbsHwInfo->m_CvbsHandle[i] == NULL)
			{
				GLOBAL_TRACE(("CVBS_Create Failed!!\n"));
				continue;
			}

			if (CVBS_Detect(pCvbsHwInfo->m_CvbsHandle[i]) == TRUE)
			{
				pCvbsHwInfo->m_CvbsIsExist[i] = TRUE;
				GLOBAL_TRACE(("CVBS SubBoard [%d] Exist!!\n", i));
			}
			else
			{
				pCvbsHwInfo->m_CvbsIsExist[i] = FALSE;
				CVBS_Destroy(pCvbsHwInfo->m_CvbsHandle[i]);
				pCvbsHwInfo->m_CvbsHandle[i] = NULL;
				GLOBAL_TRACE(("CVBS SubBoard [%d] Not Exist!!\n", i));
			}
		}
	}
///////////////////////////////////////////////CVBS模块初始化 END///////////////////////////////////////////////

	return TRUE;
}

BOOL HWL_GN_Terminate()
{
	S32 i;
	
	//ds8b20 温控芯片
	Hwl_Ds18b20Close();

	//destory mfpge mutex
	MFPGA_Terminate();

///////////////////////////////////////////////DXT8243模块注销 START///////////////////////////////////////////////
	{
		GN_Dx8243HwInfo *pDx8243HwInfo = &s_GN_Dx8243HwInfo;

		if (pDx8243HwInfo)
		{
			for (i=0; i<GN_ENC_BOARD_NUM; i++)
			{
				if (pDx8243HwInfo->m_Dxt8243Handle[i])
				{
					DXT8243_Destroy(pDx8243HwInfo->m_Dxt8243Handle[i]);
					pDx8243HwInfo->m_Dxt8243Handle[i] = NULL;
				}
			}
		}
	}
///////////////////////////////////////////////DXT8243模块注销 END///////////////////////////////////////////////

///////////////////////////////////////////////CVBS模块初注销 START///////////////////////////////////////////////
	{
		GN_CvbsHwInfo *pCvbsHwInfo = &s_GN_CvbsHwInfo;

		if (pCvbsHwInfo)
		{
			for (i = 0; i < GN_CVBS_SUBBOARD_NUM; ++i)
			{
				if (pCvbsHwInfo->m_CvbsHandle[i])
				{
					CVBS_Destroy(pCvbsHwInfo->m_CvbsHandle[i]);
					pCvbsHwInfo->m_CvbsHandle[i] = NULL;
				}
			}
		}

	}
///////////////////////////////////////////////CVBS模块初注销 END///////////////////////////////////////////////
	return TRUE;
}




