

/*****************************
**	
******************************/

#include "stm32f10x.h"

#include "fr_drv_mem.h"
#include "FreeRTOS.h"
#include "board.h"


/*******************************/

static unsigned char  					Update_Flag = 'N';									//������־
//static unsigned short int 			gNumofRecPackets = 1; 		 					//��ǰ����֡��
//static unsigned int 						gFirmwareDataSize;	       					//�����̼��ܳ���
//static unsigned int 						gFirmwareDataPaketNum;		 					//�����̼���֡��
//static unsigned int 						gFirmwareCheckSum;    		 					//�����̼�У���
static unsigned int 						gDataLenSum     = 0;  		 					//���յ��������ֽ�����
//static unsigned int 						gCntSend        = 0;  		 					//������������ʱ����
static unsigned int 						FlashDestination = ADDR_APP_BKP;   	//


/********************************************
**	����̼�ҳ���С
*********************************************/
unsigned int FLASH_PagesMask(unsigned int Size)
{
    unsigned int pagenumber = 0x0;
    unsigned int size = Size;

    if((size % PAGE_SIZE) != 0)
    {
        pagenumber = (size / PAGE_SIZE) + 1;
    }
    else
    {
        pagenumber = size / PAGE_SIZE;
    }
    return pagenumber;

}



/*******************************
**	��������
*******************************/
FLASH_Status  EraseSectors(unsigned int destination_address, unsigned int flashsize)
{
    FLASH_Status FLASHStatus  = FLASH_COMPLETE;
    unsigned int 								NbrOfPage = 0;
    unsigned int EraseCounter = 0x0;
    unsigned short int PageSize = PAGE_SIZE;

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    NbrOfPage = FLASH_PagesMask(flashsize);						//������Ҫ����Flash��ҳ

    FLASH_Unlock();
    for (EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_ErasePage(destination_address + (PageSize * EraseCounter));
        if(FLASHStatus != FLASH_COMPLETE)
        {
            fr_printf("FLASH_ErasePage is failed, error_num = %d\r\n", FLASHStatus);
        }
    }
    FLASH_Lock();

    return FLASHStatus;
}


/********************************
**	���յ�������д������
*********************************/
unsigned int Receive_Packet(unsigned char *data, unsigned int length)
{
    unsigned char *pRamSource = 0;
    unsigned short int i;
    FLASH_Status index = FLASH_COMPLETE;
    int datalen;

    datalen = length;
    gDataLenSum += datalen;
    pRamSource = data;
    taskENTER_CRITICAL();   //�ر��ж�
    FLASH_Unlock();

    for (i = 0; (i < datalen) && (FlashDestination <  ADDR_APP_BKP + APP_DATA_SIZE); i += 4)
    {
        index = FLASH_ProgramWord(FlashDestination, *(unsigned int *)pRamSource);/*�ѽ��յ������ݱ�д��Flash��*/
        if(index != FLASH_COMPLETE)
        {
            fr_printf("* Flash write data error.\r\n");
            return 1;
        }
        if (*(unsigned int *)FlashDestination != *(unsigned int *)pRamSource)
        {
            fr_printf("* Data to check failure.\r\n");
            return 1;/*flash write data not same*/
        }
        FlashDestination += 4;
        pRamSource += 4;
    }
    FLASH_Lock();
    taskEXIT_CRITICAL();   //�����ж�
    return 0;
}



/***********************************
**	д������־
************************************/
unsigned char WriteUpdateFlag(unsigned int updateFlag)
{
    unsigned int writeData = 0;
    unsigned int readData  = 0;

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    writeData = updateFlag;
    writeData = (writeData << 8) & 0xFF00;

    taskENTER_CRITICAL();
    FLASH_Unlock();
    FLASH_ErasePage(ADDR_DATA_FILED);					//������Ӧ��ַ��1ҳ(2K)
    FLASH_ProgramWord(ADDR_DATA_FILED, writeData);
    FLASH_Lock();
    taskEXIT_CRITICAL();

    readData = (*(unsigned int *)(ADDR_DATA_FILED));
    if(readData == writeData)
        return 1;
    else
        return 0;
}
/************************************************
**	��������־
*************************************************/
unsigned int ReadUpdateFlag(void)
{
    unsigned int updateFlag = 0;
	
    updateFlag = (*(unsigned int *)(ADDR_DATA_FILED));
    updateFlag = (updateFlag >> 8) & 0xFF;
	
    return updateFlag;
}

/***********************************
**	������������  OK    
***********************************/
FLASH_Status BackupEraseHandle(void)
{
    FLASH_Status res = FLASH_COMPLETE;
    res = EraseSectors(ADDR_APP_BKP, APP_DATA_SIZE);
    if(res != FLASH_COMPLETE)     //����ʧ��
    {
        Update_Flag = 'R';
        WriteUpdateFlag(Update_Flag);
        fr_printf("Erase the error code :%d \r\n", res); /*Erase operation completed successfully!*/
        __set_FAULTMASK(1);
        NVIC_SystemReset();   //д���־����������
    }
    return res;
}




/***************************
**	д����ʧ�ܱ�־��������
****************************/

void UpgradeFailReset(void)
{
	Update_Flag = 'R';     								//��������豸�������豸
  WriteUpdateFlag(Update_Flag);
  delay_ms(10);         									//�����ʱ ������
  __set_FAULTMASK(1);										//�ر��ж�
  NVIC_SystemReset();    								//����ϵͳ  �˴����᷵�أ�ֱ��
}



/**************************
**	д�����ɹ���־������λ��Ƭ��
***************************/

void UpgradeOkReset(void)
{
	Update_Flag = 'Y';     								//��������豸�������豸
  WriteUpdateFlag(Update_Flag);					//д�����ɹ���־
  delay_ms(10);         									//�����ʱ ������
  __set_FAULTMASK(1);										//�ر��ж�
  NVIC_SystemReset();    								//����ϵͳ  �˴����᷵�أ�ֱ��
}


