/* Includes-------------------------------------------------------------------- */
#include "global_micros.h"
#include "platform_assist.h"
#include "multi_main_internal.h"

#ifdef MULT_SUPPORT_FPGA_ETH
/* Global Variables (extern)--------------------------------------------------- */
/* Macro (local)--------------------------------------------------------------- */
#define FPGA_ETH_INDEX					(0xFE)
#define FPGA_ETH_MAC_FRAME_MIN_LENGTH	(22)
#define FPGA_ETH_MAC_FRAME_BUFFER_SIZE	(MACRO_MAC_FRAME_MAX_LENGTH + 100)
#define FPGA_ETH_INTERFACE_NAME			("fpga_eth")
/* Private Constants ---------------------------------------------------------- */
/* Private Types -------------------------------------------------------------- */
typedef struct  
{
	U8	m_AdpLength;
	U8	m_AdpFlag;
	U8	m_PayloadLength;
	U8	m_PayloadMod;
}FPGA_ETH_ADP;
/* Private Variables (static)-------------------------------------------------- */
static TUN_InitParam s_TunInitParam;
static HANDLE32 s_TUNHandle = NULL;

/*MAC->TS变量*/
static U8 s_pTSBuffer[MPEG2_TS_PACKET_SIZE];
static MPEG2_TsHead s_EncoderTsHead;
static U8 s_EncoderCC = 0;

/*TS->MAC变量*/
static U8 s_pMACFrameBuffer[FPGA_ETH_MAC_FRAME_BUFFER_SIZE];
static S32 s_MACFrameCurrentSize = 0;
static MPEG2_TsHead s_DecoderTsHead;
static U8 s_DecoderCC = 0;
/* Private Function prototypes ------------------------------------------------ */
/* Functions ------------------------------------------------------------------ */
S32 MULTL_AdpEncoder(FPGA_ETH_ADP *pAdp, U8 *pBuff)
{
	GLOBAL_MSB8_E(pBuff, pAdp->m_AdpLength);
	GLOBAL_MSB8_E(pBuff, pAdp->m_AdpFlag);
	GLOBAL_MSB8_E(pBuff, pAdp->m_PayloadLength);
	GLOBAL_MSB8_E(pBuff, pAdp->m_PayloadMod);
	return 4;
}


S32 MULTL_AdpDecoder(FPGA_ETH_ADP *pAdp, U8 *pBuff)
{
	GLOBAL_MSB8_D(pBuff, pAdp->m_AdpLength);
	GLOBAL_MSB8_D(pBuff, pAdp->m_AdpFlag);
	GLOBAL_MSB8_D(pBuff, pAdp->m_PayloadLength);
	GLOBAL_MSB8_D(pBuff, pAdp->m_PayloadMod);
	return 4;
}


void MULTL_TUNReadFromTUNCB(void* pParam, U8* pBuff, S32 Len)
{
	/*将MAC帧分割打包成TS发送给FPGA*/
	S32 lDataLeft;
	S32 lTsPayloadSize;
	U8 *plTmpTsBuf, *plTmpMACBuf;
	BOOL blFirstPacket = TRUE;
	BOOL blLastPacket = FALSE;
	BOOL blExLastPacket = FALSE;
	FPGA_ETH_ADP lAdp;

#ifdef DEBUG_IP_TO_TS_PROTOCOL
	CAL_PrintDataBlock("TUN -> PHY", pBuff, Len);
#endif

	lAdp.m_AdpLength = 0x03;
	lAdp.m_PayloadMod = (4- Len % 4) & 0x03;//得到无效字节数
	lAdp.m_AdpFlag = 0x00;

	lDataLeft = Len;

	plTmpMACBuf = pBuff;

	while(lDataLeft > 0)
	{

		plTmpTsBuf = s_pTSBuffer;

		if (lDataLeft <= 180)
		{
			blLastPacket = TRUE;
			s_EncoderTsHead.adaptation_field_control = 0x03;
			lTsPayloadSize = lDataLeft;
		}
		else if (lDataLeft <= 184)
		{
			blExLastPacket = TRUE;
			s_EncoderTsHead.adaptation_field_control = 0x00;
			lTsPayloadSize = lDataLeft;
		}
		else
		{
			s_EncoderTsHead.adaptation_field_control = 0x00;
			lTsPayloadSize = 184;
		}

		lDataLeft -= lTsPayloadSize;

		if (blFirstPacket == TRUE)
		{
			blFirstPacket = FALSE;
			s_EncoderTsHead.payload_unit_start_indicator = 0x01;
		}
		else
		{
			s_EncoderTsHead.payload_unit_start_indicator = 0x00;
		}


		/*修正连续计数*/
		s_EncoderTsHead.continuity_counter = s_EncoderCC;
		s_EncoderCC = (s_EncoderCC + 1) & 0x1F;

		plTmpTsBuf += MPEG2L_TsHeadPacker(&s_EncoderTsHead, plTmpTsBuf);
		if (blLastPacket == TRUE)
		{
			lAdp.m_PayloadLength = lTsPayloadSize / 4 + ((lAdp.m_PayloadMod > 0)?1:0);
			plTmpTsBuf += MULTL_AdpEncoder(&lAdp, plTmpTsBuf);
		}
		GLOBAL_BYTES_E(plTmpTsBuf, plTmpMACBuf, lTsPayloadSize);
		plTmpMACBuf += lTsPayloadSize;

		HWL_FPGAETHSendToFPGA(FPGA_ETH_INDEX, s_pTSBuffer);
	}


	if (blExLastPacket == TRUE)
	{
		plTmpTsBuf = s_pTSBuffer;
		s_EncoderTsHead.payload_unit_start_indicator = 0x00;
		s_EncoderTsHead.continuity_counter = s_EncoderCC;
		s_EncoderTsHead.adaptation_field_control = 0x02;//Payload Only
		s_EncoderCC = (s_EncoderCC + 1) & 0x1F;
		plTmpTsBuf += MPEG2L_TsHeadPacker(&s_EncoderTsHead, plTmpTsBuf);
		lAdp.m_PayloadLength = 0;
		plTmpTsBuf += MULTL_AdpEncoder(&lAdp, plTmpTsBuf);
		HWL_FPGAETHSendToFPGA(FPGA_ETH_INDEX, s_pTSBuffer);
	}

}

void MULTL_TUN_FPGAEthTSCB(S32 Index, U8* pTsBuff)
{
	U8 *plTmpTsBuf;
	S32 lPayloadSize = 0;
	FPGA_ETH_ADP lAdp;
	/*将TS重新组合成MAC帧，发送给TUN接口*/

	plTmpTsBuf = pTsBuff;
	MPEG2_TsHeadParser(&s_DecoderTsHead, plTmpTsBuf);
	plTmpTsBuf += 4;

#ifdef DEBUG_IP_TO_TS_PROTOCOL
	GLOBAL_TRACE(("payload_unit_start_indicator = %d, adaptation_field_control = %d, Current Frame Size = %d\n", s_DecoderTsHead.payload_unit_start_indicator, s_DecoderTsHead.adaptation_field_control, s_MACFrameCurrentSize));
#endif

	lAdp.m_AdpLength = 0;
	if (s_DecoderTsHead.payload_unit_start_indicator > 0)
	{
		/*这个是第一帧*/
		s_MACFrameCurrentSize = 0;
	}
	else
	{
		if (s_MACFrameCurrentSize <= 0)
		{
			/*没有接收到包头，剩余数据丢弃*/
			GLOBAL_TRACE(("MAC Frame without Head, Droped, Current Length = %d\n", s_MACFrameCurrentSize));
			return;
		}
	}

	if ((s_DecoderTsHead.adaptation_field_control & 0x02) != 0)
	{
		plTmpTsBuf += MULTL_AdpDecoder(&lAdp, plTmpTsBuf);
		lPayloadSize = lAdp.m_PayloadLength * 4;
	}
	else
	{
		lPayloadSize = 184;
	}

#ifdef DEBUG_IP_TO_TS_PROTOCOL
	GLOBAL_TRACE(("Payload Size = %d\n", lPayloadSize));
#endif


	if (sizeof(s_pMACFrameBuffer) >= s_MACFrameCurrentSize + lPayloadSize)
	{
		if (lPayloadSize >= 0)
		{
			GLOBAL_MEMCPY(s_pMACFrameBuffer + s_MACFrameCurrentSize, plTmpTsBuf, lPayloadSize);
			s_MACFrameCurrentSize += lPayloadSize;
		}

		if ((s_DecoderTsHead.adaptation_field_control & 0x02) != 0)
		{
#ifdef DEBUG_IP_TO_TS_PROTOCOL
			GLOBAL_TRACE(("lAdp.m_PayloadMod = %d\n", lAdp.m_PayloadMod));
#endif

			s_MACFrameCurrentSize -= lAdp.m_PayloadMod;

			if (s_MACFrameCurrentSize > FPGA_ETH_MAC_FRAME_MIN_LENGTH)
			{
#ifdef DEBUG_IP_TO_TS_PROTOCOL
				CAL_PrintDataBlock("PHY -> TUN", s_pMACFrameBuffer, s_MACFrameCurrentSize);
#endif

				TUN_WriteToTun(s_TUNHandle, s_pMACFrameBuffer, s_MACFrameCurrentSize);
			}
			else
			{
				GLOBAL_TRACE(("MAC FRAME LENGTH ERROR! Length = %d\n", s_MACFrameCurrentSize));
			}
			s_MACFrameCurrentSize = 0;
		}
	}
	else
	{
		GLOBAL_TRACE(("MAC Frame Buffer Overload! Length = %d, TsPayload = %d\n", s_MACFrameCurrentSize, lPayloadSize));
	}
}

//#ifdef MULT_SUPPORT_FPGA_ETH
//void TestRelayCapability(void)
//{
//	S32 i;
//	HANDLE32 lSocket;
//	U8 *plData;
//	S32 lActLen;
//	U32 lSrcIP = 0;
//	U16 lSrcPort = 0;
//	CRYPTO_DESContext lCT;
//	U8 plKey[8];
//
//	plData = (U8 *)GLOBAL_MALLOC(1024*64);
//
//	lSocket = PFC_SocketCreate(PFC_SOCKET_TYPE_UDP);
//	if (lSocket)
//	{
//		GLOBAL_TRACE(("Socket = 0x%.8x\n", lSocket));
//
//		PFC_SocketBind(lSocket, 0, 9999);
//
//		lActLen = 10000;
//		PFC_SocketOption(lSocket, PFC_SOCKET_OPTION_TYPE_RECVTIMEOUT, &lActLen, sizeof(lActLen));
//		lActLen = 10*1024*1024;
//		PFC_SocketOption(lSocket, PFC_SOCKET_OPTION_TYPE_RECVBUF, &lActLen, sizeof(lActLen));
//		lActLen = 10*1024*1024;
//		PFC_SocketOption(lSocket, PFC_SOCKET_OPTION_TYPE_SENDBUF, &lActLen, sizeof(lActLen));
//		while(TRUE)
//		{
//			if (PFC_SocketRecvFrom(lSocket, plData, 1024*64, &lActLen, &lSrcIP, &lSrcPort))
//			{
//				if (lActLen > 0)
//				{
//#ifdef DEBUG_IP_TO_TS_PROTOCOL
//					CAL_PrintDataBlock("RECV Data", plData, lActLen);
//#endif
//
//
//					if (PFC_SocketSendTo(lSocket, plData, lActLen, &lActLen, lSrcIP, 9999))
//					{
//
//					}
//					else
//					{
//						GLOBAL_TRACE(("Send Error!\n"));
//					}
//				}
//				else
//				{
//					GLOBAL_TRACE(("Recv Timeout!\n"));
//				}
//			}
//			else
//			{
//				GLOBAL_TRACE(("Recv Error!\n"));
//			}
//		}
//	}
//}
//#endif

void MULT_FPGAEthInitiate(U32 ChipID)
{
	/*加载内核模块*/
	PFC_System("insmod /tmp/tun.ko");

	s_TunInitParam.m_bUseOneQueue = TRUE;
	s_TunInitParam.m_bUseTAP = TRUE;
	s_TunInitParam.m_bUseTUNHead = FALSE;
	s_TunInitParam.m_bHead32BITAlign = TRUE;
	s_TunInitParam.m_pTunReadDataCB = MULTL_TUNReadFromTUNCB;
	s_TunInitParam.m_pUserParam = NULL;
	GLOBAL_STRCPY(s_TunInitParam.m_TUNInterfaceName, FPGA_ETH_INTERFACE_NAME);
	s_TunInitParam.m_TUNIPAddr = 0xC0A80A0A;//192.168.10.10
	s_TunInitParam.m_TUNIPMask = 0xFFFFFF00;//255.255.255.0
	s_TunInitParam.m_TUNIPGate = 0xC0A80A01;//192.168.10.1
	MULTL_GenerateMAC(MULT_DEVICE_TYPE, MULT_DEVICE_SUB_TYPE, FPGA_ETH_INDEX, ChipID, s_TunInitParam.m_TUNMAC, GLOBAL_MAC_BUF_SIZE);

	s_TunInitParam.m_TUNMAC[4] = 0x55;
	s_TunInitParam.m_TUNMAC[5] = 0x55;

	HWL_FPGAEthSetCB(MULTL_TUN_FPGAEthTSCB);

	GLOBAL_ZEROMEM(&s_EncoderTsHead, sizeof(s_EncoderTsHead));
	s_EncoderTsHead.pid = 0x800;
	s_EncoderTsHead.sync_byte = MPEG2_TS_PACKET_SYN_BYTE;


//#ifdef MULT_SUPPORT_FPGA_ETH
//	PFC_TaskCreate(__FUNCTION__, 1024, (PFC_ENTRY_FUNC)TestRelayCapability, 1, NULL);
//#endif
}

void MULT_FPGAEthApply(BOOL bOpen)
{
	GLOBAL_TRACE(("Apply FPGA ETH\n"));
	if (s_TUNHandle)
	{
		TUN_Destroy(s_TUNHandle);
		s_TUNHandle = NULL;
	}

	//GLOBAL_TRACE(("Set FPGA ETH IP/MAC\n"));
	/*设置IP和MAC地址*/
	HWL_ETHSetChnParameter(FPGA_ETH_INDEX, s_TunInitParam.m_TUNIPAddr, s_TunInitParam.m_TUNMAC, TRUE, 0);

	if (bOpen == TRUE)
	{
		//GLOBAL_TRACE(("Open FPGA Eth\n"));
		s_TUNHandle = TUN_Create(&s_TunInitParam);
		TUN_ReadTaskStart(s_TUNHandle);
	}
}

TUN_InitParam *MULT_FPGAEthGetParameterPtr(void)
{
	return &s_TunInitParam;
}

void MULT_FPGAEthTerminate(void)
{
	MULT_FPGAEthApply(FALSE);
	PFC_System("rmmod /tmp/tun.ko");
}
#endif

/*EOF*/
