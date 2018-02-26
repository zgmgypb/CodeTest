#include "multi_drv.h"
#include "multi_hwl.h"
#include "multi_hwl_internal.h"
#include "global_micros.h"
#include "mpeg2.h"
#include "mpeg2_micro.h"
#include "platform_assist.h"
#include "multi_main_internal.h"
#ifdef SUPPORT_NEW_HWL_MODULE
#include "multi_hwl_monitor.h"
#endif

#ifdef GM8358Q
#include "gn_hwl.h"
#endif

#ifdef GN1846
#include "fpga_ps_gpio.h"
#include "hisi_gpio.h"
#include "hdmi_rx.h"
#include "hwl_fpga_spi.h"
#include "multi_hwl_local_encoder.h"
#endif

#ifdef SUPPORT_VRS232_MODULE
#include "hwl_vrs232.h"
#endif

#ifdef INSERTER_PID_TEST
static S32 s_PIDCount[MPEG2_TS_PACKET_MAX_PID_NUM];
#endif


#define HWL_MAX_SLOT_INDEX	32
#define BUFF_LENGTH   20

#ifdef SUPPORT_DRL_MODULE_USE_ETH
#define DRL_FPGA_BUF_SIZE	(1024 * 16)
#else
#define DRL_FPGA_BUF_SIZE	(1024)
#endif

typedef struct  
{
	HWL_TsFilterCB	m_pDataCB;
	S32				m_TsIndex;
	U16				m_PID;
}HWL_SlotData;

static HWL_SlotData s_HWLSlot[HWL_MAX_SLOT_INDEX];

static S32	s_InternalPacketCount = 0;
static S32	s_IntarnalBitrate = 0;
static HANDLE32	s_HWLPoolHandle = NULL;
static U8	s_HardwareVersion = 1;


static BOOL s_MUXFPGAOK = FALSE;

static U8 s_HWLICPBuf[DRL_FPGA_BUF_SIZE];

#ifdef GN1846
typedef struct  
{
	HANDLE32	m_pHdmiRxHandle[MULT_MAX_CHN_NUM]; /* HDMI 接收 AD 模块 */
}HWL_Handle;
static HWL_Handle s_HWLHandle;

void HWLL_GpioSetValue(FPS_GpioPin *pPinAddr, S32 Value, void *pUserParam)
{
	HI_GpioSetValue(pPinAddr->m_GpioIndex, pPinAddr->m_Pin, Value);
}

S32 HWLL_GpioGetValue(FPS_GpioPin *pPinAddr, void *pUserParam)
{
	return HI_GpioGetValue(pPinAddr->m_GpioIndex, pPinAddr->m_Pin);
}

void HWLL_GpioSetup(FPS_GpioPin *pPinAddr, BOOL IsInput, BOOL IsPullUp, void *pUserParam)
{
	HI_GpioSetup(pPinAddr->m_GpioIndex, pPinAddr->m_Pin, IsInput, IsPullUp);
}
#endif

/* FPGA配置函数 ----------------------------------------------------------------------------------------------------------------------------------- */
void HWL_FpgaConfig(void)
{
#ifdef MULT_DEVICE_FPGA_CONFIG_IN_AS_MODE
#else

#ifdef GM8358Q
	{
		{
			Init_FPGA();//编码器主板FPGA通讯接口必须先启动

#ifdef DEBUG_MODE_FPGA_CONFIG_ONCE
			if ((READ_FPGA(0, 0x302) == 0x5A) && (READ_FPGA(0, 0x303) == 0x88) )
			{
				return;
			}
#endif
			/*配置主板FPGA*/
			if (DRL_FpgaConfig(0) == FALSE)
			{
				GLOBAL_TRACE(("Config Main Board Failed!!!\n"));
			}
			else
			{
				sleep(1);//这里必须读取一次，才能要主板FPGA恢复正常，原因是逻辑代码错误，导致总线始终处于弱上拉状态，这个会导致子板FPGA读出来为0xFF								
				if ((READ_FPGA(0, 0x302) == 0x5A) && (READ_FPGA(0, 0x303) == 0x88) )
				{
					printf("main board fpga rbf config success!\n");
				}

				GLOBAL_TRACE(("Config Main Board Successful!!!\n"));

			}

			if (DRL_FpgaConfig(1) == FALSE)
			{
				GLOBAL_TRACE(("Config MUX Board Failed!!!\n"));
			}
			else
			{
				GLOBAL_TRACE(("Config MUX Board Successful!!!\n"));
				s_MUXFPGAOK = TRUE;

			}


			if (DRL_FpgaConfig(2) == FALSE)
			{
				GLOBAL_TRACE(("Config QAM Board Failed!!!\n"));
			}
			else
			{
				GLOBAL_TRACE(("Config QAM Board Successful!!!\n"));

			}


			PFC_TaskSleep(1000);
		}
	}
#elif defined(GN1846)
	FPS_GpioCfgParam lCfgParam;
	
	lCfgParam.m_IsUseConfigDone = TRUE;
	lCfgParam.m_DataPinAddr.m_GpioIndex = HI_GPIO10;
	lCfgParam.m_DataPinAddr.m_Pin = HI_PIN7;
	lCfgParam.m_DclkPinAddr.m_GpioIndex = HI_GPIO10;
	lCfgParam.m_DclkPinAddr.m_Pin = HI_PIN6;
	lCfgParam.m_NStatusPinAddr.m_GpioIndex = HI_GPIO10;

	lCfgParam.m_NStatusPinAddr.m_Pin = HI_PIN4;
	lCfgParam.m_NConfigPinAddr.m_GpioIndex = HI_GPIO10;
	lCfgParam.m_NConfigPinAddr.m_Pin = HI_PIN5;
	lCfgParam.m_ConfigDonePinAddr.m_GpioIndex = HI_GPIO10;
	lCfgParam.m_ConfigDonePinAddr.m_Pin = HI_PIN3;
	lCfgParam.m_pUserParam = NULL;

	lCfgParam.m_GpioSetValueCB = HWLL_GpioSetValue;
	lCfgParam.m_GpioGetValueCB = HWLL_GpioGetValue;
	lCfgParam.m_GpioSetupCB = HWLL_GpioSetup;

	if (FPS_GpioConfig(HWL_CONST_FPGA_FILE_PATH, &lCfgParam) == TRUE) {
		s_MUXFPGAOK = TRUE;
	}
#else

	/*如果使用ZYNQ平台则需要初始化ZYNQ自带的FPGA*/
	/*确保在设置以前讲子板电源关闭*/
#ifdef ENCODER_CARD_PLATFORM
#ifdef DEBUG_MODE_FPGA_CONFIG_ONCE
#else
	PFC_GPIOSetPIN(PFC_GPIOGetPINAddr((54 + 0 * 10) + 0), 0);
	PFC_GPIOSetPIN(PFC_GPIOGetPINAddr((54 + 1 * 10) + 0), 0);
	PFC_GPIOSetPIN(PFC_GPIOGetPINAddr((54 + 2 * 10) + 0), 0);
	PFC_GPIOSetPIN(PFC_GPIOGetPINAddr((54 + 3 * 10) + 0), 0);
	PFC_GPIOSetPIN(PFC_GPIOGetPINAddr((54 + 4 * 10) + 0), 0);
	PFC_GPIOSetPIN(PFC_GPIOGetPINAddr((54 + 5 * 10) + 0), 0);
#endif
#endif


#ifdef zynq
	DRL_ZYNQInitiate();
#endif

#ifdef NEW_FPGA_CONFIG_SYSTEM
	{

#ifdef DEBUG_MODE_FPGA_CONFIG_ONCE
		PFC_GPIOSetupPIN(PFCGPIO_EMIO(63), TRUE, TRUE);
		if (PFC_GPIOGetPIN(PFCGPIO_EMIO(63)) > 0)
		{
			GLOBAL_TRACE(("FPGA ALREAD ONLINE!!!\n"));
			s_MUXFPGAOK = TRUE;
		}
		else
#endif
		{
			PAL_FPGAPSPINs lPIN;

			GLOBAL_ZEROSTRUCT(lPIN);

			PFC_System("rmmod universal_spi_driver");
			PFC_System("insmod /tmp/universal_spi_driver.ko");


			lPIN.m_DCLK = PFCGPIO_EMIO(60);
			lPIN.m_bHanveNStatus = FALSE;
			lPIN.m_bHaveConfigDone = TRUE;
			lPIN.m_DIN = PFCGPIO_EMIO(61);
			lPIN.m_nConfig = PFCGPIO_EMIO(62);
			lPIN.m_ConfigDone = PFCGPIO_EMIO(63);
#ifdef USE_ZYNQ_AXI_FPGACONFIG_IP
			lPIN.m_bUseZYNQAXIFPGAConfig = TRUE;
			lPIN.m_ZYNQAXIFPGATimeout = 3000;
			lPIN.m_ZYNQAXIFPGAConfigBaseAddr = PL_ZYNQ_AXI_FPGACONFIG_DEFAULT_BASE_ADDR;
			lPIN.m_ZYNQAXIFPGAConfigSelectMask = 1 << 6;
#else
			lPIN.m_UNISPIFD = PFC_UNISPIUserOpen(FALSE);
#endif

			if (PAL_FPGAPSConfigFile(HWL_CONST_FPGA_FILE_PATH, &lPIN))
			{
				GLOBAL_TRACE(("FPGA %s Config OK\n", HWL_CONST_FPGA_FILE_PATH));
				s_MUXFPGAOK = TRUE;
			}
			else
			{
				GLOBAL_TRACE(("FPGA %s Config Failed\n", HWL_CONST_FPGA_FILE_PATH));
			}

#ifdef USE_ZYNQ_AXI_FPGACONFIG_IP
#else
			PFC_UNISPIUserClose(lPIN.m_UNISPIFD);
#endif

			/*子板01*/

			//PFC_GPIOSetupPIN(PFCGPIO_EMIO(0), FALSE, FALSE);

			//PFC_GPIOSetPIN(PFCGPIO_EMIO(0), 1);

			//GLOBAL_ZEROSTRUCT(lPIN);

			//lPIN.m_DCLK = PFCGPIO_EMIO(1);
			//lPIN.m_bHanveNStatus = FALSE;
			//lPIN.m_bHaveConfigDone = TRUE;
			//lPIN.m_DIN = PFCGPIO_EMIO(2);
			//lPIN.m_nConfig = PFCGPIO_EMIO(3);
			//lPIN.m_ConfigDone = PFCGPIO_EMIO(4);

			//if (PAL_FPGAPSConfigFile("/tmp/transcoder_vixs_pro100.rbf", &lPIN))
			//{
			//	GLOBAL_TRACE(("FPGA %s Config OK\n", HWL_CONST_FPGA_FILE_PATH));
			//	s_MUXFPGAOK = TRUE;
			//}
			//else
			//{
			//	GLOBAL_TRACE(("FPGA %s Config Failed\n", HWL_CONST_FPGA_FILE_PATH));
			//}


		}
	}
#else
#ifdef SUPPORT_2730_IP_BOARD_4CE55_SUPPORT
	DRL_GetMainBoardVersionBitValue();
	if (DRL_GetMainBoardIS4CE55())
	{
		GLOBAL_TRACE(("4CE55 BOARD!!!\n"));
		if (DRL_FpgaConfiguration(HWL_CONST_FPGA_DEV_PATH, HWL_CONST_FPGA_FILE_PATH_4CE55) != -1)
		{
			s_MUXFPGAOK = TRUE;
		}
	}
	else
	{
		GLOBAL_TRACE(("3C55 BOARD!!!\n"));
		if (DRL_FpgaConfiguration(HWL_CONST_FPGA_DEV_PATH, HWL_CONST_FPGA_FILE_PATH) != -1)
		{
			s_MUXFPGAOK = TRUE;
		}
	}
#else
	if (DRL_FpgaConfiguration(HWL_CONST_FPGA_DEV_PATH, HWL_CONST_FPGA_FILE_PATH) != -1)
	{
		s_MUXFPGAOK = TRUE;
	}
#endif
#endif//NEW_FPGA_CONFIG_SYSTEM

#endif

#endif
}

BOOL HWL_FPGAGetMuxOK(void)
{
	return s_MUXFPGAOK;
}

#ifdef GN1846
S32 ICPL_DataProcess(S32 TagId, U8 *pData, S32 DataLen)
{
	ICPL_Cmd_t *cmd;

	/**	根据数据中的lTagID查询处理体。*/
	cmd = ICPL_CmdResponseFind(TagId);
	if(cmd == NULL) {
		GLOBAL_TRACE(("Unknow TagID = 0x%x\n", TagId));
		cmd = NULL;
		return ERROR;
	}

	/*调试用读操作打印接口*/
	//CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, pData, DataLen);

	ICPL_Cmd_Lock(cmd);

	cmd->time++;
	cmd->m_RecvTick = PFC_GetTickCount();//给最后获取数据时间复制

	cmd->buff_len = DataLen;

	/**	如果处理体中缓冲区被指定，则拷贝所有回复数据到其buff.
	*	若没有，则临时指定cmd->buff_len为plTmpBuf参与计算。
	*/

	cmd->buff = pData;

	if(cmd->perform != NULL) {
		HWL_DEBUG("Perform\n");
		cmd->perform(cmd, cmd->data);
	}

	if(cmd->callback != NULL) {
		cmd->callback(cmd, cmd->data);
	}

	ICPL_Cmd_UnLock(cmd);
}
#endif

/* FPGA数据读取任务函数 --------------------------------------------------------------------------------------------------------------------------- */
U32 HWL_ReadPoolThreadFn(void *pUserparam, S32 Duration)
{
	U8 *plTmpBuf;
	U8 lTagID, lTagLen;
	S32 lDataLeft;
	S32 lActRead;
	S32 lPacketSize;
	ICPL_Cmd_t *cmd;

	/**	从FPGA设备中拷贝数据。*/

	lDataLeft = 0;
	while(TRUE)
	{
		lActRead = DRL_FpgaReadLock(s_HWLICPBuf, DRL_FPGA_BUF_SIZE);
		if(lActRead < 4)
		{
#ifdef SUPPORT_DRL_MODULE_USE_ETH
			continue;
#else
			break;
#endif
		}
		else
		{
			plTmpBuf = s_HWLICPBuf;
			lDataLeft = lActRead;
			lPacketSize = 0;

			while(TRUE)
			{
				lDataLeft -= lPacketSize;
				plTmpBuf += lPacketSize;

				if (lDataLeft >= 2)
				{
					lTagID = plTmpBuf[0];
					lTagLen = plTmpBuf[1];

					lPacketSize = (lTagLen + 1) * 4;

					//GLOBAL_TRACE(("TagID = 0x%02X, TagLen = %d, Packet Size = %d, DataLeft = %d\n", lTagID, lTagLen, lPacketSize, lDataLeft));

					if (lPacketSize <= lDataLeft)
					{

#ifdef ENCODER_CARD_PLATFORM
						if (lTagID == 0x45)
						{
							if (MULT_CARDModuleICPProsessRecv(plTmpBuf, lPacketSize))
							{
							}
							continue;
						}
#endif

#ifdef SUPPORT_NEW_HWL_MODULE
						if (HWL_MonitorParser(plTmpBuf, lPacketSize))
						{
							continue;
						}
#endif

#ifdef SUPPORT_HWL_FPGA_REG_MODULE
						if (lTagID == 0x56)
						{
							if (HWL_FPGARegICPRecv(plTmpBuf, lPacketSize))
							{
							}
							continue;
						}
#endif

						if (lTagID == ICPL_TAGID_FPGA_ETH_TS)
						{
							HWL_FPGAEthRedvFromFPGA(plTmpBuf, lPacketSize);
							continue;
						}

#ifdef SUPPORT_VRS232_MODULE
						if (lTagID == 0x32)
						{
							HWL_VRS232ReadProtocol(plTmpBuf, lPacketSize);
							continue;
						}
#endif


						if (lTagID == 0x33)
						{
							/*TAG是相同的*/
#ifdef SUPPORT_SFN_MODULATOR
							/*SFN帧处理*/
							MULT_SFNSetInternalCOMData(plTmpBuf, lPacketSize);
#endif

#ifdef SUPPORT_SFN_ADAPTER
							/*SFN帧处理*/
							MULT_SFNASetInternalCOMData(plTmpBuf, lPacketSize);
#endif

#ifdef SUPPORT_CLK_ADJ_MODULE
							/*CLK帧处理*/
							MULT_CLKProtocolParser(plTmpBuf, lPacketSize);
#endif
							continue;
						}

						if (lTagID == 0x35)
						{
#ifdef SUPPORT_FGPIO_MODULE
							FGPIO_ProtocolParser(plTmpBuf, lPacketSize);
#endif
							continue;
						}

						if ((lTagLen + 1) * 4 != lPacketSize)
						{
							GLOBAL_TRACE(("Tag = 0x%x, Len = %d, Size = %d\n", lTagID, lTagLen, lPacketSize));
						}
						else
						{
							/**	根据数据中的lTagID查询处理体。*/
							cmd = ICPL_CmdResponseFind(lTagID);
							if(cmd == NULL)
							{
								GLOBAL_TRACE(("Unknow TagID = 0x%x\n", lTagID));
								cmd = NULL;
								break;
							}

							/*调试用读操作打印接口*/
							if (lTagID == ICPL_TAGID_PHY /*|| lTagID == ICPL_TAGID_HEADBEAT || lTagID == ICPL_TAGID_CHANNELRATE*/)
							{
								CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, plTmpBuf, (lTagLen + 1) * 4);
							}

							ICPL_Cmd_Lock(cmd);

							cmd->time++;
							cmd->m_RecvTick = PFC_GetTickCount();//给最后获取数据时间复制

							cmd->buff_len = lTagLen * 4 + 4;

							/**	如果处理体中缓冲区被指定，则拷贝所有回复数据到其buff.
							*	若没有，则临时指定cmd->buff_len为plTmpBuf参与计算。
							*/

							cmd->buff = plTmpBuf;

							if(cmd->perform != NULL)
							{
								HWL_DEBUG("Perform\n");
								cmd->perform(cmd, cmd->data);
							}

							if(cmd->callback != NULL)
							{
								cmd->callback(cmd, cmd->data);
							}

							ICPL_Cmd_UnLock(cmd);
						}

					}
					else
					{
						GLOBAL_TRACE(("ICP TAG = 0x%.2X, Protocol Length Error! TagLen = %d, PacketSize = %d, DataLeft = %d\n", lTagID, lTagLen, lPacketSize, lDataLeft));
						CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, s_HWLICPBuf, lActRead);
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	return 0;
}

#ifdef GN1846



S32 g_LedFlashInterval = 1500; 
void HWLL_RunStatusMonitor(void *pParam) 
{
	HI_GpioSetup(HI_GPIO15, HI_PIN0, FALSE, TRUE);

	while(1) {
		HI_GpioSetValue(HI_GPIO15, HI_PIN0, 0);
		PL_TaskSleep(g_LedFlashInterval);
		HI_GpioSetValue(HI_GPIO15, HI_PIN0, 1);
		PL_TaskSleep(g_LedFlashInterval);
	}
}

void HWL_RunStatusDebug(void)
{	
	PL_TaskCreate(1024, HWLL_RunStatusMonitor, TRUE, NULL);
}

void HWL_FpgaSpiDebug(void)
{
	HANDLE32 lFspiHandle = FSPI_Create();
	FSPI_ConfigParam lCfgParam;
	FSPI_StatusParam lStatParam;
	S32 i;

	if (lFspiHandle) {
#if 1
		lCfgParam.m_CfgTag = FSPI_CFG_EX_IO;
		while (1) {
			lCfgParam.m_CfgTag = FSPI_CFG_EX_IO;
			for (i = 0; i < 12; i++) { 
				lCfgParam.m_CfgParam.m_ExIo.m_IoIndex = i;
				lCfgParam.m_CfgParam.m_ExIo.m_IoDir = 0;
				lCfgParam.m_CfgParam.m_ExIo.m_IoLvl = 1;
				FSPI_SetParam(lFspiHandle, &lCfgParam);
			}
			GLOBAL_TRACE(("ExIo: OUT-1\n"));
			sleep(1);
			for (i = 0; i < 12; i++) { 
				lCfgParam.m_CfgParam.m_ExIo.m_IoIndex = i;
				lCfgParam.m_CfgParam.m_ExIo.m_IoDir = 0;
				lCfgParam.m_CfgParam.m_ExIo.m_IoLvl = 0;
				FSPI_SetParam(lFspiHandle, &lCfgParam);
			}
			GLOBAL_TRACE(("ExIo: OUT-0\n"));
			sleep(1);

			lStatParam.m_StatTag = FSPI_STAT_DEV_ID;
			FSPI_GetStatus(lFspiHandle, &lStatParam);
			GLOBAL_TRACE(("FPGA devID = 0x%X\n", lStatParam.m_StatParam.m_DevId));
			lStatParam.m_StatTag = FSPI_STAT_CARD_ID;
			FSPI_GetStatus(lFspiHandle, &lStatParam);
			GLOBAL_TRACE(("FPGA CardID = 0x%08X\n", lStatParam.m_StatParam.m_CardId));
			lStatParam.m_StatTag = FSPI_STAT_VER;
			FSPI_GetStatus(lFspiHandle, &lStatParam);
			GLOBAL_TRACE(("FPGA Ver = %s\n", lStatParam.m_StatParam.m_pVer));
			lStatParam.m_StatTag = FSPI_STAT_RELEASE;
			FSPI_GetStatus(lFspiHandle, &lStatParam);
			GLOBAL_TRACE(("FPGA Release = %s\n", lStatParam.m_StatParam.m_pRelease));
			lStatParam.m_StatTag = FSPI_STAT_TEMP;
			FSPI_GetStatus(lFspiHandle, &lStatParam);
			GLOBAL_TRACE(("FPGA Temp = %.1f\n", lStatParam.m_StatParam.m_Temp));
		}
#endif
		FSPI_Destroy(lFspiHandle);
	}

	GLOBAL_TRACE(("FSPI_Create Failed!\n"));
}

static void HWLL_HdmiRxGpioSetValue(HDMI_RxGpioPin *pPinAddr, S32 Value, void *pUserParam)
{
	//GLOBAL_TRACE(("GPIOType:%d WriteGpio: Gpio%d_%d, Value:%d\n", pPinAddr->m_GpioType, pPinAddr->m_GpioIndex, pPinAddr->m_Pin, Value));
	if (pPinAddr->m_GpioType == HDMI_RX_GPIO_CPU) {
		HI_GpioSetValue(pPinAddr->m_GpioIndex, pPinAddr->m_Pin, Value);
	}
	else if(pPinAddr->m_GpioType == HDMI_RX_GPIO_FPGA_EXT) {
		FSPI_ConfigParam lCfgParam;
		HANDLE32 lFspiHandle = FSPI_GetHandle();

		if (lFspiHandle) {
			lCfgParam.m_CfgTag = FSPI_CFG_EX_IO;
			lCfgParam.m_CfgParam.m_ExIo.m_IoIndex = pPinAddr->m_Pin;
			lCfgParam.m_CfgParam.m_ExIo.m_IoDir = 0;
			lCfgParam.m_CfgParam.m_ExIo.m_IoLvl = Value;
			FSPI_SetParam(lFspiHandle, &lCfgParam);
		}
	}
}

static S32 HWLL_HdmiRxGpioGetValue(HDMI_RxGpioPin *pPinAddr, void *pUserParam)
{
	if (pPinAddr->m_GpioType == HDMI_RX_GPIO_CPU) {
		return HI_GpioGetValue(pPinAddr->m_GpioIndex, pPinAddr->m_Pin);
	}
	else if(pPinAddr->m_GpioType == HDMI_RX_GPIO_FPGA_EXT) {
		FSPI_StatusParam lStatus;
		HANDLE32 lFspiHandle = FSPI_GetHandle();

		if (lFspiHandle) {
			lStatus.m_StatTag = FSPI_STAT_EX_IO;
			lStatus.m_StatParam.m_ExIo.m_IoIndex = pPinAddr->m_Pin;
			FSPI_GetStatus(lFspiHandle, &lStatus);
			return lStatus.m_StatParam.m_ExIo.m_IoLvl;
		}

		return 0;
	}
}

static void HWLL_HdmiRxGpioSetup(HDMI_RxGpioPin *pPinAddr, BOOL IsInput, BOOL IsPullUp, void *pUserParam)
{
	//GLOBAL_TRACE(("GPIOType:%d GpioSet: Gpio%d_%d, %s, %s\n", pPinAddr->m_GpioType, pPinAddr->m_GpioIndex, 
	//	pPinAddr->m_Pin, (IsInput ? "IN" : "OUT"), (IsPullUp ? "PU" : "PD")));
	if (pPinAddr->m_GpioType == HDMI_RX_GPIO_CPU) {
		HI_GpioSetup(pPinAddr->m_GpioIndex, pPinAddr->m_Pin, IsInput, IsPullUp);
	}
	else if(pPinAddr->m_GpioType == HDMI_RX_GPIO_FPGA_EXT) {
		FSPI_ConfigParam lCfgParam;
		HANDLE32 lFspiHandle = FSPI_GetHandle();
		
		if (lFspiHandle) {
			lCfgParam.m_CfgTag = FSPI_CFG_EX_IO;
			lCfgParam.m_CfgParam.m_ExIo.m_IoIndex = pPinAddr->m_Pin;
			lCfgParam.m_CfgParam.m_ExIo.m_IoDir = (IsInput ? 1 : 0);
			lCfgParam.m_CfgParam.m_ExIo.m_IoLvl = (IsPullUp ? 1 : 0);
			FSPI_SetParam(lFspiHandle, &lCfgParam);
		}
	}
}

HANDLE32 plHdmiRxHandle[4] = {NULL};
void HWLL_DebugLoop(void *pParam) 
{
	S32 i;

	while(1) {
		HDMI_RxStatusParam lStatusParam;
		for (i = 0; i < 4; i++) {
			if (plHdmiRxHandle[i]) {
				HDMI_RxGetStatus(plHdmiRxHandle[i], &lStatusParam);
				GLOBAL_TRACE(("HDMI[%d] Lock:%d VidStandard:%d AudSample:%d\n", i, lStatusParam.m_SignalIsLocked, 
					lStatusParam.m_VideoStandard, lStatusParam.m_AudioSample));
			}
			sleep(2);
		}
	}
}

void HWL_HdmiRxDebug(void)
{
	HDMI_RxInitParam lInitParam;
	HANDLE32 lFspiHandle = FSPI_Create(); /* FPGA 通信打通 */
	S32 i;

	/* scl,sda,rst */
	HDMI_RxGpioPin plAdv7612PinList[4][3] = {
		{{HDMI_RX_GPIO_CPU, HI_GPIO14, HI_PIN7}, {HDMI_RX_GPIO_CPU, HI_GPIO14, HI_PIN6}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_0}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO24, HI_PIN0}, {HDMI_RX_GPIO_CPU, HI_GPIO24, HI_PIN1}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_3}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO19, HI_PIN7}, {HDMI_RX_GPIO_CPU, HI_GPIO19, HI_PIN6}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_6}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO9, HI_PIN2}, {HDMI_RX_GPIO_CPU, HI_GPIO9, HI_PIN3}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_9}}
	};

	/* scl,sda,wp */
	HDMI_RxGpioPin plEepromPinList[4][3] = {
		{{HDMI_RX_GPIO_CPU, HI_GPIO14, HI_PIN5}, {HDMI_RX_GPIO_CPU, HI_GPIO14, HI_PIN4}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_2}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO10, HI_PIN1}, {HDMI_RX_GPIO_CPU, HI_GPIO10, HI_PIN0}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_5}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO12, HI_PIN6}, {HDMI_RX_GPIO_CPU, HI_GPIO12, HI_PIN5}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_8}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO9, HI_PIN4}, {HDMI_RX_GPIO_CPU, HI_GPIO9, HI_PIN5}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_11}}
	};

	/* hdmi_hpd */
	HDMI_RxGpioPin plHpdPinList[4] = {
		{HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_1},
		{HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_4},
		{HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_7},
		{HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_10}
	};

	GLOBAL_TRACE(("===HDMI RX Debug===\n"));
	if (0)
	{ // GPIO 验证
		U32 lGpioIndex = HI_GPIO14;
		U32 lGpioPin = HI_PIN7;

		while (1) {
			HI_GpioSetup(lGpioIndex, lGpioPin, FALSE, TRUE);
			HI_GpioSetValue(lGpioIndex, lGpioPin, 1);
			GLOBAL_TRACE(("GPIO%d_%d: %d\n", lGpioIndex, lGpioPin, HI_GpioGetValue(lGpioIndex, lGpioPin)));
			sleep(1);
			HI_GpioSetValue(lGpioIndex, lGpioPin, 0);
			GLOBAL_TRACE(("GPIO%d_%d: %d\n", lGpioIndex, lGpioPin, HI_GpioGetValue(lGpioIndex, lGpioPin)));
			sleep(1);
		}
	}

	for (i = 0; i < 4; i++) {
		lInitParam.m_Adv7612I2cSclPin.m_GpioType = plAdv7612PinList[i][0].m_GpioType;
		lInitParam.m_Adv7612I2cSclPin.m_GpioIndex = plAdv7612PinList[i][0].m_GpioIndex;
		lInitParam.m_Adv7612I2cSclPin.m_Pin = plAdv7612PinList[i][0].m_Pin;
		lInitParam.m_Adv7612I2cSdaPin.m_GpioType = plAdv7612PinList[i][1].m_GpioType;
		lInitParam.m_Adv7612I2cSdaPin.m_GpioIndex = plAdv7612PinList[i][1].m_GpioIndex;
		lInitParam.m_Adv7612I2cSdaPin.m_Pin = plAdv7612PinList[i][1].m_Pin;
		lInitParam.m_Adv7612RstPin.m_GpioType = plAdv7612PinList[i][2].m_GpioType;
		lInitParam.m_Adv7612RstPin.m_GpioIndex = plAdv7612PinList[i][2].m_GpioIndex;
		lInitParam.m_Adv7612RstPin.m_Pin = plAdv7612PinList[i][2].m_Pin;

		lInitParam.m_EepromI2cSclPin.m_GpioType = plEepromPinList[i][0].m_GpioType;
		lInitParam.m_EepromI2cSclPin.m_GpioIndex = plEepromPinList[i][0].m_GpioIndex;
		lInitParam.m_EepromI2cSclPin.m_Pin = plEepromPinList[i][0].m_Pin;
		lInitParam.m_EepromI2cSdaPin.m_GpioType = plEepromPinList[i][1].m_GpioType;
		lInitParam.m_EepromI2cSdaPin.m_GpioIndex = plEepromPinList[i][1].m_GpioIndex;
		lInitParam.m_EepromI2cSdaPin.m_Pin = plEepromPinList[i][1].m_Pin;
		lInitParam.m_EepromWpPin.m_GpioType = plEepromPinList[i][2].m_GpioType;
		lInitParam.m_EepromWpPin.m_GpioIndex = plEepromPinList[i][2].m_GpioIndex;
		lInitParam.m_EepromWpPin.m_Pin = plEepromPinList[i][2].m_Pin;

		lInitParam.m_HdmiHpdPin.m_GpioType = plHpdPinList[i].m_GpioType;
		lInitParam.m_HdmiHpdPin.m_GpioIndex = plHpdPinList[i].m_GpioIndex;
		lInitParam.m_HdmiHpdPin.m_Pin = plHpdPinList[i].m_Pin;

		lInitParam.m_GpioGetValueCB = HWLL_HdmiRxGpioGetValue;
		lInitParam.m_GpioSetupCB = HWLL_HdmiRxGpioSetup;
		lInitParam.m_GpioSetValueCB = HWLL_HdmiRxGpioSetValue;

		lInitParam.m_pUserParam = NULL;

		if ((plHdmiRxHandle[i] = HDMI_RxCreate(&lInitParam)) == NULL) {
			GLOBAL_TRACE(("HDMI_RxCreate Failed!\n"));
			continue;
		}
		//HDMI_RxDownloadEdid(plHdmiRxHandle[i], HDMI_RX_EDID_PCM);
		{
			HDMI_RxCfgParam lCfgParam;

			lCfgParam.m_Brightness = 50;
			lCfgParam.m_Contrast = 50;
			lCfgParam.m_Hue = 50;
			lCfgParam.m_Saturation = 50;
			lCfgParam.m_IsAc3Bypass = FALSE;
			HDMI_RxSetParam(plHdmiRxHandle[i], &lCfgParam);
		}
	}

	PL_TaskCreate(1024 * 1024, HWLL_DebugLoop, TRUE, NULL);
}

static void HWL_HdmiRxInit(void)
{
	HDMI_RxInitParam lInitParam;
	S32 i;

	/* scl,sda,rst */
	HDMI_RxGpioPin plAdv7612PinList[MULT_MAX_CHN_NUM][3] = {
		{{HDMI_RX_GPIO_CPU, HI_GPIO14, HI_PIN7}, {HDMI_RX_GPIO_CPU, HI_GPIO14, HI_PIN6}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_0}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO24, HI_PIN0}, {HDMI_RX_GPIO_CPU, HI_GPIO24, HI_PIN1}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_3}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO19, HI_PIN7}, {HDMI_RX_GPIO_CPU, HI_GPIO19, HI_PIN6}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_6}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO9, HI_PIN2}, {HDMI_RX_GPIO_CPU, HI_GPIO9, HI_PIN3}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_9}}
	};

	/* scl,sda,wp */
	HDMI_RxGpioPin plEepromPinList[MULT_MAX_CHN_NUM][3] = {
		{{HDMI_RX_GPIO_CPU, HI_GPIO14, HI_PIN5}, {HDMI_RX_GPIO_CPU, HI_GPIO14, HI_PIN4}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_2}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO10, HI_PIN1}, {HDMI_RX_GPIO_CPU, HI_GPIO10, HI_PIN0}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_5}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO12, HI_PIN6}, {HDMI_RX_GPIO_CPU, HI_GPIO12, HI_PIN5}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_8}},
		{{HDMI_RX_GPIO_CPU, HI_GPIO9, HI_PIN4}, {HDMI_RX_GPIO_CPU, HI_GPIO9, HI_PIN5}, {HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_11}}
	};

	/* hdmi_hpd */
	HDMI_RxGpioPin plHpdPinList[MULT_MAX_CHN_NUM] = {
		{HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_1},
		{HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_4},
		{HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_7},
		{HDMI_RX_GPIO_FPGA_EXT, 0, FSPI_EX_IO_10}
	};

	GLOBAL_TRACE(("===HDMI RX Initialize===\n"));

	for (i = 0; i < MULT_MAX_CHN_NUM; i++) {
		lInitParam.m_Adv7612I2cSclPin.m_GpioType = plAdv7612PinList[i][0].m_GpioType;
		lInitParam.m_Adv7612I2cSclPin.m_GpioIndex = plAdv7612PinList[i][0].m_GpioIndex;
		lInitParam.m_Adv7612I2cSclPin.m_Pin = plAdv7612PinList[i][0].m_Pin;
		lInitParam.m_Adv7612I2cSdaPin.m_GpioType = plAdv7612PinList[i][1].m_GpioType;
		lInitParam.m_Adv7612I2cSdaPin.m_GpioIndex = plAdv7612PinList[i][1].m_GpioIndex;
		lInitParam.m_Adv7612I2cSdaPin.m_Pin = plAdv7612PinList[i][1].m_Pin;
		lInitParam.m_Adv7612RstPin.m_GpioType = plAdv7612PinList[i][2].m_GpioType;
		lInitParam.m_Adv7612RstPin.m_GpioIndex = plAdv7612PinList[i][2].m_GpioIndex;
		lInitParam.m_Adv7612RstPin.m_Pin = plAdv7612PinList[i][2].m_Pin;

		lInitParam.m_EepromI2cSclPin.m_GpioType = plEepromPinList[i][0].m_GpioType;
		lInitParam.m_EepromI2cSclPin.m_GpioIndex = plEepromPinList[i][0].m_GpioIndex;
		lInitParam.m_EepromI2cSclPin.m_Pin = plEepromPinList[i][0].m_Pin;
		lInitParam.m_EepromI2cSdaPin.m_GpioType = plEepromPinList[i][1].m_GpioType;
		lInitParam.m_EepromI2cSdaPin.m_GpioIndex = plEepromPinList[i][1].m_GpioIndex;
		lInitParam.m_EepromI2cSdaPin.m_Pin = plEepromPinList[i][1].m_Pin;
		lInitParam.m_EepromWpPin.m_GpioType = plEepromPinList[i][2].m_GpioType;
		lInitParam.m_EepromWpPin.m_GpioIndex = plEepromPinList[i][2].m_GpioIndex;
		lInitParam.m_EepromWpPin.m_Pin = plEepromPinList[i][2].m_Pin;

		lInitParam.m_HdmiHpdPin.m_GpioType = plHpdPinList[i].m_GpioType;
		lInitParam.m_HdmiHpdPin.m_GpioIndex = plHpdPinList[i].m_GpioIndex;
		lInitParam.m_HdmiHpdPin.m_Pin = plHpdPinList[i].m_Pin;

		lInitParam.m_GpioGetValueCB = HWLL_HdmiRxGpioGetValue;
		lInitParam.m_GpioSetupCB = HWLL_HdmiRxGpioSetup;
		lInitParam.m_GpioSetValueCB = HWLL_HdmiRxGpioSetValue;

		lInitParam.m_pUserParam = NULL;

		GLOBAL_TRACE(("HDMI_Rx Module[%d] Init...\n", i));
		if ((s_HWLHandle.m_pHdmiRxHandle[i] = HDMI_RxCreate(&lInitParam)) == NULL) {
			GLOBAL_TRACE(("HDMI_RxCreate Failed!\n"));
		}
	}
}

static void HWL_HdmiRxTerminate(void)
{
	S32 i;

	for (i = 0; i < MULT_MAX_CHN_NUM; i++) {
		if (s_HWLHandle.m_pHdmiRxHandle[i]) {
			HDMI_RxDestroy(s_HWLHandle.m_pHdmiRxHandle[i]);
			s_HWLHandle.m_pHdmiRxHandle[i] = NULL;
		}
	}
}

BOOL HWL_HdmiRxDownloadEdid(S32 ChnIndex, S32 EdidType)
{
	return HDMI_RxDownloadEdid(s_HWLHandle.m_pHdmiRxHandle[ChnIndex], EdidType);
}

BOOL HWL_HdmiRxSetParam(S32 ChnIndex, HDMI_RxCfgParam *pCfgParam)
{
	return HDMI_RxSetParam(s_HWLHandle.m_pHdmiRxHandle[ChnIndex], pCfgParam);
}

BOOL HWL_HdmiRxGetStatus(S32 ChnIndex, HDMI_RxStatusParam *pStatus)
{
	return HDMI_RxGetStatus(s_HWLHandle.m_pHdmiRxHandle[ChnIndex], pStatus);
}

#endif

/* 初始化函数 ------------------------------------------------------------------------------------------------------------------------------------- */
BOOL HWL_Initiate(S32 DeviceType)
{
	GLOBAL_ZEROMEM(&s_HWLSlot, sizeof(s_HWLSlot));
	DRL_ICPInitate();				//初始化FPGA传输层.
	ICPL_CodeInit();
	ICPL_CmdArrayInit();		//初始化FPGA协议层.
	HWL_InterInit();			//接口模块初始化
	HWL_RequestBuffInit();		//FPGA数据缓冲区初始化.
#ifndef GN1846
	HWL_ScrambleEnable(FALSE);	//关闭加扰
#endif

#ifdef SUPPORT_NEW_HWL_MODULE

#ifdef DEBUG_MODE_FPGA_CONFIG_ONCE
	PFC_TaskSleep(10);
#else
	PFC_TaskSleep(1000);
#endif
	HWL_MonitorInitate();

#ifdef SUPPORT_DRL_MODULE_USE_ETH
	s_HWLPoolHandle = PFC_TaskCreate("HWL POOL", 1024 * 1024, (PFC_ENTRY_FUNC)HWL_ReadPoolThreadFn, 0, NULL);
#else
	s_HWLPoolHandle = PAL_TIMThreadCreate("HWL POOL", 1024 * 1024, 50, (PAL_TIM_FUNC)HWL_ReadPoolThreadFn, NULL, TRUE);
#endif

#else
	HWL_FanInit(); 				//初始化风扇.


	/*得到硬件版本*/
	s_HardwareVersion = HWLL_GetHardwareVersion();

	GLOBAL_TRACE(("Hardware Mark = %d, Device Type = %d\n", s_HardwareVersion, DeviceType));

#ifndef GQ3760B
	if (DeviceType == HWL_DEVICE_SIMU_3650DS || DeviceType == HWL_DEVICE_SIMU_3650DR)
	{
		HWL_QAMInitialize(HWL_CONST_MODULATOR_AD9789_QAM);
	}
	else if (DeviceType == HWL_DEVICE_SIMU_3710A)
	{
		HWL_QAMInitialize(HWL_CONST_MODULATOR_AD9789_QPSK_L);
	}
	else if (DeviceType == HWL_DEVICE_SIMU_3760A)
	{
		HWL_QAMInitialize(HWL_CONST_MODULATOR_AD9789_DTMB);
	}
	else if (DeviceType == HWL_DEVICE_SIMU_2620B || DeviceType == HWL_DEVICE_SIMU_2700X)
	{

	}
	else if (DeviceType == HWL_DEVICE_SIMU_2730X)
	{

	}
#ifdef GN1846
	else if (DeviceType == HWL_DEVICE_SIMU_1846)
	{
		
	}
#endif
	else
	{
		GLOBAL_TRACE(("Unknow DeviceType = %d\n", DeviceType));
	}


	GLOBAL_TRACE(("Wait FPGA Read Sub Chips!\n"));
#ifndef GQ3710A
	PFC_TaskSleep(4000);
#else
	PFC_TaskSleep(6000); /* 延时等待 FPGA 读取加密卡 ID 正常 */
#endif
#endif

#if defined(GQ3650DR) || defined(GQ3710A) || defined(GQ3710B) || defined(GQ3655) || defined(GQ3760A) || defined(LR1800S) || defined(GM7000) || defined(GC1804C) || defined(GQ3760) || defined(GQ3768)|| defined(GM8358Q) || defined(GC1804B)
	InitTunerIIC();
#endif

#if defined(GM8358Q)
	//add by leonli for GM8358Q encoder hardware init
	HWL_GN_Init();
#endif

#ifdef GN1846
	HWL_LENCODER_InitParam lInitParam;

	HWL_HdmiRxInit();

	lInitParam.m_ChnNum = 1;
	lInitParam.m_SubNumPerCHN = 4;
	if (!HWL_LENCODER_Initiate(&lInitParam)) {
		GLOBAL_TRACE(("HWL_LENCODER_Initiate Failed!\n"));
	}
#endif

#ifdef SUPPORT_VRS232_MODULE
	HWL_VRS232Initiate();
#endif

#ifdef SUPPORT_NTS_DPD_BOARD
	DPD_ControlInitiate();
#endif

#ifndef GN1846
	s_HWLPoolHandle = PAL_TIMThreadCreate("HWL POOL", 1024 * 1024, 50, (PAL_TIM_FUNC)HWL_ReadPoolThreadFn, NULL, TRUE);
#endif

	HWL_PhyStatusQskSend();		//请求硬件信息.
	HWL_HeatBeatSend();
#endif


#ifdef SUPPORT_DRL_MODULE_USE_ETH
	{
		S32 lWaitICPLinkBackCount;

		lWaitICPLinkBackCount = 10;

#ifdef SUPPORT_NEW_HWL_MODULE
		HWL_MonitorHeartBeateSend();
#else
		HWL_HeatBeatSend();
#endif

		while(lWaitICPLinkBackCount > 0)
		{
			if (PL_GetETHStat(1) > 0)
			{
				GLOBAL_TRACE(("ICP Link OK\n"));
				PFC_TaskSleep(2000);
				break;
			}
			else
			{
				GLOBAL_TRACE(("ICP Link Not OK, Count = %d\n", lWaitICPLinkBackCount));
#ifdef SUPPORT_NEW_HWL_MODULE
				HWL_MonitorHeartBeateSend();
#else
				HWL_HeatBeatSend();
#endif
			}

			lWaitICPLinkBackCount--;
			PFC_TaskSleep(2000);
		}
	}
#endif

	/*初始化FPGA REG模块*/
#ifdef SUPPORT_HWL_FPGA_REG_MODULE
	HWL_FPGARegInitate();
	HWL_FPGARegPHYInitate();

#if  defined(GN1772)
	HWL_FPGARegPHYDetectAddr(0);
	HWL_FPGARegPHYDetectAddr(1);

	{
		S32 i;
		for (i = 0; i < 2; i++)
		{
			if (HWL_FPGARegPHYDetectOK(i))
			{

			}
		}
	}

#endif

#endif

	return TRUE;
}

typedef callFun(void) ;

void HWL_Access(S32 Duration)
{
#ifdef SUPPORT_NEW_HWL_MODULE
	HWL_MonitorAccess(Duration);
#else
	static S32 count = 0;
	callFun *fun;
	callFun *funArray[]=
	{
		HWL_InputChnRateRequest,
#ifndef LR1800S
		HWL_OutputChnRateReqeust,
#endif
#ifdef GM2730S
		HWL_OutputChnRateReqeustForS,
#endif
		HWL_HeatBeatSend,
	};

	s_IntarnalBitrate = s_InternalPacketCount * (MPEG2_TS_PACKET_SIZE * 8 * 1000 / Duration);
	s_InternalPacketCount = 0;

#ifdef INSERTER_PID_TEST
	{
		S32 i;
		for (i = 0; i < MPEG2_TS_PACKET_MAX_PID_NUM; i++)
		{
			if (s_PIDCount[i] != 0)
			{
				GLOBAL_TRACE(("PID[%d] Count = %d\n", i, s_PIDCount[i]));
				s_PIDCount[i] = 0;
			}
		}
	}
#endif
	// 	HWL_DEBUG("access\n");
	fun=funArray[  count% (sizeof(funArray)/sizeof(callFun*)) ];	
	fun();
	count++;
#endif


}


void HWL_Terminate(void)
{
	if (s_HWLPoolHandle)
	{
#ifdef SUPPORT_DRL_MODULE_USE_ETH
		PFC_TaskWait(s_HWLPoolHandle, 1000);
#else
		PAL_TIMThreadDestroy(s_HWLPoolHandle);
#endif
		s_HWLPoolHandle = NULL;
	}

#ifndef GN1846
	HWL_QAMTerminate();
	HWL_RequestBuffDestory();
	HWL_InterDestory();
	ICPL_CmdArrayClear();
	HWL_FanDestroy();
	DRL_FpgaDestroy();
#endif

#ifdef GN1846
	{
		HANDLE32 lFspiHandle = FSPI_GetHandle();

		HWL_LENCODER_Terminate();
		HWL_HdmiRxTerminate();
		if (lFspiHandle) {
			FSPI_Destroy(lFspiHandle);
		}
	}
#endif

#ifdef SUPPORT_VRS232_MODULE
	HWL_VRS232Terminate();
#endif


	/*销毁FPGA REG模块*/
#ifdef SUPPORT_HWL_FPGA_REG_MODULE
	HWL_FPGARegTerminate();
#endif
}

#ifdef GN1846
U32 HWL_GetBitrate(HANDLE32 Handle, S16 CHNIndex, S16 TsIndex)
#else
U32 HWL_GetBitrate(S16 CHNIndex, S16 TsIndex)
#endif
{
#ifdef GN1846
	S32 i;
	U32 lRet = 0;
	MULT_Handle *plHandle = (MULT_Handle *)Handle;
	/*
		CHNIndex - 0,输入；1,输出；-1,插入；
		TsIndex - -1,总码率；其他,对应 TS 码率；
	*/
	//GLOBAL_TRACE((ANSI_COLOR_LIGHT_GREEN"====POS!\n"ANSI_COLOR_NONE));
	if (CHNIndex == 0) {
		if (TsIndex == -1) {
			for (i = 0; i < MULT_MAX_CHN_NUM; i++) {
				lRet += HWL_LENCODER_GetTsBitrate(0, i);
			}
			return lRet;
		}
		else {
			return HWL_LENCODER_GetTsBitrate(0, TsIndex);
		}
	}
	else if (CHNIndex == 1) {
		if (plHandle->m_Configuration.m_IpOutputType == IP_OUTPUT_SPTS) { /* SPTS 输入与输出相同 */
			if (TsIndex == -1) {
				for (i = 0; i < MULT_MAX_CHN_NUM; i++) {
					lRet += HWL_LENCODER_GetTsBitrate(0, i);
				}
				return lRet;
			}
			else {
				return HWL_LENCODER_GetTsBitrate(0, TsIndex);
			}
		}
		else if (plHandle->m_Configuration.m_IpOutputType == IP_OUTPUT_MPTS) { /* MPTS 输出等于所有通道码率之和 */
			for (i = 0; i < MULT_MAX_CHN_NUM; i++) {
				lRet += HWL_LENCODER_GetTsBitrate(0, i);
			}
			return lRet;
		}
		else {
			return 0;
		}
	}
	else if (CHNIndex == -1) {
		return 0;
	}
	else {
		return 0;
	}
#endif

	HWL_ChannelRateArray_t rates[3];
	HWL_ChannelRateArray_t	*rp;
	HWL_AllRateInfo_t		rate;
	HWL_DataPerchaseLock(ICPL_TAGID_CHANNELRATE, rates);

#ifdef GN2000
	if (CHNIndex == 0 && TsIndex == -1)
	{
		S32 i;
		U32 lTmprate;
		rp = &rates[0];

		lTmprate = 0;
		for (i = 0; i < 15; i++)
		{
			lTmprate += rp->channelRateArray[i].logicChannelRate;
		}
		return lTmprate;
	}

	if (CHNIndex == 1 && TsIndex == -1)
	{
		rp = &rates[1];
		return rp->channelRateArray[0].logicChannelRate;
	}
#endif

	if((CHNIndex != 0) && (CHNIndex != 1) && (CHNIndex != 2))
	{
		return s_IntarnalBitrate;
	}

	rp = &rates[CHNIndex];
	if (CHNIndex == 0)
	{
		if ((TsIndex >= 0) && (TsIndex <  rp->channelRateArraySize))
		{
			return  rp->channelRateArray[TsIndex].logicChannelRate;
		}
		else
		{
			HWL_AllRateInfo(&rate);
			return rate.asiRate;
		}
	}
	else
	{
		if ((TsIndex >= 0) && (TsIndex <  rp->channelRateArraySize))
		{
			return  rp->channelRateArray[TsIndex].logicChannelRate;
		}
		else
		{
			HWL_AllRateInfo(&rate);
			return rate.qamRate;
		}
	}
	return 0;
}


static void HWL_TsRequestCB (struct tagICPL_Cmd_t *cmd, void *data)
{
	S32 lSlotIndex;
	U8 *pTmp;

	assert(cmd->tagid == ICPL_TAGID_PIDMAP);
	assert(cmd->buff!=NULL);

	pTmp = cmd->buff + 6;
	GLOBAL_MSB16_D(pTmp , lSlotIndex);

	//GLOBAL_TRACE(("TS CB Slot = %d\n", lSlotIndex));

	if (GLOBAL_CHECK_INDEX(lSlotIndex , HWL_MAX_SLOT_INDEX))
	{
		if (s_HWLSlot[lSlotIndex].m_pDataCB)
		{
			s_HWLSlot[lSlotIndex].m_pDataCB(lSlotIndex, cmd->buff + 8);
		}
		else
		{
			//GLOBAL_TRACE(("Slot Already Closed!!, Residual Data From Driver Fifo!\n"));
		}
	}
	else
	{
		GLOBAL_TRACE(("Slot Index Overflow = %d\n", lSlotIndex));
	}
}




void HWL_AddTsPacketsRequest(S32 SlotIndex, S32 InTsIndex, U16 InPID, HWL_TsFilterCB pDataCB, S32 PESCount)
{
	if (GLOBAL_CHECK_INDEX(SlotIndex , HWL_MAX_SLOT_INDEX))
	{
		s_HWLSlot[SlotIndex].m_pDataCB = pDataCB;
		s_HWLSlot[SlotIndex].m_TsIndex = InTsIndex;
		s_HWLSlot[SlotIndex].m_PID = InPID;
		if (pDataCB == NULL)
		{
			GLOBAL_TRACE(("NULL CB!!!!!!!!!!!!!For Slot = %d, Ts = %d, PID = %d\n", SlotIndex, InTsIndex, InPID));
		}

		ICPL_CmdResponseCallbackSet(ICPL_TAGID_PIDMAP , HWL_TsRequestCB);

		HWL_PidSearchStart(InTsIndex, InPID, SlotIndex, PESCount);
	}
	else
	{
		GLOBAL_TRACE(("Slot Index Overflow = %d\n", SlotIndex));
	}

}

void HWL_RemoveTsPacketsRequest(S32 SlotIndex)
{
	if (GLOBAL_CHECK_INDEX(SlotIndex , HWL_MAX_SLOT_INDEX))
	{
		HWL_PidSearchStop(s_HWLSlot[SlotIndex].m_TsIndex, s_HWLSlot[SlotIndex].m_PID, SlotIndex);
		s_HWLSlot[SlotIndex].m_pDataCB = NULL;
	}
	else
	{
		GLOBAL_TRACE(("Slot Index Overflow = %d\n", SlotIndex));
	}
}



typedef struct
{
	U32	buff_size;
	U8 	request_buff[DRL_FPGA_BUF_SIZE];
	HANDLE32 mutex;
} HWL_RequestBuff_t;


static HWL_RequestBuff_t __hwl_requestbuff;

void HWL_RequestBuffInit(void)
{
	__hwl_requestbuff.mutex = PFC_SemaphoreCreate("HWL_RequestBuffMutex", 1);
	__hwl_requestbuff.buff_size = DRL_FPGA_BUF_SIZE;
	PFC_SemaphoreSignal(__hwl_requestbuff.mutex);
}


void HWL_RequestBuffDestory(void)
{
	PFC_SemaphoreDestroy(__hwl_requestbuff.mutex);
}

static inline void __HWL_RequestBuffLock(void)
{
	assert(__hwl_requestbuff.mutex != NULL);
	if(__hwl_requestbuff.mutex == NULL)
	{
		return ;
	}
	PFC_SemaphoreWait(__hwl_requestbuff.mutex, -1);

}

static inline void __HWL_RequestBuffUnLock()
{
	assert(__hwl_requestbuff.mutex != NULL);
	if(__hwl_requestbuff.mutex == NULL)
	{
		return ;
	}

	PFC_SemaphoreSignal(__hwl_requestbuff.mutex);
}

static inline U8 *__HWL_RequestBuffGet()
{
	return __hwl_requestbuff.request_buff;
}

static inline U32  __HWL_RequestBuffSize()
{
	return __hwl_requestbuff.buff_size;
}


#define DATA_REQUEST 	1
#define DATA_RESPONSE	0



/**	这里两个函数进行了严格的加锁处理.
*	__HWL_DataPerchaseLock..获得数据
*	__HWL_DataPerformLock.. 设置数据
*/

/**	通过TAGID搜索并将FPGA返回数据拷贝数据给用户,用于回复体.
*/
S32 ____HWL_DataPerchaseLock(S32 tagid, void *data, U32 size, void *__fun, U32 TickMark)
{
	S32 retval;
	ICPL_Cmd_t *cmd;

	retval = ERROR;
	cmd = ICPL_CmdResponseFind(tagid);
	if(cmd)
	{
		if (cmd->data)
		{
			ICPL_Cmd_Lock(cmd);

			if(__fun == NULL)
			{
				if (cmd->data_len != size)
				{
					//assert(cmd->data_len == size);		//从大小来验证输入数据是否正确。
					GLOBAL_TRACE(("ERROR!!!!!!!!!!!!! Data = %d, size = %d\n", cmd->data_len, size));
				}
				else
				{
					if (TickMark != 0)
					{
						if (cmd->m_RecvTick >= TickMark)
						{
							GLOBAL_MEMCPY(data, cmd->data, size);
							retval = SUCCESS;
						}
					}
					else
					{
						GLOBAL_MEMCPY(data, cmd->data, size);
						retval = SUCCESS;
					}
				}
			}
			else
			{
				U32 index = size;
				ICPL_CmdPickFun_t  fun = (ICPL_CmdPickFun_t)__fun;
				fun(cmd, data, index);
				retval = SUCCESS;
			}
			ICPL_Cmd_UnLock(cmd);
		}
		else
		{
			GLOBAL_TRACE(("ERROR!!!!!!!!!!!!!\n"));
			//assert(cmd->data != NULL);			//该数据域必须存在.
		}


	}

	return retval;
}



/**	加锁并执行用户定义操作.用于请求体。
*/
S32 __HWL_DataPerformLock(U32 tagid, void *fun, void *data)
{
	S32 retval;
	ICPL_Cmd_t *cmdResponse;
	ICPL_Cmd_t  cmd;

	HWL_DEBUG("tagid :0x%x\n", tagid);

	if(fun == NULL)
	{
		GLOBAL_TRACE(("Error!!!!\n"));
		return ERROR_BAD_PARAM;
	}

	cmdResponse = ICPL_CmdResponseFind(tagid);
	if(cmdResponse != NULL && cmdResponse->timeLimit > 0 && cmdResponse->time >= cmdResponse->timeLimit)
	{
		return SUCCESS;
	}

	//__HWL_RequestBuffLock();				//LOCK 	.BUFF

	GLOBAL_MEMSET(&cmd, 0, sizeof(cmd));
	cmd.buff = __HWL_RequestBuffGet();
	cmd.buff_len = __HWL_RequestBuffSize();
	cmd.perform = (ICPL_CmdFun_t)fun;

	GLOBAL_MEMSET(cmd.buff, 0, 16);


	cmd.buff[ICPL_CMD_IDX_TAGID] = tagid;
	cmd.perform(&cmd, data);
	cmd.buff_len = (cmd.buff[ICPL_CMD_IDX_LENGTH] + 1) * 4;

	/*调试用写接口*/
	//if (cmd.buff[0] == ICPL_TAGID_GROUP_MODULOR_REGISTER)
	//{
	//	CAL_PrintDataBlock(__FUNCTION__, cmd.buff, cmd.buff_len);
	//}

	if(DRL_FpgaWriteLock(cmd.buff, cmd.buff_len) < 0)
	{
		retval = ERROR;
	}
	else
	{
		retval = SUCCESS;
	}

	//__HWL_RequestBuffUnLock();			//UNLOCK BUFF.


	return retval;

}













/**	获取硬件信息 .*/



void HWL_PhyStatusRskResponseShow(HWL_PhyStatusRskResponse_t *phyResponse)
{
	U32 i;
	assert(phyResponse != NULL);
	printf("====PhyStatusRskResponse====\n");
	for(i = 0; i < phyResponse->PhyStatusRskTable.tableSize; i++)
	{

		printf("\tphysicChannelId:%d\t", phyResponse->PhyStatusRskTable.table[i].physicChannelId);
		printf(" inOrOut:%d\t", phyResponse->PhyStatusRskTable.table[i].inOrOut);
		printf("maxLogicChannelNum:%d\n", phyResponse->PhyStatusRskTable.table[i].maxLogicChannelNum);
	}

	printf("\tfpgaVersion: 0x%02x year:%d month:%d day:%d chipSN:0x%x\n", phyResponse->fpga.v,
		phyResponse->fpga.year, phyResponse->fpga.month, phyResponse->fpga.day, phyResponse->chipSn);

}


/**	发送查询通道码率命令.*/
S32 HWL_ChannelRateInfoSend(HWL_ChannelRateInfo_t *param)
{
	return __HWL_DataPerformLock(ICPL_TAGID_CHANNELRATE, (ICPL_CmdFun_t)ICPL_OPtorChannelRateInfoMake , param);
}


S32 HWL_ChannelRateArrayGet(HWL_ChannelRateArray_t *ratearray)
{
	return __HWL_DataPerchaseLock(ICPL_TAGID_CHANNELRATE, ratearray, sizeof(HWL_ChannelRateArray_t));
}



void HWL_PSIBuffSizeShow(HWL_PSIBuffSize_t *ratearray)
{
	printf("====HWL_PSIBuffSize_t====\n");
	printf("\tstableTsPackgeNum:%d\n", ratearray->stableTsPackgeNum);
	printf("\timmediateTsPackgeNum:%d\n", ratearray->immediateTsPackgeNum);
}


void HWL_PIDCopyOriginalTableShow(HWL_PIDCopyOriginalTable_t *original)
{
	U32 i;
	HWL_PIDSource_t *source;
	printf("====PIDCopyOriginalTable====\n");
	printf("\tphysicChannel:0x%x\tpidsourceSize:%d\n", original->physicChannel, original->pidsourceSize);
	for(i = 0; i < original->pidsourceSize; i++)
	{
		source = &original->pidsourceTable[i];
		printf("\tid:%d\toriginalChannelID:%d\toriginalPid:%d\toffset:%d\tnum:%d\n", source->id, source->originalChannelID, source->originalPid,
			source->destinedPidOutListOffset, source->destinedPidOutListNum);
	}
}



void HWL_PIDCopyDestinedTableShow(HWL_PIDCopyDestinedTable_t *destined)
{
	U32 i;
	HWL_PIDdestined_t *item;
	printf("====PIDCopyDestinedTable====\n");
	printf("\tphysicChannel:0x%x\tpidDestinedSize:%d\n", destined->physicChannel, destined->pidDestinedSize);
	for(i = 0; i < destined->pidDestinedSize; i++)
	{
		item = &destined->pidDestinedTable[i];
		printf("\tid:%d\tdestinedChannelID:%d\tdestinedPid:%d\n", item->id, item->destinedChannelID, item->destinedPid);
	}
}



/**	组播的加入与退出 */
S32 HWL_GroupBroadTableCastSet(HWL_GroupBroadcastTable_t *groupCast)
{
	return __HWL_DataPerformLock(ICPL_TAGID_GROUP_BROADCAST, (ICPL_CmdFun_t)ICPL_OPtorGroupBroadcastTableMake , groupCast);
}


/*	PowerPC对TS输入端口和PCR 校验的MODE 设置: (PHY_Ch = 0) */
S32 HWL_ChannelInputPcrModeSend(HWL_ChannelInputPcrMode_t *veryfyMode)
{
	return __HWL_DataPerformLock(ICPL_TAGID_PCR_VERYFY, (ICPL_CmdFun_t)ICPL_OPtorChannelInputPcrModeMake, veryfyMode);
}


/*	输入流减少时FPGA报告CPU  */
S32 HWL_InputStreamNotifierTableGet(HWL_InputStreamNotifierTable_t *streamNotifierTable)
{
	return __HWL_DataPerformLock(ICPL_TAGID_STREAM_D_NOTIFY, NULL, streamNotifierTable);
}



/*	映射源(输入逻辑通道和UDP端口)的设置 */
S32 HWLPidMapMapOriginalPortTableSend(HWL_PidMapMapOriginalPortTable_t *pidMapPortTable)
{
	return __HWL_DataPerformLock(ICPL_TAGID_PIDMAP_PORT_SET, NULL, pidMapPortTable);
}


/*	输入端口号节目的内容有变化时报告CPU */
S32 HWL_InputPortEsNotifierTableGet(HWL_InputPortEsNotifierTable_t *esnotifier)
{
	return __HWL_DataPerformLock(ICPL_TAGID_STREAM_ES_NOTIFY, NULL, esnotifier);
}

/*	PID映射.*/
S32 HWL_PidMapOriginalToDestinedTableSend(HWL_PidMapOriginalToDestinedTable_t *pidMapTable)
{
	return __HWL_DataPerformLock(ICPL_TAGID_PIDMAP_SOURCE_TO_DES_SET, (ICPL_CmdFun_t)ICPL_OPtorPidMapOriginalToDestinedTableMake, pidMapTable);
}


/* 	设置输出码率对应的96MHz的脉冲数(BL85KM模块输出需要  */
S32 HWL_OutputRate96MhzSend(HWL_OutputRate96Mhz_t   *rateSet)
{
	return __HWL_DataPerformLock(ICPL_TAGID_OUTPUT_RATE_96M_BL85KM,	(ICPL_CmdFun_t)ICPL_OPtorOutputRate96MhzMake, rateSet);

}


S32 HWL_VodInputTsParamSend(HWL_VodInputTsParam_t *vodInputTsParam )
{
	return __HWL_DataPerformLock(ICPL_TAGID_VOD_INPUT_PARAM, (ICPL_CmdFun_t)ICPL_OPtorVodInputTsParamMake, vodInputTsParam);
}


S32 HWL_VodPatAndCrcSetSend(HWL_VodPatAndCrcSet_t *vodInputTsParam )
{
	return __HWL_DataPerformLock(ICPL_TAGID_VODPATCRC , (ICPL_CmdFun_t)ICPL_OPtorVodPatAndCrcSetMake, vodInputTsParam);
}



/* 30. 清除功能模块的复位标志   */
S32 HWL_ModuleResetSend(HWL_ModuleReset_t *moduleReset )
{
	return __HWL_DataPerformLock(ICPL_TAGID_MODULE_RESET , (ICPL_CmdFun_t)ICPL_OPtorModuleResetMake, moduleReset);

}


/* 风扇模块 */
void HWL_FanInit(void)
{
#if defined(GQ3710A) ||defined(GQ3650DS) || defined(GQ3650DR) 
	DRL_FanInit();
#endif
}

void HWL_FanEnable(BOOL bEnable)
{
#if defined(GQ3710A) ||defined(GQ3650DS) || defined(GQ3650DR) 
	if (bEnable)
	{
		DRL_FanStatusSet(HWL_CONST_ON);
	}
	else
	{
		DRL_FanStatusSet(HWL_CONST_OFF);
	}
#endif
}

BOOL HWL_FanStatusGet(void)
{
#if defined(GQ3710A) ||defined(GQ3650DS) || defined(GQ3650DR) 
	if (DRL_FanStatusGet() == HWL_CONST_ON)
	{
		return TRUE;
	}
#endif
	return FALSE;
}

void HWL_FanDestroy()
{
#if defined(GQ3710A) ||defined(GQ3650DS) || defined(GQ3650DR) 
	DRL_FanDestroy();
#endif
}



/************************************************************************************/
/*	缓冲区管理 */
/************************************************************************************/

void  __HWL_PidMapLockDestroy()
{}

S32  __HWL_PidMapLockInit()
{
	return 0;
}


void __HWL_PidMapLock()
{

}

void __HWL_PidMapUnlock()
{

}

/*	管理分配在堆中的缓冲数据.提供压入和清空等操作
*	@(data):
*	@(size):当期大小
*	@(capacity):最大容量.
*/
typedef struct tagHWL_BuffStock
{
	void *data;					//指针头.
	U32	size;

	U32 itemSize;
	U32 capacity;					//容量

	//pthread_mutex_t mutex;		//互斥

} HWL_BuffStock_t;


/**	创建缓冲区
*	@(itemSize): 	单位大小.
*	@(num):		单位数目.
*	@(return):	失败-NULL;
*/
static HWL_BuffStock_t *HWL_BuffStockCreate(U32 itemSize, U32 num)
{
	HWL_BuffStock_t *stock;
	void *data;
	assert(itemSize > 0 && num > 0);
	data = GLOBAL_MALLOC(itemSize * num);
	assert(data != NULL);
	if(data == NULL)
	{
		return NULL;
	}

	stock = GLOBAL_MALLOC(sizeof(HWL_BuffStock_t));
	assert(stock != NULL);
	if(stock == NULL)
	{
		GLOBAL_FREE(data);
		return NULL;
	}


	stock->data = data;
	stock->itemSize = itemSize;
	stock->size = 0;
	stock->capacity = num;

	//pthread_mutex_init(&stock->mutex,NULL);

	return stock;
}



/**	清空缓冲区.*/
static void  HWL_BuffStockClear(HWL_BuffStock_t *stock)
{
	assert(stock != NULL);
	if(stock == NULL)
	{
		return ;
	}

	stock->size = 0;

}


/**	destory ite ! .*/
static void HWL_BuffStockDestory(HWL_BuffStock_t *stock)
{
	assert(stock != NULL);
	if(stock == NULL)
	{
		return ;
	}
	GLOBAL_FREE(stock->data);
	GLOBAL_FREE(stock);
}


/**	sort It ..*/
static void HWL_BuffStockSort(HWL_BuffStock_t *stock,  int (*compare)(const void *, const void *))
{
	assert(stock != NULL);
	if(stock == NULL)
	{
		return ;
	}


	qsort(stock->data, stock->size, stock->itemSize, compare);


}


/**查找某一单元是否已经存在.*/
static void *HWL_BuffStockExist(HWL_BuffStock_t *stock,  int (*compare)(const void *, const void *) , const void *data)
{
	U32 i;
	void *find = NULL;
	U8 *buff;
	assert(stock != NULL && data != NULL && compare != NULL);
	if(stock == NULL || data == NULL || compare == NULL)
	{
		return NULL;
	}

	buff = stock->data;
	for(i = 0; i < stock->size ; i++)
	{
		if(compare(buff , data) == 0)
		{
			find = buff;
			break;
		}
		buff += (stock->itemSize);
	}


	return find;
}



/**	遍历所有的单元.*/
static void HWL_BuffStockTravsel(HWL_BuffStock_t *stock, void (*travsel)(void *))
{
	U32 i;
	U8 *buff;
	assert(stock != NULL && travsel != NULL);
	if(stock == NULL || travsel == NULL)
	{
		return ;
	}



	buff = stock->data;
	for(i = 0; i < stock->size ; i++)
	{
		travsel(buff);
		buff += (stock->itemSize);
	}



}



/**	向缓冲区中加入新项目.*/
static S32 HWL_BuffStockAppend(HWL_BuffStock_t *stock, void *item , U32 size )
{
	S32 retval;
	U8 *buff;


	assert(size == stock->itemSize);
	if(stock == NULL || item == NULL || size != stock->itemSize)
	{
		return -2;
	}


	buff = stock->data;


	if(stock->size < stock->capacity)
	{
		GLOBAL_MEMCPY(buff + (stock->size * stock->itemSize) , item, size);
		stock->size++;
		retval = 0;
	}
	else
	{
		retval = -1;
	}


	return retval;
}



static S32 HWL_BuffStockDido(HWL_BuffStock_t *stock, S32 (*fun)(HWL_BuffStock_t *))
{
	S32 retval;
	assert(stock != NULL && fun != NULL);
	if(stock == NULL || fun == NULL)
	{
		return -2;
	}


	retval = fun(stock);
	return retval;
}




/************************************************************************************/
/*	心跳信息处理. */
/************************************************************************************/

/**	1.心跳数据接口(请求) 查询心跳信息,仅发送命令字给FPGA..*/
S32 HWL_HeatBeatSend()
{
#ifdef GN1846
	U8 plData[9];

	if (FSPI_Read(FPGA_OP_MODE_HW_INFO, plData, 9)) {
		return ICPL_DataProcess(ICPL_TAGID_HEADBEAT, plData, 9);
	}
	return ERROR;

#else
	return  __HWL_DataPerformLock(ICPL_TAGID_HEADBEAT, ICPL_OPtorHeatBeatMake, NULL);
#endif
}


/**	设备温度.*/
S32 HWL_Temperature()
{	
	U32 tmp1,tmp2;
	S32 temper;
	float tt;
	HWL_HeatBeat_t heatBeat;
	HWL_DataPerchaseLock(ICPL_TAGID_HEADBEAT, heatBeat);
	temper= heatBeat.temperature;

	tmp1=(0xFFFF-temper);
	tmp2=(temper-0);
	if( tmp1>tmp2 )
	{
		tt=	(0.0625*tmp2);
	}
	else
	{
		tt= -(0.0625*tmp2);
	}

	temper=tt;
	return temper; 
}


/**	速率统计.*/
U16 HWL_AllRateInfo(HWL_AllRateInfo_t  *rateInfo)
{
	HWL_HeatBeat_t heatBeat;
	assert(rateInfo != NULL);
	HWL_DataPerchaseLock(ICPL_TAGID_HEADBEAT, heatBeat);
	rateInfo->asiRate = heatBeat.InTSRate;
	rateInfo->qamRate = heatBeat.OutTsRate;

	return 0;
}


/**	pll值.*/
U32 HWL_PLLValue()
{
	HWL_HeatBeat_t heatBeat;
	HWL_DataPerchaseLock(ICPL_TAGID_HEADBEAT, heatBeat);
	return heatBeat.chipPll;
}

/**	输出通道缓冲区状态.*/
BOOL HWL_GetQAMBuffStatus(U8 channelId)
{
	HWL_HeatBeat_t heatBeat;
	HWL_DataPerchaseLock(ICPL_TAGID_HEADBEAT, heatBeat);
	return (heatBeat.buffFlag & (0x1 << channelId)) > 0?TRUE:FALSE;
}


/*	Tuner短路状态.*/

BOOL HWL_GetTunerShortStatus(U8 channelId)
{
	HWL_HeatBeat_t heatBeat;
	HWL_DataPerchaseLock(ICPL_TAGID_HEADBEAT, heatBeat);
	return (heatBeat.shortStatus & (0x1 << channelId)) > 0?TRUE:FALSE;
}


/************************************************************************************/
/**	向DPB板进行TS插入188数据(CPU-〉FPGA)  TAG = 0x0a */
/************************************************************************************/
void HWL_SetDirectOutTsPacket(S16 OutTsIndex, U8 *pTsData, S32 DataSize)
{
	HWL_DPBTsInsert_t dpbt;
	dpbt.physicChannelId = 0x0;
	dpbt.logicChannelId = OutTsIndex;
	dpbt.tsDataBuff = pTsData;
	dpbt.tsDataBuffSize = DataSize;

#ifdef INSERTER_PID_TEST
	s_PIDCount[MPEG2_TsGetPID(pTsData)]++;
#endif

#ifdef SUPPORT_NEW_HWL_MODULE
	HWL_MonitorPlusInserterPacketNum(1);
#else
	s_InternalPacketCount++;
#endif

	__HWL_DataPerformLock(ICPL_TAGID_DPB_TS , ICPL_OPtorDPBTsInsertMake , &dpbt);
}


S32 HWL_HWInserterSet(S16 SlotIndex, S16 TsIndex, U8 *pData, S32 DataSize, S32 Interval)
{
	S32 i;
	HWL_DPBTsInserter lHWLInserter;

	if (DataSize>MPEG2_TS_PACKET_SIZE * 4)
	{
		DataSize = MPEG2_TS_PACKET_SIZE;
	}

	lHWLInserter.save = SlotIndex;
	lHWLInserter.outLogChannel = TsIndex;
	lHWLInserter.allNumber = (DataSize / MPEG2_TS_PACKET_SIZE) - 1;
	lHWLInserter.timeInterval = Interval;
	lHWLInserter.tsBuffLen = MPEG2_TS_PACKET_SIZE;
	lHWLInserter.packetType = 0;
	lHWLInserter.evenOrOdd = 0;
	lHWLInserter.casChannelNo = 0;

	if (DataSize > MPEG2_TS_PACKET_SIZE)
	{
		GLOBAL_TRACE(("Error! Data Size = %d\n", DataSize));
	}

	//GLOBAL_TRACE(("TsInd = %d, PID = %d , Interval = %d\n", TsIndex, MPEG2_TsGetPID(pData), Interval));

	for (i = 0; i < DataSize / MPEG2_TS_PACKET_SIZE; i++)
	{
		lHWLInserter.sendNumber = i;
		lHWLInserter.tsBuff = &pData[i * MPEG2_TS_PACKET_SIZE];
		__HWL_DataPerformLock(ICPL_TAGID_STABLE_DPB_TS , ICPL_OPtorDPBStableTsInsertMake, &lHWLInserter);
	}

	return 0;
}

/**	清除永久TS插入的PSI信息
*	@(offset): 如果 = 0xffff  , 为清除全部PSI 信息.
*/
S32 HWL_HWInserterClear(U16 offset)
{
	HWL_DPBTsInserter insert;
	insert.save = offset;

	//GLOBAL_TRACE(("Remove Inserter Slot = %d\n", offset));
#ifdef GN1846
	return 0;
#else
	return __HWL_DataPerformLock(ICPL_TAGID_STABLE_DPB_TS , ICPL_OPtorDPBStableTsInsertClearMake, &insert);
#endif
}


/**	查询硬件状态信息 .*/
S32 HWL_PhyStatusQskSend(void)
{
#ifdef GN1846
	U8 plData[9];

	if (FSPI_Read(FPGA_OP_MODE_HW_INFO, plData, 9)) {
		return ICPL_DataProcess(ICPL_TAGID_PHY, plData, 9);
	}
	return ERROR;
	
#else
	HWL_PhyStatusQsk_t phyQsk;
	phyQsk.requestType = 0x00;
	phyQsk.physicChannelId = 0x00;
	return __HWL_DataPerformLock(ICPL_TAGID_PHY, ICPL_OPtorPhyQskMake , &phyQsk);
#endif
}


/**	输入码率查询.*/
S32 HWL_InputChnRateRequest()
{
#ifndef GN1846
	HWL_ChannelRateInfo_t  request;
	request.requestType = 0x01;
	request.physicChannelId = 0x00;
	return  __HWL_DataPerformLock(ICPL_TAGID_CHANNELRATE, ICPL_OPtorChannelRateInfoMake , &request);
#endif
}

/**	输出码率查询.*/
S32 HWL_OutputChnRateReqeust()
{
#ifndef GN1846
	HWL_ChannelRateInfo_t  request;
	request.requestType = 0x02;
	request.physicChannelId = 0x01;
	return  __HWL_DataPerformLock(ICPL_TAGID_CHANNELRATE, ICPL_OPtorChannelRateInfoMake , &request);
#endif
}

S32 HWL_OutputChnRateReqeustForS()
{
	HWL_ChannelRateInfo_t  request;
	request.requestType = 0x02;
	request.physicChannelId = 0x02;
	return  __HWL_DataPerformLock(ICPL_TAGID_CHANNELRATE, ICPL_OPtorChannelRateInfoMake , &request);
}

/**	码率查询应答.*/
S32 HWL_ChannelRateArray(HWL_ChannelRateArray_t *rateArray)
{
	return  __HWL_DataPerchaseLock(ICPL_TAGID_HEADBEAT, rateArray, sizeof(HWL_ChannelRateArray_t));
}



/**	pick 表格大小 */
static void __HWL_ChannelRateArraySizePick(struct tagICPL_Cmd_t *cmd, void *intParam,
										   U32 noused)
{
	HWL_ChannelRateArray_t  *array = (HWL_ChannelRateArray_t *)cmd->data;
	assert(intParam != NULL);
	assert(cmd->tagid == ICPL_TAGID_CHANNELRATE);
	HWL_DEBUG("array->channelRateArraySize:%d\n", array->channelRateArraySize);
	*(int *)(intParam) = array->channelRateArraySize;

}

/**	pick 表格的index项  */
static void __HWL_ChannelRateArrayItemPick(const struct tagICPL_Cmd_t *cmd, void *item,
										   U32 index)
{
	HWL_ChannelRateArray_t *array = (HWL_ChannelRateArray_t *)cmd->data;
	assert(item != NULL);
	assert(cmd->tagid == ICPL_TAGID_CHANNELRATE);

	if(index >= array->channelRateArraySize)
	{
		GLOBAL_MEMSET(item, 0,  sizeof(HWL_ChannelRate_t));
	}
	else
	{
		*(HWL_ChannelRate_t *)item = array->channelRateArray[index];
	}
}


/**	pick 表格头 */
static void __HWL_ChannelRateArrayHeadPick(const struct tagICPL_Cmd_t *cmd, void *param,
										   U32 noused)
{
	HWL_ChannelRateInfo_t *head = (HWL_ChannelRateInfo_t *)param;
	HWL_ChannelRateArray_t  *array = (HWL_ChannelRateArray_t *)cmd->data;
	assert(param != NULL);
	assert(cmd->tagid == ICPL_TAGID_CHANNELRATE);
	head->physicChannelId = array->physicChannelId;
	head->requestType = array->requestType;
	head->totalRate = array->totalRate;

}



/**	wrapper..*/
U32 HWL_ChannelRateArraySize()
{
	U32 size;
	____HWL_DataPerchaseLock(ICPL_TAGID_CHANNELRATE, &size, 0, __HWL_ChannelRateArraySizePick, 0);
	return size;
}


/**	wrapper..*/
U32 HWL_ChannelRateArrayItem(U32 index, HWL_ChannelRate_t *channelRate)
{

	return ____HWL_DataPerchaseLock(ICPL_TAGID_CHANNELRATE, channelRate, index, __HWL_ChannelRateArrayItemPick, 0);
}



/**	在FPGA返回的信息中包括 requestType ,physicChannelId 标识,可以先用此函数读取头来检查是不是有效数据...*/
U32 HWL_ChannelRateArrayHead(HWL_ChannelRateInfo_t *head)
{
	return ____HWL_DataPerchaseLock(ICPL_TAGID_CHANNELRATE, head, 0, __HWL_ChannelRateArrayHeadPick, 0);

}


/************************************************************************/
/* PID IP 统计                                                          */
/************************************************************************/
static HWL_PIDETHStatistcs s_HWLStatistics;

/**	IP端口码率统计信息清除..*/
S32 HWL_IPPortStatisticClear(S32 ChnIndex)
{
	HWL_EthDetectionParam lParam;
	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));
	lParam.physicChannelId = ChnIndex & 0xFF;
	if (GLOBAL_CHECK_INDEX(ChnIndex, HWL_CONST_CHANNEL_NUM_MAX))//同时清除当前已经拥有的数据
	{
		s_HWLStatistics.m_IPArray.ArrySize = 0;
		s_HWLStatistics.m_IPArray.totalRate = 0;
	}
	return __HWL_DataPerformLock(ICPL_TAGID_IPSTATICS, (ICPL_CmdFun_t)ICPL_OPtorInputIPPortStatisticClearMake , &lParam);
}

/**	IP端口码率统计信息查询..*/
S32 HWL_IPPortStatisticSend(S32 ChnIndex)
{
	HWL_EthDetectionParam lParam;
	GLOBAL_ZEROMEM(&lParam, sizeof(lParam));
	lParam.physicChannelId = ChnIndex & 0xFF;
	return __HWL_DataPerformLock(ICPL_TAGID_IPSTATICS, (ICPL_CmdFun_t)ICPL_OPtorInputIPPortStatisticSendMake , &lParam);
}

S32 HWL_IPPortStatisticArrayGet(S32 ChnIndex, HWL_IPStatisticsArray *pArray)
{
	if (pArray)
	{
		if (GLOBAL_CHECK_INDEX(ChnIndex, HWL_CONST_CHANNEL_NUM_MAX))
		{
			GLOBAL_MEMCPY(pArray, &s_HWLStatistics.m_IPArray, sizeof(HWL_IPStatisticsArray));
		}
	}
	//return __HWL_DataPerchaseLock(ICPL_TAGID_IPSTATICS, pArray, sizeof(HWL_IPStatisticsArray));
	return HWL_SUCCESS;
}


/**	PID 统计的端口码率查询端口设置*/
S32 HWL_PidStatisticSend(U8 ChnIndex, U8 SubIndex)
{
	HWL_PidStatics_t pid;
	pid.physicChannelId = ChnIndex;
	pid.logicChannelId = SubIndex;
	return __HWL_DataPerformLock(ICPL_TAGID_PIDSTATICS, ICPL_OPtorPidStatisticMake, &pid);
}

/**	PID 统计信息获取.*/
S32 HWL_PidStatisticSearch(U8 ChnIndex, U8 SubIndex)
{
	HWL_PidStatics_t pid;
	pid.physicChannelId = ChnIndex;
	pid.logicChannelId = SubIndex;
	return __HWL_DataPerformLock(ICPL_TAGID_PIDSTATICS, ICPL_OPtorPidStatisticSearchMake, &pid);
}


/**	PID 端口统计数据.*/
S32 HWL_PidStatisticArray(HWL_PIDStatisticsArray *pArray)
{
	if (pArray)
	{
		GLOBAL_MEMCPY(pArray, &s_HWLStatistics.m_PIDArray, sizeof(HWL_PIDStatisticsArray));
	}
	//return __HWL_DataPerchaseLock(ICPL_TAGID_PIDSTATICS, param, sizeof(HWL_PIDStatisticsArray) );
	return HWL_SUCCESS;
}



static void __HWL_PidStatisticArraySizePick(const struct tagICPL_Cmd_t *cmd, void *intParam,
											U32 noused)
{
	HWL_PIDStatisticsArray *array = (HWL_PIDStatisticsArray *)cmd->data;
	assert(intParam != NULL);
	assert(cmd->tagid == ICPL_TAGID_PIDSTATICS);
	*(int *)(intParam) = array->channelRateArraySize;
}


static void __HWL_PidStatisticArrayItemPick(const struct tagICPL_Cmd_t *cmd, void *item,
											U32 index)
{
	HWL_PIDStatisticsArray *array = (HWL_PIDStatisticsArray *)cmd->data;
	assert(item != NULL);
	assert(cmd->tagid == ICPL_TAGID_PIDSTATICS);
	if(index >= array->channelRateArraySize)
	{
		GLOBAL_MEMSET(item, 0,  sizeof(HWL_PidRate_t));
	}
	else
	{
		*(HWL_PidRate_t *)item = array->pidRateArray[index];
	}
}



/**	返回PID 端口统计数据中的条目个数.*/
U32 HWL_PidStatisticArraySize()
{
	U32 size;
	____HWL_DataPerchaseLock(ICPL_TAGID_PIDSTATICS, &size, 0, __HWL_PidStatisticArraySizePick, 0);
	return size;
}


/**	单条PID统计信息数据检索.
*	@(index): [0-size). size由HWL_PidStatisticArraySize()确定,
*	@(pidRate):输入缓冲.本函数将用实际数据填充此内存.
*/
S32 HWL_PidStatisticArrayItem(U32 index, HWL_PidRate_t *pidRate)
{
	return ____HWL_DataPerchaseLock(ICPL_TAGID_PIDSTATICS, pidRate, index, __HWL_PidStatisticArrayItemPick, 0);
}




/************************************************************************************/
/**	Control Word 操作.*/
/************************************************************************************/


/**	CW传送.*/
S32 HWL_ControlWordSet(HWL_ControlWord_t *cw)
{
	return __HWL_DataPerformLock(ICPL_TAGID_CW_SEND, ICPL_OPtorControlWordMake , cw);
}


/**	CW切换*/
S32 HWL_ControlWordSwitchSet(U8 evenOrOdd)
{
	HWL_ControlWord_t cw;
	cw.evenOrOdd = evenOrOdd;
	return __HWL_DataPerformLock(ICPL_TAGID_CW_SWITCH, ICPL_OPtorControlWordSwitchMake , &cw);
}


/**	设置加扰模块开关.*/
void HWL_ScrambleEnable(BOOL bEnable)
{
	HWL_ConditionAccess_t access;
	if (bEnable)
	{
		access.enabled = 1;
	}
	else
	{
		access.enabled = 0;
	}
	__HWL_DataPerformLock(ICPL_TAGID_CA_ENABLE, ICPL_OPtorCondtionAccessMake, &access);
}


/**	获取PSI接受Buffer大小*/
S32 HWL_PSIBuffSizeGet(HWL_PSIBuffSize_t *psiBuff)
{
	assert(psiBuff != NULL);
	return __HWL_DataPerchaseLock(ICPL_TAGID_PSI_NUM, psiBuff, sizeof(HWL_PSIBuffSize_t) );
}


/************************************************************************************/
/**	COPY PID 表操作..*/
/************************************************************************************/


static HWL_BuffStock_t *pidcopyoriginalstock;
static HWL_BuffStock_t *pidcopydestinedstock;

static void __HWL_PidCopyStockDestroy()
{
	HWL_BuffStockDestory(pidcopyoriginalstock);
	HWL_BuffStockDestory(pidcopydestinedstock);
}
static void __HWL_PidCopyStockInit()
{
	pidcopyoriginalstock = HWL_BuffStockCreate(sizeof(HWL_PIDSource_t) , HWL_CONST_PIDCOPY_NUM_MAX);
	pidcopydestinedstock = HWL_BuffStockCreate(sizeof(HWL_PIDdestined_t) , HWL_CONST_PIDCOPY_NUM_MAX);
	HWL_DEBUG("\n");
}

/**	以下三个函数提供 对PID源表的操作 。
*	HWL_PidCopyOriginalTableAppend...向内存中加入新的PID 源表项.		..MAX-4096
*/

S32  HWL_PidCopyOriginalTableAppend(HWL_PIDSource_t *pidSource)
{

	return HWL_BuffStockAppend(pidcopyoriginalstock , pidSource , sizeof(HWL_PIDSource_t));
}

/**
*	分帧打入FPGA.
*/
static S32 __HWL_PIDCopyOriginalTableSet(HWL_BuffStock_t *stock)
{
	U32 k;
	U32 time;
	U32 reminder;
	U32 record;
	HWL_PIDSource_t *item;
	HWL_PIDCopyOriginalTable_t table;
	table.physicChannel = 0x0;

	time = (stock->size + HWL_CONST_PIDSOURCE_MAX - 1) / HWL_CONST_PIDSOURCE_MAX;
	reminder = (stock->size) % HWL_CONST_PIDSOURCE_MAX;


	item = stock->data;

	k = 0;
	while(k < time)
	{
		if((k + 1) == time)
		{
			record = reminder;
		}
		else
		{
			record = HWL_CONST_PIDSOURCE_MAX;
		}

		table.pidsourceTable = item + (  k * HWL_CONST_PIDSOURCE_MAX );
		table.pidsourceSize = record;
		table.flag = k;

		__HWL_DataPerformLock(ICPL_TAGID_PIDCOPY_ORIGINAL, ICPL_OPtorPIDCopyOriginalTableMake, &table);

		k++;
	}



	return 0;

}



/************************************************************************************/
/**	COPY PID 表操作接口..
**	只需要指定源表项目中的id和目的表项中的id为相同，则表示其关联关系。
*	example:
*	set_copy_pid_table(){
*		HWL_PidCopyTableClear();
*
*		i=0;
*		for(PIDSource in  list){
*			PIDSource.id=i;
*			HWL_PidCopyOriginalTableAppend(item);
*			for(PIDDestined in PIDSource.output copy List){
*				PIDDestined.id=i;
*				HWL_PidCopyDestinedTableAppend(item);
*			}
*		}
*
*		HWL_PIDCopyTableSet();		//set to fpga...
*	}
*/
/************************************************************************************/



/**	以下三个函数提供 对PID源表的操作 。
*	HWL_PidCopyDestinedTableAppend...
*	HWL_PidCopyOriginalTableAppend...向内存中加入新的PID 源表项.		..MAX-4096
*/

void HWL_PidCopyTableClear()
{
	HWL_BuffStockClear(pidcopyoriginalstock);
	HWL_BuffStockClear(pidcopydestinedstock);
}


/**	以下三个函数提供 对PID目的表的操作 。
*	HWL_PidCopyOriginalTableAppend...向内存中加入新的PID 源表项.		..MAX-4096

*/

S32  HWL_PidCopyDestinedTableAppend(HWL_PIDdestined_t *pidDestined)
{
	return HWL_BuffStockAppend(pidcopydestinedstock , pidDestined , sizeof(HWL_PIDdestined_t));
}

/**
*	分帧打入FPGA.
*/
static S32  __HWL_PIDCopyDestinedTableSet(HWL_BuffStock_t *stock)
{

	U32 k;
	U32 time;
	U32 reminder;
	U32 record;
	HWL_PIDdestined_t *item;
	HWL_PIDCopyDestinedTable_t table;
	table.physicChannel = 0x0;

	time = (stock->size + HWL_CONST_PIDDESTINED_MAX - 1) / HWL_CONST_PIDDESTINED_MAX;
	reminder = (stock->size) % HWL_CONST_PIDDESTINED_MAX;


	item = stock->data;

	k = 0;
	while(k < time)
	{
		if((k + 1) == time)
		{
			record = reminder;
		}
		else
		{
			record = HWL_CONST_PIDDESTINED_MAX;
		}

		table.pidDestinedTable = item + (  k * HWL_CONST_PIDDESTINED_MAX );
		table.pidDestinedSize = record;
		table.flag = k;

		__HWL_DataPerformLock(ICPL_TAGID_PIDCOPY_DESTINED, ICPL_OPtorPIDCopyDestinedTableMake, &table);

		k++;
	}

	return 0;

}



/**	比较函数，用以排序.
*	源通道号越大的在后面,相同的源通道号PID越大的在后面.
*/
static int __HWL_PIDSourceCompare(const void *_a , const void *_b)
{
	HWL_PIDSource_t *a = (HWL_PIDSource_t *)_a;
	HWL_PIDSource_t *b = (HWL_PIDSource_t *)_b;

	assert(a != NULL && b != NULL);

	if(a->originalChannelID < b->originalChannelID)
	{
		return -1;
	}
	else if(a->originalChannelID > b->originalChannelID)
	{
		return 1;
	}



	if(a->originalPid < b->originalPid)
	{
		return -1;
	}
	else if(a->originalPid > b->originalPid)
	{
		return 1;
	}

	return 0;

}


/**	目的通道排序.按HWL_PIDdestined_t.id排序，该过程会将相同ID的对象连续排列. */
static int __HWL_PIDDestinedCompare(const void *_a , const  void *_b)
{
	HWL_PIDdestined_t *a = (HWL_PIDdestined_t *)_a;
	HWL_PIDdestined_t *b = (HWL_PIDdestined_t *)_b;

	if(a->id < b->id)
	{
		return -1;
	}
	else if(a->id > b->id)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**	从表格中查找 PIDSOURCE 项 .*/
static inline HWL_PIDSource_t *__HWL_PIDSourceTableFind( HWL_PIDSource_t *table , U32 size, U16 id)
{
	U32 i;
	for(i = 0; i < size; i++)
	{
		if(table[i].id == id)
		{
			return &table[i];
		}
	}

	return NULL;
}


/**	遍历目的PID表计算源PID表中的个数和偏移关系.*/
static   void  __HWL_PIDCopyOffset(HWL_BuffStock_t *original, HWL_BuffStock_t *destined)
{

	U32 i;
	U8	*sourceIdx;
	U8	*destinedIdx;
	U32 oldid;
	U32 count;
	HWL_PIDSource_t *sourceItem;
	HWL_PIDdestined_t *destinedItem;

	sourceIdx = original->data;
	destinedIdx = destined->data;




	destinedItem = (HWL_PIDdestined_t *)destinedIdx;
	oldid = destinedItem->id;

	i = 1;			//计数器，遍历目的表.
	while(1)
	{

		count = 0;		//相同ID的项的计数器.

		do
		{
			count++;

			destinedIdx += sizeof(HWL_PIDdestined_t);
			destinedItem = (HWL_PIDdestined_t *)destinedIdx;

		}
		while(  (i++ < destined->size)  &&  destinedItem->id == oldid) ;


		sourceItem = __HWL_PIDSourceTableFind(original->data, original->size, oldid);
		//HWL_DEBUG("old_id:%d\n",oldid);
		if(sourceItem != NULL)
		{
			//	HWL_DEBUG("count:%d\n",count);
			sourceItem->destinedPidOutListNum = count;
			sourceItem->destinedPidOutListOffset = (4 * ( i - (count) ) - 4) / 4;
		}

		oldid = destinedItem->id;

		if(i > destined->size)
		{
			break;
		}

	}

}


/**
*		1.排序源PID表。
*		2.紧缩目的PID表.
*		3.遍历目的PID表，为源PID表中的对应项指明 OFFSET和 个数.
*		4.设置源表到FPGA.
*		5.设置目的表到FPGA.
*/
void HWL_PIDCopyTableSet()
{
	HWL_BuffStock_t *source , *destined;
	source = pidcopyoriginalstock;
	destined = pidcopydestinedstock;

	HWL_DEBUG("\n");

	if(source == NULL || destined == NULL)
	{
		return ;
	}

	//sort
	HWL_BuffStockSort(source, __HWL_PIDSourceCompare);
	HWL_BuffStockSort(destined, __HWL_PIDDestinedCompare);


	//offset.
	__HWL_PIDCopyOffset(source, destined);


	//set to fpga.
	__HWL_PIDCopyOriginalTableSet(source);
	__HWL_PIDCopyDestinedTableSet(destined);




}


/************************************************************************************/
/**	PowerPC对调制器有关的芯片的寄存器设置和读取: (PHY_Ch = 0)	*/
/************************************************************************************/


S32 HWL_ModularChipTableSend(HWL_ModularChipRegister_t *table, U32 size)
{
	S32 retval;
	HWL_ModularChipRegisterTable_t chiptable;
	chiptable.physicChannel = 0x0;
	chiptable.moduleRegisterTable = table;
	chiptable.moduleRegisterSize = size;

	//     HWL_DEBUG("perform before\n");
	retval=__HWL_DataPerformLock(ICPL_TAGID_GROUP_MODULOR_REGISTER, ICPL_OPtorModularChipRegisterTableMake, &chiptable);
	// 	HWL_DEBUG("perform after\n");
}



/************************************************************************************/
/**	加密芯片 操作..*/
/************************************************************************************/


/** 	写加密芯片的通讯 */
S32 HWL_EncryptChipWrite(U8 *buff , U8 bufflen, U8  Index)
{
	HWL_EncryptChip_t chip;

	assert(bufflen == 10);

	GLOBAL_MEMCPY(chip.buff, buff, bufflen);

	chip.mainNo = 0x0b + Index / 4;
	chip.subNo = 0x10 + Index % 4;
	chip.length = 0x0a;

	return __HWL_DataPerformLock(ICPL_TAGID_ENCRYPT , ICPL_OPtorEncryptChipWriteMake, &chip);

}


/**	发送读加密芯片命令.*/
S32 HWL_EncryptChipReadSend(U8  Index )
{
	HWL_EncryptChip_t chip;
	chip.mainNo = 0x0b + Index / 4;
	chip.subNo = 0x00 + Index % 4;
	chip.length = 0x0a;

	return __HWL_DataPerformLock(ICPL_TAGID_ENCRYPT , ICPL_OPtorEncryptChipReadMake, &chip);
}


/**	读取FPGA返回的加密芯片命令.*/
S32 HWL_EncryptChipRead(U8 *buff , U8 bufflen, U8 Index , S32 Timeout)
{
	S32 lRet, lCurTick;
	HWL_EncryptChip_t chip;

	HWL_EncryptChipReadSend(Index);

	lCurTick = PFC_GetTickCount();

	lRet = 0;
	while(Timeout > 0)
	{
		if (HWL_DataPerchaseLockSync(ICPL_TAGID_ENCRYPT, chip, lCurTick) == SUCCESS)
		{
			lRet = GLOBAL_MIN(bufflen, 10);
			GLOBAL_MEMCPY(buff, chip.buff, lRet);
			break;
		}
		PFC_TaskSleep(500);
		Timeout -= 500;
	}

	if (Timeout <= 0)
	{
		GLOBAL_TRACE(("Time Out\n"));
	}

	return lRet;
}



/************************************************************************************/
/**	设置输出通道工作方式: 直通或PID的映射方式 */
/************************************************************************************/


/* 设置输出通道和PID的映射方式为输入UDP端口(0..31)直通到输出通道的0..31 */
S32 HWL_PassModeSend(HWL_PassMode_t *passMode)
{
#ifdef GN1846
		return 1;
#else
		return __HWL_DataPerformLock(ICPL_TAGID_PASSMODE , ICPL_OPtorPassModeMake, passMode);
#endif
}




/************************************************************************************/
/**	PID映射 操作.*/
/************************************************************************************/

/**	PID 映射表输入缓冲 .*/
static HWL_BuffStock_t *pidMapStock;
static HWL_BuffStock_t *pidMapCopy;

void HWL_PerformPIDMap(void)
{
	HWL_PidMapArrayApply();
}

void HWL_AddPIDMap(S16 InTsIndex, U16 InPID, S16 OutTsIndex, U16 OutPID, BOOL bScramble, S32 CwGroupIndex)
{
	HWL_PidMapItem_t item;
#ifdef GM2730S
	item.inputLogicChannelId = (InTsIndex & 0xFFF) | ((OutTsIndex & 0xF00) << 4);
#else
	item.inputLogicChannelId = InTsIndex;
#endif
	item.inputPid = InPID;
#ifdef GM2730S
	item.outputLogicChannelId = OutTsIndex & 0xFF;
#else
	item.outputLogicChannelId = OutTsIndex;
#endif
	item.outputPid = OutPID;
	item.groupIndex = CwGroupIndex & 0x7F;
	item.isCA = ((bScramble > 0)?1:0);
	HWL_PidMapArrayAppend(&item);
}


void HWL_ClearPIDMap(void)
{
	HWL_PidMapArrayClear();
}

/**	some Wrappers. ...*/
static void __HWL_PidMapArrayInit()
{
	HWL_DEBUG("\n");
	pidMapStock = HWL_BuffStockCreate(sizeof(HWL_PidMapItem_t), HWL_CONST_PIDMAP_NUM_MAX);
	pidMapCopy = HWL_BuffStockCreate(sizeof(HWL_PidMapItem_t), HWL_CONST_PIDMAP_NUM_MAX);
}

static void __HWL_PidMapArrayDestroy()
{
	HWL_BuffStockDestory(pidMapStock);
	HWL_BuffStockDestory(pidMapCopy);
}

/**	清空全部FPGA中的PID映射参数*/
static S32 HWL_PidMapFpgaClearAll()
{
	HWL_PidMapItem_t param;

	param.inputLogicChannelId = 0xFFFF;
	param.inputPid = 0xFFFF;
	param.groupIndex = 0xFF;
	param.isCA = 0x0;
	param.outputLogicChannelId = 0xFF;
	param.outputPid = 0xFFFF;
	param.disflag = 0xFF;


	__HWL_DataPerformLock(ICPL_TAGID_PIDMAP , ICPL_OPtorPidMapSetMake , &param);
	__HWL_DataPerformLock(ICPL_TAGID_PIDCOPY_ORIGINAL , ICPL_ReplicatePidMapSRCClearSetMake , NULL);
	__HWL_DataPerformLock(ICPL_TAGID_PIDCOPY_DESTINED , ICPL_ReplicatePidMapDestClearSetMake , NULL);
	return 0;
}


void HWL_PidMapArrayClear()
{


	__HWL_PidMapLock();
	HWL_BuffStockClear(pidMapStock);
	HWL_BuffStockClear(pidMapCopy);

	HWL_BuffStockClear(pidcopyoriginalstock);
	HWL_BuffStockClear(pidcopydestinedstock);


	__HWL_PidMapUnlock();
}

/**	PID映射表的比较函数，用作排序使用。*/
static int __HWL_PidMapItemCmp(const void *_a, const void *_b);



S32 HWL_PidMapArrayAppend(HWL_PidMapItem_t *pidMap)
{
	S32 retval;
	HWL_PidMapItem_t *find;
	HWL_PidMapItem_t item;
	assert(pidMap != NULL);

	assert(pidMapStock != NULL &&
		pidMapCopy != NULL);
	if(pidMapStock == NULL || pidMapCopy == NULL)
	{
		return -1;
	}


	__HWL_PidMapLock();
	find = HWL_BuffStockExist(pidMapStock, __HWL_PidMapItemCmp, pidMap);

	if(find != NULL)
	{
		//         printf("xxxxxxxxxxxxxxxxx:%d\n", find->serialNo);
		if (pidMap->outputPid == find->outputPid 
			&& pidMap->outputLogicChannelId == find->outputLogicChannelId
			&& pidMap->outputPhyChannelId == find->outputPhyChannelId
			)
		{
			//GLOBAL_TRACE(("Repeat Item Not In Copy\n"));
		}
		else
		{
			if(find->serialNo == 12)//判断是否已经添加了一次！
			{
				// set in copy pid table.
				item.inputPhyChannelId = find->outputPhyChannelId;
				item.inputLogicChannelId = find->outputLogicChannelId;
				item.inputPid = find->outputPid;

				item.outputPhyChannelId = find->outputPhyChannelId;
				item.outputLogicChannelId = find->outputLogicChannelId;
				item.outputPid = find->outputPid;

				HWL_BuffStockAppend(pidMapCopy, &item, sizeof(HWL_PidMapItem_t));
				find->serialNo = 13;
			}

			// set in copy pid table.
			item.inputPhyChannelId = find->outputPhyChannelId;
			item.inputLogicChannelId = find->outputLogicChannelId;
			item.inputPid = find->outputPid;

			item.outputPhyChannelId = pidMap->outputPhyChannelId;
			item.outputLogicChannelId = pidMap->outputLogicChannelId;
			item.outputPid = pidMap->outputPid;

			HWL_BuffStockAppend(pidMapCopy, &item, sizeof(HWL_PidMapItem_t));

		}


	}
	else
	{
		pidMap->serialNo = 12;
		retval = HWL_BuffStockAppend(pidMapStock,  pidMap, sizeof(HWL_PidMapItem_t));
	}

	__HWL_PidMapUnlock();

	return retval;
}





static void ___HWL_PidMapTravsel(void *item)
{
	__HWL_DataPerformLock(ICPL_TAGID_PIDMAP , ICPL_OPtorPidMapSetMake , item);
}



/**	从PID映射表中建立PID-COPY表.
*	输入的PID映射表需要已排序.
*/
static void HWL_PidCopyTableBuild(HWL_PidMapItem_t *array, U32 size)
{
	U32 i;
	U32 id;
	HWL_PIDSource_t oldItem;
	HWL_PIDdestined_t destined;
	HWL_PidMapItem_t *pidItem;

	oldItem.physicInputId = -1;
	oldItem.originalPid = -1;
	oldItem.originalChannelID = -1;

	HWL_PidCopyTableClear();

	id = 0;
	for(i = 0; i < size; i++)
	{
		pidItem = &array[i];



		if(oldItem.originalPid != pidItem->inputPid
			|| oldItem.originalChannelID != pidItem->inputLogicChannelId
			|| oldItem.physicInputId != pidItem->inputPhyChannelId)
		{

			id++;

			oldItem.id = id;
			oldItem.originalChannelID = pidItem->inputLogicChannelId;
			oldItem.originalPid = pidItem->inputPid;
			oldItem.physicInputId = pidItem->inputPhyChannelId;

			HWL_PidCopyOriginalTableAppend( &oldItem);


		}

		destined.id = id;
		destined.destinedChannelID = pidItem->outputLogicChannelId;
		destined.destinedPid = pidItem->outputPid;

		HWL_PidCopyDestinedTableAppend(&destined);

	}
}


/**	为数组中的每一项赋予序列号.*/
static inline void __HWL_PidMapSerialNoIndexSet(HWL_PidMapItem_t *array, U32 size)
{
	U32 i;
	for(i = 0; i < size; i++)
	{
		array[i].serialNo = i;
		array[i].outputPhyChannelId = 0x01;		/*输出TS流的通道。*/
	}
}



/**
*	设置PID映射表.
*	1.清空原FPGA中设置.
*	2.将输入PID表排序。
*	3.按顺序写入FPGA.
*/
void HWL_PidMapArrayApply()
{
	if(pidMapStock == NULL || pidMapCopy == NULL)
	{
		return ;
	}

	//Clear FPGA.
	HWL_PidMapFpgaClearAll();
	//因为FPGA轻微的初值问题，这里手动擦除第一个项目

	PFC_TaskSleep(500);

	__HWL_PidMapLock();


	HWL_BuffStockSort(pidMapCopy , __HWL_PidMapItemCmp);	//排序
	HWL_BuffStockSort(pidMapStock , __HWL_PidMapItemCmp);	//排序

	__HWL_PidMapSerialNoIndexSet(pidMapStock->data, pidMapStock->size);

	GLOBAL_TRACE(("PID Map Count = %d\n", pidMapStock->size));
#if 0//测试
	{
		S32 i;
		HWL_PidMapItem_t *plTmpItem;
		for (i = 0; i < pidMapStock->size; i++)
		{
			plTmpItem = &((HWL_PidMapItem_t*)pidMapStock->data)[i];
			GLOBAL_TRACE(("InTs %d, InPID %d -> OutTs %d, OutPID %d\n", plTmpItem->inputLogicChannelId, plTmpItem->inputPid, plTmpItem->outputLogicChannelId, plTmpItem->outputPid));
		}
	}
#endif
	GLOBAL_TRACE(("PID Copy Count = %d\n", pidMapCopy->size));

	//分帧发给FPGA...@Fix.
	HWL_BuffStockTravsel(pidMapStock, ___HWL_PidMapTravsel);


	HWL_PidCopyTableBuild(pidMapCopy->data, pidMapCopy->size);

	//发送给FPGA.
	HWL_PIDCopyTableSet();

	__HWL_PidMapUnlock();

}





/**	排序比较.*/
static int __HWL_PidMapItemCmp(const void *_a, const void *_b)
{
	HWL_PidMapItem_t *a = (HWL_PidMapItem_t *)_a;
	HWL_PidMapItem_t *b = (HWL_PidMapItem_t *)_b;

	if(a->inputPhyChannelId < b->inputPhyChannelId)
	{
		return -1;
	}
	else if(a->inputPhyChannelId > b->inputPhyChannelId)
	{
		return 1;
	}

#ifdef GM2730S
	if( (a->inputLogicChannelId & 0xFFF)< (b->inputLogicChannelId & 0xFFF) )
	{
		return -1;
	}
	else if( (a->inputLogicChannelId & 0xFFF) > (b->inputLogicChannelId & 0xFFF))
	{
		return 1;
	}
#else
	if(a->inputLogicChannelId < b->inputLogicChannelId)
	{
		return -1;
	}
	else if(a->inputLogicChannelId > b->inputLogicChannelId)
	{
		return 1;
	}
#endif


	if(a->inputPid < b->inputPid)
	{
		return -1;
	}
	else if(a->inputPid > b->inputPid)
	{
		return 1;
	}

	return 0;

}



/************************************************************************************/
/**	PID搜索 操作.*/
/************************************************************************************/


/** 	PID搜索.
*	(@pidMap.inputLogicChannelId)
*/
S32	HWL_PidSearchStart(U16 channelId, U16 pid, U8 filter, U8 FlagOrCount)
{
	HWL_PidMapItem_t item;

	GLOBAL_MEMSET(&item, 0, sizeof(item));

	item.inputPhyChannelId = 0x0;
	item.inputLogicChannelId = channelId;
	item.inputPid = pid;
	item.outputPhyChannelId = 0xFE;
	item.des_flag = FlagOrCount;
	item.serialNo = filter;
	assert(filter < 32);

	if(filter >= 32)
	{
		return -2;
	}

	HWL_DEBUG("-----------------1\n");
	return __HWL_DataPerformLock(ICPL_TAGID_PIDMAP , ICPL_OPtorPidMapSetMake , &item);

}


/**	PID搜索停止.*/
S32	HWL_PidSearchStop(U16 channelId, U16 pid, U8 filter)
{
	S32 retval;
	HWL_PidMapItem_t item;
	memset(&item,0,sizeof(item));
	item.inputPhyChannelId = 0x0;
	item.inputLogicChannelId = channelId;
	item.inputPid = 0xFFFF;
	item.outputPhyChannelId = 0xFE;
	item.outputLogicChannelId=0xFF;
	item.outputPid = 0xFFFF;		// STOP
	item.serialNo = filter;

	HWL_TRACE("-----------------1\n");

	assert(filter < 32);

	if(filter >= 32)
	{
		return -2;
	}

	retval =__HWL_DataPerformLock(ICPL_TAGID_PIDMAP , ICPL_OPtorPidMapSetMake , &item);

	HWL_TRACE("-----------------2\n");

	return retval;
}



static HANDLE32 pidresponsemutex;
static HWL_CallBack_t  pidcallbackproxy = NULL;


static void PidResponseCallBack(struct tagICPL_Cmd_t *cmd, void *data)
{
	assert(cmd != NULL && data != NULL);
	PFC_SemaphoreWait(&pidresponsemutex , -1);			//lock

	if(pidcallbackproxy != NULL)
	{
		pidcallbackproxy(cmd->buff, cmd->buff_len);
	}

	PFC_SemaphoreSignal(&pidresponsemutex);			//unlock..
}





static  void __HWL_PidCallBackDestroy()
{
	PFC_SemaphoreDestroy(pidresponsemutex);
}

static  void __HWL_PidCallBackInit()
{
	pidresponsemutex = PFC_SemaphoreCreate("pidresponsemutex", 1);
}

void HWL_InterInit(void)
{
	__HWL_PidMapArrayInit();
	__HWL_PidCopyStockInit();
	__HWL_PidCallBackInit();
	__HWL_PidMapLockInit();
}

void HWL_InterDestory(void)
{
	__HWL_PidMapArrayDestroy();
	__HWL_PidCopyStockDestroy();
	__HWL_PidCallBackDestroy();
	__HWL_PidMapLockDestroy();
}



/**	存储编码方式.*/
static S32	 codeMode;


static inline S32 __ICPL_CodeModeGet()
{
	return codeMode;
}

/**	返回系统大小端编码方式.[ICPL_BIG_ENDIAN|ICPL_LITTLE_ENDIAN ].*/
S32	ICPL_CodeMode()
{
	U32 tmp = 0x12345678;
	U8 *upoint = (U8 *)&tmp;
	if(upoint[0] == 0x12)
	{
		return ICPL_BIG_ENDIAN;
	}
	else
	{
		return ICPL_LITTLE_ENDIAN;
	}
}


/**	初始化内部模块内存.*/
void ICPL_CodeInit()
{
	codeMode = ICPL_CodeMode();
	HWL_DEBUG("Code Mode:%s\n", codeMode == ICPL_BIG_ENDIAN ? "Big Endian" : "Little Endian" );
}


void ICPL_ByteReverse(U8 *beg , U32 size)
{
	U32 i;
	for(i = 0; i < size / 2 ; i++)
	{
		U8 tmp = beg[i];
		beg[i] = beg[size-1-i];
		beg[size-1-i] = tmp;
	}
}


/** FPGA程序的所有整形表示为高字节在前(低地址)，即为大端法。*/

void ICPL_FpgaByteToHostByte(U8 *dest, U8 *src, U32 size)
{
	if(codeMode == ICPL_LITTLE_ENDIAN)
	{
		ICPL_ByteReverse(src, size);
	}

	GLOBAL_MEMCPY(dest, src, size);
}


void ICPL_HostByteToFpgaByte(U8 *dest, U8 *src, U32 size)
{
	if(codeMode == ICPL_LITTLE_ENDIAN)
	{
		ICPL_ByteReverse(src, size);
	}

	GLOBAL_MEMCPY(dest, src, size);
}





/**	解码.*/
void ICPL_ByteInfoDecode(ICPL_ByteInfo_t  *byteInfo, U32 size , U8 *buff, U32 buffLen)
{
	U32 i;
	U8 tmp;
	U32 count = 0;
	for(i = 0; i < size; i++)
	{
		ICPL_ByteInfo_t *info = byteInfo + i;

		count += info->m_DataSize;

		assert((info->address) != NULL && (info->m_StartIndex) >= 0 && (info->m_DataSize >= 0));

		switch(info->mode)
		{

		case ICPL_CODE_MODE_U8:
			*(U8 *)(info->address) = *(U8 *)(buff + info->m_StartIndex);
			break;
		case ICPL_CODE_MODE_COPY:
			GLOBAL_MEMCPY(info->address, buff + info->m_StartIndex, info->m_DataSize);
			break;
		case ICPL_CODE_MODE_U24:
			tmp = buff[info->m_StartIndex-1 ];
			buff[info->m_StartIndex-1 ] = 0;
			ICPL_FpgaByteToHostByte(info->address, buff + info->m_StartIndex-1, 4);
			buff[info->m_StartIndex-1 ] = tmp;
			break;
		case ICPL_CODE_MODE_U16:
			// ICPL_HostByteToFpgaByte(info->address, buff + info->m_StartIndex, 2);
			ICPL_FpgaByteToHostByte(info->address, buff + info->m_StartIndex, 2);
			break;
		case ICPL_CODE_MODE_U32:
			ICPL_FpgaByteToHostByte(info->address, buff + info->m_StartIndex, 4);
			break;
		default:
			assert(0);

		}
	}

	//assert(count<=buffLen);
}



/**	编码.*/
void ICPL_ByteInfoEncode(ICPL_ByteInfo_t  *byteInfo, U32 size, U8 *buff, U32 buffLen )
{
	U32 i;
	U8 tmp;
	U32 count = 0;

	assert(byteInfo != NULL);
	assert(buff != NULL);

	for(i = 0; i < size; i++)
	{
		ICPL_ByteInfo_t *info = byteInfo + i;
		count += info->m_DataSize;

		assert((info->address) != NULL && (info->m_StartIndex) >= 0 && (info->m_DataSize >= 0));

		switch(info->mode)
		{

		case ICPL_CODE_MODE_U8:
			*(U8 *)(buff + info->m_StartIndex) = *(U8 *)(info->address);
			break;
		case ICPL_CODE_MODE_COPY:
			GLOBAL_MEMCPY(buff + info->m_StartIndex, info->address, info->m_DataSize);
			break;
		case ICPL_CODE_MODE_U24:
			tmp = buff[info->m_StartIndex+info->m_DataSize ];
			ICPL_HostByteToFpgaByte(buff + info->m_StartIndex, info->address, 4);
			buff[info->m_StartIndex+info->m_DataSize ] = tmp;
			break;
		case ICPL_CODE_MODE_U16:
			ICPL_HostByteToFpgaByte(buff + info->m_StartIndex, info->address, 2);
			break;
		case ICPL_CODE_MODE_U32:
			ICPL_HostByteToFpgaByte(buff + info->m_StartIndex, info->address, 4);
			break;
		default:
			assert(0);
		}
	}

	assert(count <= buffLen);

}






#define ICPL_MIN(a,b) 		((a)<(b)?(a):(b))


#ifdef DEBUG

#define __ICPL_CmdTestTagId(cmd,tagID)  	do { \
	assert(cmd!=NULL)  ;\
	assert(cmd->buff!=NULL) ;\
	assert(cmd->buff[ICPL_CMD_IDX_TAGID] == tagID);	\
} while(0)

#else

#define __ICPL_CmdTestTagId(cmd,tagID)


#endif


/**	心跳回复操作解包函数组. */
S32 ICPL_OPtorHeatBeatParser(ICPL_CmdResponse_t  *cmd, HWL_HeatBeat_t *headbeat)
{
#if defined(MULT_DEVICE_OLD_HW_INFO_AND_HEAR_BEAT)
	ICPL_ByteInfo_t ByteArray[] =
	{
		{&headbeat->InTSRate, 4, 4, ICPL_CODE_MODE_U32 },
		{&headbeat->OutTsRate, 8, 4, ICPL_CODE_MODE_U32 },
	};
#elif defined(GN1846)
	ICPL_ByteInfo_t ByteArray[] =
	{
		{&headbeat->temperature, 7, 2, ICPL_CODE_MODE_U16}
	};
#else
	ICPL_ByteInfo_t ByteArray[] =
	{
		{&headbeat->InTSRate, 5, 3, ICPL_CODE_MODE_U24 },
		{&headbeat->OutTsRate, 9, 3, ICPL_CODE_MODE_U24 },
		{&headbeat->resetWord, 12, 4, ICPL_CODE_MODE_U32},
		{&headbeat->readError, 16, 1, ICPL_CODE_MODE_U8},
		{&headbeat->writeError, 17, 1, ICPL_CODE_MODE_U8},
		{&headbeat->psiNumber, 18, 1, ICPL_CODE_MODE_U8},
		{&headbeat->buffFlag, 19, 1, ICPL_CODE_MODE_U8},
		{&headbeat->temperature, 22, 2, ICPL_CODE_MODE_U16},
#ifdef GQ3760B
		{&headbeat->chipPll, 24, 4, ICPL_CODE_MODE_U32},
#else
		{&headbeat->shortStatus, 20, 2, ICPL_CODE_MODE_U16},
		{&headbeat->chipPll, 24, 4, ICPL_CODE_MODE_U32},
#endif

	};
#endif

	//__ICPL_CmdTestTagId(cmd, ICPL_TAGID_HEADBEAT );

	//if(cmd->buff[ICPL_CMD_IDX_LENGTH] != 0x08)
	//{
	//    HWL_DEBUG("sorry wrong length:0x%x\n", cmd->buff[ICPL_CMD_IDX_LENGTH]);
	//    return ERROR_BAD_PARAM;
	//}

	//CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, cmd->buff, cmd->buff_len);

	ICPL_ByteInfoDecode(ByteArray, sizeof(ByteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);

#ifndef GN1846
	headbeat->psiNumber = headbeat->psiNumber * 64;//协议要求
	headbeat->InTSRate= headbeat->InTSRate*188*8 ;
	headbeat->OutTsRate= headbeat->OutTsRate *188*8;

	headbeat->chipPll&=0x0FFFFFFF;
#endif

#ifdef DEBUG
	HWL_HeatBeatShow(headbeat);
#endif


	return SUCCESS;
}



/**	 硬件通道状态查询应答 (FPGA ' CPU)  TAG = 0x0c.*/
S32 ICPL_OPtorPhyRskResponseParser(ICPL_CmdRequest_t  *cmd, HWL_PhyStatusRskResponse_t *phyRsk)
{

	U32 i;
	U32 size;
	U8 *buffIdx;
	U16 tmp, lUnused;
	HWL_PhyStatusRsk_t phyRskItem;


#if defined(MULT_DEVICE_OLD_HW_INFO_AND_HEAR_BEAT)

	ICPL_ByteInfo_t ChipId 		= { &phyRsk->chipSn, 4, 4, ICPL_CODE_MODE_U32 };
	ICPL_ByteInfo_t DeviceType 	= { &lUnused, 8, 1, ICPL_CODE_MODE_U8 };
	ICPL_ByteInfo_t HardVer	= { &lUnused, 9, 1, ICPL_CODE_MODE_U8 };
	ICPL_ByteInfo_t FpgaVer	= { &tmp, 10, 2, ICPL_CODE_MODE_U16 };

	ICPL_ByteInfoDecode(&ChipId, 1, cmd->buff, 4);
	ICPL_ByteInfoDecode(&DeviceType, 1, cmd->buff, 1);
	ICPL_ByteInfoDecode(&HardVer, 1, cmd->buff, 1);
	ICPL_ByteInfoDecode(&FpgaVer, 1, cmd->buff, 2);

	phyRsk->fpga.day = (U8)tmp & 0x1F ;
	phyRsk->fpga.month = (U8)(tmp >> 5) & 0x0F;
	phyRsk->fpga.year = (U8)(tmp >> 9) & 0x0F ;
	phyRsk->fpga.year += 2008; 


	phyRsk->PhyStatusRskTable.tableSize = 1;

#elif defined(GN1846)
	ICPL_ByteInfo_t ChipId 		= { &phyRsk->chipSn, 3, 4, ICPL_CODE_MODE_U32 };
	ICPL_ByteInfo_t FpgaVer	= { &tmp, 1, 2, ICPL_CODE_MODE_U16 };

	ICPL_ByteInfoDecode(&ChipId, 1, cmd->buff, 4);
	ICPL_ByteInfoDecode(&FpgaVer, 1, cmd->buff, 2);

	phyRsk->fpga.day = (U8)tmp & 0x1F ;
	phyRsk->fpga.month = (U8)(tmp >> 5) & 0x0F;
	phyRsk->fpga.year = (U8)(tmp >> 9) & 0x0F ;
	phyRsk->fpga.year += 2008; 

	phyRsk->PhyStatusRskTable.tableSize = 1;
#else

	ICPL_ByteInfo_t ByteArray[] =
	{
		{&phyRskItem.physicChannelId, 0, 1, ICPL_CODE_MODE_U8},
		{&phyRskItem.inOrOut, 1, 1, ICPL_CODE_MODE_U8},
		{&phyRskItem.maxLogicChannelNum, 2, 2, ICPL_CODE_MODE_U16},
	};

	HWL_DEBUG("...\n");

	if(cmd->buff[ICPL_CMD_IDX_LENGTH] != 0x07 && cmd->buff[ICPL_CMD_IDX_LENGTH] != 0x04   )
	{
		HWL_DEBUG("sorry wrong length:0x%02x\n", cmd->buff[ICPL_CMD_IDX_LENGTH]);
		return ERROR_BAD_PARAM;
	}

	buffIdx = cmd->buff + 4 ;

	phyRsk->chipSn = 0x0;

	size = 0;
	for(i = 0 ; i < cmd->buff[ICPL_CMD_IDX_LENGTH]  ; i++)
	{

		ICPL_ByteInfoDecode(ByteArray, sizeof(ByteArray) / sizeof(ICPL_ByteInfo_t),
			buffIdx, 4);

		switch(phyRskItem.physicChannelId)
		{
		case 0x80:
			//fpga version.
			phyRsk->fpga.v = phyRskItem.inOrOut;
			tmp = (phyRskItem.maxLogicChannelNum & 0x1E00) >> 9 ;
			HWL_DEBUG("maxLogicChannelNum:0x%04x tmp:0x%04x\n", phyRskItem.maxLogicChannelNum, tmp);
			phyRsk->fpga.year = (tmp) + 2008;
			phyRsk->fpga.month = (phyRskItem.maxLogicChannelNum & 0x1E0) >> 5 ;
			phyRsk->fpga.day = (phyRskItem.maxLogicChannelNum & 0X1F);

			break;

		case 0x81:
			//tongcy logo.....
			break;

			//ID号
		case 0x6:
			phyRsk->chipSn |= phyRskItem.maxLogicChannelNum << 16 ;
			break;

		case 0x7:
			phyRsk->chipSn |= phyRskItem.maxLogicChannelNum;
			break;

		case 0x08:
			{
				if (phyRskItem.inOrOut == 0x07)
				{

				}
				else if (phyRskItem.inOrOut == 0x01)
				{
					//E3DS3
					phyRsk->PhyStatusRskTable.table[size].inOrOut = 0;
					phyRsk->PhyStatusRskTable.table[size].maxLogicChannelNum = 4;
					phyRsk->PhyStatusRskTable.table[size].physicChannelId = HWL_CHANNEL_TYPE_E3DS3;
					size++;
				}
				else if (phyRskItem.inOrOut == 0x05)
				{
					phyRsk->PhyStatusRskTable.table[size].inOrOut = 0;
					phyRsk->PhyStatusRskTable.table[size].maxLogicChannelNum = 4;
					phyRsk->PhyStatusRskTable.table[size].physicChannelId = HWL_CHANNEL_TYPE_TUNER_C;
					size++;
				}
				else if (phyRskItem.inOrOut == 0x04)
				{
					phyRsk->PhyStatusRskTable.table[size].inOrOut = 0;
					phyRsk->PhyStatusRskTable.table[size].maxLogicChannelNum = 4;
					phyRsk->PhyStatusRskTable.table[size].physicChannelId = HWL_CHANNEL_TYPE_TUNER_S;
					size++;
				}
#if defined(GQ3760B)//获取PLL时钟的类型：
				else if (phyRskItem.inOrOut == 0x02)
				{
					GLOBAL_TRACE(("40M Mark = %.8X!!!!!!!!!!!!!!!\n", phyRskItem.maxLogicChannelNum));
					phyRsk->PhyStatusRskTable.table[size].physicChannelId = HWL_CHANNEL_TYPE_CLK_TYPE;
					if ((phyRskItem.maxLogicChannelNum & 0x01) > 0)
					{
						phyRsk->PhyStatusRskTable.table[size].inOrOut = 1;
						//40M
					}
					else
					{
						phyRsk->PhyStatusRskTable.table[size].inOrOut = 0;
						//10M
					}
					size++;
				}
#endif
			}
			break;

		default:

			if(size >= HWL_CONST_CHANNEL_NUM_MAX)
			{
				continue;
			}

			//GLOBAL_TRACE(("type = %d, sub chn = %d\n", phyRskItem.physicChannelId, phyRskItem.maxLogicChannelNum));

			phyRsk->PhyStatusRskTable.table[size] = phyRskItem;
			size++;
			break;

		}
		buffIdx += 4;
	}

	phyRsk->PhyStatusRskTable.tableSize = size;


#ifdef DEBUG
	HWL_PhyStatusRskResponseShow(phyRsk);
#endif

#endif

	return SUCCESS;
}




/**	10.2.A 道输入码率查询应答 (FPGA ' CPU)  TAG = 0x02.*/
S32 ICPL_OPtorChannelRateParser(ICPL_CmdRequest_t  *cmd, HWL_ChannelRateArray_t *__param)
{
	U32	i;
	U8 *buffIdx;
	U32 bufflen;
	HWL_ChannelRateArray_t param;
	U16 total;
	U16 logicChannelId,logicChannelRate;



	ICPL_ByteInfo_t channelRateByteArray[] =
	{
		{&logicChannelId, 0, 2, ICPL_CODE_MODE_U16},
		{&logicChannelRate, 2, 2, ICPL_CODE_MODE_U16},
	};

	ICPL_ByteInfo_t headByteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&param.requestType, 2, 1, ICPL_CODE_MODE_U8},
		{&param.physicChannelId, 3, 1, ICPL_CODE_MODE_U8},
		{&total, 6, 2, ICPL_CODE_MODE_U16},
	};

	//CAL_PrintDataBlock(__FUNCTION__, cmd->buff, (cmd->buff[1] + 1) * 4);

	ICPL_ByteInfoDecode(headByteArray , sizeof(headByteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);


	param.channelRateArraySize = cmd->length - 1;
	param.channelRateArraySize = ICPL_MIN (param.channelRateArraySize, HWL_CONST_SUBCHN_NUM_MAX);

	buffIdx = cmd->buff + 8;
	bufflen = 4;

	for(i = 0; i < param.channelRateArraySize ; i++)
	{

		ICPL_ByteInfoDecode(channelRateByteArray , sizeof(channelRateByteArray) / sizeof(ICPL_ByteInfo_t), buffIdx, 4);
		param.channelRateArray[i].logicChannelRate =logicChannelRate * 188 * 8 * 2;
		param.channelRateArray[i].logicChannelId = logicChannelId;
#ifdef PRINTF_CHANNEL_STATISTICS
		GLOBAL_TRACE(("i = %d, id = %d, rate = %d\n", i, param.channelRateArray[i].logicChannelId, param.channelRateArray[i].logicChannelRate));
#endif
		buffIdx += 4;
	}

	param.totalRate=total*188*8;

#ifdef PRINTF_CHANNEL_STATISTICS
	GLOBAL_TRACE(("CHN = %d, Total = %d, RequestType = %d\n", param.physicChannelId, param.totalRate, param.requestType));
#endif

	switch(param.requestType)
	{
	case 0x01:
		__param[0] = param;
		break;
	case 0x02:
		__param[1] = param;
		break;	
	case 0x03:
		__param[2] = param;
		break;	
	}

	return SUCCESS;
}


/*这个函数得同时分析PID统计和IP统计*/
S32 ICPL_OPtorStatisticsArrayParser(ICPL_CmdResponse_t  *cmd, HWL_PIDETHStatistcs *param)
{
	U32 i;
	U8 *buffIdx;
	U32 bufflen;
	U8 lStatisticsMark;
	U32 lChnIndex;
	U16 lTotalRate;

	ICPL_ByteInfo_t headByteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&lStatisticsMark, 2, 1, ICPL_CODE_MODE_U8},
		{&lChnIndex, 3, 1, ICPL_CODE_MODE_U8},
		{&lTotalRate, 6, 2, ICPL_CODE_MODE_U16},
	};

	ICPL_ByteInfoDecode(headByteArray , sizeof(headByteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);

	if (lStatisticsMark == 0x01)//PID
	{
		U16 lTmpPID, lTmpBitrate;
		HWL_PidRate_t *plRate;
		HWL_PIDStatisticsArray *plPIDArray;


		ICPL_ByteInfo_t pidRByteArray[] =
		{
			{&lTmpPID, 0, 2, ICPL_CODE_MODE_U16},
			{&lTmpBitrate, 2, 2, ICPL_CODE_MODE_U16},
		};

		plPIDArray = &param->m_PIDArray;

		plPIDArray->physicChannelId = lChnIndex;
		plPIDArray->totalRate = lTotalRate * MPEG2_TS_PACKET_SIZE * 8 * 2;

		plPIDArray->channelRateArraySize = cmd->length - 1;

		if (plPIDArray->channelRateArraySize <= HWL_CONST_PID_NUM_MAX)
		{
			buffIdx = cmd->buff + 8;

			for(i = 0; i < plPIDArray->channelRateArraySize; i++)
			{
				ICPL_ByteInfoDecode(pidRByteArray , sizeof(pidRByteArray) / sizeof(ICPL_ByteInfo_t), buffIdx, bufflen);
				plRate = &plPIDArray->pidRateArray[i];
				plRate->pidRate = lTmpBitrate * MPEG2_TS_PACKET_SIZE * 8 * 2;
				plRate->pidVal = lTmpPID;
				buffIdx += 4;
			}
		}
		else
		{
			plPIDArray->channelRateArraySize = 0;
		}

	}
	else
	{
		S32 lFramIndex;
		U16 lTmpPortRate;
		U32 lTmpIPv4Addr;
		U16 lTmpPortNum;
		HWL_EthDetectionInfo *plIPInfo;;
		HWL_IPStatisticsArray *plIPArray;

		ICPL_ByteInfo_t pidRByteArray[] =
		{
			{&lTmpIPv4Addr , 0, 4, ICPL_CODE_MODE_U32},
			{&lTmpPortNum, 4, 2, ICPL_CODE_MODE_U16},
			{&lTmpPortRate, 6, 2, ICPL_CODE_MODE_U16},
		};

		if (GLOBAL_CHECK_INDEX(lChnIndex, HWL_CONST_CHANNEL_NUM_MAX))
		{
			S32 lFrmeSize, lCurArraySize;
			plIPArray = &param->m_IPArray;

			lFramIndex = lStatisticsMark - 0x10;

			GLOBAL_TRACE(("Frame Index = %d\n", lFramIndex));
			if (lFramIndex < HWL_CONST_IP_STATISTICS_FRAME_NUM_MAX)
			{
				plIPArray->totalRate = lTotalRate;
				lCurArraySize = (cmd->length - 1) / 2;
				GLOBAL_TRACE(("Frame Index = %d, CurSize = %d\n", lFramIndex, lCurArraySize));
				if (lCurArraySize <= HWL_CONST_IP_STATISTICS_FRAME_ITEM_MAX)
				{
					buffIdx = cmd->buff + 8;

					for(i = 0; i < lCurArraySize; i++)
					{
						ICPL_ByteInfoDecode(pidRByteArray , sizeof(pidRByteArray) / sizeof(ICPL_ByteInfo_t), buffIdx, bufflen);

						plIPInfo = &plIPArray->ipPortInfoArray[i + lFramIndex * HWL_CONST_IP_STATISTICS_FRAME_ITEM_MAX];
						plIPInfo->addressv4 = lTmpIPv4Addr;
						plIPInfo->portNumber = lTmpPortNum;
						plIPInfo->portRate = lTmpPortRate * MPEG2_TS_PACKET_SIZE * 8 * 2;
						GLOBAL_TRACE(("IP[%.3d] 0x%.8X:%d %d kbps\n", i, plIPInfo->addressv4, plIPInfo->portNumber, plIPInfo->portRate / 1024));
						buffIdx += 8;
					}

					plIPArray->ArrySize = lCurArraySize + lFramIndex * HWL_CONST_IP_STATISTICS_FRAME_ITEM_MAX;
				}
			}
		}
		else
		{
			//GLOBAL_TRACE(("Error! ChnIndex = %d\n", lChnIndex));
		}
	}

	return SUCCESS;
}



/**	PSI接受Buffer大小  （FPGA-）CPU） 定时发送：35ms .*/
S32 ICPL_OPtorPSIBuffSizeParser(ICPL_CmdRequest_t  *cmd, HWL_PSIBuffSize_t *psiBuff)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&psiBuff->stableTsPackgeNum, 2, 1, ICPL_CODE_MODE_U8},
		{&psiBuff->immediateTsPackgeNum, 3, 1, ICPL_CODE_MODE_U8},
	};

	ICPL_ByteInfoDecode(byteArray , sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);


#ifdef DEBUG
	HWL_PSIBuffSizeShow(psiBuff);
#endif

	if (!cmd->length)
	{
		GLOBAL_TRACE(("length = 0\n"));
	}

	//     assert(== 0x0);

	return SUCCESS;
}

/**	输入流减少时FPGA报告POWER PC  .*/
S32 ICPL_OPtorInputStreamNotifierTableParser(ICPL_CmdResponse_t  *cmd, HWL_InputStreamNotifierTable_t *table)
{
	return SUCCESS;
}


/**	21	输入端口号节目的内容有变化.*/
S32 ICPL_OPtorInputPortEsNotifierTableParser(ICPL_CmdResponse_t  *cmd, HWL_InputPortEsNotifierTable_t *table)
{
	U32 i;
	U8 *buffIdx;
	HWL_InputPortEsNotifier_t esNotifier;


	ICPL_ByteInfo_t byteHeadArray[] =
	{
		{&table->physicChannel, 3, 1, ICPL_CODE_MODE_U8},
		{&table->streamIdentifier.udpPort, 4, 2, ICPL_CODE_MODE_U16 },
		{&table->streamIdentifier.channelId, 6, 1, ICPL_CODE_MODE_U8 },
		{&table->streamIdentifier.program, 7, 1, ICPL_CODE_MODE_U8 },
	};


	ICPL_ByteInfo_t byteNotifierArray[] =
	{
		{&esNotifier.idx, 0, 1, ICPL_CODE_MODE_U8},
		{&esNotifier.esType, 1, 2, ICPL_CODE_MODE_U16},
		{&esNotifier.esPid, 3, 1, ICPL_CODE_MODE_U8},
	};


	ICPL_ByteInfoDecode(byteHeadArray , sizeof(byteHeadArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	// 	assert(cmd->length == 0x10);
	if (cmd->length == 0x10)
	{
		table->esTableSize = cmd->length - 1;
		buffIdx = cmd->buff + 4;

		for(i = 0; i < table->esTableSize; i++)
		{

			ICPL_ByteInfoDecode(byteNotifierArray , sizeof(byteNotifierArray) / sizeof(ICPL_ByteInfo_t),
				cmd->buff, cmd->buff_len);
			table->esTable[i] = esNotifier;

			buffIdx += 4;
		}
	}
	else
	{
		GLOBAL_TRACE(("length != 0x10\n"));
	}


	return SUCCESS;
}


/* 26.1	与加密芯片的通讯.接收. */
S32 ICPL_OPtorEncryptChipParser(ICPL_CmdRequest_t  *cmd, HWL_EncryptChip_t *encryptChip)
{
	static U8	buff[HWL_CONST_ENCRYPT_MAX_BUFF];
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8 },
		{&encryptChip->mainNo, 5, 1, ICPL_CODE_MODE_U8},
		{&encryptChip->subNo, 6, 1, ICPL_CODE_MODE_U8},
		{&encryptChip->length, 7, 1, ICPL_CODE_MODE_U8},
		{encryptChip->buff, 8, 10, ICPL_CODE_MODE_COPY},
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_ENCRYPT);

	ICPL_ByteInfoDecode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);

	// 	CAL_PrintDataBlock(__FUNCTION__, encryptChip->buff, encryptChip->length);

	// 	assert(ICPL_CheckRange(0, HWL_CONST_ENCRYPT_MAX_BUFF , encryptChip->length) == ICPL_CheckOk);

	return SUCCESS;
}


/*****************************************************************************************************/
/** 请求操作打包函数组.	*/
/*****************************************************************************************************/


S32 ICPL_OPtorHeatBeatMake(ICPL_CmdResponse_t  *cmd, HWL_HeatBeat_t *headbeat)
{
	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_HEADBEAT);
	cmd->buff[ICPL_CMD_IDX_LENGTH] = 0X0;
	return SUCCESS;
}



/**	2.向DPB板进行TS插入188数据(CPU-〉FPGA)  TAG = 0x0a .*/
S32 ICPL_OPtorDPBTsInsertMake(ICPL_CmdRequest_t  *cmd, HWL_DPBTsInsert_t *param)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},			//指定长度，
		{&param->physicChannelId, 3, 1, ICPL_CODE_MODE_U8 },
		{&param->logicChannelId, 4, 2, ICPL_CODE_MODE_U16 },
		{param->tsDataBuff, 8, 188, ICPL_CODE_MODE_COPY },
	};

	HWL_DEBUG("...\n");

	assert(param != NULL);
	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_DPB_TS);
	cmd->length = 0x30;
	param->physicChannelId = 0x0;		//

	memset(cmd->buff+2,0,6);

	/**	长度必须为188.*/
	if(param->tsDataBuffSize != 188)
	{
		GLOBAL_TRACE(("Ts Len = %d\n", param->tsDataBuffSize));
		assert(param->tsDataBuffSize == 188);
		return ERROR_BAD_PARAM;
	}

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);

	return SUCCESS;
}



/**	4. 输出TS 参数设置 (CPU-〉FPGA)  TAG = 0x04.*/
S32 ICPL_OPtorOutTsParamMake(ICPL_CmdResponse_t  *cmd, HWL_OutPutTsParam_t *param)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 2, 1, ICPL_CODE_MODE_U8},
		{&param->physicChannelId, 3, 1, ICPL_CODE_MODE_U8},
		{&param->outputChannelId, 4, 2, ICPL_CODE_MODE_U16},
		{&param->portNo, 6, 2, ICPL_CODE_MODE_U16},
		{&param->ipVersion, 11, 1, ICPL_CODE_MODE_U8} ,
		{&param->addressv4, 12, 4, ICPL_CODE_MODE_COPY},
		{&param->addressv6, 18, 6, ICPL_CODE_MODE_COPY},
	};

	assert(param != NULL);

	assert(param->ipVersion == HWL_CONST__IPV4
		|| param->ipVersion == HWL_CONST__IPV6);

	if(!(param->ipVersion == HWL_CONST__IPV4
		|| param->ipVersion == HWL_CONST__IPV6) )
	{
		return ERROR_BAD_PARAM;
	}

	/**	（目前只认IPV4）.*/
	assert(param->ipVersion == HWL_CONST__IPV4);

	//check ..TS_Index : 输出TS的逻辑编号。0..15
	assert(ICPL_CheckRange(0, 15, param->outputChannelId) == ICPL_CheckOk);
	if(ICPL_CheckRange(0, 15, param->outputChannelId) == ICPL_CheckNo)
	{
		return ERROR_BAD_PARAM;
	}

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_OUTTSPARAM);

	cmd->length = 0x05;
	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}


/**	7  输入TS的UDP端口的 参数设置 (CPU-〉FPGA)  TAG = 0x03.*/
S32 ICPL_OPtorInputUdpPortMake(ICPL_CmdResponse_t  *cmd, HWL_InputUdpPort_t *param)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 2, 1, ICPL_CODE_MODE_U8},
		{&param->physicChannelId, 3, 1, ICPL_CODE_MODE_U8},
		{&param->inputLogicChannelId, 4, 2, ICPL_CODE_MODE_U16},
		{&param->portNumber, 6, 2, ICPL_CODE_MODE_U16},
		{&param->ipVersion, 11, 1, ICPL_CODE_MODE_U8},
		{&param->destinedAddress.v4, 12, 4, ICPL_CODE_MODE_COPY},
		{&param->originalAddress.v4, 16, 4, ICPL_CODE_MODE_COPY},
	};

	assert(param != NULL);

	//check ip verison please.
	assert(param->ipVersion == HWL_CONST__IPV4
		|| param->ipVersion == HWL_CONST__IPV6);
	if(!(param->ipVersion == HWL_CONST__IPV4
		|| param->ipVersion == HWL_CONST__IPV6) )
	{
		return ERROR_BAD_PARAM;
	}

	/**	（目前只认IPV4）.*/
	assert(param->ipVersion == HWL_CONST__IPV4);


	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_ETH_CHN_PARAM );

	/**	TS_Index(16bits) : 输入TS的逻辑通道编号。0..249  .*/
	assert(ICPL_CheckRange(0, 249, param->inputLogicChannelId) == ICPL_CheckOk);
	if(ICPL_CheckRange(0, 249, param->inputLogicChannelId) == ICPL_CheckNo)
	{
		return ERROR_BAD_PARAM;
	}


	cmd->length = 0x05;
	param->physicChannelId = 0x0;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}


/* 	10. 硬件通道状态查询 */
S32 ICPL_OPtorPhyQskMake(ICPL_CmdRequest_t  *cmd, HWL_PhyStatusQsk_t *phyQsk)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 2, 1, ICPL_CODE_MODE_U8},
		{&phyQsk->requestType, 2, 1, ICPL_CODE_MODE_U8 },
		{&phyQsk->physicChannelId, 3, 1, ICPL_CODE_MODE_U8 },
	};

	cmd->length = 0x0 ;
	phyQsk->requestType = 0x0;
	phyQsk->physicChannelId = 0X0;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}


/**	10.2  有索引号的通道按索引号方式的输入|输出码率查询(CPU-〉FPGA)  TAG = 0x02.*/
S32 ICPL_OPtorChannelRateInfoMake(ICPL_CmdRequest_t  *cmd, HWL_ChannelRateInfo_t *param)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 2, 1, ICPL_CODE_MODE_U8},
		{&param->requestType, 2, 1, ICPL_CODE_MODE_U8 },
		{&param->physicChannelId, 3, 1, ICPL_CODE_MODE_U8 },
	};
	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_CHANNELRATE );
	assert(param != NULL);
	cmd->length = 0x0;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);


	return SUCCESS;
}


/* 	11.1 PID统计的端口设置 */
S32 ICPL_OPtorPidStatisticMake(ICPL_CmdRequest_t  *cmd, HWL_PidStatics_t *pidSearch)
{
#if 0
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},

		{&pidSearch->physicChannelId, 5, 1, ICPL_CODE_MODE_U8 },
		{&pidSearch->logicChannelId, 6, 2, ICPL_CODE_MODE_U16 },
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDSTATICS );
	assert(pidSearch != NULL);

	cmd->length = 0x01;

	GLOBAL_TRACE(("Chn Index = %d, Sub Index = %d\n", pidSearch->physicChannelId, pidSearch->logicChannelId));

	GLOBAL_MEMSET(cmd->buff + 4, 0, 4);
	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);
#else

	U8 lChnValue;
	U16 lSubValue;

	//if (pidSearch->physicChannelId == 0)
	//{
	//	lChnValue = 0;
	//	lSubValue = pidSearch->logicChannelId;
	//}
	//else if (pidSearch->physicChannelId == 1)
	//{
	//	lChnValue = 1;
	//	lSubValue = pidSearch->logicChannelId;
	//}
	//else if (pidSearch->physicChannelId == 2)
	//{
	//	lChnValue = 0;
	//	lSubValue = pidSearch->logicChannelId + 8;
	//}

	lChnValue = pidSearch->physicChannelId;
	lSubValue = pidSearch->logicChannelId;

	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},

		{&lChnValue, 3, 1, ICPL_CODE_MODE_U8 },
		{&lSubValue, 6, 2, ICPL_CODE_MODE_U16 },
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDSTATICS );
	assert(pidSearch != NULL);

	cmd->length = 0x01;

	GLOBAL_MEMSET(cmd->buff + 4, 0, 4);
	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);
#endif







	//cmd->buff[2] = 0x01;//固定值

	return SUCCESS;
}


/**	PID 统计的端口码率查询请求*/
S32 ICPL_OPtorPidStatisticSearchMake(ICPL_CmdRequest_t  *cmd, HWL_PidStatics_t *pidSearch)
{
#if 0
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDSTATICS );
	assert(pidSearch != NULL);
	cmd->length = 0x0;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);

	cmd->buff[2] = 0x01;//固定值
#else
	U8 lChnValue;

	//if (pidSearch->physicChannelId == 0)
	//{
	//	lChnValue = 0;
	//}
	//else if (pidSearch->physicChannelId == 1)
	//{
	//	lChnValue = 1;
	//}
	//else if (pidSearch->physicChannelId == 2)
	//{
	//	lChnValue = 0;
	//}

	lChnValue = pidSearch->physicChannelId;

	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&lChnValue, 3, 1, ICPL_CODE_MODE_U8 },
	};

	assert(pidSearch != NULL);
	cmd->length = 0x0;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);

	cmd->buff[2] = 0x01;//固定值
#endif

	return SUCCESS;
}





/* 	输入IP端口码率统计清除 */
S32 ICPL_OPtorInputIPPortStatisticClearMake(ICPL_CmdRequest_t  *cmd, HWL_EthDetectionParam *statistic)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&statistic->physicChannelId, 3, 1, ICPL_CODE_MODE_U8 },
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDSTATICS );
	assert(statistic != NULL);
	cmd->length = 0x01;
	cmd->buff[2] = 0x10;	//必须的.

	GLOBAL_MEMSET(cmd->buff + 4, 0, 4);

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}


/* 	输入IP端口码率统计查询 */
S32 ICPL_OPtorInputIPPortStatisticSendMake(ICPL_CmdRequest_t  *cmd, HWL_EthDetectionParam *statistic)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&statistic->physicChannelId, 3, 1, ICPL_CODE_MODE_U8 },
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDSTATICS );
	assert(statistic != NULL);
	cmd->length = 0x0;
	cmd->buff[2] = 0x10;	//必须的.
	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}


/**	传送控制字.*/
S32 ICPL_OPtorControlWordMake(ICPL_CmdRequest_t  *cmd,  HWL_ControlWord_t  *cw)
{
	U8	cwn;
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&cwn, 3, 1, ICPL_CODE_MODE_U8},
		{&cw->words, 4, 8, ICPL_CODE_MODE_COPY},	//just copy..
	};

	assert(cw != NULL);
	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_CW_SEND );

	assert(ICPL_CheckRange(0, 127, cw->cwGroup) == ICPL_CheckOk);
	if(ICPL_CheckRange(0, 127, cw->cwGroup) == ICPL_CheckNo)
	{
		return ERROR_BAD_PARAM;
	}

	cmd->length = 0x02;
	cwn = ((cw->evenOrOdd << 7) | cw->cwGroup);
	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}


/**	CW切换.
*	0x07     0x00    Chan_N   Odd_Even
*
*	Chan_N :  0x00 : 不使用
*
*	Odd_Even  : 设置后使用的控制字  ：0使用偶控制字，1使用奇控制字
*
*	ECM 发送切换:
*
*	0x07     0x00   0x10   	ECM_Odd_Even
*						ECM_Odd_Even : bit7..0 expert  CAS channel 8 to 1
*											相应Bit = 0 : 使用偶ECM   = 1使用奇ECM
*	加扰参数的设置还没有编写，一般不变。
*
*/
S32 ICPL_OPtorControlWordSwitchMake(ICPL_CmdRequest_t  *cmd,  HWL_ControlWord_t  *cw)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&cw->channelId, 2, 1, ICPL_CODE_MODE_U8},
		{&cw->evenOrOdd, 3, 1, ICPL_CODE_MODE_U8},
	};

	assert(cw != NULL);
	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_CW_SWITCH );

	cmd->length = 0x0;
	cw->channelId = 0x0;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}




/**	设置加扰模块开关.*/
S32 ICPL_OPtorCondtionAccessMake(ICPL_CmdRequest_t  *cmd, HWL_ConditionAccess_t *param)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&param->enabled, 3, 1, ICPL_CODE_MODE_U8},
	};


	assert(param != NULL);
	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_CA_ENABLE );

	assert(param->enabled == HWL_CONST_ON || param->enabled == HWL_CONST_OFF);

	if(!(param->enabled == HWL_CONST_ON || param->enabled == HWL_CONST_OFF))
	{
		return ERROR_BAD_PARAM;
	}


	cmd->length = 0x0;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}


/**	组播的加入与退出 */
S32 ICPL_OPtorGroupBroadcastTableMake(ICPL_CmdRequest_t  *cmd,  HWL_GroupBroadcastTable_t  *groupcast)
{
	U8 phy;
	U32	i;
	U8 *buffIdx;

	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&groupcast->operator, 2, 1, ICPL_CODE_MODE_U8},
		{&phy, 3, 1, ICPL_CODE_MODE_U8},
	};


	//check groupcast operator add or delete ..
	assert(groupcast != NULL);
	assert(groupcast->operator==HWL_CONST_GROUP_BROADCAST_ADD
		|| groupcast->operator==HWL_CONST_GROUP_BROADCAST_DELETE);
	if(!(groupcast->operator==HWL_CONST_GROUP_BROADCAST_ADD
		|| groupcast->operator==HWL_CONST_GROUP_BROADCAST_DELETE))
	{
		return ERROR_BAD_PARAM;
	}

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_GROUP_BROADCAST);
	cmd->length = groupcast->ipaddressSize;


	/*
	*	bit1..0  操作的端口号.。。。（物理端口ID>>>???）
	*	Bit7..4 : 设置主播组的编号: 0x0 : 开始第一组,以后组加一.
	*
	*/

	phy = ((0x3 & groupcast->physicChannelId)  |  (groupcast->groupNo << 4));

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);



	buffIdx = cmd->buff + 4;
	for(i = 0; i < groupcast->ipaddressSize; i++)
	{
		GLOBAL_MEMCPY(buffIdx, &groupcast->ipaddressTable[i].part , 4);
		buffIdx += 4;

	}

	return SUCCESS;
}


/**	 需要COPY的PID源的设置 */
S32 ICPL_OPtorPIDCopyOriginalTableMake(ICPL_CmdRequest_t  *cmd, HWL_PIDCopyOriginalTable_t *pidCopy)
{
	U32 i;
	U8 *buffIdx;


	HWL_PIDSource_t pidsource;
	ICPL_ByteInfo_t byteHeadArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&pidCopy->flag, 2, 1, ICPL_CODE_MODE_U8},
		{&pidCopy->physicChannel, 3, 1, ICPL_CODE_MODE_U8},
	};

	ICPL_ByteInfo_t byteSourceArray[] =
	{
		{&pidsource.originalChannelID , 0, 2, ICPL_CODE_MODE_U16 },
		{&pidsource.originalPid , 2, 2, ICPL_CODE_MODE_U16},
		{&pidsource.destinedPidOutListOffset, 4, 2, ICPL_CODE_MODE_U16},
		{&pidsource.destinedPidOutListNum, 6, 2, ICPL_CODE_MODE_U16},
	};


#ifdef DEBUG_SHOW_PIDMAP
	//HWL_PIDCopyOriginalTableShow(pidCopy);
#endif

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDCOPY_ORIGINAL);

	cmd->length = (pidCopy->pidsourceSize * 2);

	//pidCopy->physicChannel=0x0;

	ICPL_ByteInfoEncode(byteHeadArray, sizeof(byteHeadArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);


	buffIdx = cmd->buff + 4;
	for(i = 0; i < pidCopy->pidsourceSize ; i++)
	{
		U16	 tmp;
		pidsource = pidCopy->pidsourceTable[i];
		tmp = pidsource.originalPid;
		pidsource.originalPid = tmp & 0x1FFF;	//@Fix: 源PID的高5bit    : bit7..5 : reserved = 000  +  源PID的低8bit


		ICPL_ByteInfoEncode(byteSourceArray, sizeof(byteSourceArray) / sizeof(ICPL_ByteInfo_t),
			buffIdx, 8);

		buffIdx += 8;
	}

#ifdef DEBUG_SHOW_PIDMAP
	CAL_PrintDataBlock(__FUNCTION__, cmd->buff, (cmd->buff[1] + 1) * 4);
#endif

	return SUCCESS;
}



/**	17.2 需要COPY的PID目的PID的设置.*/
static inline U8 __ICPL_PIDCopyDestinedFlag(U32 size)
{
	U8 retval;
	if(size <= 250)
	{
		retval = 0;
	}
	else if(250 < size && size <= 500)
	{
		retval = 1;
	}
	else if(500 < size && size <= 750)
	{
		retval = 2;
	}
	else
	{
		retval = 3;			//@Fix (0 <=x <= 0x1f) :   (x+1) * 250  >=  PID源个数 > (x * 250)
	}

	return retval;
}

/**	需要COPY的PID目的PID的设置 */
S32 ICPL_OPtorPIDCopyDestinedTableMake(ICPL_CmdRequest_t  *cmd,  HWL_PIDCopyDestinedTable_t *pidTable)
{
	U8 flag;
	U32 i;
	U8 *buffIdx;
	HWL_PIDdestined_t piddestined;
	ICPL_ByteInfo_t byteHeadArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&flag, 2, 1, ICPL_CODE_MODE_U8},
		{&pidTable->physicChannel, 3, 1, ICPL_CODE_MODE_U8},
	};


	ICPL_ByteInfo_t bytedestinedArray[] =
	{
		{&piddestined.destinedChannelID, 0, 2, ICPL_CODE_MODE_U16},
		{&piddestined.destinedPid, 2, 2, ICPL_CODE_MODE_U16},
	};


	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDCOPY_DESTINED);

#ifdef DEBUG_SHOW_PIDMAP
	//HWL_PIDCopyDestinedTableShow(pidTable);
#endif

	cmd->length = (pidTable->pidDestinedSize);
	flag = __ICPL_PIDCopyDestinedFlag(pidTable->pidDestinedSize);
	//	pidTable->physicChannel=0x0;

	ICPL_ByteInfoEncode(byteHeadArray, sizeof(byteHeadArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	buffIdx = cmd->buff + 4;
	for(i = 0; i < pidTable->pidDestinedSize ; i++)
	{
		piddestined = pidTable->pidDestinedTable[i];
		ICPL_ByteInfoEncode(bytedestinedArray, sizeof(bytedestinedArray) / sizeof(ICPL_ByteInfo_t),
			buffIdx, 8);
		buffIdx += 4;
	}

	cmd->buff_len = 4 + 4 * pidTable->pidDestinedSize;

#ifdef DEBUG_SHOW_PIDMAP
	CAL_PrintDataBlock(__FUNCTION__, cmd->buff, (cmd->buff[1] + 1) * 4);
#endif

	return SUCCESS;

}





/*	PowerPC对调制器有关的芯片的寄存器设置和读取: (PHY_Ch = 0)	*/
S32 ICPL_OPtorModularChipRegisterTableMake(ICPL_CmdRequest_t  *cmd, HWL_ModularChipRegisterTable_t *registerTable)
{
	HWL_ModularChipRegister_t  regist;
	U8 *buffIdx;
	U32 i;

	ICPL_ByteInfo_t byteHeadArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&registerTable->physicChannel , 3, 1, ICPL_CODE_MODE_U8},
	};

	ICPL_ByteInfo_t byteRegisterArray[] =
	{
		{&regist.chipID, 0, 1, ICPL_CODE_MODE_U8},
		{&regist.rOrw, 1, 1, ICPL_CODE_MODE_U8},
		{&regist.address , 2, 1, ICPL_CODE_MODE_U8},
		{&regist.value, 3, 1, ICPL_CODE_MODE_U8},

	};


	assert( ICPL_CheckRange(0, 250, registerTable->moduleRegisterSize) == ICPL_CheckOk);
	if(  ICPL_CheckRange(0, 250, registerTable->moduleRegisterSize) == ICPL_CheckNo)
	{
		return ERROR_BAD_PARAM;
	}

	/**	chip check...address test ...sorry ....*/


	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_GROUP_MODULOR_REGISTER);
	cmd->length = registerTable->moduleRegisterSize;
	registerTable->physicChannel = 0x0;

	ICPL_ByteInfoEncode(byteHeadArray, sizeof(byteHeadArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	buffIdx = cmd->buff + 4;
	for(i = 0; i < registerTable->moduleRegisterSize; i++)
	{
		regist = registerTable->moduleRegisterTable[i];
		ICPL_ByteInfoEncode(byteRegisterArray, sizeof(byteRegisterArray) / sizeof(ICPL_ByteInfo_t),
			buffIdx, 4);
		buffIdx += 4;
	}

	return SUCCESS;
}




/*	19.CPU对TS输入端口和PCR 校验的MODE 设置: (PHY_Ch = 0) */
S32 ICPL_OPtorChannelInputPcrModeMake(ICPL_CmdRequest_t  *cmd, HWL_ChannelInputPcrMode_t *veryfyMode)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&veryfyMode->physicChannel , 5, 1, ICPL_CODE_MODE_U8},
		{&veryfyMode->verify , 6, 1, ICPL_CODE_MODE_U8 },
		{&veryfyMode->verifyMode, 7, 1, ICPL_CODE_MODE_U8 }
	};



	assert(ICPL_CheckRange(0, 2, veryfyMode->physicChannel) == ICPL_CheckOk);
	if(ICPL_CheckRange(0, 2, veryfyMode->physicChannel) == ICPL_CheckNo)
	{
		return ERROR_BAD_PARAM;
	}

	assert(veryfyMode->verify == HWL_CONST_VERIFY_YES || veryfyMode->verify == HWL_CONST_VERIFY_NO);
	if(!(veryfyMode->verify == HWL_CONST_VERIFY_YES || veryfyMode->verify == HWL_CONST_VERIFY_NO))
	{
		return ERROR_BAD_PARAM;
	}

	assert(veryfyMode->verifyMode == HWL_CONST_PCR_VERIFY_MODE_40MS_INSERT_NO
		|| veryfyMode->verifyMode == HWL_CONST_PCR_VERIFY_MODE_40MS_INSERT_YES) ;
	if(!(veryfyMode->verifyMode == HWL_CONST_PCR_VERIFY_MODE_40MS_INSERT_NO
		|| veryfyMode->verifyMode == HWL_CONST_PCR_VERIFY_MODE_40MS_INSERT_YES))
	{
		return ERROR_BAD_PARAM;
	}



	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PCR_VERYFY );
	cmd->length = -0x01;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);



	return SUCCESS;
}






/* 	27 .设置输出通道和PID的映射方式为输入UDP端口(0..31)直通到输出通道的0..31 */
S32 ICPL_OPtorPassModeMake(ICPL_CmdRequest_t  *cmd, HWL_PassMode_t   *passMode)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&passMode->passMode, 5, 1, ICPL_CODE_MODE_U8},
		{&passMode->outputChanneldId, 6, 1, ICPL_CODE_MODE_U8},
		{&passMode->inputChannelId , 7, 1, ICPL_CODE_MODE_U8},
	};

	//check passMode..
	assert(passMode->passMode == HWL_CONST_PASS_MODE_PIDMAP
		|| passMode->passMode == HWL_CONST_PASS_MODE_THROUGH);

	if(passMode->passMode != HWL_CONST_PASS_MODE_PIDMAP
		&& passMode->passMode != HWL_CONST_PASS_MODE_THROUGH )
	{
		return ERROR_BAD_PARAM;
	}

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PASSMODE );

	GLOBAL_MEMSET(cmd->buff + 4, 0, 4);
	cmd->length = 0x01;

	HWL_DEBUG("2\n");

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);


	return SUCCESS;
}

#if 0//2730S/M/N/X/H系列的直通命令
S32 ICPL_OPtorPassModeMake(ICPL_CmdRequest_t  *cmd, HWL_PassMode_t   *passMode)
{
	U8 tmpPassMode = 0;

	if( passMode->passMode != HWL_CONST_PASS_MODE_PIDMAP
		&& passMode->passMode != HWL_CONST_PASS_MODE_THROUGH  )
	{
		return ERROR_BAD_PARAM;
	}
	tmpPassMode |= passMode->passMode;

	if( passMode->nullPackMark != HWL_CONST_WITHOUT_NULL
		&& passMode->nullPackMark != HWL_CONST_WITH_NULL  )
	{
		return ERROR_BAD_PARAM;
	}
	tmpPassMode |= passMode->nullPackMark<<1;

	if( passMode->pcrVerify != HWL_CONST_PCR_VERIFY_OFF
		&& passMode->pcrVerify != HWL_CONST_PCR_VERIFY_ON )
	{
		return ERROR_BAD_PARAM;
	}
	tmpPassMode |= passMode->pcrVerify<< 2;

	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&tmpPassMode, 5, 1, ICPL_CODE_MODE_U8},
		{&passMode->outputChanneldId, 6, 1, ICPL_CODE_MODE_U8},
		{&passMode->inputChannelId , 7, 1, ICPL_CODE_MODE_U8},
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PASSMODE  );

	GLOBAL_MEMSET(cmd->buff + 4, 0, 4);
	cmd->length = 0x01;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}
#endif



/**	25.1	VOD模式下 PAT版本和CRC32的设置 .*/
S32 ICPL_OPtorVodPatAndCrcSetMake(ICPL_CmdRequest_t  *cmd, HWL_VodPatAndCrcSet_t *param)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&param->outputChannelId, 4, 1, ICPL_CODE_MODE_U8},
		{&param->outputProgram, 5, 1, ICPL_CODE_MODE_U8},
		{&param->patVersion, 7, 1, ICPL_CODE_MODE_U8},
		{&param->patCrc32, 8, 4, ICPL_CODE_MODE_U32},

		{&param->pmtPid, 12, 2, ICPL_CODE_MODE_U16},
		{&param->esNumber, 14, 1, ICPL_CODE_MODE_U8},
		{&param->pmtVersion, 15, 1, ICPL_CODE_MODE_U8},

		{&param->patCrc32, 16, 4, ICPL_CODE_MODE_U32},

	};

	//check ,,e....seem

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_VODPATCRC );

	cmd->length = 0x04;
	GLOBAL_MEMSET(cmd->buff + 4, 0,  cmd->length * 4);

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}

/**
*	Flag7..0 : bite7..4 reserved
*	Bit3 = 1 : ECM packet   0: PSI information
*	Bit2 :  	if bit3 = 1 ,  except  ECM used as ODD or Even   , if  bit3 = 0 : no effect
*	Bit1..0 :  CAS channel Number.   00: channel 1  …  11:channel 4
*
*/
static inline U8 __ICPL_DPBStableFlag(HWL_DPBTsInserter *param)
{
	U8 flag = 0x0;
	if(param->packetType == HWL_CONST_PACKET_PSI)
	{
		flag |= 0x04;
		flag |= param->evenOrOdd << 2;
	}

	flag |= (param->casChannelNo) & 0x3;

	return flag;
}




/* 	29. 向DPB板进行永久TS插入188数据(CPU-〉FPGA)  TAG = 0x23   */
S32 ICPL_OPtorDPBStableTsInsertMake(ICPL_CmdRequest_t  *cmd, HWL_DPBTsInserter *param)
{
	U8 lFixed, flag;
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&lFixed, 4, 1, ICPL_CODE_MODE_U8},
		{&flag, 5, 1, ICPL_CODE_MODE_U8},
		{&param->save, 6, 2, ICPL_CODE_MODE_U16},
		{&param->allNumber, 8, 1, ICPL_CODE_MODE_U8},
		{&param->sendNumber, 9, 1, ICPL_CODE_MODE_U8},
		{&param->timeInterval, 10, 2, ICPL_CODE_MODE_U16},
#if defined(GM2730S) || defined(SUPPORT_NEW_HWL_MODULE)
		{&param->outLogChannel, 14, 2, ICPL_CODE_MODE_U16},
#else
		{&param->outLogChannel, 15, 1, ICPL_CODE_MODE_U8},
#endif
		{param->tsBuff, 16, param->tsBuffLen, ICPL_CODE_MODE_COPY},
	};

	//     // param->evenOrOdd  check.
	// 	assert(param->evenOrOdd == HWL_CONST_EVEN || param->evenOrOdd == HWL_CONST_ODD);
	//     if(!(param->evenOrOdd == HWL_CONST_EVEN || param->evenOrOdd == HWL_CONST_ODD))
	//     {
	//         return ERROR_BAD_PARAM;
	//     }
	// 
	// 
	//     //CAS channel Number.check
	//     assert(ICPL_CheckRange(0, 4, param->casChannelNo) == ICPL_CheckOk);
	//     if(ICPL_CheckRange(0, 4, param->casChannelNo) != ICPL_CheckOk)
	//     {
	//         return ERROR_BAD_PARAM;
	//     }
	// 
	//     //All_N(=N-1, = 0 为1个TS包,最大为31表示32个相同PID的包)
	//     assert(ICPL_CheckRange(0, 31, param->allNumber) == ICPL_CheckOk);
	//     if(ICPL_CheckRange(0, 31, param->allNumber) != ICPL_CheckOk)
	//     {
	//         return ERROR_BAD_PARAM;
	//     }
	// 
	//     //PSI send Interval time (MS)  < 1000ms
	//     assert(ICPL_CheckRange(0, 999, param->timeInterval) == ICPL_CheckOk);
	//     if(ICPL_CheckRange(0, 999, param->timeInterval) != ICPL_CheckOk)
	//     {
	//         return ERROR_BAD_PARAM;
	//     }


	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_STABLE_DPB_TS );
	flag = __ICPL_DPBStableFlag(param);

	// 	GLOBAL_TRACE(("flag = %d\n", flag));
	flag = 0;
	lFixed = 0x01;//方向

	cmd->length = 0x32;
	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);


	//  	CAL_PrintDataBlock("HW Inserter CMD", cmd->buff, 32/*(cmd->buff[1] + 1) * 4*/);



	return SUCCESS;
}


/**	 清除 Save_H_L 位置的PSI信息 */
S32 ICPL_OPtorDPBStableTsInsertClearMake(ICPL_CmdRequest_t  *cmd, HWL_DPBTsInserter *param)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&param->save, 6, 2, ICPL_CODE_MODE_U16},
	};
	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_STABLE_DPB_TS );

	cmd->length = 0x01;
	GLOBAL_MEMSET(cmd->buff + 4, 0 , 4);

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}




/* 	30. 清除功能模块的复位标志   */
S32 ICPL_OPtorModuleResetMake(ICPL_CmdRequest_t  *cmd, HWL_ModuleReset_t *moduleReset)
{
	U32 id = 0x0;
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8 },
		{&id, 4, 4, ICPL_CODE_MODE_U32 },
	};


	//input check..
	assert( ICPL_CheckRange(0, 32, moduleReset->moduleId) == ICPL_CheckOk);
	if(ICPL_CheckRange(0, 32, moduleReset->moduleId) != ICPL_CheckOk)
	{
		return ERROR_BAD_PARAM;
	}


	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_MODULE_RESET );
	id = 0x1 << (moduleReset->moduleId);
	cmd->length = 0x01;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);


	return SUCCESS;
}



/**	PID映射操作.*/
S32 ICPL_OPtorPidMapSetMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapItem_t *_param)
{

	HWL_PidMapItem_t param2 = *_param;
	HWL_PidMapItem_t *param = &param2;

	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&param->inputLogicChannelId, 4, 2, ICPL_CODE_MODE_U16 },
		{&param->inputPid, 6, 2, ICPL_CODE_MODE_U16 },
		{&param->disflag, 8, 1, ICPL_CODE_MODE_U8},
		{&param->outputLogicChannelId, 9, 1, ICPL_CODE_MODE_U8},
		{&param->outputPid, 10, 2, ICPL_CODE_MODE_U16},
		{&param->serialNo, 12, 2, ICPL_CODE_MODE_U16},
		{&param->des_flag, 14, 1, ICPL_CODE_MODE_U16},
		{&param->outputPhyChannelId, 15, 1, ICPL_CODE_MODE_U8},
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDMAP);

	cmd->length = 0x03;

#ifdef ENCODER_CARD_PLATFORM
	{
		/*输出通道的高4位将放置到输入通道的高4位上，则输入输出通道均为12位宽*/
		param->inputLogicChannelId = param->inputLogicChannelId & 0x0FFF | ((param->outputLogicChannelId << 4) & 0xF000);
	}
#endif

	GLOBAL_MEMSET(cmd->buff + 4, 0, 12);

	param->disflag = (param->groupIndex & 0x7F)  | (!param->isCA << 7)  ;

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);

#ifdef DEBUG_SHOW_PIDMAP
	CAL_PrintDataBlock(__FUNCTION__, cmd->buff, (cmd->buff[1] + 1) * 4);
#endif

	return SUCCESS;

}


S32 ICPL_ReplicatePidMapSRCClearSetMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapItem_t *_param)
{

	GLOBAL_MEMSET(cmd->buff + 4, 0, 12);

	cmd->buff[0] = 0x0F;
	cmd->buff[1] = 0x02;
	cmd->buff[2] = 0x00;
	cmd->buff[3] = 0x00;

	cmd->buff[6] = (MPEG2_TS_PACKET_NULL_PID >> 8) & 0xFF;
	cmd->buff[7] = MPEG2_TS_PACKET_NULL_PID & 0xFF;

	cmd->buff[10] = 0x00;
	cmd->buff[11] = 0x01;

	cmd->length = 0x01;

	//CAL_PrintDataBlock(__FUNCTION__, cmd->buff, (cmd->buff[1] + 1) * 4);
	return SUCCESS;
}


S32 ICPL_ReplicatePidMapDestClearSetMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapItem_t *_param)
{

	GLOBAL_MEMSET(cmd->buff + 4, 0, 12);

	cmd->buff[0] = 0x10;
	cmd->buff[1] = 0x01;
	cmd->buff[2] = 0x00;
	cmd->buff[3] = 0x00;

	cmd->buff[6] = (MPEG2_TS_PACKET_NULL_PID >> 8) & 0xFF;
	cmd->buff[7] = MPEG2_TS_PACKET_NULL_PID & 0xFF;

	cmd->length = 0x01;

	//CAL_PrintDataBlock(__FUNCTION__, cmd->buff, (cmd->buff[1] + 1) * 4);
	return SUCCESS;
}



/**
*	bit31..24   xx2: bit23..16   xx3: bit15..8      xx4: bit7..0
*	bit31..19  : source PID
*	bit18..13  : target Channel (0..31)
*	bit12..00  : target PID
*
*/
static inline U32 __ICPL_PidMapOriginalToDestinedPackage(HWL_PidMapOriginalToDestined_t *item)
{
	U32 tmp = 0;
	assert(item != NULL);
	tmp |= (0x1FFF & item->destinedPid);
	tmp |= (0x3F & item->destinedChannelID) << 13;
	tmp |= (0x0FFF & item->originalPid) << 19;

	return tmp;
}

/**
*	Mem_Number : 当前数据帧的目的PID个数  ,
*	Flag 	=   0 :   PID源个数 <= 250
*			=   1  :   500  >=  PID源个数 > 250
*			=   2  :   750  >=  PID源个数 > 500
*			=  x   (0 <=x <= 0x0f) :   (x+1) * 250  >=  PID源个数 > (x * 250)
*	最多支持4000 个
*
*/
static inline U8  __ICPL_PidMapOriginalToDestinedTableFlag(U32  size)
{
	U8 retval;
	if(size <= 250)
	{
		retval = 0;
	}
	else if(250 < size && size <= 500)
	{
		retval = 1;
	}
	else if(500 < size && size <= 750)
	{
		retval = 2;
	}
	else
	{
		retval = 3;			//@Fix (0 <=x <= 0x1f) :   (x+1) * 250  >=  PID源个数 > (x * 250)
	}

	return retval;
}




/* 源PID到目标通道和目标PID映射的设置 */
S32 ICPL_OPtorPidMapOriginalToDestinedTableMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapOriginalToDestinedTable_t *pidMapTable)
{

	U32 i;
	U8 *buffIdx;
	U32 target;
	ICPL_ByteInfo_t headArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&pidMapTable->flag, 2, 1, ICPL_CODE_MODE_U8},
	};

	ICPL_ByteInfo_t byteArray[] =
	{
		{&target, 0, 4, ICPL_CODE_MODE_U32},
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_PIDMAP_SOURCE_TO_DES_SET);

	pidMapTable->flag = __ICPL_PidMapOriginalToDestinedTableFlag(pidMapTable->pidItemSize);
	cmd->length = pidMapTable->pidItemSize;

	//encoding...
	ICPL_ByteInfoEncode(headArray, sizeof(headArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	buffIdx = cmd->buff + 4;
	for(i = 0; i < pidMapTable->pidItemSize; i++)
	{
		target = __ICPL_PidMapOriginalToDestinedPackage( &pidMapTable->pidItemTable[i]);

		ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
			cmd->buff, cmd->buff_len);
		buffIdx += 4;		//Index move...
	}


	return SUCCESS;
}


typedef struct
{
	U16 inter;
	U32 decimal;
} ICPL_float_t;


/**
*	例:  64QAM ,  6.875M符号率
*	脉冲数  =  96 / ((6.875 * 6) * 188 / 204) * 188 * 8 = 3798.1090909090909090909090909091
*/
static inline  ICPL_float_t  __ICPL_OutputRateCalculate(F64 SymbolRate)
{
	ICPL_float_t value;
	double lPulse, lTmpPoint;

	lPulse = 96 / ((SymbolRate *  6) * 188 / 204) * 188 * 8 ;

	value.inter = lPulse;
	lTmpPoint = lPulse - value.inter;
	value.decimal = (U32)(lTmpPoint * (2 << 31)) ;


	return value;
}


/**	24  设置输出码率对应的96MHz的脉冲数(BL85KM模块输出需要) .*/
S32 ICPL_OPtorOutputRate96MhzMake(ICPL_CmdRequest_t  *cmd, HWL_OutputRate96Mhz_t *rateSet)
{
	ICPL_float_t value;

	ICPL_ByteInfo_t byteArray[] =
	{
		{ &cmd->length, 1, 1, ICPL_CODE_MODE_U8 },
		{ &rateSet->modularStand, 4, 1, ICPL_CODE_MODE_U8},
		{ &rateSet->modularMode, 5, 1, ICPL_CODE_MODE_U8},
		{ &value.inter, 6, 2, ICPL_CODE_MODE_U16},
		{ &value.decimal, 4, ICPL_CODE_MODE_U32},
	};


	value = __ICPL_OutputRateCalculate(rateSet->symbolRate);

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);


	return SUCCESS;
}





/**	25.0  VOD模式下输入TS参数的设置.*/
S32 ICPL_OPtorVodInputTsParamMake(ICPL_CmdRequest_t  *cmd, HWL_VodInputTsParam_t *vodInputTsParam)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&vodInputTsParam->channelID, 4, 2, ICPL_CODE_MODE_U16},
		{&vodInputTsParam->physicChannel, 7, 1, ICPL_CODE_MODE_U8},
		{&vodInputTsParam->inputIpAddress.part, 8, 4, ICPL_CODE_MODE_COPY  },
		{&vodInputTsParam->inputPort, 12, 2, ICPL_CODE_MODE_U16},
	};

	ICPL_ByteInfo_t byteVodArray[] =
	{
		{&vodInputTsParam->info.outputQamChannelID, 14, 1, ICPL_CODE_MODE_U8},
		{&vodInputTsParam->info.outputQamProgramID, 15, 1, ICPL_CODE_MODE_U8},
	};


	ICPL_ByteInfo_t byteMutorArray[] =
	{
		{&vodInputTsParam->info.inputPortLogicNo, 14, 2, ICPL_CODE_MODE_U16},
	};



	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_VOD_INPUT_PARAM);

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	switch(vodInputTsParam->vodOrMutor)
	{
	case HWL_CONST_VOD:
		ICPL_ByteInfoEncode(byteVodArray, sizeof(byteVodArray) / sizeof(ICPL_ByteInfo_t),
			cmd->buff, cmd->buff_len);
		break;
	case HWL_CONST_MUTOR:
		ICPL_ByteInfoEncode(byteMutorArray, sizeof(byteMutorArray) / sizeof(ICPL_ByteInfo_t),
			cmd->buff, cmd->buff_len);
		break;
	default:
		assert(0);
		return ERROR_BAD_PARAM;
	}

	return SUCCESS;
}



/* 26.1	与加密芯片的通讯 */
S32 ICPL_OPtorEncryptChipReadMake(ICPL_CmdRequest_t  *cmd, HWL_EncryptChip_t *encryptChip)
{

	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8 },
		{&encryptChip->mainNo, 5, 1, ICPL_CODE_MODE_U8},
		{&encryptChip->subNo, 6, 1, ICPL_CODE_MODE_U8},
		{&encryptChip->length, 7, 1, ICPL_CODE_MODE_U8},
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_ENCRYPT);
	cmd->length = 0x01;

	//     assert(ICPL_CheckRange(0, HWL_CONST_ENCRYPT_MAX_BUFF , encryptChip->length) == ICPL_CheckOk);
	//     if(	ICPL_CheckRange(0, HWL_CONST_ENCRYPT_MAX_BUFF , encryptChip->length) == ICPL_CheckNo)
	//     {
	//         return ERROR_BAD_PARAM;
	//     }

	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);
	return SUCCESS;
}


S32 ICPL_OPtorEncryptChipWriteMake(ICPL_CmdRequest_t  *cmd, HWL_EncryptChip_t *encryptChip)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8 },
		{&encryptChip->mainNo, 5, 1, ICPL_CODE_MODE_U8},
		{&encryptChip->subNo, 6, 1, ICPL_CODE_MODE_U8},
		{&encryptChip->length, 7, 1, ICPL_CODE_MODE_U8},
		{&encryptChip->buff, 8, 10 , ICPL_CODE_MODE_COPY},
	};

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_ENCRYPT);
	cmd->length = 0x04;

	assert(ICPL_CheckRange(0, HWL_CONST_ENCRYPT_MAX_BUFF , encryptChip->length) == ICPL_CheckOk);
	if(	ICPL_CheckRange(0, HWL_CONST_ENCRYPT_MAX_BUFF , encryptChip->length) == ICPL_CheckNo)
	{
		return ERROR_BAD_PARAM;
	}

#ifdef DEBUG
	printf("*Buff %x %x\n", encryptChip->buff[0] , encryptChip->buff[1]);
#endif
	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t),
		cmd->buff, cmd->buff_len);

	return SUCCESS;
}

S32 ICPL_OPtorPidMapMapOriginalPortTableMake(ICPL_CmdRequest_t  *cmd, HWL_ModularChipRegisterTable_t *registerTable)
{
	return 0;
}


/*****************************************************************************************************/
/** 回复数据结构体的解包操作.	*/
/*****************************************************************************************************/


/**	1. 心跳回应体.*/
static HWL_HeatBeat_t  hwl_headbeat;


/*	输入输出PID映射和节目搜索*/
static U8  icpl_cmd_pidmap_search_buff[ICPL_CMD_PIDMAP_RESPONSE_SEARCH_BUFF_LEN]
= {ICPL_TAGID_PIDMAP };

/**	硬件状态信息回复体。*/
static HWL_PhyStatusRskResponse_t hwl_phyresponse;

/**	查询通道码率回复 .*/
static HWL_ChannelRateArray_t hwl_ratearray[3];

/**	查询PID统计信息回复 .*/
//static HWL_PidRateArray_t hwl_pidratearray;

/**	PSI接受Buffer大小 */
static HWL_PSIBuffSize_t hwl_psibuff;

/**	输入流减少时FPGA报告CPU  */
static HWL_InputStreamNotifierTable_t hwl_inputstreamnotifertable;

/**	输入端口号节目的内容有变化时报告CPU   */
static HWL_InputPortEsNotifierTable_t hwl_inputportesnotifertable;

/**	加密芯片回复.*/
static HWL_EncryptChip_t  hwl_encrychip;





void ICPL_Cmd_Lock(ICPL_Cmd_t *cmd)
{
	assert(cmd != NULL);
	assert(cmd->mutex != NULL);
	if(cmd == NULL || cmd->mutex == NULL)
	{
		return ;
	}


	PFC_SemaphoreWait(cmd->mutex, -1);
}


void ICPL_Cmd_UnLock(ICPL_Cmd_t *cmd)
{
	assert(cmd != NULL);
	assert(cmd->mutex != NULL);
	if(cmd == NULL || cmd->mutex == NULL)
	{
		return ;
	}

	PFC_SemaphoreSignal(cmd->mutex);
}



/**********************************************************/



/**	回应对象表*/
static U32 ICPL_CmdResponseArraySize = 0;
static ICPL_Cmd_t ICPL_CmdResponseArray[HWL_CONST_RESPONSE_TABLE_MAX_SIZE];


static void __ICPL_CmdResponseArrayRegister(U8 tagId, void *data, U32 datalen,void *fun, U8 *buff, U32 bufflen, U32 timeLimit)
{
	U32 index = ICPL_CmdResponseArraySize;
	assert(0 <= index && index < HWL_CONST_RESPONSE_TABLE_MAX_SIZE);
	GLOBAL_MEMSET(&ICPL_CmdResponseArray[index], 0, sizeof(ICPL_Cmd_t));

	ICPL_CmdResponseArray[index].tagid = tagId;
	ICPL_CmdResponseArray[index].data = data;
	ICPL_CmdResponseArray[index].data_len = datalen;
	ICPL_CmdResponseArray[index].perform = (ICPL_CmdFun_t)fun;
	ICPL_CmdResponseArray[index].mutex = PFC_SemaphoreCreate("ICPL_CmdMutex", 1);

	ICPL_CmdResponseArray[index].time = 0;
	ICPL_CmdResponseArray[index].timeLimit = timeLimit;

	PFC_SemaphoreSignal(ICPL_CmdResponseArray[index].mutex);

	ICPL_CmdResponseArraySize++;
}


#define ICPL_CmdResponseArrayRegister(tagId,data,datalen,fun)   \
	__ICPL_CmdResponseArrayRegister(tagId,data,datalen,fun,NULL,0,-1)



static int __ICPL_CmdCompare(const void *a, const void *b)
{
	ICPL_Cmd_t *cmda = (ICPL_Cmd_t *)a;
	ICPL_Cmd_t *cmdb = (ICPL_Cmd_t *)b;
	assert(cmda != NULL && cmdb != NULL);

	return cmda->tagid - cmdb->tagid;
}


/**	排序，用于加速查找.*/
static void ICPL_CmdArraySort(ICPL_Cmd_t *array, S32 len)
{
	qsort(array, len, sizeof(ICPL_Cmd_t), __ICPL_CmdCompare );
	return ;
}


void ICPL_CmdArrayClear(void)
{
	ICPL_CmdResponseArraySize = 0;
}

/**	初始化两个命令对象表.*/
void ICPL_CmdArrayInit(void)
{
	HWL_DEBUG("sort for search\n");

	/**	心跳回应体.*/
	ICPL_CmdResponseArrayRegister(ICPL_TAGID_HEADBEAT, &hwl_headbeat , sizeof(hwl_headbeat), (ICPL_CmdFun_t)ICPL_OPtorHeatBeatParser);


	/**	该处理体拥有自己的缓冲区，供回调函数使用，因其内容需上层软件解析。*/
	__ICPL_CmdResponseArrayRegister(ICPL_TAGID_PIDMAP, NULL , 0 , NULL, icpl_cmd_pidmap_search_buff, sizeof(icpl_cmd_pidmap_search_buff) / sizeof(U8), -1);


	/**	硬件状态信息回复体。*/
	__ICPL_CmdResponseArrayRegister(ICPL_TAGID_PHY, &hwl_phyresponse , sizeof(hwl_phyresponse), ICPL_OPtorPhyRskResponseParser, NULL, 0, 3);


	/**	查询通道码率回复 .*/
	ICPL_CmdResponseArrayRegister(ICPL_TAGID_CHANNELRATE, hwl_ratearray , sizeof(hwl_ratearray),  ICPL_OPtorChannelRateParser);


	/**	查询PID统计信息回复 .*/
	ICPL_CmdResponseArrayRegister(ICPL_TAGID_PIDSTATICS, &s_HWLStatistics , sizeof(s_HWLStatistics), ICPL_OPtorStatisticsArrayParser);

	/**	PSI接受Buffer大小 */
	ICPL_CmdResponseArrayRegister(ICPL_TAGID_PSI_NUM, &hwl_psibuff , sizeof(hwl_psibuff),  ICPL_OPtorPSIBuffSizeParser);

	/**	输入流减少时FPGA报告CPU  */
	ICPL_CmdResponseArrayRegister(ICPL_TAGID_STREAM_D_NOTIFY, &hwl_inputstreamnotifertable , sizeof(hwl_inputstreamnotifertable), ICPL_OPtorInputStreamNotifierTableParser);

	/**	输入端口号节目的内容有变化时报告CPU   */
	ICPL_CmdResponseArrayRegister(ICPL_TAGID_STREAM_ES_NOTIFY, &hwl_inputportesnotifertable , sizeof(hwl_inputportesnotifertable), ICPL_OPtorInputPortEsNotifierTableParser);


	/**	加密芯片回复.*/
	ICPL_CmdResponseArrayRegister(ICPL_TAGID_ENCRYPT, &hwl_encrychip , sizeof(hwl_encrychip), ICPL_OPtorEncryptChipParser);


	ICPL_CmdArraySort(ICPL_CmdResponseArray, ICPL_CmdResponseArraySize );
}



/**	通过2分搜索查找命令对象体.*/
ICPL_Cmd_t   *__ICPL_CmdArraybinarySeach(ICPL_Cmd_t *array, int array_len, U8 tagid)
{
	S32 left;
	S32 right;
	S32 middle;
	S32 ret;

	ICPL_Cmd_t *result;

	left = 0;
	right = array_len - 1;

	HWL_DEBUG("array_len:%d\n", array_len);

	while(left <= right)
	{
		middle = left + (right - left) / 2;


		assert(chk_range(0, array_len - 1, middle) == 0);
		HWL_DEBUG("middle:%d left:%d right:%d tagid:0x%x\n", middle, left, right, array[middle].tagid);

		ret = tagid - array[middle].tagid;
		if(ret == 0)
		{
			result = &array[middle];

			return result;
		}
		else if(ret < 0)
		{
			right = middle - 1;
		}
		else
		{
			left = middle + 1;
		}

	}

	return NULL;
}




/**	通过@tagID查找回复对象.*/
ICPL_Cmd_t *ICPL_CmdResponseFind(U8 tagID)
{
	return __ICPL_CmdArraybinarySeach(ICPL_CmdResponseArray, ICPL_CmdResponseArraySize, tagID);
}


ICPL_CmdFun_t ICPL_CmdResponseCallbackSet(U8  tagID, ICPL_CmdFun_t userfun)
{
	ICPL_Cmd_t *cmd = NULL;
	ICPL_CmdFun_t old_fun = NULL;


	HWL_DEBUG("tagID:%d\n", tagID);
	assert(userfun != NULL);
	cmd = __ICPL_CmdArraybinarySeach(ICPL_CmdResponseArray,
		ICPL_CmdResponseArraySize , tagID );


	assert(cmd != NULL);	//sorry.
	if(cmd == NULL)
	{
		return NULL;
	}


	ICPL_Cmd_Lock(cmd);

	old_fun = cmd->callback;
	cmd->callback = userfun;


	ICPL_Cmd_UnLock(cmd);

	return old_fun;
}







/* E3/DE3接口 ----------------------------------------------------------------------------------------------------------------------------------------------------- */

#define HWL_E3DS3_ENCODE_BUFF_LENGTH   20
#define HWL_E3DS3_MAX_NUM 4

typedef struct  
{
	U8   ChipID;     /* 芯片编号  0x21 */
	U8   RorW;
	U8	 ChannelControl;
	U8   FunctionControl;
}MULT_SubESDS3;

typedef struct  
{
	MULT_SubESDS3	m_pE3DS3[HWL_E3DS3_MAX_NUM];
}MULT_E3DS3;

HWL_E3DS3Param_t s_E3DS3Param[HWL_E3DS3_MAX_NUM];


void HWL_E3DS3SetParameter(S16 TsIndex, HWL_E3DS3Param_t *pParam)
{
	if (GLOBAL_CHECK_INDEX(TsIndex, HWL_E3DS3_MAX_NUM))
	{
		GLOBAL_MEMCPY(&s_E3DS3Param[TsIndex], pParam, sizeof(HWL_E3DS3Param_t));
	}
}

void HWL_E3DS3ApplyParameter(S16 TsIndex)
{
	U8 *E3DS3buff = GLOBAL_MALLOC(HWL_E3DS3_ENCODE_BUFF_LENGTH);
	S16 i;
	HWL_E3DS3Param_t *plE3DS3Param;
	MULT_E3DS3 pE3DS3;
	GLOBAL_MEMSET(E3DS3buff,0,sizeof(E3DS3buff));

	for(i=0;i< HWL_E3DS3_MAX_NUM;i++)
	{		
		pE3DS3.m_pE3DS3[i].ChipID = 0x21;			
		pE3DS3.m_pE3DS3[i].RorW =0x0;	
		pE3DS3.m_pE3DS3[i].ChannelControl = i;		
		if(GS_E3DS3_SELECT_E3  == s_E3DS3Param[i].E3DS3Select)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1<<7));	
		}
		else if(GS_E3DS3_SELECT_DS3 == s_E3DS3Param[i].E3DS3Select)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl |=(1<<7);
		}
		else
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1<<7));
		}

		if(GS_E3DS3_BITORDER_MSB  == s_E3DS3Param[i].BitOrder)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1<<6));
		}
		else 
		{
			pE3DS3.m_pE3DS3[i].FunctionControl |=(1<<6);
		}

		if(GS_E3DS3_FRAMEFORM_NO  == s_E3DS3Param[i].FrameForm)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1<<5));
		}
		else 
		{
			pE3DS3.m_pE3DS3[i].FunctionControl  |=(1<<5);
		}

		if(GS_E3DS3_PACKETLENGTH_188  == s_E3DS3Param[i].InOutPacketLength)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1<<4));
		}
		else 
		{
			pE3DS3.m_pE3DS3[i].FunctionControl |=(1<<4);
		}
		if(FALSE  == s_E3DS3Param[i].InterweaveCoding)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1<<3));
		}
		else 
		{
			pE3DS3.m_pE3DS3[i].FunctionControl |=(1<<3);
		}

		if(FALSE  == s_E3DS3Param[i].ScrambleChanger)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1<<2));
		}
		else 
		{
			pE3DS3.m_pE3DS3[i].FunctionControl |=(1<<2);
		}

		if(FALSE  == s_E3DS3Param[i].RSCoding)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1<<1));
		}
		else 
		{
			pE3DS3.m_pE3DS3[i].FunctionControl |=(1<<1);
		}
		if(FALSE  == s_E3DS3Param[i].CodeRateRecover)
		{
			pE3DS3.m_pE3DS3[i].FunctionControl &=(~(1));
		}
		else 
		{
			pE3DS3.m_pE3DS3[i].FunctionControl |=(1); 
		}
		E3DS3buff[(i*4)+4] = pE3DS3.m_pE3DS3[i].ChipID;
		E3DS3buff[(i*4)+5] = pE3DS3.m_pE3DS3[i].RorW;
		E3DS3buff[(i*4)+6] = pE3DS3.m_pE3DS3[i].ChannelControl;
		E3DS3buff[(i*4)+7] = pE3DS3.m_pE3DS3[i].FunctionControl;

	}
	E3DS3buff[0] = 0x11;	
	E3DS3buff[1] = HWL_E3DS3_ENCODE_BUFF_LENGTH/4 - 1;
	E3DS3buff[2] = 0;	
	E3DS3buff[3] = 0;
	HWL_FPGAWrite(E3DS3buff, HWL_E3DS3_ENCODE_BUFF_LENGTH);
	GLOBAL_FREE(E3DS3buff);
}

/* ASI 接口 ----------------------------------------------------------------------------------------------------------------------------------------------------- */

#define		HWL_ASI_ARRAY_SIZE			(16)
#define		ICPL_TAGID_ASI_OUT_RATE		0x11


typedef struct  
{
	U8		m_ChnIndex;
	U8		m_SubIndex;
	S32		m_Bitrate;//
}HWL_ASINode;

typedef struct  
{
	HWL_ASINode m_pNode[ HWL_ASI_ARRAY_SIZE ];
	S32			m_NodeNum;
}HWL_ASIArray;

HWL_ASIArray s_ASIArray;

U16 HWL_ASIBitRateCal(S32 BitRate)
{
	U32 CalBitRate;

	CalBitRate = ((MULT_FPGA_MAIN_CLK * 1000000) /2) / ((F64)BitRate / 1504);
	CalBitRate -= 1;
	if(CalBitRate > 47000)
	{
		CalBitRate = 47000;
	}
	else if(CalBitRate < 470)
	{
		CalBitRate = 470;
	}
	//GLOBAL_TRACE(("Set Asi BitRate = %d, Value = 0x%04x\n", BitRate, CalBitRate));
	return CalBitRate & 0xFFFF;
}

void HWL_ASIClear(S32 DeviceIndex, BOOL bInput)
{
	HWL_ASIArray *plArray;
	if (bInput)
	{
		plArray = &s_ASIArray;
	}
	else
	{
		plArray = &s_ASIArray;
	}
	plArray->m_NodeNum = 0;
}


S32 HWL_ASIAdd(S32 DeviceIndex, S32 ChnIndex, S32 SubIndex, U32 BitRate, BOOL8 Active, BOOL bInput)
{
	S32 i;
	HWL_ASIArray 	*plArray;
	HWL_ASINode 	*plNode;
	if (bInput)
	{
		plArray = &s_ASIArray;
	}
	else
	{
		plArray = &s_ASIArray;
	}

	if (plArray->m_NodeNum < HWL_ASI_ARRAY_SIZE)
	{
		plNode = &plArray->m_pNode[plArray->m_NodeNum];
		plNode->m_ChnIndex 	= ChnIndex;
		plNode->m_SubIndex 	= SubIndex;
		plNode->m_Bitrate 		= BitRate;
		plArray->m_NodeNum++;
	}
}

void HWL_ASIApply(S32 DeviceIndex, S32 ChnIndex, BOOL bInput)
{
	S32 i, k;
	S32 lLen, lMemberNum;

	U8 tmp;
	U16 tmpBitRate;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf, lTagID;
	HWL_ASIArray 	*plArray;
	HWL_ASINode 	*plNode;


	if (bInput)
	{
		return;
	}
	else
	{
		plArray = &s_ASIArray;
		lTagID = ICPL_TAGID_ASI_OUT_RATE;
	}

	lLen = 0;
	plTmpBuf = plCMDBuf;
	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	GLOBAL_MSB8_EC(plTmpBuf, lTagID, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, plArray->m_NodeNum * 2, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
#ifdef GC1804C
	GLOBAL_MSB8_EC(plTmpBuf, 0x10, lLen);//和Tuner I2C通道切换命令分开
#else
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
#endif


	lMemberNum = 0;

	for (k = 0; k < plArray->m_NodeNum; k++)
	{
		plNode = &plArray->m_pNode[k];
		if (lMemberNum < HWL_ASI_ARRAY_SIZE)//最大支持16个
		{
			if (plNode->m_ChnIndex == ChnIndex)
			{
				tmpBitRate = HWL_ASIBitRateCal(plNode->m_Bitrate);
				GLOBAL_MSB8_EC(plTmpBuf, 0x22, lLen);
				GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
				tmp = (plNode->m_SubIndex << 4) |(0x01);
				GLOBAL_MSB8_EC(plTmpBuf, tmp, lLen);
				GLOBAL_MSB8_EC(plTmpBuf, (U8)((tmpBitRate >> 8) & 0x00ff) , lLen);

				GLOBAL_MSB8_EC(plTmpBuf, 0x22, lLen);
				GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
				tmp = (plNode->m_SubIndex << 4) |(0x00);
				GLOBAL_MSB8_EC(plTmpBuf, tmp, lLen);
				GLOBAL_MSB8_EC(plTmpBuf, (U8)(tmpBitRate  & 0x00ff), lLen);
				lMemberNum++;
			}
		}
		else
		{

			GLOBAL_TRACE(("\n"));
			break;
		}
	}
	HWL_FPGAWrite(plCMDBuf, lLen);
}


S32 ICPL_OPtorPhyChannelParamMake(ICPL_CmdResponse_t  *cmd, HWL_PhyChannelParam_t *param)
{
	ICPL_ByteInfo_t byteArray[] =
	{
		{&cmd->length, 1, 1, ICPL_CODE_MODE_U8},
		{&param->physicChannelId, 3, 1, ICPL_CODE_MODE_U8},
		{&param->mac.mac, 6, 6, ICPL_CODE_MODE_COPY},
		{&param->ipVersion, 15, 1, ICPL_CODE_MODE_U8},
		{&param->addressv4, 16, 4, ICPL_CODE_MODE_U32},
	};

	assert(param != NULL);

	__ICPL_CmdTestTagId(cmd, ICPL_TAGID_ETH_CHN_PARAM );
	cmd->length = 0x06;
	ICPL_ByteInfoEncode(byteArray, sizeof(byteArray) / sizeof(ICPL_ByteInfo_t), cmd->buff, cmd->buff_len);

	return SUCCESS;
}



/*PCR 接口！ --------------------------------------------------------------------------------------------------------------------------------------- */
#ifdef MULT_SYSTEM_HAVE_PCR_CORRECT_ADJUST_FUNCTION
/*
设置PCR校验开关和参数值！
ChnIndex：		会转换成物理通道号
PCRPositive：	0~127 * 2.8 ms
PCRNegative：	0~127 * 11.1 us
bOpen		设置成FALSE时该通道不参加PCR校验
*/
#define HWL_PCR_CORRECT_MODE_TAG 0x0D	

void HWL_SetChnPCRCorrectMode(S32 ChnIndex, U8 PCRUper, U8 PCRLower, BOOL bOpen)
{
	S32 i, k, lLen;
	S32 lPhyIndex;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf, lTmpValue;

	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	plTmpBuf = plCMDBuf;

	lPhyIndex = 0;

	lLen = 0;
	GLOBAL_MSB8_EC(plTmpBuf, HWL_PCR_CORRECT_MODE_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, lPhyIndex, lLen);

	lTmpValue = bOpen?1:0;
	GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);

	lTmpValue = PCRUper;
	GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);
	lTmpValue = PCRLower;
	GLOBAL_MSB8_EC(plTmpBuf, lTmpValue, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}
#endif


/*TD 接口 ---------------------------------------------------------------------------------------------------------------------------------------- */
#if defined(GQ3650DR) || defined(GQ3710A) || defined(GQ3710B) || defined(GQ3655) || defined(GQ3760A) || defined(GQ3760)|| defined(GM8358Q)
S32 HWL_TunerCountCheck(void)
{
	S32 i, k, lTunerCount, lRet;
	U8 DataBuffer[10];

	lTunerCount = 0;
	InitTunerIIC();
#ifdef GM8358Q
	for (i = 0; i< 1; i++)
#else
	for (i = 0; i< HWL_TUNER_MAX_NUM; i++)
#endif
	{
		HWL_SetI2cAndTunerReset(i,0,0);
		PFC_TaskSleep(100);
		HWL_SetI2cAndTunerReset(i,1,0);
		//检测多次
		for (k =0; k< 1; k++)
		{
			if ((lRet = TUNER_Write(0x18 ,0x1C,DataBuffer,0, 0)) == NORMAL)
			{
				if ((lRet = TUNER_Read(0x18, DataBuffer, 1)) == NORMAL)
				{
					GLOBAL_TRACE(("Get I2C Data = 0x%.2X\n", DataBuffer[0]));
					if (DataBuffer[0]!=0)
					{
						lTunerCount ++;
						break;
					}
				}
				else
				{
					GLOBAL_TRACE(("I2C Read Error = %d\n", lRet));
				}
			}
			else
			{
				GLOBAL_TRACE(("I2C Write Error = %d\n", lRet));
			}
			PFC_TaskSleep(100);
		}
		PFC_TaskSleep(100);

		if (DataBuffer[0] == 0)
		{
			break;
		}
	}

	GLOBAL_TRACE(("Get Tuner With I2C Count = %d\n", lTunerCount));
	return lTunerCount;
}

/*这个函数实际上仅仅检测ChipAdr == 0x18(7Bit)的情况下I2C是否有应答和应答的数据只要不是全0即判断有效*/
S32 HWL_TDA10025CountCheck(void)
{
	S32 i, k, lTunerCount, lRet;
	U8 DataBuffer[10];
	U8 *plTmpBuf;
	U32 lChipID;

	lTunerCount = 0;
	for (i = 0; i< HWL_TUNER_MAX_NUM; i++)
	{
		HWL_SetI2cAndTunerReset(i,0,0);
		PFC_TaskSleep(100);
		HWL_SetI2cAndTunerReset(i,1,0);
		//检测多次
		for (k =0; k< 1; k++)
		{
			plTmpBuf = DataBuffer;
			GLOBAL_MSB24_E(plTmpBuf, 0x00100000);
			if ((lRet = TUNER_Write(0x18 , DataBuffer[0], &DataBuffer[1], 2, 0)) == NORMAL)
			{
				GLOBAL_ZEROMEM(DataBuffer, 4);
				if ((lRet = TUNER_Read(0x18, DataBuffer, 4)) == NORMAL)
				{
					plTmpBuf = DataBuffer;
					lChipID = GLOBAL_MSB32_D(plTmpBuf, lChipID);
					GLOBAL_TRACE(("Get ChipID Data = 0x%.8X\n",lChipID));
					if (lChipID != 0 )
					{
						lTunerCount ++;
						break;
					}
				}
				else
				{
					GLOBAL_TRACE(("I2C Read Error = %d\n", lRet));
				}
			}
			else
			{
				GLOBAL_TRACE(("I2C Write Error = %d\n", lRet));
			}
			PFC_TaskSleep(100);
		}
		PFC_TaskSleep(100);

		if (DataBuffer[0] == 0)
		{
			break;
		}
	}

	GLOBAL_TRACE(("Get Tuner With I2C Count = %d\n", lTunerCount));
	return lTunerCount;
}

S32 HWL_AVL6211CountCheck(void)
{
	S32 i, k, lTunerCount, lRet;
	U8 DataBuffer[10];
	U8 *plTmpBuf;
	U32 lChipID;

	lTunerCount = 0;
	for (i = 0; i< HWL_TUNER_MAX_NUM; i++)
	{
		HWL_SetI2cAndTunerReset(i,0,0);
		PFC_TaskSleep(100);
		HWL_SetI2cAndTunerReset(i,1,0);
		//检测多次
		for (k =0; k< 1; k++)
		{
			plTmpBuf = DataBuffer;
			GLOBAL_MSB24_E(plTmpBuf, 0x00100000);
			if ((lRet = TUNER_Write(0x18 , DataBuffer[0], &DataBuffer[1], 2, 0)) == NORMAL)
			{
				GLOBAL_ZEROMEM(DataBuffer, 4);
				if ((lRet = TUNER_Read(0x18, DataBuffer, 4)) == NORMAL)
				{
					plTmpBuf = DataBuffer;
					lChipID = GLOBAL_MSB32_D(plTmpBuf, lChipID);
					GLOBAL_TRACE(("Get ChipID Data = 0x%.8X\n",lChipID));
					if (lChipID == 0x01000002)
					{
						lTunerCount ++;
						break;
					}
				}
				else
				{
					GLOBAL_TRACE(("I2C Read Error = %d\n", lRet));
				}
			}
			else
			{
				GLOBAL_TRACE(("I2C Write Error = %d\n", lRet));
			}
			PFC_TaskSleep(100);
		}

		HWL_SetI2cAndTunerReset(i,0,0);
		PFC_TaskSleep(100);
		HWL_SetI2cAndTunerReset(i,1,0);

		PFC_TaskSleep(100);

		if (DataBuffer[0] == 0)
		{
			break;
		}
	}

	GLOBAL_TRACE(("Get AVL6211 DVBS2 Demod With I2C, Count = %d\n", lTunerCount));
	return lTunerCount;
}

S32 HWL_ATBM8869CountCheck(void)
{
	S32 i, k, lTunerCount, lRet;
	U8 DataBuffer[10];
	U8 lChipAddr;

	lTunerCount = 0;
	for (i = 0; i< HWL_TUNER_MAX_NUM; i++)
	{
		HWL_SetI2cAndTunerReset(i,0,0);
		PFC_TaskSleep(100);
		HWL_SetI2cAndTunerReset(i,1,0);

		//检测多次
		for (k = 0; k < 1; k++)
		{
			lChipAddr = 0x80;
			DataBuffer[0] = 0x00;
			if ((lRet = TUNER_Write(lChipAddr ,0x00, DataBuffer, 1, 0)) == NORMAL)
			{
				if ((lRet = TUNER_Read(lChipAddr, DataBuffer, 1)) == NORMAL)
				{
					if (DataBuffer[0] == 0x40)
					{
						lTunerCount ++;
						break;
					}
					else
					{
						GLOBAL_TRACE(("Tuner [%d] Detected Failed, Get I2C Data = 0x%.2X should = 0x40\n", i, DataBuffer[0]));
					}
				}
				else
				{
					GLOBAL_TRACE(("I2C Read Error = %d, ChipAddr = %.2X\n", lRet, lChipAddr));
				}
			}
			else
			{
				GLOBAL_TRACE(("I2C Write Error = %d, ChipAddr = 0x%.2X\n", lRet, lChipAddr));
			}
			PFC_TaskSleep(100);
		}
		PFC_TaskSleep(100);
		if (DataBuffer[0] != 0x40)
		{
			break;
		}
	}

	GLOBAL_TRACE(("Get DTMB Tuner With I2C Count = %d~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", lTunerCount));
	return lTunerCount;
}

#endif

/* 直通相关函数（未使用）---------------------------------------------------------------------------------------------------------------------------*/
#define HWL_DIRECT_ROUTE_MODE_TAG 0x21	

void HWL_SetDirectRouteMode(S32 InChnIndex, S32 InSubIndex, S32 OutChnIndex, S32 OutSubIndex, BOOL bDirectRoute)
{
	S32 i, k, lLen;
	S32 lPhyIndex;
	U8 plCMDBuf[HWL_MSG_MAX_SIZE], *plTmpBuf, blMode;

	blMode = (bDirectRoute>0)?1:0;

	GLOBAL_ZEROMEM(plCMDBuf, sizeof(plCMDBuf));

	plTmpBuf = plCMDBuf;

#ifdef GM2730H
	if (InChnIndex == 1)
	{
		lPhyIndex = 0;
	}
	else if (InChnIndex == 2)
	{
		lPhyIndex = 1;
	}
	else
	{
		lPhyIndex = 0;
	}
#else
	lPhyIndex = 0;
#endif


	lLen = 0;
	GLOBAL_MSB8_EC(plTmpBuf, HWL_DIRECT_ROUTE_MODE_TAG, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x01, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, lPhyIndex, lLen);

	GLOBAL_MSB8_EC(plTmpBuf, 0x00, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, blMode, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, OutSubIndex, lLen);
	GLOBAL_MSB8_EC(plTmpBuf, InSubIndex, lLen);

	HWL_FPGAWrite(plCMDBuf, lLen);
}

/*FPGA写入接口设置相关函数 -------------------------------------------------------------------------------------------------------------------------------------------*/

S32 HWL_FPGAWrite(U8 *Buff, U32 Size)
{
	S32 Retval;

	//if (Buff[0] == 0x45)
	//{
	//	CAL_PrintDataBlock((CHAR_T*)__FUNCTION__, Buff, Size);
	//}

	//__HWL_RequestBuffLock();				//LOCK 	.BUFF

	if(DRL_FpgaWriteLock(Buff, Size) < 0)
	{
		GLOBAL_TRACE(("Error !!!!!\n"));
		Retval = ERROR;

	}
	else
	{
		Retval = SUCCESS;
	}
	//__HWL_RequestBuffUnLock();			//UNLOCK BUFF.

	return Retval;

}

U8 HWL_GetHardwareVersion(void)
{
	return s_HardwareVersion;
}

U8 HWLL_GetHardwareVersion(void)
{
#ifdef GN1846
	return FSPI_GetFpgaHwVer() / 10;
#else
	S32 i;
	U8 val, lLast = 0;
	for (i = 0; i < 10; i++)
	{
		val = DRL_FanHwGet() & 0x01;
		if (i > 0)
		{
			if (lLast == val)
			{
				break;
			}
		}
		lLast = val;
		PFC_TaskSleep(100);
	}
	return val;
#endif
}





/*设备个别通道参数设置函数！！！！*/
#ifdef SUPPORT_NEW_HWL_MODULE
#else
BOOL HWL_GetHWInfo(HWL_HWInfo *pHWInfo)
{
	U32 i;
	S32 lChnInd;
	HWL_PhyStatusRskResponse_t response;
	HWL_HeatBeat_t heatBeat;
	BOOL blHaveTunerS = FALSE;
	BOOL blHaveTunerC = FALSE;

	PFC_TaskSleep(1000);
	HWL_DataPerchaseLock(ICPL_TAGID_PHY, response);
	i=5;

#ifndef MULT_DEVICE_NOT_SUPPORT_ENCRYPT_CHIP
#ifdef GM8358Q
	while((i > 0) && ((response.PhyStatusRskTable.tableSize==0)))
#else
	while((i > 0) && ((response.PhyStatusRskTable.tableSize==0) || (response.chipSn == 0x0)))
#endif
#else
#ifdef GN1846
	while((i > 0) && ((response.PhyStatusRskTable.tableSize==0)))
#else
	while((i > 0) && ((response.PhyStatusRskTable.tableSize==0) || (response.chipSn == 0x0)))
#endif
#endif
	{
		PFC_TaskSleep(1000);
		HWL_DataPerchaseLock(ICPL_TAGID_PHY, response);
		HWL_PhyStatusQskSend();
		GLOBAL_TRACE(("Wait For CHIP ID\n"));
		i--;
	}

#ifdef MULT_DEVICE_NOT_SUPPORT_ENCRYPT_CHIP
	MULT_OLDSNLoadSN(MULT_SN_FILE_NAME);
	pHWInfo->m_ChipID = MULT_OLDSNGetSN();
#else
	pHWInfo->m_ChipID = response.chipSn;
#endif

#ifdef GM8358Q
	pHWInfo->m_ChipID = HWL_EncoderGetFPGAID();
#endif

	HWL_DataPerchaseLock(ICPL_TAGID_HEADBEAT, heatBeat);
	//pHWInfo->m_HWInserterNum = heatBeat.psiNumber;


	snprintf(pHWInfo->m_pFPGARelease, 64, "%d-%.2d-%.2d", response.fpga.year, response.fpga.month, response.fpga.day);

	//检测是否有Tuner板的存在
	pHWInfo->m_ChannelNum = response.PhyStatusRskTable.tableSize;
	for(i = 0; i < response.PhyStatusRskTable.tableSize; i++)
	{
		if (response.PhyStatusRskTable.table[i].physicChannelId == HWL_CHANNEL_TYPE_TUNER_C)
		{
			GLOBAL_TRACE(("Device Have Extend Board of DVB-C Demod\n"));
			blHaveTunerC = TRUE;
		}
		else if (response.PhyStatusRskTable.table[i].physicChannelId == HWL_CHANNEL_TYPE_TUNER_S)
		{
			GLOBAL_TRACE(("Device Have Extend Board of DVB-S Demod\n"));
			blHaveTunerS = TRUE;
		}
#ifdef GQ3760B
		else if (response.PhyStatusRskTable.table[i].physicChannelId == HWL_CHANNEL_TYPE_CLK_TYPE)
		{
			if (response.PhyStatusRskTable.table[i].inOrOut == 1)
			{
				//40M
				pHWInfo->m_REFCLKHz = 40 * 1000000;
			}
			else
			{
				pHWInfo->m_REFCLKHz = 10 * 1000000;
			}
			GLOBAL_TRACE(("REFCLK = %d Hz\n", pHWInfo->m_REFCLKHz));
		}
#endif
	}

#if defined(GQ3650DS) 
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DVB_C_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;


	pHWInfo->m_InTsMax = 8;
	pHWInfo->m_OutTsMax = 4;
#endif

#if defined(GQ3650DR) 
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;

	if (blHaveTunerC || blHaveTunerS)
	{
		pHWInfo->m_TunerCount = HWL_TunerCountCheck();

		if (pHWInfo->m_TunerCount > 0)
		{
			lChnInd++;
			pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
			if (blHaveTunerC)
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_C;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_C;
			}
			else
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_S;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_S_AVL6211;
			}
			pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 8;
			pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;
		}
		else
		{
			GLOBAL_TRACE(("Failed Detected Demod Chips!!!!!!!!!!!!!!!!!\n"));
		}

	}

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DVB_C_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;


	pHWInfo->m_InTsMax = 12;
	pHWInfo->m_OutTsMax = 4;
#endif



#if defined(GQ3710A)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;

	if (blHaveTunerC || blHaveTunerS)
	{
		pHWInfo->m_TunerCount = HWL_TunerCountCheck();

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		if (blHaveTunerC)
		{
			pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_C;
			pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_C;
		}
		else
		{
			pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_S;
			pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_S_AVL6211;
		}
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 8;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;
	}

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DVB_S_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;


	pHWInfo->m_InTsMax = 12;
	pHWInfo->m_OutTsMax = 1;
#endif





#if defined(GA2620B) 
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 2;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;


	pHWInfo->m_InTsMax = 2;
	pHWInfo->m_OutTsMax = 1;
#endif

# if defined(GM2700B) || defined(GM2700S)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;


	pHWInfo->m_InTsMax = pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport;
	pHWInfo->m_OutTsMax = pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport;


#endif

#if defined(GM2730X) || defined(GM2730F)
	{
		lChnInd = 0;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 4;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;
#ifdef GM2762
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 32;
#endif

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;


		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_DEP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 4;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;


		pHWInfo->m_InTsMax = 48;
		pHWInfo->m_OutTsMax = 8;

	}
#endif

#if defined(GM2730S)
	{
		lChnInd = 0;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 8;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 120;


		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_DEP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 450;


		pHWInfo->m_InTsMax = 128;
		pHWInfo->m_OutTsMax = 450;

	}
#endif

#if defined(GQ3655)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;


	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		pHWInfo->m_TunerType = lType;
		if (pHWInfo->m_TunerCount > 0)
		{
			lChnInd++;
			pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
			pHWInfo->m_pInfoList[lChnInd].m_DemodType = lType;

			if (lType == TD_DEMOD_TYPE_ATBM8869)
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_DTMB;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869;
			}
			else if (lType == TD_DEMOD_TYPE_AVL6211)
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_S;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_S_AVL6211;
			}
			pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
			pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;
		}


	}



	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 128;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DVB_C_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP_DEP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	pHWInfo->m_InTsMax = 160;
	pHWInfo->m_OutTsMax = 4;
#endif

	//add by leonli
#if defined(GM8358Q)

	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ENCODER;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ENCODER_CVBS;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ENCODER;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ENCODER_CVBS;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;//输入ASI
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 1;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;


	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		if (pHWInfo->m_TunerCount > 0)
		{
			lChnInd++;
			pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
			pHWInfo->m_pInfoList[lChnInd].m_DemodType = lType;

			if (lType == TD_DEMOD_TYPE_ATBM886X)
			{
				if (blHaveTunerC)
				{
					pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_C;
					pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_C;
				}
				else
				{
					pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_DTMB;
					pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869;
				}
			}
			else if (lType == TD_DEMOD_TYPE_AVL6211)
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_S;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_S_AVL6211;
			}
			pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 2;
			pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;
		}


	}




	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DVB_C_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	pHWInfo->m_InTsMax = 3;
	pHWInfo->m_OutTsMax = 4;

#endif


#if defined(GQ3760A)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;


	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		if (pHWInfo->m_TunerCount > 0)
		{
			lChnInd++;
			pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
			if (lType == TD_DEMOD_TYPE_ATBM886X)
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_DTMB;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869;
			}
			else if (lType == TD_DEMOD_TYPE_AVL6211)
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_S;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_S_AVL6211;
			}
			pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
			pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;
		}


	}

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 128;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DTMB_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP_DEP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 160;
	pHWInfo->m_OutTsMax = 1;
#endif

#if defined(GM2730H)
	{
		lChnInd = 0;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 16;//100Mbps

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 16;//1000Mbps


		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;


		pHWInfo->m_InTsMax = 48;
		pHWInfo->m_OutTsMax = 4;

	}
#endif

#if defined(GM4500)
	{
		lChnInd = 0;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 8;

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 16;

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 6;


		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_DEP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 12;


		pHWInfo->m_InTsMax = 48;
		pHWInfo->m_OutTsMax = 32;

	}
#endif

#if defined(GN2000)
	{
		lChnInd = 0;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 16;

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_DEP;
		pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_IP;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DVB_C_MODULATOR;
		pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

		pHWInfo->m_InTsMax = 48;
		pHWInfo->m_OutTsMax = 32;

	}
#endif

#if defined(GQ3760B)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DTMB_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	//lChnInd++;
	//pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	//pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP_DEP;
	//pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	//pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	//pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 160;
	pHWInfo->m_OutTsMax = 1;
#endif

#if defined(GQ3763)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DTMB_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP_DEP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 160;
	pHWInfo->m_OutTsMax = 1;
#endif

#if defined(GQ3765)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DTMB_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP_DEP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 160;
	pHWInfo->m_OutTsMax = 1;
#endif

#if defined(GM2750)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP_DEP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 33;
	pHWInfo->m_OutTsMax = 1;
#endif

#if defined(LR1800S)
	lChnInd = 0;

	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;

		if (lType == TD_DEMOD_TYPE_ATBM886X)
		{
			pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_DTMB;
			pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869;
		}

		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;

	}

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 16;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DTMB_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 4;
	pHWInfo->m_OutTsMax = 1;
#endif


#if defined(GM7000)

	lChnInd = 0;
	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		if (pHWInfo->m_TunerCount > 0)
		{
		}
		else
		{
			GLOBAL_TRACE(("Detected TD Failed!!!!!!!!!!!!!!!\n"));
		}

		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_C;
		pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_C;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;
	}

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 128;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DVB_C_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP_DEP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	pHWInfo->m_InTsMax = 160;
	pHWInfo->m_OutTsMax = 4;
#endif

#if defined(GC1815B)
	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		while(TRUE)
		{
			PFC_TaskSleep(1000);
		}
	}
#endif



#if defined(GC1804C)

	lChnInd = 0;
	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		if (pHWInfo->m_TunerCount <= 0)
		{
			GLOBAL_TRACE(("Detected TD Failed!!!!!!!!!!!!!!!\n"));
		}

		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_DTMB;
		pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;
	}

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType=HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;


	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;


	pHWInfo->m_InTsMax = 4;
	pHWInfo->m_OutTsMax = 4;
#endif


#if defined(GQ3760)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		if (pHWInfo->m_TunerCount > 0)
		{
			lChnInd++;
			pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		}

		if (lType == TD_DEMOD_TYPE_ATBM8869)
		{
			pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_DTMB;
			pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869;
		}

		if (lType == TD_DEMOD_TYPE_AVL6211)
		{
			pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_S;
			pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_S_AVL6211;
		}
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 16;
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;

	}

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DTMB_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;


	pHWInfo->m_InTsMax = 32;
	pHWInfo->m_OutTsMax = 1;
#endif

#if defined(GQ3768)
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	//if (HWL_TDDemodCheck(NULL) > 0)
	{
		pHWInfo->m_TunerCount = 1;
		lChnInd++;
		pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
		pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_DTMB;
		pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869;
		pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;//待定
		pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;
	}

#if 1//调试用
	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;
#endif


	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DTMB_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 8;
	pHWInfo->m_OutTsMax = 1;
#endif


#if defined(GC1804B)
	lChnInd = 0;

	{
		S32 lType;
		pHWInfo->m_TunerCount = HWL_TDDemodCheck(&lType);
		if (pHWInfo->m_TunerCount > 0)
		{
			pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
			pHWInfo->m_pInfoList[lChnInd].m_DemodType = lType;

			if (lType == TD_DEMOD_TYPE_ATBM8869)
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_DTMB;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_DTMB_ATBM8869;
			}
			else if (lType == TD_DEMOD_TYPE_AVL6211)
			{
				pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_TUNER_S;
				pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_TUNER_S_AVL6211;
			}
			pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
			pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = pHWInfo->m_TunerCount;
		}
	}


	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	pHWInfo->m_InTsMax = 4;
	pHWInfo->m_OutTsMax = 4;
#endif

#if defined(GQ3710B) 
	lChnInd = 0;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 2;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 32;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 128;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_DVB_S_MODULATOR;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_MODULATOR_AD9789;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	//lChnInd++;
	//pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	//pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP_LOOP_DEP;
	//pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	//pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	//pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 160;
	pHWInfo->m_OutTsMax = 1;
#endif

#if defined(GN1866)
	lChnInd = 0;

	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ASI;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 2;

	pHWInfo->m_InTsMax = 2;
	pHWInfo->m_OutTsMax = 2;
#endif

#if defined(GN1846)
	lChnInd = 0;

	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_IN;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_ENCODER;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_ENCODER_HI3531A;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 4;

	lChnInd++;
	pHWInfo->m_pInfoList[lChnInd].m_Direction = HWL_CHANNEL_DIRECTION_OUT;
	pHWInfo->m_pInfoList[lChnInd].m_Type = HWL_CHANNEL_TYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_SubType = HWL_CHANNEL_SUBTYPE_IP;
	pHWInfo->m_pInfoList[lChnInd].m_StartTsIndex = 0;
	pHWInfo->m_pInfoList[lChnInd].m_CurSubSupport = 1;

	pHWInfo->m_InTsMax = 4;
	pHWInfo->m_OutTsMax = 1;
#endif
	pHWInfo->m_ChannelNum = lChnInd + 1;

	return 0;
}

#endif

//EOF
