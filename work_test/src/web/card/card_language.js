var s_CardLanguageName = "chn";
var s_CardLanguageXMLDom = null;
var s_Pagename;

function JS_CLANInitiate()
{
    var lDeviceXMLDom;
    var lTmpVar;
    lDeviceXMLDom = JS_XMLCreateDomFromFile("/tmp/device_parameter.xml");

    lTmpVar = JS_XMLGetTagValueINT(lDeviceXMLDom, "language", 10);
    if (lTmpVar == 0)//在这里判断语言类型
    {
        s_CardLanguageName = "eng";
    }
    else
    {
        s_CardLanguageName = "chn";
    }

    s_CardLanguageXMLDom = JS_XMLCreateDomFromFile("/card/card_language.xml");
    
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

function JS_CLAN(String)
{
    var lTmpStr;
    var lPageNode, lStringNode, lTextNode;
    lTmpStr = String;
    if (s_CardLanguageXMLDom != null)
    {
        lPageNode = s_CardLanguageXMLDom.getElementsByTagName(s_Pagename);
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
                        lTmpStr = lTextNode[0].childNodes[0].nodeValue;
                    }
                }
            }
        }
    }
    return lTmpStr;
}

function JS_CINLAN(Str)
{
    document.write(JS_CLAN(Str));
}

function JS_CLANCMN(String)
{
    var lTmpStr;
    var lPageNode, lStringNode, lTextNode;
    lTmpStr = String;
    if (s_CardLanguageXMLDom != null)
    {
        lPageNode = s_CardLanguageXMLDom.getElementsByTagName("common_strings");
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
                        lTmpStr = lTextNode[0].childNodes[0].nodeValue;
                    }
                }
            }
        }
    }
    return lTmpStr;
}

function JS_CINLANCMN(Str)
{
    document.write(JS_CLANCMN(Str));
}

function JS_CLANGetModuleNameByTag(ModuleTag)
{
    var lTmpStr;
    var lTmpNode;

    lTmpStr = ModuleTag
    lTmpNode = JS_XMLGetTagNameFirstObj(s_CardLanguageXMLDom, "common_strings");
    if (JS_ISValidObject(lTmpNode))
    {
        lTmpNode = JS_XMLGetTagNameFirstObj(s_CardLanguageXMLDom, "module_name");
        if (JS_ISValidObject(lTmpNode))
        {
            lTmpNode = JS_XMLGetTagNameFirstObj(lTmpNode, ModuleTag);
            if (JS_ISValidObject(lTmpNode))
            {
                lTmpStr = JS_XMLGetTagValue(lTmpNode, s_CardLanguageName);
            }
        }
    }

    return lTmpStr;
}


function JS_CLANGetCHNNameByTag(CHNTag)
{
    var lTmpStr;
    var lTmpNode;

    lTmpStr = ""
    lTmpNode = JS_XMLGetTagNameFirstObj(s_CardLanguageXMLDom, "common_strings");
    if (JS_ISValidObject(lTmpNode))
    {
        lTmpNode = JS_XMLGetTagNameFirstObj(s_CardLanguageXMLDom, "chn_name");
        if (JS_ISValidObject(lTmpNode))
        {
            lTmpNode = JS_XMLGetTagNameFirstObj(lTmpNode, CHNTag);
            if (JS_ISValidObject(lTmpNode))
            {
                lTmpStr = JS_XMLGetTagValue(lTmpNode, s_CardLanguageName);
            }
        }
    }

    return lTmpStr;
}


function JS_CLANGetAlarmByID(ModuleTage, AlarmID, bDetail, UserData)
{
    var lTmpStr;
    var lTmpNode;

    lTmpStr = ""

    lTmpNode = JS_XMLGetTagNameFirstObj(s_CardLanguageXMLDom, "alarms_strings");
    if (JS_ISValidObject(lTmpNode))
    {
        lTmpNode = JS_XMLGetTagNameFirstObj(lTmpNode, ModuleTage);
        if (JS_ISValidObject(lTmpNode))
        {
            if (bDetail)
            {
                lTmpNode = JS_XMLGetTagNameFirstObj(lTmpNode, "log_detail_" + AlarmID);
                if (JS_ISValidObject(lTmpNode))
                {
                    lTmpNode = JS_XMLGetTagNameFirstObj(lTmpNode, "code_" + UserData);
                    if (JS_ISValidObject(lTmpNode))
                    {
                        lTmpStr = JS_XMLGetTagValue(lTmpNode, s_CardLanguageName);
                    }
                    else
                    {
                        lTmpNode = JS_XMLGetTagNameFirstObj(lTmpNode, "code_x");
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
                lTmpNode = JS_XMLGetTagNameFirstObj(lTmpNode, "log_type_" + AlarmID);
                if (JS_ISValidObject(lTmpNode))
                {
                    lTmpStr = JS_XMLGetTagValue(lTmpNode, s_CardLanguageName);
                }
            }
        }
    }

    return lTmpStr;
}

