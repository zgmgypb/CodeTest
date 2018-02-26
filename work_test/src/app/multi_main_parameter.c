//这个是在MAIN函数里调用的模块

/* Includes-------------------------------------------------------------------- */
#include "multi_main_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

//从OEM、授权系统中读取信息，从系统默认值中得到软件生成时间以及FPGA生成时间等信息，最后生成系统信息项目XML！
void MULTL_GenerateInfoXML(MULT_Handle *pHandle)
{
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	mxml_node_t *plXMLHolder;
	MULT_Information *plInfo;
	MULT_Config *plConfig;

	plInfo = &pHandle->m_Information;
	plConfig = &pHandle->m_Configuration;

	plXML = mxmlNewXML("1.0");
	plXMLRoot = mxmlNewElement(plXML, "root");

	/*设备信息*/
	MULTL_XMLAddNodeCDData(plXMLRoot, "sn", plInfo->m_pSNString);
	MULTL_XMLAddNodeCDData(plXMLRoot, "kernel_release", PFC_GetSystemVersion());
	MULTL_XMLAddNodeCDData(plXMLRoot, "soft_version", plInfo->m_pSoftVersion);
	MULTL_XMLAddNodeCDData(plXMLRoot, "hard_version", plInfo->m_pHardVersion);
	MULTL_XMLAddNodeCDData(plXMLRoot, "soft_release", plInfo->m_pSoftRelease);
	MULTL_XMLAddNodeCDData(plXMLRoot, "fpga_release", plInfo->m_pFPGARelease);

	/*OEM信息*/
	MULTL_XMLAddNodeCDData(plXMLRoot, "device_modle", plInfo->m_pModelName);
	plXMLHolder = mxmlNewElement(plXMLRoot, "device_name");
	MULTL_XMLAddNodeCDData(plXMLHolder, "eng", plInfo->m_pWEBENG);
	MULTL_XMLAddNodeCDData(plXMLHolder, "chn", plInfo->m_pWEBCHN);
	MULTL_XMLAddNodeCDData(plXMLRoot, "manufactury", plInfo->m_pManufacter);
	MULTL_XMLAddNodeCDData(plXMLRoot, "manufactury_web", plInfo->m_pManufacterWEBADDR);
	MULTL_XMLAddNodeUINT(plXMLRoot, "have_manual", plInfo->m_bHaveManual);
	/*设置前面板设备名称*/


	/*授权信息*/
	MULTL_XMLAddNodeUINT(plXMLRoot, "license_valid", plInfo->m_LicenseValid);
#if defined(GQ3760A) || defined(GQ3760)
	{
		CHAR_T plTmpChar[1024];
#ifdef GQ3760A
		GLOBAL_SPRINTF((plTmpChar, "ASI%.2d-IP%.2d-Loop%.2d", plInfo->m_LicenseInASINum, plInfo->m_LicenseInIPNum, plInfo->m_LicenseOutIPNum));
#else
		GLOBAL_SPRINTF((plTmpChar, "ASI%.2d", plInfo->m_LicenseInASINum));
#endif
		MULTL_XMLAddNodeText(plXMLRoot, "license_mode", plTmpChar);
	}
#else
	MULTL_XMLAddNodeUINT(plXMLRoot, "license_mode", plInfo->m_LicenseMode + 1);
#endif
	MULTL_XMLAddNodeCDData(plXMLRoot, "application_code", plInfo->m_ApplicationCode);
	MULTL_XMLAddNodeCDData(plXMLRoot, "license_time", plInfo->m_LicenseDate);
	MULTL_XMLAddNodeINT(plXMLRoot, "trail_left_time", plInfo->m_TrailTime);

#ifdef ENCODER_CARD_PLATFORM
	MULTL_XMLAddNodeText(plXMLRoot, "platform_type", "encoder_card_platform");
#else
	MULTL_XMLAddNodeText(plXMLRoot, "platform_type", "normal");
#endif

	{
		CHAR_T		plPathStr[1024];
		GLOBAL_FD	plFile;

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_DEVICE_INFORMATION_XML);
		plFile = GLOBAL_FOPEN(plPathStr, "w");
		if (plFile != NULL)
		{
			mxmlSaveFile(plXML, plFile, NULL);
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
	}

	mxmlDelete(plXML);
	plXML = NULL;

}

/*设置配置信息*/
void MULTL_DefaultConfiguration(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)//将设备信息恢复为默认值
{
	MULT_Config *plConfig;

	plConfig = &pHandle->m_Configuration;

	GLOBAL_ZEROMEM(plConfig, sizeof(MULT_Config));

	plConfig->m_WebLanguage = 0;

	plConfig->m_ManageIPv4Addr = MULTI_DEFAULT_IP_ADDR;
	plConfig->m_ManageIPv4Mask = MULTI_DEFAULT_IP_MASK;
	plConfig->m_ManageIPv4Gate = MULTI_DEFAULT_IP_GATE;

	if (pHWInfo->m_ChipID == 0)
	{
		GLOBAL_TRACE(("No Chip ID Generate Random MAC\n"));
		GLOBAL_SRAND(PFC_GetTickCount());
		MULTL_GenerateMAC(MULT_DEVICE_TYPE, MULT_DEVICE_SUB_TYPE, 0, CAL_RandGetRange(5000, 10000), plConfig->m_pMAC, GLOBAL_MAC_BUF_SIZE);
	}
	else
	{
		MULTL_GenerateMAC(MULT_DEVICE_TYPE, MULT_DEVICE_SUB_TYPE, 0, pHWInfo->m_ChipID, plConfig->m_pMAC, GLOBAL_MAC_BUF_SIZE);
	}


	plConfig->m_NTPServerAddr = MULTI_DEFAULT_IP_GATE;
	plConfig->m_NTPInterval = MULTI_DEFAULT_NTP_INTERVAL;
	plConfig->m_NTPSyncMark = FALSE;

#ifdef GN1846
	plConfig->m_IpOutputType = IP_OUTPUT_SPTS;
	plConfig->m_OutputCharset = MPEG2_PSI_CHAR_SET_GB2312;
#endif

	//plConfig->m_IGMPVersion = MULTI_DEFAULT_IGMP_VERSION;
	//plConfig->m_IGMPInterval = MULTI_DEFAULT_IGMP_INTERVAL;
	//plConfig->m_IGMPAutoMark = FALSE;

	{
		pHandle->m_Configuration.m_SNMPGlobalMark = FALSE;
		pHandle->m_Configuration.m_SNMPInitParam.m_TRAPGlobalMark = FALSE;
		pHandle->m_Configuration.m_SNMPInitParam.m_AgentPort = 161;
		pHandle->m_Configuration.m_SNMPInitParam.m_TRAPPort = 6000;
		pHandle->m_Configuration.m_SNMPInitParam.m_TRAPIPAddress = MULTI_DEFAULT_IP_GATE;
		pHandle->m_Configuration.m_SNMPInitParam.m_NormalTrapInterval = 500;
		GLOBAL_STRCPY(pHandle->m_Configuration.m_SNMPInitParam.m_pROCommunity , SNMP_DEFAULT_RO_COMMUNITY);
		GLOBAL_STRCPY(pHandle->m_Configuration.m_SNMPInitParam.m_pRWCommunity, SNMP_DEFAULT_RW_COMMUNITY);	
		GLOBAL_STRCPY(pHandle->m_Configuration.m_SNMPInitParam.m_pDeviceName , SNMP_DEFAULT_DEVICE_NAME);
		GLOBAL_STRCPY(pHandle->m_Configuration.m_SNMPInitParam.m_pDeviceLocation, SNMP_DEFAULT_DEVICE_LOCATION);	
	}

#ifdef SUPPORT_SYSLOG_MODULE
	{
		MULT_Syslog *plTmpSyslog;
		plTmpSyslog = &pHandle->m_SyslogParam;
		plTmpSyslog->m_RemoteAddr = MULTI_DEFAULT_IP_GATE;
		plTmpSyslog->m_RemotePort = PL_SYSLOG_DEFAULT_REMOTE_PORT;
		plTmpSyslog->m_RemoteMark = FALSE;
		plTmpSyslog->m_SyslogGlobalMark = FALSE;
		plTmpSyslog->m_LogLevel = PL_SYSLOG_PRIORITY_NOTICE;
	}
#endif
}



void MULTL_SaveConfigurationXML(MULT_Handle *pHandle)
{
	MULT_Config *plConfig;
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	mxml_node_t *plXMLHolder;
	CHAR_T plStrBuf[64];

	plConfig = &pHandle->m_Configuration;

	plXML = mxmlNewXML("1.0");
	plXMLRoot = mxmlNewElement(plXML, "root");

	MULTL_XMLAddNodeINT(plXMLRoot, "version_number", 0);


	MULTL_XMLAddNodeINT(plXMLRoot, "language", plConfig->m_WebLanguage);
	MULTL_XMLAddNodeINT(plXMLRoot, "frp_language", plConfig->m_FrpLanguage);



	plXMLHolder = mxmlNewElement(plXMLRoot, "manage_port");
	MULTL_XMLAddNodeText(plXMLHolder, "ipv4_addr", PFC_SocketNToA(plConfig->m_ManageIPv4Addr));
	MULTL_XMLAddNodeText(plXMLHolder, "ipv4_mask", PFC_SocketNToA(plConfig->m_ManageIPv4Mask));
	MULTL_XMLAddNodeText(plXMLHolder, "ipv4_gate", PFC_SocketNToA(plConfig->m_ManageIPv4Gate));
	CAL_StringBinToMAC(plConfig->m_pMAC, sizeof(plConfig->m_pMAC), plStrBuf, sizeof(plStrBuf));
	MULTL_XMLAddNodeText(plXMLHolder, "ip_mac", plStrBuf);

#ifdef MULT_SUPPORT_FPGA_ETH
	{
		TUN_InitParam *plTunParam = MULT_FPGAEthGetParameterPtr();
		MULTL_XMLAddNodeText(plXMLHolder, "data_ipv4_addr", PFC_SocketNToA(plTunParam->m_TUNIPAddr));
		MULTL_XMLAddNodeText(plXMLHolder, "data_ipv4_mask", PFC_SocketNToA(plTunParam->m_TUNIPMask));
		MULTL_XMLAddNodeText(plXMLHolder, "data_ipv4_gate", PFC_SocketNToA(plTunParam->m_TUNIPGate));
		CAL_StringBinToMAC(plTunParam->m_TUNMAC, sizeof(plTunParam->m_TUNMAC), plStrBuf, sizeof(plStrBuf));
		MULTL_XMLAddNodeText(plXMLHolder, "data_ip_mac", plStrBuf);
	}
#endif

#ifdef GN1846
	{
		plXMLHolder = mxmlNewElement(plXMLRoot, "ip_output_setting");
		MULTL_XMLAddNodeText(plXMLHolder, "ip_output_type", MULTL_XMLIpOutputTypeValueToStr(plConfig->m_IpOutputType));
		MULTL_XMLAddNodeINT(plXMLHolder, "output_charset", plConfig->m_OutputCharset);
	}
#endif

	plXMLHolder = mxmlNewElement(plXMLRoot, "ntp_server");
	MULTL_XMLAddNodeText(plXMLHolder, "ipv4_addr", PFC_SocketNToA(plConfig->m_NTPServerAddr));
	MULTL_XMLAddNodeINT(plXMLHolder, "sync_interval", plConfig->m_NTPInterval);
	MULTL_XMLAddNodeText(plXMLHolder, "active_mark", MULTL_XMLMarkValueToStr(plConfig->m_NTPSyncMark));


	//GLOBAL_TRACE(("Trap IP = %x\n", plConfig->m_SNMPInitParam.m_TRAPIPAddress));

	plXMLHolder = mxmlNewElement(plXMLRoot, "snmp_manage");

	MULTL_XMLAddNodeText(plXMLHolder, "snmp_active_mark", MULTL_XMLMarkValueToStr(plConfig->m_SNMPGlobalMark));
	MULTL_XMLAddNodeText(plXMLHolder, "trap_active_mark", MULTL_XMLMarkValueToStr(plConfig->m_SNMPInitParam.m_TRAPGlobalMark));
	MULTL_XMLAddNodeText(plXMLHolder, "trap_ipv4_addr", PFC_SocketNToA(plConfig->m_SNMPInitParam.m_TRAPIPAddress));
	MULTL_XMLAddNodeINT(plXMLHolder, "trap_ip_port", plConfig->m_SNMPInitParam.m_TRAPPort);
	MULTL_XMLAddNodeINT(plXMLHolder, "trap_interval", plConfig->m_SNMPInitParam.m_NormalTrapInterval);

	MULTL_XMLAddNodeINT(plXMLHolder, "agent_port", plConfig->m_SNMPInitParam.m_AgentPort);

	MULTL_XMLAddNodeCDData(plXMLHolder, "snmp_agent_readcommuniy", plConfig->m_SNMPInitParam.m_pROCommunity);
	MULTL_XMLAddNodeCDData(plXMLHolder, "snmp_agent_writecommuniy", plConfig->m_SNMPInitParam.m_pRWCommunity);
	MULTL_XMLAddNodeCDData(plXMLHolder, "snmp_device_name", plConfig->m_SNMPInitParam.m_pDeviceName);
	MULTL_XMLAddNodeCDData(plXMLHolder, "snmp_device_location", plConfig->m_SNMPInitParam.m_pDeviceLocation);


#if 0//目前不需要支持IGMP
	plXMLHolder = mxmlNewElement(plXMLRoot, "igmp");
	plXMLData = mxmlNewElement(plXMLHolder, "igmp_version");
	mxmlNewTextf(plXMLData, 0, "%d", plConfig->m_IGMPVersion);

	plXMLData = mxmlNewElement(plXMLHolder, "igmp_interval");
	mxmlNewTextf(plXMLData, 0, "%d", plConfig->m_IGMPInterval);

	plXMLData = mxmlNewElement(plXMLHolder, "igmp_auto_mark");
	mxmlNewTextf(plXMLData, 0, "%s", MULTL_XMLMarkValueToStr(plConfig->m_IGMPAutoMark));
#endif

#ifdef SUPPORT_SYSLOG_MODULE
	{
		mxml_node_t *plXMLLevelHolder;
		MULT_Syslog *plTmpSyslog;
		plTmpSyslog = &pHandle->m_SyslogParam;
		plXMLHolder = mxmlNewElement(plXMLRoot, "syslog_manage");

		plXMLLevelHolder = mxmlNewElement(plXMLHolder, "level");
		MULTL_XMLAddNodeUINT(plXMLLevelHolder, "value", PL_SYSLOG_PRIORITY_DEBUG);
		MULTL_XMLAddNodeText(plXMLLevelHolder, "txt", "DEBUG");

		plXMLLevelHolder = mxmlNewElement(plXMLHolder, "level");
		MULTL_XMLAddNodeUINT(plXMLLevelHolder, "value", PL_SYSLOG_PRIORITY_INFO);
		MULTL_XMLAddNodeText(plXMLLevelHolder, "txt", "INFO");

		plXMLLevelHolder = mxmlNewElement(plXMLHolder, "level");
		MULTL_XMLAddNodeUINT(plXMLLevelHolder, "value", PL_SYSLOG_PRIORITY_NOTICE);
		MULTL_XMLAddNodeText(plXMLLevelHolder, "txt", "NOTICE");

		plXMLLevelHolder = mxmlNewElement(plXMLHolder, "level");
		MULTL_XMLAddNodeUINT(plXMLLevelHolder, "value", PL_SYSLOG_PRIORITY_WARNING);
		MULTL_XMLAddNodeText(plXMLLevelHolder, "txt", "WARNING");

		plXMLLevelHolder = mxmlNewElement(plXMLHolder, "level");
		MULTL_XMLAddNodeUINT(plXMLLevelHolder, "value", PL_SYSLOG_PRIORITY_ERR);
		MULTL_XMLAddNodeText(plXMLLevelHolder, "txt", "ERROR");

		plXMLLevelHolder = mxmlNewElement(plXMLHolder, "level");
		MULTL_XMLAddNodeUINT(plXMLLevelHolder, "value", PL_SYSLOG_PRIORITY_CRIT);
		MULTL_XMLAddNodeText(plXMLLevelHolder, "txt", "CRITTCAL");

		plXMLLevelHolder = mxmlNewElement(plXMLHolder, "level");
		MULTL_XMLAddNodeUINT(plXMLLevelHolder, "value", PL_SYSLOG_PRIORITY_ALERT);
		MULTL_XMLAddNodeText(plXMLLevelHolder, "txt", "ALERT");

		plXMLLevelHolder = mxmlNewElement(plXMLHolder, "level");
		MULTL_XMLAddNodeUINT(plXMLLevelHolder, "value", PL_SYSLOG_PRIORITY_EMERG);
		MULTL_XMLAddNodeText(plXMLLevelHolder, "txt", "EMERGENCY");

		MULTL_XMLAddNodeText(plXMLHolder, "syslog_active_mark", MULTL_XMLMarkValueToStr(plTmpSyslog->m_SyslogGlobalMark));
		MULTL_XMLAddNodeText(plXMLHolder, "remote_active_mark", MULTL_XMLMarkValueToStr(plTmpSyslog->m_RemoteMark));
		MULTL_XMLAddNodeText(plXMLHolder, "remote_ipv4_addr", PFC_SocketNToA(plTmpSyslog->m_RemoteAddr));
		MULTL_XMLAddNodeINT(plXMLHolder, "remote_ip_port", plTmpSyslog->m_RemotePort);
		MULTL_XMLAddNodeINT(plXMLHolder, "log_level", plTmpSyslog->m_LogLevel);
	}
#endif

	/*参数备份信息*/
	{
		CHAR_T plCMD[1024];
		CHAR_T plInfo[256];
		GLOBAL_SPRINTF((plCMD, "%s%s", MULT_STORAGE_BASE_DIR, MULT_PARAMETER_BAKU_PATHNAME));
		/*解开参数文件*/
		GLOBAL_ZEROMEM(plInfo, sizeof(plInfo));
		if (MULTL_ValidationParameter(pHandle, plCMD, plInfo, sizeof(plInfo)))
		{
			MULTL_XMLAddNodeINT(plXMLRoot, "backup_parameter", 1);
			MULTL_XMLAddNodeCDData(plXMLRoot, "backup_description", plInfo);
		}
		else
		{
			MULTL_XMLAddNodeINT(plXMLRoot, "backup_parameter", 0);
			MULTL_XMLAddNodeCDData(plXMLRoot, "backup_description", "   ");
		}
	}


	{
		CHAR_T		plPathStr[1024];
		GLOBAL_FD	plFile;

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_DEVICE_PARAMETER_XML);
		plFile = GLOBAL_FOPEN(plPathStr, "w");
		if (plFile != NULL)
		{
			mxmlSaveFile(plXML, plFile, NULL);
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
	}

	mxmlDelete(plXML);
	plXML = NULL;

	MULTL_SetSaveMark(pHandle, FALSE);
}



void MULTL_LoadConfigurationXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	MULT_Config *plConfig;
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	mxml_node_t *plXMLHolder;
	//mxml_node_t *plXMLData;

	plConfig = &pHandle->m_Configuration;

	MULTL_DefaultConfiguration(pHandle, pHWInfo);//首先填充默认值！
	{
		CHAR_T plPathStr[1024], *plTmpStr;
		GLOBAL_FD plFile;

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_DEVICE_PARAMETER_XML);

		plFile = GLOBAL_FOPEN(plPathStr, "r");
		if (plFile != NULL)
		{
			plXML = mxmlLoadFile(NULL, plFile,  MXML_OPAQUE_CALLBACK);
			if (plXML)
			{
				/*分析并校验参数文件，容许XML文件缺少部分属性以应对系统升级！！！*/
				plXMLRoot = mxmlFindElement(plXML, plXML, "root", NULL, NULL, MXML_DESCEND_FIRST);
				if (plXMLRoot)
				{
					plTmpStr = MULTL_XMLGetNodeText(plXMLRoot, "version_number");
					if (plTmpStr)
					{
						S32 lVersionNumber = 0;
						lVersionNumber = GLOBAL_STRTOL(plTmpStr, NULL, 10);
						GLOBAL_TRACE(("System Config Version Number = %d\n", lVersionNumber));
						if (lVersionNumber >= 0)
						{

							plConfig->m_WebLanguage = MULTL_XMLGetNodeINT(plXMLRoot, "language", 10);
							plConfig->m_FrpLanguage = MULTL_XMLGetNodeINT(plXMLRoot, "frp_language", 10);

							plXMLHolder = mxmlFindElement(plXMLRoot, plXMLRoot, "manage_port", NULL, NULL, MXML_DESCEND_FIRST);
							if (plXMLHolder)
							{
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "ipv4_addr");
								if (plTmpStr)
								{
									plConfig->m_ManageIPv4Addr = PFC_SocketAToN(plTmpStr);
								}

								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "ipv4_mask");
								if (plTmpStr)
								{
									plConfig->m_ManageIPv4Mask = PFC_SocketAToN(plTmpStr);
								}

								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "ipv4_gate");
								if (plTmpStr)
								{
									plConfig->m_ManageIPv4Gate = PFC_SocketAToN(plTmpStr);
								}

								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "ip_mac");
								if (plTmpStr)
								{
									CAL_StringMACToBin(plTmpStr, plConfig->m_pMAC, 6);
								}
#ifdef MULT_SUPPORT_FPGA_ETH
								{
									TUN_InitParam *plTunParam = MULT_FPGAEthGetParameterPtr();
									plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "data_ipv4_addr");
									if (plTmpStr)
									{
										plTunParam->m_TUNIPAddr = PFC_SocketAToN(plTmpStr);
									}

									plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "data_ipv4_mask");
									if (plTmpStr)
									{
										plTunParam->m_TUNIPMask = PFC_SocketAToN(plTmpStr);
									}

									plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "data_ipv4_gate");
									if (plTmpStr)
									{
										plTunParam->m_TUNIPGate = PFC_SocketAToN(plTmpStr);
									}

									plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "data_ip_mac");
									if (plTmpStr)
									{
										CAL_StringMACToBin(plTmpStr, plTunParam->m_TUNMAC, 6);
									}
								}
#endif
							}
							else
							{
								GLOBAL_TRACE(("Error No Manage Port Data In XML!!!!!!!!!!!!!!!!\n"));
							}

#ifdef GN1846
							plXMLHolder = mxmlFindElement(plXMLRoot, plXMLRoot, "ip_output_setting", NULL, NULL, MXML_DESCEND_FIRST);
							if (plXMLHolder) {
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "ip_output_type");
								if (plTmpStr) {
									plConfig->m_IpOutputType = MULTL_XMLIpOutputTypeValueFromStr(plTmpStr);
								}

								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "output_charset");
								if (plTmpStr) {
									plConfig->m_OutputCharset = atoi(plTmpStr);
								}
							}
#endif

							plXMLHolder = mxmlFindElement(plXMLRoot, plXMLRoot, "ntp_server", NULL, NULL, MXML_DESCEND_FIRST);
							if (plXMLHolder)
							{
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "ipv4_addr");
								if (plTmpStr)
								{
									plConfig->m_NTPServerAddr = PFC_SocketAToN(plTmpStr);
								}

								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "sync_interval");
								if (plTmpStr)
								{
									plConfig->m_NTPInterval = GLOBAL_STRTOL(plTmpStr, NULL, 10);
								}

								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "active_mark");
								if (plTmpStr)
								{
									plConfig->m_NTPSyncMark = MULTL_XMLMarkValueFromStr(plTmpStr);
								}
							}
							else
							{
								GLOBAL_TRACE(("Error No NTP Server Data In XML!!!!!!!!!!!!!!!!\n"));
							}

							plXMLHolder = mxmlFindElement(plXMLRoot, plXMLRoot, "snmp_manage", NULL, NULL, MXML_DESCEND_FIRST);
							if (plXMLHolder)
							{
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "snmp_active_mark");
								if (plTmpStr)
								{
									plConfig->m_SNMPGlobalMark = MULTL_XMLMarkValueFromStr(plTmpStr);
								}
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "trap_active_mark");
								if (plTmpStr)
								{
									plConfig->m_SNMPInitParam.m_TRAPGlobalMark = MULTL_XMLMarkValueFromStr(plTmpStr);
								}
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "trap_ipv4_addr");
								if (plTmpStr)
								{
									plConfig->m_SNMPInitParam.m_TRAPIPAddress = PFC_SocketAToN(plTmpStr);
								}
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "trap_ip_port");
								if (plTmpStr)
								{
									plConfig->m_SNMPInitParam.m_TRAPPort = GLOBAL_STRTOL(plTmpStr, NULL, 10);
								}

								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "trap_interval");
								if (plTmpStr)
								{
									plConfig->m_SNMPInitParam.m_NormalTrapInterval = GLOBAL_STRTOL(plTmpStr, NULL, 10);
								}
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "agent_port");
								if (plTmpStr)
								{
									plConfig->m_SNMPInitParam.m_AgentPort = GLOBAL_STRTOL(plTmpStr, NULL, 10);
								}		
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "snmp_agent_readcommuniy");					

								if (plTmpStr)
								{
									if (GLOBAL_STRLEN(plTmpStr) < MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN)
									{
										GLOBAL_STRCPY(plConfig->m_SNMPInitParam.m_pROCommunity, plTmpStr);
									}
								}		
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "snmp_agent_writecommuniy");
								if (plTmpStr)
								{
									if (GLOBAL_STRLEN(plTmpStr) < MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN)
									{
										GLOBAL_STRCPY(plConfig->m_SNMPInitParam.m_pRWCommunity, plTmpStr);
									}
								}
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "snmp_device_name");					

								if (plTmpStr)
								{
									if (GLOBAL_STRLEN(plTmpStr) < MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN)
									{
										GLOBAL_STRCPY(plConfig->m_SNMPInitParam.m_pDeviceName, plTmpStr);
									}
								}		
								plTmpStr = MULTL_XMLGetNodeText(plXMLHolder, "snmp_device_location");
								if (plTmpStr)
								{
									if (GLOBAL_STRLEN(plTmpStr) < MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN)
									{
										GLOBAL_STRCPY(plConfig->m_SNMPInitParam.m_pDeviceLocation, plTmpStr);
									}
								}		
							}
							else
							{
								GLOBAL_TRACE(("Error No SNMP Data In XML!!!!!!!!!!!!!!!!\n"));
							}

#ifdef SUPPORT_SYSLOG_MODULE
							{
								MULT_Syslog *plTmpSyslog;
								plTmpSyslog = &pHandle->m_SyslogParam;
								plXMLHolder = mxmlFindElement(plXMLRoot, plXMLRoot, "syslog_manage", NULL, NULL, MXML_DESCEND_FIRST);
								MULTL_XMLLoadSyslog(pHandle, plXMLHolder);
							}
#endif

						}
					}
				}

				mxmlDelete(plXML);
				plXML = NULL;
			}
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
		else
		{
			GLOBAL_TRACE(("No Config File\n"));
		}

	}
	MULTL_SaveConfigurationXML(pHandle);
}

/*维护信息*/
void MULTL_DefaultMaintenace(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	MULT_Maintenace *plMaintenance;
	plMaintenance = &pHandle->m_MaintSetting;
	GLOBAL_ZEROMEM(plMaintenance, sizeof(MULT_Maintenace));

#ifdef GN1846
	S32 i;

	for (i = 0; i < MULT_MAX_CHN_NUM; i++) {
		plMaintenance->m_AudPtsRelativeDelayTime[i] = -250000;
		plMaintenance->m_PtsDelayTime[i] = 500000;
		plMaintenance->m_MaxPtsPcrInterval[i] = 50000;
		plMaintenance->m_MinPtsPcrInterval[i] = -50000;
		plMaintenance->m_AudDelayFrameNum[i] = 15;
	}
#endif
}

void MULTL_SaveMaintenaceXML(MULT_Handle *pHandle)
{
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	MULT_Maintenace *plMaintenance;

	plMaintenance = &pHandle->m_MaintSetting;

	plXML = mxmlNewXML("1.0");
	plXMLRoot = mxmlNewElement(plXML, "root");

	MULTL_XMLAddNodeINT(plXMLRoot, "pll_freq_offset", plMaintenance->m_PLLFreqOffset);
	MULTL_XMLAddNodeUINT(plXMLRoot, "power_up_count", plMaintenance->m_PowerUpCount);
	MULTL_XMLAddNodeUINT(plXMLRoot, "clk10m_da_value", plMaintenance->m_10MPLLDAValue);
	MULTL_XMLAddNodeFLOAT(plXMLRoot, "clk10m_trainning_value", plMaintenance->m_10MTrainningValue);
	MULTL_XMLAddNodeMark(plXMLRoot, "private_channel_setup_mark", plMaintenance->m_PrivateChannelSetupMark);

#ifdef GN1846
	S32 i;
	mxml_node_t *plXMLNode;

	for (i = 0; i < MULT_MAX_CHN_NUM; i++) {
		plXMLNode = mxmlNewElement(plXMLRoot, "enc_adjust_param");
		MULTL_XMLAddNodeINT(plXMLNode, "aud_pts_relative_delay_time", plMaintenance->m_AudPtsRelativeDelayTime[i]);
		MULTL_XMLAddNodeINT(plXMLNode, "pts_delay_time", plMaintenance->m_PtsDelayTime[i]);
		MULTL_XMLAddNodeINT(plXMLNode, "max_pts_pcr_interval", plMaintenance->m_MaxPtsPcrInterval[i]);
		MULTL_XMLAddNodeINT(plXMLNode, "min_pts_pcr_interval", plMaintenance->m_MinPtsPcrInterval[i]);
		MULTL_XMLAddNodeINT(plXMLNode, "aud_delay_frame_num", plMaintenance->m_AudDelayFrameNum[i]);
	}
#endif

	{
		CHAR_T		plPathStr[1024];
		GLOBAL_FD	plFile;

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_DEVICE_MAINTENANCE_XML);

		plFile = GLOBAL_FOPEN(plPathStr, "w");
		if (plFile != NULL)
		{
			mxmlSaveFile(plXML, plFile, NULL);
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
	}

	mxmlDelete(plXML);
	plXML = NULL;

	//保存到FLASH
	MULTL_SaveMaintenaceToStorage(pHandle);
}



void MULTL_LoadMaintenaceXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	MULT_Maintenace *plMaintenance;

	plMaintenance = &pHandle->m_MaintSetting;

	MULTL_DefaultMaintenace(pHandle, pHWInfo);//首先填充默认值！
	{
		CHAR_T plPathStr[1024];
		GLOBAL_FD plFile;

		/*从FLASH中读取数据展开到临时目录下*/
		GLOBAL_STRCPY(plPathStr, MULT_STORAGE_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_MAINTENACE_FILE_PATHNAME);
		MULTL_LoadMaintenaceFromStorage(pHandle, plPathStr);


		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_DEVICE_MAINTENANCE_XML);

		plFile = GLOBAL_FOPEN(plPathStr, "r");
		if (plFile != NULL)
		{
			plXML = mxmlLoadFile(NULL, plFile,  MXML_OPAQUE_CALLBACK);
			if (plXML)
			{
				/*分析并校验参数文件，容许XML文件缺少部分属性以应对系统升级！！！*/
				plXMLRoot = mxmlFindElement(plXML, plXML, "root", NULL, NULL, MXML_DESCEND_FIRST);
				if (plXMLRoot)
				{
					plMaintenance->m_PLLFreqOffset = MULTL_XMLGetNodeINT(plXMLRoot, "pll_freq_offset", 10);
					plMaintenance->m_PowerUpCount = MULTL_XMLGetNodeUINT(plXMLRoot, "power_up_count", 10);
					plMaintenance->m_10MPLLDAValue = MULTL_XMLGetNodeUINT(plXMLRoot, "clk10m_da_value", 10);
					plMaintenance->m_10MTrainningValue = MULTL_XMLGetNodeFLOATDefault(plXMLRoot, "clk10m_trainning_value", 0.0);
					plMaintenance->m_PrivateChannelSetupMark = MULTL_XMLGetNodeMark(plXMLRoot, "private_channel_setup_mark");

					GLOBAL_TRACE(("PLL Offset = %d\n", plMaintenance->m_PLLFreqOffset));
					GLOBAL_TRACE(("PowerUP Count = %d\n", plMaintenance->m_PowerUpCount));
					GLOBAL_TRACE(("DAValue = 0x%.4X\n", plMaintenance->m_10MPLLDAValue));
					GLOBAL_TRACE(("Tranning Value = %f\n", plMaintenance->m_10MTrainningValue));

					plMaintenance->m_PowerUpCount++;//自动+1

#ifdef GN1846
					S32 i = 0;
					mxml_node_t *plXMLNode;

					plXMLNode = mxmlFindElement(plXMLRoot, plXMLRoot, "enc_adjust_param", NULL, NULL, MXML_DESCEND_FIRST);
					while (plXMLNode) {
						if (MULTL_XMLGetNodeText(plXMLNode, "aud_pts_relative_delay_time")) {
							plMaintenance->m_AudPtsRelativeDelayTime[i] = MULTL_XMLGetNodeINT(plXMLNode, "aud_pts_relative_delay_time", 10);
						}
						if (MULTL_XMLGetNodeText(plXMLNode, "pts_delay_time")) {
							plMaintenance->m_PtsDelayTime[i] = MULTL_XMLGetNodeINT(plXMLNode, "pts_delay_time", 10);
						}
						if (MULTL_XMLGetNodeText(plXMLNode, "max_pts_pcr_interval")) {
							plMaintenance->m_MaxPtsPcrInterval[i] = MULTL_XMLGetNodeINT(plXMLNode, "max_pts_pcr_interval", 10);
						}
						if (MULTL_XMLGetNodeText(plXMLNode, "min_pts_pcr_interval")) {
							plMaintenance->m_MinPtsPcrInterval[i] = MULTL_XMLGetNodeINT(plXMLNode, "min_pts_pcr_interval", 10);
						}
						if (MULTL_XMLGetNodeText(plXMLNode, "aud_delay_frame_num")) {
							plMaintenance->m_AudDelayFrameNum[i] = MULTL_XMLGetNodeINT(plXMLNode, "aud_delay_frame_num", 10);
						}
						GLOBAL_TRACE(("CHN[%d] aud_pts_relative_delay_time = %d us\n", i, plMaintenance->m_AudPtsRelativeDelayTime[i]));
						GLOBAL_TRACE(("CHN[%d] pts_delay_time = %d us\n", i, plMaintenance->m_PtsDelayTime[i]));
						GLOBAL_TRACE(("CHN[%d] max_pts_pcr_interval = %d us\n", i, plMaintenance->m_MaxPtsPcrInterval[i]));
						GLOBAL_TRACE(("CHN[%d] min_pts_pcr_interval = %d us\n", i, plMaintenance->m_MinPtsPcrInterval[i]));
						GLOBAL_TRACE(("CHN[%d] aud_delay_frame_num = %d\n", i, plMaintenance->m_AudDelayFrameNum[i]));
						i++;
						plXMLNode = mxmlFindElement(plXMLNode, plXMLRoot, "enc_adjust_param", NULL, NULL, MXML_NO_DESCEND);
					}
#endif
				}
				mxmlDelete(plXML);
				plXML = NULL;
			}
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
		else
		{
			GLOBAL_TRACE(("No Maintenace File\n"));
		}

	}


	GLOBAL_TRACE(("Done!\n"));
	MULTL_SaveMaintenaceXML(pHandle);
}

/*设备功能参数*/
void MULTL_DefaultParameter(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{

	S32 i, k, m, lInChnInd, lOunChnInd, lIPCurCount;
	MULT_Parameter *plSystemParam;
	MULT_ChannelNode *plChnNode, *plChnNodeTmp;
	MULT_SubChannelNode *plSubNode, *plSubNodeTmp;
	HANDLE32 lDBSHandle;

#ifdef SUPPORT_NEW_HWL_MODULE
	S32 lAllETHCount;
#endif

	lDBSHandle = pHandle->m_DBSHandle;


	GLOBAL_TRACE(("Default Parameter!!!!!!!\n"));

#ifdef SUPPORT_NEW_HWL_MODULE
	pHandle->m_TunerCount = 0;
#else
	pHandle->m_TunerCount = pHWInfo->m_TunerCount;
	pHandle->m_TunerType = pHWInfo->m_TunerType;
#endif


	pHandle->m_BSSystemInfo.m_SuperCASID = 0x00050000;
	pHandle->m_BSSystemInfo.m_ActiveMark =FALSE;
	for(i = 0; i< BSS_SESSION_WORD_LENGTH; i++)
	{
		pHandle->m_BSSystemInfo.m_pSW[i] = 0x00;
		pHandle->m_BSSystemInfo.m_pKey[i] = 0x00;

	}

	plSystemParam = &pHandle->m_Parameter;

	{
		MULT_IGMP *plIGMP;
		plIGMP = &plSystemParam->m_IGMP;
		plIGMP->m_IGMPRepeateMark = FALSE;
		plIGMP->m_IGMPRepeateTime = 30;
		plIGMP->m_IGMPVersion = 2;
	}



#ifdef MULT_SYSTEM_HAVE_PCR_CORRECT_ADJUST_FUNCTION
	plSystemParam->m_PCRCorrect.m_PCRCMark = FALSE;
	plSystemParam->m_PCRCorrect.m_PCRCPos = MULT_PCR_CORRECT_POS_DEFAULT_VALUE;
	plSystemParam->m_PCRCorrect.m_PCRCNeg = MULT_PCR_CORRECT_NEG_DEFAULT_VALUE;
#endif

#ifdef SUPPORT_NEW_HWL_MODULE
	plSystemParam->m_InChannelNumber = pHWInfo->m_InChnNum;
	plSystemParam->m_OutChannelNumber = pHWInfo->m_OutChnNum;

	plSystemParam->m_pInChannel = (MULT_ChannelNode *)GLOBAL_ZMALLOC(sizeof(MULT_ChannelNode) * plSystemParam->m_InChannelNumber);
	plSystemParam->m_pOutChannel = (MULT_ChannelNode *)GLOBAL_ZMALLOC(sizeof(MULT_ChannelNode) * plSystemParam->m_OutChannelNumber);

	lIPCurCount = lInChnInd = lOunChnInd = 0;

	lAllETHCount = 0;

	for (i = 0; i < plSystemParam->m_InChannelNumber + plSystemParam->m_OutChannelNumber; i++)
	{
		HWL_ChannelInfo *plHWLChnInfo;
		if (i < plSystemParam->m_InChannelNumber)
		{
			plChnNode = &plSystemParam->m_pInChannel[lInChnInd];
			plHWLChnInfo = &pHWInfo->m_pInChn[lInChnInd];
			lInChnInd++;
		}
		else
		{
			plChnNode = &plSystemParam->m_pOutChannel[lOunChnInd];
			plHWLChnInfo = &pHWInfo->m_pOutChn[lOunChnInd];
			lOunChnInd++;
		}

		plChnNode->m_ChannelType = plHWLChnInfo->m_Type;
		plChnNode->m_SubType = plHWLChnInfo->m_SubType;
		plChnNode->m_DemodType = 0;
		plChnNode->m_SubChannelNumber = plHWLChnInfo->m_CurSubSupport;
		plChnNode->m_pSubChannelNode = (MULT_SubChannelNode *)GLOBAL_ZMALLOC(sizeof(MULT_SubChannelNode) * plChnNode->m_SubChannelNumber);
		plChnNode->m_OriginalIndex = i;

		switch (plChnNode->m_ChannelType)
		{
		case HWL_CHANNEL_TYPE_IP:
		case HWL_CHANNEL_TYPE_IP_LOOP:
			{
				lIPCurCount++;
			}
		case HWL_CHANNEL_TYPE_IP_LOOP_DEP:
		case HWL_CHANNEL_TYPE_IP_DEP:
			{
				lAllETHCount++;
				plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress = MULTI_DEFAULT_IP_ADDR + lIPCurCount;
				plChnNode->m_ChannelInfo.m_IPInfo.m_IPMask = MULTI_DEFAULT_IP_MASK;
				plChnNode->m_ChannelInfo.m_IPInfo.m_IPGate = MULTI_DEFAULT_IP_GATE;
				plChnNode->m_ChannelInfo.m_IPInfo.m_Bitrate = MULT_IP_CHN_DEFAULT_BITRATE;
				MULTL_GenerateMAC(MULT_DEVICE_TYPE, MULT_DEVICE_SUB_TYPE, lIPCurCount, pHWInfo->m_ChipID, plChnNode->m_ChannelInfo.m_IPInfo.m_MAC, GLOBAL_MAC_BUF_SIZE);

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					if ((k < 16) && (plChnNode->m_ChannelType != HWL_CHANNEL_TYPE_IP_LOOP_DEP)  && (plChnNode->m_ChannelType != HWL_CHANNEL_TYPE_IP_LOOP))
					{
						plSubNode->m_ActiveMark = TRUE;
					}
					else
					{
						plSubNode->m_ActiveMark = FALSE;
					}
					if (i < pHWInfo->m_InChnNum)
					{
						plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr = plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress;
						plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Port = MULTI_DEFAULT_SUB_IP_PORT + k ;
					}
					else
					{
						plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr = MULTI_DEFAULT_SUB_IP_ADDR + k + (lAllETHCount - lIPCurCount) * 256;
						plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Port = MULTI_DEFAULT_SUB_IP_PORT + 1000;
					}
					plSubNode->m_SubInfo.m_SubIPInfo.m_Protocol = GS_ETH_PROTOCOL_UDP;
					plSubNode->m_Bitrate = MULT_IP_SUB_DEFAULT_OUTPUT_BITRATE;
					plSubNode->m_CorrespondTsIndex = plHWLChnInfo->m_StartTsIndex + k ;
				}
			}
			break;
		case HWL_CHANNEL_TYPE_ASI:
			{
				plChnNode->m_ChannelInfo.m_ASI.m_Bitrate = plChnNode->m_SubChannelNumber * MULT_ASI_MAX_OUTPUT_BITRATE;
				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;
					plSubNode->m_CorrespondTsIndex = plHWLChnInfo->m_StartTsIndex + k;
					plSubNode->m_Bitrate = MULT_ASI_MAX_OUTPUT_BITRATE;
				}
			}
			break;
		default:
			{
				GLOBAL_TRACE(("Unknown Type = %d\n", plChnNode->m_ChannelType));
			}
			break;
		}
	}
#else
	plSystemParam->m_InChannelNumber = 0;
	plSystemParam->m_OutChannelNumber = 0;
	for (i = 0; i < pHWInfo->m_ChannelNum; i++)
	{
		if (pHWInfo->m_pInfoList[i].m_Direction == HWL_CHANNEL_DIRECTION_IN)
		{
			plSystemParam->m_InChannelNumber++;
		}
		else
		{
			plSystemParam->m_OutChannelNumber++;
		}
	}

	plSystemParam->m_pInChannel = (MULT_ChannelNode *)GLOBAL_ZMALLOC(sizeof(MULT_ChannelNode) * plSystemParam->m_InChannelNumber);
	plSystemParam->m_pOutChannel = (MULT_ChannelNode *)GLOBAL_ZMALLOC(sizeof(MULT_ChannelNode) * plSystemParam->m_OutChannelNumber);

	lIPCurCount = lInChnInd = lOunChnInd = 0;
	for (i = 0; i < pHWInfo->m_ChannelNum; i++)
	{
		HWL_ChannelInfo *plHWLChnInfo;
		if (pHWInfo->m_pInfoList[i].m_Direction == HWL_CHANNEL_DIRECTION_IN)
		{
			plChnNode = &plSystemParam->m_pInChannel[lInChnInd];
			lInChnInd++;
		}
		else
		{
			plChnNode = &plSystemParam->m_pOutChannel[lOunChnInd];
			lOunChnInd++;
		}
		plChnNode->m_ChannelType = pHWInfo->m_pInfoList[i].m_Type;
		plChnNode->m_SubType = pHWInfo->m_pInfoList[i].m_SubType;
		plChnNode->m_DemodType = pHWInfo->m_pInfoList[i].m_DemodType;
		plChnNode->m_SubChannelNumber = pHWInfo->m_pInfoList[i].m_CurSubSupport;
		plChnNode->m_pSubChannelNode = (MULT_SubChannelNode *)GLOBAL_ZMALLOC(sizeof(MULT_SubChannelNode) * plChnNode->m_SubChannelNumber);
		plChnNode->m_OriginalIndex = i;

		switch (plChnNode->m_ChannelType)
		{
		case HWL_CHANNEL_TYPE_ASI:
			{
				if (pHWInfo->m_pInfoList[i].m_Direction == HWL_CHANNEL_DIRECTION_IN)
				{
					plChnNode->m_ChannelInfo.m_ASI.m_Bitrate = plChnNode->m_SubChannelNumber * MULT_ASI_MAX_BITRATE;
				}
				else
				{
					plChnNode->m_ChannelInfo.m_ASI.m_Bitrate = plChnNode->m_SubChannelNumber * MULT_ASI_DEFAULT_OUTPUT_BITRATE;
				}
				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k;
					if (pHWInfo->m_pInfoList[i].m_Direction == HWL_CHANNEL_DIRECTION_IN)
					{
						plSubNode->m_Bitrate = MULT_ASI_MAX_BITRATE;
					}
					else
					{
						plSubNode->m_Bitrate = MULT_ASI_DEFAULT_OUTPUT_BITRATE;
					}
				}
			}
			break;
		case HWL_CHANNEL_TYPE_E3DS3:
			{

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;

					plSubNode->m_SubInfo.m_SubESDS3Info.m_E3DS3 = GS_E3DS3_SELECT_E3;
					plSubNode->m_SubInfo.m_SubESDS3Info.m_Bitorder = GS_E3DS3_BITORDER_MSB;
					plSubNode->m_SubInfo.m_SubESDS3Info.m_Frameform = GS_E3DS3_FRAMEFORM_YES;
					plSubNode->m_SubInfo.m_SubESDS3Info.m_Packetlength = GS_E3DS3_PACKETLENGTH_204;
					plSubNode->m_SubInfo.m_SubESDS3Info.m_Scramble = FALSE;
					plSubNode->m_SubInfo.m_SubESDS3Info.m_RSCoding = FALSE;
					plSubNode->m_SubInfo.m_SubESDS3Info.m_InterweaveCoding = FALSE;
					plSubNode->m_SubInfo.m_SubESDS3Info.m_RateRecover = FALSE;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k;
				}
			}
			break;
		case HWL_CHANNEL_TYPE_TUNER_S:
			{
				GLOBAL_TRACE(("Tuner S Count = %d\n", plChnNode->m_SubChannelNumber));

				plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsLow = 950000;
				plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsHigh = 2150000;
				plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsLow = 1000000;//2000000;
				plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsHigh = 45000000; //35500000;

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;					
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq = 3840000;   
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_LocalFreq = 5150000;  /*------单位（k）-------*/
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_SymbolRate = 27500000;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_PolarMethod = GS_TUNER_PLOAR_NONE;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_22kSwitch = FALSE;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv = GS_SPECINV_OFF;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
				}
			}
			break;
		case HWL_CHANNEL_TYPE_TUNER_C:
			{
				plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsLow = 12000;
				plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsHigh = 950000;
				plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsLow = 2000000;
				plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsHigh = 7000000;

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq = 474000;  /*------单位（k）-------*/
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_SymbolRate = 6875000;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_Modulation = GS_MODULATOR_QAM_64;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_Reqtype = GS_TUNER_ANNEX_A;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv = GS_SPECINV_OFF;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
				}
			}
			break;
		case HWL_CHANNEL_TYPE_TUNER_DTMB:
			{
				plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsLow = 12000;
				plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsHigh = 950000;
				plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsLow = 756000;
				plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsHigh = 756000;

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq = 474000;  /*------单位（k）-------*/
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_SymbolRate = 756000;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_Modulation = GS_MODULATOR_QAM_64;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_Reqtype = GS_TUNER_ANNEX_A;
					plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv = GS_SPECINV_OFF;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
				}
			}
			break;
		case HWL_CHANNEL_TYPE_IP:
		case HWL_CHANNEL_TYPE_IP_LOOP:
			{
				lIPCurCount++;
			}
		case HWL_CHANNEL_TYPE_IP_LOOP_DEP:
		case HWL_CHANNEL_TYPE_IP_DEP:
			{
				plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress = MULTI_DEFAULT_IP_ADDR + lIPCurCount;
				plChnNode->m_ChannelInfo.m_IPInfo.m_IPMask = MULTI_DEFAULT_IP_MASK;
				plChnNode->m_ChannelInfo.m_IPInfo.m_IPGate = MULTI_DEFAULT_IP_GATE;
				plChnNode->m_ChannelInfo.m_IPInfo.m_Bitrate = MULT_IP_CHN_DEFAULT_BITRATE;
				plChnNode->m_ChannelInfo.m_IPInfo.m_IPMark = TRUE;
				MULTL_GenerateMAC(MULT_DEVICE_TYPE, MULT_DEVICE_SUB_TYPE, lIPCurCount, pHWInfo->m_ChipID, plChnNode->m_ChannelInfo.m_IPInfo.m_MAC, GLOBAL_MAC_BUF_SIZE);

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					if ((k < 16) && (plChnNode->m_ChannelType != HWL_CHANNEL_TYPE_IP_LOOP_DEP)  && (plChnNode->m_ChannelType != HWL_CHANNEL_TYPE_IP_LOOP))
					{
						plSubNode->m_ActiveMark = TRUE;
					}
					else
					{
						plSubNode->m_ActiveMark = FALSE;
					}
					if (pHWInfo->m_pInfoList[i].m_Direction == HWL_CHANNEL_DIRECTION_IN)
					{
						plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr = plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress;
						plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Port = MULTI_DEFAULT_SUB_IP_PORT + k ;
					}
					else
					{
						plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr = MULTI_DEFAULT_SUB_IP_ADDR + k;
						plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Port = MULTI_DEFAULT_SUB_IP_PORT;
					}
					plSubNode->m_SubInfo.m_SubIPInfo.m_Protocol = GS_ETH_PROTOCOL_UDP;
					plSubNode->m_Bitrate = MULT_IP_SUB_DEFAULT_OUTPUT_BITRATE;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
				}
			}
			break;

#ifdef GM8358Q

		case HWL_CHANNEL_TYPE_ENCODER:
			{
				if (plChnNode->m_SubType == HWL_CHANNEL_SUBTYPE_ENCODER_CVBS)
				{
					for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
					{
						S32 j;
						for (j = 0; j < EncoderCvbsBoardSupportEntryNum; j++)
						{
							plSubNode = &plChnNode->m_pSubChannelNode[k];
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod = ENCODER_WORK_MODE_ENCODER;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat = ENCODER_VIDEO_FORMAT_NTSC1;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Resolution = ENCODER_VIDEO_RESOLUTION_720_480;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_FrameRate = ENCODER_FRAME_RATE_1;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_OutBitRate = ENCODER_EncoderOutBitRrateMode_CBR;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoEncodeMode = ENCODER_VideoEncodeMode_MPEG2;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoProfile = ENCODER_VideoProfileMode_High;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoAspect = ENCODER_VideoAspect3;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoBitRate = 2500;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_ImageHorizontalOffset = 20;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Brightness = 50;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Contrast = 50;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Saturation = 50;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Hue = 50;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEncodeMode = ENCODER_AudioEncodeMode1;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioBitRate = ENCODER_AudioBitRate_192;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEmbChannel = 0;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Volume = 0;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioSampleRate = ENCODER_AudioSampleRate3;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoPid = 128 + j;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioPid = 144 + j;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PcrPid = 128 + j;//160 + j;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PmtPid = 176 + j;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PidEqualSwitch = ENCODER_PID_EQUAL_SWITCH_ON;
						}
						plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
						plSubNode->m_ActiveMark = TRUE;
					}
				}
			}
			break;

#endif

#ifdef GM8398Q

		case HWL_CHANNEL_TYPE_ENCODER_CVBS_DXT8243:
			{
				if (plChnNode->m_SubType == HWL_CHANNEL_SUBTYPE_ENCODER_CVBS)
				{
					for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
					{
						S32 j;
						for (j = 0; j < EncoderCvbsBoardSupportEntryNum; j++)
						{
							plSubNode = &plChnNode->m_pSubChannelNode[k];
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod = ENCODER_WORK_MODE_ENCODER;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat = ENCODER_VIDEO_FORMAT_PAL1;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Resolution = ENCODER_VIDEO_RESOLUTION_720_576;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_OutBitRate = ENCODER_EncoderOutBitRrateMode_CBR;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoEncodeMode = ENCODER_VideoEncodeMode_MPEG2;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoProfile = ENCODER_VideoProfileMode_Main;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoAspect = ENCODER_VideoAspect3;//4:3
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoBitRate = 2500;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_ImageHorizontalOffset = 20;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Brightness = 50;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Contrast = 50;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Saturation = 50;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Hue = 50;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEncodeMode = ENCODER_AudioEncodeMode1;//MPEG-1 L2
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioBitRate = ENCODER_AudioBitRate_192;//192K
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEmbChannel = 0;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Volume = 0;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioSampleRate = ENCODER_AudioSampleRate3;//48K
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoPid = 128 + j;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioPid = 144 + j;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PcrPid = 128 + j;//160 + j;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PmtPid = 176 + j;
							plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PidEqualSwitch = ENCODER_PID_EQUAL_SWITCH_OFF;
						}
						plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
						plSubNode->m_ActiveMark = TRUE;
					}
				}
			}
			break;

#endif
#ifdef GN1846
		case HWL_CHANNEL_TYPE_ENCODER:
			{
				if (plChnNode->m_SubType == HWL_CHANNEL_SUBTYPE_ENCODER_HI3531A)
				{
					plChnNode->m_ChannelInfo.m_ENCODER.m_VidOutBitrateLimitLow = 1000;
					plChnNode->m_ChannelInfo.m_ENCODER.m_VidOutBitrateLimitHigh = 10000; 
					plChnNode->m_ChannelInfo.m_ENCODER.m_ProgBitrateLimitHigh = 15000;
					plChnNode->m_ChannelInfo.m_ENCODER.m_GopLimitLow = 1;
					plChnNode->m_ChannelInfo.m_ENCODER.m_GopLimitHigh = 30;
					plChnNode->m_ChannelInfo.m_ENCODER.m_VolLimitLow = -30;
					plChnNode->m_ChannelInfo.m_ENCODER.m_VolLimitHigh = 30;
					plChnNode->m_ChannelInfo.m_ENCODER.m_ProgNameLimitHigh = MPEG2_DB_MAX_SERVICE_NAME_LEN;
					plChnNode->m_ChannelInfo.m_ENCODER.m_ProgNumberLimitLow = 1;
					plChnNode->m_ChannelInfo.m_ENCODER.m_ProgNumberLimitHigh = 65535;
					plChnNode->m_ChannelInfo.m_ENCODER.m_PidLimitLow = 32;
					plChnNode->m_ChannelInfo.m_ENCODER.m_PidLimitHigh = 8190;
					for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
					{
						plSubNode = &plChnNode->m_pSubChannelNode[k];
						MULT_SubENCODERInfo *plEncInfo = &plSubNode->m_SubInfo.m_SubENCODERInfo;

						plEncInfo->m_ViMode = ENC_VI_MODE_AUTO;
						plEncInfo->m_VoMode = ENC_VO_MODE_AUTO;
						plEncInfo->m_BrMode = ENC_BR_MODE_CBR;
						plEncInfo->m_EncMode = ENC_MODE_H264;
						plEncInfo->m_Profile = ENC_PROFILE_MP;
						plEncInfo->m_Bitrate = 5000;
						plEncInfo->m_ProgBitrate = 8000;
						plEncInfo->m_Gop = 30;
						plEncInfo->m_AudEncMode = ENC_AUD_ENC_MODE_MPEG1_L2;
						plEncInfo->m_AudBitrate = ENC_AUD_BR_192K;
						plEncInfo->m_AudSample = ENC_AUD_SAMP_48K;
						plEncInfo->m_Volume = 0;
						GLOBAL_SNPRINTF((plEncInfo->m_pProgName, sizeof(plEncInfo->m_pProgName), "prog%d", 1 + k));
						plEncInfo->m_ProgNumber = 1 + k;
						plEncInfo->m_VidPid = 0x80 + k;
						plEncInfo->m_AudPid = 0x90 + k;
						plEncInfo->m_PcrPid = 0xa0 + k;
						plEncInfo->m_PmtPid = 0xb0 + k;
						plEncInfo->m_IPv4Addr = MULTI_DEFAULT_SUB_IP_ADDR;
						plEncInfo->m_IPv4Port = MULTI_DEFAULT_SUB_IP_PORT + k;
						plEncInfo->m_Protocol = GS_ETH_PROTOCOL_UDP;
						plEncInfo->m_ActiveMark = TRUE;

						plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
						plSubNode->m_ActiveMark = TRUE;
					}
				}
			}
			break;
#endif
		case HWL_CHANNEL_TYPE_DVB_C_MODULATOR:
			{
				if (plChnNode->m_SubType == HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789)
				{
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_AdjacentFreqNumber = 4;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsHigh = 1000000000/*860000000*/;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsLow = 12000000/*54000000*/;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_SymbolRateLimitsHigh = 7000000;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_SymbolRateLimitsLow = 4220000; //4218750 / 2;//低符号率测试！20120605
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel = 10;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevelMax = 40;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttStepping = 0.5;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttValidMark = TRUE;
					if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
					{
						plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax = 20;
					}
					else
					{
						plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax = 40;
						plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel = 0;
					}
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainStepping = 0.25;
				}

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_ITUCoding = GS_MODULATOR_STANDARD_ANNEX_A;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_AnalogBand = GS_MODULATOR_ANALOG_BAND_8M;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq = 474000000 + 8000000 * k;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_SymbolRate = 6875000;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode = GS_MODULATOR_QAM_64;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv = FALSE;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation = TRUE;
#ifdef GN2000
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq = 474000000 + 8000000 * k;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode = GS_MODULATOR_QAM_256;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_SymbolRate = 6952000;
#endif
					if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
					{
						plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel = 0;
					}
					else
					{
						plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel = 32;
					}


				}
			}
			break;
		case HWL_CHANNEL_TYPE_DVB_S_MODULATOR:
			{
				if (plChnNode->m_SubType == HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789)
				{
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_AdjacentFreqNumber = 1;

					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsHigh = 950000000U;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsLow = 70000000U;

					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_SymbolRateLimitsHigh = 32500000;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_SymbolRateLimitsLow = 2049000; //4218750 / 2;//低符号率测试！20120605

					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel = 0;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevelMax = 40;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttStepping = 0.5;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttValidMark = TRUE;

					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainStepping = 0.25;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax = 40;
				}

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;

					plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq = 950000000;//Mhz
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_SymbolRate = 27500000;//MBaud/s

					plSubNode->m_SubInfo.m_SubModulatorInfo.m_FecEncode = GS_MODULATOR_FEC_ENCODE_7_8;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel = 32;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv = FALSE;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation = TRUE;
				}
			}
			break;
		case HWL_CHANNEL_TYPE_DTMB_MODULATOR:
			{
				if (plChnNode->m_SubType == HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789)
				{
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_AdjacentFreqNumber = 4;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsHigh = 1000000000/*860000000*/;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsLow = 12000000/*54000000*/;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_SymbolRateLimitsHigh = 7560000;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_SymbolRateLimitsLow = 7560000; //4218750 / 2;//低符号率测试！20120605
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel = 10;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevelMax = 40;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttStepping = 0.5;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttValidMark = FALSE;
					if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
					{
						plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax = 20;
					}
					else
					{
						plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax = 40;
						plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel = 0;
					}
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainStepping = 0.25;

#ifdef GQ3765
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax = 59;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainStepping = 0.25;

					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel = 0;//将就这个做增益微调
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevelMax = 5;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttStepping = 0.05;
					plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttValidMark = TRUE;
#endif
				}

				for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
				{
					plSubNode = &plChnNode->m_pSubChannelNode[k];
					plSubNode->m_ActiveMark = TRUE;
					plSubNode->m_CorrespondTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex + k ;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_ITUCoding = 0;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_AnalogBand = GS_MODULATOR_ANALOG_BAND_8M;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq = 474000000 + 8000000 * k;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_SymbolRate = 7560000;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode = GS_MODULATOR_QAM_64;


					plSubNode->m_SubInfo.m_SubModulatorInfo.m_DoublePilot = FALSE;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode = GS_MODULATOR_GUARD_INTERVAL_PN_420C;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_CodeRate = GS_MODULATOR_CODE_RATE_0_8;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_TimeInterleave = GS_MODULATOR_ISDB_T_TIME_INTERLEAVER_B_52_M_240;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_CarrierMode = GS_MODULATOR_CARRIER_MODE_3780;


					plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv = FALSE;
					plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation = TRUE;
					if (HWL_GetHardwareVersion() != HWL_NEW_HARD)
					{
						plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel = 0;
					}
					else
					{
						plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel = 32;
					}
				}
			}
			break;
		}
	}
#endif


	plSystemParam->m_MaxInTsNumber = pHWInfo->m_InTsMax;
	plSystemParam->m_MaxOutTsNumber = pHWInfo->m_OutTsMax;

	/*将3710A的输出默认成直通 20120607 XJ*/
	{
		U32 lTsIDs;
		MPEG2_DBTsRouteInfo	lRouteInfo;
#if defined(GQ3710A) || defined(GQ3650DS) || defined(GQ3760B) || defined(GM2750) || defined(GQ3763)|| defined(GQ3768) || defined(GQ3765)
		lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, FALSE, 0);
		MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, FALSE, &lRouteInfo);
		lRouteInfo.m_ActiveMark = TRUE;
		lRouteInfo.m_TsIndex = 0;
		MPEG2_DBSetTsRouteInfo(lDBSHandle, lTsIDs, &lRouteInfo);
#endif

		/*不支持复用功能*/
#ifdef GC1804C
		{
			S32 i;
			for (i = 0; i < 4; i++)
			{
				lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, FALSE, i);
				MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, FALSE, &lRouteInfo);
				lRouteInfo.m_ActiveMark = TRUE;
				lRouteInfo.m_TsIndex = i;
				MPEG2_DBSetTsRouteInfo(lDBSHandle, lTsIDs, &lRouteInfo);
			}
		}
#endif

#ifdef GC1804B
		{
			S32 i;
			for (i = 0; i < 4; i++)
			{
				lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, FALSE, i);
				MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, FALSE, &lRouteInfo);
				lRouteInfo.m_ActiveMark = TRUE;
				lRouteInfo.m_TsIndex = i;
				MPEG2_DBSetTsRouteInfo(lDBSHandle, lTsIDs, &lRouteInfo);
			}
		}
#endif

	}


}


void MULTL_SaveParameterXML(MULT_Handle *pHandle)
{
	/*读取通道参数、授权信息和数据库*/
	S32 i, k;
	CHAR_T plStrBuf[1024];
	CHAR_T plPathStr[512];

	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;

	mxml_node_t *plXMLInfoHolder;
	mxml_node_t *plXMLData;

	HANDLE32 lDBSHandle;

	lDBSHandle = pHandle->m_DBSHandle;

	plXML = mxmlNewXML("1.0");
	plXMLRoot = mxmlNewElement(plXML, "root");


	MULTL_XMLAddNodeINT(plXMLRoot, "version_number", 0);
	MULTL_XMLAddNodeINT(plXMLRoot, "current_auth_signature", pHandle->m_Information.m_LicenseMode);
	MULTL_XMLAddNodeINT(plXMLRoot, "hardversion", HWL_GetHardwareVersion());


#ifdef SUPPORT_SFN_MODULATOR
	MULT_SFNXMLSave(plXMLRoot, FALSE);
#endif

#ifdef SUPPORT_SFN_ADAPTER
	MULT_SFNAXMLSave(plXMLRoot, FALSE);
#endif

#ifdef SUPPORT_IP_O_TS_MODULE
	MULT_IPoTSXMLSave(pHandle, plXMLRoot, FALSE);
#endif

#ifdef SUPPORT_GNS_MODULE
	MULT_GNSXMLSave(pHandle, plXMLRoot, FALSE);
#endif

#ifdef SUPPORT_NTS_DPD_BOARD
	MULT_NTSDPDXMLSave(plXMLRoot, FALSE);
#endif

	/*字符集支持信息*/
	{
		mxml_node_t *plXMLCharsetRoot;
		mxml_node_t *plXMLHolder;
		plXMLCharsetRoot = mxmlNewElement(plXMLRoot, "char_sets");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "char_set");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_PSI_CHAR_SET_ISO_8859_0);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "ASCII");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "char_set");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_PSI_CHAR_SET_ISO_8859_5);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "ISO 8859-5 Cyrillic");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "char_set");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_PSI_CHAR_SET_ISO_8859_9);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "ISO 8859-9 Turkish");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "char_set");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_PSI_CHAR_SET_GB2312);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "GB-2312");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "char_set");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_PSI_CHAR_SET_UCS_BIG);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "Unicode (UCS-2 BE)");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "char_set");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_PSI_CHAR_SET_UCS_LITTLE);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "Unicode (UCS-2 LE)");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "char_set");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_PSI_CHAR_SET_UTF8);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "Unicode (UTF-8)");
	}

#if MULT_SYSTEM_ENABLE_AUTO_MAP_FUNCTION
	/*自动复用支持信息*/
	{
		mxml_node_t *plXMLCharsetRoot;
		mxml_node_t *plXMLHolder;
		plXMLCharsetRoot = mxmlNewElement(plXMLRoot, "auto_maps");
		MULTL_XMLAddNodeUINT(plXMLCharsetRoot, "cur_auto_mode", MPEG2_DBExGetAnalyzeInputExParameter(lDBSHandle));

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "auto_map");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_DB_EX_AUTO_MAP_NONE);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "close");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "auto_map");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_DB_EX_AUTO_MAP_AUTO);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "auto");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "auto_map");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_DB_EX_AUTO_MAP_OVERRIDE);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "override");

		plXMLHolder = mxmlNewElement(plXMLCharsetRoot, "auto_map");
		MULTL_XMLAddNodeUINT(plXMLHolder, "value", MPEG2_DB_EX_AUTO_MAP_KEEP);
		MULTL_XMLAddNodeText(plXMLHolder, "txt", "keep");
	}
#endif

	/*复用信息设置*/
	{
		MPEG2_DBRemuxInfo lRemuxInfo;
		mxml_node_t *plXMLSettingRoot;
		MPEG2_DBProcRemuxSystemInfo(lDBSHandle, &lRemuxInfo, TRUE);
		plXMLSettingRoot = mxmlNewElement(plXMLRoot, "remux_setting");
		MULTL_XMLAddNodeUINT(plXMLSettingRoot, "input_charset", lRemuxInfo.m_DefaultInputCharSet);
		MULTL_XMLAddNodeUINT(plXMLSettingRoot, "output_charset", lRemuxInfo.m_DefaultOutputCharSet);
		MULTL_XMLAddNodeUINT(plXMLSettingRoot, "out_charset_marker", (lRemuxInfo.m_OutCharsetMarker==TRUE)?1:0);
		MULTL_XMLAddNodeINT(plXMLSettingRoot, "time_zone", lRemuxInfo.m_TimeZone);
		MULTL_XMLAddNodeINT(plXMLSettingRoot, "time_update_cycle", lRemuxInfo.m_TimeUpdateCycle);
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "tot_mark", lRemuxInfo.m_TOTMark);
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "auto_inc", lRemuxInfo.m_VersionAutoINCMark);
		MULTL_XMLAddNodeUINT(plXMLSettingRoot, "max_service_name_bytes", MPEG2_DB_MAX_SERVICE_NAME_LEN);

#ifdef SUPPORT_NEW_HWL_MODULE
		XML_WarpAddNodeONOFF(plXMLSettingRoot, "new_route_func", TRUE);
#endif

#ifdef SUPPORT_INSERTER_DISABLE
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "inserter_disable", TRUE);/*-----------------------------------------------------------------------------------------------------------------------------------*/
#endif
#ifdef SUPPORT_NIT_DISABLE
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "nit_disable", TRUE);/*-----------------------------------------------------------------------------------------------------------------------------------*/
#endif
#ifdef SUPPORT_BATCH_SERVICE_EDIT
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "batch_service_edit", TRUE);/*-----------------------------------------------------------------------------------------------------------------------------------*/
#endif

		/*显示TRANS PCR PID 功能*/
#ifdef TRANS_PCR_PID_FUNCTION
		XML_WarpAddNodeONOFF(plXMLSettingRoot, "trans_pcr_pid_enable", TRUE);
#endif
		{
			CHAR_T	plContrCode[16];
			GLOBAL_SPRINTF((plContrCode, "%c%c%c", lRemuxInfo.m_pCountryCode[0], lRemuxInfo.m_pCountryCode[1], lRemuxInfo.m_pCountryCode[2]));
			MULTL_XMLAddNodeText(plXMLSettingRoot, "country_code", plContrCode);
		}

#if defined(GM2730S)//SPTS模式专用
		{
			MPEG2_DBSPTSParam lSPTSParam;
			MPEG2_DBProcSPTSMode(lDBSHandle, &lSPTSParam, TRUE);
			MULTL_XMLAddNodeMark(plXMLSettingRoot, "spts_sdt_mark", lSPTSParam.m_UseSDT);
		}
#endif

#ifdef DISABLE_REMUX_MODULE
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "disable_remux", TRUE);
#else
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "disable_remux", FALSE);
#endif
#ifdef DISABLE_ROUTE_MODULE
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "disable_route", TRUE);
#else
		MULTL_XMLAddNodeMark(plXMLSettingRoot, "disable_route", FALSE);
#endif

	}

	/*ES加扰设置*/
	{
		S32 i;
		mxml_node_t *plXMLESSettingRoot;
		mxml_node_t *plXMLESSettingNode;
		plXMLESSettingRoot = mxmlNewElement(plXMLRoot, "scramble_advance_setting");
		for (i = 0; i< 256; i++)
		{
			plXMLESSettingNode = mxmlNewElement(plXMLESSettingRoot, "scramble_es_setting");
			MULTL_XMLAddNodeINT(plXMLESSettingNode, "es_type_value", i);
			MULTL_XMLAddNodeMark(plXMLESSettingNode, "scramble", MPEG2_DGetEsStreamTypeMark(lDBSHandle, i));	
		}
	}

#ifdef MULT_ENABLE_ECA_AND_SERVICE_LIST
	/*私有加密系统信息*/
	{
		MULTL_XMLSaveECA(pHandle, plXMLRoot);
	}
	/*私有加密系统信息*/
	{
		MULTL_XMLSaveSERVL(pHandle, plXMLRoot);
	}
#endif


	/*PIDMap信息*/
	{
		mxml_node_t *plPIDMapRoot;
		mxml_node_t *plPIDMapNode;
		MPEG2_DBPIDMapInfo lPIDMapInfo;
		U32 lPIDMapIDs;
		plPIDMapRoot = mxmlNewElement(plXMLRoot, "pid_map");

		plXMLData = mxmlNewElement(plPIDMapRoot, "map_item_max_number");
		mxmlNewTextf(plXMLData, 0, "%d", MPEG2_DBGetPIDMapMaxNum(lDBSHandle));//-------------------------------------------------------------------------------------------------------

		lPIDMapIDs = 0;
		while((lPIDMapIDs = MPEG2_DBGetPIDMapNextNode(lDBSHandle, lPIDMapIDs)) != 0)
		{
			MPEG2_DBGetPIDMapInfo(lDBSHandle, lPIDMapIDs, &lPIDMapInfo);

			plPIDMapNode = mxmlNewElement(plPIDMapRoot, "map_item");

			plXMLData = mxmlNewElement(plPIDMapNode, "pidmap_ids");
			mxmlNewTextf(plXMLData, 0, "%d", lPIDMapIDs);

			plXMLData = mxmlNewElement(plPIDMapNode, "in_ts_index");
			mxmlNewTextf(plXMLData, 0, "%d", lPIDMapInfo.m_InTsIndex);

			plXMLData = mxmlNewElement(plPIDMapNode, "in_pid");
			mxmlNewTextf(plXMLData, 0, "%d", lPIDMapInfo.m_InPID);

			plXMLData = mxmlNewElement(plPIDMapNode, "out_ts_index");
			mxmlNewTextf(plXMLData, 0, "%d", lPIDMapInfo.m_OutTsIndex);

			plXMLData = mxmlNewElement(plPIDMapNode, "out_pid");
			mxmlNewTextf(plXMLData, 0, "%d", lPIDMapInfo.m_OutPID);

			plXMLData = mxmlNewElement(plPIDMapNode, "active_mark");
			mxmlNewTextf(plXMLData, 0, MULTL_XMLMarkValueToStr(lPIDMapInfo.m_OutputMark));
		}
	}



	/*手动TS流插入*/
	{
		mxml_node_t *plManRoot;
		mxml_node_t *plManNode;
		MPEG2_DBManualTsInfo lManualTsInfo;
		U32 lManualIDs;
		S32 lTsSize;
		U8 *plTsBuf;
		GLOBAL_FD lFD;
		plManRoot = mxmlNewElement(plXMLRoot, "manual_ts_inserter");

		plXMLInfoHolder = mxmlNewElement(plManRoot, "information");
		MULTL_XMLAddNodeINT(plXMLInfoHolder, "max_num", MPEG2_DBMANGetMaxSlotNum(lDBSHandle));/*-----------------------------------------------------------------------------------------------------------------------------------*/
		MULTL_XMLAddNodeINT(plXMLInfoHolder, "max_bps", MPEG2_DBMANGetMaxBps(lDBSHandle));/*-----------------------------------------------------------------------------------------------------------------------------------*/
		MULTL_XMLAddNodeINT(plXMLInfoHolder, "max_size", MPEG2_DBMANGetMaxSpace(lDBSHandle));/*-----------------------------------------------------------------------------------------------------------------------------------*/
		MULTL_XMLAddNodeINT(plXMLInfoHolder, "name_max_bytes", MPEG2_DB_MANUAL_INSERTER_NAME_SIZE - 1);/*-----------------------------------------------------------------------------------------------------------------------------------*/

		/*调用拼命删除当前目录下的所有TS文件*/
		GLOBAL_SPRINTF((plPathStr, "rm -rf %s*.bin\n", MULT_XML_BASE_DIR));
		PFC_System(plPathStr);

		lManualIDs = 0;
		while((lManualIDs = MPEG2_DBMANGetNextNode(lDBSHandle, lManualIDs)) != 0)
		{
			if (MPEG2_DBMANProcTsInfo(lDBSHandle, lManualIDs, &lManualTsInfo, TRUE))
			{
				plManNode = mxmlNewElement(plManRoot, "manu_item");
				MULTL_XMLAddNodeUINT(plManNode, "ids", lManualIDs);
				MULTL_XMLAddNodeText(plManNode, "name", lManualTsInfo.m_pName);
				MULTL_XMLAddNodeINT(plManNode, "size", lManualTsInfo.m_Size);
				MULTL_XMLAddNodeINT(plManNode, "bitrate", lManualTsInfo.m_Bitrate);
				MULTL_XMLAddNodeINT(plManNode, "out_ts_index", lManualTsInfo.m_OutTsIndex);
				MULTL_XMLAddNodeText(plManNode, "active_mark", MULTL_XMLMarkValueToStr(lManualTsInfo.m_OutMark));
				if (MPEG2_DBMANGetTsData(lDBSHandle, lManualIDs, &plTsBuf, &lTsSize))
				{
					/*保存对应的TS数据，采用单个文件的方式，以ID号为文件名*/
					GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
					GLOBAL_SPRINTF((&plPathStr[GLOBAL_STRLEN(plPathStr)], "TS%.8X.bin", lManualIDs));
					lFD = GLOBAL_FOPEN(plPathStr, "wb");
					if (lFD)
					{
						GLOBAL_FWRITE(plTsBuf, 1, lTsSize, lFD);
						GLOBAL_FCLOSE(lFD);
						lFD = NULL;
					}
				}
			}
		}
	}


	/*NIT信息*/
	{
		mxml_node_t *plNITRoot;
		mxml_node_t *plNITNodes;
		mxml_node_t *plNITNode;
		mxml_node_t *plNITDeliveryInfo;
		mxml_node_t *plXMLDescs;
		MPEG2_DBNITInfo lNitInfo;
		MPEG2_DBNITTsInfo lTsInfo;
		U32 lNITTsIDs;

		MPEG2_DBGetNITNetworkInfo(lDBSHandle, &lNitInfo);

		plNITRoot = mxmlNewElement(plXMLRoot, "dvb_network_information");

		MULTL_XMLAddNodeINT(plNITRoot, "nit_ts_max_num", MPEG2_DBGetTsInfoMaxNum(lDBSHandle));
		MULTL_XMLAddNodeINT(plNITRoot, "name_max_bytes", MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN - 1);/*-----------------------------------------------------------------------------------------------------------------------------------*/

		plXMLData = mxmlNewElement(plNITRoot, "nit_global_mark");
		mxmlNewTextf(plXMLData, 0, MULTL_XMLMarkValueToStr(lNitInfo.m_ActiveMark));

		plXMLData = mxmlNewElement(plNITRoot, "network_id");
		mxmlNewTextf(plXMLData, 0, "%d", lNitInfo.m_NetworkID);

		plXMLData = mxmlNewElement(plNITRoot, "network_name");
		mxmlNewCDATA(plXMLData, lNitInfo.m_pNetworkName);

		plXMLData = mxmlNewElement(plNITRoot, "version");
		mxmlNewTextf(plXMLData, 0, "%d", lNitInfo.m_VersionNum);

		plXMLDescs = mxmlNewElement(plNITRoot, "network_descriptors");
		MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_NIT_NETWORK_USER, MPEG2_DBGetNITIDs(lDBSHandle));

		plNITNodes = mxmlNewElement(plNITRoot, "nit_ts_inforamtions");

		lNITTsIDs = 0;
		while((lNITTsIDs = MPEG2_DBGetNITTsInfoNextNode(lDBSHandle, lNITTsIDs, MPEG2_DBGetNITIDs(lDBSHandle))) != 0)
		{
			if (MPEG2_DBGetNITTsInfo(lDBSHandle, lNITTsIDs, &lTsInfo) == TRUE)
			{
				plNITNode = mxmlNewElement(plNITNodes, "ts_info");

				plXMLData = mxmlNewElement(plNITNode, "nit_ts_ids");
				mxmlNewTextf(plXMLData, 0, "%d", lNITTsIDs);

				plXMLData = mxmlNewElement(plNITNode, "ts_id");
				mxmlNewTextf(plXMLData, 0, "%d", lTsInfo.m_TsID);

				plXMLData = mxmlNewElement(plNITNode, "on_id");
				mxmlNewTextf(plXMLData, 0, "%d", lTsInfo.m_ONID);

				plXMLData = mxmlNewElement(plNITNode, "delivery_type");
				mxmlNewText(plXMLData, 0, MULTL_XMLDeliveryTypeToStr(lTsInfo.m_DeliveryInfo.m_Type));

				plNITDeliveryInfo = mxmlNewElement(plNITNode, "delivery_info");
				if (lTsInfo.m_DeliveryInfo.m_Type == MPEG2_PSI_CABLE_DELIVERY_SYSTEM_DESCRIPTOR_TAG)
				{
					plXMLData = mxmlNewElement(plNITDeliveryInfo, "freq");
					mxmlNewTextf(plXMLData, 0, "%d", lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_frequency);

					plXMLData = mxmlNewElement(plNITDeliveryInfo, "mode");
					mxmlNewTextf(plXMLData, 0, MULTL_XMLCableDeliveryModeValueToStr(lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_modulation));

					plXMLData = mxmlNewElement(plNITDeliveryInfo, "symbol_rate");
					mxmlNewTextf(plXMLData, 0, "%d", lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_symbol_rate);

					plXMLData = mxmlNewElement(plNITDeliveryInfo, "fec_inner");
					mxmlNewTextf(plXMLData, 0, "%d", lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_FEC_inner);

					plXMLData = mxmlNewElement(plNITDeliveryInfo, "fec_outer");
					mxmlNewTextf(plXMLData, 0, "%d", lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_FEC_outer);
				}

				plXMLDescs = mxmlNewElement(plNITNode, "nit_ts_descriptors");

				MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_NIT_TRANSPORT_USER, lNITTsIDs);
			}
		}
	}



	/*本地同密系统 SCS 信息*/
	{
		CHAR_T plHexBuf[10240];
		S32 lHexLen;
		S32 i, lSCSNum;
		U32 lSCSIDs, lSCSACIDs;
		MPEG2_DBSCSInfo lSCSInfo;
		MPEG2_DBSCSACInfo lSCSACInfo;
		MPEG2_DBSCSSystemInfo lSystemInfo;

		mxml_node_t *plXMLSCSRoot;
		mxml_node_t *plXMLSCSNode;
		mxml_node_t *plXMLACRoot;
		mxml_node_t *plXMLACNode;


		plXMLSCSRoot = mxmlNewElement(plXMLRoot, "simulcrypt_synchronizer");

		plXMLInfoHolder = mxmlNewElement(plXMLSCSRoot, "system_info");
		MPEG2_DBProcSCSSystemInfo(lDBSHandle, &lSystemInfo, TRUE);

		MULTL_XMLAddNodeUINT(plXMLInfoHolder, "default_cp", lSystemInfo.m_DefaultCPDuration);
		MULTL_XMLAddNodeUINT(plXMLInfoHolder, "network_delay", lSystemInfo.m_NetworkDelay);

		plXMLData = mxmlNewElement(plXMLInfoHolder, "fixed_cw");
		lHexLen = CAL_StringBinToHex(lSystemInfo.m_pFxiedCW, sizeof(lSystemInfo.m_pFxiedCW), plHexBuf, sizeof(plHexBuf), TRUE);

		plHexBuf[lHexLen] = 0;
		mxmlNewText(plXMLData, 0, plHexBuf);

		MULTL_XMLAddNodeMark(plXMLInfoHolder, "fixed_cw_mark", lSystemInfo.m_bUserFixedCW);

		/*   2012-10-26-------------BSS-------------------*/
		//MULTL_XMLAddNodeUINT(plXMLInfoHolder, "super_id", pHandle->m_BSSystemInfo.m_SuperCASID);
		plXMLData = mxmlNewElement(plXMLInfoHolder, "super_id");
		mxmlNewTextf(plXMLData, 0, "%.8X", pHandle->m_BSSystemInfo.m_SuperCASID);
		plXMLData = mxmlNewElement(plXMLInfoHolder, "bss_sw");
		lHexLen = CAL_StringBinToHex(pHandle->m_BSSystemInfo.m_pSW, sizeof(pHandle->m_BSSystemInfo.m_pSW), plHexBuf, sizeof(plHexBuf), TRUE);

		plHexBuf[lHexLen] = 0;
		mxmlNewText(plXMLData, 0, plHexBuf);
		MULTL_XMLAddNodeMark(plXMLInfoHolder, "sw_mark", pHandle->m_BSSystemInfo.m_ActiveMark);

		plXMLData = mxmlNewElement(plXMLInfoHolder, "key");
		lHexLen = CAL_StringBinToHex(pHandle->m_BSSystemInfo.m_pKey, sizeof(pHandle->m_BSSystemInfo.m_pKey), plHexBuf, sizeof(plHexBuf), TRUE);
		plHexBuf[lHexLen] = 0;
		mxmlNewText(plXMLData, 0, plHexBuf);


#ifdef SUPPORT_BSS_SCRAMBLE_SYSTEM
		MULTL_XMLAddNodeMark(plXMLInfoHolder, "bss_enable", TRUE);
#else
		MULTL_XMLAddNodeMark(plXMLInfoHolder, "bss_enable", FALSE);
#endif




		lSCSNum = MPEG2_DBGetSCSCount(lDBSHandle);
		for (i = 0; i < lSCSNum; i++)
		{
			lSCSIDs = MPEG2_DBGetSCSIDs(lDBSHandle, i);
			if (MPEG2_DBProcSCSInfo(lDBSHandle, lSCSIDs, &lSCSInfo, TRUE))
			{
				plXMLSCSNode = mxmlNewElement(plXMLSCSRoot, "scs_info");

				MULTL_XMLAddNodeUINT(plXMLSCSNode, "scs_ids", lSCSIDs);
				MULTL_XMLAddNodeText(plXMLSCSNode, "scs_name", lSCSInfo.m_pSCSName);

				plXMLData = mxmlNewElement(plXMLSCSNode, "supercas_id");
				mxmlNewTextf(plXMLData, 0, "%.8X", (lSCSInfo.m_CASystemID << 16) | lSCSInfo.m_CASubSystemID);

				plXMLData = mxmlNewElement(plXMLSCSNode, "emmg_port");
				mxmlNewTextf(plXMLData, 0, "%d", lSCSInfo.m_EMMPort);

				plXMLData = mxmlNewElement(plXMLSCSNode, "ecmg_port");
				mxmlNewTextf(plXMLData, 0, "%d", lSCSInfo.m_ECMPort);

				plXMLData = mxmlNewElement(plXMLSCSNode, "ecm_chn_id");//追加ECM CHN ID 20120327
				mxmlNewTextf(plXMLData, 0, "%.4X", lSCSInfo.m_ECMChnID);

				plXMLData = mxmlNewElement(plXMLSCSNode, "ecmg_ip");
				mxmlNewText(plXMLData, 0, PFC_SocketNToA(lSCSInfo.m_ECMIPv4));


				if (i < pHandle->m_Information.m_LicenseSCSNum)//加扰授权控制
				{
					MULTL_XMLAddNodeMark(plXMLSCSNode, "active_mark", lSCSInfo.m_ActiveMark);
					MULTL_XMLAddNodeINT(plXMLSCSNode, "disabled", 0);
				}
				else
				{
					MULTL_XMLAddNodeMark(plXMLSCSNode, "active_mark", FALSE);
					MULTL_XMLAddNodeINT(plXMLSCSNode, "disabled", 1);
				}


				plXMLACRoot = mxmlNewElement(plXMLSCSNode, "acs");

				lSCSACIDs = 0;
				while((lSCSACIDs = MPEG2_DBGetACNextNode(lDBSHandle, lSCSACIDs, lSCSIDs)) != 0)
				{
					if (MPEG2_DBProcACInfo(lDBSHandle, lSCSACIDs, &lSCSACInfo, TRUE))
					{
						plXMLACNode = mxmlNewElement(plXMLACRoot, "ac");

						plXMLData = mxmlNewElement(plXMLACNode, "ac_ids");
						mxmlNewTextf(plXMLData, 0, "%.8X", lSCSACIDs);

						plXMLData = mxmlNewElement(plXMLACNode, "ac_name");
						mxmlNewCDATA(plXMLData, lSCSACInfo.m_pACName);

						plXMLData = mxmlNewElement(plXMLACNode, "data");
						lHexLen = CAL_StringBinToHex(lSCSACInfo.m_pAccessData, lSCSACInfo.m_ACDataSize, plHexBuf, sizeof(plHexBuf), TRUE);
						plHexBuf[lHexLen] = 0;
						mxmlNewText(plXMLData, 0, plHexBuf);
					}
				}
			}
		}
	}

#ifdef ENCODER_CARD_PLATFORM
	/*节目转码信息！*/
	{
		U32 lServiceIDs, lServUniqueID;
		MPEG2_DBServiceTransInfo lTransInfo;
		HANDLE32 lTransServsRootNode;
		lTransServsRootNode = XML_WarpAddNode(plXMLRoot, "trans_servs");
		lServiceIDs = 0;
		while((lServiceIDs = MPEG2_DBGetServiceNextNode(lDBSHandle, lServiceIDs)) != 0)
		{
			if (MPEG2_DBExTransformServiceProc(lDBSHandle, lServiceIDs, &lTransInfo, TRUE))
			{
				if (lTransInfo.m_bEnable == TRUE)
				{
					MPEG2_DBProcessServiceUniqueID(lDBSHandle, lServiceIDs, &lServUniqueID, TRUE);
					XML_WarpAddNodeHEX(lTransServsRootNode, "uni_id", lServUniqueID, TRUE);
				}
			}

		}
	}
#endif

	/*节目信息*/
	{

		mxml_node_t *plXMLServiceS;
		mxml_node_t *plXMLServ;
		mxml_node_t *plXMLESs;
		mxml_node_t *plXMLES;
		mxml_node_t *plXMLCAs;
		mxml_node_t *plXMLDescs;
		S16 lInTsIndex, lOutTsIndex;
		U32 lServiceIDs, lEsIDs, lTsIDs, lServUniqueID;

		MPEG2_DBServiceInInfo lServiceInInfo;
		MPEG2_DBServiceOutInfo lServiceOutInfo;

		MPEG2_DBEsInInfo lEsInInfo;
		MPEG2_DBEsOutInfo lEsOutInfo;

		plXMLServiceS = mxmlNewElement(plXMLRoot, "services");

		XML_WarpAddNodeS32(plXMLServiceS, "current_input_service_num", MPEG2_DBGetServiceCountSystem(pHandle->m_DBSHandle, TRUE));
		XML_WarpAddNodeS32(plXMLServiceS, "current_output_service_num", MPEG2_DBGetServiceCountSystem(pHandle->m_DBSHandle, FALSE));
		XML_WarpAddNodeS32(plXMLServiceS, "current_service_max", MULT_SERVICE_MAX_NUM);

		lServiceIDs = 0;
		while((lServiceIDs = MPEG2_DBGetServiceNextNode(lDBSHandle, lServiceIDs)) != 0)
		{
			MPEG2_DBGetServiceInInfo(lDBSHandle, lServiceIDs, &lServiceInInfo);
			MPEG2_DBGetServiceOutInfo(lDBSHandle, lServiceIDs, &lServiceOutInfo);

#ifdef ENCODER_CARD_PLATFORM
			MPEG2_DBProcessServiceUniqueID(lDBSHandle, lServiceIDs, &lServUniqueID, TRUE);
			//GLOBAL_TRACE(("Save Serv Unique ID = 0x%08X, lServIDs = 0x%08X !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", lServUniqueID, lServiceIDs));
#endif

			lInTsIndex = MPEG2_DBGetServicTsIndex(lDBSHandle, lServiceIDs, TRUE);
			lOutTsIndex = MPEG2_DBGetServicTsIndex(lDBSHandle, lServiceIDs, FALSE);			


			plXMLServ = mxmlNewElement(plXMLServiceS, "service");

			plXMLData = mxmlNewElement(plXMLServ, "serv_ids");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceIDs);

			XML_WarpAddNodeHEX(plXMLServ, "uni_id", lServUniqueID, TRUE);

			plXMLData = mxmlNewElement(plXMLServ, "in_ts_index");
			mxmlNewTextf(plXMLData, 0, "%d", lInTsIndex);

			plXMLData = mxmlNewElement(plXMLServ, "out_ts_index");

			mxmlNewTextf(plXMLData, 0, "%d", lOutTsIndex);

			plXMLInfoHolder = mxmlNewElement(plXMLServ, "in_service_info");
			plXMLData = mxmlNewElement(plXMLInfoHolder, "pmt_pid");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceInInfo.m_PMTPID);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "pmt_v");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceInInfo.m_PMTVersion);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "pmt_crc32");
			mxmlNewTextf(plXMLData, 0, "%.8X", lServiceInInfo.m_PMTCRC32);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "pcr_pid");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceInInfo.m_PCRPID);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "service_id");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceInInfo.m_ServiceID);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "service_type");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceInInfo.m_ServiceType);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "eit_schedule_mark");
			mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(MPEG2_DB_SERVICE_MARK_IS_HAVE_EIT_SCHEDULE(lServiceInInfo.m_ServiceMark)));
			plXMLData = mxmlNewElement(plXMLInfoHolder, "eit_cn_mark");
			mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(MPEG2_DB_SERVICE_MARK_IS_HAVE_EIT_CUR_NEXT(lServiceInInfo.m_ServiceMark)));
			plXMLData = mxmlNewElement(plXMLInfoHolder, "service_name");
			mxmlNewCDATA(plXMLData, lServiceInInfo.m_ServiceName);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "service_provider");
			mxmlNewCDATA(plXMLData, lServiceInInfo.m_ServiceProviderName);

			plXMLDescs = mxmlNewElement(plXMLInfoHolder, "pmt_descriptors");
			MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_PMT_SERVICE_INFO, lServiceIDs);

			plXMLDescs = mxmlNewElement(plXMLInfoHolder, "sdt_descriptors");
			MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_SDT_SERVICE_INFO, lServiceIDs);


			plXMLCAs = mxmlNewElement(plXMLInfoHolder, "mux_cas");
			MULTL_XMLSaveMuxCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_SERVICE, lServiceIDs);


			plXMLInfoHolder = mxmlNewElement(plXMLServ, "out_service_info");

			//MULTL_XMLAddNodeINT(plXMLServ, "serv_out_mark", lServiceOutInfo.m_ServOutMark);

			plXMLData = mxmlNewElement(plXMLInfoHolder, "pmt_pid");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceOutInfo.m_PMTPID);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "pmt_v");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceOutInfo.m_PMTVersion);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "pmt_interval");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceOutInfo.m_PMTInterval);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "pmt_mark");
			mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lServiceOutInfo.m_PMTActiveMark));
			plXMLData = mxmlNewElement(plXMLInfoHolder, "pcr_pid");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceOutInfo.m_PCRPID);

#ifdef TRANS_PCR_PID_FUNCTION
			plXMLData = mxmlNewElement(plXMLInfoHolder, "trans_pcr_pid");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceOutInfo.m_TransPCRPID);
#endif

			plXMLData = mxmlNewElement(plXMLInfoHolder, "pcr_mark");
			mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lServiceOutInfo.m_PCRMark));
			plXMLData = mxmlNewElement(plXMLInfoHolder, "service_id");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceOutInfo.m_ServiceID);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "service_type");
			mxmlNewTextf(plXMLData, 0, "%d", lServiceOutInfo.m_ServiceType);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "scramble_mark");
			mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(MPEG2_DB_SERVICE_MARK_IS_SCRAMBLE(lServiceOutInfo.m_ServiceMark)));
			plXMLData = mxmlNewElement(plXMLInfoHolder, "eit_schedule_mark");
			mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(MPEG2_DB_SERVICE_MARK_IS_HAVE_EIT_SCHEDULE(lServiceOutInfo.m_ServiceMark)));
			plXMLData = mxmlNewElement(plXMLInfoHolder, "eit_cn_mark");
			mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(MPEG2_DB_SERVICE_MARK_IS_HAVE_EIT_CUR_NEXT(lServiceOutInfo.m_ServiceMark)));
			plXMLData = mxmlNewElement(plXMLInfoHolder, "service_name");
			mxmlNewCDATA(plXMLData, lServiceOutInfo.m_ServiceName);
			plXMLData = mxmlNewElement(plXMLInfoHolder, "service_provider");
			mxmlNewCDATA(plXMLData, lServiceOutInfo.m_ServiceProviderName);


			//用于显示节目是否被转码！
			{
				MPEG2_DBServiceTransInfo lTransInfo;
				MPEG2_DBExTransformServiceProc(lDBSHandle, lServiceIDs, &lTransInfo, TRUE);
				if (lTransInfo.m_bEnable == TRUE)
				{
					XML_WarpAddNodeBOOL(plXMLInfoHolder, "trans_mark", lTransInfo.m_bEnable);
					XML_WarpAddNodeS32(plXMLInfoHolder, "trans_vid_es_type", lTransInfo.m_TransOutVIDESType);
					XML_WarpAddNodeHEX(plXMLInfoHolder, "trans_vid_es_ids", lTransInfo.m_TransVIDEsIDs, TRUE);
				}
				else
				{
					XML_WarpAddNodeBOOL(plXMLInfoHolder, "trans_mark", lTransInfo.m_bEnable);
				}
			}


#ifdef MULT_ENABLE_ECA_AND_SERVICE_LIST
			MULTL_XMLAddNodeUINT(plXMLInfoHolder, "auth_class", lServiceOutInfo.m_AuthClass);
			MULTL_XMLAddNodeUINT(plXMLInfoHolder, "lcn", lServiceOutInfo.m_LCN);
			MULTL_XMLAddNodeMark(plXMLInfoHolder, "lcn_visiable", lServiceOutInfo.m_LCNVisibale);
			MULTL_XMLAddNodeUINT(plXMLInfoHolder, "bouquet_id", lServiceOutInfo.m_BouquetID);
#endif

			/*20120818新增，降低PCR复制的要求*/
			/*20130807改进*/
			MULTL_XMLAddNodeMark(plXMLInfoHolder, "ps_m", lServiceOutInfo.m_ServSharePCRMark);//该参数可以不必读取！


			plXMLDescs = mxmlNewElement(plXMLInfoHolder, "pmt_descriptors");
			MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_PMT_SERVICE_INFO_USER, lServiceIDs);

			plXMLDescs = mxmlNewElement(plXMLInfoHolder, "sdt_descriptors");
			MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_SDT_SERVICE_INFO_USER, lServiceIDs);

			plXMLCAs = mxmlNewElement(plXMLInfoHolder, "scs_cas");
			MULTL_XMLSaveSCSCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_SERVICE, lServiceIDs);


			plXMLESs = mxmlNewElement(plXMLServ, "element_streams");
			lEsIDs = 0;
			while((lEsIDs = MPEG2_DBGetEsNextNode(lDBSHandle, lEsIDs, lServiceIDs)) != 0)
			{
				plXMLES = mxmlNewElement(plXMLESs, "es");

				MPEG2_DBGetEsInInfo(lDBSHandle, lEsIDs, &lEsInInfo);
				MPEG2_DBGetEsOutInfo(lDBSHandle, lEsIDs, &lEsOutInfo);

				plXMLData = mxmlNewElement(plXMLES, "es_ids");
				mxmlNewTextf(plXMLData, 0, "%d", lEsIDs);

				plXMLInfoHolder = mxmlNewElement(plXMLES, "in_es_info"); 
				plXMLData = mxmlNewElement(plXMLInfoHolder, "es_pid");
				mxmlNewTextf(plXMLData, 0, "%d", lEsInInfo.m_EsPID);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "es_type");
				mxmlNewTextf(plXMLData, 0, "%d", lEsInInfo.m_EsType);

				plXMLDescs = mxmlNewElement(plXMLInfoHolder, "es_pmt_descriptors");
				MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_PMT_ES_INFO, lEsIDs);

				plXMLCAs = mxmlNewElement(plXMLInfoHolder, "mux_cas");
				MULTL_XMLSaveMuxCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_ES, lEsIDs);

				plXMLInfoHolder = mxmlNewElement(plXMLES, "out_es_info");
				plXMLData = mxmlNewElement(plXMLInfoHolder, "es_pid");
				mxmlNewTextf(plXMLData, 0, "%d", lEsOutInfo.m_EsPID);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "es_type");
				mxmlNewTextf(plXMLData, 0, "%d", lEsOutInfo.m_EsType);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "es_out_mark");

				/*20131210 支持ES加扰可选功能*/
				MULTL_XMLAddNodeMark(plXMLInfoHolder, "es_scr_mark", lEsOutInfo.m_ScrambleMark);

				mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lEsOutInfo.m_OutputMark));
				plXMLDescs = mxmlNewElement(plXMLInfoHolder, "es_pmt_descriptors");

				/*PID共享标志 XJ 20130807*/
				MULTL_XMLAddNodeMark(plXMLInfoHolder, "ps_m", lEsOutInfo.m_PIDShareMark);//该参数不必读取！

				MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_PMT_ES_INFO_USER, lEsIDs);
			}
		}
	}


	/*Ts信息*/
	{
		mxml_node_t *plXMLTSs;
		mxml_node_t *plXMLIOTSs;
		mxml_node_t *plXMLTS;
		mxml_node_t *plXMLCAs;
		mxml_node_t *plXMLDescs;

		S32 lIOTsNum;
		U32 lTsIDs;
		MPEG2_DBTsInInfo	lInTsInfo;
		MPEG2_DBTsOutInfo	lOutTsInfo;
		MPEG2_DBTsRouteInfo	lRouteInfo;

		plXMLTSs = mxmlNewElement(plXMLRoot, "transport_streams");
		plXMLIOTSs = mxmlNewElement(plXMLTSs, "input_ts");

		lIOTsNum = MPEG2_DBGetTsCount(lDBSHandle, TRUE);
		for (i = 0; i < lIOTsNum; i++)
		{
			plXMLTS = mxmlNewElement(plXMLIOTSs, "ts");
#ifdef ENCODER_CARD_PLATFORM
			/*2016-05-24 不保存设置为无效的TS信息*/
			if (MPEG2_DBGetTsValid(lDBSHandle, i, TRUE) == FALSE)
			{
				XML_WarpAddNodeBOOL(plXMLTS, "ts_valid", FALSE);
				continue;
			}
			else
#endif
			{
				lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, TRUE, i);
				MPEG2_DBGetTsInInfo(lDBSHandle, lTsIDs, &lInTsInfo);
				MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, TRUE, &lRouteInfo);


				plXMLInfoHolder = mxmlNewElement(plXMLTS, "ts_info");
				plXMLData = mxmlNewElement(plXMLInfoHolder, "ts_ids");
				mxmlNewTextf(plXMLData, 0, "%d", lTsIDs);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "ts_id");
				mxmlNewTextf(plXMLData, 0, "%d", lInTsInfo.m_TsID);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "on_id");
				mxmlNewTextf(plXMLData, 0, "%d", lInTsInfo.m_ONID);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "pat_v");
				mxmlNewTextf(plXMLData, 0, "%d", lInTsInfo.m_PATVersion);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "pat_crc32");
				mxmlNewTextf(plXMLData, 0, "%.8X", lInTsInfo.m_PATCRC32);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "sdt_v");
				mxmlNewTextf(plXMLData, 0, "%d", lInTsInfo.m_SDTVersion);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "cat_v");
				mxmlNewTextf(plXMLData, 0, "%d", lInTsInfo.m_CATVersion);

				/*20120801 xj 追加TS的对应通道的开启状况*/
				MULTL_XMLAddNodeMark(plXMLInfoHolder, "ts_mark", MULTL_GetTsMark(pHandle, i, TRUE));

				plXMLInfoHolder = mxmlNewElement(plXMLTS, "route_info");
				plXMLData = mxmlNewElement(plXMLInfoHolder, "ts_index");
				mxmlNewTextf(plXMLData, 0, "%d", lRouteInfo.m_TsIndex);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "route_mark");
				mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lRouteInfo.m_ActiveMark));

#ifdef SUPPORT_NEW_HWL_MODULE//增加新的Route选项
				//XML_WarpAddNodeONOFF(plXMLInfoHolder, "null", lRouteInfo.m_bRouteWithNullPacket);
				//XML_WarpAddNodeONOFF(plXMLInfoHolder, "pcr", lRouteInfo.m_bRouteWithPCRCorrection);
#endif

				//GLOBAL_TRACE(("InTsIndex = %d, RouteTs = %d, Mark = %d\n", i, lRouteInfo.m_TsIndex, lRouteInfo.m_ActiveMark));

				plXMLCAs = mxmlNewElement(plXMLTS, "mux_cas");
				MULTL_XMLSaveMuxCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_TS, lTsIDs);


				plXMLDescs = mxmlNewElement(plXMLTS, "cat_descriptors");
				MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_CAT, lTsIDs);
			}
		}


		plXMLIOTSs = mxmlNewElement(plXMLTSs, "output_ts");

		lIOTsNum = MPEG2_DBGetTsCount(lDBSHandle, FALSE);
		for (i = 0; i < lIOTsNum; i++)
		{
				lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, FALSE, i);
				MPEG2_DBGetTsOutInfo(lDBSHandle, lTsIDs, &lOutTsInfo);
				MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, FALSE, &lRouteInfo);

				plXMLTS = mxmlNewElement(plXMLIOTSs, "ts");

#ifdef ENCODER_CARD_PLATFORM
			/*2016-05-24 不保存设置为无效的TS信息*/
			if (MPEG2_DBGetTsValid(lDBSHandle, i, FALSE) == FALSE)
			{
				XML_WarpAddNodeBOOL(plXMLTS, "ts_valid", FALSE);
				continue;
			}
			else
#endif
			{
				plXMLInfoHolder = mxmlNewElement(plXMLTS, "ts_info");
				plXMLData = mxmlNewElement(plXMLInfoHolder, "ts_ids");
				mxmlNewTextf(plXMLData, 0, "%d", lTsIDs);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "ts_id");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_TsID);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "on_id");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_ONID);

				plXMLData = mxmlNewElement(plXMLInfoHolder, "pat_v");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_PATVersion);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "pat_i");
				mxmlNewTextf(plXMLData, 0, "%.d", lOutTsInfo.m_PATInterval);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "pat_mark");
				mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lOutTsInfo.m_PATActiveMark));

				plXMLData = mxmlNewElement(plXMLInfoHolder, "cat_v");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_CATVersion);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "cat_i");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_CATInterval);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "cat_mark");
				mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lOutTsInfo.m_CATActiveMark));

				plXMLData = mxmlNewElement(plXMLInfoHolder, "sdt_v");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_SDTVersion);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "sdt_i");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_SDTInterval);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "sdt_mark");
				mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lOutTsInfo.m_SDTActiveMark));

				plXMLData = mxmlNewElement(plXMLInfoHolder, "nit_i");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_NITInterval);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "nit_mark");
				mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lOutTsInfo.m_NITActiveMark));

				plXMLData = mxmlNewElement(plXMLInfoHolder, "tdttot_i");
				mxmlNewTextf(plXMLData, 0, "%d", lOutTsInfo.m_TDTTOTInterval);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "tdttot_mark");
				mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lOutTsInfo.m_TDTTOTActiveMark));

				/*20120801 xj 追加TS的对应通道的开启状况*/
				MULTL_XMLAddNodeMark(plXMLInfoHolder, "ts_mark", MULTL_GetTsMark(pHandle, i, FALSE));

				plXMLInfoHolder = mxmlNewElement(plXMLTS, "route_info");
				plXMLData = mxmlNewElement(plXMLInfoHolder, "ts_index");
				mxmlNewTextf(plXMLData, 0, "%d", lRouteInfo.m_TsIndex);
				plXMLData = mxmlNewElement(plXMLInfoHolder, "route_mark");
				mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(lRouteInfo.m_ActiveMark));

#ifdef SUPPORT_NEW_HWL_MODULE//增加新的Route选项
				XML_WarpAddNodeONOFF(plXMLInfoHolder, "null", lRouteInfo.m_bRouteWithNullPacket);
				XML_WarpAddNodeONOFF(plXMLInfoHolder, "pcr", lRouteInfo.m_bRouteWithPCRCorrection);
#endif
				//GLOBAL_TRACE(("Out Ts %d Route From In Ts %d, Mark = %d, Null = %d, PCR = %d\n", i, lRouteInfo.m_TsIndex, lRouteInfo.m_ActiveMark, lRouteInfo.m_bRouteWithNullPacket, lRouteInfo.m_bRouteWithPCRCorrection));


				plXMLCAs = mxmlNewElement(plXMLTS, "scs_cas");
				MULTL_XMLSaveSCSCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_TS, lTsIDs);

				plXMLDescs = mxmlNewElement(plXMLTS, "cat_descriptors"); 
				MULTL_XMLSaveDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_CAT_USER, lTsIDs);
			}
		}
	}





	/*通道信息*/
	{
		MULT_Parameter *plSystemParam;
		MULT_IGMP *plIGMP;
		MULT_ChannelNode *plChnNode;
		MULT_SubChannelNode *plSubNode;

		mxml_node_t *plXMLChns;
		mxml_node_t *plXMLIOChns;
		mxml_node_t *plXMLChnNode;
		mxml_node_t *plXMLSubChns;
		mxml_node_t *plXMLSubNode;

		mxml_node_t *plXMLSubEncoderEntryNode;

		mxml_node_t *plXMLIGMP;
		S32			lTmpSubChnNum;
		plSystemParam = &pHandle->m_Parameter;

		plXMLChns = mxmlNewElement(plXMLRoot, "channels");

		plIGMP = &plSystemParam->m_IGMP;
		plXMLIGMP = mxmlNewElement(plXMLChns, "igmp");
		MULTL_XMLAddNodeINT(plXMLIGMP, "igmp_group_max", 0);
		MULTL_XMLAddNodeINT(plXMLIGMP, "igmp_repeate_time", plIGMP->m_IGMPRepeateTime);
		MULTL_XMLAddNodeMark(plXMLIGMP, "igmp_repeate_mark", plIGMP->m_IGMPRepeateMark);
		MULTL_XMLAddNodeINT(plXMLIGMP, "igmp_version", plIGMP->m_IGMPVersion);


		plXMLIOChns = mxmlNewElement(plXMLChns, "input_channels");
		for (i = 0; i < plSystemParam->m_InChannelNumber; i++)
		{
			plChnNode = &plSystemParam->m_pInChannel[i];
			plXMLChnNode = mxmlNewElement(plXMLIOChns, "channel");
			MULTL_XMLAddNodeText(plXMLChnNode, "type", MULTL_XMLChnTypeValueToStr(plChnNode->m_ChannelType));
			MULTL_XMLAddNodeText(plXMLChnNode, "sub_type", MULTL_XMLChnSubTypeValueToStr(plChnNode->m_SubType));
			MULTL_XMLAddNodeINT(plXMLChnNode, "demod_type", plChnNode->m_DemodType);

#ifdef GN2000
			if (i == 1)
			{
				MULTL_XMLAddNodeMark(plXMLChnNode, "hide_mark", TRUE);
			}
#endif

			lTmpSubChnNum = plChnNode->m_SubChannelNumber;
			switch (plChnNode->m_ChannelType)
			{
			case HWL_CHANNEL_TYPE_ASI:
				{
					MULTL_XMLAddNodeINT(plXMLChnNode, "bitrate", plChnNode->m_ChannelInfo.m_ASI.m_Bitrate);//当前设置值
				}
				break;
			case HWL_CHANNEL_TYPE_TUNER_S:
				{
					plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitsLow");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitshigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsHigh);

					plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_low");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_high");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsHigh);

				}
				break;
			case HWL_CHANNEL_TYPE_TUNER_C:
				{
					plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitsLow");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitshigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsHigh);

					plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_low");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_high");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsHigh);

				}
				break;

#ifdef GM8358Q
			case HWL_CHANNEL_TYPE_ENCODER:
				{
					;
					//plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitsLow");
					//mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsLow);
					//plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitshigh");
					//mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsHigh);
					//plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_low");
					//mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsLow);
					//plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_high");
					//mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsHigh);

				}
				break;
#endif

#ifdef GM8398Q
			case HWL_CHANNEL_TYPE_ENCODER_CVBS_DXT8243:
				{
					;
					//plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitsLow");
					//mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsLow);
					//plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitshigh");
					//mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsHigh);
					//plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_low");
					//mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsLow);
					//plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_high");
					//mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsHigh);

				}
				break;
#endif
#if defined(GN1846) || defined(GN1866)
			case HWL_CHANNEL_TYPE_ENCODER:
				{
					plXMLData = mxmlNewElement(plXMLChnNode, "VidBitrateLimitLow");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_VidOutBitrateLimitLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "VidBitrateLimitHigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_VidOutBitrateLimitHigh);
					plXMLData = mxmlNewElement(plXMLChnNode, "ProgBitrateLimitHigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_ProgBitrateLimitHigh);
					plXMLData = mxmlNewElement(plXMLChnNode, "GopLimitLow");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_GopLimitLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "GopLimitHigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_GopLimitHigh);
					plXMLData = mxmlNewElement(plXMLChnNode, "VolLimitLow");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_VolLimitLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "VolLimitHigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_VolLimitHigh);
					plXMLData = mxmlNewElement(plXMLChnNode, "ProgNumberLimitLow");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_ProgNumberLimitLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "ProgNumberLimitHigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_ProgNumberLimitHigh);
					plXMLData = mxmlNewElement(plXMLChnNode, "ProgNameLimitHigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_ProgNameLimitHigh);
					plXMLData = mxmlNewElement(plXMLChnNode, "PidLimitLow");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_PidLimitLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "PidLimitHigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ENCODER.m_PidLimitHigh);
				}
				break;
#endif
			case HWL_CHANNEL_TYPE_TUNER_DTMB:
				{
					plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitsLow");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "freqLimitshigh");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_FreqLimitsHigh);

					plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_low");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_high");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_TUNER.m_SymRateLimitsHigh);
				}
				break;
			case HWL_CHANNEL_TYPE_E3DS3:
				{
				}
				break;
			case HWL_CHANNEL_TYPE_IP:
				{
					plXMLData = mxmlNewElement(plXMLChnNode, "ipv4_address");
					mxmlNewText(plXMLData, 0, PFC_SocketNToA(plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress));
					plXMLData = mxmlNewElement(plXMLChnNode, "ipv4_mask");
					mxmlNewText(plXMLData, 0, PFC_SocketNToA(plChnNode->m_ChannelInfo.m_IPInfo.m_IPMask));
					plXMLData = mxmlNewElement(plXMLChnNode, "ipv4_gate");
					mxmlNewText(plXMLData, 0, PFC_SocketNToA(plChnNode->m_ChannelInfo.m_IPInfo.m_IPGate));
					MULTL_XMLAddNodeMark(plXMLChnNode, "eth_mark", plChnNode->m_ChannelInfo.m_IPInfo.m_IPMark);

					CAL_StringBinToMAC(plChnNode->m_ChannelInfo.m_IPInfo.m_MAC, GLOBAL_MAC_BUF_SIZE, plStrBuf, sizeof(plStrBuf));
					MULTL_XMLAddNodeText(plXMLChnNode, "ip_mac", plStrBuf);
				}
				break;
			default:
				break;
			}

			plXMLSubChns = mxmlNewElement(plXMLChnNode, "sub_channels");

			for (k = 0; k < lTmpSubChnNum; k++)
			{
				plSubNode = &plChnNode->m_pSubChannelNode[k];
				plXMLSubNode = mxmlNewElement(plXMLSubChns, "sub_channel");

				/*通用参数*/
				plXMLData = mxmlNewElement(plXMLSubNode, "correspond_ts_index");
				mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_CorrespondTsIndex);

				switch (plChnNode->m_SubType)
				{
				case HWL_CHANNEL_SUBTYPE_ASI:
					{
						MULTL_XMLAddNodeINT(plXMLSubNode, "bitrate", plSubNode->m_Bitrate);//当前设置值
						MULTL_XMLAddNodeINT(plXMLSubNode, "max_bitrate", MULT_ASI_MAX_BITRATE);//允许设置的最大值

						// ASI个数授权控制
						if (k >= pHandle->m_Information.m_LicenseInASINum)
						{
							plSubNode->m_ActiveMark = FALSE;
						}
						else
						{
							plSubNode->m_ActiveMark = TRUE;
						}
						//GLOBAL_TRACE(("ASI Chn %d, Sub %d Mark = %d\n",k ,plSubNode->m_ActiveMark));
						MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", TRUE);
# if defined(GM2700B) || defined(GM2700S)//2700不支持PID检测
						MULTL_XMLAddNodeMark(plXMLSubNode, "pid_stat_mark", FALSE);
#endif

#ifdef GC1804B//GC-1804的所以接口均强制直通，所以ASI OUT不能设置码率
						MULTL_XMLAddNodeMark(plXMLSubNode, "output_bitrate_setup_disable", TRUE);
#endif

					}
					break;
				case  HWL_CHANNEL_SUBTYPE_E3DS3:
					{
						plXMLData = mxmlNewElement(plXMLSubNode, "e3ds3_mark");
						mxmlNewText(plXMLData, 0, MULTL_XMLE3DS3ValueToStr(plSubNode->m_SubInfo.m_SubESDS3Info.m_E3DS3));
						plXMLData = mxmlNewElement(plXMLSubNode, "bit_order");
						mxmlNewText(plXMLData, 0, MULTL_XMLBitOrderValueToStr(plSubNode->m_SubInfo.m_SubESDS3Info.m_Bitorder));
						plXMLData = mxmlNewElement(plXMLSubNode, "fream_from");
						mxmlNewTextf(plXMLData, 0, MULTL_XMLFrameformValueToStr(plSubNode->m_SubInfo.m_SubESDS3Info.m_Frameform));
						plXMLData = mxmlNewElement(plXMLSubNode, "packet_length");
						mxmlNewTextf(plXMLData, 0, MULTL_XMLPacketlengthValueToStr(plSubNode->m_SubInfo.m_SubESDS3Info.m_Packetlength));
						plXMLData = mxmlNewElement(plXMLSubNode, "interweave_code");
						mxmlNewTextf(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubESDS3Info.m_InterweaveCoding));
						plXMLData = mxmlNewElement(plXMLSubNode, "scramble_changer");
						mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubESDS3Info.m_Scramble));
						plXMLData = mxmlNewElement(plXMLSubNode, "rs_code");
						mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubESDS3Info.m_RSCoding));
						plXMLData = mxmlNewElement(plXMLSubNode, "code_rate_recover");
						mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubESDS3Info.m_RateRecover));
						MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", TRUE );
					}
					break;
				case  HWL_CHANNEL_SUBTYPE_TUNER_S_AVL6211:
					{
						plXMLData = mxmlNewElement(plXMLSubNode, "in_freq");
						mxmlNewTextf(plXMLData, 0, "%d", (plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq));
						plXMLData = mxmlNewElement(plXMLSubNode, "local_freq");
						mxmlNewTextf(plXMLData, 0, "%d", (plSubNode->m_SubInfo.m_SubTUNERInfo.m_LocalFreq));
						plXMLData = mxmlNewElement(plXMLSubNode, "sym_rate");
						mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_SubInfo.m_SubTUNERInfo.m_SymbolRate);
						plXMLData = mxmlNewElement(plXMLSubNode, "polar_method");
						mxmlNewText(plXMLData, 0, MULTL_XMLPolarMethodValueToStr(plSubNode->m_SubInfo.m_SubTUNERInfo.m_PolarMethod));
						plXMLData = mxmlNewElement(plXMLSubNode, "switch_22k");					
						mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubTUNERInfo.m_22kSwitch));
						plXMLData = mxmlNewElement(plXMLSubNode, "spec_inv");
						mxmlNewText(plXMLData, 0, MULTL_XMLTunerSpecinvValueToStr(plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv));

						MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", TRUE );
					}
					break;
				case  HWL_CHANNEL_SUBTYPE_TUNER_C:
					{
						plXMLData = mxmlNewElement(plXMLSubNode, "in_freq");
						mxmlNewTextf(plXMLData, 0, "%d", (plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq));
						plXMLData = mxmlNewElement(plXMLSubNode, "sym_rate");
						mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_SubInfo.m_SubTUNERInfo.m_SymbolRate);
						plXMLData = mxmlNewElement(plXMLSubNode, "modulation_mode");
						mxmlNewText(plXMLData, 0, MULTL_XMLQAMModeValueToStr(plSubNode->m_SubInfo.m_SubTUNERInfo.m_Modulation));
						plXMLData = mxmlNewElement(plXMLSubNode, "req_type");					
						mxmlNewText(plXMLData, 0, MULTL_XMLReqTunerTypeValueToStr(plSubNode->m_SubInfo.m_SubTUNERInfo.m_Reqtype));
						plXMLData = mxmlNewElement(plXMLSubNode, "spec_inv");
						mxmlNewText(plXMLData, 0, MULTL_XMLTunerSpecinvValueToStr(plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv));

						MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", TRUE );
					}
					break;
				case  HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869:
					{
						plXMLData = mxmlNewElement(plXMLSubNode, "in_freq");
						mxmlNewTextf(plXMLData, 0, "%d", (plSubNode->m_SubInfo.m_SubTUNERInfo.m_InputFreq));
						plXMLData = mxmlNewElement(plXMLSubNode, "spec_inv");
						mxmlNewText(plXMLData, 0, MULTL_XMLTunerSpecinvValueToStr(plSubNode->m_SubInfo.m_SubTUNERInfo.m_Specinv));
					}
					break;


				case HWL_CHANNEL_SUBTYPE_IP:
					{
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_des");
						mxmlNewText(plXMLData, 0, PFC_SocketNToA(plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr));
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_des_port");
						mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Port);
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_src");
						mxmlNewText(plXMLData, 0, PFC_SocketNToA(plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4LimitAddr));
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_src_port");
						mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4LimitPort);
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_src_limit_mark");
						mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4LimitMark));
						MULTL_XMLAddNodeINT(plXMLSubNode, "protocol", plSubNode->m_SubInfo.m_SubIPInfo.m_Protocol);
#if defined(GQ3655) || defined(GQ3710B) || defined(GQ3760A) || defined(GQ3760) || defined(GM8358Q) || defined(GM8398Q)
						if ((k >= 128) || (k >= pHandle->m_Information.m_LicenseInIPNum))
						{
							plSubNode->m_ActiveMark = FALSE; //修改授权后，实际关闭的通道在监控界面依然能检测到
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 1);
						}
						else
						{
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 0);
						}
#endif
					}
					break;
#ifdef GM8358Q
				case HWL_CHANNEL_SUBTYPE_ENCODER_CVBS:
					{
						S32 j ;
						for (j = 0; j < EncoderCvbsBoardSupportEntryNum; j++)
						//if(0)
						{	

							plXMLSubEncoderEntryNode = mxmlNewElement(plXMLSubNode, "encoder_info_entry");

							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "work_mod",					MULTL_XMLEncoderWorkModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "video_format",				MULTL_XMLEncoderVideoFormatValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat));

							//MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "input_hardware_index",		plSubNode->m_SubInfo.m_SubENCODERInfo.m_EncoderInputHardwareIndex);

							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "work_mod",					MULTL_XMLEncoderWorkModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "video_format",				MULTL_XMLEncoderVideoFormatValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "resolution",				MULTL_XMLEncoderVideoResolutionValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Resolution));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "frame_rate",				MULTL_XMLEncoderFrameRateValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_FrameRate));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "out_bit_rate_mode",		MULTL_XMLEncoderOutBitRrateModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_OutBitRate));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "video_encode_mode",		MULTL_XMLEncoderVideoEncodeModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoEncodeMode));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "video_profile",			MULTL_XMLEncoderVideoProfileValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoProfile));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "video_aspect",				MULTL_XMLEncoderVideoAspectValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoAspect));
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "video_bit_rate",			plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoBitRate);
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "image_horizontal_offset",	plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_ImageHorizontalOffset);
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "brightness",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Brightness);
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "contrast",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Contrast);
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "saturation",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Saturation);
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "hue",						plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Hue);
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "audio_encode_mode",		MULTL_XMLEncoderAudioEncodeModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEncodeMode));
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "audio_bit_rate",			MULTL_XMLEncoderAudioBitRateValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioBitRate));
							//MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "audio_emb_channel",		0);//if subtype is CVBS the value is channel 1 
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "volume",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Volume);
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "audio_sample_rate",		MULTL_XMLEncoderAudioSampleRateToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioSampleRate));
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "video_pid",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoPid);
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "audio_pid",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioPid);
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "pcr_pid",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PcrPid);
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "pmt_pid",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PmtPid);
							MULTL_XMLAddNodeText(plXMLSubEncoderEntryNode, "pid_equal_switch",			MULTL_XMLEncoderPidEqualSwitchValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PidEqualSwitch));

							int TempVixsIndex;
							TempVixsIndex = (i*4 + j) / 2;

							//GLOBAL_TRACE(("==================================================j = %d TempVixsIndex = %d\n" , j,TempVixsIndex));
							//GLOBAL_TRACE(("==================================================vixs_active = %d\n" ,!HWL_Check_Encoder_Board_SubChannel(TempVixsIndex)));
							MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "vixs_active", !HWL_Check_Encoder_Board_SubChannel(TempVixsIndex));
							//MULTL_XMLAddNodeINT (plXMLSubEncoderEntryNode, "vixs_active", 0);//test
						}
						
						plSubNode->m_ActiveMark = TRUE;

						MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 0);
					}
					break;				
#endif

#ifdef GM8398Q
				case HWL_CHANNEL_SUBTYPE_ENCODER_CVBS:
					{
						S32 j ;
						for (j = 0; j < EncoderCvbsBoardSupportEntryNum; j++)
							//if(0)
						{	

							plXMLSubEncoderEntryNode = mxmlNewElement(plXMLSubNode, "encoder_info_entry");

							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "work_mod",					MULTL_XMLEncoderWorkModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_format",				MULTL_XMLEncoderVideoFormatValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat));

							//MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "input_hardware_index",		plSubNode->m_SubInfo.m_SubENCODERInfo.m_EncoderInputHardwareIndex);

							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "work_mod",					MULTL_XMLEncoderWorkModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_format",				MULTL_XMLEncoderVideoFormatValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "resolution",				MULTL_XMLEncoderVideoResolutionValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Resolution));
							//MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "frame_rate",				MULTL_XMLEncoderFrameRateValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_FrameRate));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "out_bit_rate_mode",		MULTL_XMLEncoderOutBitRrateModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_OutBitRate));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_encode_mode",		MULTL_XMLEncoderVideoEncodeModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoEncodeMode));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_profile",			MULTL_XMLEncoderVideoProfileValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoProfile));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_aspect",				MULTL_XMLEncoderVideoAspectValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoAspect));
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "video_bit_rate",			plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoBitRate);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "image_horizontal_offset",	plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_ImageHorizontalOffset);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "brightness",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Brightness);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "contrast",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Contrast);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "saturation",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Saturation);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "hue",						plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Hue);
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "audio_encode_mode",		MULTL_XMLEncoderAudioEncodeModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEncodeMode));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "audio_bit_rate",			MULTL_XMLEncoderAudioBitRateValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioBitRate));
							//MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "audio_emb_channel",		0);//if subtype is CVBS the value is channel 1 
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "volume",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Volume);
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "audio_sample_rate",		MULTL_XMLEncoderAudioSampleRateToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioSampleRate));
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "video_pid",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoPid);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "audio_pid",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioPid);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "pcr_pid",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PcrPid);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "pmt_pid",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PmtPid);
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "pid_equal_switch",			MULTL_XMLEncoderPidEqualSwitchValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PidEqualSwitch));

							int TempDXT8243Index;
							TempDXT8243Index = (i*4 + j) / 2;

							//GLOBAL_TRACE(("==================================================j = %d TempVixsIndex = %d\n" , j,TempVixsIndex));
							//GLOBAL_TRACE(("==================================================DXT8243_active[%d] = %d\n", TempDXT8243Index, !HWL_CheckEncoderDx8243Exist(TempDXT8243Index)));
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "dxt8243_active", !HWL_CheckEncoderDx8243Exist(TempDXT8243Index));
							//MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "dxt8243_active", 0);//test
						}

						plSubNode->m_ActiveMark = TRUE;

						MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 0);
					}
					break;				
#endif

#ifdef GN1846
				case HWL_CHANNEL_SUBTYPE_ENCODER_HI3531A:
					{
						MULTL_XMLAddNodeText(plXMLSubNode, "vi_mode", MULTL_XMLEncViModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.m_ViMode));
						MULTL_XMLAddNodeText(plXMLSubNode, "vo_mode", MULTL_XMLEncVoModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.m_VoMode));
						MULTL_XMLAddNodeText(plXMLSubNode, "br_mode", MULTL_XMLEncBrModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.m_BrMode));
						MULTL_XMLAddNodeText(plXMLSubNode, "enc_mode", MULTL_XMLEncModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.m_EncMode));
						MULTL_XMLAddNodeText(plXMLSubNode, "profile", MULTL_XMLEncProfileValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.m_Profile));
						MULTL_XMLAddNodeUINT(plXMLSubNode, "bitrate", plSubNode->m_SubInfo.m_SubENCODERInfo.m_Bitrate);
						MULTL_XMLAddNodeUINT(plXMLSubNode, "prog_bitrate", plSubNode->m_SubInfo.m_SubENCODERInfo.m_ProgBitrate);
						MULTL_XMLAddNodeUINT(plXMLSubNode, "gop", plSubNode->m_SubInfo.m_SubENCODERInfo.m_Gop);
						MULTL_XMLAddNodeText(plXMLSubNode, "aud_enc_mode", MULTL_XMLAudEncModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.m_AudEncMode));
						MULTL_XMLAddNodeText(plXMLSubNode, "aud_bitrate", MULTL_XMLAudBitrateValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.m_AudBitrate));
						MULTL_XMLAddNodeText(plXMLSubNode, "aud_sample", MULTL_XMLAudSampValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.m_AudSample));
						MULTL_XMLAddNodeINT(plXMLSubNode, "volume", plSubNode->m_SubInfo.m_SubENCODERInfo.m_Volume);
						MULTL_XMLAddNodeUINT(plXMLSubNode, "prog_number", plSubNode->m_SubInfo.m_SubENCODERInfo.m_ProgNumber);
						MULTL_XMLAddNodeText(plXMLSubNode, "prog_name", plSubNode->m_SubInfo.m_SubENCODERInfo.m_pProgName);
						MULTL_XMLAddNodeUINT(plXMLSubNode, "vid_pid", plSubNode->m_SubInfo.m_SubENCODERInfo.m_VidPid);
						MULTL_XMLAddNodeUINT(plXMLSubNode, "aud_pid", plSubNode->m_SubInfo.m_SubENCODERInfo.m_AudPid);
						MULTL_XMLAddNodeUINT(plXMLSubNode, "pcr_pid", plSubNode->m_SubInfo.m_SubENCODERInfo.m_PcrPid);
						MULTL_XMLAddNodeUINT(plXMLSubNode, "pmt_pid", plSubNode->m_SubInfo.m_SubENCODERInfo.m_PmtPid);
						MULTL_XMLAddNodeINT(plXMLSubNode, "protocol", plSubNode->m_SubInfo.m_SubENCODERInfo.m_Protocol);
						MULTL_XMLAddNodeText(plXMLSubNode, "ipv4_des", PFC_SocketNToA(plSubNode->m_SubInfo.m_SubENCODERInfo.m_IPv4Addr));
						MULTL_XMLAddNodeINT(plXMLSubNode, "ipv4_des_port", plSubNode->m_SubInfo.m_SubENCODERInfo.m_IPv4Port);
						//GLOBAL_TRACE(("EncChn[%d]: ip-%#x, port-%d, protocol-%d\n", k, plSubNode->m_SubInfo.m_SubENCODERInfo.m_IPv4Addr, 
						//	plSubNode->m_SubInfo.m_SubENCODERInfo.m_IPv4Port, plSubNode->m_SubInfo.m_SubENCODERInfo.m_Protocol));
						MULTL_XMLAddNodeMark(plXMLSubNode, "active", plSubNode->m_SubInfo.m_SubENCODERInfo.m_ActiveMark);
					}
					break;	
#endif

#ifdef GN1866
				case HWL_CHANNEL_SUBTYPE_ENCODER_HI3519:
					{
						S32 j ;
						for (j = 0; j < EncoderCvbsBoardSupportEntryNum; j++)
							//if(0)
						{	

							plXMLSubEncoderEntryNode = mxmlNewElement(plXMLSubNode, "encoder_info_entry");

							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "work_mod",					MULTL_XMLEncoderWorkModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_format",				MULTL_XMLEncoderVideoFormatValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat));

							//MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "input_hardware_index",		plSubNode->m_SubInfo.m_SubENCODERInfo.m_EncoderInputHardwareIndex);

							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "work_mod",					MULTL_XMLEncoderWorkModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_WorkMod));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_format",				MULTL_XMLEncoderVideoFormatValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoFormat));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "resolution",				MULTL_XMLEncoderVideoResolutionValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Resolution));
							//MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "frame_rate",				MULTL_XMLEncoderFrameRateValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_FrameRate));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "out_bit_rate_mode",		MULTL_XMLEncoderOutBitRrateModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_OutBitRate));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_encode_mode",		MULTL_XMLEncoderVideoEncodeModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoEncodeMode));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_profile",			MULTL_XMLEncoderVideoProfileValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoProfile));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "video_aspect",				MULTL_XMLEncoderVideoAspectValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoAspect));
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "video_bit_rate",			plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoBitRate);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "image_horizontal_offset",	plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_ImageHorizontalOffset);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "brightness",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Brightness);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "contrast",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Contrast);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "saturation",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Saturation);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "hue",						plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Hue);
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "audio_encode_mode",		MULTL_XMLEncoderAudioEncodeModeValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioEncodeMode));
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "audio_bit_rate",			MULTL_XMLEncoderAudioBitRateValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioBitRate));
							//MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "audio_emb_channel",		0);//if subtype is CVBS the value is channel 1 
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "volume",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_Volume);
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "audio_sample_rate",		MULTL_XMLEncoderAudioSampleRateToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioSampleRate));
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "video_pid",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_VideoPid);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "audio_pid",				plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_AudioPid);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "pcr_pid",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PcrPid);
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "pmt_pid",					plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PmtPid);
							MULTL_XMLAddNodeText( plXMLSubEncoderEntryNode, "pid_equal_switch",			MULTL_XMLEncoderPidEqualSwitchValueToStr(plSubNode->m_SubInfo.m_SubENCODERInfo.SubENCODERInfoEntry[j].m_PidEqualSwitch));

							int TempDXT8243Index;
							TempDXT8243Index = (i*4 + j) / 2;

							//GLOBAL_TRACE(("==================================================j = %d TempVixsIndex = %d\n" , j,TempVixsIndex));
							//GLOBAL_TRACE(("==================================================DXT8243_active[%d] = %d\n", TempDXT8243Index, !HWL_CheckEncoderDx8243Exist(TempDXT8243Index)));
							MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "dxt8243_active", !HWL_CheckEncoderDx8243Exist(TempDXT8243Index));
							//MULTL_XMLAddNodeINT ( plXMLSubEncoderEntryNode, "dxt8243_active", 0);//test
						}

						plSubNode->m_ActiveMark = TRUE;

						MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 0);
					}
					break;	
#endif
				default:
					break;
				}


				MULTL_XMLAddNodeMark(plXMLSubNode, "active_mark", plSubNode->m_ActiveMark );
			}
		}

		plXMLIOChns = mxmlNewElement(plXMLChns, "output_channels");
		for (i = 0; i < plSystemParam->m_OutChannelNumber; i++)
		{
			plChnNode = &plSystemParam->m_pOutChannel[i];
			plXMLChnNode = mxmlNewElement(plXMLIOChns, "channel");
			MULTL_XMLAddNodeText(plXMLChnNode, "type", MULTL_XMLChnTypeValueToStr(plChnNode->m_ChannelType));
			MULTL_XMLAddNodeText(plXMLChnNode, "sub_type", MULTL_XMLChnSubTypeValueToStr(plChnNode->m_SubType));

#ifdef GN2000
			if (i == 1)
			{
				MULTL_XMLAddNodeMark(plXMLChnNode, "hide_mark", TRUE);
			}
#endif
			switch (plChnNode->m_ChannelType)
			{
			case HWL_CHANNEL_TYPE_ASI:
				{
					MULTL_XMLAddNodeINT(plXMLChnNode, "bitrate", plChnNode->m_ChannelInfo.m_ASI.m_Bitrate);//当前设置值
					//MULTL_XMLAddNodeINT(plXMLChnNode, "max_bitrate", MULT_ASI_MAX_BITRATE * plChnNode->m_SubChannelNumber);//允许设置的最大值
				}
				break;
			case HWL_CHANNEL_TYPE_IP:
			case HWL_CHANNEL_TYPE_IP_LOOP:
			case HWL_CHANNEL_TYPE_IP_DEP:
				{
					plXMLData = mxmlNewElement(plXMLChnNode, "ipv4_address");
					mxmlNewText(plXMLData, 0, PFC_SocketNToA(plChnNode->m_ChannelInfo.m_IPInfo.m_IPAddress));
					plXMLData = mxmlNewElement(plXMLChnNode, "ipv4_mask");
					mxmlNewText(plXMLData, 0, PFC_SocketNToA(plChnNode->m_ChannelInfo.m_IPInfo.m_IPMask));
					plXMLData = mxmlNewElement(plXMLChnNode, "ipv4_gate");
					mxmlNewText(plXMLData, 0, PFC_SocketNToA(plChnNode->m_ChannelInfo.m_IPInfo.m_IPGate));
					MULTL_XMLAddNodeMark(plXMLChnNode, "eth_mark", plChnNode->m_ChannelInfo.m_IPInfo.m_IPMark);

					CAL_StringBinToMAC(plChnNode->m_ChannelInfo.m_IPInfo.m_MAC, GLOBAL_MAC_BUF_SIZE, plStrBuf, sizeof(plStrBuf));
					MULTL_XMLAddNodeText(plXMLChnNode, "ip_mac", plStrBuf);
					MULTL_XMLAddNodeINT(plXMLChnNode, "bitrate", plChnNode->m_ChannelInfo.m_IPInfo.m_Bitrate);//当前设置值
					MULTL_XMLAddNodeINT(plXMLChnNode, "max_bitrate", MULT_IP_CHN_DEFAULT_BITRATE);//允许设置的最大值

				}
				break;
			case HWL_CHANNEL_TYPE_DVB_C_MODULATOR:
			case HWL_CHANNEL_TYPE_DVB_S_MODULATOR:
			case HWL_CHANNEL_TYPE_DTMB_MODULATOR:
				{
					plXMLData = mxmlNewElement(plXMLChnNode, "adjacent_freq_number");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_AdjacentFreqNumber);
					plXMLData = mxmlNewElement(plXMLChnNode, "center_frequence_limits_low");
					mxmlNewTextf(plXMLData, 0, "%u", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "center_frequence_limits_high");
					mxmlNewTextf(plXMLData, 0, "%u", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_CenterFrequenceLimitsHigh);
					plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_low");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_SymbolRateLimitsLow);
					plXMLData = mxmlNewElement(plXMLChnNode, "sym_rate_limits_high");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_SymbolRateLimitsHigh);
					plXMLData = mxmlNewElement(plXMLChnNode, "ex_att_valid_mark");
					mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttValidMark));
					plXMLData = mxmlNewElement(plXMLChnNode, "ex_att_stepping");
					mxmlNewTextf(plXMLData, 0, "%f", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttStepping);
					plXMLData = mxmlNewElement(plXMLChnNode, "ex_att_level");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevel);
					plXMLData = mxmlNewElement(plXMLChnNode, "ex_att_level_max");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ExAttLevelMax);
					plXMLData = mxmlNewElement(plXMLChnNode, "gain_stepping");
					mxmlNewTextf(plXMLData, 0, "%f", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainStepping);
					plXMLData = mxmlNewElement(plXMLChnNode, "gain_level_max");
					mxmlNewTextf(plXMLData, 0, "%d", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_GainLevelMax);

#ifdef SUPPORT_ALC
					MULTL_XMLAddNodeMark(plXMLChnNode, "alc_mark", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_ALCMark);
#endif
#ifdef SUPPORT_MODULATOR_FREQ_1_HZ
					MULTL_XMLAddNodeINT(plXMLChnNode, "freq_adj", plChnNode->m_ChannelInfo.m_ModulatorInfo.m_FreqAdj);
					MULTL_XMLAddNodeMark(plXMLChnNode, "freq_adj_disable", FALSE);
#else
					MULTL_XMLAddNodeMark(plXMLChnNode, "freq_adj_disable", TRUE);
#endif
				}
				break;

			default:
				break;
			}


			plXMLSubChns = mxmlNewElement(plXMLChnNode, "sub_channels");
			for (k = 0; k < plChnNode->m_SubChannelNumber; k++)
			{
				plSubNode = &plChnNode->m_pSubChannelNode[k];
				plXMLSubNode = mxmlNewElement(plXMLSubChns, "sub_channel");
				/*通用参数*/
				plXMLData = mxmlNewElement(plXMLSubNode, "correspond_ts_index");
				mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_CorrespondTsIndex);


				switch (plChnNode->m_SubType)
				{
				case HWL_CHANNEL_SUBTYPE_ASI:
					{
# if defined(GM2700B) || defined(GM2700S)//2700不支持PID检测
						MULTL_XMLAddNodeMark(plXMLSubNode, "pid_stat_mark", FALSE);
#endif
						if (k >= pHandle->m_Information.m_LicenseOutTsNum)//通道授权控制
						{
							plSubNode->m_ActiveMark = FALSE; //修改授权后，实际关闭的通道在监控界面依然能检测到
						}
						else
						{
							plSubNode->m_ActiveMark = TRUE;
						}

						MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 1);

						MULTL_XMLAddNodeINT(plXMLSubNode, "bitrate", plSubNode->m_Bitrate);//当前设置值

						MULTL_XMLAddNodeINT(plXMLSubNode, "max_bitrate", MULT_ASI_MAX_OUTPUT_BITRATE);//允许设置的最大值

					}
					break;
				case HWL_CHANNEL_SUBTYPE_IP:
					{
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_des");
						mxmlNewText(plXMLData, 0, PFC_SocketNToA(plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Addr));
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_des_port");
						mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4Port);
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_src");
						mxmlNewText(plXMLData, 0, PFC_SocketNToA(plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4LimitAddr));
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_src_port");
						mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4LimitPort);
						plXMLData = mxmlNewElement(plXMLSubNode, "ipv4_src_limit_mark");
						mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubIPInfo.m_IPv4LimitMark));
						MULTL_XMLAddNodeINT(plXMLSubNode, "bitrate", plSubNode->m_Bitrate);//当前设置值
						MULTL_XMLAddNodeINT(plXMLSubNode, "max_bitrate", MULT_IP_SUB_DEFAULT_OUTPUT_BITRATE);//允许设置的最大值
						MULTL_XMLAddNodeINT(plXMLSubNode, "protocol", plSubNode->m_SubInfo.m_SubIPInfo.m_Protocol);
#if defined(GQ3760A)
						//GLOBAL_TRACE(("k = %d, Limit = %d\n", k, pHandle->m_Information.m_LicenseOutIPNum));
						if ((k >= 64) || (k >= pHandle->m_Information.m_LicenseOutIPNum))
						{
							plSubNode->m_ActiveMark = FALSE; //修改授权后，实际关闭的通道在监控界面依然能检测到
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 1);
						}
						else
						{
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 0);
						}
#else
						if (k >= pHandle->m_Information.m_LicenseOutTsNum)//通道授权控制
						{
							plSubNode->m_ActiveMark = FALSE; //修改授权后，实际关闭的通道在监控界面依然能检测到
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 1);
						}
						else
						{
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 0);
						}

#endif
					}
					break;
				case HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789:
				case HWL_CHANNEL_SUBTYPE_MODULATOR_BL85KMM:
					{
						plXMLData = mxmlNewElement(plXMLSubNode, "itu_coding");
						mxmlNewText(plXMLData, 0, MULTL_XMLITUCodingValueToStr(plSubNode->m_SubInfo.m_SubModulatorInfo.m_ITUCoding));
						plXMLData = mxmlNewElement(plXMLSubNode, "analog_band");
						mxmlNewText(plXMLData, 0, MULTL_XMLAnalogValueToStr(plSubNode->m_SubInfo.m_SubModulatorInfo.m_AnalogBand));
						plXMLData = mxmlNewElement(plXMLSubNode, "center_frequence");
						mxmlNewTextf(plXMLData, 0, "%u", plSubNode->m_SubInfo.m_SubModulatorInfo.m_CenterFreq);
						plXMLData = mxmlNewElement(plXMLSubNode, "symbol_rate");
						mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_SubInfo.m_SubModulatorInfo.m_SymbolRate);
						plXMLData = mxmlNewElement(plXMLSubNode, "modulation_mode");
						mxmlNewText(plXMLData, 0, MULTL_XMLQAMModeValueToStr(plSubNode->m_SubInfo.m_SubModulatorInfo.m_Mode));
						plXMLData = mxmlNewElement(plXMLSubNode, "spectrum_invert");
						mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubModulatorInfo.m_SpectInv));
						plXMLData = mxmlNewElement(plXMLSubNode, "gain_level");
						mxmlNewTextf(plXMLData, 0, "%d", plSubNode->m_SubInfo.m_SubModulatorInfo.m_GainLevel);
						plXMLData = mxmlNewElement(plXMLSubNode, "modulation_mark");
						mxmlNewText(plXMLData, 0, MULTL_XMLMarkValueToStr(plSubNode->m_SubInfo.m_SubModulatorInfo.m_Modulation));

						//QPSK
						plXMLData = mxmlNewElement(plXMLSubNode, "fec_encode");
						mxmlNewText(plXMLData, 0, MULTL_XMLFecEncodeValueToStr(plSubNode->m_SubInfo.m_SubModulatorInfo.m_FecEncode ));

#if defined(GQ3760A) || defined(LR1800S) || defined(GQ3760B) || defined(GQ3763)  || defined(GQ3760) || defined(GQ3768) || defined(GQ3765)
						MULTL_XMLAddNodeINT(plXMLSubNode, "carrier", plSubNode->m_SubInfo.m_SubModulatorInfo.m_CarrierMode);
						MULTL_XMLAddNodeText(plXMLSubNode, "pn_mode", MULTL_XMLDTMBPNValueToStr(plSubNode->m_SubInfo.m_SubModulatorInfo.m_PNMode));
						MULTL_XMLAddNodeText(plXMLSubNode, "code_rate", MULTL_XMLDTMBCodeRateValueToStr(plSubNode->m_SubInfo.m_SubModulatorInfo.m_CodeRate));
						MULTL_XMLAddNodeINT(plXMLSubNode, "time_interleaver", plSubNode->m_SubInfo.m_SubModulatorInfo.m_TimeInterleave);
						MULTL_XMLAddNodeMark(plXMLSubNode, "double_pilot", plSubNode->m_SubInfo.m_SubModulatorInfo.m_DoublePilot);
#endif
#ifdef GN2000
						if (k > 0)
						{
							plSubNode->m_ActiveMark = FALSE;
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 1);
						}
						else
						{
							plSubNode->m_ActiveMark = TRUE;
						}
#else
						if (k >= pHandle->m_Information.m_LicenseOutTsNum)//通道授权控制
						{
							plSubNode->m_ActiveMark = FALSE; //修改授权后，实际关闭的通道在监控界面依然能检测到
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 1);
						}
						else
						{
							MULTL_XMLAddNodeINT(plXMLSubNode, "disabled", 0);
						}
#endif


					}
					break;
				default:
					break;
				}


				MULTL_XMLAddNodeMark(plXMLSubNode, "active_mark", plSubNode->m_ActiveMark);

			}
		}

	}


#ifdef MULT_SYSTEM_HAVE_PCR_CORRECT_ADJUST_FUNCTION
	{
		mxml_node_t *plXMLHolder;
		MULT_PCRCorrect *plPCRCorrect = &pHandle->m_Parameter.m_PCRCorrect;
		plXMLHolder = mxmlNewElement(plXMLRoot, "pcr_correct");

		MULTL_XMLAddNodeMark(plXMLHolder, "pcrc_mark", plPCRCorrect->m_PCRCMark);
		MULTL_XMLAddNodeUINT(plXMLHolder, "pcrc_pos", plPCRCorrect->m_PCRCPos);
		MULTL_XMLAddNodeUINT(plXMLHolder, "pcrc_neg", plPCRCorrect->m_PCRCNeg);
	}
#endif


#ifdef GN2000
	MULTL_MOSIACSaveXML(plXMLRoot);
#endif

#ifdef MULT_TS_BACKUP_SUPPORT
	MULT_BPSaveXML(plXMLRoot);
#endif


	/*保存成文件*/
	{
		CHAR_T plPathStr[1024];
		GLOBAL_FD plFile;
		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_SYSTEM_PARAMETER_XML);
		plFile = GLOBAL_FOPEN(plPathStr, "w");
		if (plFile != NULL)
		{
			mxmlSaveFile(plXML, plFile, NULL);
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
	}

	mxmlDelete(plXML);
	plXML = NULL;

	/*-----batch------*/
	{
		mxml_node_t *plBatchXML;
		mxml_node_t *plBatchXMLRoot;
		mxml_node_t *plBatchXMLInfoHolder;

		CHAR_T plPathStr[1024];
		GLOBAL_FD plFile;


		plBatchXML = mxmlNewXML("1.0");
		plBatchXMLRoot = mxmlNewElement(plBatchXML, "root");
		MULTL_XMLAddNodeINT(plBatchXMLRoot, "version_number", 0);



		mxml_node_t *plBatchXMLTSs;
		mxml_node_t *plBatchXMLIOTSs;
		mxml_node_t *plBatchXMLTS;
		mxml_node_t *plTmpXMLData;
		mxml_node_t *plServXMLHolder;
		mxml_node_t *plServXMLData;
		mxml_node_t *plTmpXMLHolder;
		MPEG2_DBServiceInInfo lServiceInInfo;
		MPEG2_DBServiceOutInfo lServiceOutInfo;
		MPEG2_DBTsRouteInfo	lRouteInfo;

		U32 lTsIDs, lServiceIDs;
		S32 lIOTsNum, lOutTsIndex, lItemIDNum, i, j;
		U32 ppArrayBuf[1024];


		/*------Ts-------*/

		plBatchXMLTSs = mxmlNewElement(plBatchXMLRoot, "transport_streams");
		plBatchXMLIOTSs = mxmlNewElement(plBatchXMLTSs, "input_ts");

		lIOTsNum = MPEG2_DBGetTsCount(lDBSHandle, TRUE);
		for (i = 0; i < lIOTsNum; i++)
		{
			lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, TRUE, i);
			MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, TRUE, &lRouteInfo);

			plBatchXMLTS = mxmlNewElement(plBatchXMLIOTSs, "ts");
			MULTL_XMLAddNodeMark(plBatchXMLTS, "ts_mark", MULTL_GetTsMark(pHandle, i, TRUE));
			plTmpXMLData = mxmlNewElement(plBatchXMLTS, "ts_index");
			mxmlNewTextf(plTmpXMLData, 0, "%d", lRouteInfo.m_TsIndex);			
			plTmpXMLData = mxmlNewElement(plBatchXMLTS, "route_mark");
			mxmlNewText(plTmpXMLData, 0, MULTL_XMLMarkValueToStr(lRouteInfo.m_ActiveMark));
		}

		lIOTsNum = MPEG2_DBGetTsCount(lDBSHandle, FALSE);
		plBatchXMLIOTSs = mxmlNewElement(plBatchXMLTSs, "output_ts");
		for (i = 0; i < lIOTsNum; i++)
		{
			lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, FALSE, i);
			MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, FALSE, &lRouteInfo);

			plBatchXMLTS = mxmlNewElement(plBatchXMLIOTSs, "ts");
			MULTL_XMLAddNodeMark(plBatchXMLTS, "ts_mark", MULTL_GetTsMark(pHandle, i, FALSE));
			plTmpXMLData = mxmlNewElement(plBatchXMLTS, "ts_index");
			mxmlNewTextf(plTmpXMLData, 0, "%d", lRouteInfo.m_TsIndex);			
			plTmpXMLData = mxmlNewElement(plBatchXMLTS, "route_mark");
			mxmlNewText(plTmpXMLData, 0, MULTL_XMLMarkValueToStr(lRouteInfo.m_ActiveMark));
		}


		/*------services-------*/
		plBatchXMLInfoHolder = mxmlNewElement(plBatchXMLRoot, "servids");
		lIOTsNum = MPEG2_DBGetTsCount(lDBSHandle, TRUE);
		for (i = 0; i<lIOTsNum; i++ )
		{
			lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, TRUE, i);
			MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, TRUE, &lRouteInfo);
			if(FALSE == lRouteInfo.m_ActiveMark)
			{
				lItemIDNum= MPEG2_DBGetServiceIDsArrayByTsIndex(lDBSHandle, i, TRUE, TRUE, ppArrayBuf, 1000);
				for (j= 0;j < lItemIDNum;j++)
				{
					lServiceIDs = ppArrayBuf[j];
					MPEG2_DBGetServiceInInfo(lDBSHandle, lServiceIDs, &lServiceInInfo);
					MPEG2_DBGetServiceOutInfo(lDBSHandle, lServiceIDs, &lServiceOutInfo);
					plServXMLHolder = mxmlNewElement(plBatchXMLInfoHolder, "service");
					plServXMLData = mxmlNewElement(plServXMLHolder, "serv_ids");
					mxmlNewTextf(plServXMLData, 0, "%d", lServiceIDs);

					plServXMLData = mxmlNewElement(plServXMLHolder, "in_ts_index");
					mxmlNewTextf(plServXMLData, 0, "%d", i);
					lOutTsIndex = MPEG2_DBGetServicTsIndex(lDBSHandle, lServiceIDs, FALSE);
					plServXMLData = mxmlNewElement(plServXMLHolder, "out_ts_index");
					mxmlNewTextf(plServXMLData, 0, "%d", lOutTsIndex);			
					plTmpXMLHolder = mxmlNewElement(plServXMLHolder, "in_service_info");			
					plServXMLData = mxmlNewElement(plTmpXMLHolder, "service_id");
					mxmlNewTextf(plServXMLData, 0, "%d", lServiceInInfo.m_ServiceID);	
					plServXMLData = mxmlNewElement(plTmpXMLHolder, "service_name");
					mxmlNewCDATA(plServXMLData, lServiceInInfo.m_ServiceName);

					plTmpXMLHolder = mxmlNewElement(plServXMLHolder, "out_service_info");			

					plServXMLData = mxmlNewElement(plTmpXMLHolder, "service_id");
					mxmlNewTextf(plServXMLData, 0, "%d", lServiceOutInfo.m_ServiceID);

					plServXMLData = mxmlNewElement(plTmpXMLHolder, "service_name");
					mxmlNewCDATA(plServXMLData, lServiceOutInfo.m_ServiceName);
				}
			}	
		}

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_SERVICE_PARAMETER_XML);
		plFile = GLOBAL_FOPEN(plPathStr, "w");
		if (plFile != NULL)
		{
			mxmlSaveFile(plBatchXML, plFile, NULL);
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
		mxmlDelete(plBatchXML);
		plBatchXML = NULL;
	}



	MULTL_SetSaveMark(pHandle, FALSE);
}

void MULTL_LoadParameterXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	MULTL_DefaultParameter(pHandle, pHWInfo);//首先填充默认值！
	{
		CHAR_T plPathStr[1024], *plTmpStr;
		GLOBAL_FD plFile;

		MULT_IDsArray lACIDsArray;

		GLOBAL_ZEROMEM(&lACIDsArray, sizeof(lACIDsArray));

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_SYSTEM_PARAMETER_XML);
		plFile = GLOBAL_FOPEN(plPathStr, "r");
		if (plFile != NULL)
		{
			mxml_node_t *plXML;
			mxml_node_t *plXMLRoot;

			/*分析并校验参数文件，容许XML文件缺少部分属性以应对系统升级！！！*/
			plXML = mxmlLoadFile(NULL, plFile,  MXML_OPAQUE_CALLBACK);
			if (plXML)
			{
				plXMLRoot = mxmlFindElement(plXML, plXML, "root", NULL, NULL, MXML_DESCEND_FIRST);
				if (plXMLRoot)
				{
					plTmpStr = MULTL_XMLGetNodeText(plXMLRoot, "version_number");
					if (plTmpStr)
					{
						S32 lVersion, lAuthMode;
						lVersion = GLOBAL_STRTOL(plTmpStr, NULL, 10);
						HANDLE32 lDBSHandle;

						lDBSHandle = pHandle->m_DBSHandle;

						//GLOBAL_TRACE(("System Parameter Version Number = %d\n", lVersion)); 

						lAuthMode = MULTL_XMLGetNodeINT(plXMLRoot, "current_auth_signature", 10);

						if(lVersion >= 0)
						{
#ifdef SUPPORT_SFN_MODULATOR
							MULT_SFNXMLLoad(plXMLRoot, FALSE);
#endif
#ifdef SUPPORT_SFN_ADAPTER
							MULT_SFNAXMLLoad(plXMLRoot, FALSE);
#endif

#ifdef SUPPORT_IP_O_TS_MODULE
							MULT_IPoTSXMLLoad(pHandle, plXMLRoot, FALSE);
#endif

#ifdef SUPPORT_GNS_MODULE
							MULT_GNSXMLLoad(pHandle, plXMLRoot, FALSE);
#endif
#ifdef SUPPORT_NTS_DPD_BOARD
							MULT_NTSDPDXMLLoad(plXMLRoot, FALSE);
#endif
							/*复用信息读取*/
							{
								MPEG2_DBRemuxInfo lRemuxInfo;
								mxml_node_t *plXMLRemuxInfoRoot;
								MPEG2_DBProcRemuxSystemInfo(lDBSHandle, &lRemuxInfo, TRUE);

								plXMLRemuxInfoRoot = mxmlFindElement(plXMLRoot, plXMLRoot, "remux_setting", NULL, NULL, MXML_DESCEND_FIRST);
								if (plXMLRemuxInfoRoot)
								{
									lRemuxInfo.m_DefaultInputCharSet = MULTL_XMLGetNodeUINT(plXMLRemuxInfoRoot, "input_charset", 10);
									lRemuxInfo.m_DefaultOutputCharSet = MULTL_XMLGetNodeUINT(plXMLRemuxInfoRoot, "output_charset", 10);
									lRemuxInfo.m_OutCharsetMarker = MULTL_XMLGetNodeUINT(plXMLRemuxInfoRoot, "out_charset_marker", 10);
									lRemuxInfo.m_TimeZone = MULTL_XMLGetNodeINT(plXMLRemuxInfoRoot, "time_zone", 10);
									lRemuxInfo.m_TimeUpdateCycle = MULTL_XMLGetNodeINT(plXMLRemuxInfoRoot, "time_update_cycle", 10);
									lRemuxInfo.m_TOTMark = MULTL_XMLGetNodeMark(plXMLRemuxInfoRoot, "tot_mark");
									lRemuxInfo.m_VersionAutoINCMark = MULTL_XMLGetNodeMark(plXMLRemuxInfoRoot, "auto_inc");

									{
										CHAR_T	*plContrCode;
										plContrCode = MULTL_XMLGetNodeText(plXMLRemuxInfoRoot, "country_code");
										GLOBAL_MEMCPY(lRemuxInfo.m_pCountryCode, "   ", 3);
										if (plContrCode)
										{
											if (GLOBAL_STRLEN(plContrCode) >= 3)
											{
												GLOBAL_MEMCPY(lRemuxInfo.m_pCountryCode, plContrCode, 3);
											}
										}

									}
#if defined(GM2730S)//SPTS模式专用
									{
										MPEG2_DBSPTSParam lSPTSParam;
										MPEG2_DBProcSPTSMode(lDBSHandle, &lSPTSParam, TRUE);

										lSPTSParam.m_SPTSMark = TRUE;
										lSPTSParam.m_UseECM = FALSE;
										lSPTSParam.m_UseSDT = MULTL_XMLGetNodeMark(plXMLRemuxInfoRoot, "spts_sdt_mark");
										MPEG2_DBProcSPTSMode(lDBSHandle, &lSPTSParam, FALSE);

									}
#endif
								}
								MPEG2_DBProcRemuxSystemInfo(lDBSHandle, &lRemuxInfo, FALSE);



							}

							/*ES加扰设置*/
							{
								mxml_node_t *plXMLESTypeInfoRoot;
								mxml_node_t *plXMLESTypeInfoNode;
								U8 lIndex;
								BOOL lScramnle;
								plXMLESTypeInfoRoot = mxmlFindElement(plXMLRoot, plXMLRoot, "scramble_advance_setting", NULL, NULL, MXML_DESCEND_FIRST);
								if (plXMLESTypeInfoRoot)
								{
									plXMLESTypeInfoNode = mxmlFindElement(plXMLESTypeInfoRoot, plXMLESTypeInfoRoot, "scramble_es_setting", NULL, NULL, MXML_DESCEND_FIRST);
									while (plXMLESTypeInfoNode)
									{
										lIndex = MULTL_XMLGetNodeINT(plXMLESTypeInfoNode, "es_type_value", 10);
										lScramnle = MULTL_XMLGetNodeMark(plXMLESTypeInfoNode, "scramble");
										MPEG2_DBSetEsStreamTypeMark(lDBSHandle, lIndex, lScramnle);
										plXMLESTypeInfoNode = mxmlFindElement(plXMLESTypeInfoNode, plXMLESTypeInfoRoot, "scramble_es_setting", NULL, NULL, MXML_NO_DESCEND);
									}

								}
							}

#ifdef MULT_ENABLE_ECA_AND_SERVICE_LIST
							/*私有加密系统信息*/
							{
								MULTL_XMLLoadECA(pHandle, plXMLRoot, FALSE);
							}
							/*私有节目表信息*/
							{
								MULTL_XMLLoadSERVL(pHandle, plXMLRoot, FALSE);
							}
#endif

							/*PIDMap信息*/
							{
								mxml_node_t *plPIDMapRoot;
								mxml_node_t *plPIDMapNode;
								MPEG2_DBPIDMapInfo lPIDMapInfo;
								U32 lPIDMapIDs;

								plPIDMapRoot = mxmlFindElement(plXMLRoot, plXMLRoot, "pid_map", NULL, NULL, MXML_DESCEND_FIRST);
								if (plPIDMapRoot)
								{
									plPIDMapNode = mxmlFindElement(plPIDMapRoot, plPIDMapRoot, "map_item", NULL, NULL, MXML_DESCEND_FIRST);
									while(plPIDMapNode)
									{
										lPIDMapInfo.m_InTsIndex = MULTL_XMLGetNodeINT(plPIDMapNode, "in_ts_index", 10);
										lPIDMapInfo.m_InPID = MULTL_XMLGetNodeUINT(plPIDMapNode, "in_pid", 10);
										lPIDMapInfo.m_OutTsIndex = MULTL_XMLGetNodeINT(plPIDMapNode, "out_ts_index", 10);
										lPIDMapInfo.m_OutPID = MULTL_XMLGetNodeUINT(plPIDMapNode, "out_pid", 10);
										lPIDMapInfo.m_OutputMark = MULTL_XMLGetNodeMark(plPIDMapNode, "active_mark");
										lPIDMapIDs = MPEG2_DBAddPIDMap(lDBSHandle, &lPIDMapInfo);
										GLOBAL_ASSERT(lPIDMapIDs);
										plPIDMapNode = mxmlFindElement(plPIDMapNode, plPIDMapRoot, "map_item", NULL, NULL, MXML_NO_DESCEND);
									}
								}
							}


							/*手动TS流插入*/
							{
								MPEG2_DBManualTsInfo lManualTsInfo;
								mxml_node_t *plManRoot;
								mxml_node_t *plManNode;
								U32 lManualIDs;
								S32 lTsSize;
								U8 *plTsBuf;
								GLOBAL_FD lFD;

								plManRoot = mxmlFindElement(plXMLRoot, plXMLRoot, "manual_ts_inserter", NULL, NULL, MXML_DESCEND_FIRST);
								if (plManRoot)
								{
									plManNode = mxmlFindElement(plManRoot, plManRoot, "manu_item", NULL, NULL, MXML_DESCEND_FIRST);
									while(plManNode)
									{
										lManualIDs = MULTL_XMLGetNodeUINT(plManNode, "ids", 10);

										GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
										GLOBAL_SPRINTF((&plPathStr[GLOBAL_STRLEN(plPathStr)], "TS%.8X.bin", lManualIDs));
										lFD = GLOBAL_FOPEN(plPathStr, "rb");
										if (lFD)
										{
											lTsSize = CAL_FileSize(lFD);
											plTsBuf = (U8*)GLOBAL_MALLOC(lTsSize);
											if (plTsBuf)
											{
												GLOBAL_FREAD(plTsBuf, 1, lTsSize, lFD);
												lManualTsInfo.m_Size = MULTL_XMLGetNodeINT(plManNode, "size", 10);
												if (lTsSize == lManualTsInfo.m_Size)
												{
													if (MPEG2_TsSynByteCheck(plTsBuf, lTsSize))
													{
														if (MPEG2_DBMANGetFreeSpace(lDBSHandle) - lTsSize > 0)
														{
															lManualIDs = MPEG2_DBMANAdd(lDBSHandle, plTsBuf, lTsSize);
															if (MPEG2_DBMANProcTsInfo(lDBSHandle, lManualIDs, &lManualTsInfo, TRUE))
															{
																if (CAL_StringValidStrcpy(lManualTsInfo.m_pName, MULTL_XMLGetNodeText(plManNode, "name"), sizeof(lManualTsInfo.m_pName)) <= 0)
																{
																	lManualTsInfo.m_pName[0] = 0;
																}

																lManualTsInfo.m_Size = lTsSize;
																lManualTsInfo.m_Bitrate = MULTL_XMLGetNodeINT(plManNode, "bitrate", 10);
																lManualTsInfo.m_OutTsIndex = MULTL_XMLGetNodeINT(plManNode, "out_ts_index", 10);
																lManualTsInfo.m_OutMark = MULTL_XMLGetNodeMark(plManNode, "active_mark");;
																MPEG2_DBMANProcTsInfo(lDBSHandle, lManualIDs, &lManualTsInfo, FALSE);
															}
															else
															{
																GLOBAL_ASSERT(0);
															}
														}
														else
														{
															GLOBAL_TRACE(("No More Space!\n"));
														}
													}
													else
													{
														GLOBAL_TRACE(("Ts Check Error!\n"));
													}
												}
												else
												{
													GLOBAL_TRACE(("Get Manual Info Failed!\n"));
												}
												GLOBAL_FREE(plTsBuf);
												plTsBuf = NULL;
											}
											GLOBAL_FCLOSE(lFD);
											lFD = NULL;
										}

										plManNode = mxmlFindElement(plManNode, plManRoot, "manu_item", NULL, NULL, MXML_NO_DESCEND);
									}
								}
							}

							/*NIT信息*/
							{
								mxml_node_t *plNITRoot;
								mxml_node_t *plNITTsRoot;
								mxml_node_t *plNITTSNode;
								mxml_node_t *plNITDeliveryInfo;
								mxml_node_t *plXMLDescRoot;
								MPEG2_DBNITInfo lNitInfo;
								MPEG2_DBNITTsInfo lTsInfo;
								CHAR_T *plTmpBuf;
								U32 lNITTsIDs;

								MPEG2_DBGetNITNetworkInfo(lDBSHandle, &lNitInfo);
								plNITRoot = mxmlFindElement(plXMLRoot, plXMLRoot, "dvb_network_information", NULL, NULL, MXML_DESCEND_FIRST);
								if (plNITRoot)
								{
									lNitInfo.m_ActiveMark = MULTL_XMLGetNodeMark(plNITRoot, "nit_global_mark");
									lNitInfo.m_NetworkID = MULTL_XMLGetNodeUINT(plNITRoot, "network_id", 10);

									if (CAL_StringValidStrcpy(lNitInfo.m_pNetworkName, MULTL_XMLGetNodeText(plNITRoot, "network_name"), sizeof(lNitInfo.m_pNetworkName)) <= 0)
									{
										lNitInfo.m_pNetworkName[0] = 0;
									}

									lNitInfo.m_VersionNum = MULTL_XMLGetNodeUINT(plNITRoot, "version", 10);
									MPEG2_DBSetNITNetworkInfo(lDBSHandle, &lNitInfo);

									plXMLDescRoot = mxmlFindElement(plNITRoot, plNITRoot, "network_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
									if (plXMLDescRoot)
									{
										MULTL_XMLLoadDescriptors(pHandle, plXMLDescRoot, MPEG2_DESCRIPTOR_NIT_NETWORK_USER, MPEG2_DBGetNITIDs(lDBSHandle));
									}

									plNITTsRoot = mxmlFindElement(plNITRoot, plNITRoot, "nit_ts_inforamtions", NULL, NULL, MXML_DESCEND_FIRST);
									if (plNITTsRoot)
									{
										GLOBAL_ZEROMEM(&lTsInfo, sizeof(lTsInfo));
										plNITTSNode = mxmlFindElement(plNITTsRoot, plNITTsRoot, "ts_info", NULL, NULL, MXML_DESCEND_FIRST);
										while(plNITTSNode != NULL)
										{
											lTsInfo.m_TsID = MULTL_XMLGetNodeUINT(plNITTSNode, "ts_id", 10);
											lTsInfo.m_ONID = MULTL_XMLGetNodeUINT(plNITTSNode, "on_id", 10);
											plTmpBuf = MULTL_XMLGetNodeText(plNITTSNode, "delivery_type");
											lTsInfo.m_DeliveryInfo.m_Type = MULTL_XMLDeliveryTypeFromStr(plTmpBuf);
											if (lTsInfo.m_DeliveryInfo.m_Type == MPEG2_PSI_CABLE_DELIVERY_SYSTEM_DESCRIPTOR_TAG)//目前仅支持这个类型
											{
												plNITDeliveryInfo = mxmlFindElement(plNITTSNode, plNITTSNode, "delivery_info", NULL, NULL, MXML_DESCEND_FIRST);
												if (plNITDeliveryInfo)
												{
													lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_frequency = MULTL_XMLGetNodeUINT(plNITDeliveryInfo, "freq", 10);
													lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_modulation = MULTL_XMLCableDeliveryModeValueFromStr(MULTL_XMLGetNodeText(plNITDeliveryInfo, "mode"));
													lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_symbol_rate = MULTL_XMLGetNodeUINT(plNITDeliveryInfo, "symbol_rate", 10);
													lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_FEC_inner = MULTL_XMLGetNodeUINT(plNITDeliveryInfo, "fec_inner", 10);
													lTsInfo.m_DeliveryInfo.m_Descriptor.m_Cable.m_FEC_outer = MULTL_XMLGetNodeUINT(plNITDeliveryInfo, "fec_outer", 10);

													lNITTsIDs = MPEG2_DBAddNITTsInfo(lDBSHandle, &lTsInfo);
													if (lNITTsIDs)
													{
														plXMLDescRoot = mxmlFindElement(plNITTSNode, plNITTSNode, "nit_ts_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
														if (plXMLDescRoot)
														{
															MULTL_XMLLoadDescriptors(pHandle, plXMLDescRoot, MPEG2_DESCRIPTOR_NIT_TRANSPORT_USER, lNITTsIDs);
														}
													}
												}

											}
											plNITTSNode = mxmlFindElement(plNITTSNode, plNITTsRoot, "ts_info", NULL, NULL, MXML_NO_DESCEND);
										}
									}

								}
							}


							/*下面需要AC相关的IDs对照数据！*/
							MULTL_CreateIDs(&lACIDsArray, MPEG2_DBGetACNodeMaxNum(lDBSHandle));
							/*本地同密系统 SCS 信息*/
							{
								U32 lSCSIDs, lACIDs, lOriACIDs, lSuperCASID;
								S32 lSCSIndex;
								CHAR_T *plTmpBuf;
								MPEG2_DBSCSInfo lSCSInfo;
								MPEG2_DBSCSACInfo lSCSACInfo;
								MPEG2_DBSCSSystemInfo lSystemInfo;
								MULT_IDs lACIDsNode;
								mxml_node_t *plXMLSCSRoot;
								mxml_node_t *plXMLSCSNode;
								mxml_node_t *plXMLSCSInfo;
								mxml_node_t *plXMLACRoot;
								mxml_node_t *plXMLACNode;



								plXMLSCSRoot = mxmlFindElement(plXMLRoot, plXMLRoot, "simulcrypt_synchronizer", NULL, NULL, MXML_DESCEND_FIRST);
								if (plXMLSCSRoot)
								{
									if (MPEG2_DBProcSCSSystemInfo(lDBSHandle, &lSystemInfo, TRUE))
									{
										plXMLSCSInfo = mxmlFindElement(plXMLSCSRoot, plXMLSCSRoot, "system_info", NULL, NULL, MXML_DESCEND_FIRST);
										if (plXMLSCSInfo)
										{
											lSystemInfo.m_DefaultCPDuration = MULTL_XMLGetNodeUINT(plXMLSCSInfo, "default_cp", 10);
											lSystemInfo.m_NetworkDelay = MULTL_XMLGetNodeUINT(plXMLSCSInfo, "network_delay", 10);

											plTmpBuf = MULTL_XMLGetNodeText(plXMLSCSInfo, "fixed_cw");

											if (CAL_StringHexToBin(plTmpBuf, lSystemInfo.m_pFxiedCW, sizeof(lSystemInfo.m_pFxiedCW)) == sizeof(lSystemInfo.m_pFxiedCW))
											{
												lSystemInfo.m_bUserFixedCW = MULTL_XMLGetNodeMark(plXMLSCSInfo, "fixed_cw_mark");
											}
										}
										MPEG2_DBProcSCSSystemInfo(lDBSHandle, &lSystemInfo, FALSE);

										/* 2012-10-26   ----BSS----------*/
										{
											pHandle->m_BSSystemInfo.m_SuperCASID = MULTL_XMLGetNodeUINT(plXMLSCSInfo, "super_id", 16);

											plTmpBuf = MULTL_XMLGetNodeText(plXMLSCSInfo, "bss_sw");

											if (CAL_StringHexToBin(plTmpBuf, pHandle->m_BSSystemInfo.m_pSW, sizeof(pHandle->m_BSSystemInfo.m_pSW)) == sizeof(pHandle->m_BSSystemInfo.m_pSW))
											{
												pHandle->m_BSSystemInfo.m_ActiveMark = MULTL_XMLGetNodeMark(plXMLSCSInfo, "sw_mark");
											}
											plTmpBuf = MULTL_XMLGetNodeText(plXMLSCSInfo, "key");
											CAL_StringHexToBin(plTmpBuf, pHandle->m_BSSystemInfo.m_pKey, sizeof(pHandle->m_BSSystemInfo.m_pKey));
										}



									}

									lSCSIndex = 0;
									plXMLSCSNode = mxmlFindElement(plXMLSCSRoot, plXMLSCSRoot, "scs_info", NULL, NULL, MXML_DESCEND_FIRST);
									while(plXMLSCSNode)
									{
										lSCSIDs = MPEG2_DBGetSCSIDs(lDBSHandle, lSCSIndex);
										if (MPEG2_DBProcSCSInfo(lDBSHandle, lSCSIDs, &lSCSInfo, TRUE))
										{
											if (CAL_StringValidStrcpy(lSCSInfo.m_pSCSName, MULTL_XMLGetNodeText(plXMLSCSNode, "scs_name"), sizeof(lSCSInfo.m_pSCSName)) <= 0)
											{
												lSCSInfo.m_pSCSName[0] = 0;
											}

											lSuperCASID = MULTL_XMLGetNodeUINT(plXMLSCSNode, "supercas_id", 16);
											lSCSInfo.m_CASystemID = (lSuperCASID >> 16) & GLOBAL_U16_MAX;
											lSCSInfo.m_CASubSystemID = lSuperCASID & GLOBAL_U16_MAX;

											lSCSInfo.m_EMMPort = MULTL_XMLGetNodeUINT(plXMLSCSNode, "emmg_port", 10);
											lSCSInfo.m_ECMPort = MULTL_XMLGetNodeUINT(plXMLSCSNode, "ecmg_port", 10);
											lSCSInfo.m_ECMChnID = MULTL_XMLGetNodeUINT(plXMLSCSNode, "ecm_chn_id", 16);
											lSCSInfo.m_ECMIPv4 = PFC_SocketAToN(MULTL_XMLGetNodeText(plXMLSCSNode, "ecmg_ip"));
											if (lSCSIndex < pHandle->m_Information.m_LicenseSCSNum)//加扰授权控制 
											{
												lSCSInfo.m_ActiveMark = MULTL_XMLGetNodeMark(plXMLSCSNode, "active_mark");
											}
											else
											{
												lSCSInfo.m_ActiveMark = FALSE;
											}

											MPEG2_DBProcSCSInfo(lDBSHandle, lSCSIDs, &lSCSInfo, FALSE);

											plXMLACRoot = mxmlFindElement(plXMLSCSNode, plXMLSCSNode, "acs", NULL, NULL, MXML_DESCEND_FIRST);
											if (plXMLACRoot)
											{
												plXMLACNode = mxmlFindElement(plXMLACRoot, plXMLACRoot, "ac", NULL, NULL, MXML_DESCEND_FIRST);
												while(plXMLACNode)
												{
													lOriACIDs = MULTL_XMLGetNodeUINT(plXMLACNode, "ac_ids", 16);

													if (CAL_StringValidStrcpy(lSCSACInfo.m_pACName, MULTL_XMLGetNodeText(plXMLACNode, "ac_name"), sizeof(lSCSACInfo.m_pACName)) <= 0)
													{
														lSCSACInfo.m_pACName[0] = 0;
													}

													plTmpBuf = MULTL_XMLGetNodeText(plXMLACNode, "data");
													lSCSACInfo.m_ACDataSize = CAL_StringHexToBin(plTmpBuf, lSCSACInfo.m_pAccessData, sizeof(lSCSACInfo.m_pAccessData));
													if (lSCSACInfo.m_ACDataSize > 0)
													{
														lACIDs = MPEG2_DBAddAC(lDBSHandle, lSCSIDs, &lSCSACInfo);
														if (lACIDs > 0)
														{
															lACIDsNode.m_ORIIDs = lOriACIDs;
															lACIDsNode.m_NewIDs = lACIDs;
															MULTL_AddIDs(&lACIDsArray, &lACIDsNode);
														}
													}
													plXMLACNode = mxmlFindElement(plXMLACNode, plXMLACRoot, "ac", NULL, NULL, MXML_NO_DESCEND);
												}
											}
										}
										lSCSIndex ++;
										plXMLSCSNode = mxmlFindElement(plXMLSCSNode, plXMLSCSRoot, "scs_info", NULL, NULL, MXML_NO_DESCEND);
									}
								}
							}



							/*节目信息*/
							{
								mxml_node_t *plXMLServiceS;
								mxml_node_t *plXMLServ;
								mxml_node_t *plXMLServInfo;
								mxml_node_t *plXMLESs;
								mxml_node_t *plXMLES;
								mxml_node_t *plXMLESInfo;
								mxml_node_t *plXMLCAs;
								mxml_node_t *plXMLDescRoot;
								S16 lInTsIndex, lOutTsIndex;
								U32 lServiceIDs, lEsIDs, lServUniqueID;
								CHAR_T *plTmpBuf;
								BOOL lTmpMark;

								MPEG2_DBServiceInInfo lServiceInInfo;
								MPEG2_DBServiceOutInfo lServiceOutInfo;
								MPEG2_DBEsInInfo lEsInInfo;
								MPEG2_DBEsOutInfo lEsOutInfo;

								//MULT_IDs lTmpIDs;


								plXMLServiceS = mxmlFindElement(plXMLRoot, plXMLRoot, "services", NULL, NULL, MXML_DESCEND_FIRST);
								if (plXMLServiceS)
								{
									plXMLServ = mxmlFindElement(plXMLServiceS, plXMLServiceS, "service", NULL, NULL, MXML_DESCEND_FIRST);
									while(plXMLServ)
									{
										lServUniqueID = MULTL_XMLGetNodeUINT(plXMLServ, "uni_id", 16);
										lInTsIndex = MULTL_XMLGetNodeUINT(plXMLServ, "in_ts_index", 10);
										lOutTsIndex = MULTL_XMLGetNodeUINT(plXMLServ, "out_ts_index", 10);
										lServiceIDs = MPEG2_DBAddService(lDBSHandle, TRUE, lInTsIndex);
										if (lServiceIDs > 0)
										{

											MPEG2_DBSetServiceTsIndex(lDBSHandle, lServiceIDs, lOutTsIndex, FALSE);
#ifdef ENCODER_CARD_PLATFORM
											//GLOBAL_TRACE(("Load Serv Unique ID = 0x%08X, lServIDs = 0x%08X !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n", lServUniqueID, lServiceIDs));
											MPEG2_DBProcessServiceUniqueID(lDBSHandle, lServiceIDs, &lServUniqueID, FALSE);
#endif

											if (MPEG2_DBGetServiceInInfo(lDBSHandle, lServiceIDs, &lServiceInInfo))
											{
												plXMLServInfo = mxmlFindElement(plXMLServ, plXMLServ, "in_service_info", NULL, NULL, MXML_DESCEND_FIRST);
												if (plXMLServInfo)
												{
													lServiceInInfo.m_PMTPID = MULTL_XMLGetNodeINT(plXMLServInfo, "pmt_pid", 10);
													lServiceInInfo.m_PMTVersion = MULTL_XMLGetNodeUINT(plXMLServInfo, "pmt_v", 10);
													lServiceInInfo.m_PMTCRC32 = MULTL_XMLGetNodeUINT(plXMLServInfo, "pmt_crc32", 16);
													lServiceInInfo.m_PCRPID = MULTL_XMLGetNodeUINT(plXMLServInfo, "pcr_pid", 10);
													lServiceInInfo.m_ServiceID = MULTL_XMLGetNodeUINT(plXMLServInfo, "service_id", 10);
													lServiceInInfo.m_ServiceType = MULTL_XMLGetNodeUINT(plXMLServInfo, "service_type", 10);
													lTmpMark = MULTL_XMLGetNodeMark(plXMLServInfo, "eit_schedule_mark");
													if (lTmpMark)
													{
														MPEG2_DB_SERVICE_MARK_SET_HAVE_EIT_SCHEDULE(lServiceInInfo.m_ServiceMark);
													}
													lTmpMark = MULTL_XMLGetNodeMark(plXMLServInfo, "eit_cn_mark");
													if (lTmpMark)
													{
														MPEG2_DB_SERVICE_MARK_SET_HAVE_EIT_CUR_NEXT(lServiceInInfo.m_ServiceMark);
													}

													if (CAL_StringValidStrcpy(lServiceInInfo.m_ServiceName, MULTL_XMLGetNodeText(plXMLServInfo, "service_name"), sizeof(lServiceInInfo.m_ServiceName)) <= 0)
													{
														lServiceInInfo.m_ServiceName[0] = 0;
													}

													if (CAL_StringValidStrcpy(lServiceInInfo.m_ServiceProviderName, MULTL_XMLGetNodeText(plXMLServInfo, "service_provider"), sizeof(lServiceInInfo.m_ServiceProviderName)) <= 0)
													{
														lServiceInInfo.m_ServiceProviderName[0] = 0;
													}


													plXMLDescRoot = mxmlFindElement(plXMLServInfo, plXMLServInfo, "pmt_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
													if (plXMLDescRoot)
													{
														MULTL_XMLLoadDescriptors(pHandle, plXMLDescRoot, MPEG2_DESCRIPTOR_PMT_SERVICE_INFO, lServiceIDs);
													}

													plXMLDescRoot = mxmlFindElement(plXMLServInfo, plXMLServInfo, "sdt_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
													if (plXMLDescRoot)
													{
														MULTL_XMLLoadDescriptors(pHandle, plXMLDescRoot, MPEG2_DESCRIPTOR_SDT_SERVICE_INFO, lServiceIDs);
													}


													plXMLCAs = mxmlFindElement(plXMLServInfo, plXMLServInfo, "mux_cas", NULL, NULL, MXML_DESCEND_FIRST);
													if (plXMLCAs)
													{
														MULTL_XMLLoadMuxCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_SERVICE, lServiceIDs);
													}

													MPEG2_DBSetServiceInInfo(lDBSHandle, lServiceIDs, &lServiceInInfo);
												}
											}


											if (MPEG2_DBGetServiceOutInfo(lDBSHandle, lServiceIDs, &lServiceOutInfo))
											{
												plXMLServInfo = mxmlFindElement(plXMLServ, plXMLServ, "out_service_info", NULL, NULL, MXML_DESCEND_FIRST);
												if (plXMLServInfo)
												{
													//lServiceOutInfo.m_ServOutMark = MULTL_XMLGetNodeINT(plXMLServ, "serv_out_mark", 10);
													lServiceOutInfo.m_PMTPID = MULTL_XMLGetNodeUINT(plXMLServInfo, "pmt_pid", 10);
													lServiceOutInfo.m_PMTVersion = MULTL_XMLGetNodeUINT(plXMLServInfo, "pmt_v", 10);
													lServiceOutInfo.m_PMTInterval = MULTL_XMLGetNodeUINT(plXMLServInfo, "pmt_interval", 10);
													lServiceOutInfo.m_PMTActiveMark = MULTL_XMLGetNodeMark(plXMLServInfo, "pmt_mark");
													lServiceOutInfo.m_PCRPID = MULTL_XMLGetNodeUINT(plXMLServInfo, "pcr_pid", 10);
#ifdef TRANS_PCR_PID_FUNCTION
													lServiceOutInfo.m_TransPCRPID = MULTL_XMLGetNodeUINT(plXMLServInfo, "trans_pcr_pid", 10);
#endif

													lServiceOutInfo.m_PCRMark = MULTL_XMLGetNodeMark(plXMLServInfo, "pcr_mark");
													lServiceOutInfo.m_ServiceID = MULTL_XMLGetNodeUINT(plXMLServInfo, "service_id", 10);
													lServiceOutInfo.m_ServiceType = MULTL_XMLGetNodeUINT(plXMLServInfo, "service_type", 10);
													lTmpMark = MULTL_XMLGetNodeMark(plXMLServInfo, "eit_schedule_mark");
													if (lTmpMark)
													{
														MPEG2_DB_SERVICE_MARK_SET_HAVE_EIT_SCHEDULE(lServiceOutInfo.m_ServiceMark);
													}
													lTmpMark = MULTL_XMLGetNodeMark(plXMLServInfo, "eit_cn_mark");
													if (lTmpMark)
													{
														MPEG2_DB_SERVICE_MARK_SET_HAVE_EIT_CUR_NEXT(lServiceOutInfo.m_ServiceMark);
													}
													lTmpMark = MULTL_XMLGetNodeMark(plXMLServInfo, "scramble_mark");
													if (lTmpMark)
													{
														MPEG2_DB_SERVICE_MARK_SET_SCRAMBLE(lServiceOutInfo.m_ServiceMark);
													}

													plTmpBuf = MULTL_XMLGetNodeText(plXMLServInfo, "service_name");
													if (plTmpBuf)
													{
														if (GLOBAL_STRLEN(plTmpBuf) < sizeof(lServiceOutInfo.m_ServiceName))
														{
															GLOBAL_STRCPY(lServiceOutInfo.m_ServiceName, plTmpBuf);
														}
													}
													else
													{
														lServiceOutInfo.m_ServiceName[0] = 0;
													}

													plTmpBuf = MULTL_XMLGetNodeText(plXMLServInfo, "service_provider");
													if (plTmpBuf)
													{
														if (GLOBAL_STRLEN(plTmpBuf) < sizeof(lServiceOutInfo.m_ServiceProviderName))
														{
															GLOBAL_STRCPY(lServiceOutInfo.m_ServiceProviderName, plTmpBuf);
														}
													}
													else
													{
														lServiceOutInfo.m_ServiceProviderName[0] = 0;
													}


													plXMLDescRoot = mxmlFindElement(plXMLServInfo, plXMLServInfo, "pmt_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
													if (plXMLDescRoot)
													{
														MULTL_XMLLoadDescriptors(pHandle, plXMLDescRoot, MPEG2_DESCRIPTOR_PMT_SERVICE_INFO_USER, lServiceIDs);
													}

													plXMLDescRoot = mxmlFindElement(plXMLServInfo, plXMLServInfo, "sdt_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
													if (plXMLDescRoot)
													{
														MULTL_XMLLoadDescriptors(pHandle, plXMLDescRoot, MPEG2_DESCRIPTOR_SDT_SERVICE_INFO_USER, lServiceIDs);
													}

													plXMLCAs = mxmlFindElement(plXMLServInfo, plXMLServInfo, "scs_cas", NULL, NULL, MXML_DESCEND_FIRST);
													if (plXMLCAs)
													{
														MULTL_XMLLoadSCSCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_SERVICE, lServiceIDs, &lACIDsArray);
													}

#ifdef MULT_ENABLE_ECA_AND_SERVICE_LIST
													lServiceOutInfo.m_AuthClass = MULTL_XMLGetNodeUINT(plXMLServInfo, "auth_class", 10);
													lServiceOutInfo.m_LCN = MULTL_XMLGetNodeUINT(plXMLServInfo, "lcn", 10);
													lServiceOutInfo.m_LCNVisibale = MULTL_XMLGetNodeMark(plXMLServInfo, "lcn_visiable");
													lServiceOutInfo.m_BouquetID = MULTL_XMLGetNodeUINT(plXMLServInfo, "bouquet_id", 10);
#endif
													MPEG2_DBSetServiceOutInfo(lDBSHandle, lServiceIDs, &lServiceOutInfo);
												}
											}

											plXMLESs = mxmlFindElement(plXMLServ, plXMLServ, "element_streams", NULL, NULL, MXML_DESCEND_FIRST);

											plXMLES = mxmlFindElement(plXMLESs, plXMLESs, "es", NULL, NULL, MXML_DESCEND_FIRST);
											while(plXMLES)
											{
												lEsIDs = MPEG2_DBAddEs(lDBSHandle, lServiceIDs);
												if (lEsIDs > 0)
												{
													if (MPEG2_DBGetEsInInfo(lDBSHandle, lEsIDs, &lEsInInfo))
													{
														plXMLESInfo = mxmlFindElement(plXMLES, plXMLES, "in_es_info", NULL, NULL, MXML_DESCEND_FIRST);
														if (plXMLESInfo)
														{
															lEsInInfo.m_EsPID = MULTL_XMLGetNodeUINT(plXMLESInfo, "es_pid", 10);
															lEsInInfo.m_EsType = MULTL_XMLGetNodeUINT(plXMLESInfo, "es_type", 10);

															plXMLDescRoot = mxmlFindElement(plXMLESInfo, plXMLESInfo, "es_pmt_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
															if (plXMLDescRoot)
															{
																MULTL_XMLLoadDescriptors(pHandle, plXMLDescRoot, MPEG2_DESCRIPTOR_PMT_ES_INFO, lEsIDs);
															}

															plXMLCAs = mxmlFindElement(plXMLESInfo, plXMLESInfo, "mux_cas", NULL, NULL, MXML_DESCEND_FIRST);
															if (plXMLCAs)
															{
																MULTL_XMLLoadMuxCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_ES, lEsIDs);
															}
															MPEG2_DBSetEsInInfo(lDBSHandle, lEsIDs, &lEsInInfo);
														}
													}

													if (MPEG2_DBGetEsOutInfo(lDBSHandle, lEsIDs, &lEsOutInfo))
													{
														plXMLESInfo = mxmlFindElement(plXMLES, plXMLES, "out_es_info", NULL, NULL, MXML_DESCEND_FIRST);
														if (plXMLESInfo)
														{
															lEsOutInfo.m_EsPID = MULTL_XMLGetNodeUINT(plXMLESInfo, "es_pid", 10);
															lEsOutInfo.m_EsType = MULTL_XMLGetNodeUINT(plXMLESInfo, "es_type", 10);
															lEsOutInfo.m_OutputMark = MULTL_XMLGetNodeMark(plXMLESInfo, "es_out_mark");
															lEsOutInfo.m_ScrambleMark = MULTL_XMLGetNodeMarkDefault(plXMLESInfo, "es_scr_mark", TRUE);
															plXMLDescRoot = mxmlFindElement(plXMLESInfo, plXMLESInfo, "es_pmt_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
															if (plXMLDescRoot)
															{
																MULTL_XMLLoadDescriptors(pHandle, plXMLDescRoot, MPEG2_DESCRIPTOR_PMT_ES_INFO_USER, lEsIDs);
															}


															plXMLCAs = mxmlFindElement(plXMLESInfo, plXMLESInfo, "scs_cas", NULL, NULL, MXML_DESCEND_FIRST);
															if (plXMLCAs)
															{
																MULTL_XMLLoadSCSCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_ES, lEsIDs, &lACIDsArray);
															}
															MPEG2_DBSetEsOutInfo(lDBSHandle, lEsIDs, &lEsOutInfo);
														}
													}
												}
												plXMLES = mxmlFindElement(plXMLES, plXMLESs, "es", NULL, NULL, MXML_NO_DESCEND);
											}
										}
										plXMLServ = mxmlFindElement(plXMLServ, plXMLServiceS, "service", NULL, NULL, MXML_NO_DESCEND);
									}
								}
							}



							/*Ts信息*/
							{
								mxml_node_t *plXMLTSs;
								mxml_node_t *plXMLIOTSs;
								mxml_node_t *plXMLTS;
								mxml_node_t *plXMLTsInfo;
								mxml_node_t *plXMLCAs;
								mxml_node_t *plXMLDescs;

								S32 lIOTsNum, lIOTsIndex;
								U32 lTsIDs;
								MPEG2_DBTsInInfo	lInTsInfo;
								MPEG2_DBTsOutInfo	lOutTsInfo;
								MPEG2_DBTsRouteInfo	lRouteInfo;

								plXMLTSs = mxmlFindElement(plXMLRoot, plXMLRoot, "transport_streams", NULL, NULL, MXML_DESCEND_FIRST);

								/*输入*/
								plXMLIOTSs = mxmlFindElement(plXMLTSs, plXMLTSs, "input_ts", NULL, NULL, MXML_DESCEND_FIRST);
								lIOTsNum = MPEG2_DBGetTsCount(lDBSHandle, TRUE);

								lIOTsIndex = 0;
								plXMLTS = mxmlFindElement(plXMLIOTSs, plXMLIOTSs, "ts", NULL, NULL, MXML_DESCEND_FIRST);
								while(plXMLTS != NULL)
								{
									lIOTsIndex++;
									plXMLTS = mxmlFindElement(plXMLTS, plXMLIOTSs, "ts", NULL, NULL, MXML_NO_DESCEND);
								}

								if (lIOTsIndex == lIOTsNum)
								{
									lIOTsIndex = 0;
									plXMLTS = mxmlFindElement(plXMLIOTSs, plXMLIOTSs, "ts", NULL, NULL, MXML_DESCEND_FIRST);
									while(plXMLTS != NULL)
									{
										lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, TRUE, lIOTsIndex);
										if (lTsIDs > 0)
										{
											plXMLTsInfo = mxmlFindElement(plXMLTS, plXMLTS, "ts_info", NULL, NULL, MXML_DESCEND_FIRST);
											if (plXMLTsInfo)
											{
												if (MPEG2_DBGetTsInInfo(lDBSHandle, lTsIDs, &lInTsInfo))
												{
													lInTsInfo.m_TsID = MULTL_XMLGetNodeUINT(plXMLTsInfo, "ts_id", 10);
													lInTsInfo.m_ONID = MULTL_XMLGetNodeUINT(plXMLTsInfo, "on_id", 10);
													lInTsInfo.m_PATVersion = MULTL_XMLGetNodeUINT(plXMLTsInfo, "pat_v", 10);
													lInTsInfo.m_PATCRC32 = MULTL_XMLGetNodeUINT(plXMLTsInfo, "pat_crc32", 16);
													lInTsInfo.m_SDTVersion = MULTL_XMLGetNodeUINT(plXMLTsInfo, "sdt_v", 10);
													lInTsInfo.m_CATVersion = MULTL_XMLGetNodeUINT(plXMLTsInfo, "cat_v", 10);
													MPEG2_DBSetTsInInfo(lDBSHandle, lTsIDs, &lInTsInfo);
												}
											}

											//输入TS不读取这个信息
 											//plXMLTsInfo = mxmlFindElement(plXMLTS, plXMLTS, "route_info", NULL, NULL, MXML_DESCEND_FIRST);
 											//if (plXMLTsInfo)
 											//{
 											//	if (MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, TRUE, &lRouteInfo))
 											//	{
 											//		lRouteInfo.m_TsIndex = MULTL_XMLGetNodeINT(plXMLTsInfo, "ts_index", 10);
 											//		lRouteInfo.m_ActiveMark = MULTL_XMLGetNodeMark(plXMLTsInfo, "route_mark");
 											//		MPEG2_DBSetTsRouteInfo(lDBSHandle, lTsIDs, TRUE, &lRouteInfo);
 											//	}
 											//}

											plXMLCAs = mxmlFindElement(plXMLTS, plXMLTS, "mux_cas", NULL, NULL, MXML_DESCEND_FIRST);
											if (plXMLCAs)
											{
												MULTL_XMLLoadMuxCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_TS, lTsIDs);
											}


											plXMLDescs = mxmlFindElement(plXMLTS, plXMLTS, "cat_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
											if (plXMLDescs)
											{
												MULTL_XMLLoadDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_CAT, lTsIDs);
											}
										}


										lIOTsIndex++;
										plXMLTS = mxmlFindElement(plXMLTS, plXMLIOTSs, "ts", NULL, NULL, MXML_NO_DESCEND);
									}
								}

								/*输出*/
								plXMLIOTSs = mxmlFindElement(plXMLTSs, plXMLTSs, "output_ts", NULL, NULL, MXML_DESCEND_FIRST);
								lIOTsNum = MPEG2_DBGetTsCount(lDBSHandle, FALSE);

								lIOTsIndex = 0;
								plXMLTS = mxmlFindElement(plXMLIOTSs, plXMLIOTSs, "ts", NULL, NULL, MXML_DESCEND_FIRST);
								while(plXMLTS != NULL)
								{
									lIOTsIndex++;
									plXMLTS = mxmlFindElement(plXMLTS, plXMLIOTSs, "ts", NULL, NULL, MXML_NO_DESCEND);
								}

								if (lIOTsIndex == lIOTsNum)
								{
									lIOTsIndex = 0;
									plXMLTS = mxmlFindElement(plXMLIOTSs, plXMLIOTSs, "ts", NULL, NULL, MXML_DESCEND_FIRST);
									while(plXMLTS != NULL)
									{
										lTsIDs = MPEG2_DBGetTsIDs(lDBSHandle, FALSE, lIOTsIndex);
										if (lTsIDs)
										{
											plXMLTsInfo = mxmlFindElement(plXMLTS, plXMLTS, "ts_info", NULL, NULL, MXML_DESCEND_FIRST);
											if (plXMLTsInfo)
											{
												if (MPEG2_DBGetTsOutInfo(lDBSHandle, lTsIDs, &lOutTsInfo))
												{
													lOutTsInfo.m_TsID = MULTL_XMLGetNodeUINT(plXMLTsInfo, "ts_id", 10);
													lOutTsInfo.m_ONID = MULTL_XMLGetNodeUINT(plXMLTsInfo, "on_id", 10);
													lOutTsInfo.m_PATVersion = MULTL_XMLGetNodeUINT(plXMLTsInfo, "pat_v", 10);
													lOutTsInfo.m_PATInterval = MULTL_XMLGetNodeUINT(plXMLTsInfo, "pat_i", 10);
													lOutTsInfo.m_PATActiveMark = MULTL_XMLGetNodeMark(plXMLTsInfo, "pat_mark");
													lOutTsInfo.m_CATVersion = MULTL_XMLGetNodeUINT(plXMLTsInfo, "cat_v", 10);
													lOutTsInfo.m_CATInterval = MULTL_XMLGetNodeUINT(plXMLTsInfo, "cat_i", 10);
													lOutTsInfo.m_CATActiveMark = MULTL_XMLGetNodeMark(plXMLTsInfo, "cat_mark");

													lOutTsInfo.m_SDTVersion = MULTL_XMLGetNodeUINT(plXMLTsInfo, "sdt_v", 10);
													lOutTsInfo.m_SDTInterval = MULTL_XMLGetNodeUINT(plXMLTsInfo, "sdt_i", 10);
													lOutTsInfo.m_SDTActiveMark = MULTL_XMLGetNodeMark(plXMLTsInfo, "sdt_mark");

													lOutTsInfo.m_NITInterval = MULTL_XMLGetNodeUINT(plXMLTsInfo, "nit_i", 10);
													lOutTsInfo.m_NITActiveMark = MULTL_XMLGetNodeMark(plXMLTsInfo, "nit_mark");

													lOutTsInfo.m_TDTTOTInterval = MULTL_XMLGetNodeUINT(plXMLTsInfo, "tdttot_i", 10);
#ifdef GN2000
													lOutTsInfo.m_TDTTOTActiveMark = FALSE;
#else
													lOutTsInfo.m_TDTTOTActiveMark = MULTL_XMLGetNodeMark(plXMLTsInfo, "tdttot_mark");
#endif


													MPEG2_DBSetTsOutInfo(lDBSHandle, lTsIDs, &lOutTsInfo);
												}
											}

											plXMLTsInfo = mxmlFindElement(plXMLTS, plXMLTS, "route_info", NULL, NULL, MXML_DESCEND_FIRST);
											if (plXMLTsInfo)
											{
												if (MPEG2_DBGetTsRouteInfo(lDBSHandle, lTsIDs, FALSE, &lRouteInfo))
												{
													lRouteInfo.m_TsIndex = MULTL_XMLGetNodeINT(plXMLTsInfo, "ts_index", 10);
													if (lIOTsIndex >= pHandle->m_Information.m_LicenseOutTsNum)
													{
														lRouteInfo.m_ActiveMark = FALSE;
													}
													else
													{
														lRouteInfo.m_ActiveMark = MULTL_XMLGetNodeMark(plXMLTsInfo, "route_mark");
													}
													MPEG2_DBSetTsRouteInfo(lDBSHandle, lTsIDs, &lRouteInfo);
												}
											}

											plXMLCAs = mxmlFindElement(plXMLTS, plXMLTS, "scs_cas", NULL, NULL, MXML_DESCEND_FIRST);
											if (plXMLCAs)
											{
												MULTL_XMLLoadSCSCAs(pHandle, plXMLCAs, MPEG2_DB_CA_TYPE_TS, lTsIDs, &lACIDsArray);
											}


											plXMLDescs = mxmlFindElement(plXMLTS, plXMLTS, "cat_descriptors", NULL, NULL, MXML_DESCEND_FIRST);
											if (plXMLDescs)
											{
												MULTL_XMLLoadDescriptors(pHandle, plXMLDescs, MPEG2_DESCRIPTOR_CAT_USER, lTsIDs);
											}

											lIOTsIndex++;
											plXMLTS = mxmlFindElement(plXMLTS, plXMLIOTSs, "ts", NULL, NULL, MXML_NO_DESCEND);
										}
									}
								}
							}
							MULTL_DestroyIDs(&lACIDsArray);//销毁ACIDs 已经不需要了。




							/*通道信息*/
							{
								MULT_Parameter *plSystemParam;
								MULT_IGMP *plIGMP;
								MULT_ChannelNode *plChnNode;
								MULT_SubChannelNode *plSubNode;

								mxml_node_t *plXMLChnRoot;
								mxml_node_t *plXMLIGMP;
								mxml_node_t *plXMLIOChnRoot;
								mxml_node_t *plXMLChnNode;
								mxml_node_t *plXMLSubChnRoot;
								mxml_node_t *plXMLSubNode;
								S32 lChnIndex, lSubChnIndex;
								S32 lChnType, lSubType;

								plSystemParam = &pHandle->m_Parameter;

								plXMLChnRoot = mxmlFindElement(plXMLRoot, plXMLRoot, "channels", NULL, NULL, MXML_DESCEND_FIRST);

								plXMLIGMP = mxmlFindElement(plXMLChnRoot, plXMLChnRoot, "igmp", NULL, NULL, MXML_DESCEND_FIRST);
								if (plXMLIGMP)
								{
									plIGMP = &plSystemParam->m_IGMP;
									plIGMP->m_IGMPRepeateTime = MULTL_XMLGetNodeINT(plXMLIGMP, "igmp_repeate_time", 10);
									plIGMP->m_IGMPRepeateMark = MULTL_XMLGetNodeMark(plXMLIGMP, "igmp_repeate_mark");
									plIGMP->m_IGMPVersion = MULTL_XMLGetNodeINT(plXMLIGMP, "igmp_version", 10);
								}	


								plXMLIOChnRoot = mxmlFindElement(plXMLChnRoot, plXMLChnRoot, "input_channels", NULL, NULL, MXML_DESCEND_FIRST);

								lChnIndex = 0;
								plXMLChnNode = mxmlFindElement(plXMLIOChnRoot, plXMLIOChnRoot, "channel", NULL, NULL, MXML_DESCEND_FIRST);
								while(plXMLChnNode != NULL)
								{
									lChnIndex++;
									plXMLChnNode = mxmlFindElement(plXMLChnNode, plXMLIOChnRoot, "channel", NULL, NULL, MXML_NO_DESCEND);
								}

								if (lChnIndex == plSystemParam->m_InChannelNumber)
								{

									lChnIndex = 0;
									plXMLChnNode = mxmlFindElement(plXMLIOChnRoot, plXMLIOChnRoot, "channel", NULL, NULL, MXML_DESCEND_FIRST);
									while(plXMLChnNode != NULL)
									{
										plChnNode = &plSystemParam->m_pInChannel[lChnIndex];

										lChnType = MULTL_XMLChnTypeValueFromStr(MULTL_XMLGetNodeText(plXMLChnNode, "type"));
										lSubType = MULTL_XMLChnSubTypeValueFromStr(MULTL_XMLGetNodeText(plXMLChnNode, "sub_type"));

										if ((lChnType == plChnNode->m_ChannelType) && (lSubType == plChnNode->m_SubType))
										{
											MULTL_LoadChannelInfoXML(pHandle, plChnNode, plXMLChnNode);


											plXMLSubChnRoot = mxmlFindElement(plXMLChnNode, plXMLChnNode, "sub_channels", NULL, NULL, MXML_DESCEND_FIRST);

											//lSubChnIndex = 0;
											//plXMLSubNode = mxmlFindElement(plXMLSubChnRoot, plXMLSubChnRoot, "sub_channel", NULL, NULL, MXML_DESCEND_FIRST);
											//while(plXMLSubNode != NULL)
											//{
											//	lSubChnIndex++;
											//	plXMLSubNode = mxmlFindElement(plXMLSubNode, plXMLSubChnRoot, "sub_channel", NULL, NULL, MXML_NO_DESCEND);
											//}

											//if (lSubChnIndex == plChnNode->m_SubChannelNumber)
											{
												lSubChnIndex = 0;
												plXMLSubNode = mxmlFindElement(plXMLSubChnRoot, plXMLSubChnRoot, "sub_channel", NULL, NULL, MXML_DESCEND_FIRST);
												while(plXMLSubNode != NULL)
												{
													if (lSubChnIndex < plChnNode->m_SubChannelNumber)
													{
														plSubNode = &plChnNode->m_pSubChannelNode[lSubChnIndex];
														MULTL_LoadSubChannelInfoXML(pHandle, plChnNode, plSubNode, plXMLSubNode);
													}
													lSubChnIndex++;
													plXMLSubNode = mxmlFindElement(plXMLSubNode, plXMLSubChnRoot, "sub_channel", NULL, NULL, MXML_NO_DESCEND);
												}
											}
										}
										else
										{
											GLOBAL_TRACE(("Error In Chn Type Not Match = %d/%d       %d/%d\n", lChnType, plChnNode->m_ChannelType, lSubType, plChnNode->m_SubType));
										}
										lChnIndex++;
										plXMLChnNode = mxmlFindElement(plXMLChnNode, plXMLIOChnRoot, "channel", NULL, NULL, MXML_NO_DESCEND);
									}
								}
								else
								{
									GLOBAL_TRACE(("Error In Chn Num Not Match = %d/%d\n", lChnIndex, plSystemParam->m_InChannelNumber));
								}


								plXMLIOChnRoot = mxmlFindElement(plXMLChnRoot, plXMLChnRoot, "output_channels", NULL, NULL, MXML_DESCEND_FIRST);

								lChnIndex = 0;
								plXMLChnNode = mxmlFindElement(plXMLIOChnRoot, plXMLIOChnRoot, "channel", NULL, NULL, MXML_DESCEND_FIRST);
								while(plXMLChnNode != NULL)
								{
									lChnIndex++;
									plXMLChnNode = mxmlFindElement(plXMLChnNode, plXMLIOChnRoot, "channel", NULL, NULL, MXML_NO_DESCEND);
								}

								if (lChnIndex == plSystemParam->m_OutChannelNumber)
								{

									lChnIndex = 0;
									plXMLChnNode = mxmlFindElement(plXMLIOChnRoot, plXMLIOChnRoot, "channel", NULL, NULL, MXML_DESCEND_FIRST);
									while(plXMLChnNode != NULL)
									{
										plChnNode = &plSystemParam->m_pOutChannel[lChnIndex];

										lChnType = MULTL_XMLChnTypeValueFromStr(MULTL_XMLGetNodeText(plXMLChnNode, "type"));
										lSubType = MULTL_XMLChnSubTypeValueFromStr(MULTL_XMLGetNodeText(plXMLChnNode, "sub_type"));
										if ((lChnType == plChnNode->m_ChannelType) && (lSubType == plChnNode->m_SubType))
										{
											MULTL_LoadChannelInfoXML(pHandle, plChnNode, plXMLChnNode);


											plXMLSubChnRoot = mxmlFindElement(plXMLChnNode, plXMLChnNode, "sub_channels", NULL, NULL, MXML_DESCEND_FIRST);

											//lSubChnIndex = 0;
											//plXMLSubNode = mxmlFindElement(plXMLSubChnRoot, plXMLSubChnRoot, "sub_channel", NULL, NULL, MXML_DESCEND_FIRST);
											//while(plXMLSubNode != NULL)
											//{
											//	lSubChnIndex++;
											//	plXMLSubNode = mxmlFindElement(plXMLSubNode, plXMLSubChnRoot, "sub_channel", NULL, NULL, MXML_NO_DESCEND);
											//}

											//if (lSubChnIndex == plChnNode->m_SubChannelNumber)
											{
												lSubChnIndex = 0;
												plXMLSubNode = mxmlFindElement(plXMLSubChnRoot, plXMLSubChnRoot, "sub_channel", NULL, NULL, MXML_DESCEND_FIRST);
												while(plXMLSubNode != NULL)
												{
													if (lSubChnIndex < plChnNode->m_SubChannelNumber)
													{
														plSubNode = &plChnNode->m_pSubChannelNode[lSubChnIndex];
														MULTL_LoadSubChannelInfoXML(pHandle, plChnNode, plSubNode, plXMLSubNode);

													}
													lSubChnIndex++;
													plXMLSubNode = mxmlFindElement(plXMLSubNode, plXMLSubChnRoot, "sub_channel", NULL, NULL, MXML_NO_DESCEND);
												}
											}
										}
										else
										{
											GLOBAL_TRACE(("Error Out Chn Type Not Match = %d/%d       %d/%d\n", lChnType, plChnNode->m_ChannelType, lSubType, plChnNode->m_SubType));
										}
										lChnIndex++;
										plXMLChnNode = mxmlFindElement(plXMLChnNode, plXMLIOChnRoot, "channel", NULL, NULL, MXML_NO_DESCEND);
									}
								}
								else
								{
									GLOBAL_TRACE(("Error Out Chn Num Not Match = %d/%d\n", lChnIndex, plSystemParam->m_OutChannelNumber));
								}

							}


#ifdef MULT_SYSTEM_HAVE_PCR_CORRECT_ADJUST_FUNCTION
							{
								mxml_node_t *plXMLHolder;
								MULT_PCRCorrect *plPCRCorrect = &pHandle->m_Parameter.m_PCRCorrect;
								plXMLHolder = mxmlFindElement(plXMLRoot, plXMLRoot, "pcr_correct", NULL, NULL, MXML_DESCEND_FIRST);

								plPCRCorrect->m_PCRCMark = MULTL_XMLGetNodeMarkDefault(plXMLHolder, "pcrc_mark", TRUE);
								plPCRCorrect->m_PCRCPos = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "pcrc_pos", 10, MULT_PCR_CORRECT_POS_DEFAULT_VALUE);
								plPCRCorrect->m_PCRCNeg = MULTL_XMLGetNodeUINTDefault(plXMLHolder, "pcrc_neg", 10, MULT_PCR_CORRECT_NEG_DEFAULT_VALUE);

							}
#endif

#ifdef GN2000
							MULTL_MOSIACLoadXML(plXMLRoot);
#endif
#ifdef MULT_TS_BACKUP_SUPPORT
							MULT_BPLoadXML(plXMLRoot);
#endif
						}
						else
						{
							/*授权类型不符合，不读取参数*/
							GLOBAL_TRACE(("Wrong License Mode Parameter = %d, Current = %d\n", lAuthMode, pHandle->m_Information.m_LicenseMode));
						}
					}
				}
				mxmlDelete(plXML);
				plXML = NULL;
			}
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
		else
		{
			GLOBAL_TRACE(("No Parameter File\n"));
		}
	}
	MULTL_SaveParameterXML(pHandle); //参数保存在应用之后
}



void MULTL_ReleaseParameter(MULT_Handle *pHandle)
{
	S32 i;
	MULT_Parameter *plSystemParam;
	// 	MULT_ChannelNode *plChnNode;
	// 	MULT_SubChannelNode *plSubNode;

	plSystemParam = &pHandle->m_Parameter;
	for (i = 0; i < plSystemParam->m_InChannelNumber; i++)
	{
		GLOBAL_SAFEFREE(plSystemParam->m_pInChannel[i].m_pSubChannelNode);
	}
	GLOBAL_SAFEFREE(plSystemParam->m_pInChannel);

	for (i = 0; i < plSystemParam->m_OutChannelNumber; i++)
	{
		GLOBAL_SAFEFREE(plSystemParam->m_pOutChannel[i].m_pSubChannelNode);
	}
	GLOBAL_SAFEFREE(plSystemParam->m_pOutChannel);
}

#ifdef GN1846
void MULTL_DefaultEdidXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	S32 i;

	for (i = 0; i < EDID_CHIP_NUM; i++) {
		pHandle->m_EdidInfo.m_EdidType[i] = HDMI_RX_EDID_NONE;
	}
}

void MULTL_SaveEdidXML(MULT_Handle *pHandle)
{
	S32 i, k, lLogNum;
	MULTL_Edid *plEdidInfo;
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	mxml_node_t *plXMLHolder;

	plEdidInfo = &pHandle->m_EdidInfo;

	plXML = mxmlNewXML("1.0");
	plXMLRoot = mxmlNewElement(plXML, "root");

	MULTL_XMLAddNodeUINT(plXMLRoot, "edid_chip_num", EDID_CHIP_NUM);
	for (i = 0; i < EDID_CHIP_NUM; i++)
	{
		plXMLHolder = mxmlNewElement(plXMLRoot, "edid");
		MULTL_XMLAddNodeText(plXMLHolder, "edid_type", MULTL_XMLEdidTypeValueToStr(plEdidInfo->m_EdidType[i]));
	}

	{
		CHAR_T plPathStr[1024];
		GLOBAL_FD plFile;

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_EDID_SETTINGS_XML);

		plFile = GLOBAL_FOPEN(plPathStr, "w");
		if (plFile != NULL)
		{
			mxmlSaveFile(plXML, plFile, NULL);
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
			GLOBAL_SPRINTF((plPathStr, "cp -f %s%s %s", MULT_XML_BASE_DIR, MULT_EDID_SETTINGS_XML, MULT_STORAGE_BASE_DIR));
			PFC_System(plPathStr);
		}
	}

	mxmlDelete(plXML);
	plXML = NULL;
}

void MULTL_LoadEdidXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	mxml_node_t *plXMLHolder;
	S32 i = 0;

	MULTL_DefaultEdidXML(pHandle, pHWInfo);
	{
		CHAR_T plPathStr[1024];
		GLOBAL_FD plFile;

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_EDID_SETTINGS_XML);
		plFile = GLOBAL_FOPEN(plPathStr, "r");
		if (plFile != NULL)
		{
			plXML = mxmlLoadFile(NULL, plFile,  MXML_OPAQUE_CALLBACK);
			if (plXML)
			{
				/*分析并校验参数文件，容许XML文件缺少部分属性以应对系统升级！！！*/
				plXMLRoot = mxmlFindElement(plXML, plXML, "root", NULL, NULL, MXML_DESCEND_FIRST);
				if (plXMLRoot)
				{
					plXMLHolder = mxmlFindElement(plXMLRoot, plXMLRoot, "edid", NULL, NULL, MXML_DESCEND_FIRST);
					while(plXMLHolder)
					{
						if (i < EDID_CHIP_NUM) {
							pHandle->m_EdidInfo.m_EdidType[i++] = MULTL_XMLEdidTypeValueFromStr(MULTL_XMLGetNodeText(plXMLHolder, "edid_type"));
						}

						plXMLHolder = mxmlFindElement(plXMLHolder, plXMLRoot, "edid", NULL, NULL, MXML_NO_DESCEND);
					}
				}
				mxmlDelete(plXML);
				plXML = NULL;
			}
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
		else
		{
			GLOBAL_TRACE(("No Edid Setting File\n"));
		}

	}
	MULTL_SaveEdidXML(pHandle);
}
#endif

/*监控信息*/
void MULTL_DefaultMonitorSetting(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	S32 i, lInTsIndex, lOutTsIndex;
	MULT_Monitor *plMonitor;
	MULT_MonitorCHN *plChn;
	CAL_LogConfig lLogConfig;
	plMonitor = &pHandle->m_Monitor;
	GLOBAL_ZEROMEM(plMonitor, sizeof(MULT_Monitor));

	plMonitor->m_GlobalMark = TRUE;
	plMonitor->m_CriticalTemp = MULT_CRITICAL_TEMP;
	plMonitor->m_FanTemp = MULT_FAN_TEMP;

#ifdef SUPPORT_NEW_HWL_MODULE

	plMonitor->m_InChnNum = pHWInfo->m_InChnNum;
	plMonitor->m_OutChnNum = pHWInfo->m_OutChnNum;

	plMonitor->m_pInChnArray = (MULT_MonitorCHN *)GLOBAL_ZMALLOC(sizeof(MULT_MonitorCHN) * plMonitor->m_InChnNum);
	plMonitor->m_pOutChnArray = (MULT_MonitorCHN *)GLOBAL_ZMALLOC(sizeof(MULT_MonitorCHN) * plMonitor->m_OutChnNum);

	lInTsIndex = lOutTsIndex = 0;
	for (i = 0; i < pHWInfo->m_InChnNum; i++)
	{
		plChn = &plMonitor->m_pInChnArray[lInTsIndex];
		lInTsIndex++;
		plChn->m_SubNumber = pHWInfo->m_pInChn[i].m_CurSubSupport;
		plChn->m_pSubArray = (MULT_MonitorSUB *)GLOBAL_ZMALLOC(sizeof(MULT_MonitorSUB) * plChn->m_SubNumber);
		plChn->m_StartTsIndex = pHWInfo->m_pInChn[i].m_StartTsIndex;
	}
	for (i = 0; i < pHWInfo->m_OutChnNum; i++)
	{
		plChn = &plMonitor->m_pOutChnArray[lOutTsIndex];
		lOutTsIndex++;
		plChn->m_SubNumber = pHWInfo->m_pOutChn[i].m_CurSubSupport;
		plChn->m_pSubArray = (MULT_MonitorSUB *)GLOBAL_ZMALLOC(sizeof(MULT_MonitorSUB) * plChn->m_SubNumber);
		plChn->m_StartTsIndex = pHWInfo->m_pOutChn[i].m_StartTsIndex;
	}

	plMonitor->m_ETHNumber = HWL_MonitorHWInfoETHChnNum(pHWInfo);

	GLOBAL_TRACE(("Total ETH Number = %d\n", plMonitor->m_ETHNumber));
#else
	for (i = 0; i < pHWInfo->m_ChannelNum; i++)
	{
		if (pHWInfo->m_pInfoList[i].m_Direction == HWL_CHANNEL_DIRECTION_IN)
		{
			plMonitor->m_InChnNum++;
		}
		else
		{
			plMonitor->m_OutChnNum++;
		}
	}

	plMonitor->m_pInChnArray = (MULT_MonitorCHN *)GLOBAL_ZMALLOC(sizeof(MULT_MonitorCHN) * plMonitor->m_InChnNum);
	plMonitor->m_pOutChnArray = (MULT_MonitorCHN *)GLOBAL_ZMALLOC(sizeof(MULT_MonitorCHN) * plMonitor->m_OutChnNum);

	lInTsIndex = lOutTsIndex = 0;
	for (i = 0; i < pHWInfo->m_ChannelNum; i++)
	{
		if (pHWInfo->m_pInfoList[i].m_Direction == HWL_CHANNEL_DIRECTION_IN)
		{
			plChn = &plMonitor->m_pInChnArray[lInTsIndex];
			lInTsIndex++;
		}
		else
		{
			plChn = &plMonitor->m_pOutChnArray[lOutTsIndex];
			lOutTsIndex++;
		}
		plChn->m_SubNumber = pHWInfo->m_pInfoList[i].m_CurSubSupport;
		plChn->m_pSubArray = (MULT_MonitorSUB *)GLOBAL_ZMALLOC(sizeof(MULT_MonitorSUB) * plChn->m_SubNumber);
		plChn->m_StartTsIndex = pHWInfo->m_pInfoList[i].m_StartTsIndex;
	}
#endif
	/*---特殊处理 tuner-s  ----*/

#ifdef SUPPORT_NEW_ALARM_SYSTEM
	plMonitor->m_LogHandle = CAL_LogCreate(MULT_MONITOR_TYPE_NUM, 255);
#if defined(GM7000)
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_PLL, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_BUFFER_STATUS, TRUE);
#elif defined(LR1800S)
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_PLL, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);

	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_BUFFER_STATUS, TRUE);
#elif defined(GQ3760B) || defined(GQ3763) || defined(GQ3768) || defined(GQ3765)

	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_PLL, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_BUFFER_STATUS, TRUE);
#elif defined(GM2750)
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);
#elif defined(GQ3760)
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_PLL, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_BUFFER_STATUS, TRUE);
#elif defined(GN1772)
	//CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);
	//CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SCS_EMM, TRUE);
	//CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SCS_ECM, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ETH_LINK, TRUE);
#elif defined(GQ3710B)
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_BUFFER_STATUS, TRUE);
#else
#ifdef GN1846
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_BUFFER_OVERFLOW_ERROR, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_HDMI_INPUT_LOST_ERROR, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_HDMI_INPUT_FORMAT_ERROR, TRUE);
#else
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TEMP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_FPGA, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_PLL, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_IN, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CHANNEL_OUT, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SCS_EMM, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SCS_ECM, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_BUFFER_STATUS, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_TUNER_SHORT_STATUS, TRUE);
#endif
#endif


#ifdef SUPPORT_GNS_MODULE
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_GPS_LOCK_LOST, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_GPS_ERROR, TRUE);
#endif

#ifdef SUPPORT_SFN_MODULATOR
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SFN_SIP_ERROR, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SFN_SIP_CRC32_ERROR, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SFN_SIP_CHANGE, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_10M_LOST, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_1PPS_LOST, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_INT_10M_LOST, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_INT_1PPS_LOST, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_SFN_ERROR, TRUE);
#endif

#ifdef SUPPORT_SFN_ADAPTER
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_10M_LOST, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_EXT_1PPS_LOST, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_INT_10M_LOST, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_INT_1PPS_LOST, TRUE);
#endif

#ifdef SUPPORT_CLK_ADJ_MODULE
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_CLK_SYNC_ERROR, TRUE);
#endif

#ifdef SUPPORT_NTS_DPD_BOARD
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_DPD_PARAMETER_SAVE_ERROR, TRUE);
#endif

#ifdef GM8358Q
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_NOBITRATE, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_OVERFLOW, TRUE);
#endif

#ifdef GM8359Q
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN1, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN2, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN3, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN4, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN5, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN6, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN7, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_CVBS_LOCK_CHN8, TRUE);

	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO1, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO2, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO3, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO4, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO5, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO6, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO7, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_INPUT_ERROR_VIDEO8, TRUE);

	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_NOBITRATE, TRUE);
	CAL_LogSetUsedMark(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_ENCODER_OUTPUT_ERROR_OVERFLOW, TRUE);
#endif

#else
	if(pHandle->m_bHaveShortTest)
	{ 

	}
	else
	{
		plMonitor->m_LogHandle = CAL_LogCreate(MULT_MONITOR_TYPE_NUM - HWL_TUNER_MAX_NUM, 255);
		for (i = 0; i < MULT_MONITOR_TYPE_NUM - HWL_TUNER_MAX_NUM; i++)
		{
			lLogConfig.m_bFP = TRUE;
			lLogConfig.m_bTrap = TRUE;
			lLogConfig.m_LogLevel = CAL_LOG_LEVEL_CRITICAL;
			CAL_LogProcConfig(plMonitor->m_LogHandle, i, &lLogConfig, FALSE);
		}
	}
#endif
}


void MULTL_SaveMonitorXML(MULT_Handle *pHandle)
{
	S32 i, k, lLogNum;
	MULT_Monitor *plMonitor;
	MULT_MonitorCHN *plChn;
	MULT_MonitorSUB *plSub;
	CAL_LogConfig lLogCFG;
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	mxml_node_t *plXMLHolder;
	mxml_node_t *plLogs;
	mxml_node_t *plXMLChns;
	mxml_node_t *plXMLINOUT;
	mxml_node_t *plXMLChn;
	mxml_node_t *plXMLSub;
	mxml_node_t *plXMLData;

	plMonitor = &pHandle->m_Monitor;

	plXML = mxmlNewXML("1.0");
	plXMLRoot = mxmlNewElement(plXML, "root");

	MULTL_XMLAddNodeINT(plXMLRoot, "version_number", 0);

	plXMLData = mxmlNewElement(plXMLRoot, "global_mark");
	mxmlNewTextf(plXMLData, 0, "%s", MULTL_XMLMarkValueToStr(plMonitor->m_GlobalMark));

	plXMLData = mxmlNewElement(plXMLRoot, "critical_temp");
	mxmlNewTextf(plXMLData, 0, "%d", plMonitor->m_CriticalTemp);

	plXMLData = mxmlNewElement(plXMLRoot, "fan_temp");
	mxmlNewTextf(plXMLData, 0, "%d", plMonitor->m_FanTemp);

#ifdef MULT_DEVICE_NO_TEMP_SENSOR
	MULTL_XMLAddNodeMark(plXMLRoot, "no_temp", TRUE);
#else
	MULTL_XMLAddNodeMark(plXMLRoot, "no_temp", FALSE);
#endif

	plXMLData = mxmlNewElement(plXMLRoot, "eth_num");
	mxmlNewTextf(plXMLData, 0, "%d", plMonitor->m_ETHNumber);

	plLogs = mxmlNewElement(plXMLRoot, "log_system");
	lLogNum = CAL_LogGetLogCFGCount(plMonitor->m_LogHandle);
	for (i = 0; i < MULT_MONITOR_TYPE_NUM; i++)
	{
		if (CAL_LogProcConfig(plMonitor->m_LogHandle, i, &lLogCFG, TRUE) == TRUE)
		{
#ifdef SUPPORT_NEW_ALARM_SYSTEM
			if (CAL_LogGetUsedMark(plMonitor->m_LogHandle, i))
#endif
			{
				plXMLHolder = mxmlNewElement(plLogs, "log");

				plXMLData = mxmlNewElement(plXMLHolder, "log_id");
				mxmlNewTextf(plXMLData, 0, "%x", MULT_MONITOR_ENCODE_ID(i));

				plXMLData = mxmlNewElement(plXMLHolder, "trap");
				mxmlNewTextf(plXMLData, 0, "%s", MULTL_XMLMarkValueToStr(lLogCFG.m_bTrap));

				plXMLData = mxmlNewElement(plXMLHolder, "panel");
				mxmlNewTextf(plXMLData, 0, "%s", MULTL_XMLMarkValueToStr(lLogCFG.m_bFP));

				plXMLData = mxmlNewElement(plXMLHolder, "level");
				mxmlNewTextf(plXMLData, 0, "%d", lLogCFG.m_LogLevel);
			}
		}
		else
		{
			GLOBAL_TRACE(("i = %d Error!!!!!!\n", i));
			GLOBAL_ASSERT(0);
		}
	}


	plXMLChns = mxmlNewElement(plXMLRoot, "chn_limils");

	plXMLINOUT = mxmlNewElement(plXMLChns, "in_chn");
	for (i = 0; i < plMonitor->m_InChnNum; i++)
	{
		plChn = &plMonitor->m_pInChnArray[i];
		plXMLChn = mxmlNewElement(plXMLINOUT, "chn");

		MULTL_XMLSaveMonitorLimitInfo(pHandle, plXMLChn, &plChn->m_LimitInfo);

		for (k = 0; k < plChn->m_SubNumber; k++)
		{
			plSub = &plChn->m_pSubArray[k];
			plXMLSub = mxmlNewElement(plXMLChn, "sub");
			MULTL_XMLSaveMonitorLimitInfo(pHandle, plXMLSub, &plSub->m_LimitInfo);
		}
	}


	plXMLINOUT = mxmlNewElement(plXMLChns, "out_chn");
	for (i = 0; i < plMonitor->m_OutChnNum; i++)
	{
		plChn = &plMonitor->m_pOutChnArray[i];
		plXMLChn = mxmlNewElement(plXMLINOUT, "chn");

		MULTL_XMLSaveMonitorLimitInfo(pHandle, plXMLChn, &plChn->m_LimitInfo);

		for (k = 0; k < plChn->m_SubNumber; k++)
		{
			plSub = &plChn->m_pSubArray[k];
			plXMLSub = mxmlNewElement(plXMLChn, "sub");
			MULTL_XMLSaveMonitorLimitInfo(pHandle, plXMLSub, &plSub->m_LimitInfo);
		}
	}


	{
		CHAR_T plPathStr[1024];
		GLOBAL_FD plFile;

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_MONITOR_SETTING_XML);

		plFile = GLOBAL_FOPEN(plPathStr, "w");
		if (plFile != NULL)
		{
			mxmlSaveFile(plXML, plFile, NULL);
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
	}

	mxmlDelete(plXML);
	plXML = NULL;

	MULTL_SetSaveMark(pHandle, FALSE);
}



void MULTL_LoadMonitorXML(MULT_Handle *pHandle, HWL_HWInfo* pHWInfo)
{
	S32 lChnIndex, lSubIndex;
	MULT_Monitor *plMonitor;
	MULT_MonitorCHN *plChn;
	MULT_MonitorSUB *plSub;
	CAL_LogConfig lLogCFG;
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	mxml_node_t *plXMLHolder;
	mxml_node_t *plXMLCurrent;
	mxml_node_t *plXMLChns;
	mxml_node_t *plXMLINOUT;
	mxml_node_t *plXMLChn;
	mxml_node_t *plXMLSub;



	MULTL_DefaultMonitorSetting(pHandle, pHWInfo);//首先填充默认值！
	{
		CHAR_T plPathStr[1024];
		GLOBAL_FD plFile;

		GLOBAL_STRCPY(plPathStr, MULT_XML_BASE_DIR);
		GLOBAL_STRCAT(plPathStr, MULT_MONITOR_SETTING_XML);
		plFile = GLOBAL_FOPEN(plPathStr, "r");
		if (plFile != NULL)
		{
			plXML = mxmlLoadFile(NULL, plFile,  MXML_OPAQUE_CALLBACK);
			if (plXML)
			{
				/*分析并校验参数文件，容许XML文件缺少部分属性以应对系统升级！！！*/
				plXMLRoot = mxmlFindElement(plXML, plXML, "root", NULL, NULL, MXML_DESCEND_FIRST);
				if (plXMLRoot)
				{
					U32 lLogIDs;
					S32 lVersionNumber;

					lVersionNumber = MULTL_XMLGetNodeINT(plXMLRoot, "version_number", 10);
					if (lVersionNumber >= 0)
					{
						GLOBAL_TRACE(("Version Number = %d\n", lVersionNumber)); 
						plMonitor = &pHandle->m_Monitor;

						plMonitor->m_GlobalMark = MULTL_XMLGetNodeMark(plXMLRoot, "global_mark");
						plMonitor->m_CriticalTemp = MULTL_XMLGetNodeINT(plXMLRoot, "critical_temp", 10);
						plMonitor->m_FanTemp = MULTL_XMLGetNodeINT(plXMLRoot, "fan_temp", 10);

						plXMLHolder = mxmlFindElement(plXMLRoot, plXMLRoot, "log_system", NULL, NULL, MXML_DESCEND_FIRST);
						plXMLCurrent = mxmlFindElement(plXMLHolder, plXMLHolder, "log", NULL, NULL, MXML_DESCEND_FIRST);
						while(plXMLCurrent)
						{
							lLogIDs = MULTL_XMLGetNodeUINT(plXMLCurrent, "log_id", 16);
							if (CAL_LogProcConfig(plMonitor->m_LogHandle, MULT_MONITOR_DECODE_ID(lLogIDs), &lLogCFG, TRUE))
							{
								lLogCFG.m_bTrap = MULTL_XMLGetNodeMark(plXMLCurrent, "trap");

								lLogCFG.m_bFP = MULTL_XMLGetNodeMark(plXMLCurrent, "panel");

								lLogCFG.m_LogLevel = MULTL_XMLGetNodeINT(plXMLCurrent, "level", 10);

								CAL_LogProcConfig(plMonitor->m_LogHandle, MULT_MONITOR_DECODE_ID(lLogIDs), &lLogCFG, FALSE);
							}

							plXMLCurrent = mxmlFindElement(plXMLCurrent, plXMLHolder, "log", NULL, NULL, MXML_NO_DESCEND);
						}

						plXMLChns = mxmlFindElement(plXMLRoot, plXMLRoot, "chn_limils", NULL, NULL, MXML_DESCEND_FIRST);
						if (plXMLChns)
						{
							plXMLINOUT = mxmlFindElement(plXMLChns, plXMLChns, "in_chn", NULL, NULL, MXML_DESCEND_FIRST);
							if (plXMLINOUT)
							{
								lChnIndex = 0;
								plXMLChn = mxmlFindElement(plXMLINOUT, plXMLINOUT, "chn", NULL, NULL, MXML_DESCEND_FIRST);
								while(plXMLChn)
								{
									if (GLOBAL_CHECK_INDEX(lChnIndex, plMonitor->m_InChnNum))
									{
										plChn = &plMonitor->m_pInChnArray[lChnIndex];
										MULTL_XMLLoadMonitorLimitInfo(pHandle, plXMLChn, &plChn->m_LimitInfo);
										lSubIndex = 0;
										plXMLSub = mxmlFindElement(plXMLChn, plXMLChn, "sub", NULL, NULL, MXML_DESCEND_FIRST);
										while(plXMLSub)
										{
											if (GLOBAL_CHECK_INDEX(lSubIndex, plChn->m_SubNumber))
											{
												plSub = &plChn->m_pSubArray[lSubIndex];
												MULTL_XMLLoadMonitorLimitInfo(pHandle, plXMLSub, &plSub->m_LimitInfo);
												lSubIndex++;
											}
											plXMLSub = mxmlFindElement(plXMLSub, plXMLChn, "sub", NULL, NULL, MXML_NO_DESCEND);
										}
										lChnIndex++;
									}
									plXMLChn = mxmlFindElement(plXMLChn, plXMLINOUT, "chn", NULL, NULL, MXML_NO_DESCEND);
								}
							}


							plXMLINOUT = mxmlFindElement(plXMLChns, plXMLChns, "out_chn", NULL, NULL, MXML_DESCEND_FIRST);
							if (plXMLINOUT)
							{
								lChnIndex = 0;
								plXMLChn = mxmlFindElement(plXMLINOUT, plXMLINOUT, "chn", NULL, NULL, MXML_DESCEND_FIRST);
								while(plXMLChn)
								{
									if (GLOBAL_CHECK_INDEX(lChnIndex, plMonitor->m_OutChnNum))
									{
										plChn = &plMonitor->m_pOutChnArray[lChnIndex];
										MULTL_XMLLoadMonitorLimitInfo(pHandle, plXMLChn, &plChn->m_LimitInfo);
										lSubIndex = 0;
										plXMLSub = mxmlFindElement(plXMLChn, plXMLChn, "sub", NULL, NULL, MXML_DESCEND_FIRST);
										while(plXMLSub)
										{
											if (GLOBAL_CHECK_INDEX(lSubIndex, plChn->m_SubNumber))
											{
												plSub = &plChn->m_pSubArray[lSubIndex];
												MULTL_XMLLoadMonitorLimitInfo(pHandle, plXMLSub, &plSub->m_LimitInfo);
												lSubIndex++;
											}
											plXMLSub = mxmlFindElement(plXMLSub, plXMLChn, "sub", NULL, NULL, MXML_NO_DESCEND);
										}
										lChnIndex++;
									}
									plXMLChn = mxmlFindElement(plXMLChn, plXMLINOUT, "chn", NULL, NULL, MXML_NO_DESCEND);
								}
							}

						}



					}
					else
					{
						GLOBAL_TRACE(("Version Number Invalid = %d\n", lVersionNumber));
					}
				}
				mxmlDelete(plXML);
				plXML = NULL;
			}
			GLOBAL_FCLOSE(plFile);
			plFile = NULL;
		}
		else
		{
			GLOBAL_TRACE(("No Moniter Setting File\n"));
		}

	}
	// 	MULTL_AdjastMonitorSetting(pHandle);
	MULTL_SaveMonitorXML(pHandle);
}


void MULTL_ReleaseMonitor(MULT_Handle *pHandle)
{
	S32 i;
	MULT_Monitor *plMonitor;

	plMonitor = &pHandle->m_Monitor;
	for (i = 0; i < plMonitor->m_InChnNum; i++)
	{
		GLOBAL_SAFEFREE(plMonitor->m_pInChnArray[i].m_pSubArray);
	}
	GLOBAL_SAFEFREE(plMonitor->m_pInChnArray);

	for (i = 0; i < plMonitor->m_OutChnNum; i++)
	{
		GLOBAL_SAFEFREE(plMonitor->m_pOutChnArray[i].m_pSubArray);
	}
	GLOBAL_SAFEFREE(plMonitor->m_pOutChnArray);

	if (plMonitor->m_LogHandle)
	{
		CAL_LogDestroy(plMonitor->m_LogHandle);
		plMonitor->m_LogHandle = NULL;
	}
}




/*调用系统命令，生成存档文件，并复制到FLASH当中*/
void MULTL_SaveParamterToStorage(MULT_Handle *pHandle)
{
	GLOBAL_FD lReadFD, lWriteFD;
	CHAR_T plCMD[1024];
	U8 *plPayLoadStart;
	S32 lFileSize, lActSize;
	CAL_FCAPHandle *plFCAHandle;

	/*将目录下的所有东西均打包，ARM下的busybox自带tar不支持该功能*/
	GLOBAL_SPRINTF((plCMD, "cd %s\n tar -czf %s ./* \n", MULT_XML_BASE_DIR, MULT_IO_TMP_FILE_PATHNAME));//这里是直接进入目录里面操作的，同样还原的时候也要相应的进入到目录里面！！！！！
	PFC_System(plCMD);

	lReadFD = GLOBAL_FOPEN(MULT_IO_TMP_FILE_PATHNAME, "rb");
	if (lReadFD)
	{
		lFileSize = CAL_FileSize(lReadFD);
		if (lFileSize > 0)
		{
			/*调用程序封装参数文件*/
			GLOBAL_TRACE(("Tar Size = %d\n", lFileSize));
			plFCAHandle = CAL_FCAPEncodeCreate(MULT_DEVICE_COMPLETE_TYPE, MULT_PARAMETER_DESCRIPTOR, lFileSize);
			if (plFCAHandle)
			{
				plPayLoadStart = CAL_FCAPGetPayloadPtr(plFCAHandle);
				if (plPayLoadStart)
				{
					lActSize = GLOBAL_FREAD(plPayLoadStart, 1, lFileSize, lReadFD);
					CAL_FCAPFinalize(plFCAHandle, lFileSize);
					if (lActSize == lFileSize)
					{
						GLOBAL_SPRINTF((plCMD, "%s%s", MULT_STORAGE_BASE_DIR, MULT_PARAMETER_FILE_PATHNAME));
						lWriteFD = GLOBAL_FOPEN(plCMD, "wb");
						if (lWriteFD)
						{
							lActSize = GLOBAL_FWRITE(plFCAHandle->m_pDataBuf, 1, plFCAHandle->m_DataSize, lWriteFD);
							GLOBAL_FCLOSE(lWriteFD);
							if (lActSize == plFCAHandle->m_DataSize)
							{
								GLOBAL_TRACE(("Save Paramters Successful, BIN Size = %d,\n", lActSize));
							}
							else
							{
								GLOBAL_TRACE(("Save Paramters Failed\n"));
							}
						}
						else
						{
							GLOBAL_TRACE(("Open %s for Write Failed!!!\n", MULT_IO_TMP_FILE_PATHNAME));
						}
					}
					else
					{
						GLOBAL_TRACE(("Read File File Error Info: %d/%d\n", lActSize, lFileSize));
					}

				}
				else
				{
					GLOBAL_TRACE(("Get PTR Failed\n"));
				}
				CAL_FCAPDestroy(plFCAHandle);
				plFCAHandle = NULL;
			}
			else
			{
				GLOBAL_TRACE(("Create Handle Failed\n"));
			}
		
		}
		else
		{
			GLOBAL_TRACE(("%s File Size = \n", lFileSize));
		}


		GLOBAL_FCLOSE(lReadFD);
	}
	else
	{
		GLOBAL_TRACE(("Open %s Failed!!!\n", MULT_IO_TMP_FILE_PATHNAME));
	}


	/*删除临时文件*/
	GLOBAL_SPRINTF((plCMD, "rm -r %s", MULT_IO_TMP_FILE_PATHNAME));
 	PFC_System(plCMD);
}

BOOL MULTL_LoadParamterFromStorage(MULT_Handle *pHandle, CHAR_T *pFilePathName)
{
	BOOL lRet = FALSE;
	GLOBAL_FD lReadFD, lWriteFD;
	CHAR_T plCMD[1024];
	U8 *plPayLoadStart;
	S32 lFileSize, lActSize;
	CAL_FCAPHandle *plFCAHandle;

	if (pFilePathName)
	{
		lReadFD = GLOBAL_FOPEN(pFilePathName, "rb");
		if (lReadFD)
		{
			lFileSize = CAL_FileSize(lReadFD);
			if (lFileSize > 0)
			{
				/*调用程序封装参数文件*/
				plFCAHandle = CAL_FCAPDencodeCreate(lFileSize);
				if (plFCAHandle)
				{
					lActSize = GLOBAL_FREAD(plFCAHandle->m_pDataBuf, 1, lFileSize, lReadFD);
					if (lActSize == lFileSize)
					{
						GLOBAL_TRACE(("Load Paramters BIN FileSize = %d\n", lFileSize));
						if (CAL_FCAPValidation(plFCAHandle, MULT_DEVICE_COMPLETE_TYPE, MULT_PARAMETER_DESCRIPTOR))
						{
							GLOBAL_SPRINTF((plCMD, "%s", MULT_IO_TMP_FILE_PATHNAME));
							lWriteFD = GLOBAL_FOPEN(plCMD, "wb");
							if (lWriteFD)
							{
								plPayLoadStart = CAL_FCAPGetPayloadPtr(plFCAHandle);
								lFileSize = CAL_FCAPGetPayloadSize(plFCAHandle);
								lActSize = GLOBAL_FWRITE(plPayLoadStart, 1, lFileSize, lWriteFD);
								GLOBAL_FCLOSE(lWriteFD);
								if (lActSize == lFileSize)
								{
									GLOBAL_SPRINTF((plCMD, "cd %s\ntar -xzf %s\n", MULT_XML_BASE_DIR, MULT_IO_TMP_FILE_PATHNAME));//这里是直接进入目录里面操作的，同样还原的时候也要相应的进入到目录里面！！！！！
									PFC_System(plCMD);

									/*删除临时文件*/
									GLOBAL_SPRINTF((plCMD, "rm -r %s\n", MULT_IO_TMP_FILE_PATHNAME));
									PFC_System(plCMD);

#if 1//2015-2-2 修正参数文件中保存的维护文件的问题！
									/*删除多余的维护文件*/
									PFC_System("rm -r %s/%s\n", MULT_XML_BASE_DIR, MULT_DEVICE_MAINTENANCE_XML);
									
#endif

									GLOBAL_TRACE(("Load Paramters From Successful, Tar Size = %d\n", lFileSize, pFilePathName));
									lRet = TRUE;
								}
								else
								{
									GLOBAL_TRACE(("Write Tmp File Failed\n"));
								}
							}
							else
							{
								GLOBAL_TRACE(("Open %s for Write Failed!!!\n", plCMD));
							}

						}
						else
						{
							GLOBAL_TRACE(("Parameter File Validation Failed\n"));
						}
					}
					else
					{
						GLOBAL_TRACE(("Read File File Error Info: %d/%d\n", lActSize, lFileSize));
					}

					CAL_FCAPDestroy(plFCAHandle);
					plFCAHandle = NULL;
				}
				else
				{
					GLOBAL_TRACE(("Create Handle Failed\n"));
				}
			}
			else
			{
				GLOBAL_TRACE(("%s File Size = \n", lFileSize));
			}
			GLOBAL_FCLOSE(lReadFD);
		}
		else
		{
			GLOBAL_TRACE(("Open %s Failed!!!\n", pFilePathName));
		}

	}
	return lRet;
}


void MULTL_GenerateParamters(MULT_Handle *pHandle, CHAR_T *pFilePathName, CHAR_T* pDescription)
{
	GLOBAL_FD lReadFD, lWriteFD;
	CHAR_T plCMD[1024];
	U8 *plPayLoadStart;
	S32 lFileSize, lActSize;
	CAL_FCAPHandle *plFCAHandle;

	if (pFilePathName)
	{
		//2016-7-26 在ZYNQ的BUSYBOX的TAR命令中，当没有指定文件时，TAR会失败！，所以这里自动生成一个0字节的stuff.bin来绕过这个问题
		{
			GLOBAL_FD lTmpFD;
			GLOBAL_SPRINTF((plCMD, "%sstuff.bin", MULT_XML_BASE_DIR));
			lTmpFD = GLOBAL_FOPEN(plCMD, "wr");
			if (lTmpFD)
			{
				GLOBAL_FCLOSE(lTmpFD);
			}

		}
		/*将目录下通道复用加扰和监控设置参数生成备份文件*/
		GLOBAL_SPRINTF((plCMD, "cd %s\n tar -czf %s", MULT_XML_BASE_DIR, MULT_IO_TMP_FILE_PATHNAME));
		GLOBAL_STRCAT(plCMD, " ./");
		GLOBAL_STRCAT(plCMD, MULT_SYSTEM_PARAMETER_XML);
		GLOBAL_STRCAT(plCMD, " ./");
		GLOBAL_STRCAT(plCMD, MULT_MONITOR_SETTING_XML);
		GLOBAL_STRCAT(plCMD, " ./*.bin");//修正参数导出没有导出PSI插入文件的BUG！20121010

#ifdef ENCODER_CARD_PLATFORM
		GLOBAL_STRCAT(plCMD, " ./");
		GLOBAL_STRCAT(plCMD, CARD_MODULE_PARAMETER_XML_FILE);
#endif
		GLOBAL_STRCAT(plCMD, "\n");
		PFC_System(plCMD);

		lReadFD = GLOBAL_FOPEN(MULT_IO_TMP_FILE_PATHNAME, "rb");
		if (lReadFD)
		{
			lFileSize = CAL_FileSize(lReadFD);
			if (lFileSize > 0)
			{
				/*调用程序封装参数文件*/
				GLOBAL_TRACE(("Tar Size = %d\n", lFileSize));
				plFCAHandle = CAL_FCAPEncodeCreate(MULT_DEVICE_COMPLETE_TYPE, MULT_PARAMETER_DESCRIPTOR, lFileSize);
				if (plFCAHandle)
				{
					plPayLoadStart = CAL_FCAPGetPayloadPtr(plFCAHandle);
					if (plPayLoadStart)
					{
						lActSize = GLOBAL_FREAD(plPayLoadStart, 1, lFileSize, lReadFD);
						if (pDescription)
						{
							GLOBAL_TRACE(("Description = %s\n", pDescription));
							CAL_FCAPSetDescription(plFCAHandle, pDescription);
						}
						else
						{
							GLOBAL_SPRINTF((plCMD, "%s Parameter Mode %d", pHandle->m_Information.m_pModelName, pHandle->m_Information.m_LicenseMode));
							CAL_FCAPSetDescription(plFCAHandle, plCMD);
						}
						CAL_FCAPFinalize(plFCAHandle, lFileSize);

						if (lActSize == lFileSize)
						{
							lWriteFD = GLOBAL_FOPEN(pFilePathName, "wb");
							if (lWriteFD)
							{
								lActSize = GLOBAL_FWRITE(plFCAHandle->m_pDataBuf, 1, plFCAHandle->m_DataSize, lWriteFD);
								GLOBAL_FCLOSE(lWriteFD);
								if (lActSize == plFCAHandle->m_DataSize)
								{
									GLOBAL_TRACE(("Save Paramters Successful, BIN Size = %d,\n", lActSize));
								}
								else
								{
									GLOBAL_TRACE(("Save Paramters Failed\n"));
								}
							}
							else
							{
								GLOBAL_TRACE(("Open %s for Write Failed!!!\n", plCMD));
							}
						}
						else
						{
							GLOBAL_TRACE(("Read File File Error Info: %d/%d\n", lActSize, lFileSize));
						}

					}
					else
					{
						GLOBAL_TRACE(("Get PTR Failed\n"));
					}
					CAL_FCAPDestroy(plFCAHandle);
					plFCAHandle = NULL;
				}
				else
				{
					GLOBAL_TRACE(("Create Handle Failed\n"));
				}
			
			}
			else
			{
				GLOBAL_TRACE(("%s File Size = \n", lFileSize));
			}


			GLOBAL_FCLOSE(lReadFD);
		}
		else
		{
			GLOBAL_TRACE(("Open %s Failed!!!\n", MULT_IO_TMP_FILE_PATHNAME));
		}

		/*删除临时文件*/
		GLOBAL_SPRINTF((plCMD, "rm -r %s", MULT_IO_TMP_FILE_PATHNAME));
 		PFC_System(plCMD);
	}
}

BOOL MULTL_ValidationParameter(MULT_Handle *pHandle, CHAR_T* pFilePathName, CHAR_T *pDescBuf, S32 BufSize)
{
	BOOL lRet = FALSE;
	GLOBAL_FD lFD;
	CHAR_T *plDescription;
	S32 lFileSize;
	CAL_FCAPHandle *plFCAHandle;

	if (pFilePathName)
	{
		lFD = GLOBAL_FOPEN(pFilePathName, "rb");
		if (lFD)
		{
			lFileSize = CAL_FileSize(lFD);
			plFCAHandle = CAL_FCAPDencodeCreate(lFileSize);
			if (plFCAHandle)
			{
				GLOBAL_FREAD(plFCAHandle->m_pDataBuf, 1, lFileSize, lFD);
				GLOBAL_FCLOSE(lFD);
				if (CAL_FCAPValidation(plFCAHandle, MULT_DEVICE_COMPLETE_TYPE, MULT_PARAMETER_DESCRIPTOR) == TRUE)
				{
					plDescription = CAL_FCAPGetDescription(plFCAHandle);
					//GLOBAL_TRACE(("Description = [%s] \n", plDescription));
					CAL_StringValidStrcpy(pDescBuf, plDescription, BufSize);
					lRet = TRUE;
				}
				CAL_FCAPDestroy(plFCAHandle);
				plFCAHandle = NULL;
			}
		}
	}
	return lRet;
}



//*EOF*/
