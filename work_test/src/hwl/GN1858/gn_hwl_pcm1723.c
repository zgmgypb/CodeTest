#include <unistd.h>
#include "gn_hwl.h"

#define PCM1723_PIN_RSTB			0x00000001
#define PCM1723_PIN_ML				0x00000002
#define PCM1723_PIN_MC				0x00000004
#define PCM1723_PIN_MD				0x00000008

// audio PLL for PCM1723 hardware reset; 
U32 AUDIO_PLL_PCM1723RST(U8 Pcm1723Ch, U32 Pcm1723Data)
{
	U32 lPcm1723Data = Pcm1723Data;

	lPcm1723Data &= ~(PCM1723_PIN_RSTB);  // clear RSTB pin	
	MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);
	usleep(100000);
	lPcm1723Data |= PCM1723_PIN_RSTB; // set RSTB pin
	MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data); 

	return lPcm1723Data;
}

// write PCM1723 register one unit ;
static void AUDIO_PLL_WE_PCM1723_REG(U8 Pcm1723Ch, U32 Data)
{
	U32 lTmpData = 0x0000FFFF & Data; // 扩展位数；
	U32 lPcm1723Data = 0xFFFF;
	U32 m = 16; // 循环次数；

	lTmpData <<= 1;

	lPcm1723Data &= ~(PCM1723_PIN_MC);  // clear pin MC
	MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);   
	lPcm1723Data &= ~(PCM1723_PIN_MD); // clear pin MD
	MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);   
	lPcm1723Data |= PCM1723_PIN_ML; // set pin ML
	MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);  

	while (m)
	{
		// data write ;
		lPcm1723Data &= ~(PCM1723_PIN_MC); // clear clock;
		MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);   

		if( ( lTmpData >> m ) & 1 )
		{
			lPcm1723Data |= PCM1723_PIN_MD;
		}
		else
		{
			lPcm1723Data &= ~(PCM1723_PIN_MD);
		}
		MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);   

		usleep(10);

		lPcm1723Data |= PCM1723_PIN_MC;
		MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);   
		m-- ;
	}

	lPcm1723Data |= PCM1723_PIN_MD;
	MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);   

	lPcm1723Data &= ~(PCM1723_PIN_ML);
	MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);  	   // clear ML
	usleep(10);
	lPcm1723Data |= PCM1723_PIN_ML;
	MFPGA_SetPcm1723(Pcm1723Ch, lPcm1723Data);	  // download parameter	
}

static void AUDIO_PLL_PCM1723_SET_ALL_PIN(unsigned char  pcm1723_cs)
{
	MFPGA_SetPcm1723(pcm1723_cs, (PCM1723_PIN_MC | PCM1723_PIN_MD | PCM1723_PIN_RSTB | PCM1723_PIN_ML));
}

void HWL_Pcm1723Config(U32 Pcm1723Index, S32	AudioSampleFreq)
{
	U32 lData = 0;
	S32 lAudioSamp = 0;

	MFPGA_SetPcm1723(Pcm1723Index, 0x00);
	usleep(200000);
	MFPGA_SetPcm1723(Pcm1723Index, 0xFF);

	usleep(200000);

	AUDIO_PLL_WE_PCM1723_REG(Pcm1723Index, 0x0520) ;

	lData = 0x0608 ;		// 44.1KHz and 256fs ;
	switch(AudioSampleFreq)
	{
	case HWL_AUD_SAMP_32K:  
		lAudioSamp = 2;
		break;
	case HWL_AUD_SAMP_441K:  
		lAudioSamp = 0;
		break;
	case HWL_AUD_SAMP_48K:  
		lAudioSamp = 1;
		break;
	default:
		lAudioSamp = 1;
		return;
	}
	lData = lData | ((lAudioSamp & 0x3) << 6);	// write Audio sample frequency ;

	AUDIO_PLL_WE_PCM1723_REG(Pcm1723Index, lData) ;
}
