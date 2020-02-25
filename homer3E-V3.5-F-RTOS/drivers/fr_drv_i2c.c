


#include "stm32f10x.h"

#include "fr_drv_i2c.h"



#define nop() 									__nop()

#define scl_out_high()         	GPIOB->BSRR = GPIO_Pin_10
#define scl_out_low()						GPIOB->BRR  = GPIO_Pin_10
   
#define sda_out_high()         	GPIOB->BSRR = GPIO_Pin_11
#define sda_out_low()         	GPIOB->BRR  = GPIO_Pin_11

#define scl_read()      				GPIOB->IDR  & GPIO_Pin_10
#define sda_read()     					GPIOB->IDR  & GPIO_Pin_11



/********************************************************************************
**	�������ƣ�void i2c_delay(void)
**	����������
**	˵����
********************************************************************************/

static void i2c_delay(void)
{        
  unsigned char i = 40; 
  
  while(i--) 
	{
		nop();
	}
}


/************************************
**	��������:uint8 CpuFlashEraseSector(uint16 Sector)
**	��������:������
**	�������:Addr:��ַ��Data��������ݵĵ�ַ��Len��Ҫд�����ݵĳ���;
**	0:�ɹ���1��ʧ��
*************************************/

void fr_init_i2c(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;    //I2C ����
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}


/****************************************************************************************
�������ƣ�
��������������I2C��ʼ�ź�
����ֵ��
****************************************************************************************/

static unsigned char i2c_start(void)
{
	sda_out_high();              //����� �����ͷ������ˣ�
	scl_out_high();
	
	i2c_delay();
	
	if(!sda_read())
		return 0; 
	
	sda_out_low();
	i2c_delay();
	
	if(sda_read())
		return 0;  

	sda_out_low();                 	//
	i2c_delay();                  	//
	
	return 1;
}


/******************************************************************
�������ƣ�void i2c_stop(void)
����������ֹͣI2C�ź�

******************************************************************/

static void i2c_stop(void)
{
	scl_out_low();
	i2c_delay();
	sda_out_low();
	i2c_delay();
	scl_out_high();
	i2c_delay();
	sda_out_high();
	i2c_delay();                   
}


/*************************************************************************
�������ƣ�void i2c_ack(void)
��������������ACK�źţ�ACK��Ӧ���ź�
*************************************************************************/

static void i2c_ack(void)
{      
	scl_out_low();
	i2c_delay();
  
	sda_out_low();
	i2c_delay();
	scl_out_high();
	
	i2c_delay();
	scl_out_low();
	i2c_delay();
}


/**********************************************************************************
�������ƣ�void i2c_no_ack(void)
����������
�����������
�����������
***********************************************************************************/
static void i2c_no_ack(void)
{    
	scl_out_low();
	i2c_delay();
	
	sda_out_high();
	
	i2c_delay();
	scl_out_high();
	
	i2c_delay();
	scl_out_low();
	i2c_delay();
}


/****************************************************************************
** 	�������ƣ�unsigned char i2c_send_byte(unsigned char sendbyte) 
**	���������������ֽ�
**	���������Ҫ���͵�һ���ֽ�����
**	���������0--���ʹ���
					1--������ȷ

*****************************************************************************/

unsigned char i2c_write_byte(unsigned char sendbyte) 
{
	unsigned char i;
	signed int cnt;
	
	cnt = 52000;
      
	for(i = 0;i < 8;i++)
	{	
		scl_out_low();
		i2c_delay();
		
		if(sendbyte & 0x80) 
			sda_out_high();
		else
			sda_out_low(); 
		
		sendbyte <<= 1;		
		i2c_delay();
		scl_out_high();
		i2c_delay();
	}
	
	scl_out_low();
	i2c_delay();
	sda_out_high();
	i2c_delay();
	scl_out_high();
	
	while(sda_read() && ((cnt--)) > 0)
	{
		nop();
	}
	
	if(cnt <= 0)
		return 0;
	
	scl_out_low();
	
	return 1;
}


/***************************************************************************************
�������ƣ�unsigned char i2c_receive_byte(unsigned char *byte)  
�����������������Դӻ����͵�һ���ֽ�����
���������ָ������ֽ����ݴ�ŵ�λ�á�
***************************************************************************************/

unsigned char i2c_read_byte(unsigned char *byte)  
{ 
    unsigned char i;
    unsigned char receive_byte;

		receive_byte = 0;
		sda_out_high();
	
    for(i = 0;i < 8;i++)
    {
			scl_out_low();
      i2c_delay();
      scl_out_high();
			i2c_delay();
			
      if(sda_read())
      {
        receive_byte = (receive_byte << 1) | 0x01;
      }
			else
			{
				receive_byte = receive_byte << 1;
			}
			
			i2c_delay();
    }
		
    scl_out_low();
		*byte = receive_byte;
		
    return 1;
}








/**************************************************************************************************************************
**	�������ƣ�signed char i2c_read(unsigned short int slave_addr,unsigned char *data,unsigned short int mem_addr,unsigned short data_len)
**	�������������ӻ�����
**	���������slave_addr----�ӻ���ַ
					data---�������ݻ�����ָ��
					mem_addr---�ӻ����ݵ�ַ
					data_len---�ӻ����ݳ���

*************************************************************************************************************************/

signed char fr_i2c_read(unsigned short int slave_addr,unsigned char *data,unsigned short int mem_addr,unsigned short data_len)
{
	unsigned short int 			i;
	
	if(i2c_start() < 1)                                   //���Ϳ�ʼ�ź�
		return 0;
		
	if(i2c_write_byte((unsigned char)slave_addr & 0xFFFE) == 0)     //���ʹӻ���ַ
		return 0;
		
	if(slave_addr & 0xFF00)                                        //�������ݵ�ַ
	{  
		if(i2c_write_byte(*((unsigned char *)&mem_addr + 1)) == 0)
			return 0;
	}
		
	if(i2c_write_byte(*(unsigned char *)&mem_addr ) == 0)
		return 0;

	if(i2c_start() < 1)                                         		//���Ϳ�ʼ�ź�
		return 0;

	if(i2c_write_byte((unsigned char)(slave_addr | 0x0001)) == 0)   	//���ʹӻ���ַ������־λ
		return 0;
	
	for(i = 0; i < data_len;i++)                               			//ѭ�����մӻ�����
	{
		if(i2c_read_byte(data + i) == 0)
			return 0;
			
		if(i == (data_len - 1))
		{
				i2c_no_ack();                                       				//���һ�����ݲ�����ACK�ź�
		}
		else
		{
			i2c_ack();
		}
	}
	i2c_stop();                       //ֹͣI2C�ź�
	               								
	return 1;
}


/***************************************************************************************************************************************************
�������ƣ�unsigned char i2c_write(unsigned short int slave_addr,unsigned char *data,unsigned short int mem_addr,unsigned short int data_len)
����������дһ�����ֽ�����
**������� ��slave_addr�������������������ӻ��豸��ַ	
						 data----ָ���������ݣ�ָ��Ҫд���ֽڻ�����
						 mem_add----�ӻ���ַ�ռ�
						 data_len-----���ݳ���
**������� ��0----д���ݴ���
						 1----д������ȷ
˵����salve_addr ,��������Ǵӻ���ַ��֮����ʹ�������ֽڣ��Ѹ��ֽ���Ϊ���ִӻ����ݿռ��ַ�ĳ��ȣ�
									��ЩI2C�ӻ��豸���ݿռ��ַ�������ֽڣ�����AT24C256EEPROM�洢�����ڷ��������豸ʱ��
									��Ҫ���������ֽڵ����ݿռ��ַ��
									����ЩI2C�ӻ��豸���ݿռ��ַֻ��һ���ֽڣ������ⲿRTCʱ���豸���ڷ��������豸ʱֻ��Ҫ����
									һ���ֽ����ݿռ��ַ���ɡ�
									�������Ǵӻ���ַ�ĸ߰�λ���������ݿռ��ַ��8λ�Ļ���16λ�ģ��߰�λ0�����ʵ���8��ַ���ݿռ䣬
									��8λ����0��˵�����ʵ����ݵ�ַ�ռ���16λ�ġ�
*****************************************************************************************************************************************************/

unsigned char fr_i2c_write(unsigned short int slave_addr,unsigned char *data,unsigned short int mem_addr,unsigned short int data_len)
{
	unsigned int i;

	if(i2c_start() < 1)    //���Ϳ�ʼ�ź�
		return 0;
	
	if(i2c_write_byte((unsigned char)slave_addr) == 0)
		return 0;
		

	if(slave_addr & 0xFF00)                                       
	{
		if(i2c_write_byte(*((unsigned char *)&mem_addr + 1)) == 0)
			return 0;
			
	}
	if(i2c_write_byte(*(unsigned char *)&mem_addr ) == 0)
		return 0;
		
	for(i = 0;i < data_len;i++)
	{
		if(i2c_write_byte(*(data + i)) == 0)
			return 0;
			
	}
		
	i2c_stop();
	
	return 1;
}









/***************File End****************/





