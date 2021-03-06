/**
  *****************************************************************************
  *                            电总YDT1363_3协议
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
  				暂未支持获取历史数据和告警	
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
                             外部函数头文件
                  应用到不同的外设头文件请在这里修改即可                        
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

#define UNIT_TYPE	"DCM4806"	//设备型号
#define SOFTWARE_VERSION	"0100"	//厂家软件版本
#define VENDER		"JSYAAO"		//厂家名称
#define USER_DEFINE_SOFT_VERSION    "20171103"

#define YDT1363_3_DATABUF_MAX	170  	//电总协议转换数据部分最大长度
#define DCMETER_LOOP_CNT_MAX	6		//直流电表最大回路数	
#define DCMETER_DATA_RATIO		100		//直流电表上传数据与实际数据的比率

#define YDT1363_3_PROTOCAL_VER						0x10
#define YDT1363_3_PROTOCAL_ADDR						0x01
#define YDT1363_3_PROTOCAL_CID1_DCMETER				0x2d








#define YDT1363_3_PROTOCAL_CID2_GET_DATE			0x4d    //获取时间信息（4DH）
#define YDT1363_3_PROTOCAL_CID2_SET_DATE			0x4e    //设置日期时间（4EH）
#define YDT1363_3_PROTOCAL_CID2_GET_VERSION			0x4f    //获取讯协议版本号（4FH）
#define YDT1363_3_PROTOCAL_CID2_GET_DEVICE_ADDR		0x50    //获取设备地址（50H）
#define YDT1363_3_PROTOCAL_CID2_GET_VENDER			0x51    //获取厂家信息（51H）
#define YDT1363_3_PROTOCAL_CID2_GET_ANALOG			0x85    //获取模拟量量化数据
#define YDT1363_3_PROTOCAL_CID2_GET_HISTORY_DATA	0x86    //获取模块历史数据(86H)

#define YDT1363_3_PROTOCAL_CID2_GET_ENERGY_MONTH	0x87    //获取月冻结电能(87H)
#define YDT1363_3_PROTOCAL_CID2_GET_PARAM           0x88    //获取表参数
#define YDT1363_3_PROTOCAL_CID2_SET_PARAM	        0x89    //设置表参数

#define YDT1363_3_PROTOCAL_CID2_GET_METER_READING	0x90    //获取抄表日
#define YDT1363_3_PROTOCAL_CID2_SET_METER_READING	0x91    //设置抄表日

#define YDT1363_3_PROTOCAL_CID2_GET_ALARM			0x92    //获取告警状态
#define YDT1363_3_PROTOCAL_CID2_GET_HISTORY_ALARM	0x95    //获取历史告警
#define YDT1363_3_PROTOCAL_CID2_GET_MODULE_PARAM	0x93    //获取模块参数
#define YDT1363_3_PROTOCAL_CID2_SET_MODULE_PARAM	0x94    //设置模块参数


/************************** 生产用  **************************/
//恢复出厂默认设置
#define YDT1363_3_PROTOCAL_CID2_FACTORY_RESET	0xe0  
//偏置校准
#define YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_OFF	0xe1   
//增益校准
#define YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_GN	0xe2 
//功率校准
#define YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_P	0xe3 
//获取校准源电压和电流满量程
#define YDT1363_3_PROTOCAL_CID2_GET_CS5460A_DC_MAX	0xe4
//设置校准源电压和电流满量程
#define YDT1363_3_PROTOCAL_CID2_SET_CS5460A_DC_MAX	0xe5  
//获取6个通道的CT值
#define YDT1363_3_PROTOCAL_CID2_GET_CT	0xe6
//设置6个通道的CT值
#define YDT1363_3_PROTOCAL_CID2_SET_CT	0xe7
//电能清零
#define YDT1363_3_PROTOCAL_CID2_POWER_RESET	0xe8
//分段校准
#define YDT1363_3_PROTOCAL_CID2_SECTION_CALIBRATION     0xe9
//获取CS5460校准值
#define YDT1363_3_PROTOCAL_CID2_GET_CS5460CALI     0xea
//获取指定页的上半页数据
#define YDT1363_3_PROTOCAL_CID2_GET_PAGE_DATA     0xeb
//获取电能修正系数
#define YDT1363_3_PROTOCAL_CID2_GET_ENERGY_RATIO     0xec
//设置电能修正系数
#define YDT1363_3_PROTOCAL_CID2_SET_ENERGY_RATIO     0xed
//获取模拟量（扩展，电能*10000）
#define YDT1363_3_PROTOCAL_CID2_GET_ANALOG_EXT     0xee
//获取软件版本号
#define YDT1363_3_PROTOCAL_CID2_GET_SOFT_VER     0xef
//系统复位
#define YDT1363_3_PROTOCAL_CID2_SET_TO_BOOT 	0xf0  

#define YDT1363_3_PROTOCAL_RTN_NORMAL				0x0   	//正常
#define YDT1363_3_PROTOCAL_RTN_VER_ERROR			0x01   	//协议版本错
#define YDT1363_3_PROTOCAL_RTN_CHKSUM_ERROR			0x02   	//CHKSUM错
#define YDT1363_3_PROTOCAL_RTN_LCHKSUM_ERROR		0x03   	//LCHKSUM错
#define YDT1363_3_PROTOCAL_RTN_CID2_INVALID			0x04   	//CID2无效
#define YDT1363_3_PROTOCAL_RTN_FORMAT_ERROR			0x05   	//命令格式错
#define YDT1363_3_PROTOCAL_RTN_DATA_INVALID			0x06  	//无效数据
#define YDT1363_3_PROTOCAL_RTN_DATA_NONE			0x07  	//无历史数据
#define YDT1363_3_PROTOCAL_RTN_CID1_INVALID			0xe1  	//CID1无效
#define YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL		0xe2  	//命令执行失败
/* 电总协议数据结构定义 
*变量均为原始数据值(未ASCII转码), Length为数据内容实际长度
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


/*************************   模拟量 ***************************************/

typedef struct __attribute__((packed)) _DCMETER_LOOP_ANALOG{
	//uint8_t		LoopNum;		//回路编号
	float	LoopVoltage;	//回路电压
	float	LoopCurrent;	//回路电流
	float	LoopPower;		//回路功率
	double	LoopEnergy;		//回路电能
}DCMETER_LOOP_ANALOG,*PDCMETER_LOOP_ANALOG;

typedef struct __attribute__((packed)) _DCMETER_AC_ANALOG{
	float	AcVoltage;	//交流电压
}DCMETER_AC_ANALOG,*PDCMETER_AC_ANALOG;

typedef struct __attribute__((packed)) _DCMETER_ANALOG{
	DCMETER_AC_ANALOG	AcAnalog;			
	uint8_t		LoopCnt;
	DCMETER_LOOP_ANALOG LoopAnalog[DCMETER_LOOP_CNT_MAX];
}DCMETER_ANALOG,*PDCMETER_ANALOG;

/*************************   模块告警 ***************************************/
/*
告警字节描述：
——00H ：正常；
——01H ：有告警；
——80H～EFH ：用户自定义；
——F0H ：其他故障。
*/
typedef enum _DCMETER_ALARM_DEFINE{
	DCMETER_ALARM_NORMAL = 0,		//
	DCMETER_ALARM_ALARM = 1,
	DCMETER_ALARM_FAILURE = 0xf0,
}DCMETER_ALARM_DEFINE;

typedef struct __attribute__((packed)) _ModuleErrorAlarm{
    DCMETER_ALARM_DEFINE	LoopLostVoltageAlarm;	
    //计量支路错误告警      有电流，有输入电压，无电能累积
    DCMETER_ALARM_DEFINE	LoopMeasureModuleAlarm;	
    //内部程序错误告警
    DCMETER_ALARM_DEFINE	InterProgramErrorAlarm;	
    //时钟错误告警
    DCMETER_ALARM_DEFINE	ClockErrorAlarm;	    
    //存储器故障告警  电能，参数，数据，告警无法保存      
    DCMETER_ALARM_DEFINE	MemFailureAlarm;
}DCMETER_ALARM_ModuleErrorAlarm;

typedef struct __attribute__((packed)) _DCMETER_LOOP_ALARM{
    //回路编号
	uint8_t		LoopNum;				
    //回路直流低压告警
	DCMETER_ALARM_DEFINE	LoopDCVoltageLowAlarm;	
    //回路直流高压告警
    DCMETER_ALARM_DEFINE	LoopDCVoltageHighAlarm;	
    //回路一次下电告警  直流输出电压小于46V，且所有分路    电流小于50V时测的电流的40%
	DCMETER_ALARM_DEFINE	LoopFirstPowerDown;		
    //回路失压告警  有电流，无输入电压
    DCMETER_ALARM_DEFINE    ModuleErrorAlarm;
    DCMETER_ALARM_ModuleErrorAlarm ModuleErrorAlarmTemp;
}DCMETER_LOOP_ALARM,*PDCMETER_LOOP_ALARM;

typedef struct __attribute__((packed)) _DCMETER_ALARM{
	uint8_t		AcVoltageAlarm;			//交流电压
	uint8_t		LoopCnt;
	DCMETER_LOOP_ALARM LoopAlarm[DCMETER_LOOP_CNT_MAX];
    uint8_t      reserve;        //存储DATAFLAG
}DCMETER_ALARM,*PDCMETER_ALARM;

/*************************   模块历史告警 ***************************************/
typedef struct __attribute__((packed)) _DCMETER_LOOP_HISTORY_DATA{
	float	LoopEnergy;		//回路电能
	float	LoopCurrent;	//回路电流
}DCMETER_LOOP_HISTORY_DATA,*PDCMETER_LOOP_HISTORY_DATA;

typedef struct __attribute__((packed)) _DCMETER_HISTORY_DATA{
	uint32_t		AcVoltage;			//交流电压
	uint8_t		LoopCnt;
	DCMETER_LOOP_HISTORY_DATA LoopHistoryData[DCMETER_LOOP_CNT_MAX];
}DCMETER_HISTORY_DATA,*PDCMETER_HISTORY_DATA;

/*
00H	无告警，或完成最后一条查询。
01H 回路直流电压过低告警
02H	回路直流电压过高告警
03H 回路一次下电告警
04H	测量模块故障
05H	交流输入停电告警
05H~FFH	用户自定义
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
告警源
设备ID：
0=交流回路
1=第1回路
2=第2回路
3=第3回路
0xFF=系统，其他保留
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
	DCMETER_HISTORY_ALARM_TYPE		HistoryAlarmType;			//交流电压
	DCMETER_HISTORY_ALARM_SOURCE	AlarmSource;
	DCMETER_ALARM_DEFINE 			AlarmWord;
}DCMETER_HISTORY_ALARM,*PDCMETER_HISTORY_ALARM;

/*************************   表参数 ***************************************/
typedef enum _DCMETER_USER_CONFIG
{
    USER_NONE = 0x00,   //未配置用户
    USER_ALL = 0x01,    //总计量
    USER_CMCC = 0x02,   //移动用户
    USER_CUCC = 0x03,   //联通用户
    USER_CTCC = 0x04,   //电信用户
    USER_RESERVE1 = 0x05,//自定义1
    USER_RESERVE2 = 0x06,//自定义2
}DCMETER_USER_CONFIG;

#define DCMETER_ADDR	    1	//地址编码默认1
#define DCMETER_VENDER	"JSYAAO"	//
#define DCMETER_BAUDRATE	    9600	//
#define DCMETER_SAVE_TIME_MINUTE	60	//

typedef struct __attribute__((packed)) _DCMETER_PARAM{					
	uint8_t		    Vender[8];		//
	uint32_t		    Addr;		//地址编码 默认01H（01H-FFH可设置）
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

/*************************   模块参数 ***************************************/

//#define YDT1363_3_PROTOCAL_VER	1	//地址编码默认1
#define DCMETER_MODULE_PARAM_DC_HIGH_DEFAULT	58	//直流高压告警阈值默认58V
#define DCMETER_MODULE_PARAM_DC_LOW_DEFAULT	    47	//直流低压告警阈值默认47V
#define DCMETER_MODULE_PARAM_DC_FIRST_POWERDOWN_DEFAULT	    46	//直流一次下电告警阈值默认46V
#define DCMETER_MODULE_PARAM_MODULE_ALARM_DEFAULT	    0	//直流一次下电告警阈值默认46V
#define DCMETER_MODULE_PARAM_AC_VOLTAGE_RATIO_DEFAULT	    0	//直流一次下电告警阈值默认46V

#define DCMETER_MODULE_PARAM_AC_POWERDOWN_DEFAULT	85	//交流输入停电告警阈值默认85V

typedef struct __attribute__((packed)) _DCMETER_MODULE_PARAM{
	//float		Addr;						//地址编码 默认01H（01H-FFH可设置）
	float		DcHighVoltageThreshold;		//直流电压过高告警阀值 默认58V，传输值5800
	float		DcLowVoltageThreshold;		//直流电压过低告警阀值 默认47V，传输值4700
	float		DcFirstPowerDownThreshold;	//直流一次下电告警阈值 默认46V，传输值4600
    /*float		LoopLostVoltageThreshold;	//模块失压告警阈值（有电流，无输入电压）
    
    float	    LoopMeasureModuleThreshold;	//计量支路错误告警
    float	    InterProgramErrorAlarm;	    //内部程序错误告警
    float	    ClockErrorThreshold;	    //时钟错误告警
    float	    MemFailureThreshold;	    //存储器故障告警
   */
    float		ModuleAlarmThreshold;
	float		AcPowerCutThreshold;		//交流输入停电告警阀值 默认85V，传输值8500
	float 		AcVoltageRatio;				//交流电压计算参数 默认0
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

/***************************** 抄表日 默认每月1日0时*****************************************/
#define DCMETER_READING_DAY_DEFAULT	1	
#define DCMETER_READING_HOUR_DEFAULT	0	
typedef struct __attribute__((packed)) _DCMETER_READING{
	uint8_t 	Day;
	uint8_t 	Hour;
}DCMETER_READING,*PDCMETER_READING;

/*****************************     校准源电压和电流满量程   *************************************/
typedef struct __attribute__((packed)) _DCMETER_CS5460_DC_MAX{
    float                   Imax;
    float                   Vmax;
}DCMETER_CS5460_DC_MAX,*PDCMETER_CS5460_DC_MAX;

/*****************************     6个通道的CT值  *************************************/
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


/*DCMETER_TEMP_ENERGY.LoopEnergy和AnalogData中LoopEnergy的有点区别，
假如每路配置被修改，则按修改前的配置将修改回路的电能累加到同一用户下的回路电能中
并清0该回路电能，此后该回路电能上的值会计算到修改后配置用户上*/
typedef struct __attribute__((packed)) _DCMETER_TEMP_ENERGY{
    float                   AllEnergy;
    float                   CMCCEnergy;
    float                   CUCCEnergy;
    float                   CTCCEnergy;
    float                   LoopEnergy[DCMETER_LOOP_CNT_MAX];
}DCMETER_TEMP_ENERGY,*PDCMETER_TEMP_ENERGY;

/***************************** 告警判断参数 ******************************/
typedef struct __attribute__((packed)) _DCMETER_ALARM_PARAM{
	float   Voltage50VCurrent[DCMETER_LOOP_CNT_MAX]; //用以判断模块一次下电
}DCMETER_ALARM_PARAM,*PDCMETER_ALARM_PARAMG;

/***************************** 分段校准******************************/
//最大分段数
#define SECTIONS_MAX    10

//输入校准源倍率
#define SECTIONS_CALIBRATION_RANGE_RATIO    100
//计算得出的修正值倍数
#define SECTIONS_CALIBRATION_RATIO    1000
typedef struct __attribute__((packed)) _SECTION_CALIBRATION_DIVISION
{
    //存储分段校准值（*SECTIONS_CALIBRATION_RATIO）
	uint16_t  CaliValue[DCMETER_LOOP_CNT_MAX];

}SECTION_CALIBRATION_DIVISION, *PSECTION_CALIBRATION_DIVISION;

typedef struct __attribute__((packed)) _SECTION_CALIBRATION_RANGE
{
    //存储分段校准值（*SECTIONS_CALIBRATION_RATIO）
	uint32_t  LoopRange[DCMETER_LOOP_CNT_MAX];

}SECTION_CALIBRATION_RANGE, *PSECTION_CALIBRATION_RANGE;

typedef struct __attribute__((packed)) _SECTION_CALIBRATION
{
    //存储分段校准值（*SECTIONS_CALIBRATION_RATIO）
	SECTION_CALIBRATION_DIVISION  SectCaliDiv[SECTIONS_MAX];
    //存储分段范围值（*SECTIONS_CALIBRATION_RATIO）
    SECTION_CALIBRATION_RANGE  SectRange[SECTIONS_MAX];
    //存储分段校准的零点
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
                                  外部功能函数                      
******************************************************************************/
extern uint8_t GetCID2(void);
extern uint8_t GetRTN(void);
extern void SetDataFlag(uint8_t IsSwitchChange,uint8_t IsAlarmChange);
/*
 ************************************************************************************************************************************************************************     
 *函数名称:	ProtocolProc_YDT1363_3_ProtocolPocess
 *功能描述:	协议处理进程
 *输入描述:	协议帧pRecvBuf、pRecvLen
 *输出描述:	pSendBuf、pSendLen
 *返回描述:	失败YDT1363_FUNC_RET_FALSE/成功YDT1363_FUNC_RET_SUCCESS
 *作者日期:	
 *全局声明: 
 *特殊说明: 
 ************************************************************************************************************************************************************************       
 */
extern int32_t ProtocolProc_YDT1363_3_ProtocolPocess(const uint8_t *pRecvBuf, uint32_t *pRecvLen, uint8_t *pSendBuf, uint32_t *pSendLen);

/*
 ************************************************************************************************************************************************************************     
 *函数名称:	ProtocolProc_YDT1363_3_CallBack
 *功能描述:	获取协议处理部分所需数据的地址指针
 *输入描述:	无
 *输出描述:	
 *返回描述:	数据的地址指针
 *作者日期:	
 *全局声明: 
 *特殊说明: 
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
