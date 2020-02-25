

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"


#include "data_type.h"
#include "fr_drv_rtc.h"


static struct time_str 			t_tm;      //ע���������



static struct time_str 			time;      //�豸ʱ��


static unsigned int 				timestamp;





QueueHandle_t  							set_time_queue;   				//������б���-����ʱ��

/***********************************
**	��������:
**	��������:ʱ���ת����
**	����һ��ָ��
***********************************/

struct time_str *timestamp_to_calendar(unsigned int t)
{
    unsigned int i;
    unsigned int tmp;
    unsigned int tmp1;

    tmp = t % 86400;
    t_tm.hour = tmp / 3600;                    //ʱ
    t_tm.min = tmp % 3600 / 60;                //��
    t_tm.sec = tmp % 3600 % 60;                //��

    tmp = t / 86400;                           //������

    for(i = 2000;i < 2100;i++)       //ȷ������

    {
        if(((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))
        {
            if(tmp < 366)
                break;        //�˳�forѭ��
            tmp -= 366;
        }
        else
        {
            if(tmp < 365)
                break;      //�˳�forѭ��
            tmp -= 365;
        }
    }

    t_tm.year = i - 2000;   //��

    for(i = 1;i <= 12;i++)       //ȷ����
    {
        switch(i)
        {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                if(tmp < 31)
                {
                    t_tm.mon = i;
                    i = 13;
                }
                else
                {
                    tmp -= 31;
                }
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                if(tmp < 30)
                {
                    t_tm.mon = i;
                    i = 13;
                }
                else
                {
                    tmp -= 30;
                }
                break;
            case 2:
                tmp1 = t_tm.year + 2000;
                if(tmp1 % 400==0 || (tmp1 % 4==0 && tmp1 % 100 != 0))
                {
                    if(tmp < 29)
                    {
                        t_tm.mon = i;
                        i = 13;
                    }
                    else
                    {
                        tmp -= 29;
                    }
                }
				else
				{
					if(tmp < 28)
					{
						t_tm.mon = i;
						i = 13;
					}

					else
					{
						tmp -= 28;
					}
				}
                break;
        }
    }

    t_tm.day = tmp + 1;   //  ����
		
		return &t_tm;
}



/**********************************************************************
**	��������:
**	��������:ʱ��תʱ���
**	�������:
**	�������:
**********************************************************************/

unsigned int calendar_to_timestamp(struct time_str *t)
{
    unsigned int rv;
    unsigned int tmp;
    unsigned short int i;

    rv = 0;
    tmp = t->year + 2000;
    for(i = 2000; i < tmp;i++)
    {
        if(i % 400==0 || (i % 4==0 && i % 100 != 0))
        {
            rv += 366;         //����
        }
        else
        {
            rv += 365;         //������
        }
    }

    tmp = i;

    for(i = 1;i < t->mon;i++)
    {
        switch(i)
        {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                rv += 31;
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                rv += 30;
                break;
            case 2:
                if(tmp % 400==0 || (tmp % 4==0 && tmp % 100 != 0))
                    rv += 29;        //����
                else
                    rv += 28;        //������
                break;
        }
    }

    for(i = 1;i < t->day;i++)
    {
        rv++;
    }

    rv *= 86400;

    rv = rv + t->hour * 3600 + t->min * 60 + t->sec;

	return rv;               //
}



/********************************************
**	RTC��������
**	
*******************************************/

void task_rtc(void *data)
{
	BaseType_t status;
	struct time_str *ptime;
	unsigned int tmp;
	int t_t;
	
	data = data;
	
	fr_init_rtc();
	
	set_time_queue = xQueueCreate(1, sizeof(unsigned int));  //����һ������
	
	for(;;)
	{
		timestamp = fr_read_rtc();                   //
		ptime = timestamp_to_calendar(timestamp);  
		
		time.year = ptime->year;
		time.mon = ptime->mon;
		time.day = ptime->day;
		time.hour = ptime->hour;
		time.min = ptime->min;
		time.sec = ptime->sec;
		
		status = xQueueReceive(set_time_queue, &tmp,50);    //
		if(status > 0)
		{
			fr_printf("%d,%d,%d:%d,%d,%d\r\n",time.year,time.mon,time.day,time.hour,time.min,time.sec);
			t_t = timestamp - tmp;
			if(t_t > 0 || t_t <= -5)
			{	
				fr_set_rtc(tmp - 3);                                //����ʱ��
				fr_printf("start verfy time............\r\n");        //
			}
			
		}
		
		//fr_printf("%d,%d,%d:%d,%d,%d\r\n",time.year,time.mon,time.day,time.hour,time.min,time.sec);
		
	}
}






/************************
**	��ȡϵͳʱ��
*************************/

void read_time(struct time_str *ptime)
{
	ptime->year = time.year;
	ptime->mon = time.mon;
	ptime->day = time.day;
	ptime->hour = time.hour;
	ptime->min = time.min;
	ptime->sec = time.sec;
}



/************************
**	��ȡϵͳʱ���
**	����ϵͳʱ���
*************************/

unsigned int read_timestamp(void)
{
	return timestamp;
}




