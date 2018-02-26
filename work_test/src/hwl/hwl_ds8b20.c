#include "multi_private.h"
#include "hwl_ds8b20.h"

#define TEMPERATURE_INIT_VALUE 0
#define DS18B20_DEV_NAME "/dev/ds18b20_driver"

static int s_Ds18b20Fd = -1;
U8 s_Temperature;

void Hwl_Ds18b20Init(void)
{
	int tmp;
	s_Temperature = TEMPERATURE_INIT_VALUE;//�¶ȳ�ֵΪ0
	s_Ds18b20Fd = open(DS18B20_DEV_NAME, O_RDWR | O_NDELAY | O_NOCTTY);
	if( s_Ds18b20Fd < 0)
	{
		GLOBAL_TRACE((" =================================== Open %s  fail!", DS18B20_DEV_NAME));
	}
	else
	{
		read(s_Ds18b20Fd, &tmp, 4);
		GLOBAL_TRACE((" =================================== Open ds18b20 ok.\n"));
	}
}

void Hwl_Ds18b20Close(void)
{
	close(s_Ds18b20Fd)	;
}

U8 Hwl_Ds18b20Read(void)
{
	int tmp;
	read(s_Ds18b20Fd, &tmp, sizeof(tmp));
	
	s_Temperature = tmp/10000;

	//GLOBAL_TRACE(("Ds18b20 Temperature is %d\n",s_Temperature));

	return s_Temperature;
}

























