
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
                             外部函数头文件
                  应用到不同的外设头文件请在这里修改即可                        
******************************************************************************/
#include "UserCommon.h"
/******************************************************************************
                                 参数宏定义
******************************************************************************/
#define PCF8563_PIN_SDA P45 
#define PCF8563_PIN_SCL P44       
        
#define PCF8563_Check_Data                       (unsigned char)0x55  //检测用，可用其他数值

#define PCF8563_Write                            (unsigned char)0xa2  //写命令
#define PCF8563_Read                             (unsigned char)0xa3  //读命令，或者用（PCF8563_Write + 1）

//
//电源复位功能
//
#define PCF8563_PowerResetEnable                 (unsigned char)0x08
#define PCF8563_PowerResetDisable                (unsigned char)0x09

//
//世纪位操作定义
//
#define PCF_Century_SetBitC                      (unsigned char)0x80

#define PCF_Century_19xx                         (unsigned char)0x03
#define PCF_Century_20xx                         (unsigned char)0x04

//
//数据格式
//
#define PCF_Format_BIN                           (unsigned char)0x01
#define PCF_Format_BCD                           (unsigned char)0x02

//
//设置PCF8563模式用
//
#define PCF_Mode_Normal                          (unsigned char)0x05
#define PCF_Mode_EXT_CLK                         (unsigned char)0x06

#define PCF_Mode_INT_Alarm                       (unsigned char)0x07
#define PCF_Mode_INT_Timer                       (unsigned char)0x08

/******************************************************************************
                             参数寄存器地址宏定义
******************************************************************************/

#define PCF8563_Address_Control_Status_1         (unsigned char)0x00  //控制/状态寄存器1
#define PCF8563_Address_Control_Status_2         (unsigned char)0x01  //控制/状态寄存器2

#define PCF8563_Address_CLKOUT                   (unsigned char)0x0d  //CLKOUT频率寄存器
#define PCF8563_Address_Timer                    (unsigned char)0x0e  //定时器控制寄存器
#define PCF8563_Address_Timer_VAL                (unsigned char)0x0f  //定时器倒计数寄存器

#define PCF8563_Address_Years                    (unsigned char)0x08  //年
#define PCF8563_Address_Months                   (unsigned char)0x07  //月
#define PCF8563_Address_Days                     (unsigned char)0x05  //日
#define PCF8563_Address_WeekDays                 (unsigned char)0x06  //星期
#define PCF8563_Address_Hours                    (unsigned char)0x04  //小时
#define PCF8563_Address_Minutes                  (unsigned char)0x03  //分钟
#define PCF8563_Address_Seconds                  (unsigned char)0x02  //秒

#define PCF8563_Alarm_Minutes                    (unsigned char)0x09  //分钟报警
#define PCF8563_Alarm_Hours                      (unsigned char)0x0a  //小时报警
#define PCF8563_Alarm_Days                       (unsigned char)0x0b  //日报警
#define PCF8563_Alarm_WeekDays                   (unsigned char)0x0c  //星期报警


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
                               参数屏蔽宏定义
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
                              PCF8563寄存器结构定义                          
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
