#include "NUC029xAN.h"
#include "PCF8563.h"
#include "UserCommon.h"

static void PCF8563_IIC_Init(void) //初始化IIC
{
    //EESDA P45
    GPIO_SetMode(P4, BIT5, GPIO_PMD_QUASI);
    //EESCL P44
    GPIO_SetMode(P4, BIT4, GPIO_PMD_OUTPUT);
    PCF8563_PIN_SDA = 1;
    PCF8563_PIN_SCL = 1;
}

static void PCF8563_I2C_Start(void)
{
    Comm_DelayUs(2);
	
	PCF8563_PIN_SCL = 1;
	PCF8563_PIN_SDA = 1;
    Comm_DelayUs(2);
	PCF8563_PIN_SDA = 0;
    Comm_DelayUs(4);
}

static void PCF8563_I2C_Stop(void)
{
    Comm_DelayUs(2);
	PCF8563_PIN_SDA = 0;
    Comm_DelayUs(2);
	PCF8563_PIN_SCL = 1;
    Comm_DelayUs(8);
	PCF8563_PIN_SDA = 1;
    Comm_DelayUs(2);
}

static void PCF8563_I2C_SendOneByte(unsigned char c)
{
    int i; 
    for (i=0; i<8; i++)
    {
        Comm_DelayUs(2);
		PCF8563_PIN_SCL = 0;
        Comm_DelayUs(2);

        if (c & (1<<(7-i)))
			PCF8563_PIN_SDA = 1;
        else
			PCF8563_PIN_SDA = 0;

        Comm_DelayUs(2);
        PCF8563_PIN_SCL = 1;
        Comm_DelayUs(2);
        PCF8563_PIN_SCL = 0;
    }
    Comm_DelayUs(4); 
}

static unsigned char PCF8563_I2C_RecvOneByte(void)
{
    unsigned char j=0;
    int i;
	int data1,data2,data3;

    for (i=0; i<8; i++)
    {
        Comm_DelayUs(2);
        PCF8563_PIN_SCL = 0;
		Comm_DelayUs(2);
        PCF8563_PIN_SCL = 1;
        Comm_DelayUs(2);

		data1 = PCF8563_PIN_SDA;
		Comm_DelayUs(4);
		data2 = PCF8563_PIN_SDA;
		Comm_DelayUs(4);
		data3 = PCF8563_PIN_SDA;
        if (data1 && data2 && data3)
            j+=(1<<(7-i));

        Comm_DelayUs(2);
        PCF8563_PIN_SCL = 0;
    }
    
    Comm_DelayUs(2);

    return j;
}

static int PCF8563_I2C_RecvAck(void)
{
    int nack;
	int nack2;
	int nack3;
    
	Comm_DelayUs(2);

    PCF8563_PIN_SCL = 0;
    Comm_DelayUs(2);
    PCF8563_PIN_SCL = 1;
    //Comm_DelayUs(1);
	Comm_DelayUs(8);
    nack = PCF8563_PIN_SDA;
	Comm_DelayUs(4);
	nack2 = PCF8563_PIN_SDA;
	Comm_DelayUs(4);
	nack3 = PCF8563_PIN_SDA;

    Comm_DelayUs(2);
    PCF8563_PIN_SCL = 0;
    Comm_DelayUs(2);

    if (nack == 0 && nack2 == 0 && nack3 == 0)
        return 1; 

    return 0;
}

static void PCF8563_I2C_SendAck(void)
{
    Comm_DelayUs(2);
    PCF8563_PIN_SDA = 0;
    Comm_DelayUs(2);
    PCF8563_PIN_SCL = 1;
    Comm_DelayUs(4);
    PCF8563_PIN_SCL = 0;
    Comm_DelayUs(2);
	PCF8563_PIN_SDA = 1;
    Comm_DelayUs(2);
}

static void PCF8563_I2C_SendNack(void)
{
    Comm_DelayUs(2);
    PCF8563_PIN_SDA = 1;
    Comm_DelayUs(2);
    PCF8563_PIN_SCL = 1;
    Comm_DelayUs(4);
    PCF8563_PIN_SCL = 0;
    Comm_DelayUs(2);
    PCF8563_PIN_SDA = 0;
    Comm_DelayUs(2);
}


static unsigned char GetPCF8563(unsigned char firsttype,unsigned char count,unsigned char *buff)
{
	unsigned char i; 	
	
	PCF8563_I2C_Start();
	PCF8563_I2C_SendOneByte((unsigned char)(PCF8563_Write));
	if (!PCF8563_I2C_RecvAck())
	{
		PCF8563_I2C_Stop();
		//printf("GetPCF8563 function step1 no ack\n");
		return	0;
	}
	PCF8563_I2C_SendOneByte((unsigned char)(firsttype));
	if (!PCF8563_I2C_RecvAck())
	{
		PCF8563_I2C_Stop();
		//printf("GetPCF8563 function step2 no ack\n");
		return	0;
	}
	PCF8563_I2C_Start();
	PCF8563_I2C_SendOneByte((unsigned char)(PCF8563_Read));
	if (!PCF8563_I2C_RecvAck())
	{
		PCF8563_I2C_Stop();
		//printf("GetPCF8563 function step3 no ack\n");
		return	0;
	}
	for (i=0;i<count;i++)
	{
		buff[i]=PCF8563_I2C_RecvOneByte();
		if (i==(count-1)) 
            PCF8563_I2C_SendNack();
        else
			PCF8563_I2C_SendAck(); //????????,?????MASTER????
	}
	
	PCF8563_I2C_Stop();
	return	1;
}

static unsigned char SetPCF8563(unsigned char timetype,unsigned char value)
{
	//unsigned char i; 	
	
	PCF8563_I2C_Start();
	PCF8563_I2C_SendOneByte((unsigned char)(PCF8563_Write));
	if (!PCF8563_I2C_RecvAck())
	{
		PCF8563_I2C_Stop();
		//printf("GetPCF8563 function step1 no ack\n");
		return	0;
	}
	PCF8563_I2C_SendOneByte((unsigned char)(timetype));
	if (!PCF8563_I2C_RecvAck())
	{
		PCF8563_I2C_Stop();
		//printf("GetPCF8563 function step1 no ack\n");
		return	0;
	}
	PCF8563_I2C_SendOneByte((unsigned char)(value));
	if (!PCF8563_I2C_RecvAck())
	{
		PCF8563_I2C_Stop();
		//printf("GetPCF8563 function step1 no ack\n");
		return	0;
	}
	PCF8563_I2C_Stop();
	
	return	1;
}

#define BCD2BIN(val)	(((val) & 0x0f) + ((val)>>4)*10)
#define BIN2BCD(val)	((((val)/10)<<4) + (val)%10)

/**
  *****************************************************************************
  * @Name   : PCF8563??????
  *
  * @Brief  : ????????????????????????,????,?????
  *
  * @Input  : none
  *
  * @Output : none
  *
  * @Return : 0: ??
  *           1: PCF8563??????
  *****************************************************************************
**/

extern unsigned char PCF8563_Check(void)
{
	unsigned char test_value = 0;
	unsigned char Time_Count = 0;  //??????????
    unsigned char u8Temp;
    
    if(GetPCF8563(PCF8563_Address_Timer,1,&u8Temp)==0)
        return 1;
    
	if(u8Temp & 0x80)  //????????,????
	{
		if(0 == SetPCF8563(PCF8563_Address_Timer, PCF_Timer_Close))
            return 1;
        if(GetPCF8563(PCF8563_Address_Timer_VAL,1,&u8Temp)==0)
            return 1;
		Time_Count = u8Temp;  //??????
	}

    if(0 == SetPCF8563(PCF8563_Address_Timer_VAL, PCF8563_Check_Data))
        return 1;
	for(test_value = 0;test_value < 50;test_value++)  {}  //?????????
        
    if(GetPCF8563(PCF8563_Address_Timer_VAL,1,&u8Temp)==0)
        return 1;
	test_value = u8Temp;  //?????

	if(Time_Count != 0)  //????????,???
	{
        if(0 == SetPCF8563(PCF8563_Address_Timer_VAL, Time_Count))
            return 1;
        if(0 == SetPCF8563(PCF8563_Address_Timer, PCF_Timer_Open))
            return 1;
	}

	if(test_value != PCF8563_Check_Data)  
        return 1;  //????????
	
	return 0;  //??
}

/**
  *****************************************************************************
  * @Name   : PCF8563??
  *
  * @Brief  : none
  *
  * @Input  : none
  *
  * @Output : none
  *
  * @Return : 0成功、1失败
  *****************************************************************************
**/
extern unsigned char PCF8563_Start(void)
{
	unsigned char u8Temp;
    
    if(GetPCF8563(PCF8563_Address_Control_Status_1,1,&u8Temp)==0)
        return 1;
    
	if (u8Temp & PCF_Control_ChipStop)
	{
		u8Temp &= PCF_Control_ChipRuns;  //????
	}
	if ((u8Temp & (1<<7)) == 0)  //????
	{
		u8Temp &= PCF_Control_TestcClose;  //????????
	}

    if(0 == SetPCF8563(PCF8563_Address_Control_Status_1, u8Temp))
        return 1;
    //if(0 == SetPCF8563(0x0d, 0x80))
    //    return 1;
    return 0;
}

/**
  *****************************************************************************
  * @Name   : PCF8563_Stop
  *
  * @Brief  : ??????CLKOUT ? 32.768kHz ???
  *
  * @Input  : none
  *
  * @Output : none
  *
  * @Return : 0成功、1失败
  *****************************************************************************
**/
extern unsigned char PCF8563_Stop(void)
{
	unsigned char u8Temp;
    
    if(GetPCF8563(PCF8563_Address_Control_Status_1,1,&u8Temp)==0)
        return 1;
	u8Temp |= PCF_Control_ChipStop;  //????
    if(0 == SetPCF8563(PCF8563_Address_Control_Status_1, u8Temp))
        return 1;
    return 0;
}


extern int PCF8563_Get_time(PPM5KT_COMM_TM rtc_tm)
{
    uint8_t u8Temp;
    uint8_t Is19XX=0;
	//unsigned long flags;
	//unsigned char temp[20]={0};
    /*unsigned char sta1,sta2,clk;
    if(GetPCF8563(PCF8563_Address_Control_Status_1,1,&sta1)==0)
		return 0;
    if(GetPCF8563(PCF8563_Address_Control_Status_2,1,&sta2)==0)
		return 0;*/
    
	if(GetPCF8563(PCF8563_Address_Seconds,1,&rtc_tm->Second)==0)
		return 0;
	if(GetPCF8563(PCF8563_Address_Minutes,1,&rtc_tm->Minute)==0)
		return 0; 
	if(GetPCF8563(PCF8563_Address_Hours,1,&rtc_tm->Hour)==0)
		return 0; 
	if(GetPCF8563(PCF8563_Address_Days,1,&rtc_tm->Day)==0)
		return 0; 
	if(GetPCF8563(PCF8563_Address_Months,1,&rtc_tm->Month)==0)
		return 0; 
	if(GetPCF8563(PCF8563_Address_Years,1,&u8Temp)==0)
		return 0;  
    
    if(rtc_tm->Month&0x80)
        Is19XX = 1;
    else
        Is19XX = 0;
        
    rtc_tm->Year = (uint16_t)u8Temp;
    
	rtc_tm->Second	&= PCF8563_Shield_Seconds;
	rtc_tm->Minute	&= PCF8563_Shield_Minutes;
	rtc_tm->Hour	&= PCF8563_Shield_Hours;
	rtc_tm->Day		&= PCF8563_Shield_Days;
	rtc_tm->Month	&= PCF8563_Shield_Months_Century;
	

	rtc_tm->Second 	= BCD2BIN(rtc_tm->Second);
	rtc_tm->Minute 	= BCD2BIN(rtc_tm->Minute);
	rtc_tm->Hour 	= BCD2BIN(rtc_tm->Hour);
	rtc_tm->Day 	= BCD2BIN(rtc_tm->Day);
	rtc_tm->Month 	= BCD2BIN(rtc_tm->Month);
	rtc_tm->Year 	= BCD2BIN(rtc_tm->Year);
    
	if (0 == Is19XX)
		rtc_tm->Year += 2000;
    else
        rtc_tm->Year += 1900;

	//rtc_tm->Month;

	return 1;
}

extern int PCF8563_Set_time(PPM5KT_COMM_TM rtc_tm)
{
	unsigned char mon, day, hrs, min, sec;//, leap_yr;
	unsigned int yrs;
    //unsigned char sta1,sta2;
    
    
    /*if((rtc_tm->Month==0)||(rtc_tm->Month>12))
        return 0;
    if((rtc_tm->Day==0)||(rtc_tm->Day>31))
        return 0;
    if(rtc_tm->Hour>23)
        return 0;
    if(rtc_tm->Minute>59)
        return 0;
    if(rtc_tm->Second>59)
        return 0;
    */
	yrs = rtc_tm->Year;
	//mon = rtc_tm->Month + 1;   /* tm_mon starts at zero */
    mon = rtc_tm->Month;
	day = rtc_tm->Day;
	hrs = rtc_tm->Hour;
	min = rtc_tm->Minute;
	sec = rtc_tm->Second;

	//leap_yr = ((!(yrs % 4) && (yrs % 100)) || !(yrs % 400));
    
    
	if (yrs >= 2000){
        yrs -= 2000;/* RTC (0, 1, ... 69) */
    }else{
        yrs -= 1900;	/* RTC (70, 71, ... 99) */
        mon |= 0x80;
    }

	sec = BIN2BCD(sec);
	min = BIN2BCD(min);
	hrs = BIN2BCD(hrs);
	day = BIN2BCD(day);
	mon = BIN2BCD(mon);
	yrs = BIN2BCD(yrs);

    /*if(SetPCF8563(PCF8563_Address_Control_Status_1,0x0)==0)
		return 0;
    if(SetPCF8563(PCF8563_Address_Control_Status_2,0xc0)==0)
		return 0; */
    
	if(SetPCF8563(PCF8563_Address_Seconds,sec)==0)
		return 0;
	if(SetPCF8563(PCF8563_Address_Minutes,min)==0)
		return 0;
	if(SetPCF8563(PCF8563_Address_Hours,hrs)==0)
		return 0;
	if(SetPCF8563(PCF8563_Address_Days,day)==0)
		return 0;
	if(SetPCF8563(PCF8563_Address_Months,mon)==0)
		return 0;
	if(SetPCF8563(PCF8563_Address_Years,yrs)==0)
		return 0;
    
    
    /*
    if(GetPCF8563(PCF8563_Address_Control_Status_1,1,&sta1)==0)
		return 0;
    if(GetPCF8563(PCF8563_Address_Control_Status_2,1,&sta2)==0)
		return 0;*/
    
	return 1;
}

extern unsigned char PCF8563_Init(void)
{
    PCF8563_IIC_Init();
    return PCF8563_Start();
}
