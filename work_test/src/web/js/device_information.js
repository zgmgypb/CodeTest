/*UTF-8编码*/
var s_OEMXMLDom = null;

function JS_InfoInitiate() 
{
    s_OEMXMLDom = JS_XMLCreateDomFromFile("/tmp/device_information.xml");
}

function JS_InfoGePlatformType()
{
	var lTmpStr;
	var lTmpVar;
	if (s_OEMXMLDom != null)
	{
	    lTmpVar = s_OEMXMLDom.getElementsByTagName("platform_type");
		if (lTmpVar.length)
		{
			if (lTmpVar[0].childNodes.length)
			{
				return lTmpVar[0].childNodes[0].nodeValue;
			}
		}
	}
	return ""
}

function JS_InfoGetModelIPv4()
{
	var lTmpStr;
	var lTmpVar;
	if (s_OEMXMLDom != null)
	{
		lTmpVar = s_OEMXMLDom.getElementsByTagName("ipv4_addr");
		if (lTmpVar.length)
		{
			if (lTmpVar[0].childNodes.length)
			{
				return lTmpVar[0].childNodes[0].nodeValue;
			}
		}
	}
	return ""
}

function JS_InfoGetModelName() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("device_modle");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return "";
}

function JS_InfoGetDeviceName(LanguageName) 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("device_name");
        if (lTmpVar.length) 
        {
            lTmpVar = lTmpVar[0].getElementsByTagName(LanguageName);
            if (lTmpVar.length) 
            {
                if (lTmpVar[0].childNodes.length) 
                {
                    return lTmpVar[0].childNodes[0].nodeValue;
                }
            }
        }
    }
    return ""
}


function JS_InfoGetSN() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("sn");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
}


function JS_InfoGetSoftwareVersion() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("soft_version");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}


function JS_InfoGetHardwareVersion() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("hard_version");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}



function JS_InfoGetSoftRelease() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("soft_release");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}


function JS_InfoGetFPGARelease() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("fpga_release");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}

function JS_InfoGetKernelRelease() {
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("kernel_release");
        if (lTmpVar.length) {
            if (lTmpVar[0].childNodes.length) {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}



function JS_InfoGetManufacture() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
    	lTmpVar = s_OEMXMLDom.getElementsByTagName("manufactury");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}


function JS_InfoGetOffcialWebsite() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("manufactury_web");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}


function JS_InfoHaveManual()
{
	var lTmpStr;
	var lTmpVar;
	if (s_OEMXMLDom != null)
	{
		return parseInt(JS_XMLGetTagValue(s_OEMXMLDom, "have_manual"), 10);
	}
	return 0;
}

function JS_InfoLicenseValid()
{
	var lTmpStr;
	var lTmpVar;
	if (s_OEMXMLDom != null)
	{
		return parseInt(JS_XMLGetTagValue(s_OEMXMLDom, "license_valid"), 10);
	}
	return 0;
}


function JS_InfoGetAuthorizationCode() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("license_code");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}


function JS_InfoGetAuthMode() 
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
    	lTmpVar = s_OEMXMLDom.getElementsByTagName("license_mode");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}


function JS_InfoGetApplicationCode()
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("application_code");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}


function JS_InfoGetAuthorizationTime()
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null) 
    {
        lTmpVar = s_OEMXMLDom.getElementsByTagName("license_time");
        if (lTmpVar.length) 
        {
            if (lTmpVar[0].childNodes.length) 
            {
                return lTmpVar[0].childNodes[0].nodeValue;
            }
        }
    }
    return ""
}


function JS_InfoGetLeftTrailTime()
{
    var lTmpStr;
    var lTmpVar;
    if (s_OEMXMLDom != null)
    {
    	if (JS_InfoLicenseValid() > 0)
    	{
    		return "--------";
    	}
    	else
    	{
			lTmpVar = s_OEMXMLDom.getElementsByTagName("trail_left_time");
			if (lTmpVar.length) 
			{
				if (lTmpVar[0].childNodes.length) 
				{
					return lTmpVar[0].childNodes[0].nodeValue;
				}
			}
    	}
    }
    return ""
}




