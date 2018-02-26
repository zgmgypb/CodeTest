#include <sys/time.h> /* gettimeofday() */
#include "os_assist.h"
#include <sys/ioctl.h>
#include <stdio.h>
//#include <net/if.h>

/******************************************************************************
**
** Copyright 2014 gospell Corporation All RIGHTS Reserved                      
**
*******************************************************************************
** File Name      : RngBuf.c
** Version        : v1.0
** Author         : dh.tang <xuanhunshi@hotmail.com>
** Created        : 2014.9.25
** Modification   : Created file
** Description    : 环形缓冲区
******************************************************************************/

/* Macro Define */
#define min(X,Y) (((X) < (Y)) ? (X) : (Y))

/* Function List*/
/*****************************************************************************
 Function			:RngCreate
 Description		:创建缓冲区
 Date			:2014/9/25
 Author			:dh.tang <xuanhunshi@hotmail.com>
 Modification		:Created
*****************************************************************************/
RNGBUF_t * RngCreate(int nbytes)
{
	char * buffer;
	RNGBUF_t * pBuf = (RNGBUF_t *)malloc(sizeof (RNGBUF_t));
	if (!pBuf)
		return (NULL);

	buffer = (char *)malloc(++nbytes);
	if (buffer == NULL)
	{
		free(pBuf);
		return (NULL);
	}
	
	pBuf->m_Lock = PFC_SemaphoreCreate("RngBuf Lock", 1);
	if (!pBuf->m_Lock) {
		free(pBuf);
		free(buffer);
		return (NULL);
	}
	PFC_SemaphoreSignal(pBuf->m_Lock);

	pBuf->bufSize = nbytes;
	pBuf->buf = buffer;
	RngFlush(pBuf);
	
	return (pBuf);
}

/*****************************************************************************
 Function			:RngFlush
 Description		:清空缓冲区
 Date			:2014/9/25
 Author			:dh.tang <xuanhunshi@hotmail.com>
 Modification		:Created
*****************************************************************************/
void RngFlush( RNGBUF_t * pBuf)
{
	PFC_SemaphoreWait(pBuf->m_Lock, -1);
    pBuf->pWritePos = 0;
    pBuf->pReadPos = 0;
	PFC_SemaphoreSignal(pBuf->m_Lock);
}

/*****************************************************************************
 Function			:RngBufGet
 Description		:向缓冲区取数据
 Date			:2014/9/25
 Author			:dh.tang <xuanhunshi@hotmail.com>
 Modification		:Created
*****************************************************************************/
int RngBufGet( RNGBUF_t * pBuf,char *buffer, int maxbytes)
{
	int bytesgot = 0;
	int pWritePos;
	int bytes2;
	int pRngTmp = 0;

	PFC_SemaphoreWait(pBuf->m_Lock, -1);
	pWritePos = pBuf->pWritePos;
	if (pWritePos >= pBuf->pReadPos)
	{
		/* pWritePos has not wrapped around */

		bytesgot = min(maxbytes, pWritePos - pBuf->pReadPos);
		memcpy(buffer,&pBuf->buf[pBuf->pReadPos],bytesgot);
		pBuf->pReadPos += bytesgot;
	}
	else
	{
		/* pWritePos has wrapped around.  Grab chars up to the end of the
		* buffer, then wrap around if we need to. */

		bytesgot = min(maxbytes, pBuf->bufSize - pBuf->pReadPos);
		memcpy( buffer,&pBuf->buf [pBuf->pReadPos], bytesgot);
		pRngTmp = pBuf->pReadPos + bytesgot;

		/* If pReadPos is equal to bufSize, we've read the entire buffer,
		* and need to wrap now.  If bytesgot < maxbytes, copy some more chars
		* in now. */

		if (pRngTmp == pBuf->bufSize)
		{
			bytes2 = min(maxbytes - bytesgot, pWritePos);
			memcpy( buffer + bytesgot,pBuf->buf, bytes2);
			pBuf->pReadPos = bytes2;
			bytesgot += bytes2;
		}
		else
			pBuf->pReadPos = pRngTmp;
	}

	PFC_SemaphoreSignal(pBuf->m_Lock);
	return (bytesgot);
}

/*****************************************************************************
 Function			:RngBufPut
 Description		:向缓冲区中放数据(非覆盖)
 Date			:2014/9/25
 Author			:dh.tang <xuanhunshi@hotmail.com>
 Modification		:Created
*****************************************************************************/
int RngBufPut( RNGBUF_t * pBuf,char *buffer,int nbytes)
{
	int bytesput = 0;
	int pReadPos;
	int bytes2;
	int pRngTmp = 0;

	PFC_SemaphoreWait(pBuf->m_Lock, -1);
	pReadPos = pBuf->pReadPos;
	if (pReadPos > pBuf->pWritePos)
	{
		/* pReadPos is ahead of pWritePos.  We can fill up to two bytes
		* before it */

		bytesput = min (nbytes, pReadPos - pBuf->pWritePos - 1);
		memcpy(&pBuf->buf [pBuf->pWritePos],buffer, bytesput);
		pBuf->pWritePos += bytesput;
	}
	else if (pReadPos == 0)
	{
		/* pReadPos is at the beginning of the buffer.  We can fill till
		* the next-to-last element */

		bytesput = min(nbytes, pBuf->bufSize - pBuf->pWritePos - 1);
		memcpy(&pBuf->buf [pBuf->pWritePos],buffer,  bytesput);
		pBuf->pWritePos += bytesput;
	}
	else
	{
		/* pReadPos has wrapped around, and its not 0, so we can fill
		* at least to the end of the ring buffer.  Do so, then see if
		* we need to wrap and put more at the beginning of the buffer. */

		bytesput = min(nbytes, pBuf->bufSize - pBuf->pWritePos);
		memcpy(&pBuf->buf [pBuf->pWritePos],buffer,  bytesput);
		pRngTmp = pBuf->pWritePos + bytesput;

		if (pRngTmp == pBuf->bufSize)
		{
			/* We need to wrap, and perhaps put some more chars */

			bytes2 = min (nbytes - bytesput, pReadPos - 1);
			memcpy( pBuf->buf,buffer + bytesput, bytes2);
			pBuf->pWritePos = bytes2;
			bytesput += bytes2;
		}
		else
			pBuf->pWritePos = pRngTmp;
	}
	PFC_SemaphoreSignal(pBuf->m_Lock);
	return (bytesput);
}

/*****************************************************************************
 Function			:RngIsEmpty
 Description		:缓冲区是否空
 Date			:2014/9/25
 Author			:dh.tang <xuanhunshi@hotmail.com>
 Modification		:Created
*****************************************************************************/
int RngIsEmpty(RNGBUF_t * pBuf)
{
	return (pBuf->pWritePos == pBuf->pReadPos);
}


/*****************************************************************************
 Function			:RngIsFull
 Description		:缓冲区是否满
 Date			:2014/9/25
 Author			:dh.tang <xuanhunshi@hotmail.com>
 Modification		:Created
*****************************************************************************/
int RngIsFull(RNGBUF_t * pBuf)
{
	int n;
	int lRet;

	PFC_SemaphoreWait(pBuf->m_Lock, -1);
	n = pBuf->pWritePos - pBuf->pReadPos + 1;
	lRet = ((n == 0) || (n == pBuf->bufSize));
	PFC_SemaphoreSignal(pBuf->m_Lock);

	return lRet;
}


/*****************************************************************************
 Function			:RngFreeBytes
 Description		:获取缓冲区剩余空间
 Date			:2014/9/25
 Author			:dh.tang <xuanhunshi@hotmail.com>
 Modification		:Created
*****************************************************************************/
int RngFreeBytes(RNGBUF_t * pBuf)
{
	int n;

	PFC_SemaphoreWait(pBuf->m_Lock, -1);
	n = pBuf->pReadPos - pBuf->pWritePos - 1;
	if (n < 0)
		n += pBuf->bufSize;
	PFC_SemaphoreSignal(pBuf->m_Lock);

	return (n);
}

/*****************************************************************************
 Function			:RngNBytes
 Description		:获取缓冲区当前使用量
 Date			:2014/9/25
 Author			:dh.tang <xuanhunshi@hotmail.com>
 Modification		:Created
*****************************************************************************/
int RngNBytes(RNGBUF_t * pBuf)
{
	int n;

	PFC_SemaphoreWait(pBuf->m_Lock, -1);
	n = pBuf->pWritePos - pBuf->pReadPos;
	if (n < 0)
		n += pBuf->bufSize;
	PFC_SemaphoreSignal(pBuf->m_Lock);

	return (n);
}

void RngDestroy(RNGBUF_t *pBuf)
{
	if (pBuf) {
		if (pBuf->buf) {
			free(pBuf->buf);
			pBuf->buf = NULL;
		}
		free(pBuf);
		pBuf = NULL;
	}
}

void OS_TimeNow(S64 *p64_us_time)
{
	struct timespec time_spec;

	if (clock_gettime(CLOCK_REALTIME, &time_spec) != 0) {
		GLOBAL_TRACE(("clock_gettime err!\n"));
	}
	*p64_us_time = (S64)time_spec.tv_sec * 1000000 + (S64)time_spec.tv_nsec/1000;
}

/* 网络连接检测 */
struct ethtool_value {
	__uint32_t      cmd;
	__uint32_t      data;
};
BOOL OS_EthLinkIsOk(U8 *pEthName)
{
	struct ethtool_value edata;
	int fd = -1, err = 0;
	struct ifreq ifr;
	BOOL ret = TRUE;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, pEthName);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return FALSE;
	}

	edata.cmd = 0x0000000a;
	ifr.ifr_data = (caddr_t)&edata;
	err = ioctl(fd, 0x8946, &ifr);
	if (err == 0) {
		//fprintf(stdout, "Link detected: %s/n",
		//	edata.data ? "yes":"no");
		ret = (edata.data ? TRUE : FALSE);
	} else if (errno != EOPNOTSUPP) {
		perror("Cannot get link status");
		ret = FALSE;
	}
	
	close(fd);

	return ret;
}