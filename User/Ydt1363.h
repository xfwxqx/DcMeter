/**
  *****************************************************************************
  *                            ����YDT1363_3Э��
  *
  *                       (C) Copyright 2017-2018, ***
  *                            All Rights Reserved
  *****************************************************************************
  *
  * @File    : Ydt1363.h
  * @By      : XFW
  * @Version : V1.0
  * @Date    : 2017 / 03 / 28
  *
  *****************************************************************************
  *                                   Update
  * @Version : V1.0
  * @By      : XFW
  * @Date    : 2017 / 03 / 08
  * @Brief   : The Fisrt Edition
  				��δ֧�ֻ�ȡ��ʷ���ݺ͸澯	
  *
  *
  *****************************************************************************
**/


#ifndef __YDT1363_h_ 
#define __YDT1363_h_


/**
  ****************************** Support C++ **********************************
**/
#ifdef __cplusplus
	extern "C"{
#endif		
/**
  *****************************************************************************
**/
		

/******************************************************************************
                             �ⲿ����ͷ�ļ�
                  Ӧ�õ���ͬ������ͷ�ļ����������޸ļ���                        
******************************************************************************/
#include "UserCommon.h"
#include "RN8209C.h"       

#define YDT1363_FUNC_RET_FALSE 		-1
#define YDT1363_FUNC_RET_SUCCESS 	0
/**/
#define DATE_YEAR_IS_VALID(x) (((x)>=2000)&&((x)<=2099))
#define DATE_MONTH_IS_VALID(x) (((x)>=1)&&((x)<=12))
#define DATE_DAY_IS_VALID(x) (((x)>=1)&&((x)<=31))
#define DATE_HOUR_IS_VALID(x) (((x)>=0)&&((x)<=23))
#define DATE_MINUTE_OR_SECOND_IS_VALID(x) (((x)>=0)&&((x)<=59))

#define SWAP_DWORD(x)	((((x)<<24)&0xFF000000)|(((x)<<8)&0xFF0000)|(((x)>>8)&0xFF00)|(((x)>>24)&0xFF))
#define SWAP_WORD(x)	((((x)>>8)&0xFF)|(((x)<<8)&0xFF00))
#define SWAP_LONG(x)	((signed long int)((((x)<<24)&0xFF000000)|(((x)<<8)&0xFF0000)|(((x)>>8)&0xFF00)|(((x)>>24)&0xFF)))
#define SWAP_SHORT(x)	((signed short int)((((x)>>8)&0xFF)|(((x)<<8)&0xFF00)))

#define UNIT_TYPE	"DCM4806"	//�豸�ͺ�
#define SOFTWARE_VERSION	"0100"	//���������汾
#define VENDER		"JSYAAO"		//��������
#define USER_DEFINE_SOFT_VERSION    "20170831"

#define YDT1363_3_DATABUF_MAX	170  	//����Э��ת�����ݲ�����󳤶�
#define DCMETER_LOOP_CNT_MAX	6		//ֱ���������·��	
#define DCMETER_DATA_RATIO		100		//ֱ������ϴ�������ʵ�����ݵı���

#define YDT1363_3_PROTOCAL_VER						0x10
#define YDT1363_3_PROTOCAL_ADDR						0x01
#define YDT1363_3_PROTOCAL_CID1_DCMETER				0x2d








#define YDT1363_3_PROTOCAL_CID2_GET_DATE			0x4d    //��ȡʱ����Ϣ��4DH��
#define YDT1363_3_PROTOCAL_CID2_SET_DATE			0x4e    //��������ʱ�䣨4EH��
#define YDT1363_3_PROTOCAL_CID2_GET_VERSION			0x4f    //��ȡѶЭ��汾�ţ�4FH��
#define YDT1363_3_PROTOCAL_CID2_GET_DEVICE_ADDR		0x50    //��ȡ�豸��ַ��50H��
#define YDT1363_3_PROTOCAL_CID2_GET_VENDER			0x51    //��ȡ������Ϣ��51H��
#define YDT1363_3_PROTOCAL_CID2_GET_ANALOG			0x85    //��ȡģ������������
#define YDT1363_3_PROTOCAL_CID2_GET_HISTORY_DATA	0x86    //��ȡģ����ʷ����(86H)

#define YDT1363_3_PROTOCAL_CID2_GET_ENERGY_MONTH	0x87    //��ȡ�¶������(87H)
#define YDT1363_3_PROTOCAL_CID2_GET_PARAM           0x88    //��ȡ������
#define YDT1363_3_PROTOCAL_CID2_SET_PARAM	        0x89    //���ñ�����

#define YDT1363_3_PROTOCAL_CID2_GET_METER_READING	0x90    //��ȡ������
#define YDT1363_3_PROTOCAL_CID2_SET_METER_READING	0x91    //���ó�����

#define YDT1363_3_PROTOCAL_CID2_GET_ALARM			0x92    //��ȡ�澯״̬
#define YDT1363_3_PROTOCAL_CID2_GET_HISTORY_ALARM	0x95    //��ȡ��ʷ�澯
#define YDT1363_3_PROTOCAL_CID2_GET_MODULE_PARAM	0x93    //��ȡģ�����
#define YDT1363_3_PROTOCAL_CID2_SET_MODULE_PARAM	0x94    //����ģ�����


/************************** ������  **************************/
//�ָ�����Ĭ������
#define YDT1363_3_PROTOCAL_CID2_FACTORY_RESET	0xe0  
//ƫ��У׼
#define YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_OFF	0xe1   
//����У׼
#define YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_GN	0xe2 
//����У׼
#define YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_P	0xe3 
//��ȡУ׼Դ��ѹ�͵���������
#define YDT1363_3_PROTOCAL_CID2_GET_CS5460A_DC_MAX	0xe4
//����У׼Դ��ѹ�͵���������
#define YDT1363_3_PROTOCAL_CID2_SET_CS5460A_DC_MAX	0xe5  
//��ȡ6��ͨ����CTֵ
#define YDT1363_3_PROTOCAL_CID2_GET_CT	0xe6
//����6��ͨ����CTֵ
#define YDT1363_3_PROTOCAL_CID2_SET_CT	0xe7
//��������
#define YDT1363_3_PROTOCAL_CID2_POWER_RESET	0xe8
//�ֶ�У׼
#define YDT1363_3_PROTOCAL_CID2_SECTION_CALIBRATION     0xe9
//��ȡCS5460У׼ֵ
#define YDT1363_3_PROTOCAL_CID2_GET_CS5460CALI     0xea
//��ȡָ��ҳ���ϰ�ҳ����
#define YDT1363_3_PROTOCAL_CID2_GET_PAGE_DATA     0xeb
//��ȡ��������ϵ��
#define YDT1363_3_PROTOCAL_CID2_GET_ENERGY_RATIO     0xec
//���õ�������ϵ��
#define YDT1363_3_PROTOCAL_CID2_SET_ENERGY_RATIO     0xed
//��ȡģ��������չ������*10000��
#define YDT1363_3_PROTOCAL_CID2_GET_ANALOG_EXT     0xee
//��ȡ�����汾��
#define YDT1363_3_PROTOCAL_CID2_GET_SOFT_VER     0xef
//ϵͳ��λ
#define YDT1363_3_PROTOCAL_CID2_SET_TO_BOOT 	0xf0  

#define YDT1363_3_PROTOCAL_RTN_NORMAL				0x0   	//����
#define YDT1363_3_PROTOCAL_RTN_VER_ERROR			0x01   	//Э��汾��
#define YDT1363_3_PROTOCAL_RTN_CHKSUM_ERROR			0x02   	//CHKSUM��
#define YDT1363_3_PROTOCAL_RTN_LCHKSUM_ERROR		0x03   	//LCHKSUM��
#define YDT1363_3_PROTOCAL_RTN_CID2_INVALID			0x04   	//CID2��Ч
#define YDT1363_3_PROTOCAL_RTN_FORMAT_ERROR			0x05   	//�����ʽ��
#define YDT1363_3_PROTOCAL_RTN_DATA_INVALID			0x06  	//��Ч����
#define YDT1363_3_PROTOCAL_RTN_DATA_NONE			0x07  	//����ʷ����
#define YDT1363_3_PROTOCAL_RTN_CID1_INVALID			0xe1  	//CID1��Ч
#define YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL		0xe2  	//����ִ��ʧ��
/* ����Э�����ݽṹ���� 
*������Ϊԭʼ����ֵ(δASCIIת��), LengthΪ��������ʵ�ʳ���
*/
typedef struct __attribute__((packed)) _PROTOCAL_YDT1363_3
{
	uint8_t		Ver;
	uint8_t		Addr;
	uint8_t		CID1;
	uint8_t		CID2;
	uint16_t		Length;			
	uint8_t		DataBuf[YDT1363_3_DATABUF_MAX];
}PROTOCAL_YDT1363_3,*PPROTOCAL_YDT1363_3;


/*************************   ģ���� ***************************************/

typedef struct __attribute__((packed)) _DCMETER_LOOP_ANALOG{
	//uint8_t		LoopNum;		//��·���
	float	LoopVoltage;	//��·��ѹ
	float	LoopCurrent;	//��·����
	float	LoopPower;		//��·����
	double	LoopEnergy;		//��·����
}DCMETER_LOOP_ANALOG,*PDCMETER_LOOP_ANALOG;

typedef struct __attribute__((packed)) _DCMETER_AC_ANALOG{
	float	AcVoltage;	//������ѹ
}DCMETER_AC_ANALOG,*PDCMETER_AC_ANALOG;

typedef struct __attribute__((packed)) _DCMETER_ANALOG{
	DCMETER_AC_ANALOG	AcAnalog;			
	uint8_t		LoopCnt;
	DCMETER_LOOP_ANALOG LoopAnalog[DCMETER_LOOP_CNT_MAX];
}DCMETER_ANALOG,*PDCMETER_ANALOG;

/*************************   ģ��澯 ***************************************/
/*
�澯�ֽ�������
����00H ��������
����01H ���и澯��
����80H��EFH ���û��Զ��壻
����F0H ���������ϡ�
*/
typedef enum _DCMETER_ALARM_DEFINE{
	DCMETER_ALARM_NORMAL = 0,		//
	DCMETER_ALARM_ALARM = 1,
	DCMETER_ALARM_FAILURE = 0xf0,
}DCMETER_ALARM_DEFINE;

typedef struct __attribute__((packed)) _ModuleErrorAlarm{
    DCMETER_ALARM_DEFINE	LoopLostVoltageAlarm;	
    //����֧·����澯      �е������������ѹ���޵����ۻ�
    DCMETER_ALARM_DEFINE	LoopMeasureModuleAlarm;	
    //�ڲ��������澯
    DCMETER_ALARM_DEFINE	InterProgramErrorAlarm;	
    //ʱ�Ӵ���澯
    DCMETER_ALARM_DEFINE	ClockErrorAlarm;	    
    //�洢�����ϸ澯  ���ܣ����������ݣ��澯�޷�����      
    DCMETER_ALARM_DEFINE	MemFailureAlarm;
}DCMETER_ALARM_ModuleErrorAlarm;

typedef struct __attribute__((packed)) _DCMETER_LOOP_ALARM{
    //��·���
	uint8_t		LoopNum;				
    //��·ֱ����ѹ�澯
	DCMETER_ALARM_DEFINE	LoopDCVoltageLowAlarm;	
    //��·ֱ����ѹ�澯
    DCMETER_ALARM_DEFINE	LoopDCVoltageHighAlarm;	
    //��·һ���µ�澯  ֱ�������ѹС��46V�������з�·    ����С��50Vʱ��ĵ�����40%
	DCMETER_ALARM_DEFINE	LoopFirstPowerDown;		
    //��·ʧѹ�澯  �е������������ѹ
    DCMETER_ALARM_DEFINE    ModuleErrorAlarm;
    DCMETER_ALARM_ModuleErrorAlarm ModuleErrorAlarmTemp;
}DCMETER_LOOP_ALARM,*PDCMETER_LOOP_ALARM;

typedef struct __attribute__((packed)) _DCMETER_ALARM{
	uint8_t		AcVoltageAlarm;			//������ѹ
	uint8_t		LoopCnt;
	DCMETER_LOOP_ALARM LoopAlarm[DCMETER_LOOP_CNT_MAX];
    uint8_t      reserve;        //�洢DATAFLAG
}DCMETER_ALARM,*PDCMETER_ALARM;

/*************************   ģ����ʷ�澯 ***************************************/
typedef struct __attribute__((packed)) _DCMETER_LOOP_HISTORY_DATA{
	float	LoopEnergy;		//��·����
	float	LoopCurrent;	//��·����
}DCMETER_LOOP_HISTORY_DATA,*PDCMETER_LOOP_HISTORY_DATA;

typedef struct __attribute__((packed)) _DCMETER_HISTORY_DATA{
	uint32_t		AcVoltage;			//������ѹ
	uint8_t		LoopCnt;
	DCMETER_LOOP_HISTORY_DATA LoopHistoryData[DCMETER_LOOP_CNT_MAX];
}DCMETER_HISTORY_DATA,*PDCMETER_HISTORY_DATA;

/*
00H	�޸澯����������һ����ѯ��
01H ��·ֱ����ѹ���͸澯
02H	��·ֱ����ѹ���߸澯
03H ��·һ���µ�澯
04H	����ģ�����
05H	��������ͣ��澯
05H~FFH	�û��Զ���
*/
typedef enum _DCMETER_HISTORY_ALARM_TYPE{
	DCMETER_HISTORY_ALARM_TYPE_NONE = 0x0,		//
	DCMETER_HISTORY_ALARM_TYPE_DCVOLTAGE_LOW = 0x1,		//	
	DCMETER_HISTORY_ALARM_TYPE_DCVOLTAGE_HIGH = 0x2,		//
	DCMETER_HISTORY_ALARM_TYPE_FIRST_POWER_DOWN = 0x3,		//
	DCMETER_HISTORY_ALARM_TYPE_MEASURE_MODULE = 0x4,		//
	DCMETER_HISTORY_ALARM_TYPE_AC_POWER_CUT = 0x5,		//
}DCMETER_HISTORY_ALARM_TYPE;
/*
�澯Դ
�豸ID��
0=������·
1=��1��·
2=��2��·
3=��3��·
0xFF=ϵͳ����������
*/
typedef enum _DCMETER_HISTORY_ALARM_SOURCE{
	DCMETER_HISTORY_ALARM_SOURCE_AC = 0x0,		//
	DCMETER_HISTORY_ALARM_SOURCE_LOOP_1 = 0x1,		//	
	DCMETER_HISTORY_ALARM_SOURCE_LOOP_2 = 0x2,		//
	DCMETER_HISTORY_ALARM_SOURCE_LOOP_3 = 0x3,	
    DCMETER_HISTORY_ALARM_SOURCE_LOOP_4 = 0x4,
    DCMETER_HISTORY_ALARM_SOURCE_LOOP_5 = 0x5,	
    DCMETER_HISTORY_ALARM_SOURCE_LOOP_6 = 0x6,	//
	DCMETER_HISTORY_ALARM_SOURCE_SYSTEM = 0xff,		//
}DCMETER_HISTORY_ALARM_SOURCE;

typedef struct __attribute__((packed)) _DCMETER_HISTORY_ALARM{
	DCMETER_HISTORY_ALARM_TYPE		HistoryAlarmType;			//������ѹ
	DCMETER_HISTORY_ALARM_SOURCE	AlarmSource;
	DCMETER_ALARM_DEFINE 			AlarmWord;
}DCMETER_HISTORY_ALARM,*PDCMETER_HISTORY_ALARM;

/*************************   ������ ***************************************/
typedef enum _DCMETER_USER_CONFIG
{
    USER_NONE = 0x00,   //δ�����û�
    USER_ALL = 0x01,    //�ܼ���
    USER_CMCC = 0x02,   //�ƶ��û�
    USER_CUCC = 0x03,   //��ͨ�û�
    USER_CTCC = 0x04,   //�����û�
    USER_RESERVE1 = 0x05,//�Զ���1
    USER_RESERVE2 = 0x06,//�Զ���2
}DCMETER_USER_CONFIG;

#define DCMETER_ADDR	    1	//��ַ����Ĭ��1
#define DCMETER_VENDER	"JSYAAO"	//
#define DCMETER_BAUDRATE	    9600	//
#define DCMETER_SAVE_TIME_MINUTE	60	//

typedef struct __attribute__((packed)) _DCMETER_PARAM{					
	uint8_t		    Vender[8];		//
	uint32_t		    Addr;		//��ַ���� Ĭ��01H��01H-FFH�����ã�
	uint32_t		    Baudrate;	//
    uint32_t		    SaveTime;	//
    /*DCMETER_USER_CONFIG          Config_Loop1;//
    DCMETER_USER_CONFIG          Config_Loop2;//
    DCMETER_USER_CONFIG          Config_Loop3;//
    DCMETER_USER_CONFIG          Config_Loop4;//
    DCMETER_USER_CONFIG          Config_Loop5;//
    DCMETER_USER_CONFIG          Config_Loop6;//
    */
    DCMETER_USER_CONFIG          LoopConfig[DCMETER_LOOP_CNT_MAX];
}DCMETER_PARAM,*PDCMETER_PARAM;

/*************************   ģ����� ***************************************/

//#define YDT1363_3_PROTOCAL_VER	1	//��ַ����Ĭ��1
#define DCMETER_MODULE_PARAM_DC_HIGH_DEFAULT	58	//ֱ����ѹ�澯��ֵĬ��58V
#define DCMETER_MODULE_PARAM_DC_LOW_DEFAULT	    47	//ֱ����ѹ�澯��ֵĬ��47V
#define DCMETER_MODULE_PARAM_DC_FIRST_POWERDOWN_DEFAULT	    46	//ֱ��һ���µ�澯��ֵĬ��46V
#define DCMETER_MODULE_PARAM_MODULE_ALARM_DEFAULT	    0	//ֱ��һ���µ�澯��ֵĬ��46V
#define DCMETER_MODULE_PARAM_AC_VOLTAGE_RATIO_DEFAULT	    0	//ֱ��һ���µ�澯��ֵĬ��46V

#define DCMETER_MODULE_PARAM_AC_POWERDOWN_DEFAULT	85	//��������ͣ��澯��ֵĬ��85V

typedef struct __attribute__((packed)) _DCMETER_MODULE_PARAM{
	//float		Addr;						//��ַ���� Ĭ��01H��01H-FFH�����ã�
	float		DcHighVoltageThreshold;		//ֱ����ѹ���߸澯��ֵ Ĭ��58V������ֵ5800
	float		DcLowVoltageThreshold;		//ֱ����ѹ���͸澯��ֵ Ĭ��47V������ֵ4700
	float		DcFirstPowerDownThreshold;	//ֱ��һ���µ�澯��ֵ Ĭ��46V������ֵ4600
    /*float		LoopLostVoltageThreshold;	//ģ��ʧѹ�澯��ֵ���е������������ѹ��
    
    float	    LoopMeasureModuleThreshold;	//����֧·����澯
    float	    InterProgramErrorAlarm;	    //�ڲ��������澯
    float	    ClockErrorThreshold;	    //ʱ�Ӵ���澯
    float	    MemFailureThreshold;	    //�洢�����ϸ澯
   */
    float		ModuleAlarmThreshold;
	float		AcPowerCutThreshold;		//��������ͣ��澯��ֵ Ĭ��85V������ֵ8500
	float 		AcVoltageRatio;				//������ѹ������� Ĭ��0
}DCMETER_MODULE_PARAM,*PDCMETER_MODULE_PARAM;

/*****************************************************************************/
typedef struct __attribute__((packed)) _DCMETER_DATE{
	uint16_t	Year;						
	uint8_t 	Month;
	uint8_t 	Day;
	uint8_t 	Hour;
	uint8_t 	Minute;
	uint8_t 	Second;
}DCMETER_DATE,*PDCMETER_DATE;

/***************************** ������ Ĭ��ÿ��1��0ʱ*****************************************/
#define DCMETER_READING_DAY_DEFAULT	1	
#define DCMETER_READING_HOUR_DEFAULT	0	
typedef struct __attribute__((packed)) _DCMETER_READING{
	uint8_t 	Day;
	uint8_t 	Hour;
}DCMETER_READING,*PDCMETER_READING;

/*****************************     У׼Դ��ѹ�͵���������   *************************************/
typedef struct __attribute__((packed)) _DCMETER_CS5460_DC_MAX{
    float                   Imax;
    float                   Vmax;
}DCMETER_CS5460_DC_MAX,*PDCMETER_CS5460_DC_MAX;

/*****************************     6��ͨ����CTֵ  *************************************/
typedef struct __attribute__((packed)) _DCMETER_CT{
    float   CT[DCMETER_LOOP_CNT_MAX];
}DCMETER_CT,*PDCMETER_CT;

/*************************************************************************************/
typedef struct __attribute__((packed)) _DCMETER_SAVE_COUNT
{
    uint32_t EnergyMonthCnt;
	uint32_t MinuteDataCnt; 
    uint32_t HourDataCnt;
    uint32_t AlarmCnt; 
    uint32_t MonthDataCnt;    
}DCMETER_SAVE_COUNT,*PDCMETER_SAVE_COUNT;


/*DCMETER_TEMP_ENERGY.LoopEnergy��AnalogData��LoopEnergy���е�����
����ÿ·���ñ��޸ģ����޸�ǰ�����ý��޸Ļ�·�ĵ����ۼӵ�ͬһ�û��µĻ�·������
����0�û�·���ܣ��˺�û�·�����ϵ�ֵ����㵽�޸ĺ������û���*/
typedef struct __attribute__((packed)) _DCMETER_TEMP_ENERGY{
    float                   AllEnergy;
    float                   CMCCEnergy;
    float                   CUCCEnergy;
    float                   CTCCEnergy;
    float                   LoopEnergy[DCMETER_LOOP_CNT_MAX];
}DCMETER_TEMP_ENERGY,*PDCMETER_TEMP_ENERGY;

/***************************** �澯�жϲ��� ******************************/
typedef struct __attribute__((packed)) _DCMETER_ALARM_PARAM{
	float   Voltage50VCurrent[DCMETER_LOOP_CNT_MAX]; //�����ж�ģ��һ���µ�
}DCMETER_ALARM_PARAM,*PDCMETER_ALARM_PARAMG;

/***************************** �ֶ�У׼******************************/
//���ֶ���
#define SECTIONS_MAX    10

//����У׼Դ����
#define SECTIONS_CALIBRATION_RANGE_RATIO    100
//����ó�������ֵ����
#define SECTIONS_CALIBRATION_RATIO    1000
typedef struct __attribute__((packed)) _SECTION_CALIBRATION_DIVISION
{
    //�洢�ֶ�У׼ֵ��*SECTIONS_CALIBRATION_RATIO��
	uint16_t  CaliValue[DCMETER_LOOP_CNT_MAX];

}SECTION_CALIBRATION_DIVISION, *PSECTION_CALIBRATION_DIVISION;

typedef struct __attribute__((packed)) _SECTION_CALIBRATION_RANGE
{
    //�洢�ֶ�У׼ֵ��*SECTIONS_CALIBRATION_RATIO��
	uint32_t  LoopRange[DCMETER_LOOP_CNT_MAX];

}SECTION_CALIBRATION_RANGE, *PSECTION_CALIBRATION_RANGE;

typedef struct __attribute__((packed)) _SECTION_CALIBRATION
{
    //�洢�ֶ�У׼ֵ��*SECTIONS_CALIBRATION_RATIO��
	SECTION_CALIBRATION_DIVISION  SectCaliDiv[SECTIONS_MAX];
    //�洢�ֶη�Χֵ��*SECTIONS_CALIBRATION_RATIO��
    SECTION_CALIBRATION_RANGE  SectRange[SECTIONS_MAX];
    //�洢�ֶ�У׼�����
    signed short  SectCaliZero[DCMETER_LOOP_CNT_MAX];
    uint16_t  CaliCnt[DCMETER_LOOP_CNT_MAX];
}SECTION_CALIBRATION, *PSECTION_CALIBRATION;

typedef struct __attribute__((packed)) _DCMETER_ENERGY_RATIO{
	float   Value[DCMETER_LOOP_CNT_MAX]; 
}DCMETER_ENERGY_RATIO,*PDCMETER_ENERGY_RATIO;
//
typedef struct __attribute__((packed)) _DCMETER_ENERGY_READING{
	float                   AllEnergy;
    float                   CMCCEnergy;
    float                   CUCCEnergy;
    float                   CTCCEnergy;
}DCMETER_ENERGY_READING,*PDCMETER_ENERGY_READING;
/*typedef struct __attribute__((packed)) _TEST_CNT
{
    uint8_t  Flag[50];
    uint8_t  Cnt;
}TEST_CNT, *PTEST_CNT;
*/
typedef struct __attribute__((packed)) _DCMETER_ADC_RATIO{
	float   k;
    float   b;
    float   x1;   
    float   y1;
    float   x2;
    float   y2;
}DCMETER_ADC_RATIO,*PDCMETER_ADC_RATIO;
/************************************************************************/
typedef struct _DCMETER_DATA
{
	DCMETER_ANALOG			AnalogData;
	DCMETER_ALARM			AlarmData;
	DCMETER_HISTORY_DATA	HistoryData;
	DCMETER_HISTORY_ALARM	HistoryAlarm;
    DCMETER_PARAM           Param;
	DCMETER_MODULE_PARAM	ModuleParam;
	DCMETER_DATE			Date;
    DCMETER_READING         Reading;
    DCMETER_CS5460_DC_MAX   Cs5460Dc; 
    DCMETER_TEMP_ENERGY		TempEnergy;//
    DCMETER_SAVE_COUNT      SaveCnt;
    DCMETER_ALARM_PARAM     AlarmParam;
    DCMETER_CT              CTValue;
    SECTION_CALIBRATION     SectCali;
    DCMETER_ENERGY_RATIO    EgyRatio;
    DCMETER_ENERGY_READING  EgyReading;
    RN8209C_DEF             Rn8209c[3]; 
    DCMETER_ADC_RATIO       AdcRatio;
}DCMETER_DATA,*PDCMETER_DATA;//__attribute__((packed)) 


/******************************************************************************
                                  �ⲿ���ܺ���                      
******************************************************************************/
extern uint8_t GetCID2(void);
extern uint8_t GetRTN(void);
extern void SetDataFlag(uint8_t IsSwitchChange,uint8_t IsAlarmChange);
/*
 ************************************************************************************************************************************************************************     
 *��������:	ProtocolProc_YDT1363_3_ProtocolPocess
 *��������:	Э�鴦������
 *��������:	Э��֡pRecvBuf��pRecvLen
 *�������:	pSendBuf��pSendLen
 *��������:	ʧ��YDT1363_FUNC_RET_FALSE/�ɹ�YDT1363_FUNC_RET_SUCCESS
 *��������:	
 *ȫ������: 
 *����˵��: 
 ************************************************************************************************************************************************************************       
 */
extern int32_t ProtocolProc_YDT1363_3_ProtocolPocess(const uint8_t *pRecvBuf, uint32_t *pRecvLen, uint8_t *pSendBuf, uint32_t *pSendLen);

/*
 ************************************************************************************************************************************************************************     
 *��������:	ProtocolProc_YDT1363_3_CallBack
 *��������:	��ȡЭ�鴦�������������ݵĵ�ַָ��
 *��������:	��
 *�������:	
 *��������:	���ݵĵ�ַָ��
 *��������:	
 *ȫ������: 
 *����˵��: 
 ************************************************************************************************************************************************************************       
 */
extern PDCMETER_DATA ProtocolProc_YDT1363_3_CallBack(void);

/**
  ****************************** Support C++ **********************************
**/
#ifdef __cplusplus
	}
#endif
/**
  *****************************************************************************
**/


#endif  /* end Ydt1363.h */