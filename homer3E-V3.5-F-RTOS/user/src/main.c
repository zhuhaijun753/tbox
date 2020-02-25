

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
//

#include "gnss.h"
#include "gsm.h"
#include "rtc.h"
#include "config.h"
#include "mon.h"
#include "can.h"
#include "rtc.h"
#include "pro_data.h"
#include "ddp.h"
#include "fifo.h"


//һ����ϢΪ�����

#include "board.h"
#include "fr_drv_uart.h"
#include "fr_drv_timer.h"
#include "fr_drv_gpio.h"
#include "fr_drv_adc.h"
#include "fr_drv_mem.h"
#include "fr_drv_rtc.h"



static unsigned char 	run_state;


/***************������*******************/

TaskHandle_t 			task_handler_gnss;						//GNSS����
TaskHandle_t 			task_handler_gsm;							//gsm������������
TaskHandle_t 			task_handler_rtc;							//rtc����
TaskHandle_t			task_handler_in;							//��������
TaskHandle_t			task_handler_config;					//��������
TaskHandle_t			task_handler_can_lock;				//can��������
TaskHandle_t			task_handler_can_rx;					//can��������
TaskHandle_t 			task_handler_up_load;
TaskHandle_t      task_handler_build_data;

/*****************************************/

static TickType_t 							mon_ticks = 0xFFFFFFFF;     //

//static unsigned char 						mon_recv_buf[4096];         //���ջ�����

static SemaphoreHandle_t 							mon_semaphore = NULL;				//


/****************************
**	������ɻص�����
******************************/
static void mon_ticks_handle(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken;
	
	if(mon_ticks == 0xFFFFFFFF)
		return;
	if(xTaskGetTickCount() - mon_ticks > 2)
	{
		mon_ticks = 0xFFFFFFFF;
		xSemaphoreGiveFromISR(mon_semaphore,&xHigherPriorityTaskWoken);
	}
}


/**********************
**	�յ�GNSS����
************************/

static void mon_rx_handle(void)
{
	mon_ticks = xTaskGetTickCount();
}



/***********************
**	��������
**	������������
************************/

void task_config(void *data)
{
	unsigned char buf[200];
	//unsigned char 						mon_recv_buf[4096]
		
	BaseType_t mon_state;
	int index;
	
	data = data;
	
	for(;;)
	{
		mon_state = xSemaphoreTake(mon_semaphore,100);     //�ȴ���Ӧ
		if(mon_state > 0)
		{
			mon_state = read_uart_pkt(2,buf,sizeof(buf));
			if(strstr((const char *)buf, "HOMER3ETESTOVER!") != NULL)     //ϵͳ���񣬻�����������
			{
				save_config_info();
				run_state = 1;
				
				vTaskDelete(task_handler_config);  		//ɾ������
				break;
			}
		
			if((index = look_for_str(buf, (unsigned char *)"BDWMODIF:",mon_state)) >= 0)
			{
				mon_state = mon_state - 9;
				analysis_config_info(buf + index + 8,mon_state);
			}
			
			if(strstr((const char *)buf, "Reset") != NULL)     //ϵͳ���񣬻�����������
			{
				vTaskDelay(100);
				fr_reset_sys();
				break;
			}
		}
		memset(buf,'\0',sizeof(buf));
		index = build_config_info((char *)buf,sizeof(buf),0);
		fr_send_console((const char *)buf,index);
	}
}

/*********************************
**	�ϴ���
*********************************/

void task_up_load(void *data)
{
	unsigned short int len;
	unsigned char buf[400];
	
	data = data;
	
	for(;;)
	{
		vTaskDelay(100);
		if(read_net_state() != 2)          //����δ���ӳɹ����������κ�����
			continue;
		len = ReadCmdDataBuf(buf);     //(?????? ?????????)
		if(len > 0)
		{
			if(gsm_send_data(1,buf,len) > 0)
			{
				ClearCmdData();
				//fr_printf("Send CMD..................\r\n");
				continue;
			}
		}
		
		len = ReadSendQueue(buf);
		if(len > 0)
		{
			if(gsm_send_data(1,buf,len) > 0)
			{
				fr_printf("Send Up load data........\r\n");
				ClearSendQueue();
			}
			
		}
	}
}


/*********************************
**	��������
**	
*********************************/

void task_build_data(void *data)
{
	unsigned int 					next_time_posi;
	unsigned int 					next_time_comp;
	unsigned char 				step;
	unsigned int 					tmp;
	unsigned short int		pack_len;
	unsigned char 				work_state;
	unsigned char 				data_packet_buf[512];
	data = data;
	
	next_time_posi = 0;
	next_time_comp = 0;
	step = 0;
	vTaskDelay(300);
	
	for(;;)
	{
		vTaskDelay(10);
		work_state = (read_can_actual_rotate() >= 2200) ? 1 : 0;    //�����ж�����
		tmp = read_timestamp();
		switch(step)
		{
			case 0:													//  
				if(tmp >= next_time_comp)
				{
					next_time_comp =  tmp + read_config_travel_upload_cycle();
					pack_len = build_complete_packet(data_packet_buf,0);
					
					if(pack_len > 0)
					{
						//fr_printf("build_complete_packet:%u\r\n",pack_len);
						WriteSendQueue(data_packet_buf,pack_len);         //
						break;
					}
				}
			
				if(work_state > 0)                   //����״̬
					step++;
				break;
			case 1:                   		//   
				if(work_state == 0)                              //
				{
					pack_len = build_position_packet(data_packet_buf,1);  //�������
					if(pack_len > 0)
					{	
						//fr_printf("acc over build_position_packet:%u\r\n",pack_len);
						WriteSendQueue(data_packet_buf,pack_len);    //
						step = 0;
						break;
					}
				}
			
				if(tmp >= (next_time_comp - 1))
				{	
					next_time_posi = tmp + read_config_work_upload_cycle();
					pack_len = build_position_packet(data_packet_buf,1);         	//
					if(pack_len > 0)
					{
						//fr_printf("work over build_position_packet:%u\r\n",pack_len);
						WriteSendQueue(data_packet_buf,pack_len);   				//
						step++;
						break;
					}

				}
				if(tmp < next_time_posi)           						//
					break;
				next_time_posi = tmp + read_config_work_upload_cycle();
				pack_len = build_position_packet(data_packet_buf,0);
				if(pack_len > 0)
				{
					//fr_printf("work build_position_packet:%u\r\n",pack_len);
					WriteSendQueue(data_packet_buf,pack_len);    					//
				}
				break;
			case 2:
				if(tmp >= next_time_comp)
				{
					next_time_comp =  tmp + read_config_travel_upload_cycle();
					next_time_posi =  tmp + read_config_work_upload_cycle();

					pack_len = build_complete_packet(data_packet_buf,0);
					if(pack_len > 0)
					{
						//fr_printf("work build_complete_packet:%u\r\n",pack_len);
						WriteSendQueue(data_packet_buf,pack_len); 
					}
					step = 1;
				}
			
				break;
			default: 
				step = 0;
				break;
		}
		
		//��������
	}
}


/*

void task_test(void *data)
{
	unsigned char step = 0;
	int i;
	
	data = data;
	
	for(;;)
	{
		switch(step)
		{
			case 0:
				step++;
				vTaskDelay(100);
			storage_medium_init();	
				break;
			case 1:
				fr_printf("Write Ex Flash\r\n");
				for(i = 0;i < 4096;i++)
					mon_recv_buf[i] = i;
			taskENTER_CRITICAL(); 
				w25qxx_flash_write(mon_recv_buf, 0,1);
			taskEXIT_CRITICAL(); 
				step++;
				break;
			case 2:
				vTaskDelay(100);
				step++;
				break;
			case 3:
				fr_printf("Read Ex Flash\r\n");
				memset(mon_recv_buf,0,sizeof(mon_recv_buf));
				w25qxx_flash_read(mon_recv_buf, 0, 1);
				step++;
				break;
			case 4:
				vTaskDelay(100);
				fr_printf("Test tast run.............\r\n");
				break;
		}
	}
}


*/



/**************************
**	ϵͳ����ʼ
**	��������״̬�豸��ʼ����
**	
***************************/

void task_start(void *data)
{
	unsigned portBASE_TYPE stack_;
	BaseType_t mon_state;
	unsigned int acc_close_cnt;
	unsigned char tmp;
	unsigned char 						mon_recv_buf[256];
	
	data = data;

	acc_close_cnt = 0;
	run_state = 0;
	init_fifo_sema();
	
	tmp = 1;
	if(tmp == 1)
		fr_printf("init storage ok...............\r\n");
	
	for(;;)
	{
		switch(run_state)
		{
			case 0:													//ϵͳ��ʼ������������
				
				taskENTER_CRITICAL();    	
						
				fr_led_red_on(); 							//������ɫLED
				read_config_info();           //��ȡ������Ϣ���豸���������Ϣ��
	
				xTaskCreate((TaskFunction_t)task_gnss,			(const char *)"task_gnss",			100,NULL,3,&task_handler_gnss);  	//������������
				xTaskCreate((TaskFunction_t)task_gsm,				(const char *)"task_gsm",				128,NULL,4,&task_handler_gsm);  		//������������
				xTaskCreate((TaskFunction_t)task_rtc,				(const char *)"task_rtc",				100,NULL,5,&task_handler_rtc);  		//������������
				xTaskCreate((TaskFunction_t)task_in,				(const char *)"task_in",				100,NULL,6,&task_handler_in);  		//������������
				xTaskCreate((TaskFunction_t)task_can_lock,	(const char *)"task_can_lock",	128,NULL,7,&task_handler_can_lock); 
				xTaskCreate((TaskFunction_t)task_can_rx,		(const char *)"task_can_rx",		100,NULL,8,&task_handler_can_rx);  	
				xTaskCreate((TaskFunction_t)task_build_data,(const char *)"task_build_data",256,NULL,9,&task_handler_build_data); 
				//xTaskCreate((TaskFunction_t)task_test,(const char *)"task_test",512,NULL,15,&task_handler_build_data); 
				
				
				taskEXIT_CRITICAL();             //�˳��ٽ���
				
				//if(mon_semaphore == NULL)
					mon_semaphore = xSemaphoreCreateBinary();    //�ź���
				fr_set_uart2_rx_hook(mon_rx_handle);
				fr_timer3_sethook(mon_ticks_handle);
				vTaskDelay(100);      //
				if(read_config_state() == 0x56)
				{
					xTaskCreate((TaskFunction_t)task_config,			(const char *)"task_config",		512,NULL,10,&task_handler_config);  		//������������
					run_state = 3;
					break;
				}
				
				xTaskCreate((TaskFunction_t)task_up_load,		(const char *)"task_up_load",		256,NULL,2,&task_handler_up_load);  	
				
				fr_init_wdt(5);
				acc_close_cnt = 0;
				run_state = 1;
				break;
			case 1:														//�豸��������
				fr_reset_iwdg();
				
				mon_state = xSemaphoreTake(mon_semaphore,(TickType_t)70);     //�ȴ���Ӧ
				if(mon_state > 0)
				{
					mon_state = read_uart_pkt(2,mon_recv_buf,sizeof(mon_recv_buf));
					if(strstr((const char *)mon_recv_buf, "Reset") != NULL)     //ϵͳ���񣬻�����������
					{
						fr_printf("Reset from mon.....\r\n");
						vTaskDelay(100);
						fr_reset_sys();
						break;
					}
					if(strstr((const char *)mon_recv_buf, "AT+Test") != NULL)     //ϵͳ���񣬻�����������
					{
						vTaskDelete(task_handler_up_load);            //ɾ��
						vTaskDelay(10);
						xTaskCreate((TaskFunction_t)task_config,			(const char *)"task_config",		512,NULL,31,&task_handler_config);  		//������������
						run_state = 3;
						break;
					}
				}
				
				fr_led_red_on();
				vTaskDelay(30);
				fr_led_red_off();
				
				/*
				acc_close_cnt++;
				if(acc_close_cnt++ > 30)
				{
					acc_close_cnt = 0;
					tmp = gsm_send_data(1,(unsigned char *)"123456789abcdef\r\n",sizeof("123456789abcdef\r\n"));
					if(tmp > 0)
						fr_printf("Send Data OK...........\r\n");
					if(tmp == 0)
						fr_printf("Send Data Fail...........\r\n");
				}
				
				test_SendQueue();
				*/
				stack_ = uxTaskGetStackHighWaterMark(task_handler_up_load);
				//fr_printf("task ta.....%d\r\n",stack_);
				if(read_acc_state() == 1)
				{
					acc_close_cnt = 0;
					break;
				}
				
				acc_close_cnt++;
				fr_printf("Acc close.....%d\r\n",acc_close_cnt);
				if(acc_close_cnt > read_config_info_run_time() || read_batter_vol()  <= 34)
					run_state = 2; 
				//mon_state = read_config_info_sleep_time() + read_timestamp();   //�Զ�����ʱ��
				mon_state = 3600 + read_timestamp();   //�Զ�����ʱ��
				break;
			case 2:					//����ģʽ(���ߵ�ʱ������ϵͳֹͣ����)
				//���������������
				fr_init_wdt(25);
				vTaskDelete(task_handler_up_load);							//ɾ������
				vTaskDelete(task_handler_gnss);  								//ɾ������
				vTaskDelete(task_handler_gsm);  								//ɾ������
				vTaskDelete(task_handler_rtc);  								//ɾ������
				vTaskDelete(task_handler_in);  									//ɾ������
				vTaskDelete(task_handler_build_data);
				vTaskDelete(task_handler_can_lock);   					//ɾ������
				vTaskDelete(task_handler_can_rx);  							//ɾ������			
					
				close_gnss_modle();
				gsm_modle_close();
				fr_can_power_off();
				fr_power_flash_off();
				
			  fr_led_red_off();
				fr_led_blue_off();
				fr_led_green_off();
				
				
				fr_init_hsi_8mhz();      //ϵͳ�л�8MHz��Ƶ
				do
				{
					fr_reset_iwdg();
					fr_enter_sleep();      //��������
					
					fr_init_adc();
					
					tmp	 = 0;
					while(tmp < 10)
					{
						tmp++;
						delay_ms(10);
						process_in();
					}
					                  //
				}while((read_acc_state() == 0 && mon_state >= fr_read_rtc()) || read_batter_vol() <= 34);
				//fr_reset_sys();      //ϵͳ��λ
				fr_exit_sleep();
				run_state = 0;
				break;	
			case 3:          //����ģʽ()
				vTaskDelay(10);
				fr_led_red_on();            //��ɫLED��ָʾ�豸���ڿ���ģʽ
				vTaskDelay(10);
				fr_led_red_off();	
				fr_reset_iwdg();			
				if(read_acc_state() == 0)
					run_state = 2;
				break;
			default:
				run_state = 0;
				break;
		}
	}
}


/*************************
**	�����ܺ�����
**************************/
int main(void)				 
{
	fr_init_board();
	
	xTaskCreate((TaskFunction_t)task_start,(const char *)"task_start",200,NULL,1,NULL);   //ɾ������
	
	vTaskStartScheduler();   				//����ʼ���� 
	
	return 0;

}


/************************************************
*  �������ƣ�vApplicationIdleHook
*  ��    �ܣ����������Ӻ���
*************************************************/
void vApplicationIdleHook( void )
{
   // __WFI();
}



/********************File End***********************/


