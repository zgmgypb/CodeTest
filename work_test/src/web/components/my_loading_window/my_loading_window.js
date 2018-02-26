
/*显示一个透明一个半透明的窗口，主要作用是阻止用户的操作*/


function MyLoadingWindow(ParentDoc, ID)
{
	this.m_ParentDoc = ParentDoc;
	this.m_ID = ID;
	this.m_CountDown = 0;
	this.m_bShowCount = false;
	this.m_bAutoHide = false;
	this.m_Padding = 0;
	this.m_OnCloseScript = "";
}

MyLoadingWindow.prototype.SetPadding = function(Padding)
{
	this.m_Padding = Padding;
}

MyLoadingWindow.prototype.ShowWindow = function(CountDown, bShowCount, bAutoHide, OnCloseScript)
{
	var lTmpStr;
	var lTransSheet, lMaskSheet, lLoading, lIFrameSheet;
	var lWindowWidth = parseInt(this.m_ParentDoc.documentElement.scrollWidth, 10);
	var lWindowHeight = parseInt(this.m_ParentDoc.documentElement.scrollHeight, 10);
	var lWidth, lHeight, lLeft, lTop, lPadding;
	var lSys;
	
	lSys = getBrowerAndVersion();

	lPadding = this.m_Padding;
	
	this.m_CountDown = CountDown;
	this.m_bShowCount = bShowCount;
	this.m_bAutoHide = bAutoHide;
	this.m_OnCloseScript = OnCloseScript;


	lTransSheet = this.m_ParentDoc.createElement("div");
	lTransSheet.id = this.m_ID;
	lTmpStr = "text-align:center;background:#666;position:absolute;z-index:1;top:" + lPadding + "px;left:" + lPadding + "px;width:" + (lWindowWidth - lPadding * 2) + "px;height:" + (lWindowHeight - lPadding * 2) + "px;";
	lTmpStr += (JS_IsIE(5)) ? "filter:alpha(opacity=10);" : "opacity:0.1;";
	lTransSheet.style.cssText = lTmpStr;

	if (lSys.ie > 8) 
	{
		lTmpStr = "<img src='components/my_loading_window/img/hide.gif' style='padding-top:" + 0 + "px;'  />";
	}
	else 
	{
		lTmpStr = "<img src='' style='display:none;'/>";
	}
	
	

	lTransSheet.innerHTML = lTmpStr;
	lIFrameSheet = this.m_ParentDoc.createElement("iframe");
	lTmpStr = "text-align:center;background:#666;position:absolute;z-index:-1;top:" + lPadding + "px;left:" + lPadding + "px;width:" + (lWindowWidth - lPadding * 2) + "px;height:" + (lWindowHeight - lPadding * 2) + "px;"
	lTmpStr += (JS_IsIE(5)) ? "filter:alpha(opacity=10);" : "opacity:0.1;";
	lIFrameSheet.style.cssText = lTmpStr;
	lTransSheet.appendChild(lIFrameSheet);
	this.m_ParentDoc.body.appendChild(lTransSheet);


	if (lWindowWidth > 640)
	{
		lWidth = 640;
	}
	else
	{
		lWidth = lWindowWidth;
	}

	if (lWindowHeight > 480)
	{
		lHeight = 480;
	}
	else
	{
		lHeight = lWindowHeight;
	}

	lTop = (lWindowHeight - lHeight) / 2;
	lLeft = (lWindowWidth - lWidth) / 2;

	lMaskSheet = this.m_ParentDoc.createElement("div");
	lMaskSheet.id = this.m_ID + "_mask";
	lTmpStr = "text-align:center;background:#666;position:absolute;z-index:2;top:" + lTop + "px;left:" + lLeft + "px;width:" + lWidth + "px;height:" + lHeight + "px;";
	lTmpStr += (JS_IsIE(5)) ? "filter:alpha(opacity=40);" : "opacity:0.4;";
	lMaskSheet.style.cssText = lTmpStr;
	lTmpStr = "<img src='components/my_loading_window/img/loading.gif' style='padding-top:" + (lHeight / 2 - 20) + "px;'  />";
	lMaskSheet.innerHTML = lTmpStr;
	this.m_ParentDoc.body.appendChild(lMaskSheet);


	lTop = (lWindowHeight - 30) / 2 + 80;
	lLeft = (lWindowWidth - 40) / 2;

	lLoading = this.m_ParentDoc.createElement("div");
	lLoading.id = this.m_ID + "_count";
	lTmpStr = "text-align:center;background:#666;position:absolute;z-index:3;top:" + lTop + "px;left:" + lLeft + "px;width:" + 40 + "px;height:" + 20 + "px;";
	lTmpStr += (JS_IsIE(5)) ? "filter:alpha(opacity=100);" : "opacity:1.0;";
	lLoading.style.cssText = lTmpStr;
	if (this.m_bShowCount == false)
	{
		lLoading.style.display = "none";
	}
	if (lSys.ie > 8) 
	{
		lTmpStr = "<img src='components/my_loading_window/img/hide.gif' style='width:1px;height:1px;display:block;'  />";
	}
	else 
	{
		lTmpStr = "<img src='' style='display:none;'/>";
	}
	lTmpStr += "<input type='text' style=text-align:center;width:40px;' value='" + this.m_CountDown + "' />";
	lLoading.innerHTML = lTmpStr;
	this.m_ParentDoc.body.appendChild(lLoading);

}

MyLoadingWindow.prototype.HideWindow = function(bForce)
{
	if (bForce)
	{
		this.m_CountDown = 0;
	}

	if (this.m_CountDown <= 0)
	{
		var lItem;
		
		/*执行脚本*/
		if (this.m_OnCloseScript)
		{
			JS_HTMLExecScript(this.m_ParentDoc, this.m_OnCloseScript);
			this.m_OnCloseScript = null;
		}
		
		lItem = this.m_ParentDoc.getElementById(this.m_ID);
		if (lItem) {
			lItem.parentNode.removeChild(lItem);
		}
		lItem = this.m_ParentDoc.getElementById(this.m_ID + "_mask");
		if (lItem) {
			lItem.parentNode.removeChild(lItem);
		}
		lItem = this.m_ParentDoc.getElementById(this.m_ID + "_count");
		if (lItem)
		{
			lItem.parentNode.removeChild(lItem);
		}
	}
}

MyLoadingWindow.prototype.OnResize = function()
{
	var lWindowWidth = parseInt(this.m_ParentDoc.documentElement.scrollWidth, 10);
	var lWindowHeight = parseInt(this.m_ParentDoc.documentElement.scrollHeight, 10);
	var lWidth, lHeight;

	if (this.m_ParentDoc.getElementById(this.m_ID)) 
	{
		this.m_ParentDoc.getElementById(this.m_ID).style.width = lWindowWidth - 20;
		this.m_ParentDoc.getElementById(this.m_ID).style.height = lWindowHeight - 20;

		if (lWindowWidth > 640)
		{
			lWidth = 640;
		}
		else
		{
			lWidth = lWindowWidth;
		}

		if (lWindowHeight > 480)
		{
			lHeight = 480;
		}
		else
		{
			lHeight = lWindowHeight;
		}

		lTop = (lWindowHeight - lHeight) / 2;
		lLeft = (lWindowWidth - lWidth) / 2;

		this.m_ParentDoc.getElementById(this.m_ID + "_mask").style.top = lTop;
		this.m_ParentDoc.getElementById(this.m_ID + "_mask").style.left = lLeft;

		lTop = (lWindowHeight - 30) / 2 + 80;
		lLeft = (lWindowWidth - 40) / 2;

		this.m_ParentDoc.getElementById(this.m_ID + "_count").style.top = lTop;
		this.m_ParentDoc.getElementById(this.m_ID + "_count").style.left = lLeft;
	}

}

MyLoadingWindow.prototype.OnTimer = function()
{
	if (this.m_CountDown > 0)
	{
		this.m_CountDown--;
		if (this.m_bShowCount)
		{
			var lCountVar;
			lCountVar = this.m_ParentDoc.getElementById(this.m_ID + "_count");
			lCountVar.getElementsByTagName("input")[0].value = this.m_CountDown;
			if (this.m_CountDown == 0)
			{
				lCountVar.style.display = "none";
				if (this.m_bAutoHide)
				{
					this.HideWindow(false);
				}
			}
		}
	}
}

MyLoadingWindow.prototype.IsVisiable = function()
{
	if (this.m_ParentDoc.getElementById(this.m_ID))
	{
		return true;
	}
	return false;
}
/************************************************************************
* Java Script for Check Browser Status
************************************************************************/		
function JS_IsIE( version )
{
	return true;
}

/************************************************************************
* Java Script Create Transparet Window To Cover Whole Window (HtmlDom)
************************************************************************/		
function JS_ShowTransparetWindow(ParentDoc, Id, bFullWindow)
{
	var lTmpStr;
	var lTransSheet, lMaskSheet, lLoading;
	var lWindowWidth = parseInt(ParentDoc.documentElement.scrollWidth, 10);
	var lWindowHeight = parseInt(ParentDoc.documentElement.scrollHeight, 10);
	var lWidth, lHeight;
	
	lTransSheet = ParentDoc.createElement("div");
	lTransSheet.id = Id;
	lTmpStr = "text-align:center;background:#666;position:absolute;z-index:1;top:0px;left:0px;width:" + lWindowWidth + "px;height:" + lWindowHeight + "px;";
	lTmpStr += (JS_IsIE(5))?"filter:alpha(opacity=40);":"opacity:0.00;";
	lTransSheet.style.cssText = lTmpStr;
	lTmpStr = "<img src='' style='display:none;'/>";
	lTransSheet.innerHTML = lTmpStr;

	if (lWindowWidth > 640)
	{
		lWidth = 640;
	}
	else
	{
		lWidth = lWindowWidth;
	}
	
	if (lWindowHeight > 480)
	{
		lHeight = 480;
	}
	else
	{
		lHeight = lWindowHeight;
	}

	lMaskSheet = ParentDoc.createElement("div");
	lMaskSheet.id = Id + "_mask";
	lTmpStr = "text-align:center;background:#666;position:absolute;z-index:2;top:" + (lWindowHeight - lHeight) / 2 + "px;left:" + (lWindowWidth - lWidth) / 2 + "px;width:" + lWidth + "px;height:" + lHeight + "px;";
 	lTmpStr += (JS_IsIE(5))?"filter:alpha(opacity=40);":"opacity:0.4;";
	lMaskSheet.style.cssText = lTmpStr;
	lTmpStr = "<img src='components/my_loading_window/img/loading.gif' style='padding-top:" + (lHeight / 2 - 20) + "px;'  />";
	lTmpStr += "<div>6</div>";
	lMaskSheet.innerHTML = lTmpStr;


	lLoading = ParentDoc.createElement("div");
	lLoading.id = Id + "_Count";
	lTmpStr = "text-align:center;background:#FFFFFF;position:absolute;z-index:3;top:" + ((lWindowHeight - 30) / 2 + 80) + "px;left:" + (lWindowWidth - 40) / 2 + "px;width:" + 40 + "px;height:" + 20 + "px;";
	lTmpStr += (JS_IsIE(5)) ? "filter:alpha(opacity=100);" : "opacity:1.0;";
	lLoading.style.cssText = lTmpStr;
	lTmpStr = "<img src='' style='display:none;'/>";
	lTmpStr += "<input type='text' style=text-align:center;width:40px;' value='0'>";
	lLoading.innerHTML = lTmpStr;
	
	ParentDoc.body.appendChild(lTransSheet);
	ParentDoc.body.appendChild(lMaskSheet);
 	ParentDoc.body.appendChild(lLoading);
}

function JS_HideTransparetWindow(ParentDoc, Id)
{
	var lItem = ParentDoc.getElementById(Id);
	if( lItem != null)
	{
		lItem.parentNode.removeChild(lItem);
	}
	else
	{
		alert("Can't Find Id = <" + Id + ">");
	}
}
