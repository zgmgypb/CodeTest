
#include "linuxplatform.h"
#include "multi_drv.h"
#include "multi_private.h"

#ifdef GM8358Q
#include "../../drivers/fpga_ps/fpga_ps_driver.h"
#endif

#ifdef GM8398Q
#include "../../drivers/fpga_ps_GM8398Q/fpga_ps_driver.h"
#endif

#ifdef GM8358Q

//#define FPGA_PS_IOCTL_MAGIE 'F'
//#define FPGA_PS_IOCTL_MAXNR 5
//#define FPGA_PS_SELECT  _IOW(FPGA_PS_IOCTL_MAGIE, 0, char) 
//#define FPGA_PS_PIN_INIT  _IOR(FPGA_PS_IOCTL_MAGIE, 1, char)
//#define FPGA_PS_PIN_END  _IOR(FPGA_PS_IOCTL_MAGIE, 2, char)
//#define FPGA_PS_CLEAN_PROG _IO(FPGA_PS_IOCTL_MAGIE, 3)
//#define FPGA_PS_GPIO_INIT _IO(FPGA_PS_IOCTL_MAGIE, 4)

#define FPGA_PS_DEV_NAME "/dev/fpga_ps_driver"


#define GN_RBF_SIZE_LIMIT	(1024*1024*1024)

/* FPGA配置：下载rbf到fpga芯片
	在项目中的所有FPGA均采用PS的方式进行rbf下载，下载完成后读fpgaID号进行判断是否下载成功，规定重试次数
*/
BOOL DRL_FpgaPSConfig(S32 FpgaIndex, FILE * pRbfFile, S32 FpgaCfgNodeFD)
{
	char lResult; 
	char *pDataBuf = NULL;
	int lFlen = 0;
	
	GLOBAL_FSEEK(pRbfFile, 0, SEEK_END);
	lFlen = GLOBAL_FTELL(pRbfFile);
	GLOBAL_FSEEK(pRbfFile, 0, SEEK_SET);

	if (lFlen > GN_RBF_SIZE_LIMIT) 
	{
		GLOBAL_TRACE(("Rbf feof error!!\n"));
		return FALSE;
	}


	if ((pDataBuf = (char*) GLOBAL_MALLOC(lFlen)) == NULL)
	{
		GLOBAL_TRACE(("GLOBAL_MALLOC Error\n"));
		return FALSE;
	}

	if (FpgaIndex == GN_FPGA_DVB_C_MODULE)
	{
		GLOBAL_TRACE(("USE FPGA CMD\n"));
	}
	else
	{
		/* PS开始的时序操作 */
		if (ioctl(FpgaCfgNodeFD, FPGA_PS_PIN_INIT, &lResult) < 0) 
		{
			GLOBAL_TRACE(("ioctl error\n"));
			GLOBAL_FREE(pDataBuf);	
			return FALSE;
		}
		else 
		{
			if (!lResult)
			{
				GLOBAL_TRACE(("FPGA_PS_PIN_INIT Failed"));
				GLOBAL_FREE(pDataBuf);	
				return FALSE;
			}
		}
	}

	/* 写入配置文件数据 */
	if (lFlen > 0)
	{
		if (lFlen !=GLOBAL_FREAD(pDataBuf, 1, lFlen, pRbfFile))
		{
			GLOBAL_TRACE(("fread error\n"));
			GLOBAL_FREE(pDataBuf);	
			return FALSE;
		}
		if (write(FpgaCfgNodeFD, pDataBuf, lFlen) < 0) /* 将数据写入 */
		{
			GLOBAL_TRACE(("write error\n"));
			GLOBAL_FREE(pDataBuf);	
			return FALSE;
		}
	}

	GLOBAL_FREE(pDataBuf);	

	if (FpgaIndex == GN_FPGA_DVB_C_MODULE)
	{
		GLOBAL_TRACE(("USE FPGA CMD\n"));
	}
	else
	{
		/* PS结束判断 */
		if (ioctl(FpgaCfgNodeFD, FPGA_PS_PIN_END, &lResult) < 0) 
		{
			GLOBAL_TRACE(("ioctl error\n"));
			return FALSE;
		}
		else 
		{
			if (!lResult)
			{
				GLOBAL_TRACE(("FPGA_PS_PIN_END Failed"));
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL DRL_FpgaConfig(U32 FpgaIndex)
{
	FILE *lFpgaFileFD;
	S32 lFpgaCfgNodeFD;
	CHAR_T lRBFFilePathname[256];
	char lData;

	switch (FpgaIndex)
	{
	case GN_FPGA_INDEX_MAIN:
		GLOBAL_STRCPY(lRBFFilePathname, HWL_MAIN_FPGA_FILE_PATH);
		break;
	case GN_FPGA_INDEX_IP_ASI_OUTPUT:
		GLOBAL_STRCPY(lRBFFilePathname, HWL_MUX_OUT_FPGA_FILE_PATH);
		break;
	case GN_FPGA_DVB_C_MODULE:
		GLOBAL_STRCPY(lRBFFilePathname, HWL_DVB_C_OUT_FPGA_FILE_PATH);
		break;
	default: break;
	}

	lFpgaCfgNodeFD = open(FPGA_PS_DEV_NAME, O_RDWR | O_NOCTTY | O_NDELAY);
	if (lFpgaCfgNodeFD < 0) 
	{
		GLOBAL_TRACE(("GLOBAL_OPEN File %s Failed\n", FPGA_PS_DEV_NAME));
		return FALSE;
	} 

	if (FpgaIndex == GN_FPGA_DVB_C_MODULE)
	{
		lData = (char)(GN_FPGA_INDEX_IP_ASI_OUTPUT & 0xff);
	}
	else
	{
		lData = (char)(FpgaIndex & 0xff);
	}

	if (ioctl(lFpgaCfgNodeFD, FPGA_PS_SELECT, &lData) < 0) 
	{
		GLOBAL_TRACE(("ioctl error\n"));
		return FALSE;
	}


	if (ioctl(lFpgaCfgNodeFD, FPGA_PS_GPIO_INIT) < 0) /* GPIO初始化 */
	{
		GLOBAL_TRACE(("ioctl err\n"));
		close(lFpgaCfgNodeFD);
		return FALSE;
	}

	lFpgaFileFD = GLOBAL_FOPEN(lRBFFilePathname , "rb") ;
	if (lFpgaFileFD == NULL)
	{
		GLOBAL_TRACE(("GLOBAL_FOPEN File %s Failed\n", lRBFFilePathname));
		close(lFpgaCfgNodeFD);
		return FALSE;
	}

	if (DRL_FpgaPSConfig(FpgaIndex, lFpgaFileFD, lFpgaCfgNodeFD) == FALSE) 
	{
		GLOBAL_TRACE(("Config Err\n"));
		GLOBAL_FCLOSE(lFpgaFileFD);
		close(lFpgaCfgNodeFD);
		return FALSE;
	}

	GLOBAL_FCLOSE(lFpgaFileFD);
	close(lFpgaCfgNodeFD);

	return TRUE;
}
#endif

#ifdef GM8398Q
//#define FPGA_PS_IOCTL_MAGIE 'F'
//#define FPGA_PS_IOCTL_MAXNR 5
//#define FPGA_PS_SELECT  _IOW(FPGA_PS_IOCTL_MAGIE, 0, char) 
//#define FPGA_PS_PIN_INIT  _IOR(FPGA_PS_IOCTL_MAGIE, 1, char)
//#define FPGA_PS_PIN_END  _IOR(FPGA_PS_IOCTL_MAGIE, 2, char)
//#define FPGA_PS_CLEAN_PROG _IO(FPGA_PS_IOCTL_MAGIE, 3)
//#define FPGA_PS_GPIO_INIT _IO(FPGA_PS_IOCTL_MAGIE, 4)

#define FPGA_PS_DEV_NAME "/dev/fpga_ps_driver"


#define GN_RBF_SIZE_LIMIT	(1024*1024*1024)

/* FPGA配置：下载rbf到fpga芯片
	在项目中的所有FPGA均采用PS的方式进行rbf下载，下载完成后读fpgaID号进行判断是否下载成功，规定重试次数
*/
BOOL DRL_FpgaPSConfig(S32 FpgaIndex, FILE * pRbfFile, S32 FpgaCfgNodeFD)
{
	char lResult; 
	char *pDataBuf = NULL;
	int lFlen = 0;
	
	GLOBAL_FSEEK(pRbfFile, 0, SEEK_END);
	lFlen = GLOBAL_FTELL(pRbfFile);
	GLOBAL_FSEEK(pRbfFile, 0, SEEK_SET);

	if (lFlen > GN_RBF_SIZE_LIMIT) 
	{
		GLOBAL_TRACE(("Rbf feof error!!\n"));
		return FALSE;
	}


	if ((pDataBuf = (char*) GLOBAL_MALLOC(lFlen)) == NULL)
	{
		GLOBAL_TRACE(("GLOBAL_MALLOC Error\n"));
		return FALSE;
	}

	if (FpgaIndex == GN_FPGA_DVB_C_MODULE)
	{
		GLOBAL_TRACE(("USE FPGA CMD\n"));
	}
	else
	{
		/* PS开始的时序操作 */
		if (ioctl(FpgaCfgNodeFD, FPGA_PS_PIN_INIT, &lResult) < 0) 
		{
			GLOBAL_TRACE(("ioctl error\n"));
			GLOBAL_FREE(pDataBuf);	
			return FALSE;
		}
		else 
		{
			if (!lResult)
			{
				GLOBAL_TRACE(("FPGA_PS_PIN_INIT Failed"));
				GLOBAL_FREE(pDataBuf);	
				return FALSE;
			}
		}
	}

	/* 写入配置文件数据 */
	if (lFlen > 0)
	{
		if (lFlen !=GLOBAL_FREAD(pDataBuf, 1, lFlen, pRbfFile))
		{
			GLOBAL_TRACE(("fread error\n"));
			GLOBAL_FREE(pDataBuf);	
			return FALSE;
		}
		if (write(FpgaCfgNodeFD, pDataBuf, lFlen) < 0) /* 将数据写入 */
		{
			GLOBAL_TRACE(("write error\n"));
			GLOBAL_FREE(pDataBuf);	
			return FALSE;
		}
	}

	GLOBAL_FREE(pDataBuf);	

	if (FpgaIndex == GN_FPGA_DVB_C_MODULE)
	{
		GLOBAL_TRACE(("USE FPGA CMD\n"));
	}
	else
	{
		/* PS结束判断 */
		if (ioctl(FpgaCfgNodeFD, FPGA_PS_PIN_END, &lResult) < 0) 
		{
			GLOBAL_TRACE(("ioctl error\n"));
			return FALSE;
		}
		else 
		{
			if (!lResult)
			{
				GLOBAL_TRACE(("FPGA_PS_PIN_END Failed"));
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL DRL_FpgaConfig(U32 FpgaIndex)
{
	FILE *lFpgaFileFD;
	S32 lFpgaCfgNodeFD;
	CHAR_T lRBFFilePathname[256];
	char lData;

	switch (FpgaIndex)
	{
	case GN_FPGA_INDEX_MAIN:
		GLOBAL_STRCPY(lRBFFilePathname, HWL_MAIN_FPGA_FILE_PATH);
		break;
	case GN_FPGA_INDEX_IP_ASI_OUTPUT:
		GLOBAL_STRCPY(lRBFFilePathname, HWL_MUX_OUT_FPGA_FILE_PATH);
		break;
	case GN_FPGA_DVB_C_MODULE:
		GLOBAL_STRCPY(lRBFFilePathname, HWL_DVB_C_OUT_FPGA_FILE_PATH);
		break;
	default: break;
	}

	lFpgaCfgNodeFD = open(FPGA_PS_DEV_NAME, O_RDWR | O_NOCTTY | O_NDELAY);
	if (lFpgaCfgNodeFD < 0) 
	{
		GLOBAL_TRACE(("GLOBAL_OPEN File %s Failed\n", FPGA_PS_DEV_NAME));
		return FALSE;
	} 

	if (FpgaIndex == GN_FPGA_DVB_C_MODULE)
	{
		lData = (char)(GN_FPGA_INDEX_IP_ASI_OUTPUT & 0xff);
	}
	else
	{
		lData = (char)(FpgaIndex & 0xff);
	}

	if (ioctl(lFpgaCfgNodeFD, FPGA_PS_SELECT, &lData) < 0) 
	{
		GLOBAL_TRACE(("ioctl error\n"));
		return FALSE;
	}


	if (ioctl(lFpgaCfgNodeFD, FPGA_PS_GPIO_INIT) < 0) /* GPIO初始化 */
	{
		GLOBAL_TRACE(("ioctl err\n"));
		close(lFpgaCfgNodeFD);
		return FALSE;
	}

	lFpgaFileFD = GLOBAL_FOPEN(lRBFFilePathname , "rb") ;
	if (lFpgaFileFD == NULL)
	{
		GLOBAL_TRACE(("GLOBAL_FOPEN File %s Failed\n", lRBFFilePathname));
		close(lFpgaCfgNodeFD);
		return FALSE;
	}

	if (DRL_FpgaPSConfig(FpgaIndex, lFpgaFileFD, lFpgaCfgNodeFD) == FALSE) 
	{
		GLOBAL_TRACE(("Config Err\n"));
		GLOBAL_FCLOSE(lFpgaFileFD);
		close(lFpgaCfgNodeFD);
		return FALSE;
	}

	GLOBAL_FCLOSE(lFpgaFileFD);
	close(lFpgaCfgNodeFD);

	return TRUE;
}
#endif
