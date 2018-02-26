#include <stdio.h> 
#include <unistd.h>
#include <fcntl.h> 
#include <time.h>
#include <errno.h>

#ifdef GM8358Q

#include "global_def.h"
#include "linuxplatform.h"
#ifdef GM8358Q
#include "../../drivers/fpga_ps/fpga_ps_driver.h"
#endif

#ifdef GM8398Q
#include "../../drivers/fpga_ps_GM8398Q/fpga_ps_driver.h"
#endif

/* Fpga Localbus Driver */
#define FPGA_LOCALBUS_IOCTL_MAGIE 'L'
#define FPGA_LOCALBUS_IOCTL_MAXNR 1
#define FPGA_LOCALBUS_CONFIG  _IOW(FPGA_LOCALBUS_IOCTL_MAGIE, 0, int) 
typedef struct 
{
	int m_FpgaIndex;
	int m_RegAddr;
	unsigned short m_Data;
}DRL_FpgaLocalbusProtocol; /* Localbus组包协议 */

static S32 s_FpgaLocalbusFd = -1;

/* 锁定义 */
static inline  void lock_fd_w(S32 fd)
{
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	// 获取不到,就阻塞
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

S32 DRL_FdLock(int Fd)
{
	if(Fd < 0)
	{
		return -1;
	}

	lock_fd_w(Fd);
	return 0;
}

S32 DRL_FdUnLock(int Fd)
{
	if(Fd < 0)
	{
		return -1;
	}

	unlock_fd(Fd);
	return 0;
}

BOOL DRL_Encoder_Fpga_Initiate( void )
{
	int lData = GN_FPGA_INDEX_MAIN; 

	s_FpgaLocalbusFd = open("/dev/fpga_localbus_driver",O_RDWR);
	if(s_FpgaLocalbusFd < 0)
	{
		printf("open fpga error\n");
		return FALSE;
	}

	/* 在localbus的操作前，必须先进行初始配置 */
	if (ioctl(s_FpgaLocalbusFd, FPGA_LOCALBUS_CONFIG, &lData) < 0) 
	{
		GLOBAL_TRACE(("ioctl error\n"));
		return -1;
	}

	lData = GN_FPGA_INDEX_IP_ASI_OUTPUT; 
	if (ioctl(s_FpgaLocalbusFd, FPGA_LOCALBUS_CONFIG, &lData) < 0) 
	{
		GLOBAL_TRACE(("ioctl error\n"));
		return -1;
	}

	return TRUE;
}

S32 DRL_FpgaRead(U32 FpgaIndex, U32 RegAddr, U8 *pBuf, U32 Size)
{
	S32 lActLen = -1;

	GLOBAL_ASSERT(pBuf);

	switch (FpgaIndex)
	{
	case GN_FPGA_INDEX_MAIN: 
	case GN_FPGA_INDEX_IP_ASI_OUTPUT:
		{
			int i;
			DRL_FpgaLocalbusProtocol lLocalbusProto;

			lLocalbusProto.m_FpgaIndex = FpgaIndex;
			for (i=0; i<Size; i++)
			{
				lLocalbusProto.m_RegAddr = RegAddr + i;
				DRL_FdLock(s_FpgaLocalbusFd);
				if (read(s_FpgaLocalbusFd, &lLocalbusProto, sizeof(lLocalbusProto)) < 0)
				{
					GLOBAL_TRACE(("read localbus error!\n"));
					DRL_FdUnLock(s_FpgaLocalbusFd);
					return -1;
				}
				DRL_FdUnLock(s_FpgaLocalbusFd);
				pBuf[i] = lLocalbusProto.m_Data & 0xFF; /* 取低8位 */
			}	
			lActLen = Size;
		}
		break;
	default: break;
	}
	return lActLen;
}

S32 DRL_FpgaWrite(U32 FpgaIndex, U32 RegAddr, U8 *pBuf, U32 Size)
{
	S32 lActLen = -1;

	switch (FpgaIndex)
	{
	case GN_FPGA_INDEX_MAIN: 
	case GN_FPGA_INDEX_IP_ASI_OUTPUT:
		{
			int i;
			DRL_FpgaLocalbusProtocol lLocalbusProto;

			lLocalbusProto.m_FpgaIndex = FpgaIndex;
			for (i=0; i<Size; i++)
			{
				lLocalbusProto.m_RegAddr = RegAddr + i;
				lLocalbusProto.m_Data = pBuf[i];
				DRL_FdLock(s_FpgaLocalbusFd);
				if (write(s_FpgaLocalbusFd, &lLocalbusProto, sizeof(lLocalbusProto)) < 0)
				{
					GLOBAL_TRACE(("write localbus error!\n"));
					DRL_FdUnLock(s_FpgaLocalbusFd);
					return -1;
				}
				DRL_FdUnLock(s_FpgaLocalbusFd);
			}		
			lActLen = Size;
		}
		break;
	default: break;
	}
	return lActLen;
}

unsigned short READ_FPGA(unsigned char B_TYPE, unsigned long ADDRESS)
{
	U8 lData = 0;

	if (DRL_FpgaRead(B_TYPE, ADDRESS, &lData, 1) < 0)
	{
		//GLOBAL_TRACE(("Read Encoder Fpga Error!\n"));
		return -1;
	}

	return lData;
}

void WRITE_FPGA(unsigned char B_TYPE, unsigned long ADDRESS, int DATA)
{
	U8 lData = (U8)(DATA & 0xFF);

	DRL_FpgaWrite(B_TYPE, ADDRESS, &lData, 1);
}

#endif
