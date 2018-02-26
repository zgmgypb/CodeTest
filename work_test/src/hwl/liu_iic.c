#include "multi_drv.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "global_micros.h"
#include "mpeg2_micro.h"
#include "multi_main_internal.h"

#include "liu_iic.h"

#undef TUNER_DEBUG             /* undef it, just in case */
#define TUNERDBUG
#ifdef TUNERDBUG

#	define TUNER_DEBUG(fmt, args...) fprintf(stderr,fmt, ## args)
#else

#	define TUNER_DEBUG(fmt, args...) /* not debugging: nothing */

#endif

#define TUNERDBUG1
#ifdef TUNERDBUG1

#	define TUNER_DEBUG1(fmt, args...) fprintf(stderr,fmt, ## args)
#else

#	define TUNER_DEBUG1(fmt, args...) /* not debugging: nothing */

#endif
#if 0
enum
{
	DTMB=0,
	DVB_C,
	DVB_S,
	DVB_T,
};
#endif
#define DEV_NAME "/dev/tuner_device"
static int DevIicFile;
static U16 TunerNo;
static U16 IIC_TunerType;

void Delay(unsigned int count)      //Sleep   ÑÓÊ±×Ó³ÌÐò
{
  unsigned int i;
  while(count)
  {
    i=5;
    while(i>0)
    i--;
    count--;
  }
}
static void DevIoctl(U32 Cmd,U16 Value)
{
	U16 tmp;
	tmp=Value;
	ioctl(DevIicFile,Cmd,&tmp);
	//Delay(50);
}
void delay_ms(U32 dMilliSeconds)
{
	#if 1
	S32 ret;
	struct timespec tvp,tvr;

	tvp.tv_sec = dMilliSeconds / 1000;
	tvp.tv_nsec = (dMilliSeconds % 1000) * 1000 * 1000;

	while(1)
	{
		ret = nanosleep(&tvp,&tvr);
		if (-1 == ret)
		{
			if (errno == EINTR)
			{
				tvp.tv_sec = tvr.tv_sec;
				tvp.tv_nsec = tvr.tv_nsec;
				continue;
			}
			fprintf (stderr, " select error . errno=%d [%s]\n", errno,strerror (errno));
		}
		break;
	}
	#endif
	//usleep(dMilliSeconds * 1000);
}
void ResetTuner(U16 Channel)
{
	U16 tmp=0;
	#if 0
	#if   1//ndef DVB_C_DAUL
	
	tmp = Channel;//TunerNo ;
	tmp <<= 8;
	tmp &= 0xfffe;
	TUNER_DEBUG1("tmp1:%x,TunerNo:%d\n",tmp,Channel);//TunerNo
	DevIoctl(TUNER_RESET_CLEAN,tmp);//TUNER_PIN_LOW	
	delay_ms(20);//sleep(1);

	tmp = 0;
	tmp = Channel;//TunerNo ;
	tmp <<= 8;
	tmp |=0x01;
	TUNER_DEBUG1("tmp2:%x,TunerNo:%d\n",tmp,Channel);//TunerNo
	DevIoctl(TUNER_RESET_SET,tmp);//TUNER_PIN_HIGH
	delay_ms(20);//sleep(1);
	#else
	tmp = 0;
	tmp = Channel;//TunerNo ;
	tmp <<= 8;
	tmp |=0x01;
	TUNER_DEBUG1("tmp2:%x,TunerNo:%d\n",tmp,Channel);//TunerNo
	DevIoctl(TUNER_RESET_SET,tmp);//TUNER_PIN_HIGH
	delay_ms(10);//sleep(1);

	tmp = Channel;//TunerNo ;
	tmp <<= 8;
	tmp &= 0xfffe;
	TUNER_DEBUG1("tmp1:%x,TunerNo:%d\n",tmp,Channel);//TunerNo
	DevIoctl(TUNER_RESET_CLEAN,tmp);//TUNER_PIN_LOW	
	delay_ms(10);//sleep(1);
	#endif
	#endif
}
//
#if 0
static void IIC_Start()
{
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_HIGH);
	DevIoctl(IIC_DATA_WRITE,TUNER_PIN_HIGH);
	DevIoctl(IIC_DATA_WRITE,TUNER_PIN_LOW);
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_LOW);
}
static void IIC_Stop()
{
	DevIoctl(IIC_DATA_WRITE,TUNER_PIN_LOW);
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_HIGH);
	DevIoctl(IIC_DATA_WRITE,TUNER_PIN_HIGH);
}
void SendNoACK()
{
	DevIoctl(IIC_DATA_WRITE,TUNER_PIN_HIGH);
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_HIGH);
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_LOW);
}
void SendACK()
{
	DevIoctl(IIC_DATA_WRITE,TUNER_PIN_LOW);
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_HIGH);
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_LOW);
}
static U8 waitACK()
{
	U16 i,tmp;
	
	i=1000;
	DevIoctl(IIC_DATA_WRITE,TUNER_PIN_HIGH);
	Delay(10);
	//direction in
	DevIoctl(IIC_DATA_DIR_WRITE,SDA_DIR_IN);
	//DevIoctl(IIC_CLK_WRITE,TUNER_PIN_HIGH);	
	
	while(i--)
	{
		if(i==1)
			break;
		ioctl(DevIicFile,IIC_DATA_READ,&tmp);
		if(tmp==0)
		{
			break;;
		}	

	}
 	//printf("read value=%d,i=%d\n",tmp,i);
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_HIGH);		
	DevIoctl(IIC_CLK_WRITE,TUNER_PIN_LOW);
	DevIoctl(IIC_DATA_DIR_WRITE,SDA_DIR_OUT);
	if(i>1)
	{
		i=0;
	}
	else
	{
		IIC_Stop();
	}
	return i;
}
static void send8bit(U8 value)
{
	U8 i;
	U8 tmp;
	
	tmp=0x80;
	for(i=0;i<8;i++)
	{
		if(tmp&value)
		{
			DevIoctl(IIC_DATA_WRITE,TUNER_PIN_HIGH);
		}
		else
		{
			DevIoctl(IIC_DATA_WRITE,TUNER_PIN_LOW);
		}
		DevIoctl(IIC_CLK_WRITE,TUNER_PIN_HIGH);
		tmp>>=1;
		DevIoctl(IIC_CLK_WRITE,TUNER_PIN_LOW);
	}
}

TUNER_Error_Type TUNER_Write(U8 ChipAddr,U8 RegAddr, U8 *DataBuffer, U8 len,U8 StopFlag)
{
	U8 i;
	U8 tmpChipAddr,tmpRegAddr,tmpData;
	
	tmpChipAddr = ChipAddr;
	tmpRegAddr = RegAddr;
	IIC_Start();
	send8bit(tmpChipAddr);
	if(waitACK())
	{
		//printf("[TUNER_Write]Send Chip address!addr=%x\n",tmpChipAddr);
		//printf("[TUNER_Write]reg address!addr=%x\n",tmpRegAddr);
		return NO_ACK;
	}
	send8bit(tmpRegAddr);
//	printf("one ack\n");
	if(waitACK())
	{
		//printf("[TUNER_Write]Send Reg address error!\n");
		return NO_ACK;
	}
	for(i=0;i<len;i++)
	{
		//printf("write data i= %d\n",i);
		tmpData = *DataBuffer;
		send8bit(tmpData);
		if(waitACK())
		{
			printf("[TUNER_Write]send data error!\n");
			return NO_ACK;
		}
		DataBuffer++;
		Delay(50);
	}
	if(StopFlag)
	{
			IIC_Stop();
	}
	return NORMAL;
}
TUNER_Error_Type TUNER_Read(U8 ChipAddr, U8 *DataBuffer, U8 len)
{
	U8 j,k,m,tmp,readChar,tmpAddr;

	tmpAddr = ChipAddr+1;
	//printf("[TUNER_Read]tmpAddr=%x\n",tmpAddr);
	Delay(50);
	IIC_Start();
	send8bit(tmpAddr);
	if(waitACK())
	{
		//printf("**[TUNER_Read]Send Chip address!addr=%x\n",ChipAddr);
		return NO_ACK;
	}
	m = len;
	for(k=0;k<len;k++)
	{
		readChar = 0;
		j=8;
		DevIoctl(IIC_DATA_WRITE,TUNER_PIN_HIGH);
		DevIoctl(IIC_DATA_DIR_WRITE,SDA_DIR_IN);
		while(j)
		{
				j--;
				DevIoctl(IIC_CLK_WRITE,TUNER_PIN_HIGH);
				ioctl(DevIicFile,IIC_DATA_READ,&tmp);
				if(tmp>0)
					readChar = (1<<j) | readChar ;
				DevIoctl(IIC_CLK_WRITE,TUNER_PIN_LOW);
		}
		DevIoctl(IIC_DATA_DIR_WRITE,SDA_DIR_OUT);
		if(m==1)
		{
			SendNoACK();
		}
		else
		{
			SendACK();	
		}
		m--;
		*DataBuffer=readChar;
		DataBuffer++;
	}
	IIC_Stop();
	return NORMAL;
}
#else
U8 getTunerType(void);
void SetIICTunerType(U16 Type)
{
	IIC_TunerType = Type;
}
void SetTunerNo(U16 No)
{
	#if 1//ndef MK40
	TunerNo = No;
	#else
	
	switch(IIC_TunerType)
	{
		case DVB_C:
			if(No<2)
			{
				TunerNo = 0;
			}
			else
			{
				TunerNo = 2;
			}
			break;
		default:
			TunerNo = No;
			break;
	}
	//TUNER_DEBUG1("%s,Tuner NO:%x.\n",__func__,TunerNo);
	#endif
	
}

U16 GetTunerNO(void)
{
	if(TunerNo>4)
	{
		TUNER_DEBUG1("%s,error Tuner NO.\n",__func__);
	}
	return TunerNo;
}
static inline  void lock_fd_w(S32 fd)
{
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

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


S32 dev_lock(void)
{
    if(DevIicFile < 0)
    {
        return -1;
    }

    lock_fd_w(DevIicFile);
    return 0;
}

S32 dev_unLock(void)
{
    if(DevIicFile < 0)
    {
        return -1;
    }

    unlock_fd(DevIicFile);
    return 0;
}

//channel chipaddr regaddr stopf len  data
TUNER_Error_Type TUNER_Write(U8 ChipAddr,U8 RegAddr, U8 *DataBuffer, U8 len,U8 StopFlag)
{
	U8 tmpBuffer[200];
	U8 i;
	//U8 j;
	TUNER_Error_Type err = NORMAL;
	S32 Count=0;
	
	if(len<196)
	{
		dev_lock();
		i=0;
		tmpBuffer[i++] = TunerNo;
		tmpBuffer[i++] = ChipAddr;
		tmpBuffer[i++] = RegAddr;
		tmpBuffer[i++] = StopFlag;
		tmpBuffer[i++] = len;

		//TUNER_DEBUG("***[%s],addr:0x%x,len%d\n",__func__,ChipAddr,len);
		memcpy(&tmpBuffer[i],DataBuffer,len);
		Count =write(DevIicFile,tmpBuffer, len+i);
		if(Count<0)
		{
			write(DevIicFile,tmpBuffer, len+i);
		}
		else if(Count==1)
		{
			err = NO_ACK;
			//printf("\033[31m&&&&&&&TUNER_Write&&&ch:%d,&&&&&no ack chip:%x,Count:%d\033[m\n",TunerNo,ChipAddr,Count);
		}
		dev_unLock();
	}	
	return err;
}
TUNER_Error_Type TUNER_Read(U8 ChipAddr, U8 *DataBuffer, U8 len)
{
	U8 tmpBuffer[200];
	U8 i;
	//U8 j;
	TUNER_Error_Type err = NORMAL;
	S32 Count=0;
	
	if(len<196)
	{
		dev_lock();
		i=0;
		tmpBuffer[i++] = TunerNo;
		tmpBuffer[i++] = ChipAddr;
		//tmpBuffer[i++] = RegAddr;
		//tmpBuffer[i++] = StopFlag;
		tmpBuffer[i++] = len;

		//memcpy(&tmpBuffer[i],DataBuffer,len);
		Count =read(DevIicFile,tmpBuffer, i);
		if(Count<0)
		{
			
		}
		else if(Count==1)
		{
			err = NO_ACK;
			//printf("\033[31m&&&&&&&TUNER_Read&&&ch:%d,&&&&&no ack chip:%x\033[m\n",TunerNo,ChipAddr);
		}
		//TUNER_DEBUG("***[%s],addr:0x%x,len%d\n",__func__,ChipAddr,len);
		for(i=0;i<len;i++)
		{
			DataBuffer[i]= tmpBuffer[i];
			//TUNER_DEBUG("0x%x,",tmpBuffer[i]);
		}
		//TUNER_DEBUG("\n");
		dev_unLock();
	}	
	return err;
}
#endif
void EnablePower(void) 
{
	//DevIoctl(LNB_POWER_EN_WRITE,TUNER_PIN_LOW);//TUNER_PIN_HIGH
}
void	L14VPower(void) 
{
	//DevIoctl(LNB_SEL13OR18_WRITE,TUNER_PIN_LOW);
}
void L18VPower(void) 
{
	//DevIoctl(LNB_SEL13OR18_WRITE,TUNER_PIN_HIGH);
}
void	DisEnablePower(void) 
{
	//DevIoctl(LNB_POWER_EN_WRITE,TUNER_PIN_HIGH);//TUNER_PIN_LOW
}
void OpenTuner()
{
	
	DevIicFile = open(DEV_NAME, O_RDWR | O_NDELAY | O_NOCTTY);
	if( DevIicFile < 0)
	{
		perror("open dev  fail!");
	}
	else
	{
		printf("[%s]open %s success\n",__func__,DEV_NAME);
	}
}
void CloseTuner()
{
	close(DevIicFile)	;
}

void InitTunerIIC()
{
	OpenTuner();
	//ResetTuner();
}

