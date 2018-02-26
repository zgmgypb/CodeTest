#ifndef GN_HWL_SDI_H
#define GN_HWL_SDI_H

#include "gn_global.h"

#define IOCTOL_MAGIC 'l'
#define SPI_RESET		_IOW(IOCTOL_MAGIC, 0,char) 
#define IOCTL_MAXNE 1

#define FPGA_SPI_CTRL_SRC4193_RESET		0x01
#define FPGA_SPI_CTRL_SRC4193_BYPASS	0x02
#define FPGA_SPI_CTRL_SRC4193_MUTE		0x04

#define FPGA_SPI_CTRL_SRC4193_CS		0x08
#define FPGA_SPI_CTRL_SRC4193_CLK		0x10
#define FPGA_SPI_CTRL_SRC4193_DATA		0x20

#define CVBS_SDI_CH		4	/* CVBS/SDI输入通道数目 */
#define MAXCHANNEL		CVBS_SDI_CH /* CVBS/SDI的输入通道数目，在CVBS和SDI模块中使用 */
#define GN_RELEASE_DATE_STRING_BUF_LEN	20	 /* 形如"2012-03-30 12:20:59" */

//接口定义
typedef struct
{
	int sdi_embed_ch;
}SDI_Embed_CH;

typedef struct
{
	char soft_version[16];
	char hard_version[16];
}SDI_Version;

typedef struct
{
	int SdiCdStatus;
	int SdiFormat;
	int SdiAudioEmbStatus;
	int SdiSrc4193Status;
}SDI_SPI_Status;

typedef struct 
{
	BOOL (*m_pFpgaConfig)(void);
	BOOL (*m_pFpgaReset)(void);
	int (*m_pFpgaOpen)(void);
	int (*m_pFpgaClose)(int Fd);
	int (*m_pFpgaRead)(int Fd, unsigned int RegAddr, char *pData, unsigned int DataLen);
	int (*m_pFpgaWrite)(int Fd, unsigned int RegAddr, char *pData, unsigned int DataLen);
}HWL_SdiInitPara;

/* SDI输入子板接口 */
typedef struct 
{
	int m_sdi_embed_ch; /* 0~7 */
}HWL_SdiConfigPara;

typedef struct 
{
	char m_FpgaRelease[GN_RELEASE_DATE_STRING_BUF_LEN];

	int m_sdi_cd_status; /* 信号锁定状态 0:unlock    1:lock */
	int m_sdi_format; /* 视频制式 0:PAL    1:NTSC */
	int m_sdi_audio_emb_status; /* 判断设置的嵌入通道号是否有声音嵌入  1/0   正确/错误 */
	int m_sdi_src4193_status; /* 读取SRC4193的工作状态；返回1，正常；0: 错误 */
}HWL_SdiStatusPara;

SDI_Embed_CH Get_SDI_Embed(int ch_num);
SDI_Version Get_SDI_Version(int ch_num);
SDI_SPI_Status Get_SDI_SPI_Status(int ch_num);

void Set_SDI_Embed(int ch_num, SDI_Embed_CH val);
void Set_SDI_Version(int ch_num, SDI_Version val);
void Set_SDI_SPI_Status(int ch_num);

#endif /* GN_HWL_SDI_H */
/* EOF */
