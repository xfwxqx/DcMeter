#include "UserCommon.h"
#include "Ydt1363.h"
//#include "AT24Cxx.h"
#include "NRX8010.h"
//#include "Cs5460.h"
#include "AT45DBXXX.h"
#include "RN8209C.h"

#define FUNC_SUCCESS 0
#define FUNC_FAILURE !FUNC_SUCCESS
#define SUCCESS 0
#define FAILURE !SUCCESS

uint8_t  Temp[200];
//uint8_t  gCaliCnt=0;//当前校准计数
//SECTION_CALIBRATION gSectCali;

#define WD_FEED P32
extern void WatchDogInit(void)
{
    GPIO_SetMode(P3, BIT2, GPIO_PMD_OUTPUT);
    WD_FEED = 1;
}
extern void WatchDogFeed(void)
{
    WD_FEED = (WD_FEED==0?1:0);
}

extern void Comm_DelayUs(uint32_t UsCount)
{
	TIMER_Delay(TIMER1,2*UsCount);
    //TIMER_Delay(TIMER1,2*UsCount);
}
extern uint32_t CheckUserConfig(void)
{
    uint32_t Config[2],ret=0;

    SYS_UnlockReg();
    FMC_Open();
    FMC_ENABLE_CFG_UPDATE();
    FMC_ReadConfig(Config,2);
    if((Config[0]!=0xff9fff7f)||(Config[1]!=0xffffffff))
    {
        Config[0]=0xff9fff7f;
        Config[1]=0xffffffff;
        ret = FMC_WriteConfig(Config,2);
    }
    FMC_Close();
    SYS_LockReg();
    
    return ret;
}
extern uint8_t GetSum(uint8_t *buf,uint8_t cnt)
{
    uint8_t i,sum=0;
    
    for(i=0;i<cnt;i++)
        sum += buf[i];
    
    return sum;
}
extern void Bubble_Sort(uint32_t *unsorted,uint32_t len)
{
    uint32_t i,j,temp;
    
    for (i = 0; i < len; i++)
    {
        for (j = i; j < len; j++)
        {
            if (unsorted[i] > unsorted[j])
            {
                temp = unsorted[i];
                unsorted[i] = unsorted[j];
                unsorted[j] = temp;
            }
        }
    }
}

//检查日期时间的有效性
extern uint32_t CheckDateValid(const PPM5KT_COMM_TM pTm)
{
	if((pTm == NULL))
		return 1;
	if(!DATE_YEAR_IS_VALID(pTm->Year))
		return 1;
	if(!DATE_MONTH_IS_VALID(pTm->Month))
		return 1;
	if(!DATE_DAY_IS_VALID(pTm->Day))
		return 1;
	if(!DATE_HOUR_IS_VALID(pTm->Hour))
		return 1;
	if(!DATE_MINUTE_OR_SECOND_IS_VALID(pTm->Minute))
		return 1;
	if(!DATE_MINUTE_OR_SECOND_IS_VALID(pTm->Second))
		return 1;
    
	return 0;
}

//输入：pData为NULL时，仅检查数据有效性
//0 ：成功 2:数据错误 3:输入参数错误
static uint32_t MeterData_GetValidDataFromAT24CXX(uint8_t *pData,uint32_t StartAddr,uint16_t ReadBytes)
{
    uint8_t CheckSum;
    uint8_t TempBuf[151]={0};
    
    if(ReadBytes>150)
        return 3;
    //if((StartAddr>EE_TYPE)||(CheckAddr>EE_TYPE))
    //    return 3;
    
    AT45DBxxxD_Read(TempBuf,StartAddr,(uint32_t)ReadBytes+1);
    CheckSum = TempBuf[ReadBytes];
    //if(CheckSum != AT45DBxxxD_INVALID_BYTE){
        if(CheckSum == GetSum(TempBuf,ReadBytes)){
            if(pData!=NULL)
                memcpy(pData,TempBuf,ReadBytes);
            return 0;
        }
        return 2;     
    //}
    //return 1;
}

//Input: IsReadBackup为1时,读取备份区，0时不读取备份区
//0 ：成功  2:失败 3:输入参数错误
extern uint32_t MeterData_WaitReadSucc(uint8_t IsReadBackup,uint8_t *pData,uint32_t StartAddr,uint16_t ReadBytes)
{
    uint8_t i=0;
    uint32_t ret;
    uint32_t StartAddrTemp;//,CheckAddrTemp;
    
    
    if(pData==NULL)
        return 3;
    if(0 != AT45DBXXX_Check())
        return 3;

    
    StartAddrTemp = StartAddr;
    //CheckAddrTemp = CheckAddr;
    //超时获取数据
    i=0;
    do{
        i++;
        ret = MeterData_GetValidDataFromAT24CXX(pData,StartAddrTemp,ReadBytes);
        }while((ret==2)&&(i<3));
    
    if((ret==2)&&(IsReadBackup)){
        //获取失败，则从备份区获取
        StartAddrTemp = StartAddr + AT24CXX_ADDR_BACKUP_OFFSET;
        i=0;
        do{
            i++;
            ret = MeterData_GetValidDataFromAT24CXX(pData,StartAddrTemp,ReadBytes);
        }while((i<3)&&(ret==2));
        if(ret==2)
            return 2;
        return 0; 
    }else if(ret==2)
        return 2;
    
    return 0; 

}
//0:成功 1：参数错误 2：写数据失败 
static uint32_t MeterData_SetValidDataToAT24CXX(uint8_t *pData,uint32_t StartAddr,uint16_t WriteBytes)
{
    uint8_t Sum=0;
    uint8_t TempBuf[151]={0};
    
    if(pData==NULL)
        return 1;
    
    if(WriteBytes>150)
        return 1;
    
    memcpy(TempBuf,pData,WriteBytes);
    Sum = GetSum(TempBuf,WriteBytes);
    TempBuf[WriteBytes] = Sum;
    //printf("MeterData_SetValidDataToAT24CXX StartAddr=%x WriteBytes=%x\n",StartAddr,WriteBytes);
    return AT45DBxxxD_Write(TempBuf,StartAddr,(uint32_t)WriteBytes+1);
}
//0:成功 1：参数错误 2：写数据失败 
static uint32_t MeterData_SetAndCheckToAT24CXX(uint8_t *pData,uint32_t StartAddr,uint16_t WriteBytes)
{
    uint8_t i=0;
    uint32_t ret=0;
    
    if(pData==NULL)
        return 1;
    
    i=0;
    do{
        i++;
        ret = MeterData_SetValidDataToAT24CXX(pData,StartAddr,WriteBytes);
        if(ret == 0){
           ret = MeterData_GetValidDataFromAT24CXX(NULL,StartAddr,WriteBytes);
        }
    }while((i<3)&&(ret!=0));
    
    if(ret!=0)
       return 2;
    return 0;
}

//Input: IsWriteBackup为1时,读取备份区，0时不读取备份区
//0:成功 1：参数错误 2：写数据失败 3：写备份数据失败
extern uint32_t MeterData_WaitWriteSucc(uint8_t IsWriteBackup,uint8_t *pData,uint32_t StartAddr,uint16_t WriteBytes)
{
    uint32_t ret=0;
    uint32_t StartAddrTemp;
    
    if(pData==NULL)
        return 1;
    //首先检查AT24CXX是否存在
    if(0 != AT45DBXXX_Check())
        return 1;
    
    StartAddrTemp = StartAddr;
    //超时等待写成功
    ret = MeterData_SetAndCheckToAT24CXX(pData,StartAddrTemp,WriteBytes);
    if(ret != 0)
       return 2;
    
    if(IsWriteBackup){
        StartAddrTemp = StartAddr+AT24CXX_ADDR_BACKUP_OFFSET;
        //超时等待写备份数据成功
        ret = MeterData_SetAndCheckToAT24CXX(pData,StartAddrTemp,WriteBytes);
        if(ret != 0)
            return 3;
    }
    return 0;
}


//#define DATA_FLASH_START_ADDR 0x00007c00 //APROM最后1K
//#define DATA_FLASH_CS54601_OFF (DATA_FLASH_START_ADDR+0)
//#define DATA_FLASH_CS54601_GN (DATA_FLASH_START_ADDR+16)
//#define DATA_FLASH_CS54601_P (DATA_FLASH_START_ADDR+24)



extern uint32_t WriteDataFlash(uint32_t StartAddr,__packed uint32_t *pData,uint8_t WriteCnt)
{
    uint8_t i;
    uint32_t Data,Addr;
    
    if(pData==NULL)
        return 2;
    
    SYS_UnlockReg();
    FMC_Open();
    FMC_ENABLE_AP_UPDATE();
    //printf("Read Data:0x%x From Addr:0x%x\n",FMC_Read(u32DataFlashStartAddr),u32DataFlashStartAddr);

    for(i=0;i<WriteCnt;i++){
        Addr = StartAddr+4*i;
        FMC_Write(Addr,pData[i]);
        Data = FMC_Read(Addr);
        //printf("Read Data:0x%x From Addr:0x%x\n",Data,Addr);
        if(Data!=pData[i])
            return 1;
    }
        
    FMC_DISABLE_AP_UPDATE();

    FMC_Close();


    SYS_LockReg();
    return 0;
}
extern uint32_t ReadDataFlash(uint32_t StartAddr,__packed uint32_t *pData,uint8_t ReadCnt)
{
    uint8_t i;
    uint32_t Addr;
    
    if(pData==NULL)
        return 2;
    
    SYS_UnlockReg();
    FMC_Open();

    for(i=0;i<ReadCnt;i++){
        Addr = StartAddr+4*i;
        pData[i] = FMC_Read(Addr);
        //printf("Read Data:0x%x From Addr:0x%x\n",pData[i],Addr);
    }


    FMC_Close();


    SYS_LockReg();
    return 0;
}  
extern uint32_t EraseDataFlash_Page(uint32_t StartAddr)
{

    SYS_UnlockReg();
    FMC_Open();

    FMC_Erase(StartAddr);


    FMC_Close();


    SYS_LockReg();
    return 0;
}  


extern uint32_t RN8209C_Calibration_OFF_ALL(PRN8209C_DEF pRn8209c)
{
    uint32_t i,ret=0;

    for(i=0;i<3;i++){
        RN8209C_Select(i+1);
        Comm_DelayUs(10);
        ret += RN8209C_DCOffsetCalibration(&pRn8209c[i].InitParam);
        RN8209C_Select(0);//拉高所有片选
        Comm_DelayUs(10);

        //ret += WriteDataFlash(DATA_FLASH_CS54601_OFF+28*i,CalibrationData,(uint8_t)CalibrationDataLen);
    }
    return ret;
}
extern uint32_t RN8209C_Calibration_GN_ALL(PRN8209C_DEF pRn8209c)
{
    uint32_t i,ret=0;
   
    for(i=0;i<3;i++){
        RN8209C_Select(i+1);
        Comm_DelayUs(50000);
        ret += RN8209C_DCGainCalibration(&pRn8209c[i]);
        RN8209C_Select(0);//拉高所有片选
        Comm_DelayUs(50000);
        //ret += WriteDataFlash(DATA_FLASH_CS54601_GN+28*i,CalibrationData,(uint8_t)CalibrationDataLen);
    }
    return ret;
}
//将Rn8209校准后数据写入DataFlash(起始地址：0x1f000-0x1fffff)
extern uint32_t WriteRn8209CalidataToFlash(PRN8209C_DEF pRn8209c)
{
    uint32_t DWCnt;
    uint32_t StartAddr = FMC_ReadDataFlashBaseAddr();
    //uint8_t *p=(uint8_t *)pRn8209c;
    
    EraseDataFlash_Page(StartAddr);
    DWCnt = sizeof(RN8209C_DEF)/4;
    DWCnt *= 3;

    return WriteDataFlash(StartAddr,(__packed uint32_t *)pRn8209c,(uint8_t)DWCnt);
}
//从DataFlash(起始地址：0x1f000-0x1fffff)读出Rn8209校准数据
extern uint32_t ReadRn8209CalidataFromFlash(PRN8209C_DEF pRn8209c)
{
    uint32_t DWCnt;
    uint32_t StartAddr = FMC_ReadDataFlashBaseAddr();
    //uint8_t *p=(uint8_t *)pRn8209c;
    
    //EraseDataFlash(StartAddr);
    DWCnt = sizeof(RN8209C_DEF)/4;
    DWCnt *= 3;

    return ReadDataFlash(StartAddr,(__packed uint32_t *)pRn8209c,(uint8_t)DWCnt);
}
/*
extern uint32_t CS5460A_Calibration_P_ALL(void)
{
    uint32_t CalibrationData[7]={0},ret=0;
	uint8_t CalibrationDataLen=0,i,sum=0;
   
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        CS5460A_Select(i+1);
        Comm_DelayUs(10);
        CS5460A_Calibration_P(CalibrationData,&CalibrationDataLen);
        CS5460A_Select(0);//拉高所有片选
        Comm_DelayUs(10);

        ret += WriteDataFlash(DATA_FLASH_CS54601_P+28*i,CalibrationData,(uint8_t)CalibrationDataLen);
    }
    return ret;
}
*/
#define GET_REAL_TIME_DATA_PERIOD_S 3   //获取实时数据间隔，但是电能每次都会获取
extern uint32_t GetRealTimeData_Loop(uint8_t ChipNum,PDCMETER_DATA pData)
{
    volatile PDCMETER_LOOP_ANALOG pLoopAnalog=NULL;
    volatile PDCMETER_TEMP_ENERGY pTempEgy=NULL;
    volatile PDCMETER_CS5460_DC_MAX pDcMax=NULL;
    volatile PDCMETER_CT pCTRadio=NULL;
    volatile PSECTION_CALIBRATION pSectCali=NULL;
    volatile PDCMETER_ENERGY_RATIO pEgyRatio=NULL;
    PRN8209C_DEF pRn8209c=NULL;
    double dfTemp;
    float fTemp;
    float Imea;
    volatile uint8_t LoopIndex,i;//,Chip,AorB;
    signed long Curr=0;
    signed long CurrSum=0;
    
    #ifdef GET_REAL_CURR_AVERAGE
        static signed short CurrTemp[DCMETER_LOOP_CNT_MAX][5];
        static signed char CurrTempCnt[DCMETER_LOOP_CNT_MAX];
    #endif
    
    //#ifdef GET_REAL_DATA_SLOWLY
    static unsigned char LoopGetDataCnt[DCMETER_LOOP_CNT_MAX];
    
    if((ChipNum>3)||(ChipNum==0)||(pData==NULL))
        return FUNC_FAILURE;
    
    pTempEgy = &pData->TempEnergy;
    pDcMax = &pData->Cs5460Dc;
    pCTRadio = &pData->CTValue;
    pSectCali = &pData->SectCali;
    pEgyRatio = &pData->EgyRatio; 
    
    if(ChipNum<=3){
        //Chip = (LoopNum+1)/2;
        //AorB = LoopNum%2;//选择A通道还是B通道（1:A 0:B）
        pRn8209c = &pData->Rn8209c[ChipNum-1];
        
        RN8209C_Select(ChipNum);
        Comm_DelayUs(10);
        //CS5460A_Wait_DRDY_High();
        //RN8209C_WaitDataUpdate();

        if(RN8209C_TestDataUpdate()){    
            
            for(i=0;i<2;i++){
                
                LoopIndex = 2*ChipNum-2+i;
                pLoopAnalog = &pData->AnalogData.LoopAnalog[LoopIndex];
                
                if(LoopGetDataCnt[LoopIndex]%GET_REAL_TIME_DATA_PERIOD_S == 0){
                    //获取回路电压
                    RN8209C_GetU(&pRn8209c->CaliParam,&fTemp);
                    pLoopAnalog->LoopVoltage = fTemp;
                    //debug("Loop%d Voltage=%f\n\r",LoopNum,pLoopAnalog->LoopVoltage);
                    
                    //获取回路电流
                    if(i==0)
                        RN8209C_GetIa(&pRn8209c->CaliParam,&Imea);
                    else
                        RN8209C_GetIb(&pRn8209c->CaliParam,&Imea);
                    //对于单电源霍尔传感器，其零点是2.5V,如果电流输入通道没有接的话，出现较大的负电流、复功率、电能减少
                    if(Imea<(-2.0)){
                        pLoopAnalog->LoopCurrent = 0;
                        pLoopAnalog->LoopPower = 0;
                        continue;
                    }
                    Imea *= pCTRadio->CT[LoopIndex];
                    Imea *= GetSectionCaliRatio(LoopIndex+1,Imea,pSectCali);
                    Imea *= 100.0;
                    Curr = (signed long)Imea;
                    //去除零点
                    Curr -= (signed short)pSectCali->SectCaliZero[LoopIndex];
                    
                    #ifdef GET_REAL_CURR_AVERAGE
                        CurrTemp[LoopIndex][CurrTempCnt[LoopIndex]] = Curr;
                        CurrTempCnt[LoopIndex] += 1;
                        //读取三次，取平均值
                        if(CurrTempCnt[LoopIndex]==5){
                            CurrTempCnt[LoopIndex] = 0;
                            CurrSum = CurrTemp[LoopIndex][3]+CurrTemp[LoopIndex][1]+CurrTemp[LoopIndex][2];
                            //if(CurrSum<100)//小于0.33A，不显示
                            //    pLoopAnalog->LoopCurrent = 0;
                            //else
                                pLoopAnalog->LoopCurrent = (float)CurrSum/300.0;
                        }
                        //debug("Loop%d Current=%f\n\r",LoopNum,pLoopAnalog->LoopCurrent);
                    #else
                        pLoopAnalog->LoopCurrent = (float)Curr/100.0;
                    #endif
                        
                    //获取回路功率
                    if(i==0){
                        RN8209C_GetPa(&pRn8209c->CaliParam,&fTemp);
                    }else{
                        RN8209C_GetPb(&pRn8209c->CaliParam,&fTemp);
                    }
                    fTemp *= pCTRadio->CT[LoopIndex];
                    pLoopAnalog->LoopPower = fTemp;
                }
                
                LoopGetDataCnt[LoopIndex]++;
                
                //获取回路电能
                if(i==0){
                    RN8209C_GetEa(&pRn8209c->CaliParam,&dfTemp);
                }else{
                    RN8209C_GetEb(&pRn8209c->CaliParam,&dfTemp);
                }
                //修正电能
                dfTemp *= pCTRadio->CT[LoopIndex];
                dfTemp *= (double)pEgyRatio->Value[LoopIndex];
                pLoopAnalog->LoopEnergy += dfTemp;
                

                //if(pLoopAnalog->LoopEnergy>99999999.0)
                //    pLoopAnalog->LoopEnergy = 0;
                pTempEgy->LoopEnergy[LoopIndex] += (float)dfTemp; 
            
            }
            
            //if(pTempEgy->LoopEnergy[LoopNum-1]>99999999.0)
            //    pTempEgy->LoopEnergy[LoopNum-1] = 0;
            //printf("Loop%d Energy=%f\n\r",LoopNum,pLoopAnalog->LoopEnergy);
           
            
        }
        RN8209C_Select(0);//拉高所有片选
        Comm_DelayUs(10);
        return FUNC_SUCCESS;
        //printf("%d-%d-%d %d:%d:%d\n\r",1900+Tm.Year,Tm.Month,Tm.Day,Tm.Hour,Tm.Minute,Tm.Second);
    }
    return FUNC_FAILURE;
}


extern uint32_t GetRealTimeData(PDCMETER_DATA pData)
{
    uint8_t i,ret;

    if(pData==NULL)
        return FUNC_FAILURE;

    for(i=0;i<3;i++)
    {
        ret += GetRealTimeData_Loop(i+1,pData);
    }
    //pAnalog->LoopCnt = DCMETER_LOOP_CNT_MAX;
    
    return ret;
}
static uint32_t ADC_GetValue(uint32_t *pVal)
{
    uint32_t u32Temp[10],i;
    uint64_t SumAverage;
    
    if(pVal==NULL)
        return 1;
    
    for(i=0;i<10;i++){
        if(!ADC_IS_BUSY(ADC)) {
            ADC_START_CONV(ADC);
            while(!ADC_GET_INT_FLAG(ADC, ADC_ADF_INT));
            ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT); 
            // Get ADC convert result
            //printf("Convert result is %x\n", (uint32_t)ADC_GET_CONVERSION_DATA(ADC, 0));
            u32Temp[i] = ADC_GET_CONVERSION_DATA(ADC, 1);
            //printf("Convert result is %u\n", u32Temp);            
        }
    }
    Bubble_Sort(u32Temp,10);
    SumAverage = u32Temp[4]+u32Temp[5]+u32Temp[6]+u32Temp[7];
    SumAverage /= 4;
    
    SumAverage *= 33000;
    SumAverage >>= 12;
    *pVal = (uint32_t)SumAverage;
    
    return 0;
}

static uint32_t GetAdcData(float *f)
{
    uint32_t AcVol;
    float fTemp=0.0;
    
    if(f==NULL)
        return 1;
    
    if(0 == ADC_GetValue(&AcVol)){
        debug("Convert result is %u\n",AcVol);
        fTemp = (float)AcVol;
        fTemp /= 1803.27869;// 分压比：22/122
        *f = fTemp; 
        return 0;
    }
        
    return 1;
}

extern uint32_t GetRealTimeAcData(PDCMETER_DATA pData)
{
    float fTemp=0.0;
    static float f[5];
    static uint32_t Cnt;
    
    if(pData==NULL)
        return 1;
    
    if(((uint32_t)pData->ModuleParam.AcVoltageRatio)==0){
        pData->AnalogData.AcAnalog.AcVoltage = 0;
        return 2;
    }    
    
    if(0!=GetAdcData(&f[Cnt++])){
        
        return 1;
    }
    if(Cnt==5){
        Cnt = 0;
        fTemp = f[0]+f[1]+f[2]+f[3]+f[4];
        fTemp /= 5.0;
        pData->AnalogData.AcAnalog.AcVoltage = fTemp*pData->AdcRatio.k+pData->AdcRatio.b;
        pData->AnalogData.AcAnalog.AcVoltage *= pData->ModuleParam.AcVoltageRatio;
        return 0;
    }
    
    return 1;
}

extern uint32_t ADCCalibration(uint32_t SecondFlag,float y,PDCMETER_ADC_RATIO pAdcRatio)
{
    static float x1,y1,x2,y2;
    //float fTemp=0.0;
    
    if(pAdcRatio==NULL)
        return 1;
    
    if(SecondFlag==0){
        y1 = y;
        return GetAdcData(&x1);
    }else{
        y2 = y;
        
        if(0==GetAdcData(&x2)){
            pAdcRatio->x1 = x1;
            pAdcRatio->x2 = x2;
            pAdcRatio->y1 = y1;
            pAdcRatio->y2 = y2;
            pAdcRatio->k = (y2-y1)/(x2-x1);
            pAdcRatio->b = y1-x1*pAdcRatio->k;
            return 0;
        }
    }   
    return 1;
}
/*
extern uint32_t CS5460_GetCalibrationData(uint8_t *pBuf,uint32_t *pLen)
{
    uint8_t i,len;
    
    if((pBuf==NULL)||(pLen==NULL))
        return 1;
    if(*pLen<168)
        return 1;
    
    len = 0;
    for(i=0;i<6;i++){
        
        if(0 == ReadDataFlash((DATA_FLASH_CS54601_OFF+len),(uint32_t *)&pBuf[len],7)){   
            len+=28;   
        }else{
            return 1;
        }
    }
    *pLen = len;
    return 0;
}


extern uint32_t GetCS5460CalibrationDataAndSet(void)
{
    uint8_t CheckSum,i,sum=0;
    uint32_t RecvBuf[7]={0};
    
    for(i=0;i<6;i++){
        if(0 == ReadDataFlash((DATA_FLASH_CS54601_OFF+28*i),&RecvBuf[0],7)){
            CS5460A_Select(i+1);
            Comm_DelayUs(10);
            if(0 != CS5460A_Config_Calibration(RecvBuf,7)){
                CS5460A_Select(0);//拉高所有片选    
                return 1;
            }
            CS5460A_Select(0);//拉高所有片选   
            Comm_DelayUs(10);
        }
    }
    return 0;
}
*/

extern uint32_t SaveFrequentUpdateData(PDCMETER_DATA pData)
{
    uint8_t i,len;
    volatile float fTemp;
    volatile uint32_t u32Temp,Addr;
    volatile PDCMETER_FREQUENT_UPDATE pTemp=NULL;
    
    if(pData==NULL)
        return FUNC_FAILURE;
    
    len = 0;
    pTemp = (PDCMETER_FREQUENT_UPDATE)Temp;
    
    memcpy(&pTemp->Date,&pData->Date,sizeof(DCMETER_DATE));
    len += sizeof(DCMETER_DATE);
    pTemp->MinuteDataCnt = pData->SaveCnt.MinuteDataCnt;
    len += 4;
    memcpy(&pTemp->TempEnergy,&pData->TempEnergy,sizeof(DCMETER_TEMP_ENERGY));
    len += sizeof(DCMETER_TEMP_ENERGY);
    memcpy(&pTemp->AlarmParam,&pData->AlarmParam,sizeof(DCMETER_ALARM_PARAM));
    len += sizeof(DCMETER_ALARM_PARAM);
    
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++)
    {
        fTemp = (float)pData->AnalogData.LoopAnalog[i].LoopEnergy;
        pTemp->LoopEnergy[i] = fTemp;
        len += 4;
    }
    memcpy(&pTemp->EgyReading,&pData->EgyReading,sizeof(DCMETER_ENERGY_READING));
    len += sizeof(DCMETER_ENERGY_READING);
    //pTemp->CheckSum = GetSum((uint8_t *)pTemp,len);
    //len += 1;
    
    Addr = FLASH_ADDR_FREQUENT_UPDATE;
    Addr += 0x200*pData->Date.Minute;
    
    //printf("SaveFrequentUpdateData Addr=%x\n",Addr);
    
    return MeterData_WaitWriteSucc(0,(uint8_t *)pTemp,Addr,len);
}


extern uint32_t GetFrequentUpdateData(PDCMETER_DATA pData)
{
    uint8_t i,ret;//,MinTemp,HourTemp,DayTemp;
    //volatile uint32_t TimeTemp=0,NearTime=0xffffffff;
    volatile float fTemp=0.0,fMaxEnergy=0.0;
    volatile uint32_t Addr;
    volatile PDCMETER_FREQUENT_UPDATE pFrqData=NULL;
    
    if(pData==NULL)
        return FUNC_FAILURE;
    
    //MinTemp = pData->Date.Minute;  
    //HourTemp = pData->Date.Hour;
    //DayTemp = pData->Date.Day;
    fMaxEnergy = 0.0;
    for(i=0;i<60;i++)
    {   
        
        Addr = FLASH_ADDR_FREQUENT_UPDATE;
        Addr += 0x200*i;
        
        ret = MeterData_WaitReadSucc(0,Temp,Addr,SAVE_FREQUENT_UPDATE_SIZE);
        if(ret==0){
            pFrqData = (PDCMETER_FREQUENT_UPDATE)Temp;
            fTemp = pFrqData->LoopEnergy[0];
            if(fTemp>fMaxEnergy){
                fMaxEnergy = fTemp;
                pData->SaveCnt.MinuteDataCnt = pFrqData->MinuteDataCnt;
                memcpy(&pData->TempEnergy,&pFrqData->TempEnergy,sizeof(DCMETER_TEMP_ENERGY));
                //memcpy(&pData->AlarmParam,&pFrqData->AlarmParam,sizeof(DCMETER_ALARM_PARAM));
                
                for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
                    fTemp = pFrqData->LoopEnergy[i];
                    pData->AnalogData.LoopAnalog[i].LoopEnergy = (double)fTemp;
                }
                memcpy(&pData->EgyReading,&pFrqData->EgyReading,sizeof(DCMETER_ENERGY_READING));
            }
        }
        WatchDogFeed();
    }
    if(pFrqData!=NULL){ 
        return FUNC_SUCCESS;
    }
        
    return FUNC_FAILURE;
}

extern void MeterDataInit(uint8_t IsInit,PDCMETER_DATA pData)
{ 
    int32_t Ret;
    uint32_t i,Cnt;
    uint8_t  FirstInitFlag=0,UseBackup=1;
    uint32_t FirstInitDW,Addr;
    
    if(pData==NULL)
        return;
    WatchDogFeed();
    //AT24CXX_Check();
    if(0 == AT45DBXXX_Check()){
        
        //FirstInitFlag
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&FirstInitDW,AT24CXX_ADDR_FIRST_INIT,AT24CXX_ADDR_FIRST_INIT_SIZE);
        if(Ret != 0){
            FirstInitFlag = 1;
            FirstInitDW = AT24CXX_ADDR_FIRST_INIT_DW;
            //AT45DB161_BulkErase();
            
            MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&FirstInitDW,AT24CXX_ADDR_FIRST_INIT,AT24CXX_ADDR_FIRST_INIT_SIZE);
        }
        else
            FirstInitFlag=0;
        WatchDogFeed();
        //初始化电表校准参数
        /**/
        if(IsInit){
            do{
                i++;
                WatchDogFeed();
            }while((0 != ReadRn8209CalidataFromFlash(pData->Rn8209c))&&(i<5));
        }
        
        //初始化电表参数
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->Param,AT24CXX_ADDR_PARAM,AT24CXX_ADDR_PARAM_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                memset(pData->Param.Vender,0x20,sizeof(pData->Param.Vender));
                memcpy(pData->Param.Vender,DCMETER_VENDER,sizeof(DCMETER_VENDER)-1);
                pData->Param.Addr = DCMETER_ADDR;
                pData->Param.Baudrate = DCMETER_BAUDRATE;
                pData->Param.SaveTime = DCMETER_SAVE_TIME_MINUTE;
                pData->Param.LoopConfig[0] = USER_ALL;
                pData->Param.LoopConfig[1] = USER_CMCC;
                pData->Param.LoopConfig[2] = USER_CUCC;
                pData->Param.LoopConfig[3] = USER_CTCC;
                pData->Param.LoopConfig[4] = USER_NONE;
                pData->Param.LoopConfig[5] = USER_NONE;
                
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->Param,AT24CXX_ADDR_PARAM,AT24CXX_ADDR_PARAM_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        //初始化电表直流电压校准最大值和电流最大值
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->Cs5460Dc.Imax,AT24CXX_ADDR_PARAM_CS5460_DC_I,\
                    AT24CXX_ADDR_PARAM_CS5460_DC_I_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->Cs5460Dc.Imax = DC_I_MAX;
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->Cs5460Dc.Imax,AT24CXX_ADDR_PARAM_CS5460_DC_I,\
                        AT24CXX_ADDR_PARAM_CS5460_DC_I_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->Cs5460Dc.Vmax,AT24CXX_ADDR_PARAM_CS5460_DC_V,\
                    AT24CXX_ADDR_PARAM_CS5460_DC_V_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->Cs5460Dc.Vmax = DC_V_MAX;
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->Cs5460Dc.Vmax,AT24CXX_ADDR_PARAM_CS5460_DC_V,\
                        AT24CXX_ADDR_PARAM_CS5460_DC_V_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        //初始化模块参数
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->ModuleParam,AT24CXX_ADDR_MODULE_PARAM,\
                    AT24CXX_ADDR_MODULE_PARAM_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->ModuleParam.DcHighVoltageThreshold = (float)DCMETER_MODULE_PARAM_DC_HIGH_DEFAULT/1.0;
                pData->ModuleParam.DcLowVoltageThreshold = (float)DCMETER_MODULE_PARAM_DC_LOW_DEFAULT/1.0;
                pData->ModuleParam.DcFirstPowerDownThreshold = (float)DCMETER_MODULE_PARAM_DC_FIRST_POWERDOWN_DEFAULT/1.0;
                pData->ModuleParam.ModuleAlarmThreshold = (float)DCMETER_MODULE_PARAM_MODULE_ALARM_DEFAULT/1.0;
                pData->ModuleParam.AcPowerCutThreshold = (float)DCMETER_MODULE_PARAM_AC_POWERDOWN_DEFAULT/1.0;
                pData->ModuleParam.AcVoltageRatio = (float)DCMETER_MODULE_PARAM_AC_VOLTAGE_RATIO_DEFAULT/1.0;
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->ModuleParam,AT24CXX_ADDR_MODULE_PARAM, \
                                AT24CXX_ADDR_MODULE_PARAM_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }  
        WatchDogFeed();        
        //初始化模块抄表日
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->Reading,AT24CXX_ADDR_READING,AT24CXX_ADDR_READING_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->Reading.Day = DCMETER_READING_DAY_DEFAULT;
                pData->Reading.Hour = DCMETER_READING_HOUR_DEFAULT;
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->Reading,AT24CXX_ADDR_READING,AT24CXX_ADDR_READING_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        //初始化各回路CT变比
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
            Addr = AT24CXX_ADDR_CT1+(AT24CXX_ADDR_CT1_SIZE+1)*i;
            Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->CTValue.CT[i],Addr,AT24CXX_ADDR_CT1_SIZE);
            if(Ret != 0){
                if(FirstInitFlag == 1){
                    pData->CTValue.CT[i] = 25.0;
                    MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->CTValue.CT[i],Addr,AT24CXX_ADDR_CT1_SIZE);
                }
                //printf("%s %d Read Error(Ret=%ld) i=%u\n",__FUNCTION__,__LINE__,Ret,i);
            }
            WatchDogFeed();
        }
        
        
        //初始化FLASH中存储各项数据计数
        Ret = MeterData_WaitReadSucc(UseBackup,
                (uint8_t *)&pData->SaveCnt.EnergyMonthCnt,\
                AT24CXX_ADDR_ENERGY_CNT,\
                AT24CXX_ADDR_ENERGY_CNT_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->SaveCnt.EnergyMonthCnt = 0;
                MeterData_WaitWriteSucc(UseBackup,
                    (uint8_t *)&pData->SaveCnt.EnergyMonthCnt,\
                    AT24CXX_ADDR_ENERGY_CNT,\
                    AT24CXX_ADDR_ENERGY_CNT_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        Ret = MeterData_WaitReadSucc(UseBackup,
                (uint8_t *)&pData->SaveCnt.HourDataCnt,\
                AT24CXX_ADDR_HOUR_DATA_CNT,\
                AT24CXX_ADDR_HOUR_DATA_CNT_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->SaveCnt.HourDataCnt = 0;
                MeterData_WaitWriteSucc(UseBackup,
                    (uint8_t *)&pData->SaveCnt.HourDataCnt,\
                    AT24CXX_ADDR_HOUR_DATA_CNT,\
                    AT24CXX_ADDR_HOUR_DATA_CNT_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        Ret = MeterData_WaitReadSucc(UseBackup,
                (uint8_t *)&pData->SaveCnt.AlarmCnt,\
                AT24CXX_ADDR_ALARM_CNT,\
                AT24CXX_ADDR_ALARM_CNT_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->SaveCnt.EnergyMonthCnt = 0;
                MeterData_WaitWriteSucc(UseBackup,
                    (uint8_t *)&pData->SaveCnt.AlarmCnt,\
                    AT24CXX_ADDR_ALARM_CNT,\
                    AT24CXX_ADDR_ALARM_CNT_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        Ret = MeterData_WaitReadSucc(UseBackup,
                (uint8_t *)&pData->SaveCnt.MonthDataCnt,\
                AT24CXX_ADDR_MONTH_DATA_CNT,\
                AT24CXX_ADDR_MONTH_DATA_CNT_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->SaveCnt.MonthDataCnt = 0;
                MeterData_WaitWriteSucc(UseBackup,
                    (uint8_t *)&pData->SaveCnt.MonthDataCnt,\
                    AT24CXX_ADDR_MONTH_DATA_CNT,\
                    AT24CXX_ADDR_MONTH_DATA_CNT_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();


        
        //初始化分段校准计数值
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->SectCali.CaliCnt[0],
                    AT24CXX_ADDR_SECTION_RANGE_CNT,AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                memset((void *)&pData->SectCali.CaliCnt[0],0,AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE);
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->SectCali.CaliCnt[0],AT24CXX_ADDR_SECTION_RANGE_CNT,AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        //初始化分段校准参数
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->SectCali.SectCaliDiv[0],AT24CXX_ADDR_SECTION,AT24CXX_ADDR_SECTION_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                for(i=0;i<SECTIONS_MAX;i++)
                    for(Cnt=0;Cnt<DCMETER_LOOP_CNT_MAX;Cnt++)
                        pData->SectCali.SectCaliDiv[i].CaliValue[Cnt] = 1*SECTIONS_CALIBRATION_RATIO;
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->SectCali.SectCaliDiv[0],AT24CXX_ADDR_SECTION,AT24CXX_ADDR_SECTION_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld) i=%u\n",__FUNCTION__,__LINE__,Ret,i);
        }
        WatchDogFeed();
        //初始化分段校准范围
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->SectCali.SectRange[0],AT24CXX_ADDR_SECTION_RANGE,AT24CXX_ADDR_SECTION_RANGE_SIZE);
        if(Ret!=0){
            if(FirstInitFlag == 1){
                memset(&pData->SectCali.SectRange[0],0,AT24CXX_ADDR_SECTION_RANGE_SIZE);
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->SectCali.SectRange[0],AT24CXX_ADDR_SECTION_RANGE,AT24CXX_ADDR_SECTION_RANGE_SIZE);
            }
        }
        WatchDogFeed();
        //初始化分段校准零点
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->SectCali.SectCaliZero[0],AT24CXX_ADDR_SECTION_ZERO,AT24CXX_ADDR_SECTION_ZERO_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                memset((void *)&pData->SectCali.SectCaliZero[0],0,AT24CXX_ADDR_SECTION_ZERO_SIZE);
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->SectCali.SectCaliZero[0],AT24CXX_ADDR_SECTION_ZERO,AT24CXX_ADDR_SECTION_ZERO_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        //初始化电能修正系数
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
            Addr = AT24CXX_ADDR_ENERGY1_RATIO+(AT24CXX_ADDR_ENERGY1_RATIO_SIZE+1)*i;
            Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->EgyRatio.Value[i],Addr,AT24CXX_ADDR_ENERGY1_RATIO_SIZE);
            if(Ret != 0){
                if(FirstInitFlag == 1){
                    pData->EgyRatio.Value[i] = 1.0;
                    MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->EgyRatio.Value[i],Addr,AT24CXX_ADDR_ENERGY1_RATIO_SIZE);
                }
                //printf("%s %d Read Error(Ret=%ld) i=%u\n",__FUNCTION__,__LINE__,Ret,i);
            }
            WatchDogFeed();
        }
        
        //初始化ADC校准值
        Ret = MeterData_WaitReadSucc(UseBackup,(uint8_t *)&pData->AdcRatio,AT24CXX_ADDR_ADC_RATIO,AT24CXX_ADDR_ADC_RATIO_SIZE);
        if(Ret != 0){
            if(FirstInitFlag == 1){
                pData->AdcRatio.k = 1.0;
                pData->AdcRatio.b = 0;
                MeterData_WaitWriteSucc(UseBackup,(uint8_t *)&pData->AdcRatio,AT24CXX_ADDR_ADC_RATIO,AT24CXX_ADDR_ADC_RATIO_SIZE);
            }
            //printf("%s %d Read Error(Ret=%ld)\n",__FUNCTION__,__LINE__,Ret);
        }
        WatchDogFeed();
        
        if(FirstInitFlag != 1)
            GetFrequentUpdateData(pData);
     }

    //初始化其余无需存储到EEPROM中的数据
    pData->AnalogData.LoopCnt = DCMETER_LOOP_CNT_MAX;
    pData->AlarmData.LoopCnt = DCMETER_LOOP_CNT_MAX;
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++)
        pData->AlarmData.LoopAlarm[i].LoopNum = i+1;    
    for(i=0;i<3;i++){
        pData->Rn8209c[i].CaliParam.In = pData->Cs5460Dc.Imax;
        pData->Rn8209c[i].CaliParam.Un = pData->Cs5460Dc.Vmax;
    }
}

    
static uint32_t GetRealTimeALARM_Loop(uint8_t IsFirstLoop,uint8_t LoopNum,PDCMETER_DATA pData,uint8_t *pIsAlarm)
{
    uint8_t i;
    uint16_t u16Temp;
    
    PDCMETER_LOOP_ANALOG pLoopAng=NULL;
    PDCMETER_LOOP_ALARM pLoopAlarm=NULL;
    __packed float *pLoop50VCurr=NULL;
    
    
    if((LoopNum>DCMETER_LOOP_CNT_MAX)
        ||(LoopNum==0)||(pData==NULL)
        ||(pIsAlarm==NULL))
        return FUNC_FAILURE;
    i = LoopNum-1;
    pLoopAng = &pData->AnalogData.LoopAnalog[i];
    pLoopAlarm = &pData->AlarmData.LoopAlarm[i];
    pLoop50VCurr = &pData->AlarmParam.Voltage50VCurrent[i];
    
    u16Temp = pLoopAng->LoopCurrent*100;
    if((u16Temp>100)&&(pLoopAng->LoopCurrent>*pLoop50VCurr)){
        *pLoop50VCurr = pLoopAng->LoopCurrent;
    }
    if(IsFirstLoop){
        

        
        //判断是否是直流电压高告警
        if(pLoopAng->LoopVoltage >= pData->ModuleParam.DcHighVoltageThreshold){
            pLoopAlarm->LoopDCVoltageHighAlarm = DCMETER_ALARM_ALARM;
            *pIsAlarm |= DC_VOL_HIGH_ALARM_MASK;
        }
        //等电压下降到回差范围外，告警结束
        if(pLoopAng->LoopVoltage <= 
            (pData->ModuleParam.DcHighVoltageThreshold-DC_VOLTAGE_ALARM_BACKLASH)){
            pLoopAlarm->LoopDCVoltageHighAlarm = DCMETER_ALARM_NORMAL;
            *pIsAlarm &= (~DC_VOL_HIGH_ALARM_MASK);
        }
            
        //判断是否是直流电压低告警
        if(pLoopAng->LoopVoltage <= pData->ModuleParam.DcLowVoltageThreshold){
            
            pLoopAlarm->LoopDCVoltageLowAlarm = DCMETER_ALARM_ALARM;
            *pIsAlarm |= DC_VOL_LOW_ALARM_MASK;
        }
        //等电压上升到回差范围外，告警结束
        if((pLoopAng->LoopVoltage >= 
            (pData->ModuleParam.DcLowVoltageThreshold+DC_VOLTAGE_ALARM_BACKLASH))){
            pLoopAlarm->LoopDCVoltageLowAlarm = DCMETER_ALARM_NORMAL;
            *pIsAlarm &= (~DC_VOL_LOW_ALARM_MASK);
        }    
        /*
        //判断是否是直流一次下电告警
        if((pLoopAng->LoopVoltage <= pData->ModuleParam.DcFirstPowerDownThreshold)
            &&(pLoopAng->LoopCurrent <= *pLoop50VCurr)){
            pLoopAlarm->LoopFirstPowerDown = DCMETER_ALARM_ALARM;
            *pIsAlarm |= DC_FIRST_POWERDOWN_ALARM_MASK;
        }
        //等电压上升到回差范围外，告警结束
        if(pLoopAng->LoopVoltage >= 
            (pData->ModuleParam.DcFirstPowerDownThreshold+DC_VOLTAGE_ALARM_BACKLASH)){
            pLoopAlarm->LoopFirstPowerDown = DCMETER_ALARM_NORMAL;
            *pIsAlarm &= (~DC_FIRST_POWERDOWN_ALARM_MASK);
        }  */
            
        //判断是否是失压告警
        /*if((pLoopAng->LoopCurrent >1.0)&&(pLoopAng->LoopVoltage < 0.1)){
            pLoopAlarm->ModuleErrorAlarmTemp.LoopLostVoltageAlarm = DCMETER_ALARM_ALARM; 
            //*pIsAlarm |= 1;
        }
        else
            pLoopAlarm->ModuleErrorAlarmTemp.LoopLostVoltageAlarm = DCMETER_ALARM_NORMAL;*/
    }

    //判断是否是计量支路错误
    /*if((pLoopAng->LoopCurrent >1.0)&&(pLoopAng->LoopVoltage >1.0)
        &&(pLoopAng->LoopEnergy < 0.001)){
        pLoopAlarm->ModuleErrorAlarmTemp.LoopMeasureModuleAlarm = DCMETER_ALARM_ALARM;
        //*pIsAlarm |= 1;
    }
    else
        pLoopAlarm->ModuleErrorAlarmTemp.LoopMeasureModuleAlarm = DCMETER_ALARM_NORMAL;
    
    //判断是否是存储器故障
    if(AT45DBxxxD_OP_OK != AT45DBXXX_Check()){
        pLoopAlarm->ModuleErrorAlarmTemp.MemFailureAlarm = DCMETER_ALARM_ALARM;
        //*pIsAlarm |= 1;
    }
    else
        pLoopAlarm->ModuleErrorAlarmTemp.MemFailureAlarm = DCMETER_ALARM_NORMAL;

    //判断是否是时钟故障
    if(0 != PCF8563_Check()){
        pLoopAlarm->ModuleErrorAlarmTemp.ClockErrorAlarm = DCMETER_ALARM_ALARM;
        //*pIsAlarm |= 1;
    }
    else
        pLoopAlarm->ModuleErrorAlarmTemp.ClockErrorAlarm = DCMETER_ALARM_NORMAL;
    
    if(pLoopAlarm->ModuleErrorAlarmTemp.ClockErrorAlarm
        ||pLoopAlarm->ModuleErrorAlarmTemp.InterProgramErrorAlarm
        ||pLoopAlarm->ModuleErrorAlarmTemp.LoopLostVoltageAlarm
        ||pLoopAlarm->ModuleErrorAlarmTemp.LoopMeasureModuleAlarm
        ||pLoopAlarm->ModuleErrorAlarmTemp.MemFailureAlarm){
        pLoopAlarm->ModuleErrorAlarm = DCMETER_ALARM_ALARM; 
        *pIsAlarm |= MODULE_ERROR_ALARM_MASK;
    }else{
        pLoopAlarm->ModuleErrorAlarm = DCMETER_ALARM_NORMAL; 
        *pIsAlarm &= (~MODULE_ERROR_ALARM_MASK);
    }*/
    
    //暂不判断交流停电    

    return FUNC_FAILURE;
}

//pIsAlarm[0]:交流告警 pIsAlarm[1]-[6]:回路1-6告警字
extern uint32_t GetRealTimeAlarm(PDCMETER_DATA pData,uint8_t *pIsAlarm)
{
    volatile uint8_t i,ret,LoopNum;
    float ftemp=0.0;
    
    if((pData==NULL)||(pIsAlarm==NULL))
        return FUNC_FAILURE;

    if(((uint32_t)pData->ModuleParam.AcVoltageRatio)!=0){
        //判断是否是直流电压高告警
        if(pData->AnalogData.AcAnalog.AcVoltage < pData->ModuleParam.AcPowerCutThreshold){
            pData->AlarmData.AcVoltageAlarm = DCMETER_ALARM_ALARM;  
            pIsAlarm[0] |= AC_POWER_CUT_ALARM_MASK;
        }
        //等电压下降到回差范围外，告警结束
        if(pData->AnalogData.AcAnalog.AcVoltage >= 
            (pData->ModuleParam.AcPowerCutThreshold+AC_VOLTAGE_ALARM_BACKLASH)){
            pData->AlarmData.AcVoltageAlarm = DCMETER_ALARM_NORMAL;
            pIsAlarm[0] &= (~AC_POWER_CUT_ALARM_MASK);
        }
    }else{
        pData->AlarmData.AcVoltageAlarm = DCMETER_ALARM_NORMAL;
    }
    
    for(i=0;i<6;i++){
        LoopNum = i+1;
        if(i==0)
            ret += GetRealTimeALARM_Loop(1,LoopNum,pData,&pIsAlarm[LoopNum]); 
        else
            ret += GetRealTimeALARM_Loop(0,LoopNum,pData,&pIsAlarm[LoopNum]); 
    }  
    //等电压上升到回差范围外，告警结束
    if(pData->AnalogData.LoopAnalog[0].LoopVoltage >= 
        (pData->ModuleParam.DcFirstPowerDownThreshold+DC_VOLTAGE_ALARM_BACKLASH)){
        pData->AlarmData.LoopAlarm[0].LoopFirstPowerDown = DCMETER_ALARM_NORMAL;
        pIsAlarm[1] &= (~DC_FIRST_POWERDOWN_ALARM_MASK);
    }
    //判断是否是直流一次下电告警()
    for(i=0;i<6;i++){
        if(pData->AnalogData.LoopAnalog[i].LoopVoltage > pData->ModuleParam.DcFirstPowerDownThreshold)
            return ret;
        ftemp = (float)0.4*pData->AlarmParam.Voltage50VCurrent[i];
        //假如此时电流很小，则不判断此回路(记录电流最大值的阈值为1A，其40%为0.4A)
        if(pData->AnalogData.LoopAnalog[i].LoopCurrent>0.4){
            if(pData->AnalogData.LoopAnalog[i].LoopCurrent>ftemp)
                return ret;
        }
    }
    pData->AlarmData.LoopAlarm[0].LoopFirstPowerDown = DCMETER_ALARM_ALARM;
    pIsAlarm[1] |= DC_FIRST_POWERDOWN_ALARM_MASK;
      
    return ret;
}


//复位应用程序
extern void Sys_Soft_Reset(void) 
{    
    //SCB->AIRCR =0X05FA0000|(uint32_t)0x04;	
    SYS_UnlockReg();
    //SYS_ResetChip();
    SYS_ResetCPU();
    SYS_LockReg();
}
//复位从LDROM开始
extern void Sys_Hard_Reset(void) 
{    
    //SCB->AIRCR =0X05FA0000|(uint32_t)0x04;	
    SYS_UnlockReg();
    SYS_ResetChip();
    //SYS_ResetCPU();
    SYS_LockReg();
}
//恢复出厂设置
extern void MeterFactoryReset(void)
{
    if(0 == AT45DBXXX_Check()){
        AT45DB161_BulkErase();
        Sys_Soft_Reset();
    }
}


/*********************************历史数据***********************************************/
extern uint32_t MeterMonthEnergy_MakeRecord(PMonthEnergyData pData,uint8_t *pRecord,uint8_t *plen)
{
    uint8_t Offset=0,i;
    
    if((pData==NULL)||(pRecord==NULL)||(plen == NULL))
        return 1;
    if(*plen<SAVE_ENERGY_BYTES)
        return 1;
    GetWord(&pRecord[Offset]) = SWAP_WORD(pData->Date.Year);
    Offset += 2;
    pRecord[Offset] = pData->Date.Month;
    Offset += 1;
    pRecord[Offset] = pData->Date.Day;
    Offset += 1;
    pRecord[Offset] = pData->Date.Hour;
    Offset += 1;
    pRecord[Offset] = pData->Date.Minute;
    Offset += 1;
    pRecord[Offset] = pData->Date.Second;
    Offset += 1;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->MonEgy.CMCCEnergy);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->MonEgy.CMCCRatio);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->MonEgy.CUCCEnergy);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->MonEgy.CUCCRatio);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->MonEgy.CTCCEnergy);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->MonEgy.CTCCRatio);
    Offset += 4;
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->MonEgy.LoopEnergy[i]);
        Offset += 4;
    }
    
    pData->CheckSum = GetSum(pRecord,Offset);
    pRecord[Offset] = pData->CheckSum;
    Offset += 1;
    
    
    *plen = Offset;
    return 0;
}
extern void MeterGetUserTotalEnergy(PDCMETER_PARAM pParam,DCMETER_USER_CONFIG User,PDCMETER_TEMP_ENERGY pTemp,float *pEnergy)
{
    uint8_t i;
    float fTemp=0.0;
    
    if((pParam==NULL)||(pEnergy==NULL)||(pTemp==NULL))
        return;
    
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        if(pParam->LoopConfig[i] == User)
           fTemp += pTemp->LoopEnergy[i]; 
    }
    *pEnergy = fTemp;
}


//获取指定保存计数的月冻结电能记录
extern uint32_t MeterMonthEnergy_GetRecord(uint32_t AssignCnt,PMonthEnergyData pMonEgyData)
{
    //uint8_t Temp[200];
    volatile uint32_t ReadAddr;
    
    if((pMonEgyData==NULL)||(AssignCnt==0))
        return 1;
    
    
    ReadAddr = AssignCnt%METER_SAVE_ENERGY_MONTH_CNT_MAX;
    ReadAddr -= 1;
    ReadAddr *= SAVE_ENERGY_BYTES;
    ReadAddr += SAVE_ENERGY_FLASH_START_ADDR;

    if(0 == MeterData_WaitReadSucc(1,Temp,ReadAddr,SAVE_ENERGY_BYTES-1)){
        memcpy(pMonEgyData,Temp,SAVE_ENERGY_BYTES);
        return  0;
    }
    return 1;     
}

#define HISTORY_DATA_FACTOR     100
#define HISTORY_DATA_FACTOR_SQUARE     10000
//功能：存储月冻结电能数据，环形存储  参数：pSaveCnt 保存当前存储计数 pData 电表数据
extern uint32_t MeterMonthEnergy_SaveRecord(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData)
{
    uint8_t Len;//Temp[200]={0};
    MonthEnergyData MonEgyData={0};
    //PMonthEnergyData LastMonEgyData=NULL;
    uint8_t i=0;
    uint32_t Addr;
    
    float NowEnergy=0.0,EnergyDiff=0.0;//LastEnergy=0.0,
    float AllEnergy=0.0;
    
    if((pSaveCnt==NULL)||(pData==NULL))
        return 1;
    memcpy((uint8_t *)&MonEgyData.Date,(uint8_t *)&pData->Date,sizeof(DCMETER_DATE));
    //计算月总电能，pData->TempEnergy.AllEnergy中缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_ALL,&pData->TempEnergy,&NowEnergy);
    AllEnergy = NowEnergy - pData->TempEnergy.AllEnergy;
    
    pData->TempEnergy.AllEnergy = NowEnergy;

    //计算CMCC用户月总电能，pData->TempEnergy.CMCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CMCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CMCCEnergy;
    pData->TempEnergy.CMCCEnergy = NowEnergy;
    MonEgyData.MonEgy.CMCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
         MonEgyData.MonEgy.CMCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    //计算CUCC用户月总电能，pData->TempEnergy.CUCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CUCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CUCCEnergy;
    pData->TempEnergy.CUCCEnergy = NowEnergy;
    MonEgyData.MonEgy.CUCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        MonEgyData.MonEgy.CUCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    //计算CTCC用户月总电能，pData->TempEnergy.CTCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CTCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CTCCEnergy;
    pData->TempEnergy.CTCCEnergy = NowEnergy;
    MonEgyData.MonEgy.CTCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        MonEgyData.MonEgy.CTCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    //保存用户总电能数据
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        MonEgyData.MonEgy.LoopEnergy[i] = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopEnergy*HISTORY_DATA_FACTOR);
    }
    
    
    Len = sizeof(Temp);
    if(0 != MeterMonthEnergy_MakeRecord(&MonEgyData,Temp,&Len))
        return 1;
  
    Addr = SAVE_ENERGY_FLASH_START_ADDR;
    Addr += (*pSaveCnt%METER_SAVE_ENERGY_MONTH_CNT_MAX)*SAVE_ENERGY_BYTES;
    
    if(0 != MeterData_WaitWriteSucc(1,Temp,Addr,Len-1)){
        return 1;
    }
   
    
    *pSaveCnt += 1;
    
    if(0== MeterData_WaitWriteSucc(1,(uint8_t *)pSaveCnt,AT24CXX_ADDR_ENERGY_CNT,AT24CXX_ADDR_ENERGY_CNT_SIZE))
        return 0;
    else
        *pSaveCnt -= 1;
    return 1;
}



extern uint32_t MeterMinuteData_MakeRecord(PSaveHistoryData pData,uint8_t *pRecord,uint8_t *plen)
{
    uint8_t Offset=0,i;
    
    if((pData==NULL)||(pRecord==NULL)||(plen == NULL))
        return 1;
    if(*plen<SAVE_DATA_MINUTE_BYTES)
        return 1;
    
    GetWord(&pRecord[Offset]) = SWAP_WORD(pData->Date.Year);
    Offset += 2;
    pRecord[Offset] = pData->Date.Month;
    Offset += 1;
    pRecord[Offset] = pData->Date.Day;
    Offset += 1;
    pRecord[Offset] = pData->Date.Hour;
    Offset += 1;
    pRecord[Offset] = pData->Date.Minute;
    Offset += 1;
    pRecord[Offset] = pData->Date.Second;
    Offset += 1;
    
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->Loop[i].LoopVoltage);
        Offset += 4;
        GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->Loop[i].LoopCurrent);
        Offset += 4;
        GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->Loop[i].LoopPower);
        Offset += 4;
        GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->Loop[i].LoopEnergy);
        Offset += 4;
    }
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->CMCCEnergy);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->CMCCRatio);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->CUCCEnergy);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->CUCCRatio);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->CTCCEnergy);
    Offset += 4;
    GetDWord(&pRecord[Offset]) = SWAP_DWORD(pData->CTCCRatio);
    Offset += 4;
    
    pData->CheckSum = GetSum(pRecord,Offset);
    pRecord[Offset] = pData->CheckSum;
    Offset += 1;
    
    *plen = Offset;
    return 0;
}

//获取每分钟的历史数据
extern uint32_t MeterMinuteData_GetRecord(uint32_t AssignCnt,PSaveHistoryData pMinData)
{
    //uint8_t Temp[200];
    uint32_t ReadAddr;
    
    if((pMinData==NULL)||(AssignCnt==0))
        return 1;
    
    ReadAddr = SAVE_DATA_MINUTE_FLASH_START_ADDR;
    ReadAddr += (AssignCnt%METER_SAVE_DATA_MINUTE_CNT_MAX-1)*SAVE_DATA_MINUTE_BYTES;
    

    if(0==MeterData_WaitReadSucc(1,Temp,ReadAddr,SAVE_DATA_MINUTE_BYTES-1)){
        memcpy(pMinData,Temp,SAVE_DATA_MINUTE_BYTES-1);
        return  0; 
    }
    return 1;   
}
//功能：存储每秒历史数据 参数：pSaveCnt 保存当前存储计数 pData 电表数据
extern uint32_t MeterMinuteData_SaveRecord(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData)
{
    uint8_t Len;//Temp[200]={0};
    SaveHistoryData MinData={0};
    //PMonthEnergyData LastMonEgyData=NULL;
    uint8_t i=0;
    uint32_t Addr;
    
    float NowEnergy=0.0,EnergyDiff=0.0;//LastEnergy=0.0,
    float AllEnergy=0.0;
    
    if((pSaveCnt == NULL)||(pData == NULL))
        return 1;
    memcpy((uint8_t *)&MinData.Date,(uint8_t *)&pData->Date,sizeof(DCMETER_DATE));
    //计算当月总电能，pData->TempEnergy.AllEnergy中缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_ALL,&pData->TempEnergy,&NowEnergy);
    AllEnergy = NowEnergy - pData->TempEnergy.AllEnergy;
    //pData->TempEnergy.AllEnergy = NowEnergy;
    
    //计算CMCC用户月总电能，pData->TempEnergy.CMCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CMCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CMCCEnergy;

    MinData.CMCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        MinData.CMCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    //计算CUCC用户月总电能，pData->TempEnergy.CUCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CUCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CUCCEnergy;
    
    MinData.CUCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        MinData.CUCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    //计算CTCC用户月总电能，pData->TempEnergy.CTCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CTCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CTCCEnergy;
    
    MinData.CTCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        MinData.CTCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        MinData.Loop[i].LoopVoltage = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopVoltage*HISTORY_DATA_FACTOR);
        MinData.Loop[i].LoopCurrent = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopCurrent*HISTORY_DATA_FACTOR);
        MinData.Loop[i].LoopPower = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopPower*HISTORY_DATA_FACTOR);
        MinData.Loop[i].LoopEnergy = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopEnergy*HISTORY_DATA_FACTOR);
    }
    
    
    Len = sizeof(Temp);
    if(0 != MeterMinuteData_MakeRecord(&MinData,Temp,&Len))
        return 1;
  
    Addr = SAVE_DATA_MINUTE_FLASH_START_ADDR;
    Addr += (*pSaveCnt%METER_SAVE_DATA_MINUTE_CNT_MAX)*SAVE_DATA_MINUTE_BYTES;
    
    if(0 != MeterData_WaitWriteSucc(1,Temp,Addr,Len-1)){
        return 1;
    }
    
    *pSaveCnt += 1;
    return 0;
    
    //return MeterData_WaitWriteSucc((uint8_t *)pSaveCnt,AT24CXX_ADDR_MINUTE_DATA_CNT,AT24CXX_ADDR_MINUTE_DATA_CNT_SIZE);
}

//获取每小时的历史数据
extern uint32_t MeterHourData_GetRecord(uint32_t AssignCnt,PSaveHistoryData pHourData)
{
    //uint8_t Temp[200];
    uint32_t ReadAddr;
    
    if((pHourData==NULL)||(AssignCnt==0))
        return 1;
    
    ReadAddr = SAVE_DATA_HOUR_FLASH_START_ADDR;
    ReadAddr += (AssignCnt%METER_SAVE_DATA_HOUR_CNT_MAX-1)*SAVE_DATA_HOUR_BYTES;

    if(0==MeterData_WaitReadSucc(1,Temp,ReadAddr,SAVE_DATA_HOUR_BYTES-1)){
        memcpy(pHourData,Temp,SAVE_DATA_HOUR_BYTES-1);
        return  0; 
    }
    return 1;   
}

//功能：存储每小时历史数据 参数：pSaveCnt 保存当前存储计数 pData 电表数据
extern uint32_t MeterHourData_SaveRecord(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData)
{
    uint8_t Len;//Temp[200]={0};
    SaveHistoryData HourData={0};
    //PMonthEnergyData LastMonEgyData=NULL;
    uint8_t i=0;
    uint32_t Addr;
    
    float NowEnergy=0.0,EnergyDiff=0.0;//LastEnergy=0.0,
    float AllEnergy=0.0;
    
    if((pSaveCnt == NULL)||(pData == NULL))
        return 1;
    
    memcpy((uint8_t *)&HourData.Date,(uint8_t *)&pData->Date,sizeof(DCMETER_DATE));
    //计算当月总电能，pData->TempEnergy.AllEnergy中缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_ALL,&pData->TempEnergy,&NowEnergy);
    AllEnergy = NowEnergy - pData->TempEnergy.AllEnergy;

    //pData->TempEnergy.AllEnergy = NowEnergy;
    
    
    //计算CMCC用户月总电能，pData->TempEnergy.CMCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CMCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CMCCEnergy;

    HourData.CMCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        HourData.CMCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    //计算CUCC用户月总电能，pData->TempEnergy.CUCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CUCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CUCCEnergy;
    
    HourData.CUCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        HourData.CUCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    //计算CTCC用户月总电能，pData->TempEnergy.CTCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CTCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->TempEnergy.CTCCEnergy;
    
    HourData.CTCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        HourData.CTCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        HourData.Loop[i].LoopVoltage = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopVoltage*HISTORY_DATA_FACTOR);
        HourData.Loop[i].LoopCurrent = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopCurrent*HISTORY_DATA_FACTOR);
        HourData.Loop[i].LoopPower = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopPower*HISTORY_DATA_FACTOR);
        HourData.Loop[i].LoopEnergy = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopEnergy*HISTORY_DATA_FACTOR);
    }
    
    
    Len = sizeof(Temp);
    if(0 != MeterMinuteData_MakeRecord(&HourData,Temp,&Len))
        return 1;
  
    Addr = SAVE_DATA_HOUR_FLASH_START_ADDR;
    Addr += (*pSaveCnt%METER_SAVE_DATA_HOUR_CNT_MAX)*SAVE_DATA_HOUR_BYTES;
    
    if(0 != MeterData_WaitWriteSucc(1,Temp,Addr,Len-1)){
        return 1;
    }
    
    *pSaveCnt += 1;
    //return 0;
    if(0==MeterData_WaitWriteSucc(1,(uint8_t *)pSaveCnt,AT24CXX_ADDR_HOUR_DATA_CNT,AT24CXX_ADDR_HOUR_DATA_CNT_SIZE))
        return 0;
    else
        *pSaveCnt -= 1;
    return 1;
}

//获取每月的历史数据
extern uint32_t MeterMonthData_GetRecord(uint32_t AssignCnt,PSaveHistoryData pMonthData)
{
    //uint8_t Temp[200];
    uint32_t ReadAddr;
    
    if((pMonthData==NULL)||(AssignCnt==0))
        return 1;
    
    ReadAddr = SAVE_DATA_MONTH_FLASH_START_ADDR;
    ReadAddr += (AssignCnt%METER_SAVE_DATA_MONTH_CNT_MAX-1)*SAVE_DATA_MONTH_BYTES;

    if(0==MeterData_WaitReadSucc(1,Temp,ReadAddr,SAVE_DATA_MONTH_BYTES-1)){
        memcpy(pMonthData,Temp,SAVE_DATA_MONTH_BYTES-1);
        return  0; 
    }
    return 1;   
}

//功能：存储每月历史数据 参数：pSaveCnt 保存当前存储计数 pData 电表数据
extern uint32_t MeterMonthData_SaveRecord(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData)
{
    uint8_t Len;//Temp[200]={0};
    SaveHistoryData MonthData={0};
    //PMonthEnergyData LastMonEgyData=NULL;
    uint8_t i=0;
    uint32_t Addr;

    float NowEnergy=0.0,EnergyDiff=0.0;//LastEnergy=0.0,
    float AllEnergy=0.0;
    
    if((pSaveCnt == NULL)||(pData == NULL))
        return 1;
    
    memcpy((uint8_t *)&MonthData.Date,(uint8_t *)&pData->Date,sizeof(DCMETER_DATE));
    //计算当月总电能，pData->TempEnergy.AllEnergy中缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_ALL,&pData->TempEnergy,&NowEnergy);
    //AllEnergy = NowEnergy - pData->TempEnergy.AllEnergy;
    AllEnergy = NowEnergy - pData->EgyReading.AllEnergy;
    //pData->TempEnergy.AllEnergy = NowEnergy;
    pData->EgyReading.AllEnergy = NowEnergy;
    
    //计算CMCC用户月总电能，pData->TempEnergy.CMCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CMCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->EgyReading.CMCCEnergy;
    pData->EgyReading.CMCCEnergy = NowEnergy;
    MonthData.CMCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        MonthData.CMCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    //计算CUCC用户月总电能，pData->TempEnergy.CUCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CUCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->EgyReading.CUCCEnergy;
    pData->EgyReading.CUCCEnergy = NowEnergy;
    MonthData.CUCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        MonthData.CUCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    //计算CTCC用户月总电能，pData->TempEnergy.CTCCEnergy缓存上月总电能读数（出厂以来历史总电能）
    MeterGetUserTotalEnergy(&pData->Param,USER_CTCC,&pData->TempEnergy,&NowEnergy);
    EnergyDiff = NowEnergy - pData->EgyReading.CTCCEnergy;
    pData->EgyReading.CTCCEnergy = NowEnergy;
    MonthData.CTCCEnergy = (uint32_t)(EnergyDiff*HISTORY_DATA_FACTOR);
    if(AllEnergy!=0.0)
        MonthData.CTCCRatio = (uint32_t)(EnergyDiff/AllEnergy*HISTORY_DATA_FACTOR_SQUARE);
    
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        MonthData.Loop[i].LoopVoltage = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopVoltage*HISTORY_DATA_FACTOR);
        MonthData.Loop[i].LoopCurrent = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopCurrent*HISTORY_DATA_FACTOR);
        MonthData.Loop[i].LoopPower = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopPower*HISTORY_DATA_FACTOR);
        MonthData.Loop[i].LoopEnergy = (uint32_t)(pData->AnalogData.LoopAnalog[i].LoopEnergy*HISTORY_DATA_FACTOR);
    }
    
    
    Len = sizeof(Temp);
    if(0 != MeterMinuteData_MakeRecord(&MonthData,Temp,&Len))
        return 1;
  
    Addr = SAVE_DATA_MONTH_FLASH_START_ADDR;
    Addr += (*pSaveCnt%METER_SAVE_DATA_MONTH_CNT_MAX)*SAVE_DATA_MONTH_BYTES;
    
    if(0 != MeterData_WaitWriteSucc(1,Temp,Addr,Len-1)){
        return 1;
    }
    
    *pSaveCnt += 1;

    if(0== MeterData_WaitWriteSucc(1,(uint8_t *)pSaveCnt,AT24CXX_ADDR_MONTH_DATA_CNT,AT24CXX_ADDR_MONTH_DATA_CNT_SIZE))
        return 0;
    else 
        *pSaveCnt -= 1;
    return 1;
}


//获取每条告警记录
extern uint32_t MeterAlarm_MakeRecord(PSaveHistoryAlarm pAlarm,uint8_t *pRecord,uint8_t *plen)
{
    uint8_t Offset=0;
    
    if((pAlarm==NULL)||(pRecord==NULL)||(plen == NULL))
        return 1;
    if(*plen<SAVE_ALARM_BYTES)
        return 1;
    
    GetWord(&pRecord[Offset]) = SWAP_WORD(pAlarm->Date.Year);
    Offset += 2;
    pRecord[Offset] = pAlarm->Date.Month;
    Offset += 1;
    pRecord[Offset] = pAlarm->Date.Day;
    Offset += 1;
    pRecord[Offset] = pAlarm->Date.Hour;
    Offset += 1;
    pRecord[Offset] = pAlarm->Date.Minute;
    Offset += 1;
    pRecord[Offset] = pAlarm->Date.Second;
    Offset += 1;
    
    
    pRecord[Offset] = pAlarm->Alarm.HistoryAlarmType;
    Offset += 1;
    pRecord[Offset] = pAlarm->Alarm.AlarmSource;
    Offset += 1;
    pRecord[Offset] = pAlarm->Alarm.AlarmWord;
    Offset += 1;
    
    
    pAlarm->CheckSum = GetSum(pRecord,Offset);
    pRecord[Offset] = pAlarm->CheckSum;
    Offset += 1;
    
    *plen = Offset;
    return 0;
}

extern uint32_t MeterAlarm_GetRecord(uint32_t AssignCnt,PSaveHistoryAlarm pAlarm)
{
    
    uint32_t ReadAddr;
    
    if((pAlarm==NULL)||(AssignCnt==0))
        return 1;
    
    ReadAddr = SAVE_ALARM_FLASH_START_ADDR;
    ReadAddr += (AssignCnt%METER_SAVE_ALARM_CNT_MAX-1)*SAVE_ALARM_BYTES;
   
    if(0==MeterData_WaitReadSucc(1,Temp,ReadAddr,SAVE_ALARM_BYTES-1)){
        memcpy(pAlarm,Temp,SAVE_ALARM_BYTES-1);
        return  0; 
    }
    return 1;   
}


//功能：存储历史告警数据 参数：pSaveCnt 保存当前存储计数 pData 电表告警数据
extern uint32_t MeterAlarm_SaveRecord(__packed uint32_t *pSaveCnt,PSaveHistoryAlarm pHtyAlarm)
{
    uint32_t AddrTemp;
    uint8_t Len;
    
    if((pSaveCnt == NULL)||(pHtyAlarm == NULL))
        return 1;
   
    AddrTemp = (*pSaveCnt);
    AddrTemp %= METER_SAVE_ALARM_CNT_MAX;
    AddrTemp *= SAVE_ALARM_BYTES;
    AddrTemp += SAVE_ALARM_FLASH_START_ADDR;
    //Addr = SAVE_ALARM_FLASH_START_ADDR;
    //Addr += (*pSaveCnt%METER_SAVE_ALARM_CNT_MAX)*SAVE_ALARM_BYTES; 
    
    Len = sizeof(Temp);
    if(0 != MeterAlarm_MakeRecord(pHtyAlarm,Temp,&Len))
        return 1; 
    
    if(0 != MeterData_WaitWriteSucc(1,Temp,AddrTemp,Len-1)){
        return 1;
    }
    
    *pSaveCnt = *pSaveCnt+1;
    //return 0;
    if(0==MeterData_WaitWriteSucc(1,(uint8_t *)pSaveCnt,AT24CXX_ADDR_ALARM_CNT,AT24CXX_ADDR_ALARM_CNT_SIZE))
        return 0;
    else
       *pSaveCnt -= 1; 
    return 1;
}


extern uint32_t MeterAlarm_SaveRecordJudge(__packed uint32_t *pSaveCnt,PDCMETER_DATA pData) 
{
  
    volatile uint8_t i=0,Len; 
    SaveHistoryAlarm AlarmData={0};
    volatile PDCMETER_ALARM pAlarm = &pData->AlarmData;
    volatile static uint8_t AlarmSaveFlag[DCMETER_LOOP_CNT_MAX]={0};
    volatile static uint8_t ACAlarmSaveFlag=0;
    
    if((pSaveCnt == NULL)||(pData == NULL))
        return 1;
   
    memcpy((uint8_t *)&AlarmData.Date,(uint8_t *)&pData->Date,sizeof(DCMETER_DATE));
    
    //
    if(pAlarm->AcVoltageAlarm == DCMETER_ALARM_ALARM){
        if((ACAlarmSaveFlag&AC_POWER_CUT_ALARM_MASK)!=AC_POWER_CUT_ALARM_MASK){//同一告警只保存一次
            
            AlarmData.Alarm.AlarmSource = 0;
            AlarmData.Alarm.HistoryAlarmType = DCMETER_HISTORY_ALARM_TYPE_AC_POWER_CUT;
            AlarmData.Alarm.AlarmWord = DCMETER_ALARM_ALARM; 
            if(0!=MeterAlarm_SaveRecord(pSaveCnt,&AlarmData))
                return 1;
            ACAlarmSaveFlag |= (uint8_t)AC_POWER_CUT_ALARM_MASK;
        }
    }else{
        ACAlarmSaveFlag &= (uint8_t)(~AC_POWER_CUT_ALARM_MASK);
    }
    //printf("AlarmSaveFlag[0]:%d\n",AlarmSaveFlag[0]);
    for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
        if(pAlarm->LoopAlarm[i].LoopDCVoltageHighAlarm == DCMETER_ALARM_ALARM){
            if((AlarmSaveFlag[i]&DC_VOL_HIGH_ALARM_MASK)!=DC_VOL_HIGH_ALARM_MASK){//同一告警只保存一次
                AlarmData.Alarm.AlarmSource = pAlarm->LoopAlarm[i].LoopNum;
                AlarmData.Alarm.HistoryAlarmType = DCMETER_HISTORY_ALARM_TYPE_DCVOLTAGE_HIGH;
                AlarmData.Alarm.AlarmWord = DCMETER_ALARM_ALARM; 
                if(0!=MeterAlarm_SaveRecord(pSaveCnt,&AlarmData))
                    return 1;
                //printf("Save Alarm %d\n",__LINE__);
                AlarmSaveFlag[i] |= (uint8_t)DC_VOL_HIGH_ALARM_MASK;
            }
        }else{
            AlarmSaveFlag[i] &= (uint8_t)(~DC_VOL_HIGH_ALARM_MASK);
        }
        
        if(pAlarm->LoopAlarm[i].LoopDCVoltageLowAlarm == DCMETER_ALARM_ALARM){
            //printf("!!LoopDCVoltageLowAlarm= DCMETER_ALARM_ALARM\n");
            if((AlarmSaveFlag[i]&DC_VOL_LOW_ALARM_MASK)!=DC_VOL_LOW_ALARM_MASK){//同一告警只保存一次
                AlarmData.Alarm.AlarmSource = pAlarm->LoopAlarm[i].LoopNum;
                AlarmData.Alarm.HistoryAlarmType = DCMETER_HISTORY_ALARM_TYPE_DCVOLTAGE_LOW;
                AlarmData.Alarm.AlarmWord = DCMETER_ALARM_ALARM; 
                if(0!=MeterAlarm_SaveRecord(pSaveCnt,&AlarmData))
                    return 1;
                //printf("Save Alarm %d\n",__LINE__);
                AlarmSaveFlag[i] |= (uint8_t)DC_VOL_LOW_ALARM_MASK;
            }
        }else{
            AlarmSaveFlag[i] &= (uint8_t)(~DC_VOL_LOW_ALARM_MASK);
            //printf("LoopDCVoltageLowAlarm= DCMETER_ALARM_NORMAL\n");
        }
        
        if(pAlarm->LoopAlarm[i].LoopFirstPowerDown == DCMETER_ALARM_ALARM){
            if((AlarmSaveFlag[i]&DC_FIRST_POWERDOWN_ALARM_MASK)!=DC_FIRST_POWERDOWN_ALARM_MASK){//同一告警只保存一次
                
                AlarmData.Alarm.AlarmSource = pAlarm->LoopAlarm[i].LoopNum;
                AlarmData.Alarm.HistoryAlarmType = DCMETER_HISTORY_ALARM_TYPE_FIRST_POWER_DOWN;
                AlarmData.Alarm.AlarmWord = DCMETER_ALARM_ALARM; 
                if(0!=MeterAlarm_SaveRecord(pSaveCnt,&AlarmData))
                    return 1;
                //printf("Save Alarm %d\n",__LINE__);
                AlarmSaveFlag[i] |= (uint8_t)DC_FIRST_POWERDOWN_ALARM_MASK;
            }
        }else{
            AlarmSaveFlag[i] &= (uint8_t)(~DC_FIRST_POWERDOWN_ALARM_MASK);
        }
        /*
        if(pAlarm->LoopAlarm[i].ModuleErrorAlarm == DCMETER_ALARM_ALARM){
            if((AlarmSaveFlag[i]&MODULE_ERROR_ALARM_MASK)!=MODULE_ERROR_ALARM_MASK){//同一告警只保存一次
                
                AlarmData.Alarm.AlarmSource = pAlarm->LoopAlarm[i].LoopNum;
                AlarmData.Alarm.HistoryAlarmType = DCMETER_HISTORY_ALARM_TYPE_MEASURE_MODULE;
                AlarmData.Alarm.AlarmWord = DCMETER_ALARM_ALARM; 
                if(0!=MeterAlarm_SaveRecord(pSaveCnt,&AlarmData))
                    return 1;
                //printf("Save Alarm %d\n",__LINE__);
                AlarmSaveFlag[i] |= (uint8_t)MODULE_ERROR_ALARM_MASK;
            }
        }else{
            AlarmSaveFlag[i] &= (uint8_t)(~MODULE_ERROR_ALARM_MASK);
        }*/
    }
    //printf("AlarmSaveFlag[0]:%d\n",AlarmSaveFlag[0]);
    return 0;
}

//根据标志位判断哪些记录是否需要保存
extern void MeterSaveHistoryData(__packed uint32_t *pSaveFlag,const PDCMETER_DATA pData)
{ 
    uint32_t SaveFlag = *pSaveFlag;

    if(pData==NULL)
        return;
     
    if((pData->Date.Day == pData->Reading.Day)
        &&(pData->Date.Hour == pData->Reading.Hour)
        &&(pData->Date.Minute == 0)){
        if((SaveFlag&SAVE_FLAG_MONTH_DATA_MASK)!=SAVE_FLAG_MONTH_DATA_MASK){
            if(0==MeterMonthData_SaveRecord(&pData->SaveCnt.MonthDataCnt,pData))
                SaveFlag |= SAVE_FLAG_MONTH_DATA_MASK; 
        }  
    }
    //if(pData->Date.Second == 0){
        if((SaveFlag&SAVE_FLAG_MINUTE_DATA_MASK)!=SAVE_FLAG_MINUTE_DATA_MASK){
            if(0==MeterMinuteData_SaveRecord(&pData->SaveCnt.MinuteDataCnt,pData))
                SaveFlag |= SAVE_FLAG_MINUTE_DATA_MASK;
        }  
   // }
    
    if(pData->Date.Minute==0){
        if((SaveFlag&SAVE_FLAG_HOUR_DATA_MASK)!=SAVE_FLAG_HOUR_DATA_MASK){
            //printf("Minute:%u Sec:%u SAVE_HOUR_DATA Cnt:%lu Flag:%lu\n",pData->Date.Minute,pData->Date.Second,pData->SaveCnt.HourDataCnt,SaveFlag);
            if(0==MeterHourData_SaveRecord(&pData->SaveCnt.HourDataCnt,pData))
                SaveFlag |= SAVE_FLAG_HOUR_DATA_MASK;
            //printf("SAVE_HOUR_DATA Cnt:%lu Flag:%lu\n",pData->SaveCnt.HourDataCnt,SaveFlag);
        }
    }
    
    if((pData->Date.Day == 1)&&(pData->Date.Hour == 0)&&(pData->Date.Minute==0)){
        if((SaveFlag&SAVE_FLAG_MONTH_ENERGY_MASK)!=SAVE_FLAG_MONTH_ENERGY_MASK){
            if(0==MeterMonthEnergy_SaveRecord(&pData->SaveCnt.EnergyMonthCnt,pData))
                SaveFlag |= SAVE_FLAG_MONTH_ENERGY_MASK;
        }    
    }
    *pSaveFlag = SaveFlag;
}

//校指定回路霍尔传感器 CaliSrc 校准源*SECTIONS_CALIBRATION_RATIO
//LoopNum:0x1-0x6 校准某个通道
static uint32_t CalibrationHallSensor_Loop(uint8_t LoopNum,uint32_t CaliSrc,PSECTION_CALIBRATION pSectCali,PDCMETER_DATA pData)
{
    uint8_t CaliCnt,LoopIndex;
    uint32_t TimeOut=0;
    uint32_t CaliValue=0;
    float Irms,fzero;
    
    if((pSectCali==NULL)||(pData==NULL))
        return 1;
    if((LoopNum<1)||(LoopNum>6))
        return 1;
    
    LoopIndex = LoopNum-1;
    
    CaliCnt = (uint8_t)pSectCali->CaliCnt[LoopIndex];
       
    //等待实测电流值
    //Imax = pData->CTValue.CT[LoopIndex]*pData->Cs5460Dc.Imax;
    RN8209C_Select((LoopNum+1)/2);
    Comm_DelayUs(10);
    TimeOut=0;
    do{
        TimeOut++;
    }
    while((TimeOut<100000)&&(0==RN8209C_TestDataUpdate()));
    
    //超时返回错误
    if(TimeOut==100000)
        return 2;
    //CS5460A_Clear_DRDY();
    
    if(LoopNum%2){
        if(0!=RN8209C_GetIa(&pData->Rn8209c[(LoopNum+1)/2-1].CaliParam,&Irms))
            return 2;
    }else{
        if(0!=RN8209C_GetIb(&pData->Rn8209c[(LoopNum+1)/2-1].CaliParam,&Irms))
            return 2;
    }
    Irms *= pData->CTValue.CT[LoopIndex];
    if(CaliSrc!=0){
        //去除零点漂移
        fzero = (float)((signed short)pSectCali->SectCaliZero[LoopIndex]);
        fzero /= 100.0;
        Irms -= fzero; 
    }
    
    RN8209C_Select(0);//拉高所有片选
    Comm_DelayUs(10);
    if(CaliSrc!=0){   
        CaliValue = Irms*SECTIONS_CALIBRATION_RATIO;
        CaliValue = CaliSrc*SECTIONS_CALIBRATION_RATIO/CaliValue;
        //只校正倍数在0.5到1.5内的数据
        if((CaliValue>=500)&&(CaliValue<=1500)){
            pSectCali->SectCaliDiv[CaliCnt].CaliValue[LoopIndex] = (uint16_t)CaliValue;
            pSectCali->SectRange[CaliCnt].LoopRange[LoopIndex] = (uint32_t)(Irms*SECTIONS_CALIBRATION_RATIO);//以测量值作为段上限
        }   
        else
           pSectCali->SectCaliDiv[CaliCnt].CaliValue[LoopIndex] = SECTIONS_CALIBRATION_RATIO;
    }else{
        Irms *= 100.0;
        pSectCali->SectCaliZero[LoopIndex] = (signed short)Irms;
    }
    return 0;
}
//校准回路霍尔传感器 CaliSrc 校准源*SECTIONS_CALIBRATION_RATIO
//LoopNum:0xff-校准所有通道 0x1-0x6 校准某个通道
extern uint32_t CalibrationHallSensor(uint8_t LoopNum,uint32_t CaliSrc,PSECTION_CALIBRATION pSectCali,PDCMETER_DATA pData)
{ 
    
    uint8_t i=0;
    uint32_t ret;
    
    if((pSectCali==NULL)||(pData==NULL))
        return 1;
    if((LoopNum!=0xff)&&((LoopNum<1)||(LoopNum>DCMETER_LOOP_CNT_MAX)))
        return 1;
    
    
    
    if(LoopNum==0xff){
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
            if(pSectCali->CaliCnt[i] >= SECTIONS_MAX)
                pSectCali->CaliCnt[i] = 0;
            ret = CalibrationHallSensor_Loop(i+1,CaliSrc,pSectCali,pData);
            if((ret == 0)&&(CaliSrc!=0))
                pSectCali->CaliCnt[i] += 1;
            else if(ret != 0)
                return 1;
                
        }
    }else{
        i = LoopNum-1;
        if(pSectCali->CaliCnt[i] >= SECTIONS_MAX)
            pSectCali->CaliCnt[i] = 0;
        ret  = CalibrationHallSensor_Loop(i+1,CaliSrc,pSectCali,pData);
        if((0 == ret)&&(CaliSrc!=0))
            pSectCali->CaliCnt[i] += 1;
        else if(ret != 0)
            return 1;
    }
    
    if(CaliSrc!=0){  
        if(0!=MeterData_WaitWriteSucc(1,(uint8_t *)&pSectCali->SectCaliDiv[0],AT24CXX_ADDR_SECTION,AT24CXX_ADDR_SECTION_SIZE))
            return 1;
        
        if(0!=MeterData_WaitWriteSucc(1,(uint8_t *)&pSectCali->SectRange[0],AT24CXX_ADDR_SECTION_RANGE,AT24CXX_ADDR_SECTION_RANGE_SIZE))
            return 1;
        
        if(0!=MeterData_WaitWriteSucc(1,(uint8_t *)&pSectCali->CaliCnt,AT24CXX_ADDR_SECTION_RANGE_CNT,AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE))
            return 1;
    }else{
        if(0!=MeterData_WaitWriteSucc(1,(uint8_t *)&pSectCali->SectCaliZero[0],AT24CXX_ADDR_SECTION_ZERO,AT24CXX_ADDR_SECTION_ZERO_SIZE))
            return 1;
    }
    return 0;
}
//撤销上次校准
extern uint8_t CalibrationHallSensor_Repeal(uint8_t LoopNum,PSECTION_CALIBRATION pSectCali)
{
    uint8_t i;
    
    if(pSectCali==NULL)
        return 1;
    
    if((LoopNum!=0xff)&&((LoopNum<1)||(LoopNum>DCMETER_LOOP_CNT_MAX)))
        return 1;
    
    if(LoopNum==0xff){
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
            pSectCali->CaliCnt[i] = ((pSectCali->CaliCnt[i]==0)?pSectCali->CaliCnt[i]:pSectCali->CaliCnt[i]-1);
        }
    }else{
        i = LoopNum-1;
        pSectCali->CaliCnt[i] = ((pSectCali->CaliCnt[i]==0)?pSectCali->CaliCnt[i]:pSectCali->CaliCnt[i]-1);
    }
        
    if(0!=MeterData_WaitWriteSucc(1,(uint8_t *)&pSectCali->CaliCnt[0],AT24CXX_ADDR_SECTION_RANGE_CNT,AT24CXX_ADDR_SECTION_RANGE_CNT_SIZE))
        return 1;
    return 0;
}
//撤销零点校准
extern uint8_t CalibrationHallSensor_ZERO_Repeal(uint8_t LoopNum,PSECTION_CALIBRATION pSectCali)
{
    uint8_t i=0;
    uint32_t ret;
    
    if(pSectCali==NULL)
        return 1;
    if((LoopNum!=0xff)&&((LoopNum<1)||(LoopNum>DCMETER_LOOP_CNT_MAX)))
        return 1;
    
    if(LoopNum==0xff){
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++)
            pSectCali->SectCaliZero[i] = 0; 
    }else{
        pSectCali->SectCaliZero[LoopNum-1] = 0; 
    }

    ret = MeterData_WaitWriteSucc(1,(uint8_t *)&pSectCali->SectCaliZero[0],AT24CXX_ADDR_SECTION_ZERO,AT24CXX_ADDR_SECTION_ZERO_SIZE);

    return ret;
}
/**/
//根据输入电流值获取校准系数(LoopNum：回路号1-6 Ival：测量电流值 pSectCali：校准系数指针)
extern float GetSectionCaliRatio(uint32_t LoopNum,float Ival,PSECTION_CALIBRATION pSectCali)
{
    uint8_t i,LoopIndex;
    signed long s32Temp;
    float Ratio=1.0;
    
    if((LoopNum>DCMETER_LOOP_CNT_MAX) 
        ||(LoopNum==0)
        ||(pSectCali==NULL))
        return Ratio;
    
    s32Temp = (signed long)(Ival*SECTIONS_CALIBRATION_RATIO);
    LoopIndex = LoopNum-1;
    for(i=0;i<(uint8_t)pSectCali->CaliCnt[LoopIndex];i++)
    {
        if(s32Temp <= pSectCali->SectRange[i].LoopRange[LoopIndex]){
            Ratio = (float)pSectCali->SectCaliDiv[i].CaliValue[LoopIndex]/((float)SECTIONS_CALIBRATION_RATIO);
            break;
        }   
    }
    return Ratio;
}
