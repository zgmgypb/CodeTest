#ifdef GN2000
#include "tvp5146.h"
#include "platform_conf.h"

//===================================
#undef GLOBAL_TRACE
#define GLOBAL_TRACE(x) {}
//===================================

#define VIDEO_STANDARD_REG                      0x02
#define TVP5146_REG_STATUS_1                    0x3A
#define AUTOSWITCH_MASK_REG                     0x04
#define LUMINANCE_PROCESSING_CONTROL_3_REG      0x08
#define LUMINANCE_BRIGHTNESS_REG                0x09
#define LUMINANCE_CONTRAST_REG                  0x0a
#define CHROMINANCE_SATURATION_REG              0x0b
#define CHROMINANCE_HUE_REG                     0x0c
#define CHROMINANCE_PROCESSING_CONTROL_2_REG    0x0E
#define OUTPUT_FORMATTER_1_REG                  0x33
#define OUTPUT_FORMATTER_2_REG                  0x34
#define OUTPUT_FORMATTER_3_REG                  0x35
#define OUTPUT_FORMATTER_4_REG                  0x36
#define OUTPUT_FORMATTER_5_REG                  0x37
#define OUTPUT_FORMATTER_6_REG                  0x38

#define VIDOE_FORMATE_PAL		0
#define VIDOE_FORMATE_NTSC	1

#define BRIGHTHNESS_MIN		0
#define BRIGHTHNESS_MAX		100

#define CONTRAST_MIN		0
#define CONTRAST_MAX		100

#define SATURATION_MIN		0
#define SATURATION_MAX		100

#define HUE_MIN				0
#define HUE_MAX				100

static TVP5146_SetRegisterFn SetRegister = NULL;
static TVP5146_GetRegisterFn GetRegister = NULL;

static BOOL8 TVP5146_SetInputVideoStandard(InputVideoStandardType InputVideoStandard)
{
	U8 lData = InputVideoStandard;
	return SetRegister(VIDEO_STANDARD_REG, lData);
}

/*
函数名：TVP5146_SetBrightness
功能：设置亮度
返回值：TRUE FALSE
*/
static BOOL8 TVP5146_SetBrightness(U8 brightness)
{
	U8 lData = 0;
	lData = 28 + brightness*2;
	if(SetRegister(LUMINANCE_BRIGHTNESS_REG, lData) == FALSE)
	{
		return FALSE;
	}
	// for ypbpr
	if(SetRegister(0x14, lData) == FALSE)
	{
		return FALSE;
	}
	//usleep(5000);

	return TRUE;
}

/*
函数名：TVP5146_SetContrast
功能：设置对比度
返回值：TRUE FALSE
*/
static BOOL8 TVP5146_SetContrast(U8 contrast)
{
	U8 lData = 0;
	lData = 78 + contrast;
	if(SetRegister(LUMINANCE_CONTRAST_REG, lData) == FALSE)
	{
		return FALSE;
	}
	//for Ypbpr
	if(SetRegister(0x11, lData) == FALSE)
	{
		return FALSE;
	}
	//usleep(5000);

	return TRUE;
}

/*
函数名：TVP5146_SetSaturation
功能：设置饱和度
返回值：TRUE FALSE
*/
static BOOL8 TVP5146_SetSaturation(S32 saturation)
{
	U8 lData = 0;
	lData = 20+2*saturation;
	if (SetRegister(CHROMINANCE_SATURATION_REG, lData) == FALSE)
	{
		return FALSE;
	}

	// for YPbPr
	if (SetRegister(0x10, lData) == FALSE)
	{
		return FALSE;
	}

	if (SetRegister(0x12, lData) == FALSE)
	{
		return FALSE;
	}

	return TRUE;


}

/*
函数名：TVP5146_SetHue
功能：设置色调
返回值：TRUE FALSE
*/
static BOOL8 TVP5146_SetHue(U8 hue)
{
	U8 lData = 0;
	if (hue >= 50)
	{
		lData = hue -50;
	}
	else
	{
		lData = 206 + hue;
	}
	if(SetRegister(CHROMINANCE_HUE_REG, lData) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

/*
函数名：TVP5146_SetHlocation
功能：设置图像水平位移
返回值：TRUE FALSE
*/
static BOOL8 TVP5146_SetHlocation(InputVideoStandardType InputVideoStandard)
{
	S32 lInputVideoFormatFalg;
	U8 lData_0 = 0;
	U8 lData_1 = 0;
	U8 lData_2 = 0;
	U8 lData_3 = 0;
	switch (InputVideoStandard)
	{
		case	VIDEO_STANDARD_NTSC_M_J:
		case VIDEO_STANDARD_NTSC_4_43:
		case VIDEO_STANDARD_PAL_M:
		lInputVideoFormatFalg = VIDOE_FORMATE_NTSC;
		break;

		default:
		lInputVideoFormatFalg = VIDOE_FORMATE_PAL;
		break;
	}

	if (lInputVideoFormatFalg == VIDOE_FORMATE_NTSC)
	{
		lData_0 = 0x58;
		lData_1 = 0x00;
		lData_2 = 0x28;
		lData_3 = 0x03;
	}
	else
	{
		lData_0 = 0x55;
		lData_1 = 0x00;
		lData_2 = 0x25;
		lData_3 = 0x03;
	}

	if (SetRegister(0x16, lData_0) == FALSE)
	{
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(0x17, lData_1) == FALSE)
	{
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(0x18, lData_2) == FALSE)
	{
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(0x19, lData_3) == FALSE)
	{
		return FALSE;
	}
	//usleep(5000);

	return TRUE;
}

/*
函数名：TVP5146_SetContrast
功能：检测输入信号是否锁定
返回值：TRUE 锁定 FALSE 未锁定
*/
BOOL8 TVP5146_CheckLockStatus(void)
{
	U8 lLockStatus = 0;
	GetRegister(TVP5146_REG_STATUS_1, &lLockStatus);
	if ((lLockStatus &0x0e) == 0x0e)
	{
        GLOBAL_TRACE(("tvp5146 locked!\n"));
		return TRUE;
	}
	else
	{
        GLOBAL_TRACE(("tvp5146 unlocked!\n"));
		return FALSE;
	}
}

static BOOL8 CheckTvp5146Parameter(Tvp5146Parameter *pTvp5146Parameter)
{
	S32 lCheckFlag = TRUE;
	if (!pTvp5146Parameter)
	{
		return FALSE;
	}

	//PDEBUG("InputVideoStandard:%d\n",pTvp5146Parameter->InputVideoStandard);
	if ((pTvp5146Parameter->InputVideoStandard <VIDEO_STANDARD_AUTO_SWITCH) || (pTvp5146Parameter->InputVideoStandard > VIDEO_STANDARD_PAL_60))
	{
		printf("InputVideoStandard [%d - %d]\n", VIDEO_STANDARD_AUTO_SWITCH, VIDEO_STANDARD_PAL_60);
		lCheckFlag = FALSE;
	}

	//PDEBUG("brightness:%d\n",pTvp5146Parameter->brightness);
	if ((pTvp5146Parameter->brightness < BRIGHTHNESS_MIN) || (pTvp5146Parameter->brightness > BRIGHTHNESS_MAX))
	{
		printf("brightness [%d - %d]\n", BRIGHTHNESS_MIN, BRIGHTHNESS_MAX);
		lCheckFlag = FALSE;
	}

	//PDEBUG("contrast:%d\n",pTvp5146Parameter->contrast);
	if ((pTvp5146Parameter->contrast <CONTRAST_MIN) || (pTvp5146Parameter->contrast > CONTRAST_MAX))
	{
		printf("contrast [%d - %d]\n", CONTRAST_MIN, CONTRAST_MAX);
		lCheckFlag = FALSE;
	}

	//PDEBUG("saturation:%d\n",pTvp5146Parameter->saturation);
	if ((pTvp5146Parameter->saturation <SATURATION_MIN) || (pTvp5146Parameter->saturation > SATURATION_MAX))
	{
		printf("saturation [%d - %d]\n", SATURATION_MIN, SATURATION_MAX);
		lCheckFlag = FALSE;
	}

	//PDEBUG("hue:%d\n",pTvp5146Parameter->hue);
	if ((pTvp5146Parameter->hue <HUE_MIN) || (pTvp5146Parameter->hue > HUE_MAX))
	{
		printf("hue [%d - %d]\n", HUE_MIN, HUE_MAX);
		lCheckFlag = FALSE;
	}

	return lCheckFlag;
}

BOOL8 ConfigTvp5146AllParameterToDefault(Tvp5146Parameter *pTvp5146Parameter)
{
	if (!pTvp5146Parameter)
	{
		return FALSE;
	}
	pTvp5146Parameter->InputVideoStandard = VIDEO_STANDARD_PAL_B_D_G_H_I_N;
	pTvp5146Parameter->brightness = 50;
	pTvp5146Parameter->saturation = 50;
	pTvp5146Parameter->contrast = 50;
	pTvp5146Parameter->hue = 50;

	return TRUE;
}

BOOL8 ConfigTvp5146AllParameter(Tvp5146Parameter *pTvp5146Parameter)
{
	GLOBAL_TRACE(("Start!\n"));
    
    GLOBAL_TRACE(("Check TVP5146 Param!\n"));
	if (CheckTvp5146Parameter(pTvp5146Parameter) == FALSE)
	{
		GLOBAL_TRACE(("Error!\n"));
		return FALSE;
	}

    GLOBAL_TRACE(("Set In Port\n"));
	if (SetRegister(0x00, 0x00) == FALSE)
	{
		GLOBAL_TRACE(("Error!\n"));
		return FALSE;
	}
    
    
	//usleep(5000);
    
    GLOBAL_TRACE(("Set Input Video Standard!\n"));
	if (TVP5146_SetInputVideoStandard(pTvp5146Parameter->InputVideoStandard) == FALSE)
	{
		GLOBAL_TRACE(("Error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(AUTOSWITCH_MASK_REG, 0x3f) == FALSE)
	{
		GLOBAL_TRACE(("config autoswitch_mask_reg error!\n"));
		return FALSE;
	}
	//usleep(5000);
    GLOBAL_TRACE(("Set luminance_processing_control_3_reg!\n"));
	if (SetRegister(LUMINANCE_PROCESSING_CONTROL_3_REG, 0x00) == FALSE)
	{
		GLOBAL_TRACE(("config luminance_processing_control_3_reg error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(CHROMINANCE_PROCESSING_CONTROL_2_REG, 0x04) == FALSE)
	{
		GLOBAL_TRACE(("config chrominance_processing_control_2_reg error!\n"));
		return FALSE;
	}	
	//usleep(5000);

	if (TVP5146_SetBrightness(pTvp5146Parameter->brightness) == FALSE)
	{
		GLOBAL_TRACE(("TVP5146_SetBrightness error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (TVP5146_SetContrast(pTvp5146Parameter->contrast) == FALSE)
	{
		GLOBAL_TRACE(("TVP5146_SetContrast error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (TVP5146_SetSaturation(pTvp5146Parameter->saturation) == FALSE)
	{
		GLOBAL_TRACE(("TVP5146_SetSaturation error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if(TVP5146_SetHue(pTvp5146Parameter->hue) == FALSE)
	{
		GLOBAL_TRACE(("TVP5146_SetHue error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(0X32, 0x10) == FALSE)
	{
		GLOBAL_TRACE(("config output_formatter_1_reg error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(OUTPUT_FORMATTER_1_REG, 0x40) == FALSE)
	{
		GLOBAL_TRACE(("config output_formatter_1_reg error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(OUTPUT_FORMATTER_2_REG, 0x11) == FALSE)
	{
		GLOBAL_TRACE(("config output_formatter_2_reg error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(OUTPUT_FORMATTER_3_REG, 0xfe) == FALSE)
	{
		GLOBAL_TRACE(("config output_formatter_3_reg error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(OUTPUT_FORMATTER_4_REG, 0xef) == FALSE)
	{
		GLOBAL_TRACE(("config output_formatter_4_reg error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(OUTPUT_FORMATTER_5_REG, 0xff) == FALSE)
	{
		GLOBAL_TRACE(("config output_formatter_5_reg error !\n"));
		return FALSE;
	}
	//usleep(5000);

	if (SetRegister(OUTPUT_FORMATTER_6_REG, 0xff) == FALSE)
	{
		GLOBAL_TRACE(("config output_formatter_6_reg error!\n"));
		return FALSE;
	}
	//usleep(5000);

	if (TVP5146_SetHlocation(pTvp5146Parameter->InputVideoStandard) == FALSE)
	{
		GLOBAL_TRACE(("TVP5146_SetHlocation Error!\n"));
		return FALSE;
	}
	//usleep(5000);

	GLOBAL_TRACE(("End!\n"));

	return TRUE;
}

BOOL8 InitTvp5146(TVP5146_SetRegisterFn pSetRigister, TVP5146_GetRegisterFn pGetRigister)
{
	if ((!pSetRigister) || (!pGetRigister))
	{
		return FALSE;
	}
	else
	{
		SetRegister = pSetRigister;
		GetRegister = pGetRigister;
		return TRUE;
	}
}
#endif
/*EOF*/
