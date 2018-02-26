#include "multi_main.h"
#include "multi_drv.h"
#include "linuxplatform.h"


//#define TEST_MODULES

int g_auto_reboot_times = 0;


#ifdef TEST_MODULES
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
Tuner_Notify TEST_SetI2cAndTunerReset(U8 ChannelNo, U8 Reset,U8 LNBPower)
{                                                                                                                                                          
}

#endif

int main(int argc, char ** argv)
{
	int lWaitStatus;
	pid_t lProcessPIDs;

	PFC_System("rm -rf /tmp/web/tmp/*");//删除遗留的XML文件

#ifdef zynq
	PFC_System("mkdir -p /lib/modules/4.4.0-xilinx");//给ZYNQ新内核建立文件
	PFC_System("sysctl -w net.core.rmem_max=104857600");//设置网络输入缓存最大值
	PFC_System("sysctl -w net.core.wmem_max=104857600");//设置网络输出缓存最大值
	PFC_System("echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6 ");//关闭IPV6
#endif
	GLOBAL_TRACE(("Software Release DateTime = %s %s\n", __DATE__, __TIME__));
	GLOBAL_TRACE(("Core Module ReleaseDateTime = %s\n", PFC_GetRelease()));


#if 0//测试部分
	{
		HANDLE32 lComHande;
		U8 plTmpBuf[32];
		S32 lActRead;
		lComHande = PFC_ComDeviceOpen(2, FALSE);
		if (lComHande)
		{
			PFC_ComDeviceSetState(lComHande, 115200, 8, 'N', 1);//固定！！

			PFC_ComDeviceSetOption(lComHande, 512, 512, 1000, 1000);

			while(TRUE)
			{
				if ((lActRead = PFC_ComDeviceRead(lComHande, plTmpBuf, sizeof(plTmpBuf))) > 0)
				{
					GLOBAL_TRACE(("Got Data Len = %d\n", lActRead));
					PFC_ComDeviceWrite(lComHande, plTmpBuf, lActRead);
				}
				else
				{
					GLOBAL_TRACE(("Timeout\n"));
				}
			}

		}
	}
#endif

	PL_MiscInitiate(NULL, NULL);

#if 1//可自动复位的系统
	while(1)
	{
		PL_SyslogdRestart(NULL, 1, 0, 0, 0, FALSE);
#ifdef GQ3650DS
		PL_TRACE(("Ready Enter GQ-3650DS Process\n"));
#endif
#ifdef GQ3650DR
		PL_TRACE(("Ready Enter GQ-3650DR Process\n"));
#endif
#ifdef GQ3710A
		PL_TRACE(("Ready Enter GQ-3710A Process\n"));
#endif
#ifdef GQ3710B
		PL_TRACE(("Ready Enter GQ-3710B Process\n"));
#endif
#ifdef GQ3760A
		PL_TRACE(("Ready Enter GQ-3760A Process\n"));
#endif
#ifdef GA2620B
		PL_TRACE(("Ready Enter GA-2620B Process\n"));
#endif
#ifdef GM2700B
		PL_TRACE(("Ready Enter GM2700B Process\n"));
#endif
#ifdef GM2700S
		PL_TRACE(("Ready Enter GM2700S Process\n"));
#endif
#ifdef GQ3655
		PL_TRACE(("Ready Enter GQ-3655 Process\n"));
#endif
#ifdef GM8358Q
		PL_TRACE(("Ready Enter GM-8358Q Process\n"));
#endif
#ifdef GM8398Q
		PL_TRACE(("Ready Enter GM-8398Q Process\n"));
#endif
#ifdef GM4500
		PL_TRACE(("Ready Enter GM-4500 Process\n"));
#endif
#ifdef GN2000
		PL_TRACE(("Ready Enter GN-2000 Process\n"));
#endif
#ifdef GQ3760B
		PL_TRACE(("Ready Enter GQ-3760B Process\n"));
#endif
#ifdef GQ3763
		PL_TRACE(("Ready Enter GQ-3763 Process\n"));
#endif
#ifdef GM2750
		PL_TRACE(("Ready Enter GM-2750 Process\n"));
#endif
#ifdef GC1804C
		PL_TRACE(("Ready Enter GC-1804 Process\n"));
#endif
#ifdef GM2730X
		PL_TRACE(("Ready Enter GM-2730X Process\n"));
#endif
#ifdef GM2730S
		PL_TRACE(("Ready Enter GM-2730S Process\n"));
#endif
#ifdef GM2730H
		PL_TRACE(("Ready Enter GM-2730H Process\n"));
#endif
#ifdef LR1800S
		PL_TRACE(("Ready Enter LR-1800S Process\n"));
#endif
#ifdef GM7000
		PL_TRACE(("Ready Enter GM-7000 Process\n"));
#endif
#ifdef GQ3760
		PL_TRACE(("Ready Enter GQ-3760 Process\n"));
#endif
#ifdef GQ3765
		PL_TRACE(("Ready Enter GQ-3765 Process\n"));
#endif
#ifdef GQ3768
		PL_TRACE(("Ready Enter GQ-3768 Process\n"));
#endif
#ifdef GC1815B
		PL_TRACE(("Ready Enter GC-1815B Process\n"));
#endif
#ifdef GC1804B
		PL_TRACE(("Ready Enter GC-1804B Process\n"));
#endif
#ifdef GN1866
		PL_TRACE(("Ready Enter GC-1866 Process\n"));
#endif
#ifdef GN1846
		PL_TRACE(("Ready Enter GC-1846 Process\n"));
#endif
		lProcessPIDs = fork(); 
		if (lProcessPIDs == -1) 
		{
			fprintf(stderr, "fork() error.errno:%d error:%s\n", errno, strerror(errno));
			break;
		}
		else if (lProcessPIDs == 0) 
		{
			/*子进程*/
			MULT_Enter();
			exit(0);
		}
		else if (lProcessPIDs > 0) 
		{
			/*主进程*/
			lProcessPIDs = wait(&lWaitStatus);
			printf("\n###########################################################\n[%s] SubProcess Exited, Exit Status: %d, Now Rebooted!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n###########################################################\n", __FUNCTION__, lWaitStatus);
			g_auto_reboot_times ++;
		}
	}
#else

	MULT_Enter();
#endif

	PL_MiscTerminate();

	return 0;
}

