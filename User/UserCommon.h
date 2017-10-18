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
����FLASH���ݷ���(
PageSize=0x200,
BlockSize=0x1000,
SectorSize=0x20000,
Capacity=0x200000,
TotalPages=0x1080,)

��2M�ռ�ֳ������֣�
    1)������:0x0-0xfffff,��0x100000Bytes
    2)������:0x100000-0x1fffff,��0x100000Bytes
    
   
�������ֿ飺
    0x0     -   0x1bff      �¶������(56B*120=6780 ��ҳ����Ϊ:7168)
    0x1c00  -   0x57ff      ��ʷ������(128B*120=15360 ��ҳ����Ϊ:15360,+7168=22528)
    0x5800  -   0xd9ff      �澯����(11B*3000=33000,��ҳ����Ϊ:33280,+22528=55808))
    0xda00  -   0x6b5ff     ��ʷ��������(128B*3000=384000,��ҳ����Ϊ:384000,+55808=439808)
    0x6b600 -   0xc91ff     ��ʷСʱ����(128B*3000=384000,��ҳ����Ϊ:384000,+439808=823808)
    0xc9200 -   0xcxxxx     ϵͳ����(1024B,��ҳ����Ϊ:1024,+823808=824832)
    0xd0000 -   0xd77ff    ��̬������0x7800Bytes,60Pages           

***********************************/

#define AT24CXX_ADDR_BACKUP_OFFSET 0x100000  	//?����?��?�䨮?����?��??����y 200

//#define PROTOCAL_BUF_MAX 400  	//?����?��?�䨮?����?��??����y 200
#define PROTOCAL_BUF_MAX 300  	//?����?��?�䨮?����?��??����y 200
//Ǩ��EEPROM���ݵ�FLASH��
//ǰһҳ�ų��ı�Ĳ���
//��һҳ�Ų����ı�Ĳ���(��ʼ����־����ռһҳ)
#define EEPROM_FLASH_START_ADDR_PAGE1  0xc9200
#define EEPROM_FLASH_START_ADDR_PAGE2  0xc9400

//���澯������������һ��ҳ���Ϊ�澯�Ĳ���Ƶ�ʲ��̶�
#define EEPROM_FLASH_START_ADDR_PAGE3  0xc9600
//���ֶ�У׼���ݵ�����һ��ҳ
#define EEPROM_FLASH_START_ADDR_PAGE4  0xc9800
//����������ϵ��������һҳ
#define EEPROM_FLASH_START_ADDR_PAGE5  0xc9a00
//��ADC����ϵ��������һҳ
#define EEPROM_FLASH_START_ADDR_PAGE6  0xc9c00
//������
#define AT24CXX_ADDR_PARAM     (EEPROM_FLASH_START_ADDR_PAGE1) //ռ��26���ֽ�
#define AT24CXX_ADDR_PARAM_SIZE     sizeof(DCMETER_PARAM) 
#define AT24CXX_ADDR_PARAM_CHECKSUM     (EEPROM_FLASH_START_ADDR_PAGE1+AT24CXX_ADDR_PARAM_SIZE)

//CS5460оƬ�ĵ���ͨ��У׼ֵ
#define AT24CXX_ADDR_PARAM_CS5460_DC_I     (AT24CXX_ADDR_PARAM_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_PARAM_CS5460_DC_I_SIZE     4 
#define AT24CXX_ADDR_PARAM_CS5460_DC_I_CHECKSUM     (AT24CXX_ADDR_PARAM_CS5460_DC_I+AT24CXX_ADDR_PARAM_CS5460_DC_I_SIZE)
//CS5460оƬ�ĵ�ѹͨ��У׼ֵ
#define AT24CXX_ADDR_PARAM_CS5460_DC_V     (AT24CXX_ADDR_PARAM_CS5460_DC_I_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_PARAM_CS5460_DC_V_SIZE     4 
#define AT24CXX_ADDR_PARAM_CS5460_DC_V_CHECKSUM     (AT24CXX_ADDR_PARAM_CS5460_DC_V+AT24CXX_ADDR_PARAM_CS5460_DC_V_SIZE)
//ģ�����
#define AT24CXX_ADDR_MODULE_PARAM     (AT24CXX_ADDR_PARAM_CS5460_DC_V_CHECKSUM+1) //ռ��24���ֽ�
#define AT24CXX_ADDR_MODULE_PARAM_SIZE     sizeof(DCMETER_MODULE_PARAM) 
#define AT24CXX_ADDR_MODULE_PARAM_CHECKSUM     (AT24CXX_ADDR_MODULE_PARAM+AT24CXX_ADDR_MODULE_PARAM_SIZE)
//������
#define AT24CXX_ADDR_READING     (AT24CXX_ADDR_MODULE_PARAM_CHECKSUM+1) //ռ��2���ֽ�
#define AT24CXX_ADDR_READING_SIZE     2 
#define AT24CXX_ADDR_READING_CHECKSUM     (AT24CXX_ADDR_READING+AT24CXX_ADDR_READING_SIZE)

//��·1-6��Ӧ��CTֵ
#define AT24CXX_ADDR_CT1     (AT24CXX_ADDR_READING_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_CT1_SIZE     4 
#define AT24CXX_ADDR_CT1_CHECKSUM     (AT24CXX_ADDR_CT1+AT24CXX_ADDR_CT1_SIZE)
#define AT24CXX_ADDR_CT2     (AT24CXX_ADDR_CT1_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_CT2_SIZE     4 
#define AT24CXX_ADDR_CT2_CHECKSUM     (AT24CXX_ADDR_CT2+AT24CXX_ADDR_CT2_SIZE)
#define AT24CXX_ADDR_CT3     (AT24CXX_ADDR_CT2_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_CT3_SIZE     4 
#define AT24CXX_ADDR_CT3_CHECKSUM     (AT24CXX_ADDR_CT3+AT24CXX_ADDR_CT3_SIZE)
#define AT24CXX_ADDR_CT4     (AT24CXX_ADDR_CT3_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_CT4_SIZE     4 
#define AT24CXX_ADDR_CT4_CHECKSUM     (AT24CXX_ADDR_CT4+AT24CXX_ADDR_CT4_SIZE)
#define AT24CXX_ADDR_CT5     (AT24CXX_ADDR_CT4_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_CT5_SIZE     4 
#define AT24CXX_ADDR_CT5_CHECKSUM     (AT24CXX_ADDR_CT5+AT24CXX_ADDR_CT5_SIZE)
#define AT24CXX_ADDR_CT6     (AT24CXX_ADDR_CT5_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_CT6_SIZE     4 
#define AT24CXX_ADDR_CT6_CHECKSUM     (AT24CXX_ADDR_CT6+AT24CXX_ADDR_CT6_SIZE)

//FLASH�б���������ݼ���
#define AT24CXX_ADDR_MONTH_DATA_CNT     (AT24CXX_ADDR_CT6_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_MONTH_DATA_CNT_SIZE     4 
#define AT24CXX_ADDR_MONTH_DATA_CNT_CHECKSUM     (AT24CXX_ADDR_MONTH_DATA_CNT+AT24CXX_ADDR_MONTH_DATA_CNT_SIZE)

#define AT24CXX_ADDR_ENERGY_CNT     (AT24CXX_ADDR_MONTH_DATA_CNT_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_ENERGY_CNT_SIZE     4 
#define AT24CXX_ADDR_ENERGY_CNT_CHECKSUM     (AT24CXX_ADDR_ENERGY_CNT+AT24CXX_ADDR_ENERGY_CNT_SIZE)

#define AT24CXX_ADDR_HOUR_DATA_CNT     (AT24CXX_ADDR_ENERGY_CNT_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_HOUR_DATA_CNT_SIZE     4 
#define AT24CXX_ADDR_HOUR_DATA_CNT_CHECKSUM     (AT24CXX_ADDR_HOUR_DATA_CNT+AT24CXX_ADDR_HOUR_DATA_CNT_SIZE)

//PAGE1Ŀǰռ��110�ֽ�

//�״γ�ʼ�����ݱ�־
#define AT24CXX_ADDR_FIRST_INIT_DW        0x5aa5a55a
#define AT24CXX_ADDR_FIRST_INIT     (EEPROM_FLASH_START_ADDR_PAGE2) //ռ4���ֽ�
#define AT24CXX_ADDR_FIRST_INIT_SIZE     4 
#define AT24CXX_ADDR_FIRST_INIT_CHECKSUM     (AT24CXX_ADDR_FIRST_INIT+AT24CXX_ADDR_FIRST_INIT_SIZE)
//PAGE2Ŀǰռ��4�ֽ�

#define AT24CXX_ADDR_ALARM_CNT     (EEPROM_FLASH_START_ADDR_PAGE3) //ռ��4���ֽ�
#define AT24CXX_ADDR_ALARM_CNT_SIZE     4 
#define AT24CXX_ADDR_ALARM_CNT_CHECKSUM     (AT24CXX_ADDR_ALARM_CNT+AT24CXX_ADDR_ALARM_CNT_SIZE)
//PAGE3Ŀǰռ��4�ֽ�

//�ֶ�У׼ֵ
#define AT24CXX_ADDR_SECTION     (EEPROM_FLASH_START_ADDR_PAGE4) //ռ��12*10���ֽ�
#define AT24CXX_ADDR_SECTION_SIZE     120 
#define AT24CXX_ADDR_SECTION_CHECKSUM     (AT24CXX_ADDR_SECTION+AT24CXX_ADDR_SECTION_SIZE)

//�ֶ�У׼��Χ
#define AT24CXX_ADDR_SECTION_RANGE     (AT24CXX_ADDR_SECTION_CHECKSUM+1) //ռ��24*10���ֽ�
#define AT24CXX_ADDR_SECTION_RANGE_SIZE     240 
#define AT24CXX_ADDR_SECTION_RANGE_CHECKSUM     (AT24CXX_ADDR_SECTION_RANGE+AT24CXX_ADDR_SECTION_RANGE_SIZE)


//�ֶ�У׼��ǰ����ֵ
#define AT24CXX_ADDR_SECTION_RANGE_CNT     (AT24CXX_ADDR_SECTION_RANGE_CHECKSUM+1) //ռ��2*6���ֽ�
#define AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE     12 
#define AT24CXX_ADDR_SECTION_RANGE_CNT_CHECKSUM     (AT24CXX_ADDR_SECTION_RANGE_CNT+AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE)

//�ֶ�У׼���ֵ
#define AT24CXX_ADDR_SECTION_ZERO     (AT24CXX_ADDR_SECTION_RANGE_CNT_CHECKSUM+1) //ռ��2*6=12���ֽ�
#define AT24CXX_ADDR_SECTION_ZERO_SIZE     12 
#define AT24CXX_ADDR_SECTION_ZERO_CHECKSUM     (AT24CXX_ADDR_SECTION_ZERO+AT24CXX_ADDR_SECTION_ZERO_SIZE)
//PAGE4Ŀǰռ��388�ֽ�

//��������ϵ��
#define AT24CXX_ADDR_ENERGY1_RATIO     EEPROM_FLASH_START_ADDR_PAGE5 //ռ��5*6=30���ֽ�
#define AT24CXX_ADDR_ENERGY1_RATIO_SIZE     4 
#define AT24CXX_ADDR_ENERGY1_RATIO_CHECKSUM     (AT24CXX_ADDR_ENERGY1_RATIO+AT24CXX_ADDR_ENERGY1_RATIO_SIZE)
//PAGE5Ŀǰռ��30�ֽ�

//ADCУ��ϵ����k,b��
#define AT24CXX_ADDR_ADC_RATIO     EEPROM_FLASH_START_ADDR_PAGE6 //ռ��4+4+1���ֽ�
#define AT24CXX_ADDR_ADC_RATIO_SIZE     sizeof(DCMETER_ADC_RATIO) 
#define AT24CXX_ADDR_ADC_RATIO_CHECKSUM     (AT24CXX_ADDR_ADC_RATIO+AT24CXX_ADDR_ADC_RATIO_SIZE)
//PAGE6Ŀǰռ��9�ֽ�


//��·1-6��ѹΪ50Vʱ��Ӧ����
#define AT24CXX_ADDR_VOL50V_CURRENT1     (AT24CXX_ADDR_READING_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_VOL50V_CURRENT1_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT1_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT1+AT24CXX_ADDR_VOL50V_CURRENT1_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT2     (AT24CXX_ADDR_VOL50V_CURRENT1_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_VOL50V_CURRENT2_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT2_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT2+AT24CXX_ADDR_VOL50V_CURRENT2_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT3     (AT24CXX_ADDR_VOL50V_CURRENT2_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_VOL50V_CURRENT3_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT3_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT3+AT24CXX_ADDR_VOL50V_CURRENT3_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT4     (AT24CXX_ADDR_VOL50V_CURRENT3_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_VOL50V_CURRENT4_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT4_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT4+AT24CXX_ADDR_VOL50V_CURRENT4_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT5     (AT24CXX_ADDR_VOL50V_CURRENT4_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_VOL50V_CURRENT5_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT5_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT5+AT24CXX_ADDR_VOL50V_CURRENT5_SIZE)
#define AT24CXX_ADDR_VOL50V_CURRENT6     (AT24CXX_ADDR_VOL50V_CURRENT5_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_VOL50V_CURRENT6_SIZE     4 
#define AT24CXX_ADDR_VOL50V_CURRENT6_CHECKSUM     (AT24CXX_ADDR_VOL50V_CURRENT6+AT24CXX_ADDR_VOL50V_CURRENT6_SIZE)



#define AT24CXX_ADDR_MINUTE_DATA_CNT     (AT24CXX_ADDR_ENERGY_CNT_CHECKSUM+1) //ռ��4���ֽ�
#define AT24CXX_ADDR_MINUTE_DATA_CNT_SIZE     4 
#define AT24CXX_ADDR_MINUTE_DATA_CNT_CHECKSUM     (AT24CXX_ADDR_MINUTE_DATA_CNT+AT24CXX_ADDR_MINUTE_DATA_CNT_SIZE)

//ȫ������
#define AT24CXX_ADDR_ENERGY_ALL     (AT24CXX_ADDR_ALARM_CNT_CHECKSUM+1) //ռ��6*4=24���ֽ�
#define AT24CXX_ADDR_ENERGY_ALL_SIZE     24 
#define AT24CXX_ADDR_ENERGY_ALL_CHECKSUM     (AT24CXX_ADDR_ENERGY_ALL+AT24CXX_ADDR_ENERGY_ALL_SIZE)


//������ʱ�ṹ��
#define AT24CXX_ADDR_TEMP_ENERGY     (AT24CXX_ADDR_ENERGY_ALL_CHECKSUM+1) //ռ��6*4=24���ֽ�
#define AT24CXX_ADDR_TEMP_ENERGY_SIZE     40 
#define AT24CXX_ADDR_TEMP_ENERGY_CHECKSUM     (AT24CXX_ADDR_TEMP_ENERGY+AT24CXX_ADDR_TEMP_ENERGY_SIZE)


//У����ܸ��������ڻָ���������
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
//�������ʱ�����Ч��
extern uint32_t CheckDateValid(const PPM5KT_COMM_TM pTm);
extern uint8_t GetSum(uint8_t *buf,uint8_t cnt);
extern void Bubble_Sort(uint32_t *unsorted,uint32_t len);
extern uint32_t WriteDataFlash(uint32_t StartAddr,__packed uint32_t *pData,uint8_t WriteCnt);
extern uint32_t ReadDataFlash(uint32_t StartAddr,__packed uint32_t *pData,uint8_t ReadCnt);
extern uint32_t EraseDataFlash_Page(uint32_t StartAddr);

extern uint32_t RN8209C_Calibration_OFF_ALL(PRN8209C_DEF pRn8209c);
extern uint32_t RN8209C_Calibration_GN_ALL(PRN8209C_DEF pRn8209c);
//��Rn8209У׼������д��DataFlash(��ʼ��ַ��0x1f000-0x1fffff)
extern uint32_t WriteRn8209CalidataToFlash(PRN8209C_DEF pRn8209c);
//��DataFlash(��ʼ��ַ��0x1f000-0x1fffff)����Rn8209У׼����
extern uint32_t ReadRn8209CalidataFromFlash(PRN8209C_DEF pRn8209c);

extern uint32_t GetRealTimeData_Loop(uint8_t ChipNum,PDCMETER_DATA pData);
    
extern uint32_t GetRealTimeData(PDCMETER_DATA pData);

extern uint32_t GetRealTimeAcData(PDCMETER_DATA pData);
extern uint32_t ADCCalibration(uint32_t SecondFlag,float y,PDCMETER_ADC_RATIO pAdcRatio);
//extern uint32_t CS5460_GetCalibrationData(uint8_t *pBuf,uint32_t *pLen);
    
//extern uint32_t GetCS5460CalibrationDataAndSet(void);
  
//-1:???? 0 :?? 1:??????
//extern uint32_t MeterData_GetValidDataFromAT24CXX(uint8_t *pData,uint32_t StartAddr,uint16_t ReadBytes);
//0 ���ɹ� 1������δ��ʼ�� 2:���ݴ��� 3:�����������
extern uint32_t MeterData_WaitReadSucc(uint8_t IsReadBackup,uint8_t *pData,uint32_t StartAddr,uint16_t ReadBytes);
    
//extern uint32_t MeterData_SetValidDataAndCheckToAT24CXX(uint8_t *pData,uint32_t StartAddr,uint16_t WriteBytes);

//Input: IsReadBackupΪ1ʱ,��ȡ��������0ʱ����ȡ������
//0:�ɹ� 1���������� 2��д����ʧ�� 3��д��������ʧ��
extern uint32_t MeterData_WaitWriteSucc(uint8_t IsReadBackup,uint8_t *pData,uint32_t StartAddr,uint16_t WriteBytes);
 

//��̬�������ݽṹ�壬ռ��99�ֽ�,δ����100,ÿ�ζ�дһҳ������ʵʱ���еķ��ӱ��浽��ӦҶ
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
//���ݱ�־λ�ж���Щ��¼�Ƿ���Ҫ����
extern void MeterSaveHistoryData(__packed uint32_t *pSaveFlag,const PDCMETER_DATA pData);



//��ȡУ׼ֵ�洢��ʼ��ַ
//extern PSECTION_CALIBRATION GetCaliValuePoint(void);

//extern void CaliValueInit(PSECTION_CALIBRATION pData);

//У׼���������� CaliSrc У׼Դ*SECTIONS_CALIBRATION_RATIO
extern uint32_t CalibrationHallSensor(uint8_t LoopNum,uint32_t CaliSrc,PSECTION_CALIBRATION pSectCali,PDCMETER_DATA pData);
//�����ϴ�У׼
extern uint8_t CalibrationHallSensor_Repeal(uint8_t LoopNum,PSECTION_CALIBRATION pSectCali);
//�������У׼
extern uint8_t CalibrationHallSensor_ZERO_Repeal(uint8_t LoopNum,PSECTION_CALIBRATION pSectCali);
//�����������ֵ��ȡУ׼ϵ��(LoopNum����·��1-6 Ival����������ֵ pSectCali��У׼ϵ��ָ��)
extern float GetSectionCaliRatio(uint32_t LoopNum,float Ival,PSECTION_CALIBRATION pSectCali);

extern uint32_t CheckUserConfig(void);
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