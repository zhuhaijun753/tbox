
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "FreeRTOS.h"

#include "data_type.h"
#include "gsm.h"
#include "pro_data.h"
#include "gnss.h"
#include "version.h"
#include "mon.h"
#include "fr_drv_mem.h"
#include "can.h"



static struct config_str	config_info;    //�豸������Ϣ



/***************************************
**	������Ϣ�������ⲿEEPROM�ڲ�
****************************************/

unsigned char read_config_info(void)
{
	unsigned char rv;
	
	rv = fr_fram_read(0,(unsigned char *)&config_info, sizeof(config_info));
	
	if(config_info.flag != 0x55)
	{
		config_info.flag = 0x56;
		
		memset(config_info.terminal_id,'\0',sizeof(config_info.terminal_id));          //�豸��
		memcpy(config_info.terminal_id,"YMJ41219606100001",sizeof("YMJ41219606100001"));          //����ʹ��
		config_info.terminal_type = 0;		 							//�豸����
		config_info.fir_ver = 12;			 									//�̼��汾��
		memcpy(config_info.gateway_addr1,"hdyx-2013.xicp.net",sizeof("hdyx-2013.xicp.net"));      					//���ص�ַ
		config_info.gateway_port1 = 13011;     	 						//���ض˿� 
		config_info.socket1 = 1;
		
		
		
		config_info.run_time = 300;            	 					//�Ͽ�ACC������ʱ��(��λ���룬���磺300S��5����)
		config_info.sleep_time = 21600;          					//˯��ʱ��(��λ����)
		config_info.travel_upload_cycle = 60;    				//��ʻ�ϴ�����(��λ����)
		config_info.work_upload_cycle = 1;      				//��ҵ�ϴ�����(��λ����)
		config_info.distance_upload = 200;     	 					//�����뱨λ (��λ����)
		config_info.azimuth_upload = 30;       					//����Ǳ�λ(��λ����)
		config_info.car_type = 3;             					//��װ��������
		config_info.user_code = 3;            					//�û�����
		config_info.can_num = 22;												//CANЭ��� 					
		memcpy(config_info.apn,"CMNET",sizeof("CMNET"));    //APN
		memcpy(config_info.user,"",sizeof(""));												//GSMģ��APN�û���
		memcpy(config_info.password,"",sizeof(""));										//GSMģ��APN����
		memset(config_info.icc_id,'\0',sizeof(config_info.icc_id));											//SIM��ICC-ID��
		config_info.hard_ware = 35;									//Ӳ���汾
		memset(config_info.dev_id,0,8);
		memset(config_info.dev_secret,0,8);
	}
	config_info.gateway_port1 = 27000;
	config_info.travel_upload_cycle = 20;
//	config_info.run_time = 10;
//	memcpy(config_info.gateway_addr1,"123.127.244.154",sizeof("123.127.244.154"));      					//���ص�ַ
//	config_info.gateway_port1 = 13012;     	 						//���ض˿� 
	
	return rv;
}


/***************************************
**	������Ϣ�������ⲿEEPROM�ڲ�
****************************************/
unsigned char read_config_info_hard_ware(void)
{
	return config_info.hard_ware;
}



/***************************************
**	������Ϣ�������ⲿEEPROM�ڲ�
****************************************/
unsigned short int read_config_info_run_time(void)
{
	return config_info.run_time;
}


unsigned short int read_config_info_sleep_time(void)
{
	return config_info.sleep_time;
}

/***************************************
**	������Ϣ�������ⲿEEPROM�ڲ�
****************************************/

unsigned char save_config_info(void)
{
	unsigned char rv;
	
	config_info.flag = 0x55;
	
	rv = fr_fram_write(0,(unsigned char *)&config_info, sizeof(config_info));
	
	return rv;
}

/***************************************
**	�����豸��
****************************************/

unsigned char read_terminal_id(unsigned char *buf,unsigned char buf_size)
{
	if(buf_size < 16)
		return 0;
	
	memcpy(buf,config_info.terminal_id,16);
	
	return 16;
}

unsigned char read_gateway_addr1(unsigned char *buf,unsigned char buf_size)
{
	if(buf_size < 50)
		return 0;
	
	memcpy(buf,config_info.gateway_addr1,50);
	
	return 50;
}

unsigned short int read_gateway_port1(void)
{
	return config_info.gateway_port1;
}


/************************
**	��ȡ��������
*******************/

unsigned char read_car_type(void)
{
	return config_info.car_type;
}


/*****************************
**	��������״̬
******************************/

unsigned char read_config_state(void)
{
	return config_info.flag;
}


/*****************************
**	��������-�ϴ�ʱ������
******************************/

unsigned int read_config_travel_upload_cycle(void)
{
	return config_info.travel_upload_cycle;
}

/**********************
**	����
***********************/

unsigned int read_config_work_upload_cycle(void)
{
	return config_info.work_upload_cycle;
}


/**********************
**	����
***********************/

void read_config_apn(unsigned char *buf,unsigned char buf_size)
{
	if(buf_size < 20)
		return;
	memcpy(buf,config_info.apn,20);
}


void read_config_dev_id(unsigned char *buf,unsigned char buf_size)
{
	if(buf_size < 3)
		return;
	memcpy(buf,config_info.dev_id,3);
}


void read_config_dev_secret(unsigned char *buf,unsigned char buf_size)
{
	if(buf_size < 3)
		return;
	memcpy(buf,config_info.dev_secret,3);
}


/**************************************************
**	��������:
***************************************************/

unsigned char analysis_config_info(unsigned char *source,unsigned short len)
{
	unsigned char *p = source;
	unsigned char i;
	unsigned char 			tmp_s[40];
	
	if(*(p) != ':')
		return 0;
	
	p += 1;
	memset(config_info.terminal_id,'\0',sizeof(config_info.terminal_id));    //�����豸��
	for(i = 0;i < 16;i++)                                  			//
		config_info.terminal_id[i] = *(p + i);
	
	p += i;
	if(*p != ',')
		return 0;
				
	i = get_data_str(1,2,p,tmp_s,len);    //���ص�ַ
	if(i > 0)
	{
		tmp_s[i] = '\0';
		memcpy(config_info.gateway_addr1,tmp_s,i + 1);          	//				
	}
				
	i = get_data_str(2,3,p,tmp_s,len);              //�˿ں�
	if(i > 0)
	{	
		tmp_s[i] = '\0';
		config_info.gateway_port1 = fr_atof((const char *)tmp_s);         			//
	}
				
	i = get_data_str(3,4,p,tmp_s,len);  
	if(i > 0)
	{
		tmp_s[i] = '\0';
		config_info.sleep_time = fr_atof((const char *)tmp_s);         			//����ʱ��
	}
				
	i = get_data_str(4,5,p,tmp_s,len);  
	if(i > 0)
	{
		tmp_s[i] = '\0';
		config_info.distance_upload = fr_atof((const char *)tmp_s);   			//�����ϴ�
	}


	i = get_data_str(5,6,p,tmp_s,len);  
	if(i > 0)
	{
		tmp_s[i] = '\0';
		config_info.travel_upload_cycle = fr_atof((const char *)tmp_s);  			//
	}

	i = get_data_str(6,7,p,tmp_s,len);  
	if(i > 0)
	{
		tmp_s[i] = '\0';
		config_info.hard_ware = fr_atof((const char *)tmp_s);      			//
	}
				
	i = get_data_str(7,8,p,tmp_s,len);  
	if(i > 0)
	{
		tmp_s[i] = '\0';
		config_info.can_num = fr_atof((const char *)tmp_s);   			//����CANЭ���
					
		if(config_info.can_num == 0x40 )                       //
		{
			config_info.car_type = 0x0A; 
		}
		else
		{
			config_info.can_num = 0x40;                       	//
			config_info.car_type = 0x0A;												//
		}
	}
	i = get_data_str(8,9,p,tmp_s,len);  
	if(i > 0)
	{
		memcpy(config_info.apn,tmp_s,i);           			//�豸APN
		config_info.apn[i] = '\0';
	}
	
	i = get_data_str(9,10,p,tmp_s,len);              
	if(i > 0)
	{
		if(tmp_s[0] == '0')
		{
			
		}
		else if(tmp_s[0] == '1')
		{
			
		}
	}
	
	for(i = 0;i < 3;i++)
	{
		len = 0;
		len = tmp_s[i * 2 + 14] - 0x30;
		len <<= 4;
		len += (tmp_s[i * 2 + 15] - 0x30);
		config_info.dev_secret[i] = len;
	}
	
	for(i = 0;i < 3;i++)
	{
		len = 0;
		len = config_info.terminal_id[i * 2 + 10] - 0x30;
		len <<= 4;
		len += (config_info.terminal_id[i * 2 + 11] - 0x30);
		config_info.dev_id[i] = len;
	}
	
	read_icc_id(config_info.icc_id,20);
	
	return 1;
}



/**************************************************
**	��������:
**	��������:����������Ϣ
***************************************************/
unsigned short int build_config_info(char *buf,unsigned short int buf_size,unsigned char flag)
{
	unsigned short int 			len;
	unsigned int 						tmp;
	int 										i;
	unsigned char 					tmp_c[20];
	struct time_str 				*ptime;
	
	if(buf_size < 200)
		return 0;
	
	if(flag == 0)
		config_info.flag = 0x56;
	
	len = 0; 
	memcpy((char *)buf,(char *)"homer3x:",sizeof("homer3x:") - 1);
	len += sizeof("homer3x:") - 1;				
	for(i = 0;i < 16;i++)
		*(buf + len + i) = config_info.terminal_id[i];   			//
	len += i;
	*(buf + len) = ',';                               		//
	len++;
	
	if(flag == 0)
		read_icc_id((unsigned char *)buf + len,20);
	else
		memcpy(buf + len,config_info.icc_id,20);
	
	len += 20;
	buf[len++] = ',';                                		//
	
	
	tmp = strlen((const char *)config_info.gateway_addr1);
	memcpy(buf + len,config_info.gateway_addr1,tmp);     //
	len += tmp;
	buf[len++] = ',';                                //

	tmp = int_to_str(config_info.gateway_port1,(char *)tmp_c,sizeof(tmp_c));
	for(i = 0;i < tmp;i++)            //���ض˿ں�
		buf[len++] = tmp_c[i];
	buf[len++] = ',';

	tmp = strlen((const char *)config_info.apn);
	memcpy(buf + len,config_info.apn,tmp);        // GSM���� 
	len += tmp;
	buf[len++] = ',';                             //
					
	
	if(read_gnss_state() == 'A')                    //��λ״̬
	{
		buf[len] = '8';
		buf[len + 1] = '0';
	}
	else
	{
		buf[len] = '0';
		buf[len + 1] = '0';
	}
	len += 2;
					
	tmp = read_gnss_latitude() * 100000;                   //ά��(��γ��)
	
	tmp = int_to_str(tmp,(char *)tmp_c,sizeof(tmp_c));
	memset(buf + len,'0',9);
	memcpy(buf + len,tmp_c,tmp);              								//
	len += 9;
	tmp = read_gnss_longitude() * 10000;                  //����
	tmp = int_to_str(tmp,(char *)tmp_c,sizeof(tmp_c));                       
	memset(buf + len,'0',9);
	memcpy(buf + len,tmp_c,tmp);              //
	len += 9;

	tmp = sprintf((char *)&buf[len],(char *)"%04d%04d",read_gnss_speed(),read_gnss_heading());
	
	len += 8;

	ptime = (struct time_str *)tmp_c;
	read_gnss_utc_time(ptime);
	
	tmp = sprintf((char *)&buf[len],(char *)"%02d%02d%02d%02d%02d%02d",ptime->year,ptime->mon,ptime->day,ptime->hour,ptime->min,ptime->sec);
	len += tmp;
	buf[len++] = ',';
	
	tmp = read_satellite_num();			
	tmp = int_to_str(tmp,(char *)tmp_c,sizeof(tmp_c));
	memset(buf + len,'0',9);
	memcpy(buf + len,tmp_c,tmp);
	len += 2;
	buf[len++] = ',';
	
	tmp = read_gsm_signal();                              //�ź�ֵ
	tmp = int_to_str(tmp,(char *)tmp_c,sizeof(tmp_c));
	memset(buf + len,'0',9);
	memcpy(buf + len,tmp_c,tmp);
	len += 2;
	buf[len++] = ',';
					
	tmp = read_power_vol();    //�ⲿ�����ѹ
	tmp = int_to_str(tmp,(char *)tmp_c,sizeof(tmp_c));          //
	memcpy(buf + len,tmp_c,tmp);
	len += tmp;
	buf[len++] = ',';
					
	tmp = read_batter_vol();
	tmp = int_to_str(tmp,(char *)tmp_c,sizeof(tmp_c));          //��ص�ѹ
	memcpy(buf + len,tmp_c,tmp);
	len += tmp;
	buf[len++] = ',';
					
	buf[len++] = read_acc_state() + 0x30;   				// ACC״̬
	buf[len++] = ',';
						
	buf[len++] = read_net_state() + 0x30;   //��������״̬ 0��δע������ 1���Ѿ���������	2���Ѿ����ӵ�������
	buf[len++] = ',';
					
	tmp = read_fir_ver();
	buf[len++] = tmp / 10 + 0x30;
	buf[len++] = '.';
	buf[len++] = tmp % 10 + 0x30;
					
	buf[len++] = ',';

	
	tmp = int_to_str(config_info.sleep_time,(char *)tmp_c,sizeof(tmp_c));      		//
	memcpy(buf + len,tmp_c,tmp);
	len += tmp;
	buf[len++] = ',';

	
	tmp = int_to_str(config_info.distance_upload,(char *)tmp_c,sizeof(tmp_c));      		//   //
	memcpy(buf + len,tmp_c,tmp);
	len += tmp;
	buf[len++] = ',';

	tmp = int_to_str(config_info.travel_upload_cycle,(char *)tmp_c,sizeof(tmp_c));  
	memcpy(buf + len,tmp_c,tmp);
	len += tmp;
	buf[len++] = ',';

	buf[len++] = config_info.hard_ware / 10 + 0x30;      //
	buf[len++] = '.';
	buf[len++] = config_info.hard_ware % 10 + 0x30;
	buf[len++] = ',';
					
	buf[len++] = '1';	   							//
	buf[len++] = ',';

	buf[len++] = '1';									//
	buf[len++] = ',';

	buf[len++] = '1';								//
	buf[len++] = ',';
					
	buf[len++] = read_ant_state() + 0x30;    //����״̬
	buf[len++] = ',';
			
	buf[len++] = read_shell_state() + 0x30; 	//���״̬
	buf[len++] = ',';

	buf[len++] = '1';                                //
	buf[len++] = ',';

	buf[len++] = '1';								//
	buf[len++] = ',';

	
	tmp = sizeof("0102") - 1;                            	//
	
	memcpy(buf + len,"0102",sizeof("0102") - 1);
	len += tmp;
	tmp = read_fir_ver();

	buf[len++] = tmp / 10 + 0x30;     
	buf[len++] = tmp % 10 + 0x30;
	buf[len++] = tmp / 10 + 0x30;   //
	buf[len++] = tmp % 10 + 0x30;   //
	buf[len++] = ',';
	
	tmp = int_to_str(config_info.can_num,(char *)tmp_c,sizeof(tmp_c));      //CANЭ���
	memcpy(buf + len,tmp_c,tmp);
	len += tmp;
	buf[len++] = ',';

	//
					
	if(read_can_connect_state() == 0)						  //CAN����״̬
		buf[len++] = '0';                              //
	else
		buf[len++] = '1';                              //
					
	buf[len++] = ',';
	
	tmp = read_fir_ver();
	buf[len++] = tmp / 10 + 0x30; ;   //��Ƭ���汾��
	buf[len++] = '.';
	buf[len++] = tmp % 10 + 0x30; ;   //
	buf[len++] = ',';
	
	buf[len++] = read_mon_expect_state() + 0x30;   //��������״̬
	buf[len++] = ',';
	
	buf[len++] = read_lock_expect_state() + 0x30;   //��������״̬
	buf[len++] = ',';
	
	buf[len++] = 0x0d;
	buf[len++] = 0x0a;   //�س����з�
	
	return len;
}






