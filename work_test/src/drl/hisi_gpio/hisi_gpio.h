#ifndef HISI_GPIO_H
#define HISI_GPIO_H

#include "global_def.h"
#include "platform_assist.h"

typedef enum {
	HI_GPIO0,
	HI_GPIO1,
	HI_GPIO2,
	HI_GPIO3,
	HI_GPIO4,
	HI_GPIO5,
	HI_GPIO6,
	HI_GPIO7,
	HI_GPIO8,
	HI_GPIO9,
	HI_GPIO10,
	HI_GPIO11,
	HI_GPIO12,
	HI_GPIO13,
	HI_GPIO14,
	HI_GPIO15,
	HI_GPIO16,
	HI_GPIO17,
	HI_GPIO18,
	HI_GPIO19,
	HI_GPIO20,
	HI_GPIO21,
	HI_GPIO22,
	HI_GPIO23,
	HI_GPIO24
} HI_GpioIndex;

typedef enum {
	HI_PIN0,
	HI_PIN1,
	HI_PIN2,
	HI_PIN3,
	HI_PIN4,
	HI_PIN5,
	HI_PIN6,
	HI_PIN7
} HI_GpioPin;

void HI_GpioSetValue(HI_GpioIndex PortInd, HI_GpioPin Pin, S32 Value);
S32 HI_GpioGetValue(HI_GpioIndex PortInd, HI_GpioPin Pin);
void HI_GpioSetup(HI_GpioIndex PortInd, HI_GpioPin Pin, BOOL IsInput, BOOL IsPullUp);

#endif /* HISI_GPIO_H */
