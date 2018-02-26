
/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#ifdef MULT_TS_BACKUP_SUPPORT
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define MULT_BP_DEFAULT_INPUT_CHANGE_TOLERANCE				(1)
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */


typedef struct
{
	MULT_BPTsInBackupInfo	m_InBPInfo;
	MULT_BPTsBackupMode		m_CurrentStatus;//当前工作状态 MULT_BP_TS_BACKUP_MODE_MAIN /MULT_BP_TS_BACKUP_MODE_BACKUP
	//S32						m_ChangedMark;//切换过标志，切换过一次之后为高，防止在AUTO_ONCE模式之下切换多次，重新设置参数则复位该状态
	S32						m_InputChangeTolerance;
	BOOL					m_bUsedAsBackupTs;
}MULT_BPInNode;



typedef struct
{
	MULT_BPTsOutBackupInfo	m_OutBPInfo;
	MULT_BPTsBackupMode		m_CurrentStatus;//当前工作状态 MULT_BP_TS_BACKUP_MODE_MAIN /MULT_BP_TS_BACKUP_MODE_BACKUP
	BOOL					m_bUsedAsBackupTs;
}MULT_BPOutNode;


typedef struct
{
	MULT_BPInNode			*m_pInTsArray;
	S32						m_InTsNum;

	MULT_BPOutNode			*m_pOutTsArray;
	S32						m_OutTsNum;

	BOOL					m_bStatusChanged;//状态发生变化后设置为TRUE，则监控进程会发起切换动作并将此变量设置为FALSE

	MULT_Handle				*m_pMultiHandle;

	S32						m_MaxInputChangeTolerance;
}MULT_BPHandle;

/* Private Variables (static)-------------------------------------------------- */
static MULT_BPHandle s_BPHandle;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */

/*模块初始化函数，分配资源*/
void MULT_BPInitiate(MULT_Handle *pHandle, S32 InTsMax, S32 OutTsMax)
{
	if (pHandle)
	{
		GLOBAL_ZEROMEM(&s_BPHandle, sizeof(s_BPHandle));
		s_BPHandle.m_InTsNum = InTsMax;
		s_BPHandle.m_pInTsArray = (MULT_BPInNode *)GLOBAL_ZMALLOC(sizeof(MULT_BPInNode) * s_BPHandle.m_InTsNum);

		s_BPHandle.m_OutTsNum = OutTsMax;
		s_BPHandle.m_pOutTsArray = (MULT_BPOutNode *)GLOBAL_ZMALLOC(sizeof(MULT_BPOutNode) * s_BPHandle.m_OutTsNum);

		s_BPHandle.m_MaxInputChangeTolerance = MULT_BP_DEFAULT_INPUT_CHANGE_TOLERANCE;

		GLOBAL_TRACE(("TsIn = %d, TsOut = %d\n", InTsMax, OutTsMax));
	}

}

/*如果没有备份则返回自身的TS序号，被TSP模块调用*/
S32 MULT_BPGetTsBackupTs(S32 TsInd, BOOL bInput)
{
	S32 lRetInd;
	lRetInd = TsInd;
	if (bInput)
	{
		if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_InTsNum))
		{
			MULT_BPInNode *plNode = &s_BPHandle.m_pInTsArray[TsInd];
			if (plNode->m_CurrentStatus == MULT_BP_TS_BACKUP_MODE_BACKUP)
			{
				if (GLOBAL_CHECK_INDEX(plNode->m_InBPInfo.m_BackupTsInd, s_BPHandle.m_InTsNum))
				{
					lRetInd = plNode->m_InBPInfo.m_BackupTsInd;
				}
				else
				{
					GLOBAL_TRACE(("TsInd = %d, BackupTsInd Error = %d\n", TsInd, plNode->m_InBPInfo.m_BackupTsInd));
				}

			}
		}
	}
	else
	{
		if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_OutTsNum))
		{
			MULT_BPOutNode *plNode = &s_BPHandle.m_pOutTsArray[TsInd];

			if (plNode->m_CurrentStatus == MULT_BP_TS_BACKUP_MODE_BACKUP)
			{
				if (GLOBAL_CHECK_INDEX(plNode->m_OutBPInfo.m_BackupTsInd, s_BPHandle.m_OutTsNum))
				{
					lRetInd = plNode->m_OutBPInfo.m_BackupTsInd;
				}
				else
				{
					GLOBAL_TRACE(("TsInd = %d, BackupTsInd Error = %d\n", TsInd, plNode->m_OutBPInfo.m_BackupTsInd));
				}
			}
		}
	}
	//GLOBAL_TRACE(("bInput %d Main Ts = %d Backup Ts = %d\n", bInput, TsInd, lRetInd));
	return lRetInd;
}

/*设置当前监控码率信息，输入备份切换以此作为切换动作*/
void MULT_BPSetInputTsBitrate(S32 TsInd, U32 Bitrate)
{
	if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_InTsNum))
	{
		MULT_BPInNode *plNode = &s_BPHandle.m_pInTsArray[TsInd];
		if (Bitrate < plNode->m_InBPInfo.m_LowLimit || Bitrate > plNode->m_InBPInfo.m_HighLimit)
		{
			if (plNode->m_InputChangeTolerance < s_BPHandle.m_MaxInputChangeTolerance)
			{
				plNode->m_InputChangeTolerance++;
			}
		}
		else
		{
			if (plNode->m_InputChangeTolerance > 0)
			{
				plNode->m_InputChangeTolerance--;
			}
		}

		if (plNode->m_InBPInfo.m_Mode == MULT_BP_TS_BACKUP_MODE_AUTO_REPEAT)
		{
			if ((plNode->m_CurrentStatus == MULT_BP_TS_BACKUP_MODE_MAIN) && (plNode->m_InputChangeTolerance == s_BPHandle.m_MaxInputChangeTolerance))
			{
				GLOBAL_TRACE(("AUTO REPEAT InTsInd %d Switch To Backup %d\n", TsInd, plNode->m_InBPInfo.m_BackupTsInd));
				plNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_BACKUP;
				s_BPHandle.m_bStatusChanged = TRUE;
			}

			if ((plNode->m_CurrentStatus == MULT_BP_TS_BACKUP_MODE_BACKUP) && (plNode->m_InputChangeTolerance == 0))
			{
				GLOBAL_TRACE(("AUTO REPEAT InTsInd %d Switch BackTo MAIN %d\n", TsInd, plNode->m_InBPInfo.m_BackupTsInd));
				plNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_MAIN;
				s_BPHandle.m_bStatusChanged = TRUE;
			}
		}
		else if (plNode->m_InBPInfo.m_Mode == MULT_BP_TS_BACKUP_MODE_AUTO_ONCE)
		{
			if ((plNode->m_CurrentStatus == MULT_BP_TS_BACKUP_MODE_MAIN) && (plNode->m_InputChangeTolerance == s_BPHandle.m_MaxInputChangeTolerance))
			{
				GLOBAL_TRACE(("AUTO ONCE InTsInd %d Switch To Backup %d\n", TsInd, plNode->m_InBPInfo.m_BackupTsInd));
				plNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_BACKUP;
				s_BPHandle.m_bStatusChanged = TRUE;
			}
		}
	}

}

/*远程控制模块接口，预计被SNMP模块控制实现对输出的备份切换*/
void MULT_BPSetOutputRemoteControl(S32 TsInd, BOOL bMain)
{
	if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_OutTsNum))
	{
		MULT_BPOutNode *plNode = &s_BPHandle.m_pOutTsArray[TsInd];

		if (plNode->m_OutBPInfo.m_Mode == MULT_BP_TS_BACKUP_MODE_REMOTE)//仅仅在设置为Remote控制是响应动作
		{
			S32 lOldStatus;
			lOldStatus = plNode->m_CurrentStatus;
			if (bMain)
			{
				plNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_MAIN;
			}
			else
			{
				plNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_BACKUP;
			}

			if (lOldStatus != plNode->m_CurrentStatus)
			{
				s_BPHandle.m_bStatusChanged = TRUE;
			}
		}
		else
		{
			GLOBAL_TRACE(("Ts %d Not In Remote Mode\n", TsInd));
		}
	}
}

/*监控函数，判断是否发生了状态变化*/
BOOL MULT_BPCheckApplyParameters(void)
{
	BOOL lRet = s_BPHandle.m_bStatusChanged;
	if (s_BPHandle.m_bStatusChanged)
	{
		GLOBAL_TRACE(("Backup Status Changed!!!!!!!\n"));
		s_BPHandle.m_bStatusChanged = FALSE;
	}
	return lRet;
}

/*内部函数，用于在参数设置时提取TS是否为备用TS，并设置相应变量*/
BOOL MULTL_BPCheckTsIsUsedByBackup(S32 TsInd, BOOL bInput)
{
	BOOL lRet = FALSE;
	S32 i;
	if (bInput)
	{
		MULT_BPInNode *plInNode;
		if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_InTsNum))
		{
			for (i = 0; i < s_BPHandle.m_InTsNum; i++)
			{
				plInNode = &s_BPHandle.m_pInTsArray[i];
				if (i != TsInd)
				{
					if (plInNode->m_InBPInfo.m_Mode != MULT_BP_TS_BACKUP_MODE_OFF)
					{
						if (plInNode->m_InBPInfo.m_BackupTsInd == TsInd)
						{
							lRet = TRUE;
							break;
						}
					}
				}
			}
		}
	}
	else 
	{
		MULT_BPOutNode *plOutNode;
		if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_OutTsNum))
		{
			for (i = 0; i < s_BPHandle.m_OutTsNum; i++)
			{
				plOutNode = &s_BPHandle.m_pOutTsArray[i];
				if (i != TsInd)
				{
					if (plOutNode->m_OutBPInfo.m_Mode != MULT_BP_TS_BACKUP_MODE_OFF)
					{
						if (plOutNode->m_OutBPInfo.m_BackupTsInd == TsInd)
						{
							lRet = TRUE;
							break;
						}
					}
				}
			}
		}
	}
	return lRet;
}

/*判断TS是否被用于备用TS，以做相应处理*/
BOOL MULT_BPGetTsIsUsedByBackup(S32 TsInd, BOOL bInput)
{
	BOOL lRet = FALSE;
	if (bInput)
	{
		if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_InTsNum))
		{
			lRet = s_BPHandle.m_pInTsArray[TsInd].m_bUsedAsBackupTs;
		}
	}
	else
	{
		if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_OutTsNum))
		{
			lRet = s_BPHandle.m_pOutTsArray[TsInd].m_bUsedAsBackupTs;
		}
	}
	return lRet;
}


/*获取实时备份工作状态*/
MULT_BPTsBackupMode MULT_BPGetBackupStatus(S32 TsInd, BOOL bInput)
{
	MULT_BPTsBackupMode lRetStatus = 0;
	if (bInput)
	{
		if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_InTsNum))
		{
			lRetStatus = s_BPHandle.m_pInTsArray[TsInd].m_CurrentStatus;
		}
	}
	else 
	{
		if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_OutTsNum))
		{
			lRetStatus = s_BPHandle.m_pOutTsArray[TsInd].m_CurrentStatus;
		}
	}
	return lRetStatus;
}

/*枚举变量<->字符串互转函数*/
CHAR_T* MULT_BPModeValueToStr(S32 Value)
{
	switch (Value)
	{
	case MULT_BP_TS_BACKUP_MODE_OFF:	
		return "OFF";
		break;
	case MULT_BP_TS_BACKUP_MODE_MAIN:	
		return "MAIN";
		break;
	case MULT_BP_TS_BACKUP_MODE_BACKUP:
		return "BACKUP";
		break;
	case MULT_BP_TS_BACKUP_MODE_AUTO_REPEAT:
		return "AUTO_REPEAT";
		break;
	case MULT_BP_TS_BACKUP_MODE_AUTO_ONCE:
		return "AUTO_ONCE";
		break;
	case MULT_BP_TS_BACKUP_MODE_REMOTE:
		return "REMOTE";
		break;
	default:
		break;
	}
	return "NONE";
}


S32 MULT_BPModeValueFromStr(CHAR_T* pString)
{
	S32 lValue = 0;
	if (pString)
	{
		if (GLOBAL_STRCMP(pString, "OFF") == 0)
		{
			lValue = MULT_BP_TS_BACKUP_MODE_OFF;
		}
		if (GLOBAL_STRCMP(pString, "MAIN") == 0)
		{
			lValue = MULT_BP_TS_BACKUP_MODE_MAIN;
		}
		else if (GLOBAL_STRCMP(pString, "BACKUP") == 0)
		{
			lValue = MULT_BP_TS_BACKUP_MODE_BACKUP;
		}
		else if (GLOBAL_STRCMP(pString, "AUTO_REPEAT") == 0)
		{
			lValue = MULT_BP_TS_BACKUP_MODE_AUTO_REPEAT;
		}
		else if (GLOBAL_STRCMP(pString, "AUTO_ONCE") == 0)
		{
			lValue = MULT_BP_TS_BACKUP_MODE_AUTO_ONCE;
		}
		else if (GLOBAL_STRCMP(pString, "REMOTE") == 0)
		{
			lValue = MULT_BP_TS_BACKUP_MODE_REMOTE;
		}
	}
	return lValue;

}

/*得到当前备份信息*/
BOOL MULT_BPProcInTsBackupInfo(S32 TsInd, MULT_BPTsInBackupInfo *pInInfo, BOOL bRead)
{
	BOOL lRet = FALSE;
	if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_InTsNum) && pInInfo)
	{
		if (bRead)
		{
			GLOBAL_MEMCPY(pInInfo, &s_BPHandle.m_pInTsArray[TsInd].m_InBPInfo, sizeof(MULT_BPTsInBackupInfo));
		}
		else 
		{
			GLOBAL_MEMCPY(&s_BPHandle.m_pInTsArray[TsInd].m_InBPInfo, pInInfo, sizeof(MULT_BPTsInBackupInfo));
		}
		lRet = TRUE;
	}
	return lRet;
}


BOOL MULT_BPProcOutTsBackupInfo(S32 TsInd, MULT_BPTsOutBackupInfo *pOutInfo, BOOL bRead)
{
	BOOL lRet = FALSE;
	if (GLOBAL_CHECK_INDEX(TsInd, s_BPHandle.m_OutTsNum) && pOutInfo)
	{
		if (bRead)
		{
			GLOBAL_MEMCPY(pOutInfo, &s_BPHandle.m_pOutTsArray[TsInd].m_OutBPInfo, sizeof(MULT_BPTsOutBackupInfo));
		}
		else 
		{
			GLOBAL_MEMCPY(&s_BPHandle.m_pOutTsArray[TsInd].m_OutBPInfo, pOutInfo, sizeof(MULT_BPTsOutBackupInfo));
		}
		lRet = TRUE;
	}
	return lRet;
}

/*状态XML*/
void MULT_BPSaveStatusXMLToBuf(CHAR_T *pBuf, S32 BufSize)
{
	S32 i;
	MULT_BPInNode *plInNode;
	MULT_BPOutNode *plOutNode;
	mxml_node_t *plXML;
	mxml_node_t *plXMLRoot;
	mxml_node_t *plXMLHolder;

	if (pBuf && BufSize > 0)
	{
		plXML = mxmlNewXML("1.0");
		plXMLRoot = mxmlNewElement(plXML, "root");

		plXMLHolder = mxmlNewElement(plXMLRoot, "in_ts_status");
		for (i = 0; i < s_BPHandle.m_InTsNum; i++)
		{
			plInNode = &s_BPHandle.m_pInTsArray[i];
			MULTL_XMLAddNodeText(plXMLHolder, "status", MULT_BPModeValueToStr(plInNode->m_CurrentStatus));
		}
		plXMLHolder = mxmlNewElement(plXMLRoot, "out_ts_status");
		for (i = 0; i < s_BPHandle.m_OutTsNum; i++)
		{
			plOutNode = &s_BPHandle.m_pOutTsArray[i];
			MULTL_XMLAddNodeText(plXMLHolder, "status", MULT_BPModeValueToStr(plOutNode->m_CurrentStatus));
		}
		mxmlSaveString(plXML, pBuf, BufSize, NULL);
		mxmlDelete(plXML);
		plXML = NULL;
	}

}

/*参数读取*/
void MULT_BPLoadXML(mxml_node_t *pXMLRoot)
{
	S32 lTsInd;
	CHAR_T *plTmpStr;
	mxml_node_t *plXMLBPRoot;
	mxml_node_t *plXMLBPTsRoot;
	mxml_node_t *plXMLBPTs;
	MULT_BPInNode *plInNode;
	MULT_BPOutNode *plOutNode;

	//GLOBAL_TRACE(("Load Ts Back Up XML\n"));

	plXMLBPRoot = mxmlFindElement(pXMLRoot, pXMLRoot, "ts_backup_setting_root", NULL, NULL, MXML_DESCEND_FIRST);
	if (plXMLBPRoot)
	{
		plXMLBPTsRoot = mxmlFindElement(plXMLBPRoot, plXMLBPRoot, "backup_ts_array_in", NULL, NULL, MXML_DESCEND_FIRST);
		if (plXMLBPTsRoot)
		{
			plXMLBPTs = mxmlFindElement(plXMLBPTsRoot, plXMLBPTsRoot, "backup_ts_info", NULL, NULL, MXML_DESCEND_FIRST);
			while(plXMLBPTs)
			{
				lTsInd =  MULTL_XMLGetNodeINT(plXMLBPTs, "ts_ind", 10);
				if (GLOBAL_CHECK_INDEX(lTsInd, s_BPHandle.m_InTsNum))
				{
					plInNode = &s_BPHandle.m_pInTsArray[lTsInd];
					plInNode->m_InBPInfo.m_BackupTsInd = MULTL_XMLGetNodeINT(plXMLBPTs, "backup_ts_ind", 10);
					plInNode->m_InBPInfo.m_LowLimit = MULTL_XMLGetNodeUINT(plXMLBPTs, "low", 10);
					plInNode->m_InBPInfo.m_HighLimit = MULTL_XMLGetNodeUINT(plXMLBPTs, "high", 10);
					plInNode->m_InBPInfo.m_Mode = MULT_BPModeValueFromStr(MULTL_XMLGetNodeText(plXMLBPTs, "mode"));

					//GLOBAL_TRACE(("In Ts %d, Backup = %d, Low = %u, High = %u, Mode = %s\n", lTsInd, plInNode->m_InBPInfo.m_BackupTsInd, plInNode->m_InBPInfo.m_LowLimit, plInNode->m_InBPInfo.m_HighLimit, MULT_BPModeValueToStr(plInNode->m_InBPInfo.m_Mode)));
					/*切换实时参数恢复到默认值*/
					if (plInNode->m_InBPInfo.m_Mode == MULT_BP_TS_BACKUP_MODE_BACKUP)
					{
						plInNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_BACKUP;
					}
					else
					{
						plInNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_MAIN;
					}
					plInNode->m_InputChangeTolerance = 0;
				}
				else
				{
					GLOBAL_TRACE(("Error! In Ts Index = %d!\n", lTsInd));
					break;
				}
				plXMLBPTs = mxmlFindElement(plXMLBPTs, plXMLBPTsRoot, "backup_ts_info", NULL, NULL, MXML_NO_DESCEND);
			}
		}

		plXMLBPTsRoot = mxmlFindElement(plXMLBPRoot, plXMLBPRoot, "backup_ts_array_out", NULL, NULL, MXML_DESCEND_FIRST);
		if (plXMLBPTsRoot)
		{
			plXMLBPTs = mxmlFindElement(plXMLBPTsRoot, plXMLBPTsRoot, "backup_ts_info", NULL, NULL, MXML_DESCEND_FIRST);
			while(plXMLBPTs)
			{
				lTsInd =  MULTL_XMLGetNodeINT(plXMLBPTs, "ts_ind", 10);
				if (GLOBAL_CHECK_INDEX(lTsInd, s_BPHandle.m_OutTsNum))
				{
					plOutNode = &s_BPHandle.m_pOutTsArray[lTsInd];
					plOutNode->m_OutBPInfo.m_BackupTsInd = MULTL_XMLGetNodeINT(plXMLBPTs, "backup_ts_ind", 10);
					plOutNode->m_OutBPInfo.m_Mode =MULT_BPModeValueFromStr(MULTL_XMLGetNodeText(plXMLBPTs, "mode"));
					/*切换实时参数恢复到默认值*/
					//GLOBAL_TRACE(("Out Ts %d, Backup = %d, Mode = %s\n", lTsInd, plOutNode->m_OutBPInfo.m_BackupTsInd, MULT_BPModeValueToStr(plOutNode->m_OutBPInfo.m_Mode)));
					if (plOutNode->m_OutBPInfo.m_Mode == MULT_BP_TS_BACKUP_MODE_BACKUP)
					{
						plOutNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_BACKUP;
					}
					else
					{
						plOutNode->m_CurrentStatus = MULT_BP_TS_BACKUP_MODE_MAIN;
					}
				}
				else
				{
					GLOBAL_TRACE(("Error! Out Ts Index = %d!\n", lTsInd));
					break;
				}
				plXMLBPTs = mxmlFindElement(plXMLBPTs, plXMLBPTsRoot, "backup_ts_info", NULL, NULL, MXML_NO_DESCEND);
			}
		}
	}

	{
		S32 i;
		for (i = 0; i < s_BPHandle.m_InTsNum; i++)
		{
			s_BPHandle.m_pInTsArray[i].m_bUsedAsBackupTs = MULTL_BPCheckTsIsUsedByBackup(i, TRUE);
		}
		for (i = 0; i < s_BPHandle.m_OutTsNum; i++)
		{
			s_BPHandle.m_pOutTsArray[i].m_bUsedAsBackupTs = MULTL_BPCheckTsIsUsedByBackup(i, FALSE);
		}
	}

}

/*参数保存*/
void MULT_BPSaveXML(mxml_node_t *pXMLRoot)
{
	mxml_node_t *plXMLBPRoot;
	mxml_node_t *plXMLBPTsRoot;
	mxml_node_t *plXMLBPTs;
	MULT_BPInNode *plInNode;
	MULT_BPOutNode *plOutNode;

	//GLOBAL_TRACE(("\n"));

	plXMLBPRoot = mxmlNewElement(pXMLRoot, "ts_backup_setting_root");
	if (plXMLBPRoot)
	{
		S32 i;
		plXMLBPTsRoot = mxmlNewElement(plXMLBPRoot, "backup_ts_array_in");
		for (i = 0; i < s_BPHandle.m_InTsNum; i++)
		{
			plInNode = &s_BPHandle.m_pInTsArray[i];
			plXMLBPTs = mxmlNewElement(plXMLBPTsRoot, "backup_ts_info");
			MULTL_XMLAddNodeINT(plXMLBPTs, "ts_ind", i);
			MULTL_XMLAddNodeINT(plXMLBPTs, "backup_ts_ind", plInNode->m_InBPInfo.m_BackupTsInd);
			MULTL_XMLAddNodeUINT(plXMLBPTs, "low", plInNode->m_InBPInfo.m_LowLimit);
			MULTL_XMLAddNodeUINT(plXMLBPTs, "high", plInNode->m_InBPInfo.m_HighLimit);
			MULTL_XMLAddNodeText(plXMLBPTs, "mode", MULT_BPModeValueToStr(plInNode->m_InBPInfo.m_Mode));
		}

		plXMLBPTsRoot = mxmlNewElement(plXMLBPRoot, "backup_ts_array_out");
		for (i = 0; i < s_BPHandle.m_OutTsNum; i++)
		{
			plOutNode = &s_BPHandle.m_pOutTsArray[i];
			plXMLBPTs = mxmlNewElement(plXMLBPTsRoot, "backup_ts_info");
			MULTL_XMLAddNodeINT(plXMLBPTs, "ts_ind", i);
			MULTL_XMLAddNodeINT(plXMLBPTs, "backup_ts_ind", plOutNode->m_OutBPInfo.m_BackupTsInd);
			MULTL_XMLAddNodeText(plXMLBPTs, "mode", MULT_BPModeValueToStr(plOutNode->m_OutBPInfo.m_Mode));
		}
	}
}

/*模块退出函数，释放资源*/
void MULT_BPTerminate(void)
{
	GLOBAL_SAFEFREE(s_BPHandle.m_pInTsArray);
	GLOBAL_SAFEFREE(s_BPHandle.m_pOutTsArray);
}




#endif
/*EOF*/
