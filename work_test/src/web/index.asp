<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Frameset//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<!- Copyright (c) Go Ahead Software Inc., 1994-2000. All Rights Reserved. ->
<head>
	<meta http-equiv="X-UA-Compatible" content="IE=8" />
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<title></title>
	<link href="/style/doc.css" rel="stylesheet" type="text/css" />
    <script type="text/javascript" src="/js/common_process.js"></script>
</head>
<body>
	<noscript>
		<div style="text-align:center; height:400px; vertical-align:middle">
			<div style="font: bold 20px Arial; color: #F8F8FF; background: maroon; vertical-align: middle">Sorry! Your Browser is not enable the Javsscript Support!</div>
			<div style="font: bold 20px Arial; color: #F8F8FF; background: maroon; vertical-align: middle">对不起，你的浏览器没有打开Javascript脚本支持！</div>
		</div>
	</noscript>
	<div id="activex" style="text-align:center; height:400px; vertical-align:middle;display: none">
		<div style="font: bold 20px Arial; color: #F8F8FF; background: maroon; vertical-align: middle">Sorry! Your Browser is not enable the ActiveX Support!</div>
		<div style="font: bold 20px Arial; color: #F8F8FF; background: maroon; vertical-align: middle">对不起，你的浏览器没有打开ActiveX支持！</div>
	</div>

	<script type="text/javascript">

		if (DisActiveX() == false)
		{
			$("activex").style.display = "block";
		}
		else
		{
			//如果javascript是打开的，系统自动跳转到home.html上。反之则显示上面的提示信息
			window.location="<% 
				ASP_CheckSuperVisor();
				if (AspCheckValue == 1) 
				{
					write("./maintenance_setting.html");
				}
				else
				{
					write("./home.html");
				}
			%>"
		}
	</script>
</body>
</html>
