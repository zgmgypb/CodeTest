/******************************************************************************
  Some simple Hisilicon HI3531 video input functions.

  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"

/******************************************************************************
* function : Set vpss system memory location
******************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_MemConfig()
{
    HI_CHAR * pcMmzName;
    MPP_CHN_S stMppChnVpss;
    HI_S32 s32Ret, i;

    /*vpss group max is VPSS_MAX_GRP_NUM, not need config vpss chn.*/
    for(i=0;i<VPSS_MAX_GRP_NUM;i++)
    {
        stMppChnVpss.enModId  = HI_ID_VPSS;
        stMppChnVpss.s32DevId = i;
        stMppChnVpss.s32ChnId = 0;

        if(0 == (i%2))
        {
            pcMmzName = NULL;  
        }
        else
        {
            pcMmzName = "ddr1";
        }

        /*vpss*/
        s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVpss, pcMmzName);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Vpss HI_MPI_SYS_SetMemConf ERR !\n");
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_Start(HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt, VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_GRP_PARAM_S stVpssParam = {0};
    HI_S32 s32Ret;
    HI_S32 i, j;

    /*** Set Vpss Grp Attr ***/
    memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
    
    for(i=0; i<s32GrpCnt; i++)
    {
        VpssGrp = i;
        /*** create vpss group ***/
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** set vpss param ***/
        s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        stVpssParam.u32IeStrength = 0;
        s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** enable vpss chn, with frame ***/
        for(j=0; j<s32ChnCnt; j++)
        {
            VpssChn = j;
            /* Set Vpss Chn attr */
            stChnAttr.bSpEn = HI_FALSE;
            stChnAttr.bUVInvert = HI_FALSE;
            stChnAttr.bBorderEn = HI_FALSE;
            stChnAttr.stBorder.u32Color = 0xff00;
            stChnAttr.stBorder.u32LeftWidth = 0;
            stChnAttr.stBorder.u32RightWidth = 0;
            stChnAttr.stBorder.u32TopWidth = 0;
            stChnAttr.stBorder.u32BottomWidth = 0;
            
            s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
    
            s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
        }
        
        /*** start vpss group ***/
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }

    }
    return HI_SUCCESS;
}

HI_S32 VPSS_Start(VPSS_GRP VpssGrp, SIZE_S *pstSize, HI_S32 s32ChnCnt, VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
	VPSS_CHN VpssChn;
	VPSS_CHN_ATTR_S stChnAttr = {0};
	VPSS_GRP_PARAM_S stVpssParam = {0};
	HI_S32 s32Ret;
	HI_S32 j;

	/*** create vpss group ***/
	s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	/*** set vpss param ***/
	s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	stVpssParam.u32IeStrength = 0;
	s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	/*** enable vpss chn, with frame ***/
	for(j=0; j<s32ChnCnt; j++)
	{
		VpssChn = j;
		/* Set Vpss Chn attr */
		stChnAttr.bSpEn = HI_FALSE;
		stChnAttr.bUVInvert = HI_FALSE;
		//stChnAttr.bUVInvert = HI_TRUE;
		stChnAttr.bBorderEn = HI_FALSE;
		stChnAttr.stBorder.u32Color = 0xff00;
		stChnAttr.stBorder.u32LeftWidth = 0;
		stChnAttr.stBorder.u32RightWidth = 0;
		stChnAttr.stBorder.u32TopWidth = 0;
		stChnAttr.stBorder.u32BottomWidth = 0;

		s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}

		s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}
	}

	/*** start vpss group ***/
	s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

/*****************************************************************************
* function : disable vi dev
*****************************************************************************/
HI_S32 VPSS_Stop(VPSS_GRP VpssGrp, HI_S32 s32ChnCnt)
{
    HI_S32 j;
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHN VpssChn;

    s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    for(j=0; j<s32ChnCnt; j++)
    {
        VpssChn = j;
        s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
HI_S32 SAMPLE_COMM_DisableVpssPreScale(VPSS_GRP VpssGrp,SIZE_S stSize)
{
    HI_S32 s32Ret;
    VPSS_PRESCALE_INFO_S stPreScaleInfo;
        
    stPreScaleInfo.bPreScale = HI_FALSE;
    stPreScaleInfo.stDestSize.u32Width = stSize.u32Width;
    stPreScaleInfo.stDestSize.u32Height = stSize.u32Height;
    s32Ret = HI_MPI_VPSS_SetPreScale(VpssGrp, &stPreScaleInfo);
    if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

    return s32Ret;
}
HI_S32 SAMPLE_COMM_EnableVpssPreScale(VPSS_GRP VpssGrp,SIZE_S stSize)
{
    HI_S32 s32Ret;
    VPSS_PRESCALE_INFO_S stPreScaleInfo;
        
    stPreScaleInfo.bPreScale = HI_TRUE;
    stPreScaleInfo.stDestSize.u32Width = stSize.u32Width;
    stPreScaleInfo.stDestSize.u32Height = stSize.u32Height;
    s32Ret = HI_MPI_VPSS_SetPreScale(VpssGrp, &stPreScaleInfo);
    if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

    return s32Ret;
}

/******************************************************************************
* function : VPSS Display ColorBar           
******************************************************************************/
#define COLORBAR_NUM 8
HI_S32 VPSS_DisplayColorBar(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 Width, HI_U32 Height)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stChn;
	RGN_ATTR_S stRegion;
	HI_U32 lColorBar[COLORBAR_NUM] = { // ARGB_1555
		0xFFFF, /* °× */
		0xFFE0, /* »Æ */
		0x83FF, /* Çà */
		0x83E0, /* ÂÌ */
		0xFC1F, /* Æ· */
		0xFC00, /* ºì */
		0x801F, /* À¶ */
		0x8000 /* ºÚ */
	};
	HI_S32 i;
	RGN_HANDLE lHandle;

	stChn.enModId = HI_ID_VPSS;
	stChn.s32DevId = VpssGrp;
	stChn.s32ChnId = VpssChn;
	lHandle = (VpssGrp * 4 + VpssChn) * COLORBAR_NUM;

	for (i = 0; i < COLORBAR_NUM; i++) { 
		RGN_CHN_ATTR_S stRgnChnAttr;

		stRgnChnAttr.bShow = HI_TRUE;
		stRgnChnAttr.enType = OVERLAY_RGN;
		stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE; 
		stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = i * Width / COLORBAR_NUM;
		stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
		stRgnChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 0xFF;
		stRgnChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 0xFF;
		stRgnChnAttr.unChnAttr.stOverlayChn.stQpInfo.bQpDisable = HI_TRUE;
		stRgnChnAttr.unChnAttr.stOverlayChn.u32Layer = i;

		stRegion.enType = OVERLAY_RGN;
		stRegion.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
		stRegion.unAttr.stOverlay.stSize.u32Height = Height;
		stRegion.unAttr.stOverlay.stSize.u32Width = Width / COLORBAR_NUM;
		stRegion.unAttr.stOverlay.u32BgColor = lColorBar[i];
		s32Ret = HI_MPI_RGN_Create(lHandle + i, &stRegion);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("====CHN[%d]failed with %#x!\n", lHandle + i, s32Ret);
			return HI_FAILURE;
		}

		s32Ret = HI_MPI_RGN_AttachToChn(lHandle + i, &stChn, &stRgnChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("====CHN[%d]failed with %#x!\n", lHandle + i, s32Ret);
			return HI_FAILURE;
		}
	}

	return s32Ret;
}

HI_S32 VPSS_UnDisplayColorBar(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stChn;
	HI_S32 i;
	RGN_HANDLE lHandle;

	stChn.enModId = HI_ID_VPSS;
	stChn.s32DevId = VpssGrp;
	stChn.s32ChnId = VpssChn;
	lHandle = (VpssGrp * 4 + VpssChn) * COLORBAR_NUM;

	for (i = 0; i < COLORBAR_NUM; i++) {
		s32Ret = HI_MPI_RGN_DetachFromChn(lHandle + i, &stChn);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("====CHN[%d]failed with %#x!\n", lHandle + i, s32Ret);
			return HI_FAILURE;
		}

		s32Ret = HI_MPI_RGN_Destroy(lHandle + i);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("====CHN[%d]failed with %#x!\n", lHandle + i, s32Ret);
			return HI_FAILURE;
		}
	}

	return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
