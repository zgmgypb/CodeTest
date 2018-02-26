#ifndef __MULTI_HWL_INTERNAL_H__
#define __MULTI_HWL_INTERNAL_H__

#include "platform_conf.h"
#include "multi_private.h"

#define HWL_DEBUG {}
#define HWL_TRACE {}

/* check range */
#define chk_range(begin ,end ,now )  ( !(  (begin)<=(now) &&(now)<=(end) ) )


#define HWL_CONST_MAX_QAM_CHN_NUM				4
#define		HWL_MSG_MAX_SIZE			(1024)



/*TagID*/
#define ICPL_TAGID_HEADBEAT 			0x01
#define ICPL_TAGID_DPB_TS				0x0A
#define ICPL_TAGID_ETH_CHN_PARAM		0x0B
#define ICPL_TAGID_PIDMAP				0x05
#define ICPL_TAGID_INPUT_ETH_SUB_PARAM	0x03
#define ICPL_TAGID_OUTPUT_ETH_SUB_PARAM	0x04
#define ICPL_TAGID_PHY					0x0C
#define ICPL_TAGID_CHANNELRATE			0x02

#define ICPL_TAGID_PIDSTATICS			0x08

#define ICPL_TAGID_IPSTATICS			0x08

#define ICPL_TAGID_CW_SEND				0x06
#define ICPL_TAGID_CW_SWITCH			0x07

#define ICPL_TAGID_CA_ENABLE			0x06
#define ICPL_TAGID_PSI_NUM				0x0D
#define ICPL_TAGID_GROUP_BROADCAST		0x03
#define ICPL_TAGID_PIDCOPY_ORIGINAL			0x0f
#define ICPL_TAGID_PIDCOPY_DESTINED			0x10
#define ICPL_TAGID_GROUP_MODULOR_REGISTER 0x011

#define ICPL_TAGID_PCR_VERYFY			0X12
#define ICPL_TAGID_STREAM_D_NOTIFY		0X13
#define ICPL_TAGID_STREAM_ES_NOTIFY		0X14
#define ICPL_TAGID_PIDMAP_PORT_SET				0X15
#define ICPL_TAGID_PIDMAP_SOURCE_TO_DES_SET	0X16
#define ICPL_TAGID_OUTPUT_RATE_96M_BL85KM		0X17
#define ICPL_TAGID_VOD_INPUT_PARAM		0X18
#define ICPL_TAGID_VODPATCRC			0x09
#define ICPL_TAGID_ENCRYPT			0X20
#define ICPL_TAGID_PASSMODE			0X21
#define ICPL_TAGID_IGMP_MODE		0X22
#define ICPL_TAGID_STABLE_DPB_TS		0X23
#define ICPL_TAGID_MODULE_RESET		0X24

#define ICPL_TAGID_FPGA_ETH_TS		0X31

#define ICPL_TAGID_SFN_STATUS		0X33

#define HWL_CONST__IPV4		0x04
#define HWL_CONST__IPV6		0x06



/**	最大通道数目。*/
#define HWL_CONST_CHANNEL_NUM_MAX		256
#define HWL_CONST_SUBCHN_NUM_MAX		250


//#define HWL_CONST_PID_NUM_MAX			255
#define HWL_CONST_IPPORT_NUM_MAX		250




#define HWL_CONST_PIDSOURCE_MAX		125
#define HWL_CONST_PIDDESTINED_MAX		250


/**	硬件通道状态查询应答的最大条目数.*/
#define HWL_CONST_PHYINFO_NUM_MAX  	10

/**	PID映射设置的最大条目数.*/
#define HWL_CONST_PIDMAP_NUM_MAX	4096

/**	PID COPY 源表和PID映射表最大条目数 */
#define HWL_CONST_PIDCOPY_NUM_MAX	512


/*	18.1 CPU对调制器有关的芯片的寄存器设置和读取: (PHY_Ch = 0)	*/
#define HWL_CONST_CHIPID_DS1775	0X10
#define HWL_CONST_CHIPID_ADF4360	0X14
#define HWL_CONST_CHIPID_AD5245	0X18
#define HWL_CONST_CHIPID_BL85MM	0X20

#define HWL_CONST_READ		0
#define HWL_CONST_WRITE	1


/*	19.CPU对TS输入端口和PCR 校验的MODE 设置: (PHY_Ch = 0) */
#define HWL_CONST_VERIFY_YES	1
#define HWL_CONST_VERIFY_NO	0



/**	间隔低于40mS[不]插入PCR包*/
#define HWL_CONST_PCR_VERIFY_MODE_40MS_INSERT_YES		1
#define HWL_CONST_PCR_VERIFY_MODE_40MS_INSERT_NO		0


#define HWL_CONST_MODULAR_STAND_J83A		0
#define HWL_CONST_MODULAR_STAND_J83B		1
#define HWL_CONST_MODULAR_STAND_J83C		2


/* 	24.设置输出码率对应的96MHz的脉冲数(BL85KM模块输出需要),   */
#define HWL_CONST_MODULAR_MODE_QPSK		0
#define HWL_CONST_MODULAR_MODE_8QPSK		1
#define HWL_CONST_MODULAR_MODE_16QAM		2
#define HWL_CONST_MODULAR_MODE_32QAM		3
#define HWL_CONST_MODULAR_MODE_64QAM		4
#define HWL_CONST_MODULAR_MODE_128QAM		5
#define HWL_CONST_MODULAR_MODE_256QAM 		6
#define HWL_CONST_MODULAR_MODE_512QAM		7
#define HWL_CONST_MODULAR_MODE_1024QAM		8


/* 	25.VOD模式下输入TS参数的设置: */
#define HWL_CONST_VOD		1		//VOD模式
#define HWL_CONST_MUTOR	0		//复用广播节目


/* 	加密芯片存储的最大长度.*/
#define HWL_CONST_ENCRYPT_MAX_BUFF	80




/* 	29. 向DPB板进行永久TS插入188数据(CPU-〉FPGA)  TAG = 0x23   */
#define HWL_CONST_PACKET_ECM		1
#define HWL_CONST_PACKET_PSI		0


#define HWL_SUCCESS		0
#define HWL_FAILED		-1


#define HWL_CONST_REQUEST_TABLE_MAX_SIZE	100
#define HWL_CONST_RESPONSE_TABLE_MAX_SIZE	100




#define SUCCESS				0
#define ERROR				-1
#define ERROR_BAD_PARAM		-2


/*FPGA IIC相关*/
#define ICPL_IIC_IC_ID_AD9789_START				0x01
#define ICPL_IIC_IC_ID_DS1775					0x10
#define ICPL_IIC_IC_ID_ADF4360					0x14
#define ICPL_IIC_IC_ID_AD5245					0x18
#define ICPL_IIC_IC_ID_ADF4350					0x15
#define ICPL_IIC_IC_ID_FPGA_DTMB				0x24




#define ICPL_IIC_REG_AD5245						0x00

#define ICPL_IIC_REG_ADF4360_COUNTERLATCH		0x00
#define ICPL_IIC_REG_ADF4360_R_COUNTERLATCH		0x10
#define ICPL_IIC_REG_ADF4360_N_COUNTERLATCH		0x20

#define ICPL_IIC_REG_DS1775_REG_CONF_L			0x01
#define ICPL_IIC_REG_DS1775_REG_CONF_H			0x11

#define ICPL_IIC_REG_DS1775_REG_TEMP_LOW_L		0x02
#define ICPL_IIC_REG_DS1775_REG_TEMP_LOW_H		0x12

#define ICPL_IIC_REG_DS1775_REG_TEMP_HIGHT_L	0x03
#define ICPL_IIC_REG_DS1775_REG_TEMP_HIGHT_H	0x13


#define HWL_AD9789_MODULE_NUM					1
#define HWL_AD9789_MODULE_SUB_CHENNEL_NUM		4

#define HWL_BL85KMM_MODULE_NUM					2
#define HWL_BL85KMM_MODULE_SUB_CHENNEL_NUM		16


#define ICPL_IIC_REG_AD9789_QAM_SP_INFO			0x80
#define ICPL_IIC_REG_AD9789_QBSK_SP_INFO		0x60
#define ICPL_BUFFER_DEVICE_TYPE_BL85KMM			0x00



/************************************************************************************/
/* 	IP地址抽象结构 .*/
/************************************************************************************/
typedef struct
{
	U8	part[6];
} HWL_IPAddressV6_t;

#define HWL_CONST_MAC_LEN	8

typedef struct
{
	S8 	mac[HWL_CONST_MAC_LEN]; //Mac 地址.
} HWL_MacAddress_t;


S32 __HWL_DataPerformLock(U32 tagid, void *fun, void *data);
S32 ____HWL_DataPerchaseLock(S32 tagid, void *data, U32 size, void *__fun, U32 TickMark);


/** just copy */
#define __HWL_DataPerchaseLock(tagid,address,size)  ____HWL_DataPerchaseLock(tagid,address,size,NULL, 0)
#define HWL_DataPerchaseLock(tagId, data)   ____HWL_DataPerchaseLock(tagId, &data, sizeof(data), NULL, 0)
#define HWL_DataPerchaseLockSync(tagId, data, mark)   ____HWL_DataPerchaseLock(tagId,&data,sizeof(data), NULL, mark)



/************************************************************************************/
/*	1.心跳数据接口*/
/************************************************************************************/
typedef struct tagHWL_NicInfo
{
	U8	flag;
	U32	rate;
} HWL_NicInfo_t;

typedef struct tagHWL_HeatBeat
{
	U16 temperature;
	U32 InTSRate;
	U32 OutTsRate;
	U32 resetWord;
	S32	psiNumber;
	U8	readError;
	U8	writeError;
	U8 	buffFlag;
	U32	chipPll;
	U8 shortStatus;

} HWL_HeatBeat_t;

void HWL_HeatBeatShow(HWL_HeatBeat_t *headbeat);


/**	输入码率查询.*/
S32 HWL_InputChnRateRequest();

/**	输出码率查询.*/
S32 HWL_OutputChnRateReqeust();
S32 HWL_OutputChnRateReqeustForS();


/************************************************************************************/
/* 组播的加入与退出	*/


#define HWL_CONST_GROUP_BROADCAST_ADD			1		//加入
#define HWL_CONST_GROUP_BROADCAST_DELETE		0		//退出

typedef struct tagHWL_IPAddressV4
{
	U8	part[4];
} HWL_IPAddressV4_t;



typedef struct tagHWL_GroupBroadcast
{
	S32 operator;						//[HWL_CONST_GRPOUP-*]
	U8	physicChannelId;				//操作物理通道端口号.
	U8	groupNo;						//设置主播组的编号
	HWL_IPAddressV4_t *ipaddressTable;		//IP地址表
	S32 ipaddressSize;

} HWL_GroupBroadcastTable_t;


/**	组播的加入与退出 */
S32 HWL_GroupBroadTableCastSet( HWL_GroupBroadcastTable_t *groupCast);

/************************************************************************************/
/*	2.向DPB板进行TS插入188数据(CPU-〉FPGA)  TAG = 0x0a */
/************************************************************************************/

typedef struct tagHWL_DPBTsInsert
{
	U8  physicChannelId;
	U16 logicChannelId;
	U8	*tsDataBuff;
	S32	tsDataBuffSize;

} HWL_DPBTsInsert_t;



typedef struct
{
	U32 asiRate;
	U32 qamRate;

} HWL_AllRateInfo_t ;

U16 HWL_AllRateInfo( HWL_AllRateInfo_t  *rateInfo);


/************************************************************************************/
/*	4.输出TS 参数设置 (CPU-〉FPGA)  TAG = 0x04.*/
/************************************************************************************/



typedef struct tagHWL_OutPutTsParam
{
	U8	physicChannelId;		//01:phy IP port = 01
	S16 outputChannelId;		//TS_Index : 输出TS的逻辑编号。0..15
	S16 portNo;
	S8	ipVersion;
	HWL_IPAddressV4_t addressv4;
	HWL_IPAddressV4_t addressv6;

} HWL_OutPutTsParam_t;





/************************************************************************************/
/*	5.物理通道参数设置接口*/
/************************************************************************************/
typedef struct
{
	U8	physicChannelId;
	S8	ipVersion;

	HWL_MacAddress_t mac;
	U32 addressv4;
	U32 addressv6;

} HWL_PhyChannelParam_t;





/************************************************************************************/
/*	6. 输入输出PID映射和节目搜索*/
/************************************************************************************/



//	PID映射信息.
typedef struct
{
	U16	inputLogicChannelId;		//输入TS通道号
	U16 inputPid;					//输入PID
	U16 outputLogicChannelId;	//输出TS通道号
	U16 outputPid;				//输入PID
	U8	groupIndex;				//使用的控制器组.
	U8	disflag;					/**	内部使用此字段，无须填充 */
	U8	isCA;					//是否加扰.
	U8	inputPhyChannelId;			//		无用字段.可忽略
	U8 	outputPhyChannelId;		//物理通道标识.

	U16	serialNo;
	U8	des_flag;
	U8  index;					//排序序号.

} HWL_PidMapItem_t;


/************************************************************************************/
/* 7.输入TS的UDP端口的 参数设置 */
/************************************************************************************/


typedef struct tagHWL_InputUdpPort
{
	U8	physicChannelId;

	U16 inputLogicChannelId;				//输入TS通道号 [0..249]
	U16 portNumber;						//要接收TS的目的端口号。

	U8	serialNumber;					//按 IP : PortNumber的组成方式以升序排列的方式所得的排序序号.(0..N-1)

	U8   ipVersion;

	union
	{
		HWL_IPAddressV4_t v4;	//输入IP端口的源端口地址(32bits).
		HWL_IPAddressV6_t v6;	//输入IP端口的源端口地址(32bits).
	} originalAddress;


	union
	{
		HWL_IPAddressV4_t v4;	//输入IP端口的源端口地址(32bits).
		HWL_IPAddressV6_t v6;	//输入IP端口的源端口地址(32bits).
	} destinedAddress;

} HWL_InputUdpPort_t;



S32 HWL_InputUdpPortSend(HWL_InputUdpPort_t *inputUpdPort );



/************************************************************************************/
/* 10. 硬件通道状态查询 */
/************************************************************************************/


typedef struct tagHWL_PhyStatusQsk
{
	U8	requestType;				//查询类型、
	U8	physicChannelId;			//硬件通道标识.

} HWL_PhyStatusQsk_t;




/**
*	@PhysicType硬件类型
-- 0 : ASI        attu : 0 : input   1:output
-- 1 : 1000M IP
-- 2 : 100M IP
-- 3 : E3/DS3
-- 4 :  AD9789调制版   In/OUT : 1 out  MAX_CH = 16
-- 5 :  美国Logic调制版 In/OUT : 1 out  MAX_CH = 16/32
-- 6 :  Board ID : In/OUT : = 0 : MAX_CH  2Bytes : ID31..24  ID23..16
-- 7 :  Board ID : In/OUT : = 0 : MAX_CH  2Bytes : ID15..8   ID7..0
-- 0x80 -- haveware version
-- 0x81 -- Toncy Log
@InOrOut:输入或输出 [ HWL_CONTS_INPUT| HWL_CONST_OUTPUT]
@MaxChNum:物理通路中最大支持的逻辑通路.
*/

typedef struct tagHWL_PhyStatusRsk
{
	U8	physicChannelId;			//{HWL_CONST_PHY_* }
	U8	inOrOut;					//[HWL_CONST_INPUT | HWL_CONST_OUTPUT ]
	U16 maxLogicChannelNum;	//物理通路中最大支持的逻辑通路。

} HWL_PhyStatusRsk_t;



/**	FPGA版本..*/
typedef struct tagHWL_FpgaVersion
{
	U8 v;
	U32 year;
	U8 month;
	U8 day;

} HWL_FpgaVersion_t;


/**	硬件通道信息回复.*/
typedef struct
{
	HWL_PhyStatusRsk_t table[HWL_CONST_PHYINFO_NUM_MAX];
	U32 			tableSize;

} HWL_PhyStatusRskTable_t;


typedef struct
{
	HWL_FpgaVersion_t fpga;
	HWL_PhyStatusRskTable_t PhyStatusRskTable;
	U32 chipSn;

} HWL_PhyStatusRskResponse_t;


void HWL_PhyStatusRskResponseShow(HWL_PhyStatusRskResponse_t *hwlPhyStatusTable);

/*永久插入TS*/
typedef struct
{
	U16 save;			//(0..N-1)为存储的位置,  
	U8	allNumber;
	U8	sendNumber;
	U16	timeInterval;	
	U16	outLogChannel;

	U8 	packetType;		// if bit3 = 1 ,  except  ECM used as ODD or Even   , if  bit3 = 0 : no effect    
	U8	evenOrOdd;		
	U8	casChannelNo;	// CAS channel Number.   00: channel 1  …  11:channel 4 

	U8	*tsBuff;
	U32	tsBuffLen;

}HWL_DPBTsInserter;



/************************************************************************************/
/*	码率查询.*/
/************************************************************************************/

#define HWL_CONST_CHANNEL_INFO_REQ_INPUT_RATE		0x01		//输入码率
#define HWL_CONST_CHANNEL_INFO_REQ_OUTPUT_RATE		0x02		//输出码率


/**	查询结构体。*/
typedef struct tagHWL_ChannelRateInfo
{
	U8	requestType;		//[HWL_CONST_TSINFO_REQ_INPUT_RATE |
	// HWL_CONST_TSINFO_REQ_OUTPUT_RATE]
	U8	physicChannelId;
	U16 totalRate;		//总码率

} HWL_ChannelRateInfo_t;



/**	发送查询通道码率命令.*/
S32 HWL_ChannelRateInfoSend(HWL_ChannelRateInfo_t *);


typedef struct tagHWL_ChannelRate
{
	U16	logicChannelId;
	U32 logicChannelRate;

} HWL_ChannelRate_t;


typedef struct tagHWL_ChannelRateArray
{
	U8	requestType;	//[ HWL_CONST_TSINFO_REQ_INPUT_RATE |
	//  HWL_CONST_TSINFO_REQ_OUTPUT_RATE ]
	U8	physicChannelId;
	U32 totalRate;		//总码率

	HWL_ChannelRate_t channelRateArray[HWL_CONST_CHANNEL_NUM_MAX];
	U32	channelRateArraySize;

} HWL_ChannelRateArray_t;




#if 0

/************************************************************************************/
/* PID统计的端口设置 */
/************************************************************************************/

/**	查询结构体。*/
typedef struct tagHWL_PidStatics
{
	U8	physicChannelId;
	U16	logicChannelId;

} HWL_PidStatics_t;



/**	PID 统计的端口设置.*/
S32 HWL_PidStaticsSend(HWL_PidStatics_t *pidSearch);

/**	 PID 统计的端口码率查询请求.*/
S32 HWL_PidStaticsSearch(HWL_PidStatics_t *pidSearch);


typedef struct tagHWL_PidRate
{
	U16	pidVal;
	U16 pidRate;

} HWL_PidRate_t;



/**	回复结构体*/
typedef struct tagHWL_PidRateArray
{
	//HWL_PidStatics_t search;	//标识查询参数.
	U8 physicChannelId;			//
	U16 totalRate;				//通道总码率
	HWL_PidRate_t pidRateArray[HWL_CONST_PID_NUM_MAX];
	U32 channelRateArraySize;

} HWL_PidRateArray_t;

#endif


#if 0

/************************************************************************************/
/* 输入IP端口码率统计查询|清除 */
/************************************************************************************/


typedef struct tagHWL_InputIPPortStatistic
{
	U8	physicChannelId;
} HWL_InputIPPortStatistic_t;



typedef struct tagHWL_InputIPPortStatisticInfo
{
	HWL_IPAddressV4_t addressv4;
	U16 portNumber;
	U16 portRate;
} HWL_InputIPPortStatisticInfo_t;



typedef struct tagHWL_InputIPPortStatisticInfoArray
{
	U8	physicChannelID;
	U16	totalRate;
	HWL_InputIPPortStatisticInfo_t *ipPortInfoArray;
	U32		ipPortInfoArraySize;

} HWL_InputIPPortStatisticInfoArray_t;




/**	输入IP端口码率统计信息清除..*/
S32 HWL_InputIPPortStatisticClear( HWL_InputIPPortStatistic_t *statistic );

/**	输入IP端口码率统计信息查询..*/
S32 HWL_InputIPPortStatisticSend(HWL_InputIPPortStatistic_t *statistic);


S32 HWL_InputIPPortStatisticArrayGet(HWL_InputIPPortStatisticInfoArray_t *array);


#endif

typedef struct tagHWL_ConditionAccess
{
	U8 enabled;
} HWL_ConditionAccess_t;


/************************************************************************************/
/* 输入IP端口码率统计查询|清除 */
/************************************************************************************/

typedef struct
{
	U8	physicChannelId;
}HWL_EthDetectionParam;


typedef struct
{
	HWL_IPStatisticsArray		m_IPArray;
	HWL_PIDStatisticsArray		m_PIDArray;
}HWL_PIDETHStatistcs;


/************************************************************************************/
/*	15.PSI接受Buffer大小  	*/
/************************************************************************************/

typedef struct tagHWL_PSIBuffSize
{
	U8 stableTsPackgeNum;			//当前能接收永久PSI的TS包个数
	U8 immediateTsPackgeNum;		//即时PSI能接受的TS包个数。
} HWL_PSIBuffSize_t;


void HWL_PSIBuffSizeShow(HWL_PSIBuffSize_t *ratearray);



/**PID查询结构体。*/
typedef struct tagHWL_PidStatics
{
	U8	physicChannelId;
	U16	logicChannelId;

} HWL_PidStatics_t;




/************************************************************************************/
/*	copy PID 的设置: (PHY_Ch = 0)	*/
/************************************************************************************/


/**	 需要COPY的PID源的设置。
*
*	@(id):用户需要给每条映射信息赋予一个ID，该ID必须唯一。
*		在其相对应的目的表缓冲中加入一个或多个对应条目时，目的表中的表项的ID必须和此ID相同，
*		以表示其为映射关系.
*/
typedef struct tagHWL_PIDSource
{
	U16	id;
	U8	physicInputId;
	U16 originalChannelID;			//源通道
	U16	originalPid;					//源PID


	/*	以下成员不需要用户设置...*/
	U16 destinedPidOutListOffset;		//目的PID在输出LIST中的偏移量
	U16 destinedPidOutListNum;		//目的PID在输出LIST中的个数

} HWL_PIDSource_t;



typedef struct tagHWL_PIDCopy
{
	U8	physicChannel;
	U8	flag;
	HWL_PIDSource_t  *pidsourceTable;
	S32		pidsourceSize;

} HWL_PIDCopyOriginalTable_t;



void HWL_PIDCopyOriginalTableShow(HWL_PIDCopyOriginalTable_t *original);

/**	17.2 需要COPY的PID目的PID的设置。
*	@(id):该值必须为其对应的源表中的HWL_PIDSource_t.id相同
*/

typedef struct tagHWL_PIDdestined
{
	U16	id;
	U16 destinedChannelID;			//源通道
	U16	destinedPid;					//源PID

} HWL_PIDdestined_t;



typedef struct tagHWL_PIDCopyDestinedTable
{
	U8 flag;
	U8	physicChannel;
	HWL_PIDdestined_t  *pidDestinedTable;
	S32		pidDestinedSize;

} HWL_PIDCopyDestinedTable_t;



void HWL_PIDCopyDestinedTableShow(HWL_PIDCopyDestinedTable_t *destined);



/************************************************************************************/
/*	18.1 CPU对调制器有关的芯片的寄存器设置和读取: (PHY_Ch = 0)	*/
/************************************************************************************/


typedef struct tagHWL_ModularChipRegister
{
	U8	chipID;		//[ CONST_CHIPID_* ]
	U8	rOrw;		//[HWL_CONST_READ|HWL_CONST_WRITE]
	U8	address;		//读取或设置控制的芯片内寄存器地址.
	U8	value;		//设置控制的芯片内寄存器的值

} HWL_ModularChipRegister_t;

typedef struct tagHWL_E3DS3ChipRegister
{
U32						ChipID;
U8						ReadControl;
U8						E3DS3Select;					/*0 ..3 分别控制E3/DS3子板的输入通道1..4  */
}HWL_E3DS3ChipRegister_t;


typedef struct tagHWL_ModularChipRegisterTable
{
	U8	physicChannel;
	HWL_ModularChipRegister_t	*moduleRegisterTable;
	S32		moduleRegisterSize;

} HWL_ModularChipRegisterTable_t;




/************************************************************************************/
/*	19.CPU对TS输入端口和PCR 校验的MODE 设置: (PHY_Ch = 0) */
/************************************************************************************/




typedef struct tagHWL_ChannelInputPcrMode
{
	U8	physicChannel;
	U8	verify;
	U8	verifyMode;			//[HWL_CONST_PCR_VERIFY_40MS_INSERT_YES|
	//HWL_CONST_PCR_VERIFY_40MS_INSERT_NO ]
} HWL_ChannelInputPcrMode_t;

S32 HWL_ChannelInputPcrModeSend( HWL_ChannelInputPcrMode_t *veryfyMode);



/************************************************************************************/
/*	20.输入流减少时FPGA报告CPU  */
/************************************************************************************/

typedef struct tagHWL_InputStreamNotifier
{
	U16	udpPort;
	U8   channelId;
	U8	program;

} HWL_InputStreamNotifier_t;


typedef struct tagHWL_InputStreamNotifierTable
{
	U8	physicChannel;
	HWL_InputStreamNotifier_t *streamTable;
	U32				streamSize;
} HWL_InputStreamNotifierTable_t;



S32 HWL_InputStreamNotifierTableGet( HWL_InputStreamNotifierTable_t *streamNotifierTable);



/************************************************************************************/
/*	21.输入端口号节目的内容有变化时报告CPU   */
/************************************************************************************/


typedef struct tagHWL_InputPortEsNotifier
{
	U8 idx;
	U8 esType;			// PES流的类型,与流中一样,
	U16 esPid;			//PES流的 PID ;如果无此PES,  ES_Type = 0xff    ES_PID = 0xffff
} HWL_InputPortEsNotifier_t;



typedef struct tagHWL_InputPortEsNotifierTable
{
	U8 physicChannel;
	HWL_InputStreamNotifier_t streamIdentifier;
	HWL_InputPortEsNotifier_t *esTable;
	U32				esTableSize;

} HWL_InputPortEsNotifierTable_t;


S32 HWL_InputPortEsNotifierTableGet( HWL_InputPortEsNotifierTable_t *esnotifier);




/************************************************************************************/
/*	22.1 映射源(输入逻辑通道和UDP端口)的设置 */
/************************************************************************************/

typedef struct tagHWL_PidMapMapOriginalPort
{
	HWL_IPAddressV4_t  inputIpAddress;
	U8			inputlogicChannelID;			//输入的IP逻辑通道号
	U8			udpPort;						// UDP端口
	U16			pidMapTableBeginIdx;				// PID映射表的起始位置
} HWL_PidMapMapOriginalPort_t;



typedef struct tagHWL_PidMapMapOriginalPortTable
{
	HWL_PidMapMapOriginalPort_t *pidMapPortTable;
	U32			idMapPortTableSize;
} HWL_PidMapMapOriginalPortTable_t;



S32 HWLPidMapMapOriginalPortTableSend( HWL_PidMapMapOriginalPortTable_t *pidMapPortTable);





/************************************************************************************/
/*	22.2 源PID到目标通道和目标PID映射的设置   */
/************************************************************************************/

typedef struct tagHWL_PidMapOriginalToDestined
{
	U16	originalPid;			//源PID.
	U8	destinedChannelID;	//目的通道号.
	U16	destinedPid;			//目的PID.
} HWL_PidMapOriginalToDestined_t;


typedef struct tagHWL_PidMapItemTable
{
	U8 flag;
	HWL_PidMapOriginalToDestined_t *pidItemTable;
	U32	pidItemSize;

} HWL_PidMapOriginalToDestinedTable_t;



S32 HWL_PidMapOriginalToDestinedTableSend( HWL_PidMapOriginalToDestinedTable_t *pidMapTable);


/************************************************************************************/
/* 	24.设置输出码率对应的96MHz的脉冲数(BL85KM模块输出需要),   */
/************************************************************************************/





typedef struct tagHWL_OutputRate96Mhz
{

	U8	modularStand;	//HWL_CONST_MODULAR_STAND_*
	U8	modularMode;	//HWL_CONST_MODULAR_MODE_*
	F64  symbolRate;		// etc:6.875..

} HWL_OutputRate96Mhz_t;


S32 HWL_OutputRate96MhzSend( HWL_OutputRate96Mhz_t *  );



/************************************************************************************/
/* 25.VOD模式下输入TS参数的设置: */
/************************************************************************************/




typedef struct tagHWL_VodInputTsParam
{
	U8 physicChannel;			// 输入TS的物理端口编号(0..N-1)
	U16 channelID	;			//输入列表中的序号(0..N-1)   (最大N为1023)(发送顺序为从0到N-1)
	HWL_IPAddressV4_t  inputIpAddress;	//输入TS的IP地址
	U16	inputPort;				//输入TS的UDP端口号.

	U8	vodOrMutor;				// Vod模式或复用模式.[HWL_CONST_VOD|HWL_CONST_MUTOR ]


	union
	{
		U16 	inputPortLogicNo;	//输入端口的逻辑编号 :
		struct
		{
			U8 outputQamChannelID;	//输出QAM通道编号(0..N-1)
			U8 outputQamProgramID;	//输出QAM通道中的节目编号(0..63)  如果 Num >= 64  为MPTS ,
		};
	} info;

} HWL_VodInputTsParam_t;

S32 HWL_VodInputTsParamSend( HWL_VodInputTsParam_t *vodInputTsParam  );


/************************************************************************************/
/* 26.1	与加密芯片的通讯 */
/************************************************************************************/


typedef struct tagHWL_EncryptChip
{

	U8	mainNo;
	U8	subNo;
	U8	length;
	U8 	buff[10];

} HWL_EncryptChip_t;



/************************************************************************************/
/* 25.1	VOD模式下 PAT版本和CRC32的设置 */
/************************************************************************************/

typedef struct tagHWL_VodPatAndCrcSet
{
	U8 	outputChannelId;
	U8 	outputProgram;
	U8 	patVersion;
	U32 patCrc32;
	U16 pmtPid;
	U8 	esNumber;
	U8 	pmtVersion;
	U32 pmtCrc32;

} HWL_VodPatAndCrcSet_t;


S32 HWL_VodPatAndCrcSetSend( HWL_VodPatAndCrcSet_t *vodPatCrc  );



/************************************************************************************/
/* 30. 清除功能模块的复位标志   */
/************************************************************************************/

typedef struct tagHWL_ModuleReset
{
	U8	moduleId;
} HWL_ModuleReset_t;

S32 HWL_ModuleResetSend( HWL_ModuleReset_t *moduleReset  );


/**	查询心跳信息,仅发送命令字给FPGA..*/
S32 HWL_HeatBeatSend();


/**	查询硬件状态信息命令发送 .*/
S32 HWL_PhyStatusQskSend();

#define HWL_CONST_PHY_ASI				0x0
#define HWL_CONST_PHY_1000MIP			0x01
#define HWL_CONST_PHY_100MIP			0x02
#define HWL_CONST_PHY_ESDS3				0x03
#define HWL_CONST_PHY_AD9789			0x04
#define HWL_CONST_PHY_USLOGIC			0x05
#define HWL_CONST_PHY_BOARD_ID0X6		0x06
#define HWL_CONST_PHY_BOARD_ID0X7		0X7
#define HWL_CONST_PHY_HARDVER			0X80
#define HWL_CONST_PHY_TONCYLOG			0X81

#define HWL_CONST_INPUT		0
#define HWL_CONST_OUTPUT	1

#define HWL_CONST_FPGA_DEV_PATH		"/dev/fpgaconfig"


#if 0

/**	PID 统计的端口设置.
*	@(inOrOut):输入或输出TS.{HWL_CONST_INPUT| HWL_CONST_OUTPUT }
*/
S32 HWL_PidStatisticSend(U8 inOrOut, U8 logicChannelId);


/**	PID 端口统计查询.*/
S32 HWL_PidStatisticSearch(U8 inOrOut);


/**	xxxSize和xxxItem函数用于循环遍历 .*/

/**	返回PID 端口统计数据中的条目个数
*	@(return):size.//
.*/
U32 HWL_PidStatisticArraySize();


/**	单条PID统计信息数据检索.
*	@(index): [0-size). size由HWL_PidStatisticArraySize()确定,
*	@(pidRate):输入缓冲.本函数将用实际数据填充此内存.
*/
S32 HWL_PidStatisticArrayItem(U32 index, HWL_PidRate_t *pidRate);

#endif



/**	获取PSI接受Buffer大小*/
S32 HWL_PSIBuffSizeGet(HWL_PSIBuffSize_t *psiBuff);




/************************************************************************************/
/*	芯片寄存器操作.	*/
/************************************************************************************/

/*	CPU对调制器有关的芯片的寄存器设置和读取: (PHY_Ch = 0)	*/
S32 HWL_ModularChipTableSend(HWL_ModularChipRegister_t *table, U32 size);


S32 HWL_FPGAWrite(U8 *Buff, U32 Size);
/************************************************************************************/
/**	PID映射 操作.*/
/************************************************************************************/



/**	以下三个函数提供 对PID映射表的操作 。
*	HWL_PidMapArrayClear...将内存中暂存的映射表清空。
*	HWL_PidMapArrayAppend...向内存中加入新的PID映射表项.		..MAX-4096
*	HWL_PidMapArrayApply..该函数会将内存中存储的PID映射表项排序，并发送给FPGA.
*/
void HWL_PidMapArrayClear();
S32  HWL_PidMapArrayAppend(HWL_PidMapItem_t *pidMap);
void HWL_PidMapArrayApply();




/************************************************************************************/
/**	PID搜索 操作.*/
/************************************************************************************/


/**	批量设置PID映射参数.
*	内部会修改@(pidMapArray).进行排序后发送到FPGA.
*/


/** 	PID搜索.
*	(@pidMap.inputLogicChannelId)开始。
*/
S32	HWL_PidSearchStart(U16 channelId, U16 pid, U8 filter, U8 FlagOrCount);

/**	PID搜索停止.*/
S32	HWL_PidSearchStop(U16 channelId, U16 pid, U8 filter);




/**	回调函数类型.用户可以直接处理缓冲区数据.*/
typedef  void (*HWL_CallBack_t)(U8 *buff, U32 bufflen);


/**	本模块初始化*/
void HWL_InterInit();


void __HWL_Pool(void *noused);

U32 HWL_ChannelRateArraySize();


/**查询单通道码率*/
U32 HWL_ChannelRateArrayItem(U32 index, HWL_ChannelRate_t *channelRate);





/**	命令字对象结构体.*/
typedef struct tagICPL_Cmd_t
{
	U8 tagid;
	U8 *buff;
	U32 buff_len;
	//mutex;

	void *data;
	U32 data_len;
	void (*perform)(struct tagICPL_Cmd_t *, void *data);
	HANDLE32 mutex;
	U8	length;
	void (*callback)(struct tagICPL_Cmd_t *, void *data);		//回调函数..
	U32 time;		//接收到的次数.
	U32 timeLimit;	//最大接收的次数.如果接收次数 time>=timeLimit,关于此TAGID的send操作将不执行.


	U32	m_RecvTick;
} ICPL_Cmd_t;



void ICPL_Cmd_Lock(ICPL_Cmd_t *cmd);
void ICPL_Cmd_UnLock(ICPL_Cmd_t *cmd);


typedef  void (*ICPL_CmdFun_t )(struct tagICPL_Cmd_t *, void *data);
typedef  void (*ICPL_CmdPickFun_t )(const struct tagICPL_Cmd_t *, void *data,  U32 );



typedef ICPL_Cmd_t ICPL_CmdRequest_t;
typedef ICPL_Cmd_t ICPL_CmdResponse_t;




/**	各种命令字预分配缓冲区长度.大部分情况用户不需要自己处理缓冲数据，先保留，可优化。*/
#define ICPL_CMD_HEAT_REQUEST_BUFF_LEN 			4
#define ICPL_CMD_HEAT_RESPONSE_BUFF_LEN 		0x0c
#define ICPL_CMD_DPB_TSINSERT_REQUEST_BUFF_LEN	 	22
#define ICPL_CMD_OUTTSPARAM_BUFF_LEN 			10
#define ICPL_CMD_PHYCHANNELPARAM_LEN			0x06
#define ICPL_CMD_PHY_REQUEST_BUFF_LEN			0x00

#define ICPL_CMD_PHYPARAM_REQUEST_BUFF_LEN 	22
#define ICPL_CMD_PIDMAP_REQUEST_BUFF_LEN		14
#define ICPL_CMD_PIDMAP_RESPONSE_SEARCH_BUFF_LEN		55

#define ICPL_CMD_PHY_RESPONSE_BUFF_LEN			100
#define ICPL_CMD_TS_RATE_REQUEST_BUFF_LEN		100


/**
适用对象行为:
CPU---->FPGA.

1. 	从表中查找命令对象实体.
2.	使用用用户自定义函数和参数填充对象实体中的命令体.
3.	适用DRL传输层传送到FPGA.

FPGA--->CPU.
1.	从FPGA中读取到数据。
2.	通过命令数据中的TAGID查找其控制对象实体.
3.	使用控制对象中的解析函数解析出数据内容为抽象结构体.
3.	使用抽象结构体操作XX.

*/

/* 静态对象 */

/*
typedef enum  tagICPL_ActionId
{
ICPL_ACTION_ID_HEATBEAT,
ICPL_ACTION_ID_OUTTSPARAM,
ICPL_ACTION_ID_PHYPARAM,
ICPL_ACTION_ID_PIDMAP,
}ICPL_ActionId_t;
*/


/**	使用本模块前必须要先调用此函数，完成映射表初始化等工作.	*/
void ICPL_CmdArrayInit();



/**	查找函数，通过tagID查找相应的处理体。仅用在模块内部使用。
*/
ICPL_Cmd_t *ICPL_CmdResponseFind(U8 tagID);



/**	提供自定义处理回复数据的操作.
*	可以用此函数注册每个命令字回复的处理函数。
*	@(tagID):命令字ID;
*	@(userfun):ICPL_CmdFun_t.必须是此类型。例如: ICPL_OPtorPidMapSearchDemo .
*	@(userdata):用户自定义数据。
*
*
*	当接收器根据tagID查找到处理体之后，会调用。
*	cmd->perform(cmd->userdata);
*	ICPL_CmdResponseOPtorSet所做的即是:
*			cmd->perform=userfun
*			cmd->data=userdata;
*/
ICPL_CmdFun_t ICPL_CmdResponseCallbackSet(U8  tagID, ICPL_CmdFun_t userfun);



#define ICPL_CheckOk		1
#define ICPL_CheckNo 		0
#define ICPL_CheckRange(a,b,c) 	((c)<=b)




void ICPL_Cmd_Lock(ICPL_Cmd_t *cmd);
void ICPL_Cmd_UnLock(ICPL_Cmd_t *cmd);




#define ICPL_CMD_IDX_TAGID		0
#define ICPL_CMD_IDX_LENGTH	 	1



/**	各种命令字预分配缓冲区长度.大部分情况用户不需要自己处理缓冲数据，先保留，可优化。*/
#define ICPL_CMD_HEAT_REQUEST_BUFF_LEN 			4
#define ICPL_CMD_HEAT_RESPONSE_BUFF_LEN 		0x0c
#define ICPL_CMD_DPB_TSINSERT_REQUEST_BUFF_LEN	 	22
#define ICPL_CMD_OUTTSPARAM_BUFF_LEN 			10
#define ICPL_CMD_PHYCHANNELPARAM_LEN			0x06
#define ICPL_CMD_PHY_REQUEST_BUFF_LEN			0x00

#define ICPL_CMD_PHYPARAM_REQUEST_BUFF_LEN 	22
#define ICPL_CMD_PIDMAP_REQUEST_BUFF_LEN		14
#define ICPL_CMD_PIDMAP_RESPONSE_SEARCH_BUFF_LEN		55

#define ICPL_CMD_PHY_RESPONSE_BUFF_LEN			100
#define ICPL_CMD_TS_RATE_REQUEST_BUFF_LEN		100


/**
适用对象行为:
CPU---->FPGA.

1. 	从表中查找命令对象实体.
2.	使用用用户自定义函数和参数填充对象实体中的命令体.
3.	适用DRL传输层传送到FPGA.

FPGA--->CPU.
1.	从FPGA中读取到数据。
2.	通过命令数据中的TAGID查找其控制对象实体.
3.	使用控制对象中的解析函数解析出数据内容为抽象结构体.
3.	使用抽象结构体操作XX.

*/

/* 静态对象 */


/**
*	大小端定义:
*	ICPL_BIG_ENDIAN:		大端,高位在低地址.
*	ICPL_LITTLE_ENDIAN:	小端，高位在高地址.
*/
#define ICPL_BIG_ENDIAN			0
#define ICPL_LITTLE_ENDIAN		1

/**	返回系统大小端编码方式.[ICPL_BIG_ENDIAN|ICPL_LITTLE_ENDIAN ].*/
S32	ICPL_CodeMode();



/**	初始化内部模块内存.*/
void ICPL_CodeInit();



#define ICPL_CODE_MODE_COPY	    0	//原样拷贝.
#define ICPL_CODE_MODE_U8		1	//类型转换
#define ICPL_CODE_MODE_U16		2	//类型转换
#define ICPL_CODE_MODE_U24		3
#define ICPL_CODE_MODE_U32		4	//类型转换



typedef struct
{
	void	*address;
	U32		m_StartIndex;		//开始下标.
	U32		m_DataSize;		//长度.
	U8		mode;			//
} ICPL_ByteInfo_t;



/**	解码.*/
void ICPL_ByteInfoDecode( ICPL_ByteInfo_t  *byteInfo, U32 size, U8 *buff, U32 buffLen  );

/**	编码.*/
void ICPL_ByteInfoEncode( ICPL_ByteInfo_t  *byteInfo, U32 size, U8 *buff, U32 buffLen  );


/**	回复操作解包函数组. */
S32 ICPL_OPtorHeatBeatParser(ICPL_CmdResponse_t  *cmd, HWL_HeatBeat_t *headbeat);
S32 ICPL_OPtorPhyRskResponseParser(ICPL_CmdRequest_t  *cmd, HWL_PhyStatusRskResponse_t *phyRsk);
S32 ICPL_OPtorChannelRateParser(ICPL_CmdRequest_t  *cmd, HWL_ChannelRateArray_t *);
S32 ICPL_OPtorStatisticsArrayParser(ICPL_CmdResponse_t  *cmd, HWL_PIDETHStatistcs *param);
//S32 ICPL_OPtorInputIPPortStatisticParser(ICPL_CmdRequest_t  *cmd, HWL_InputIPPortStatisticInfoArray_t *statistic);
S32 ICPL_OPtorPSIBuffSizeParser(ICPL_CmdRequest_t  *cmd, HWL_PSIBuffSize_t *psiBuff);
S32 ICPL_OPtorInputStreamNotifierTableParser(ICPL_CmdResponse_t  *cmd, HWL_InputStreamNotifierTable_t *table);
S32 ICPL_OPtorInputPortEsNotifierTableParser(ICPL_CmdResponse_t  *cmd, HWL_InputPortEsNotifierTable_t *table);
S32 ICPL_OPtorEncryptChipParser(ICPL_CmdRequest_t  *cmd, HWL_EncryptChip_t *encryptChip);


/** 请求操作打包函数组.	*/
S32 ICPL_OPtorHeatBeatMake(ICPL_CmdResponse_t  *cmd, HWL_HeatBeat_t *headbeat);
S32 ICPL_OPtorDPBTsInsertMake(ICPL_CmdRequest_t  *cmd, HWL_DPBTsInsert_t *param);
S32 ICPL_OPtorOutTsParamMake(ICPL_CmdResponse_t  *cmd, HWL_OutPutTsParam_t *param);
S32 ICPL_OPtorPhyChannelParamMake(ICPL_CmdResponse_t  *cmd, HWL_PhyChannelParam_t *param);
S32 ICPL_OPtorInputUdpPortMake(ICPL_CmdResponse_t  *cmd, HWL_InputUdpPort_t *param);
S32 ICPL_OPtorPhyQskMake(ICPL_CmdRequest_t  *cmd, HWL_PhyStatusQsk_t *phyQsk);
S32 ICPL_OPtorChannelRateInfoMake(ICPL_CmdRequest_t  *cmd, HWL_ChannelRateInfo_t *param);
S32 ICPL_OPtorPidStatisticMake(ICPL_CmdRequest_t  *cmd, HWL_PidStatics_t *pidSearch);
S32 ICPL_OPtorPidStatisticSearchMake(ICPL_CmdRequest_t  *cmd, HWL_PidStatics_t *pidSearch);
S32 ICPL_OPtorInputIPPortStatisticClearMake(ICPL_CmdRequest_t  *cmd, HWL_EthDetectionParam *statistic);
S32 ICPL_OPtorInputIPPortStatisticSendMake(ICPL_CmdRequest_t  *cmd, HWL_EthDetectionParam *statistic);
S32 ICPL_OPtorControlWordMake(ICPL_CmdRequest_t  *cmd,  HWL_ControlWord_t  *cw );
S32 ICPL_OPtorControlWordSwitchMake(ICPL_CmdRequest_t  *cmd,  HWL_ControlWord_t  *cw );
S32 ICPL_OPtorCondtionAccessMake(ICPL_CmdRequest_t  *cmd, HWL_ConditionAccess_t *access);
S32 ICPL_OPtorGroupBroadcastTableMake(ICPL_CmdRequest_t  *cmd,  HWL_GroupBroadcastTable_t  *groupcast );
S32 ICPL_OPtorPIDCopyOriginalTableMake(ICPL_CmdRequest_t  *cmd, HWL_PIDCopyOriginalTable_t *pidCopy);
S32 ICPL_OPtorPIDCopyDestinedTableMake(ICPL_CmdRequest_t  *cmd,  HWL_PIDCopyDestinedTable_t *pidTable);
S32 ICPL_OPtorModularChipRegisterTableMake(ICPL_CmdRequest_t  *cmd, HWL_ModularChipRegisterTable_t *registerTable);
S32 ICPL_OPtorChannelInputPcrModeMake(ICPL_CmdRequest_t  *cmd, HWL_ChannelInputPcrMode_t *veryfyMode);
S32 ICPL_OPtorPassModeMake(ICPL_CmdRequest_t  *cmd, HWL_PassMode_t *  );
S32 ICPL_OPtorVodPatAndCrcSetMake(ICPL_CmdRequest_t  *cmd, HWL_VodPatAndCrcSet_t *param);
S32 ICPL_OPtorDPBStableTsInsertMake(ICPL_CmdRequest_t  *cmd, HWL_DPBTsInserter *param);
S32 ICPL_OPtorDPBStableTsInsertClearMake(ICPL_CmdRequest_t  *cmd, HWL_DPBTsInserter *param);
S32 ICPL_OPtorModuleResetMake(ICPL_CmdRequest_t  *cmd, HWL_ModuleReset_t *moduleReset);
S32 ICPL_OPtorPidMapClearMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapItem_t *param);
S32 ICPL_OPtorPidMapOriginalToDestinedTableMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapOriginalToDestinedTable_t *pidMapTable);
S32 ICPL_OPtorOutputRate96MhzMake(ICPL_CmdRequest_t  *cmd, HWL_OutputRate96Mhz_t *rateSet);
S32 ICPL_OPtorPidMapSetMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapItem_t *param);
S32 ICPL_ReplicatePidMapSRCClearSetMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapItem_t *_param);
S32 ICPL_ReplicatePidMapDestClearSetMake(ICPL_CmdRequest_t  *cmd, HWL_PidMapItem_t *_param);
S32 ICPL_OPtorEncryptChipReadMake(ICPL_CmdRequest_t  *cmd, HWL_EncryptChip_t *encryptChip);
S32 ICPL_OPtorEncryptChipWriteMake(ICPL_CmdRequest_t  *cmd, HWL_EncryptChip_t *encryptChip);
S32 ICPL_OPtorVodInputTsParamMake(ICPL_CmdRequest_t  *cmd, HWL_VodInputTsParam_t *vodInputTsParam);
S32 ICPL_OPtorPidMapMapOriginalPortTableMake(ICPL_CmdRequest_t  *cmd, HWL_ModularChipRegisterTable_t *registerTable);


void ICPL_CmdArrayClear(void);

void HWL_RequestBuffInit(void);
void HWL_RequestBuffDestory(void);

void HWL_InterInit(void);
void HWL_InterDestory(void);

U8 HWLL_GetHardwareVersion(void);









void HWL_PlatformInfoSet(HWL_HWInfo * pHWInfo);


#endif
