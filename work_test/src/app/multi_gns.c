/* Includes-------------------------------------------------------------------- */
#include "multi_private.h"
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#include "multi_hwl_internal.h"
#include "multi_tsp.h"
/*使用GPS/BDS分析库 可以兼容GS-97U/UM220模块*/
#ifdef SUPPORT_GNS_MODULE

//#define GNS_DEBUG

#include "gps_parser.h"

#ifdef SUPPORT_VRS232_MODULE
#include "hwl_vrs232.h"
#endif

/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define MULT_GNS_DEFAULT_TIMING_INTERVAL	(8 * 3600)/*秒*/
#define MULT_GNS_ERROR_TIME_OUT_MS			(8000)/*毫秒*/
#define MULT_GNS_UART_DATA_BUFFER_SIZE		(64 * 1024)
/* Private Constants ---------------------------------------------------------- */

/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	BOOL					m_bEnableGPS;

	/*设置参数*/
	BOOL					m_bGNSUseToTiming;//是否使用GPS给系统授时

	S32						m_GNSTimingInterval;//授时间隔（秒）


	S32						m_SYSType;/*见GNS_UM220_WORK_MODE_HYBIRD*///系统类型（0,GPS, 1, BDS, 2，混合系统）
}MULT_GNS_Param;


typedef struct  
{
	/*设置参数*/
	MULT_GNS_Param			m_GNSParam;

	S32						m_GNSTimingTimeoutMS;//授时超时定时器（毫秒）

	S32						m_GNSErrorTimeoutMS;//错误超时定时器（秒）

	GPS_ALL_T				m_GNSStatus;

	BOOL					m_ThreadMark;
	HANDLE32				m_ThreadHandle;
	HANDLE32				m_UARTHandle;
	BOOL					m_bModuleDetected;
	CHAR_T					m_pGNSTmpBuf[MULT_GNS_UART_DATA_BUFFER_SIZE];
	S32						m_GNSTmpBufSize;
	S32						m_RedvPauseTimeout;//授时间隔（秒）
}MULT_GNS_HANDLE;

/* Private Variables (static)-------------------------------------------------- */
static MULT_GNS_HANDLE s_GNSHandle;
/* Private Function prototypes ------------------------------------------------ */
static void MULTL_GNSStart(BOOL bStart);
static void MULTL_GNSStop(void);
static void MULTL_GNSModuleDetect(void);
static void MULTL_GNSCOMOpen(void);
static void MULTL_GNSCOMFlush(void);
static S32 MULTL_GNSCOMWrite(S32 Slot, U8 *pData, S32 DataLen);
static S32 MULTL_GNSCOMRead(S32 Slot, U8 *pBuf, S32 BufLen);
static void MULTL_GNSCOMClose(void);
static void MULTL_PrintInfo(GPS_ALL_T *pGPSAllData);
/* Functions ------------------------------------------------------------------ */

void MULT_GNSInitiate(void)
{
	GLOBAL_ZEROMEM(&s_GNSHandle, sizeof(MULT_GNS_HANDLE));
	s_GNSHandle.m_GNSParam.m_SYSType = GNS_UM220_WORK_MODE_HYBIRD;
	s_GNSHandle.m_GNSParam.m_GNSTimingInterval = MULT_GNS_DEFAULT_TIMING_INTERVAL;
	s_GNSHandle.m_GNSErrorTimeoutMS = MULT_GNS_ERROR_TIME_OUT_MS;

	MULTL_GNSModuleDetect();

	GLOBAL_TRACE(("Initiate Complete !!! Detected = %d\n", s_GNSHandle.m_bModuleDetected));
}


void MULT_GNSXMLLoad(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bPost)
{
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLHolder;

	if (bPost)
	{
		plXMLHolder = pXMLRoot;
	}
	else
	{
		plXMLHolder = mxmlFindElement(pXMLRoot, pXMLRoot, "gns_setting", NULL, NULL, MXML_DESCEND_FIRST);
	}

	if (plXMLHolder)
	{
		s_GNSHandle.m_GNSParam.m_bEnableGPS = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "gns_mark", FALSE);
		s_GNSHandle.m_GNSParam.m_bGNSUseToTiming = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "gns_timing_mark", FALSE);
		s_GNSHandle.m_GNSParam.m_GNSTimingInterval = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "gns_timing_interval", 10, MULT_GNS_DEFAULT_TIMING_INTERVAL);
		s_GNSHandle.m_GNSParam.m_SYSType = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "gns_sys_type", 10, MULT_GNS_DEFAULT_TIMING_INTERVAL);
	}
}

void MULT_GNSXMLSave(MULT_Handle *pHandle, mxml_node_t *pXMLRoot, BOOL bStat)
{
	CHAR_T plTmpStr[512];
	S32 lTmpStrLen;
	mxml_node_t *plXMLHolder;

	if (bStat)
	{
		S32 i, k;
		BOOL lbFound;
		GPS_ALL_T *plGPS;
		GPS_UTC_T *plUTC;
		mxml_node_t *plXMLSatHolder;

		plGPS = &s_GNSHandle.m_GNSStatus;
		plUTC = &plGPS->Utc;
		plXMLHolder = pXMLRoot;

		MULTL_XMLAddNodeMark(plXMLHolder, "gns_lock", (s_GNSHandle.m_GNSStatus.DeviceStatus.LocalStatus == 'A')?1:0);
		MULTL_XMLAddNodeFLOATE(plXMLHolder, "gns_h_accuracy", s_GNSHandle.m_GNSStatus.DeviceStatus.HDOP);
		MULTL_XMLAddNodeFLOATE(plXMLHolder, "gns_v_accuracy", s_GNSHandle.m_GNSStatus.DeviceStatus.VDOP);
		MULTL_XMLAddNodeFLOATE(plXMLHolder, "gns_p_accuracy", s_GNSHandle.m_GNSStatus.DeviceStatus.PDOP);

		if (s_GNSHandle.m_GNSStatus.DeviceStatus.LocalStatus != 'A')
		{
			s_GNSHandle.m_GNSStatus.Position.Latitude.NS = 'X';
			s_GNSHandle.m_GNSStatus.Position.Longitude.EW = 'X';
		}

		GLOBAL_SPRINTF((plTmpStr, "%c", s_GNSHandle.m_GNSStatus.Position.Latitude.NS));
		MULTL_XMLAddNodeText(plXMLHolder, "gns_latitude_mark", plTmpStr);
		MULTL_XMLAddNodeUINT(plXMLHolder, "gns_latitude_degree", s_GNSHandle.m_GNSStatus.Position.Latitude.Degree);
		MULTL_XMLAddNodeUINT(plXMLHolder, "gns_latitude_minite", s_GNSHandle.m_GNSStatus.Position.Latitude.Minute);
		MULTL_XMLAddNodeFLOAT(plXMLHolder, "gns_latitude_second", s_GNSHandle.m_GNSStatus.Position.Latitude.Sec);

		GLOBAL_SPRINTF((plTmpStr, "%.2d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d\n", plUTC->Year, plUTC->Month, plUTC->Day, plUTC->Hour, plUTC->Minute, plUTC->Sec, plUTC->mSec));
		MULTL_XMLAddNodeText(plXMLHolder, "gns_time", plTmpStr);

		GLOBAL_SPRINTF((plTmpStr, "%c", s_GNSHandle.m_GNSStatus.Position.Longitude.EW));
		MULTL_XMLAddNodeText(plXMLHolder, "gns_longitude_mark", plTmpStr);
		MULTL_XMLAddNodeUINT(plXMLHolder, "gns_longitude_degree", s_GNSHandle.m_GNSStatus.Position.Longitude.Degree);
		MULTL_XMLAddNodeUINT(plXMLHolder, "gns_longitude_minite", s_GNSHandle.m_GNSStatus.Position.Longitude.Minute);
		MULTL_XMLAddNodeFLOAT(plXMLHolder, "gns_longitude_second", s_GNSHandle.m_GNSStatus.Position.Longitude.Sec);




		MULTL_XMLAddNodeFLOAT(plXMLHolder, "gns_altitude", s_GNSHandle.m_GNSStatus.Position.Altitude.MslHeight);

		MULTL_XMLAddNodeUINT(plXMLHolder, "gns_sat_total", s_GNSHandle.m_GNSStatus.Satellite.Total);
		MULTL_XMLAddNodeUINT(plXMLHolder, "gns_sat_used", s_GNSHandle.m_GNSStatus.Satellite.Used);


		for (i = 0; i < s_GNSHandle.m_GNSStatus.Satellite.Total; i++)
		{
			plXMLSatHolder = mxmlNewElement(plXMLHolder, "sat_info");
			if (plXMLSatHolder)
			{
				MULTL_XMLAddNodeUINT(plXMLSatHolder, "sat_id", s_GNSHandle.m_GNSStatus.Satellite.ID[i]);
				MULTL_XMLAddNodeUINT(plXMLSatHolder, "sat_snr", s_GNSHandle.m_GNSStatus.Satellite.SNR[i]);
				MULTL_XMLAddNodeUINT(plXMLSatHolder, "sat_elv", s_GNSHandle.m_GNSStatus.Satellite.Elevation[i]);
				MULTL_XMLAddNodeUINT(plXMLSatHolder, "sat_azi", s_GNSHandle.m_GNSStatus.Satellite.Azimuth[i]);

				lbFound = FALSE;
				if (s_GNSHandle.m_GNSStatus.Satellite.ID[i] < 33)
				{
					for (k = 0; k < GPS_MAX_USED_SAT_NUM; k++)
					{
						if (s_GNSHandle.m_GNSStatus.Satellite.GPSChannel[k] == s_GNSHandle.m_GNSStatus.Satellite.ID[i])
						{
							lbFound = TRUE;
							break;
						}
					}
				}
				else
				{
					for (k = 0; k < GPS_MAX_USED_SAT_NUM; k++)
					{
						if (s_GNSHandle.m_GNSStatus.Satellite.BDSChannel[k] == s_GNSHandle.m_GNSStatus.Satellite.ID[i])
						{
							lbFound = TRUE;
							break;
						}
					}
				}
				MULTL_XMLAddNodeMark(plXMLSatHolder, "gns_used", lbFound);
			}

		}

	}
	else
	{
		if (s_GNSHandle.m_bModuleDetected)
		{
			plXMLHolder = mxmlNewElement(pXMLRoot, "gns_setting");
			if (plXMLHolder)
			{
				MULTL_XMLAddNodeMark(plXMLHolder, "gns_mark", s_GNSHandle.m_GNSParam.m_bEnableGPS);
				MULTL_XMLAddNodeMark(plXMLHolder, "gns_timing_mark", s_GNSHandle.m_GNSParam.m_bGNSUseToTiming);
				MULTL_XMLAddNodeUINT(plXMLHolder, "gns_timing_interval", s_GNSHandle.m_GNSParam.m_GNSTimingInterval);
				MULTL_XMLAddNodeUINT(plXMLHolder, "gns_sys_type", s_GNSHandle.m_GNSParam.m_SYSType);
			}
		}
	}
}

void MULT_GNSApplyParameter(void)
{
	if (s_GNSHandle.m_GNSParam.m_bEnableGPS)
	{
		if (s_GNSHandle.m_GNSParam.m_bGNSUseToTiming)
		{
			GLOBAL_TRACE(("Use GPS Timing!!!!!!!! \n"));
			s_GNSHandle.m_GNSTimingTimeoutMS = 5000;
		}
		s_GNSHandle.m_RedvPauseTimeout = 1000;
		MULTL_GNSStart(TRUE);
	}
	else
	{
		MULTL_GNSStart(FALSE);
	}
}

void MULT_GNSMonitorProcess(S32 Duration)
{
	if (s_GNSHandle.m_GNSParam.m_bEnableGPS)
	{
		if (s_GNSHandle.m_RedvPauseTimeout > 0)
		{
			s_GNSHandle.m_RedvPauseTimeout -= Duration;
		}

		if (s_GNSHandle.m_GNSErrorTimeoutMS > 0)
		{
			s_GNSHandle.m_GNSErrorTimeoutMS -= Duration;
		}

		GPS_AllGet(&s_GNSHandle.m_GNSStatus);
		//MULTL_PrintInfo(&s_GNSHandle.m_GNSStatus);

		if (s_GNSHandle.m_GNSParam.m_bGNSUseToTiming == TRUE)
		{
			//GLOBAL_TRACE(("s_GNSHandle.m_GNSTimingTimeoutMS = %d\n", s_GNSHandle.m_GNSTimingTimeoutMS));
			if (s_GNSHandle.m_GNSTimingTimeoutMS > 0)
			{
				s_GNSHandle.m_GNSTimingTimeoutMS -= Duration;
			}
			else
			{
				/*如果GPS状态正常则设定时间，否则等待到正常位置*/
				if (s_GNSHandle.m_GNSStatus.DeviceStatus.LocalStatus == 'A')
				{
					TIME_T lTimeStamp;
					STRUCT_TM lTimeT;
					GPS_UTC_T *plGPSUTC;
					/*设置时间*/
					GLOBAL_TRACE(("GPS Timing!!!!!!!!!!!!\n"));

					plGPSUTC = &s_GNSHandle.m_GNSStatus.Utc;
					lTimeT.tm_year = plGPSUTC->Year - 1900;
					lTimeT.tm_mday = plGPSUTC->Day;
					lTimeT.tm_mon = plGPSUTC->Month - 1;
					lTimeT.tm_hour = plGPSUTC->Hour;
					lTimeT.tm_min = plGPSUTC->Minute;
					lTimeT.tm_sec = plGPSUTC->Sec;


					GLOBAL_TRACE(("Timing = %d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d\n", plGPSUTC->Year, plGPSUTC->Month, plGPSUTC->Day, plGPSUTC->Hour, plGPSUTC->Minute, plGPSUTC->Sec, plGPSUTC->mSec));
					lTimeStamp = GLOBAL_MKTIME(&lTimeT);

					GLOBAL_STIME(&lTimeStamp);

					s_GNSHandle.m_GNSTimingTimeoutMS = s_GNSHandle.m_GNSParam.m_GNSTimingInterval * 1000;
				}
				else
				{
					//GLOBAL_TRACE(("GPS Not Locked!!\n"));
				}
			}


		}
	}
}

BOOL MULT_GNSCheckEnabled(void)
{
	return s_GNSHandle.m_GNSParam.m_bEnableGPS;
}

BOOL MULT_GNSCheckLocked(void)
{
	return (s_GNSHandle.m_GNSStatus.DeviceStatus.LocalStatus == 'A')?TRUE:FALSE;
}

BOOL MULT_GNSCheckHaveGNS(void)
{
	return s_GNSHandle.m_bModuleDetected;
}

BOOL MULT_GNSCheckError(void)
{
	return (s_GNSHandle.m_GNSErrorTimeoutMS < 0)?TRUE:FALSE;
}

void MULT_GNSTerminate(void)
{
	MULTL_GNSStop();


}

/* 本地函数 --------------------------------------------------------------------------------------------------------------------------------------- */
void MULTL_GNSThreadFunction(void *pUserParam)
{
	S32 lActSize;
	CHAR_T *plTmpBuf;

	//GLOBAL_TRACE(("GNS Read Thread Start!!!!!!!!!!!\n"));
	/*初始化串口*/
	MULTL_GNSCOMOpen();

	/*清空GPS设置*/
	GPS_Clear();

	MULTL_GNSCOMFlush();
	/*配置模块*/
	{
		CHAR_T *plCMDStr;
		S32 lCMDLen = 0;

		plCMDStr = GNS_UNICOREEncodeCFGSYSCMD(s_GNSHandle.m_GNSParam.m_SYSType, TRUE);
		lCMDLen = GLOBAL_STRLEN(plCMDStr);
		MULTL_GNSCOMWrite(0, (U8*)plCMDStr, lCMDLen);
		PFC_TaskSleep(200);

		plCMDStr = GNS_UNICOREEncodeCFGTPCMD(TRUE, TRUE);
		lCMDLen = GLOBAL_STRLEN(plCMDStr);
		MULTL_GNSCOMWrite(0, (U8*)plCMDStr, lCMDLen);
	}

	//GLOBAL_TRACE(("GNS Init Config Done!!!!!!!!!!!\n"));

	s_GNSHandle.m_GNSTmpBufSize = 0;

	while(s_GNSHandle.m_ThreadMark)
	{
		if (s_GNSHandle.m_GNSTmpBufSize >= sizeof(s_GNSHandle.m_pGNSTmpBuf))
		{
			s_GNSHandle.m_GNSTmpBufSize = 0;
		}
		plTmpBuf = &s_GNSHandle.m_pGNSTmpBuf[s_GNSHandle.m_GNSTmpBufSize];
		lActSize = MULTL_GNSCOMRead(0, (U8*)plTmpBuf, 1);
		if (lActSize > 0)
		{
			/*重置超时定时器*/
			s_GNSHandle.m_GNSErrorTimeoutMS = MULT_GNS_ERROR_TIME_OUT_MS;

			if (plTmpBuf[0] == '\r' || plTmpBuf[0] == '\n')
			{
				if (s_GNSHandle.m_GNSTmpBufSize > 1)
				{
#ifdef GNS_DEBUG
					GLOBAL_PRINTF((s_GNSHandle.m_pGNSTmpBuf));
					GLOBAL_PRINTF(("\n"));
#endif
					if (s_GNSHandle.m_RedvPauseTimeout < 0)
					{
						GPS_SentenceParse(s_GNSHandle.m_pGNSTmpBuf);
					}
				}
				GLOBAL_ZEROMEM(s_GNSHandle.m_pGNSTmpBuf, sizeof(s_GNSHandle.m_pGNSTmpBuf));
				s_GNSHandle.m_GNSTmpBufSize = 0;
			}
			else
			{
				s_GNSHandle.m_GNSTmpBufSize++;
			}
		}
	}

	MULTL_GNSCOMClose();
}

void MULTL_GNSStart(BOOL bStart)
{
	if (s_GNSHandle.m_ThreadHandle)
	{
		MULTL_GNSStop();
	}

	s_GNSHandle.m_ThreadMark = bStart;
	s_GNSHandle.m_ThreadHandle = PFC_TaskCreate(__FUNCTION__, 4096 * 1024, MULTL_GNSThreadFunction, 0, NULL);
}

void MULTL_GNSStop(void)
{
	s_GNSHandle.m_ThreadMark = FALSE;
	if (PFC_TaskWait(s_GNSHandle.m_ThreadHandle, 3000))
	{
		s_GNSHandle.m_ThreadHandle = NULL;
	}
	else
	{

		GLOBAL_TRACE(("Wait Thread Failed!!!\n"));
	}
}

void MULTL_GNSModuleDetect(void)
{
	/*初始化串口*/
	MULTL_GNSCOMOpen();

	MULTL_GNSCOMFlush();

	/*判断UM220是否存在！*/
	{
		CHAR_T *plCMDStr;
		S32 lCMDLen = 0;
		S32 lActSize;
		U8 *plTmpBuf;
		S32 lCount;
		S32 lFoundInd;


		lCount = 0;
		s_GNSHandle.m_GNSTmpBufSize = 0;
		while(lCount < 3)
		{
			/*发送UM220的设备信息获取消息*/
			plCMDStr = GNS_UNICOREEncodePDTInfoCMD();
			lCMDLen = GLOBAL_STRLEN(plCMDStr);
			MULTL_GNSCOMWrite(0, (U8*)plCMDStr, lCMDLen);

			PFC_TaskSleep(500);

			plTmpBuf = &s_GNSHandle.m_pGNSTmpBuf[s_GNSHandle.m_GNSTmpBufSize];
			lActSize = MULTL_GNSCOMRead(0, (U8*)plTmpBuf, 1024);

			if (lActSize > 0)
			{
				s_GNSHandle.m_GNSTmpBufSize += lActSize;
			}
			lCount++;
		}



		lFoundInd = 0;
		plTmpBuf = &s_GNSHandle.m_pGNSTmpBuf[0];


		s_GNSHandle.m_pGNSTmpBuf[s_GNSHandle.m_GNSTmpBufSize] = 0;

		lFoundInd = CAL_StringStrFind(plTmpBuf, "UM220", 0);

		if (lFoundInd != GLOBAL_INVALID_INDEX)
		{
			GLOBAL_TRACE(("GNS Module UM220 Detected!!!!!!\n"));
			s_GNSHandle.m_bModuleDetected = TRUE;
		}
		else
		{
			lFoundInd = CAL_StringStrFind(plTmpBuf, "$GP", 0);
			if (lFoundInd != GLOBAL_INVALID_INDEX)
			{
				GLOBAL_TRACE(("GNS Module GSTAR Detected!!!!!!\n"));
				s_GNSHandle.m_bModuleDetected = TRUE;
			}
		}

		/*调试用打印函数*/
		if (s_GNSHandle.m_bModuleDetected == FALSE)
		{
			GLOBAL_PRINTF(("%s\n", s_GNSHandle.m_pGNSTmpBuf));
		}

		s_GNSHandle.m_GNSTmpBufSize = 0;

	}

	MULTL_GNSCOMClose();
}
/*串口封装函数*/
void MULTL_GNSCOMOpen(void)
{
#ifdef SUPPORT_VRS232_MODULE
	{
		HWL_VRS232_Param lVRS232Param;
		GLOBAL_ZEROMEM(&lVRS232Param, sizeof(HWL_VRS232_Param));
		lVRS232Param.m_TimeOutMS = 1000;

		HWL_VRS232Open(0, &lVRS232Param);
	}
#else
	s_GNSHandle.m_UARTHandle = PFC_ComDeviceOpen(2, TRUE);
	if (s_GNSHandle.m_UARTHandle)
	{
		PFC_ComDeviceSetState(s_GNSHandle.m_UARTHandle, 9600, 8, 'N', 1);//固定！！

		PFC_ComDeviceSetOption(s_GNSHandle.m_UARTHandle, 512, 512, 200, 200);
	}
	else
	{
		GLOBAL_TRACE(("Init COM2 Failed!!!!!!!!!!!\n"));
	}
#endif
}

void MULTL_GNSCOMFlush(void)
{
#ifdef SUPPORT_VRS232_MODULE
	HWL_VRS232Flush(0);
#else
	PFC_ComDeviceFlush(s_GNSHandle.m_UARTHandle);	
#endif
}


S32 MULTL_GNSCOMWrite(S32 Slot, U8 *pData, S32 DataLen)
{
#ifdef SUPPORT_VRS232_MODULE
	return HWL_VRS232Write(Slot, pData, DataLen);
#else
	return PFC_ComDeviceWrite(s_GNSHandle.m_UARTHandle, pData, DataLen);
#endif
}

S32 MULTL_GNSCOMRead(S32 Slot, U8 *pBuf, S32 BufLen)
{
#ifdef SUPPORT_VRS232_MODULE
	return HWL_VRS232Read(Slot, (U8*)pBuf, BufLen);
#else
	return PFC_ComDeviceRead(s_GNSHandle.m_UARTHandle, (U8*)pBuf, BufLen);
#endif
}

void MULTL_GNSCOMClose(void)
{
#ifdef SUPPORT_VRS232_MODULE
	HWL_VRS232Close(0);
#else
	PFC_ComDeviceClose(s_GNSHandle.m_UARTHandle);
	s_GNSHandle.m_UARTHandle = NULL;
#endif
}

/*调试打印函数*/
void MULTL_PrintInfo(GPS_ALL_T *pGPSAllData)
{
	S32 i;
	GPS_DEVICESTATUS_T *plStatus;
	GPS_UTC_T *plUTC;
	GPS_LONGITUDE_T *plLONGT;
	GPS_LATITUDE_T *plLATIT;
	GPS_ALTITUDE_T *plALTIT;
	GPS_SATELLITE_T *plSAT;

	plStatus = &pGPSAllData->DeviceStatus;
	plUTC = &pGPSAllData->Utc;
	plLONGT = &pGPSAllData->Position.Longitude;
	plLATIT = &pGPSAllData->Position.Latitude;
	plALTIT = &pGPSAllData->Position.Altitude;
	plSAT = &pGPSAllData->Satellite;


	GLOBAL_TRACE(("GPS Status	= %c, Type = %d\n", plStatus->LocalStatus, plStatus->Type));
	GLOBAL_TRACE(("Time			= %d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d\n", plUTC->Year, plUTC->Month, plUTC->Day, plUTC->Hour, plUTC->Minute, plUTC->Sec, plUTC->mSec));
	GLOBAL_TRACE(("POS			= Latitude :%c, %.2d'%f'' - %c, %.2d'%f'' - H, %f / %f\n", plLATIT->NS, plLATIT->Degree, plLATIT->Sec, plLONGT->EW, plLONGT->Degree, plLONGT->Sec, plALTIT->MslHeight, plALTIT->GeoidHeight));
	GLOBAL_TRACE(("Accracy		= P %e Meter, H %e Meter, v %e Meter\n", plStatus->PDOP, plStatus->HDOP, plStatus->VDOP));

	for (i = 0; i < 12; i++)
	{
		GLOBAL_TRACE(("Chan %d, ID = %d\n", i, plSAT->GPSChannel[i]))
	}
	for (i = 0; i < 12; i++)
	{
		GLOBAL_TRACE(("Chan %d, ID = %d\n", i, plSAT->BDSChannel[i]))
	}
	for (i = 0; i < plSAT->Total; i++)
	{
		GLOBAL_TRACE(("Slot %d, ID = %d, SNR = %d\n", i, plSAT->ID[i], plSAT->SNR[i]))
	}

}

#endif

/*EOF*/
