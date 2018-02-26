#include "linuxplatform.h"
#include "gn_hwl.h"
#include "gn_hwl_cvbs.h"
#include "gn_drv.h"

#define I2C_IOCTL_MAGIE 'I'
#define I2C_IOCTL_MAXNR 1
#define I2C_GPIO_INIT  _IOW(I2C_IOCTL_MAGIE, 0, int) 

#define GN_I2C_NUM								(8) 
typedef enum
{
	GN_I2C_INDEX_CVBS_CH1 = 0, 
	GN_I2C_INDEX_CVBS_CH2,	
	GN_I2C_INDEX_CVBS_CH3,
	GN_I2C_INDEX_CVBS_CH4,
	GN_I2C_INDEX_CVBS_CH5,	
	GN_I2C_INDEX_CVBS_CH6,	
	GN_I2C_INDEX_CVBS_CH7,
	GN_I2C_INDEX_CVBS_CH8
}GN_I2cIndex;

#define I2C_DATA_BUFFER_SIZE 512
typedef struct 
{
	char m_I2cIndex;
	int m_DevAddr; 
	int m_RegAddrWidth; 
	char m_RegAddr[4]; 
	int m_DataLen;
	char m_Data[I2C_DATA_BUFFER_SIZE];
}DRL_I2cProtocol; 

#define DRL_I2C_NODE_PATH	"/dev/i2c_driver"

static S32 s_I2cFd = -1;


#define VOLUME_IOCTL_MAGIE 'V'
#define VOLUME_IOCTL_MAXNR 1
#define VOLUME_GPIO_INIT  _IOW(VOLUME_IOCTL_MAGIE, 0, int) 

#define GN_VOLUME_NUM								(8) 
typedef enum
{
	GN_VOLUME_INDEX_CVBS_CH1 = 0,
	GN_VOLUME_INDEX_CVBS_CH2,	
	GN_VOLUME_INDEX_CVBS_CH3,
	GN_VOLUME_INDEX_CVBS_CH4,
	GN_VOLUME_INDEX_CVBS_CH5,
	GN_VOLUME_INDEX_CVBS_CH6,	
	GN_VOLUME_INDEX_CVBS_CH7,
	GN_VOLUME_INDEX_CVBS_CH8
}GN_VolumeIndex;

#define VOLUME_DATA_BUFFER_SIZE 2
typedef struct 
{
	char m_VolumeIndex;
	int m_DataLen;
	char m_Data[VOLUME_DATA_BUFFER_SIZE];
}DRL_VolumeProtocol; 

#define DRL_VOLUME_NODE_PATH	"/dev/volume_driver"

static S32 s_VolumeFd = -1;

/* GPIO驱动 */
#define IO_CTRL_IOCTL_MAGIE 'G'
#define IO_CTRL_IOCTL_MAXNR 2
#define IO_CTRL_CVBS1_RESET _IO(IO_CTRL_IOCTL_MAGIE, 0)
#define IO_CTRL_CVBS2_RESET _IO(IO_CTRL_IOCTL_MAGIE, 1)

#define DRL_IO_CTRL_NODE_PATH		"/dev/io_ctrl_driver" /* GPIO 控制设备节点 */

static S32 s_IOCtrlFd = -1;

/* 串口相关驱动 */
#define DRL_UART_BAUDRATE   115200  
#define DRL_UART_DECODER_DATABITS    8
#define DRL_UART_DECODER_PARITY  'N'    
#define DRL_UART_DECODER_STOPBITS    1
static const struct {
	U32	m_ComIndex;
	U32	m_BaudRate;
	U8		m_DataBits;
	U8		m_Parity;
	U8		m_StopBits;
}sc_ComInfo[2] = 
{
	{3/* /dev/ttyS3 */, DRL_UART_BAUDRATE, DRL_UART_DECODER_DATABITS, DRL_UART_DECODER_PARITY, DRL_UART_DECODER_STOPBITS},
	{2/* /dev/ttyS2 */, DRL_UART_BAUDRATE, DRL_UART_DECODER_DATABITS, DRL_UART_DECODER_PARITY, DRL_UART_DECODER_STOPBITS}
};
static HANDLE32 s_UartHandle[2];

/* 锁定义 */
static inline  void lock_fd_w(S32 fd)
{
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	/* 获取不到,就阻塞 */
	fcntl(fd, F_SETLKW, &lock);
}

static inline void unlock_fd(S32 fd)
{
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	fcntl(fd, F_SETLKW, &lock);
}

static S32 DRL_FdLock(int Fd)
{
	if(Fd < 0)
	{
		return -1;
	}

	lock_fd_w(Fd);
	return 0;
}

static S32 DRL_FdUnLock(int Fd)
{
	if(Fd < 0)
	{
		return -1;
	}

	unlock_fd(Fd);
	return 0;
}

static S32 DRL_I2cWrite(U32 I2cIndex, S32 Fd, S32 DevAddr, U32 RegAddr, U8 *pData, S32 DataSize)
{
	S32 lActLen = -1;

	DRL_I2cProtocol lI2cProtol;

	lI2cProtol.m_I2cIndex = I2cIndex;
	lI2cProtol.m_DevAddr = DevAddr;
	lI2cProtol.m_RegAddrWidth = 1;
	lI2cProtol.m_RegAddr[0] = RegAddr & 0xFF;
	if (DataSize > I2C_DATA_BUFFER_SIZE)
	{
		lI2cProtol.m_DataLen = I2C_DATA_BUFFER_SIZE;
		GLOBAL_TRACE(("IIC Driver Max Support Continue %d Bytes Write!!\n", I2C_DATA_BUFFER_SIZE));
	}
	else
	{
		lI2cProtol.m_DataLen = DataSize; 
	}
	GLOBAL_MEMCPY(lI2cProtol.m_Data, pData, lI2cProtol.m_DataLen);

	DRL_FdLock(Fd);
	lActLen = write(Fd, &lI2cProtol, sizeof(lI2cProtol));
	DRL_FdUnLock(Fd);

	return lActLen;
}

static S32 DRL_I2cRead(U32 I2cIndex, S32 Fd, S32 DevAddr, U32 RegAddr, U8 *pData, S32 DataSize)
{
	S32 lActLen = -1;
	DRL_I2cProtocol lI2cProtol;

	GLOBAL_ASSERT(pData);

	if (I2cIndex >= GN_I2C_NUM) 
		return -1;

	lI2cProtol.m_I2cIndex = I2cIndex;
	lI2cProtol.m_DevAddr = DevAddr;
	lI2cProtol.m_RegAddrWidth = 1;
	lI2cProtol.m_RegAddr[0] = RegAddr & 0xFF;
	if (DataSize > I2C_DATA_BUFFER_SIZE)
	{
		lI2cProtol.m_DataLen = I2C_DATA_BUFFER_SIZE;
		GLOBAL_TRACE(("IIC Driver Max Support Continue %d Bytes Read!!\n", I2C_DATA_BUFFER_SIZE));
	}
	else
	{
		lI2cProtol.m_DataLen = DataSize; 
	}

	DRL_FdLock(Fd);
	lActLen = read(Fd, &lI2cProtol, sizeof(lI2cProtol));
	DRL_FdUnLock(Fd);

	if (lActLen > 0)
	{
		GLOBAL_MEMCPY( pData, lI2cProtol.m_Data, lActLen);
	}

	return lActLen;
}

static S32 DRL_VolumeWrite(U32 VolumeIndex, S32 Fd, U32 Data)
{
	S32 lActLen = -1;
	DRL_VolumeProtocol lVolumeProtol;

	if (VolumeIndex >= GN_VOLUME_NUM)
		return -1;

	lVolumeProtol.m_VolumeIndex = VolumeIndex;
	lVolumeProtol.m_DataLen = 2;
	lVolumeProtol.m_Data[0] = (Data >> 8) & 0xFF; /* MSB First */
	lVolumeProtol.m_Data[1] = Data & 0xFF;

	DRL_FdLock(Fd);
	lActLen = write(Fd, &lVolumeProtol, sizeof(lVolumeProtol));
	DRL_FdUnLock(Fd);

	return lActLen;
}

BOOL DRL_Initiate(void)
{
	S32 i;

	GGPIO_Initiate();

	if ((s_I2cFd = open(DRL_I2C_NODE_PATH, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
		GLOBAL_TRACE(("GLOBAL_OPEN File %s Failed\n", DRL_I2C_NODE_PATH));
		return FALSE;
	}

	if ((s_VolumeFd = open(DRL_VOLUME_NODE_PATH, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
		GLOBAL_TRACE(("GLOBAL_OPEN File %s Failed\n", DRL_VOLUME_NODE_PATH));
		return FALSE;
	}

	if ((s_IOCtrlFd = open(DRL_IO_CTRL_NODE_PATH, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
		GLOBAL_TRACE(("GLOBAL_OPEN File %s Failed\n", DRL_IO_CTRL_NODE_PATH));
		return FALSE;
	}

	for (i=0; i<2; i++)
	{
		if ((s_UartHandle[i] = PL_ComDeviceOpen(sc_ComInfo[i].m_ComIndex, FALSE)) == NULL)
		{
			GLOBAL_TRACE(("Open File /dev/ttyS%d Failed\n", sc_ComInfo[i].m_ComIndex));
			return FALSE;
		}

		PL_ComDeviceSetState(s_UartHandle[i], sc_ComInfo[i].m_BaudRate, sc_ComInfo[i].m_DataBits, sc_ComInfo[i].m_Parity, sc_ComInfo[i].m_StopBits);
		PL_ComDeviceSetOption(s_UartHandle[i], 0, 0, 0, 0);
	}

	return TRUE;
}

void DRL_Terminate(void)
{
	S32 i;

	GGPIO_Terminate();

	for (i=0; i<2; i++)
	{
		if (s_UartHandle[i] != 0)
		{
			PL_ComDeviceClose(s_UartHandle[i]);
			s_UartHandle[i] = NULL;
		}
	}
}

////////////////////////DXT8243的串口通信////////////////////////////////
S32 DRL_Dxt8243Read(void *pUserParam, U8 *pData, S32 DataSize)
{
	U32 lBoardIndex = (U32)pUserParam;

	MFPGA_SetUartSelect(lBoardIndex);

	return PL_ComDeviceRead(s_UartHandle[lBoardIndex / 2], pData, DataSize);
}

S32 DRL_Dxt8243Write(void *pUserParam, U8 *pData, S32 DataSize)
{
	U32 lBoardIndex = (U32)pUserParam;

	MFPGA_SetUartSelect(lBoardIndex);

	return PL_ComDeviceWrite(s_UartHandle[lBoardIndex / 2], pData, DataSize);
}

BOOL DRL_Dxt8243Reset(void *pUserParam)
{
	U32 lBoardIndex = (U32)pUserParam;

	switch(lBoardIndex)
	{
	case 0:
		GGPIO_SetupPIN(GGPIO_PA(2), FALSE, TRUE);
		GGPIO_SetPIN(GGPIO_PA(2), 0);
		usleep(200000);
		GGPIO_SetPIN(GGPIO_PA(2), 1);
		break;
	case 1:
		GGPIO_SetupPIN(GGPIO_PA(4), FALSE, TRUE);
		GGPIO_SetPIN(GGPIO_PA(4), 0);
		usleep(200000);
		GGPIO_SetPIN(GGPIO_PA(4), 1);
		break;
	case 2:
		GGPIO_SetupPIN(GGPIO_PA(5), FALSE, TRUE);
		GGPIO_SetPIN(GGPIO_PA(5), 0);
		usleep(200000);
		GGPIO_SetPIN(GGPIO_PA(5), 1);
		break;
	case 3:
		GGPIO_SetupPIN(GGPIO_PA(6), FALSE, TRUE);
		GGPIO_SetPIN(GGPIO_PA(6), 0);
		usleep(200000);
		GGPIO_SetPIN(GGPIO_PA(6), 1);
		break;
	default: break;
	}

	return TRUE;
}

/* 多个编码子板的升级使能线是一起控制的 */
void DRL_Dxt8243UpgradeEnable(BOOL Enable)
{
	S32 lLevel = 0;

	GGPIO_SetupPIN(GGPIO_PA(1), FALSE, TRUE);

	lLevel = (Enable == TRUE) ? 1 : 0;
	GGPIO_SetPIN(GGPIO_PA(1), lLevel); 
}


////////////////////////CVBS////////////////////////////////
struct  stCvbsChMapCvbsIndex
{
	U32 m_CvbsCh;
	U32 m_I2cIndex;
	S32 m_I2cDevAddr;
	U32 m_VolumeIndex;
};
static const struct stCvbsChMapCvbsIndex sc_CvbsChMapCvbsIndex[GN_CVBS_CH_NUM] = 
{
	{0, GN_I2C_INDEX_CVBS_CH1, 0xB8, GN_VOLUME_INDEX_CVBS_CH1},
	{1, GN_I2C_INDEX_CVBS_CH2, 0xBA, GN_VOLUME_INDEX_CVBS_CH2},
	{2, GN_I2C_INDEX_CVBS_CH3, 0xB8, GN_VOLUME_INDEX_CVBS_CH3},
	{3, GN_I2C_INDEX_CVBS_CH4, 0xBA, GN_VOLUME_INDEX_CVBS_CH4},
	{4, GN_I2C_INDEX_CVBS_CH5, 0xB8, GN_VOLUME_INDEX_CVBS_CH5},
	{5, GN_I2C_INDEX_CVBS_CH6, 0xBA, GN_VOLUME_INDEX_CVBS_CH6},
	{6, GN_I2C_INDEX_CVBS_CH7, 0xB8, GN_VOLUME_INDEX_CVBS_CH7},
	{7, GN_I2C_INDEX_CVBS_CH8, 0xBA, GN_VOLUME_INDEX_CVBS_CH8}
};
static U32 DRL_CvbsGetI2cIndex(U32 CvbsCh)
{
	int i;

	for (i=0; i<GN_CVBS_CH_NUM; i++)
	{
		if (CvbsCh == sc_CvbsChMapCvbsIndex[i].m_CvbsCh)
		{
			return sc_CvbsChMapCvbsIndex[i].m_I2cIndex;
		}
	}

	GLOBAL_TRACE(("Cvbs I2c Channel Err!\n"));
	return 0;
}
static S32 DRL_CvbsGetI2cDevAddr(U32 CvbsCh)
{
	int i;

	for (i=0; i<GN_CVBS_CH_NUM; i++)
	{
		if (CvbsCh == sc_CvbsChMapCvbsIndex[i].m_CvbsCh)
		{
			return sc_CvbsChMapCvbsIndex[i].m_I2cDevAddr;
		}
	}

	GLOBAL_TRACE(("Cvbs I2c Channel Err!\n"));
	return 0;
}
static U32 DRL_CvbsGetVolumeIndex(U32 CvbsCh)
{
	int i;

	for (i=0; i<GN_CVBS_CH_NUM; i++)
	{
		if (CvbsCh == sc_CvbsChMapCvbsIndex[i].m_CvbsCh)
		{
			return sc_CvbsChMapCvbsIndex[i].m_VolumeIndex;
		}
	}

	GLOBAL_TRACE(("Cvbs Volume Channel Err!\n"));
	return 0;
}


S32 DRL_CvbsI2cOpen(void *pUserParam)
{
	int i;
	S32 lCvbsIndex = (S32) pUserParam;

	for (i=lCvbsIndex * GN_CVBS_CHANNEL_NUM; i<(lCvbsIndex + 1) * GN_CVBS_CHANNEL_NUM; i++)
	{
		if (ioctl(s_I2cFd, I2C_GPIO_INIT, &sc_CvbsChMapCvbsIndex[i].m_I2cIndex) < 0) 
		{
			GLOBAL_TRACE(("ioctl error\n"));
			return -1;
		}
	}

	return s_I2cFd;
}

S32 DRL_CvbsI2cClose(void *pUserParam, S32 Fd)
{
	return 0;
}

S32 DRL_CvbsI2cRead(void *pUserParam, U32 ChNum, S32 Fd, U32 RegAddr, U8 *pData, S32 DataSize)
{
	S32 lCvbsIndex = (S32) pUserParam;
	S32 lCvbsCh = ChNum + lCvbsIndex * GN_CVBS_CHANNEL_NUM;

	return DRL_I2cRead(DRL_CvbsGetI2cIndex(lCvbsCh), Fd, DRL_CvbsGetI2cDevAddr(lCvbsCh), RegAddr, pData, DataSize);
}

S32 DRL_CvbsI2cWrite(void *pUserParam, U32 ChNum, S32 Fd, U32 RegAddr, U8 *pData, S32 DataSize)
{
	S32 lCvbsIndex = (S32) pUserParam;
	S32 lCvbsCh = ChNum + lCvbsIndex * GN_CVBS_CHANNEL_NUM;

	return DRL_I2cWrite(DRL_CvbsGetI2cIndex(lCvbsCh), Fd, DRL_CvbsGetI2cDevAddr(lCvbsCh), RegAddr, pData, DataSize);
}

S32 DRL_CvbsVolumeOpen(void *pUserParam)
{
	int i;
	S32 lCvbsIndex = (S32) pUserParam;

	for (i=lCvbsIndex * GN_CVBS_CHANNEL_NUM; i<(lCvbsIndex + 1) * GN_CVBS_CHANNEL_NUM; i++)
	{
		if (ioctl(s_VolumeFd, VOLUME_GPIO_INIT, &sc_CvbsChMapCvbsIndex[i].m_VolumeIndex) < 0) 
		{
			GLOBAL_TRACE(("ioctl error\n"));
			return -1;
		}
	}

	return s_VolumeFd;
}

S32 DRL_CvbsVolumeClose(void *pUserParam, S32 Fd)
{
	return 0;
}

S32 DRL_CvbsVolumeWrite(void *pUserParam, U32 ChNum, S32 Fd, S32 Data)
{
	S32 lCvbsIndex = (S32) pUserParam;
	S32 lCvbsCh = ChNum + lCvbsIndex * GN_CVBS_CHANNEL_NUM;

	return DRL_VolumeWrite(DRL_CvbsGetVolumeIndex(lCvbsCh), Fd, Data);
}

BOOL DRL_CvbsReset(void *pUserParam)
{
	S32 lCvbsIndex = (S32) pUserParam;
	S32 lCmd = 0;

	if (lCvbsIndex == 0)
		lCmd = IO_CTRL_CVBS1_RESET;
	else
		lCmd = IO_CTRL_CVBS2_RESET;

	DRL_FdLock(s_IOCtrlFd);
	if (ioctl(s_IOCtrlFd, lCmd) < 0)
	{
		GLOBAL_TRACE(("ioctl err\n"));
		DRL_FdUnLock(s_IOCtrlFd);
		return FALSE;
	}
	DRL_FdUnLock(s_IOCtrlFd);

	return TRUE;
}





















