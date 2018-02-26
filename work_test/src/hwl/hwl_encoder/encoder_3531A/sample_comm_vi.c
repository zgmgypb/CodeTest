/******************************************************************************
  Some simple Hisilicon HI3531 video input functions.

  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-8 Created
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
//#include "tw2865.h"
//#include "tw2960.h"

VI_DEV_ATTR_S DEV_ATTR_BT656D1_4MUX =
{
    /*interface mode*/
    VI_MODE_BT656,
    /*work mode, 1/2/4 multiplex*/
    VI_WORK_MODE_4Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},

	/* for single/double edge, must be set when double edge*/
	VI_CLK_EDGE_SINGLE_UP,
	
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, just support yuv*/
    VI_INPUT_DATA_YVYU,
    /*sync info*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*timing info*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*whether use isp*/
    VI_PATH_BYPASS,
    /*data type*/
    VI_DATA_TYPE_YUV
};

VI_DEV_ATTR_S DEV_ATTR_TP2823_720P_2MUX_BASE =
{
    /*interface mode*/
    VI_MODE_BT656,
    /*work mode, 1/2/4 multiplex*/
    VI_WORK_MODE_2Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},

	/* for single/double edge, must be set when double edge*/
	VI_CLK_EDGE_SINGLE_UP,
	
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, just support yuv*/
    VI_INPUT_DATA_YVYU,
    /*sync info*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*timing info*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*whether use isp*/
    VI_PATH_BYPASS,
    /*data type*/
    VI_DATA_TYPE_YUV
};


VI_DEV_ATTR_S DEV_ATTR_TP2823_720P_1MUX_BASE =
{
    /*interface mode*/
    VI_MODE_BT656,
    /*work mode, 1/2/4 multiplex*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},

	/* for single/double edge, must be set when double edge*/
	VI_CLK_EDGE_SINGLE_UP,
	
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, just support yuv*/
    VI_INPUT_DATA_YVYU,
    /*sync info*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*timing info*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*whether use isp*/
    VI_PATH_BYPASS,
    /*data type*/
    VI_DATA_TYPE_YUV
};


VI_DEV_ATTR_S DEV_ATTR_GV7704_1080P_1MUX_BASE =
{
    /*�ӿ�ģʽ*/
	VI_MODE_BT1120_INTERLEAVED,
    /*1��2��4·����ģʽ*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},

	/* ˫������ʱ�������� */
	//VI_CLK_EDGE_SINGLE_UP,  //@30
    VI_CLK_EDGE_DOUBLE,

    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, ��֧��YUV��ʽ*/
	VI_INPUT_DATA_UVUV,
    /*ͬ����Ϣ����Ӧreg�ֲ����������, --bt1120ʱ����Ч*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*timing��Ϣ����Ӧreg�ֲ����������*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*ʹ���ڲ�ISP*/
    VI_PATH_BYPASS,
    /*�����������*/
    VI_DATA_TYPE_YUV
};

VI_DEV_ATTR_S DEV_ATTR_7441_BT1120_STANDARD_BASE =
{
    /*interface mode*/
    VI_MODE_BT1120_STANDARD,
    /*work mode, 1/2/4 multiplex*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0xFF0000},

	/* for single/double edge, must be set when double edge*/
	VI_CLK_EDGE_SINGLE_UP,
	
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, just support yuv*/
    VI_INPUT_DATA_UVUV,

    /*sync info*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,

    /*timing info*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1920,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            1080,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*whether use isp*/
    VI_PATH_BYPASS,
    /*data type*/
    VI_DATA_TYPE_YUV

};

VI_DEV_ATTR_S DEV_ATTR_BT656_1MUX_SINGLE =
{
	/*interface mode*/
	VI_MODE_BT656,
	/*work mode, 1/2/4 multiplex*/
	VI_WORK_MODE_1Multiplex,
	/* r_mask    g_mask    b_mask*/
	{0xFF000000,    0x0},

	/* for single/double edge, must be set when double edge*/
	VI_CLK_EDGE_SINGLE_UP,

	/*AdChnId*/
	{-1, -1, -1, -1},
	/*enDataSeq, just support yuv*/
	VI_INPUT_DATA_YVYU,
	/*sync info*/
	{
		/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

			/*timing info*/
			/*hsync_hfb    hsync_act    hsync_hhb*/
		{0,            0,        0,
		/*vsync0_vhb vsync0_act vsync0_hhb*/
		0,            0,        0,
		/*vsync1_vhb vsync1_act vsync1_hhb*/
		0,            0,            0}
	},
		/*whether use isp*/
		VI_PATH_BYPASS,
		/*data type*/
		VI_DATA_TYPE_YUV
};

VI_DEV_ATTR_S DEV_ATTR_BT1120_1MUX_SINGLE =
{
	/*interface mode*/
	VI_MODE_BT1120_STANDARD,
	/*work mode, 1/2/4 multiplex*/
	VI_WORK_MODE_1Multiplex,
	/* r_mask    g_mask    b_mask*/
	{0xFF000000,    0xFF0000},

	/* for single/double edge, must be set when double edge*/
	VI_CLK_EDGE_SINGLE_UP,

	/*AdChnId*/
	{-1, -1, -1, -1},
	/*enDataSeq, just support yuv*/
	VI_INPUT_DATA_UVUV,

	/*sync info*/
	{
		/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,

			/*timing info*/
			/*hsync_hfb    hsync_act    hsync_hhb*/
		{0,            1920,        0,
		/*vsync0_vhb vsync0_act vsync0_hhb*/
		0,            1080,        0,
		/*vsync1_vhb vsync1_act vsync1_hhb*/
		0,            0,            0}
	},
		/*whether use isp*/
		VI_PATH_BYPASS,
		/*data type*/
		VI_DATA_TYPE_YUV
};

VI_DEV g_as32ViDev[VIU_MAX_DEV_NUM];
VI_CHN g_as32MaxChn[VIU_MAX_CHN_NUM];
VI_CHN g_as32SubChn[VIU_MAX_CHN_NUM];

HI_S32 SAMPLE_COMM_VI_Mode2Param(SAMPLE_VI_MODE_E enViMode, SAMPLE_VI_PARAM_S *pstViParam)
{
    switch (enViMode)
    {
        case SAMPLE_VI_MODE_32_D1:
        case SAMPLE_VI_MODE_32_960H:
        case SAMPLE_VI_MODE_32_1280H:
        case SAMPLE_VI_MODE_32_HALF720P:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 16;
            pstViParam->s32ViChnInterval = 1;
            break;
        case SAMPLE_VI_MODE_16_720P:
        case SAMPLE_VI_MODE_16_1080P:
            /* use chn 0,2,4,6,8,10,12,14,16,18,20,22,24,28 */
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 2;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 8;
            break;
        case SAMPLE_VI_MODE_8_720P:
        case SAMPLE_VI_MODE_8_1080P:        
            /* use chn 0,4,8,12,16,20,24,28 */
            pstViParam->s32ViDevCnt = 8;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 8;
            pstViParam->s32ViChnInterval = 4;
            break;
        case SAMPLE_VI_MODE_YD_1080P:
			pstViParam->s32ViDevCnt = 6;
			pstViParam->s32ViDevInterval = 1;
			pstViParam->s32ViChnCnt = 6;
			pstViParam->s32ViChnInterval = 4;
			break;
		case SAMPLE_VI_MODE_GS_4_1080P:
			/* use chn 0,8,16,24 */
			pstViParam->s32ViDevCnt = 4;
			pstViParam->s32ViDevInterval = 2;
			pstViParam->s32ViChnCnt = 4;
			pstViParam->s32ViChnInterval = 8;
			break;
        default:
            SAMPLE_PRT("ViMode invaild!\n");
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 VI_Mode2Size(VI_MODE_E enViMode, VIDEO_NORM_E enNorm, RECT_S *pstCapRect, SIZE_S *pstDestSize)
{
	pstCapRect->s32X = 0;
	pstCapRect->s32Y = 0;
	switch (enViMode)
	{
	case VI_MODE_D1_1MUX_SINGLE:
		pstDestSize->u32Width  = D1_WIDTH;
		pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 576 : 480;
		pstCapRect->u32Width  = D1_WIDTH;
		pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 576 : 480;
		break;
	case VI_MODE_720P_BT1120_1MUX_SINGLE:
		pstDestSize->u32Width  = _720P_WIDTH;
		pstDestSize->u32Height = _720P_HEIGHT;
		pstCapRect->u32Width  = _720P_WIDTH;
		pstCapRect->u32Height = _720P_HEIGHT;
		break;
	case VI_MODE_1080I_BT1120_1MUX_SINGLE:
	case VI_MODE_1080P_BT1120_1MUX_SINGLE:
		pstDestSize->u32Width  = HD_WIDTH;
		pstDestSize->u32Height = HD_HEIGHT;
		pstCapRect->u32Width  = HD_WIDTH;
		pstCapRect->u32Height = HD_HEIGHT;
		break;           
	default:
		SAMPLE_PRT("vi mode invaild!\n");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 VI_StartChn(VI_CHN ViChn, VI_MODE_E enViMode, VIDEO_NORM_E enNorm, SAMPLE_VI_CHN_SET_E enViChnSet)
{
	HI_S32 s32Ret;
	VI_CHN_ATTR_S stChnAttr;

	/* step  5: config & start vicap dev */
	s32Ret = VI_Mode2Size(enViMode, enNorm, &stChnAttr.stCapRect, &stChnAttr.stDestSize);
	if (HI_SUCCESS !=s32Ret)
	{
		SAMPLE_PRT("vi get size failed!\n");
		return HI_FAILURE;
	}

	stChnAttr.enCapSel = VI_CAPSEL_BOTH;
	stChnAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;   /* sp420 or sp422 */
	stChnAttr.bMirror = (VI_CHN_SET_MIRROR == enViChnSet) ? HI_TRUE : HI_FALSE;
	stChnAttr.bFlip = (VI_CHN_SET_FILP == enViChnSet) ? HI_TRUE : HI_FALSE;
	stChnAttr.s32SrcFrameRate = -1;
	stChnAttr.s32DstFrameRate = -1;
	switch (enViMode)
	{
	case VI_MODE_D1_1MUX_SINGLE:
	case VI_MODE_1080I_BT1120_1MUX_SINGLE:
		stChnAttr.enScanMode = VI_SCAN_INTERLACED;
		break;
	case VI_MODE_720P_BT1120_1MUX_SINGLE:
	case VI_MODE_1080P_BT1120_1MUX_SINGLE:
		stChnAttr.enScanMode = VI_SCAN_PROGRESSIVE;
		break;
	default:
		SAMPLE_PRT("ViMode invaild!\n");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VI_EnableChn(ViChn);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_VOID VI_SetMask(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	switch (ViDev % 2)
	{
	case 0: 
		pstDevAttr->au32CompMask[0] = 0xFF0000;
		if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
		{
			pstDevAttr->au32CompMask[0] = 0xFF000000;
			pstDevAttr->au32CompMask[1] = 0xFF000000>>8;
		}
		else 
		{
			pstDevAttr->au32CompMask[1] = 0x0;
		}
		break;

	case 1:
		pstDevAttr->au32CompMask[0] = 0xFF000000;
		pstDevAttr->au32CompMask[1] = 0x0;              
		break;
	default:
		HI_ASSERT(0);
	}
}

HI_S32 VI_StartDev(VI_DEV ViDev, VI_MODE_E enViMode)
{
    HI_S32 s32Ret;
    VI_DEV_ATTR_S stViDevAttr;

    memset(&stViDevAttr, 0, sizeof(stViDevAttr));

    switch (enViMode)
    {
		case VI_MODE_D1_1MUX_SINGLE:
			memcpy(&stViDevAttr, &DEV_ATTR_BT656_1MUX_SINGLE, sizeof(stViDevAttr));
			VI_SetMask(ViDev, &stViDevAttr); 
			break; 
        case VI_MODE_1080I_BT1120_1MUX_SINGLE:
		case VI_MODE_720P_BT1120_1MUX_SINGLE:
		case VI_MODE_1080P_BT1120_1MUX_SINGLE:
			memcpy(&stViDevAttr, &DEV_ATTR_BT1120_1MUX_SINGLE, sizeof(stViDevAttr));
			VI_SetMask(ViDev, &stViDevAttr);
			break;    
        default:
            SAMPLE_PRT("vi input type[%d] is invalid!\n", enViMode);
            return HI_FAILURE;
    }

	
	s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	if (VI_MODE_D1_1MUX_SINGLE != enViMode) { /* 高清模式，使用双分量 */
		PFC_System("himm %#x 0xFF000008 > /dev/null", 0x130C0000 + 0x10310 + 0x4000 * ViDev); /* 0 分量使用 offset=8 */
		PFC_System("himm %#x 0xFF000000 > /dev/null", 0x130C0000 + 0x10314 + 0x4000 * ViDev); /* 1 分量使用 offset=0 */
	} else {
		PFC_System("himm %#x 0xFF000000 > /dev/null", 0x130C0000 + 0x10310 + 0x4000 * ViDev); /* 0 分量 */
	}

#if 0 /* 调试扩展参数时使用 */
	VI_DEV_ATTR_EX_S lViDevExAttr;
	memset(&lViDevExAttr, 0, sizeof(lViDevExAttr));

	lViDevExAttr.enInputMode = VI_INPUT_MODE_BT656;
	lViDevExAttr.enWorkMode = VI_WORK_MODE_1Multiplex;         /*1-, 2-, or 4-channel multiplexed work mode */

	lViDevExAttr.enCombineMode = VI_COMBINE_SEPARATE;      /* Y/C composite or separation mode */
	lViDevExAttr.enCompMode = VI_COMP_MODE_DOUBLE;         /* Component mode (single-component or dual-component) */
	lViDevExAttr.enClkEdge = VI_CLK_EDGE_SINGLE_UP;          /* Clock edge mode (sampling on the rising or falling edge) */
	lViDevExAttr.au32CompMask[0] = 0xFF000000;    /* Component mask */
	lViDevExAttr.au32CompMask[1] = 0xFF0000;

	lViDevExAttr.s32AdChnId[0] = -1;      /* AD channel ID. Typically, the default value -1 is recommended */
	lViDevExAttr.s32AdChnId[1] = -1;
	lViDevExAttr.s32AdChnId[2] = -1;
	lViDevExAttr.s32AdChnId[3] = -1;

	lViDevExAttr.enDataSeq = VI_INPUT_DATA_UVUV;          /* Input data sequence (only the YUV format is supported) */

	lViDevExAttr.stBT656SynCfg.enFixCode = BT656_FIXCODE_1; 
	lViDevExAttr.stBT656SynCfg.enFieldPolar = BT656_FIELD_POLAR_NSTD;      /* Sync timing. This member must be configured in BT.656 mode */

	lViDevExAttr.enDataPath = VI_PATH_ISP;         /* ISP enable or bypass */
	lViDevExAttr.enInputDataType = VI_DATA_TYPE_YUV;    /* RGB: CSC-709 or CSC-601, PT YUV444 disable; YUV: default yuv CSC coef PT YUV444 enable. */

	lViDevExAttr.bDataRev = HI_FALSE;           /* Data reverse */
	s32Ret = HI_MPI_VI_SetDevAttrEx(ViDev, &lViDevExAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VI_SetDevAttrEx failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
#endif

    s32Ret = HI_MPI_VI_EnableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/* VI 模块带通道的设置，根据设备号和通道号设置对应 */
HI_S32 VI_Start(VI_PARAM_S *pViParam)
{
	HI_S32 s32Ret;
	VI_DEV lViDev;
	VI_CHN lViChn;
	HI_S32 i;

	/* 开始 VI 设备 */
	for (i = 0; i < pViParam->m_ViDevCnt; i++) 
	{
		lViDev = i * pViParam->m_ViDevInterval;

		s32Ret = VI_StartDev(lViDev, pViParam->m_ViDevParam[lViDev].m_ViMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("VI_StartDev failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}
	}

	for(i = 0; i < pViParam->m_ViChnCnt; i++)
	{
		lViDev = (i * pViParam->m_ViChnInterval) / VIU_MAX_CHN_NUM_PER_DEV;
		lViChn = (i * pViParam->m_ViChnInterval) % VIU_MAX_CHN_NUM_PER_DEV;

		s32Ret = VI_StartChn(lViChn, pViParam->m_ViDevParam[lViDev].m_ViMode, pViParam->m_ViDevParam[lViDev].m_ViChnParam[lViChn].m_ViNorm, VI_CHN_SET_NORMAL);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("call VI_StarChn failed with %#x\n", s32Ret);
			return HI_FAILURE;
		} 
	}

	return HI_SUCCESS;
}

HI_S32 VI_StopChn(VI_CHN ViChn)
{
	HI_S32 s32Ret;

	/* Stop vi phy-chn */
	s32Ret = HI_MPI_VI_DisableChn(ViChn);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("VI_StopChn failed with %#x\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 VI_StopDev(VI_DEV ViDev)
{
	HI_S32 s32Ret;

	/* Stop vi phy-chn */
	s32Ret = HI_MPI_VI_DisableDev(ViDev);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("VI_StopDev failed with %#x\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 VI_Stop(HI_S32 ViDevCnt, HI_S32 ViDevInterval, HI_S32 ViChnCnt, HI_S32 ViChnInterval)
{
	HI_S32 i;
	HI_S32 s32Ret;

	for(i = 0; i < ViChnCnt; i++)
	{
		/* Stop vi phy-chn */
		s32Ret = HI_MPI_VI_DisableChn(i * ViChnInterval);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("VI_StopChn failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}
	}

	/*** Stop VI Dev ***/
	for(i = 0; i < ViDevCnt; i++)
	{
		s32Ret = HI_MPI_VI_DisableDev(i * ViDevInterval);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("VI_StopDev failed with %#x\n", s32Ret);
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;
}

/*****************************************************************************
* function : Vi chn bind vpss group
*****************************************************************************/
HI_S32 VI_BindVpss(HI_S32 ViChn, HI_S32 VpssGrp)
{
    HI_S32 j, s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
   
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 VI_UnBindVpss(VI_CHN ViChn, VPSS_GRP VpssGrp)
{
	HI_S32 j, s32Ret;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	stSrcChn.enModId = HI_ID_VIU;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = ViChn;

	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;
	stDestChn.s32ChnId = 0;

	s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/*****************************************************************************
* function : Vi chn unbind vpss group
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_UnBindVpss(SAMPLE_VI_MODE_E enViMode)
{
    HI_S32 i, j, s32Ret;
    VPSS_GRP VpssGrp;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    SAMPLE_VI_PARAM_S stViParam;
    VI_DEV ViDev;
    VI_CHN ViChn;

    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    VpssGrp = 0;    
    for (i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;

        for (j=0; j<stViParam.s32ViChnCnt; j++)
        {
            ViChn = j * stViParam.s32ViChnInterval;
            
            stSrcChn.enModId = HI_ID_VIU;
            stSrcChn.s32DevId = ViDev;
            stSrcChn.s32ChnId = ViChn;
        
            stDestChn.enModId = HI_ID_VPSS;
            stDestChn.s32DevId = VpssGrp;
            stDestChn.s32ChnId = 0;
        
            s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
            
            VpssGrp ++;
        }
    }
    return HI_SUCCESS;
}
 
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
