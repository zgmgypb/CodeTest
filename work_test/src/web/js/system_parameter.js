/*UTF-8编码，读取子板（功能）参数*/
var s_ParameterXMLDOM;

function JS_ParamInitate()
{
	s_ParameterXMLDOM = JS_XMLCreateDomFromFile("/tmp/system_parameter.xml");
	return s_ParameterXMLDOM;
}

function JS_ParamGetHardVersion()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		return JS_XMLGetTagValue(s_ParameterXMLDOM, "hardversion");
	}
	return false;
}

function JS_ParamGetChannelArray(bInput)
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("channels");
		if (lTmpVar.length)
		{
			if (bInput)
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("input_channels");
				if (lTmpVar.length)
				{
					return lTmpVar[0].getElementsByTagName("channel");
				}
			}
			else
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("output_channels");
				if (lTmpVar.length)
				{
					return lTmpVar[0].getElementsByTagName("channel");
				}
			}
		}
	}
	return null;
}



function JS_ParamGetSubChannelArray(Obj)
{
	var lTmpVar;
	if (Obj != null)
	{
		return Obj.getElementsByTagName("sub_channel");
	}
	return null;
}



function JS_ParamGetInChannelNum()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("channels");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("input_channels");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("channel");
				return lTmpVar.length;
			}
		}
	}
	return 0;
}

function JS_ParamGetInSubChannelNum()
{
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("channels");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("input_channels");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("sub_channel");
				return lTmpVar.length;
			}
		}
	}
	return 0;
}


function JS_ParamGetOutChannelNum()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("channels");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("output_channels");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("channel");
				return lTmpVar.length;
			}
		}
	}
	return 0;
}



function JS_ParamGetOutSubChannelNum()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("channels");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("output_channels");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("sub_channel");
				return lTmpVar.length;
			}
		}
	}
	return 0;
}


function JS_ParamGetInChannelObject(ChnIndex)
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("input_channels");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("channel");
			if (lTmpVar.length > ChnIndex)
			{
				return lTmpVar[ChnIndex];
			}
		}
	}
	return null;
}

function JS_ParamGetOutCHNObj(ChnIndex)
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("output_channels");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("channel");
			if (lTmpVar.length > ChnIndex)
			{
				return lTmpVar[ChnIndex];
			}
		}
	}
	return null;
}

function JS_ParamGetChannelType(Obj)
{
	return JS_XMLGetTagValue(Obj, "type");
}


function JS_ParamGetChannelSubType(Obj)
{
	return JS_XMLGetTagValue(Obj, "sub_type");
}

function JS_ParamGetDemodType(Obj)
{
	return parseInt(JS_XMLGetTagValue(Obj, "demod_type"), 10);
}

function JS_ParamGetChannelHideMark(Obj)
{
	return JS_XMLGetTagValue(Obj, "hide_mark");
}

function JS_ParamGetChannelInfoASI(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_Bitrate = parseInt(JS_XMLGetTagValue(Obj, "bitrate"), 10);
	//lSubInfo.m_MaxBitrate = parseInt(JS_XMLGetTagValue(Obj, "max_bitrate"), 10);
	return lSubInfo;
}


function JS_ParamGetChnInfoETH(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_IPv4Addr = JS_XMLGetTagValue(Obj, "ipv4_address");
	lSubInfo.m_IPv4Mask = JS_XMLGetTagValue(Obj, "ipv4_mask");
	lSubInfo.m_IPv4Gate = JS_XMLGetTagValue(Obj, "ipv4_gate");
	lSubInfo.m_IPMac = JS_XMLGetTagValue(Obj, "ip_mac");
	lSubInfo.m_EthMark = JS_XMLGetTagValue(Obj, "eth_mark");
	
	lSubInfo.m_Bitrate = parseInt(JS_XMLGetTagValue(Obj, "bitrate"), 10);
	lSubInfo.m_MaxBitrate = parseInt(JS_XMLGetTagValue(Obj, "max_bitrate"), 10);
	
	lSubInfo.m_EthNum = parseInt(JS_XMLGetTagValue(Obj, "eth_num"), 10);
	lSubInfo.m_EthSel = parseInt(JS_XMLGetTagValue(Obj, "eth_select"), 10);
	return lSubInfo;
}


function JS_ParamGetChannelInfoMODULATOR(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_ADJFreqNum = parseInt(JS_XMLGetTagValue(Obj, "adjacent_freq_number"), 10);
	lSubInfo.m_CenterLow = parseInt(JS_XMLGetTagValue(Obj, "center_frequence_limits_low"), 10);
	lSubInfo.m_CenterHigh = parseInt(JS_XMLGetTagValue(Obj, "center_frequence_limits_high"), 10);
	lSubInfo.m_SymLow = parseInt(JS_XMLGetTagValue(Obj, "sym_rate_limits_low"), 10);
	lSubInfo.m_SymHigh = parseInt(JS_XMLGetTagValue(Obj, "sym_rate_limits_high"), 10);
	lSubInfo.m_ExAttValid = JS_XMLGetTagValue(Obj, "ex_att_valid_mark");
	lSubInfo.m_ExAttStepping = JS_XMLGetTagValue(Obj, "ex_att_stepping");
	lSubInfo.m_ExAttLevel = JS_XMLGetTagValue(Obj, "ex_att_level");
	lSubInfo.m_ExAttLevelMax = JS_XMLGetTagValue(Obj, "ex_att_level_max");
	lSubInfo.m_GainLevelMax = JS_XMLGetTagValue(Obj, "gain_level_max");
	lSubInfo.m_GainStepping = JS_XMLGetTagValue(Obj, "gain_stepping");
	lSubInfo.m_DPDMark = JS_XMLGetTagValue(Obj, "dpd_mark");
	lSubInfo.m_ALCMark = JS_XMLGetTagValue(Obj, "alc_mark");
	lSubInfo.m_FreqAdj = JS_XMLGetTagValue(Obj, "freq_adj");
	lSubInfo.m_FreqAdjDisable = JS_XMLGetTagValue(Obj, "freq_adj_disable");
	return lSubInfo;
}
function JS_ParamGetChannelInfoTUNER(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_FreqLimitsLow = JS_XMLGetTagValue(Obj, "freqLimitsLow");
	lSubInfo.m_FreqLimitsHigh = JS_XMLGetTagValue(Obj, "freqLimitshigh");
	lSubInfo.m_SymRateLimitsLow = JS_XMLGetTagValue(Obj, "sym_rate_limits_low");
	lSubInfo.m_SymRateLimitsHigh = JS_XMLGetTagValue(Obj, "sym_rate_limits_high");
	return lSubInfo;
}


function JS_ParamGetNodeBitrateLimits(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	/*输入参数*/
	lTmpVar = Obj.getElementsByTagName("bitrate_limit")[0];
	if (lTmpVar != null)
	{
		// 		lSubInfo.m_MaxBitrate = parseInt(JS_XMLGetTagValue(lTmpVar, "max_bitrate"), 10);
		// 		lSubInfo.m_MinBitrate = parseInt(JS_XMLGetTagValue(lTmpVar, "min_bitrate"), 10);
		// 		lSubInfo.m_BitrateType = JS_XMLGetTagValue(lTmpVar, "bitrate_type");
		lSubInfo.m_MaxBitrate = 1000;
		lSubInfo.m_MinBitrate = 1000;
		lSubInfo.m_BitrateType = "OFF";
	}
	return lSubInfo;
}


function JS_ParamGetSubChannelNum(Obj)
{
	var lTmpVar;
	if (Obj != null)
	{
		lTmpVar = Obj.getElementsByTagName("sub_channel");
		return lTmpVar.length;
	}
	return 0;
}


function JS_ParamGetSubChnObj(Obj, SubIndex)
{
	var lTmpVar;
	if (Obj != null)
	{
		lTmpVar = Obj.getElementsByTagName("sub_channel");
		if (lTmpVar.length > SubIndex)
		{
			return lTmpVar[SubIndex];
		}
	}
	return null;
}

function JS_ParamGetSubEncoderEntryObj(Obj, SubIndex) {
    var lTmpVar;
    if (Obj != null) {
        lTmpVar = Obj.getElementsByTagName("encoder_info_entry");
        if (lTmpVar.length > SubIndex) {
            return lTmpVar[SubIndex];
        }
    }
    return null;
}

function JS_ParamGetSubChannelInfo(Obj)
{
    var lTmpVar;
    var lSubInfo = new Object();
    lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
    lSubInfo.m_Active = JS_XMLGetTagValue(Obj, "active_mark");
    return lSubInfo;
}

function JS_ParamGetSubChannelInfoMonitor(Obj)
{
    var lTmpVar;
    var lSubInfo = new Object();
    lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
    lSubInfo.m_Active = JS_XMLGetTagValue(Obj, "active_mark");
    lSubInfo.m_PIDStatMark = JS_XMLGetTagValue(Obj, "pid_stat_mark");
    return lSubInfo;
}


function JS_ParamGetSubChannelInfoASI(Obj) 
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
	lSubInfo.m_Active = JS_XMLGetTagValue(Obj, "active_mark");
	lSubInfo.m_PIDStatMark = JS_XMLGetTagValue(Obj, "pid_stat_mark");
	lSubInfo.m_Bitrate = parseInt(JS_XMLGetTagValue(Obj, "bitrate"), 10);
	lSubInfo.m_Disabled = parseInt(JS_XMLGetTagValue(Obj, "disabled"), 10);
	
	lSubInfo.BitSetupDisable = JS_XMLGetTagValue(Obj, "output_bitrate_setup_disable");
	return lSubInfo;
}

function JS_ParamGetSubChannelInfoE3DS3(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
	lSubInfo.m_E3DS3 = JS_XMLGetTagValue(Obj, "e3ds3_mark");
	lSubInfo.m_BitOrder = JS_XMLGetTagValue(Obj, "bit_order");
	lSubInfo.m_FrameFrom = JS_XMLGetTagValue(Obj, "fream_from");
	lSubInfo.m_PacketLength = parseInt(JS_XMLGetTagValue(Obj, "packet_length"), 10);
	lSubInfo.m_Scramble = JS_XMLGetTagValue(Obj, "scramble_changer");
	lSubInfo.m_RSCoding = JS_XMLGetTagValue(Obj, "rs_code");
	lSubInfo.m_InterweaveCoding = JS_XMLGetTagValue(Obj, "interweave_code");
	lSubInfo.m_CodeRateRecover = JS_XMLGetTagValue(Obj, "code_rate_recover");
	lSubInfo.m_Disabled = parseInt(JS_XMLGetTagValue(Obj, "disabled"), 10);
	return lSubInfo;
}

function JS_ParamGetSubChannelInfoTUNER(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
	lSubInfo.m_InputFreq = parseInt(JS_XMLGetTagValue(Obj, "in_freq"), 10);
	lSubInfo.m_LocalFreq = parseInt(JS_XMLGetTagValue(Obj, "local_freq"), 10);
	lSubInfo.m_SymbolRate = parseInt(JS_XMLGetTagValue(Obj, "sym_rate"), 10);
	lSubInfo.m_PolarMethod = (JS_XMLGetTagValue(Obj, "polar_method"));
	lSubInfo.m_22kSwitch = (JS_XMLGetTagValue(Obj, "switch_22k"));
	lSubInfo.m_Specinv = (JS_XMLGetTagValue(Obj, "spec_inv"));
	lSubInfo.m_Modulation = (JS_XMLGetTagValue(Obj, "modulation_mode"));
	lSubInfo.m_Reqtype = JS_XMLGetTagValue(Obj, "req_type");
	lSubInfo.m_Disabled = parseInt(JS_XMLGetTagValue(Obj, "disabled"), 10);
	return lSubInfo;
}

function JS_ParamGetChannelInfoEncoderNewLimits(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_VidBitrateLimitLow = JS_XMLGetTagValue(Obj, "VidBitrateLimitLow");
	lSubInfo.m_VidBitrateLimitHigh = JS_XMLGetTagValue(Obj, "VidBitrateLimitHigh");
	lSubInfo.m_ProgBitrateLimitHigh = JS_XMLGetTagValue(Obj, "ProgBitrateLimitHigh");
	lSubInfo.m_GopLimitLow = JS_XMLGetTagValue(Obj, "GopLimitLow");
	lSubInfo.m_GopLimitHigh = JS_XMLGetTagValue(Obj, "GopLimitHigh");
	lSubInfo.m_VolLimitLow = JS_XMLGetTagValue(Obj, "VolLimitLow");
	lSubInfo.m_VolLimitHigh = JS_XMLGetTagValue(Obj, "VolLimitHigh");
	lSubInfo.m_ProgNumberLimitLow = JS_XMLGetTagValue(Obj, "ProgNumberLimitLow");
	lSubInfo.m_ProgNumberLimitHigh = JS_XMLGetTagValue(Obj, "ProgNumberLimitHigh");
	lSubInfo.m_ProgNameLimitHigh = JS_XMLGetTagValue(Obj, "ProgNameLimitHigh");
	lSubInfo.m_PidLimitLow = JS_XMLGetTagValue(Obj, "PidLimitLow");
	lSubInfo.m_PidLimitHigh = JS_XMLGetTagValue(Obj, "PidLimitHigh");
	return lSubInfo;
}

function JS_ParamGetSubChannelInfoEncoderNew(Obj) {
	var lTmpVar;
	var lSubInfo = new Object();
	
	lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
	lSubInfo.m_ViMode = JS_XMLGetTagValue(Obj, "vi_mode");
	lSubInfo.m_VoMode = JS_XMLGetTagValue(Obj, "vo_mode");
    lSubInfo.m_BrMode = JS_XMLGetTagValue(Obj, "br_mode");
    lSubInfo.m_EncMode = JS_XMLGetTagValue(Obj, "enc_mode");
    lSubInfo.m_Profile = JS_XMLGetTagValue(Obj, "profile");
    lSubInfo.m_Bitrate = JS_XMLGetTagValue(Obj, "bitrate");
    lSubInfo.m_ProgBitrate = JS_XMLGetTagValue(Obj, "prog_bitrate");
    lSubInfo.m_Gop = JS_XMLGetTagValue(Obj, "gop");
    lSubInfo.m_AudEncMode = JS_XMLGetTagValue(Obj, "aud_enc_mode");
    lSubInfo.m_AudBitrate = JS_XMLGetTagValue(Obj, "aud_bitrate");
    lSubInfo.m_AudSample = JS_XMLGetTagValue(Obj, "aud_sample");
    lSubInfo.m_Volume = parseInt(JS_XMLGetTagValue(Obj, "volume"), 10);
    lSubInfo.m_ProgName = JS_XMLGetTagValue(Obj, "prog_name");
    lSubInfo.m_ProgNumber = parseInt(JS_XMLGetTagValue(Obj, "prog_number"), 10);
    lSubInfo.m_VidPid = parseInt(JS_XMLGetTagValue(Obj, "vid_pid"), 10);
    lSubInfo.m_AudPid = parseInt(JS_XMLGetTagValue(Obj, "aud_pid"), 10);
    lSubInfo.m_PcrPid = parseInt(JS_XMLGetTagValue(Obj, "pcr_pid"), 10);
    lSubInfo.m_PmtPid = parseInt(JS_XMLGetTagValue(Obj, "pmt_pid"), 10);
    lSubInfo.m_Protocol = JS_XMLGetTagValue(Obj, "protocol");
	lSubInfo.m_IPV4Des = JS_XMLGetTagValue(Obj, "ipv4_des");
	lSubInfo.m_IPV4DesPort = JS_XMLGetTagValue(Obj, "ipv4_des_port");
    lSubInfo.m_Active = JS_XMLGetTagValue(Obj, "active");
	return lSubInfo;
}

//#ifdef GM8358Q
function JS_ParamGetSubChannelInfoENCODER(Obj) {
    var lSUBInfo = new Object();

    lSUBInfo.m_EncodeInputIndex = JS_XMLGetTagValue(Obj, "encode_input_index");
    lSUBInfo.m_WorkMod = JS_XMLGetTagValue(Obj, "work_mod");
    lSUBInfo.m_VideoFormat = JS_XMLGetTagValue(Obj, "video_format");
    lSUBInfo.m_FrameRate = JS_XMLGetTagValue(Obj, "frame_rate");
    lSUBInfo.m_Resolution = JS_XMLGetTagValue(Obj, "resolution");
    lSUBInfo.m_OutBitRate = JS_XMLGetTagValue(Obj, "out_bit_rate_mode");
    lSUBInfo.m_VideoEncodeMode = JS_XMLGetTagValue(Obj, "video_encode_mode");
    lSUBInfo.m_VideoProfile = JS_XMLGetTagValue(Obj, "video_profile");
    lSUBInfo.m_VideoAspect = JS_XMLGetTagValue(Obj, "video_aspect");
    lSUBInfo.m_VideoBitRate = parseInt(JS_XMLGetTagValue(Obj, "video_bit_rate") , 10);
    lSUBInfo.m_AudioEncodeMode = JS_XMLGetTagValue(Obj, "audio_encode_mode");
    lSUBInfo.m_AudioBitRate = JS_XMLGetTagValue(Obj, "audio_bit_rate");
    lSUBInfo.m_ImageHorizontalOffset = parseInt(JS_XMLGetTagValue(Obj, "image_horizontal_offset") , 10);
    lSUBInfo.m_VideoPid = parseInt(JS_XMLGetTagValue(Obj, "video_pid") , 10);
    lSUBInfo.m_AudioPid = parseInt(JS_XMLGetTagValue(Obj, "audio_pid") , 10);
    lSUBInfo.m_PcrPid = parseInt(JS_XMLGetTagValue(Obj, "pcr_pid") , 10);
    lSUBInfo.m_PmtPid = parseInt(JS_XMLGetTagValue(Obj, "pmt_pid") , 10);
    lSUBInfo.m_Brightness = parseInt(JS_XMLGetTagValue(Obj, "brightness") , 10);
    lSUBInfo.m_Contrast = parseInt(JS_XMLGetTagValue(Obj, "contrast") , 10);
    lSUBInfo.m_Saturation = parseInt(JS_XMLGetTagValue(Obj, "saturation") , 10);
    lSUBInfo.m_Hue = parseInt(JS_XMLGetTagValue(Obj, "hue"), 10);
    lSUBInfo.m_Volume = parseInt(JS_XMLGetTagValue(Obj, "volume"), 10);
    lSUBInfo.m_AudioEmbChannel = JS_XMLGetTagValue(Obj, "audio_emb_channel");
    lSUBInfo.m_AudioSampleRate = JS_XMLGetTagValue(Obj, "audio_sample_rate");
    lSUBInfo.m_PidEqualSwitch = JS_XMLGetTagValue(Obj, "pid_equal_switch");

    lSUBInfo.m_VixsActive = JS_XMLGetTagValue(Obj, "vixs_active");
    
    return lSUBInfo;
}
//#endif 

//#ifdef GM8398Q
function JS_ParamGetSubChannelInfoENCODER(Obj) {
    var lSUBInfo = new Object();

    lSUBInfo.m_EncodeInputIndex = JS_XMLGetTagValue(Obj, "encode_input_index");
    lSUBInfo.m_WorkMod = JS_XMLGetTagValue(Obj, "work_mod");
    lSUBInfo.m_VideoFormat = JS_XMLGetTagValue(Obj, "video_format");
    lSUBInfo.m_FrameRate = JS_XMLGetTagValue(Obj, "frame_rate");
    lSUBInfo.m_Resolution = JS_XMLGetTagValue(Obj, "resolution");
    lSUBInfo.m_OutBitRate = JS_XMLGetTagValue(Obj, "out_bit_rate_mode");
    lSUBInfo.m_VideoEncodeMode = JS_XMLGetTagValue(Obj, "video_encode_mode");
    lSUBInfo.m_VideoProfile = JS_XMLGetTagValue(Obj, "video_profile");
    lSUBInfo.m_VideoAspect = JS_XMLGetTagValue(Obj, "video_aspect");
    lSUBInfo.m_VideoBitRate = parseInt(JS_XMLGetTagValue(Obj, "video_bit_rate"), 10);
    lSUBInfo.m_AudioEncodeMode = JS_XMLGetTagValue(Obj, "audio_encode_mode");
    lSUBInfo.m_AudioBitRate = JS_XMLGetTagValue(Obj, "audio_bit_rate");
    lSUBInfo.m_ImageHorizontalOffset = parseInt(JS_XMLGetTagValue(Obj, "image_horizontal_offset"), 10);
    lSUBInfo.m_VideoPid = parseInt(JS_XMLGetTagValue(Obj, "video_pid"), 10);
    lSUBInfo.m_AudioPid = parseInt(JS_XMLGetTagValue(Obj, "audio_pid"), 10);
    lSUBInfo.m_PcrPid = parseInt(JS_XMLGetTagValue(Obj, "pcr_pid"), 10);
    lSUBInfo.m_PmtPid = parseInt(JS_XMLGetTagValue(Obj, "pmt_pid"), 10);
    lSUBInfo.m_Brightness = parseInt(JS_XMLGetTagValue(Obj, "brightness"), 10);
    lSUBInfo.m_Contrast = parseInt(JS_XMLGetTagValue(Obj, "contrast"), 10);
    lSUBInfo.m_Saturation = parseInt(JS_XMLGetTagValue(Obj, "saturation"), 10);
    lSUBInfo.m_Hue = parseInt(JS_XMLGetTagValue(Obj, "hue"), 10);
    lSUBInfo.m_Volume = parseInt(JS_XMLGetTagValue(Obj, "volume"), 10);
    lSUBInfo.m_AudioEmbChannel = JS_XMLGetTagValue(Obj, "audio_emb_channel");
    lSUBInfo.m_AudioSampleRate = JS_XMLGetTagValue(Obj, "audio_sample_rate");
    lSUBInfo.m_PidEqualSwitch = JS_XMLGetTagValue(Obj, "pid_equal_switch");

    lSUBInfo.m_Dxt8243Active = JS_XMLGetTagValue(Obj, "dxt8243_active");

    return lSUBInfo;
}
//#endif 

function JS_ParamGetSubChnETHIn(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
	lSubInfo.m_Active = JS_XMLGetTagValue(Obj, "active_mark");
	lSubInfo.m_Bitrate = parseInt(JS_XMLGetTagValue(Obj, "bitrate"), 10);
	lSubInfo.m_MaxBitrate = parseInt(JS_XMLGetTagValue(Obj, "max_bitrate"), 10);
	lSubInfo.m_IPV4Des = JS_XMLGetTagValue(Obj, "ipv4_des");
	lSubInfo.m_IPV4DesPort = JS_XMLGetTagValue(Obj, "ipv4_des_port");
	lSubInfo.m_IPV4SRC = JS_XMLGetTagValue(Obj, "ipv4_src");
	lSubInfo.m_IPV4SRCPort = JS_XMLGetTagValue(Obj, "ipv4_src_port");
	lSubInfo.m_SRCLimit = JS_XMLGetTagValue(Obj, "ipv4_src_limit");
	lSubInfo.m_Protocol = JS_XMLGetTagValue(Obj, "protocol");
	lSubInfo.m_Disabled = parseInt(JS_XMLGetTagValue(Obj, "disabled"), 10);
	return lSubInfo;
}

function JS_ParamGetSubChnETHOut(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
	lSubInfo.m_Active = JS_XMLGetTagValue(Obj, "active_mark");
	lSubInfo.m_Bitrate = parseInt(JS_XMLGetTagValue(Obj, "bitrate"), 10);
	lSubInfo.m_MaxBitrate = parseInt(JS_XMLGetTagValue(Obj, "max_bitrate"), 10);
	lSubInfo.m_IPV4Des = JS_XMLGetTagValue(Obj, "ipv4_des");
	lSubInfo.m_IPV4DesPort = JS_XMLGetTagValue(Obj, "ipv4_des_port");
	lSubInfo.m_Protocol = JS_XMLGetTagValue(Obj, "protocol");
//	lSubInfo.m_ChnSel = JS_XMLGetTagValue(Obj, "chn_sel");
	lSubInfo.m_Disabled = parseInt(JS_XMLGetTagValue(Obj, "disabled"), 10);
	return lSubInfo;
}

function JS_ParamGetSubChannelInfoMODULATOR(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(Obj, "correspond_ts_index"), 10);
	lSubInfo.m_ITU = JS_XMLGetTagValue(Obj, "itu_coding");
	lSubInfo.m_Band = JS_XMLGetTagValue(Obj, "analog_band");
	lSubInfo.m_Freq = parseInt(JS_XMLGetTagValue(Obj, "center_frequence"), 10);
	lSubInfo.m_Mode = JS_XMLGetTagValue(Obj, "modulation_mode");
	lSubInfo.m_Sym = parseInt(JS_XMLGetTagValue(Obj, "symbol_rate"), 10);
	lSubInfo.m_INV = JS_XMLGetTagValue(Obj, "spectrum_invert");
	lSubInfo.m_Modulation = JS_XMLGetTagValue(Obj, "modulation_mark");
	lSubInfo.m_Gain = JS_XMLGetTagValue(Obj, "gain_level");
	lSubInfo.m_RF = JS_XMLGetTagValue(Obj, "active_mark");
	lSubInfo.m_FecEncode = JS_XMLGetTagValue(Obj, "fec_encode");
	lSubInfo.m_Carrier = JS_XMLGetTagValue(Obj, "carrier");
	lSubInfo.m_PNMode = JS_XMLGetTagValue(Obj, "pn_mode");
	lSubInfo.m_CodeRate = JS_XMLGetTagValue(Obj, "code_rate");
	lSubInfo.m_TI = JS_XMLGetTagValue(Obj, "time_interleaver");
	lSubInfo.m_DP = JS_XMLGetTagValue(Obj, "double_pilot");
	lSubInfo.m_Disabled = parseInt(JS_XMLGetTagValue(Obj, "disabled"), 10);
	return lSubInfo;
}

function JS_ParamGetSubChannelInfoDirecRoute(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lTmpVar = Obj.getElementsByTagName("route_mode")[0];
	if (lTmpVar != null)
	{
		lSubInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(lTmpVar, "ts_index"), 10);
		lSubInfo.m_RoutMark = JS_XMLGetTagValue(lTmpVar, "route_mark");
	}
	return lSubInfo;
}

function JS_ParamGetPIDMapSetting()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("pid_map");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_MaxItemNum = JS_XMLGetTagValue(lTmpVar, "map_item_max_number");
		}
	}
	return lItemInfo;
}

function JS_ParamGetPIDMapArray()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("pid_map");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("map_item");
		}
	}
	return 0;
}

function JS_ParamGetPIDMapInfo(ItemNode)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (ItemNode != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(ItemNode, "pidmap_ids");
		lItemInfo.m_InTsIndex = parseInt(JS_XMLGetTagValue(ItemNode, "in_ts_index"), 10);
		lItemInfo.m_InPID = parseInt(JS_XMLGetTagValue(ItemNode, "in_pid"), 10);
		lItemInfo.m_OutTsIndex = parseInt(JS_XMLGetTagValue(ItemNode, "out_ts_index"), 10);
		lItemInfo.m_OutPID = parseInt(JS_XMLGetTagValue(ItemNode, "out_pid"), 10);
		lItemInfo.m_Active = JS_XMLGetTagValue(ItemNode, "active_mark");
	}
	return lItemInfo;
}

function JS_ParamGetInserterInfo()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("manual_ts_inserter");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("information");
			if (lTmpVar.length)
			{
				lItemInfo.m_MaxItemNum = parseInt(JS_XMLGetTagValue(lTmpVar[0], "max_num"), 10);
				lItemInfo.m_MaxTotalBitrate = parseInt(JS_XMLGetTagValue(lTmpVar[0], "max_bps"), 10);
				lItemInfo.m_MaxTotalSize = parseInt(JS_XMLGetTagValue(lTmpVar[0], "max_size"), 10);
				lItemInfo.m_MaxNameBytes = parseInt(JS_XMLGetTagValue(lTmpVar[0], "name_max_bytes"), 10);
				lItemInfo.m_MaxNameBytes = parseInt(JS_XMLGetTagValue(lTmpVar[0], "name_max_bytes"), 10);
			}
		}
	}
	return lItemInfo;
}


function JS_ParamGetInserterArray()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("manual_ts_inserter");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("manu_item");
		}
	}
	return 0;
}


function JS_ParamGetInserterItem(ItemNode)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(ItemNode, "ids");
		lItemInfo.m_Name = JS_XMLGetTagValue(ItemNode, "name");
		lItemInfo.m_Size = parseInt(JS_XMLGetTagValue(ItemNode, "size"), 10);
		lItemInfo.m_Bitrate = parseInt(JS_XMLGetTagValue(ItemNode, "bitrate"), 10);
		lItemInfo.m_OutTsIndex = parseInt(JS_XMLGetTagValue(ItemNode, "out_ts_index"), 10);
		lItemInfo.m_Active = JS_XMLGetTagValue(ItemNode, "active_mark");
	}
	return lItemInfo;
}


function JS_ParamGetNITInformation()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("dvb_network_information");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_NetID = JS_XMLGetTagValue(lTmpVar, "network_id");
			lItemInfo.m_NetName = JS_XMLGetTagValue(lTmpVar, "network_name");
			lItemInfo.m_NitVersion = JS_XMLGetTagValue(lTmpVar, "version");
			lItemInfo.m_NitMark = JS_XMLGetTagValue(lTmpVar, "nit_global_mark");
			lItemInfo.m_MaxTsInfoNum = parseInt(JS_XMLGetTagValue(lTmpVar, "nit_ts_max_num"), 10);
			lItemInfo.m_MaxNameBytes = parseInt(JS_XMLGetTagValue(lTmpVar, "name_max_bytes"), 10);
			if (isNaN(lItemInfo.m_MaxTsInfoNum))
			{
				lItemInfo.m_MaxTsInfoNum = 0;
			}
		}
	}
	return lItemInfo;
}


function JS_ParamGetNITTSArray()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("dvb_network_information");
		if (lTmpVar.length)
		{
			lTmpVar = s_ParameterXMLDOM.getElementsByTagName("nit_ts_inforamtions");
			if (lTmpVar.length)
			{
				return lTmpVar[0].getElementsByTagName("ts_info");
			}
		}
	}
	return null;
}

function JS_ParamGetNITTSNodeByIDs(IDs)
{
	var lTmpVar;
	var i;
	lTmpVar = JS_ParamGetNITTSArray();
	if (lTmpVar != null)
	{
		var lTsInfo;
		for (i = 0; i < lTmpVar.length; i++)
		{
			lTsInfo = JS_ParamGetNITTSInfo(lTmpVar[i]);
			if (lTsInfo.m_IDs == IDs)
			{
				lTsInfo = null;
				return lTmpVar[i];
			}
			lTsInfo = null;
		}
	}
	return null;
}


function JS_ParamGetNITTSInfo(ObjectNode)
{
	var lItemInfo = new Object();

	lItemInfo.m_IDs = JS_XMLGetTagValue(ObjectNode, "nit_ts_ids");
	lItemInfo.m_TSID = JS_XMLGetTagValue(ObjectNode, "ts_id");
	lItemInfo.m_ONID = JS_XMLGetTagValue(ObjectNode, "on_id");
	lItemInfo.m_DeliveryType = JS_XMLGetTagValue(ObjectNode, "delivery_type");

	return lItemInfo;
}



function JS_ParamGetNITCableDeliveryInfo(ObjectNode)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = ObjectNode.getElementsByTagName("delivery_info");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_Freq = parseInt(JS_XMLGetTagValue(lTmpVar, "freq"), 10);
			lItemInfo.m_Mode = JS_XMLGetTagValue(lTmpVar, "mode");
			lItemInfo.m_SR = parseInt(JS_XMLGetTagValue(lTmpVar, "symbol_rate"), 10);
			lItemInfo.m_FECOut = parseInt(JS_XMLGetTagValue(lTmpVar, "fec_outer"), 10);
			lItemInfo.m_FECIn = parseInt(JS_XMLGetTagValue(lTmpVar, "fec_inner"), 10);
		}
	}
	return lItemInfo;
}




function JS_ParamGetNITDescriptorArray()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("dvb_network_information");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("network_descriptors");
			if (lTmpVar.length)
			{
				return lTmpVar[0].getElementsByTagName("descriptor");
			}
		}
	}
	return null;
}


function JS_ParamGetNITTSDescriptorArray(Obj)
{
	var lTmpVar;
	if (Obj != null)
	{
		lTmpVar = Obj.getElementsByTagName("nit_ts_descriptors");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("descriptor");
		}
	}
	return null;
}



function JS_ParamGetTsDescrArray(Obj)
{
	var lTmpVar;
	if (Obj != null)
	{
		lTmpVar = Obj.getElementsByTagName("cat_descriptors");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("descriptor");
		}
	}
	return null;
}


function JS_ParamGetServPMTDescArray(Obj, bInput)
{
	var lTmpVar;
	if (Obj != null)
	{
		if (bInput)
		{
			lTmpVar = Obj.getElementsByTagName("in_service_info");
		}
		else
		{
			lTmpVar = Obj.getElementsByTagName("out_service_info");
		}
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("pmt_descriptors");
			if (lTmpVar.length)
			{
				return lTmpVar[0].getElementsByTagName("descriptor");
			}
		}
	}
	return null;
}

function JS_ParamGetServSDTDescArray(Obj, bInput)
{
	var lTmpVar;
	if (Obj != null)
	{
		if (bInput)
		{
			lTmpVar = Obj.getElementsByTagName("in_service_info");
		}
		else
		{
			lTmpVar = Obj.getElementsByTagName("out_service_info");
		}
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("sdt_descriptors");
			if (lTmpVar.length)
			{
				return lTmpVar[0].getElementsByTagName("descriptor");
			}
		}
	}
	return null;
}


function JS_ParamGetESPMTDescArray(Obj, bInput)
{
	var lTmpVar;
	if (Obj != null)
	{
		if (bInput)
		{
			lTmpVar = Obj.getElementsByTagName("in_es_info");
		}
		else
		{
			lTmpVar = Obj.getElementsByTagName("out_es_info");
		}
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("es_pmt_descriptors");
			if (lTmpVar.length)
			{
				return lTmpVar[0].getElementsByTagName("descriptor");
			}
		}
	}
	return null;
}


function JS_ParamGetDescriptorInfo(ObjectNode)
{
	var lItemInfo = new Object();
	if (ObjectNode != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(ObjectNode, "desc_ids");
		lItemInfo.m_Data = JS_XMLGetTagValue(ObjectNode, "data");
		lItemInfo.m_Active = JS_XMLGetTagValue(ObjectNode, "out_mark");
	}
	return lItemInfo;
}


function JS_ParamGetTsCAArray(Obj, bInput)
{
	var lTmpVar;
	if (Obj != null)
	{
		if (bInput)
		{
			lTmpVar = JS_XMLGetTagNameFirstObj(Obj, "mux_cas");
			if (lTmpVar != null) 
			{
			    return lTmpVar.getElementsByTagName("mux_ca");
			}
		}
		else
		{
		    lTmpVar = JS_XMLGetTagNameFirstObj(Obj, "scs_cas");
		    if (lTmpVar != null)
		    {
		        return lTmpVar.getElementsByTagName("scs_ca_info");
		    }
		}
	}
	return null;
}

function JS_ParamGetTsSCSCAIDs(Obj)
{
	var lTmpVar;
	if (Obj != null)
	{	
		lTmpVar = JS_XMLGetTagNameFirstObj(Obj, "scs_cas");
		if (lTmpVar != null)
		{
		    return JS_XMLGetTagValue(lTmpVar, "scs_ca_ids");
		}
}
	return null;
}


function JS_ParamGetInTsRemuxCAGroup(Obj)
{
	var lTmpVar;
	var lCAArray, lCAGroup;
	var ca, group;
	var lCAInfo;
	lCAGroup = new Array();
	if (Obj != null)
	{
		lTmpVar = JS_XMLGetTagNameFirstObj(Obj, "mux_cas");
		if (lTmpVar != null)
		{
		    lCAArray = lTmpVar.getElementsByTagName("mux_ca");

		    /*每个复用CA都会对应通道个数的项目，将其按CASID组合成一个一个的组，然后按OutTsIndex排序*/
		    for (ca = 0; ca < lCAArray.length; ca++)
		    {
		        lCAInfo = JS_ParamGetMuxCAInfo(lCAArray[ca]);

		        for (group = 0; group < lCAGroup.length; group++)
		        {
		            if (lCAGroup[group].CASID == lCAInfo.m_CASID)
		            {
		                lCAGroup[group].m_InfoArray[lCAGroup[group].m_InfoArray.length] = lCAInfo;
		                break;
		            }
		        }

		        if (group == lCAGroup.length)
		        {
		            lCAGroup[group] = new Object();
		            lCAGroup[group].CASID = lCAInfo.m_CASID;
		            lCAGroup[group].m_InfoArray = new Array();
		            lCAGroup[group].m_InfoArray[lCAGroup[group].m_InfoArray.length] = lCAInfo;
		        }
		    }
		}
	}
	return lCAGroup;
}


function JS_ParamGetOutTsRemuxCAGroup(OutTsIndex)
{
	var ts, ca, group;
	var lTsArray, lTsObj;
	var lCAArray, lCAObj, lCAInfo;
	var lCAGroup;

	lCAGroup = new Array();
	lTsArray = JS_ParamGetTsArray(true);
	if (lTsArray)
	{
		for (ts = 0; ts < lTsArray.length; ts++)
		{
			lTsObj = lTsArray[ts];
			lCAArray = JS_ParamGetTsCAArray(lTsObj, true);
			if (lCAArray != null) 
			{
			    for (ca = 0; ca < lCAArray.length; ca++)
			    {
			        lCAInfo = JS_ParamGetMuxCAInfo(lCAArray[ca]);
			        if (lCAInfo.m_OutTsIndex == OutTsIndex)
			        {
			            group = lCAGroup.length;
			            lCAGroup[group] = new Object();
			            lCAGroup[group].m_InTsIndex = ts;
			            lCAGroup[group].m_CAInfo = lCAInfo;
			        }
			    }
			}
		}
	}
	return lCAGroup;
}


function JS_ParamGetServCAArray(Obj, bInput)
{
	var lTmpVar;
	if (Obj != null)
	{
		if (bInput)
		{
			lTmpVar = Obj.getElementsByTagName("in_service_info")[0];
			lTmpVar = lTmpVar.getElementsByTagName("mux_cas")[0];
			return lTmpVar.getElementsByTagName("mux_ca");
		}
		else
		{
			lTmpVar = Obj.getElementsByTagName("out_service_info")[0];
			lTmpVar = lTmpVar.getElementsByTagName("scs_cas")[0];
			return lTmpVar.getElementsByTagName("scs_ca_info");
		}
	}
	return null;
}


function JS_ParamGetServSCSCAIDs(Obj)
{
	var lTmpVar;
	if (Obj != null)
	{
		lTmpVar = Obj.getElementsByTagName("out_service_info")[0];
		lTmpVar = lTmpVar.getElementsByTagName("scs_cas")[0];
		return JS_XMLGetTagValue(lTmpVar, "scs_ca_ids");
	}
	return 0;
}



function JS_ParamGetEsCAArray(Obj, bInput)
{
	var lTmpVar;
	if (Obj != null)
	{
		if (bInput)
		{
			lTmpVar = Obj.getElementsByTagName("in_es_info")[0];
			return lTmpVar.getElementsByTagName("mux_ca");
		}
		else
		{
			lTmpVar = Obj.getElementsByTagName("out_es_info")[0];
			lTmpVar = lTmpVar.getElementsByTagName("scs_cas")[0];
			return Obj.getElementsByTagName("scs_ca_info");
		}
	}
	return null;
}

function JS_ParamGetEsSCSCAIDs(Obj)
{
	var lTmpVar;
	if (Obj != null)
	{
		lTmpVar = Obj.getElementsByTagName("out_es_info")[0];
		lTmpVar = lTmpVar.getElementsByTagName("scs_cas")[0];
		return lTmpVar.getElementsByTagName("scs_ca_ids");
	}
	return null;
}


function JS_ParamGetMuxCAInfo(Obj)
{
	var lItemInfo = new Object();
	var lTmpVar, i;
	if (Obj != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(Obj, "mux_ca_ids");
		lTmpVar = Obj.getElementsByTagName("in_ca_info");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_CASID = JS_XMLGetTagValue(lTmpVar, "cas_id");
			lItemInfo.m_CASID = JS_StrLeftPading(parseInt(lItemInfo.m_CASID, 10).toString(16), 4);
			lItemInfo.m_CAPID = JS_XMLGetTagValue(lTmpVar, "ca_pid");
		}

		lTmpVar = Obj.getElementsByTagName("out_ca_info");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_OutPID = JS_XMLGetTagValue(lTmpVar, "ca_pid");
			lItemInfo.m_OutTsIndex = parseInt(JS_XMLGetTagValue(lTmpVar, "out_ts_index"), 10);
			lItemInfo.m_OutMark = JS_XMLGetTagValue(lTmpVar, "out_mark");
			lItemInfo.m_PSM = JS_XMLGetTagValue(lTmpVar, "ps_m");
		}

		lTmpVar = Obj.getElementsByTagName("ca_private_info");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_PrivateData = JS_XMLGetTagValue(lTmpVar, "data");
			lItemInfo.m_PrivateMark = JS_XMLGetTagValue(lTmpVar, "out_mark");
		}
	}
	return lItemInfo;
}


function JS_ParamGetSCSCAInfo(Obj)
{
	var lItemInfo = new Object();
	var lTmpVar, i;
	if (Obj != null)
	{
		lItemInfo.m_ACIDs = JS_XMLGetTagValue(Obj, "ac_ids");
		lItemInfo.m_CAPID = JS_XMLGetTagValue(Obj, "ca_pid");
		lItemInfo.m_OutMark = JS_XMLGetTagValue(Obj, "out_mark");
		lItemInfo.m_PrivateData = JS_XMLGetTagValue(Obj, "ca_private_data");
		lItemInfo.m_PrivateMark = JS_XMLGetTagValue(Obj, "ca_private_mark");
	}
	return lItemInfo;
}


function JS_ParamGetTsArray(bInput)
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("transport_streams");
		if (lTmpVar.length)
		{
			if (bInput == true)
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("input_ts");
				if (lTmpVar.length > 0)
				{
					return lTmpVar[0].getElementsByTagName("ts");
				}
			}
			else
			{
				lTmpVar = lTmpVar[0].getElementsByTagName("output_ts");
				if (lTmpVar.length > 0)
				{
					return lTmpVar[0].getElementsByTagName("ts");
				}
			}
		}
	}
	return null;
}

function JS_ParamGetTsNum(bInput)
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = JS_ParamGetTsArray(bInput);
		if (lTmpVar) 
		{
		    return lTmpVar.length;
		}
	}
	return 0;
}


function JS_ParamGetTsObject(TsIndex, bInput)
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
	    lTmpVar = JS_ParamGetTsArray(bInput);
	    if (lTmpVar != null) 
	    {
	        if (lTmpVar.length > TsIndex)
	        {
	            return lTmpVar[TsIndex];
	        }
	    }
	}
	return null;
}


function JS_ParamGetTsByIDs(TsIDs, bInput)
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
	    lTmpVar = JS_ParamGetTsArray(bInput);
		if (lTmpVar != null)
		{
			var i;
			var lIDs;
			var lTsInfo;
			for (i = 0; i < lTmpVar.length; i++)
			{
				lTsInfo = lTmpVar[i].getElementsByTagName("ts_info");
				if (lTsInfo.length)
				{
					lTsInfo = lTsInfo[0];
					lIDs = JS_XMLGetTagValue(lTsInfo, "ts_ids");
					if (lIDs == TsIDs)
					{
						return lTmpVar[i];
					}
				}
			}
		}
	}
	return null;
}


function JS_ParamGetTsMark(TsObj, bInput)
{
	if (TsObj != null)
	{
		var lTmpVar;
		lTmpVar = TsObj.getElementsByTagName("ts_info");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			return JS_XMLGetTagValue(lTmpVar, "ts_mark");
		}
	}
	return null;
}

function JS_ParamGetTsInfo(TsObj, bInput)
{
	var lItemInfo = new Object();
	if (TsObj != null)
	{
		var TsInfoNode;
		TsInfoNode = TsObj.getElementsByTagName("ts_info");
		if (TsInfoNode.length)
		{
			TsInfoNode = TsInfoNode[0];
			lItemInfo.m_IDs = JS_XMLGetTagValue(TsInfoNode, "ts_ids");
			lItemInfo.m_TsMark = JS_XMLGetTagValue(TsInfoNode, "ts_mark");
			if (bInput)
			{
				lItemInfo.m_TSID = JS_XMLGetTagValue(TsInfoNode, "ts_id");
				lItemInfo.m_ONID = JS_XMLGetTagValue(TsInfoNode, "on_id");
				lItemInfo.m_PATV = JS_XMLGetTagValue(TsInfoNode, "pat_v");
				lItemInfo.m_PATCRC32 = JS_XMLGetTagValue(TsInfoNode, "pat_crc32");
				lItemInfo.m_SDTV = JS_XMLGetTagValue(TsInfoNode, "sdt_v");
				lItemInfo.m_CATV = JS_XMLGetTagValue(TsInfoNode, "cat_v");
			}
			else
			{
				lItemInfo.m_TSID = JS_XMLGetTagValue(TsInfoNode, "ts_id");
				lItemInfo.m_ONID = JS_XMLGetTagValue(TsInfoNode, "on_id");
				lItemInfo.m_PATV = JS_XMLGetTagValue(TsInfoNode, "pat_v");
				lItemInfo.m_PATI = JS_XMLGetTagValue(TsInfoNode, "pat_i");
				lItemInfo.m_PATM = JS_XMLGetTagValue(TsInfoNode, "pat_mark");
				lItemInfo.m_CATV = JS_XMLGetTagValue(TsInfoNode, "cat_v");
				lItemInfo.m_CATI = JS_XMLGetTagValue(TsInfoNode, "cat_i");
				lItemInfo.m_CATM = JS_XMLGetTagValue(TsInfoNode, "cat_mark");
				lItemInfo.m_SDTV = JS_XMLGetTagValue(TsInfoNode, "sdt_v");
				lItemInfo.m_SDTI = JS_XMLGetTagValue(TsInfoNode, "sdt_i");
				lItemInfo.m_SDTM = JS_XMLGetTagValue(TsInfoNode, "sdt_mark");
				lItemInfo.m_NITI = JS_XMLGetTagValue(TsInfoNode, "nit_i");
				lItemInfo.m_NITM = JS_XMLGetTagValue(TsInfoNode, "nit_mark");
				lItemInfo.m_TDTI = JS_XMLGetTagValue(TsInfoNode, "tdttot_i");
				lItemInfo.m_TDTM = JS_XMLGetTagValue(TsInfoNode, "tdttot_mark");
			}

		}

	}
	return lItemInfo;
}


function JS_ParamGetTsRouteMark(TsObj)
{
    if (TsObj != null)
    {
        var TsInfoNode;
        TsInfoNode = TsObj.getElementsByTagName("route_info");
        if (TsInfoNode.length)
        {
            TsInfoNode = TsInfoNode[0];
            return JS_XMLGetTagValue(TsInfoNode, "route_mark");
        }
    }
    return null;
}

function JS_ParamGetTsRouteInfo(TsObj)
{
	var lItemInfo = new Object();
	if (TsObj != null)
	{
		var TsInfoNode;
		TsInfoNode = TsObj.getElementsByTagName("route_info");
		if (TsInfoNode.length)
		{
			TsInfoNode = TsInfoNode[0];
			lItemInfo.m_TsIndex = parseInt(JS_XMLGetTagValue(TsInfoNode, "ts_index"), 10);
			lItemInfo.m_RouteMark = JS_XMLGetTagValue(TsInfoNode, "route_mark");
			lItemInfo.m_RouteNULLMark = JS_XMLGetTagValue(TsInfoNode, "null");
			lItemInfo.m_RoutePCRMark = JS_XMLGetTagValue(TsInfoNode, "pcr");        }
	}
	return lItemInfo;
}

function JS_ParamGetServArray()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("services");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("service");
		}
	}
	return null;
}

function JS_ParamGetServCountInfo()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("services");
        if (lTmpVar.length)
        {
            lItemInfo.CurrentServTotal = parseInt(JS_XMLGetTagValue(lTmpVar[0], "current_input_service_num"), 10);
            lItemInfo.CurrentServOutput = parseInt(JS_XMLGetTagValue(lTmpVar[0], "current_output_service_num"), 10);
            lItemInfo.CurrentServMax = parseInt(JS_XMLGetTagValue(lTmpVar[0], "current_service_max"), 10);
        }
    }
    return lItemInfo;
}


function JS_ParamInServiceSortFunction(Item1, Item2)
{
	var lServID1, lServID2;
	var lTmpVar;

	lTmpVar = Item1.getElementsByTagName("in_service_info");
	lServID1 = parseInt(JS_XMLGetTagValue(lTmpVar[0], "service_id"), 10);
	lTmpVar = Item2.getElementsByTagName("in_service_info");
	lServID2 = parseInt(JS_XMLGetTagValue(lTmpVar[0], "service_id"), 10);
	return lServID1 - lServID2;
}

function JS_ParamOutServiceSortFunction(Item1, Item2)
{
	var lServID1, lServID2;
	var lTmpVar;

	lTmpVar = Item1.getElementsByTagName("out_service_info");
	lServID1 = parseInt(JS_XMLGetTagValue(lTmpVar[0], "service_id"), 10);
	lTmpVar = Item2.getElementsByTagName("out_service_info");
	lServID2 = parseInt(JS_XMLGetTagValue(lTmpVar[0], "service_id"), 10);
	return lServID1 - lServID2;
}


function JS_ParamGetServArrayByTsIndex(TsIndex, bInput)
{
	var lTmpArray, lTmpVar;
	var i;

	lTmpArray = new Array();
	lTmpVar = JS_ParamGetServArray();
	if (lTmpVar)
	{
		for (i = 0; i < lTmpVar.length; i++)
		{
			if (bInput)
			{
				lTmpValue = JS_XMLGetTagValue(lTmpVar[i], "in_ts_index");
				if (lTmpValue == TsIndex)
				{
					lTmpArray[lTmpArray.length] = lTmpVar[i];
				}
			}
			else
			{
				lTmpValue = JS_XMLGetTagValue(lTmpVar[i], "out_ts_index");
				if (lTmpValue == TsIndex)
				{
					lTmpArray[lTmpArray.length] = lTmpVar[i];
				}
			}

		}
	}

	if (bInput)
	{
		lTmpArray.sort(JS_ParamInServiceSortFunction);
	}
	else
	{
		lTmpArray.sort(JS_ParamOutServiceSortFunction);
	}

	return lTmpArray;
}


function JS_ParamGetServByIDs(ServIDs)
{
	var lTmpArray, lTmpVar;
	var i;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("services");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("service");
			for (i = 0; i < lTmpVar.length; i++)
			{
				lTmpValue = JS_XMLGetTagValue(lTmpVar[i], "serv_ids");
				if (lTmpValue == ServIDs)
				{
					return lTmpVar[i];
				}
			}
		}
	}
	return null;
}


function JS_ParamGetServiceIO(ServObj)
{
	var lItemInfo = new Object();
	if (ServObj != null)
	{
		lItemInfo.m_InTsIndex = parseInt(JS_XMLGetTagValue(ServObj, "in_ts_index"), 10);
		lItemInfo.m_OutTsIndex = parseInt(JS_XMLGetTagValue(ServObj, "out_ts_index"), 10);
	}
	return lItemInfo;
}


function JS_ParamGetServiceInfo(ServObj, bInput)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (ServObj != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(ServObj, "serv_ids");
		lItemInfo.m_UniqueID = JS_XMLGetTagValue(ServObj, "uni_id");
		if (bInput)
		{
			lTmpVar = ServObj.getElementsByTagName("in_service_info");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0];
				lItemInfo.m_PMTPID = parseInt(JS_XMLGetTagValue(lTmpVar, "pmt_pid"), 10);
				lItemInfo.m_PMTV = parseInt(JS_XMLGetTagValue(lTmpVar, "pmt_v"), 10);
				lItemInfo.m_PMTCRC32 = JS_XMLGetTagValue(lTmpVar, "pmt_crc32");
				lItemInfo.m_PCRPID = parseInt(JS_XMLGetTagValue(lTmpVar, "pcr_pid"), 10);
				lItemInfo.m_TransPCRPID = parseInt(JS_XMLGetTagValue(lTmpVar, "trans_pcr_pid"), 10);
				lItemInfo.m_ServID = parseInt(JS_XMLGetTagValue(lTmpVar, "service_id"), 10);
				lItemInfo.m_ServType = parseInt(JS_XMLGetTagValue(lTmpVar, "service_type"), 10);
				lItemInfo.m_EITSCMark = JS_XMLGetTagValue(lTmpVar, "eit_schedule_mark");
				lItemInfo.m_EITCNMark = JS_XMLGetTagValue(lTmpVar, "eit_cn_mark");
				lItemInfo.m_ServName = JS_XMLGetTagValue(lTmpVar, "service_name");
				lItemInfo.m_ProvName = JS_XMLGetTagValue(lTmpVar, "service_provider");
			}
		}
		else
		{
			lTmpVar = ServObj.getElementsByTagName("out_service_info");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0];
				lItemInfo.m_PMTPID = parseInt(JS_XMLGetTagValue(lTmpVar, "pmt_pid"), 10);
				lItemInfo.m_PMTV = parseInt(JS_XMLGetTagValue(lTmpVar, "pmt_v"), 10);
				lItemInfo.m_PMTI = parseInt(JS_XMLGetTagValue(lTmpVar, "pmt_interval"), 10);
				lItemInfo.m_PMTM = JS_XMLGetTagValue(lTmpVar, "pmt_mark");
				lItemInfo.m_PCRPID = parseInt(JS_XMLGetTagValue(lTmpVar, "pcr_pid"), 10);
				lItemInfo.m_PCRM = JS_XMLGetTagValue(lTmpVar, "pcr_mark");
				lItemInfo.m_ServID = parseInt(JS_XMLGetTagValue(lTmpVar, "service_id"), 10);
				lItemInfo.m_ServType = parseInt(JS_XMLGetTagValue(lTmpVar, "service_type"), 10);
				lItemInfo.m_EITSCMark = JS_XMLGetTagValue(lTmpVar, "eit_schedule_mark");
				lItemInfo.m_EITCNMark = JS_XMLGetTagValue(lTmpVar, "eit_cn_mark");
				lItemInfo.m_ScrambleMark = JS_XMLGetTagValue(lTmpVar, "scramble_mark");
				lItemInfo.m_ServName = JS_XMLGetTagValue(lTmpVar, "service_name");
				lItemInfo.m_ProvName = JS_XMLGetTagValue(lTmpVar, "service_provider");
				lItemInfo.m_PSM = JS_XMLGetTagValue(lTmpVar, "ps_m");

				lItemInfo.m_AuthClass = parseInt(JS_XMLGetTagValue(lTmpVar, "auth_class"), 10);
				lItemInfo.m_LCN = parseInt(JS_XMLGetTagValue(lTmpVar, "lcn"), 10);
				lItemInfo.m_LCNVisiable = JS_XMLGetTagValue(lTmpVar, "lcn_visiable");
				lItemInfo.m_BouquetID = parseInt(JS_XMLGetTagValue(lTmpVar, "bouquet_id"), 10);

				lItemInfo.m_TransPCRPID = parseInt(JS_XMLGetTagValue(lTmpVar, "trans_pcr_pid"), 10);
}
		}
	}
	return lItemInfo;
}

function JS_ParamGetServiceInfoTransMark(ServObj)
{
	var lTmpVar;
	var lMark = false;
	if (ServObj != null)
	{
		
		lTmpVar = ServObj.getElementsByTagName("out_service_info");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lMark = (JS_XMLGetTagValueINT(lTmpVar, "trans_mark", 10) == 1)?true:false;
		}
	}
	return lMark;
}

function JS_ParamGetServiceInfoTransInfo(ServObj)
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (ServObj != null)
    {
        lTmpVar = ServObj.getElementsByTagName("out_service_info");
        if (lTmpVar.length)
        {
            lTmpVar = lTmpVar[0];
            lItemInfo.m_Mark = (JS_XMLGetTagValueINT(lTmpVar, "trans_mark", 10) == 1) ? true : false;
            lItemInfo.m_VIDEsType = JS_XMLGetTagValueINT(lTmpVar, "trans_vid_es_type", 10);
            lItemInfo.m_VIDEsIDs = JS_XMLGetTagValueINT(lTmpVar, "trans_vid_es_ids", 16);
        }
    }
    return lItemInfo;
}


function JS_ParamCheckServicePCRESMatch(ServObj)
{
	var i, lTmpVar;
	var lServInfo, lESArray, lEsInfo;
	if (ServObj != null)
	{
		lServInfo = JS_ParamGetServiceInfo(ServObj, true);
		lESArray = JS_ParamGetEsArray(ServObj);
		for (i = 0; i < lESArray.length; i++)
		{
			lEsInfo = JS_ParamGetEsInfo(lESArray[i], true);
			if (lEsInfo.m_ESPID == lServInfo.m_PCRPID)
			{
				return i;
			}
			lEsInfo = null;
		}

		lESArray = null;
		lServInfo = null;

	}
	return null;
}


function JS_ParamGetEsNum(ServObj)
{
	var lTmpVar;
	if (ServObj != null)
	{
		lTmpVar = ServObj.getElementsByTagName("element_streams");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("es");
			return lTmpVar.length;
		}
	}
	return 0;
}


function JS_ParamGetEsObj(ServObj, EsIndex)
{
	var lTmpVar;
	if (ServObj != null)
	{
		lTmpVar = ServObj.getElementsByTagName("element_streams");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("es");
			if (lTmpVar.length > EsIndex)
			{
				return lTmpVar[EsIndex];
			}
		}
	}
	return null;
}


function JS_ParamGetEsArray(ServObj)
{
	var lTmpVar;
	if (ServObj != null)
	{
		lTmpVar = ServObj.getElementsByTagName("element_streams");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("es");
		}
	}
	return null;
}

function JS_ParamGetEsArraySystem()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("services");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("es");
		}
	}
	return null;
}

function JS_ParamGetEsByIDs(EsIDs)
{
	var lTmpVar;
	var lItemArray;
	var lIDs;
	var i;

	lItemArray = JS_ParamGetEsArraySystem();
	if (lItemArray)
	{
		for (i = 0; i < lItemArray.length; i++)
		{
			lIDs = JS_XMLGetTagValue(lItemArray[i], "es_ids");
			if (lIDs == EsIDs)
			{
				return lItemArray[i];
			}
		}
	}
	return null;
}

function JS_ParamGetEsInfo(EsObj, bInput)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (EsObj != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(EsObj, "es_ids");
		if (bInput)
		{
			lTmpVar = EsObj.getElementsByTagName("in_es_info");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0];
				lItemInfo.m_ESPID = JS_XMLGetTagValue(lTmpVar, "es_pid");
				lItemInfo.m_ESType = parseInt(JS_XMLGetTagValue(lTmpVar, "es_type"), 10);
			}
		}
		else
		{
			lTmpVar = EsObj.getElementsByTagName("out_es_info");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0];
				lItemInfo.m_ESPID = JS_XMLGetTagValue(lTmpVar, "es_pid");
				lItemInfo.m_ESType = parseInt(JS_XMLGetTagValue(lTmpVar, "es_type"), 10);
				lItemInfo.m_OutMark = JS_XMLGetTagValue(lTmpVar, "es_out_mark");
				lItemInfo.m_SCRMark = JS_XMLGetTagValue(lTmpVar, "es_scr_mark");
				lItemInfo.m_PSM = JS_XMLGetTagValue(lTmpVar, "ps_m");
			}
		}
	}
	return lItemInfo;
}

function JS_ParamGetSCSSystemInfo()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("system_info");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_DefaultCP = JS_XMLGetTagValue(lTmpVar, "default_cp");
			lItemInfo.m_NetDelayCP = JS_XMLGetTagValue(lTmpVar, "network_delay");
			lItemInfo.m_FixedCW = JS_XMLGetTagValue(lTmpVar, "fixed_cw");
			lItemInfo.m_bUserFixedCWMark = JS_XMLGetTagValue(lTmpVar, "fixed_cw_mark");

			lItemInfo.m_BSSMark = JS_XMLGetTagValue(lTmpVar, "bss_enable");
			lItemInfo.m_Supercasid = JS_XMLGetTagValue(lTmpVar, "super_id");
			lItemInfo.m_BSSSW = JS_XMLGetTagValue(lTmpVar, "bss_sw");
			lItemInfo.m_bUserBSSSWMark = JS_XMLGetTagValue(lTmpVar, "sw_mark");
			lItemInfo.m_Key = JS_XMLGetTagValue(lTmpVar, "key");
		}
	}
	return lItemInfo;
}


function JS_ParamGetSCSArray()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("simulcrypt_synchronizer");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("scs_info");
		}
	}
	return null;
}


function JS_ParamGetSCSInfo(SCSObj)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (SCSObj != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(SCSObj, "scs_ids");
		lItemInfo.m_Name = JS_XMLGetTagValue(SCSObj, "scs_name");
		lItemInfo.m_SuperCASID = JS_XMLGetTagValue(SCSObj, "supercas_id");
		lItemInfo.m_CASID = JS_StrLeftPading(parseInt(parseInt(lItemInfo.m_SuperCASID, 16) / 65536).toString(16), 4);
		lItemInfo.m_EMMGPort = parseInt(JS_XMLGetTagValue(SCSObj, "emmg_port"), 10);
		lItemInfo.m_ECMGIP = JS_XMLGetTagValue(SCSObj, "ecmg_ip");
		lItemInfo.m_ECMGPort = parseInt(JS_XMLGetTagValue(SCSObj, "ecmg_port"), 10);
		lItemInfo.m_ECMCHNID = JS_XMLGetTagValue(SCSObj, "ecm_chn_id");
		lItemInfo.m_Active = JS_XMLGetTagValue(SCSObj, "active_mark");
		lItemInfo.m_Disabled = parseInt(JS_XMLGetTagValue(SCSObj, "disabled"), 10);
	}
	return lItemInfo;
}


function JS_ParamGetSCSACArray(SCSObj)
{
	var lItemInfo = new Object();
	if (SCSObj != null)
	{
		lTmpVar = SCSObj.getElementsByTagName("acs");
		if (lTmpVar.length)
		{
			return lTmpVar[0].getElementsByTagName("ac");
		}
	}
	return lItemInfo;
}

function JS_ParamGetACInfo(SCSObj)
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (SCSObj != null)
	{
		lItemInfo.m_IDs = JS_XMLGetTagValue(SCSObj, "ac_ids");
		lItemInfo.m_Name = JS_XMLGetTagValue(SCSObj, "ac_name");
		lItemInfo.m_Data = JS_XMLGetTagValue(SCSObj, "data");
	}
	return lItemInfo;
}


function JS_ParamGetRemuxSystemInfo()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("remux_setting");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_DefaultInCharset = parseInt(JS_XMLGetTagValue(lTmpVar, "input_charset"), 10);
			lItemInfo.m_DefaultOutCharset = parseInt(JS_XMLGetTagValue(lTmpVar, "output_charset"), 10);
			lItemInfo.m_OutCharsetMarker = parseInt(JS_XMLGetTagValue(lTmpVar, "out_charset_marker"), 10);
			lItemInfo.m_TZ = parseInt(JS_XMLGetTagValue(lTmpVar, "time_zone"), 10);
			lItemInfo.m_TimeUpdate = parseInt(JS_XMLGetTagValue(lTmpVar, "time_update_cycle"), 10);
			lItemInfo.m_TOTMark = JS_XMLGetTagValue(lTmpVar, "tot_mark");
			lItemInfo.m_CountrCode = JS_XMLGetTagValue(lTmpVar, "country_code");
			lItemInfo.m_AutoMark = JS_XMLGetTagValue(lTmpVar, "auto_inc");
			lItemInfo.m_PCRMark = JS_XMLGetTagValue(lTmpVar, "auto_inc");
			lItemInfo.m_MaxServNameBytes = JS_XMLGetTagValue(lTmpVar, "max_service_name_bytes");
			lItemInfo.m_DisableRemuxMark = JS_XMLGetTagValue(lTmpVar, "disable_remux");
			lItemInfo.m_DisableRouteMark = JS_XMLGetTagValue(lTmpVar, "disable_route");
			lItemInfo.m_UseSDT = JS_XMLGetTagValue(lTmpVar, "spts_sdt_mark");
			
			lItemInfo.m_NITDisable = JS_XMLGetTagValue(lTmpVar, "nit_disable");
			lItemInfo.m_InserterDisable = JS_XMLGetTagValue(lTmpVar, "inserter_disable");
			lItemInfo.m_BatchServiceEdit = JS_XMLGetTagValue(lTmpVar, "batch_service_edit");

			lItemInfo.m_NewRouteFunctionMark = JS_XMLGetTagValue(lTmpVar, "new_route_func");
			
			lItemInfo.m_TransPCRPIDEnable = JS_XMLGetTagValue(lTmpVar, "trans_pcr_pid_enable");}
	}
	return lItemInfo;
}

function JS_ParamGetPCRInfo()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("pcr_correct");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
			lItemInfo.m_PCRCMark = JS_XMLGetTagValue(lTmpVar, "pcrc_mark");
			lItemInfo.m_PCRCPos = parseInt(JS_XMLGetTagValue(lTmpVar, "pcrc_pos"), 10);
			lItemInfo.m_PCRCNeg = parseInt(JS_XMLGetTagValue(lTmpVar, "pcrc_neg"), 10);
		}
	}
	return lItemInfo;
}


function JS_ParamGetIGMPInfo()
{
	var lTmpVar;
	var lItemInfo = new Object();
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("channels");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0].getElementsByTagName("igmp");
			if (lTmpVar.length)
			{
				lTmpVar = lTmpVar[0];
				lItemInfo.m_IGMPGroupMax = parseInt(JS_XMLGetTagValue(lTmpVar, "igmp_group_max"), 10);
				lItemInfo.m_IGMPRepeateTime = parseInt(JS_XMLGetTagValue(lTmpVar, "igmp_repeate_time"), 10);
				lItemInfo.m_IGMPRepeateMark = JS_XMLGetTagValue(lTmpVar, "igmp_repeate_mark");
				lItemInfo.m_IGMPVersion = parseInt(JS_XMLGetTagValue(lTmpVar, "igmp_version"), 10);
			}
		}
	}
	return lItemInfo;
}



function JS_ParamGetCharSetArray()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		return s_ParameterXMLDOM.getElementsByTagName("char_set");
	}
	return null;

}


function JS_ParamGetCharSetInfo(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_Value = JS_XMLGetTagValue(Obj, "value");
	lSubInfo.m_Txt = JS_XMLGetTagValue(Obj, "txt");
	return lSubInfo;
}

function JS_ParamGetAutoMapArray()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		return s_ParameterXMLDOM.getElementsByTagName("auto_map");
	}
	return null;

}

function JS_ParamGetAutoMapInfo(Obj)
{
	var lTmpVar;
	var lSubInfo = new Object();
	lSubInfo.m_Value = JS_XMLGetTagValue(Obj, "value");
	lSubInfo.m_Txt = JS_XMLGetTagValue(Obj, "txt");
	return lSubInfo;
}

function JS_ParamGetCurrentAutoMapMode()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("auto_maps");
		if (lTmpVar.length != 0)
		{
			return parseInt(JS_XMLGetTagValue(lTmpVar[0], "cur_auto_mode"), 10);
		}
	}
	return null;
}


function JS_ParamCheckEMMConflictWithOtherDev(EMMPort)
{
	var lTmpVar;
	var i;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("other_emms");
		if (lTmpVar != null)
		{
			for (i = 0; i < lTmpVar.length; i++)
			{
				if (parseInt(EMMPort) == parseInt(JS_XMLGetTagValue(lTmpVar[i], "other_emm_port"), 10))
				{
					return true;
				}
			}
		}
	}
	return false;
}



function JS_ParamGetECA()
{
	var lTmpVar;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("eca_setting");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
		}
		else
		{
			lTmpVar = null;
		}

	}
	return lTmpVar;
}

function JS_ParamGetSERVL()
{
	var lTmpVar = null;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("servl_setting");
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[0];
		}
		else
		{
			lTmpVar = null;
		}
	}
	return lTmpVar;
}

function JS_ParamGetESScrambleObj()
{
	var lTmpVar = null;
	if (s_ParameterXMLDOM != null)
	{
		lTmpVar = s_ParameterXMLDOM.getElementsByTagName("scramble_advance_setting");
		if (lTmpVar.length)
		{

			lTmpVar = lTmpVar[0];
		}
		else
		{
			lTmpVar = null;
		}
	}
	return lTmpVar;
}

function JS_ParamGetESScrambleInfoObj(Obj, Index)
{
	var lTmpVar = null;
	if (Obj != null)
	{
		lTmpVar = Obj.getElementsByTagName("scramble_es_setting")
		if (lTmpVar.length)
		{
			lTmpVar = lTmpVar[Index];
		}
		else
		{
			lTmpVar = null;
		}
	}
	return lTmpVar;
}

function JS_ParamGetESScrambleNum(Obj)
{
	var lTmpVar = null;
	if (Obj != null)
	{
		lTmpVar = Obj.getElementsByTagName("scramble_es_setting")
		if (lTmpVar.length)
		{
			return lTmpVar.length;
		}
	}
	return 0;
}


function JS_ParamGetESScrambleInfo(Obj)
{
	var lESScrambleInfo = new Object();
	lESScrambleInfo.m_ESTypeValue = parseInt(JS_XMLGetTagValue(Obj, "es_type_value"), 10);
	lESScrambleInfo.m_Scramble = JS_XMLGetTagValue(Obj, "scramble");
	return lESScrambleInfo;
}


function JS_ParamGetMosiacArray()
{
    var lTmpArray, lTmpVar, lItemInfo;
    var i;

    lTmpArray = new Array();
	lTmpVar = s_ParameterXMLDOM.getElementsByTagName("mosiac_gn2000");
    if (lTmpVar.length) 
    {
        lTmpVar = lTmpVar[0].getElementsByTagName("group")
        for (i = 0; i < lTmpVar.length; i++)
        {
            lItemInfo = new Object();
            lItemInfo.m_InTsInd = JS_XMLGetTagValue(lTmpVar[i], "in_ts_ind");
            lItemInfo.m_ServiceID = JS_XMLGetTagValue(lTmpVar[i], "service_id");
            lItemInfo.m_ServiceName = JS_XMLGetTagValue(lTmpVar[i], "service_name");
            lItemInfo.m_Offset = parseInt(JS_XMLGetTagValue(lTmpVar[i], "offset"), 10);
            lItemInfo.m_OriServiceID = JS_XMLGetTagValue(lTmpVar[i], "ori_service_id");
            lItemInfo.m_OriTSID = JS_XMLGetTagValue(lTmpVar[i], "ori_ts_id");
            lTmpArray[lTmpArray.length] = lItemInfo;
        }
    }
    return lTmpArray;
}

function JS_ParamHaveTsBackuFunc()
{
    var lTmpArray, lTmpVar;
    lTmpArray = new Array();
    lTmpVar = s_ParameterXMLDOM.getElementsByTagName("ts_backup_setting_root");
    if (lTmpVar.length)
    {
        return true;
    }
    else
    {
        return false;
    }

}

function JS_ParamGetTsBackupInTsArray()
{
    var lTmpArray, lTmpVar, lItemInfo;
    var i;

    lTmpArray = new Array();
    lTmpVar = s_ParameterXMLDOM.getElementsByTagName("ts_backup_setting_root");
    if (lTmpVar.length)
    {
        lTmpVar = lTmpVar[0].getElementsByTagName("backup_ts_array_in");
        if (lTmpVar.length)
        {
            lTmpVar = lTmpVar[0].getElementsByTagName("backup_ts_info");
            for (i = 0; i < lTmpVar.length; i++)
            {
                lItemInfo = new Object();
                lItemInfo.m_BackupTsInd = parseInt(JS_XMLGetTagValue(lTmpVar[i], "backup_ts_ind"), 10);
                lItemInfo.m_Low = parseInt(JS_XMLGetTagValue(lTmpVar[i], "low"), 10);
                lItemInfo.m_High = parseInt(JS_XMLGetTagValue(lTmpVar[i], "high"), 10);
                lItemInfo.m_Mode = JS_XMLGetTagValue(lTmpVar[i], "mode");
                lTmpArray[lTmpArray.length] = lItemInfo;
            }
        }
    }
    return lTmpArray;
}


function JS_ParamGetTsBackupOutTsArray()
{
    var lTmpArray, lTmpVar, lItemInfo;
    var i;

    lTmpArray = new Array();
    lTmpVar = s_ParameterXMLDOM.getElementsByTagName("ts_backup_setting_root");
    if (lTmpVar.length)
    {
        lTmpVar = lTmpVar[0].getElementsByTagName("backup_ts_array_out");
        if (lTmpVar.length)
        {
            lTmpVar = lTmpVar[0].getElementsByTagName("backup_ts_info");
            for (i = 0; i < lTmpVar.length; i++)
            {
                lItemInfo = new Object();
                lItemInfo.m_BackupTsInd = parseInt(JS_XMLGetTagValue(lTmpVar[i], "backup_ts_ind"), 10);
                lItemInfo.m_Mode = JS_XMLGetTagValue(lTmpVar[i], "mode");
                lTmpArray[lTmpArray.length] = lItemInfo;
            }
        }
    }
    return lTmpArray;
}

function JS_ParamCheckSFNM()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("sfn_modulator_setting");
        if (lTmpVar.length)
        {
            return true;
        }
    }
    return false;
}

function JS_ParamGetSFNMInfo()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("sfn_modulator_setting");
        if (lTmpVar.length)
        {
            lTmpVar = lTmpVar[0];
            lItemInfo.m_bHaveSFN = true;
            lItemInfo.m_SFNMark = JS_XMLGetTagValue(lTmpVar, "sfn_mark");
            lItemInfo.m_SFN10MSel = JS_XMLGetTagValue(lTmpVar, "sfn_10m_sel");
            lItemInfo.m_SFNEx1PPSMark = JS_XMLGetTagValue(lTmpVar, "sfn_ex1pps_mark");
            lItemInfo.m_SFNAddDelay100NS = parseInt(JS_XMLGetTagValue(lTmpVar, "sfn_add_delay_100ns"), 10);
            
            lItemInfo.m_SFNTsInd = parseInt(JS_XMLGetTagValue(lTmpVar, "sfn_input_asi_mod"), 10);
                       
            lItemInfo.m_SFNAddrID = parseInt(JS_XMLGetTagValue(lTmpVar, "sfn_addr_id"), 10);
            lItemInfo.m_SFNUSEINDV = JS_XMLGetTagValue(lTmpVar, "sfn_use_indv_mark");
            lItemInfo.m_SFNUSECMN = JS_XMLGetTagValue(lTmpVar, "sfn_use_cmn_mark");
            
            lItemInfo.m_SFNSatNullPID = parseInt(JS_XMLGetTagValue(lTmpVar, "sfn_sat_null_pid"), 10);
            lItemInfo.m_SFNSatMark = JS_XMLGetTagValue(lTmpVar, "sfn_sat_mark");
            lItemInfo.m_SFNSatCRC32Mark = JS_XMLGetTagValue(lTmpVar, "sfn_sat_crc_checkmark");

            lItemInfo.m_SFNSIPDELMark = JS_XMLGetTagValue(lTmpVar, "sfn_sip_del_mark");


            lItemInfo.m_bTsLostMUTE = JS_XMLGetTagValue(lTmpVar, "sfn_ts_mute_mark");
            lItemInfo.m_bREFLostMUTE = JS_XMLGetTagValue(lTmpVar, "sfn_ref_mute_mark");
            lItemInfo.m_b1PPSLostMUTE = JS_XMLGetTagValue(lTmpVar, "sfn_1pps_mute_mark");
            lItemInfo.m_bSIPLostMUTE = JS_XMLGetTagValue(lTmpVar, "sfn_sip_mute_mark");
        }
        else
        {
            lItemInfo.m_bHaveSFN = false;
        }
    }
    return lItemInfo;
}

function JS_ParamCheckSFNA()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("sfn_adaptor_setting");
        if (lTmpVar.length)
        {
            return true;
        }
    }
    return false;
}


function JS_ParamGetSFNAInfo()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("sfn_adaptor_setting");
        if (lTmpVar.length)
        {
            lTmpVar = lTmpVar[0];
            lItemInfo.m_MaxBitrate = parseInt(JS_XMLGetTagValue(lTmpVar, "sfn_max_bitrate"));
            lItemInfo.m_SFN10MSel = JS_XMLGetTagValue(lTmpVar, "sfn_10m_sel");
            lItemInfo.m_SFNEx1PPSMark = JS_XMLGetTagValue(lTmpVar, "sfn_ex1pps_mark");
            lItemInfo.m_SFNMaxDelay100NS = parseInt(JS_XMLGetTagValue(lTmpVar, "sfn_max_delay_100ns"), 10);

            lItemInfo.m_SFNTsInd = parseInt(JS_XMLGetTagValue(lTmpVar, "sfn_input_asi_mod"), 10);
            lItemInfo.m_CMode = JS_XMLGetTagValue(lTmpVar, "sfn_carrier_mode");
            lItemInfo.m_CodeRate = JS_XMLGetTagValue(lTmpVar, "sfn_code_rate");
            lItemInfo.m_Constellation = JS_XMLGetTagValue(lTmpVar, "sfn_constellation");
            lItemInfo.m_PNMode = JS_XMLGetTagValue(lTmpVar, "sfn_pn_mode");
            lItemInfo.m_TIMode = JS_XMLGetTagValue(lTmpVar, "sfn_ti_mode");
            lItemInfo.m_DP = JS_XMLGetTagValue(lTmpVar, "sfn_dp");

            lItemInfo.m_SFNSatNullPID = parseInt(JS_XMLGetTagValue(lTmpVar, "sfn_sat_null_pid"), 10);
            lItemInfo.m_SFNSatMark = JS_XMLGetTagValue(lTmpVar, "sfn_sat_mark");

            lItemInfo.m_SFNINDVArray = lTmpVar.getElementsByTagName("sfn_indv_node");
        }
        else
        {
            lItemInfo.m_bHaveSFN = false;
        }
    }
    return lItemInfo;
}

function JS_ParamGetSFNAINDVInfo(INDVNode)
{
    var lTmpVar;
    var lItemInfo = new Object();

    lItemInfo.m_Addr = parseInt(JS_XMLGetTagValue(INDVNode, "indv_addr"), 10);
    lItemInfo.m_FreqOffsetHz = parseInt(JS_XMLGetTagValue(INDVNode, "indv_freq_offset_hz"), 10);
    lItemInfo.m_Delay = parseInt(JS_XMLGetTagValue(INDVNode, "indv_delay"), 10);
    lItemInfo.m_Power = parseInt(JS_XMLGetTagValue(INDVNode, "indv_power"), 10);
    lItemInfo.m_PowMark = JS_XMLGetTagValue(INDVNode, "indv_power_mark");
    lItemInfo.m_ActiveMark = JS_XMLGetTagValue(INDVNode, "active_mark");

    return lItemInfo;
}


function JS_ParamCheckGNS()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("gns_setting");
        if (lTmpVar.length)
        {
            return true;
        }
    }
    return false;
}

function JS_ParamGetGNSInfo()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("gns_setting");
        if (lTmpVar.length)
        {
            lTmpVar = lTmpVar[0];
            lItemInfo.m_bGNSMark = JS_XMLGetTagValue(lTmpVar, "gns_mark");
            lItemInfo.m_bTimingMark = JS_XMLGetTagValue(lTmpVar, "gns_timing_mark");
            lItemInfo.m_TimingInterval = parseInt(JS_XMLGetTagValue(lTmpVar, "gns_timing_interval"), 10);
            lItemInfo.m_GNSType = parseInt(JS_XMLGetTagValue(lTmpVar, "gns_sys_type"), 10);
        }
    }
    return lItemInfo;
}

function JS_ParamCheckIPoTs()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("sfn_modulator_setting");
        if (lTmpVar.length)
        {
            return true;
        }
    }
    return false;
}

function JS_ParamGetIPoTSInfo()
{
    var lTmpVar;
    var lItemInfo = new Object();
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("ipots_setting");
        if (lTmpVar.length)
        {
            lTmpVar = lTmpVar[0];
            lItemInfo.m_UPFreq = parseInt(JS_XMLGetTagValue(lTmpVar, "uplink_freq"), 10);
            lItemInfo.m_UPBand = parseInt(JS_XMLGetTagValue(lTmpVar, "uplink_bandwidth"), 10);
            lItemInfo.m_UPConstella = JS_XMLGetTagValue(lTmpVar, "uplink_constellation");
            lItemInfo.m_UPGain = parseInt(JS_XMLGetTagValue(lTmpVar, "uplink_gain"), 10);
            
            lItemInfo.m_DownFreq = parseInt(JS_XMLGetTagValue(lTmpVar, "downlink_freq"), 10);
            lItemInfo.m_DownBand = parseInt(JS_XMLGetTagValue(lTmpVar, "downlink_bandwidth"), 10);
            lItemInfo.m_DownIQSwaptMark = JS_XMLGetTagValue(lTmpVar, "downlink_iq_swapt_mark");
            
            lItemInfo.m_AdaptMark = JS_XMLGetTagValue(lTmpVar, "constellatio_adapt_mark");
            lItemInfo.m_DebugModeMark = JS_XMLGetTagValue(lTmpVar, "debug_mode_mark");
            
            lItemInfo.m_GainLevelMax = parseInt(JS_XMLGetTagValue(lTmpVar, "gain_level_max"), 10);
            lItemInfo.m_GainLevelStep = parseFloat(JS_XMLGetTagValue(lTmpVar, "gain_stepping"));
            
            lItemInfo.m_UPNum = parseInt(JS_XMLGetTagValue(lTmpVar, "uplink_chn_num"), 10);
            lItemInfo.m_DownNUM = parseInt(JS_XMLGetTagValue(lTmpVar, "downlink_chn_num"), 10);
        }
    }
    return lItemInfo;
}

function JS_ParamGetDPDSetting()
{
    var lTmpVar;
    if (s_ParameterXMLDOM != null)
    {
        lTmpVar = s_ParameterXMLDOM.getElementsByTagName("dpd_setting");
    }

    if (lTmpVar.length > 0) 
    {
        return lTmpVar[0];
    }
    return null;
}

function JS_ParamGetTransServUNIIDArray()
{
    var lRetVar;
    
    lRetVar = null;
    
    if (s_ParameterXMLDOM != null)
    {
        var lTmpVar;

        lTmpVar = JS_XMLGetTagNameFirstObj(s_ParameterXMLDOM, "trans_servs");

        if (JS_ISValidObject(lTmpVar))
        {
            lRetVar = JS_XMLGetTagNameArray(lTmpVar, "uni_id");
        }
    }
    
    return lRetVar;
}

