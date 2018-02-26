#ifndef __TVP5146_H__
#define __TVP5146_H__

#include "global_def.h"

typedef enum
{
	VIDEO_STANDARD_AUTO_SWITCH = 0,
	VIDEO_STANDARD_NTSC_M_J,
	VIDEO_STANDARD_PAL_B_D_G_H_I_N,
	VIDEO_STANDARD_PAL_M,
	VIDEO_STANDARD_PAL_COMBINATION_N,
	VIDEO_STANDARD_NTSC_4_43,
	VIDEO_STANDARD_SECAM,
	VIDEO_STANDARD_PAL_60,

	VIDEO_STANDARD_MAX,

}InputVideoStandardType;

typedef struct
{
	InputVideoStandardType InputVideoStandard;
	S8 brightness;	//亮度 0-100
	S8 contrast;	//对比度 0-100
	S8 saturation;	//饱和度 0-100
	S8 hue;			//色调 0-100
}Tvp5146Parameter;


typedef BOOL8 ( *TVP5146_SetRegisterFn )(U8 RegiserId, U8 RegisterValue);
typedef BOOL8 ( *TVP5146_GetRegisterFn )(U8 RegiserId, U8 *pBuf );

BOOL8 InitTvp5146( TVP5146_SetRegisterFn pSetRigister, TVP5146_GetRegisterFn pGetRigister );
BOOL8 ConfigTvp5146AllParameterToDefault( Tvp5146Parameter *pTvp5146Parameter );
BOOL8 ConfigTvp5146AllParameter(Tvp5146Parameter *pTvp5146Parameter);
BOOL8 TVP5146_CheckLockStatus( void );

#endif

