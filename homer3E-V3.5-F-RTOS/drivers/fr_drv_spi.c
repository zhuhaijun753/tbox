


#include "stm32f10x.h"
#include "fr_drv_gpio.h"
#include "fr_drv_spi.h"
#include "board.h"

/***********************************************
**	�������ƣ�
**	��������:��ʼ����Ƭ��SPI
************************************************/
void fr_init_spi1(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;           		//SPI1_WP
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_4);
 	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;           		//SPI1_HOLD
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_0); 	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;								//SPI1_MISO
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	 
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;								//SPI1_MISO
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	 
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;								//SPI1_MOSI
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;               //SPI1_CS  //SPI1_NSS
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  SPI_FLASH_CS_HIGH();     //

  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);
  SPI_Cmd(SPI1, ENABLE);
	
}



/***************************************
**	��������������һ���ֽ�
****************************************/

unsigned char spi_flash_sendbyte(unsigned char  byte)
{
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  SPI_I2S_SendData(SPI1, byte);

  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);    //ע���������

  return SPI_I2S_ReceiveData(SPI1);
}





uint8_t spi_read_write_byte(SPI_TypeDef *SPIx, uint8_t t_data)
{
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
        ;
    SPI_I2S_SendData(SPIx, t_data);
    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
        ;
    return SPI_I2S_ReceiveData(SPIx);
}



/******************************************************************************
*                                 spi pos
******************************************************************************/
void spi_init(SPI_TypeDef *SPIx)
{
    SPI_InitTypeDef SPI_InitStructure;

    /* http://bbs.elecfans.com/jishu_426501_1_1.html*/
    if (SPIx == SPI1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    }
    else if (SPIx == SPI2)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    }

    fr_gpio_set_mode(SPI_PRO, SPI_CS_PIN, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);                /* spi cs*/
    fr_gpio_set_mode(SPI_PRO, SPI_MISO_PIN, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz);         /* spi miso*/
    fr_gpio_set_mode(SPI_PRO, SPI_SCK_PIN | SPI_MOSI_PIN, GPIO_Mode_AF_PP, GPIO_Speed_50MHz); /* spi miso*/

    GPIO_SetBits(SPI_PRO, SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN); /* ��spi��������-*/

    SPI_I2S_DeInit(SPIx);
    SPI_Cmd(SPIx, DISABLE);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; /* ˫��ȫ˫��ģʽ*/
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                      /* ��ģʽ*/
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  /* 8bit����ģʽ*/
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                        /* ʱ������ʱΪ��*/
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                       /* �ڶ���ʱ�Ӳ�������*/
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          /* nss����ʱ��*/
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; /* ������Ԥ��Ƶֵ8*/
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                 /* ���ݴ�msb��ʼ*/
    SPI_InitStructure.SPI_CRCPolynomial = 7;                           /* crcУ�����ʽ*/
    SPI_Init(SPIx, &SPI_InitStructure);

    SPI_Cmd(SPIx, ENABLE);

    spi_read_write_byte(SPIx, 0xff); /* ��������*/
}
void spi_set_speed(SPI_TypeDef *SPIx, uint8_t speed)
{
    SPI_Cmd(SPIx, DISABLE);
    SPIx->CR1 &= 0XFFC7;
    SPIx->CR1 |= speed;
    SPI_Cmd(SPIx, ENABLE);
}


/**************************
**
***************************/

int storage_medium_init(void)
{
    int res;

    spi_init(SPI_DEV);

#if defined(NOR_FLASH)
    GPIO_SetBits(SPI_PRO, SPI_CS_PIN);
    res = nor_flash_init();
#elif defined(SD_CARD)
    res = dev_sd_init();
#endif
    return res;
}







