/*UTF-8编码*/








function MyListControl(ObjectName, Width, Height, DivTargetID, bCheckCol)
{
	var lHtmlString = "";
	var lTargetItem;

	this.div =
	{
		width: Width,
		height: Height
	};

	this.title =
	{
		bhave_title: false,
		title_str: "",
		title_align: "center",
		height: 25
	};

	this.head =
	{
		bg: "#DDDDff",
		bg_hover: "#DDDDff",
		bcheck: true, //是否将最后一行作为选择行，此时Title行自动生成全选Check控件！
		height: 25//列标题行高度
	};

	this.rows =
	{
		bg: "#f0f0fe",
		bg_hover: "#DDDDff",
		height: 25
	};

	this.m_TitleCheckObj = null;

	this.Object = ObjectName;
	this.DivTagetID = DivTargetID;
	this.TitleAlignArray = [];
	this.ColWidthArray = [];
	this.ColAlignArray = [];
	this.ColValueArray = [];
	this.bCheckCol = bCheckCol;
}

MyListControl.prototype.SetRowProperty = function(BG, BGHover, Height)
{
	if (BG != null)
	{
		this.rows.bg = BG;
	}
	if (BGHover != null)
	{
		this.rows.bg_hover = BGHover;
	}
	if (Height != null)
	{
		this.rows.height = Height;
	}
};

MyListControl.prototype.SetHeadProperty = function(BG, BGHover, Height, bHaveCheck)
{
	if (BG != null)
	{
		this.head.bg = BG;
	}

	if (BGHover != null)
	{
		this.head.bg_hover = BGHover;
	}

	if (Height != null)
	{
		this.head.height = Height;
	}

	if (bHaveCheck != null)
	{
		this.head.bcheck = bHaveCheck;
	}
};


MyListControl.prototype.SetTitleProperty = function(bHaveTitle, TitleStr, TitleAlign, TitleHeight)
{
	this.title.bhave_title = bHaveTitle;
	this.title.title_str = TitleStr;
	this.title.title_align = TitleAlign;
	this.title.height = TitleHeight;
};


MyListControl.prototype.SetColProperty = function(ColIndex, Width, TitleString, TitleAlign, ColAlign)
{
	this.ColWidthArray[ColIndex] = Width;
	this.ColAlignArray[ColIndex] = ColAlign;
	this.TitleAlignArray[ColIndex] = TitleAlign;
	this.ColValueArray[ColIndex] = TitleString;
};

MyListControl.prototype.UpdateListFrame = function()
{
	var lCSSString;
	var i;
	lCSSString = "";
	lCSSString += ".css_" + this.Object + " ul { list-style-type: none;} \r\n";
	//JS_UIAddCSSString(lCSSString);
	lCSSString += ".css_" + this.Object + " li { background:" + this.rows.bg + ";text-decoration:none;height:" + this.rows.height + "px; margin-left: 10px;margin-right: 10px;margin-bottom: 2px;margin-top: 2px;} \r\n";
	//JS_UIAddCSSString(lCSSString);
	lCSSString += ".css_" + this.Object + " li:hover { background:" + this.rows.bg_hover + ";}";
	//JS_UIAddCSSString(lCSSString);
	lCSSString += ".css_" + this.Object + " #" + this.Object + "_title li { background:" + this.head.bg + ";text-decoration:none;height:" + this.head.height + "px; margin-left: 10px;margin-right: 10px;margin-bottom: 2px;margin-top: 2px;} \r\n";
	//JS_UIAddCSSString(lCSSString);
	lCSSString += ".css_" + this.Object + " #" + this.Object + "_title li:hover { background:" + this.head.bg_hover + ";} \r\n";
	//JS_UIAddCSSString(lCSSString);


	for (i = 0; i < this.ColWidthArray.length; i++)
	{
		lCSSString += ".css_title_" + this.Object + "_col_" + i + " {padding-top:3px;width:" + this.ColWidthArray[i] + "px; display:inline-block;text-align:" + this.TitleAlignArray[i] + ";border: 0px solid #999;} \r\n";
		//JS_UIAddCSSString(lCSSString);
	}

	for (i = 0; i < this.ColWidthArray.length; i++)
	{
		lCSSString += ".css_" + this.Object + "_col_" + i + " {padding-top:3px;width:" + this.ColWidthArray[i] + "px; display:inline-block;text-align:" + this.ColAlignArray[i] + ";border: 0px solid #999;} \r\n";
		//JS_UIAddCSSString(lCSSString);
	}

	JS_UIAddCSSString(lCSSString);
	
	//生成框架
	lHtmlString = "<div class='css_" + this.Object + "' id='" + this.Object + "' style='align:left;height:" + this.div.height + "px; width:" + this.div.width + "px; overflow:auto;text-align:left;' >"
	lHtmlString += "<ul id='" + this.Object + "_title' ></ul>";
	lHtmlString += "<div style='height:" + (this.div.height - this.head.height - 10) + "px; width:" + (this.div.width - 10) + "px;overflow:auto;'>"
	lHtmlString += "<ul id='" + this.Object + "_data' class='" + this.Object + "' >";
	lHtmlString += "</ul>";
	lHtmlString += "</div>";
	lHtmlString += "</div>";

	lTargetItem = document.getElementById(this.DivTagetID);
	if (lTargetItem != null)
	{
		lTargetItem.innerHTML = lHtmlString;
	}

	this.AddRowToDom(true, 0, false, null, null);
};

MyListControl.prototype.SetColValue = function(ColIndex, HtmlItem)
{
	this.ColValueArray[ColIndex] = null;
	this.ColValueArray[ColIndex] = HtmlItem;
};


MyListControl.prototype.AddRowToDom = function(bHead, RowData, bScrollToView, OnMouseOver, OnMouseOut)
{
	//生成新的HTML并将其添加到队列尾部
	var i;
	var lHtmlString = "";
	var lULNode = null;
	var lLINode, lSpanNode;
	if (bHead)
	{
		lULNode = document.getElementById(this.Object + "_title");
	}
	else
	{
		lULNode = document.getElementById(this.Object + "_data");
	}
	if (lULNode != null)
	{
		lLINode = document.createElement("li");
		if (!bHead)
		{
			this.ColValueArray[0] = (lULNode.getElementsByTagName("li").length + 1);
		}

		lLINode.name = RowData;

		for (i = 0; i < this.ColWidthArray.length; i++)
		{
			lSpanNode = document.createElement("span");
			if (bHead)
			{
				lSpanNode.className = "css_title_" + this.Object + "_col_" + i;
			}
			else
			{
				lSpanNode.className = "css_" + this.Object + "_col_" + i;
			}

			if (this.bCheckCol && (i == this.ColWidthArray.length - 1) && ((!bHead) || (bHead && this.head.bcheck)))
			{
				var lCheckBox;
				lCheckBox = document.createElement("input");
				lCheckBox.type = "checkbox";
				lCheckBox.style.width = "40px";
				lCheckBox.name = "checkbox_" + i;
				if (bHead)
				{
					lCheckBox.onclick = this.OnClickCheck(this);
					this.m_TitleCheckObj = lCheckBox;
				}
				this.ColValueArray[i] = lCheckBox;
			}

			var lTmpValue = JS_HTMLGetType(this.ColValueArray[i]);
			if (JS_HTMLGetType(this.ColValueArray[i]) != "object")
			{
				lSpanNode.innerHTML = this.ColValueArray[i];
			}
			else
			{
				lSpanNode.appendChild(this.ColValueArray[i]);
			}

			if (typeof (OnMouseOver) != 'undefined')
			{
				lLINode.onmouseover = OnMouseOver;
			}
			
			if (typeof (OnMouseOut) != 'undefined')
			{
				lLINode.onmouseout = OnMouseOut;
			}
			
			lLINode.appendChild(lSpanNode);
		}
		lULNode.appendChild(lLINode);

		if (bScrollToView)
		{
			lLINode.scrollIntoView();
		}
	}
};

MyListControl.prototype.RemoveRow = function(RowIndex)
{
	var i;
	var lULNode = null;
	var lLINode, lSpanNode;

	lULNode = document.getElementById(this.Object + "_data");
	if (lULNode != null)
	{
		lLINode = lULNode.getElementsByTagName("li");
		if (lLINode.length > RowIndex)
		{
			lULNode.removeChild(lLINode[RowIndex]);
		}
	}
};


MyListControl.prototype.RemoveAllRow = function()
{
	var i;
	var lULNode = null;
	var lLINode, lSpanNode;

	lULNode = document.getElementById(this.Object + "_data");
	if (lULNode != null)
	{
		lLINode = lULNode.getElementsByTagName("li");
		for (i = lLINode.length - 1; i >=0 ; i--)
		{
			lULNode.removeChild(lLINode[i]);
		}
	}
};


MyListControl.prototype.GetRowData = function(RowIndex)
{
	var i;
	var lULNode = null;
	var lLINode, lSpanNode;

	lULNode = document.getElementById(this.Object + "_data");
	if (lULNode != null)
	{
		lLINode = lULNode.getElementsByTagName("li");
		if (lLINode.length > RowIndex)
		{
			return lLINode[RowIndex].name;
		}
	}
	return 0;
};


MyListControl.prototype.RebuildIndex = function()
{
	//生成新的HTML并将其添加到队列尾部
	var i;
	var lULNode = null;
	var lLINode, lSpanNode;

	lULNode = document.getElementById(this.Object + "_data");

	if (lULNode != null)
	{
		lLINode = lULNode.getElementsByTagName("li");
		for (i = 0; i < lLINode.length; i++)
		{
			lSpanNode = lLINode[i].getElementsByTagName("span");
			if (lSpanNode.length)
			{
				lSpanNode[0].innerHTML = i + 1;
			}
		}
	}
};


MyListControl.prototype.RemoveSelectedRow = function()
{
	var lTmpNode;
	var i, lRowNum;
	lRowNum = this.GetRowNumber();
	lCheckColIndex = this.ColWidthArray.length - 1;
	for (i = lRowNum - 1; i >= 0; i--)
	{
		lTmpNode = this.GetColNodeByIndex(i, lCheckColIndex);
		if (lTmpNode.checked == true)
		{
			this.RemoveRow(i);
		}
	}
	this.RebuildIndex();
	if (this.m_TitleCheckObj) 
	{
		this.m_TitleCheckObj.checked = false;
	}
};


MyListControl.prototype.GetRowChecked = function(RowNum)
{
    return this.GetColNodeByIndex(RowNum, this.ColWidthArray.length - 1).checked;
};

MyListControl.prototype.GetColNodeByName = function(RowIndex, ColName)
{
	//生成新的HTML并将其添加到队列尾部
	var i;
	var lHtmlString = "";
	var lULNode = null;
	var lLINode, lSpanNode;

	lULNode = document.getElementById(this.Object + "_data");

	if (lULNode != null)
	{
		lLINode = lULNode.getElementsByTagName("li");
		if (RowIndex < lLINode.length)
		{
			lSpanNode = lLINode[RowIndex].getElementsByTagName("span");
			for (i = 0; i < lSpanNode.length; i++)
			{
				if (lSpanNode[i].firstChild != null)
				{
					if (lSpanNode[ColIndex].firstChild.name == ColName)
					{
						return lSpanNode[ColIndex].firstChild;
					}
				}
			}
		}
	}
	return null;
};

MyListControl.prototype.GetColNodeByIndex = function(RowIndex, ColIndex)
{
	//生成新的HTML并将其添加到队列尾部
	var lULNode = null;
	var lLINode, lSpanNode;

	lULNode = document.getElementById(this.Object + "_data");

	if (lULNode != null)
	{
		lLINode = lULNode.getElementsByTagName("li");
		if (RowIndex < lLINode.length)
		{
			lSpanNode = lLINode[RowIndex].getElementsByTagName("span");
			for (i = 0; i < lSpanNode.length; i++)
			{
				if (lSpanNode[i].className == "css_" + this.Object + "_col_" + ColIndex)
				{
					return lSpanNode[i].firstChild;
				}
			}
		}
	}
	return null;
};


MyListControl.prototype.OnClickCheck = function(Item)
{
	return function()
	{
		var lTmpNode;
		var i, lRowNum;
		var lbCheck, lCheckColIndex;
		var lULNode;

		lCheckColIndex = Item.ColWidthArray.length - 1;
		lbCheck = Item.m_TitleCheckObj.checked;
		lULNode = document.getElementById(Item.Object + "_title");
		lRowNum = Item.GetRowNumber();
		for (i = 0; i < lRowNum; i++)
		{
			lTmpNode = Item.GetColNodeByIndex(i, lCheckColIndex);
			lTmpNode.checked = lbCheck;
		}
	}
}

MyListControl.prototype.GetRowNumber = function()
{
	//生成新的HTML并将其添加到队列尾部
	var i;
	var lHtmlString = "";
	var lULNode = null;
	var lLINode, lSpanNode;

	lULNode = document.getElementById(this.Object + "_data");

	if (lULNode != null)
	{
		lLINode = lULNode.getElementsByTagName("li");
		return lLINode.length;
	}

};

MyListControl.prototype.GetColNumber = function()
{
	return lCheckColIndex = this.ColWidthArray.length - 1;
};
