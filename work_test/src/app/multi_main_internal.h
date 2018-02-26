#ifndef _MUL_MULT_INTERNAL_H_
#define _MUL_MULT_INTERNAL_H_

/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "multi_main.h"
#include "global_micros.h"
#include "platform_conf.h"
#include "libc_assist.h"
#include "platform_assist.h"
#include "multi_hwl.h"

//encoder main board temperature
#include "hwl_ds8b20.h"

#ifdef GM8358Q
#include "gn_hwl.h"
#include "spi_flash_upgrade.h"
#endif
#ifdef GM8398Q
#include "gn_hwl.h"
#include "spi_flash_upgrade.h"
#endif
#include "multi_tsp.h"
#include "webmain.h"
#include "mpeg2.h"
#include "mxml.h"
#include "scs_main.h"
#include "upgrade.h"
#include "authorization_mid.h"
#include "multi_tsp.h"
#include "crypto.h"
#include "TunerDemod.h"
#include "multi_hwl_monitor.h"
#include "mxml_warp.h"

#ifdef ENCODER_CARD_PLATFORM
#include "card_app.h"
#endif

#ifdef USE_CARD_FRP
#else
#include "frp_menu.h"
#endif

#include "fpga_reg.h"

#include "mpeg2_pes_covert_micro.h"

#define MULT_NEW_KERNEL_FILE_PATHNAME			"/mnt/baku/uImage"
#define MULT_MODULE_FILE_STORAGE_PATHNAME		"/mnt/baku/modules/"
#define MULT_CARD_SYSTEM_MAX_MODULE_NUM			(6)
#define MULT_CARD_SYSTEM_MAX_TYPE_NUM			(16)
/* Macro ---------------------------------------------------------------------- */
#define MULT_DEVICE_COMPLETE_TYPE			((MULT_DEVICE_TYPE << 8) & 0xFF00) | (MULT_DEVICE_SUB_TYPE & 0xFF)

#define MULT_TASK_STATCK_SIZE				(1024*1024)
#define MULT_MAIN_MSG_QUEUE_MAX_TIMEOUT		5000
#define MULT_RELEASE_DATE_STRING_BUF_LEN	20	
#define MULT_RELEASE_DATE_STRING_LEN		19	//"2012-03-30 12:20:59"
#define MULT_SN_STRING_BUF_LEN				20
#define MULT_SN_STRING_LEN					18	//"TC73F84FFFXXXXXXXX"
// #define MULT_AUTH_STRING_BUF_LEN			48
// #define MULT_AUTH_STRING_LEN				32	//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
#define MULT_INFO_STRING_BUF				64		
#define MULT_APPLICATION_CODE_BUF_SIZE		(4 * 1024)		

#define MULT_CHANNEL_TYPE_ASI_NAME				("ASI")
#define MULT_CHANNEL_TYPE_ETH_NAME				("ETH")
#define MULT_CHANNEL_TYPE_ETH_LOOP_NAME			("ETH_LOOP")
#define MULT_CHANNEL_TYPE_ETH_DEP_NAME			("ETH_DEP")
#define MULT_CHANNEL_TYPE_ETH_LOOP_DEP_NAME		("ETH_LOOP_DEP")
#define MULT_CHANNEL_TYPE_E3DS3_NAME			("E3DS3")  
#define MULT_CHANNEL_TYPE_TUNER_S_NAME			("TUNER_S")
#define MULT_CHANNEL_TYPE_TUNER_C_NAME			("TUNER_C")
#define MULT_CHANNEL_TYPE_TUNER_DTMB_NAME		("TUNER_DTMB")

#if defined(GN1846) || defined(GN1866)
#define MULT_CHANNEL_TYPE_ENCODER_NAME						("ENCODER")	
#endif

#ifdef GM8358Q
#define MULT_CHANNEL_TYPE_ENCODER_NAME						("ENCODER")	
#endif
#ifdef GM8398Q
#define MULT_CHANNEL_TYPE_ENCODER_CVBS_DXT8243_NAME			("ENCODER_CVBS_DXT8243")	
#endif

#define MULT_CHANNEL_TYPE_QPSK_NAME				("QPSK")
#define MULT_CHANNEL_TYPE_DTMB_NAME				("DTMB")

#define MULT_CHANNEL_TYPE_MODULATOR_NAME		("MODULATOR")
#define MULT_CHANNEL_TYPE_MODULATOR_NAME2		("QAM")
#define MULT_CHANNEL_TYPE_DEMODULATOR_NAME		("DEMODULATOR")

#define MULT_CHANNEL_SUB_TYPE_TS_OVER_ASI_NAME	("ASI")
#define MULT_CHANNEL_SBU_TYPE_E3DS3_NAME		("E3DS3")  
#define MULT_CHANNEL_SBU_TYPE_TUNER_S_NAME		("TUNER_S")  
#define MULT_CHANNEL_SBU_TYPE_TUNER_C_NAME		("TUNER_C")  
#define MULT_CHANNEL_SBU_TYPE_TUNER_DTMB_NAME	("TUNER_DTMB")  

#ifdef GM8358Q
#define MULT_CHANNEL_SUB_TYPE_ENCODER_CVBS		("CVBS")  
#endif
#ifdef GM8398Q
#define MULT_CHANNEL_SUB_TYPE_ENCODER_CVBS		("CVBS")  
#endif

#ifdef GN1846
#define MULT_CHANNEL_SUB_TYPE_ENCODER_HI3531A		("ENC_HI3531A")  
#endif
#ifdef GN1866
#define MULT_CHANNEL_SUB_TYPE_ENCODER_HI3519		("ENC_HI3519")  
#endif

#define MULT_CHANNEL_SUB_TYPE_TS_OVER_UDP_NAME	("TS_OVER_IP")
#define MULT_MODULATOR_SUB_TYPE_AD9789_NAME		("QAM_A")
#define MULT_MODULATOR_SUB_TYPE_BL85KMM_NAME	("QAM_B")

#define MULT_CHANNEL_SBU_TYPE_E3DS3_MAX_NUM      (4)  

#define MULT_AUTH_TOTAL_TRAIL_TIME				(3600 * 24 * 30)
#define MULT_AUTH_COUNT_DOWN_CYCLE				(300)

#define MULT_WEB_BASE_DIR						("/tmp/web/")
#define MULT_DEFAULT_HOME_PAGE					("index.asp")
#define MULTI_DEFAULT_IP_ADDR					0x7878780A
#define MULTI_DEFAULT_IP_MASK					0xFFFFFF00
#define MULTI_DEFAULT_IP_GATE					0x78787801
#define MULTI_DEFAULT_NTP_INTERVAL				(8*3600)
#define MULTI_MIN_NTP_INTERVAL					(10)
#define MULTI_DEFAULT_SUB_IP_ADDR				0xE00A0A01
#define MULTI_DEFAULT_SUB_IP_PORT				5000

#define CGIC_UPLOAD_FILE_MAX_SIZE				(1024*1024)

#define MULT_PARAMETER_DESCRIPTOR			("PARAM%.8X")
#define MULT_MAINTANCE_DESCRIPTOR			("MAIT%.8X")

#define MULT_XML_BASE_DIR					("/tmp/web/tmp/")//xml以及其它临时文件存放和展开位置
// #define MULT_TS_BASE_DIR					("/tmp/web/ts/")//TS文件存放和展开位置


#define MULT_DEVICE_INFORMATION_XML			("device_information.xml")
#define MULT_DEVICE_MAINTENANCE_XML			("maintenance.xml")
#define MULT_DEVICE_PARAMETER_XML			("device_parameter.xml")
#define MULT_SYSTEM_PARAMETER_XML			("system_parameter.xml")
#define MULT_SERVICE_PARAMETER_XML			("service_parameter.xml")
#define MULT_MONITOR_SETTING_XML			("monitor_setting.xml")
#define MULT_WEB_USER_MANAGE_FILE_PATHNAME	("umconfig.txt")
#ifdef GN1846
#define MULT_EDID_SETTINGS_XML				("edid_settings.xml")
#endif

#define CGIC_UPLOAD_FILE_PATHNAME			("/tmp/web/upload/upload.bin")
#define MULT_IO_TMP_FILE_PATHNAME			"/tmp/temp.tar.gz"

#define MULT_LOGO_FILE_WEB_PATH				("/tmp/web/graphics/logo.jpg")
#define MULT_INTRO_FILE_WEB_PATH			("/tmp/web/graphics/intro.jpg")
#define MULT_MANUAL_FILE_WEB_PATH			("/tmp/web/graphics/manual.pdf")
#define MULT_OVERRIDE_FILE_WEB_PATH			("/tmp/override.tar.gz")//20130709增加OEM文件覆盖功能



#define MULT_STORAGE_BASE_DIR				("/mnt/mtd/")//xml以及其它临时文件存放和展开位置
// #define MULT_STORAGE_BASE_DIR				("/tmp/")//xml以及其它临时文件存放和展开位置
#define MULT_OEM_FILE_PATHNAME				("oem.bin")
#define MULT_LICENSE_FILE_PATHNAME			("license.bin")
#define MULT_PARAMETER_FILE_PATHNAME		("parameter.bin")
#define MULT_PARAMETER_BAKU_PATHNAME		("parameter.bk")
#define MULT_PARAMETER_DEF_PATHNAME			("parameter.def")
#define MULT_PROGRAM_FILE_PATHNAME			("software.tar.gz")
#define MULT_PROGRAM_BAKU_PATHNAME			("software.tar.bk")
#define MULT_MAINTENACE_FILE_PATHNAME		("maintenance.bin")
#ifdef GM8358Q
#define MULT_FIRMWARE_FILE_PATHNAME			("firmware.tar.gz")
#endif
#ifdef GM8398Q
#define MULT_FIRMWARE_FILE_PATHNAME			("firmware.tar.gz")
#endif
#define MULT_PROGRAM_INFO_FILE_PATHNAME		("/tmp/web/upload/program.xml")




// #define MULTI_DEFAULT_IGMP_VERSION			2
// #define MULTI_DEFAULT_IGMP_INTERVAL			20

#define MULT_FRP_LANGUAGE_ENG					0
#define MULT_FRP_LANGUAGE_CHN					1

#define MULT_CONFIG_VERSION_NUMBER				0
#define MULT_PARAMETER_VERSION_NUMBER			0

#define MULT_CRITICAL_TEMP						65
#define MULT_FAN_TEMP							35
#define MULTL_TEMP_ERROR_TORLARANCE				(3)
#define MULT_FAN_OFFSET_TEMP					10

#define MULT_SNTP_FAILED_RETRY_TIMEOUT			10//秒

#define SNMP_WARP_MAX_DESCR						260

#define SNMP_DEFAULT_RO_COMMUNITY			"public"
#define SNMP_DEFAULT_RW_COMMUNITY			"private"
#define SNMP_DEFAULT_DEVICE_NAME			"------"
#define SNMP_DEFAULT_DEVICE_LOCATION		"------"
/* Types Define (struct typedef)----------------------------------------------- */

typedef enum
{
	MULT_TASK_MARK_STOP = 0,
	MULT_TASK_MARK_RUN,
	MULT_TASK_MARK_WAIT_STOP
}MULT_TaskMark;

/*系统信息----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
	U8		m_pSoftVersion[MULT_INFO_STRING_BUF];
	U8		m_pHardVersion[MULT_INFO_STRING_BUF];
	U8		m_pSoftRelease[MULT_INFO_STRING_BUF];
	U8		m_pFPGARelease[MULT_INFO_STRING_BUF];
	U8		m_pMuxFPGARelease[MULT_INFO_STRING_BUF];
	U8		m_pSNString[MULT_SN_STRING_BUF_LEN];//序列号字符串

	BOOL	m_LicenseValid;
	U32		m_LicenseMode;
	/*设备子定义授权信息*/
	S32		m_LicenseInASINum;
	S32		m_LicenseInIPNum;
	S32		m_LicenseDescrambleNum;
	S32		m_LicenseOutIPNum;
	S32		m_LicenseOutTsNum;
	S32		m_LicenseSCSNum;

	CHAR_T	m_ApplicationCode[MULT_APPLICATION_CODE_BUF_SIZE];
	CHAR_T	m_LicenseDate[MULT_INFO_STRING_BUF];
	CHAR_T	m_ExpireDate[MULT_INFO_STRING_BUF];
	S32		m_TrailTime;



	CHAR_T	m_pModelName[MULT_INFO_STRING_BUF];
	CHAR_T	m_pWEBENG[MULT_INFO_STRING_BUF];
	CHAR_T	m_pWEBCHN[MULT_INFO_STRING_BUF];
	CHAR_T	m_pLCDENG[MULT_INFO_STRING_BUF];
	CHAR_T	m_pLCDCHN[MULT_INFO_STRING_BUF];
	CHAR_T	m_pManufacter[MULT_INFO_STRING_BUF];
	CHAR_T	m_pManufacterWEBADDR[MULT_INFO_STRING_BUF];
	BOOL	m_bHaveManual;
}MULT_Information;//全部参数均为开机时动态生成！

/*系统参数----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
typedef struct  
{
	S32		m_PLLFreqOffset;//锁相环时钟频率矫正值
	U32		m_10MPLLDAValue;//锁相环时钟频率矫正值
	F64		m_10MTrainningValue;//锁相环时钟频率矫正值
	U32		m_PowerUpCount;//没启动一次即+1
	BOOL	m_PrivateChannelSetupMark;//私有CAS开关

#ifdef GM8358Q
	S32     m_EncoderChannelTsStatusCount[8];
	S32		m_EncoderCCASStatusCount[8];
	S32		m_EncoderConCount[8];
	S32     m_EncoderParaApplyCount[4];
#endif	
//报警计数
#ifdef GM8398Q
	S32     m_EncoderChannelTsStatusCount[8];
	S32		m_EncoderCCASStatusCount[8];
	S32		m_EncoderConCount[8];
	S32     m_EncoderParaApplyCount[4];
#endif	

#ifdef GN1846 
	S32				m_AudPtsRelativeDelayTime[MULT_MAX_CHN_NUM]; /* 单位：us */
	S32				m_PtsDelayTime[MULT_MAX_CHN_NUM]; /* 单位：us */
	S32				m_MaxPtsPcrInterval[MULT_MAX_CHN_NUM]; /* 单位：us */
	S32				m_MinPtsPcrInterval[MULT_MAX_CHN_NUM]; /* 单位：us */
	S32				m_AudDelayFrameNum[MULT_MAX_CHN_NUM];
#endif

}MULT_Maintenace;

/*----SNMP 参数----*/
typedef struct  
{
	CHAR_T					m_pRWCommunity[SNMP_WARP_MAX_DESCR];//读写“社区”
	CHAR_T					m_pROCommunity[SNMP_WARP_MAX_DESCR];//自读“社区”
	CHAR_T					m_pDeviceName[SNMP_WARP_MAX_DESCR];
	CHAR_T					m_pDeviceLocation[SNMP_WARP_MAX_DESCR];
	S32						m_NormalTrapInterval;//每个TRAP节点，发送两次TRAP的间隔(毫秒，系统最小解析度为10ms)
	U32						m_TRAPIPAddress;//TRAP目标地址
	U16						m_TRAPPort;//TRAP目标端口号
	U16						m_AgentPort;//TRAP目标端口号
	BOOL					m_TRAPGlobalMark;//TRAP全局开关
}SNMP_InitParam;

#ifdef SUPPORT_SYSLOG_MODULE

#define MULT_SYSLOG_MAX_SUBMODULE_SUPPORT		32

typedef struct  
{
	BOOL					m_SyslogGlobalMark;//系统全局开关
	U32						m_RemoteAddr;//远程日志地址
	U16						m_RemotePort;//远程日志端口
	BOOL					m_RemoteMark;//远程日志开关

	S32						m_LogLevel;//开启日志的等级，对应调用 PL_SyslogLevel()
}MULT_Syslog;

#endif

typedef struct
{
	S16		m_WebLanguage;//Web页面语言：0，英语(ENG)；1，中文(CHN)；
	S16		m_FrpLanguage;//前面板语言索引号：0，英语(ENG)；1，中文(CHN)；

	U32		m_ManageIPv4Addr;//IP地址
	U32		m_ManageIPv4Mask;//IP掩码
	U32		m_ManageIPv4Gate;//IP网关
	U8		m_pMAC[GLOBAL_MAC_BUF_SIZE];

	U32		m_NTPServerAddr;
	S32		m_NTPInterval;//更新间隔（秒） 默认值8*3600(8个小时)
	BOOL8	m_NTPSyncMark;

#ifdef GN1846
	U32		m_IpOutputType;
	U32		m_OutputCharset;
#endif

	//S8		m_IGMPVersion;//2,3；默认为2，暂不支持3；
	//S32		m_IGMPInterval;//自动发送IGMP JOIN包的间隔（秒），推荐值20；
	//BOOL8	m_IGMPAutoMark;//自动发送IGMP JOIN包的开关

	SNMP_InitParam m_SNMPInitParam;
	BOOL	m_SNMPGlobalMark;



}MULT_Config;//需要保存的参数！



/*通道参数---------------------------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
	GS_ETH_PROTOCOL_UDP = 0,
	GS_ETH_PROTOCOL_RTP,
}GS_CONST_PROTOCOL;

typedef enum
{
	GS_MODULATOR_STANDARD_ANNEX_A = 0,
	GS_MODULATOR_STANDARD_ANNEX_B,
	GS_MODULATOR_STANDARD_ANNEX_C
}GS_ModulatorStandard;

typedef enum
{
	GS_MODULATOR_QAM_16 = 0,
	GS_MODULATOR_QAM_32,
	GS_MODULATOR_QAM_64,
	GS_MODULATOR_QAM_128,
	GS_MODULATOR_QAM_256,
	GS_MODULATOR_QAM_512,
	GS_MODULATOR_QAM_1024,
	GS_MODULATOR_QAM_UNUSED,
	GS_MODULATOR_QAM_4,
	GS_MODULATOR_QAM_4NR,
	GS_MODULATOR_QAM_INVALID,
}GS_ModulatorQamMode;


typedef enum
{
	GS_MODULATOR_ANALOG_BAND_8M = 0,
	GS_MODULATOR_ANALOG_BAND_6M,
	GS_MODULATOR_ANALOG_BAND_INVLAID
}GS_ModulatorAnalogBand;

typedef enum
{
	GS_MODULATOR_FEC_ENCODE_1_2 = 0,
	GS_MODULATOR_FEC_ENCODE_2_3,
	GS_MODULATOR_FEC_ENCODE_3_4,
	GS_MODULATOR_FEC_ENCODE_5_6,
	GS_MODULATOR_FEC_ENCODE_7_8,

	GS_MODULATOR_FEC_ENCODE_INVLAID
}GS_ModulatorFec;


typedef enum
{
	GS_MODULATOR_CARRIER_MODE_1 = 0,
	GS_MODULATOR_CARRIER_MODE_3780,
}GS_ModulatorlCarrierMode;

typedef enum
{
	GS_MODULATOR_GUARD_INTERVAL_PN_420C = 0,
	GS_MODULATOR_GUARD_INTERVAL_PN_595,
	GS_MODULATOR_GUARD_INTERVAL_PN_945C,
	GS_MODULATOR_GUARD_INTERVAL_PN_420F,
	GS_MODULATOR_GUARD_INTERVAL_PN_945F,
}GS_ModulatorGuardInterval;

typedef enum
{
	GS_MODULATOR_CODE_RATE_0_4 = 0,//DTMB
	GS_MODULATOR_CODE_RATE_0_6,
	GS_MODULATOR_CODE_RATE_0_8,
}GS_ModulatorCodeRate;


typedef enum
{
	GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240 = 0,
	GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_720,
}GS_ModulatorTimeInterleaver;

/////////////////////////////////////////////

typedef struct  
{
	/*ASI子通道参数*/
	U32		m_Researved;
}MULT_SubASIInfo;


typedef struct  
{
	
	U32		m_E3DS3;
	U32		m_Bitorder;
	U32		m_Frameform;
	U32		m_Packetlength;
	U32		m_Scramble;
	U32		m_RSCoding;
	U32		m_InterweaveCoding;
	U32		m_RateRecover;
}MULT_SubESDS3Info;


typedef struct  
{

	U32		m_InputFreq;//单位KHz
	U32		m_LocalFreq;
	U32		m_SymbolRate;//单位KBaud
	U32		m_PolarMethod;
	U32		m_22kSwitch;
	U32		m_Modulation;
	U32		m_Reqtype;
	U32		m_SignalStrength;
	U32		m_SignalQuality;
	U32		m_LockStatus;
	U8		m_Specinv;
}MULT_SubTUNERInfo;

#ifdef GM8358Q
#define EncoderCvbsBoardSupportEntryNum 4

typedef struct  
{
	//U32 m_EncoderInputHardwareIndex;//实际硬件通道号 1~8
	BOOL m_WorkMod;
	S32 m_VideoFormat; 
	S32 m_FrameRate;
	S32 m_Resolution;
	S32 m_OutBitRate;
	U8 m_VideoEncodeMode;
	S32 m_VideoProfile;
	S32 m_VideoAspect;
	S32 m_VideoBitRate;
	U8 m_AudioEncodeMode;
	U8 m_AudioBitRate;
	S32 m_ImageHorizontalOffset;
	S32 m_VideoPid;
	S32 m_AudioPid;
	S32 m_PcrPid;
	U32 m_PmtPid;
	S32 m_Brightness;
	S32 m_Contrast;
	U32 m_Saturation;
	S32 m_Hue;
	S16 m_Volume;
	U8 m_AudioEmbChannel;
	S32 m_AudioSampleRate;
	BOOL m_PidEqualSwitch;

}MULT_SubENCODERInfoEntry;

typedef struct  
{
	//U32 m_EncoderInputHardwareIndex;//实际硬件通道号 1~8
	MULT_SubENCODERInfoEntry SubENCODERInfoEntry[4];

}MULT_SubENCODERInfo;

typedef enum
{
	ENCODER_WORK_MODE_FREE =0,
	ENCODER_WORK_MODE_ENCODER,
	ENCODER_WORK_MODE_NUM
}EncoderWorkMode;
S32 MULTL_XMLEncoderWorkModeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderWorkModeValueToStr(S32 Value);

typedef enum
{
	ENCODER_VIDEO_FORMAT_NTSC1 = 1, //NTSC(M/J)
	ENCODER_VIDEO_FORMAT_PAL1,		//PAL(D/K/B/G/H/I/N)
	ENCODER_VIDEO_FORMAT_PAL2,		//PAL(M)
	ENCODER_VIDEO_FORMAT_PAL3,		//PAL(COMBINATION-N)
	ENCODER_VIDEO_FORMAT_NTSC2,		//NTSC4.43
	ENCODER_VIDEO_FORMAT_SECAM,		//SECAM
	ENCODER_VIDEO_FORMAT_PAL4,		//PAL60
	ENCODER_VIDEO_FORMAT_NUM
}EncoderVideoFormat;
S32 MULTL_XMLEncoderVideoFormatValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoFormatValueToStr(S32 Value);

#define ENCODER_VIDEO_RESOLUTION_352_576 HWL_VR_352_576I 
#define ENCODER_VIDEO_RESOLUTION_544_576 HWL_VR_544_576I
#define ENCODER_VIDEO_RESOLUTION_640_576 HWL_VR_640_576I
#define ENCODER_VIDEO_RESOLUTION_704_576 HWL_VR_704_576I
#define ENCODER_VIDEO_RESOLUTION_720_576 HWL_VR_720_576I
#define ENCODER_VIDEO_RESOLUTION_544_480 HWL_VR_544_480I
#define ENCODER_VIDEO_RESOLUTION_640_480 HWL_VR_640_480I
#define ENCODER_VIDEO_RESOLUTION_704_480 HWL_VR_704_480I
#define ENCODER_VIDEO_RESOLUTION_720_480 HWL_VR_720_480I
#define ENCODER_VIDEO_RESOLUTION_352_480 HWL_VR_352_480I


S32 MULTL_XMLEncoderVideoResolutionValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoResolutionValueToStr(S32 Value);

typedef enum
{
	ENCODER_FRAME_RATE_1 = HWL_FRAME_FREQ_2997,	//29.97
	ENCODER_FRAME_RATE_2 = HWL_FRAME_FREQ_25,		//25
	ENCODER_FRAME_RATE_NUM
}EncoderFrameRate;

S32 MULTL_XMLEncoderFrameRateValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderFrameRateValueToStr(S32 Value);

typedef enum
{
	ENCODER_EncoderOutBitRrateMode_VBR = 0,	
	ENCODER_EncoderOutBitRrateMode_CBR,	
	ENCODER_EncoderOutBitRrateMode_NUM
}EncoderOutBitRrate;
S32 MULTL_XMLEncoderOutBitRrateModeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderOutBitRrateModeValueToStr(S32 Value);

typedef enum
{
	ENCODER_VideoEncodeMode_H264 = 0,	
	ENCODER_VideoEncodeMode_MPEG2,	
	ENCODER_VideoEncodeMode_NUM
}EncoderVideoEncodeMode;
S32 MULTL_XMLEncoderVideoEncodeModeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoEncodeModeValueToStr(S32 Value);

typedef enum
{
	ENCODER_VideoProfileMode_High = 0,	
	ENCODER_VideoProfileMode_Main = 1,	
	ENCODER_VideoProfileMode_NUM
}EncoderVideoProfileMode;
S32 MULTL_XMLEncoderVideoProfileValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoProfileValueToStr(S32 Value);

typedef enum
{
	ENCODER_VideoAspect1 = 0,	//16:9
	ENCODER_VideoAspect2,		//1:1
	ENCODER_VideoAspect3,		//4:3
	ENCODER_VideoAspect_NUM
}EncoderVideoAspect;
S32 MULTL_XMLEncoderVideoAspectValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoAspectValueToStr(S32 Value);


typedef enum
{
	ENCODER_AudioEncodeMode1 = 0,	//MPEG-1 L2
	ENCODER_AudioEncodeMode2,		//LC-AAC
	ENCODER_AudioEncodeMode_NUM
}EncoderAudioEncodeMode;
S32 MULTL_XMLEncoderAudioEncodeModeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderAudioEncodeModeValueToStr(S32 Value);

typedef enum
{
	ENCODER_AudioBitRate_96 = HWL_AUDIO_BITRATE_96K,
	ENCODER_AudioBitRate_112,
	ENCODER_AudioBitRate_128,
	ENCODER_AudioBitRate_160,
	ENCODER_AudioBitRate_192,	
	ENCODER_AudioBitRate_224,		
	ENCODER_AudioBitRate_256,
	ENCODER_AudioBitRate_320,		
	ENCODER_AudioBitRate_384,
	ENCODER_AudioBitRate_NUM
}EncoderAudioBitRate;
S32 MULTL_XMLEncoderAudioBitRateValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderAudioBitRateValueToStr(S32 Value);

typedef enum
{
	ENCODER_AudioSampleRate1 = 0,	//32
	ENCODER_AudioSampleRate2,		//44.1
	ENCODER_AudioSampleRate3,		//48
	ENCODER_AudioSampleRate_NUM
}EncoderAudioSampleRate;
S32 MULTL_XMLEncoderAudioSampleRateFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderAudioSampleRateToStr(S32 Value);

typedef enum
{
	ENCODER_PID_EQUAL_SWITCH_ON = 0,
	ENCODER_PID_EQUAL_SWITCH_OFF,
	ENCODER_PID_EQUAL_SWITCH_NUM
}EncoderPidEqualSwitch;
S32 MULTL_XMLEncoderPidEqualSwitchValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderPidEqualSwitchValueToStr(S32 Value);

#endif

#ifdef GM8398Q
#define EncoderCvbsBoardSupportEntryNum 4
#define EncoderDx8243SupportChannelEntryNum 2

typedef struct  
{
	//U32 m_EncoderInputHardwareIndex;//实际硬件通道号 1~8
	BOOL m_WorkMod;
	S32 m_VideoFormat; 
	S32 m_FrameRate;
	S32 m_Resolution;
	S32 m_OutBitRate;
	U8 m_VideoEncodeMode;
	S32 m_VideoProfile;
	S32 m_VideoAspect;
	S32 m_VideoBitRate;
	U8 m_AudioEncodeMode;
	U8 m_AudioBitRate;
	S32 m_ImageHorizontalOffset;
	S32 m_VideoPid;
	S32 m_AudioPid;
	S32 m_PcrPid;
	U32 m_PmtPid;
	S32 m_Brightness;
	S32 m_Contrast;
	U32 m_Saturation;
	S32 m_Hue;
	S16 m_Volume;
	U8 m_AudioEmbChannel;
	S32 m_AudioSampleRate;
	BOOL m_PidEqualSwitch;

}MULT_SubENCODERInfoEntry;

typedef struct  
{
	//U32 m_EncoderInputHardwareIndex;//实际硬件通道号 1~8
	MULT_SubENCODERInfoEntry SubENCODERInfoEntry[4];

}MULT_SubENCODERInfo;

//Working Mode 
typedef enum
{
	ENCODER_WORK_MODE_FREE =0,
	ENCODER_WORK_MODE_ENCODER,
	ENCODER_WORK_MODE_NUM
}EncoderWorkMode;
S32 MULTL_XMLEncoderWorkModeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderWorkModeValueToStr(S32 Value);

typedef enum
{
	ENCODER_VIDEO_FORMAT_NTSC1 = 1, //NTSC(M/J)
	ENCODER_VIDEO_FORMAT_PAL1,		//PAL(D/K/B/G/H/I/N)
	ENCODER_VIDEO_FORMAT_PAL2,		//PAL(M)
	ENCODER_VIDEO_FORMAT_PAL3,		//PAL(COMBINATION-N)
	ENCODER_VIDEO_FORMAT_NTSC2,		//NTSC4.43
	ENCODER_VIDEO_FORMAT_SECAM,		//SECAM
	ENCODER_VIDEO_FORMAT_PAL4,		//PAL60
	ENCODER_VIDEO_FORMAT_NUM
}EncoderVideoFormat;
S32 MULTL_XMLEncoderVideoFormatValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoFormatValueToStr(S32 Value);

//ifdef GM8358Q 
#if 0

	#define ENCODER_VIDEO_RESOLUTION_352_576 HWL_VR_352_576I 
	#define ENCODER_VIDEO_RESOLUTION_544_576 HWL_VR_544_576I
	#define ENCODER_VIDEO_RESOLUTION_640_576 HWL_VR_640_576I
	#define ENCODER_VIDEO_RESOLUTION_704_576 HWL_VR_704_576I
	#define ENCODER_VIDEO_RESOLUTION_720_576 HWL_VR_720_576I
	#define ENCODER_VIDEO_RESOLUTION_544_480 HWL_VR_544_480I
	#define ENCODER_VIDEO_RESOLUTION_640_480 HWL_VR_640_480I
	#define ENCODER_VIDEO_RESOLUTION_704_480 HWL_VR_704_480I
	#define ENCODER_VIDEO_RESOLUTION_720_480 HWL_VR_720_480I
	#define ENCODER_VIDEO_RESOLUTION_352_480 HWL_VR_352_480I

#else

typedef enum
{
	ENCODER_VIDEO_RESOLUTION_352_576 = 0,
	ENCODER_VIDEO_RESOLUTION_544_576, 
	ENCODER_VIDEO_RESOLUTION_640_576, 
	ENCODER_VIDEO_RESOLUTION_704_576, 
	ENCODER_VIDEO_RESOLUTION_720_576, 
	ENCODER_VIDEO_RESOLUTION_544_480, 
	ENCODER_VIDEO_RESOLUTION_640_480, 
	ENCODER_VIDEO_RESOLUTION_704_480, 
	ENCODER_VIDEO_RESOLUTION_720_480, 
	ENCODER_VIDEO_RESOLUTION_352_480, 
	ENCODER_VIDEO_RESOLUTION_NUM
}EncoderVideoResolution;

#endif


S32 MULTL_XMLEncoderVideoResolutionValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoResolutionValueToStr(S32 Value);

typedef enum
{
	ENCODER_EncoderOutBitRrateMode_VBR = 0,	
	ENCODER_EncoderOutBitRrateMode_CBR,	
	ENCODER_EncoderOutBitRrateMode_NUM
}EncoderOutBitRrate;
S32 MULTL_XMLEncoderOutBitRrateModeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderOutBitRrateModeValueToStr(S32 Value);

typedef enum
{
	ENCODER_VideoEncodeMode_H264 = 0,	
	ENCODER_VideoEncodeMode_MPEG2,	
	ENCODER_VideoEncodeMode_NUM
}EncoderVideoEncodeMode;
S32 MULTL_XMLEncoderVideoEncodeModeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoEncodeModeValueToStr(S32 Value);

typedef enum
{
	ENCODER_VideoProfileMode_High = 0,	
	ENCODER_VideoProfileMode_Main = 1,	
	ENCODER_VideoProfileMode_NUM
}EncoderVideoProfileMode;
S32 MULTL_XMLEncoderVideoProfileValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoProfileValueToStr(S32 Value);

typedef enum
{
	ENCODER_VideoAspect1 = 0,	//16:9
	ENCODER_VideoAspect2,		//1:1
	ENCODER_VideoAspect3,		//4:3
	ENCODER_VideoAspect_NUM
}EncoderVideoAspect;
S32 MULTL_XMLEncoderVideoAspectValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderVideoAspectValueToStr(S32 Value);


typedef enum
{
	ENCODER_AudioEncodeMode1 = 0,	//MPEG-1 L2
	ENCODER_AudioEncodeMode2,		//LC-AAC
	ENCODER_AudioEncodeMode3,		//HE-ACC V1
	ENCODER_AudioEncodeMode4,		//HE-ACC V2
	ENCODER_AudioEncodeMode_NUM
}EncoderAudioEncodeMode;
S32 MULTL_XMLEncoderAudioEncodeModeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderAudioEncodeModeValueToStr(S32 Value);

typedef enum
{
	ENCODER_AudioBitRate_96 = HWL_AUDIO_BITRATE_96K,
	ENCODER_AudioBitRate_112,
	ENCODER_AudioBitRate_128,
	ENCODER_AudioBitRate_160,
	ENCODER_AudioBitRate_192,	
	ENCODER_AudioBitRate_224,		
	ENCODER_AudioBitRate_256,
	ENCODER_AudioBitRate_320,		
	ENCODER_AudioBitRate_384,
	ENCODER_AudioBitRate_NUM
}EncoderAudioBitRate;
S32 MULTL_XMLEncoderAudioBitRateValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderAudioBitRateValueToStr(S32 Value);

typedef enum
{
	ENCODER_AudioSampleRate1 = 0,	//32
	ENCODER_AudioSampleRate2,		//44.1
	ENCODER_AudioSampleRate3,		//48
	ENCODER_AudioSampleRate_NUM
}EncoderAudioSampleRate;
S32 MULTL_XMLEncoderAudioSampleRateFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderAudioSampleRateToStr(S32 Value);

typedef enum
{
	ENCODER_PID_EQUAL_SWITCH_ON = 0,
	ENCODER_PID_EQUAL_SWITCH_OFF,
	ENCODER_PID_EQUAL_SWITCH_NUM
}EncoderPidEqualSwitch;
S32 MULTL_XMLEncoderPidEqualSwitchValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLEncoderPidEqualSwitchValueToStr(S32 Value);

#endif

#if defined(GN1846) || defined(GN1866)

typedef enum
{
	IP_OUTPUT_SPTS = 0,
	IP_OUTPUT_MPTS,
	IP_OUTPUT_TYPE_NUM
} IP_OUTPUT_TYPE;

typedef enum
{
	ENC_VI_MODE_PAL,
	ENC_VI_MODE_NTSC,
	ENC_VI_MODE_720P_50,
	ENC_VI_MODE_720P_5994,
	ENC_VI_MODE_720P_60,
	ENC_VI_MODE_1080I_50,
	ENC_VI_MODE_1080I_5994,
	ENC_VI_MODE_1080I_60,
	ENC_VI_MODE_1080P_50,
	ENC_VI_MODE_1080P_5994,
	ENC_VI_MODE_1080P_60,
	ENC_VI_MODE_AUTO,
	ENC_VI_MODE_NUM
} ENCODER_ViMode;

typedef enum
{
	ENC_VO_MODE_576P25,
	ENC_VO_MODE_480P2997,
	ENC_VO_MODE_720P25,
	ENC_VO_MODE_720P2997,
	ENC_VO_MODE_720P30,
	ENC_VO_MODE_720P50,
	ENC_VO_MODE_720P5994,
	ENC_VO_MODE_720P60,
	ENC_VO_MODE_1080P25,
	ENC_VO_MODE_1080P2997,
	ENC_VO_MODE_1080P30,
	ENC_VO_MODE_1080P50,
	ENC_VO_MODE_1080P5994,
	ENC_VO_MODE_1080P60,
	ENC_VO_MODE_AUTO,
	ENC_VO_MODE_NUM
} ENCODER_VoMode;

typedef enum
{
	ENC_BR_MODE_CBR = 0,
	ENC_BR_MODE_VBR,
	ENC_BR_MODE_NUM
} ENCODER_BrMode;

typedef enum
{
	ENC_MODE_H264,
	ENC_MODE_H265,
	ENC_MODE_NUM
} ENCODER_EncMode;

typedef enum
{
	ENC_PROFILE_BASE = 0,
	ENC_PROFILE_MP,
	ENC_PROFILE_HP,
	ENC_PROFILE_NUM
} ENCODER_Profile;

typedef enum
{
	ENC_AUD_ENC_MODE_MPEG1_L2 = 0,
	ENC_AUD_ENC_MODE_LC_AAC,
	ENC_AUD_ENC_MODE_AC3,
	ENC_AUD_ENC_MODE_EAC3,
	ENC_AUD_ENC_MODE_NUM
} ENCODER_AudEncMode;

typedef enum
{
	ENC_AUD_BR_16K = 0,
	ENC_AUD_BR_64K,
	ENC_AUD_BR_96K, 
	ENC_AUD_BR_112K,
	ENC_AUD_BR_128K,
	ENC_AUD_BR_160K,
	ENC_AUD_BR_192K,
	ENC_AUD_BR_224K,
	ENC_AUD_BR_256K,
	ENC_AUD_BR_320K,
	ENC_AUD_BR_384K,
	ENC_AUD_BR_NUM
} ENCODER_AudBitrate;

typedef enum
{
	ENC_AUD_SAMP_32K = 0, 
	ENC_AUD_SAMP_44_1K,
	ENC_AUD_SAMP_48K,
	ENC_AUD_SAMP_AUTO,
	ENC_AUD_SAMP_NUM
} ENCODER_AudSample;

typedef struct  
{
	ENCODER_ViMode		m_ViMode;
	ENCODER_VoMode		m_VoMode;
	ENCODER_BrMode		m_BrMode;
	ENCODER_EncMode		m_EncMode;
	ENCODER_Profile		m_Profile;
	S32					m_Bitrate;
	S32					m_ProgBitrate; /* 节目码率 */
	S32					m_Gop;
	ENCODER_AudEncMode	m_AudEncMode;
	ENCODER_AudBitrate	m_AudBitrate;
	ENCODER_AudSample	m_AudSample;
	S32					m_Volume;
	U8					m_pProgName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN];
	S32					m_ProgNumber;
	S32					m_VidPid;
	S32					m_AudPid;
	S32					m_PcrPid;
	S32					m_PmtPid;
	U32 				m_Protocol;
	U32					m_IPv4Addr;
	U32					m_IPv4Port;
	BOOL				m_ActiveMark;
}MULT_SubENCODERInfo;

S32 MULTL_XMLIpOutputTypeValueFromStr(CHAR_T *pStr); 
CHAR_T* MULTL_XMLIpOutputTypeValueToStr(S32 Value);
S32 MULTL_XMLEdidTypeValueFromStr(CHAR_T *pStr); 
CHAR_T* MULTL_XMLEdidTypeValueToStr(S32 Value);
S32 MULTL_XMLEncViModeValueFromStr(CHAR_T *pStr); 
CHAR_T* MULTL_XMLEncViModeValueToStr(S32 Value);
S32 MULTL_XMLEncVoModeValueFromStr(CHAR_T *pStr); 
CHAR_T* MULTL_XMLEncVoModeValueToStr(S32 Value);
S32 MULTL_XMLEncBrModeValueFromStr(CHAR_T *pStr);
CHAR_T* MULTL_XMLEncBrModeValueToStr(S32 Value);
S32 MULTL_XMLEncModeValueFromStr(CHAR_T *pStr); 
CHAR_T* MULTL_XMLEncModeValueToStr(S32 Value);
S32 MULTL_XMLEncProfileValueFromStr(CHAR_T *pStr); 
CHAR_T* MULTL_XMLEncProfileValueToStr(S32 Value);
S32 MULTL_XMLAudEncModeValueFromStr(CHAR_T *pStr);
CHAR_T* MULTL_XMLAudEncModeValueToStr(S32 Value);
S32 MULTL_XMLAudBitrateValueFromStr(CHAR_T *pStr);
CHAR_T* MULTL_XMLAudBitrateValueToStr(S32 Value);
S32 MULTL_XMLAudSampValueFromStr(CHAR_T *pStr);
CHAR_T* MULTL_XMLAudSampValueToStr(S32 Value);
#endif

typedef struct  
{
	/*IP子通道参数*/
	U32 	m_Protocol;
	U32		m_IPv4Addr;
	U32		m_IPv4Port;

	U32		m_IPv4LimitAddr;//输入通道源地址限定；IGMPv3限定（等价）；作为输出使用时，是IP欺骗功能
	U32		m_IPv4LimitPort;
	BOOL8	m_IPv4LimitMark;
}MULT_SubIPInfo;

typedef struct  
{
	/*调制器子通道参数*/
	U8		m_ITUCoding;
	U8		m_AnalogBand;
	U32		m_CenterFreq;	
	U32		m_SymbolRate;	
	U8		m_Mode;	
	BOOL8	m_SpectInv;
	U8		m_GainLevel;
	BOOL8	m_Modulation;

	U32		m_LevelAttenuation;	//电平衰减

	//for qpsk ---add by ding
	U32		m_FecEncode;		//fec 编码

	//DTMB
	S32		m_CarrierMode;//多载波，单载波
	S32		m_PNMode;
	S32		m_CodeRate;//0.4 0.6 0.8
	S32		m_TimeInterleave;//交织 2个模式
	BOOL	m_DoublePilot;

#ifdef SUPPORT_SFN_MODULATOR
	//SFN
	BOOL	m_RFMute;
#endif

	//RF开关放置在外侧
}MULT_SubModulatorInfo;

typedef struct
{
	union
	{
		MULT_SubASIInfo			m_SubASIInfo;
		MULT_SubIPInfo			m_SubIPInfo;
		MULT_SubModulatorInfo	m_SubModulatorInfo;	
		MULT_SubESDS3Info       m_SubESDS3Info;
		MULT_SubTUNERInfo		m_SubTUNERInfo;
#ifdef GM8358Q
		MULT_SubENCODERInfo		m_SubENCODERInfo;
#endif

#ifdef GM8398Q
		MULT_SubENCODERInfo		m_SubENCODERInfo;
#endif

#if defined(GN1846) || defined(GN1866)
		MULT_SubENCODERInfo		m_SubENCODERInfo;
#endif
	}
	m_SubInfo;
	S32		m_CorrespondTsIndex;//对应的逻辑TS序号。在启动时由具体平台指定。
	BOOL8	m_ActiveMark;//通道物理关闭属性，关闭后该子通道不接收也不输出数据，此时数据库中复用参数独立工作，对应TS的信息依然要输出到FPGA！
	U32		m_Bitrate;//单位MBps 2-99
}MULT_SubChannelNode;


typedef struct  
{
	/*ASI通道参数*/
	U32		m_Bitrate;//最大输出码率216M * N
}MULT_ASIInfo;

typedef struct  
{
	
	U32		m_Researbed;
}MULT_E3DS3Info;
typedef struct  
{
	
	U32		m_FreqLimitsLow;
	U32		m_FreqLimitsHigh;
	U32		m_SymRateLimitsLow;
	U32		m_SymRateLimitsHigh;
}MULT_TUNERInfo;

#ifdef GM8358Q

typedef struct  
{

	//U32		m_FreqLimitsLow;
	//U32		m_FreqLimitsHigh;
	//U32		m_SymRateLimitsLow;
	//U32		m_SymRateLimitsHigh;
	;
}MULT_ENCODERInfo;

#endif

#ifdef GM8398Q

typedef struct  
{

	//U32		m_FreqLimitsLow;
	//U32		m_FreqLimitsHigh;
	//U32		m_SymRateLimitsLow;
	//U32		m_SymRateLimitsHigh;
	;
}MULT_ENCODERInfo;

#endif

#if defined(GN1846) || defined(GN1866)
typedef struct  
{
	U32	m_VidOutBitrateLimitLow;
	U32	m_VidOutBitrateLimitHigh;
	U32 m_ProgBitrateLimitHigh;
	U32	m_GopLimitLow;
	U32 m_GopLimitHigh;
	S32	m_VolLimitLow;
	S32 m_VolLimitHigh;
	U32 m_ProgNumberLimitLow;
	U32 m_ProgNumberLimitHigh;
	U32 m_ProgNameLimitHigh; /* 节目名长度限制 */
	U32	m_PidLimitLow;
	U32	m_PidLimitHigh;
}MULT_ENCODERInfo;
#endif

typedef struct  
{
	/*IP通道参数*/
	U32		m_IPAddress;
	U32		m_IPMask;
	U32		m_IPGate;

	U8		m_MAC[GLOBAL_MAC_BUF_SIZE];

	U32		m_Bitrate;//最大输出码率5-850

	BOOL	m_IPMark;
}MULT_IPInfo;


typedef struct  
{
	/*调制器通道参数*/
	S32		m_AdjacentFreqNumber;
	U32		m_CenterFrequenceLimitsLow;
	U32		m_CenterFrequenceLimitsHigh;
	S32		m_SymbolRateLimitsLow;
	S32		m_SymbolRateLimitsHigh;
	S32		m_ExAttValidMark;
	F64		m_ExAttStepping;
	S32		m_ExAttLevel;
	S32		m_ExAttLevelMax;
	F64		m_GainStepping;
	S32		m_GainLevelMax;
	BOOL	m_DPDMark;
	BOOL	m_ALCMark;
	S32		m_FreqAdj;
}MULT_ModulatorInfo;

typedef struct
{
	/*通用参数*/
	S32		m_OriginalIndex;
	S32		m_ChannelType;
	S32		m_SubType;
	S32		m_DemodType;

// 	U32		m_MaxBitrate;
// 	U32		m_MinBitrate;

	union
	{
		MULT_ModulatorInfo	m_ModulatorInfo;
		MULT_IPInfo			m_IPInfo;
		MULT_ASIInfo		m_ASI;
		MULT_E3DS3Info      m_E3DS3;
		MULT_TUNERInfo		m_TUNER;

#ifdef GM8358Q

		MULT_ENCODERInfo	m_ENCODER;

#endif

#ifdef GM8398Q

		MULT_ENCODERInfo	m_ENCODER;

#endif

#if defined(GN1846) || defined(GN1866)
		MULT_ENCODERInfo	m_ENCODER;
#endif

	}m_ChannelInfo;

	MULT_SubChannelNode *m_pSubChannelNode;
	S32		m_SubChannelNumber;
}MULT_ChannelNode;

typedef struct
{
	S16					m_IGMPRepeateTime;//20~255
	BOOL				m_IGMPRepeateMark;
	S16					m_IGMPVersion;//2或3
}MULT_IGMP;


#ifdef MULT_SYSTEM_HAVE_PCR_CORRECT_ADJUST_FUNCTION
#define MULT_PCR_CORRECT_POS_DEFAULT_VALUE	7
#define MULT_PCR_CORRECT_NEG_DEFAULT_VALUE	2
typedef struct
{
	BOOL				m_PCRCMark;
	S32					m_PCRCPos;
	S32					m_PCRCNeg;
}MULT_PCRCorrect;
#endif


typedef struct
{
	MULT_ChannelNode	*m_pInChannel;
	S32					m_InChannelNumber;
	MULT_ChannelNode	*m_pOutChannel;
	S32					m_OutChannelNumber;

	S32					m_MaxInTsNumber;
	S32					m_MaxOutTsNumber;

	S32					m_TDTUpdateInterval;/*TDT时间更新间隔*/

	MULT_IGMP			m_IGMP;

#ifdef MULT_SYSTEM_HAVE_PCR_CORRECT_ADJUST_FUNCTION
	MULT_PCRCorrect		m_PCRCorrect;
#endif

}MULT_Parameter;



/*监控参数/状态----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define MULT_MONITOR_ENCODE_ID(Slot) ((Slot) + 1)
#define MULT_MONITOR_DECODE_ID(ID) ((ID) - 1)

typedef enum 
{
	MULT_MONITOR_TYPE_TEMP = 0,
	MULT_MONITOR_TYPE_FPGA,
	MULT_MONITOR_TYPE_NTP,
	MULT_MONITOR_TYPE_SCS_EMM,
	MULT_MONITOR_TYPE_SCS_ECM,
	MULT_MONITOR_TYPE_ETH_LINK,
	MULT_MONITOR_TYPE_PLL,
	MULT_MONITOR_TYPE_CHANNEL_IN,//通道相关报警：码率溢出、低于或高于用户设定值等。
	MULT_MONITOR_TYPE_CHANNEL_OUT,//通道相关报警：码率溢出、低于或高于用户设定值等。
	MULT_MONITOR_TYPE_BUFFER_STATUS,
	MULT_MONITOR_TYPE_FAN_FAILURE,
	MULT_MONITOR_TYPE_POWER_FAILURE,
	MULT_MONITOR_TYPE_INPUT_TS_CHANGED,
	MULT_MONITOR_TYPE_TUNER_SHORT_STATUS,

	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN,
	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO,
	MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_NOBITRATE,
	MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_OVERFLOW,


	MULT_MONITOR_TYPE_GPS_LOCK_LOST,//GPS模块提供的未锁定信息
	MULT_MONITOR_TYPE_EXT_10M_LOST,//FPGA提供的外部10M丢失信息
	MULT_MONITOR_TYPE_EXT_1PPS_LOST,//FPGA提供的外部1PPS错误信息
	MULT_MONITOR_TYPE_GPS_ERROR,//GPS模块无法接收到串口数据
	MULT_MONITOR_TYPE_SFN_SIP_ERROR,//SFN模式下没有收到SIP包
	MULT_MONITOR_TYPE_INT_10M_LOST,//FPGA提供的内部10M丢失信息
	MULT_MONITOR_TYPE_INT_1PPS_LOST,//FPGA提供的内部1PPS错误信息
	MULT_MONITOR_TYPE_SFN_SIP_CRC32_ERROR,//卫星SFN模式下收到的SIP包CRC32错误
    MULT_MONITOR_TYPE_SFN_ERROR,//SFN模式各种错误导致单频网失效
	MULT_MONITOR_TYPE_DPD_PARAMETER_SAVE_ERROR,//DPD参数保存错误
	MULT_MONITOR_TYPE_CLK_SYNC_ERROR,//CLK模块，时钟校正过程中出现的错误提示
	MULT_MONITOR_TYPE_SFN_SIP_CHANGE,//SIP 变化报警 //27

#ifdef GN1846
	MULT_MONITOR_TYPE_ENCODER_BUFFER_OVERFLOW_ERROR, /* 编码缓冲器溢出 */
	MULT_MONITOR_TYPE_HDMI_INPUT_LOST_ERROR, /* HDMI 输入信号丢失 */
	MULT_MONITOR_TYPE_HDMI_INPUT_FORMAT_ERROR, /* HDMI 输入信号格式错误 */
#endif

#ifdef GM8398Q
	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN1,
	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN2,
	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN3,
	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN4,
	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN5,
	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN6,
	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN7,
	MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN8,

	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO1,
	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO2,
	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO3,
	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO4,
	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO5,
	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO6,
	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO7,
	MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO8,

	MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_NOBITRATE,
	MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_OVERFLOW,

#endif
	MULT_MONITOR_TYPE_NUM 
}MULT_MonitorLogSlot;

typedef struct  
{
	S32		m_LowLimit;
	S32		m_HighLimit;
	U8		m_AlarmCount;
	BOOL	m_Mark;
}MULT_MonitorLimitInfo;

typedef struct  
{
	MULT_MonitorLimitInfo	m_LimitInfo;
	S32						m_CurValue;
// 	BOOL					m_Mark;//有效开关，仅开启了的子通道才有效和显示

#if defined(GN1846) | defined(GN1866)
	U32						m_HdmiSignalLostCount; /* 信号丢失计数，使用该参数防止误报信号丢失 */			
#endif
}MULT_MonitorSUB;


typedef struct  
{
	MULT_MonitorLimitInfo	m_LimitInfo;
	S32						m_CurValue;
	MULT_MonitorSUB			*m_pSubArray;
	S32						m_SubNumber;
	S32						m_StartTsIndex;
}MULT_MonitorCHN;


typedef struct  
{
	/*状态信息*/
	BOOL8			m_GlobalTempMark;//报警系统全局临时开关，在参数设置时可能需要临时关闭报警日志系统

	/*当前参数状态*/
	BOOL8			m_bApplyMark;//为TRUE时，表明参数设置后没有应用
	BOOL8			m_bSaveMark;//为TRUE时，表明参数设置后没有保存

	/*监控信息和配置*/
	BOOL8			m_GlobalMark;//报警系统全局开关
	U8				m_CriticalTemp;//温度报警门限（上限）
	U8				m_FanTemp;//风扇启动的温度门限（上限）
	S32				m_TempErrorCount;

	S32				m_CurrentTemp;//当前温度
	BOOL8			m_FanStatus;//当前风扇开启状态！动态！
	BOOL8			m_FPGAStatus;//当前FPGA心跳状态！
	BOOL8			m_PowerStatus;

	S32				m_EthStatus[4];
	S32				m_LastEthStatus[4];

	BOOL8			m_NTPStatus;//仅当NTP开启时
	BOOL8			m_NTPWorkingStatus;//NTP异步操作状态
	S32				m_NTPSyncTimeout;//NTP同步超时
// 	BOOL8			m_InputLink;//仅限IP等综合通道
// 	BOOL8			m_OutputLink;//仅限IP等综合通道

	/*报警日志信息设计：关键在于把通道报警当做一个(两个，输入输出分开)对象来处理*/
	HANDLE32		m_LogHandle;

	MULT_MonitorCHN *m_pInChnArray;//通道监控信息
	S32				m_InChnNum;
	U32				m_TotalInBitrate;

	MULT_MonitorCHN *m_pOutChnArray;//通道监控信息
	S32				m_OutChnNum;
	U32				m_TotalOutBitrate;

	U32				m_InserterBitrate;

	S32				m_CPUUsage;//0~100 CPU暂用率0%~100%
	S32				m_MEMUsage;//0~100 Memory暂用率0%~100%

	S32				m_ETHNumber;

	BOOL8			m_BusyMark;
	BOOL8			m_ApplyParametersBusyMark;


	CHAR_T			m_pTimeDataBuf[64];

	U32				m_CurrentPLLValue;
	S32				m_PLLErrorCount;
	U8				m_ShortCount[4];

	S32				m_FlowCount[MULT_DEVICE_MODUATION_NUM];
	F64				m_CurDuration;


#ifdef SUPPORT_NEW_HWL_MODULE
	HWL_MonitorPIDStatArray	m_PIDArray;
#else
	HWL_PIDStatisticsArray	m_PIDArray;
	S32						m_PIDStatisticsChnIndex;//对应原始通道号，即不分输出输出类型的
	S16						m_PIDStatisticsSubIndex;
#endif



	HWL_IPStatisticsArray	m_IPArray;
	S32						m_IPStatisticsChnIndex;//对应原始通道号，即不分输出输出类型的

#ifdef GN1846
	HDMI_RxStatusParam		m_HdmiRxStatus[MULT_MAX_CHN_NUM];
#endif

}MULT_Monitor;

/* 同密加扰 --------------------------------------------------------------------------------------------------------------------------------------- */
#define MULT_SCS_MAX_SERVICE_NUM_PER_TS				64
#define MULT_SCS_MAX_EMM_BITRATE					(1024)//Kbps
#define MULT_SCS_MAX_CRYPTO_CHN						128


#define BSS_SESSION_WORD_LENGTH						16
#define BSS_CONTROL_WORD_LENGTH						8
#define BSS_USERID_LENGTH							16

typedef struct  
{
	U32	m_ISRID;
	U16	m_PID;
	S16 m_ECMInterval;
	BOOL m_OutputMark;
}MULT_SCSECMInfo;



typedef struct  
{
	MULT_SCSECMInfo	m_pInfo[MPEG2_DB_SIMULCRYPT_CA_MAX_NUM];
	S16 m_OutTsIndex;
	U32 m_CryptoStreamID;
}MULT_SCSECMSlot;

typedef struct  
{
	U16	m_PID;
	BOOL m_OutputMark;
}MULT_SCSEMMInfo;

typedef struct  
{
	MULT_SCSEMMInfo	m_pInfo[MPEG2_DB_SIMULCRYPT_CA_MAX_NUM];
	S32	m_OutTsIndex;
}MULT_SCSEMMSlot;


typedef struct  
{
	S32	m_SuperCASID;
	S32	m_EMMActualBandwidth;
	S32	m_EMMCurBitrate;
	S32	m_EMMLinkStatus;
	S32	m_ECMLinkStatus;
	BOOL m_bActiveMark;
}MULT_SCSSlot;


typedef struct  
{
	HANDLE32 m_SCSHandle;
	HANDLE32 m_ThreadHandle;
	BOOL	 m_ThreadMark;

	MULT_SCSSlot m_SCSSlot[MPEG2_DB_SIMULCRYPT_CA_MAX_NUM];
	S32		m_SCSCount;

	MULT_SCSECMSlot *m_pECMSlotArray;
	S32		m_ECMMaxNum;
	S32		m_ECMCurNum;

	MULT_SCSEMMSlot *m_pEMMSlotArray;
	S32		m_EMMMaxNum;/*基本等于输出TS的数量*/

	BOOL	m_ScrambleStatus;//加扰状态
	S32		m_CurrentCPDuration;//当前实际加扰周期
	S32		m_CurrentComputeTime;//当前实际计算实际
	U16		m_CurrentCPNumber;//当前实际周期

	CHAR_T		m_pECMTimeDataBuf[64];
}MULT_SCS;


/*BSS*/
typedef struct
{	
	U8	m_pSW[BSS_SESSION_WORD_LENGTH];
	U8	m_pKey[BSS_SESSION_WORD_LENGTH];
	U8	m_ActiveMark;
	U32 m_SuperCASID;
}MULT_BISS;


#ifdef GN1846
#define EDID_CHIP_NUM (4)
typedef struct
{	
	S32	m_EdidType[EDID_CHIP_NUM];
}MULTL_Edid;
#endif


/*系统句柄----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define MULT_MAX_XML_STRING_SIZE	(1*1024*1024)

typedef struct
{
	MULT_Maintenace		m_MaintSetting;//隐藏参数等信息,注意在恢复出厂设置的时候需要保留！！！！！！！
	MULT_Information	m_Information;//设备OEM、授权H信息；每次启动时动态生成！
	MULT_Config			m_Configuration;//设备设定信息 IPV4 NTP，不可导入导出，仅在出厂设置时恢复。
	MULT_Parameter		m_Parameter;//设备参数，可导入导出。在恢复默认参数和出厂设置时均恢复。
	MULT_Monitor		m_Monitor;//设备监控参数，可导入导出。在恢复默认参数和出厂设置时均恢复。其中包含运行时动态修改的部分；同时也是监控和底部状态页面的信息来源！
#ifdef GN1846
	MULTL_Edid			m_EdidInfo;
#endif
	U32					m_MonitoerTimerID;
	CHAR_T				m_pReplayXMLBuf[MULT_MAX_XML_STRING_SIZE];

#ifdef SUPPORT_SYSLOG_MODULE
	MULT_Syslog			m_SyslogParam;
#endif

	HANDLE32	m_MainMsgHandle;//主消息队列句柄

	HANDLE32	m_DBSHandle;//DVB数据库模块句柄
	HANDLE32	m_DBSLock;

	MULT_SCS	m_MultSCS;

	MULT_BISS	m_BSSystemInfo;

	HANDLE32	m_AUTHHandle;//授权控制模块句柄

	HANDLE32	m_UPGHandle;//升级OEM模块句柄

	U8			m_TaskMark;

	U32			m_RetCode;

	S16					m_BatchCurTsIndex;//批量搜索时的当前TS号
	S16					m_BatchMaxTsNumber;
	U8					m_BatchCharset;
	S32					m_BatchTimeout;
	BOOL				m_BatchRemoveService;
	BOOL				m_bBatchAnalyzeActive;
	S32					m_TunerCount;
	S32					m_TunerType;

	/*特殊变量*/
	U32					m_bHaveShortTest;
	U8					m_TunerSPolar[4];

	S32					m_MonitorPauseTimeout;
}MULT_Handle;



typedef struct
{
	U8		m_MsgType;
	U8		m_MsgParam;
	U16		m_MsgParamResearved;
	S32		m_MsglValue;
	U32		m_MsgwValue;
}MULT_MainMsg;

typedef enum  
{
	MULTI_MAIN_MSG_TYPE_PSI_ARRIVE = 0,
	MULTI_MAIN_MSG_TYPE_INIPUT_ANALYZE_COMPLETE,
	MULTI_MAIN_MSG_TYPE_WEB_XML_CMD,
	MULTI_MAIN_MSG_TYPE_MONITOR_TIMER,
	MULTI_MAIN_MSG_TYPE_COUNT
}MULT_MainMsgType;


typedef struct  
{
	U32	m_ORIIDs;
	U32	m_NewIDs;
}MULT_IDs;


typedef struct  
{
	MULT_IDs	*m_pIDArray;
	S32			m_CurNum;
	S32			m_MaxNum;
}MULT_IDsArray;


/* Functions prototypes ------------------------------------------------------- */
S16 MULTL_DBActionCB(void* pUserParam, S32 DBActionType, U8* pBuf,S16 Datalen);


void MULTL_WEBXMLPostProcess(MULT_Handle *pHandle, CHAR_T* pXMLStr, S32 StrBufSize);
void MULTL_WEBXMLGetProcess(MULT_Handle *pHandle, CHAR_T* pXMLStr, S32 StrBufSize);

void MULTL_ApplyRemuxParameter(MULT_Handle *pHandle);
void MULTL_ApplyDirectRouteMode(MULT_Handle *pHandle);
void MULTL_ApplyAllParamter(MULT_Handle *pHandle);
void MULTL_SetRemuxApplyMark(MULT_Handle *pHandle, BOOL bClear);
void MULTL_SetSaveMark(MULT_Handle *pHandle, BOOL bClear);
void MULTL_MonitorProcess(MULT_Handle *pHandle, S32 Duration);

void MULTL_SaveParamterToStorage(MULT_Handle *pHandle);
BOOL MULTL_LoadParamterFromStorage(MULT_Handle *pHandle, CHAR_T *pFilePathName);
void MULTL_GenerateParamters(MULT_Handle *pHandle, CHAR_T *pFilePathName, CHAR_T* pDescription);
BOOL MULTL_ValidationParameter(MULT_Handle *pHandle, CHAR_T* pFilePathName, CHAR_T *pDescBuf, S32 BufSize);
void MULTL_RebootSequence(MULT_Handle *pHandle);


CHAR_T *MULTL_XMLGetNodeText(mxml_node_t *pParent, CHAR_T* pTagname);
S32 MULTL_XMLGetNodeINT(mxml_node_t *pParent, CHAR_T* pTagname, S32 Radix);
S32 MULTL_XMLGetNodeINTDefault(mxml_node_t *pParent, CHAR_T* pTagname, S32 Radix, S32 Default);
U32 MULTL_XMLGetNodeUINT(mxml_node_t *pParent, CHAR_T* pTagname, S32 Radix);
U32 MULTL_XMLGetNodeUINTDefault(mxml_node_t *pParent, CHAR_T* pTagname, S32 Radix, U32 Default);
F64 MULTL_XMLGetNodeFLOATDefault(mxml_node_t *pParent, CHAR_T* pTagname, F64 Default);
BOOL MULTL_XMLGetNodeBOOL(mxml_node_t *pParent, CHAR_T* pTagname);
BOOL MULTL_XMLGetNodeMark(mxml_node_t *pParent, CHAR_T* pTagname);
BOOL MULTL_XMLGetNodeMarkDefault(mxml_node_t *pParent, CHAR_T* pTagname, BOOL Default);


void MULTL_XMLAddNodeCDData(mxml_node_t *pParent, CHAR_T* pTagname, CHAR_T *pValue);
void MULTL_XMLAddNodeText(mxml_node_t *pParent, CHAR_T* pTagname, CHAR_T *pValue);
void MULTL_XMLAddNodeINT(mxml_node_t *pParent, CHAR_T* pTagname, S32 Value);
void MULTL_XMLAddNodeUINT(mxml_node_t *pParent, CHAR_T* pTagname, U32 Value);
void MULTL_XMLAddNodeFLOAT(mxml_node_t *pParent, CHAR_T* pTagname, F64 Value);
void MULTL_XMLAddNodeFLOATE(mxml_node_t *pParent, CHAR_T* pTagname, F64 Value);
void MULTL_XMLAddNodeHEX(mxml_node_t *pParent, CHAR_T* pTagname, U32 Value);
void MULTL_XMLAddNodeBOOL(mxml_node_t *pParent, CHAR_T* pTagname, BOOL Value);
void MULTL_XMLAddNodeMark(mxml_node_t *pParent, CHAR_T* pTagname, BOOL Value);


CHAR_T* MULTL_XMLMarkValueToStr(BOOL Value);
BOOL MULTL_XMLMarkValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLITUCodingValueToStr(S32 ITUCoding);
S32 MULTL_XMLITUCodingValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLAnalogValueToStr(S32 Analog);
S32 MULTL_XMLAnalogValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLQAMModeValueToStr(S32 Mode);
S32 MULTL_XMLQAMModeValueFromStr(CHAR_T* pString);
S32  MULTL_XMLETHValueFromStr( CHAR_T* pString);
CHAR_T*  MULTL_XMLETHValueToStr( S32 value );
CHAR_T* MULTL_XMLDeliveryTypeToStr(S32 Value);
S32 MULTL_XMLDeliveryTypeFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLCableDeliveryModeValueToStr(S32 Value);
S32 MULTL_XMLCableDeliveryModeValueFromStr(CHAR_T* pString);
U32 MULTL_XMLGetNodeIP(mxml_node_t *pParent, CHAR_T* pTagname);

S32 MULTL_XMLFecEncodeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLFecEncodeValueToStr( S32 Value );

/////////////////////////////////////////////////
CHAR_T* MULTL_XMLE3DS3ValueToStr(S32 Value);
S32 MULTL_XMLE3DS3ValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLBitOrderValueToStr(S32 Value);
S32 MULTL_XMLBitOrderValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLFrameformValueToStr(S32 Value);
S32 MULTL_XMLFrameformValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLPacketlengthValueToStr(S32 Value);
S32 MULTL_XMLPacketlengthformValueFromStr(CHAR_T* pString);
/* -DTMB PN----------------------------------------------------------------------------------------------------------------------------------- */
CHAR_T* MULTL_XMLDTMBPNValueToStr(S32 Value);
S32 MULTL_XMLDTMBPNValueFromStr(CHAR_T* pString);
/* -DTMB CodeRate----------------------------------------------------------------------------------------------------------------------------------- */
CHAR_T* MULTL_XMLDTMBCodeRateValueToStr(S32 Value);
S32 MULTL_XMLDTMBCodeRateValueFromStr(CHAR_T* pString);




S32 MULTL_XMLPolarMethodValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLPolarMethodValueToStr(BOOL Value);
S32 MULTL_XMLReqTunerTypeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLReqTunerTypeValueToStr(BOOL Value);

S32 MULTL_XMLTunerSpecinvValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLTunerSpecinvValueToStr(BOOL Value);



CHAR_T* MULTL_XMLChnTypeValueToStr(S32 Value);
S32 MULTL_XMLChnTypeValueFromStr(CHAR_T* pString);
CHAR_T* MULTL_XMLChnSubTypeValueToStr(S32 Value);
S32 MULTL_XMLChnSubTypeValueFromStr(CHAR_T* pString);


S32 MULTL_GenerateSN(U8 DeviceType, U8 DeviceSubType, U32 ChipID, U8 *pSN, S32 BufSize);
void MULTL_GenerateMAC(U8 DeviceType, U8 DeviceSubType, S32 Index, U32 ChipIDSeries, U8 *pMAC, S32 BufSize);
BOOL MULTL_LoadOEM(MULT_Handle *pHandle, CHAR_T *pPathname, BOOL bCheckValid, CHAR_T* pInformation, S32 BufSize);


void MULTL_GenerateInfoXML(MULT_Handle *pHandle);

void MULTL_DefaultConfiguration(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);
void MULTL_SaveConfigurationXML(MULT_Handle *pHandle);
void MULTL_LoadConfigurationXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);

void MULTL_LoadMaintenaceXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);
void MULTL_SaveMaintenaceXML(MULT_Handle *pHandle);


void MULTL_DefaultParameter(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);
void MULTL_SaveParameterXML(MULT_Handle *pHandle);
void MULTL_LoadParameterXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);
void MULTL_ReleaseParameter(MULT_Handle *pHandle);


void MULTL_DefaultMonitorSetting(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);
void MULTL_SaveMonitorXML(MULT_Handle *pHandle);
void MULTL_LoadMonitorXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);
void MULTL_ReleaseMonitor(MULT_Handle *pHandle);


void MULTL_XMLSaveDescriptors(MULT_Handle *pHandle, mxml_node_t *pXMLParent, U8 DescriptorType, U32 ParentIDs);
void MULTL_XMLLoadDescriptors(MULT_Handle *pHandle, mxml_node_t *pXMLParent, U8 DescriptorType, U32 ParentIDs);
void MULTL_XMLSaveMuxCAs(MULT_Handle *pHandle, mxml_node_t *pXMLParent, U8 CAType, U32 ParentIDs);
void MULTL_XMLLoadMuxCAs(MULT_Handle *pHandle, mxml_node_t *pXMLParent, U8 CAType, U32 ParentIDs);
void MULTL_XMLSaveSCSCAs(MULT_Handle *pHandle, mxml_node_t *pXMLParent, U8 CAType, U32 ParentIDs);
void MULTL_XMLLoadSCSCAs(MULT_Handle *pHandle, mxml_node_t *pXMLParent, U8 CAType, U32 ParentIDs, MULT_IDsArray *pACArray);
void MULTL_XMLSaveMonitorLimitInfo(MULT_Handle *pHandle, mxml_node_t *pXMLParent, MULT_MonitorLimitInfo *pInfo);
void MULTL_XMLLoadMonitorLimitInfo(MULT_Handle *pHandle, mxml_node_t *pXMLParent, MULT_MonitorLimitInfo *pInfo);

void MULTL_LoadChannelInfoXML(MULT_Handle *pHandle, MULT_ChannelNode *pChnNode, mxml_node_t *pXMLParent);
void MULTL_LoadSubChannelInfoXML(MULT_Handle *pHandle, MULT_ChannelNode *pChnNode, MULT_SubChannelNode *pSubNode, mxml_node_t *pXMLParent);

#ifdef GN1846
#include "adv7612.h"
#include "encoder_3531A.h"

void MULTL_LoadEdidXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);
void MULTL_SaveEdidXML(MULT_Handle *pHandle);
void MULTL_ApplyLEncoderParameter(MULT_Handle *pHandle, S32 ChnIndex);
ENC_3531AViMode MULTL_Adv7612ViMode2LEncoderViMode(VideoStd_Type lVidStd);
ENC_3531AAudSample MULTL_Adv7612AudSample2LEncoderAudSample(samplerate_items lAudSample);
#endif

void MULTL_CreateIDs(MULT_IDsArray *pArray, S32 MaxNum);
void MULTL_AddIDs(MULT_IDsArray *pArray, MULT_IDs *pACIDs);
U32 MULTL_GetNewIDs(MULT_IDsArray *pArray, U32 OriIDs);
void MULTL_DestroyIDs(MULT_IDsArray *pArray);

void MULTL_ForceSNTPSync(MULT_Handle *pHandle);


void MULTL_ApplyQAMParameter(MULT_Handle *pHandle, S32 Index);


////////////////////////////////////////
void MULTL_ApplyInE3DS3Parameter(MULT_Handle *pHandle, S32 ChnIndex);
#ifdef GM8358Q
void MULTL_ApplyEncoderParameter(MULT_Handle *pHandle, S32 ChnIndex);
#endif
#ifdef GM8398Q
void MULTL_ApplyEncoderParameter(MULT_Handle *pHandle, S32 ChnIndex);
#endif
void MULTL_ApplyTunerParameter(MULT_Handle *pHandle, S32 ChnIndex);
void MULTL_ApplyOutASIParameter(MULT_Handle *pHandle, S32 ChnIndex);
void MULTL_ApplyInETHParameter(MULT_Handle *pHandle, S32 ChnIndex);
void MULTL_ApplyOutETHParameter(MULT_Handle *pHandle, S32 ChnIndex);





void MULTL_ResetAlarmCount(MULT_Handle *pHandle, S32 AlarmIndex);
void MULTL_ParameterReset(MULT_Handle *pHandle);
void MULTL_FactoryPreset(MULT_Handle *pHandle);
void MULTL_ManagePortConfig(MULT_Handle *pHandle);
#if defined(GN1846) || defined(GN1866)
BOOL MULTL_GetEthSetFlag(void); 
#endif

#ifdef USE_CARD_FRP
#ifdef GN1846
typedef enum {
	FRP_ALARM_TOTAL = 0,
	FRP_ALARM_INPUT,
	FRP_ALARM_OUTPUT
}FRP_AlarmType;
void FRP_CardInitiate(void);
void FRP_CardSetAlarm(S32 AlarmType, BOOL IsAlarm);
void FRP_CardDisplayInfoUpdate(MULT_Handle *pHandle); 
void FRP_CardShowInitateProgressString(CHAR_T *pString);
void FRP_CardSetInit(MULT_Handle *pHandle, BOOL bEnableInit);
void FRP_CardAccess(MULT_Handle *pHandle, S32 Duration);
void FRP_CardTerminate(void);
#else
void FRP_CardInitiate(void);
void FRP_CardSetInit(BOOL bEnableInit);
void FRP_CardAccess(MULT_Handle *pHandle, S32 Duration);
void FRP_CardTerminate(void);
#endif
#else
void FRP_AgentInitiate(MULT_Handle *pHandle);
void FRP_AgentAccess(MULT_Handle *pHandle, S32 Duration);
void FRP_AgentSetManagePortRelatedData(MULT_Handle *pHandle);
void FRP_AgentTerminate(void);
#endif


#ifdef USE_FRP_PRE_CONSOLE
void MULT_FRPPreConsoleIntiate(void);
void MULT_FRPPreConsoleSetText(CHAR_T *pLineText, S32 Line);
void MULT_FRPPreConsoleTerminate(void);
#endif


BOOL MULTL_LoadMaintenaceFromStorage(MULT_Handle *pHandle, CHAR_T *pFilePathName);
void MULTL_SaveMaintenaceToStorage(MULT_Handle *pHandle);
void MULTL_GenerateServiceXML(MULT_Handle *pHandle, CHAR_T *pFilePathName);


void MULT_SetApplyBusyMark(MULT_Handle *pHandle, BOOL bActive);
void MULT_AnalyzeBatchStart(MULT_Handle *pHandle, S16 DevIndex, S16 TimeOutForPSIMs, S32 AnalyzeCharset, BOOL bTimeoutRemoveService);
void MULT_AnalyzeStart(MULT_Handle *pHandle, S16 DevIndex, S16 TsIndex, S16 TimeOutForPSIMs, S32 AnalyzeCharset, BOOL bTimeoutRemoveService, S32 AutoMapMode);
BOOL MULTL_GetTsMark(MULT_Handle *pHandle, S32 TsIndex, BOOL bInput);
void MULT_AnalyzeDone(MULT_Handle *pHandle);


void MULT_SNMPInitiate(MULT_Handle *pHandle);
S32 MULTL_SNMPCallbackFunc(HANDLE32 UserHandle, U32 ID, S32 ValueType, void *pValue, S32 ValueSize, BOOL bRead);
void MULT_SNMPTerminate(MULT_Handle *pHandle);
void MULT_SNMPAccess(MULT_Handle *pHandle, S32 Duration);

/*SYSLOG*/
#ifdef SUPPORT_SYSLOG_MODULE
void MULT_SyslogApply(MULT_Handle *pHandle, BOOL bClose);
void MULT_SyslogClean(MULT_Handle *pHandle);
S32 MULT_SyslogGet(MULT_Handle *pHandle, CHAR_T *pXMLBuf, S32 BufSize);
#endif

#ifdef SUPPORT_SYSLOG_MODULE
void MULTL_XMLLoadSyslog(MULT_Handle *pHandle, mxml_node_t *pXMLHolder);
#endif


#ifdef MULT_DEVICE_NOT_SUPPORT_ENCRYPT_CHIP
U32 MULT_OLDSNGetSN(void);
BOOL MULT_OLDSNLoadSN(CHAR_T *pPathName);
#endif

#ifdef MULT_ENABLE_ECA_AND_SERVICE_LIST
void MULTL_XMLLoadECA(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bPost);
void MULTL_XMLSaveECA(MULT_Handle *pHandle, mxml_node_t *pXMLRoot);

void MULTL_XMLLoadSERVL(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bPost);
void MULTL_XMLSaveSERVL(MULT_Handle *pHandle, mxml_node_t *pXMLRoot);
#endif


#ifdef GN2000

#define MULT_ENCODER_IN_TS_IND 16
#define MULT_REAL_OUT_TS_IND 0
#define MULT_DECODER_OUT_TS_IND 16
void MULTI_MOSIACAdjustReMuxParameter(MULT_Handle *pHandle);
void MULTI_MOSIACInitiate(void);
void MULTL_MOSIACSaveXML(mxml_node_t *pXMLParent);
void MULTL_MOSIACLoadXML(mxml_node_t *pXMLParent);
void MULTI_MOSIACTerminate(void);


#endif

#ifdef MULT_SUPPORT_FPGA_ETH

#include "tun_module.h"

void MULT_FPGAEthInitiate(U32 ChipID);
void MULT_FPGAEthApply(BOOL bOpen);
void MULT_FPGAEthTerminate(void);
TUN_InitParam *MULT_FPGAEthGetParameterPtr(void);

#endif

#ifdef MULT_TS_BACKUP_SUPPORT

typedef enum  
{
	MULT_BP_TS_BACKUP_MODE_OFF,
	MULT_BP_TS_BACKUP_MODE_MAIN,
	MULT_BP_TS_BACKUP_MODE_BACKUP,
	MULT_BP_TS_BACKUP_MODE_AUTO_REPEAT,
	MULT_BP_TS_BACKUP_MODE_AUTO_ONCE,
	MULT_BP_TS_BACKUP_MODE_REMOTE,
	MULT_BP_TS_BACKUP_MODE_NUM
}MULT_BPTsBackupMode;


typedef struct
{
	MULT_BPTsBackupMode		m_Mode;
	S32						m_BackupTsInd;
	U32						m_LowLimit;
	U32						m_HighLimit;
}MULT_BPTsInBackupInfo;

typedef struct
{
	MULT_BPTsBackupMode		m_Mode;
	S32						m_BackupTsInd;
}MULT_BPTsOutBackupInfo;

void MULT_BPInitiate(MULT_Handle *pHandle, S32 InTsMax, S32 OutTsMax);
S32 MULT_BPGetTsBackupTs(S32 TsInd, BOOL bInput);
void MULT_BPSetInputTsBitrate(S32 TsInd, U32 Bitrate);
void MULT_BPSetOutputRemoteControl(S32 TsInd, BOOL bMain);
BOOL MULT_BPCheckApplyParameters(void);//自动恢复，如果读取该变量，改变量自动回复FALSE
BOOL MULT_BPGetTsIsUsedByBackup(S32 TsInd, BOOL bInput);//查看该TS是否是备用TS
CHAR_T* MULT_BPModeValueToStr(S32 Value);
S32 MULT_BPModeValueFromStr(CHAR_T* pString);
MULT_BPTsBackupMode MULT_BPGetBackupStatus(S32 TsInd, BOOL bInput);
BOOL MULT_BPProcInTsBackupInfo(S32 TsInd, MULT_BPTsInBackupInfo *pInInfo, BOOL bRead);
BOOL MULT_BPProcOutTsBackupInfo(S32 TsInd, MULT_BPTsOutBackupInfo *pOutInfo, BOOL bRead);


void MULT_BPSaveStatusXMLToBuf(CHAR_T *pBuf, S32 BufSize);
void MULT_BPLoadXML(mxml_node_t *pXMLRoot);
void MULT_BPSaveXML(mxml_node_t *pXMLRoot);
void MULT_BPTerminate(void);




#endif


#ifdef SUPPORT_SFN_MODULATOR

typedef struct  
{
	U32						m_SFNMaxDelay100NS;				//最大延迟
	U32						m_SFNSIPPN;					//0，PN420；1，PN595；2，PN945
	U32						m_SFNSIPCarrier;			//0，单载波；1多载波
	U32						m_SFNSIPConstellation;			//000 000:4QAM-NR, 001:4QAM, 010:16QAM, 011:32QAM, 100:64QAM, 101~111:保留
	U32						m_SFNSIPCodeRate;			//0，0.4；1， 0.6；2，0.8；
	U32						m_SFNSIPTI;					//0，240符号；1， 720符号
	U32						m_SFNSIPDoublePilot;		//0，不加，1 加，
	U32						m_SFNSIPPN_Shift;			//0, 不旋转，1 旋转
}MULT_SFN_CMN_SIP;//所有发射机通用参数

typedef struct  
{
	U32						m_SFNAddr;					//SIP包广播寻址
	U32						m_SFNSIPIndviDelay;			//本激励器延迟
	S32						m_SFNSIPIndvFreqOffsetHz;	//输出频率偏移(Hz)
	U16						m_SFNSIPIndvPower;			//本激励器功率控制单位0.1dBm
	BOOL					m_SFNSIPIndvPowerMark;		//本激励器功率控制有效开关
}MULT_SFN_INDV_SIP;//独立寻址发射机参数


typedef struct  
{

	/*设置参数*/
	BOOL					m_bUseSFN;//是否进入单频网模式

	S32						m_SFNASIMode;

	//S32						m_CurrentInputTsInd;

	U8						m_bUse10MClkSynSrc;//0内部，1外部，2，1PPS
	U8						m_Last10MCLKSyncSrc;
	U8						m_bUseEx1PPS;
	U32						m_SFNAddDelay100ns;//数据激励器中的附加的时间（100ns），附件在SFN的设置之外的额外延时

	U16						m_SFNAddrID;//独立寻址ID号
	BOOL					m_bUseIndvSIP;//是否使用独立寻址功能
	BOOL					m_bUseCMNSIP;//是否使用SIP中的调制方式！

	BOOL					m_bDeleteSIP;//是否移除SIP包


	/*卫星SFN参数*/
	BOOL					m_bEnableSatSFN;//打开此功能的时候需要MPEG2的空包丢弃并按照下面的参数执行
	U16						m_SatSFNNullPacketPID;//需要将这个PID替换成MPEG2的空包
	BOOL					m_bSatSFNSIPCRC32Check;//打开此功能的时将对SIP包的CRC32进行校验

	/*MUTE*/
	BOOL					m_bTsLostMUTE;
	BOOL					m_bREFLostMUTE;
	BOOL					m_b1PPSLostMUTE;
	BOOL					m_bSIPLostMUTE;


}MULT_SFN_Param;

typedef struct  
{
	/*单频网适配器状态*/
	MULT_SFN_CMN_SIP		m_SFNCMNSIP;//当前的单频网通用参数，当接受到的SIP和这个不符时，则重新设定调制器的参数
	MULT_SFN_INDV_SIP		m_SFNIndvSIP;//当前单频网的独立参数

	/*单频网参数更新以及更新时间*/
	S32						m_SFNSIPRecvCount;
	BOOL					m_SFNSIPINDVUpdated;


	BOOL					m_bInt1PPSStatus;//本地1PPS状态
	S32						m_AlarmInt1PPSLostCount;
	BOOL					m_bExt1PPSStatus;//外部1PPS状态
	S32						m_AlarmExt1PPSLostCount;
	BOOL					m_bInt10MStatus;//本地10M状态
	S32						m_AlarmInt10MLostCount;
	BOOL					m_bExt10MStatus;//外部10M状态
	S32						m_AlarmExt10MLostCount;
	BOOL					m_bTS0Status;
	BOOL					m_bTS1Status;
	S32						m_AlarmTSXLostCount;
	S32						m_CurrentUseTsInd;
	S32						m_AlarmSIPLostCount;
	S32						m_AlarmBitrateErrorCount;
	S32						m_AlarmSIPChangeCount;
	S32						m_AlarmSIPCRC32ErrCount;

	U32						m_NetDelay100ns;

	S32					m_1PPSLosted;
	S32					m_InputLosted;
	/*10M的错误恢复教给时钟模块自己控制*/
	BOOL					m_SIPLosted;


	BOOL					m_RFMuted;//记录当前输出是否MUTE

}MULT_SFN_Status;


/*模块主要函数*/
void MULT_SFNInitiate(MULT_Handle *pHandle);
void MULT_SFNXMLLoad(mxml_node_t *pXMLRoot, BOOL bPost);
void MULT_SFNXMLSave(mxml_node_t *pXMLRoot, BOOL bStat);
void MULT_SFNApplyParameter(void);
void MULT_SFNApplySIP(MULT_SubModulatorInfo *pSubModInfo);
void MULT_SFNMonitorProcess(S32 Duration);
void MULT_SFNPauseSIPRecv(void);
void MULT_SFNResumeSIPRecv(void);
void MULT_SFNSetInternalCOMData(U8 *pData, S32 DataSize);
void MULT_SFNTerminate(void);

F64 MULTL_SFNGetDTMBBitrate(MULT_SFN_CMN_SIP *pCMNSIP);
void MULTL_SFNSIPParamChange(MULT_SubModulatorInfo *pSubModInfo, MULT_SFN_CMN_SIP *pSFNSISIP, BOOL bToSFNParam);


void MULT_SFNProcSFNParam(MULT_SFN_Param *pParam, BOOL bRead);
void MULT_SFNGetSFNStatus(MULT_SFN_Status *pStatus);
void MULT_SFNApplyByQAMModule(void);

/*状态获取函数*/
BOOL MULT_SFNCheckEnabled(void);
BOOL MULT_SFNCheckSIPRecved(void);
void MULT_SFNForceCMDSIPUpdate(void);//调用此函数后，在单频网下，会触发SIP更新，并自动重新设置RF参数！
typedef enum
{
	SFN_ERROR_SIP_LOST = 0,
	SFN_ERROR_INT_10M_LOST,
	SFN_ERROR_EXT_10M_LOST,
	SFN_ERROR_INT_1PPS_LOST,
	SFN_ERROR_EXT_1PPS_LOST,
	SFN_ERROR_ASI_LOST,
	SFN_ERROR_SIP_CRC32_ERR,
	SFN_ERROR_SIP_CHANGE,
	SFN_ERROR_BITRATE_ERROR,
	SFN_ERROR_NUM,
}MULT_SFNErrType;

BOOL MULT_SFNCheckSFNError(S32 ErrorType, BOOL bClear);

#endif


#ifdef SUPPORT_SFN_ADAPTER
#define SFN_MAX_INDV_NUM			(32)

typedef enum
{
	SFNA_ERROR_INT_10M_LOST = 0,
	SFNA_ERROR_EXT_10M_LOST,
	SFNA_ERROR_INT_1PPS_LOST,
	SFNA_ERROR_EXT_1PPS_LOST,
	SFNA_ERROR_ASI_LOST,
	SFNA_ERROR_NUM,
}MULT_SFNAErrType;

typedef struct  
{
	U32						m_SFNSIPPN;					//0，PN420；1，PN595；2，PN945
	U32						m_SFNSIPCarrier;			//0，单载波；1多载波
	U32						m_SFNSIPConstellation;		//000 000:4QAM-NR, 001:4QAM, 010:16QAM, 011:32QAM, 100:64QAM, 101~111:保留
	U32						m_SFNSIPCodeRate;			//0，0.4；1， 0.6；2，0.8；
	U32						m_SFNSIPTI;					//0，240符号；1， 720符号
	U32						m_SFNSIPDoublePilot;		//0，不加，1 加，
	U32						m_SFNSIPPN_Shift;			//0, 不旋转，1 旋转
}MULT_SFNA_CMN_SIP;//所有发射机通用参数

typedef struct  
{
	U32						m_SFNAddr;					//SIP包广播寻址
	U32						m_SFNSIPIndviDelay;			//本激励器延迟
	S32						m_SFNSIPIndvFreqOffsetHz;	//输出频率偏移(Hz)
	U16						m_SFNSIPIndvPower;			//本激励器功率控制单位0.1dBm
	BOOL					m_SFNSIPIndvPowerMark;		//本激励器功率控制有效开关
}MULT_SFNA_INDV_SIP;//独立寻址发射机参数

typedef struct  
{
	MULT_SFNA_INDV_SIP		m_Info;
	BOOL					m_ActiveMark;
}MULT_SFNA_INDV_SIP_NODE;//独立寻址

typedef struct  
{

	/*设置参数*/
	S32						m_SFNASIMode;/*1=ASI1 2=ASI2 3=AUTO*/

	U8						m_bUse10MClkSynSrc;
	U8						m_bUseEx1PPS;
	U32						m_SFNMaxDelay100NS;			//最大延迟

	/*单频网适配器参数*/
	TD_DTMBParameter		m_DTMBParam;//单频网工作参数

	MULT_SFNA_INDV_SIP_NODE	m_pINDVArrap[SFN_MAX_INDV_NUM];

	/*卫星SFN参数*/
	BOOL					m_bEnableSatSFN;//打开此功能的时候需要MPEG2的空包丢弃并按照下面的参数执行
	U16						m_SatSFNNullPacketPID;//需要将这个PID替换成MPEG2的空包
}MULT_SFNA_Param;

typedef struct  
{
	BOOL					m_bInt1PPSStatus;//本地1PPS状态
	S32						m_AlarmInt1PPSLostCount;
	BOOL					m_bExt1PPSStatus;//外部1PPS状态
	S32						m_AlarmExt1PPSLostCount;
	BOOL					m_bInt10MStatus;//本地10M状态
	S32						m_AlarmInt10MLostCount;
	BOOL					m_bExt10MStatus;//外部10M状态
	S32						m_AlarmExt10MLostCount;
	BOOL					m_bTS0Status;
	BOOL					m_bTS1Status;
	S32						m_AlarmTSXLostCount;

	S32						m_CurrentUseTsInd;

}MULT_SFNA_Status;


void MULT_SFNAInitiate(MULT_Handle *pHandle);
void MULT_SFNAXMLLoad(mxml_node_t *pXMLRoot, BOOL bPost);
void MULT_SFNAXMLSave(mxml_node_t *pXMLRoot, BOOL bStat);
void MULT_SFNAApplyParameter(void);
void MULT_SFNAMonitorProcess(S32 Duration);
BOOL MULT_SFNACheckSFNError(S32 ErrorType, BOOL bClear);
void MULT_SFNASetInternalCOMData(U8 *pData, S32 DataSize);
void MULT_SFNATerminate(void);

void MULT_SFNAProcSFNParam(MULT_SFNA_Param *pParam, BOOL bRead);
void MULT_SFNAGetSFNStatus(MULT_SFNA_Status *pStatus);

#endif




#ifdef SUPPORT_GNS_MODULE
/*模块主要函数*/
void MULT_GNSInitiate(void);
void MULT_GNSXMLLoad(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bPost);
void MULT_GNSXMLSave(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bStat);
void MULT_GNSApplyParameter(void);
void MULT_GNSMonitorProcess(S32 Duration);
void MULT_GNSTerminate(void);
/*状态获取函数*/
BOOL MULT_GNSCheckEnabled(void);
BOOL MULT_GNSCheckLocked(void);
BOOL MULT_GNSCheckError(void);
BOOL MULT_GNSCheckHaveGNS(void);
#endif


#ifdef SUPPORT_IP_O_TS_MODULE
void MULT_IPoTSInitiate(void);
void MULT_IPoTSXMLLoad(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bPost);
void MULT_IPoTSXMLSave(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bStatistics);
void MULT_IPoTSMonitorProcess(S32 Duration);
void MULT_IPoTSApply(void);
void MULT_IPoTSTerminate(void);
#endif

#ifdef SUPPORT_CLK_ADJ_MODULE
BOOL MULT_CLKInitiate(MULT_Handle *pMainHandle);
void MULT_CLKResetByModulator(void);
void MULT_CLKSetDefaultDAValue(U32 DAValue);
U32 MULT_CLKGetDefaultDAValue(void);
void MULT_CLKSetTranningValue(F64 TrainningValue);
F64 MULT_CLKGetTranningValue(void);
BOOL MULT_CLKGet10MLockStatus(void);
void MULT_CLKSet10MCLKSYNCSyc(S32 NewSRC);
S32 MULT_CLKGetCurSumOffset(void);
BOOL MULT_CLKProtocolParser(U8 *pData, S32 DataSize);
BOOL MULT_CLKGetSyncError(BOOL bClear);
void MULT_CLKTerminate(void);
void MULTL_CLKProtocolPacker();
#endif

#ifdef SUPPORT_FGPIO_MODULE
#define FPGA_GPIO_DPD_CONTROL_BIT					(0x00000001)
BOOL FGPIO_Initiate(void);
BOOL FGPIO_ProtocolParser(U8 *pData, S32 DataSize);
void FGPIO_ValueSet(S32 PortInd, U32 GPIOMask, U8 bHigh);
void FGPIO_IOMaskSet(S32 PortInd, U32 GPIOMask, U8 bInput);
void FGPIO_Write(S32 PortInd);
BOOL FGPIO_Read(S32 PortInd, U32 *pValue, S32 Timeout);
void FGPIO_Terminate(void);
#endif


#ifdef GM2730S
BOOL MULT_GetOutChannelState(S16 TsIndex);
#endif




#ifdef SUPPORT_NTS_DPD_BOARD
#define NTS_DPD_LOG_FILE_PATH_NAME		"/tmp/dpd_log.tgz"
typedef struct  
{
	U8		m_DPDMark;
	U8		m_DPDTrackResetMark;
	U8		m_DPDFeedbackSelect;//0是A，1是B
	U32		m_TimeTick;
	U32		m_Reserved;
}DPD_CONTROL_DPD_PARAM;


typedef struct  
{
	U8		m_DPDStatus;//DPDPLL 工作状态：1，正常；0，异常；
	U8		m_DPDTrackStatus;//DPD 跟踪状态：0，跟踪成功完成；1，跟踪进行中；2跟踪失败；
	U8		m_DPDFeedebackStatus;//DPD反馈信号状态；0，正常；1，过高；2，过低；
	U16		m_DPDFeedebacklevel;//DPD反馈信号电平：70dBuV = 70；
	U16		m_DPDBoardTemp;//DPD模块温度：44摄氏度=44
	U8		m_DPDClkStatus;
	U8		m_DPDIOStatus;
	U16		m_DPDTxPower;
	U8		m_DPDRunFlag;
	S32		m_DPDSN;//带肩比 Value/100=显示的dB数
	U8		m_pReserved[4];
}DPD_CONTROL_STATUS;
void MULT_NTSDPDInitiate(MULT_Handle *pHandle);
void MULT_NTSDPDXMLLoad(mxml_node_t *pXMLRoot, BOOL bPost);
void MULT_NTSDPDXMLSave(mxml_node_t *pXMLRoot, BOOL bStat);
BOOL MULT_NTSDPDProcParam(DPD_CONTROL_DPD_PARAM *pParam, BOOL bRead);
BOOL MULT_NTSDPDProcStatus(DPD_CONTROL_STATUS *pStatus);
void MULT_NTSDPDApplyParameter(void);
void MULT_NTSDPDGenerateDPDLogFile(void);
void MULT_NTSDPDSaveDPDTable(void);
BOOL MULT_NTSDPDGetError(S32 ErrorType, BOOL bClear);
BOOL MULT_NTSDPDGetPLLError(void);
void MULT_NTSDPDTerminate(void);

#endif


#ifdef SUPPORT_NEW_FRP_MENU_MODULE
void MULT_FRPMenuADVInitiate(MULT_Handle *pHandle);
void MULT_FRPMenuADVShowInitateProgressString(CHAR_T *pString);
void MULT_FRPMenuADVShowRebootProgressString(void);
void MULT_FRPMenuADVStart(MULT_Handle *pHandle);
void MULT_FRPMenuADVDestroy(void);
#endif


void MULT_SaveCLKADJModuleParameters(MULT_Handle *pHandle);
void MULT_ResetCLKADJModuleParameters(MULT_Handle *pHandle);


/*SCS接口*/
void MULTL_SCSManagerInitiate(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo);
void MULTL_SCSManagerApply(MULT_Handle *pHandle, BOOL bClose);
void MULTL_SCSManagerDestroy(MULT_Handle *pHandle);

/*授权接口*/
void MULTL_GenerateAuth(MULT_Handle *pHandle);
BOOL MULTL_LoadLicense(MULT_Handle *pHandle, CHAR_T *pPathname, BOOL bCheckValid, CHAR_T* pInformation, S32 BufSize);

/*SNTP模块*/
void MULTL_SNTPRequestThread(void *pParam);


/*卡式设备接口*/
#ifdef ENCODER_CARD_PLATFORM
void MULT_CARDModuleInitiate(MULT_Handle *pHandle, BOOL bSubFlashUpgradeMode);
BOOL MULT_CARDModuleICPProsessRecv(U8 *pData, S32 DataSize);
void MULT_CARDModuleXMLGetProcess(MULT_Handle *pHandle, mxml_node_t* pXML);
void MULT_CARDModuleXMLPostProcess(MULT_Handle *pHandle, mxml_node_t* pXML);
BOOL MULT_CARDModuleIsBusy(void);
void MULT_CARDModuleTerminate(void);
#endif


#ifdef KERNEL_AUTO_UPDATE
void MULT_KernelAutoUpdate(void);
#endif



BOOL PES_Initiate(HANDLE32 DBSHandle);
void PES_Terminate(void);
void PES_XMLAcess(HANDLE32 XMLLoad, HANDLE32 XMLSave);


/*固件升级系统*/
typedef enum
{
	MULT_SUB_FLASH_OP_TYPE_VALIDATION_GET = 0,
	MULT_SUB_FLASH_OP_TYPE_CONFIRM,
	MULT_SUB_FLASH_OP_TYPE_UPGRADE_MODE_SWITCH,
	MULT_SUB_FLASH_OP_TYPE_NUM
}MULT_SUB_FLASH_OP_TYPE;

typedef struct  
{
	UPG_MODULEInfo	m_UPGInfo;
	U8		*m_pDataPtr;
	S32		m_DataSize;
	U32		m_DataCRC32;
}MULT_SUB_FLASH_INFO;


BOOL MULT_SubFlashXMLProcess(MULT_Handle *pHandle, mxml_node_t* pBodyNode, CHAR_T *pParameterOrSubType, S32 OPType);
BOOL MULT_SubFlashGetFlashInfo(MULT_SUB_FLASH_INFO *pInfo);
void MULT_SubFlashTestInit(void);

#endif
