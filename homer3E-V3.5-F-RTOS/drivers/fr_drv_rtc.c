

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"

#include "board.h"




static void (*fr_rtc_hook)();


/*************************************
**	�����ʱ
**************************************/

static void fr_rtc_delay_ms(unsigned int delayms)
{
    delayms = delayms > 10000UL ? 10000UL : delayms;

    delayms = 8000 * delayms;
    while(delayms--)
    {
        for(; delayms > 0; delayms--)
            __nop();
    }
}


/**************************************
**	��ʼ��RTC�ڲ�����
**************************************/

unsigned char fr_init_rtc_lsi(void)
{
	unsigned short int temp=0;
	
	BKP_DeInit();
	RCC_LSICmd(ENABLE);
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	//�ȴ��ȶ�
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && temp < 1000)
	{
		temp++;
		delay_ms(10);
	}
	if(temp >= 1000)
		return 1;//��ʼ��ʱ��ʧ��,��������
   //RTC����
   RCC_RTCCLKCmd(ENABLE);
		//��������Ҫ�ȴ�APB1ʱ����RTCʱ��ͬ�������ܶ�д�Ĵ���
   RTC_WaitForSynchro();
    //��д�Ĵ���ǰ��Ҫȷ����һ�������Ѿ�����
   RTC_WaitForLastTask();
    //����RTC��Ƶ����ʹRTCʱ��Ϊ1Hz
    //RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
   RTC_SetPrescaler(39999);
   //�ȴ��Ĵ���д�����
   RTC_WaitForLastTask();
	 RTC_ExitConfigMode(); 							//�˳�����ģʽ
   //ʹ�����ж�
   //RTC_ITConfig(RTC_IT_SEC, ENABLE);
   RTC_ITConfig(RTC_IT_ALR, ENABLE);
   //�ȴ�д�����
   RTC_WaitForLastTask();
	 
	 return 0;
}







/***********************************
**	��ʼ��RTC�ⲿ����
************************************/

unsigned char fr_init_rtc_lse(void)
{
	unsigned short int temp = 0;
	
	BKP_DeInit(); 
	RCC_LSEConfig(RCC_LSE_ON); //�����ⲿ���پ���(LSE)
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && temp < 1000)
	{
		temp++;
		delay_ms(10);
	}
	if(temp >= 1000)
	{
		return 1;//��ʼ��ʱ��ʧ��,���������� 
	}
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //���� RTC ʱ��
		
	RCC_RTCCLKCmd(ENABLE); 							//ʹ�� RTC ʱ��
	RTC_WaitForLastTask(); 							//�ȴ����һ�ζ� RTC �Ĵ�����д�������
	RTC_WaitForSynchro(); 							//�ȴ� RTC �Ĵ���ͬ��
	RTC_ITConfig(RTC_IT_SEC, ENABLE); 	//ʹ�� RTC ���ж�
	RTC_WaitForLastTask(); 							//�ȴ����һ�ζ� RTC �Ĵ�����д�������
	RTC_EnterConfigMode(); 							// ��������
	RTC_SetPrescaler(32767); 						//���� RTC Ԥ��Ƶ��ֵ
	RTC_WaitForLastTask(); 							//�ȴ����һ�ζ� RTC �Ĵ�����д�������
	RTC_ExitConfigMode(); 							//�˳�����ģʽ
	//RTC_ITConfig(RTC_IT_SEC, ENABLE);
  RTC_ITConfig(RTC_IT_ALR, ENABLE);
   //�ȴ�д�����
  RTC_WaitForLastTask();
	
	return 0; //ok
	
}


/*************************************
**	��������:
**	��������:
**************************************/
void fr_init_rtc(void)
{
    unsigned short int temp = 0;

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); 	//����PWR��BKP��ʱ��(from APB1)
    PWR_BackupAccessCmd(ENABLE);            																	//�������
		
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
			if(fr_init_rtc_lse() == 0)
			{
				//DebugPrintf(">RTC LSE OSC Init OK......\r\n");
			}
			
			else if(fr_init_rtc_lsi() == 0)
			{
				//DebugPrintf(">RTC LSI RC Init OK......\r\n");
			}
			else
			{
				//DebugPrintf("RTC Init Fail............\r\n");
			}
      BKP_WriteBackupRegister(BKP_DR1, 0xA5A5); //������ɺ���󱸼Ĵ�����д�����ַ�0xA5A5
    }
    else 			//���󱸼Ĵ���û�е��磬��������������RTC
    {
        if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)  //*�������ǿ�������RCC_GetFlagStatus()�����鿴���θ�λ����*/
        {
           //DebugPrintf("> Power On Reset occurred....\r\n");
        }
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
        {
            //DebugPrintf("> External Reset occurred....\r\n");
        }
				//DebugPrintf("> No Need To configure Rtc....\r\n");
        RCC_ClearFlag();		                      //���RCC�и�λ��־*/
        temp = 0;
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
            temp++;
            //DelayMS(10);
						fr_rtc_delay_ms(10);
            if(temp >= 300)
            {
                //DebugPrintf(">RTC_LSE Not Available \r\n");//	�ȴ��ȶ�
                return;
            }
        }
				
				RTC_WaitForSynchro(); //�ȴ����һ�ζ� RTC �Ĵ�����д�������
				RTC_ITConfig(RTC_IT_SEC,DISABLE); //ʹ�� RTC ���ж�
				RTC_WaitForLastTask(); //�ȴ����һ�ζ� RTC �Ĵ�����д�������
		}
}




/***********************
**	����RTC
************************/

void fr_set_rtc(unsigned int n)
{
	RTC_WaitForLastTask();
	RTC_SetCounter(n);
	RTC_WaitForLastTask();	
}


/***********************
**	��ȡRTC
************************/

unsigned int fr_read_rtc(void)
{
	unsigned int rv;
	
	rv =  RTC_GetCounter();	
	
	return rv;
}



/*************************
**	��������ʱ��
*************************/

void fr_set_rtc_alarm(unsigned int sec)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);        //ע�������д���

    RTC_WaitForLastTask();  /*�ȴ���һ�ε�д��*/
    RTC_EnterConfigMode();  /*��������ģʽ*/
    RTC_SetAlarm(sec); /*��������ʱ��:RTC_ALR = RTC_CNT+sec */
    RTC_WaitForLastTask();  /*�ȴ����ε�д��*/
    RTC_ExitConfigMode();   /*�˳�����ģʽ*/
}



/***********************
**	��ʱ�������ж�
************************/
void RTCAlarm_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
  {
		EXTI_ClearITPendingBit(EXTI_Line17);						//����ģʽ���Ѳ���Ҫ����line17
        
		if(PWR_GetFlagStatus(PWR_FLAG_WU) != RESET) 			//Check if the Wake-Up flag is set 
    {
			PWR_ClearFlag(PWR_FLAG_WU);									//Clear Wake Up flag
    }
    RTC_WaitForLastTask();
    RTC_ClearITPendingBit(RTC_IT_ALR);							//Clear RTC Alarm interrupt pending bit
    RTC_WaitForLastTask();
  }
}


/********************
**
**
**********************/


void fr_set_rtc_hook(void (*hook)(void))
{
	if(hook == NULL)
		return;
	
	if(fr_rtc_hook == NULL)
		fr_rtc_hook = hook;
		
}

/*******************************
**
**
**	RTC�жϷ������
**
**	
********************************/


void RTC_IRQHandler(void)
{
		if(RTC_GetITStatus(RTC_IT_SEC) != RESET)
		{
			RTC_ClearITPendingBit(RTC_IT_SEC);
			if(fr_rtc_hook == NULL)
				fr_rtc_hook();
		}            
}


