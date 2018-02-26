/*UTF-8编码*/
var s_BatchParameterXMLDOM = null;

function JS_BatchParamInitate() 
{
    s_BatchParameterXMLDOM = JS_XMLCreateDomFromFile("tmp/service_parameter.xml");
}

function JS_BatchParamGetTsArray(bInput) 
{
    var lTmpVar;
    if (s_BatchParameterXMLDOM != null) 
    {
        lTmpVar = s_BatchParameterXMLDOM.getElementsByTagName("transport_streams");
        if (lTmpVar.length) 
        {
            if (bInput == true) 
            {
                lTmpVar = lTmpVar[0].getElementsByTagName("input_ts");
                if (lTmpVar.length > 0) 
                {
                    return lTmpVar[0].getElementsByTagName("ts");
                }
            }
            else 
            {
                lTmpVar = lTmpVar[0].getElementsByTagName("output_ts");
                if (lTmpVar.length > 0) 
                {
                    return lTmpVar[0].getElementsByTagName("ts");
                }
            }
        }
    }
    return null;
}

function JS_BatchParamGetTsObject(TsIndex, bInput) 
{
    var lTmpVar;
    if (s_BatchParameterXMLDOM != null) 
    {
        lTmpVar = s_BatchParameterXMLDOM.getElementsByTagName("transport_streams");
        if (lTmpVar.length) 
        {
            if (bInput == true) 
            {
                lTmpVar = lTmpVar[0].getElementsByTagName("input_ts");
                if (lTmpVar.length > 0) 
                {
                    lTmpVar = lTmpVar[0].getElementsByTagName("ts");
                    if (lTmpVar.length > TsIndex) 
                    {
                        return lTmpVar[TsIndex];
                    }
                }
            }
            else 
            {
                lTmpVar = lTmpVar[0].getElementsByTagName("output_ts");
                if (lTmpVar.length > 0) 
                {
                    lTmpVar = lTmpVar[0].getElementsByTagName("ts");
                    if (lTmpVar.length > TsIndex) 
                    {
                        return lTmpVar[TsIndex];
                    }
                }
            }
        }
    }
    return null;
}



function JS_BatchParamGetTsInfo(TsObj, bInput) 
{
    var lItemInfo = new Object();
    if (TsObj != null) 
    {

        lItemInfo.m_TsMark = JS_XMLGetTagValue(TsObj, "ts_mark");

    }
    return lItemInfo;
}

function JS_BatchParamGetTsRouteInfo(TsObj) 
{
    var lItemInfo = new Object();
    if (TsObj != null) 
    {
        lItemInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(TsObj, "ts_index"), 10);
        lItemInfo.m_RouteMark = JS_XMLGetTagValue(TsObj, "route_mark");

    }
    return lItemInfo;
}

function JS_BatchParamGetServArray() 
{
    var lTmpArray, lTmpVar;
    var i;

    if (s_BatchParameterXMLDOM != null) 
    {
        lTmpVar = s_BatchParameterXMLDOM.getElementsByTagName("servids");
        if (lTmpVar.length) 
        {
            return lTmpVar = lTmpVar[0].getElementsByTagName("service");
        }
    }
    return null;
}


function JS_BatchParamGetServArrayByTsIndex(TsIndex, bInput) 
{
    var lTmpArray, lTmpVar;
    var i;

    lTmpArray = new Array();
    lTmpVar = JS_BatchParamGetServArray();
    if (lTmpVar) 
    {
        for (i = 0; i < lTmpVar.length; i++) 
        {
            if (bInput) 
            {
                lTmpValue = JS_XMLGetTagValue(lTmpVar[i], "in_ts_index");
                if (lTmpValue == TsIndex) 
                {
                    lTmpArray[lTmpArray.length] = lTmpVar[i];
                }
            }
            else 
            {
                lTmpValue = JS_XMLGetTagValue(lTmpVar[i], "out_ts_index");
                if (lTmpValue == TsIndex) 
                {
                    lTmpArray[lTmpArray.length] = lTmpVar[i];
                }
            }

        }
    }
    return lTmpArray;
}


function JS_BatchParamGetServiceInfo(ServObj, bInput) 
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (ServObj != null) 
    {

        lItemInfo.m_IDs = JS_XMLGetTagValue(ServObj, "service_ids");
        lItemInfo.m_InTsIndex = parseInt(JS_XMLGetTagValue(ServObj, "in_ts_index"), 10);
        lItemInfo.m_OutTsIndex = parseInt(JS_XMLGetTagValue(ServObj, "out_ts_index"), 10);

        if (bInput) 
        {
            lTmpVar = ServObj.getElementsByTagName("in_service_info");
            if (lTmpVar.length) 
            {
                lTmpVar = lTmpVar[0];

                lItemInfo.m_ServID = parseInt(JS_XMLGetTagValue(lTmpVar, "service_id"), 10);
                lItemInfo.m_ServName = JS_XMLGetTagValue(lTmpVar, "service_name");



            }
        }
        else 
        {
            lTmpVar = ServObj.getElementsByTagName("out_service_info");
            if (lTmpVar.length) 
            {
                lTmpVar = lTmpVar[0];

                lItemInfo.m_ServID = parseInt(JS_XMLGetTagValue(lTmpVar, "service_id"), 10);
                lItemInfo.m_ServName = JS_XMLGetTagValue(lTmpVar, "service_name");

            }
        }
    }
    return lItemInfo;
}
