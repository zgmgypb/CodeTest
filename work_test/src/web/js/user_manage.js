/*UTF-8编码*/
var s_UserXMLDom = null;

function JS_UserSettingInitiate() 
{
	s_UserXMLDom = JS_XMLCreateDomFromFile("/tmp/dynamic_user.xml");
}

function JS_UserGetInfo(Obj)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_UserXMLDom != null)
	{
		lItemInfo.m_UserLimit = parseInt(JS_XMLGetTagValue(s_UserXMLDom, "user_limit"), 10);
		lItemInfo.m_NameLimit = parseInt(JS_XMLGetTagValue(s_UserXMLDom, "name_limit"), 10);
		lItemInfo.m_PasLimit = parseInt(JS_XMLGetTagValue(s_UserXMLDom, "pas_limit"), 10);
	}
	return lItemInfo;
}


function JS_UserGetUserArray()
{
	var lTmpVar;
	if (s_UserXMLDom != null)
	{
		lTmpVar = s_UserXMLDom.getElementsByTagName("user_list");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("user");
		}
	}
	return null;
}


function JS_UserGetUserInfo(Obj)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (Obj != null)
	{
		lItemInfo.m_Name = JS_XMLGetTagValue(Obj, "name");
		lItemInfo.m_Group = JS_XMLGetTagValue(Obj, "group");
	}
	return lItemInfo;
}


















