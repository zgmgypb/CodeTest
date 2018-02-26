#ifndef HWL_DEVICE_H
#define HWL_DEVICE_H

#include "global_def.h"
#include "platform_assist.h"

/*PS模式配置FPGA逻辑------------------------------------------------------------------------------------------------------------------------*/
int DRL_FpgaConfiguration(const char *dev_path, const char *fpga_path);
BOOL DRL_ZYNQInitiate(void);
/*
BitValue = 0x01 ； 【PB17，PB16 = b01】为3C55的板子（旧）电阻R461和R560， 
BitValue = 0x00 ； 【PB17，PB16 = b00】为4C55的板子（新）电阻R561和R560， 
*/
#define GM2730X_4CE55_VALUE 0x00
#define GM2730X_3C55_VALUE 0x01

U32 DRL_GetMainBoardVersionBitValue(void);
BOOL DRL_GetMainBoardIS4CE55(void);
/*基本控制------------------------------------------------------------------------------------------------------------------------*/
int DRL_ICPInitate(void);
void DRL_FpgaDestroy(void);

/*数据流模式------------------------------------------------------------------------------------------------------------------------*/

/** 该函数适用于写入一个完整的命令
 * 内部自动获取和释放锁.,永远从seek==0 开始进行写入操作.
 * 当命令在@(buf)中准备完成后，本函数会互斥的将其写入FPGA.
 */
int DRL_FpgaWriteLock(unsigned char *buf , unsigned int size );

/**
 * 从FPGA模块中读入数据到缓冲区，缓冲区大小应该为 .至少>=1024
 * @(buff):读入数据缓冲区.
 * @(size):缓冲区大小.
 * @(return): 如果@(return)<4 则说明读入数据无效.
 */
int DRL_FpgaReadLock(unsigned char *buff, unsigned int size);



/*地址模式------------------------------------------------------------------------------------------------------------------------*/


/*风扇控制------------------------------------------------------------------------------------------------------------------------*/
#define HWL_CONST_ON		1
#define HWL_CONST_OFF		0
int DRL_FanInit(void);
int DRL_FanStatusSet(int val);
int DRL_FanStatusGet(void);
void DRL_FanDestroy(void);

int DRL_FanHwGet(void);



BOOL DRL_FpgaConfig(U32 FpgaIndex);

/*GM-8358Q的接口*/
BOOL Init_FPGA( void );
unsigned short READ_FPGA(unsigned char B_TYPE, unsigned long ADDRESS);
void WRITE_FPGA(unsigned char B_TYPE, unsigned long ADDRESS, int DATA);

#ifdef GN1846
S32 DRL_FspiWrite(U16 RegAddr, U8 *pBuf, U32 BufSize);
S32 DRL_FspiRead(U16 RegAddr, U8 *pBuf, U32 BufSize);
BOOL DRL_GetFspiComIsOk(void);
#endif

#endif

