
/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
#ifdef SUPPORT_SYSLOG_MODULE

void MULT_SyslogApply(MULT_Handle *pHandle, BOOL bClose)
{
	MULT_Syslog *plTmpSyslog;
	plTmpSyslog = &pHandle->m_SyslogParam;
	PL_SyslogLevel(plTmpSyslog->m_LogLevel);
	GLOBAL_TRACE(("Syslog GlobalMark = %d, RemoteMark = %d, Addr = 0x%.8X, Port = %d\n", plTmpSyslog->m_SyslogGlobalMark, plTmpSyslog->m_RemoteMark, plTmpSyslog->m_RemoteAddr, plTmpSyslog->m_RemotePort));
	PL_SyslogdRestart(PL_SYSLOG_DEFAULT_PATH_NAME, 100 * 1024, 1, (plTmpSyslog->m_RemoteMark?plTmpSyslog->m_RemoteAddr:0), plTmpSyslog->m_RemotePort, (bClose?FALSE:plTmpSyslog->m_SyslogGlobalMark));
}

void MULT_SyslogClean(MULT_Handle *pHandle)
{
	//MULT_SyslogApply(pHandle, TRUE);
	PFC_System("rm -f %s", PL_SYSLOG_DEFAULT_PATH_NAME);
	//MULT_SyslogApply(pHandle, FALSE);
}


S32 MULT_SyslogGet(MULT_Handle *pHandle, CHAR_T *pXMLBuf, S32 BufSize)
{
	S32 lFileSize, lRetLen;
	GLOBAL_FD lFD;
	lRetLen = 0;
	lFD = GLOBAL_FOPEN(PL_SYSLOG_DEFAULT_PATH_NAME, "rb");
	if (lFD != 0)
	{
		lFileSize = CAL_FileSize(lFD);
		if ((lFileSize > 0) && (lFileSize <= 100 * 1024))
		{
			GLOBAL_FREAD(pXMLBuf, 1, lFileSize, lFD);
			lRetLen = lFileSize;
		}
		GLOBAL_FCLOSE(lFD);
	}
	return lRetLen;
}


#endif


/*EOF*/
