extern "C"{

#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#undef CRYPTO

extern "C" {
#include <librtmp/rtmp.h>
#include <librtmp/log.h>
#include <librtmp/amf.h>
}
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

extern "C" {
#include <common/common.h>
#include <common/cpu.h>
#include <x264.h>
#include <encoder/set.h>   

}
#include "capture.h"
#include "vencoder.h"
#include "sender.h"


#define VIDEO_WIDTH 1280
#define VIDEO_HEIGHT 720
#define VIDEO_FPS 30.0

#define TARGET_IP "127.0.0.1"
#define TARGET_PORT 3030

unsigned long GetTickCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

}


char * put_byte( char *output, uint8_t nVal ) 
{ 
    output[0] = nVal; 
    return output+1; 
} 
char * put_be16(char *output, uint16_t nVal ) 
{ 
    output[1] = nVal & 0xff; 
    output[0] = nVal >> 8; 
    return output+2; 
} 
char * put_be24(char *output,uint32_t nVal ) 
{ 
    output[2] = nVal & 0xff; 
    output[1] = nVal >> 8; 
    output[0] = nVal >> 16; 
    return output+3; 
} 
char * put_be32(char *output, uint32_t nVal ) 
{ 
    output[3] = nVal & 0xff; 
    output[2] = nVal >> 8; 
    output[1] = nVal >> 16; 
    output[0] = nVal >> 24; 
    return output+4; 
} 
char *  put_be64( char *output, uint64_t nVal ) 
{ 
    output=put_be32( output, nVal >> 32 ); 
    output=put_be32( output, nVal ); 
    return output; 
}
char * put_amf_string( char *c, const char *str ) 
{ 
    uint16_t len = strlen( str ); 
    c=put_be16( c, len ); 
    memcpy(c,str,len); 
    return c+len; 
} 

char * put_amf_double( char *c, double d ) 
{ 
    *c++ = AMF_NUMBER;  /* type: Number */ 
    { 
        unsigned char *ci, *co; 
        ci = (unsigned char *)&d; 
        co = (unsigned char *)c; 
        co[0] = ci[7]; 
        co[1] = ci[6]; 
        co[2] = ci[5]; 
        co[3] = ci[4]; 
        co[4] = ci[3]; 
        co[5] = ci[2]; 
        co[6] = ci[1]; 
        co[7] = ci[0]; 
    } 
    return c+8; 
}

RTMP *openRtmp(char *uri)
{
	int err = 0;
	RTMP *rtmp;
	RTMP_debuglevel = RTMP_LOGINFO; 
	rtmp = RTMP_Alloc(); 

	printf("Start openRtmp TRUE : %d, FALSE : %d\n", TRUE, FALSE);
	
	RTMP_Init(rtmp); 
	err = RTMP_SetupURL(rtmp, (char*)uri); 
	printf("SetupURL [%s] ret : %d\n", uri, err);
	if(err < 0)
	{
		printf("SetupURL [%s] err : %d\n", uri, err);
		return NULL;  
	} 

	//rtmp->Link.lFlags |= RTMP_LF_LIVE;
	rtmp->Link.lFlags = RTMP_LF_LIVE;
	RTMP_SetBufferMS(rtmp, 10);//1hour

	RTMP_EnableWrite(rtmp); 
	err = RTMP_Connect(rtmp, NULL); 
	printf("Connect [%s] ret : %d\n", uri, err);
	if(err < 0)
	{
		printf("Connect [%s] err : %d\n", uri, err);
		return NULL;  
	} 

	err = RTMP_ConnectStream(rtmp, 0); 
	printf("ConnectStream [%s] ret : %d\n", uri, err);
	if(err < 0)
	{
		printf("ConnectStream [%s] err : %d\n", uri, err);
		return NULL;
	}
	RTMP_UpdateBufferMS (rtmp);	
	return rtmp;
} 

int connectRTMP(RTMP *r, RTMPPacket *packet, unsigned char* szBodyBuffer, x264_param_t * p264Param)
{
	int ret;
	memset(packet,0,sizeof(RTMPPacket)); 
	packet->m_nChannel = 0x04; 
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE; 
	packet->m_nTimeStamp = 0; 
	packet->m_nInfoField2 = r->m_stream_id; 
	packet->m_hasAbsTimestamp = 0; 
	packet->m_body =(char *) szBodyBuffer; 
	char * szTmp=(char *)szBodyBuffer; 
	packet->m_packetType = RTMP_PACKET_TYPE_INFO; 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string(szTmp, "@setDataFrame" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string(szTmp, "onMetaData" ); 
	szTmp=put_byte(szTmp, AMF_OBJECT ); 
	szTmp=put_amf_string( szTmp, "author" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "copyright" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "description" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "keywords" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "rating" );
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_amf_string( szTmp, "presetname" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "Custom" ); 
	szTmp=put_amf_string( szTmp, "width" ); 
	szTmp=put_amf_double( szTmp, p264Param->i_width ); 
	szTmp=put_amf_string( szTmp, "width" ); 
	szTmp=put_amf_double( szTmp, p264Param->i_width ); 
	szTmp=put_amf_string( szTmp, "height" ); 
	szTmp=put_amf_double( szTmp, p264Param->i_height ); 
	szTmp=put_amf_string( szTmp, "framerate" ); 
	szTmp=put_amf_double( szTmp,
	(double)p264Param->i_fps_num/p264Param->i_fps_den ); 
	szTmp=put_amf_string( szTmp, "videocodecid" ); 
	szTmp=put_byte(szTmp, AMF_STRING ); 
	szTmp=put_amf_string( szTmp, "avc1" ); 
	szTmp=put_amf_string( szTmp, "videodatarate" ); 
	szTmp=put_amf_double( szTmp, p264Param->rc.i_bitrate );  
	szTmp=put_amf_string( szTmp, "avclevel" ); 
	szTmp=put_amf_double( szTmp, p264Param->i_level_idc );  
	szTmp=put_amf_string( szTmp, "avcprofile" ); 
	szTmp=put_amf_double( szTmp, 0x42 );  
	szTmp=put_amf_string( szTmp, "videokeyframe_frequency" ); 
	szTmp=put_amf_double( szTmp, 3 );  
	szTmp=put_amf_string( szTmp, "" ); 
	szTmp=put_byte( szTmp, AMF_OBJECT_END ); 
	packet->m_nBodySize=szTmp-(char *)szBodyBuffer; 
	
	ret = RTMP_SendPacket(r, packet,1);
	fprintf(stderr, "connectRTMP RTMP_SendPacket ret : %d\r\n", ret);

	return 0;
}

int startRTMP(RTMP *r, RTMPPacket *packet, unsigned char* szBodyBuffer, unsigned char* szNalBuffer, x264_t *p264Handle)
{
	int ret = 0;
	bs_t bs;
	char * szTmp=(char *)szBodyBuffer;  
	packet->m_body =(char *) szBodyBuffer;
	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;   /* VIDEO */  

	szBodyBuffer[ 0]=0x17;  
	szBodyBuffer[ 1]=0x00;  
	szBodyBuffer[ 2]=0x00;  
	szBodyBuffer[ 3]=0x00;  
	szBodyBuffer[ 4]=0x00;  
	szBodyBuffer[ 5]=0x01;  
	szBodyBuffer[ 6]=0x42;  
	szBodyBuffer[ 7]=0xC0;  
	szBodyBuffer[ 8]=0x15;  
	szBodyBuffer[ 9]=0x03;  
	szBodyBuffer[10]=0x01;  
	szTmp=(char *)szBodyBuffer+11;  
	short slen=0;  
	bs_init(&bs,szNalBuffer,16);//初始话bs  

	x264_sps_write(&bs, p264Handle->sps);//读取编码器的SPS  
	slen=bs.p-bs.p_start+1;//spslen（short）  
	slen=htons(slen);  
	memcpy(szTmp,&slen,sizeof(short));  
	szTmp+=sizeof(short);  
	*szTmp=0x67;  
	szTmp+=1;  
	memcpy(szTmp,bs.p_start,bs.p-bs.p_start);  
	szTmp+=bs.p-bs.p_start;  
	*szTmp=0x01;  
	szTmp+=1;  
	bs_init(&bs,szNalBuffer,16);//初始话bs  
	x264_pps_write(&bs, p264Handle->sps, p264Handle->pps);//读取编码器的PPS  
	slen=bs.p-bs.p_start+1;//spslen（short）  
	slen=htons(slen);  
	memcpy(szTmp,&slen,sizeof(short));  
	szTmp+=sizeof(short);  
	*szTmp=0x68;  
	szTmp+=1;  
	memcpy(szTmp,bs.p_start,bs.p-bs.p_start);  
	szTmp+=bs.p-bs.p_start;  
	packet->m_nBodySize=szTmp-(char *)szBodyBuffer;  
	ret = RTMP_SendPacket(r, packet,0);
	fprintf(stderr, "startRTMP RTMP_SendPacket ret : %d\r\n", ret);

	return 0;
}
static RTMP *rtmpX264;
#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)
int send_video_sps_pps(unsigned char *sps, int sps_len, unsigned char *pps, int pps_len)
{
	RTMPPacket * packet;
	unsigned char * body;
	int i;

	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+1024);
	memset(packet,0,RTMP_HEAD_SIZE);

	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (unsigned char *)packet->m_body;

	//memcpy(winsys->pps,buf,len);
	//winsys->pps_len = len;

	i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	/*sps*/
	body[i++]   = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i],sps,sps_len);
	i +=  sps_len;

	/*pps*/
	body[i++]   = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i], pps, pps_len);
	i +=  pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 0x04;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet->m_nInfoField2 = rtmpX264->m_stream_id;

	/*调用发送接口*/
	RTMP_SendPacket(rtmpX264,packet,TRUE);
	free(packet);    

	return 0;
}

static unsigned long start_time = 0;
int send_rtmp_video(unsigned char * buf,int len)
{
	int type;
	RTMPPacket * packet;
	unsigned char * body;
	static unsigned long timeoffset = 0;

	timeoffset = GetTickCount() - start_time;  /*start_time为开始直播时的时间戳*/
 
	/*去掉帧界定符*/
	if (buf[2] == 0x00) { /*00 00 00 01*/
		buf += 4;
		len -= 4;
	} else if (buf[2] == 0x01){ /*00 00 01*/
		buf += 3;
		len -= 3;
	}
	type = buf[0]&0x1f;
 
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+len+9);
	memset(packet,0,RTMP_HEAD_SIZE);
 
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	packet->m_nBodySize = len + 9;
 
	/*send video packet*/
	body = (unsigned char *)packet->m_body;
	memset(body,0,len+9);
 
	/*key frame*/
	body[0] = 0x27;
	if (type == NAL_SLICE_IDR)
	{
	    body[0] = 0x17;
	}
 
	body[1] = 0x01;   /*nal unit*/
	body[2] = 0x00;
	body[3] = 0x00;
	body[4] = 0x00;

	body[5] = (len >> 24) & 0xff;
	body[6] = (len >> 16) & 0xff;
	body[7] = (len >>  8) & 0xff;
	body[8] = (len ) & 0xff;
 
	/*copy data*/
	memcpy(&body[9],buf,len);
 
	packet->m_hasAbsTimestamp = 0;
	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nInfoField2 = rtmpX264->m_stream_id;
	packet->m_nChannel = 0x04;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet->m_nTimeStamp = timeoffset;
	/*调用发送接口*/
	RTMP_SendPacket(rtmpX264, packet,TRUE);

	free(packet);
}

int main (int argc, char **argv)
{
	RTMP *pRtmp;	
	RTMPPacket packet={0};
	unsigned char szNalBuffer[1024*32]; 
	unsigned char szBodyBuffer[1024*32]; 
	char uri[]="rtmp://127.0.0.1:1935/live/webcam";

	unsigned int nTimes=0; 
	unsigned int oldTick = 0; 
   	unsigned int newTick=0; 
	x264_t *p264Handle;
	x264_param_t * p264Param; 

	void *capture = capture_open("/dev/video0", VIDEO_WIDTH, VIDEO_HEIGHT, PIX_FMT_YUV420P);
	if (!capture) {
		fprintf(stderr, "ERR: can't open '/dev/video0'\n");
		exit(-1);
	}

	void *encoder = vencoder_open(VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_FPS);
	if (!encoder) {
		fprintf(stderr, "ERR: can't open x264 encoder\n");
		exit(-1);
	}

	
        #ifdef RTSP	
	void *sender = sender_open(TARGET_IP, TARGET_PORT);
	if (!sender) {
		fprintf(stderr, "ERR: can't open sender for %s:%d\n", TARGET_IP, TARGET_PORT);
		exit(-1);
	}
	#endif

	int tosleep = 1000000 / VIDEO_FPS;

	FILE *fp_save = fopen("./my1.264", "wb");

	rtmpX264 = pRtmp = openRtmp(uri);
	if(pRtmp == NULL)
	{
		goto EXIT;
	}
	p264Param = get_x264_param_t(encoder); 
	p264Handle = get_x264_handle(encoder);

	connectRTMP(pRtmp, &packet, szBodyBuffer, p264Param);
	//startRTMP(pRtmp, &packet, szBodyBuffer, szNalBuffer, p264Handle);

	start_time = oldTick = GetTickCount(); 
   	packet.m_nTimeStamp=0; 

	//while(1)
	//	sleep(1);

#if 01
	for (; ; ) {
		// 抓
		Picture pic;
		capture_get_picture(capture, &pic);

		// 压
		const void *outdata;
		int outlen;
		int rc = vencoder(encoder, pic.data, pic.stride, &outdata, &outlen);
		if (rc < 0) 
			continue;
	}

	fclose(fp_save);
	
	#ifdef RTSP
	sender_close(sender);
	#endif
#endif
EXIT:
	printf("error , exit !\r\n");
	vencoder_close(encoder);
	capture_close(capture);

	while(1)
		sleep(1);

	return 0;
}

