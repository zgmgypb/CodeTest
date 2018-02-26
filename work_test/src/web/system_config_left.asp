<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Frameset//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <meta http-equiv="X-UA-Compatible" content="IE=8" />
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <title></title>
    <link href="/style/doc.css" rel="stylesheet" type="text/css" />
    <link rel="StyleSheet" href="/components/my_tree_control/my_tree_control.css" type="text/css">
    <style type="text/css">
        body
        {
            background-image: url(/graphics/bg001.gif);
        }
    </style>

    <script type="text/javascript" src="/components/my_tree_control/my_tree_control.js"></script>
    <script type="text/javascript" src="/js/language.js"></script>
    <script type="text/javascript" src="/js/common_process.js"></script>
 	<script type="text/javascript" src="/js/device_setting.js"></script>
    <script type="text/javascript" src="/js/device_information.js"></script>

    <script type="text/javascript">
		var TreeItem;
        var s_ChannelXMLDoc = null;
        JS_LANInitiate();//初始化语言脚本组件
        JS_InfoInitiate();
        JS_DevSettingInitiate();
       
        function OnLoad() 
        {
            var lTmpStr, lTmpVar;
            var i, lTmpValue, lChannelNum, lSubChannelNum, lChannelType;
            var lRootID, lSecondID, lThridID, lFourthID;

			TreeItem = null;
			TreeItem = new MyTreeControl("TreeItem");
			
			lTmpStr = JS_LAN("head");
			lRootID = TreeItem.AddNewNode(0, lTmpStr);

			lTmpStr = "<a href='system_config_right_basic.html' target='right_frame'>" + JS_LAN("basic") + "</a>";
			TreeItem.AddNewNode(lRootID, lTmpStr);

			lTmpStr = "<a href='system_config_right_advance.html' target='right_frame'>" + JS_LAN("adv") + "</a>";
			TreeItem.AddNewNode(lRootID, lTmpStr);
			
			lTmpStr = "<a href='system_config_right_usermanage.html' target='right_frame'>" + JS_LAN("usr") + "</a>";
			
			<%
			    ASP_CheckSuperVisor();
			    if (AspCheckValue == 2) 
			    {
				    write("TreeItem.AddNewNode(lRootID, lTmpStr);");
			    }
			%>
			
			if ((JS_InfoGetSN().substr(0,7) != "TC41A07") && (JS_InfoGetSN().substr(0,7) != "TC41A08")) {
			    lTmpStr = "<a href='system_config_right_snmp.html' target='right_frame'>" + JS_LAN("snmp") + "</a>";
			    TreeItem.AddNewNode(lRootID, lTmpStr);
			}
			lItemInfo = JS_DevGetSyslogInfo()
			if (lItemInfo.m_SyslogEnable == true)
			{
				lTmpStr = "<a href='system_config_right_syslog.html' target='right_frame'>" + JS_LAN("syslog") + "</a>";
				TreeItem.AddNewNode(lRootID, lTmpStr);
			}
						
						
		    lItemInfo = JS_InfoGePlatformType()
			if (lItemInfo == "encoder_card_platform")
			{
				lTmpStr = "<a href='system_config_right_module_files.html' target='right_frame'>" + JS_LAN("module_files") + "</a>";
				TreeItem.AddNewNode(lRootID, lTmpStr);
			}
						
						
			document.getElementById("item_tree").innerHTML = TreeItem.toString();
			TreeItem.ToggleCollapse(lRootID, 4, 2);
			JS_ForcusKeepChangeCallbackSetup($("item_tree"), "red", "white");
		}
    </script>

</head>
<body onload="OnLoad()">
    <table width="310px" height="560px" border="0" align="center" cellpadding="0" cellspacing="0"
        id="Table1">
        <tr>
            <td height="30px" background="/graphics/boxmenu_310.gif" align="center">
                <span style="font-weight: bold"><script type="text/javascript">JS_INLAN("title")</script></span>
            </td>
        </tr>
        <tr>
            <td valign="top" align="left" bgcolor="#F5F9FE">
                <div id="item_tree" style="height: 507px;width:290px; overflow: auto; padding: 10px;">
                </div>
            </td>
        </tr>
        <tr>
            <td height="13" align="center" valign="top" background="/graphics/boxbottom_310.gif">
            </td>
        </tr>
    </table>
</body>
</html>
