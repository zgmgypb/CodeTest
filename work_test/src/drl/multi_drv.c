
#include "fpga.h"
#include "fan.h"
#include "linuxplatform.h"
#include "multi_drv.h"
#include "multi_private.h"
#include "platform_conf.h"

#ifdef GN1846
#include "hwl_fpga_spi.h"
#endif

#define DRL_FPGA_FILE "/dev/fpga"

static int fpga_fd = -1;
#ifdef SUPPORT_DRL_MODULE_USE_ETH
#define DRL_ETH_CPU_IP							(0xC0A8BC01)
#define DRL_ETH_FPGA_IP							(0xC0A8BC02)
#define DRL_ETH_CPU_PORT						(20001)
#define DRL_ETH_FPGA_PORT						(20000)

static HANDLE32	s_ETHICPSocketHandle = NULL;
#endif

/*FPGA 配置接口 --------------------------------------------------------------------------------------------------------------------------------- */
#ifdef MULT_USE_ALTERNATE_FPGA_CONFIG

#define IOCTOL_MAGIC					'f'
#define FPGA_CONFIG_STATUS_READ  		_IOR(IOCTOL_MAGIC, 0,char) 
#define FPGA_CONFIG_STATUS_WRITE  		_IOW(IOCTOL_MAGIC, 1,char) 
#define FPGA_CONFIG_nCONFIG_READ  		_IOR(IOCTOL_MAGIC, 2,char) 
#define FPGA_CONFIG_nCONFIG_WRITE 	 	_IOW(IOCTOL_MAGIC, 3,char) 

#define FPGA_CONFIG_DCLK_WRITE  		_IOW(IOCTOL_MAGIC, 4,char) 
#define FPGA_CONFIG_DATA_WRITE  		_IOW(IOCTOL_MAGIC, 5,char)

#define FPGA_CONFIG_DONE_READ 			_IOR(IOCTOL_MAGIC, 6,char) 

#define FPGA_RESET_SET  				_IOW(IOCTOL_MAGIC, 7,char) 
#define FPGA_RESET_CLEAN  				_IOW(IOCTOL_MAGIC, 8,char) 

#define IOCTL_MAXNR 9
#define FPGA_PS_CONFIG_BUFF_MAX			( 1024 * 6 + 1 )

static void __FPGA_PsWritePin( int fd, unsigned int  cmd, unsigned char value )
{
	char data[1];
	data[0] = value;
	ioctl( fd, cmd, &data[0]);     
}

static unsigned char __FPGA_PsReadPin( int fd, unsigned int cmd)
{
	char data[ 1 ];
	ioctl( fd, cmd, &data[ 0 ] );
	return data[ 0 ];
}

int DRL_FpgaConfiguration(const char *dev_path, const char *fpga_path)
{
	int fpga_config_fp ;
	FILE *fpga_configFileP ;
	int i , j ;
	int dataLength ;
	unsigned char myReadBuffer[ FPGA_PS_CONFIG_BUFF_MAX ] ;
	int retval = 0;

	int lReadTimes = 0;
	int lConfigStatus = 0;
	int lConfigDone = 0;

	GLOBAL_TRACE(("2730X IP Board Config Function\n"));

	fpga_config_fp = open( dev_path , O_RDWR | O_NOCTTY | O_NDELAY);
	if (fpga_config_fp < 0)
	{
		GLOBAL_TRACE(("open dev path [%s] failed!\n", dev_path));
		return (-1) ;
	}

#ifdef DEBUG_MODE_FPGA_CONFIG_ONCE
	if( __FPGA_PsReadPin( fpga_config_fp, FPGA_CONFIG_DONE_READ ) == 1 ){
		GLOBAL_TRACE( ( "No Need To Down FPGA!\n" ) );
		close(fpga_config_fp) ;
		return 0;
	}
#endif

	__FPGA_PsWritePin( fpga_config_fp, FPGA_CONFIG_nCONFIG_WRITE, 	0x01 );
	__FPGA_PsWritePin( fpga_config_fp, FPGA_CONFIG_DCLK_WRITE, 		0x00 );
	__FPGA_PsWritePin( fpga_config_fp, FPGA_CONFIG_DATA_WRITE, 		0x01 );
	//clear nconfig
	__FPGA_PsWritePin( fpga_config_fp, FPGA_CONFIG_nCONFIG_WRITE, 	0x00 );
	//wait status
	lReadTimes = 10;
	while( lReadTimes )
	{
		lConfigStatus = __FPGA_PsReadPin( fpga_config_fp, FPGA_CONFIG_STATUS_READ );
		lConfigDone = __FPGA_PsReadPin( fpga_config_fp, FPGA_CONFIG_DONE_READ );
		if( ( lConfigStatus == 0 ) || ( lConfigDone == 0 ) )
		{
			//GLOBAL_TRACE( ( "Wait Status1\n" ) );
			break;
		}

		lReadTimes--;
		if( lReadTimes == 0 )
		{
			return -1;
		}
		PFC_TaskSleep( 500 );
	}

	//set nconfig high ;
	lReadTimes = 10;
	__FPGA_PsWritePin( fpga_config_fp, FPGA_CONFIG_nCONFIG_WRITE, 0x01 );

	while ( lReadTimes  )
	{
		lConfigStatus = __FPGA_PsReadPin( fpga_config_fp, FPGA_CONFIG_STATUS_READ );

		if(  lConfigStatus == 1  )
		{
			//GLOBAL_TRACE( ( "Wait Status1\n" ) );
			break;
		}

		lReadTimes--;
		if( lReadTimes == 0 )
		{
			return -1;
		}
		PFC_TaskSleep( 500 );
	}

	//start to write
	fpga_configFileP = fopen( fpga_path , "rb" ) ;
	if ( fpga_configFileP == NULL ){
		GLOBAL_TRACE(("open fpga_file path [%s] failed!\n", fpga_path));
		close( fpga_config_fp );
		return  ( -1 ) ;
	}

	dataLength = 1 ;
	i = 0 ;
	j = 0 ;

	while( dataLength ){

		dataLength = fread( myReadBuffer , 1 , FPGA_PS_CONFIG_BUFF_MAX -1  , fpga_configFileP );

		if( write( fpga_config_fp, myReadBuffer, dataLength ) != dataLength  )
		{
			fprintf( stderr, "%s error:(%s)\n", __func__, strerror( errno ) );
			retval = -1;
			break; 
		}
		printf( "." );
		GLOBAL_FFUSH(GLOBAL_STDOUT);
		usleep( 100 );

	}

	printf( "\n" );

	if( (__FPGA_PsReadPin( fpga_config_fp, FPGA_CONFIG_DONE_READ ) == 1 ) ) {
		//GLOBAL_TRACE( ( "FPGA data down success\n" ) );
	}
	else{
		retval = -1;
		GLOBAL_TRACE( ("FPGA data down fail\n" ) );
	}

	close(fpga_config_fp) ;

	return retval;
}

#else

int DRL_FpgaConfiguration(const char *dev_path, const char *fpga_path)
{

#ifdef DEBUG_MODE_FPGA_CONFIG_ONCE
	{
		return 0;
	}
#endif

	int fpga_config_fp ;
	FILE *fpga_configFileP ;
	int i , j ;
	int dataLength ;
	unsigned char myReadBuffer[1024] ;
	int retval;
	U32 lTick;

	PAL_TimeStart(&lTick);





	fpga_config_fp = open(dev_path , O_RDWR | O_NOCTTY | O_NDELAY);
	if (fpga_config_fp < 0)
	{
		GLOBAL_TRACE(("open dev path [%s] failed!\n", dev_path));
		return (-1) ;
	}

	if(write(fpga_config_fp , NULL , 0) == 0)
	{
		return  (-1);
	}
	////////////////////
	fpga_configFileP = fopen(fpga_path , "rb") ;
	if (fpga_configFileP == NULL)
	{
		GLOBAL_TRACE(("open fpga_file path [%s] failed!\n", fpga_path));
		close(fpga_config_fp);
		return  (-1) ;
	}
	dataLength = 1 ;
	i = 0 ;
	j = 0 ;
	while(dataLength)
	{
		dataLength = fread(myReadBuffer , 1 , 512 , fpga_configFileP) ;
		i += dataLength ;

		j = ioctl(fpga_config_fp , dataLength , (char *)myReadBuffer) ;
		if (j > 0)
		{
			break ;
		}
		if (dataLength <= 0)
		{
			break ;
		}
	}

	close(fpga_config_fp) ;

	if (j == 1)
	{
		retval = 0 ;
	}
	else
	{
		retval = (-1) ;

	}

	GLOBAL_TRACE(("ConfigTime = %.2f s\n", (F64)PAL_TimeEnd(&lTick) / 1000));

	return retval;
}




#endif

BOOL DRL_ZYNQCheckConfigDone(void)
{
	BOOL lRet = FALSE;
	CHAR_T lDoneMark;
	GLOBAL_FD lFPGAStatus;
	lFPGAStatus = fopen("/sys/devices/soc0/amba/f8007000.devcfg/prog_done", "r");
	if (lFPGAStatus)
	{
		GLOBAL_FREAD(&lDoneMark, 1, 1, lFPGAStatus);
		if (lDoneMark == '1')
		{
			lRet = TRUE;
		}
		fclose(lFPGAStatus);
	}
	return lRet;
}

BOOL DRL_ZYNQInitiate(void)
{
	BOOL lRet = FALSE;
	S32 lFpgaCfgNodeFD;
	FILE *lFpgaFileFD;
	S32 lDataLen ;
	U8 plReadBuf[1024];

	CHAR_T *fpga_path = "/tmp/zynq7010.bin";

	PFC_TaskSleep(100);

#ifdef DEBUG_MODE_FPGA_CONFIG_ONCE
	if (DRL_ZYNQCheckConfigDone())
	{
		GLOBAL_TRACE(("ZYNQ ALREADY ON LINE!!!\n"));
		return TRUE;
	}
#endif

	lFpgaFileFD = fopen(fpga_path , "rb") ;
	if (lFpgaFileFD != NULL)
	{
		fclose(lFpgaFileFD);

		PFC_System("cat %s > %s\n", fpga_path, "/dev/xdevcfg");

		/*Zynq 自带FPGA读取方法！*/
		if (DRL_ZYNQCheckConfigDone())
		{
			GLOBAL_TRACE(("Zynq Config Successful!!!!\n"));
			lRet = TRUE;
		}
		else
		{
			GLOBAL_TRACE(("Zynq Config Failed!!!!!!!!!!!\n"));
		}
	}
	else
	{
		GLOBAL_TRACE(("Open File [%s] Failed\n", fpga_path));
	}

	return lRet;
}

#include "platform_assist.h"
#include "global_micros.h"
//#include "universal_spi.h"
#ifdef GN1846
#include "hisi_gpio.h"
#include "spi_gpio.h"
static HANDLE32 s_SpiHandle = NULL;
static BOOL s_FspiComOk = TRUE;

static void FSPI_SetValue(SPI_GpioPin *pPinAddr, S32 Value, void *pUserParam)
{
	HI_GpioSetValue(pPinAddr->m_GpioIndex, pPinAddr->m_Pin, Value);
}

static S32 FSPI_GetValue(SPI_GpioPin *pPinAddr, void *pUserParam)
{	
	return HI_GpioGetValue(pPinAddr->m_GpioIndex, pPinAddr->m_Pin);
}

static void FSPI_Setup(SPI_GpioPin *pPinAddr, BOOL IsInput, BOOL IsPullUp, void *pUserParam)
{
	HI_GpioSetup(pPinAddr->m_GpioIndex, pPinAddr->m_Pin, IsInput, IsPullUp);
}

S32 DRL_FspiWrite(U16 RegAddr, U8 *pBuf, U32 BufSize)
{
	U8 plAddr[2];

	plAddr[0] = (RegAddr >> 8) & 0xFF;
	plAddr[1] = RegAddr & 0xFF;
	if (SPI_GpioWrite(s_SpiHandle, plAddr, 2, pBuf, BufSize) != BufSize) {
		GLOBAL_TRACE(("SPI_GpioWrite Err!\n"));
		s_FspiComOk = FALSE;
		return -1;
	}
	s_FspiComOk = TRUE;

	return BufSize;
}

S32 DRL_FspiRead(U16 RegAddr, U8 *pBuf, U32 BufSize)
{
	U8 plAddr[2];
	S32 lRet;

	plAddr[0] = (RegAddr >> 8) & 0xFF | 0x80;
	plAddr[1] = RegAddr & 0xFF;
	if ((lRet = SPI_GpioRead(s_SpiHandle, plAddr, 2, pBuf, BufSize)) == -1) {
		GLOBAL_TRACE(("SPI_GpioRead Err!\n"));
		s_FspiComOk = FALSE;
		return -1;
	}
	s_FspiComOk = TRUE;

	return lRet;
}

BOOL DRL_GetFspiComIsOk(void)
{
	return s_FspiComOk;
}
#endif

/*内部通讯接口 --------------------------------------------------------------------------------------------------------------------------------- */
int DRL_ICPInitate(void)
{
#ifdef SUPPORT_DRL_MODULE_USE_ETH

	/*设置内部通许IP地址*/
	PFC_System("ifconfig eth1 %s netmask 255.255.255.0\r\n", PFC_SocketNToA(DRL_ETH_CPU_IP));
	//PFC_System("ifconfig eth1 down\r\n");
	//PFC_System("ifconfig eth1 up\r\n");

	if (s_ETHICPSocketHandle == NULL)
	{
		S32 lValue;
		s_ETHICPSocketHandle = PFC_SocketCreate(PFC_SOCKET_TYPE_UDP);

		PFC_SocketBind(s_ETHICPSocketHandle, DRL_ETH_CPU_IP, DRL_ETH_CPU_PORT);

		lValue = 0;
		PFC_SocketOption(s_ETHICPSocketHandle, PFC_SOCKET_OPTION_TYPE_NONBLOCK, &lValue,sizeof(lValue));

		lValue = 500;
		PFC_SocketOption(s_ETHICPSocketHandle, PFC_SOCKET_OPTION_TYPE_RECVTIMEOUT, &lValue,sizeof(lValue));

		lValue = 500;
		PFC_SocketOption(s_ETHICPSocketHandle, PFC_SOCKET_OPTION_TYPE_SENDTIMEOUT, &lValue,sizeof(lValue));

		lValue = 1;
		PFC_SocketOption(s_ETHICPSocketHandle, PFC_SOCKET_OPTION_TYPE_NO_CHECK_SUM, &lValue,sizeof(lValue));

		lValue = 1;
		PFC_SocketOption(s_ETHICPSocketHandle, PFC_SOCKET_OPTION_TYPE_ADDRREUSE, &lValue,sizeof(lValue));

		lValue = 4 * 1024 *1024;
		PFC_SocketOption(s_ETHICPSocketHandle, PFC_SOCKET_OPTION_TYPE_RECVBUF, &lValue,sizeof(lValue));
	}

	GLOBAL_TRACE(("Initate Socket Complete!!! Handle = %08X\n", s_ETHICPSocketHandle));
#else
#ifdef GN1846
	if (HWL_FPGAGetMuxOK()) {
		if (!s_SpiHandle) {
			s_SpiHandle = FSPI_Create();
		}
	}

	return 1;
#else
	if (HWL_FPGAGetMuxOK())
	{
		if( fpga_fd < 0 )
		{
			fpga_fd = open(DRL_FPGA_FILE , O_RDWR);
		}

		assert(fpga_fd > 0);
	}

	if(fpga_fd > 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
#endif

#endif
}

int DRL_ICPLock(void)
{
	if(fpga_fd < 0)
	{
		return -1;
	}

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	fcntl(fpga_fd, F_SETLKW, &lock);
	return 0;
}

int DRL_ICPUnLock(void)
{
	if(fpga_fd < 0)
	{
		return -1;
	}

	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	fcntl(fpga_fd, F_SETLKW, &lock);

	return 0;
}

int DRL_FpgaReadLock(unsigned char *buff, unsigned int size)
{
	int lActLen = 0;

#ifdef SUPPORT_DRL_MODULE_USE_ETH
	if (PFC_SocketRecv(s_ETHICPSocketHandle, buff, size, &lActLen) == FALSE)
	{
		GLOBAL_TRACE(("ICP Recv Error!\n"));
	}
#else

	if (HWL_FPGAGetMuxOK())
	{
		assert(fpga_fd > 0);
		if(fpga_fd < 0)
		{
			return -1;
		}

		DRL_ICPLock();

		lActLen = read(fpga_fd, buff, size);

		DRL_ICPUnLock();
	}

#endif

#ifdef SUPPORT_NEW_HWL_MODULE
	HWL_MonitorPlusICPByteNum(lActLen, TRUE);
#endif

	return lActLen;
}


int DRL_FpgaWriteLock(unsigned char *buff , unsigned int size )
{

#ifdef SUPPORT_NEW_HWL_MODULE
	HWL_MonitorPlusICPByteNum(size, FALSE);
#endif

#ifdef SUPPORT_DRL_MODULE_USE_ETH
	S32 lActLen = 0;
	if (PFC_SocketSendTo(s_ETHICPSocketHandle, buff, size, &lActLen, DRL_ETH_FPGA_IP, DRL_ETH_FPGA_PORT) == FALSE)
	{
		GLOBAL_TRACE(("ICP Send Error!\n"));
	}
#else
	int lActLen = 0;

	if (HWL_FPGAGetMuxOK())
	{
		assert(fpga_fd > 0);
		if(fpga_fd < 0)
		{
			return -1;
		}

		DRL_ICPLock();

		lActLen = write(fpga_fd, buff, size);

		DRL_ICPUnLock();
	}

#endif


	return lActLen;
}

void DRL_FpgaDestroy(void)
{
#ifdef SUPPORT_DRL_MODULE_USE_ETH
	if (s_ETHICPSocketHandle)
	{
		PFC_SocketClose(s_ETHICPSocketHandle);
		s_ETHICPSocketHandle = NULL;
	}
#else
	if(fpga_fd > 0)
	{
		close(fpga_fd);
	}
	fpga_fd = -1;
#endif
}

/*风扇驱动接口*/

# if defined(GQ3650DS) || defined(GQ3650DR) || defined(GQ3710A)
#define DRL_FAN_DEV_PATH "/dev/fan"
static int  fan_fd = -1;
#endif

int DRL_FanInit(void)
{
# if defined(GQ3650DS) || defined(GQ3650DR) || defined(GQ3710A)
	fan_fd = open(DRL_FAN_DEV_PATH, O_RDWR);
	DRL_FanStatusSet(0);
#endif
	return 0;
}


int DRL_FanStatusSet(int val)
{
# if defined(GQ3650DS) || defined(GQ3650DR) || defined(GQ3710A)
	return ioctl(fan_fd, FAN_IOCSETVAL, &val);
#else
	return 0;
#endif
}


int DRL_FanStatusGet(void)
{
# if defined(GQ3650DS) || defined(GQ3650DR) || defined(GQ3710A)
	int ret;
	int val;

	assert(fan_fd > 0);

	if(fan_fd < 0)
	{
		return -1;
	}
	ret = ioctl(fan_fd, FAN_IOCGETVAL, &val);
	if(ret < 0)
	{
		return ret;
	}

	return val;
#else
	return 0;
#endif

}

int DRL_FanHwGet(void)
{
# if defined(GQ3650DS) || defined(GQ3650DR) || defined(GQ3710A)
	int ret;
	int val;

	assert(fan_fd > 0);

	if(fan_fd < 0)
	{
		return -1;
	}
	ret = ioctl(fan_fd, FAN_CHECKHW, &val);
	if(ret < 0)
	{
		return ret;
	}
	return val;
#else
	return 0;
#endif
}

void DRL_FanDestroy(void)
{
# if defined(GQ3650DS) || defined(GQ3650DR) || defined(GQ3710A)
	if(fan_fd > 0)
	{
		close(fan_fd);
	}
	fan_fd = -1;
#endif
}


static U32	s_2730BoardVersion = 0;

BOOL DRL_GetMainBoardIS4CE55(void)
{
	return (s_2730BoardVersion == GM2730X_4CE55_VALUE);
}

/*2730X系列 通过主板来确定是4CE55还是3C55*/
U32 DRL_GetMainBoardVersionBitValue(void)
{
	U32 lBitValue, lLastBitValue;
	S32 lReadCount;

	S32 lPB16, lPB17;


	lBitValue = 0;
	lLastBitValue = GLOBAL_INVALID_INDEX;

	//PFC_GPIOInitiate();

	PFC_GPIOSetupPIN(PFCGPIO_PB(16), TRUE, TRUE);
	PFC_GPIOSetupPIN(PFCGPIO_PB(17), TRUE, TRUE);

	PFC_TaskSleep(500);



	lReadCount = 0;
	while(lReadCount < 10)
	{
		lPB16 = PFC_GPIOGetPIN(PFCGPIO_PB(16));
		lPB17 = PFC_GPIOGetPIN(PFCGPIO_PB(17));

		lBitValue = (lPB17 << 1) | lPB16;

		GLOBAL_TRACE(("PB17 = %d, PB16 = %d, lBitValue = 0x%02X\n", lPB17, lPB16, lBitValue));

		if (lLastBitValue == GLOBAL_INVALID_INDEX)
		{
			lLastBitValue = lBitValue;
		}
		else
		{
			if (lLastBitValue == lBitValue)
			{
				break;
			}
			else
			{
				lLastBitValue = lBitValue;
			}
		}

		lReadCount ++;
		PFC_TaskSleep(100);
	}

	//lBitValue = GM2730X_3C55_VALUE;

	s_2730BoardVersion = lBitValue;

	return lBitValue;
}





