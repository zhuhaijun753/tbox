/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 * 2013-11-15     bright       add RCC initial and print RCC freq function
 */


/*
����ʱ�����̣�
1����RCC�Ĵ�����������ΪĬ��ֵ  RCC_DeInit
2�����ⲿ����ʱ�Ӿ���HSE    RCC_HSEConfig(RCC_HSE_ON);
3���ȴ��ⲿ����ʱ�Ӿ�����    HSEStartUpStatus = RCC_WaitForHSEStartUp();
4������PLL              RCC_PLLConfig
5����PLL              RCC_PLLCmd(ENABLE);
6������ϵͳʱ��          RCC_SYSCLKConfig
7������AHBʱ��           RCC_HCLKConfig;
8�����õ�����AHBʱ��      RCC_PCLK1Config
9�����ø���AHBʱ��        RCC_PCLK2Config;
10���ȴ�PLL����          while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) 
11���ж��Ƿ�PLL��ϵͳʱ��  while(RCC_GetSYSCLKSource() != 0x08)
12����Ҫʹ�õ�����ʱ��   RCC_APB2PeriphClockCmd()/RCC_APB1PeriphClockCmd����
��������������������������������
��Ȩ����������ΪCSDN������Ocarina_123����ԭ�����£���ѭ CC 4.0 BY-SA ��ȨЭ�飬ת���븽��ԭ�ĳ������Ӽ���������
ԭ�����ӣ�https://blog.csdn.net/u010659754/article/details/88933141

*/



/*

void HSE_sysclock_config( uint32_t  RCC_PLLMul_x )
{
    RCC_DeInit();    //�ȸ�λRCC�Ĵ���
    RCC_HSEConfig( RCC_HSE_ON );    //ʹ��HSE
    //���HSE�Ƿ������ɹ�
    if ( SUCCESS == RCC_WaitForHSEStartUp() )
    {
        //ʹ��Ԥȡָ������FLASH�̼��еĺ���
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        //����FLASH�ȴ����ڡ�  ��Ϊ��Ƶ��72M ���Եȴ��������ڡ�
        FLASH_SetLatency(FLASH_Latency_2);  
        //�����������ߵı�Ƶ����
        //HCLK --> AHB ���Ϊ72M������ֻ��Ҫ1��Ƶ
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        //PCLK1 --> APB1 ���Ϊ36M������Ҫ2��Ƶ
		RCC_PCLK1Config(RCC_HCLK_Div2);
        //PCLK2 --> APB2 ���Ϊ72M������ֻ��Ҫ1��Ƶ
		RCC_PCLK2Config(RCC_HCLK_Div1);
        //���������໷ PLLCLK = HSE * ��Ƶ����
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_x);
        RCC_PLLCmd(ENABLE);        //ʹ��PLL
        while ( RESET == RCC_GetFlagStatus(RCC_FLAG_PLLRDY) );  //�ȴ�PLL�ȶ�
        //ѡ��ϵͳʱ�ӣ�ѡ�����໷�����
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        //�ȴ�ѡ���ȶ�
        while ( 0x08 != RCC_GetSYSCLKSource() );
    }
    else
    {
        //HSE����ʧ��
    }

}
��������������������������������
��Ȩ����������ΪCSDN������Ocarina_123����ԭ�����£���ѭ CC 4.0 BY-SA ��ȨЭ�飬ת���븽��ԭ�ĳ������Ӽ���������
ԭ�����ӣ�https://blog.csdn.net/u010659754/article/details/88933141


*/



#define STM32_ALARM_TIMEOUT 20



#include "FreeRTOS.h"           //ע��
#include "stm32f10x.h"
#include "board.h"
#include "fr_drv_uart.h"
#include "fr_drv_gpio.h"
#include "fr_drv_timer.h"
#include "fr_drv_rtc.h"
#include "fr_drv_mem.h"

/**
 * @addtogroup STM32
 */

/*@{*/


/*************************************
**	�����ʱ
**************************************/

void delay_ms(unsigned int delayms)
{
    delayms = delayms > 10000UL ? 10000UL : delayms;

    delayms = 8000 * delayms;
    while(delayms--)
    {
        for(; delayms > 0; delayms--)
            __nop();
    }
}



/********************************
**	�������ƣ�uint8 WdtInit(INT16U time_ms)
**	������������ʼ�����Ź���CPU���ã��򿪿��Ź�uint8 WdtInit(INT16U time_ms)
*********************************/

unsigned char fr_init_wdt(unsigned short int sec)
{
	float tmp;
	
	if(sec > 25)
		sec = 0;
	
	sec *= 1000;
	
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
      RCC_ClearFlag();/* ���IWDG��λ,�����λ��־*/
    

  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);/*д��0x5555,ȥ��д����*/

  IWDG_SetPrescaler(IWDG_Prescaler_256);/*����Ԥ��ƵֵΪ256 �� 40KHz(LSI) / 256 = 156.25Hz, (6.4ms)*/
    /*����Ĵ�����ֵ0x000-0xfff*/
	
	tmp = (sec - 6.4) / 6.4;
	
	sec = (unsigned short int) tmp;
	
  IWDG_SetReload(sec);/*���װ�ؼĴ���ֵΪ:4000 ��ǰ�������Ź������ʱ��Ϊ(4000+1)*6.4 =25.6064s*/
  IWDG_ReloadCounter();/*д��0xAAAAι��һ��*/
  IWDG_Enable();	/*д��0xCCCC����������*/
		
	return 0;
}





/********************
**	��������:
**	��������:
*********************/
void fr_init_hse_72mhz(void)
{
	RCC_DeInit();                                          //��ʼ��Ϊȱʡֵ
  RCC_HSEConfig(RCC_HSE_ON);                             //ʹ���ⲿ�ĸ���ʱ��
  while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) {}; //�ȴ��ⲿ����ʱ��ʹ�ܾ���
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);  //ʹ��Ԥȡָ������FLASH�̼��еĺ���
	FLASH_SetLatency(FLASH_Latency_2);    //����FLASH�ȴ����ڡ�  ��Ϊ��Ƶ��72M ���Եȴ��������ڡ�
		// 8M/2*16=64M RCC_PLLMul_x = RCC_PLLMul_16
  RCC_HCLKConfig(RCC_SYSCLK_Div1);                       //���� AHB����ʱ�ӵ���ϵͳʱ��
  RCC_PCLK2Config(RCC_HCLK_Div1);                        //���� APB2ʱ�ӵ���ϵͳʱ��
  RCC_PCLK1Config(RCC_HCLK_Div2);                        //����APB1ʱ�ӵ���ϵͳʱ��/2
  RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);   //�������໷��Ƶֵ PLLCLK = 8MHZ * 9 =72MHZ
  RCC_PLLCmd(ENABLE);                                    //�������໷

  while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}; //�ȴ����໷�������?  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);             //Select PLL as system clock
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);             //Select PLL as system clock
	while(RCC_GetSYSCLKSource() != 0x08);                  //Wait till PLL is used as system clock source	
}




/***********************
**	��������:
**	��������:
************************/
void fr_init_hse_8mhz(void)
{
    RCC_DeInit();                                          			//��ʼ��Ϊȱʡֵ
    RCC_HSEConfig(RCC_HSE_ON);                             			//ʹ���ⲿ�ĸ���ʱ��
    while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) {}; 			//�ȴ��ⲿ����ʱ��ʹ�ܾ���
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);  //ʹ��Ԥȡָ������FLASH�̼��еĺ���
		FLASH_SetLatency(FLASH_Latency_0);    //����FLASH�ȴ����ڡ�  ��Ϊ��Ƶ��72M ���Եȴ��������ڡ�	
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                       			//���� AHB����ʱ�ӵ���ϵͳʱ��
    RCC_PCLK2Config(RCC_HCLK_Div1);                        			//���� APB2ʱ�ӵ���ϵͳʱ��
    RCC_PCLK1Config(RCC_HCLK_Div2);                        			//����APB1ʱ�ӵ���ϵͳʱ��/2
		RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);  	
			
    while(RCC_GetSYSCLKSource() != 0x04);                  			//Wait till PLL is used as system clock source 
		
}



/******************************
**
*********************************/

void fr_init_hsi_64mhz(void)
{
	__IO uint32_t HSIStatus = 0;
    
  RCC_DeInit();    //�ȸ�λRCC�Ĵ���
    
  RCC_HSICmd(ENABLE);   //ʹ��HSI
    
  HSIStatus = RCC->CR & RCC_CR_HSIRDY;    //���HSE�Ƿ������ɹ�
    
	if ( RCC_CR_HSIRDY == HSIStatus )
  {
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);  //ʹ��Ԥȡָ������FLASH�̼��еĺ���
		FLASH_SetLatency(FLASH_Latency_2);    //����FLASH�ȴ����ڡ�  ��Ϊ��Ƶ��72M ���Եȴ��������ڡ�
		// 8M/2*16=64M RCC_PLLMul_x = RCC_PLLMul_16
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16); //���������໷ PLLCLK = HSI * ��Ƶ����
		//�����������ߵı�Ƶ����
		// HCLK = SYSCLK AHB 36M
		RCC_HCLKConfig(RCC_SYSCLK_Div1);//HCLK --> AHB ����?2M������ֻ��Ҫ1��Ƶ
		RCC_PCLK1Config(RCC_HCLK_Div2); //PCLK1 --> APB1 ����?6M������Ҫ2��Ƶ	
		RCC_PCLK2Config(RCC_HCLK_Div1); //PCLK2 --> APB2 ����?2M������ֻ��Ҫ1��Ƶ
		RCC_PLLCmd(ENABLE); //ʹ��PLL
		// Wait till PLL is ready
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); //�ȴ�PLL�ȶ�
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); //ѡ��ϵͳʱ�ӣ�ѡ�����໷�����?		// Wait till PLL is used as system clock source
		while (RCC_GetSYSCLKSource() != 0x08); //�ȴ�ѡ���ȶ�
		//RCC_GetClocksFreq(&RCC_ClockFreq);//�鿴Ƶ��ֵ
	}
	else
	{
			//HSI����ʧ��
	}
}



/******************************
**	ʹ���ڲ�8MHz����
*********************************/

void fr_init_hsi_8mhz(void)
{
	__IO uint32_t HSIStatus = 0;
    
  RCC_DeInit();    //�ȸ�λRCC�Ĵ���
    
  RCC_HSICmd(ENABLE);  //ʹ��HSI
    //���HSE�Ƿ������ɹ�
  HSIStatus = RCC->CR & RCC_CR_HSIRDY;
  
	if (RCC_CR_HSIRDY == HSIStatus )
  {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);  //ʹ��Ԥȡָ������FLASH�̼��еĺ���
		FLASH_SetLatency(FLASH_Latency_0);    //����FLASH�ȴ����ڡ�  ��Ϊ��Ƶ��72M ���Եȴ��������ڡ�
		
		RCC_HCLKConfig(RCC_SYSCLK_Div1);//HCLK --> AHB ����?2M������ֻ��Ҫ1��Ƶ
		RCC_PCLK1Config(RCC_HCLK_Div2); //PCLK1 --> APB1 ����?6M������Ҫ2��Ƶ	
		RCC_PCLK2Config(RCC_HCLK_Div1); //PCLK2 --> APB2 ����?2M������ֻ��Ҫ1��Ƶ
		//RCC_PLLCmd(ENABLE); //ʹ��PLL
		// Wait till PLL is ready
		while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET); //�ȴ�PLL�ȶ�
		RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); //ѡ��ϵͳʱ�ӣ�ѡ�����໷�����?		// Wait till PLL is used as system clock source
		while (RCC_GetSYSCLKSource() != 0x00); //�ȴ�ѡ���ȶ�
		//RCC_GetClocksFreq(&RCC_ClockFreq);//�鿴Ƶ��ֵ
	}
	else
	{
		//HSI����ʧ��
	}
}










/***************************
**	��ʼ��PVD
*****************************/

void fr_init_pvd(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;

    /* Enable PWR and BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Configure EXTI Line to generate an interrupt on falling edge */
    /* Configure EXTI Line16(PVD Output) to generate an interrupt on rising and falling edges */

	EXTI_DeInit();                                            /*��EXIT�Ĵ���������Ϊȱʡֵ*/
	
  EXTI_InitStructure.EXTI_Line = EXTI_Line16;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

    /* Configure the PVD Level to 2.9V */
  PWR_PVDLevelConfig(PWR_PVDLevel_2V9);
    /* Enable the PVD Output */
  PWR_PVDCmd(ENABLE);
  EXTI_ClearITPendingBit(EXTI_Line16);
}



/***************************
**	��ʼ��WKUP
****************************/

void fr_init_wkup(void)
{
    EXTI_InitTypeDef  EXTI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    EXTI_InitStructure.EXTI_Line = EXTI_Line0 ;		            /*�ж���*/
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt ;      /*�ж�ģʽ*/
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising ;   /*�����ش���*/
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
	
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

		EXTI_ClearITPendingBit(EXTI_Line17);

    RTC_WaitForLastTask();
    //RTC_ITConfig(RTC_IT_ALR, ENABLE);
		RTC_ITConfig(RTC_IT_SEC, ENABLE); //ʹ�� RTC ���ж�
    RTC_WaitForLastTask();	   				//���������ж�
}



/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void fr_nvic_config(void)
{
NVIC_InitTypeDef NVIC_InitStructure;

   //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0xE000);	// 0x08042C00        0xE000
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);/* 4 bits for Preemption Priority*/

   NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
	
	 NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
	
	 NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

	 NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
	 
	 	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

/**
* @brief  Inserts a delay time.
* @param  nCount: specifies the delay time length.
* @retval None
*/
void Delay(__IO uint32_t nCount)
{
	/* Decrement nCount value */
	while (nCount != 0)
	{
		nCount--;
	}
}


/**
 * print RCC freq information
 *
 * for example:
 *
 * SYSCLK_Frequency is 48000000HZ
 * PCLK_Frequency is 48000000HZ
 * HCLK_Frequency is 48000000HZ
 * CECCLK_Frequency is 32786HZ
 * ADCCLK_Frequency is 14000000HZ
 * USART1CLK_Frequency is 48000000HZ
 * I2C1CLK_Frequency is 8000000HZ
 * SystemCoreClock is 48000000HZ
 *
 */

/**
 * This is the timer interrupt service routine.
 *
 */


/************************
**
**************************/

void fr_reset_sys(void)
{
	NVIC_SystemReset();
}




/******************************
 ** This function will initial STM32 board.
 ******************************/
void fr_init_board(void)
{
	fr_init_pvd();
	
	if(PWR_GetFlagStatus(PWR_FLAG_PVDO) == 0)
	{
		fr_set_console("uart1");
		fr_gpio_init();
		fr_nvic_config();
		fr_timer3_init();
		fr_init_fram(); 
			
	}
}









/******************************
**	
*******************************/
void fr_wkup_idle(void)
{
	EXTI_InitTypeDef  EXTI_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  PWR_BackupAccessCmd(ENABLE);

  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);

  EXTI_ClearITPendingBit(EXTI_Line17);

  RTC_WaitForLastTask();
  RTC_ITConfig(RTC_IT_ALR, DISABLE);
  RTC_WaitForLastTask();	    /*--���������ж�--*/
}


/******************************
 ** ����ģʽ��������ģʽ
******************************/
void fr_enter_sleep(void)
{
	unsigned int tmp;

	fr_init_wkup();
	tmp = RTC_GetCounter();				//
  fr_set_rtc_alarm(tmp + STM32_ALARM_TIMEOUT);		//�������Ź������ι�����25s*    //   STM32_ALARM_TIMEOUT
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;   //�رյδ�ʱ��
  PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); 			//����ֹͣģʽ
}


/******************************
**	˯��ģʽ��������ģʽ
*******************************/

void fr_exit_sleep(void)
{
	fr_init_hse_72mhz();
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; 						 // ʹ�ܵδ�ʱ�� 
  fr_wkup_idle();
}



/******************************
**	ι��
*******************************/

void fr_reset_iwdg(void)
{
	IWDG_ReloadCounter();
}


/*@}*/
