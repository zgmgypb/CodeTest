/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "libc_assist.h"
#include "crypto.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
void MULTL_SNTPRequestThread(void *pParam)
{
	MULT_Handle *plHandle = (MULT_Handle*)pParam;
	if (plHandle)
	{
		TIME_T lTimeStamp;
		S32 lOffset, lFraction;
		MULT_Config *plConfig;
		MULT_Monitor *plMonitor;

		lOffset = 0;
		lFraction = 0;

		GLOBAL_TIME(&lTimeStamp);
		plMonitor = &plHandle->m_Monitor;
		plConfig = &plHandle->m_Configuration;
		if (PAL_SNTPGetOffset(plConfig->m_NTPServerAddr, &lOffset, &lFraction))
		{
			lTimeStamp += lOffset;
			GLOBAL_TRACE(("Time Offset = %d s, %d ms, Set New Time = 0x%.8X\n", lOffset, lFraction / 1000000000, lTimeStamp));
			GLOBAL_STIME(&lTimeStamp);
			plMonitor->m_NTPSyncTimeout = plConfig->m_NTPInterval * 1000;/*参数单位为秒*/
			plMonitor->m_NTPWorkingStatus = FALSE;
		}
		else
		{
			if (plMonitor->m_GlobalMark)
			{
				CAL_LogAddLog(plMonitor->m_LogHandle, MULT_MONITOR_TYPE_NTP, 0, 0, FALSE);
			}
			GLOBAL_TRACE(("NTP Sync Failed!!!\n"));
			plMonitor->m_NTPSyncTimeout = MULT_SNTP_FAILED_RETRY_TIMEOUT * 1000;
			plMonitor->m_NTPWorkingStatus = FALSE;
		}

	}
}

/*EOF*/
