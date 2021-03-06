#ifndef __USER_COMMON_H_
#define __USER_COMMON_H_

#include "NUC029xAN.h"
#include "Ydt1363.h"
#include "RN8209C.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
 

#define GetDWord(addr) (*(__packed uint32_t *)(addr))
#define GetWord(addr) (*(__packed uint16_t *)(addr))
#define DC_I_MAX 2.5
#define DC_V_MAX 60.0
/*********************************
关于FLASH数据分区(
PageSize=0x200,
BlockSize=0x1000,
SectorSize=0x20000,
Capacity=0x200000,
TotalPages=0x1080,)

将2M空间分成三部分：
    1)数据区:0x0-0xfffff,共0x100000Bytes
    2)备份区:0x100000-0x1fffff,共0x100000Bytes
    
   
数据区分块：
    0x0     -   0x1bff      月冻结电能(56B*120=6780 按页对齐为:7168)
    0x1c00  -   0x57ff      历史月数据(128B*120=15360 按页对齐为:15360,+7168=22528)
    0x5800  -   0xd9ff      告警数据(11B*3000=33000,按页对齐为:33280,+22528=55808))
    0xda00  -   0x6b5ff     历史分钟数据(128B*3000=384000,按页对齐为:384000,+55808=439808)
    0x6b600 -   0xc91ff     历史小时数据(128B*3000=384000,按页对齐为:384000,+439808=823808)
    0xc9200 -   0xcxxxx     系统参数(1024B,按页对齐为:1024,+823808=824832)
    0xd0000 -   0xd77ff    动态数据区0x7800Bytes,60Pages           

***********************************/

#define AT24CXX_ADDR_BACKUP_OFFSET 0x100000  	//?¨ò?×?′ó?óê?×??úêy 200

//#define PROTOCAL_BUF_MAX 400  	//?¨ò?×?′ó?óê?×??úêy 200
#define PROTOCAL_BUF_MAX 300  	//?¨ò?×?′ó?óê?×??úêy 200
//迁移EEPROM数据到FLASH中
//前一页放常改变的参数
//后一页放不常改变的参数(初始化标志，独占一页)
#define EEPROM_FLASH_START_ADDR_PAGE1  0xc9200
#define EEPROM_FLASH_START_ADDR_PAGE2  0xc9400

//将告警计数单独放在一个页里，因为告警的产生频率不固定
#define EEPROM_FLASH_START_ADDR_PAGE3  0xc9600
//将分段校准数据单独放一个页
#define EEPROM_FLASH_START_ADDR_PAGE4  0xc9800
//将电能修正系数单独放一页
#define EEPROM_FLASH_START_ADDR_PAGE5  0xc9a00
//将ADC修正系数单独放一页
#define EEPROM_FLASH_START_ADDR_PAGE6  0xc9c00
//表参数
#define AT24CXX_ADDR_PARAM     (EEPROM_FLASH_START_ADDR_PAGE1) //占用26个字节
#define AT24CXX_ADDR_PARAM_SIZE     sizeof(DCMETER_PARAM) 
#define AT24CXX_ADDR_PARAM_CHECKSUM     (EEPROM_FLASH_START_ADDR_PAGE1+AT24CXX_ADDR_PARAM_SIZE)

//CS5460芯片的电流通道校准值
#define AT24CXX_ADDR_PARAM_CS5460_DC_I     (AT24CXX_ADDR_PARAM_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_PARAM_CS5460_DC_I_SIZE     4 
#define AT24CXX_ADDR_PARAM_CS5460_DC_I_CHECKSUM     (AT24CXX_ADDR_PARAM_CS5460_DC_I+AT24CXX_ADDR_PARAM_CS5460_DC_I_SIZE)
//CS5460芯片的电压通道校准值
#define AT24CXX_ADDR_PARAM_CS5460_DC_V     (AT24CXX_ADDR_PARAM_CS5460_DC_I_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_PARAM_CS5460_DC_V_SIZE     4 
#define AT24CXX_ADDR_PARAM_CS5460_DC_V_CHECKSUM     (AT24CXX_ADDR_PARAM_CS5460_DC_V+AT24CXX_ADDR_PARAM_CS5460_DC_V_SIZE)
//模块参数
#define AT24CXX_ADDR_MODULE_PARAM     (AT24CXX_ADDR_PARAM_CS5460_DC_V_CHECKSUM+1) //占用24个字节
#define AT24CXX_ADDR_MODULE_PARAM_SIZE     sizeof(DCMETER_MODULE_PARAM) 
#define AT24CXX_ADDR_MODULE_PARAM_CHECKSUM     (AT24CXX_ADDR_MODULE_PARAM+AT24CXX_ADDR_MODULE_PARAM_SIZE)
//抄表日
#define AT24CXX_ADDR_READING     (AT24CXX_ADDR_MODULE_PARAM_CHECKSUM+1) //占用2个字节
#define AT24CXX_ADDR_READING_SIZE     2 
#define AT24CXX_ADDR_READING_CHECKSUM     (AT24CXX_ADDR_READING+AT24CXX_ADDR_READING_SIZE)

//回路1-6对应的CT值
#define AT24CXX_ADDR_CT1     (AT24CXX_ADDR_READING_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_CT1_SIZE     4 
#define AT24CXX_ADDR_CT1_CHECKSUM     (AT24CXX_ADDR_CT1+AT24CXX_ADDR_CT1_SIZE)
#define AT24CXX_ADDR_CT2     (AT24CXX_ADDR_CT1_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_CT2_SIZE     4 
#define AT24CXX_ADDR_CT2_CHECKSUM     (AT24CXX_ADDR_CT2+AT24CXX_ADDR_CT2_SIZE)
#define AT24CXX_ADDR_CT3     (AT24CXX_ADDR_CT2_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_CT3_SIZE     4 
#define AT24CXX_ADDR_CT3_CHECKSUM     (AT24CXX_ADDR_CT3+AT24CXX_ADDR_CT3_SIZE)
#define AT24CXX_ADDR_CT4     (AT24CXX_ADDR_CT3_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_CT4_SIZE     4 
#define AT24CXX_ADDR_CT4_CHECKSUM     (AT24CXX_ADDR_CT4+AT24CXX_ADDR_CT4_SIZE)
#define AT24CXX_ADDR_CT5     (AT24CXX_ADDR_CT4_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_CT5_SIZE     4 
#define AT24CXX_ADDR_CT5_CHECKSUM     (AT24CXX_ADDR_CT5+AT24CXX_ADDR_CT5_SIZE)
#define AT24CXX_ADDR_CT6     (AT24CXX_ADDR_CT5_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_CT6_SIZE     4 
#define AT24CXX_ADDR_CT6_CHECKSUM     (AT24CXX_ADDR_CT6+AT24CXX_ADDR_CT6_SIZE)

//FLASH中保存各项数据计数
#define AT24CXX_ADDR_MONTH_DATA_CNT     (AT24CXX_ADDR_CT6_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_MONTH_DATA_CNT_SIZE     4 
#define AT24CXX_ADDR_MONTH_DATA_CNT_CHECKSUM     (AT24CXX_ADDR_MONTH_DATA_CNT+AT24CXX_ADDR_MONTH_DATA_CNT_SIZE)

#define AT24CXX_ADDR_ENERGY_CNT     (AT24CXX_ADDR_MONTH_DATA_CNT_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_ENERGY_CNT_SIZE     4 
#define AT24CXX_ADDR_ENERGY_CNT_CHECKSUM     (AT24CXX_ADDR_ENERGY_CNT+AT24CXX_ADDR_ENERGY_CNT_SIZE)

#define AT24CXX_ADDR_HOUR_DATA_CNT     (AT24CXX_ADDR_ENERGY_CNT_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_HOUR_DATA_CNT_SIZE     4 
#define AT24CXX_ADDR_HOUR_DATA_CNT_CHECKSUM     (AT24CXX_ADDR_HOUR_DATA_CNT+AT24CXX_ADDR_HOUR_DATA_CNT_SIZE)

//PAGE1目前占用110字节

//首次初始化数据标志
#define AT24CXX_ADDR_FIRST_INIT_DW        0x5aa5a55a
#define AT24CXX_ADDR_FIRST_INIT     (EEPROM_FLASH_START_ADDR_PAGE2) //占4个字节
#define AT24CXX_ADDR_FIRST_INIT_SIZE     4 
#define AT24CXX_ADDR_FIRST_INIT_CHECKSUM     (AT24CXX_ADDR_FIRST_INIT+AT24CXX_ADDR_FIRST_INIT_SIZE)
//PAGE2目前占用4字节

#define AT24CXX_ADDR_ALARM_CNT     (EEPROM_FLASH_START_ADDR_PAGE3) //占用4个字节
#define AT24CXX_ADDR_ALARM_CNT_SIZE     4 
#define AT24CXX_ADDR_ALARM_CNT_CHECKSUM     (AT24CXX_ADDR_ALARM_CNT+AT24CXX_ADDR_ALARM_CNT_SIZE)
//PAGE3目前占用4字节

//分段校准值
#define AT24CXX_ADDR_SECTION     (EEPROM_FLASH_START_ADDR_PAGE4) //占用12*10个字节
#define AT24CXX_ADDR_SECTION_SIZE     120 
#define AT24CXX_ADDR_SECTION_CHECKSUM     (AT24CXX_ADDR_SECTION+AT24CXX_ADDR_SECTION_SIZE)

//分段校准范围
#define AT24CXX_ADDR_SECTION_RANGE     (AT24CXX_ADDR_SECTION_CHECKSUM+1) //占用24*10个字节
#define AT24CXX_ADDR_SECTION_RANGE_SIZE     240 
#define AT24CXX_ADDR_SECTION_RANGE_CHECKSUM     (AT24CXX_ADDR_SECTION_RANGE+AT24CXX_ADDR_SECTION_RANGE_SIZE)


//分段校准当前计数值
#define AT24CXX_ADDR_SECTION_RANGE_CNT     (AT24CXX_ADDR_SECTION_RANGE_CHECKSUM+1) //占用2*6个字节
#define AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE     12 
#define AT24CXX_ADDR_SECTION_RANGE_CNT_CHECKSUM     (AT24CXX_ADDR_SECTION_RANGE_CNT+AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE)

//分段校准零点值
#define AT24CXX_ADDR_SECTION_ZERO     (AT24CXX_ADDR_SECTION_RANGE_CNT_CHECKSUM+1) //占用2*6=12个字节
#define AT24CXX_ADDR_SECTION_ZERO_SIZE     12 
#define AT24CXX_ADDR_SECTION_ZERO_CHECKSUM     (AT24CXX_ADDR_SECTION_ZERO+AT24CXX_ADDR_SECTION_ZERO_SIZE)
//PAGE4目前占用388字节

//电能修正系数
#define AT24CXX_ADDR_ENERGY1_RATIO     EEPROM_FLASH_START_ADDR_PAGE5 //占用5*6=30个字节
#define AT24CXX_ADDR_ENERGY1_RATIO_SIZE     4 
#define AT24CXX_ADDR_ENERGY1_RATIO_CHECKSUM     (AT24CXX_ADDR_ENERGY1_RATIO+AT24CXX_ADDR_ENERGY1_RATIO_SIZE)
//PAGE5目前占用30字节

//ADC校正系数（k,b）
#define AT24CXX_ADDR_ADC_RATIO     EEPROM_FLASH_START_ADDR_PAGE6 //占用4+4+1个字节
#define AT24CXX_ADDR_ADC_RATIO_SIZE     sizeof(DCMETER_ADC_RATIO) 
#define AT24CXX_ADDR_ADC_RATIO_CHECKSUM     (AT24CXX_ADDR_ADC_RATIO+AT24CXX_ADDR_ADC_RATIO_SIZE)
//PAGE6目前占用9字节


//回路1-6电压为50V时对应电流
#define AT24CXX_ADDR_VOL50V_CURRENT1     (AT24CXX_ADDR_READING_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_VOL50V_CURRENT1_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT1_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT1+AT24CXX_ADDR_VOL50V_CURRENT1_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT2     (AT24CXX_ADDR_VOL50V_CURRENT1_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_VOL50V_CURRENT2_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT2_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT2+AT24CXX_ADDR_VOL50V_CURRENT2_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT3     (AT24CXX_ADDR_VOL50V_CURRENT2_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_VOL50V_CURRENT3_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT3_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT3+AT24CXX_ADDR_VOL50V_CURRENT3_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT4     (AT24CXX_ADDR_VOL50V_CURRENT3_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_VOL50V_CURRENT4_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT4_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT4+AT24CXX_ADDR_VOL50V_CURRENT4_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT5     (AT24CXX_ADDR_VOL50V_CURRENT4_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_VOL50V_CURRENT5_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT5_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT5+AT24CXX_ADDR_VOL50V_CURRENT5_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT6     (AT24CXX_ADDR_VOL50V_CURRENT5_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_VOL50V_CURRENT6_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT6_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT6+AT24CXX_ADDR_VOL50V_CURRENT6_SIZE)



#define AT24CXX_ADDR_MINUTE_DATA_CNT     (AT24CXX_ADDR_ENERGY_CNT_CHECKSUM+1) //占用4个字节
#define AT24CXX_ADDR_MINUTE_DATA_CNT_SIZE     4 
#define AT24CXX_ADDR_MINUTE_DATA_CNT_CHECKSUM     (AT24CXX_ADDR_MINUTE_DATA_CNT+AT24CXX_ADDR_MINUTE_DATA_CNT_SIZE)

//全部电能
#define AT24CXX_ADDR_ENERGY_ALL     (AT24CXX_ADDR_ALARM_CNT_CHECKSUM+1) //占用6*4=24个字节
#define AT24CXX_ADDR_ENERGY_ALL_SIZE     24 
#define AT24CXX_ADDR_ENERGY_ALL_CHECKSUM     (AT24CXX_ADDR_ENERGY_ALL+AT24CXX_ADDR_ENERGY_ALL_SIZE)


//能量临时结构体
#define AT24CXX_ADDR_TEMP_ENERGY     (AT24CXX_ADDR_ENERGY_ALL_CHECKSUM+1) //占用6*4=24个字节
#define AT24CXX_ADDR_TEMP_ENERGY_SIZE     40 
#define AT24CXX_ADDR_TEMP_ENERGY_CHECKSUM     (AT24CXX_ADDR_TEMP_ENERGY+AT24CXX_ADDR_TEMP_ENERGY_SIZE)


//校验和总个数，用于恢复出厂设置
//#define AT24CXX_COUNT_CHECKSUM     111

/*
#define COMM_TM_YEAR_VALID(year)  ((year>=2000)&&(year<=2099))
#define COMM_TM_MONTH_VALID(Month)  ((Month>=1)&&(Month<=12))
#define COMM_TM_DAY_VALID(Day)  ((Day>=1)&&(Day<=31))
#define COMM_TM_HOUR_VALID(x)  ((x>=0)&&(x<=23))
#define COMM_TM_MIN_SEC_VALID(x)  ((x>=0)&&(x<=59))
*/
typedef struct __attribute__((packed)) _PM5KT_COMM_TM
{
	uint16_t			Year;		/*1970--*/
	uint8_t			Month;		/*1-12*/
	uint8_t			Day;		/*1-31*/
	uint8_t			Hour;		/*0-23*/
	uint8_t			Minute;		/*0-59*/
	uint8_t			Second;		/*0-59*/
}PM5KT_COMM_TM, *PPM5KT_COMM_TM;

//typedef struct __attribute__((packed)) _PM5KT_COMM_TM_EX
//{
//	uint8_t			Year;		/*70--*/
//	uint8_t			Month;		/*1-12*/
//	uint8_t			Day;		/*1-31*/
//	uint8_t			Hour;		/*0-23*/
//	uint8_t			Minute;		/*0-59*/
//	uint8_t			Second;		/*0-59*/
//}PM5KT_COMM_TM_EX, *PPM5KT_COMM_TM_EX;
extern void WatchDogInit(void);
extern void WatchDogFeed(void);

extern void Comm_DelayUs(uint32_t UsCount);
//检查日期时间的有效性
extern uint32_t CheckDateValid(const PPM5KT_COMM_TM pTm);
extern uint8_t GetSum(uint8_t *buf,uint8_t cnt);
extern void Bubble_Sort(uint32_t *unsorted,uint32_t len);
extern uint32_t WriteDataFlash(uint32_t StartAddr,__packed uint32_t *pData,uint8_t WriteCnt);
extern uint32_t ReadDataFlash(uint32_t StartAddr,__packed uint32_t *pData,uint8_t ReadCnt);
extern uint32_t EraseDataFlash_Page(uint32_t StartAddr);

extern uint32_t RN8209C_Calibration_OFF_ALL(PRN8209C_DEF pRn8209c);
extern uint32_t RN8209C_Calibration_GN_ALL(PRN8209C_DEF pRn8209c);
//将Rn8209校准后数据写入DataFlash(起始地址：0x1f000-0x1fffff)
extern uint32_t WriteRn8209CalidataToFlash(PRN8209C_DEF pRn8209c);
//从DataFlash(起始地址：0x1f000-0x1fffff)读出Rn8209校准数据
extern uint32_t ReadRn8209CalidataFromFlash(PRN8209C_DEF pRn8209c);

extern uint32_t GetRealTimeData_Loop(uint8_t ChipNum,PDCMETER_DATA pData);
    
extern uint32_t GetRealTimeData(PDCMETER_DATA pData);

extern uint32_t GetRealTimeAcData(PDCMETER_DATA pData);
extern uint32_t ADCCalibration(uint32_t SecondFlag,float y,PDCMETER_ADC_RATIO pAdcRatio);
//extern uint32_t CS5460_GetCalibrationData(uint8_t *pBuf,uint32_t *pLen);
    
//extern uint32_t GetCS5460CalibrationDataAndSet(void);
  
//-1:???? 0 :?? 1:??????
//extern uint32_t MeterData_GetValidDataFromAT24CXX(uint8_t *pData,uint32_t StartAddr,uint16_t ReadBytes);
//0 ：成功 1：数据未初始化 2:数据错误 3:输入参数错误
extern uint32_t MeterData_WaitReadSucc(uint8_t IsReadBackup,uint8_t *pData,uint32_t StartAddr,uint16_t ReadBytes);
    
//extern uint32_t MeterData_SetValidDataAndCheckToAT24CXX(uint8_t *pData,uint32_t StartAddr,uint16_t WriteBytes);

//Input: IsReadBackup为1时,读取备份区，0时不读取备份区
//0:成功 1：参数错误 2：写数据失败 3：写备份数据失败
extern uint32_t MeterData_WaitWriteSucc(uint8_t IsReadBackup,uint8_t *pData,uint32_t StartAddr,uint16_t WriteBytes);
 

//动态更新数据结构体，占用99字节,未超过100,每次读写一页，根据实时适中的分钟保存到相应叶
#define FLASH_ADDR_FREQUENT_UPDATE  0xd0000
#define SAVE_FREQUENT_UPDATE_SIZE  115
typedef struct __attribute__((packed)) _DCMETER_FREQUENT_UPDATE{
     
	DCMETER_DATE Date;
    uint32_t MinuteDataCnt;
    DCMETER_TEMP_ENERGY		TempEnergy;
    DCMETER_ALARM_PARAM     AlarmParam;
	float	LoopEnergy[DCMETER_LOOP_CNT_MAX];	//????
    DCMETER_ENERGY_READING EgyReading;
    //uint8_t  CheckSum;
}DCMETER_FREQUENT_UPDATE,*PDCMETER_FREQUENT_UPDATE;

extern uint32_t SaveFrequentUpdateData(PDCMETER_DATA pData);
extern uint32_t GetFrequentUpdateData(PDCMETER_DATA pData);

extern void MeterDataInit(uint8_t IsInit,PDCMETER_DATA pData);

//??????????
#define DC_VOLTAGE_ALARM_BACKLASH   0.5 
//??????????
#define AC_VOLTAGE_ALARM_BACKLASH   9

#define DC_VOL_LOW_ALARM_MASK  0x01
#define DC_VOL_HIGH_ALARM_MASK  0x02
#define DC_FIRST_POWERDOWN_ALARM_MASK  0x04
#define MODULE_ERROR_ALARM_MASK  0x08

#define AC_POWER_CUT_ALARM_MASK  0x01
/**/

extern uint32_t GetRealTimeAlarm(PDCMETER_DATA pData,uint8_t *pIsAlarm);

extern void Sys_Soft_Reset(void);
extern void Sys_Hard_Reset(void);
extern void MeterFactoryReset(void); 
//extern void MeterTimePcf8563ToYdt1363(PPM5KT_COMM_TM_EX const pTmEx,PDCMETER_DATE pProcTm);
//extern void MeterTimeYdt1363ToPcf8563(PDCMETER_DATE const pProcTm,PPM5KT_COMM_TM_EX pTmEx);

/*
???????????,10?=120?
??(7+48+1)*120 = 6720 Bytes
??7???,44??????,1????
*/
#define METER_SAVE_ENERGY_MONTH_CNT_MAX  120
//????????? 0-9999
#define SAVE_ENERGY_FLASH_START_ADDR  0
//??FLASH??????????,?????EEPROM
#define SAVE_ENERGY_BYTES  56


typedef struct __attribute__((packed)) _MonthEnergy{
    uint32_t CMCCEnergy;
    uint32_t CMCCRatio;
    uint32_t CUCCEnergy;
    uint32_t CUCCRatio;
    uint32_t CTCCEnergy;
    uint32_t CTCCRatio;
    uint32_t LoopEnergy[DCMETER_LOOP_CNT_MAX];
    
}MonthEnergy,*PMonthEnergy;

typedef struct __attribute__((packed)) _MonthEnergyData{
    DCMETER_DATE Date;
    MonthEnergy MonEgy;
    uint8_t  CheckSum;
}MonthEnergyData,*PMonthEnergyData;

extern uint32_t MeterMonthEnergy_MakeRecord(PMonthEnergyData pData,uint8_t *pRecord,uint8_t *plen);
extern void MeterGetUserTotalEnergy(PDCMETER_PARAM pParam,DCMETER_USER_CONFIG User,PDCMETER_TEMP_ENERGY pTemp,float *pEnergy);
extern uint32_t MeterMonthEnergy_GetRecord(uint32_t AssignCnt,PMonthEnergyData pMonEgyData);
extern uint32_t MeterMonthEnergy_SaveRecord(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData);

/*
???????????? 3600=1h
??(7+120+1)*3600 = 460800 Bytes
??7???,120??????,1????
*/
#define METER_SAVE_DATA_MINUTE_CNT_MAX  3000
//?????????? 1000000-1499999
#define SAVE_DATA_MINUTE_FLASH_START_ADDR  0xda00
//??FLASH???????????,?????EEPROM
#define SAVE_DATA_MINUTE_BYTES  128

typedef struct __attribute__((packed)) _HistoryData_Loop{
    uint32_t	LoopVoltage;	//????
	uint32_t	LoopCurrent;	//????
	uint32_t	LoopPower;		//????
	uint32_t	LoopEnergy;		//????
}HistoryData_Loop,*PHistoryData_Loop;

typedef struct __attribute__((packed)) _SaveHistoryData{
    DCMETER_DATE Date;
    HistoryData_Loop Loop[DCMETER_LOOP_CNT_MAX];
    uint32_t CMCCEnergy;
    uint32_t CMCCRatio;
    uint32_t CUCCEnergy;
    uint32_t CUCCRatio;
    uint32_t CTCCEnergy;
    uint32_t CTCCRatio;
    uint8_t  CheckSum;
}SaveHistoryData,*PSaveHistoryData;

extern uint32_t MeterMinuteHourData_MakeRecord(PSaveHistoryData pData,uint8_t *pRecord,uint8_t *plen);
extern uint32_t MeterMinuteData_GetRecord(uint32_t AssignCnt,PSaveHistoryData pMinData);
extern uint32_t MeterMinuteData_SaveRecord(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData);

/*
????????????? 3600=150Day=5Month
??(7+120+1)*3600 = 460800 Bytes
??7???,120??????,1????
*/
#define METER_SAVE_DATA_HOUR_CNT_MAX  3000
//?????????? 1500000-1999999
#define SAVE_DATA_HOUR_FLASH_START_ADDR  0x6b600
//??FLASH??????????,?????EEPROM
#define SAVE_DATA_HOUR_BYTES  128
extern uint32_t MeterHourData_GetRecord(uint32_t AssignCnt,PSaveHistoryData pHourData);
extern uint32_t MeterHourData_SaveRecord(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData);

/*
???????????? 120
??(7+120+1)*120 = 15360 Bytes
??7???,120??????,1????
*/
#define METER_SAVE_DATA_MONTH_CNT_MAX  120
//?????????? 10000-29999
#define SAVE_DATA_MONTH_FLASH_START_ADDR  0x1c00
//??FLASH??????????,?????EEPROM
#define SAVE_DATA_MONTH_BYTES  128
extern uint32_t MeterMonthData_GetRecord(uint32_t AssignCnt,PSaveHistoryData pHourData);
extern uint32_t MeterMonthData_SaveRecord(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData);

/*
?????????? 3000
??(7+3+1)*3000 = 33000 Bytes
??7???,3??????,1????
*/
#define METER_SAVE_ALARM_CNT_MAX  3000
//?????????? 30000-69999
#define SAVE_ALARM_FLASH_START_ADDR  0x5800
//??FLASH??????,?????EEPROM
#define SAVE_ALARM_BYTES  11

typedef struct __attribute__((packed)) _SaveHistoryAlarm{
    DCMETER_DATE Date;
    DCMETER_HISTORY_ALARM Alarm;
    uint8_t  CheckSum;
}SaveHistoryAlarm,*PSaveHistoryAlarm;

extern uint32_t MeterAlarm_GetRecord(uint32_t AssignCnt,PSaveHistoryAlarm pAlarm);
extern uint32_t MeterAlarm_SaveRecordJudge(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData);

#define  SAVE_FLAG_MONTH_ENERGY_MASK (1<<0)
#define  SAVE_FLAG_MINUTE_DATA_MASK (1<<1)
#define  SAVE_FLAG_HOUR_DATA_MASK (1<<2)
#define  SAVE_FLAG_MONTH_DATA_MASK (1<<3)
//#define  SAVE_FLAG_ALARM_MASK (1<<4)
//???????????????,1s???????
//extern void GetSaveFlagFromDate(const DCMETER_DATE Date,
//                const DCMETER_READING ReadDay,uint32_t *pSetFlag,uint32_t *pSaveFlag);
//extern void GetSaveFlagFromDate(const DCMETER_DATE Date,const DCMETER_READING ReadDay,uint32_t *pSaveFlag);
//extern void SaveRecordJudge(uint32_t *pFlag,const PDCMETER_DATA pDcMeterData);
//根据标志位判断哪些记录是否需要保存
extern void MeterSaveHistoryData(__packed uint32_t *pSaveFlag,const PDCMETER_DATA pData);



//获取校准值存储起始地址
//extern PSECTION_CALIBRATION GetCaliValuePoint(void);

//extern void CaliValueInit(PSECTION_CALIBRATION pData);

//校准霍尔传感器 CaliSrc 校准源*SECTIONS_CALIBRATION_RATIO
extern uint32_t CalibrationHallSensor(uint8_t LoopNum,uint32_t CaliSrc,PSECTION_CALIBRATION pSectCali,PDCMETER_DATA pData);
//撤销上次校准
extern uint8_t CalibrationHallSensor_Repeal(uint8_t LoopNum,PSECTION_CALIBRATION pSectCali);
//撤销零点校准
extern uint8_t CalibrationHallSensor_ZERO_Repeal(uint8_t LoopNum,PSECTION_CALIBRATION pSectCali);
//根据输入电流值获取校准系数(LoopNum：回路号1-6 Ival：测量电流值 pSectCali：校准系数指针)
extern float GetSectionCaliRatio(uint32_t LoopNum,float Ival,PSECTION_CALIBRATION pSectCali);

extern uint32_t CheckUserConfig(void);

/*
关于参数有效性检查操作，参数有效范围说明:
1、校准时电流通道输入值	[0,10],默认:2.5
2、校准时电压通道输入值 [0,100],默认:60.0
3、系统参数 
		设备地址		(0,255],默认:1
		波特率			(0,115200],默认:9600
		分路1用户配置	[0,6],默认:1(USER_ALL)
		分路2用户配置	[0,6],默认:2(USER_CMCC)
		分路3用户配置	[0,6],默认:3(USER_CUCC)
		分路4用户配置	[0,6],默认:4(USER_CTCC)
		分路5用户配置	[0,6],默认:5(USER_RESERVE1)
		分路6用户配置	[0,6],默认:6(USER_RESERVE2)
4、模块参数 
		直流电压高告警阈值	(0,65],默认:DCMETER_MODULE_PARAM_DC_HIGH_DEFAULT(58)
		直流电压低告警阈值	(0,60],默认:DCMETER_MODULE_PARAM_DC_LOW_DEFAULT(47)
		直流一次下电告警阈值(0,60],默认:DCMETER_MODULE_PARAM_DC_FIRST_POWERDOWN_DEFAULT(46)
		交流停电告警阈值	(0,220],默认:DCMETER_MODULE_PARAM_AC_POWERDOWN_DEFAULT(85)
5、抄表日 
		日	(0,31],默认:DCMETER_READING_DAY_DEFAULT(1)
		时	[0,23],默认:DCMETER_READING_HOUR_DEFAULT(0)

6、CT变比
		分路1	(0,200],默认:100
		分路2	(0,200],默认:50
		分路3	(0,200],默认:50
		分路4	(0,200],默认:50
		分路5	(0,200],默认:25
		分路6	(0,200],默认:25
7、电能修正系数
		分路1	[0.8,1.2],默认:1.0
		分路2	[0.8,1.2],默认:1.0
		分路3	[0.8,1.2],默认:1.0
		分路4	[0.8,1.2],默认:1.0
		分路5	[0.8,1.2],默认:1.0
		分路6	[0.8,1.2],默认:1.0
*/

#define CHECK_PARAM_DC_I_MAX(x) do{	\
	if(((x)<0)||((x)>10)){			\
		(x) = DC_I_MAX;				\
	}								\
}while(0)			

#define CHECK_PARAM_DC_V_MAX(x) do{	\
		if(((x)<0)||((x)>100)){ 		\
			(x) = DC_V_MAX; 			\
		}								\
	}while(0)

#define CHECK_PARAM_SYS_PARAM(x) do{	\
			if(((x.Addr)<=0)||((x.Addr)>0xff)){		\
				(x.Addr) = DCMETER_ADDR; 			\
			}								\
			if(((x.Baudrate)<=0)||((x.Baudrate)>115200)){		\
				(x.Baudrate) = DCMETER_BAUDRATE; 			\
			}								\
			if(((x.LoopConfig[0])<USER_NONE)||((x.LoopConfig[0])>USER_RESERVE2)){		\
				(x.LoopConfig[0]) = USER_ALL; 			\
			}								\
			if(((x.LoopConfig[1])<USER_NONE)||((x.LoopConfig[1])>USER_RESERVE2)){		\
				(x.LoopConfig[1]) = USER_CMCC; 			\
			}								\
			if(((x.LoopConfig[2])<USER_NONE)||((x.LoopConfig[2])>USER_RESERVE2)){		\
				(x.LoopConfig[2]) = USER_CUCC; 			\
			}								\
			if(((x.LoopConfig[3])<USER_NONE)||((x.LoopConfig[3])>USER_RESERVE2)){		\
				(x.LoopConfig[3]) = USER_CTCC; 			\
			}								\
			if(((x.LoopConfig[4])<USER_NONE)||((x.LoopConfig[4])>USER_RESERVE2)){		\
				(x.LoopConfig[4]) = USER_RESERVE1; 			\
			}								\
			if(((x.LoopConfig[5])<USER_NONE)||((x.LoopConfig[5])>USER_RESERVE2)){		\
				(x.LoopConfig[5]) = USER_RESERVE2; 			\
			}								\
		}while(0)

#define CHECK_PARAM_MODULE_PARAM(x) do{	\
						if(((x.DcHighVoltageThreshold)<=0)||((x.DcHighVoltageThreshold)>65)){		\
							(x.DcHighVoltageThreshold) = (float)DCMETER_MODULE_PARAM_DC_HIGH_DEFAULT/1.0;			\
						}								\
						if(((x.DcLowVoltageThreshold)<=0)||((x.DcLowVoltageThreshold)>60)){		\
							(x.DcLowVoltageThreshold) = (float)DCMETER_MODULE_PARAM_DC_LOW_DEFAULT/1.0;		\
						}								\
						if(((x.DcFirstPowerDownThreshold)<=0)||((x.DcFirstPowerDownThreshold)>60)){		\
							(x.DcFirstPowerDownThreshold) = (float)DCMETER_MODULE_PARAM_DC_FIRST_POWERDOWN_DEFAULT/1.0;			\
						}								\
						if(((x.AcPowerCutThreshold)<=0)||((x.AcPowerCutThreshold)>220)){		\
							(x.AcPowerCutThreshold) = (float)DCMETER_MODULE_PARAM_AC_POWERDOWN_DEFAULT/1.0;			\
						}								\
					}while(0)

#define CHECK_PARAM_READING(x) do{	\
	if(((x.Day)<=0)||((x.Day)>31)){ 		\
			(x.Day) = DCMETER_READING_DAY_DEFAULT; 			\
		}											\
	if((x.Hour)>=24){ 		\
			(x.Hour) = DCMETER_READING_HOUR_DEFAULT; 			\
		}                                               \
	}while(0)
	
#define CHECK_PARAM_CT_VALUE(x) do{	\
	if(((x.CT[0])<=0)||((x.CT[0])>200)){		\
			(x.CT[0]) = 100;			\
		}								\
	if(((x.CT[1])<=0)||((x.CT[1])>200)){		\
			(x.CT[1]) = 50;			\
		}								\
	if(((x.CT[2])<=0)||((x.CT[2])>200)){		\
			(x.CT[2]) = 50;			\
		}								\
	if(((x.CT[3])<=0)||((x.CT[3])>200)){		\
			(x.CT[3]) = 50;			\
		}								\
	if(((x.CT[4])<=0)||((x.CT[4])>200)){		\
			(x.CT[4]) = 25;			\
		}								\
	if(((x.CT[5])<=0)||((x.CT[5])>200)){		\
			(x.CT[5]) = 25;			\
		}								\
	}while(0)
	
#define CHECK_PARAM_EGY_RATIO(x) do{	\
		if(((x.Value[0])<0.8)||((x.Value[0])>1.2)){		\
				(x.Value[0]) = 1.0;			\
			}								\
		if(((x.Value[1])<0.8)||((x.Value[1])>1.2)){		\
				(x.Value[1]) = 1.0; 		\
			}								\
		if(((x.Value[2])<0.8)||((x.Value[2])>1.2)){		\
				(x.Value[2]) = 1.0; 		\
			}								\
		if(((x.Value[3])<0.8)||((x.Value[3])>1.2)){		\
				(x.Value[3]) = 1.0; 		\
			}								\
		if(((x.Value[4])<0.8)||((x.Value[4])>1.2)){		\
				(x.Value[4]) = 1.0; 		\
			}								\
		if(((x.Value[5])<0.8)||((x.Value[5])>1.2)){		\
				(x.Value[5]) = 1.0; 		\
			}								\
		}while(0)


//#define DEBUG

#ifdef DEBUG
	#define _DEBUG 1
#else 
	#define _DEBUG 0
#endif

#define debug_conf(conf,fmt,args...) \
	do{	\
		if(conf)	\
			printf(fmt,##args);\
	}while(0)
		
#define debug(fmt,args...) debug_conf(_DEBUG,fmt,##args)


#endif
