
#ifndef __PCF8563_H_ 
#define __PCF8563_H_


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
/******************************************************************************
                                 �����궨��
******************************************************************************/
#define PCF8563_PIN_SDA P45 
#define PCF8563_PIN_SCL P44       
        
#define PCF8563_Check_Data                       (unsigned char)0x55  //����ã�����������ֵ

#define PCF8563_Write                            (unsigned char)0xa2  //д����
#define PCF8563_Read                             (unsigned char)0xa3  //����������ã�PCF8563_Write + 1��

//
//��Դ��λ����
//
#define PCF8563_PowerResetEnable                 (unsigned char)0x08
#define PCF8563_PowerResetDisable                (unsigned char)0x09

//
//����λ��������
//
#define PCF_Century_SetBitC                      (unsigned char)0x80

#define PCF_Century_19xx                         (unsigned char)0x03
#define PCF_Century_20xx                         (unsigned char)0x04

//
//���ݸ�ʽ
//
#define PCF_Format_BIN                           (unsigned char)0x01
#define PCF_Format_BCD                           (unsigned char)0x02

//
//����PCF8563ģʽ��
//
#define PCF_Mode_Normal                          (unsigned char)0x05
#define PCF_Mode_EXT_CLK                         (unsigned char)0x06

#define PCF_Mode_INT_Alarm                       (unsigned char)0x07
#define PCF_Mode_INT_Timer                       (unsigned char)0x08

/******************************************************************************
                             �����Ĵ�����ַ�궨��
******************************************************************************/

#define PCF8563_Address_Control_Status_1         (unsigned char)0x00  //����/״̬�Ĵ���1
#define PCF8563_Address_Control_Status_2         (unsigned char)0x01  //����/״̬�Ĵ���2

#define PCF8563_Address_CLKOUT                   (unsigned char)0x0d  //CLKOUTƵ�ʼĴ���
#define PCF8563_Address_Timer                    (unsigned char)0x0e  //��ʱ�����ƼĴ���
#define PCF8563_Address_Timer_VAL                (unsigned char)0x0f  //��ʱ���������Ĵ���

#define PCF8563_Address_Years                    (unsigned char)0x08  //��
#define PCF8563_Address_Months                   (unsigned char)0x07  //��
#define PCF8563_Address_Days                     (unsigned char)0x05  //��
#define PCF8563_Address_WeekDays                 (unsigned char)0x06  //����
#define PCF8563_Address_Hours                    (unsigned char)0x04  //Сʱ
#define PCF8563_Address_Minutes                  (unsigned char)0x03  //����
#define PCF8563_Address_Seconds                  (unsigned char)0x02  //��

#define PCF8563_Alarm_Minutes                    (unsigned char)0x09  //���ӱ���
#define PCF8563_Alarm_Hours                      (unsigned char)0x0a  //Сʱ����
#define PCF8563_Alarm_Days                       (unsigned char)0x0b  //�ձ���
#define PCF8563_Alarm_WeekDays                   (unsigned char)0x0c  //���ڱ���


//
//??/?????1 --> 0x00
//
#define PCF_Control_Status_NormalMode            (unsigned char)(~(1<<7))  //????
#define PCF_Control_Status_EXT_CLKMode           (unsigned char)(1<<7)     //EXT_CLK????
#define PCF_Control_ChipRuns                     (unsigned char)(~(1<<5))  //????
#define PCF_Control_ChipStop                     (unsigned char)(1<<5)     //??????,????????????0
#define PCF_Control_TestcClose                   (unsigned char)(~(1<<3))  //????????(????????0)
#define PCF_Control_TestcOpen                    (unsigned char)(1<<3)     //????????


//
//???????? --> 0x0e
//
#define PCF_Timer_Open                           (unsigned char)(1<<7)     //?????
#define PCF_Timer_Close                          (unsigned char)(~(1<<7))  //?????

/******************************************************************************
                               �������κ궨��
******************************************************************************/

#define PCF8563_Shield_Control_Status_1          (unsigned char)0xa8
#define PCF8563_Shield_Control_Status_2          (unsigned char)0x1f

#define PCF8563_Shield_Seconds                   (unsigned char)0x7f
#define PCF8563_Shield_Minutes                   (unsigned char)0x7f
#define PCF8563_Shield_Hours                     (unsigned char)0x3f

#define PCF8563_Shield_Days                      (unsigned char)0x3f
#define PCF8563_Shield_WeekDays                  (unsigned char)0x07
#define PCF8563_Shield_Months_Century            (unsigned char)0x1f
#define PCF8563_Shield_Years                     (unsigned char)0xff

#define PCF8563_Shield_Minute_Alarm              (unsigned char)0x7f
#define PCF8563_Shield_Hour_Alarm                (unsigned char)0x3f
#define PCF8563_Shield_Day_Alarm                 (unsigned char)0x3f
#define PCF8563_Shield_WeekDays_Alarm            (unsigned char)0x07

#define PCF8563_Shield_CLKOUT_Frequency          (unsigned char)0x03
#define PCF8563_Shield_Timer_Control             (unsigned char)0x03
#define PCF8563_Shield_Timer_Countdown_Value     (unsigned char)0xff

/******************************************************************************
                              PCF8563�Ĵ����ṹ����                          
******************************************************************************/

extern unsigned char PCF8563_Check(void);
extern unsigned char PCF8563_Start(void);
extern unsigned char PCF8563_Stop(void);
extern int PCF8563_Get_time(PPM5KT_COMM_TM rtc_tm);
extern int PCF8563_Set_time(PPM5KT_COMM_TM rtc_tm);
extern unsigned char PCF8563_Init(void);

/**
  ****************************** Support C++ **********************************
**/
#ifdef __cplusplus
	}
#endif
/**
  *****************************************************************************
**/


#endif  /* end pcf8563.h */
