/*UTF-8编码*/
var s_DeviceXMLDom = null;

function JS_DevSettingInitiate() 
{
	s_DeviceXMLDom = JS_XMLCreateDomFromFile("/tmp/device_parameter.xml");
}

function JS_DevGetManagePortSetting()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_DeviceXMLDom != null)
	{
		lTmpVar = s_DeviceXMLDom.getElementsByTagName("manage_port");
		if (lTmpVar.length > 0)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_IPv4Addr = JS_XMLGetTagValue(lTmpVar, "ipv4_addr");
			lItemInfo.m_IPv4Mask = JS_XMLGetTagValue(lTmpVar, "ipv4_mask");
			lItemInfo.m_IPv4Gate = JS_XMLGetTagValue(lTmpVar, "ipv4_gate");
			lItemInfo.m_IPMac = JS_XMLGetTagValue(lTmpVar, "ip_mac");
			
			lItemInfo.m_DataIPv4Addr = JS_XMLGetTagValue(lTmpVar, "data_ipv4_addr");
			lItemInfo.m_DataIPv4Mask = JS_XMLGetTagValue(lTmpVar, "data_ipv4_mask");
			lItemInfo.m_DataIPv4Gate = JS_XMLGetTagValue(lTmpVar, "data_ipv4_gate");
			lItemInfo.m_DataIPMac = JS_XMLGetTagValue(lTmpVar, "data_ip_mac");
		}
	}
	return lItemInfo;
}


function JS_DevGetIpOutputType()
{
	var lTmpVar;

	if (s_DeviceXMLDom != null)
	{
		lTmpVar = s_DeviceXMLDom.getElementsByTagName("ip_output_setting");
		if (lTmpVar.length > 0)
		{
			lTmpVar = lTmpVar[0];
			return JS_XMLGetTagValue(lTmpVar, "ip_output_type");
		}
	}
	return "MPTS";
}

function JS_DevGetOutputCharset()
{
	var lTmpVar;

	if (s_DeviceXMLDom != null)
	{
		lTmpVar = s_DeviceXMLDom.getElementsByTagName("ip_output_setting");
		if (lTmpVar.length > 0)
		{
			lTmpVar = lTmpVar[0];
			return JS_XMLGetTagValue(lTmpVar, "output_charset");
		}
	}
	return "GB-2312";
}

function JS_DevGetNTPPortSetting()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_DeviceXMLDom != null)
	{
		lTmpVar = s_DeviceXMLDom.getElementsByTagName("ntp_server");
		if (lTmpVar.length > 0)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_IPv4Addr = JS_XMLGetTagValue(lTmpVar, "ipv4_addr");
			lItemInfo.m_Interval = parseInt(JS_XMLGetTagValue(lTmpVar, "sync_interval"), 10);
			lItemInfo.m_Active = JS_XMLGetTagValue(lTmpVar, "active_mark");
		}
	}
	return lItemInfo;
}

function JS_DevGetIGMPSetting()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_DeviceXMLDom != null)
	{
		lTmpVar = s_DeviceXMLDom.getElementsByTagName("igmp");
		if (lTmpVar.length > 0)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_Version = JS_XMLGetTagValue(lTmpVar, "igmp_version");
			lItemInfo.m_Interval = JS_XMLGetTagValue(lTmpVar, "igmp_interval");
		}
	}
	return lItemInfo;
}




function JS_DevGetBackupParameterInfo()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_DeviceXMLDom != null)
	{
		lItemInfo.m_bHaveBackup = parseInt(JS_XMLGetTagValue(s_DeviceXMLDom, "backup_parameter"), 10);
		lItemInfo.m_BackupInfo = JS_XMLGetTagValue(s_DeviceXMLDom, "backup_description");
	}
	return lItemInfo;
}


function JS_DevGetSNMPInfo()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_DeviceXMLDom != null)
	{
		lTmpVar = s_DeviceXMLDom.getElementsByTagName("snmp_manage");
		if (lTmpVar.length > 0)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_SNMPEnable = true;
			lItemInfo.m_SNMPActive = JS_XMLGetTagValue(lTmpVar, "snmp_active_mark");
			lItemInfo.m_TrapActive = JS_XMLGetTagValue(lTmpVar, "trap_active_mark");
			lItemInfo.m_IPv4Addr = JS_XMLGetTagValue(lTmpVar, "trap_ipv4_addr");
			lItemInfo.m_IPPort = parseInt(JS_XMLGetTagValue(lTmpVar, "trap_ip_port"), 10);
			lItemInfo.m_TrapInterval = parseInt(JS_XMLGetTagValue(lTmpVar, "trap_interval"), 10);

			lItemInfo.m_ReadCommunity = JS_XMLGetTagValue(lTmpVar, "snmp_agent_readcommuniy");
			lItemInfo.m_WRCommunity = JS_XMLGetTagValue(lTmpVar, "snmp_agent_writecommuniy");
			lItemInfo.m_AgentPort = parseInt(JS_XMLGetTagValue(lTmpVar, "agent_port"), 10);
			lItemInfo.m_DeviceName = JS_XMLGetTagValue(lTmpVar, "snmp_device_name");
			lItemInfo.m_DeviceLocation = JS_XMLGetTagValue(lTmpVar, "snmp_device_location");
		}
		else 
		{
			lItemInfo.m_SNMPEnable = false;
		}
	
	}
	return lItemInfo;
}

function JS_DevGetSyslogInfo() 
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_DeviceXMLDom != null) 
	{
		lTmpVar = s_DeviceXMLDom.getElementsByTagName("syslog_manage");
		if (lTmpVar.length > 0) 
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_SyslogEnable = true;
			lItemInfo.m_SyslogActive = JS_XMLGetTagValue(lTmpVar, "syslog_active_mark");
			lItemInfo.m_RemoteActive = JS_XMLGetTagValue(lTmpVar, "remote_active_mark");
			lItemInfo.m_IPv4Addr = JS_XMLGetTagValue(lTmpVar, "remote_ipv4_addr");
			lItemInfo.m_IPPort = parseInt(JS_XMLGetTagValue(lTmpVar, "remote_ip_port"), 10);
			lItemInfo.m_LogLevel = parseInt(JS_XMLGetTagValue(lTmpVar, "log_level"), 10);
		}
		else 
		{
			lItemInfo.m_SyslogEnable = false;
		}
	}
	return lItemInfo;
}



function JS_ParamGetSyslogLevelArray() 
{
	var lTmpVar;
	if (s_DeviceXMLDom != null) 
	{
		lTmpVar = s_DeviceXMLDom.getElementsByTagName("syslog_manage");
		if (lTmpVar.length > 0) 
		{
			lTmpVar = lTmpVar[0];
			return lTmpVar.getElementsByTagName("level");
		}
	}
	return null;
}


function JS_ParamGetSyslogLevel(Obj) 
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_Value = parseInt(JS_XMLGetTagValue(Obj, "value"));
	lSubInfo.m_Txt = JS_XMLGetTagValue(Obj, "txt");
	return lSubInfo;
}




