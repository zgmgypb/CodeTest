#include "twolame.h"
#include "faac.h"
#include "audio_enc.h"

typedef struct {
	U8				*m_pMp2Buf;
	S32				m_Mp2BufSize;
	twolame_options	*m_pTwolameOpt;
} AUD_EncMp2Handle;

typedef struct {
	U8				*m_pAacBuf;
	S32				m_AacBufSize;
	S32				m_InputSamples; /* 输入采样数 */
	faacEncHandle	m_FaacHandle;
} AUD_EncAacHandle;

typedef struct {
	AUD_EncInitParam m_InitParam;

	union {
		AUD_EncMp2Handle m_Mp2Handle;
		AUD_EncAacHandle m_AacHandle;
	} m_ObjParam;
} AUD_EncHandle;

#define MP2BUFSIZE		(1152 * 8 * 2)
#define CAPTURE_AUDIO_CHANNEL     2

static BOOL AUD_EncMp2Init(AUD_EncMp2Handle *pHandle, S32 AudSample, S32 AudBitrate)
{
	/* grab a set of default encode options */
	pHandle->m_pTwolameOpt = twolame_init();
	if (!pHandle->m_pTwolameOpt) {
		GLOBAL_TRACE(("twolame_init Failed!\n"));
		return FALSE;
	}
	GLOBAL_TRACE(("Using libtwolame version %s.\n", get_twolame_version()));

	/* Use sound file to over-ride preferences for
	mono/stereo and sampling-frequency */
	twolame_set_num_channels(pHandle->m_pTwolameOpt, CAPTURE_AUDIO_CHANNEL); /* 立体声固定设置为2 */
	twolame_set_mode(pHandle->m_pTwolameOpt, TWOLAME_STEREO);
	/* Set the input and output sample rate to the same */
	twolame_set_in_samplerate(pHandle->m_pTwolameOpt, AudSample);
	twolame_set_out_samplerate(pHandle->m_pTwolameOpt, AudSample);
	/* Set the bitrate */
	twolame_set_bitrate(pHandle->m_pTwolameOpt, AudBitrate);
	/* initialise twolame with this set of options */
	if (twolame_init_params(pHandle->m_pTwolameOpt) != 0) {
		GLOBAL_TRACE(("Error: configuring libtwolame encoder failed\n"));
		twolame_close(&pHandle->m_pTwolameOpt);
		return FALSE;
	}
	/* 显示当前配置参数 for debug */
	//twolame_print_config(pHandle->m_pTwolameOpt);

	pHandle->m_Mp2BufSize = MP2BUFSIZE;
	/* Allocate some space for the encoded MP2 audio data */
	if ((pHandle->m_pMp2Buf = (U8 *)calloc(pHandle->m_Mp2BufSize, sizeof(U8))) == NULL) {
		GLOBAL_TRACE(("mp2buffer alloc failed.\n"));
		twolame_close(&pHandle->m_pTwolameOpt);
		return FALSE;
	}

	GLOBAL_TRACE(("mpa handle create is ok.\n"));
	return TRUE;
}

static void AUD_EncMp2Terminate(AUD_EncMp2Handle *pHandle)
{
	if (pHandle) {
		if (pHandle->m_pMp2Buf) {
			GLOBAL_SAFEFREE(pHandle->m_pMp2Buf);
		}
		if (pHandle->m_pTwolameOpt) {
			twolame_close(&pHandle->m_pTwolameOpt);
		}
	}
}

static BOOL AUD_EncAacInit(AUD_EncAacHandle *pHandle, S32 AudSample, S32 AudBitrate /* 单位 bps */)
{
	unsigned long   nInputSamples = 0; /* 输入采样点数目 */ 
	unsigned long   nMaxOutputBytes = 0;  
	faacEncConfigurationPtr pConfiguration;  

	/* 打开faac编码器引擎 */
	pHandle->m_FaacHandle = faacEncOpen(AudSample, CAPTURE_AUDIO_CHANNEL, &nInputSamples, &nMaxOutputBytes);  
	if(pHandle->m_FaacHandle == NULL) {  
		GLOBAL_TRACE(("faacEncOpen Failed!\n"));  
		return FALSE;
	}  
	GLOBAL_TRACE(("nInputSamples:%d, nMaxOutputBytes:%d\n", nInputSamples, nMaxOutputBytes));
	
	pHandle->m_AacBufSize = nMaxOutputBytes;
	pHandle->m_InputSamples = nInputSamples;
	pHandle->m_pAacBuf = (U8 *)GLOBAL_ZMALLOC(pHandle->m_AacBufSize * sizeof(U8));
	if(pHandle->m_pAacBuf == NULL) {
		fprintf(stderr, "pbAACBuffer malloc err!\n");
		faacEncClose(pHandle->m_FaacHandle);
		pHandle->m_FaacHandle = NULL;
		return FALSE;
	}
	
	/* 重置编码器的配置信息 */  
	pConfiguration = faacEncGetCurrentConfiguration(pHandle->m_FaacHandle);  
	pConfiguration->inputFormat = FAAC_INPUT_16BIT; 
	 /* Bitstream output format (取值：0 = Raw; 1 = ADTS) - 输出数据类型（是否包包含adts头），录制MP4文件时，要用raw流，ADTS详细见补充说明3*/
	pConfiguration->outputFormat = 1;   
	pConfiguration->aacObjectType = LOW;  /* AAC object type - AAC对象类型,取值:1-MAIN 2-LOW 3-SSR 4-LTP*/
	pConfiguration->mpegVersion = MPEG2;
	pConfiguration->shortctl = SHORTCTL_NORMAL;
	pConfiguration->useTns = 0;   /* Use Temporal Noise Shaping - 瞬时噪声定形(temporal noise shaping，TNS)滤波器,取值：0-NO 1-YES*///时域噪音控制,大概就是消爆音
	pConfiguration->allowMidside = 0; /* Allow mid/side coding - 是否允许mid/side coding, 取值：0-NO 1-YES*/ 
	pConfiguration->useLfe = 0;   /* Use one of the channels as LFE channel - 是否允许一个通道为低频通道，取值：0-NO 1-YES*/
	pConfiguration->bitRate = AudBitrate/2; /* bitrate / channel of AAC file */
	pConfiguration->quantqual = 100; /* Quantizer quality - 编码质量，取值：efault=100 LOWER<100 HIGHER>100*/
	//pConfiguration->bandWidth = 96000;  
	/* AAC file frequency bandwidth - 频宽 取值：，暂时不清楚参数作用*/
	faacEncSetConfiguration(pHandle->m_FaacHandle, pConfiguration); 

	GLOBAL_TRACE(("aac handle create is ok.\n"));
	return TRUE;
}

static void AUD_EncAacTerminate(AUD_EncAacHandle *pHandle)
{
	if (pHandle) {
		if (pHandle->m_pAacBuf) {
			GLOBAL_SAFEFREE(pHandle->m_pAacBuf);
		}
		if (pHandle->m_FaacHandle) {
			faacEncClose(pHandle->m_FaacHandle);
		}
	}
}

static BOOL AUD_EncMp2Process(AUD_EncMp2Handle *pHandle, U8 *pData, S32 DataLen, U8 **pBuf, S32 *pActLen)
{
	*pActLen = twolame_encode_buffer_interleaved(pHandle->m_pTwolameOpt, 
		(short int *)pData, 
		DataLen / 2, 
		pHandle->m_pMp2Buf,
		pHandle->m_Mp2BufSize);
	*pBuf = pHandle->m_pMp2Buf;

	return TRUE;
}

static BOOL AUD_EncAacProcess(AUD_EncAacHandle *pHandle, U8 *pData, S32 DataLen, U8 **pBuf, S32 *pActLen)
{
	*pActLen = faacEncEncode(pHandle->m_FaacHandle, (int32_t *)pData, DataLen / 2, pHandle->m_pAacBuf, pHandle->m_AacBufSize);
	*pBuf = pHandle->m_pAacBuf;

	return TRUE;
}

HANDLE32 AUD_EncCreate(AUD_EncInitParam *pInitParam)
{
	AUD_EncHandle *plHandle;

	plHandle = (AUD_EncHandle *)GLOBAL_ZMALLOC(sizeof(AUD_EncHandle));
	if (plHandle) {
		memcpy(&plHandle->m_InitParam, pInitParam, sizeof(AUD_EncInitParam));

		if (pInitParam->m_AudEncMode == AUD_ENC_MODE_MPEG1_L2) {
			if (!AUD_EncMp2Init(&plHandle->m_ObjParam.m_Mp2Handle, pInitParam->m_AudSample, pInitParam->m_AudBitrate)) {
				GLOBAL_TRACE(("AUD_EncMp2Init Failed!\n"));
				GLOBAL_SAFEFREE(plHandle);
				return NULL;
			}
		}
		else if (pInitParam->m_AudEncMode == AUD_ENC_MODE_LC_AAC) {
			if (!AUD_EncAacInit(&plHandle->m_ObjParam.m_AacHandle, pInitParam->m_AudSample, pInitParam->m_AudBitrate * 1000)) {
				GLOBAL_TRACE(("AUD_EncAacInit Failed!\n"));
				GLOBAL_SAFEFREE(plHandle);
				return NULL;
			}
		}
		else if ((pInitParam->m_AudEncMode == AUD_ENC_MODE_AC3) ||
			(pInitParam->m_AudEncMode == AUD_ENC_MODE_EAC3)) {
					
		}
		else {
			GLOBAL_SAFEFREE(plHandle);
			return NULL;
		}
	}

	return plHandle;
}

BOOL AUD_EncProcess(HANDLE32 Handle, U8 *pData, S32 DataLen, U8 **pBuf, S32 *pActLen)
{
	AUD_EncHandle *plHandle = (AUD_EncHandle *)Handle;

	if (plHandle) {
		if (plHandle->m_InitParam.m_AudEncMode == AUD_ENC_MODE_MPEG1_L2) {
			return AUD_EncMp2Process(&plHandle->m_ObjParam.m_Mp2Handle, pData, DataLen, pBuf, pActLen);
		}
		else if (plHandle->m_InitParam.m_AudEncMode == AUD_ENC_MODE_LC_AAC) {
			return AUD_EncAacProcess(&plHandle->m_ObjParam.m_AacHandle, pData, DataLen, pBuf, pActLen);
		}
	}

	return FALSE;
}

void AUD_EncDestroy(HANDLE32 Handle)
{
	AUD_EncHandle *plHandle = (AUD_EncHandle *)Handle;

	if (plHandle) {
		if (plHandle->m_InitParam.m_AudEncMode == AUD_ENC_MODE_MPEG1_L2) {
			AUD_EncMp2Terminate(&plHandle->m_ObjParam.m_Mp2Handle);
		}
		else if (plHandle->m_InitParam.m_AudEncMode == AUD_ENC_MODE_LC_AAC) {
			AUD_EncAacTerminate(&plHandle->m_ObjParam.m_AacHandle);
		}
		
		GLOBAL_SAFEFREE(plHandle);
	}
}

/* 由于 AAC 需要输入固定大小的数据buffer，需要获取需要的大小 */
S32 AUD_EncAacGetInputBufSize(HANDLE32 Handle)
{
	AUD_EncHandle *plHandle = (AUD_EncHandle *)Handle;

	if (plHandle && plHandle->m_ObjParam.m_AacHandle.m_FaacHandle) {
		return plHandle->m_ObjParam.m_AacHandle.m_InputSamples * 2;
	}
}


