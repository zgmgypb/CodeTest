/*处理模块的参数（即需要保存读取的）*/
var s_CardParameterXMLDOM;

function JS_CardParamInitiate()
{
    s_CardParameterXMLDOM = JS_XMLCreateDomFromFile("/tmp/card_parameter.xml");
    return s_CardParameterXMLDOM;
}

function JS_CardParamGetSubModuleNode(ModuleSlot) 
{
    var lTmpVar;
    if (JS_ISValidObject(s_CardParameterXMLDOM))
    {
        lTmpVar = JS_XMLGetTagNameFirstObj(s_CardParameterXMLDOM, "module_" + ModuleSlot);
        if (JS_ISValidObject(lTmpVar)) 
        {
        }
        else
        {
            lTmpVar = null;
        }
    }
    return lTmpVar;
}

function JS_CardParamGetSubModuleParamNode(ModuleSlot)
{
    var lTmpVar;
    lTmpVar = JS_CardParamGetSubModuleNode(ModuleSlot);
    if (JS_ISValidObject(lTmpVar))
    {
        lTmpVar = JS_XMLGetTagNameFirstObj(lTmpVar, "module_param");
    }
    else
    {
        lTmpVar = null;
    }
    return lTmpVar;
}

function JS_CardParamGetSubModuleAlarmNode(ModuleSlot)
{
    var lTmpVar;
    lTmpVar = JS_CardParamGetSubModuleNode(ModuleSlot);
    if (JS_ISValidObject(lTmpVar))
    {
        lTmpVar = JS_XMLGetTagNameFirstObj(lTmpVar, "alarm_system");
    }
    else
    {
        lTmpVar = null;
    }
    return lTmpVar;
}

function JS_CardParamGetSubModuleType(ModuleSlot)
{
    var lTmpVar;
    lTmpVar = JS_CardParamGetSubModuleNode(ModuleSlot);
    if (JS_ISValidObject(lTmpVar))
    {
        lTmpVar = JS_XMLGetTagNameFirstObj(lTmpVar, "alarm_system");
    }
    else
    {
        lTmpVar = null;
    }
    return lTmpVar;
}




/*处理模块信息（不需要保存，动态生成的）----------------------------------------------------------------------------------------------------------- */

var s_CardInfoXMLDOM;

function JS_CardInfoInitiate()
{
    s_CardInfoXMLDOM = JS_XMLCreateDomFromFile("/tmp/card_info.xml");
    return s_CardInfoXMLDOM;
}

function JS_CardInfoGetModuleArray()
{
    var lTmpVar;
    if (JS_ISValidObject(s_CardInfoXMLDOM))
    {
        lTmpVar = JS_XMLGetTagNameArray(s_CardInfoXMLDOM, "module");
    }
    else
    {
        lTmpVar = null;
    }
    return lTmpVar;
}

function JS_CardInfoGetModuleState(ModuleObj)
{
    return JS_XMLGetTagValue(ModuleObj, "state");
}

function JS_CardInfoGetModuleInfo(ModuleObj)
{
    var lItemInfo = new Object();
    if (JS_ISValidObject(ModuleObj))
    {
        lItemInfo.m_ModuleType = JS_XMLGetTagValueINT(ModuleObj, "module_type", 10);
        lItemInfo.m_ModuleTag = JS_XMLGetTagValue(ModuleObj, "module_tag");
        lItemInfo.m_SN = JS_XMLGetTagValue(ModuleObj, "sn");
        lItemInfo.m_FPGAArray = JS_XMLGetTagNameArray(ModuleObj, "fpga_r");
        lItemInfo.m_FirmwareV = JS_XMLGetTagValue(ModuleObj, "firmware_v");
        lItemInfo.m_ChnArray = JS_XMLGetTagNameArray(ModuleObj, "chn");
    }
    return lItemInfo;
}


function JS_CardInfoGetModuleChnInfo(ChnObj)
{
    var lItemInfo = new Object();
    if (JS_ISValidObject(ChnObj))
    {
        lItemInfo.m_Slot = JS_XMLGetTagValueINT(ChnObj, "chn_slot", 10);
        lItemInfo.m_ChnTag = JS_XMLGetTagValue(ChnObj, "chn_tag", 10);
        lItemInfo.m_MTOSStart = JS_XMLGetTagValueINT(ChnObj, "chn_m_to_s_ts_start_ind", 10);
        lItemInfo.m_MTOSCount = JS_XMLGetTagValueINT(ChnObj, "chn_m_to_s_ts_count", 10);
        lItemInfo.m_STOMStart = JS_XMLGetTagValueINT(ChnObj, "chn_s_to_m_ts_start_ind", 10);
        lItemInfo.m_STOMCount = JS_XMLGetTagValueINT(ChnObj, "chn_s_to_m_ts_count", 10);
    }
    return lItemInfo;
}
