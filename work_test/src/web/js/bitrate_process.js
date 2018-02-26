/*UTF-8编码*/

function JS_BitrateGetDirectionTotal(XMLDom, Direction) 
{
    var lTmpVar;
    lTmpVar = XMLDom.getElementsByTagName(Direction);
    return JS_XMLGetTagValue(lTmpVar[0], "total");
}

function JS_BitrateGetChannelNum(XMLDom, Direction) 
{
    var lTmpVar;
    lTmpVar = XMLDom.getElementsByTagName(Direction);
    lTmpVar = lTmpVar[0].getElementsByTagName("channel");

    return lTmpVar.length;
}


function JS_BitrateGetChannelTotal(XMLDom, Direction, ChannelIndex) 
{
    var lTmpVar;
    lTmpVar = XMLDom.getElementsByTagName(Direction);
    lTmpVar = lTmpVar[0].getElementsByTagName("channel");
    
    if (ChannelIndex < lTmpVar.length) 
    {
        return JS_XMLGetTagValue(lTmpVar[ChannelIndex], "channel_total");
    }
    return 0;
}



function JS_BitrateGetSubChannelNum(XMLDom, Direction, ChannelIndex) 
{
    var lTmpVar;
    lTmpVar = XMLDom.getElementsByTagName(Direction);
    lTmpVar = lTmpVar[0].getElementsByTagName("channel");
    
    if (ChannelIndex < lTmpVar.length) 
    {
        lTmpVar = lTmpVar[ChannelIndex].getElementsByTagName("sub_channel");
        
        return lTmpVar.length;
    }
    return 0;
}


function JS_BitrateGetSubChannel(XMLDom, Direction, ChannelIndex, SubChannelIndex) 
{
    var lTmpVar;
    lTmpVar = XMLDom.getElementsByTagName(Direction);
    lTmpVar = lTmpVar[0].getElementsByTagName("channel");
    
    if (ChannelIndex < lTmpVar.length) 
    {
        lTmpVar = lTmpVar[ChannelIndex].getElementsByTagName("sub_channel");
        if (SubChannelIndex < lTmpVar.length) 
        {
            return JS_XMLGetValue(lTmpVar[SubChannelIndex]);
        }
    }
    return 0;
}



function JS_BitrateGetPIDStatisticArray(XMLDom) 
{
	return XMLDom.getElementsByTagName("pid");
}

 
function JS_BitrateGetPID(Obj) 
{
	return parseInt(JS_XMLGetTagValue(Obj, "value"), 10);
}

function JS_BitrateGetBps(Obj) 
{
	return parseInt(JS_XMLGetTagValue(Obj, "bps"), 10);
}













function JS_BitrateGetSubChannelArray(XMLDom, Direction, ChannelIndex)
{
	var lTmpVar;
	lTmpVar = XMLDom.getElementsByTagName(Direction);
	lTmpVar = lTmpVar[0].getElementsByTagName("chn");

	if (ChannelIndex < lTmpVar.length)
	{
		return lTmpVar[ChannelIndex].getElementsByTagName("sub");
	}
	return null;
}

function JS_BitrateGetSubBitrate(Obj) 
{
	return parseInt(JS_XMLGetTagValue(Obj, "bit"), 10);
}

function JS_BitrateGetSubIndex(Obj) 
{
	return parseInt(JS_XMLGetTagValue(Obj, "ind"), 10);
}

function JS_BitrateGetTotalBitrate(XMLDom, Direction, ChannelIndex)
{
	var lTmpVar;
	lTmpVar = XMLDom.getElementsByTagName(Direction);
	lTmpVar = lTmpVar[0].getElementsByTagName("chn");
	if (ChannelIndex < lTmpVar.length)
	{
		return parseInt(lTmpVar[ChannelIndex].getElementsByTagName("total")[0].childNodes[0].nodeValue, 10);
	}
	
	return 0;
}

function JS_IPStatGetArray(XMLDom)
{
	return XMLDom.getElementsByTagName("ip");
}

function JS_IPStatGetPort(Obj)
{
	return JS_XMLGetTagValue(Obj, "port"); ;
}

function JS_IPStatGetIPAddr(Obj)
{
	return JS_XMLGetTagValue(Obj, "ipv4");
}

function JS_IPGetBps(Obj)
{
	return parseInt(JS_XMLGetTagValue(Obj, "bps"), 10);
}

function JS_IPGetProtocol(Obj)
{
	return JS_XMLGetTagValue(Obj, "protocol");
}





