/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 3 $
 * $Date: 14/01/28 11:45a $
 * @brief    Template project for NUC029 series MCU
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NUC029xAN.h"
#include "Usart.h"
#include "RN8209C.h"
#include "UserCommon.h"
#include "NRX8010.h"
#include "AT45DBXXX.h" 


uint8_t USART0_SendBuf[PROTOCAL_BUF_MAX];
uint32_t USART0_SendLen;
//??????
volatile uint32_t SysSecond=0;

//#define RunLed P32
#define RunLed P43
void RunLedInit(void)
{
    //GPIO_SetMode(P3, BIT2, GPIO_PMD_OUTPUT);
    GPIO_SetMode(P4, BIT3, GPIO_PMD_OUTPUT);
    RunLed = 1;
}

void AC_ADC_Init(void)
{
    // Enable channel 1
    //ADC_Open(ADC, 0, 0, 0x01 << 5);
    ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_SINGLE, 0x1 << 1);
    // Power on ADC
    ADC_POWER_ON(ADC);
    // Enable ADC convert complete interrupt
    //ADC_EnableInt(ADC, ADC_ADIF_INT);
    //NVIC_EnableIRQ(ADC_IRQn);
}

void TMR0_IRQHandler(void)
{
    if(TIMER_GetIntFlag(TIMER0) == 1)
    {
        /* Clear Timer0 time-out interrupt flag */
        TIMER_ClearIntFlag(TIMER0);

        SysSecond++;
        
    }
}

void SYS_Init(void)
{
	
	
    /* Unlock protected registers */
    SYS_UnlockReg();

	/*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Enable Internal RC 22.1184MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Enable external XTAL 12MHz clock */
    //CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    //CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);
    
    /* Switch HCLK clock source to XTL */
    //CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_50MHZ);
	//CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLK_S_HXT);
	
	/* STCLK to XTL STCLK to XTL */
	//CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLK_S_HXT);
    //CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLK_S_HXT);
    /* Switch HCLK clock source to XTL */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(2));
    
    /* Enable IP module clock */
    CLK_EnableModuleClock(UART0_MODULE);
	CLK_EnableModuleClock(UART1_MODULE);
	CLK_EnableModuleClock(TMR0_MODULE);
    CLK_EnableModuleClock(TMR1_MODULE);
    CLK_EnableModuleClock(ADC_MODULE);
    CLK_EnableModuleClock(WDT_MODULE);
    /* Select UART module clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_CLKDIV_UART(2));
	CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_CLKDIV_UART(2));
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0_S_HCLK, CLK_CLKDIV_HCLK(1));
	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1_S_HIRC, CLK_CLKDIV_HCLK(2));
    CLK_SetModuleClock(ADC_MODULE,CLK_CLKSEL1_ADC_S_HIRC,CLK_CLKDIV_ADC(11));
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
     
    /* Set P3 multi-function pins for UART0 RXD and TXD */
    SYS->P3_MFP &= ~(SYS_MFP_P30_Msk | SYS_MFP_P31_Msk);
    SYS->P3_MFP |= (SYS_MFP_P30_RXD0 | SYS_MFP_P31_TXD0);

	SYS->P1_MFP &= ~(SYS_MFP_P12_Msk | SYS_MFP_P13_Msk);
    SYS->P1_MFP |= (SYS_MFP_P12_RXD1 | SYS_MFP_P13_TXD1);
    SYS->P1_MFP &= ~(SYS_MFP_P10_Msk | SYS_MFP_P11_Msk);
    SYS->P1_MFP |= (SYS_MFP_P10_AIN0 | SYS_MFP_P11_AIN1);
    
    /* Disable the P1.0 - P1.1 digital input path to avoid the leakage current */
    GPIO_DISABLE_DIGITAL_PATH(P1, 0x03);
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock and cyclesPerUs automatically. */
    SystemCoreClockUpdate();


    /* Lock protected registers */
    SYS_LockReg();

}

void WDT_Init(uint32_t u32TimeoutInterval,
               uint32_t u32ResetDelay,
               uint32_t u32EnableReset,
               uint32_t u32EnableWakeup)
{
    SYS_UnlockReg();
    WDT_Open(u32TimeoutInterval, u32ResetDelay, u32EnableReset, u32EnableWakeup);
    SYS_LockReg();
}
void WDT_Feed(void)
{
    SYS_UnlockReg();
    WDT_RESET_COUNTER();
    SYS_LockReg();
}
//秒发生器
void Timer0Init(void)
{
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);
}
/*
uint8_t WBuf[10]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a};
uint8_t RBuf[10]={0};
void PrintfBuf(uint8_t *pBuf,uint32_t Len)
{
    uint32_t i=0;
    do{
        printf("%u ",pBuf[i]);
    }while(++i<Len);
    printf("\n");
}*/

int main()
{
    uint8_t i,CID2Temp;
    volatile uint8_t LastMinute=0;
    volatile uint32_t LastSysSnd=0;
    volatile uint8_t DateSnd=0;
    volatile uint8_t LastDateSnd=0;
    uint32_t SaveHtyFlag=0;
    PM5KT_COMM_TM TmTemp;
    //uint32_t SetFlag=0;
    //u32 RunCount=0;
    //uint32_t Offset=0,u32Temp;
    uint8_t Alarm[7]={0},LastAlarm[7]={0};
    
    //RN8209C_DEF RN8209Def;
    uint32_t u32Temp;
    PDCMETER_DATA pMeterData=NULL;

    SYS_Init();
    RunLedInit();
    WatchDogInit();
    debug("Run Start\n");
    
    UART1_Init();
   
    RX8010_Init();
    
    AT45DBXXX_Init();
    
    AC_ADC_Init();
    Timer0Init();
    pMeterData = ProtocolProc_YDT1363_3_CallBack();
    memset(pMeterData,0,sizeof(DCMETER_DATA));
    CheckUserConfig();
    WatchDogFeed();
    if(0 == RX8010_Check()){
        RX8010_GetTime(&TmTemp);
        if(0==CheckDateValid(&TmTemp)){
            memcpy(&pMeterData->Date,&TmTemp,sizeof(DCMETER_DATE));
        }
    }
    WatchDogFeed();
    //LastMinute = pMeterData->Date.Minute;每次重启多一条分钟记录
    MeterDataInit(1,pMeterData);
    for(i=0;i<3;i++){
        WatchDogFeed();
        RN8209C_Select(i+1);
        Comm_DelayUs(10);
        RN8209C_Init(&pMeterData->Rn8209c[i].InitParam);
        RN8209C_Select(0);
    }

    if((pMeterData->Param.Baudrate == 1200)
        ||(pMeterData->Param.Baudrate == 2400)
        ||(pMeterData->Param.Baudrate == 4800)
        ||(pMeterData->Param.Baudrate == 9600)
        ||(pMeterData->Param.Baudrate == 19200))
        UART0_Init(pMeterData->Param.Baudrate);
    else
        UART0_Init(9600);
    //???1.6s
    //WDT_Init(WDT_TIMEOUT_2POW14, 0, TRUE, FALSE);
    WatchDogFeed();
    while(1)
    {
       
        //定时读取实时时钟
        if(SysSecond!=LastSysSnd){
            LastSysSnd = SysSecond;
            RunLed = (RunLed==0?1:0);
            WatchDogFeed();
            debug("RunLed ...\n");
            GetRealTimeAcData(pMeterData);
            if(0 == RX8010_Check()){
                RX8010_GetTime((PPM5KT_COMM_TM)&TmTemp);
                if(0==CheckDateValid(&TmTemp)){
                    memcpy(&pMeterData->Date,&TmTemp,sizeof(DCMETER_DATE));
                }
            }
            DateSnd = pMeterData->Date.Second;
            //debug("Run....\n");
        }
        
        //每分钟清除保存标记，所有记录在一分钟内（每秒保存一次）仍未保存成功，则不再保存
        //SaveHtyFlag = 0;语句所在位置很重要，之前当秒到0时，保存了历史数据一次，
        //然后while循环一圈，发现是新的1分钟，则清除保存标记，在下一秒又保存了一次
        if(LastMinute != pMeterData->Date.Minute){
            
            LastMinute = pMeterData->Date.Minute;
            WatchDogFeed();
            SaveFrequentUpdateData(pMeterData);
            SaveHtyFlag = 0;
			//每分钟检查下电能芯片的参数是否被篡改（实测有效）
			for(i=0;i<3;i++){
                WatchDogFeed();
				RN8209C_Select(i+1);
				Comm_DelayUs(10);
				if(pMeterData->Rn8209c[i].InitParam.UNION.PARAM_List.CHKSUM != RN8209C_GetCheckSum()){
					RN8209C_Init(&pMeterData->Rn8209c[i].InitParam);
				}
				RN8209C_Select(0);
			}
			
        }
        //将定时读取时钟和定时检查数据的判断分开
        if(LastDateSnd!=DateSnd){
            LastDateSnd = DateSnd;
            WatchDogFeed();
            GetRealTimeData(pMeterData); 
            
            MeterSaveHistoryData(&SaveHtyFlag,pMeterData);
            
            if(SysSecond>2){
                memset(Alarm,0,7);
                GetRealTimeAlarm(pMeterData,Alarm);
                if(0!=memcmp(Alarm,LastAlarm,7)){   
                    //printf("GetRealTimeAlarm Alarm[1]:%d LastAlarm[1]:%d\n",Alarm[1],LastAlarm[1]);
                    SetDataFlag(0,1);
                }
            }
            
            if(0==MeterAlarm_SaveRecordJudge(&pMeterData->SaveCnt.AlarmCnt,pMeterData)){
                memcpy(LastAlarm,Alarm,7);
            }
        }
        
        if(USART0_RX_Succ)
        {
            //WatchDogFeed();
            USART0_RX_Succ = 0;
            USART0_SendLen = sizeof(USART0_SendBuf);
            
            if(YDT1363_FUNC_RET_SUCCESS == ProtocolProc_YDT1363_3_ProtocolPocess(\
                USART0_RX_Buf,&USART0_RX_Len,USART0_SendBuf,&USART0_SendLen)){
                
                USART0_SendData(USART0_SendBuf,USART0_SendLen);
                CID2Temp = GetCID2();
                //?????????,?????
                if((CID2Temp == YDT1363_3_PROTOCAL_CID2_SET_PARAM)
                    ||(CID2Temp == YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_GN)
                    ||(CID2Temp == YDT1363_3_PROTOCAL_CID2_SET_CS5460A_DC_MAX))//同步RN8209C的极值
                    Sys_Soft_Reset();
                else if(CID2Temp == YDT1363_3_PROTOCAL_CID2_FACTORY_RESET)
                    MeterFactoryReset();
                else if(CID2Temp == YDT1363_3_PROTOCAL_CID2_GET_ALARM)
                    SetDataFlag(0,0);
                else if((CID2Temp == YDT1363_3_PROTOCAL_CID2_SET_TO_BOOT)
                    &&(YDT1363_3_PROTOCAL_RTN_NORMAL == GetRTN())){
                    Comm_DelayUs(10*1000);//保证串口发送完成
                    Sys_Hard_Reset();
                }
                    
            }else{
               debug("Usart1 Parse Error\n\r");     
            }
            WatchDogFeed();
       }
    }
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
