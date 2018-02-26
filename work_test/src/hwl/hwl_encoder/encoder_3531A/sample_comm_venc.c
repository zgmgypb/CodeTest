/******************************************************************************
  Some simple Hisilicon Hi3531 video encode functions.

  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
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

    
const HI_U8 g_SOI[2] = {0xFF, 0xD8};
const HI_U8 g_EOI[2] = {0xFF, 0xD9};
static SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara[VENC_MAX_CHN_NUM];
static HI_S32 gs_s32SnapCnt = 0;

/******************************************************************************
* function : Set venc memory location
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_MemConfig(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;

    HI_CHAR * pcMmzName;
    MPP_CHN_S stMppChnVENC;

    /* group, venc max chn is 64*/
    for(i=0;i<64;i++)
    {
        stMppChnVENC.enModId = HI_ID_VENC;
        stMppChnVENC.s32DevId = 0;
        stMppChnVENC.s32ChnId = i;

        pcMmzName = NULL;  

        /*venc*/
        s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVENC,pcMmzName);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_SYS_SetMemConf with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
    
    return HI_SUCCESS;
}

/******************************************************************************
* function : venc bind vpss           
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_BindVpss(VENC_CHN VeChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 VENC_BindVpss(VENC_CHN VeChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;

	stDestChn.enModId = HI_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = VeChn;

	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

HI_S32 VENC_UnBindVpss(VENC_CHN VeChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;

	stDestChn.enModId = HI_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = VeChn;

	s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

/******************************************************************************
* function : venc unbind vpss           
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_UnBindVpss(VENC_CHN VeChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}


/******************************************************************************
* function : venc bind vo           
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_BindVo(VO_DEV VoDev,VO_CHN VoChn,VENC_CHN VeChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VOU;
    stSrcChn.s32DevId = VoDev;
    stSrcChn.s32ChnId = VoChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

/******************************************************************************
* function : venc unbind vo           
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_UnBindVo(VENC_CHN GrpChn,VO_DEV VoDev,VO_CHN VoChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VOU;
    stSrcChn.s32DevId = VoDev;
    stSrcChn.s32ChnId = VoChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = GrpChn;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}


/******************************************************************************
* function : vdec bind venc          
******************************************************************************/
HI_S32 SAMPLE_COMM_VDEC_BindVenc(VDEC_CHN VdChn,VENC_CHN VeChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

/******************************************************************************
* function : venc unbind vo           
******************************************************************************/
HI_S32 SAMPLE_COMM_VDEC_UnBindVenc(VDEC_CHN VdChn,VENC_CHN VeChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;


    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}


/******************************************************************************
* funciton : get file postfix according palyload_type.
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload, char *szFilePostfix)
{
    if (PT_H264 == enPayload)
    {
        strcpy(szFilePostfix, ".h264");
    }
    else if (PT_JPEG == enPayload)
    {
        strcpy(szFilePostfix, ".jpg");
    }
    else if (PT_MJPEG == enPayload)
    {
        strcpy(szFilePostfix, ".mjp");
    }
    else if (PT_MP4VIDEO == enPayload)
    {
        strcpy(szFilePostfix, ".mp4");
    }
    else
    {
        SAMPLE_PRT("payload type err!\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : save mjpeg stream. 
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveMJpeg(FILE* fpJpegFile, VENC_STREAM_S *pstStream)
{
    VENC_PACK_S*  pstData;
    HI_U32 i;

    //fwrite(g_SOI, 1, sizeof(g_SOI), fpJpegFile); //in Hi3531, user needn't write SOI!

    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        pstData = &pstStream->pstPack[i];
        fwrite(pstData->pu8Addr+pstData->u32Offset, pstData->u32Len-pstData->u32Offset, 1, fpJpegFile);
        fflush(fpJpegFile);
    }

    return HI_SUCCESS;
}

static HI_S32 VENC_SetLostFrameStrategy(VENC_CHN VencChn, HI_U32 MaxBitrate)
{
	VENC_PARAM_FRAMELOST_S lFrmLostParam;

	lFrmLostParam.bFrmLostOpen = HI_TRUE;
	//lFrmLostParam.enFrmLostMode = FRMLOST_NORMAL;
	lFrmLostParam.enFrmLostMode = FRMLOST_PSKIP;
	lFrmLostParam.u32EncFrmGaps = 0;
	lFrmLostParam.u32FrmLostBpsThr = MaxBitrate; /* Bit/s 单位与说明书不一致 */

	return HI_MPI_VENC_SetFrameLostStrategy(VencChn, &lFrmLostParam);
}

static HI_S32 VENC_CloseLostFrameStrategy(VENC_CHN VencChn)
{
	VENC_PARAM_FRAMELOST_S lFrmLostParam;

	HI_MPI_VENC_GetFrameLostStrategy(VencChn, &lFrmLostParam);
	lFrmLostParam.bFrmLostOpen = HI_FALSE;

	return HI_MPI_VENC_SetFrameLostStrategy(VencChn, &lFrmLostParam);
}

HI_S32 VENC_Start(VENC_PARAM_S *pVencParam, HI_BOOL SignalIsLocked, HI_U32 VoFrameRate)
{
    HI_S32 s32Ret;
	HI_U32 lBitrate;

    s32Ret = HI_MPI_VENC_CreateChn(pVencParam->m_VencChn, &pVencParam->m_VencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                pVencParam->m_VencChn, s32Ret);
        return s32Ret;
    }

	switch (pVencParam->m_VencChnAttr.stRcAttr.enRcMode) {
		case VENC_RC_MODE_H264CBR:
			lBitrate = pVencParam->m_VencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate * 1000;
			break;
		case VENC_RC_MODE_H264VBR:
			lBitrate = pVencParam->m_VencChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate * 1000;
			break;
		default:
			lBitrate = 1000000; /* 1M */
	}

	/* Set VUI parameter */
	if (1)
	{
		VENC_PARAM_H264_VUI_S lH264Vui;

		s32Ret = HI_MPI_VENC_GetH264Vui(pVencParam->m_VencChn, &lH264Vui);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_VENC_GetH264Vui faild with%#x!\n", s32Ret);
			return HI_FAILURE;
		}

		lH264Vui.stVuiAspectRatio.aspect_ratio_idc = 1;
		lH264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = 1; // for aspect_ratio_idc
		lH264Vui.stVuiAspectRatio.overscan_appropriate_flag = 0;
		lH264Vui.stVuiAspectRatio.overscan_info_present_flag = 0;
		lH264Vui.stVuiAspectRatio.sar_height = 1;
		lH264Vui.stVuiAspectRatio.sar_width = 1;
		lH264Vui.stVuiTimeInfo.fixed_frame_rate_flag = 1;
		switch (VoFrameRate) {
			case 25:
				lH264Vui.stVuiTimeInfo.num_units_in_tick = 540000;
				lH264Vui.stVuiTimeInfo.time_scale = 27000000;
				break;
			case 2997:
				lH264Vui.stVuiTimeInfo.num_units_in_tick = 450450;
				lH264Vui.stVuiTimeInfo.time_scale = 27000000;
				break;
			case 30:
				lH264Vui.stVuiTimeInfo.num_units_in_tick = 450000;
				lH264Vui.stVuiTimeInfo.time_scale = 27000000;
				break;
			case 50:
				lH264Vui.stVuiTimeInfo.num_units_in_tick = 270000;
				lH264Vui.stVuiTimeInfo.time_scale = 27000000;
				break;
			case 5994:
				lH264Vui.stVuiTimeInfo.num_units_in_tick = 225225;
				lH264Vui.stVuiTimeInfo.time_scale = 27000000;
				break;
			case 60:
				lH264Vui.stVuiTimeInfo.num_units_in_tick = 225000;
				lH264Vui.stVuiTimeInfo.time_scale = 27000000;
				break;
			default:
				break;
		}
		lH264Vui.stVuiTimeInfo.timing_info_present_flag = 1; // 1 for framerate get
		lH264Vui.stVuiBitstreamRestric.bitstream_restriction_flag = 0;
		lH264Vui.stVuiVideoSignal.colour_description_present_flag = 1;
		lH264Vui.stVuiVideoSignal.colour_primaries = 1;
		lH264Vui.stVuiVideoSignal.matrix_coefficients = 1;
		lH264Vui.stVuiVideoSignal.transfer_characteristics = 1;
		lH264Vui.stVuiVideoSignal.video_format = 5;
		lH264Vui.stVuiVideoSignal.video_full_range_flag = 1;
		lH264Vui.stVuiVideoSignal.video_signal_type_present_flag = 1;
		s32Ret = HI_MPI_VENC_SetH264Vui(pVencParam->m_VencChn, &lH264Vui);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_VENC_SetH264Vui faild with%#x!\n", s32Ret);
			return HI_FAILURE;
		}

#if 1 /* 调试打印 */
		s32Ret = HI_MPI_VENC_GetH264Vui(pVencParam->m_VencChn, &lH264Vui);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_VENC_GetH264Vui faild with%#x!\n", s32Ret);
			return HI_FAILURE;
		}
		SAMPLE_PRT("aspect_ratio_idc: %d\n", lH264Vui.stVuiAspectRatio.aspect_ratio_idc);
		SAMPLE_PRT("aspect_ratio_info_present_flag: %d\n", lH264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag);
		SAMPLE_PRT("overscan_appropriate_flag: %d\n", lH264Vui.stVuiAspectRatio.overscan_appropriate_flag);
		SAMPLE_PRT("overscan_info_present_flag: %d\n", lH264Vui.stVuiAspectRatio.overscan_info_present_flag);
		SAMPLE_PRT("sar_height: %d\n", lH264Vui.stVuiAspectRatio.sar_height);
		SAMPLE_PRT("sar_width: %d\n", lH264Vui.stVuiAspectRatio.sar_width);
		SAMPLE_PRT("bitstream_restriction_flag: %d\n", lH264Vui.stVuiBitstreamRestric.bitstream_restriction_flag);
		SAMPLE_PRT("fixed_frame_rate_flag: %d\n", lH264Vui.stVuiTimeInfo.fixed_frame_rate_flag);
		SAMPLE_PRT("num_units_in_tick: %d\n", lH264Vui.stVuiTimeInfo.num_units_in_tick);
		SAMPLE_PRT("time_scale: %d\n", lH264Vui.stVuiTimeInfo.time_scale);
		SAMPLE_PRT("timing_info_present_flag: %d\n", lH264Vui.stVuiTimeInfo.timing_info_present_flag);
		SAMPLE_PRT("colour_description_present_flag: %d\n", lH264Vui.stVuiVideoSignal.colour_description_present_flag);
		SAMPLE_PRT("colour_primaries: %d\n", lH264Vui.stVuiVideoSignal.colour_primaries);
		SAMPLE_PRT("matrix_coefficients: %d\n", lH264Vui.stVuiVideoSignal.matrix_coefficients);
		SAMPLE_PRT("transfer_characteristics: %d\n", lH264Vui.stVuiVideoSignal.transfer_characteristics);
		SAMPLE_PRT("video_format: %d\n", lH264Vui.stVuiVideoSignal.video_format);
		SAMPLE_PRT("video_full_range_flag: %d\n", lH264Vui.stVuiVideoSignal.video_full_range_flag);
		SAMPLE_PRT("video_signal_type_present_flag: %d\n", lH264Vui.stVuiVideoSignal.video_signal_type_present_flag);
#endif
	}

#if 1 /* 方便开关调试 */
	VENC_CloseLostFrameStrategy(pVencParam->m_VencChn); /* 关闭丢帧策略 */
	if (!SignalIsLocked) { /* 没有信号的时候编码码率限制不了进来的码率，会非常大，只有用丢帧策略进行限制码率 */
		s32Ret = VENC_SetLostFrameStrategy(pVencParam->m_VencChn, lBitrate);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("VENC_SetLostFrameStrategy [%d] faild with %#x!\n",\
				pVencParam->m_VencChn, s32Ret);
			return s32Ret;
		}
	}
#endif

    /******************************************
     step 2:  Start Recv Venc Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StartRecvPic(pVencParam->m_VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 VENC_Stop(VENC_CHN VencChn)
{
    HI_S32 s32Ret;

    /******************************************
     step 1:  Stop Recv Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 2:  Distroy Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : Start venc stream mode (h264, mjpeg)
* note      : rate control parameter need adjust, according your case.
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_Start(VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode,HI_U32 u32Profile)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_H264_S stH264Attr;
    VENC_ATTR_H264_CBR_S    stH264Cbr;
    VENC_ATTR_H264_VBR_S    stH264Vbr;
    VENC_ATTR_H264_FIXQP_S  stH264FixQp;
    VENC_ATTR_MJPEG_S stMjpegAttr;
    VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
    VENC_ATTR_JPEG_S stJpegAttr;
    SIZE_S stPicSize;
	VENC_FRAME_RATE_S pstFrameRate;
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enSize, &stPicSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Get picture size failed!\n");
        return HI_FAILURE;
    }

    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVeAttr.enType = enType;
    switch(enType)
    {
        case PT_H264:
        {
            stH264Attr.u32MaxPicWidth  = stPicSize.u32Width;
            stH264Attr.u32MaxPicHeight = stPicSize.u32Height;
            stH264Attr.u32PicWidth     = stPicSize.u32Width;/*the picture width*/
            stH264Attr.u32PicHeight    = stPicSize.u32Height;/*the picture height*/
            stH264Attr.u32BufSize      = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size*/
            stH264Attr.u32Profile      = u32Profile;/*0: baseline; 1:MP; 2:HP 3:svc-t */
            stH264Attr.bByFrame        = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

            if(SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
               // stH264Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
              // stH264Cbr.u32Gop            = 30;
			   if(enSize==PIC_HD1080)
                	{
						 stH264Cbr.u32Gop            =30;
				}
                else{
						 stH264Cbr.u32Gop            = 1;

				}
                stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
               // stH264Cbr.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* input (vi) frame rate */
               // stH264Cbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* target frame rate */
               if(enSize==PIC_HD1080)
                	{
						stH264Cbr.u32SrcFrmRate      = 60;/* input (vi) frame rate */
                		stH264Cbr.fr32DstFrmRate = 30;/* target frame rate */
				}
                else{
						stH264Cbr.u32SrcFrmRate      = 60;/* input (vi) frame rate */
                		stH264Cbr.fr32DstFrmRate = 30;/* target frame rate */
				}
         
                switch (enSize)
                {
                  case PIC_QCIF:
                        stH264Cbr.u32BitRate = 256; /* average bit rate */
                        break;
                  case PIC_QVGA:    /* 320 * 240 */
                  case PIC_CIF: 
                        stH264Cbr.u32BitRate = 512;
                        break;
                  case PIC_D1:
                  case PIC_VGA:	   /* 640 * 480 */
                        stH264Cbr.u32BitRate = 1024*4;
                        break;
                  case PIC_HD720:   /* 1280 * 720 */
                	   stH264Cbr.u32BitRate = 1024*2;
                	   break;
                  case PIC_HD1080:  /* 1920 * 1080 */
                  	   stH264Cbr.u32BitRate = 1024*5;
                	   break;
                  default :
                        stH264Cbr.u32BitRate = 1024*4;
                        break;
                }
                
                stH264Cbr.u32FluctuateLevel = 0; /* average bit rate */
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
            }
            else if (SAMPLE_RC_FIXQP == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32IQp = 20;
                stH264FixQp.u32PQp = 23;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
            }
            else if (SAMPLE_RC_VBR == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                //stH264Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                 if(enSize==PIC_HD1080)
                	{
						 stH264Vbr.u32Gop            = 30;
				}
                else{
						 stH264Vbr.u32Gop            = 15;

				}
                stH264Vbr.u32StatTime = 1;
                //stH264Vbr.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                //stH264Vbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                if(enSize==PIC_HD1080)
                	{
						stH264Vbr.u32SrcFrmRate = 60;
                		stH264Vbr.fr32DstFrmRate = 30;
				}
                else{
						stH264Vbr.u32SrcFrmRate = 60;
                		stH264Vbr.fr32DstFrmRate = 15;

				}
                stH264Vbr.u32MinQp = 10;
                stH264Vbr.u32MaxQp = 40;
                switch (enSize)
                {
                    case PIC_QCIF:
                	   stH264Vbr.u32MaxBitRate= 256*3; /* average bit rate */
                	   break;
                    case PIC_QVGA:    /* 320 * 240 */
                    case PIC_CIF:
                	   stH264Vbr.u32MaxBitRate = 512*3;
                       break;
                    case PIC_D1:
                    case PIC_VGA:	   /* 640 * 480 */
                	   stH264Vbr.u32MaxBitRate = 1024*2;
                        break;
                    case PIC_HD720:   /* 1280 * 720 */
                	   stH264Vbr.u32MaxBitRate = 1024*3;
                	   break;
                    case PIC_HD1080:  /* 1920 * 1080 */
                  	   stH264Vbr.u32MaxBitRate = 1024*6;
                	   break;
                    default :
                        stH264Vbr.u32MaxBitRate = 1024*4*3;
                        break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
            }
            else
            {
                return HI_FAILURE;
            }
        }
        break;
        
        case PT_MJPEG:
        {
            stMjpegAttr.u32MaxPicWidth = stPicSize.u32Width;
            stMjpegAttr.u32MaxPicHeight = stPicSize.u32Height;
            stMjpegAttr.u32PicWidth = stPicSize.u32Width;
            stMjpegAttr.u32PicHeight = stPicSize.u32Height;
            stMjpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
            stMjpegAttr.bByFrame = HI_TRUE;  /*get stream mode is field mode  or frame mode*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));

            if(SAMPLE_RC_FIXQP == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
                stMjpegeFixQp.u32Qfactor        = 90;
                stMjpegeFixQp.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stMjpegeFixQp.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                memcpy(&stVencChnAttr.stRcAttr.stAttrMjpegeFixQp, &stMjpegeFixQp,
                       sizeof(VENC_ATTR_MJPEG_FIXQP_S));
            }
            else if (SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32StatTime       = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32FluctuateLevel = 0;
                switch (enSize)
                {
                    case PIC_QCIF:
                        stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 384*3; /* average bit rate */
                	   break;
                    case PIC_QVGA:    /* 320 * 240 */
                    case PIC_CIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 768*3;
                        break;
                    case PIC_D1:
                    case PIC_VGA:	   /* 640 * 480 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*3*3;
                        break;
                    case PIC_HD720:   /* 1280 * 720 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*5*3;
                	   break;
                    case PIC_HD1080:  /* 1920 * 1080 */
                  	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*10*3;
                	   break;
                    default :
                        stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*7*3;
                        break;
                }
            }
            else if (SAMPLE_RC_VBR == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32StatTime = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL == enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.fr32DstFrmRate = 5;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MinQfactor = 50;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxQfactor = 95;
                switch (enSize)
                {
                    case PIC_QCIF:
                        stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate= 256*3; /* average bit rate */
                	   break;
                    case PIC_QVGA:    /* 320 * 240 */
                    case PIC_CIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 512*3;
                        break;
                    case PIC_D1:
                    case PIC_VGA:	   /* 640 * 480 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*2*3;
                        break;
                    case PIC_HD720:   /* 1280 * 720 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*3*3;
                	   break;
                    case PIC_HD1080:  /* 1920 * 1080 */
                  	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*6*3;
                	   break;
                    default :
                       stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*4*3;
                       break;
                }
            }
            else 
            {
                SAMPLE_PRT("cann't support other mode in this version!\n");
                return HI_FAILURE;
            }
        }
        break;
            
        case PT_JPEG:
            stJpegAttr.u32PicWidth  = stPicSize.u32Width;
            stJpegAttr.u32PicHeight = stPicSize.u32Height;
            stJpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
            stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrJpeg, &stJpegAttr, sizeof(stJpegAttr));
            break;
        default:
            return HI_ERR_VENC_NOT_SUPPORT;
    }

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }
	/*if(1)
		{
	s32Ret = HI_MPI_VENC_GetFrameRate(VencChn, &pstFrameRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_GetFrameRate [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }
	 SAMPLE_PRT("srcfrmrate:%d.\n",pstFrameRate.s32SrcFrmRate);
	 SAMPLE_PRT("s32DstFrmRate:%d.\n",pstFrameRate.s32DstFrmRate);
	pstFrameRate.s32SrcFrmRate=50;
	pstFrameRate.s32DstFrmRate=100;
	s32Ret = HI_MPI_VENC_SetFrameRate(VencChn, &pstFrameRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_SetRcParam [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }

	}*/

    /******************************************
     step 2:  Start Recv Venc Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;

}

/******************************************************************************
* funciton : Stop venc ( stream mode -- H264, MJPEG )
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_Stop(VENC_CHN VencChn)
{
    HI_S32 s32Ret;

    /******************************************
     step 1:  Stop Recv Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 2:  Distroy Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : Start snap
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SnapStart(VENC_CHN VencChn, SIZE_S *pstSize)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_JPEG_S stJpegAttr;

    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVeAttr.enType = PT_JPEG;
    
    stJpegAttr.u32MaxPicWidth  = pstSize->u32Width;
    stJpegAttr.u32MaxPicHeight = pstSize->u32Height;
    stJpegAttr.u32PicWidth  = pstSize->u32Width;
    stJpegAttr.u32PicHeight = pstSize->u32Height;
    stJpegAttr.u32BufSize = pstSize->u32Width * pstSize->u32Height * 2;
    stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
    stJpegAttr.bSupportDCF = HI_FALSE;
    memcpy(&stVencChnAttr.stVeAttr.stAttrJpeg, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : Stop snap
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SnapStop(VENC_CHN VencChn)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n", VencChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n", VencChn, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID SAMPLE_COMM_VENC_ReadOneFrame( FILE * fp, HI_U8 * pY, HI_U8 * pU, HI_U8 * pV,
                                              HI_U32 width, HI_U32 height, HI_U32 stride, HI_U32 stride2)
{
    HI_U8 * pDst;
    HI_U32 u32Row;
    
    pDst = pY;
    for ( u32Row = 0; u32Row < height; u32Row++ )
    {
        fread( pDst, width, 1, fp );
        pDst += stride;
    }
    
    pDst = pU;
    for ( u32Row = 0; u32Row < height/2; u32Row++ )
    {
        fread( pDst, width/2, 1, fp );
        pDst += stride2;
    }
    
    pDst = pV;
    for ( u32Row = 0; u32Row < height/2; u32Row++ )
    {
        fread( pDst, width/2, 1, fp );
        pDst += stride2;
    }
   
}

HI_S32 SAMPLE_COMM_VENC_PlanToSemi(HI_U8 *pY, HI_S32 yStride, 
                       HI_U8 *pU, HI_S32 uStride,
					   HI_U8 *pV, HI_S32 vStride, 
					   HI_S32 picWidth, HI_S32 picHeight)
{
    HI_S32 i;
    HI_U8* pTmpU, *ptu;
    HI_U8* pTmpV, *ptv;
    
    HI_S32 s32HafW = uStride >>1 ;
    HI_S32 s32HafH = picHeight >>1 ;
    HI_S32 s32Size = s32HafW*s32HafH;
        
    pTmpU = malloc( s32Size ); ptu = pTmpU;
    pTmpV = malloc( s32Size ); ptv = pTmpV;
    if((pTmpU==HI_NULL)||(pTmpV==HI_NULL))
    {
        printf("malloc buf failed\n");
        return HI_FAILURE;
    }

    memcpy(pTmpU,pU,s32Size);
    memcpy(pTmpV,pV,s32Size);
    
    for(i = 0;i<s32Size>>1;i++)
    {
        *pU++ = *pTmpV++;
        *pU++ = *pTmpU++;
        
    }
    for(i = 0;i<s32Size>>1;i++)
    {
        *pV++ = *pTmpV++;
        *pV++ = *pTmpU++;        
    }

    free( ptu );
    free( ptv );

    return HI_SUCCESS;
}


