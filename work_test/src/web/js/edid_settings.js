/*UTF-8±àÂë*/
var s_EdidXMLDom = null;

function JS_EdidSettingsInitiate() 
{
	s_EdidXMLDom = JS_XMLCreateDomFromFile("/tmp/edid_settings.xml");
}

function JS_DevGetEdidChipNum()
{
    if (s_EdidXMLDom != null) {
        return parseInt(JS_XMLGetTagValue(s_EdidXMLDom, "edid_chip_num"), 10);
    }
    return 0;
}

function JS_DevGetEdidTypeByChn(ChnIndex)
{
    var lTmpVar;

    if (s_EdidXMLDom != null) {
        lTmpVar = s_EdidXMLDom.getElementsByTagName("edid");
        if (lTmpVar.length > 0)
		{
			lTmpVar = lTmpVar[ChnIndex];
			return JS_XMLGetTagValue(lTmpVar, "edid_type");
		}
    }
    return 0;
}
