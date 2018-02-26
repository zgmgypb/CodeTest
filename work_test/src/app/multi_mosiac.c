
/* Includes-------------------------------------------------------------------- */
#include "multi_main_internal.h"
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define MULTI_MAX_MOSIAC_SERVICE_NUM	6
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
/* Private Variables (static)-------------------------------------------------- */
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
#ifdef GN2000
typedef struct  
{
	S32		m_Offset;
	CHAR_T	m_pServiceName[MPEG2_DB_MAX_SERVICE_NAME_BUF_LEN];
	U16		m_OriServiceID;
	U16		m_OriTSID;
	S32		m_InTsInd;
}MULT_MOSIAC_Node;



typedef struct  
{
	MULT_MOSIAC_Node	m_pNode[MULTI_MAX_MOSIAC_SERVICE_NUM];
}MULT_MOSIAC;


static MULT_MOSIAC s_MosiacHandle;


#define MULT_MOSIAC_USER_DESICRIPTOR_TAG		0xCF

S32 MULT_MOSIACEncodeUserDescryptor(MULT_Handle *pHandle, MULT_MOSIAC_Node *plMosiac, MPEG2_DBServiceOutInfo *pInfo, U8 *pBuf, S32 BufLen)
{
	S32 lDescLen, lTmpValue;
	U8 *plTmpBuf;

	lDescLen = 0;
	plTmpBuf = pBuf;

	GLOBAL_MSB8_EC(plTmpBuf, MULT_MOSIAC_USER_DESICRIPTOR_TAG, lDescLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0, lDescLen);
	GLOBAL_MSB16_EC(plTmpBuf, pInfo->m_ServiceID, lDescLen);
	GLOBAL_MSB16_EC(plTmpBuf, plMosiac->m_OriServiceID, lDescLen);
	GLOBAL_MSB16_EC(plTmpBuf, plMosiac->m_OriTSID, lDescLen);
	pBuf[1] = lDescLen - 2;

	return lDescLen;
}

void MULTL_MOSIACCopyDescriptors(MULT_Handle *pHandle, U8 DescriptorType, U32 SrcParentIDs, U32 DesParentIDs)
{
	U32 lItemIDs;
	CHAR_T plHexBuf[10240];
	S32 lHexLen;
	MPEG2_DBDescriptorInfo lDescInfo;

	lItemIDs = 0;
	while((lItemIDs = MPEG2_DBGetDescriptorNextNode(pHandle->m_DBSHandle, lItemIDs, DescriptorType, SrcParentIDs)) != 0)
	{
		if (MPEG2_DBGetDescriptorInfo(pHandle->m_DBSHandle, lItemIDs, &lDescInfo) == TRUE)
		{
			MPEG2_DBAddDescriptor(pHandle->m_DBSHandle, DesParentIDs, DescriptorType, &lDescInfo);
		}
		else
		{
			GLOBAL_ASSERT(0);
		}
	}
}

void MULTI_MOSIACAdjustReMuxParameter(MULT_Handle *pHandle)
{
	HANDLE32 lDBHandle;
	S32 i;
	S16 lInTsIndex, lOutTsIndex, lServNum;
	U32 lServiceIDs, lEsIDs, lTsIDs;
	U32 lOutServIDs, lOutEsIDs;

	U16 lTmpPID, lTmpServiceID, lTmpPMTPID;
	U32 ppArrayBuf[1024];
	HWL_AudioOffsetNode plAudioOffSetArray[HWL_MAX_AUDIO_OFFSET_PID_NUM];
	S32 lAudioOffsetCount;


	MPEG2_DBPIDMapInfo lPIDMapInfo;

	MPEG2_DBServiceInInfo lServiceInInfo;
	MPEG2_DBServiceOutInfo lServiceOutInfo;

	MPEG2_DBEsInInfo lEsInInfo;
	MPEG2_DBEsOutInfo lEsOutInfo;

	MPEG2_DBDescriptorInfo lDescInfo;

	lAudioOffsetCount = 0;

	lDBHandle = pHandle->m_DBSHandle;

	GLOBAL_TRACE(("Start Clear In Ts!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));

	/*清除编码器输入TS上的所有节目*/
	MPEG2_DBClearInTs(lDBHandle ,MULT_ENCODER_IN_TS_IND);
	/*清除所有自定义PID映射*/
	MPEG2_DBRemovePIDMapAll(lDBHandle);

	{
		MPEG2_DBTsOutInfo lTsOutInfo;
		MPEG2_DBGetTsOutInfo(lDBHandle, MPEG2_DBGetTsIDs(lDBHandle, FALSE, MULT_DECODER_OUT_TS_IND), &lTsOutInfo);
		lTsOutInfo.m_PATVersion = (lTsOutInfo.m_PATVersion + 1) & 0x1F;
		MPEG2_DBSetTsOutInfo(lDBHandle, MPEG2_DBGetTsIDs(lDBHandle, FALSE, MULT_DECODER_OUT_TS_IND), &lTsOutInfo);
	}

	/*新建虚拟节目*/
	lOutServIDs = MPEG2_DBAddService(lDBHandle, TRUE, MULT_ENCODER_IN_TS_IND);
	if (lOutServIDs != 0)
	{
		GLOBAL_TRACE(("New Service IDs = 0x%.8X\n", lOutServIDs));
		/*设置虚拟节目的TS参数*/
		MPEG2_DBSetServiceTsIndex(lDBHandle, lOutServIDs, MULT_ENCODER_IN_TS_IND, TRUE);

		/*设置虚拟节目的Service参数以及PCR映射*/
		MPEG2_DBGetServiceInInfo(lDBHandle, lOutServIDs, &lServiceInInfo);
		lServiceInInfo.m_PCRPID = 888;
		GLOBAL_STRCPY(lServiceInInfo.m_ServiceName, "MOSIAC Program");
		lServiceInInfo.m_PMTPID = 555;
		lServiceInInfo.m_PMTVersion = 5;
		lServiceInInfo.m_ServiceID = 5;
		lServiceInInfo.m_ServiceType = MPEG2_SERVICE_TYPE_DTV;
		MPEG2_DBSetServiceInInfo(lDBHandle, lOutServIDs, &lServiceInInfo);

		/*建立虚拟节目的编码视频PID映射*/
		lOutEsIDs = MPEG2_DBAddEs(lDBHandle, lOutServIDs);
		if (lOutEsIDs)
		{
			lEsInInfo.m_EsPID = 889;
			lEsInInfo.m_EsType = MPEG2_STREAM_TYPE_MPEG2_VIDEO;
			MPEG2_DBSetEsInInfo(lDBHandle, lOutEsIDs, &lEsInInfo);

			lEsOutInfo.m_EsPID = 102;
			lEsOutInfo.m_EsType = MPEG2_STREAM_TYPE_MPEG2_VIDEO;
			lEsOutInfo.m_OutputMark = TRUE;
			MPEG2_DBSetEsOutInfo(lDBHandle, lOutEsIDs, &lEsOutInfo);
		}
		else
		{
			GLOBAL_TRACE(("Add Es Failed!!!!!!!!!!!!!!\n"));
		}

		MPEG2_DBTsServiceRoute(lDBHandle, lOutServIDs, MULT_REAL_OUT_TS_IND);

		MPEG2_DBGetServiceOutInfo(lDBHandle, lOutServIDs, &lServiceOutInfo);
		lServiceOutInfo.m_ServiceID = 1;
		lServiceOutInfo.m_ServiceType = MPEG2_SERVICE_TYPE_MOSAIC;
		GLOBAL_STRCPY(lServiceOutInfo.m_ServiceName, "MOSIAC Program");
		GLOBAL_STRCPY(lServiceOutInfo.m_ServiceProviderName, "");
		lServiceOutInfo.m_PMTVersion = (lServiceOutInfo.m_PMTVersion + 1) & 0x1F;
		lServiceOutInfo.m_PMTPID = 100;
		lServiceOutInfo.m_PCRPID = 101;
		lServiceOutInfo.m_PMTInterval = MPEG2_PSI_PMT_DEFAULT_INTERVAL_MS;
		lServiceOutInfo.m_ServOutMark = TRUE;
		MPEG2_DBSetServiceOutInfo(lDBHandle, lOutServIDs, &lServiceOutInfo);

		lTmpPMTPID = 0x20;
		lTmpServiceID = 1;//6个节目的起始ServiceID
		/*获取调制通道输出TS上的所有节目*/
		lServNum = MPEG2_DBGetServiceIDsArrayByTsIndex(lDBHandle, MULT_DECODER_OUT_TS_IND, FALSE, TRUE, ppArrayBuf, 1000);
		if (lServNum > MULTI_MAX_MOSIAC_SERVICE_NUM)
		{
			lServNum = MULTI_MAX_MOSIAC_SERVICE_NUM;
		}

		for (i = 0; i < lServNum; i++)
		{
			lServiceIDs = ppArrayBuf[i];

			lInTsIndex = MPEG2_DBGetServicTsIndex(lDBHandle, lServiceIDs, TRUE);
			MPEG2_DBGetServiceOutInfo(lDBHandle, lServiceIDs, &lServiceOutInfo);

			GLOBAL_TRACE(("ServiceIDs = %.8X, InTs = %d, Service Name = %s, ServicID = %d\n", lServiceIDs, lInTsIndex, lServiceOutInfo.m_ServiceName, lServiceOutInfo.m_ServiceID));

			GLOBAL_STRCPY(s_MosiacHandle.m_pNode[i].m_pServiceName, lServiceOutInfo.m_ServiceName);

			lServiceOutInfo.m_ServiceID = lTmpServiceID;
			lTmpServiceID++;
			lServiceOutInfo.m_PMTPID = lTmpPMTPID;
			lTmpPMTPID++;
			MPEG2_DBSetServiceOutInfo(lDBHandle, lServiceIDs, &lServiceOutInfo);


			/*复制非视频ES的输出ES信息（输入的不显示也不使用）*/
			lTmpPID = 200 + (lServiceOutInfo.m_ServiceID - 1) * 10;
			lEsIDs = 0;
			while((lEsIDs = MPEG2_DBGetEsNextNode(lDBHandle, lEsIDs, lServiceIDs)) != 0)
			{
				MPEG2_DBGetEsInInfo(lDBHandle, lEsIDs, &lEsInInfo);
				if ((lEsInInfo.m_EsType != MPEG2_STREAM_TYPE_MPEG1_VIDEO) && (lEsInInfo.m_EsType != MPEG2_STREAM_TYPE_MPEG2_VIDEO) && (lEsInInfo.m_EsType != MPEG2_STREAM_TYPE_H264_VIDEO))
				{
					MPEG2_DBGetEsInInfo(lDBHandle, lEsIDs, &lEsInInfo);
					MPEG2_DBGetEsOutInfo(lDBHandle, lEsIDs, &lEsOutInfo);

					GLOBAL_TRACE(("InEs PID = %d, InEsType = %d\n", lEsInInfo.m_EsPID, lEsInInfo.m_EsType));

					lEsOutInfo.m_OutputMark = FALSE;//禁止输出到解码器
					MPEG2_DBSetEsOutInfo(lDBHandle, lEsIDs, &lEsOutInfo);

					lOutEsIDs = MPEG2_DBAddEs(lDBHandle, lOutServIDs);
					if (lOutEsIDs)
					{
						MPEG2_DBSetEsInInfo(lDBHandle, lOutEsIDs, &lEsInInfo);

						lEsOutInfo.m_EsPID = lTmpPID;
						lEsOutInfo.m_OutputMark = TRUE;//输出到IP
						lTmpPID++;

						GLOBAL_TRACE(("Map To OutEs PID = %d, InEsType = %d\n", lEsOutInfo.m_EsPID, lEsOutInfo.m_EsType));

						MPEG2_DBSetEsOutInfo(lDBHandle, lOutEsIDs, &lEsOutInfo);
						MULTL_MOSIACCopyDescriptors(pHandle, MPEG2_DESCRIPTOR_PMT_ES_INFO, lEsIDs, lOutEsIDs);

						/*添加自定义描述符，用于监控系统区分ES的来源*/
						lDescInfo.m_DescriptorDataSize = MULT_MOSIACEncodeUserDescryptor(pHandle, &s_MosiacHandle.m_pNode[i], &lServiceOutInfo, lDescInfo.m_pDescriptorData, sizeof(lDescInfo.m_pDescriptorData));
						lDescInfo.m_OutputMark = TRUE;
						MPEG2_DBAddDescriptor(lDBHandle, lOutEsIDs, MPEG2_DESCRIPTOR_PMT_ES_INFO, &lDescInfo);

						/*添加自定义映射用于映射音频*/
						lPIDMapInfo.m_InTsIndex = lInTsIndex;
						lPIDMapInfo.m_InPID = lEsInInfo.m_EsPID;
						lPIDMapInfo.m_OutTsIndex = MULT_REAL_OUT_TS_IND;
						lPIDMapInfo.m_OutPID = lEsOutInfo.m_EsPID;
						lPIDMapInfo.m_OutputMark = TRUE;

						if (lAudioOffsetCount < HWL_MAX_AUDIO_OFFSET_PID_NUM)
						{
							plAudioOffSetArray[lAudioOffsetCount].m_Offset = s_MosiacHandle.m_pNode[i].m_Offset;
							plAudioOffSetArray[lAudioOffsetCount].m_PCRPID = 888;
							plAudioOffSetArray[lAudioOffsetCount].m_PID = lEsOutInfo.m_EsPID;
							plAudioOffSetArray[lAudioOffsetCount].m_TsInd = MULT_REAL_OUT_TS_IND;
							lAudioOffsetCount++;
						}

						MPEG2_DBAddPIDMap(lDBHandle, &lPIDMapInfo);
					}
					else
					{
						GLOBAL_TRACE(("Add Es Failed!!!!!!!!!!!!!!\n"));
					}
				}
				else
				{
					MPEG2_DBGetEsOutInfo(lDBHandle, lEsIDs, &lEsOutInfo);
					lEsOutInfo.m_OutputMark = TRUE;//必须输出到解码器
					MPEG2_DBSetEsOutInfo(lDBHandle, lEsIDs, &lEsOutInfo);

				}
			}
		}

		HWL_EncoderSetAudioPIDDelay(plAudioOffSetArray, lAudioOffsetCount);
	}
	else
	{
		GLOBAL_TRACE(("Add Services Failed !!!!!!!!!!!!!!!!!!!!!!\n"));
	}

}


void MULTI_MOSIACInitiate(void)
{
	GLOBAL_ZEROMEM(&s_MosiacHandle, sizeof(s_MosiacHandle));
}

void MULTL_MOSIACSaveXML(mxml_node_t *pXMLParent)
{
	S32 i;
	{
		MULT_MOSIAC_Node *plNode;
		mxml_node_t *plXMLHolder;
		mxml_node_t *plXMLDescs;

		GLOBAL_TRACE(("Save MOSIAC\n"));

		plXMLHolder = mxmlNewElement(pXMLParent, "mosiac_gn2000");
		for (i = 0; i < MULTI_MAX_MOSIAC_SERVICE_NUM; i++)
		{
			plNode = &s_MosiacHandle.m_pNode[i];
			plXMLDescs = mxmlNewElement(plXMLHolder, "group");
			MULTL_XMLAddNodeUINT(plXMLDescs, "in_ts_ind", plNode->m_InTsInd);
			MULTL_XMLAddNodeUINT(plXMLDescs, "service_id", i + 1);
			MULTL_XMLAddNodeText(plXMLDescs, "service_name", plNode->m_pServiceName);
			MULTL_XMLAddNodeINT(plXMLDescs, "offset", plNode->m_Offset);
			MULTL_XMLAddNodeINT(plXMLDescs, "ori_service_id", plNode->m_OriServiceID);
			MULTL_XMLAddNodeINT(plXMLDescs, "ori_ts_id", plNode->m_OriTSID);
		}
	}
}

void MULTL_MOSIACLoadXML(mxml_node_t *pXMLParent)
{
	{
		S32 lIndex;
		MULT_MOSIAC_Node *plNode;
		mxml_node_t *plMOSIACRoot;
		mxml_node_t *plMOSIACGroupNode;

		GLOBAL_TRACE(("Load MOSIAC\n"));

		plMOSIACRoot = mxmlFindElement(pXMLParent, pXMLParent, "mosiac_gn2000", NULL, NULL, MXML_DESCEND_FIRST);
		if (plMOSIACRoot)
		{
			plMOSIACGroupNode = mxmlFindElement(plMOSIACRoot, plMOSIACRoot, "group", NULL, NULL, MXML_DESCEND_FIRST);
			while(plMOSIACGroupNode)
			{
				lIndex = MULTL_XMLGetNodeINT(plMOSIACGroupNode, "service_id", 10) - 1;

				GLOBAL_TRACE(("Index = %d\n", lIndex));
				if (GLOBAL_CHECK_INDEX(lIndex, MULTI_MAX_MOSIAC_SERVICE_NUM))
				{
					plNode = &s_MosiacHandle.m_pNode[lIndex];
					plNode->m_Offset = MULTL_XMLGetNodeINT(plMOSIACGroupNode, "offset", 10);
					plNode->m_OriServiceID = MULTL_XMLGetNodeINT(plMOSIACGroupNode, "ori_service_id", 10);
					plNode->m_OriTSID = MULTL_XMLGetNodeINT(plMOSIACGroupNode, "ori_ts_id", 10);

					GLOBAL_TRACE(("OffSet = %d\n", plNode->m_Offset));

				}
				plMOSIACGroupNode = mxmlFindElement(plMOSIACGroupNode, plMOSIACRoot, "group", NULL, NULL, MXML_NO_DESCEND);
			}
		}
	}
}


void MULTI_MOSIACTerminate(void)
{

}


#endif


/*EOF*/
