
/**************************************
**
**	
***************************************/

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"

#include "fr_drv_gpio.h"
#include "fr_drv_uart.h"
#include "fr_drv_timer.h"

#include "data_type.h"
#include "pro_data.h"
#include "mon.h"
#include "config.h"



/*********Ĭ��Ԥ����***************/

#define DEF_SERVER_ADDR "123.127.244.154"
#define DEF_SERVER_PORT 27000

//#define DEF_SERVER_ADDR "211.103.179.234"//
//#define DEF_SERVER_PORT 13011

//#define DEF_SERVER_ADDR "was.ym.bcnyyun.com"      
//#define DEF_SERVER_PORT 26000


#define SOCKET_TCP 						0						

#define SOCKET_UDP 						1


extern QueueHandle_t  					lock_cmd_queue;


/******************************************/

static TickType_t 							gsm_ticks = 0xFFFFFFFF;     //gsmģ��

static unsigned char 						gsm_recv_buf[1024];         //���ջ�����

static SemaphoreHandle_t 				gsm_semaphore = NULL;       //gsm�ź���

static SemaphoreHandle_t 				gsm_send_semaphore = NULL;       //gsm�ź���
    
static struct gsm_info_str			gsm_info;										//gsmʵʱ��Ϣ

static unsigned char 						gsm_send_buf[1024];					//���ͻ�����

static TimerHandle_t						gsm_timer_handle					= NULL;  			//

/*******************************************
**	���ͳɹ��ص�����	
********************************************/





/*******************************************
**		ͬ��ͨѶ��ʽ���Ͳ�����ATָ��
**		 buff �����͵������׵�ַ
**          len ���������ݳ���
**          func �������ݽ�������
**          timeout ��ʱʱ�� ��λs
**          retry �Ƿ�1�����ط���־ RETRY/NOT_RETRY
*************************************************/

unsigned char at_cmd_send_syn(void *buff, unsigned char len, unsigned char(*func)(unsigned char *,unsigned short int len),unsigned short int timeout_s, unsigned char retry)
{
	BaseType_t 					sem_state;
	unsigned char tmp;
	
	if(buff == NULL)
		return 0;
	
	tmp = 0;
	
	timeout_s = timeout_s / 10;
	
	do
	{
		tmp++;
		fr_send_uart4((unsigned char *)buff,len);
		sem_state = xSemaphoreTake(gsm_semaphore,timeout_s);     //�ȴ���Ӧ  
		if(sem_state > 0)
		{
			sem_state = read_uart_pkt(4,gsm_recv_buf,sizeof(gsm_recv_buf));
			if(func(gsm_recv_buf,sem_state) > 0)    //���ûص�����
				return 1;
		}
		if(tmp >= retry)
			break;
		vTaskDelay(100);
	}while(1);

	return 0;
}








/******************************
** ��ȡ�ź�ֵ
******************************/

static unsigned char at_cmd_csq(unsigned char *data,unsigned short int len)
{
	int index;
	
  index = look_for_str(data, (unsigned char *)"+CSQ:",len);
	if(index >= 0)
	{
		sscanf((const char *)data + index, "+CSQ: %d,%d",&gsm_info.csq,&index);
		gsm_info.csq++;
		//fr_printf("+CSQ:%d\r\n",gsm_info.csq);
		return 1;
	}
	
  return 0;
}



/******************************
** ����CGREG������Ϣ
******************************/

static unsigned char at_cmd_cpin(unsigned char *data,unsigned short int len)
{
	int index;
	
  index = look_for_str(data, (unsigned char *)"+CPIN: READY",len);
	
  if(index >= 0)
	{
		gsm_info.sim_state = 1;
		return 1;
	}

  return 0;
}




/******************************
** ����CGREG������Ϣ
******************************/

static unsigned char at_cmd_creg(unsigned char *data,unsigned short int len)
{
	unsigned char *p = NULL;
	int index;
	
  index = look_for_str(data, (unsigned char *)"+CREG:",len);
	
  if(index >= 0)
  {
		p = data + index;
    sscanf((const char *)p, "+CREG: %*d,%d", &gsm_info.net_register);
    if((gsm_info.net_register == 1) || (gsm_info.net_register == 5))
    {
			gsm_info.led_state = 1;
      return 1;
    }
  }
	
  return 0;
}



/***********************************
**	����SIM��ICC ID
************************************/
static unsigned char at_cmd_icc_id(unsigned char *data,unsigned short int len)
{
    unsigned char *p = NULL;
		int index;
	
		index = look_for_str(data,(unsigned char *)"+CCID:",len);
		if(index >= 0)
		{
			p = data + index;
			sscanf((const char *)p, "+CCID: %s\r\n",gsm_info.iccid);
			return 1;
		}
		
		return 0;
}




/***********************************
**	����GPRS���總��״̬
************************************/
static unsigned char at_cmd_cgatt(unsigned char *data,unsigned short int len)
{
    unsigned char *p = NULL;
		int index;
	
		index = look_for_str(data,(unsigned char *)"+CGATT:",len);
		if(index >= 0)
		{
			p = data + index;
			sscanf((const char *)p, "+CGATT: %d",&gsm_info.net_state);
			if(gsm_info.net_state == 1)
			{
				return 1;
			}
		}
		
		return 0;
}

/***********************************
**	����GPRS���總��״̬
************************************/
static unsigned char at_cmd_ok(unsigned char *data,unsigned short int len)
{
		int index;
	
		index = look_for_str(data,(unsigned char *)"OK",len);
		if(index >= 0)
			return 1;
		
		return 0;
}



/********************************
**	����pdp�رռ���ص�״̬
********************************/

//static unsigned char at_cmd_pdp_close(unsigned char *data,unsigned short int len)
//{
//    if(look_for_str(data, (unsigned char *)"+MIPCALL: 0",len) >= 0)
//    {
//        gsm_info.pdp_state = 0;
//        
//        return 1;
//    }
//    return 0;
//}


/***************************** 
**	��ȡ����ip
*****************************/

static unsigned char at_get_local_ip(unsigned char *data,unsigned short int len)
{
	int index;
	
	if((index = look_for_str(data, (unsigned char *)"+MIPCALL:",len)) >= 0)
	{
		sscanf((const char *)data + index, "+MIPCALL: %d",&index);
		if(index == 1)
		{
			gsm_info.pdp_state = 1;
			sscanf((const char *)data + index, "+MIPCALL: 1,%d.%d.%d.%d\r\n",&gsm_info.local_ip[0],&gsm_info.local_ip[1],&gsm_info.local_ip[2],&gsm_info.local_ip[3]);
			return 1;
		}
		
	}
	
	return 0;
}


/***************************** 
**	��ȡ����ip
*****************************/

//static unsigned char at_cmd_mipopen(unsigned char *data,unsigned short int len)
//{
//	unsigned int tmp; 
//	int index;
//	
//	if((index = look_for_str(data, (unsigned char *)"+MIPOPEN:",len)) >= 0)
//	{
//		sscanf((const char *)data + index, "+MIPOPEN: %d,%d",&index,&tmp);
//		if(tmp == 1)
//		{
//			gsm_info.net_state = 2;       //�Ѿ����ӵ�������
//		}
//		return 1;
//	}

//	return 0;
//}


/***************************** 
**	��ȡ����ip
*****************************/

//static unsigned char at_cmd_mipsend(unsigned char *data,unsigned short int len,unsigned int socket_num)
//{
//	unsigned int tmp; 
//	int index;
//	
//	if((index = look_for_str(data, (unsigned char *)">",len)) >= 0)
//	{
//		sscanf((const char *)data + index, "+MIPOPEN: %d,%d",&index,&tmp);
//		if(index == socket_num && tmp == 1)
//		{
//			gsm_info.net_state = 2;       //�Ѿ����ӵ�������
//		}
//		return 1;
//	}
//	
//	return 0;
//}



/*********************************
**	������ɻص�����
*********************************/
static void gsm_ticks_handle(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken; 
	
	xHigherPriorityTaskWoken = pdTRUE;          //ע��������ź�������
	
	if(gsm_ticks == 0xFFFFFFFF)
		return;
	if(xTaskGetTickCount() - gsm_ticks > 1)
	{
		gsm_ticks = 0xFFFFFFFF;
		xSemaphoreGiveFromISR(gsm_semaphore,&xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}


/**********************
**	�յ�������
************************/

static void gsm_rx_handle(void)
{
	gsm_ticks = xTaskGetTickCount();
}






/**********************
**	GSMģ�鿪��
**	0������ʧ�� ��
**	1�������ɹ�
************************/

static unsigned char gsm_open_power(void)
{
	BaseType_t 					sem_state;
	int index;
	unsigned char tmp;
	
	if(read_power_vol() < 60 && read_batter_vol() < 34)   //�ж��ⲿ��Դ��ѹ�����õ�ص�ѹ
		return 0;
	
	fr_power_gprs_on();                          //�򿪵�Դ
	vTaskDelay(30);
	
	if(fr_read_gpio_state(PORT_GPIO_GPRS_STATE, PIN_GPIO_GPRS_STATE) > 0)
	{
		fr_gprs_switch_high();
		vTaskDelay(200);
		fr_gprs_switch_low();           //����
		vTaskDelay(20);
		if(fr_read_gpio_state(PORT_GPIO_GPRS_STATE, PIN_GPIO_GPRS_STATE) == 0)     //������״̬
			return 0;
		vTaskDelay(200);            //�ػ��ɹ�����ʱ2����
	}
	fr_gprs_switch_high();
	vTaskDelay(200);
	fr_gprs_switch_low();           //����
	vTaskDelay(20);
	if(fr_read_gpio_state(PORT_GPIO_GPRS_STATE, PIN_GPIO_GPRS_STATE) == 0)     //������״̬
		return 0;
	gsm_info.gsm_up_state = 1;      //�����ɹ�
	
	tmp = 0;
	do
	{
		tmp++;
		sem_state = xSemaphoreTake(gsm_semaphore,100);     //�ȴ���Ӧ
		if(sem_state > 0)
		{
			sem_state = read_uart_pkt(4,gsm_recv_buf,sizeof(gsm_recv_buf));
			index = look_for_str(gsm_recv_buf,(unsigned char *)"AT command ready",sem_state);
			if(index >= 0)
				break;                  //����ѭ��
		}
	}while(tmp > 30);            //
	
	if(tmp >= 30)
		return 0;                 //
	
	if(at_cmd_send_syn("ATE0\r\n", sizeof("ATE0\r\n"),at_cmd_ok,500, 3) == 0)//�رջ���
		return 0;
	gsm_info.gsm_at_state = 1;
	
	return 1;            //�����ɹ�
}


/**********************
**	GSMע������
**	
************************/

static unsigned char gsm_register_network(void)
{	
	if(at_cmd_send_syn("AT+CPIN?\r\n", sizeof("AT+CPIN?\r\n"),at_cmd_cpin,1000,30) == 0)//��ѯSIM��״̬  
		return 0;
	if(at_cmd_send_syn("AT+CCID?\r\n", sizeof("AT+CCID?\r\n"),at_cmd_icc_id,500,3) == 0)//��ѯ����ע��״̬
		return 0;
	if(at_cmd_send_syn("AT+GTSET=\"IPRFMT\",2\r\n", sizeof("AT+GTSET=\"IPRFMT\",2\r\n"),at_cmd_ok,500,3) == 0)//
		return 0;
	if(at_cmd_send_syn("AT+CREG?\r\n", sizeof("AT+CREG?\r\n"),at_cmd_creg,1000,30) == 0)//��ѯ����ע��״̬
		return 0;
	if(at_cmd_send_syn("AT+CSQ\r\n", sizeof("AT+CSQ\r\n"),at_cmd_csq,500,3) == 0)//��ѯ����ע��״̬
		return 0;
	if(at_cmd_send_syn("AT+CGATT?\r\n", sizeof("AT+CGATT?\r\n"),at_cmd_cgatt,1000,30) == 0)//GPRS����״̬  (????ע�⸽�ŵ�����)
		return 0;
	return 1;
	
}


/**********************
**	GSMģ�鸴λ
************************/

void gsm_modle_close(void)
{
	fr_close_uart4();
	fr_power_gprs_off();
}


/*************************
**	�������ݣ�������˸
**	����ʧ�ܣ��ر�
**	ע����Ź�˾���磺1S��˸
**	���ӵ����أ�3S��˸
**	��������10��ʱ��Ƭ  100ms
**	
**************************/

void gsm_led_handle(void)
{
	static unsigned int counter;
	static unsigned char step;
	
	unsigned short int led_off_cycle;
	unsigned short int led_on_cycle;
	
	if(gsm_info.gsm_up_state == 0)
	{
		fr_led_green_off();
		return;
	}
		
	if(gsm_info.led_state == 1)
	{
		led_off_cycle = 7;
		led_on_cycle = 3;
	}
	else if(gsm_info.led_state == 2)
	{
		led_off_cycle = 20;
		led_on_cycle = 5;
	}
	else if(gsm_info.led_state == 3)
	{
		led_off_cycle = 0;
		led_on_cycle = 0;
	}
	else
	{
		fr_led_green_off();
		return ;
	}
	
	counter++;
	switch(step)
	{
		case 0:
			fr_led_green_on();
			if(counter < led_on_cycle)
				break;
			step++;
			counter = 0;
			break;
		case 1:
			fr_led_green_off();
			if(counter < led_off_cycle)
				break;
			step = 0;
			counter = 0;
			break;
		default:
			counter = 0;
			step = 0;
			break;
	}
}


/******************************
**
**	函数名称:
**	功能描述:
**	
********************************/

void init_gsm_timer(void)
{
	 if(gsm_timer_handle == NULL)
	 {
		 gsm_timer_handle = xTimerCreate
                   //
                   (NULL,
                   //
                   10,   
                   //
                   pdTRUE,
                   //
                  NULL,
                   //
                  (TimerCallbackFunction_t)gsm_led_handle);

     if(gsm_timer_handle != NULL ) 
		 {
        xTimerStart(gsm_timer_handle, 0);     //
		 }
	 }
}



/***************************
**	��ȡ�ź�ֵ
****************************/

unsigned char read_gsm_signal(void)
{
	return gsm_info.csq;    //
}


/**********************
**	��������
***********************/

unsigned char gsm_send_data(unsigned char socket_num,unsigned char *data,unsigned short int data_size)
{
	int i;
	
	if(data_size > 1024 || data_size == 0 || socket_num != 1)
		return 0;
	if(data == NULL)
		return 0;
	
	if(gsm_info.net_state != 2)
		return 0;
	
	xSemaphoreTake(gsm_send_semaphore,portMAX_DELAY );     //��ȡ�ź���
	
	for(i = 0 ; i < data_size;i++)
	{
		gsm_info.send_buf[i] = *(data + i);
	}
	
	gsm_info.send_len = data_size;
	gsm_info.send_socket = socket_num;
	i = 0;
	
	do
	{
		i++;
		vTaskDelay(100);
	}while(gsm_info.send_state == 0 && i < 30);
	
	xSemaphoreGive(gsm_send_semaphore);
	
	if(i >= 30)
		return 0;
	
	return 1;
	
	
}



/*****************************
**	�����������Ͽ�
******************************/

static unsigned char at_cmd_return_mipstat(unsigned char *buf,unsigned short int buf_size)
{
	int index;
	int socket_num;
	
	if((index = look_for_str(buf, (unsigned char *)"+MIPSTAT:",buf_size)) >= 0)
	{
		sscanf((const char *)gsm_recv_buf + index, "+MIPSTAT: %d,%d",&socket_num,&index);
		if(index == 1)
		{
			gsm_info.net_state = 1;       //�Ѿ����ӵ�������
			gsm_info.led_state = 1;
			return socket_num;
		}
	}
	return 0;
}




/****************************
**	GSMģ�鷢������
**	0:����ʧ��
**	1�����ͳɹ�
*****************************/

static unsigned char at_cmd_mipsend_data(unsigned char num,unsigned char *buf,unsigned short int data_len)
{
	BaseType_t 					sem_state;
	unsigned char step = 0;
	int index,socket_num;
	unsigned char tmp;
	
	
	
	do
	{
		switch(step)
		{
			case 0:
				gsm_info.led_state = 3;
				index = sprintf((char *)gsm_send_buf,(char *)"AT+MIPSEND=%d,%d\r",num,data_len);
				fr_send_uart4(gsm_send_buf,index);
				step++;
				break;
			case 1:
				sem_state = xSemaphoreTake(gsm_semaphore,100);     //�ȴ���Ӧ  
				if(sem_state > 0)
				{
					sem_state = read_uart_pkt(4,gsm_recv_buf,sizeof(gsm_recv_buf));
					tmp = at_cmd_return_mipstat(gsm_recv_buf,sem_state);                 //����յ�ƽ̨�����Ͽ�
					if(tmp > 0)
					{
						gsm_info.led_state = 1;
						return 0;
					}
					
				  if((index = look_for_str(gsm_recv_buf, (unsigned char *)">",sem_state)) >= 0)
					{
						vTaskDelay(5);
						fr_send_uart4(buf,data_len);
						step++;
						break;
					}
				}
				return 0;
			case 2:
				sem_state = xSemaphoreTake(gsm_semaphore,2000);     //�ȴ���Ӧ 
				if(sem_state > 0)
				{
					sem_state = read_uart_pkt(4,gsm_recv_buf,sizeof(gsm_recv_buf));
					at_cmd_return_mipstat(gsm_recv_buf,sem_state);
					tmp = at_cmd_return_mipstat(gsm_recv_buf,sem_state);
					if(tmp > 0)
					{
						gsm_info.led_state = 1;
						return 0;
					}
					if((index = look_for_str(gsm_recv_buf, (unsigned char *)"+MIPSEND:",sem_state)) >= 0)   //���յ�����
					{
						sscanf((const char *)gsm_recv_buf + index, "+MIPSEND: %d,%d,%s\r\n",&socket_num,&index,gsm_send_buf);
						if(socket_num == num && index == 0)
						{
							return 1;           //�������
						}
					}
				}
				return 0;	
		}
	}while(1);
	
}




/*******************************************
** ����0������ʧ��
*******************************************/

unsigned char at_cmd_creat_socket(unsigned char num, unsigned char *server_addr,unsigned short int port,unsigned short int time_out)
{
	BaseType_t 					sem_state;
	unsigned char tmp;
	int index,socket_num;
	
	if(num == 0 || num > 4 || server_addr == NULL)
		return 0;
	
	tmp = time_out;
	
	index = sprintf((char *)gsm_send_buf,(char *)"AT+MIPOPEN=%d,,\"%s\",%d,0\r\n",num,server_addr,port);
	
	
	fr_send_uart4(gsm_send_buf,index);
	
	do
	{
		sem_state = xSemaphoreTake(gsm_semaphore,100);     //�ȴ���Ӧ  
			
		if(sem_state > 0)
		{
			sem_state = read_uart_pkt(4,gsm_recv_buf,sizeof(gsm_recv_buf));
			if(at_cmd_return_mipstat(gsm_recv_buf,sem_state) > 0)
				return 0;
			if((index = look_for_str(gsm_recv_buf, (unsigned char *)"+MIPOPEN:",sem_state)) >= 0)
			{
				sscanf((const char *)gsm_recv_buf + index, "+MIPOPEN: %d,%d",&socket_num,&index);
				if(socket_num == num && index == 1)
				{
					gsm_info.net_state = 2;       //�Ѿ����ӵ�������
					gsm_info.led_state = 2;
					return 1;
				}
			}
			
			if((index = look_for_str(gsm_recv_buf, (unsigned char *)"ERROR",sem_state)) >= 0)
			{
					gsm_info.net_state = 1;       //�Ѿ����ӵ�������
					gsm_info.led_state = 1;
					return 2;
			}

		}
		tmp--;
	}while(tmp > 0);

	return 0;
}



/************************
**	�������ݣ�
**	
*************************/

void analysis_down_data(unsigned char *data,unsigned short int data_len)
{
//	int i;
	BaseType_t 						xStatus;
	struct fr_event 			event;
	unsigned char 				tmp;
	unsigned char 				*p;
	unsigned short int 				CmdID;							//
	int16_to_char     		TmpData;
	//int32_to_char					TmpData32;
	int 									StrLen;       //
	unsigned char 				Buf[50];      //
	SysCmdStr							*pSysCmd;     //

	
	if(data_len < 25 || data_len > 1024)                    //
		return;
	
//	for(i = 0;i < data_len;i++)			   //��ӡ���յ�������(����ʹ��)
//		fr_printf("20x% ",*(data + i));
//	fr_printf("\r\n");
//	
	pSysCmd = (SysCmdStr *)data;

	   
	if(pSysCmd->FrameStart[0] != 0xF1 && pSysCmd->FrameStart[1] != 0xF2 && pSysCmd->FrameStart[2] != 0xFF)   //
		return;

	if(*(data + data_len - 1) != 0x0d)   //
		return;
	
	tmp = BccVerify(data + 3,data_len - 5);    //
	
	if(tmp != *(data + data_len - 2))          //
		return;

	
	if(pSysCmd->msg_id != 0x48)
		return;

	TmpData.byte[0] = *(data + sizeof(SysCmdStr));   		//
	TmpData.byte[1] = *(data + sizeof(SysCmdStr) + 1);  	//
	CmdID = TmpData.value;

	switch(pSysCmd->msg_id)
	{
		case 0x89:
			break;
		case 0xff:
			break;
		case 0x02:
			break;
		case 0x47:
			break;
		case 0x48:
			switch (CmdID)				
			{ 
					case 0x0200:     //����
						tmp = *(data + 29);
						
						if(tmp == 0)    //
						{
							event.cmd = 0;
							event.arg1 = 0;    
							event.arg2 = pSysCmd->DataPackFlag;
							event.arg3 = CmdID;
							xStatus = xQueueSendToBack(lock_cmd_queue, &event, 0);
							if( xStatus != pdPASS )
							{
								fr_printf("Could not send to the queue.........\r\n");
							}
							//fr_printf("Recv Unlock CmdID............\r\n");
						}
						break;
					case 0x0201:     //����
						tmp = *(data + 29);
					
						if(tmp <= 5)                 //
						{
							event.cmd = 0;
							event.arg1 = (tmp > 2) ? 2 : (tmp + 1);
							event.arg2 = pSysCmd->DataPackFlag;
							event.arg3 = CmdID;
							xStatus = xQueueSendToBack(lock_cmd_queue, &event, 0);
							if( xStatus != pdPASS )
							{
								fr_printf("Could not send to the queue.........\r\n");
							}
							//fr_printf("Recv Lock Cmd............\r\n");
						}
						break;
					case 0x0203:      									//
						tmp = *(data + 29);
						if(tmp == 0 || tmp == 2)
						{
							event.cmd = 1;
							event.arg1 = 1;
							event.arg2 = pSysCmd->DataPackFlag;
							event.arg3 = CmdID;
							xStatus = xQueueSendToBack(lock_cmd_queue, &event, 0);
							if( xStatus != pdPASS )
							{
								fr_printf("Could not send to the queue.........\r\n");
							}
							//fr_printf("Open Lock Cmd............\r\n");
						}
						else
						{
							event.cmd = 1;
							event.arg1 = 0;
							event.arg2 = pSysCmd->DataPackFlag;
							event.arg3 = CmdID;
							xStatus = xQueueSendToBack(lock_cmd_queue, &event, 0);
							if( xStatus != pdPASS )
							{
								fr_printf("Could not send to the queue.........\r\n");
							}
							//fr_printf("Close Lock Cmd............\r\n");
						}
						break;
					case 0xFE00:     //
					case 0xFD00:     //��������
						p = data + sizeof(SysCmdStr) + 4;    //
						data_len -= sizeof(SysCmdStr);		    //
						data_len -= 4;
						StrLen = get_comma_posi(1,p,data_len);         //
						if(StrLen > 0)
						{
							memcpy(gsm_info.ftp_server,p,StrLen - 1);     		//   
							fr_printf("FTP Server Addr:%s\r\n",gsm_info.ftp_server);	 // ��������
						}
							
						memset(Buf,'\0',20); 
						if(get_data_str(1,2,p,Buf,data_len) > 0)        // 
						{
							gsm_info.ftp_port = fr_atoi((const char *)Buf);
							fr_printf("FTP Server Port:%d\r\n",gsm_info.ftp_port);     //??????
							
						}
									
						memset(Buf,'\0',20); 
						if(get_data_str(2,3,p,Buf,data_len) > 0)       //
						{
							memcpy(gsm_info.update_file,Buf,20);
							fr_printf("FTP FtpUserName:%s\r\n",gsm_info.update_file);     //
						}
								
						memset(Buf,'\0',20); 
						if(get_data_str(3,4,p,Buf,data_len) > 0)       //
						{
							memcpy(gsm_info.ftp_passd,Buf,20);
							fr_printf("FTP FtpUserPassd:%s\r\n",gsm_info.ftp_passd);     //
						}

						memset(Buf,'\0',20); 
						if(get_data_str(4,5,p,Buf,data_len) > 0)       //
						{
							memcpy(gsm_info.update_file,Buf,20);
							fr_printf("File Name:%s\r\n",gsm_info.update_file);
						}
						
//						pUpgrade = GetUpgradeDataSpace();

//						TcpIpSocket.LinkType = 1;         //ע���������ӵ�����
//						TcpIpSocket.ResetLink = 20;       //
//						pUpgrade->UpgradeFlag = 0;		  //  
						//memcpy((unsigned char *)&ComCmdRes,(unsigned char *)pSysCmd,sizeof(ComCmdRes));
						break;
					case 0x0001:            //
						//memcpy((unsigned char *)&ComCmdRes,(unsigned char *)pSysCmd,sizeof(ComCmdRes));
//						TmpData.TTbyte[0] = GprsRecvBuf[29];
//						TmpData.TTbyte[1] = GprsRecvBuf[30]; 
//						pSysCfg->ServerPort = TmpData.IntII;     //
//						TmpData.TTbyte[0] = GprsRecvBuf[27];
//						TmpData.TTbyte[1] = GprsRecvBuf[28];
//						StrLen = TmpData.IntII - 2;
//						if(StrLen > 50)
//							StrLen = 50;
//						SL_Memcpy(pSysCfg->ServerAddr,&GprsRecvBuf[31],StrLen);   //
//						pSysCfg->ServerAddr[StrLen] = '\0';  
//						SL_Memcpy((unsigned char *)&ComCmdRes,(unsigned char *)pSysCmd,sizeof(ComCmdRes)); 
//						SaveSysCfg();                           	//
//						SL_Print("Set Server Addr......%s:%d",pSysCfg->ServerAddr,pSysCfg->ServerPort); 
//						TcpIpSocket.ResetLink = 20;              //
						break;
					case 0x0003:           //
//						SL_Memcpy((unsigned char *)&ComCmdRes,(unsigned char *)pSysCmd,sizeof(ComCmdRes));
//						TmpData32.TTbyte[0] = GprsRecvBuf[29];
//						TmpData32.TTbyte[1] = GprsRecvBuf[30];
//						TmpData32.TTbyte[2] = GprsRecvBuf[31];
//						TmpData32.TTbyte[3] = GprsRecvBuf[32];

//						if(TmpData32.LongLL < 5)						 //
//							TmpData32.LongLL = 5;
//						if(TmpData32.LongLL > 120)
//							TmpData32.LongLL = 120;
//						
//						pSysCfg->TravelUpLoadTime = TmpData32.LongLL; 
//						SL_Memcpy((unsigned char *)&ComCmdRes,(unsigned char *)pSysCmd,sizeof(ComCmdRes)); 
//						//SL_Print("Set TravelUpLoadTime......%d",pSysCfg->TravelUpLoadTime); 
//						SaveSysCfg();     //
						break;
					case 0x0005:    //��������ʱ��
//						SL_Memcpy((unsigned char *)&ComCmdRes,(unsigned char *)pSysCmd,sizeof(ComCmdRes));
//						TmpData32.TTbyte[0] = GprsRecvBuf[29];
//						TmpData32.TTbyte[1] = GprsRecvBuf[30];
//						TmpData32.TTbyte[2] = GprsRecvBuf[31];
//						TmpData32.TTbyte[3] = GprsRecvBuf[32];

//						if(TmpData32.LongLL < 600)						 //
//							TmpData32.LongLL = 600;
//						if(TmpData32.LongLL > 86400)
//							TmpData32.LongLL = 86400;					
//						pSysCfg->SleepTime 	= TmpData32.LongLL; 
//						SL_Memcpy((unsigned char *)&ComCmdRes,(unsigned char *)pSysCmd,sizeof(ComCmdRes)); 
//						SaveSysCfg();
						//SL_Print("Set Sleep Time......%d",pSysCfg->SleepTime); 
						break;
					case 0x0009:    //��ȡ��־�ļ�
//						//SL_Print("Recv Read Log Cmd\r\n");
//						SL_Memcpy((unsigned char *)&ComCmdRes,(unsigned char *)pSysCmd,sizeof(ComCmdRes));
//						p = GprsRecvBuf + sizeof(SysCmdStr) + 4;    //
//						GprsRecvLen -= sizeof(SysCmdStr);		    //
//						GprsRecvLen -= 4;
						//StrLen = GetComma(1,p,GprsRecvLen);         //
						//if(StrLen > 0)
						//{
						//	SL_Memcpy(TcpIpSocket.FtpAddr,p,StrLen - 1);     		//   
						//	SL_Print("FTP Server Addr:%s\r\n",TcpIpSocket.FtpAddr);	 //
						//	
						//}

//						SL_Memcpy(TcpIpSocket.FtpAddr,p,30);
//						for(int i = 29;i >= 0;i--)
//						{
//							if(TcpIpSocket.FtpAddr[i] == 0x20)
//								TcpIpSocket.FtpAddr[i] = '\0';
//						}
//						SL_Print("FTP Server Addr:%s\r\n",TcpIpSocket.FtpAddr);	 //��ȡ
//						SL_Memset(Buf,'\0',20); 
//						//if(GetDataStr(1,2,p,Buf,GprsRecvLen) > 0)        // 
//						//{
//							//TcpIpSocket.FtpPort = StrToDouble(Buf);
//						p += 30;
//						TmpData.TTbyte[0] = *(p + 0);
//						TmpData.TTbyte[1]  =  *(p + 1);
//						TcpIpSocket.FtpPort = TmpData.IntII;

//						SL_Print("FTP Server Port:%d\r\n",TcpIpSocket.FtpPort);     //
//							
//						//}
//									
//						//SL_Memset(Buf,'\0',20); 
//						//if(GetDataStr(2,3,p,Buf,GprsRecvLen) > 0)       //
//						//{
//						p += 2;
//						SL_Memcpy(TcpIpSocket.FtpUserName,p,15);
//						for(int i = 14;i >= 0;i--)
//						{
//							if(TcpIpSocket.FtpUserName[i] == 0x20)
//								TcpIpSocket.FtpUserName[i] = '\0';
//						}
//						SL_Print("FTP FtpUserName:%s\r\n",TcpIpSocket.FtpUserName);     //
//						//}
//								
//						//SL_Memset(Buf,'\0',20); 
//						//if(GetDataStr(3,4,p,Buf,GprsRecvLen) > 0)       //
//						//{
//						p += 15;
//						SL_Memcpy(TcpIpSocket.FtpUserPassd,p,15);
//						for(int i = 14;i >= 0;i--)
//						{
//							if(TcpIpSocket.FtpUserPassd[i] == 0x20)
//								TcpIpSocket.FtpUserPassd[i] = '\0';
//						}
//						SL_Print("FTP FtpUserPassd:%s\r\n",TcpIpSocket.FtpUserPassd);     //
//						//}
//						SaveLocalLogFile();    //������־
//						TcpIpSocket.LinkType = 1;         			//  �ϴ���־
//						TcpIpSocket.ResetLink = 10;       			//  10��֮���ϴ���־
//						TcpIpSocket.LocalLogFlag = 1;				//
						break;
					default:
						break;
			}
		}
}





/********************************
**	GNSS��λģ������̩��N303
**********************************/

void task_gsm(void *data)
{
	BaseType_t 					sem_state;
	unsigned char 			gsm_step;
	unsigned char 			tmp;
	int index;
	int socket_num;
	unsigned char 			csq_cnt;
	
	unsigned char 			tmp_str[50];
	unsigned short int	port;
	
	data = data;
	if(gsm_semaphore == NULL)
		gsm_semaphore = xSemaphoreCreateBinary();    //���յ������ź���
	if(gsm_send_semaphore == NULL)
		gsm_send_semaphore = xSemaphoreCreateMutex();
	
	init_gsm_timer();
	
	fr_init_uart4();
	fr_set_uart4_rx_hook(gsm_rx_handle);
	fr_timer3_sethook(gsm_ticks_handle);
	csq_cnt = 0;
	vTaskDelay(100);
	for(;;)
	{
		switch(gsm_step)
		{
			case 0:
				if(gsm_open_power() > 0)   //����
				{
					gsm_step++;
					gsm_info.cnt_reconnect = 0;
					gsm_info.led_state = 1;
					fr_printf("open gsm ok\r\n");
					fr_led_green_on();
				}
				else
				{
					gsm_step = 7;
				}
				break;
			case 1:               //
				if(gsm_register_network() > 0)               //ע������
				{
					gsm_step++;
					fr_printf("register net work ok.........\r\n");
				}
				else
				{
					gsm_step = 7;
				}
				break;
			case 2:								//���Ȳ�ѯ������IP��ַ��
				tmp = at_cmd_send_syn("AT+MIPCALL?\r\n", sizeof("AT+MIPCALL?\r\n"),at_get_local_ip,500,3);//��ѯSIM��״̬ 
				if(tmp == 0)
				{
					gsm_step++;       //���������豸
					break;
				}
				
				gsm_step = 4;
				break;
			case 3:								//������������
				read_config_apn(tmp_str,20);
				index = sprintf((char *)gsm_send_buf,(char *)"AT+MIPCALL=1,\"%s\"\r\n",tmp_str);
				if(at_cmd_send_syn((char *)gsm_send_buf,index,at_cmd_ok,2000,1) == 0)//
				{
					gsm_step = 7;         //
					break;
				}
				gsm_step = 2;
				break;
			case 4:                       //����Socket1
				vTaskDelay(10);
				gsm_info.cnt_reconnect++;
				memset(tmp_str,'\0',sizeof(tmp_str));
				if(gsm_info.cnt_reconnect <= 3)   //�������Ӳ��ɹ�������Ĭ�Ϸ�����
				{
					port = read_gateway_port1();
					read_gateway_addr1(tmp_str,sizeof(tmp_str));
				}
				else
				{
					port = DEF_SERVER_PORT;
					memcpy(tmp_str,DEF_SERVER_ADDR,sizeof(DEF_SERVER_ADDR));
					gsm_info.cnt_reconnect = 0;
				}	
				fr_printf("Start link server:%d,%s,%d\r\n",gsm_info.cnt_reconnect,tmp_str,port);
				tmp = at_cmd_creat_socket(1,tmp_str,port,30);
				if(tmp == 1)
				{
					vTaskDelay(100);      //
					fr_printf("creat socket.........OK\r\n");
					gsm_step++;
				}
				else if(tmp == 0)
				{
					vTaskDelay(100);
					fr_printf("creat socket.........fail\r\n");
					gsm_step = 1;
				}
				else 
				{
					gsm_step = 7;          //��������
				}
				break;							
			case 5:    
				sem_state = xSemaphoreTake(gsm_semaphore,100);     //���GSMʹ�ô�������
				if(sem_state > 0)
				{
					sem_state = read_uart_pkt(4,gsm_recv_buf,sizeof(gsm_recv_buf));
					if(at_cmd_return_mipstat(gsm_recv_buf,sem_state) > 0)       //(��������)         
						gsm_step = 1;        //
					
					if((index = look_for_str(gsm_recv_buf, (unsigned char *)"+MIPRTCP:",sem_state)) >= 0)   //���յ�����
					{
						sscanf((const char *)gsm_recv_buf + index, "+MIPRTCP: %d,%d",&socket_num,&socket_num);
						memcpy(gsm_send_buf,gsm_recv_buf + index + 15,socket_num); 
						
						analysis_down_data(gsm_send_buf,socket_num);  //�������ݵĽ���
						fr_send_console((const char *)gsm_send_buf,index);
						break;
					}
					
					at_cmd_csq(gsm_recv_buf,sem_state);
				}
				
				if(csq_cnt++ > 20)     //Լ20���ӣ�ˢ��һ��CSQ
				{
					fr_printf("Get CSQ............\r\n");
					fr_send_uart4((unsigned char *)"AT+CSQ\r\n",sizeof("AT+CSQ\r\n"));   //�����ź�ֵ
					csq_cnt = 0;
				}
				
				(gsm_info.led_state == 3)? (gsm_info.led_state = 2):(gsm_info.led_state);
				
				if(gsm_info.net_state != 2)
					break;
				
				if(gsm_info.send_socket == 0)
					break;
				gsm_info.send_state = 0;
				fr_printf("gsm run 5.......\r\n");
				gsm_info.send_state = at_cmd_mipsend_data(gsm_info.send_socket,gsm_info.send_buf,gsm_info.send_len);
				
				if(gsm_info.send_state == 1)
				{
					gsm_info.send_socket = 0;    //���ͳɹ�
					break;
				}
				fr_printf("sen fail.......\r\n");
				gsm_info.send_socket = 0;
				gsm_info.net_state = 1;
				gsm_step = 1;
				break;
			case 6:               //
				
				break;
			case 7:             //���������豸
				fr_printf("Reset GSM Model.........\r\n");
				fr_power_gprs_off();
				gsm_info.gsm_up_state = 0;
				vTaskDelay(1000);
				gsm_step = 0;
				
				break;
			default:
				gsm_step = 0;
				break;
		}
	}
}


/***************************
**	�̶�����20
**	����0����ȡʧ��
**	����20:ICCID�̶�����
**	
****************************/

unsigned char read_icc_id(unsigned char *source,unsigned char buf_size)
{
	int i;
	
	if(buf_size < 20)
		return 0;
	for(i = 0;i < 20;i++)
		*(source + i) = gsm_info.iccid[i];
	
	return 20;
}



/***************************
**	��ȡ��������״̬
**	0���������(δע�ᵽ����)
****************************/

unsigned char read_net_state(void)
{
	return gsm_info.net_state;
}
