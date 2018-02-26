#include "hisi_gpio.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* HI3531A GPIO Register */
#define HI3531A_GPIO_BASE (0x12150000)
#define HI3531A_GPIO_SIZE (0x10000) /* GPIO 寄存器的 Size */
#define HI3531A_GPIO_DATA_OFFSET (0x000)
#define HI3531A_GPIO_DIR_OFFSET (0x400)
#define HI3531A_IO_CONFIG_BASE (0x120F0000)
#define HI3531A_GPIO_NUM (25)
#define HI3531A_GPIO_PIN (8)

/* [0] reg address [1] mux gpio value */
#define NO_USE_ADDR (0xFFFFFFFF)
static const U32 sc_HI_GpioMapIOConfigReg[HI3531A_GPIO_NUM * HI3531A_GPIO_PIN][2] = {
	{0x120F01D4, 0}, /* GPIO0_0 */
	{0x120F01D8, 0}, /* GPIO0_1 */
	{0x120F01DC, 0}, /* GPIO0_2 */
	{0x120F01E0, 0}, /* GPIO0_3 */
	{0x120F01E4, 0}, /* GPIO0_4 */
	{0x120F01E8, 0}, /* GPIO0_5 */
	{0x120F01EC, 0}, /* GPIO0_6 */
	{0x120F01F0, 0}, /* GPIO0_7 */

	{0x120F0004, 1}, /* GPIO1_0 */
	{0x120F0008, 1}, /* GPIO1_1 */
	{0x120F000C, 1}, /* GPIO1_2 */
	{0x120F0010, 1}, /* GPIO1_3 */
	{0x120F0014, 1}, /* GPIO1_4 */
	{0x120F0018, 1}, /* GPIO1_5 */
	{0x120F001C, 1}, /* GPIO1_6 */
	{0x120F0020, 1}, /* GPIO1_7 */

	{0x120F0028, 1}, /* GPIO2_0 */
	{0x120F002C, 1}, /* GPIO2_1 */
	{0x120F0030, 1}, /* GPIO2_2 */
	{0x120F0034, 1}, /* GPIO2_3 */
	{0x120F0038, 1}, /* GPIO2_4 */
	{0x120F003C, 1}, /* GPIO2_5 */
	{0x120F0040, 1}, /* GPIO2_6 */
	{0x120F0044, 1}, /* GPIO2_7 */

	{0x120F0050, 1}, /* GPIO3_0 */
	{0x120F0054, 1}, /* GPIO3_1 */
	{0x120F0058, 1}, /* GPIO3_2 */
	{0x120F005C, 1}, /* GPIO3_3 */
	{0x120F0060, 1}, /* GPIO3_4 */
	{0x120F0064, 1}, /* GPIO3_5 */
	{0x120F0068, 1}, /* GPIO3_6 */
	{0x120F006C, 1}, /* GPIO3_7 */

	{0x120F0074, 1}, /* GPIO4_0 */
	{0x120F0078, 1}, /* GPIO4_1 */
	{0x120F007C, 1}, /* GPIO4_2 */
	{0x120F0080, 1}, /* GPIO4_3 */
	{0x120F0084, 1}, /* GPIO4_4 */
	{0x120F0088, 1}, /* GPIO4_5 */
	{0x120F008C, 1}, /* GPIO4_6 */
	{0x120F0090, 1}, /* GPIO4_7 */

	{0x120F009C, 1}, /* GPIO5_0 */
	{0x120F00A0, 1}, /* GPIO5_1 */
	{0x120F00A4, 1}, /* GPIO5_2 */
	{0x120F00A8, 1}, /* GPIO5_3 */
	{0x120F00AC, 1}, /* GPIO5_4 */
	{0x120F00B0, 1}, /* GPIO5_5 */
	{0x120F00B4, 1}, /* GPIO5_6 */
	{0x120F00B8, 1}, /* GPIO5_7 */

	{0x120F00C0, 1}, /* GPIO6_0 */
	{0x120F00C4, 1}, /* GPIO6_1 */
	{0x120F00C8, 1}, /* GPIO6_2 */
	{0x120F00CC, 1}, /* GPIO6_3 */
	{0x120F00D0, 1}, /* GPIO6_4 */
	{0x120F00D4, 1}, /* GPIO6_5 */
	{0x120F00D8, 1}, /* GPIO6_6 */
	{0x120F00DC, 1}, /* GPIO6_7 */

	{0x120F00E8, 1}, /* GPIO7_0 */
	{0x120F00EC, 1}, /* GPIO7_1 */
	{0x120F00F0, 1}, /* GPIO7_2 */
	{0x120F00F4, 1}, /* GPIO7_3 */
	{0x120F00F8, 1}, /* GPIO7_4 */
	{0x120F00FC, 1}, /* GPIO7_5 */
	{0x120F0100, 1}, /* GPIO7_6 */
	{0x120F0104, 1}, /* GPIO7_7 */

	{0x120F010C, 1}, /* GPIO8_0 */
	{0x120F0110, 1}, /* GPIO8_1 */
	{0x120F0114, 1}, /* GPIO8_2 */
	{0x120F0118, 1}, /* GPIO8_3 */
	{0x120F011C, 1}, /* GPIO8_4 */
	{0x120F0120, 1}, /* GPIO8_5 */
	{0x120F0124, 1}, /* GPIO8_6 */
	{0x120F0128, 1}, /* GPIO8_7 */

	{0x120F0158, 0}, /* GPIO9_0 */
	{0x120F015C, 0}, /* GPIO9_1 */
	{0x120F0160, 0}, /* GPIO9_2 */
	{0x120F0164, 0}, /* GPIO9_3 */
	{0x120F0168, 0}, /* GPIO9_4 */
	{0x120F016C, 0}, /* GPIO9_5 */
	{0x120F0170, 0}, /* GPIO9_6 */
	{0x120F0174, 0}, /* GPIO9_7 */

	{0x120F0178, 0}, /* GPIO10_0 */
	{0x120F017C, 0}, /* GPIO10_1 */
	{0x120F0180, 0}, /* GPIO10_2 */
	{0x120F0184, 0}, /* GPIO10_3 */
	{0x120F0188, 0}, /* GPIO10_4 */
	{0x120F018C, 0}, /* GPIO10_5 */
	{0x120F0190, 0}, /* GPIO10_6 */
	{0x120F0194, 0}, /* GPIO10_7 */

	{0x120F0130, 0}, /* GPIO11_0 */
	{0x120F0134, 0}, /* GPIO11_1 */
	{0x120F0138, 0}, /* GPIO11_2 */
	{0x120F013C, 0}, /* GPIO11_3 */
	{0x120F0140, 0}, /* GPIO11_4 */
	{0x120F0144, 0}, /* GPIO11_5 */
	{0x120F01AC, 0}, /* GPIO11_6 */
	{0x120F01B0, 0}, /* GPIO11_7 */

	{0x120F01B4, 0}, /* GPIO12_0 */
	{0x120F00E0, 0}, /* GPIO12_1 */
	{0x120F00E4, 1}, /* GPIO12_2 */
	{0x120F01B8, 0}, /* GPIO12_3 */
	{0x120F01BC, 0}, /* GPIO12_4 */
	{0x120F0198, 0}, /* GPIO12_5 */
	{0x120F019C, 0}, /* GPIO12_6 */
	{0x120F01A0, 0}, /* GPIO12_7 */

	{0x120F0148, 0}, /* GPIO13_0 */
	{0x120F014C, 3}, /* GPIO13_1 */
	{0x120F0150, 0}, /* GPIO13_2 */
	{0x120F02C0, 2}, /* GPIO13_3 */
	{0x120F02C4, 2}, /* GPIO13_4 */
	{0x120F02C8, 2}, /* GPIO13_5 */
	{0x120F02CC, 2}, /* GPIO13_6 */
	{0x120F01F4, 0}, /* GPIO13_7 */

	{0x120F02D4, 0}, /* GPIO14_0 */
	{0x120F02D8, 0}, /* GPIO14_1 */
	{0x120F02DC, 0}, /* GPIO14_2 */
	{0x120F02E0, 0}, /* GPIO14_3 */
	{0x120F02E4, 0}, /* GPIO14_4 */
	{0x120F02E8, 0}, /* GPIO14_5 */
	{0x120F02EC, 0}, /* GPIO14_6 */
	{0x120F02F0, 0}, /* GPIO14_7 */

	{0x120F02F4, 0}, /* GPIO15_0 */
	{0x120F02F8, 0}, /* GPIO15_1 */
	{0x120F02FC, 0}, /* GPIO15_2 */
	{0x120F0300, 0}, /* GPIO15_3 */
	{0x120F01C0, 1}, /* GPIO15_4 */
	{0x120F01C4, 1}, /* GPIO15_5 */
	{0x120F01C8, 1}, /* GPIO15_6 */
	{0x120F0108, 1}, /* GPIO15_7 */

	{0x120F0220, 0}, /* GPIO16_0 */
	{0x120F0224, 0}, /* GPIO16_1 */
	{0x120F0228, 0}, /* GPIO16_2 */
	{0x120F022C, 0}, /* GPIO16_3 */
	{0x120F0230, 0}, /* GPIO16_4 */
	{0x120F0234, 0}, /* GPIO16_5 */
	{0x120F0238, 0}, /* GPIO16_6 */
	{0x120F023C, 0}, /* GPIO16_7 */

	{0x120F0240, 0}, /* GPIO17_0 */
	{0x120F0244, 0}, /* GPIO17_1 */
	{0x120F0248, 0}, /* GPIO17_2 */
	{0x120F024C, 0}, /* GPIO17_3 */
	{0x120F0250, 0}, /* GPIO17_4 */
	{0x120F0254, 0}, /* GPIO17_5 */
	{0x120F0258, 0}, /* GPIO17_6 */
	{0x120F025C, 0}, /* GPIO17_7 */

	{0x120F01F8, 2}, /* GPIO18_0 */
	{0x120F01FC, 2}, /* GPIO18_1 */
	{0x120F0200, 0}, /* GPIO18_2 */
	{0x120F0204, 0}, /* GPIO18_3 */
	{0x120F0208, 0}, /* GPIO18_4 */
	{0x120F020C, 0}, /* GPIO18_5 */
	{0x120F0210, 0}, /* GPIO18_6 */
	{0x120F0214, 0}, /* GPIO18_7 */

	{0x120F0274, 1}, /* GPIO19_0 */
	{0x120F0278, 1}, /* GPIO19_1 */
	{0x120F0280, 1}, /* GPIO19_2 */
	{0x120F0284, 1}, /* GPIO19_3 */
	{0x120F0288, 1}, /* GPIO19_4 */
	{0x120F028C, 1}, /* GPIO19_5 */
	{0x120F01CC, 0}, /* GPIO19_6 */
	{0x120F01D0, 0}, /* GPIO19_7 */

	{NO_USE_ADDR, 0}, /* GPIO20_0 */
	{NO_USE_ADDR, 0}, /* GPIO20_1 */
	{NO_USE_ADDR, 0}, /* GPIO20_2 */
	{NO_USE_ADDR, 0}, /* GPIO20_3 */
	{NO_USE_ADDR, 0}, /* GPIO20_4 */
	{0x120F0270, 0}, /* GPIO20_5 */
	{0x120F012C, 0}, /* GPIO20_6 */
	{0x120F0154, 0}, /* GPIO20_7 */

	{0x120F0000, 1}, /* GPIO21_0 */
	{0x120F0024, 1}, /* GPIO21_1 */
	{0x120F0048, 0}, /* GPIO21_2 */
	{0x120F004C, 1}, /* GPIO21_3 */
	{0x120F0070, 1}, /* GPIO21_4 */
	{0x120F0094, 0}, /* GPIO21_5 */
	{0x120F0098, 1}, /* GPIO21_6 */
	{0x120F00BC, 1}, /* GPIO21_7 */

	{0x120F0290, 2}, /* GPIO22_0 */
	{0x120F0294, 2}, /* GPIO22_1 */
	{0x120F0298, 2}, /* GPIO22_2 */
	{0x120F029C, 2}, /* GPIO22_3 */
	{0x120F02A0, 2}, /* GPIO22_4 */
	{0x120F02A4, 2}, /* GPIO22_5 */
	{0x120F02A8, 2}, /* GPIO22_6 */
	{0x120F02AC, 2}, /* GPIO22_7 */

	{0x120F02D0, 0}, /* GPIO23_0 */
	{0x120F02B0, 2}, /* GPIO23_1 */
	{0x120F02B4, 2}, /* GPIO23_2 */
	{0x120F02B8, 2}, /* GPIO23_3 */
	{0x120F02BC, 2}, /* GPIO23_4 */
	{0x120F026C, 0}, /* GPIO23_5 */
	{0x120F0218, 0}, /* GPIO23_6 */
	{0x120F021C, 0}, /* GPIO23_7 */

	{0x120F01A4, 0}, /* GPIO24_0 */
	{0x120F01A8, 0}, /* GPIO24_1 */
	{0x120F0260, 0}, /* GPIO24_2 */
	{0x120F0264, 0}, /* GPIO24_3 */
	{0x120F0268, 0}, /* GPIO24_4 */
	{0x120F027C, 1}, /* GPIO24_5 */
	{NO_USE_ADDR, 0}, /* GPIO24_6 */
	{NO_USE_ADDR, 0}, /* GPIO24_7 */
};

typedef struct tag_MMAP_Node
{
	unsigned int Start_P;
	unsigned int Start_V;
	unsigned int length;
	unsigned int refcount;  /* map后的空间段的引用计数 */
	struct tag_MMAP_Node * next;
}TMMAP_Node_t;

TMMAP_Node_t * pTMMAPNode = NULL;

#define PAGE_SIZE 0x1000
#define PAGE_SIZE_MASK 0xfffff000

static int fd = -1;
static const char dev[]="/dev/mem";

/* no need considering page_size of 4K */
void *memmap(U32 phy_addr, U32 size)
{
	U32 phy_addr_in_page;
	U32 page_diff;

	U32 size_in_page;

	TMMAP_Node_t * pTmp;
	TMMAP_Node_t * pNew;

	void *addr=NULL;

	if(size == 0) {
		GLOBAL_TRACE(("memmap():size can't be zero!\n"));
		return NULL;
	}

	/* check if the physical memory space have been mmaped */
	pTmp = pTMMAPNode;
	while(pTmp != NULL) {
		if((phy_addr >= pTmp->Start_P) && 
			((phy_addr + size) <= (pTmp->Start_P + pTmp->length)))
		{
			pTmp->refcount++;   /* reference count increase by 1  */
			return (void *)(pTmp->Start_V + phy_addr - pTmp->Start_P);
		}

		pTmp = pTmp->next;
	}

	/* not mmaped yet */
	if(fd < 0) {
		/* dev not opened yet, so open it */
		fd = open (dev, O_RDWR | O_SYNC);
		if (fd < 0) {
			GLOBAL_TRACE(("memmap():open %s error!\n", dev));
			return NULL;
		}
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
	page_diff = phy_addr - phy_addr_in_page;

	/* size in page_size */
	size_in_page =((size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;

	addr = mmap((void *)0, size_in_page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
	if (addr == MAP_FAILED) {
		GLOBAL_TRACE(("memmap():mmap @ 0x%x error!\n", phy_addr_in_page));
		return NULL;
	}

	/* add this mmap to MMAP Node */
	pNew = (TMMAP_Node_t *)malloc(sizeof(TMMAP_Node_t));
	if(NULL == pNew) {
		GLOBAL_TRACE(("memmap():malloc new node failed!\n"));
		return NULL;
	}
	pNew->Start_P = phy_addr_in_page;
	pNew->Start_V = (unsigned int)addr;
	pNew->length = size_in_page;
	pNew->refcount = 1;
	pNew->next = NULL;

	if(pTMMAPNode == NULL) {
		pTMMAPNode = pNew;
	}
	else {
		pTmp = pTMMAPNode;
		while(pTmp->next != NULL) {
			pTmp = pTmp->next;
		}

		pTmp->next = pNew;
	}

	return (void *)(addr+page_diff);
}

#define DEFAULT_MD_LEN 128
static void HI_MemSet(U32 Addr, U32 Value)
{
	void *pMem  = NULL;

	//GLOBAL_TRACE(("reg[0x%08x] val[0x%x]\n", Addr, Value));
	pMem = memmap(Addr, DEFAULT_MD_LEN);
	if (pMem) {
		*(U32*)pMem = Value;
	}
}

static U32 HI_MemGet(U32 Addr)
{
	void *pMem  = NULL;

	pMem = memmap(Addr, DEFAULT_MD_LEN);
	if (pMem) {
		return *(U32*)pMem;
	}

	return 0;
}

void HI_GpioSetValue(HI_GpioIndex PortInd, HI_GpioPin Pin, S32 Value)
{
	/* 
	当 GPIO_DIR相应的比特配置为输入时，有效读取的结果将返回管脚的值；当配置为
	输出的时候，有效读取的结果将返回写入的值。 
	GPIO_DATA寄存器利用PADDR[9:2]实现了读写寄存器比特的屏蔽操作。该寄存器对
	应 256个地址空间。PADDR[9:2]分别对应 GPIO_DATA[7:0]，当相应的bit 为高时，则
	可以对相应的位进行读写操作；反之，若对应 bit 为低则不能进行操作。例如： 
	 若地址为 0x3FC（0b11_1111_1100） ，则对GPIO_DATA[7:0]这 8bit 操作全部有效。 
	 若地址为 0x200（0b10_0000_0000） ，则仅对GPIO_DATA[7]的操作有效。
	*/

	U32 lValue = 0;
	U32 lAddr;

	if (PortInd >= HI3531A_GPIO_NUM) {
		GLOBAL_TRACE(("GPIO Index Error, max: %d, input: %d\n", HI3531A_GPIO_NUM - 1, PortInd));
		return;
	}

	if (Pin >= HI3531A_GPIO_PIN) {
		GLOBAL_TRACE(("Pin Index Error, max: %d, input: %d\n", HI3531A_GPIO_PIN - 1, Pin));
		return;
	}

	if (lValue != 0 && lValue != 1) {
		GLOBAL_TRACE(("Pin Value Error, valid: 0/1, input: %d\n", lValue));
		return;
	}
	
	lAddr = HI3531A_GPIO_BASE + HI3531A_GPIO_SIZE * PortInd + HI3531A_GPIO_DATA_OFFSET + (0x3FF & ((1 << Pin) << 2));
	lValue = HI_MemGet(lAddr);
	lValue = (lValue & ~(1 << Pin) | (Value << Pin)) & 0xFF;
	HI_MemSet(lAddr, lValue);
}

S32 HI_GpioGetValue(HI_GpioIndex PortInd, HI_GpioPin Pin)
{
	U32 lValue = 0;
	U32 lAddr;

	if (PortInd >= HI3531A_GPIO_NUM) {
		GLOBAL_TRACE(("GPIO Index Error, max: %d, input: %d\n", HI3531A_GPIO_NUM - 1, PortInd));
		return;
	}

	if (Pin >= HI3531A_GPIO_PIN) {
		GLOBAL_TRACE(("Pin Index Error, max: %d, input: %d\n", HI3531A_GPIO_PIN - 1, Pin));
		return;
	}

	lAddr = HI3531A_GPIO_BASE + HI3531A_GPIO_SIZE * PortInd + HI3531A_GPIO_DATA_OFFSET + (0x3FF & ((1 << Pin) << 2));
	lValue = HI_MemGet(lAddr);

	return (((lValue & (1 << Pin)) & 0xFF) ? 1 : 0);
}

void HI_GpioSetup(HI_GpioIndex PortInd, HI_GpioPin Pin, BOOL IsInput, BOOL IsPullUp)
{
	U32 lValue = 0;

	if (PortInd >= HI3531A_GPIO_NUM) {
		GLOBAL_TRACE(("GPIO Index Error, max: %d, input: %d\n", HI3531A_GPIO_NUM - 1, PortInd));
		return;
	}

	if (Pin >= HI3531A_GPIO_PIN) {
		GLOBAL_TRACE(("Pin Index Error, max: %d, input: %d\n", HI3531A_GPIO_PIN - 1, Pin));
		return;
	}

	/* 设置复用寄存器为 GPIO 模式 */
	HI_MemSet(sc_HI_GpioMapIOConfigReg[PortInd * HI3531A_GPIO_PIN + Pin][0], sc_HI_GpioMapIOConfigReg[PortInd * HI3531A_GPIO_PIN + Pin][1]);

	/* 设置输入输出 */
	lValue = HI_MemGet(HI3531A_GPIO_BASE + HI3531A_GPIO_SIZE * PortInd + HI3531A_GPIO_DIR_OFFSET);
	lValue = IsInput ? (lValue & ~(1 << Pin)) : (lValue | (1 << Pin));
	HI_MemSet(HI3531A_GPIO_BASE + HI3531A_GPIO_SIZE * PortInd + HI3531A_GPIO_DIR_OFFSET, lValue);
}

/* EOF */
