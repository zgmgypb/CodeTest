#ifndef GS_COMM_H
#define GS_COMM_H

/* Linux Base Header */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h> /* for open/close */
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h> /* for ioctl */
#include <limits.h>
#include <signal.h>
#include <syslog.h> /* for syslog */

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/* Global Macros */
typedef unsigned char           GS_U8;
typedef unsigned short          GS_U16;
typedef unsigned int            GS_U32;
typedef double                  GS_DOUBLE;
typedef signed char             GS_S8;
typedef short                   GS_S16;
typedef int                     GS_S32;
typedef char					GS_CHAR;
typedef long					GS_LONG;
typedef void *					GS_HANDLE;

#ifndef _M_IX86
    typedef unsigned long long  GS_U64;
    typedef long long           GS_S64;
#else
    typedef __int64             GS_U64;
    typedef __int64             GS_S64;
#endif

#define GS_VOID                 void

typedef enum {
    GS_FALSE = 0,
    GS_TRUE  = 1,
} GS_BOOL;

#ifndef NULL
    #define NULL    0L
#endif

#define GS_NULL     0L
#define GS_SUCCESS  0
#define GS_FAILURE  (-1)

/*
 * ANSI terminal
 */
#define ANSI_CURSOR_UP            "\e[%dA"
#define ANSI_CURSOR_DOWN          "\e[%dB"
#define ANSI_CURSOR_FORWARD       "\e[%dC"
#define ANSI_CURSOR_BACK          "\e[%dD"
#define ANSI_CURSOR_NEXTLINE      "\e[%dE"
#define ANSI_CURSOR_PREVIOUSLINE  "\e[%dF"
#define ANSI_CURSOR_COLUMN        "\e[%dG"
#define ANSI_CURSOR_POSITION      "\e[%d;%dH"
#define ANSI_CURSOR_SHOW          "\e[?25h"
#define ANSI_CURSOR_HIDE          "\e[?25l"
#define ANSI_CLEAR_CONSOLE        "\e[2J"
#define ANSI_CLEAR_LINE_TO_END    "\e[0K"
#define ANSI_CLEAR_LINE           "\e[2K"
#define ANSI_COLOR_RESET          "\e[0m"
#define ANSI_COLOR_REVERSE        "\e[7m"

/* 打印颜色定义*/
#define ANSI_COLOR_NONE           "\033[0m"
#define ANSI_COLOR_RED            "\033[0;31m"
#define ANSI_COLOR_LIGHT_RED      "\033[1;31m"
#define ANSI_COLOR_GREEN          "\033[0;32m"
#define ANSI_COLOR_LIGHT_GREEN    "\033[1;32m"
#define ANSI_COLOR_BLUE           "\033[0;34m"
#define ANSI_COLOR_LIGHT_BLUE     "\033[1;34m"
#define ANSI_COLOR_DARY_GRAY      "\033[1;30m"
#define ANSI_COLOR_CYAN           "\033[0;36m"
#define ANSI_COLOR_LIGHT_CYAN     "\033[1;36m"
#define ANSI_COLOR_PURPLE         "\033[0;35m"
#define ANSI_COLOR_LIGHT_PURPLE   "\033[1;35m"
#define ANSI_COLOR_BROWN          "\033[0;33m"
#define ANSI_COLOR_YELLOW         "\033[1;33m"
#define ANSI_COLOR_LIGHT_GRAY     "\033[0;37m"
#define ANSI_COLOR_WHITE          "\033[1;37m"

/* Debug Print */
enum Gs_DebugLevel{
	GS_DBG_LVL_DISABLE = 0, /* 没有调试信息允许输出 */
	GS_DBG_LVL_ERR, /* 仅仅错误信息被允许输出 */
	GS_DBG_LVL_WRN, /* 警告条件,对可能出现问题的情况进行警告 */
	GS_DBG_LVL_INFO, /* 提示信息 */
	GS_DBG_LVL_DBG /* 开启所有调试 */
};
#define GS_DEBUG_SET_LEVEL(x) static int _GS_DEBUG_LEVEL = (x) /* 在需要调试的文件前加该句，指定调试打印级别 */

#define GS_ISDEBUG() (_GS_DEBUG_LEVEL >= GS_DBG_LVL_DBG)
#define GS_DBGERR(fmt, ...) \
	do { \
		if (_GS_DEBUG_LEVEL >= GS_DBG_LVL_ERR) { \
			GS_COMM_Printf(ANSI_COLOR_RED"[ERR ]"ANSI_COLOR_NONE"[%s][%s][%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
		} \
	} while (0)
#define GS_DBGWRN(fmt, ...) \
	do { \
		if (_GS_DEBUG_LEVEL >= GS_DBG_LVL_WRN) { \
			GS_COMM_Printf(ANSI_COLOR_YELLOW"[WRN ]"ANSI_COLOR_NONE"[%s][%s][%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
		} \
	} while (0)
#define GS_DBGINFO(fmt, ...) \
	do { \
		if (_GS_DEBUG_LEVEL >= GS_DBG_LVL_INFO) { \
			GS_COMM_Printf(ANSI_COLOR_GREEN"[INFO]"ANSI_COLOR_NONE"[%s][%s][%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
		} \
	} while (0)
#define GS_DBGWHERE() \
	do { \
		if (_GS_DEBUG_LEVEL >= GS_DBG_LVL_DBG) { \
			GS_COMM_Printf(ANSI_COLOR_CYAN"[POS ]"ANSI_COLOR_NONE"[%s][%s][%d]\n", __FILE__, __FUNCTION__, __LINE__); \
		} \
	} while (0) /* 定位 */

/* trace print */
#define GS_TRACE(fmt, ...) \
	do { \
		GS_COMM_Printf("[%s][%s][%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
	} while (0)

/* 整数比较 */
#define GS_MIN(a, b) (((a) > (b)) ? (b) : (a))
#define GS_MAX(a, b) (((a)>(b)) ? (a) : (b))

/* 位操作 */
#define GS_SET_BIT(x, y)	do {(x) |= (1 << (y));} while(0) /* 将X的第Y位置1 */
#define GS_CLEAR_BIT(x, y)	do {(x) &= ~(1 << (y));} while(0) /* 将X的第Y位清0 */
#define GS_IS_BIT(x, y)		((((x) & (1 << (y))) != 0) ? TRUE : FALSE) /* 判断x的第y位是否是1，如果是1则表达式为真 */

/* 低位在前,打包 */
#define GS_LSB8_E(b, x) \
	do { \
		*(GS_CHAR *)(b) = (x) & 0xFF; \
	} while(0) /* b为存放单字节数据的buffer，x为多字节数据 */
#define GS_LSB16_E(b, x) \
	do { \
		GS_CHAR *p = (GS_CHAR *)(b); \
		*p++ = (x) & 0xFF; \
		*p++ = ((x) >> 8) & 0xFF; \
	} while(0)
#define GS_LSB24_E(b, x) \
	do { \
		GS_CHAR *p = (GS_CHAR *)(b); \
		*p++ = (x) & 0xFF; \
		*p++ = ((x) >> 8) & 0xFF; \
		*p++ = ((x) >> 16) & 0xFF; \
	} while(0)
#define GS_LSB32_E(b, x) \
	do { \
		GS_CHAR *p = (GS_CHAR *)(b); \
		*p++ = (x) & 0xFF; \
		*p++ = ((x) >> 8) & 0xFF; \
		*p++ = ((x) >> 16) & 0xFF; \
		*p++ = ((x) >> 24) & 0xFF; \
	} while(0)

/* 低位在前,解包 */
#define GS_LSB8_D(b, x)		do {x = *(GS_U8 *)(b);} while(0) /* b为存放单字节数据的buffer，x为多字节数据 */
#define GS_LSB16_D(b, x)	do {GS_U8 *p = (GS_U8 *)(b); x = (p[1] << 8) | p[0];} while(0)
#define GS_LSB24_D(b, x)	do {GS_U8 *p = (GS_U8 *)(b); x = (p[2] << 16) | (p[1] << 8) | p[0];} while(0)
#define GS_LSB32_D(b, x)	do {GS_U8 *p = (GS_U8 *)(b); x = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];} while(0)

/* 高位在前,打包 */
#define GS_MSB8_E(b, x) \
	do { \
		*(GS_U8 *)(b) = (x) & 0xFF; \
	} while(0) /* b为存放单字节数据的buffer，x为多字节数据 */
#define GS_MSB16_E(b, x) \
	do { \
		GS_U8 *p = (GS_U8 *)(b); \
		*p++ = ((x) >> 8) & 0xFF; \
		*p++ = (x) & 0xFF; \
	} while(0)
#define GS_MSB24_E(b, x) \
	do { \
		GS_U8 *p = (GS_U8 *)(b); \
		*p++ = ((x) >> 16) & 0xFF; \
		*p++ = ((x) >> 8) & 0xFF; \
		*p++ = (x) & 0xFF; \
	} while(0)
#define GS_MSB32_E(b, x) \
	do { \
		GS_U8 *p = (GS_U8 *)(b); \
		*p++ = ((x) >> 24) & 0xFF; \
		*p++ = ((x) >> 16) & 0xFF; \
		*p++ = ((x) >> 8) & 0xFF; \
		*p++ = (x) & 0xFF; \
	} while(0)

/* 高位在前,解包 */
#define GS_MSB8_D(b, x)		do {x = *(GS_U8 *)(b);} while(0) /* b为存放单字节数据的buffer，x为多字节数据 */
#define GS_MSB16_D(b, x)	do {GS_U8 *p = (GS_U8 *)(b); x = (p[0] << 8) | p[1];} while(0)
#define GS_MSB24_D(b, x)	do {GS_U8 *p = (GS_U8 *)(b); x = (p[0] << 16) | (p[1] << 8) | p[2];} while(0)
#define GS_MSB32_D(b, x)	do {GS_U8 *p = (GS_U8 *)(b); x = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];} while(0)

/* Global Functions Define */
GS_VOID GS_COMM_Printf(const GS_CHAR *format, ...);
GS_S32 GS_COMM_System(const GS_CHAR *pCmd, ...);
GS_VOID GS_COMM_PrintDataBlock(GS_CHAR* pTitle, GS_U8 *pBuf, GS_S32 Size);
GS_U64 GS_COMM_GetTickCount(GS_VOID); /* 获取系统Tick值 */
GS_S32 GS_COMM_GetChar(GS_VOID);
GS_U32 GS_COMM_Crc32Calc(GS_U8 *pData, GS_U32 Size); /* CRC32 计算 */

/* 用户 Buffer 操作 */
GS_HANDLE GS_COMM_UserBufCreate(GS_S32 Len);
GS_VOID GS_COMM_UserBufDestroy(GS_HANDLE Handle);
GS_U8 *GS_COMM_UserBufMalloc(GS_HANDLE Handle, GS_S32 Len);

/* 错误处理函数 */
GS_VOID GS_COMM_ErrExit(GS_S32 ErrNo, const GS_CHAR *Fmt, ...); /* 系统函数调用并错误，传入错误号打印退出 */
GS_VOID GS_COMM_ErrSys(const GS_CHAR *Fmt, ...); /* 系统错误，打印退出 */
GS_VOID GS_COMM_ErrRet(const GS_CHAR *Fmt, ...); /* 非致命系统错误，打印返回 */
GS_VOID GS_COMM_ErrDump(const GS_CHAR *Fmt, ...); /* 致命系统错误，打印转储内核信息并退出 */
GS_VOID GS_COMM_ErrMsg(const GS_CHAR *Fmt, ...); /* 非系统错误，打印信息并返回 */
GS_VOID GS_COMM_ErrQuit(const GS_CHAR *Fmt, ...); /* 非系统致命错误，打印信息并退出 */

/* ---消息队列模块--- */
typedef struct  
{
	GS_LONG	m_MsgType; /* 消息类型 */
	struct {
		GS_S32	m_MsgTag;
		GS_S32	m_MsgParam1;
		GS_S32	m_MsgParam2;
		GS_S32	m_MsgDataLen; /* m_MsgData数据长度 */
		GS_U8	*m_pMsgData;
	} m_MsgText; /* 消息内容 */	
}GS_MSGBUF_S;

GS_HANDLE GS_COMM_MsgQueueCreate(GS_S32 MaxQueueSize); 
GS_S32 GS_COMM_MsgQueueDestroy(GS_HANDLE Handle);
GS_S32 GS_COMM_MsgQueueSend(GS_HANDLE Handle, GS_MSGBUF_S *pMsg, GS_BOOL IsBlock);
GS_S32 GS_COMM_MsgQueueRecv(GS_HANDLE Handle, GS_MSGBUF_S *pMsg, GS_LONG MsgType, GS_BOOL IsBlock);
/* ---消息队列模块 End--- */

/* ---环形 Buffer 模块--- */
GS_HANDLE GS_COMM_RingBufferCreate(GS_S32 BuffSize);
GS_VOID GS_COMM_RingBufferDestroy(GS_HANDLE Handle);
GS_S32 GS_COMM_RingBufferGetUsedSize(GS_HANDLE Handle);
GS_S32 GS_COMM_RingBufferWrite(GS_HANDLE Handle, GS_U8 *pBuff, GS_S32 BuffSize);
GS_S32 GS_COMM_RingBufferRead(GS_HANDLE Handle, GS_U8 *pBuff, GS_S32 BuffSize);
GS_VOID GS_COMM_RingBufferInfoPrint(GS_HANDLE Handle);
/* ---环形 Buffer 模块 End--- */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* GS_COMM_H */
/* EOF */
