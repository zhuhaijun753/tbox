


#ifndef _DATA_TYPE_H
#define _DATA_TYPE_H


#define BLIND_NUM  20        	/****/                                 
#define BLIND_BUF  390	      /****/


struct time_str
{
	unsigned char year;
	unsigned char mon;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
};


/**************************
**	CAN���ݽṹ��
***************************/

struct can_tx_data_str
{
    unsigned int id;													//CANID��
    unsigned char data0;											//CAN��������   ��8�ֽ�
    unsigned char data1;
    unsigned char data2;
    unsigned char data3;
    unsigned char data4;
    unsigned char data5;
    unsigned char data6;
    unsigned char data7;
};



/**********************
**	
***********************/

struct can_rx_data_str
{
	unsigned int can_id;								//CAN��ϢID
	unsigned char data[8];						  //CAN��Ϣ����
};


/****************************
**	CAN����״̬
*****************************/

#pragma pack(1)
struct lock_expect_str
{
	unsigned char lock_expect_state;   	//0:������1��һ��������2����������
	unsigned char mon_expect_state;    	//�������״̬  0���������ܴ�;1:�������ܹر�
	unsigned char crc_area;							//����У������
};
#pragma pack()




struct lock_current_str
{
	unsigned char lock_current_state;			//��ǰ����״̬
	unsigned char mon_current_state;			//��ǰ����״̬
	unsigned char handshake_state;         //����״̬
	unsigned char	lock_execute_state;			//����ִ��״̬
	unsigned char mon_execute_state;      //����ִ��״̬��
};



//
typedef union  
{
	unsigned short int			value;
	unsigned char						byte[2];
}int16_to_char;


//
typedef union  
{
	unsigned int						value;
	unsigned char						byte[4];
}int32_to_char;



/*****************************************
**
**
*******************************************/
#pragma pack(1)
struct can_info_str
{
	unsigned short int 				sys_vol;           	 					//ϵͳ��ѹ
	unsigned char 						temp_water;           				//��ȴˮ��
	unsigned short int 				engine_actual_rotate;		 			//������ʵ��ת��
	unsigned short int 				engine_set_rotate;     				//�������趨ת��
	unsigned char 						engine_torque;								//������ʵ��Ť��
	unsigned char 						fuel_temp;										//ȼ���¶�
	unsigned short int 				oil_temp;         						//�����¶�
	unsigned char 						air_pressure;         				//����ѹ��
	unsigned short int   			engine_nacelle_temp;					//�����������¶�
	unsigned short int 				air_temp;											//�����¶�
	unsigned char 						entere_air_temp;							//�����������¶�
	unsigned int							engine_work_time;							//����������ʱ��
	unsigned short int 				travel_speed;									//��ʻ�ٶ�
	unsigned int 							once_travel;									//������ʽ�ٶ�
	unsigned int 							total_travel;  								//����ʽ����
	unsigned int 							once_fuel;      							//�����ͺ�
	unsigned int 	   					total_fuel;  									//���ͺ�
	unsigned char							relative_oil_pressure;				//��Ի���ѹ��
	unsigned char 						absolute_oil_pressure;				//���Ի��ͼ���
	unsigned char							relative_add_pressure;					//�����ѹѹ��
	unsigned char 						absolute_add_pressure;					//������ѹѹ��
	unsigned short int				fuel_position;								//ȼ��λ��
	unsigned char		 					fuel_percent;									//ȼ�Ͱٷֱ�
	unsigned char 						oil_position;									//����Һλ
	unsigned short int  			crankcase_pressure;   				//������ѹ��
	unsigned char 						cool_pressure;  							//��ȴҺѹ��
	unsigned char 						cool_position;								//��ȴҺλ��
	unsigned char     				lock_car_state;								//����״̬
	unsigned char							activate_status;   						//�豸���״̬
	unsigned char 						key_status;     							//Key��֤״̬
	unsigned char 						ter_id_status;								//�豸ID״̬
	unsigned char 						work_flag;   									//��ҵ������־
	unsigned char 						car_warn_value1;							//��������1
	unsigned char 						car_warn_value2;							//��������2
	unsigned char 						food_full_warn;								//��������
	unsigned char 						clutch_state;  								//�����״̬
	unsigned short int	 			strip_rotate;									//��Ƥ��ת��
	unsigned short int	 			lift_rotate;									//������ת��
	unsigned short int				cut_table_high;								//��̨�߶�
};
#pragma pack()



/**********************
**	GNSS��λ��Ϣ
*************************/

struct gnss_info_str
{
		double 							longitude;   					// ���� xxxyyyyyy = xxx.yyyyyy(��)*1000000*/
    double 							latitude;    					// γ��
			
		unsigned char 			longitude_ew; 				// ������ 0:�� 1:��
    unsigned char 			latitude_sn;  				// �ϱ�γ 0:�� 1:��
    struct time_str			utc_time;
		struct time_str 		btc_time;
		unsigned int 				timestamp;         		// ʱ���
    unsigned char 			state;        				// �Ƿ�λ "A":δ��λ
    
    unsigned short int	altitude;    					// ���� ��
		unsigned short int 	heading;     					// ���� ��
		
		double 							speed;       					// �ٶ� ����/Сʱ
		double              hdop;        					// ˮƽ��������
  
    unsigned char 			satellite_num; 				// ʹ�����Ǹ���
		unsigned char 			bd_sate_num; 					// ������������(����)
		unsigned char 			gps_sate_num;					// ����΢������()
};




/***********************
**	GPRS������Ϣ
************************/

struct gsm_info_str
{
		unsigned char 			socket[4];							//socket���
		unsigned char 			server_addr[50];				//���ص�ַ
		unsigned short int	port;										//���ض˿�
	
    unsigned char	 			gsm_up_state;         	// GSMģ�鿪��״̬ 
		unsigned char 			gsm_at_state;
		unsigned char	  		sim_state;            	// sim��״̬: 0 ����; 1 �쳣	
		unsigned int 				csq;                 		// �ź�ǿ��*/
		unsigned char 			iccid[25];           		// ICCID 
		unsigned int 				net_register;       		// ����ע��״̬ 1 �������� 5 �������� 
		unsigned char 			pdp_state;           		// pdp����״̬ 1 �Ѽ��� 0 δ���� 
		unsigned int 				net_state;          		// 0����������δע�ᵽ���Ź�Ӧ�����磻1���Ѿ�ע�ᵽ���磬���Ը���GPRS����;2���Ѿ����ӵ�������
		unsigned char 			send_state;							//����״̬
		unsigned char 			led_state;							/* 0����������δע�ᵽ���Ź�Ӧ�����磬LED�Ʋ�����
																								 * 1���Ѿ�ע�ᵽ���磬LED��1��������˸��
																								 * 2���Ѿ����ӵ���������LED��3��������˸��
																								 * 3�����ڷ������ݣ�LED��0.1S��˸��
																								 */
		unsigned char 			link_type;							//�������� 0�������ӣ�1 TCP���ӣ�2��FTP����
		unsigned char 			reset_link;							//�������ӱ�־
		unsigned char 			ftp_server[50];					//FTP��������ַ
		unsigned short int  ftp_port;								//FTP�˿�
		unsigned char 			ftp_admin[20];					//FTP�û���
		unsigned char 			ftp_passd[20];             //FTP����
		unsigned char 			update_file[20];
		unsigned int 				update_crc;              //�����ļ�У����
	
    unsigned char	 			disable;              	// GPRS �������� (ʹ��FTPʱ ����) 
    
     
    unsigned char 			sms_run_flag;         	// �������ݽ�����־: RT_FALSE δ���յ�����; RT_TRUE ���յ����� 
    
    
    
    unsigned int 	  		dbm;               			// �ŵ������� 
		 
		
    unsigned char 			socket_status;        	// socket����״̬ RT_TRUE�������� RT_FALSE���ӶϿ�  
    unsigned short int 	cnt_reconnect;      		// socket�������Ӵ��� �������3�γ�ʼ��ʧ�� ��Ҫ��λϵͳ ()
    unsigned short int 	cnt_reset;          		// ��ʼ��ʧ�ܺ�����ʱ����ʱʱ����� 
    unsigned  int 			cnt_total_send;    			// ���������ۼƷ��ͳɹ��������� 
		
		unsigned int 				local_ip[4];
		unsigned char 			send_buf[1024];					//socket���ͻ�����
		unsigned char 			send_len;               //���ͳ���
		unsigned char 			send_socket;						//socket���к�
};




/************************************************
**	
************************************************/


struct config_str
{
	unsigned char 					flag;                    //���ñ�־
	
	unsigned char 					terminal_id[18];      					//�豸ID
	unsigned char           terminal_type;		 							//�豸����
	unsigned char 					fir_ver;			 									//�̼��汾��
	unsigned char 					gateway_addr1[50];      					//���ص�ַ
	unsigned int short 			gateway_port1;     	 						//���ض˿�   
	unsigned int 						run_time;            	 					//�Ͽ�ACC������ʱ��
	unsigned int 						sleep_time;           					//˯��ʱ��
	unsigned int 						travel_upload_cycle;    				//��ʻ�ϴ�����
	unsigned int        		work_upload_cycle;      				//��ҵ�ϴ�����
	unsigned short int  		distance_upload;     	 					//�����뱨λ
	unsigned short int  		azimuth_upload;       					//����Ǳ�λ
	unsigned char 					car_type;             					//��װ��������
	unsigned char 					user_code;            					//�û�����
	unsigned short int			can_num;												//CANЭ��� 					
	unsigned char           apn[20];												//GSMģ������
	unsigned char						user[20];												//GSMģ��APN�û���
	unsigned char						password[20];										//GSMģ��APN����
	unsigned char 					icc_id[25];											//SIM��ICC-ID��
	unsigned char 					hard_ware;											//Ӳ���汾
	unsigned char 					dev_id[3];											//�豸ID
	unsigned char						dev_secret[3];									//�豸��Կ
	
	
	unsigned char 					gateway_addr2[50];      				//���ص�ַ
	unsigned int short 			gateway_port2;     	 						//���ض˿�  
	
	unsigned char 					gateway_addr3[50];      				//���ص�ַ
	unsigned int short 			gateway_port3;     	 						//���ض˿�  
	
	unsigned char 					gateway_addr4[50];      				//���ص�ַ
	unsigned int short 			gateway_port4;     	 						//���ض˿�  
	
	unsigned char 					socket1;
	unsigned char 					socket2;
	
	unsigned char 					socket3;
	unsigned char 					socket4;
};



/***************************
**	�豸��Ϣ
****************************/
#pragma pack(1)

struct in_info_str
{
	unsigned char 				acc_state;								//ACC״̬  0  1
	unsigned char 				gnss_ant_state;           //Gnss��������״̬  0:������1��δ����;2:δ����
	unsigned char 				shell_state;							//���״̬ 					0:������1��δ����;2:δ����
	unsigned short int 		power_vol;               //�ⲿ�����ѹ
	unsigned short int 		battery_vol;             //��ص�ѹ
	unsigned short int 		mcu_temp;                //��Ƭ���¶�
};

#pragma pack()


#pragma pack(1)
struct run_data_str
{

	unsigned int 				acc_total_time;             //ACC�ܼƹ���ʱ�䣬�Է���Ϊ��λ
	unsigned int				engine_total_time;					//�������ܹ���ʱ�� ��������ת�ٴ��� 750ת��ʱ��
	
	double 							longitude;   								//���� ���������һ��λ�ã�
  double 							latitude;    								// γ��
};
#pragma pack()





//��Ϣͷ
#pragma pack(1)
typedef struct 
{
    unsigned char 				frame_start[3];			//
    unsigned char					msg_id;          		//
    unsigned char 				device_id[16];   		// 
    unsigned char 				CarTypeNum;             // 
    unsigned char 				DataPackFlag;			//
    unsigned char 				msg_body_num;     		//
    unsigned short int 		msg_len;        		//
}MsgHeadStr;

#pragma pack()


#pragma pack(1)
typedef struct 
{
	unsigned short int 		MsgDeviceType;                  //
	unsigned short int 		MsgDeviceLen;                 	//
	
	unsigned char      		MsgManuNum;            			//    	      	
	unsigned char      		MsgTerminalType;        		// 
	unsigned char 				MsgUserNum;                     //
	unsigned char      		MsgAppVer1;              		// 
	unsigned char 				MsgAppVer2;           			//
	unsigned char      		MsgHardwareVer;             	//
	
}MsgDeviceStr;
#pragma pack()


/***********************************************
**	
************************************************/
#pragma pack(1)

typedef struct 
{
	unsigned short int  	MsgGnssType;                  	//
	unsigned short int 		MsgGnssLen;                   	//
	
	unsigned int       		MsgGnssLatitude;        		//
	unsigned int       		MsgGnssLongitude;             	//
	unsigned short int	 	MsgGnssSpeed;           		//
	unsigned short int	 	MsgGnssAzimuth;         		//
	short int 						MsgGnssAltitude;        		//
	unsigned char      		MsgGnssYear;                  	//
	unsigned char      		MsgGnssMon;                   	//
	unsigned char      		MsgGnssDay;						//
	unsigned char      		MsgGnssHour;                 	//
	unsigned char      		MsgGnssMin;                   	//
	unsigned char      		MsgGnssSec;                   	//
	unsigned char      		MsgGnssSatelliteNum;         	//
	unsigned char      		MsgGnssViewNum;              	//
	unsigned short 				MsgGhdopV;						//
	unsigned char      		MsgGnssStatus;              	//
}MsgGnssStr;

#pragma pack()



/*************************************************
**
*************************************************/
#pragma pack(1)

typedef struct
{
	unsigned short int   			MsgInputType;					//
	unsigned short int 				MsgInputLen;					//
		
	unsigned char      				MsgInputIo;						//
	unsigned char 						MsgAcc;     					//
	unsigned char 						MsgMoto;						//
	unsigned short int 				MsgInputFrq1;					//
	unsigned short int 				MsgInputFrq2;					//
	unsigned short int 				MsgInputFrq3;					//
	unsigned short int 				MsgInputFrq4;					//
	unsigned short int 				MsgPowVol;						//
	unsigned short int 				MsgBatteryVol;					//
	unsigned short int 				MsgInputVol1;					//
	unsigned short int 				MsgInputVol2;					//
	unsigned int							MsgWarnValue;					//
	unsigned char 						MsgLine;   						//
} MsgInputStr;

#pragma pack()



/************************
**	�������׻� CAN��Ϣ��
**	
*************************/

#pragma pack(1)
typedef struct
{
	unsigned short int 					MsgMaizeMechType;				//
	unsigned short int					MsgMaizeMechLen;        		//

	unsigned short int 					MsgSysVol;           	 		//
	unsigned char 							MsgTempWater;           		//
	unsigned short int 					MsgEngineRotate;		 		//
	unsigned short int 					MsgEngineRotateSet;     		//
	unsigned char 							MsgEngineTorque;				//
	unsigned char 							MsgFuelTemp;					//
	unsigned short int 					MsgOilTemp;         			//
	unsigned char 							MsgAirPressure;         		//
	unsigned short int   				MsgEngineNacelleTemp;			//
	unsigned short int 					MsgAirTemp;						//
	unsigned char 							MsgEnteredAirTemp;				//
	unsigned int								MsgEngineWorkTime;				//
	unsigned short int 					MsgTravelSpeed;					//
	unsigned int 								MsgOnceTravel;					//
	unsigned int 								MsgTotalTravel;  				//
	unsigned int 								MsgOnceFuel;      				//
	unsigned int 	   						MsgTotalFuel;  					//
	unsigned char								MsgRelativeOilPressure;			//
	unsigned char 							MsgAbsoluteOilPressure;			//
	unsigned char								MsgRelativeAddPressure;			//
	unsigned char 							MsgAbsoluteAddPressure;			//
	unsigned short int					MsgFuelNum;						//
	unsigned char		 						MsgFuelPercent;					//
	unsigned char 							MsgOilPosition;					//
	unsigned short int  				MsgCrankcasePressure;   		// 
	unsigned char 							MsgCoolPressure;  				//
	unsigned char 							MsgCoolPosition;				//
	unsigned char     					MsgLockCarState;				//
	unsigned char								MsgActivateStatus;   			//
	unsigned char 							MsgKeyStatus;     				//
	unsigned char 							MsgTerIDStatus;					//
	unsigned char 							MsgWorkFlag;   					//
	unsigned char 							MsgCarWarnValue1;				//
	unsigned char 							MsgCarWarnValue2;				//
	unsigned char 							MsgFoodFullWarn;				//
	unsigned char 							MsgClutchState;  				//
	unsigned short int	 				MsgStripRotate;					//
	unsigned short int	 				MsgLiftRotate;					//
	unsigned short int					MsgCutTableHigh;				//
	unsigned short int    			MsgNC1;							//
	unsigned short int     			MsgNC2;							//
	unsigned int  							MsgNC3;							//
	unsigned int 								MsgNC4;     					//
}MsgCanMaizeStr;



/******************************************
**	
**
******************************************/
#pragma pack(1)
typedef struct
{
	unsigned char 			FrameStart[3];			// 
  unsigned char				msg_id;          		//
  unsigned char 			device_id[16];   		// 
  unsigned short int 	DataPackFlag;			//
  unsigned char 			msg_body_num;     		// 	
  unsigned short int 	msg_len;        		//  
}SysCmdStr;
#pragma pack()




/************************************************
**	
*************************************************/
#pragma pack(1)

typedef struct
{
	unsigned char 			DataBuf[BLIND_BUF];     				//
	unsigned short int 	DataLen;          				//
	unsigned char 			DataCrc;          				//
	unsigned char       DataBig;          				//
	
}FiFoStr;
#pragma pack()

/*************************************************************
**	
*************************************************************/
#pragma pack(1)

typedef struct
{
	unsigned short int 		QNum;             						//
	unsigned short int 		QWrite;           						//
	unsigned short int 		QRead;            						//
	FiFoStr            		QData[BLIND_NUM];       				//
}SendQueueStr;
#pragma pack()






#endif

