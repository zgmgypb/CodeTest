
/************************************************************************/
/* AJAX(XML HTTP)相关                                                             */
/************************************************************************/
function JSL_CreateXMLHttpRequest()//创建XMLHttpRequest对象       
{
	var XMLHttpReq;
	if (window.XMLHttpRequest) //IE7 和其他浏览器
	{
		XMLHttpReq = new XMLHttpRequest();
	}
	else if (window.ActiveXObject) // ie5和ie6支持
	{
		try
		{
			XMLHttpReq = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e)
		{
			try
			{
				XMLHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
			}
			catch (e)
			{
				alert(e.name + " : " + e.message);
			}
		}
	}
	return XMLHttpReq;
}

function JS_HttpGet(Target, Parameter, BackFunction, bSync)//发送Get请求函数
{
	var lRandom = Math.round(Math.random() * 10000);
	var lXMLHttpReq = JSL_CreateXMLHttpRequest();

	lXMLHttpReq.open("GET", Target + "?random=" + lRandom + "&" + Parameter, !bSync);
	lXMLHttpReq.onreadystatechange = function()
	{
		if (lXMLHttpReq.readyState == 4)  // 判断对象状态
		{
			if (lXMLHttpReq.status == 200) // 信息已经成功返回，开始处理信息
			{
				if (BackFunction != null)
				{
					if (lXMLHttpReq.responseXML.xml.length != 0) 
					{
						BackFunction(lXMLHttpReq.responseXML);
					}
					else 
					{
						BackFunction(lXMLHttpReq.responseText);
					}
				}
			}
			else  //页面不正常
			{
				if (BackFunction != null)
				{
					BackFunction(null);
				}
			}
		}
	}

	lXMLHttpReq.send(null);  // 发送请求
}

function JS_HttpPost(Target, Parameter, bSync)//发送Post请求函数
{
	var lXMLHttpReq = JSL_CreateXMLHttpRequest();
	try
	{
		lXMLHttpReq.open("POST", Target, !bSync);
		lXMLHttpReq.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
		lXMLHttpReq.send(Parameter);
	}
	catch (e)
	{
		alert(e.name + " : " + e.message);
	}
}


/************************************************************************/
/* XML DOM处理相关                                                      */
/************************************************************************/
function JS_XMLCreateDomFromFile(XMLPathName)
{
	var lXMLDom = null;

	try //Internet Explorer 5.5+
	{
		lXMLDom = new ActiveXObject("Microsoft.XMLDOM");
	}
	catch (e)
	{
		try //Firefox, Mozilla, Opera, etc.
		{
			lXMLDom = document.implementation.createDocument("", "", null);
		}
		catch (e)
		{
			alert(e.name + " : " + e.message);
			return;
		}
	}

	lXMLDom.async = false;

	if (XMLPathName == null)
	{
		var lStatement;
		lStatement = lXMLDom.createProcessingInstruction("xml", 'version="1.0" encoding="utf-8"');
		lXMLDom.appendChild(lStatement);
	}
	else
	{
		lXMLDom.load(XMLPathName);
	}
	return lXMLDom;
}

function JS_XMLAddNormalChild(XMLDom, ParentNode, TagName)
{
	var lTmpNode;
	lTmpNode = XMLDom.createElement(TagName);
	ParentNode.appendChild(lTmpNode);
	return lTmpNode;
}

function JS_XMLAddTextChild(XMLDom, ParentNode, TagName, NodeValue)
{
	var lTmpNode, lHolderNode;
	lHolderNode = JS_XMLAddNormalChild(XMLDom, ParentNode, TagName);
	lTmpNode = XMLDom.createTextNode(NodeValue);
	lHolderNode.appendChild(lTmpNode);
}

function JS_XMLParamSetHead(lXMLDom, Type, Subtype, Param)
{
	var lRootNode, lHolderNode;
	lRootNode = JS_XMLAddNormalChild(lXMLDom, lXMLDom, "root");
	lHolderNode = JS_XMLAddNormalChild(lXMLDom, lRootNode, "post_head");
	JS_XMLAddTextChild(lXMLDom, lHolderNode, "type", Type);
	JS_XMLAddTextChild(lXMLDom, lHolderNode, "subtype", Subtype);
	JS_XMLAddTextChild(lXMLDom, lHolderNode, "parameter", Param);
	lHolderNode = JS_XMLAddNormalChild(lXMLDom, lRootNode, "post_body");
	return lHolderNode;
}


function JS_XMLParamSetModuleHead(lXMLDom, Subtype, Param)
{
    return JS_XMLParamSetHead(lXMLDom, "modules", Subtype, Param);
}

function JS_XMLParamSetSubModuleHead(lXMLDom, ModuleSlot, SubModuleInfo)
{
    var lTmpXMLDom
    lTmpXMLDom = JS_XMLParamSetModuleHead(lXMLDom, "submodules", ModuleSlot);
    JS_XMLAddTextChild(lXMLDom, lTmpXMLDom, "sub_module_info", SubModuleInfo);
    return lTmpXMLDom;
}

function JS_XMLGetTagNameArray(Parent, TagName)
{
    return Parent.getElementsByTagName(TagName);
}

function JS_XMLGetTagNameFirstObj(Parent, TagName)
{
    var lTmpVar;

    lTmpVar = JS_XMLGetTagNameArray(Parent, TagName);
    if (lTmpVar.length > 0)
    {
        return lTmpVar[0];
    }
    return null;
}

function JS_XMLGetValue(Parent)
{
    if (Parent != null)
    {
        if (Parent.childNodes.length)
        {
            return Parent.childNodes[0].nodeValue;
        }
        else
        {
            return "";
        }
    }
    return null;
}

function JS_XMLGetValueINT(Parent, Radix)
{
    var lTmpVar;
    lTmpVar = JS_XMLGetValue(Parent);
    if (lTmpVar != null)
    {
        return parseInt(lTmpVar, Radix);
    }
    return null;
}

function JS_XMLGetValueFloat(Parent)
{
    var lTmpVar;
    lTmpVar = JS_XMLGetValue(Parent);
    if (lTmpVar != null)
    {
        return parseFloat(lTmpVar);
    }
    return null;
}

function JS_XMLGetTagValue(Parent, TagName)
{
	if (Parent != null)
	{
	    return JS_XMLGetValue(JS_XMLGetTagNameFirstObj(Parent, TagName));
	}
	return null;
}

function JS_XMLGetTagValueINT(Parent, TagName, Radix)
{
    return JS_XMLGetValueINT(JS_XMLGetTagNameFirstObj(Parent, TagName), Radix);
}

function JS_XMLGetTagValueFloat(Parent, TagName)
{
    return JS_XMLGetValueFloat(JS_XMLGetTagNameFirstObj(Parent, TagName));
}




/************************************************************************/
/* HTML 工具                                                            */
/************************************************************************/


function $()
{
	var elements = new Array();
	for (var i = 0; i < arguments.length; i++)
	{
		var element = arguments[i];
		if (typeof element == 'string')
			element = document.getElementById(element);
		if (arguments.length == 1)
			return element;
		elements.push(element);
	}
	return elements;
}


/************************************************************************/
/* HTML DOM处理相关                                                     */
/************************************************************************/
/************************************************************************/
/* UI处理相关															*/
/************************************************************************/

function JS_HTMLGetValueByID(ID)
{
	var lTmpNode;
	lTmpNode = $(ID);
	return JS_HTMLGetValue(lTmpNode);
}


function JS_HTMLGetValue(Obj)//用于取得 input，select value属性
{
	var lTmpNode;
	lTmpNode = Obj;
	if (lTmpNode != null)
	{
		if (lTmpNode.nodeName.toLowerCase() == "select")
		{
			return JS_UISelectGetCurValue(lTmpNode);
		}
		else if (lTmpNode.nodeName.toLowerCase() == "input")
		{
			if (lTmpNode.type.toLowerCase() == "checkbox")
			{
				if (lTmpNode.checked == true)
				{
					return "ON";
				}
				else
				{
					return "OFF";
				}
			}
		}
		return lTmpNode.value;
	}
	return "";
}

function JS_HTMLCloneNode(HtmlNode)
{
	var div = document.createElement('div');
	var lCloneNode = HtmlNode.cloneNode(true);
	lCloneNode.innerHTML = HtmlNode.innerHTML;
	return lCloneNode;
}


function JS_HTMLGetOuterHTML(HtmlNode)
{
	var div = document.createElement('div');
	var lCloneNode = JS_HTMLCloneNode(HtmlNode);
	div.appendChild(lCloneNode);
	return div.innerHTML;
}



function JS_HTMLFindElementByID(ParentItem, TagName, ID)
{
	var lTmpItems;
	var i;
	lTmpItems = ParentItem.getElementsByTagName(TagName);
	for (i = 0; i < lTmpItems.length; i++)
	{
		if (lTmpItems[i].id == ID)
		{
			return lTmpItems[i];
		}
	}
	return null;
}

function JS_HTMLFindElementByName(ParentItem, TagName, Name)
{
	var lTmpItems;
	var i;
	lTmpItems = ParentItem.getElementsByTagName(TagName);
	for (i = 0; i < lTmpItems.length; i++)
	{
		if (lTmpItems[i].name == Name)
		{
			return lTmpItems[i];
		}
	}
	return null;
}


function JS_HTMLCheckIsParent(Item, ParentID)
{
	while (Item != document.body)
	{
		if (Item.id == ParentID)
		{
			return true;
		}
		else
		{
			Item = Item.parentNode;
		}
	}
	return false;
}

function JS_HTMLGetType(lCheckVar)
{
	var _t;
	return ((_t = typeof (lCheckVar)) == "object" ? lCheckVar == null && "null" || Object.prototype.toString.call(lCheckVar).slice(8, -1) : _t).toLowerCase();
}


function JS_HTMLReclusiveShow(Obj, TopObj)
{
	if (Obj != null)
	{
		if (Obj != TopObj)
		{
			if (Obj.style.display == "none")
			{
				Obj.style.display = "block";
			}
			JS_HTMLReclusiveShow(Obj.parentNode, TopObj)
		}
	}
}


function JS_HTMLStringByteLength(s)
{
	var i;
	var lTotalByteLength = 0;
	var lCharCode;
	for (i = 0; i < s.length; i++)
	{
		lCharCode = s.charCodeAt(i);
		if (lCharCode < 0x007f)
		{
			lTotalByteLength = lTotalByteLength + 1;
		}
		else if ((0x0080 <= lCharCode) && (lCharCode <= 0x07ff))
		{
			lTotalByteLength += 2;
		}
		else if ((0x0800 <= lCharCode) && (lCharCode <= 0xffff))
		{
			lTotalByteLength += 3;
		}
	}
	return lTotalByteLength;
}

function JS_HTMLExecScript(ParentDoc, script)
{
	var oHead = ParentDoc.getElementsByTagName('HEAD').item(0);
	var oScript = ParentDoc.createElement("script");
	oScript.language = "javascript";
	oScript.type = "text/javascript";
	oScript.defer = true;
	oScript.text = script;
	oHead.appendChild(oScript);
}

function JS_UISelectCreate()
{
	return document.createElement("select");
}


function JS_UISelectAddOption(SelectObj, Value, Text)
{
	var lTmpItem;
	lTmpItem = document.createElement("option");
	lTmpItem.value = Value;
	lTmpItem.text = Text;
	SelectObj.add(lTmpItem);
}

function JS_UISelectAddOptionColor(SelectObj, Value, Text, Color, BGcolor)
{
    var lTmpItem;
    lTmpItem = document.createElement("option");
    lTmpItem.value = Value;
    lTmpItem.text = Text;
    lTmpItem.style.color = Color;
    lTmpItem.style.backgroundColor = BGcolor;
    SelectObj.add(lTmpItem);
}

function JS_UISelectSetCurIndexByValue(SelectObj, Value, bDefault)
{
	for (i = 0; i < SelectObj.length; i++)
	{
		if (SelectObj.options[i].value == Value)
		{
			SelectObj.options[i].selected = true;
			if (bDefault)
			{
				SelectObj.options[i].defaultSelected = true;
			}
			break;
		}
	}

	if ((i == SelectObj.length) && (SelectObj.length > 0))
	{
		SelectObj.options[0].selected = true;
	}
}

function JS_UISelectGetCurValue(SelectObj)
{
	return SelectObj.options[SelectObj.selectedIndex].value;
}

function JS_UISelectSetCurSelectOptionColor(SelectObj, Color, BGColor)
{
    SelectObj.options[SelectObj.selectedIndex].style.color = Color;
    SelectObj.options[SelectObj.selectedIndex].style.backgroundColor = BGColor;
}

function JS_UISelectRemoveAll(SelectObj)
{
	var i;
	for (i = SelectObj.length - 1; i >= 0; i--)
	{
		SelectObj.remove(i);
	}
}


function JS_UIAddCSSString(str_css)
{
	var lRet;

	try
	{ //IE下可行
		var style = document.createStyleSheet();
		style.cssText = str_css;
	}
	catch (e)
	{ //Firefox,Opera,Safari,Chrome下可行
		try
		{
			var style = document.createElement("style");
			style.type = "text/css";
			style.textContent = str_css;

			lRet = document.getElementsByTagName("HEAD").item(0).appendChild(style);

		}
		catch (e)
		{
			alert(e.name + ": " + e.message)
		}
	}
}

function JS_UISetErrorIndicator(Obj, bClear, bScrollToView)
{
	if (Obj != null)
	{
		if (bClear)
		{
			Obj.style.borderColor = "";
		}
		else
		{
			Obj.style.borderColor = "#FF0000";
			if (bScrollToView)
			{

				Obj.scrollIntoView();
			}
		}
	}
}

function JS_UISetErrorIndicatorByID(ID, bClear, bScrollToView)
{
	var lHtmlNode;
	lHtmlNode = $(ID);
	JS_UISetErrorIndicator(lHtmlNode, bClear, bScrollToView);
}


function JS_UISetDisableByID(ID, bClear)
{
	var lTmpVar = null;
	lTmpVar = $(ID);
	if (lTmpVar != null)
	{
		if (bClear)
		{
			lTmpVar.disabled = false;
		}
		else
		{
			lTmpVar.disabled = true;
		}
	}
}


function JS_UISetReadOnlyByID(ID, bClear)
{
	var lTmpVar = null;
	lTmpVar = $(ID);
	if (lTmpVar != null)
	{
		if (bClear)
		{
			lTmpVar.readOnly = false;
		}
		else
		{
			lTmpVar.readOnly = true;
		}
	}
}

function JS_ValidIPv4(IpAddressStr)
{
	var lTmpStr = IpAddressStr;
	if (/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(lTmpStr) && parseInt(RegExp.$1) < 256 && parseInt(RegExp.$2) < 256 && parseInt(RegExp.$3) < 256 && parseInt(RegExp.$4) < 256)
	{
		if (RegExp.$1 >= 224 && RegExp.$1 <= 239)
		{
			return 2;
		}
		else if (RegExp.$1 > 239 || RegExp.$1 == 0 || RegExp.$1 == 127)
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	return 1;
}

function JS_ValidIPv4WithMaskConfilict(IP1, MASK1, IP2, MASK2)
{
    var lIP1, lMask1, lIP2, lMask2;
    
    
    
    /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(IP1);
    lIP1 = 0;
    lIP1 += (parseInt(RegExp.$1) << 24) >>> 0;
    lIP1 += (parseInt(RegExp.$2) << 16) >>> 0;
    lIP1 += (parseInt(RegExp.$3) << 8) >>> 0;
    lIP1 += (parseInt(RegExp.$4) << 0) >>> 0;
    
    /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(MASK1);
    lMask1 = 0;
    lMask1 += (parseInt(RegExp.$1) << 24) >>> 0;
    lMask1 += (parseInt(RegExp.$2) << 16) >>> 0;
    lMask1 += (parseInt(RegExp.$3) << 8) >>> 0;
    lMask1 += (parseInt(RegExp.$4) << 0) >>> 0;
    
    /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(IP2);
    lIP2 = 0;
    lIP2 += (parseInt(RegExp.$1) << 24) >>> 0;
    lIP2 += (parseInt(RegExp.$2) << 16) >>> 0;
    lIP2 += (parseInt(RegExp.$3) << 8) >>> 0;
    lIP2 += (parseInt(RegExp.$4) << 0) >>> 0;
    
    /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(MASK2);
    lMask2 = 0;
    lMask2 += (parseInt(RegExp.$1) << 24) >>> 0;
    lMask2 += (parseInt(RegExp.$2) << 16) >>> 0;
    lMask2 += (parseInt(RegExp.$3) << 8) >>> 0;
    lMask2 += (parseInt(RegExp.$4) << 0) >>> 0;


    if ((lIP1 & lMask1) == (lIP2 & lMask2))
    {
        return false;
    }
    
    return true;
}


function JS_ValidIPv4Mask(IpAddressStr)
{
	var lTmpStr = IpAddressStr;
	if (/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(lTmpStr) && parseInt(RegExp.$1, 10) < 256 && parseInt(RegExp.$2, 10) < 256 && parseInt(RegExp.$3, 10) < 256 && parseInt(RegExp.$4, 10) < 256)
	{

	}
	else
	{
		return 0;
	}
	return 1;
}


function JS_IPv4StrToPart(IpAddressStr, Index)
{
	var IPValue;
	var lTmpStr = IpAddressStr;
	if (/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(lTmpStr) && parseInt(RegExp.$1, 10) < 256 && parseInt(RegExp.$2, 10) < 256 && parseInt(RegExp.$3, 10) < 256 && parseInt(RegExp.$4, 10) < 256)
	{
		if (Index == 0)
		{
			return parseInt(RegExp.$1, 10);
		}
		else if (Index == 1)
		{
			return parseInt(RegExp.$2, 10);
		}
		else if (Index == 2)
		{
			return parseInt(RegExp.$3, 10);
		}
		else if (Index == 3)
		{
			return parseInt(RegExp.$4, 10);
		}
	}
	return 0;
}

function JS_IPv4StrToBIN(IpAddressStr)
{
	var IPValue;
	var lTmpStr = IpAddressStr;
	if (/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(lTmpStr) && parseInt(RegExp.$1, 10) < 256 && parseInt(RegExp.$2, 10) < 256 && parseInt(RegExp.$3, 10) < 256 && parseInt(RegExp.$4, 10) < 256)
	{
		return ((parseInt(RegExp.$4, 10) * 256 + parseInt(RegExp.$3, 10)) * 256 + parseInt(RegExp.$2, 10)) * 256 + parseInt(RegExp.$1, 10);
	}
	else
	{
		return 0;
	}
}



function JS_ValidPort(PortStr, Min, Max)
{
	var lTmpStr = PortStr;
	if ((/^[0-9]*$/).test(lTmpStr) && (parseInt(lTmpStr, 10) >= Min) && (parseInt(lTmpStr, 10) <= Max))
	{
		return true;
	}
	else
	{
		return false;
	}

}



function JS_ValidHexData(HexString)
{
	var lDescriptorLen;
	if (HexString.length % 2 != 0 || (HexString.length < 2))
	{
		return false;
	}
	return true;
}


function JS_ParseIPString(IpAddressStr)
{
	var lTmpStr = IpAddressStr;
	var IPAddress = 0;
	if (/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.test(lTmpStr) && parseInt(RegExp.$1, 10) < 256 && parseInt(RegExp.$2, 10) < 256 && parseInt(RegExp.$3, 10) < 256 && parseInt(RegExp.$4, 10) < 256)
	{
		IPAddress = IPAddress * 256 + parseInt(RegExp.$1, 10);
		IPAddress = IPAddress * 256 + parseInt(RegExp.$2, 10);
		IPAddress = IPAddress * 256 + parseInt(RegExp.$3, 10);
		IPAddress = IPAddress * 256 + parseInt(RegExp.$4, 10);
	}
	return IPAddress;
}


function JS_StrLeftPading(Str, Length)
{
	if (JS_HTMLGetType(Str) != "string")
	{
		Str = Str.toString();
	}
	if (Str.length >= Length)
	{
		return Str;
	}
	else
	{
		return JS_StrLeftPading(("0" + Str), Length);
	}
}

function JS_StrLeftPadingSpace(Str, Length)
{
    if (JS_HTMLGetType(Str) != "string")
    {
        Str = Str.toString();
    }
    if (Str.length >= Length)
    {
        return Str;
    }
    else
    {
        return JS_StrLeftPadingSpace((" " + Str), Length);
    }
}

function JS_StrBitrateString(Bitrate)
{
	var lValue = "?? Bps";
	if (Bitrate / 1000000 > 9)
	{
		lValue = (Bitrate / 1000000).toFixed(3) + " Mbps";
	}
	else if (Bitrate / 1000 > 9)
	{
		lValue = (Bitrate / 1000).toFixed(3) + " Kbps";
	}
	else
	{
		lValue = Bitrate + " bps";
	}
	return lValue;
}

function JS_StrParseIntToPoint(Value)
{
	var lStr = "";
	var lTmpStr;
	var lTargetStr = "";
	var i;

	lStr = lStr + Value;

	var lPointIndex = lStr.indexOf(".");
	if (lPointIndex != -1)
	{
		lStr = lStr.substr(0, lPointIndex);
	}

	while (lStr.length > 3)
	{
		lTmpStr = lStr.substr(lStr.length - 3, 3);
		lTargetStr = "," + lTmpStr + lTargetStr;
		lStr = lStr.substr(0, lStr.length - 3);
	}

	lTargetStr = lStr + lTargetStr;
	return lTargetStr;
}


function JS_RestrictInput(obj, Reg, InputStr)
{
	var lSel, lSrcRange;
	var lTmpStr;
	var lSelRange;
	lSelRange = document.selection.createRange();
	if (lSelRange.parentElement().tagName != "INPUT")
	{
		return false;
	}
	lSel = lSelRange.duplicate();
	lSel.text = "";
	lSrcRange = obj.createTextRange();
	lSel.setEndPoint("StartToStart", lSrcRange);
	var lTmpStr = lSel.text + InputStr + lSrcRange.text.substr(lSel.text.length);
	return Reg.test(lTmpStr);
}

function JS_RestrctInputInt(Obj)
{
	Obj.onkeypress = function()
	{
		return JS_RestrictInput(this, /^[0-9]*$/, String.fromCharCode(event.keyCode));
	}
	Obj.onpaste = function()
	{
		return JS_RestrictInput(this, /^[0-9]*$/, window.clipboardData.getData('Text'));
	}
	Obj.ondrop = function()
	{
		return JS_RestrictInput(this, /^[0-9]*$/, event.dataTransfer.getData('Text'));
	}
	Obj.style.imeMode = "disabled";
}

function JS_RestrctInputFloat(Obj)
{
	Obj.onkeypress = function()
	{
		return JS_RestrictInput(this, /^[0-9.]*$/, String.fromCharCode(event.keyCode));
	}
	Obj.onpaste = function()
	{
		return JS_RestrictInput(this, /^[0-9.]*$/, window.clipboardData.getData('Text'));
	}
	Obj.ondrop = function()
	{
		return JS_RestrictInput(this, /^[0-9.]*$/, event.dataTransfer.getData('Text'));
	}
	Obj.style.imeMode = "disabled";
}

function JS_RestrctInputHEX(Obj)
{
	Obj.onkeypress = function()
	{
		return JS_RestrictInput(this, /^[a-fA-F0-9]*$/, String.fromCharCode(event.keyCode));
	}
	Obj.onpaste = function()
	{
		return JS_RestrictInput(this, /^[a-fA-F0-9]*$/, window.clipboardData.getData('Text'));
	}
	Obj.ondrop = function()
	{
		return JS_RestrictInput(this, /^[a-fA-F0-9]*$/, event.dataTransfer.getData('Text'));
	}
	Obj.style.imeMode = "disabled";
}

function JS_RestrctInputEngNum(Obj)
{
	Obj.onkeypress = function()
	{
		return JS_RestrictInput(this, /^[0-9a-zA-Z]*$/, String.fromCharCode(event.keyCode));
	}
	Obj.onpaste = function()
	{
		return JS_RestrictInput(this, /^[0-9a-zA-Z]*$/, window.clipboardData.getData('Text'));
	}
	Obj.ondrop = function()
	{
		return JS_RestrictInput(this, /^[0-9a-zA-Z]*$/, event.dataTransfer.getData('Text'));
	}
	Obj.style.imeMode = "disabled";
}


function JS_RestrctInputMAC(Obj)
{
	Obj.onkeypress = function()
	{
		return JS_RestrictInput(this, /^[a-fA-F0-9:]*$/, String.fromCharCode(event.keyCode));
	}
	Obj.onpaste = function()
	{
		return JS_RestrictInput(this, /^[a-fA-F0-9:]*$/, window.clipboardData.getData('Text'));
	}
	Obj.ondrop = function()
	{
		return JS_RestrictInput(this, /^[a-fA-F0-9:]*$/, event.dataTransfer.getData('Text'));
	}
	Obj.style.imeMode = "disabled";
}


function JS_BitrateFormatINT(Bitrate)
{
    if (Bitrate < 10000000)
    {
        return (Bitrate / 1000000).toFixed(3);
    }
    else if (Bitrate < 100000000)
    {
        return (Bitrate / 1000000).toFixed(2);
    }
    else
    {
        return (Bitrate / 1000000).toFixed(1);
    }
}
/************************************************************************/
/* 时间验证相关                                                         */
/************************************************************************/
var s_CMMGTsimeTestStart;

function JS_TIMETestSeed()
{
	s_CMMGTsimeTestStart = new Date().getTime();

}

function JS_TIMETestGetMs()
{
	var lEnd;
	lEnd = new Date().getTime();
	return lEnd - s_CMMGTsimeTestStart;
}


/************************************************************************/
/* 页面参数处理                                                         */
/************************************************************************/
function JS_ArgGetArgs()
{
	var i;
	var lUrlInfo;
	var lUrlInfoLen;
	var lIndexOfStart;
	var lArgsInfo;
	var lArgs;

	lUrlInfo = window.location.href; //获取当前页面的url 
	lUrlInfoLen - lUrlInfo.length;

	lIndexOfStart = lUrlInfo.indexOf("?"); //设置参数字符串开始的位置 
	lArgsInfo = lUrlInfo.substr(lIndexOfStart + 1, lUrlInfoLen); //取出参数字符串  

	lArgs = lArgsInfo.split("&"); //对获得的参数字符串按照“&”进行分割，的到数个类似“id=1”的参数字符串数组 

	return lArgs;
}

function JS_ArgGetValueByName(Args, ArgName)
{
	var i;
	var lArgCombo;
	var lArgv;

	lArgv = null;
	for (i = 0; i < Args.length; i = i + 1)
	{
		lArgCombo = Args[i].split("=");
		if (lArgCombo[0] == ArgName)
		{
			lArgv = lArgCombo[1];
			break;
		}
	}
	return lArgv;
}

function JS_ArgGetValueByNumber(Args, ArgNumber)
{
	var i;
	var lArgv;

	lArgv = null;
	if (Args.length > ArgNumber)
	{
		lArgv = Args[ArgNumber].split("=")[1];
	}
	return lArgv;
}

function JS_ISValidObject(Object)
{
    return (Object != null) && (typeof(Object) == "object");
}

function str_repeat(i, m)
{
	for (var o = []; m > 0; o[--m] = i);
	return o.join('');
}

function JS_Printf()
{
	var i = 0, a, f = arguments[i++], o = [], m, p, c, x, s = '';
	while (f)
	{
		if (m = /^[^\x25]+/.exec(f))
		{
			o.push(m[0]);
		}
		else if (m = /^\x25{2}/.exec(f))
		{
			o.push('%');
		}
		else if (m = /^\x25(?:(\d+)\$)?(\+)?(0|'[^$])?(-)?(\d+)?(?:\.(\d+))?([b-fosuxX])/.exec(f))
		{
			if (((a = arguments[m[1] || i++]) == null) || (a == undefined))
			{
				throw ('Too few arguments.');
			}
			if (/[^s]/.test(m[7]) && (typeof (a) != 'number'))
			{
				throw ('Expecting number but found ' + typeof (a));
			}
			switch (m[7])
			{
				case 'b': a = a.toString(2); break;
				case 'c': a = String.fromCharCode(a); break;
				case 'd': a = parseInt(a); break;
				case 'e': a = m[6] ? a.toExponential(m[6]) : a.toExponential(); break;
				case 'f': a = m[6] ? parseFloat(a).toFixed(m[6]) : parseFloat(a); break;
				case 'o': a = a.toString(8); break;
				case 's': a = ((a = String(a)) && m[6] ? a.substring(0, m[6]) : a); break;
				case 'u': a = Math.abs(a); break;
				case 'x': a = a.toString(16); break;
				case 'X': a = a.toString(16).toUpperCase(); break;
			}
			a = (/[def]/.test(m[7]) && m[2] && a >= 0 ? '+' + a : a);
			c = m[3] ? m[3] == '0' ? '0' : m[3].charAt(1) : ' ';
			x = m[5] - String(a).length - s.length;
			p = m[5] ? str_repeat(c, x) : '';
			o.push(s + (m[4] ? a + p : p + a));
		}
		else
		{
			throw ('Huh ?!');
		}
		f = f.substring(m[0].length);
	}
	return o.join('');
}

function getOs() { var OsObject = ""; if (navigator.userAgent.indexOf("MSIE") > 0) { return "MSIE"; } if (isFirefox = navigator.userAgent.indexOf("Firefox") > 0) { return "Firefox"; } if (isSafari = navigator.userAgent.indexOf("Safari") > 0) { return "Safari"; } if (isCamino = navigator.userAgent.indexOf("Camino") > 0) { return "Camino"; } if (isMozilla = navigator.userAgent.indexOf("Gecko/") > 0) { return "Gecko"; } }

function getBrowerAndVersion()
{
	var Sys = {};
	var ua = navigator.userAgent.toLowerCase();
	var s;
	(s = ua.match(/msie ([\d.]+)/)) ? Sys.ie = s[1] :
    (s = ua.match(/firefox\/([\d.]+)/)) ? Sys.firefox = s[1] :
    (s = ua.match(/chrome\/([\d.]+)/)) ? Sys.chrome = s[1] :
    (s = ua.match(/opera.([\d.]+)/)) ? Sys.opera = s[1] :
    (s = ua.match(/version\/([\d.]+).*safari/)) ? Sys.safari = s[1] : 0;

	//以下进行测试
//	if (Sys.ie) document.write('IE: ' + Sys.ie);
//	if (Sys.firefox) document.write('Firefox: ' + Sys.firefox);
//	if (Sys.chrome) document.write('Chrome: ' + Sys.chrome);
//	if (Sys.opera) document.write('Opera: ' + Sys.opera);
//	if (Sys.safari) document.write('Safari: ' + Sys.safari);
	return Sys;
}



function DisActiveX() 
 {
 	//微软IE支持的xmlhttp对象
 	var lSys;
 	lSys = getBrowerAndVersion();
 	if (lSys.ie) {
 		var aVersionhs = ["MSXML2.XMLHttp.5.0",
           "MSXML2.XMLHttp.4.0",
           "MSXML2.XMLHttp.3.0",
           "MSXML2.XMLHttp",
           "Microsoft.XMLHttp"];
 		//IE创建方式
 		for (var i = 0; i < aVersionhs.length; i++) 
 		{
 			try 
 			{
 				kXmlHttp = new ActiveXObject(aVersionhs[i]);
 				return true;
 			}
 			catch (e)
        { }
 		}
 	}
 	else
 	 {
	var kXmlHttp = null;
	try {
		//非微软IE支持的xmlhttp对象
		if (typeof XMLHttpRequest != "undefined") 
		{
			kXmlHttp = new XMLHttpRequest();
			return true;
		}
	}
	catch (e)
       { }
 	}
 	
	
       
	return false;
}

function JS_HTMLSetBitrateUsageBackground(lHtmlNode, Percentage)
{
    Percentage = parseInt(Percentage, 10);
    if (Percentage < 30)
    {
        lHtmlNode.style.backgroundColor = "aqua";
        lHtmlNode.style.color = "black";
    }
    else if (Percentage < 50)
    {
        lHtmlNode.style.backgroundColor = "lime";
        lHtmlNode.style.color = "black";
    }
    else if (Percentage < 70)
    {
        lHtmlNode.style.backgroundColor = "yellow";
        lHtmlNode.style.color = "white";
    }
    else if (Percentage < 90)
    {
        lHtmlNode.style.backgroundColor = "orange";
        lHtmlNode.style.color = "white";
    }
    else
    {
        lHtmlNode.style.backgroundColor = "red";
        lHtmlNode.style.color = "white";
    }

}

function JS_GetRandomRage(Min, Max)
{
    var Range = Max - Min;
    var Rand = Math.random();
    return (Min + Math.round(Rand * Range));
}  

function JS_ForcusKeepChangeOnClick(ParentItem, Obj, Color, BGColor)
{
    var i;
    var lLinkArray;
    var lTmpObj;

    lLinkArray = ParentItem.getElementsByTagName("a");

    for (i = 0; i < lLinkArray.length; i++)
    {
        lTmpObj = lLinkArray[i];
        if (lTmpObj == Obj) 
        {
            lTmpObj.style.color = Color;
            lTmpObj.style.backgroundColor  = BGColor;
        }
        else
        {
            lTmpObj.style.color = 'black';
            lTmpObj.style.backgroundColor  = 'white';
        }
    }
}


function JS_ForcusKeepChangeCallbackSetup(ParentItem, Color, BGColor)
{
    var i;
    var lLinkArray;
    var lTmpObj;

    lLinkArray = ParentItem.getElementsByTagName("a");

    for (i = 0; i < lLinkArray.length; i++)
    {
        lTmpObj = lLinkArray[i];
        if (lTmpObj.target == 'right_frame') 
        {
            lTmpObj.onclick = function()
            {
                JS_ForcusKeepChangeOnClick(ParentItem, this, Color, BGColor);
            }
        }
    }
}


/*EOF*/

