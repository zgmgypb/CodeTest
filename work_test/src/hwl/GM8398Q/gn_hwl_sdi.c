#include "gn_hwl_sdi.h"

static int SpiFdCh = -1;
HWL_SdiInitPara *gInitSDIPara = NULL;

static SDI_Version			 		s_SdiVersionString[MAXCHANNEL];
static SDI_Embed_CH					s_SdiEmbed[MAXCHANNEL];
static SDI_SPI_Status			 	s_SdiSpiStatus[MAXCHANNEL];

//以下为协议里定义的命令
//读取
U8 SdiSoftVersion[2];			//查询软件版本
U8 SdiHardVersion[2]; 			//查询硬件版本
U8 SdiFormat[2]; 				//查询制式
U8 SdiAudioEmbStatus[2]; 		//查询嵌入音频状态
U8 SdiCdStatus[2]; 				//查询CD状态
U8 SdiSrc4193Status[2];			//查询SRC4193状态
//设置
U8 SdiAudioChannelSet[4]; 			//设置通道的音频组号和声道号
U8 SdiSrc4193Reg[7]; 			//设置SRC4193 register Value;
U8 SdiSrc4193Set[4]; 			//设置SRC4193

void SdiGetSoftVersion(U8 subboard_channel);
void SdiGetHardwareVersion(U8 subboard_channel);
unsigned int SdiGetFormat(U8 sdi_channel);
unsigned int SdiGetAudioEmbStatus(int sdi_channel);
unsigned int SdiGetCdStatus(U8 sdi_channel);
unsigned int SdiGetSrc4193Status(U8 sdi_channel);
unsigned int SdiGetFpgaStatus(U8 sdi_channel);

void SdiSetPara();
void SdiSetVersion(int ch_num, SDI_Version val);

void SdiIninCommand(void)
{
	//get
	SdiSoftVersion[0] = 0x80;
	SdiSoftVersion[1] = 0x00;

	SdiHardVersion[0] = 0x80;
	SdiHardVersion[1] = 0x01;

	SdiFormat[0] = 0xC0;
	SdiFormat[1] = 0x00;

	SdiAudioEmbStatus[0] = 0xC0;
	SdiAudioEmbStatus[1] = 0x01;

	SdiCdStatus[0] = 0xC0;
	SdiCdStatus[1] = 0x02;

	SdiSrc4193Status[0] = 0xC0;
	SdiSrc4193Status[1] = 0x03;

	//set
	SdiAudioChannelSet[0] = 0x40;
	SdiAudioChannelSet[1] = 0x00;
	SdiAudioChannelSet[2] = 0x00;
	SdiAudioChannelSet[3] = 0x00;

	SdiSrc4193Set[0] = 0x40;		
	SdiSrc4193Set[1] = 0x01;
	SdiSrc4193Set[2] = 0x00;
	SdiSrc4193Set[3] = 0x00;

	SdiSrc4193Reg[0] = 0x01 ;
	SdiSrc4193Reg[1] = 0xFF ;	
	SdiSrc4193Reg[2] = 0x03 ;
	SdiSrc4193Reg[3] = 0x00 ;
	SdiSrc4193Reg[4] = 0x11 ;
	SdiSrc4193Reg[5] = 0x00 ;
	SdiSrc4193Reg[6] = 0x00 ;	
}

int ReadSdiStatusOneTime(U8 sdi_channel,U8 *type)
{
	int Value = 0;
	int tmp_channel = sdi_channel;

	U8 cmd[4];
	unsigned int regaddr = 0;
	regaddr = ((type[0]&0xFF)<<8)|(type[1]&0xFF);

	//如果要访问的是0~3通道，操作1号子板
	if( (tmp_channel>=0)&&(tmp_channel <4) )
	{
		//如果没有打开过spi_fd_ch0，重新打开一次
		if( SpiFdCh <= 0 )
		{
			SpiFdCh = (*gInitSDIPara->m_pFpgaOpen)();

			if(SpiFdCh < 0)
			{
				GLOBAL_TRACE(("Open dev spi driver error!\n"));
			}	
		}

		(*gInitSDIPara->m_pFpgaRead)(SpiFdCh,regaddr,cmd,2);
		Value = (cmd[0]<<8)|cmd[1];

		if( Value < 0 )
		{
			GLOBAL_TRACE(("Read sdi error!\n"));
		}
	}
	else
	{
		GLOBAL_TRACE(("Read sdi ch error!\n"));
		Value = -1;
	}

	usleep(1000);
	return Value;	
}

// 连续读两次，均是相同的数据后，就认为是正确的；如果错误，再连续读5次；
int ReadSdiStatus(U8 sdi_channel,U8 *type)
{
	int temp_data[2] ;
	int counter ;

	counter = 5 ;
	while(counter)
	{
		temp_data[0] = ReadSdiStatusOneTime(sdi_channel, type) ;
		temp_data[1] = ReadSdiStatusOneTime(sdi_channel, type) ;

		if(	temp_data[0] == temp_data[1])
		{
			counter = 0 ;
		}
		else
		{
			counter-- ;
			if(counter == 0)
			{
				GLOBAL_TRACE(("Read Sdi ch%d error!\n", sdi_channel)) ;
			}
		}
	}

	return temp_data[0] ;
}

void WriteSdiStatusOneCommand(U8 sdi_channel,U8 *command)
{
	int Value = 0;
	int tmp_channel = sdi_channel;
	unsigned int regaddr = 0;
	U8 Data[2];

	regaddr = ((command[0]&0xFF)<<8)|(command[1]&0xFF);

	Data[0] = command[2]&0xFF;
	Data[1] = command[3]&0xFF;

	if( (tmp_channel>=0)&&(tmp_channel <4) )
	{
		Value = (*gInitSDIPara->m_pFpgaWrite)(SpiFdCh, regaddr, Data, 2);

		if( Value < 0 )
		{
			GLOBAL_TRACE(("Write sdi error!\n"));
		}
	}
	else
	{
		GLOBAL_TRACE(("Write sdi ch error!\n"));
	}
}

U8 WriteSdiStatusAll(U8 sdi_channel,U8 *command)
{
	U8 temp_command[2];
	U8 read_command[4];
	U8 counter ;
	int temp_read, write_data ;

	temp_command[0] = *command;
	temp_command[1] = *(command + 1) ;
	write_data = *(command + 2) ;
	write_data = write_data << 8 ;
	write_data = write_data | *(command + 3) ;

	switch(temp_command[1])
	{
	case 0x00 :		// read sdi audio embed channle Value ;
		read_command[0] = 0xC0 | temp_command[0] ; 
		read_command[1] = 0x06 ; 
		read_command[2] = 0x00 ; 
		read_command[3] = 0x00 ; 
		break ;

	case 0x01 :		// read sdi audio 4193 control Value ;
		read_command[0] = 0xC0 ; 
		read_command[1] = 0x07 ; 
		read_command[2] = 0x00 ; 
		read_command[3] = 0x00 ; 
		break ;

	case 0x05 :		// read sdi test data Value ;
		read_command[0] = 0xC0 ; 
		read_command[1] = 0x05 ; 
		read_command[2] = 0x00 ; 
		read_command[3] = 0x00 ; 
		break ;

	default :
		break ;
	}


	counter = 5 ;
	while(counter)
	{
		WriteSdiStatusOneCommand(sdi_channel, command) ;

		temp_read = ReadSdiStatus(sdi_channel, read_command) ;
		GLOBAL_TRACE(("Wdata=0x%x,Rdata=0x%x.\n", write_data, temp_read)) ;

		if(write_data == temp_read)
		{
			counter = 0 ;
		}
		else
		{
			counter-- ;
			if(counter == 0)
			{
				GLOBAL_TRACE(("Write Sdi ch%d error!\n", sdi_channel)) ;
				return 0 ;
			}
		}
	}

	return 1 ;
}

//根据协议做成
void SdiChannelSelect(U8 *new_command,U8 *ori_command,int channel)
{
	new_command[0] = ori_command[0]|channel;
	new_command[1] = ori_command[1];
}

//加入subboard_channel参数，以后加入检测输入子板是否为sdi功能可使用
void SdiGetSoftVersion(U8 subboard_channel)
{
	SDI_Version	 temp_SdiSoftVersionString[MAXCHANNEL];	
	U8 ret[16];
	U8 sdi_channel;
	unsigned int r_sdi_soft_version;

	int year;
	int month;
	int day;

	memset(ret, 0, sizeof(ret));

	//temp_SdiSoftVersionString[0] = SdiGetVersion(0);
	strcpy(temp_SdiSoftVersionString[0].soft_version, s_SdiVersionString[0].soft_version);

	sdi_channel = 4*subboard_channel;
	r_sdi_soft_version = ReadSdiStatus(sdi_channel,SdiSoftVersion);

	year = (r_sdi_soft_version>>9)& 0x0F;
	year = year + 9;
	month = (r_sdi_soft_version>>5)&0x0F;
	day = r_sdi_soft_version & 0x1F;

	snprintf(ret, sizeof(ret), "%02d:%02d:%02d", year,month, day);
	//PDEBUG("%02d:%02d:%02d\n", year,month, day);
	strcpy(temp_SdiSoftVersionString[0].soft_version,ret);
	SdiSetVersion(0,temp_SdiSoftVersionString[0]);
}

//加入subboard_channel参数，以后加入检测输入子板是否为sdi功能可使用
void SdiGetHardwareVersion(U8 subboard_channel)
{
	SDI_Version	 temp_SdiHardVersionString[MAXCHANNEL];	
	U8 ret[16];
	U8 sdi_channel;
	unsigned int r_sdi_hard_version;

	//temp_SdiHardVersionString[0] = SdiGetVersion(0);
	strcpy(temp_SdiHardVersionString[0].hard_version, s_SdiVersionString[0].hard_version);

	sdi_channel = 4*subboard_channel;
	r_sdi_hard_version = ReadSdiStatus(sdi_channel,SdiHardVersion);

	snprintf(ret, sizeof(ret),"%02d.%02d",((r_sdi_hard_version>>4)&0x0F),(r_sdi_hard_version&0x0F));  
	strcpy(temp_SdiHardVersionString[0].hard_version,ret);
	SdiSetVersion(0,temp_SdiHardVersionString[0]);
}

//in :sdi_channel 0~3
//return: 0:PAL    1:NTSC
unsigned int SdiGetFormat(U8 sdi_channel)
{
	unsigned int r_sdi_format;
	U8 channel_sdi_format[2];

	SdiChannelSelect(channel_sdi_format, SdiFormat, sdi_channel);
	r_sdi_format = ReadSdiStatus(sdi_channel,channel_sdi_format);
	r_sdi_format = r_sdi_format & 0x0001;
	return r_sdi_format;
}

//in: sdi_channel  0~3
//out:  1/0   正确/错误
// 判断设置的嵌入通道号是否有声音嵌入；
unsigned int SdiGetAudioEmbStatus(int sdi_channel)
{
	unsigned int r_sdi_audio_emb_status ;
	U8 channel_sdi_audio_emb_status[2] ;

	SDI_Embed_CH inChannel[MAXCHANNEL];

	SdiChannelSelect(channel_sdi_audio_emb_status, SdiAudioEmbStatus, sdi_channel);
	r_sdi_audio_emb_status = ReadSdiStatus(sdi_channel,channel_sdi_audio_emb_status);
	//PDEBUG("r_sdi_audio_emb_status 1st  %x\n",r_sdi_audio_emb_status);

	r_sdi_audio_emb_status = r_sdi_audio_emb_status & 0xFFFF;

	//inChannel[sdi_channel] = SdiGetEmbed(sdi_channel);		// 得到设置的嵌入通道号；
	inChannel[sdi_channel] = s_SdiEmbed[sdi_channel];

	if(inChannel[sdi_channel].sdi_embed_ch >= 8)			// 判断是否超过8个嵌入通道；
	{
		inChannel[sdi_channel].sdi_embed_ch = 0 ;
	}
	if((r_sdi_audio_emb_status >> (inChannel[sdi_channel].sdi_embed_ch * 2)) & 0x0003)
	{
		r_sdi_audio_emb_status = 1 ;
		//PDEBUG("ch%d auido embed ok, %x\n", sdi_channel, r_sdi_audio_emb_status);
	}
	else
	{
		r_sdi_audio_emb_status = 0 ;
		//PDEBUG("ch%d no auido embed, %x\n", sdi_channel, r_sdi_audio_emb_status);
	}

	//PDEBUG("r_sdi_audio_emb_status 2nd  %x\n",r_sdi_audio_emb_status);
	return r_sdi_audio_emb_status;
}

//return: 0:unlock    1:lock
unsigned int SdiGetCdStatus(U8 sdi_channel)
{
	unsigned int r_sdi_cd_status;
	U8 channel_sdi_cd_status[2];

	SdiChannelSelect(channel_sdi_cd_status, SdiCdStatus, sdi_channel);

	r_sdi_cd_status = ReadSdiStatus(sdi_channel,channel_sdi_cd_status);
	r_sdi_cd_status = r_sdi_cd_status & 0x0001;
	return r_sdi_cd_status;
}

//读取SRC4193的工作状态；返回1，正常；
unsigned int SdiGetSrc4193Status(U8 sdi_channel)
{
	unsigned int r_sdi_src4193_status;
	U8 channel_sdi_src4193_status[2];

	SdiChannelSelect(channel_sdi_src4193_status, SdiSrc4193Status, sdi_channel);
	r_sdi_src4193_status = ReadSdiStatus(sdi_channel,channel_sdi_src4193_status);

	r_sdi_src4193_status = r_sdi_src4193_status & 0x0003;//根据协议最低2位为Bit 0  RATIO、Bit1   RDY
	//PDEBUG("channel %d  SdiFormat %d\n",sdi_channel,r_sdi_src4193_status);
	if(r_sdi_src4193_status & 0x0002)
	{
		return 0 ;	// work status error; 
	}
	else
	{
		return 1 ;	// work status ok; 
	}
}

//in : subboard_channel  0/4
unsigned int SdiGetFpgaStatus(U8 sdi_channel)
{
	U8 command[4];
	unsigned int i;
	command[0] = 0xC0;
	command[1] = 0x04;
	command[2] = 0x00;
	command[3] = 0x00;
	i = ReadSdiStatus(sdi_channel,command);
	//PDEBUG("sdi_channel %d is 0x%x\n",sdi_channel,i);
	return i;
}

//in: sdi_channel  0~3   audio_channel 0~3
void SdiSetAudioEmbChannel(U8 sdi_channel,U8 audio_channel)
{
	U8 command[4];
	U8 counter ;

	command[0] = SdiAudioChannelSet[0]|sdi_channel;
	command[1] = SdiAudioChannelSet[1];
	command[2] = SdiAudioChannelSet[2];

	switch(audio_channel)		// set embed channel select; 
	{
		case 0 :
			command[3] = 0x01 ;		// embed ch1 ;
			break ;
		case 1 :
			command[3] = 0x23 ;		// embed ch2 ;
			break ;
		case 2 :
			command[3] = 0x45 ;		// embed ch3 ;
			break ;
		case 3 :
			command[3] = 0x67 ;		// embed ch4 ;
			break ;
		case 4 :
			command[3] = 0x89 ;		// embed ch5 ;
			break ;
		case 5 :
			command[3] = 0xAB ;		// embed ch6 ;
			break ;
		case 6 :
			command[3] = 0xCD ;		// embed ch7 ;
			break ;
		case 7 :
			command[3] = 0xEF ;		// embed ch8 ;
			break ;
		default:
			command[3] = 0x01 ;		// embed ch1 ;
			break ;
	}

	counter = 5 ;
	while(counter)
	{
		if(WriteSdiStatusAll(sdi_channel,command) == 1)
		{
			counter = 0 ;
			GLOBAL_TRACE(("write SDI embed ch%d sucess------------------------->!\n", sdi_channel)) ;
		}
		else
		{
			counter-- ;
			if(counter == 0)
			{
				GLOBAL_TRACE(("write SDI embed ch%d error------------------------->!\n", sdi_channel)) ;
			}
		}
	}
}

// write to signal Value to src4193 register; 
void SdiSetSrc4193Register(U8 sdi_channel, U8 reg_value)
{
	U8 command[4];

	command[0] = SdiSrc4193Set[0];
	command[1] = SdiSrc4193Set[1];
	command[2] = SdiSrc4193Set[2];
	command[3] = SdiSrc4193Set[3] | reg_value ;

	WriteSdiStatusAll(sdi_channel, command);		// write to SDI fpga; 
}

// 写4193控制寄存器；4193写入的数据在FPGA写入；
void SdiSetSrc4193Config(U8 sdi_channel)
{
	U8 command[4] ;
	U8 counter, error ; 

	command[0] = 0x40 ;
	command[1] = 0x01 ;
	command[2] = 0x00 ;
	command[3] = 0x00 & (~FPGA_SPI_CTRL_SRC4193_BYPASS) & (~FPGA_SPI_CTRL_SRC4193_MUTE) ; 

	counter = 5 ; 
	while(counter)
	{
		error = 0 ;

		command[3] = command[3] & (~FPGA_SPI_CTRL_SRC4193_RESET) ;
		if(WriteSdiStatusAll(sdi_channel, command) == 0)
		{
			error++ ;
		}

		sleep(1) ;
		command[3] = command[3] | FPGA_SPI_CTRL_SRC4193_RESET ;
		if(WriteSdiStatusAll(sdi_channel, command) == 0)
		{
			error++ ;
		}

		if(error == 0)
		{
			counter = 0 ;
			GLOBAL_TRACE(("Write Sdi src4193 ch%d sucess.\n", sdi_channel)) ;
		}
		else
		{
			counter-- ;
			if(counter == 0)
			{
				GLOBAL_TRACE(("Write Sdi src4193 ch%d error!\n", sdi_channel)) ;
			}
		}
	}
}

//in:   subboard_channel: 0~1
//和1898做法匹配; 
// 设置参数：1:4个音频嵌入通道选择；    2：复位SRC4193，让FPGA写数据到4193；
void SdiSetPara()
{
	SDI_Embed_CH inChannel[MAXCHANNEL];
	int i;
	U8 sdi_channel;

	for (i = 0;i<MAXCHANNEL;i++)
	{
		//inChannel[i] = SdiGetEmbed(i);
		inChannel[i] = s_SdiEmbed[i];
		SdiSetAudioEmbChannel(i,inChannel[i].sdi_embed_ch);
	}	

	sdi_channel = 0;
	SdiSetSrc4193Config(sdi_channel) ;		// 写4193控制寄存器；4193写入的数据在FPGA写入；
}

SDI_Embed_CH SdiGetEmbed(int ch_num)
{
	return s_SdiEmbed[ch_num];
}

void SdiSetEmbed(int ch_num, SDI_Embed_CH val)
{
	s_SdiEmbed[ch_num].sdi_embed_ch = val.sdi_embed_ch;
}

SDI_Version SdiGetVersion(int ch_num)
{
	return s_SdiVersionString[ch_num];
}

void SdiSetVersion(int ch_num, SDI_Version val)
{
	strcpy(s_SdiVersionString[ch_num].soft_version,val.soft_version);
	strcpy(s_SdiVersionString[ch_num].hard_version,val.hard_version);
}

void SdiSetSpiStatus(int ch_num)
{
	s_SdiSpiStatus[ch_num].SdiCdStatus = SdiGetCdStatus(ch_num);
	//PDEBUG("ch %d check %d      ,",ch_num,s_SdiSpiStatus[ch_num].SdiCdStatus);

	s_SdiSpiStatus[ch_num].SdiFormat = SdiGetFormat(ch_num);
	//PDEBUG("ch %d format %d          ,",ch_num,s_SdiSpiStatus[ch_num].SdiFormat);

	s_SdiSpiStatus[ch_num].SdiAudioEmbStatus = SdiGetAudioEmbStatus(ch_num);
	//PDEBUG("ch%d audio embed %d,       ", ch_num, s_SdiSpiStatus[ch_num].SdiAudioEmbStatus);

	s_SdiSpiStatus[ch_num].SdiSrc4193Status = SdiGetSrc4193Status(ch_num);
	//PDEBUG("ch%d audio 4193 %d,       \n", ch_num, s_SdiSpiStatus[ch_num].SdiSrc4193Status);	
}

SDI_SPI_Status SdiGetSpiStatus(int ch_num)
{
	return s_SdiSpiStatus[ch_num];
}

BOOL HWL_SdiInit(HWL_SdiInitPara *InitPara)
{
	gInitSDIPara = malloc( sizeof(HWL_SdiInitPara));
	memcpy(gInitSDIPara,InitPara,sizeof(HWL_SdiInitPara));

	SpiFdCh = (*InitPara->m_pFpgaOpen)(); 

	if( SpiFdCh < 0)
	{
		GLOBAL_TRACE(("open spi_driver fail!\n"));

		return FALSE;
	}
	else
	{
		GLOBAL_TRACE(("open spi_driver success\n"));
	}

	if( (*InitPara->m_pFpgaReset)() == 0 )
	{
		return FALSE;
	}

	if( (*InitPara->m_pFpgaConfig)() == 0 )
	{
		return FALSE;
	}

	SdiIninCommand();
	SdiGetSoftVersion(0);
	SdiGetHardwareVersion(0);

	return TRUE;
}

BOOL HWL_SdiTerminate(void)
{
	(*gInitSDIPara->m_pFpgaClose)(SpiFdCh);

	free(gInitSDIPara);

	return TRUE;
}

BOOL HWL_SdiSetPara(HWL_SdiConfigPara ConfigPara[MAXCHANNEL])
{
	int i;

	for( i = 0; i < MAXCHANNEL;  i++ )
	{
		SDI_Embed_CH inChannel;
		inChannel.sdi_embed_ch = ConfigPara[i].m_sdi_embed_ch;

		SdiSetEmbed(i, inChannel);
	}

	gInitSDIPara->m_pFpgaReset();
	SdiSetPara();

	return TRUE;
}

BOOL HWL_SdiGetPara(HWL_SdiStatusPara StatusPara[MAXCHANNEL])
{
	int i;

	for( i = 0; i < MAXCHANNEL;  i++ )
	{
		SdiSetSpiStatus(i);
		SdiGetSpiStatus(i);

		StatusPara[i].m_sdi_cd_status = s_SdiSpiStatus[i].SdiCdStatus;
		StatusPara[i].m_sdi_format = s_SdiSpiStatus[i].SdiFormat;
		StatusPara[i].m_sdi_audio_emb_status = s_SdiSpiStatus[i].SdiAudioEmbStatus;
		StatusPara[i].m_sdi_src4193_status = s_SdiSpiStatus[i].SdiSrc4193Status;
	}

	return TRUE;
}
/* EOF */
