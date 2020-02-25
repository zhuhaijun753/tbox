
/********************
**	�����ⲿ�����Դ��ѹ
**	����һλ��ЧС��
************************/

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "data_type.h"
#include "fr_drv_adc.h"
#include "fr_drv_gpio.h"


#define	AD_CH_NUM   3


static unsigned int 						AdcValueTmp[AD_CH_NUM][5];

static unsigned int 						AdcValueSum[AD_CH_NUM];

static unsigned char 						AdcCounter;		


static struct in_info_str 			in_info;           //

struct run_data_str							run_data;          //�������е�����


/*************************
**	
**************************/


void process_in(void)
{
	unsigned char i,j;
	
	for(i = 0;i < AD_CH_NUM;i++)
	{
		AdcValueTmp[i][AdcCounter] = fr_read_adc_value(i);
	}
	AdcCounter++;
	if(AdcCounter >= 5)
			AdcCounter = 0;
	for(i =0;i < AD_CH_NUM;i++)
	{
		for(j = 0;j < 5;j++)
			AdcValueSum[i] += AdcValueTmp[i][j];
	}
	
	AdcValueSum[0]			= AdcValueSum[0] / 5;    				//��Դ��ѹ
	AdcValueSum[1]			= AdcValueSum[1] / 5;						//��Ƭ���¶�
	AdcValueSum[2] 		  = AdcValueSum[2] / 5;						//��ص�ѹ
	
	in_info.power_vol = AdcValueSum[0] * 3.3 / 4095.0 * 480;    		//�ⲿ�����ѹ
	in_info.battery_vol = AdcValueSum[1] * 3.3 / 4095.0 * 20; 			//��ص�ѹ
	in_info.mcu_temp = AdcValueSum[2];																//��Ƭ���¶�  
	in_info.acc_state = fr_read_gpio_state(PORT_GPIO_ACC, PIN_GPIO_ACC); 
	in_info.shell_state = 0;
	in_info.gnss_ant_state = 0;
	
	memset((unsigned char *)&AdcValueSum,0,sizeof(AdcValueSum));
}


/********************
**	�����ⲿ�����Դ��ѹ
**	����һλ��ЧС��
************************/

unsigned short int read_power_vol(void)
{
	return in_info.power_vol;
}



/********************
**	���ص�ص�ѹ��
**	����һλ��ЧС����
************************/

unsigned short int read_batter_vol(void)
{
	return in_info.battery_vol;
}



/********************
**	ACC״̬
************************/
unsigned char read_acc_state(void)
{
	return in_info.acc_state;
}



/********************
**	���״̬
********************/

unsigned char read_shell_state(void)
{
	return in_info.shell_state;
}


/********************
**	����״̬
************************/
unsigned char read_ant_state(void)
{
	return in_info.gnss_ant_state;
}


/********************
**	���ص�ص�ѹ��
**	����һλ��ЧС����
************************/

void task_in(void *data)
{
	data = data;
	
	fr_init_adc();
	
	for(;;)
	{
		process_in();
		vTaskDelay(10);
	}
}



/***************************
**	
****************************/

void task_(void *data)
{
	data = data;
	
	for(;;)
	{
		
	}
}





