/*UTF-8编码*/
var s_MonitorXMLDom = null;

function JS_MONInitiate()
{
	s_MonitorXMLDom = JS_XMLCreateDomFromFile("/tmp/monitor_setting.xml");
}



function JS_MONGetSystemInfo()
{
	var lItemInfo = new Object();
	if (s_MonitorXMLDom != null)
	{
		lItemInfo.m_Active = JS_XMLGetTagValue(s_MonitorXMLDom, "global_mark");
		lItemInfo.m_CritcalTemp = parseInt(JS_XMLGetTagValue(s_MonitorXMLDom, "critical_temp"), 10);
		lItemInfo.m_FanTemp = parseInt(JS_XMLGetTagValue(s_MonitorXMLDom, "fan_temp"), 10);
		lItemInfo.m_NoTemp = JS_XMLGetTagValue(s_MonitorXMLDom, "no_temp");
		lItemInfo.m_ETHNum = parseInt(JS_XMLGetTagValue(s_MonitorXMLDom, "eth_num"), 10);
}
	return lItemInfo;
}



function JS_MONGetLogSystemArray()
{
	var lTmpVar;
	if (s_MonitorXMLDom != null)
	{
		lTmpVar = s_MonitorXMLDom.getElementsByTagName("log_system");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("log");
		}
	}
	return null;
}


function JS_MONGetLogInfo(Obj)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (Obj != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(Obj, "log_id");
		lItemInfo.m_Trap = JS_XMLGetTagValue(Obj, "trap");
		lItemInfo.m_Panel = JS_XMLGetTagValue(Obj, "panel");
		lItemInfo.m_Level = parseInt(JS_XMLGetTagValue(Obj, "level"), 10);
	}
	return lItemInfo;
}


function JS_MONGetChnArray(bInput)
{
	if (s_MonitorXMLDom != null)
	{
		lTmpVar = s_MonitorXMLDom.getElementsByTagName("chn_limils");
		if (lTmpVar.length)
		{
			if (bInput)
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("in_chn");
			}
			else
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("out_chn");
			}

			if (lTmpVar.length) 
			{
				return lTmpVar[0].getElementsByTagName("chn");
			}
		}
	}
	return null
}

function JS_MONGetSubArray(Obj)
{
	return Obj.getElementsByTagName("sub");
}


function JS_MONGetLimitInfo(Obj)
{
	var lTmpVar;
	var lItemInfo = new Object();
	lTmpVar = Obj.getElementsByTagName("limit");
	if (lTmpVar.length) 
	{
		lTmpVar = lTmpVar[0];
		lItemInfo.m_Low = parseInt(JS_XMLGetTagValue(lTmpVar, "low"), 10);
		lItemInfo.m_High = parseInt(JS_XMLGetTagValue(lTmpVar, "high"), 10);
		lItemInfo.m_Mark = JS_XMLGetTagValue(lTmpVar, "mark");
	}
	return lItemInfo;
}













