#include "gn_hwl_cvbs.h"

BOOL HWL_SetEncParaToCvbs(HANDLE32 lHandle, HWL_CvbsCfgParam CvbsCfgParam[GN_CVBS_CHANNEL_NUM])
{
	if(lHandle != NULL)
	{
		//如需对参数进一步处理在此添加代码
		if(CVBS_SetPara(lHandle, CvbsCfgParam) == FALSE)
		{
			GLOBAL_TRACE(("Cvbs Set Para Error!\n"));
			return FALSE;
		}
		else
		{	
			GLOBAL_TRACE(("Cvbs Set Para Success!\n"));
			return TRUE;
		}
	}
	else
		return FALSE;
}

BOOL HWL_GetEncoderCvbsLockStatus(HANDLE32 lHandle, HWL_CvbsStatusParam EncoderCvbsLockStatusPara[GN_CVBS_CHANNEL_NUM])
{
	if(lHandle != NULL)
	{
		S32 i;

		for (i=0; i<GN_CVBS_CHANNEL_NUM; i++)
		{
			CVBS_GetPara(lHandle, &EncoderCvbsLockStatusPara[i], i); 
		}
		return TRUE;
	}
	else
		return FALSE;	
}



