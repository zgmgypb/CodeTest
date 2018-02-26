var s_LanguageName = "chn";
var s_LanguageXMLDom = null;
var s_Pagename;

function JS_LANInitiate()
{
	var lDeviceXMLDom;
	var lTmpVar;
	lDeviceXMLDom = JS_XMLCreateDomFromFile("/tmp/device_parameter.xml");

	lTmpVar = parseInt(JS_XMLGetTagValue(lDeviceXMLDom, "language"), 10);
	if (lTmpVar == 0)//完全在这里判断语言类型
	{
		s_LanguageName = "eng";
	}
	else
	{
		s_LanguageName = "chn";
	}

	s_LanguageName = s_LanguageName.toLowerCase();

	if (s_LanguageName == "auto")
	{
		s_LanguageName = JS_LANGetBrowerDefaultLanguage().toLocaleLowerCase();
	}

	s_LanguageXMLDom = JS_XMLCreateDomFromFile("/language.xml");

	lTmpVar = window.location.href;
	if (lTmpVar.indexOf("?") != -1)
	{
		s_Pagename = lTmpVar.substring(lTmpVar.lastIndexOf("/", lTmpVar.indexOf("?")) + 1, lTmpVar.indexOf("?"));
	}
	else
	{
		s_Pagename = lTmpVar.substring(lTmpVar.lastIndexOf("/") + 1);
	}
}

function JS_LANGetBrowerDefaultLanguage()
{
	var type = navigator.appName
	if (type == "Netscape")
	{
		var lang = navigator.language;
	}
	else
	{
		var lang = navigator.browserLanguage;
	}
	//取得国家代码的前两个字母
	lang = lang.substr(0, 2);

	if (lang == "zh")
	{
		return "CHN";
	}
	else
	{
		return "ENG";
	}
}

function JS_LANGetLanguageIndex() 
{
    if ((s_LanguageName == "eng")) 
    {
        return 0;
    }
    else if ((s_LanguageName == "chn")) 
    {
        return 1;
    }
    return 0;
}


function JS_LANGetLanguageString() 
{
    return s_LanguageName;
}


function JS_LAN(String)
{
	var lTmpStr;
    var lPageNode, lStringNode, lTextNode;
    lTmpStr = String;
    if (s_LanguageXMLDom != null) 
    {
         lPageNode = s_LanguageXMLDom.getElementsByTagName(s_Pagename);
         if (lPageNode.length) 
         {
             String = String.toLowerCase();
             lStringNode = lPageNode[0].getElementsByTagName(String);
             if (lStringNode.length) 
             {
                 lTextNode = lStringNode[0].getElementsByTagName(s_LanguageName);
                 if (lTextNode.length) 
                 {
                    if (lTextNode[0].childNodes.length) 
                    {
                        lTmpStr = lTextNode[0].childNodes[0].nodeValue ;
                    }
                 }
             }
         }
    }
    return lTmpStr;
}

function JS_INLAN(Str)
{
	document.write(JS_LAN(Str));
}       
 
function JS_INCMNLAN(Str)
{
	document.write(JS_LANCMN(Str));
}       

function JS_LANCMN(String)
{
    var lTmpStr;
    var lPageNode, lStringNode, lTextNode;
    lTmpStr = String;
    if (s_LanguageXMLDom != null) 
    {
         lPageNode = s_LanguageXMLDom.getElementsByTagName("common_strings");
         if (lPageNode.length) 
         {
             String = String.toLowerCase();
             lStringNode = lPageNode[0].getElementsByTagName(String);
             if (lStringNode.length) 
             {
                 lTextNode = lStringNode[0].getElementsByTagName(s_LanguageName);
                 if (lTextNode.length) 
                 {
                    if (lTextNode[0].childNodes.length) 
                    {
                        lTmpStr = lTextNode[0].childNodes[0].nodeValue ;
                    }
                 }
             }
         }
    }
    return lTmpStr;
}

function JS_LANGetMpeg2ServiceTypeString(Index)
{
    var lTmpStr, lTmpValue;
    var i;
    lTmpStr = "13818-" + JS_StrLeftPading(Index, 3); 
    var lPageNode, lTypeNode, lTextNode;
    if (s_LanguageXMLDom != null) 
    {
         lPageNode = s_LanguageXMLDom.getElementsByTagName("mpeg2_strings");
         if (lPageNode.length) 
         {
			lTypeNode = lPageNode[0].getElementsByTagName("service_type");
			for(i = 0; i < lTypeNode.length; i++)
			{
				if (JS_XMLGetTagValue(lTypeNode[i], "index") == Index) 
				{
					lTextNode = lTypeNode[i].getElementsByTagName(s_LanguageName);
					if (lTextNode.length) 
					{
						lTmpStr = JS_XMLGetValue(lTextNode[0]);
					}
					break;
				}
			}
         }
    }
    return lTmpStr;
}     


function JS_LANGetMpeg2EsTypeString(Index)
{
    var lTmpStr, lTmpValue;
    var i;
    lTmpStr = "13818-" + JS_StrLeftPading(Index, 3); 
    var lPageNode, lTypeNode, lTextNode;
    if (s_LanguageXMLDom != null) 
    {
         lPageNode = s_LanguageXMLDom.getElementsByTagName("mpeg2_strings");
         if (lPageNode.length) 
         {
			lTypeNode = lPageNode[0].getElementsByTagName("es_type");
			for(i = 0; i < lTypeNode.length; i++)
			{
				if (JS_XMLGetTagValue(lTypeNode[i], "index") == Index) 
				{
					lTextNode = lTypeNode[i].getElementsByTagName(s_LanguageName);
					if (lTextNode.length) 
					{
						lTmpStr = JS_XMLGetValue(lTextNode[0]);
					}
					break;
				}
			}
         }
    }
    return lTmpStr;
}

function JS_LANGetAlarmByID(AlarmID, bDetail, UserData)
{
    var lTmpStr;
    var lAlarmRoot, lTypeRoot, lTmpNode;

    lTmpStr = ""
    
    lAlarmRoot = JS_XMLGetTagNameFirstObj(s_LanguageXMLDom, "alarms_strings");
    if (JS_ISValidObject(lAlarmRoot))
    {
        if (bDetail)
        {
            lTypeRoot = JS_XMLGetTagNameFirstObj(lAlarmRoot, "log_detail_" + parseInt(AlarmID, 16));
            if (JS_ISValidObject(lTypeRoot))
            {
                lTmpNode = JS_XMLGetTagNameFirstObj(lTypeRoot, "code_" + UserData);
                if (JS_ISValidObject(lTmpNode))
                {
                    lTmpStr = JS_XMLGetTagValue(lTmpNode, s_LanguageName);
                }
                else
                {
                    lTmpNode = JS_XMLGetTagNameFirstObj(lTypeRoot, "code_x");
                    if (JS_ISValidObject(lTmpNode))
                    {
                        lTmpStr = JS_Printf(JS_XMLGetTagValue(lTmpNode, s_LanguageName), UserData)
                    }
                    else
                    {
                        lTmpStr = "code=" + UserData;
                    }
                }
            }
            else
            {
                lTmpStr = "log_detail_" + AlarmID;
            }
        }
        else
        {
            lTypeRoot = JS_XMLGetTagNameFirstObj(lAlarmRoot, "log_type_" + AlarmID);
            if (JS_ISValidObject(lTypeRoot))
            {
                lTmpStr = JS_XMLGetTagValue(lTypeRoot, s_LanguageName);
            }
        }
    }

    return lTmpStr;
}







   
