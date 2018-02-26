#include "gn_drv.h"
#include <termios.h> 

/* I2C部分 */
#define I2C_IOCTL_MAGIE 'I'
#define I2C_IOCTL_MAXNR 1
#define I2C_GPIO_INIT  _IOW(I2C_IOCTL_MAGIE, 0, int) 

#define GN_I2C_NUM								(8) /* IIC通信数目 */
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
	int m_DevAddr; /* 设备地址,由于当前我们的设备使用的都是7位的IIC寻址芯片，所以这里仅考虑7位的，对于10位寻址的使用时再扩展 */
	int m_RegAddrWidth; 
	char m_RegAddr[4]; /* 高字节在前 */
	int m_DataLen; /* 数据长度 */
	char m_Data[I2C_DATA_BUFFER_SIZE];
}DRL_I2cProtocol; /* I2C组包协议 */

#define DRL_I2C_NODE_PATH	"/dev/i2c_driver"

static S32 s_I2cFd = -1;

/* 音量控制 */
#define VOLUME_IOCTL_MAGIE 'V'
#define VOLUME_IOCTL_MAXNR 1
#define VOLUME_GPIO_INIT  _IOW(VOLUME_IOCTL_MAGIE, 0, int) 

#define GN_VOLUME_NUM								(8) /* 音量控制通信数目 */
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
	int m_DataLen; /* 数据长度 */
	char m_Data[VOLUME_DATA_BUFFER_SIZE];
}DRL_VolumeProtocol; /* Volume组包协议 */

#define DRL_VOLUME_NODE_PATH	"/dev/volume_driver"

static S32 s_VolumeFd = -1;

/* GPIO驱动 */
#define IO_CTRL_IOCTL_MAGIE 'G'
#define IO_CTRL_IOCTL_MAXNR 2
#define IO_CTRL_CVBS1_RESET _IO(IO_CTRL_IOCTL_MAGIE, 0)
#define IO_CTRL_CVBS2_RESET _IO(IO_CTRL_IOCTL_MAGIE, 1)

#define DRL_IO_CTRL_NODE_PATH		"/dev/io_ctrl_driver" /* GPIO 控制设备节点 */

static S32 s_IOCtrlFd = -1;

/* 串口驱动 */
#define GN_UART_BAUDRATE   115200  
#define GN_UART_BITS    8
#define GN_UART_PARITY  'N'    
#define GN_UART_STOPBIT    1
#define  UART_COM0 		 "/dev/ttyS2"
#define  UART_COM1 		 "/dev/ttyS3"
static struct 
{
	char *m_pUartNodeName;
	S32 m_ComFd;
}s_ComInfo[GN_VIXS_SUBBOARD_NUM / 2] =  /* 两个子板公用一个串口 */
{
	{UART_COM0, -1},
	{UART_COM1, -1}
};

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

static BOOL DRL_InitArmUartPort(S32 UartIndex ,S32 nSpeed, S32 nBits, S32 nEvent, S32 nStop) 
{
	struct termios newtio,oldtio; 
	S32 lComFd = -1;

	lComFd = open(s_ComInfo[UartIndex].m_pUartNodeName, O_RDWR|O_NOCTTY|O_NDELAY); 
	if (lComFd < 0)
	{
		GLOBAL_TRACE(("Can't Open Serial Port %s\n", s_ComInfo[UartIndex].m_pUartNodeName));
	}

	/* set dev status flag */
	if ( fcntl(lComFd , F_SETFL, 0) < 0) 
	{
		GLOBAL_PRINTF(("\n fcntl failed !")); 
	}else{ 
		GLOBAL_PRINTF(("\nfcntl=%d\n",fcntl(lComFd , F_SETFL,0))); 
	}

	if (isatty(lComFd) == 0){ 
		GLOBAL_PRINTF(("\nstandard input is not a terminal device\n")); 
		return(-1); 
	}else {
		GLOBAL_PRINTF(("\nname = %s" , ttyname(lComFd))); 
		GLOBAL_PRINTF(("\nfd-open=%d",lComFd)); 
	}

	/* config port */
	if  ( tcgetattr( lComFd , &oldtio)  !=  0) { 
		perror("\nget attr error "); 
		return -1; 
	} 
	bzero( &newtio, sizeof( newtio ) ); 
	newtio.c_cflag  |= CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 
	switch( nBits ) 
	{ 
	case 7: 
		newtio.c_cflag |= CS7; 
		break; 
	default :
		newtio.c_cflag |= CS8; 
		break; 
	} 
	switch( nEvent ) 
	{ 
	case 'O': //ODD 
		newtio.c_cflag |= PARENB; 
		newtio.c_cflag |= PARODD; 
		newtio.c_iflag |= (INPCK | ISTRIP); 
		break; 
	case 'E': //ENEV
		newtio.c_iflag |= (INPCK | ISTRIP); 
		newtio.c_cflag |= PARENB; 
		newtio.c_cflag &= ~PARODD;
		break; 
	case 'N': //No para
		newtio.c_cflag &= ~PARENB; 
		break; 
	default : 
		break;
	} 
	switch( nSpeed ) 
	{ 
	case 2400: 
		cfsetispeed(&newtio, B2400); 
		cfsetospeed(&newtio, B2400); 
		break; 
	case 4800: 
		cfsetispeed(&newtio, B4800); 
		cfsetospeed(&newtio, B4800); 
		break; 
	case 19200:
		cfsetispeed(&newtio, B19200); 
		cfsetospeed(&newtio, B19200); 
		break; 
	case 115200: 
		cfsetispeed(&newtio, B115200); 
		cfsetospeed(&newtio, B115200); 
		break; 
	case 460800: 
		cfsetispeed(&newtio, B460800); 
		cfsetospeed(&newtio, B460800); 
		break; 
	default: 
		cfsetispeed(&newtio, B9600); 
		cfsetospeed(&newtio, B9600); 
		break; 
	} 
	if( nStop == 1 )
	{ 
		newtio.c_cflag &=  ~CSTOPB; 
	}
	else if ( nStop == 2 )
	{ 
		newtio.c_cflag |=  CSTOPB;
	}
	newtio.c_cc[VTIME] = 0; 
	newtio.c_cc[VMIN] = 0; //最小接收个数
	//tcflush( fd, TCIOFLUSH );

	if((tcsetattr(lComFd , TCSANOW , &newtio))!=0) 
	{ 
		perror("set com attr error\n"); 
		s_ComInfo[UartIndex].m_ComFd = lComFd;
		return FALSE; 
	} 
	s_ComInfo[UartIndex].m_ComFd = lComFd;

	return TRUE; 
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

	for (i=0; i<GN_VIXS_SUBBOARD_NUM / 2; i++)
	{
		if (DRL_InitArmUartPort(i, GN_UART_BAUDRATE, GN_UART_BITS, GN_UART_PARITY, GN_UART_STOPBIT) == FALSE)
			return FALSE;
	}

	return TRUE;
}

void DRL_Terminate(void)
{
	S32 i;

	if (s_I2cFd >= 0)
	{
		close(s_I2cFd);
		s_I2cFd = -1;
	}

	if (s_VolumeFd >= 0)
	{
		close(s_VolumeFd);
		s_VolumeFd = -1;
	}

	if (s_IOCtrlFd >= 0)
	{
		close(s_IOCtrlFd);
		s_IOCtrlFd = -1;
	}

	for (i=0; i<GN_VIXS_SUBBOARD_NUM / 2; i++)
	{
		if (s_ComInfo[i].m_ComFd >= 0)
		{
			close(s_ComInfo[i].m_ComFd);
			s_ComInfo[i].m_ComFd = -1;
		}
	}
}

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

	/* 在每个模块的打开时，对当前模块的IO口进行初始化 */
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

	/* 在每个模块的打开时，对当前模块的IO口进行初始化 */
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

/* 串口操作 */
S32 DRL_UartOpen(U32 UartIndex)
{
	return s_ComInfo[UartIndex].m_ComFd;
}

S32 DRL_UartClose(S32 Fd)
{
	return 0;
}

S32 DRL_UartRead(S32 Fd, U8 *pData)
{
	unsigned int i ;
	struct timeval lTimeout;
	fd_set lReadFd;

	FD_ZERO(&lReadFd);

	if (Fd < 0) 
		return(-1) ;

	FD_SET(Fd, &lReadFd);

	lTimeout.tv_sec = 1;
	lTimeout.tv_usec = 0;

	tcflush(Fd, TCIOFLUSH);

	if(select(Fd + 1, &lReadFd, NULL, NULL, &lTimeout))
	{
		if (FD_ISSET(Fd, &lReadFd))
		{
			i = read(Fd, pData, 50) ;
			tcflush(Fd, TCIOFLUSH);
			return(i) ;
		}
	}
	return -1;
}

S32 DRL_UartWrite(S32 Fd, U8 *pData, S32 DataSize)
{
	if (Fd < 0) 
		return -1;

	//GLOBAL_TRACE(("Fd: %d data: %s datasize: %d\n", Fd, pData, DataSize));

	return write(Fd, pData, DataSize);
}
