#ifdef GN2000

#include "mb86391.h"
//extern S32 FlushUartWriteAndReceiveBuffer(void); //清空串口接收缓存

//===================================
/*
#undef GLOBAL_TRACE
#define GLOBAL_TRACE(x)
#
*/
//===================================

#define			COMMAND_RESET						   (0x10)
#define			COMMAND_RESET_SUBCODE			       (0x06)
#define			COMMAND_START_ENCODE			       (0x20)

#define         PARA_ID_IN_SIGNAL_MODE				   (0x105) // 0 : NTSC   1 : PAL
#define	        PARA_ID_OUT_PICTURE_NORMAL		       (0x107)
#define         PARA_ID_RESOLUTION_FILTER			   (0x22D) //0: D1 1:HD1 2:SIF 3:QSIF 4:Slice Screen out
#define         PARA_ID_H_SLICESIZE					   (0x227) //10H 4EH
#define         PARA_ID_V_SLICESIZE					   (0x228)
#define         PARA_ID_GOP_STRUCTURE				   (0x343) //0:III 1:IPPP 2:IBIPBPBPB 3:IBBPBBPBB
#define         PARA_ID_AUDIO_BITRATE				   (0x71c)  //(kb/s)
#define         PARA_ID_AUDIO_SAMPFREQ				   (0x71B) //44100 : 44.1k  48000:48K  32000:32k
#define         PARA_ID_AUDIO_ES_MODE					 0x725  //0:stereo 1:joint stereo 2:dual Channel 3:single Channel
#define         PARA_ID_STREAM_OUT_MODE				     0x61B  // 0: external clock sync mode
#define         PARA_ID_SYSTEM_MULTIPLEXING_METHOD       0x61e  // 2: MPEG-1 system 3:mpeg-2 TS 4:MPEG-2 PS
#define         PARA_ID_SYSTEM_BITRATE					 0x61f  //specify in a multiple of 16K bits
#define         PARA_IDGOP_SIZE							 0x342  //0x01 - 0xff:specified picture frame count
#define         PARA_ID_PMT_PID							 0x674
#define         PARA_ID_VIDEO_PID						 0x678
#define         PARA_ID_AUDIO_PID						 0x67D
#define         PARA_ID_PCR_PID							 0x694

#define		    PARA_ID_TOP_FIELD_VALID_LINE_COUNT		 0x112
#define		    PARA_ID_BOTTOM_FIELD_VALID_LINE_COUNT	 0x113
#define		    PARA_ID_HSYNC_VALID_PIXEL				 0x116
#define		    PARA_ID_PIXEL_PER_LINE					 0x117
#define		    PARA_ID_VIDEO_INPUT_MODE				 0x104
#define		    PARA_ID_BITRATE_MODE					 0x360

#define		    PARA_ID_AES_707			                 0x707
#define		    PARA_ID_AES_703			                 0x703
#define		    PARA_ID_AES_704			                 0x704
#define		    PARA_ID_AES_709			                 0x709

//音频PID 视频PID PCR PID 和PMT PID的限制按照datasheet进行，限制到【2-8190】,更进一步的限制留个上层去决定。列如1824P限制的范围是【32-8190】
#define         AUDIO_PID_MIN	                          2
#define         AUDIO_PID_MAX	                          8190
#define         VIDEO_PID_MIN	                          2
#define         VIDEO_PID_MAX	                          8190
#define         PCR_PID_MIN		                          2
#define         PCR_PID_MAX		                          8190
#define         PMT_PID_MIN		                          2
#define         PMT_PID_MAX		                          8190

//图像水平位移范围根据1824P而来
#define         H_LOCATION_LEFT                     	   0
#define         H_LOCATION_RIGHT	                       16

#define         GOP_SIZE_MIN		                          1
#define         GOP_SIZE_MAX		                      0XFF

typedef enum
{
	HEAD_ALIGNMENT = 0,
	TAIL_ALIGNMENT
}AdataAndAlrckAlignmetType;

typedef enum
{
	ALRCK_AND_ACLK_PHASE_0 = 0,
	ALRCK_AND_ACLK_PHASE_1,
	ALRCK_AND_ACLK_PHASE_2,
	ALRCK_AND_ACLK_PHASE_3
}AlrckAndAclkPhaseType;

typedef enum
{
	ADATA_AND_ALRCK_PHASE_0 = 0,
	ADATA_AND_ALRCK_PHASE_1,
	ADATA_AND_ALRCK_PHASE_2
}AdataAndAlrckPhaseType;

typedef enum
{
	VIDEO_INPUT_MODE_D1 = 0,
	VIDEO_INPUT_MODE_Y_C_MULTIPLEXING
}VideoInputModeType;

typedef enum
{
	METHOD_EXTERNAL_CLOCK_SYNC = 0,
	METHOD_27MHZ_SYNC,
	METHOD_HANDSHAKE
}StreamOutMethodType;

typedef enum
{
	MPEG_1_SYSTEM = 2,
	MPEG_2_TS,
	MPEG_2_PS
}SystemMultiplexingMethodType;

typedef enum
{
	BITRATE_MODE_CBR = 0,
	BITRATE_MODE_VBR
}CbrVbrType;

static Mb86391_UartSendFn pMb86391_UartSendFn = NULL;
static Mb86391_UartReceiveFn pMb86391_UartReceiveFn = NULL;
static Mb86391_DelayMSCB pMb86391_Delay = NULL;
static Mb86391_DelayMSCB pMb86391_Reset = NULL;


//gWaitMb86391AnwserFlag 表示每次发送命令或者配置参数给mb86391后是否等待mb86391的应答，如果设置为TRUE表示要等待并且判断应答是否正确，否则不等待
static S32 gWaitMb86391AnwserFlag = TRUE;

S32 SendToUart(U8 *pBuf, U32 len)
{
	S32 lRetValue = 0;
	U8 lTmpData = 0;
	U8 lData=0;
	S32 i;	

	lTmpData = *pBuf;
	for (i=0; i<8; i++)
	{
		lData <<= 1;
		if ((lTmpData>>i) & 0x01)
		{
			lData |= 0x01;
		}
	}
	*pBuf = lData;

	lRetValue = (*pMb86391_UartSendFn)(pBuf, len);
	
	return lRetValue;
}

/*
函数名：
功能：
返回值：-1失败，0未读取到数据，>0读取到的字节数
*/
S32 ReadFromeUart(U8 *pBuf, U32 ReadLen)
{
	S32 lRetValue = 0;
	U8 lTmpData;
	U8 lData;
	S32 i;
	*pBuf = 0xff; //for debug
	lRetValue = (*pMb86391_UartReceiveFn)(pBuf, ReadLen);
	lTmpData = *pBuf;
	lData = 0;
	for (i=0; i<8; i++)
	{
		lData <<= 1;
		if ((lTmpData >>i) & 0x01)
		{
			lData |= 0x01;
		}
	}
	*pBuf = lData;
	//GLOBAL_TRACE(("uart readed data:0x%x\n", lData));

	return lRetValue;
}

static S32 SendOneCommand(U8 command , U8 parameter)
{
	U8 lReadChar ;
	U8 lSendChar;

	/*
	if (FlushUartWriteAndReceiveBuffer() == FALSE) //清空串口接收缓存
	{
		printf("FlushUartWriteAndReceiveBuffer error file:%s line:%d\n", __FILE__, __LINE__);
	}
	*/
	
	SendToUart(&command, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}
	SendToUart(&parameter, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x1e)
		{
			return FALSE;
		}
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x7a)
		{
			return FALSE;
		}
	}	
	
	lSendChar = 0x0e ;
	SendToUart(&lSendChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x00) 
		{
			return FALSE;
		}
	}	
	//GLOBAL_TRACE(("command success \n"));
	lSendChar = 0x1e ;
	SendToUart(&lSendChar, 1) ;
	return TRUE;
}

static S32 SendOneParameter(U32 ParameterID, U32 ParamterValue)
{
	U8 lChar ;
	U8 lReadChar;	
	lChar = 0x16 ;
	
	/*	
	if (FlushUartWriteAndReceiveBuffer() == FALSE)  //清空串口接收缓存
	{
		printf("FlushUartWriteAndReceiveBuffer error file:%s line:%d\n", __FILE__, __LINE__);
	}
	*/
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1);
		//GLOBAL_TRACE(("readed:0x%x line:%d\n", lReadChar, __LINE__));
		if (lReadChar != 0x0e) 
		{
			return FALSE;
		}
	}
	
	lChar = 0x02 ;          //have parameter 
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;	
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}
	lChar = 0x10 ;         //parameter length = 08
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}
	lChar = (ParameterID >> 5) & 0x7e ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}
	lChar = (ParameterID << 1) & 0x7e ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}

	lChar = (ParamterValue >> 25) & 0x7e ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e) 
		{
			return FALSE;
		}
	}
	lChar = (ParamterValue >> 19) & 0x7e ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}
	lChar = (ParamterValue >> 13) & 0x7e ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}
	lChar = (ParamterValue >> 7) & 0x7e ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}
	lChar = (ParamterValue >> 1) & 0x7e ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x0e)
		{
			return FALSE;
		}
	}
	lChar = (ParamterValue << 5) & 0x60 ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x1e)
		{
			return FALSE;
		}
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x7a)
		{
			return FALSE;
		}
	}
	lChar = 0x0e ;
	SendToUart(&lChar, 1) ;
	if (gWaitMb86391AnwserFlag)
	{
		ReadFromeUart(&lReadChar, 1) ;
		if (lReadChar != 0x00)
		{
			return FALSE;
		}
	}

	//GLOBAL_TRACE(("parameter success \n"));
	lChar = 0x1e ;
	SendToUart(&lChar, 1) ;
	return TRUE;
}

static S32 Reset86391(void)
{
	return SendOneCommand(COMMAND_RESET , COMMAND_RESET_SUBCODE);
}

static S32 Start86390(void)
{
   if (SendOneParameter(PARA_ID_OUT_PICTURE_NORMAL, 0X00) == FALSE)
   {
	   GLOBAL_TRACE(("SendOneParameter error, file:%s line:%d\n", __FILE__, __LINE__));
	   return FALSE;
   }

	if (SendOneCommand(COMMAND_START_ENCODE , 0x00) == FALSE)
	{
		GLOBAL_TRACE(("SendOneCommand error, file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
	return TRUE;
}

/*
函数名：SelectStreamOutputMethod
功能：
参数：
返回值：TRUE FALSE
*/
static S32 SelectStreamOutputMethod(StreamOutMethodType mode)
{
	return SendOneParameter(PARA_ID_STREAM_OUT_MODE, mode);
}

/*
函数名：SelectSystemMultiplexingMethod
功能：
参数：
返回值：TRUE FALSE
*/
static S32 SelectSystemMultiplexingMethod(SystemMultiplexingMethodType method)
{
	return SendOneParameter(PARA_ID_SYSTEM_MULTIPLEXING_METHOD, method);
}

/*
函数名：SelectGopStructure
功能：
参数：
返回值：TRUE FALSE
*/
static S32 SelectGopStructure(S32 GopStructure)
{
	return SendOneParameter(PARA_ID_GOP_STRUCTURE, GopStructure);
}

/*
函数名：ChangeGopSize
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangeGopSize(S32 GopSize)
{
	return SendOneParameter(PARA_IDGOP_SIZE, GopSize);
}

/*
函数名：
功能：
参数：resolution 
返回值：TRUE FALSE
*/
static S32 ChangeResolution(VideoResolutionType resolution, VideoInputFormatType VideoInputFormat)
{
	S32 lResolutionH = 0;
	S32 lResolutionV = 0;
	S32 lResolutionFilter = 0;

	if (VideoInputFormat == VIDEO_FORMAT_PAL)
	{
		switch (resolution)
		{
			case RESOLUTION_PAL_720_576_NTSC_720_480:
				lResolutionH = 720;
				lResolutionV = 576;
				lResolutionFilter = 0;
				break;
			case RESOLUTION_PAL_544_576_NTSC_544_480:
				lResolutionH = 544;
				lResolutionV = 576;
				lResolutionFilter = 6;
				break;
			case RESOLUTION_PAL_480_576_NTSC_480_480:
				lResolutionH = 480;
				lResolutionV = 576;
				lResolutionFilter = 5;
				break;
			case RESOLUTION_PAL_352_576_NTSC_352_480:
				lResolutionH = 352;
				lResolutionV = 576;
				lResolutionFilter = 1;
				break;
			case RESOLUTION_PAL_352_288_NTSC_352_240:
				lResolutionH = 352;
				lResolutionV = 288;
				lResolutionFilter = 2;
				break;
			default:
				GLOBAL_TRACE(("not supported resolution\n"));
				return FALSE;
		}
	}
	else
	{
		switch (resolution)
		{
			case RESOLUTION_PAL_720_576_NTSC_720_480:
				lResolutionH = 720;
				lResolutionV = 480;
				lResolutionFilter = 0;
				break;
			case RESOLUTION_PAL_544_576_NTSC_544_480:
				lResolutionH = 544;
				lResolutionV = 480;
				lResolutionFilter = 6;
				break;
			case RESOLUTION_PAL_480_576_NTSC_480_480:
				lResolutionH = 480;
				lResolutionV = 480;
				lResolutionFilter = 5;
				break;
			case RESOLUTION_PAL_352_576_NTSC_352_480:
				lResolutionH = 352;
				lResolutionV = 480;
				lResolutionFilter = 1;
				break;
			case RESOLUTION_PAL_352_288_NTSC_352_240:
				lResolutionH = 352;
				lResolutionV = 256; //参阅1824P程序
				lResolutionFilter = 2;
				break;
			default:
				GLOBAL_TRACE(("not supported resolution\n"));
				return FALSE;
		}
	}
	
	if (SendOneParameter(PARA_ID_H_SLICESIZE, lResolutionH) == FALSE)
	{
		GLOBAL_TRACE(("SendOneParameter error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
	if (SendOneParameter(PARA_ID_V_SLICESIZE, lResolutionV) == FALSE)
	{
		GLOBAL_TRACE(("SendOneParameter error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
	if (SendOneParameter(PARA_ID_RESOLUTION_FILTER, lResolutionFilter) == FALSE)
	{
		GLOBAL_TRACE(("SendOneParameter error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

	return TRUE;
}

/*
函数名：ChangeAudioBitRate
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangeAudioBitRate(S32 AudioBitRate)
{
	return SendOneParameter(PARA_ID_AUDIO_BITRATE, AudioBitRate);
}

/*
函数名：ChangeAudioSampleFreq
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangeAudioSampleFreq(S32 AudiosampleFreq)
{
	return SendOneParameter(PARA_ID_AUDIO_SAMPFREQ, AudiosampleFreq);
}

/*
函数名：ChangeAudioSampleFreq
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangeAudioEsMode(S32 AudioEsMode)
{
	return SendOneParameter(PARA_ID_AUDIO_ES_MODE, AudioEsMode);
}

/*
函数名：ChangeAudioPid
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangeAudioPid(S32 AudioPid)
{
	return SendOneParameter(PARA_ID_AUDIO_PID, AudioPid);
}

/*
函数名：ChangeVideoBitrae
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangeVideoBitrae(S32 VideoBitrate)
{
	return SendOneParameter(PARA_ID_SYSTEM_BITRATE, VideoBitrate);
}

/*
函数名：ChangeVideoPid
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangeVideoPid(S32 VideoPid)
{
	return SendOneParameter(PARA_ID_VIDEO_PID, VideoPid);
}

/*
函数名：ChangePcrPid
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangePcrPid(S32 PcrPid)
{
	return SendOneParameter(PARA_ID_PCR_PID, PcrPid);
}

/*
函数名：ChangePmtPid
功能：
参数：
返回值：TRUE FALSE
*/
static S32 ChangePmtPid(S32 PmtPid)
{
	return SendOneParameter(PARA_ID_PMT_PID, PmtPid);
}

/*
函数名：Select_ADATA_And_ALRCK_Alignment
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 Select_ADATA_And_ALRCK_Alignment(AdataAndAlrckAlignmetType alignment)
{
	return SendOneParameter(PARA_ID_AES_707, alignment);
}

/*
函数名：Set_ALRCK_And_ACLK_Phase
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 Set_ALRCK_And_ACLK_Phase(AlrckAndAclkPhaseType phase)
{
	return SendOneParameter(PARA_ID_AES_703, phase);
}

/*
函数名：Set_ADATA_And_ALRCK_Phase
功能：
参数：
返回值：TRUE FALSE
*/
static S32 Set_ADATA_And_ALRCK_Phase(AdataAndAlrckPhaseType phase)
{
	return SendOneParameter(PARA_ID_AES_704, phase);
}

/*
函数名：SetTopFieldValidLineCount
功能：
参数：
返回值：TRUE FALSE
*/
static S32 SetTopFieldValidLineCount(S32 count)
{
	return SendOneParameter(PARA_ID_TOP_FIELD_VALID_LINE_COUNT, count);
}

/*
函数名：SetBottomFieldValidLineCount
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 SetBottomFieldValidLineCount(S32 count)
{
	return SendOneParameter(PARA_ID_BOTTOM_FIELD_VALID_LINE_COUNT, count);
}

/*
函数名：SetHsyncValidPixel
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 SetHsyncValidPixel(S32 count)
{
	return SendOneParameter(PARA_ID_HSYNC_VALID_PIXEL, count);
}

/*
函数名：SetHlocation
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 SetHlocation(S32 Hlocation, VideoInputFormatType VideoInpputType)
{
	S32 lCount = 0;
	if (VideoInpputType == VIDEO_FORMAT_NTSC)
	{
		lCount = 0x8c + (Hlocation<<2);
	}
	else
	{
		lCount = 0x84 + (Hlocation<<2);
	}
	return SetHsyncValidPixel(lCount);
}

/*
函数名：SetPixelPerLine
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 SetPixelPerLine(S32 count)
{
	return SendOneParameter(PARA_ID_PIXEL_PER_LINE, count);
}

/*
函数名：SelectVideoInputFormat
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 SelectVideoInputFormat(S32 format)
{
	return SendOneParameter(PARA_ID_IN_SIGNAL_MODE, format);
}

/*
函数名：SelectVideoInputMode
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 SelectVideoInputMode(VideoInputModeType mode)
{
	return SendOneParameter(PARA_ID_VIDEO_INPUT_MODE, mode);
}


/*
函数名：SelectAudioIF
功能：参阅数据手册
参数：
返回值：TRUE FALSE
*/
static S32 SelectAudioIF(AudioIfType AudioIF)
{
	return SendOneParameter(PARA_ID_AES_709, AudioIF);
}

/*
函数名：SelectBitrateMode
功能：
参数：
返回值：TRUE FALSE
*/
static S32 SelectBitrateMode(CbrVbrType mode)
{
	return SendOneParameter(PARA_ID_BITRATE_MODE, mode);
}

/*
函数名：CheckMb86391Parameter
功能：
参数：
返回值：TRUE FALSE
*/
static S32 CheckMb86391Parameter(Mb86391EncodeParameter *pEncodeParameter)
{
	S32 lParameterValidFlag = TRUE;
	if (pEncodeParameter == NULL)
	{
		return FALSE;
	}
	if ((pEncodeParameter->m_VideoInputFormat<VIDEO_FORMAT_NTSC) || (pEncodeParameter->m_VideoInputFormat>VIDEO_FORMAT_PAL))
	{
		GLOBAL_TRACE(("VideoInputFormat not valid [%d|%d]\n", VIDEO_FORMAT_NTSC, VIDEO_FORMAT_PAL));
		lParameterValidFlag = FALSE;
	}

	if ((pEncodeParameter->m_VideoResolution<RESOLUTION_PAL_720_576_NTSC_720_480) || (pEncodeParameter->m_VideoResolution>RESOLUTION_PAL_352_288_NTSC_352_240))
	{
		GLOBAL_TRACE(("VideoResolution not valid [%d - %d]\n", RESOLUTION_PAL_720_576_NTSC_720_480, RESOLUTION_PAL_352_288_NTSC_352_240));
		lParameterValidFlag = FALSE;
	}
	if ((pEncodeParameter->m_GopStructure<GOP_STRUCTURE_IIIIIIIII) || (pEncodeParameter->m_GopStructure>GOP_STRUCTURE_IBBPBBPBB))
	{
		GLOBAL_TRACE(("GopStructure not valid [%d - %d]\n", GOP_STRUCTURE_IIIIIIIII, GOP_STRUCTURE_IBBPBBPBB));
		lParameterValidFlag = FALSE;
	}
	
	if ((pEncodeParameter->m_GopSize <GOP_SIZE_MIN)|| (pEncodeParameter->m_GopSize > GOP_SIZE_MAX))
	{
		GLOBAL_TRACE(("GopSize not valid [%d - %d]\n", GOP_SIZE_MIN, GOP_SIZE_MAX));
		lParameterValidFlag = FALSE;
	}
	switch(pEncodeParameter->m_AudioBitRate)
	{
		case AUDIO_BITRATE_32:
		case AUDIO_BITRATE_64:
		case AUDIO_BITRATE_128:
		case AUDIO_BITRATE_192:
		case AUDIO_BITRATE_256:
		case AUDIO_BITRATE_384:
				break;
		default:
			GLOBAL_TRACE(("AudioBitRate not valid [%d|%d|%d|%d|%d|%d]\n", AUDIO_BITRATE_32, AUDIO_BITRATE_64, AUDIO_BITRATE_128, AUDIO_BITRATE_192, AUDIO_BITRATE_256, AUDIO_BITRATE_384));
			lParameterValidFlag = FALSE;
				break;
	}
	switch(pEncodeParameter->m_AudioSampleFreq)
	{
		case AUDIO_SAMPLE_FREQ_32K:
		case AUDIO_SAMPLE_FREQ_44_1K:
		case AUDIO_SAMPLE_FREQ_48K:
			break;
		default:
		GLOBAL_TRACE(("AudioSampleFreq not valid [%d|%d|%d]\n", AUDIO_SAMPLE_FREQ_32K, AUDIO_SAMPLE_FREQ_44_1K, AUDIO_SAMPLE_FREQ_48K));
		lParameterValidFlag = FALSE;
		break;
	}
	if ((pEncodeParameter->m_AudioEsMode <AUDIO_ES_MODE_STEREO) || (pEncodeParameter->m_AudioEsMode > AUDIO_ES_MODE_SINGLE_CHANNLE))
	{
		GLOBAL_TRACE(("AudioEsMode not valid [%d - %d]\n", AUDIO_ES_MODE_STEREO, AUDIO_ES_MODE_SINGLE_CHANNLE));
		lParameterValidFlag = FALSE;
	}
	
	if ((pEncodeParameter->m_AudioPid <AUDIO_PID_MIN) || (pEncodeParameter->m_AudioPid >AUDIO_PID_MAX))
	{
		GLOBAL_TRACE(("AudioPid not valid [%d - %d]\n", AUDIO_PID_MIN, AUDIO_PID_MAX));
		lParameterValidFlag = FALSE;
	}
	if ((pEncodeParameter->m_VideoBitrate <VIDEO_BITRATE_MIN) || (pEncodeParameter->m_VideoBitrate >VIDEO_BITRATE_MAX))
	{
		GLOBAL_TRACE(("VideoBitrate not valid [%d - %d]\n", VIDEO_BITRATE_MIN, VIDEO_BITRATE_MAX));
		lParameterValidFlag = FALSE;
	}
	if ((pEncodeParameter->m_VideoPid <VIDEO_PID_MIN) || (pEncodeParameter->m_VideoPid >VIDEO_PID_MAX))
	{
		GLOBAL_TRACE(("VideoPid not valid [%d - %d]\n", VIDEO_PID_MIN, VIDEO_PID_MAX));
		lParameterValidFlag = FALSE;
	}
	if ((pEncodeParameter->m_PcrPid <PCR_PID_MIN) || (pEncodeParameter->m_PcrPid >PCR_PID_MAX))
	{
		GLOBAL_TRACE(("PcrPid not valid [%d - %d]\n", PCR_PID_MIN, PCR_PID_MAX));
		lParameterValidFlag = FALSE;
	}
	if ((pEncodeParameter->m_PmtPid <PMT_PID_MIN) || (pEncodeParameter->m_PmtPid >PMT_PID_MAX))
	{
		GLOBAL_TRACE(("PmtPid not valid [%d - %d]\n", PMT_PID_MIN, PMT_PID_MAX));
		lParameterValidFlag = FALSE;
	}
	if ((pEncodeParameter->m_Hlocation <H_LOCATION_LEFT) || (pEncodeParameter->m_Hlocation >H_LOCATION_RIGHT))
	{
		GLOBAL_TRACE(("Hlocation not valid [%d - %d]\n", H_LOCATION_LEFT, H_LOCATION_RIGHT));
		lParameterValidFlag = FALSE;
	}

	if ((pEncodeParameter->m_VideoPid == pEncodeParameter->m_AudioPid) || (pEncodeParameter->m_VideoPid == pEncodeParameter->m_PmtPid))
	{
		GLOBAL_TRACE(("pid same error\n"));
		lParameterValidFlag = FALSE;
	}
	if ((pEncodeParameter->m_AudioPid == pEncodeParameter->m_PcrPid) || (pEncodeParameter->m_AudioPid == pEncodeParameter->m_PmtPid))
	{
		GLOBAL_TRACE(("pid same error\n"));
		lParameterValidFlag = FALSE;
	}
	if (pEncodeParameter->m_PcrPid == pEncodeParameter->m_PmtPid)
	{
		GLOBAL_TRACE(("pid same error\n"));
		lParameterValidFlag = FALSE;
	}

	return lParameterValidFlag;
}



BOOL8 InitMb86391(Mb86391_UartSendFn pUartSend, Mb86391_UartReceiveFn pUartReceive, Mb86391_DelayMSCB pDelay, S8 bWaitMb86391AnwserFlag)
{
	if (!pUartSend || !pUartReceive || !pDelay)
	{
		return FALSE;
	}
	else
	{
		pMb86391_UartSendFn = pUartSend;
		pMb86391_UartReceiveFn = pUartReceive;
		pMb86391_Delay = pDelay;
	}
	gWaitMb86391AnwserFlag =  bWaitMb86391AnwserFlag;
	return TRUE;
}

BOOL8 ConfigMb86391AllParameterToDefault(Mb86391EncodeParameter *pEncodeParameter)
{
	if (!pEncodeParameter)
	{
		return FALSE;
	}
	pEncodeParameter->m_VideoInputFormat = VIDEO_FORMAT_PAL;
	pEncodeParameter->m_VideoResolution = RESOLUTION_PAL_720_576_NTSC_720_480;
	pEncodeParameter->m_GopStructure = GOP_STRUCTURE_IBBPBBPBB;
	pEncodeParameter->m_GopSize = 15;
	pEncodeParameter->m_AudioBitRate = AUDIO_BITRATE_192;
	pEncodeParameter->m_AudioSampleFreq = AUDIO_SAMPLE_FREQ_48K;
	pEncodeParameter->m_AudioEsMode = AUDIO_ES_MODE_STEREO;
	pEncodeParameter->m_AudioPid = 144;
	pEncodeParameter->m_VideoBitrate = 4200; //4.2M
	pEncodeParameter->m_VideoPid = 128;
	pEncodeParameter->m_PcrPid = 160;
	pEncodeParameter->m_PmtPid = 176;
	pEncodeParameter->m_Hlocation = 3;

	return TRUE;
}

BOOL8 ConfigMb86391AllParameter(Mb86391EncodeParameter *pEncodeParameter)
{
	S32 lTopFieldValidLineCount = 0;
	S32 lBottomFieldValidLineCount = 0;
	S32 lPixelPerLine = 1820;	
	CbrVbrType lCbrVbr = BITRATE_MODE_CBR;
	VideoInputModeType lVideoInputMode = VIDEO_INPUT_MODE_Y_C_MULTIPLEXING;
	StreamOutMethodType lStreamOutMethod = METHOD_EXTERNAL_CLOCK_SYNC;
	SystemMultiplexingMethodType lSystemMultiplexingMethod = MPEG_2_TS;

	if(CheckMb86391Parameter(pEncodeParameter) == FALSE)
	{
		GLOBAL_TRACE(("encode parameters not invalid\n"));
		return FALSE;
	}

	GLOBAL_TRACE(("start config all mb86391 paramters\n"));
	if (Reset86391() == FALSE)
	{
		GLOBAL_TRACE(("Reset86391 error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    pMb86391_Delay(10);
    
    //GLOBAL_TRACE(("Change Resolution\n"));
	if (ChangeResolution(pEncodeParameter->m_VideoResolution, pEncodeParameter->m_VideoInputFormat) == FALSE)
	{
		GLOBAL_TRACE(("ChangeResolution error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("SelectStreamOutputMethod\n"));
	if (SelectStreamOutputMethod(lStreamOutMethod) == FALSE)
	{
		GLOBAL_TRACE(("SelectStreamOutputMethod error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("SelectSystemMultiplexingMethod\n"));
	if (SelectSystemMultiplexingMethod(lSystemMultiplexingMethod) == FALSE)
	{
		GLOBAL_TRACE(("SelectSystemMultiplexingMethod error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("SelectGopStructure\n"));
	if (SelectGopStructure(pEncodeParameter->m_GopStructure) == FALSE)
	{
		GLOBAL_TRACE(("SelectGopStructure error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("ChangeAudioBitRate\n"));
	if (ChangeAudioBitRate(pEncodeParameter->m_AudioBitRate) == FALSE)
	{
		GLOBAL_TRACE(("ChangeAudioBitRate error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("ChangeAudioSampleFreq\n"));
	if (ChangeAudioSampleFreq(pEncodeParameter->m_AudioSampleFreq) == FALSE)
	{
		GLOBAL_TRACE(("ChangeAudioSampleFreq error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("ChangeAudioEsMode\n"));
	if (ChangeAudioEsMode(pEncodeParameter->m_AudioEsMode) == FALSE)
	{
		GLOBAL_TRACE(("ChangeAudioEsMode error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("SelectStreamOutputMethod\n"));
	if (SelectStreamOutputMethod(lStreamOutMethod) == FALSE)  //重复一次？参阅1824P代码
	{
		GLOBAL_TRACE(("SelectStreamOutputMethod error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("ChangeVideoBitrae\n"));
	if (ChangeVideoBitrae(pEncodeParameter->m_VideoBitrate) == FALSE)
	{
		GLOBAL_TRACE(("ChangeVideoBitrae error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("ChangeGopSize\n"));
	if (ChangeGopSize(pEncodeParameter->m_GopSize) == FALSE)
	{
		GLOBAL_TRACE(("ChangeGopSize error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("ChangeVideoPid\n"));
	if (ChangeVideoPid(pEncodeParameter->m_VideoPid) == FALSE)
	{
		GLOBAL_TRACE(("ChangeVideoPid error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("ChangeAudioPid\n"));
	if (ChangeAudioPid(pEncodeParameter->m_AudioPid) == FALSE)
	{
		GLOBAL_TRACE(("ChangeAudioPid error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("ChangePcrPid\n"));
	if (ChangePcrPid(pEncodeParameter->m_PcrPid) == FALSE)
	{
		GLOBAL_TRACE(("ChangePcrPid error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("ChangePmtPid\n"));
	if (ChangePmtPid(pEncodeParameter->m_PmtPid) == FALSE)
	{
		GLOBAL_TRACE(("ChangePmtPid error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("Select_ADATA_And_ALRCK_Alignment\n"));
	if (Select_ADATA_And_ALRCK_Alignment(HEAD_ALIGNMENT) == FALSE)
	{
		GLOBAL_TRACE(("Select_ADATA_And_ALRCK_Alignment error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("Set_ALRCK_And_ACLK_Phase\n"));
	if (Set_ALRCK_And_ACLK_Phase(ALRCK_AND_ACLK_PHASE_1) == FALSE)
	{
		GLOBAL_TRACE(("Set_ALRCK_And_ACLK_Phase error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("Set_ADATA_And_ALRCK_Phase\n"));
	if (Set_ADATA_And_ALRCK_Phase(ADATA_AND_ALRCK_PHASE_0) == FALSE)
	{
		GLOBAL_TRACE(("Set_ADATA_And_ALRCK_Phase error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("SelectAudioIF\n"));
	if (SelectAudioIF(AUDIO_IF_TYPE_MASTER) == FALSE)
	{
		GLOBAL_TRACE(("SelectAudioIF error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
   
	if (pEncodeParameter->m_VideoInputFormat == VIDEO_FORMAT_PAL)
	{
		lTopFieldValidLineCount = 10;
		lBottomFieldValidLineCount = 10;
	}
	else
	{
		lTopFieldValidLineCount = 12;
		lBottomFieldValidLineCount = 12;
	}
    
    //GLOBAL_TRACE(("SetTopFieldValidLineCount\n"));
	if (SetTopFieldValidLineCount(lTopFieldValidLineCount) == FALSE)
	{
		GLOBAL_TRACE(("SetTopFieldValidLineCount error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
	//GLOBAL_TRACE(("SetBottomFieldValidLineCount\n"));
    if (SetBottomFieldValidLineCount(lBottomFieldValidLineCount) == FALSE)
	{
		GLOBAL_TRACE(("SetBottomFieldValidLineCount error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("SetHlocation\n"));
	if (SetHlocation(pEncodeParameter->m_Hlocation, pEncodeParameter->m_VideoInputFormat) == FALSE)
	{
		GLOBAL_TRACE(("SetHlocation error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("SetPixelPerLine\n"));
	if (SetPixelPerLine(lPixelPerLine) == FALSE)
	{
		GLOBAL_TRACE(("SetPixelPerLine error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("SelectVideoInputMode\n"));
	if (SelectVideoInputMode(lVideoInputMode) == FALSE)
	{
		GLOBAL_TRACE(("SelectVideoInputMode error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("SelectVideoInputFormat\n"));
	if (SelectVideoInputFormat(pEncodeParameter->m_VideoInputFormat) == FALSE)
	{
		GLOBAL_TRACE(("SelectVideoInputFormat error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("ChangeResolution\n"));
	if (ChangeResolution(pEncodeParameter->m_VideoResolution, pEncodeParameter->m_VideoInputFormat) == FALSE)
	{
		GLOBAL_TRACE(("ChangeResolution error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}

    //GLOBAL_TRACE(("SelectBitrateMode\n"));
	if (SelectBitrateMode(lCbrVbr) == FALSE)
	{
		GLOBAL_TRACE(("SelectBitrateMode error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
    
    //GLOBAL_TRACE(("Start86390\n"));
	if (Start86390() == FALSE)
	{
		GLOBAL_TRACE(("Start86390 error file:%s line:%d\n", __FILE__, __LINE__));
		return FALSE;
	}
	GLOBAL_TRACE(("end config all mb86391 paramters\n"));

	return TRUE;
}
#endif
/*EOF*/
