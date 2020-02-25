
/***********************
**	FileName:
**	Time:
*************************/

#include <string.h>

#include "DataType.h"
#include "CanProcess.h"
#include "SysProcess.h"

#include "bsp_can.h"         															//
#include "BspTick.h"
#include "bsp_mem.h"
#include "bsp_in.h"
#include "BspCfg.h"


/***********����Ԥ�����***************/

#define CMD_LOCK1   						1
#define CMD_UNLOCK 							2

#define CMD_MONITOR_ON					1
#define CMD_MONITOR_OFF					2


/************���ݱ��س���**************/
//static  
Str_CanTxdData LockCarCmd 					=	{0x00};  //��������
const Str_CanTxdData UpLockCarCmd 	= {0x00000501,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00};	 //��������
const Str_CanTxdData MonOnCmd 			= {0x00000502,0x02,0x06,0x02,0x00,0x00,0x00,0x00,0x00};	 //��ؿ���
const Str_CanTxdData MonOffCmd 			= {0x00000502,0x01,0x04,0x01,0x00,0x00,0x00,0x00,0x00};	 //��عر�
const Str_CanTxdData HeartBeat 			= {0x00000505,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00};	 //������



/************����ȫ�ֱ���*****************/

static CanLockCarStr								CanLockTcp;						//CAN�������ƿ� ��������ķ��͡�������״̬

static CanLockStateStr              CanLockState;      //����״̬

//static unsigned char 								AccStatusBack;
/******************************
**	������������
******************************/
void LockCarInfoHandle(unsigned char cmd,unsigned char n)
{
    switch (cmd)
    {
			case CMD_LOCK1:              //����   CMD_LOCK1:1,����
				CanLockTcp.LockCarCmdFlag = 1; 
				CanLockTcp.LockCarCmdStatus = 1;
				CanLockTcp.n = n;
				CanLockState.LockCurrentState = 1;    //�����ɹ�
				CanLockState.LockExpectState = 1;
				
				SaveLockCarInfo(&CanLockState);            //����������Ϣ
				//BasicPrintf(">��������\r\n");
        break;
			case CMD_UNLOCK:             //����   CMD_UNLOCK:2,����   
				CanLockTcp.LockCarCmdFlag = 2;
				CanLockTcp.LockCarCmdStatus = 1;
				CanLockState.LockCurrentState = 0;    //�����ɹ�
				CanLockState.LockExpectState = 0;
				SaveLockCarInfo(&CanLockState);            //����������Ϣ
				//BasicPrintf(">��������\r\n");
        break;
			case 3:
				CanLockTcp.LockCarCmdFlag = 1; 
				CanLockTcp.LockCarCmdStatus = 1;
				break;
			default :
				CanLockTcp.LockCarCmdFlag = 0;    //������ִ�������
				CanLockTcp.LockCarCmdStatus = 0;
				//BasicPrintf(">���������޷�ʶ��\r\n");
        break;
    }
}



/********************************
**	��ؿ��أ�
*********************************/
 void MonitorInfoHandle(unsigned char cmd)
{
    switch (cmd)
    {
			case CMD_MONITOR_ON:              //
				CanLockTcp.MonCarCmdFlag = 1;
				CanLockTcp.MonCarCmdStatus = 1;
				//BasicPrintf(">��ش�����\r\n");
        break;
			case CMD_MONITOR_OFF:              //
				CanLockTcp.MonCarCmdFlag = 2;
				CanLockTcp.MonCarCmdStatus = 1;
				//BasicPrintf(">��عر�����\r\n");
        break;
			default :
				CanLockTcp.MonCarCmdFlag = 0;
				CanLockTcp.MonCarCmdStatus = 0;
        //BasicPrintf(">��������޷�ʶ��\r\n");
        break;
    }
}


/*****************************
**
**	����豸��������״̬����û�н��յ��������������£�
**	ÿ��ACC����֮�󣬶�Ҫ������������
*****************************/

void InitCanLockInfo(void)
{
	ReadLockCarInfo(&CanLockState);  			//���������������Ϣ
		
	if(CanLockState.LockExpectState == 1)  //����
	{
			//LockCarInfoHandle(3);     //��������
	}
}


/*******************************
**	
**	����ⲿ��Դ״̬
**	����ⲿ��Դ���磬ִ����������
*******************************/

void MonExternPower(void)
{
	static unsigned char counter;
	
	SysDataInfoStr 					mData;
	
	GetSysDataInfo(&mData); 
	
	if(mData.PowerVol < 60)   //
	{
		counter++;
		if(counter > 10)
		{
		}
			//LockCarInfoHandle(3);
	}
	else
	{
		counter = 0;
	}
}


/*****************************
**	��������:
**	��������:
*****************************/
void  SendLockCarCmd(unsigned short int n)
{
	IntToChar Tmp16;
	SysConfigStr	Tmp;
	
	
	LockCarCmd.id = 0x18FE0DEE;
	Tmp16.IntII = n * 8;
	
	LockCarCmd.data0 = Tmp16.TTbyte[0];
	LockCarCmd.data1 = Tmp16.TTbyte[1];   //����ת�ٵ�ֵ
	LockCarCmd.data2 = 0xFF;
	LockCarCmd.data3 = 0xFF;
	LockCarCmd.data4 = 0xFF;
	ReadSysCfgInfo(&Tmp);
	LockCarCmd.data5 = Tmp.DevID[0];	 //
	LockCarCmd.data6 = Tmp.DevID[1];	 //
	LockCarCmd.data7 = Tmp.DevID[2];   //
	CAN_TransmitBeatCanMsg(1, 250, &LockCarCmd);
}


/******************************
**	
*******************************/

void  SendActivateCmd(unsigned char n)
{
	IntToChar Tmp16;
	SysConfigStr	Tmp;
	
	if(n == 0)
	{
		LockCarCmd.id = 0x18FE0BEE;
		Tmp16.IntII = 0x5176;
		LockCarCmd.data0 = Tmp16.TTbyte[0];    //��������
		LockCarCmd.data1 = Tmp16.TTbyte[1];
		ReadSysCfgInfo(&Tmp);
		LockCarCmd.data2 = Tmp.DevID[0];  //
		LockCarCmd.data3 = Tmp.DevID[1];	//
    LockCarCmd.data4 = Tmp.DevID[2];	//
		
		LockCarCmd.data5 = Tmp.DevSecret[0];  //GPS��Կ
		LockCarCmd.data6 = Tmp.DevSecret[1];
		LockCarCmd.data7 = Tmp.DevSecret[2]; 
		CAN_TransmitBeatCanMsg(1, 250, &LockCarCmd);
		return;
	}
	
	if(n == 1)
	{
		LockCarCmd.id = 0x18FE0BEE;
		Tmp16.IntII = 0x6715;
		LockCarCmd.data0 = Tmp16.TTbyte[0];    //��������
		LockCarCmd.data1 = Tmp16.TTbyte[1];
		ReadSysCfgInfo(&Tmp);
		LockCarCmd.data2 = Tmp.DevID[0];//Tmp.DevSecret[0];  //GPS ID,��ʱʹ���豸��Կ
		LockCarCmd.data3 = Tmp.DevID[1];//Tmp.DevSecret[1];
		LockCarCmd.data4 = Tmp.DevID[2];//Tmp.DevSecret[2];  
		
		LockCarCmd.data5 = Tmp.DevSecret[0];  //GPS��Կ
		LockCarCmd.data6 = Tmp.DevSecret[1];
		LockCarCmd.data7 = Tmp.DevSecret[2]; 
		CAN_TransmitBeatCanMsg(1, 250, &LockCarCmd);
	}
}

/*******************************
**	����CAN����
**	
*******************************/

void ProcessCanLockCar(void)
{
	static struct stopwatch32 sw2;
	static unsigned char step2 = 0;
	static unsigned char Num;
	
	CanDataStr TmpCan;
	
	GetCanData(&TmpCan);
	
	if(ReadSysRunStatus() > 0)     //�����ж�CAN��־
		return;
	
	switch(step2)
	{
		case 0:      																	//����
			if(CanLockTcp.LockCarCmdFlag == 1)   				//
			{
				if(CanLockTcp.n == 0)
				{
					Num = 0;
					step2 = 1;      //1������
					break;
				}
				else if(CanLockTcp.n == 1)
				{
					if(TmpCan.LockCarState == 1)     //���ж�����״̬
					{
						Num = 0;
						step2 = 3;     //��ȥ����
						break;
					}
					Num = 0;
					step2 = 9;     //2������
					break;
				}
			}                                  				 //����
			if(CanLockTcp.LockCarCmdFlag == 2)   			 //����
			{
				Num = 0;
				step2 = 3;
				break;
			}
			if(CanLockTcp.MonCarCmdFlag == 1)        //�򿪼��
			{
				Num = 0;
				step2 = 5;
				break;
			}
			if(CanLockTcp.MonCarCmdFlag == 2)   			//�رռ��
			{
				Num = 0;
				step2 = 7;
				break;
			}
			break;
		case 1:
			SendLockCarCmd(1000);     //����
			InitStopwatch32(&sw2);
			step2++;
			break;
		case 2:
			if(ReadStopwatch32Value(&sw2) < 99)   		//
				break;
			Num++;
			if(Num > 3)    														//��ȡ30
			{
				CanLockTcp.LockCarCmdStatus = 0;   				//
				CanLockTcp.LockCarCmdFlag = 0;   					//ֹͣ����
				step2 = 0;
				
				break;
			}
			
			step2 = 1;
			break;
		case 3:
			SendLockCarCmd(3500);      //����
			InitStopwatch32(&sw2);
			step2++;
			break;
		case 4:
			if(ReadStopwatch32Value(&sw2) < 99)   			//
				break;
			Num++;
			if(Num > 3)                                 //��������
			{
				CanLockTcp.LockCarCmdStatus = 0;  					//
				
				CanLockTcp.LockCarCmdFlag = 0;   						//
				if(CanLockTcp.n == 1)
				{
					Num = 0;
					step2 = 9;     //2������
					CanLockTcp.n = 0;
					break;
				}
				step2 = 0;
				break;
			}
			step2 = 3;
			break;
		case 5:
			SendActivateCmd(1);					//���ͼ�����
			InitStopwatch32(&sw2);
			step2++;
			break;
		case 6:
			if(ReadStopwatch32Value(&sw2) < 99)   													//
				break;
		
			Num++;
			if(Num > 3)
			{
				step2 = 0;
				CanLockTcp.MonCarCmdStatus = 0;
				CanLockTcp.MonCarCmdFlag = 0;
				break;
			}
			step2 = 5;
			break;
		case 7:
			SendActivateCmd(0);   				//ȡ������
			InitStopwatch32(&sw2);
			step2++;
			break;
		case 8:
			if(ReadStopwatch32Value(&sw2) < 99)   //
				break;
			Num++;
			if(Num > 3)
			{
				step2 = 0;
				CanLockTcp.MonCarCmdStatus = 0;
				CanLockTcp.MonCarCmdFlag = 0;
				break;
			}
			step2 = 7;
			break;
		case 9:
			SendLockCarCmd(0);     //����
			InitStopwatch32(&sw2);
			step2++;
			break;
		case 10:
			if(ReadStopwatch32Value(&sw2) < 99)   		//
				break;
			Num++;
			if(Num > 3)    														//��ȡ30
			{
				CanLockTcp.LockCarCmdStatus = 0;   				//
				CanLockTcp.LockCarCmdFlag = 0;   					//ֹͣ����
				step2 = 0;
				CanLockTcp.n = 0;
				break;
			}
			
			step2 = 9;
			break;
		default:
			step2 = 0;
			break;
	}
	
	
}



/********************************
**	��ȡCAN�������ƿ�
*********************************/

void GetCanLockCarTcp(CanLockCarStr *p)
{
	memcpy(p,(unsigned char *)&CanLockTcp,sizeof(CanLockCarStr));
}


/*****************************
**	����������״̬
******************************/

void ClearLockCarStatus(void)
{
	CanLockTcp.LockCarCmdStatus = 0;
}


/******************************
**	�����������״̬
*******************************/

void ClearMonStatus(void)
{
	CanLockTcp.MonCarCmdStatus = 0;
}


/******************File End****************/

