/*UTF-8编码*/
function JS_MPEG2ValidateDescriptor(HexString)
{
	var lDescriptorLen;
// 	var lDescripptorTag;
	if (HexString.length %2 != 0 || (HexString.length < 4)) 
	{
		return false;
	}
	else
	{
// 		lDescripptorTag = parseInt(HexString.substr(0, 2), 16);
// 		if ((lDescripptorTag == 0xFF) || (lDescripptorTag == 0x00))
// 		{
// 			return false;
// 		}
		
		lDescriptorLen = parseInt(HexString.substr(2,2), 16);
		if (!isNaN(lDescriptorLen)) 
		{
			if ((lDescriptorLen + 2) != HexString.length / 2) 
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

function JS_MPEG2GetStreamType(TypeIndex, LanguageCode, bIndex)
{
	var lString;
	switch (TypeIndex)
	{
	case 0x01:
		{
			if (LanguageCode == 0)
			{
				lString = "MPEG-1 视频";
			}
			else
			{
				lString = "MPEG-1 Video";
			}
		}
		break;
	case 0x02:
		{
			if (LanguageCode == 0)
			{
				lString = "MPEG-2 视频";
			}
			else
			{
				lString = "MPEG-2 Video";
			}
		}
		break;
	case 0x03:
		{
			if (LanguageCode == 0)
			{
				lString = "MPEG-1 音频";
			}
			else
			{
				lString = "MPEG-1 Audio";
			}
		}
		break;
	case 0x04:
		{
			if (LanguageCode == 0)
			{
				lString = "MPEG-2 音频";
			}
			else
			{
				lString = "MPEG-2 Audio";
			}
		}
		break;
	case 0x05:
		{
			if (LanguageCode == 0)
			{
				lString = "私有分段";
			}
			else
			{
				lString = "Private Section";
			}
		}
		break;
	case 0x06:
		{
			if (LanguageCode == 0)
			{
				lString = "PES 私有数据";
			}
			else
			{
				lString = "Private PES Packet";
			}
		}
		break;
	case 0x07:
		{
			if (LanguageCode == 0)
			{
				lString = "MHEG";
			}
			else
			{
				lString = "MHEG";
			}
		}
		break;
	case 0x08:
		{
			if (LanguageCode == 0)
			{
				lString = "DSMCC";
			}
			else
			{
				lString = "DSMCC";
			}
		}
		break;
	case 0x09:
		{
			if (LanguageCode == 0)
			{
				lString = "ITU-T Rec.H.222.1";
			}
			else
			{
				lString = "ITU-T Rec.H.222.1";
			}
		}
		break;
	case 0x0A:
		{
			if (LanguageCode == 0)
			{
				lString = "Multi-protocol Encap";
			}
			else
			{
				lString = "Multi-protocol Encap";
			}
		}
		break;
	case 0x0B:
		{
			if (LanguageCode == 0)
			{
				lString = "DSM-CC U-N Msg";
			}
			else
			{
				lString = "DSM-CC U-N Msg";
			}
		}
		break;
	case 0x0C:
		{
			if (LanguageCode == 0)
			{
				lString = "DSM-CC Stream Desc";
			}
			else
			{
				lString = "DSM-CC Stream Desc";
			}
		}
		break;
	case 0x0D:
		{
			if (LanguageCode == 0)
			{
				lString = "DSM-CC Sections";
			}
			else
			{
				lString = "DDSM-CC Sections";
			}
		}
		break;
	case 0x10:
		{
			if (LanguageCode == 0)
			{
				lString = "MPEG-4 视频";
			}
			else
			{
				lString = "MPEG-4 Video";
			}
		}
		break;
	case 0x0F:
	case 0x11:
		{
			if (LanguageCode == 0)
			{
				lString = "AAC 音频";
			}
			else
			{
				lString = "AAC Audio";
			}
		}
		break;
	case 0x12:
		{
			if (LanguageCode == 0)
			{
				lString = "AVS 音频";
			}
			else
			{
				lString = "AVS Audio";
			}
		}
		break;
	case 0x1B:
		{
			if (LanguageCode == 0)
			{
				lString = "H.264 视频";
			}
			else
			{
				lString = "H.264 Video";
			}
		}
		break;
	default:
		{
			if (TypeIndex < 0x7F) 
			{
				if(LanguageCode == 0)
				{
					lString = "未来使用预留";
				}
				else
				{
					lString = "Reserved for Future";
				}
			}
			else
			{
				if(LanguageCode == 0)
				{
					lString = "用户自定义";
				}
				else
				{
					lString = "User defined";
				}
			}
		}
		break;
	}
	if (bIndex) 
	{
		return (lString + "(" + TypeIndex + ")");
	}
	return lString;
}



function JS_MPEG2GetServiceType(TypeIndex, LanguageCode, bIndex)
{
	var lString;
	switch(TypeIndex)
	{
	case 0x01:
		if(LanguageCode == 0)
		{
			lString = "数字电视";
		}
		else
		{
			lString = "Digital TV";
		}
		break;
	case 0x02:
		if(LanguageCode == 0)
		{
			lString = "数字音频广播";
		}
		else
		{
			lString = "Digital Radio";
		}
		break;
	case 0x03:
		if(LanguageCode == 0)
		{
			lString = "图文电视";
		}
		else
		{
			lString = " Teletext";
		}
		break;
	case 0x04:
		if(LanguageCode == 0)
		{
			lString = "准实时点播参考";
		}
		else
		{
			lString = "NVOD Reference";
		}
		break;
	case 0x05:
		if(LanguageCode == 0)
		{
			lString = "准实时点播时移";
		}
		else
		{
			lString = "NVOD Time-shifted";
		}
		break;
	case 0x06:
		if(LanguageCode == 0)
		{
			lString = "马赛克";
		}
		else
		{
			lString = "Mosaic";
		}
		break;
	case 0x07:
		if(LanguageCode == 0)
		{
			lString = "PAL制式编码信号";
		}
		else
		{
			lString = "PAL coded signal";
		}
		break;
	case 0x08:
		if(LanguageCode == 0)
		{
			lString = "SECAM制式编码信号";
		}
		else
		{
			lString = "SECAM coded signal";
		}
		break;
	case 0x09:
		if(LanguageCode == 0)
		{
			lString = "D/D2-MAC";
		}
		else
		{
			lString = "D/D2-MAC";
		}
		break;
	case 0x0A:
		if(LanguageCode == 0)
		{
			lString = "调频广播";
		}
		else
		{
			lString = "FM Radio";
		}
		break;
	case 0x0B:
		if(LanguageCode == 0)
		{
			lString = "NTSC制式编码信号";
		}
		else
		{
			lString = "NTSC coded signal";
		}
		break;
	case 0x0C:
		if(LanguageCode == 0)
		{
			lString = "数据广播";
		}
		else
		{
			lString = "Data Broadcast";
		}
		break;
	case 0x0D:
		if(LanguageCode == 0)
		{
			lString = "公共接口使用预留";
		}
		else
		{
			lString = "Reserved";
		}
		break;
	case 0x0E:
		if(LanguageCode == 0)
		{
			lString = "RCS 映射";
		}
		else
		{
			lString = "RCS Map";
		}
		break;
	case 0x0F:
		if(LanguageCode == 0)
		{
			lString = "RCS FLS";
		}
		else
		{
			lString = "RCS FLS";
		}
		break;
	case 0x10:
		if(LanguageCode == 0)
		{
			lString = "DVB MHP";
		}
		else
		{
			lString = "DVB MHP";
		}
		break;
	case 0x11:
		if(LanguageCode == 0)
		{
			lString = "MPEG-2高清数字电视";
		}
		else
		{
			lString = "MPEG-2 HD Digital TV";
		}
		break;
	case 0x16:
		if(LanguageCode == 0)
		{
			lString = "标清数字电视（高级编码）";
		}
		else
		{
			lString = "Advanced Codec SD Digital TV";
		}
		break;
	case 0x17:
		if(LanguageCode == 0)
		{
			lString = "标清准实时点播时移（高级编码）";
		}
		else
		{
			lString = "Advanced Codec SD NVOD Time-shifted";
		}
		break;
	case 0x18:
		if(LanguageCode == 0)
		{
			lString = "标清准实时点播参考（高级编码）";
		}
		else
		{
			lString = "Advanced Codec SD NVOD Reference";
		}
		break;
	case 0x19:
		if(LanguageCode == 0)
		{
			lString = "高清数字电视（高级编码）";
		}
		else
		{
			lString = "Advanced Codec HD Digital TV";
		}
		break;
	case 0x1A:
		if(LanguageCode == 0)
		{
			lString = "高清准实时点播时移（高级编码）";
		}
		else
		{
			lString = "Advanced Codec HD NVOD Time-shifted";
		}
		break;
	case 0x1B:
		if(LanguageCode == 0)
		{
			lString = "高清准实时点播参考（高级编码）";
		}
		else
		{
			lString = "Advanced Codec HD NVOD Reference";
		}
		break;
	default:
		{
			if (TypeIndex < 0x7F) 
			{
				if(LanguageCode == 0)
				{
					lString = "未来使用预留";
				}
				else
				{
					lString = "Reserved";
				}
			}
			else
			{
				if(LanguageCode == 0)
				{
					lString = "用户自定义";
				}
				else
				{
					lString = "User defined";
				}
			}
		}
		break;
	}
	if (bIndex) 
	{
		return (lString + "(" + TypeIndex + ")");
	}
	return lString;
}

function JS_MPEG2GetPSISIType(PID, TableID, LanguageCode, bIndex)
{
	var lString = null;
	if (PID == 0) 
	{
		lString = "PAT";
	}
	else if (PID == 0x01) 
	{
		lString = "CAT";
	}
	else if (PID == 0x02) 
	{
		lString = "TSDT";
	}
	else if (PID == 0x10) 
	{
		if (TableID == null) 
		{
			lString = "NIT";
		}
		else if (TableID == 0x40) 
		{
			lString = "NIT(Actual)";
		}
		else if (TableID == 0x41) 
		{
			lString = "NIT(Other)";
		}
	}
	else if (PID == 0x11) 
	{
		if (TableID == null) 
		{
			lString = "SDT/BAT";
		}
		else if (TableID == 0x42) 
		{
			lString = "SDT(Actual)";
		}
		else if (TableID == 0x46) 
		{
			lString = "SDT(Other)";
		}
		else if (TableID == 0x4A) 
		{
			lString = "BAT";
		}
	}
	else if (PID == 0x12) 
	{
		if (TableID == null) 
		{
			lString = "EIT";
		}
		else if (TableID == 0x4E) 
		{
			lString = "EIT P/F(Actual)";
		}
		else if (TableID == 0x4F) 
		{
			lString = "EIT P/F(Other)";
		}
		else if ((TableID >= 0x50) || (TableID <= 0x5F)) 
		{
			lString = "EIT Schdule(Actual)";
		}
		else if ((TableID >= 0x60) || (TableID <= 0x6F)) 
		{
			lString = "EIT Schdule(Actual)";
		}
	}
	else if (PID == 0x14) 
	{
		if (TableID == null) 
		{
			lString = "TDT/TOT";
		}
		else if (TableID == 0x71) 
		{
			lString = "TDT";
		}
		else if (TableID == 0x73) 
		{
			lString = "TOT";
		}
	}
	else if (PID == 0x1FFF) 
	{
		if (LanguageCode == 0) 
		{
			lString = "空包";
		}
		else
		{
			lString = "STUFFING PACKE";
		}
	}
	else
	{
		if (TableID == 0x02) 
		{
			lString = "PMT";
		}
	}
	
	if (lString != null) 
	{
		if (bIndex) 
		{
			lString = "(" + PID + ")";
		}
	}
	
	return lString ;
}


function JS_MPEG2CalculateDVBCSS2MaxBitrate(Symrate, Constellation, IC) 
{
	var lTmpRM;
	var lTmpIC;
	switch (Constellation)
	{
		case "QPSK":
			lTmpRM = 2;
			break;
		case "4QAM":
			lTmpRM = 2;
			break;
		case "8PSK":
			lTmpRM = 3;
			break;
		case "16QAM":
			lTmpRM = 4;
			break;
		case "32QAM":
			lTmpRM = 5;
			break;
		case "64QAM":
			lTmpRM = 6;
			break;
		case "128QAM":
			lTmpRM = 7;
			break;
		case "256QAM":
			lTmpRM = 8;
			break;
		default:
			lTmpRM = 4;
			break;
	}
	
	switch (IC)
	{
		case "1/2":
			lTmpIC = 1 / 2;
			break;
		case "2/3":
			lTmpIC = 2 / 3;
			break;
		case "3/5":
			lTmpIC = 3 / 5;
			break;
		case "3/4":
			lTmpIC = 3 / 4;
			break;
		case "4/5":
			lTmpIC = 4 / 5;
			break;
		case "5/6":
			lTmpIC = 5 / 6;
			break;
		case "7/8":
			lTmpIC = 7 / 8;
			break;
		case "8/9":
			lTmpIC = 8 / 9;
			break;
		case "9/10":
			lTmpIC = 9 / 10;
			break;
		default:
			lTmpIC = 1;
			break;
	}
	return Symrate * lTmpRM * lTmpIC * 188 / 204;
}


function JS_MPEG2CalculateDVBTMaxBitrate(GI, Constellation, IC)
{
	var lTmpRM;
	var lTmpIC;
	var lTmpGIM;

	
	switch (GI)
	{
		case "1/4":
			lTmpGIM = 1/4;
			break;
		case "1/8":
			lTmpGIM = 1/8;
			break;
		case "1/16":
			lTmpGIM = 1/16;
			break;
		case "1/32":
			lTmpGIM = 1/32;
			break;
		default:
			lTmpGIM = 1;
			break;
	}

	lTmpGIM = 1 / (1 + lTmpGIM);
	
	switch (Constellation)
	{
		case "QPSK":
			lTmpRM = 2;
			break;
		case "16QAM":
			lTmpRM = 4;
			break;
		case "64QAM":
			lTmpRM = 6;
			break;
		default:
			lTmpRM = 1;
			break;
	}

	switch (IC)
	{
		case "1/2":
			lTmpIC = 1 / 2;
			break;
		case "2/3":
			lTmpIC = 2 / 3;
			break;
		case "3/4":
			lTmpIC = 3 / 4;
			break;
		case "5/6":
			lTmpIC = 5 / 6;
			break;
		case "7/8":
			lTmpIC = 7 / 8;
			break;
		default:
			lTmpIC = 1;
			break;
	}
	
	return 6750000 * lTmpRM * lTmpGIM * lTmpIC * 188 / 204;
}



function JS_MPEG2CalculateISDBTMaxBitrate(GI, Constellation, IC, SegNum)
{
	var lTmpRM;
	var lTmpIC;
	var lTmpGIM;


	switch (GI)
	{
		case "1/4":
			lTmpGIM = 1 / 4;
			break;
		case "1/8":
			lTmpGIM = 1 / 8;
			break;
		case "1/16":
			lTmpGIM = 1 / 16;
			break;
		case "1/32":
			lTmpGIM = 1 / 32;
			break;
		default:
			lTmpGIM = 1;
			break;
	}

	lTmpGIM = 1 / (1 + lTmpGIM);

	switch (Constellation)
	{
		case "QPSK":
			lTmpRM = 2;
			break;
		case "DQPSK":
			lTmpRM = 2;
			break;
		case "16QAM":
			lTmpRM = 4;
			break;
		case "64QAM":
			lTmpRM = 6;
			break;
		default:
			lTmpRM = 1;
			break;
	}

	switch (IC)
	{
		case "1/2":
			lTmpIC = 1 / 2;
			break;
		case "2/3":
			lTmpIC = 2 / 3;
			break;
		case "3/4":
			lTmpIC = 3 / 4;
			break;
		case "5/6":
			lTmpIC = 5 / 6;
			break;
		case "7/8":
			lTmpIC = 7 / 8;
			break;
		default:
			lTmpIC = 1;
			break;
	}

	return 380952 * lTmpRM * lTmpGIM * lTmpIC * 188 / 204 * SegNum;
}
function JS_MPEG2CalculateDTMBMaxBitrate(PN, Constellation, IC)
{
	var lTmpRM;
	switch (Constellation)
	{
		case "4QAM-NR":
			lTmpRM = 1;
			break;
		case "4QAM":
			lTmpRM = 2;
			break;
		case "16QAM":
			lTmpRM = 4;
			break;
		case "32QAM":
			lTmpRM = 5;
			break;
		case "64QAM":
			lTmpRM = 6;
			break;
		default:
			lTmpRM = 6;
			break;
	}
	
	return (3744 + 16) / (parseInt(PN) + 3780) * parseFloat(IC) * lTmpRM * 7560000;
}
