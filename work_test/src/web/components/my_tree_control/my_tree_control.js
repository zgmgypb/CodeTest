/*--------------------------------------------------|

| MyTreeControl 2.05 | www.destroydrop.com/javascript/tree/ |

|---------------------------------------------------|

| Copyright (c) 2002-2003 Geir Landr?              |

|                                                   |

| This script can be used freely as long as all     |

| copyright messages are intact.                    |

|                                                   |

| Updated: 17.04.2003                               |

|--------------------------------------------------*/

function Node(id, pid, html, icon, iconOpen) 
{
	this.id = id;

	this.pid = pid;

	this.html = html;

	this.icon = icon;

	this.iconOpen = iconOpen;

	this._io = false;

	this.LastSibling = true;

	this.bHaveChild = false;

	this.FirstChild = -1;
	
	this.LastChild = -1;
	
	this._ai = 0;

	this._p = 0;

}

// Tree Objectect

function MyTreeControl(ObjectName) {

	this.config = 
	{
		target				: null,

		folderLinks			: true,

		useSelection		: true,

		useCookies			: true,

		useLines			: true,

		useIcons			: true,

		useStatusText		: false,

		closeSameLevel		: false,

		inOrder				: false
	};

	this.icon = 
	{
		RootNode			: './components/my_tree_control/img/globe.gif',

		folder				: './components/my_tree_control/img/folder.gif',

		folderOpen			: './components/my_tree_control/img/folderopen.gif',

		node				: './components/my_tree_control/img/page.gif',

		empty				: './components/my_tree_control/img/empty.gif',

		line				: './components/my_tree_control/img/line.gif',

		join				: './components/my_tree_control/img/join.gif',

		joinBottom			: './components/my_tree_control/img/joinbottom.gif',

		plus				: './components/my_tree_control/img/plus.gif',

		plusBottom			: './components/my_tree_control/img/plusbottom.gif',

		minus				: './components/my_tree_control/img/minus.gif',

		minusBottom			: './components/my_tree_control/img/minusbottom.gif',

		nlPlus				: './components/my_tree_control/img/nolines_plus.gif',

		nlMinus				: './components/my_tree_control/img/nolines_minus.gif',
		
		minusHead			: './components/my_tree_control/img/minushead.gif',

		plusHead			: './components/my_tree_control/img/plushead.gif'
	};

	this.Object = ObjectName;

	this.NodesArray = [];

	this.NodesArray[0] = new Node(0,-1,"");

	this.selectedNode = -1;

	this.selectedFound = false;

	this.completed = false;
	
// 	this.count_id = 0;

}

// Adds a new node to the node array

MyTreeControl.prototype.AddNewNode = function(PIndex, Html, Icon, IconOpen) 
{
	//var lPID;
	var lCurIndex = this.NodesArray.length;
	//if (PIndex == -1) 
	//{
		//根节点
	//	lPID = 0;
	//}
	//else
	{
		//得到父节点ID
		//lPID = this.NodesArray[PIndex].id;
		//修正父节点的子节点索引,以及修正最后一个子节点的表示自己是最后的兄弟节点的标识。
		if (this.NodesArray[PIndex].bHaveChild == true) 
		{
			this.NodesArray[this.NodesArray[PIndex].LastChild].LastSibling = false;
			this.NodesArray[PIndex].LastChild = lCurIndex;
		}
		else
		{
			this.NodesArray[PIndex].bHaveChild = true;
			this.NodesArray[PIndex].FirstChild = lCurIndex;
			this.NodesArray[PIndex].LastChild = lCurIndex;
		}
		
// 		this.count_id ++;
		this.NodesArray[this.NodesArray.length] = new Node(this.NodesArray.length, this.NodesArray[PIndex].id, Html, Icon, IconOpen);
	}
	
	return lCurIndex;
};


MyTreeControl.prototype.GetNode = function(id)
{
	var n;
	for (n = 0; n<this.NodesArray.length; n++) 
	{
		if (this.NodesArray[n].id == id) 
		{
			return this.NodesArray[n];
		}
	}
	return 0;
};

// Outputs the tree to the page
MyTreeControl.prototype.toString = function() 
{
	var str = "";
	
	if (this.config.useCookies) 
	{
		//this.selectedNode = this.getSelected();
	}
	
	str += this.GetSubLevelHTML(0, 1, false);
	
	this.completed = true;
	
	return str;
};


MyTreeControl.prototype.CollapseNode = function(status, id, bottom, node, RecurLevel, OpenLevel)
{
	var eIcon;
	var eDiv = document.getElementById('d' + this.Object + id);
	var eJoin = document.getElementById('j' + this.Object + id);
	var n;
	if (this.config.useIcons)
	{
		eIcon = document.getElementById('i' + this.Object + id);
		eIcon.src = (status) ? node.iconOpen : node.icon;
	}

	/*切换 + - 号图标*/
	if (eJoin)
	{
		if (id === 1)
		{
			eJoin.src = (status) ? this.icon.minusHead : this.icon.plusHead;
		}
		else
		{
			eJoin.src = (this.config.useLines) ? ((status) ? ((bottom) ? this.icon.minusBottom : this.icon.minus) : ((bottom) ? this.icon.plusBottom : this.icon.plus)) : ((status) ? this.icon.nlMinus : this.icon.nlPlus);
		}
	}

	if (eDiv)
	{
		eDiv.style.display = (status) ? 'block' : 'none';

		if (eDiv.innerHTML === "")
		{
			/*生成子节点HTML并导入到innerHTML*/
			eDiv.innerHTML = this.GetSubLevelHTML(id, RecurLevel, OpenLevel);
		}
		else
		{
			var lNewRecLevel = RecurLevel - 1;
			var lNewOpenLevel = OpenLevel - 1;
			for (n = node.FirstChild; n <= node.LastChild; n++)
			{
				if (this.NodesArray[n].pid == node.id)
				{
					this.ToggleCollapse(this.NodesArray[n].id, lNewRecLevel, lNewOpenLevel);
				}
			}
		}
	}
};


/* 绘制参数节点的子节点 */
MyTreeControl.prototype.GetSubLevelHTML = function(id, RecurLevel, OpenLevel)
{
	var lStr = '';
	var plNode = null;
	var n;

	RecurLevel--;
	OpenLevel--;

	for (n = 0; n < this.NodesArray.length; n++)
	{
		if (this.NodesArray[n].id == id)
		{
			plNode = this.NodesArray[n];
			break;
		}
	}

	if (n == this.NodesArray.length)
	{
		return "";
	}

	n = 0;

	if (this.config.inOrder)
	{
		n = plNode._ai;
	}

	/*Test Time*/
	//var tDate1 = new Date();

	for (n = plNode.FirstChild; n <= plNode.LastChild; n++)
	{
		if (this.NodesArray[n].pid == plNode.id)
		{
			var lCurNode = this.NodesArray[n];
			lCurNode._p = plNode;
			lCurNode._ai = n;
			//this.SetCS(lCurNode);

			if (lCurNode.bHaveChild && !lCurNode._io && this.config.useCookies)
			{
				lCurNode._io = false /*this.isOpen(lCurNode.id)*/;
			}

			if (!this.config.folderLinks && lCurNode.bHaveChild)
			{
				lCurNode.url = null;
			}

			lStr += this.GetNodeHTML(lCurNode, this.NodesArray[n].id, RecurLevel, OpenLevel);

			if (lCurNode.LastSibling)
			{
				break;
			}
		}
	}

	//var tDate2 = new Date();
	//alert(tDate2.getTime() - tDate1.getTime());

	return lStr;
};


MyTreeControl.prototype.GetNodeHTML = function(node, nodeId, RecurLevel, OpenLevel) 
{

	if (OpenLevel > 0 && RecurLevel > 0) 
	{
		node._io = true;
	}
	
	var str = '<div class="my_tree_control" >' + this.GetLineHTML(node, nodeId);

	if (this.config.useIcons) 
	{
		if (!node.icon)
		{
			node.icon = (0 == node.pid) ? this.icon.RootNode : ((node.bHaveChild) ? this.icon.folder : this.icon.node);
		}

		if (!node.iconOpen)
		{
			node.iconOpen = (node.bHaveChild) ? this.icon.folderOpen : this.icon.node;
		}

		if (0 == node.pid) 
		{
			node.icon = this.icon.RootNode;
			node.iconOpen = this.icon.RootNode;
		}

		str += '<img id="i' + this.Object + nodeId + '" src="' + ((node._io) ? node.iconOpen : node.icon) + '" alt="" />';

	}
	
	str += node.html;

	
	str += '</div>';
	
	if (node.bHaveChild) 
	{
		str += '<div id="d' + this.Object + nodeId + '" class="clip" style="display:' + ((0 == node.pid || node._io) ? 'block' : 'none') + ';">';
		
		if (RecurLevel > 0) 
		{
			str += this.GetSubLevelHTML(node.id, RecurLevel, OpenLevel);
		}

		str += '</div>';
	}


	return str;


};


/* 绘制节点前面的线和点*/
MyTreeControl.prototype.GetLineHTML = function(node, nodeId) {

	var str = '';
	
	var lTmpNode = node._p;
	
	while(lTmpNode)
	{
		if (0 == lTmpNode.id) 
		{
			break;
		}
		if (lTmpNode.LastSibling) 
		{
			str = '<img src="' +  this.icon.empty  + '" alt="" />' + str;
		}
		else
		{
			str = '<img src="' +  this.icon.line  + '" alt="" />'+ str;
		}
		lTmpNode = lTmpNode._p;
	}
	
	if (node.bHaveChild) 
	{
		str += '<a href="javascript: ' + this.Object + '.OnCollapse(' + nodeId + ');"><img id="j' + this.Object + nodeId + '" src="';

		if (!this.config.useLines)
		{
			str += (node._io) ? this.icon.nlMinus : this.icon.nlPlus;
		}

		else 
		{
			if (nodeId === 1) 
			{
				str += this.icon.plusHead;
			}
			else
			{
				str += ( (node._io) ? ((node.LastSibling && this.config.useLines) ? this.icon.minusBottom : this.icon.minus) : ((node.LastSibling && this.config.useLines) ? this.icon.plusBottom : this.icon.plus ) );
			}
		
		}

		str += '" alt="" /></a>';
	} 
	else 
	{
		str += '<img src="' + ( (this.config.useLines) ? ((node.LastSibling) ? this.icon.joinBottom : this.icon.join ) : this.icon.empty) + '" alt="" />';
	}
	return str;

};

// Checks if a node has any children and if it is the last sibling

MyTreeControl.prototype.SetCS = function(node) 
{
	var lastId;
	for (var n=0; n<this.NodesArray.length; n++) 
	{
		if (this.NodesArray[n].pid == node.id)
		{ 
			node.bHaveChild = true;
		}

		if (this.NodesArray[n].pid == node.pid)
		{
			lastId = this.NodesArray[n].id;
		}
	}
	
	if (lastId==node.id) 
	{
		node.LastSibling = true;
	}
};


// Toggle Open or close

MyTreeControl.prototype.OnCollapse = function(id) 
{
	var lNode = this.GetNode(id);

	this.CollapseNode(!lNode._io, id, lNode.LastSibling, lNode, 1, 1);

	lNode._io = !lNode._io;
};

MyTreeControl.prototype.ToggleCollapse = function(id, RecurLevel, OpenLevel)
{
	if (OpenLevel > RecurLevel)
	{
		RecurLevel = OpenLevel;
	}
	if (RecurLevel > 0)
	{
		var lNode = this.GetNode(id);

		this.CollapseNode(!lNode._io, id, lNode.LastSibling, lNode, RecurLevel, OpenLevel);

		lNode._io = !lNode._io;
	}
};



// If Push and pop is not implemented by the browser

if (!Array.prototype.push) 
{
	Array.prototype.push = function array_push() 
	{
		for(var i=0;i<arguments.length;i++)
		{
			this[this.length]=arguments[i];
		}
		return this.length;
	};
}

if (!Array.prototype.pop) 
{
	var lastElement;
	Array.prototype.pop = function array_pop() 
	{
		lastElement = this[this.length-1];
		this.length = Math.max(this.length-1,0);
		return lastElement;
	};

}