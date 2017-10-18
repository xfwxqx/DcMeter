//#include "PM5000T.h"

#include "Usart.h"
#include <string.h>
#include "NUC029xAN.h"
#include "RN8209C.h"
#include "UserCommon.h"
/**************************************
	全局变量声明
****************************************/
RN8209C_INIT_PARAM	sInitParam;

#define EN1 P04
#define EN2 P05
#define EN3 P06
#define EN4 P07
/*****************************************************************************
** Function name:RN8209C_ReadReg(uint8_t RegAddr,uint8_t *pBuf,uint8_t Len)
**
** Description:读RN8209寄存器
**
** Parameters:RegAddr 寄存器地址，*pBuf读出值的存放地址，Len：待读值的长度   
**
** Returned value:	操作标识-0成功,1-输入参数错误 2-读取失败
**
******************************************************************************/
//累积和不能超过uint32_t的最大值
static uint64_t GetSum_u32(uint32_t *s,uint8_t Cnt)
{
    uint8_t i;
    uint64_t Sum;
    
    for(i=0,Sum=0;i<Cnt;i++){
        Sum += s[i];
    }
    return Sum;
}
extern void RN8209C_Select(uint8_t Num)
{
    GPIO_SetMode(P0, BIT4, GPIO_PMD_OUTPUT);
	GPIO_SetMode(P0, BIT5, GPIO_PMD_OUTPUT);
	GPIO_SetMode(P0, BIT6, GPIO_PMD_OUTPUT);
	GPIO_SetMode(P0, BIT7, GPIO_PMD_OUTPUT);
    
    EN1 = (Num==1?0:1); 
    EN2 = (Num==2?0:1); 
    EN3 = (Num==3?0:1); 
    EN4 = (Num==4?0:1); 
}
extern uint32_t RN8209C_ReadReg(uint8_t RegAddr,uint8_t *pBuf,uint8_t Len)
{
	uint8_t i,j,CheckSum=0,Timeout;
	uint8_t ReadBuf[5]={0};
	
	if(pBuf==NULL)
		return FUNC_RET_PARAM_ERR;
	if((Len==0)||(Len>4))
		return FUNC_RET_PARAM_ERR;

	Timeout = 0;
	//ReadBytes = Len+1;
	CheckSum = RegAddr;
	 do{
	 	
		USART1_SendData(&RegAddr,1);
		if(FALSE == UART_Read(UART1,ReadBuf,Len+1))
			return FUNC_RET_FAIL;
		for(i=0;i<Len;i++)
			CheckSum += ReadBuf[i];
		CheckSum = ~CheckSum;
		if(CheckSum == ReadBuf[Len]){
            for(j=0;j<Len;j++)
                pBuf[Len-j-1] = ReadBuf[j];
			//memcpy(pBuf,ReadBuf,Len);
			return FUNC_RET_SUCC;
		}
		Timeout++;
	 }while(Timeout<3);
	 return FUNC_RET_FAIL;
}

/*****************************************************************************
** Function name:RN8209C_WiteReg(uint8_t RegAddr,uint8_t *pBuf,uint8_t Len)
**
** Description:写RN8209寄存器
**
** Parameters:RegAddr 寄存器地址，*pBuf读出值的存放地址，Len：待读值的长度   
**
** Returned value:	操作标识-0写入成功,1-输入参数错误 2-写入失败
**
******************************************************************************/
extern uint32_t RN8209C_WiteReg(uint8_t RegAddr,uint8_t *pBuf,uint8_t Len)
{
	uint8_t i,CheckSum=0;
    uint8_t WriteBuf[5]={0};
    uint8_t ReadBuf[5]={0};
	
	if(pBuf==NULL)
		return FUNC_RET_PARAM_ERR;
	if((Len==0)||(Len>2))
		return FUNC_RET_PARAM_ERR;

	CheckSum = 0;
	
	WriteBuf[0] = RegAddr|0x80;
	//memcpy(&WriteBuf[1],pBuf,Len);
	for(i=0;i<Len;i++)
        WriteBuf[Len-i] = pBuf[i];
	for(i=0;i<(Len+1);i++)
		CheckSum += WriteBuf[i];
	CheckSum = ~CheckSum;
	WriteBuf[Len+1] = CheckSum;
	i=0;
	do{	
		USART1_SendData(WriteBuf,Len+2);
		if(FUNC_RET_SUCC==RN8209C_ReadReg(RN8209C_REG_WData,ReadBuf,RN8209C_REG_WData_LEN)){
            if(0==memcmp(ReadBuf,pBuf,Len))
				return FUNC_RET_SUCC;
            /*if((Len==1)&&(ReadBuf[1]==pBuf[0]))
                return FUNC_RET_SUCC;//(Len==2)&&(ReadBuf[0]==pBuf[1])&&(ReadBuf[1]==pBuf[0])
			else if(0==memcmp(ReadBuf,pBuf,Len))
				return FUNC_RET_SUCC;*/
		}
		i++;
	}while(i<3);
	
	return FUNC_RET_FAIL;
}
extern uint32_t RN8209C_SoftReset(void) 
{
    uint8_t Buf[1]= {0xfa};
    
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(0xea,Buf,1))
        return FUNC_RET_FAIL;

    if(FUNC_RET_SUCC!=RN8209C_ReadReg(RN8209C_REG_SysStatus,Buf,RN8209C_REG_SysStatus_LEN))
        return FUNC_RET_FAIL;
    
    return ((Buf[0]&(1<<1))?FUNC_RET_SUCC:FUNC_RET_FAIL);
   
}
extern uint32_t RN8209C_WriteEnable(void) 
{
    uint8_t Buf[1]= {0xe5};
    
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(0xea,Buf,1))
        return FUNC_RET_FAIL;
    
    if(FUNC_RET_SUCC!=RN8209C_ReadReg(RN8209C_REG_SysStatus,Buf,RN8209C_REG_SysStatus_LEN))
        return FUNC_RET_FAIL;
    return ((Buf[0]&(1<<4))?FUNC_RET_SUCC:FUNC_RET_FAIL);

}
extern uint32_t RN8209C_WriteProtect(void) 
{
    uint8_t Buf[1]={0xdc};

    
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(0xea,Buf,1))
        return FUNC_RET_FAIL;
    
    if(FUNC_RET_SUCC!=RN8209C_ReadReg(RN8209C_REG_SysStatus,Buf,RN8209C_REG_SysStatus_LEN))
        return FUNC_RET_FAIL;
    return ((Buf[0]&(1<<4))?FUNC_RET_FAIL:FUNC_RET_SUCC);

}
extern void RN8209C_WaitDataUpdate(void) 
{
    uint8_t Buf[1]={0}; 
    do
    {
        RN8209C_ReadReg(RN8209C_REG_IF,Buf,RN8209C_REG_IF_LEN);
    }while(0==(Buf[0]&0x01));
}
extern uint8_t RN8209C_TestDataUpdate(void) 
{
    uint8_t Buf[1]={0}; 
    RN8209C_ReadReg(RN8209C_REG_IF,Buf,RN8209C_REG_IF_LEN);
    return Buf[0]&0x01;
}
extern uint16_t RN8209C_GetCheckSum(void) 
{
    uint8_t Buf[3]={0}; 
    do
    {
        RN8209C_WaitDataUpdate();
        ////等待校验值可用，11.2us
        RN8209C_ReadReg(RN8209C_REG_EMUStatus,Buf,RN8209C_REG_EMUStatus_LEN);
    }while(0!=(Buf[2]&0x01));
    return GetWord(&Buf[0]);
}
//获取功率符号位
static uint16_t RN8209C_GetPowerSign(void) 
{
    uint8_t Buf[3]={0}; 
    
    RN8209C_ReadReg(RN8209C_REG_EMUStatus,Buf,RN8209C_REG_EMUStatus_LEN);
    
    return (Buf[2]>>1)&0x01;
}
static uint32_t RN8209C_Init_ResetParam(PRN8209C_INIT_PARAM pInitParam)
{
    if(pInitParam==NULL)
        return FUNC_RET_PARAM_ERR;
    
    memset((void *)pInitParam,0,sizeof(RN8209C_INIT_PARAM));
    //pInitParam->UNION.PARAM_List.SYSCON = 0x0040;
    pInitParam->UNION.PARAM_List.SYSCON  &= (~REG_SYSCON_PGAIA_MSK);
    pInitParam->UNION.PARAM_List.SYSCON |= REG_SYSCON_PGAIA_1;
    pInitParam->UNION.PARAM_List.SYSCON  &= (~REG_SYSCON_PGAU_MSK);
    pInitParam->UNION.PARAM_List.SYSCON |= REG_SYSCON_PGAU_1;
    pInitParam->UNION.PARAM_List.SYSCON  &= (~REG_SYSCON_PGAIB_MSK);
    pInitParam->UNION.PARAM_List.SYSCON |= REG_SYSCON_PGAIB_1;
    pInitParam->UNION.PARAM_List.SYSCON  &= (~REG_SYSCON_ADC2ON_MSK);
    pInitParam->UNION.PARAM_List.SYSCON |= REG_SYSCON_ADC2ON_ON;
    //关闭IB、IA、U通道数字高通滤波器;//关闭QF、PF脉冲输出和自定义电能寄存器累加
    pInitParam->UNION.PARAM_List.EMUCON = 0x4063;
    //选择电表脉冲常数EC=360000,
    //pInitParam->UNION.PARAM_List.HFConst = 0x1c7f;//
    //pInitParam->UNION.PARAM_List.HFConst = 0x1000;
    //pInitParam->UNION.PARAM_List.HFConst = 0x194b;
    //pInitParam->UNION.PARAM_List.HFConst = 0x2a6e;
    pInitParam->UNION.PARAM_List.HFConst = 0xb8e;
    pInitParam->UNION.PARAM_List.PStart = 0x0060;
    pInitParam->UNION.PARAM_List.DStart = 0x0060;
    /*pInitParam->UNION.PARAM_List.GPQA = 0x0000;
    pInitParam->UNION.PARAM_List.GPQB = 0x0000;
    pInitParam->UNION.PARAM_List.PhsA= 0x0000;
    pInitParam->UNION.PARAM_List.PhsB= 0x0000;
    pInitParam->UNION.PARAM_List.QPhsCal = 0x0000;
    pInitParam->UNION.PARAM_List.APOSA = 0x0000;
    pInitParam->UNION.PARAM_List.APOSB = 0x0000;
    pInitParam->UNION.PARAM_List.RPOSA = 0x0000;
    pInitParam->UNION.PARAM_List.RPOSB = 0x0000;
    pInitParam->UNION.PARAM_List.IARMSOS = 0x0000;
    pInitParam->UNION.PARAM_List.IBRMSOS = 0x0000;
    pInitParam->UNION.PARAM_List.IBGain = 0x0000;
    pInitParam->UNION.PARAM_List.D2FPL = 0x0000;
    pInitParam->UNION.PARAM_List.D2FPH = 0x0000;
    pInitParam->UNION.PARAM_List.DCIAH = 0x0000;
    pInitParam->UNION.PARAM_List.DCIBH = 0x0000;
    pInitParam->UNION.PARAM_List.DCUH = 0x0000;
    pInitParam->UNION.PARAM_List.DCL = 0x0000;*/
    pInitParam->UNION.PARAM_List.EMUCON2 = 0x0030;//自定义电能输入选择为通道B有功功率
    
    return FUNC_RET_SUCC;
}

extern uint32_t RN8209C_Init(PRN8209C_INIT_PARAM pInitParam)
{
	uint8_t	i;
	uint16_t CheckSum=0;
	//uint16_t Temp[24]={0};
    
    if(pInitParam==NULL)
        return FUNC_RET_PARAM_ERR;
    
    if(FUNC_RET_SUCC != RN8209C_WriteEnable())
        return FUNC_RET_FAIL;
    
    if(FUNC_RET_SUCC != RN8209C_SoftReset())
        goto Err;


	if(FUNC_RET_SUCC != RN8209C_WriteEnable())
        return FUNC_RET_FAIL;
    //RN8209C_Init_ResetParam(pInitParam);
	CheckSum=0;
	for(i=0;i<24;i++){
		CheckSum += (uint16_t)pInitParam->UNION.PARAM_Union[i];
		if((i==7)||(i==8)){
			if(FUNC_RET_SUCC!=RN8209C_WiteReg(i,(uint8_t *)&pInitParam->UNION.PARAM_Union[i],1))
				goto Err;
		}
		else{
			if(FUNC_RET_SUCC!=RN8209C_WiteReg(i,(uint8_t *)&pInitParam->UNION.PARAM_Union[i],2))
				goto Err;
		}
	}
	CheckSum += 0x1600;//自己添加
	CheckSum = ~CheckSum;
    pInitParam->UNION.PARAM_List.CHKSUM = CheckSum;
    if(pInitParam->UNION.PARAM_List.CHKSUM != RN8209C_GetCheckSum())
        goto Err;
    
    RN8209C_WriteProtect(); 
    return FUNC_RET_SUCC;
	/*
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_SYSCON,(uint8_t *)&pInitParam->SYSCOM,RN8209C_REG_SYSCON_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_EMUCON,(uint8_t *)&pInitParam->EMUCON,RN8209C_REG_EMUCON_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_HFConst,(uint8_t *)&pInitParam->HFConst,RN8209C_REG_HFConst_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_PStart,(uint8_t *)&pInitParam->PStart,RN8209C_REG_PStart_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_DStart,(uint8_t *)&pInitParam->DStart,RN8209C_REG_DStart_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_GPQA,(uint8_t *)&pInitParam->GPQA,RN8209C_REG_GPQA_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_GPQB,(uint8_t *)&pInitParam->GPQB,RN8209C_REG_GPQB_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_PhsA,(uint8_t *)&pInitParam->PhsA,RN8209C_REG_PhsA_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_PhsB,(uint8_t *)&pInitParam->PhsB,RN8209C_REG_PhsB_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_QPHSCAL,(uint8_t *)&pInitParam->QPhsCal,RN8209C_REG_QPHSCAL_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_APOSA,(uint8_t *)&pInitParam->APOSA,RN8209C_REG_APOSA_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_APOSB,(uint8_t *)&pInitParam->APOSB,RN8209C_REG_APOSB_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_RPOSA,(uint8_t *)&pInitParam->RPOSA,RN8209C_REG_RPOSA_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_RPOSB,(uint8_t *)&pInitParam->RPOSB,RN8209C_REG_RPOSB_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_IARMSOS,(uint8_t *)&pInitParam->IARMSOS,RN8209C_REG_IARMSOS_LEN))
		return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_IBRMSOS,(uint8_t *)&pInitParam->IBRMSOS,RN8209C_REG_IBRMSOS_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_IBGain,(uint8_t *)&pInitParam->IBGain,RN8209C_REG_IBGain_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_D2FPL,(uint8_t *)&pInitParam->D2FPL,RN8209C_REG_D2FPL_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_D2FPH,(uint8_t *)&pInitParam->D2FPH,RN8209C_REG_D2FPH_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_DCIAH,(uint8_t *)&pInitParam->DCIAH,RN8209C_REG_DCIAH_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_DCIBH,(uint8_t *)&pInitParam->DCIBH,RN8209C_REG_DCIBH_LEN))
		return FUNC_RET_FAIL;
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_DCUH,(uint8_t *)&pInitParam->DCUH,RN8209C_REG_DCUH_LEN))
		return FUNC_RET_FAIL;	
	if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_DCL,(uint8_t *)&pInitParam->DCL,RN8209C_REG_DCL_LEN))
		return FUNC_RET_FAIL;	
    if(FUNC_RET_SUCC!=RN8209C_WiteReg(RN8209C_REG_EMUCON2,(uint8_t *)&pInitParam->EMUCON2,RN8209C_REG_EMUCON2_LEN))
		return FUNC_RET_FAIL;
	*/
    Err:RN8209C_WriteProtect(); 
	//RN8209C_Select(0);
    return FUNC_RET_FAIL;
}

extern uint32_t RN8209C_GetRegValue(uint8_t RegAddr,uint32_t *pVal)
{
    uint8_t RegAddrTemp=0,RegLen=0;
    uint32_t u32Temp=0;
    
    if(pVal==NULL)
        return FUNC_RET_PARAM_ERR;
    
    switch(RegAddr)
    {
        case RN8209C_REG_PhsA:
        case RN8209C_REG_PhsB:
        case RN8209C_REG_IE:
        case RN8209C_REG_IF:
        case RN8209C_REG_SysStatus:
            RegAddrTemp = RegAddr;
            RegLen = 1;
            break;
        case RN8209C_REG_IARMS:
        case RN8209C_REG_IBRMS:  
        case RN8209C_REG_URMS:
        case RN8209C_REG_EnergyP:
        case RN8209C_REG_EnergyP2:
        case RN8209C_REG_EnergyD:
        case RN8209C_REG_EnergyD2:
        case RN8209C_REG_EMUStatus:
        case RN8209C_REG_SPL_IA:
        case RN8209C_REG_SPL_IB:
        case RN8209C_REG_SPL_U:
        case RN8209C_REG_UFreq2:
        case RN8209C_REG_DeviceID:
            RegAddrTemp = RegAddr;
            RegLen = 3;
            break;
        case RN8209C_REG_SYSCON:
        case RN8209C_REG_EMUCON:  
        case RN8209C_REG_HFConst:
        case RN8209C_REG_PStart:
        case RN8209C_REG_DStart:
        case RN8209C_REG_GPQA:
        case RN8209C_REG_GPQB:
        case RN8209C_REG_QPHSCAL:
        case RN8209C_REG_APOSA:
        case RN8209C_REG_APOSB:
        case RN8209C_REG_RPOSA:
        case RN8209C_REG_RPOSB:
        case RN8209C_REG_IARMSOS:
        case RN8209C_REG_IBRMSOS:
        case RN8209C_REG_IBGain:  
        case RN8209C_REG_D2FPL:
        case RN8209C_REG_D2FPH:
        case RN8209C_REG_DCIAH:
        case RN8209C_REG_DCIBH:
        case RN8209C_REG_DCUH:
        case RN8209C_REG_DCL:
        case RN8209C_REG_EMUCON2:
        case RN8209C_REG_PFCnt:
        case RN8209C_REG_DFcnt:
        case RN8209C_REG_UFreq:
        case RN8209C_REG_WData:
            RegAddrTemp = RegAddr;
            RegLen = 2;
            break;
        case RN8209C_REG_PowerPA:
        case RN8209C_REG_PowerPB:
        case RN8209C_REG_PowerQ:
        case RN8209C_REG_RDataS:
            RegAddrTemp = RegAddr;
            RegLen = 4;
            break;
        
        default:
            RegAddrTemp = RN8209C_REG_DeviceID;
            RegLen = RN8209C_REG_DeviceID_LEN;
            break;
    }
    
    if(0==RN8209C_ReadReg(RegAddrTemp,(uint8_t *)&u32Temp,RegLen)){
        *pVal = u32Temp;
        return FUNC_RET_SUCC;
    }
    return FUNC_RET_FAIL;
        //printf("IARMS:0x%x\n",GetDWord(u8Temp)&0xffffff);
}
#define RN8209C_GET_AVERAGE_CNT 6
//多次测量取Ia、Ib、U平均值
static uint32_t RN8209C_GetCurrVolAverage(uint32_t *pIa,uint32_t *pIb,uint32_t *pU)
{
    uint8_t Cnt;
    uint32_t IaTemp[RN8209C_GET_AVERAGE_CNT]={0};
    uint32_t IbTemp[RN8209C_GET_AVERAGE_CNT]={0};
    uint32_t UTemp[RN8209C_GET_AVERAGE_CNT]={0};
    uint64_t Sum;
    
    for(Cnt=0;Cnt<RN8209C_GET_AVERAGE_CNT;Cnt++){
        //TIMER_Delay(TIMER1,700000);//接近数据更新频率
        RN8209C_WaitDataUpdate();
        if((pIa!=NULL)&&(FUNC_RET_SUCC != RN8209C_GetRegValue(RN8209C_REG_IARMS,&IaTemp[Cnt])))
            return FUNC_RET_FAIL;
        if((pIb!=NULL)&&(FUNC_RET_SUCC != RN8209C_GetRegValue(RN8209C_REG_IBRMS,&IbTemp[Cnt])))
            return FUNC_RET_FAIL;
        if((pU!=NULL)&&(FUNC_RET_SUCC != RN8209C_GetRegValue(RN8209C_REG_URMS,&UTemp[Cnt])))
            return FUNC_RET_FAIL;
    }
    Bubble_Sort(IaTemp,RN8209C_GET_AVERAGE_CNT);
    Sum = GetSum_u32(&IaTemp[1],RN8209C_GET_AVERAGE_CNT-2);
    Sum /= (RN8209C_GET_AVERAGE_CNT-2);
    if(pIa!=NULL)
        *pIa = (uint32_t)Sum;
    Bubble_Sort(IbTemp,RN8209C_GET_AVERAGE_CNT);
    Sum = GetSum_u32(&IbTemp[1],RN8209C_GET_AVERAGE_CNT-2);
    Sum /= (RN8209C_GET_AVERAGE_CNT-2);
    if(pIb!=NULL)
        *pIb = (uint32_t)Sum;
    Bubble_Sort(UTemp,RN8209C_GET_AVERAGE_CNT);
    Sum = GetSum_u32(&UTemp[1],RN8209C_GET_AVERAGE_CNT-2);
    Sum /= (RN8209C_GET_AVERAGE_CNT-2);
    if(pU!=NULL)
        *pU = (uint32_t)Sum;
    
    return FUNC_RET_SUCC;
}
//多次测量取Pa、Pb平均值
static uint32_t RN8209C_GetPowerAverage(int32_t *pPa,int32_t *pPb)
{
    uint8_t Cnt;
    uint32_t PaTemp[RN8209C_GET_AVERAGE_CNT]={0};
    uint32_t PbTemp[RN8209C_GET_AVERAGE_CNT]={0};
    uint64_t Sum;
    
    for(Cnt=0;Cnt<RN8209C_GET_AVERAGE_CNT;Cnt++){
        //TIMER_Delay(TIMER1,700000);//接近数据更新频率
        RN8209C_WaitDataUpdate();
        if(FUNC_RET_SUCC != RN8209C_GetRegValue(RN8209C_REG_PowerPA,&PaTemp[Cnt]))
            return FUNC_RET_FAIL;
        if(FUNC_RET_SUCC != RN8209C_GetRegValue(RN8209C_REG_PowerPB,&PbTemp[Cnt]))
            return FUNC_RET_FAIL;
        
    }
	Sum = 0;
    Bubble_Sort(PaTemp,RN8209C_GET_AVERAGE_CNT);
    Sum = GetSum_u32(&PaTemp[1],RN8209C_GET_AVERAGE_CNT-2);
    Sum /= (RN8209C_GET_AVERAGE_CNT-2);
    if(pPa!=NULL)
        *pPa = Sum;
	
    Sum = 0;
    Bubble_Sort(PbTemp,RN8209C_GET_AVERAGE_CNT);
	Sum = GetSum_u32(&PbTemp[1],RN8209C_GET_AVERAGE_CNT-2);
    Sum /= (RN8209C_GET_AVERAGE_CNT-2);
    if(pPb!=NULL)
        *pPb = Sum;
    
    return FUNC_RET_SUCC;
}

extern uint32_t RN8209C_GetIa(PRN8209C_CALI_PARAM pCali,float *pI)
{
    uint32_t u32Temp=0;
    float fTemp=0.0;
    int32_t s32Temp=0;
    
    if(pCali==NULL)
        return FUNC_RET_PARAM_ERR;
    
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_IARMS,&u32Temp))
        return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_PowerPA,(uint32_t *)&s32Temp))
        return FUNC_RET_FAIL;
    
    //printf("IARMS:%u\n",u32Temp&0xffffff);
    u32Temp &= 0x7fffff;
    fTemp = (float)u32Temp;
    fTemp /= 1000000.0;
    fTemp *= pCali->Ki;
    fTemp = (s32Temp<0)?-fTemp:fTemp;
    
    if(pI!=NULL){
        if(u32Temp&0x800000){
            *pI = 0.0;
        }else{
            *pI = fTemp;
        }
    }

    return FUNC_RET_SUCC;
}

extern uint32_t RN8209C_GetIb(PRN8209C_CALI_PARAM pCali,float *pI)
{
    uint32_t u32Temp=0;
    float fTemp=0.0;
    int32_t s32Temp=0;
    
    if(pCali==NULL)
        return FUNC_RET_PARAM_ERR;
    
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_IBRMS,&u32Temp))
        return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_PowerPB,(uint32_t *)&s32Temp))
        return FUNC_RET_FAIL;
    
    //printf("IARMS:%u\n",u32Temp&0xffffff);
    u32Temp &= 0x7fffff;
    fTemp = (float)u32Temp;
    fTemp /= 1000000.0;
    fTemp *= pCali->Ki;
    fTemp = (s32Temp<0)?-fTemp:fTemp;
    
    if(pI!=NULL){
        if(u32Temp&0x800000){
            *pI = 0.0;
        }else{
            *pI = fTemp;
        }
    }
    return FUNC_RET_SUCC;
}

extern uint32_t RN8209C_GetU(PRN8209C_CALI_PARAM pCali,float *pU)
{
    uint32_t u32Temp=0;
    float fTemp=0.0;
    
    
    if(pCali==NULL)
        return FUNC_RET_PARAM_ERR;
    
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_URMS,&u32Temp))
        return FUNC_RET_FAIL;
    u32Temp &= 0x7fffff;
    fTemp = (float)u32Temp;
    fTemp /= 1000000.0;
    fTemp *= pCali->Kv;
    if(pU!=NULL)
        *pU = fTemp;
    //printf("IARMS:%u\n",u32Temp&0xffffff);
    /*if(pCali->UReg != 0){
        pCali->UReal = (float)u32Temp/pCali->UReg;
        pCali->UReal *= pCali->Un;
    }*/
    return FUNC_RET_SUCC;
}
extern uint32_t RN8209C_GetPa(PRN8209C_CALI_PARAM pCali,float *pP)
{
    int32_t s32Temp=0;
    float fTemp=0.0;
    
    if(pCali==NULL)
        return FUNC_RET_PARAM_ERR;
    
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_PowerPA,(uint32_t *)&s32Temp))
        return FUNC_RET_FAIL;
    fTemp = (float)s32Temp;
    fTemp /= 1000000.0;
    fTemp *= pCali->Kp;
    fTemp /= 1000.0;
    if(pP!=NULL)
        *pP = fTemp;
    //printf("IARMS:%u\n",u32Temp&0xffffff);
    /*if(pCali->PaReg != 0){
        pCali->PaReal = (float)s32Temp/pCali->PaReg;
        pCali->PaReal *= pCali->Un;
        pCali->PaReal *= pCali->In;
    }*/
    return FUNC_RET_SUCC;
}
extern uint32_t RN8209C_GetEa(PRN8209C_CALI_PARAM pCali,double *pE)
{
    uint32_t u32Temp=0;
    float fTemp=0.0;
    int32_t s32Temp;
    
    if(pCali==NULL)
        return FUNC_RET_PARAM_ERR;
    
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_EnergyP2,&u32Temp))
        return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_PowerPA,(uint32_t *)&s32Temp))
        return FUNC_RET_FAIL;
    fTemp = (double)u32Temp;
    fTemp /= 360000.0;
    fTemp = (s32Temp<0)?-fTemp:fTemp;
    if(pE!=NULL)
        *pE = fTemp;

    //printf("IARMS:%u\n",u32Temp&0xffffff);
    /*if(pCali->PaReg != 0){
        pCali->PaReal = (float)s32Temp/pCali->PaReg;
        pCali->PaReal *= pCali->Un;
        pCali->PaReal *= pCali->In;
    }*/
    return FUNC_RET_SUCC;
}
extern uint32_t RN8209C_GetPb(PRN8209C_CALI_PARAM pCali,float *pP)
{
    int32_t s32Temp=0;
    float fTemp=0.0;
	
    if(pCali==NULL)
        return FUNC_RET_PARAM_ERR;
    
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_PowerPB,(uint32_t *)&s32Temp))
        return FUNC_RET_FAIL;

    //printf("IARMS:%u\n",u32Temp&0xffffff);
    fTemp = (float)s32Temp;
    fTemp /= 1000000.0;
    fTemp *= pCali->Kp;
    fTemp /= 1000.0;
    if(pP!=NULL)
        *pP = fTemp;
    
    return FUNC_RET_SUCC;
}
extern uint32_t RN8209C_GetEb(PRN8209C_CALI_PARAM pCali,double *pE)
{
    uint32_t u32Temp=0;
    float fTemp=0.0;
    int32_t s32Temp;
    
    if(pCali==NULL)
        return FUNC_RET_PARAM_ERR;
    
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_EnergyD2,&u32Temp))
        return FUNC_RET_FAIL;
    if(FUNC_RET_SUCC!=RN8209C_GetRegValue(RN8209C_REG_PowerPB,(uint32_t *)&s32Temp))
        return FUNC_RET_FAIL;
    fTemp = (double)u32Temp;
    fTemp /= 360000.0;
    fTemp = (s32Temp<0)?-fTemp:fTemp;
    if(pE!=NULL)
        *pE = fTemp;
    //printf("IARMS:%u\n",u32Temp&0xffffff);
    /*if(pCali->PaReg != 0){
        pCali->PaReal = (float)s32Temp/pCali->PaReg;
        pCali->PaReal *= pCali->Un;
        pCali->PaReal *= pCali->In;
    }*/
    return FUNC_RET_SUCC;
}

//直流偏置校准
extern uint32_t RN8209C_DCOffsetCalibration(PRN8209C_INIT_PARAM pInitParam)
{
    uint32_t IARMS1=0;
    uint32_t IARMS2=0;
    uint32_t IBRMS1=0;
    uint32_t IBRMS2=0;
    uint32_t URMS1=0;
    uint32_t URMS2=0;
    
    uint8_t NeedInitFlag=0;
    
    if(pInitParam==NULL)
        return FUNC_RET_PARAM_ERR;
    
    RN8209C_Init_ResetParam(pInitParam);
    if(FUNC_RET_SUCC != RN8209C_Init(pInitParam))
        return FUNC_RET_FAIL;
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    //输入接地，读IA、IB、U三路的有效值10次，计算有效值的平均值IARMS1、IBRMS1、URMS1
    /**/debug("RN8209C_GetCurrVolAverage\n");
    RN8209C_GetCurrVolAverage(&IARMS1,&IBRMS1,&URMS1);
    //将IARMS1平均值的BIT[23:8]写入DCIAH寄存器，BIT[7:4]写入DCL寄存器的BIT[3:0]
    debug("IARMS1=%x\n",IARMS1);
    //IARMS1 = ~IARMS1;
    pInitParam->UNION.PARAM_List.DCIAH = (uint16_t)(IARMS1>>8);
    pInitParam->UNION.PARAM_List.DCL &= (uint16_t)(~0x000f);
    pInitParam->UNION.PARAM_List.DCL |= (uint16_t)((IARMS1>>4)&0xf);
    //将IBRMS1平均值的BIT[23:8]写入DCIBH寄存器，BIT[7:4]写入DCL寄存器的BIT[7:4]
    debug("IBRMS1=%x\n",IBRMS1);
    //IBRMS1 = ~IBRMS1;
    pInitParam->UNION.PARAM_List.DCIBH = (uint16_t)(IBRMS1>>8);
    pInitParam->UNION.PARAM_List.DCL &= (uint16_t)(~0x00f0);
    pInitParam->UNION.PARAM_List.DCL |= (uint16_t)((IBRMS1>>4)&0xf)<<4;
    //将URMS1平均值的BIT[23:8]写入DCUH寄存器，BIT[7:4]写入DCL寄存器的BIT[11:8]
    debug("URMS1=%x\n",URMS1);
    //URMS1 = ~URMS1;
    pInitParam->UNION.PARAM_List.DCUH = (uint16_t)(URMS1>>8);
    pInitParam->UNION.PARAM_List.DCL &= (uint16_t)(~0x0f00);
    pInitParam->UNION.PARAM_List.DCL |= (uint16_t)((URMS1>>4)&0xf)<<8;
    //重新写入初始化参数，并等待两秒
    if(FUNC_RET_SUCC != RN8209C_Init(pInitParam))
        return FUNC_RET_FAIL;
    debug("Delay Start\n");
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    debug("Delay Stop\n");
    //读IA、IB、U三路的有效值10次，计算有效值的平均值IARMS2、IBRMS2、URMS2
    debug("RN8209C_GetCurrVolAverage --2\n");
    RN8209C_GetCurrVolAverage(&IARMS2,&IBRMS2,&URMS2);
    debug("IARMS2=%x\n",IARMS2);
    if(IARMS2>IARMS1){
        IARMS2 = ~IARMS1;
        debug("~IARMS1=%x\n",IARMS2);
        pInitParam->UNION.PARAM_List.DCIAH = (uint16_t)(IARMS2>>8);
        pInitParam->UNION.PARAM_List.DCL &= (uint16_t)(~0x000f);
        pInitParam->UNION.PARAM_List.DCL |= (uint16_t)((IARMS2>>4)&0xf);
        NeedInitFlag = 1;
    }
    
    debug("IBRMS2=%x\n",IBRMS2);
    if(IBRMS2>IBRMS1){
        IBRMS2 = ~IBRMS1;
        debug("~IBRMS1=%x\n",IBRMS2);
        pInitParam->UNION.PARAM_List.DCIBH = (uint16_t)(IBRMS2>>8);
        pInitParam->UNION.PARAM_List.DCL &= (uint16_t)(~0x00f0);
        pInitParam->UNION.PARAM_List.DCL |= (uint16_t)((IBRMS2>>4)&0xf)<<4;
        NeedInitFlag = 1;
    }
    
    debug("URMS2=%x\n",URMS2);
    if(URMS2>URMS1){
        URMS2 = ~URMS1;
        debug("~URMS1=%x\n",URMS2);
        pInitParam->UNION.PARAM_List.DCUH = (uint16_t)(URMS2>>8);
        pInitParam->UNION.PARAM_List.DCL &= (uint16_t)(~0x0f00);
        pInitParam->UNION.PARAM_List.DCL |= (uint16_t)((URMS2>>4)&0xf)<<8;
        NeedInitFlag = 1;
    }
    if(1==NeedInitFlag){
        if(FUNC_RET_SUCC != RN8209C_Init(pInitParam))
            return FUNC_RET_FAIL;
    }
    //printf("Delay Start\n");
    //稳定2s，读取IA、IB电流的有效值10次，求各自平均值进行
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    //printf("Delay Stop\n");
    RN8209C_GetCurrVolAverage(&IARMS1,&IBRMS1,NULL);
    debug("IARMS1=%x\n",IARMS1);
    IARMS1 = IARMS1*IARMS1;
    IARMS1>>=8;
    IARMS1 = ~IARMS1;
    pInitParam->UNION.PARAM_List.IARMSOS = IARMS1&0xffff;
    debug("IBRMS1=%x\n",IBRMS1);
    IBRMS1 = IBRMS1*IBRMS1;
    IBRMS1>>=8;
    IBRMS1 = ~IBRMS1;
    pInitParam->UNION.PARAM_List.IBRMSOS = IBRMS1&0xffff;
    //重新写入初始化参数
    if(FUNC_RET_SUCC != RN8209C_Init(pInitParam))
        return FUNC_RET_FAIL;
    if(0){
        Comm_DelayUs(500000);
        Comm_DelayUs(500000);
        RN8209C_GetCurrVolAverage(&IARMS1,&IBRMS1,&URMS1);
        debug("IARMS1=%x IBRMS1=%x URMS1=%x\n",IARMS1,IBRMS1,URMS1);
    }
    
    return FUNC_RET_SUCC;
}
static uint32_t RN8209C_GetPGAIN(int32_t P,RN8209C_CALI_PARAM *pCali,uint16_t *pPGAIN)
{
	float ftemp;
	float ERR=0.0;

	if(pCali==NULL)
        return FUNC_RET_PARAM_ERR;
	
	ftemp = (float)P;
	ftemp /= 1000000.0;
	ftemp *= pCali->Kp;//P测
	ftemp /= (pCali->In*pCali->Un);
	
	ERR = ftemp - 1.0;
	
	ftemp = -ERR/(1.0+ERR);//PGAIN
	
	if(ftemp>0.0001)
		*pPGAIN = (uint16_t)(ftemp*32768);
	else
		*pPGAIN = (uint16_t)(65536+ftemp*0x8000);
	
	return FUNC_RET_SUCC;
}
//直流增益校准
extern uint32_t RN8209C_DCGainCalibration(PRN8209C_DEF pRn8209c)
{
    RN8209C_CALI_PARAM *pCali=NULL;
    RN8209C_INIT_PARAM *pInit=NULL;
    uint32_t Ia=0,Ib=0,U=0;
	int32_t Pa=0,Pb=0;
    float ftemp;
    uint16_t u16Temp=0;
	
    if(pRn8209c==NULL)
        return FUNC_RET_PARAM_ERR;
    pCali = &pRn8209c->CaliParam;
    pInit = &pRn8209c->InitParam;

	//RN8209C_WaitDataUpdate();
    pInit->UNION.PARAM_List.IBGain = 0;
    pInit->UNION.PARAM_List.GPQA = 0;
    pInit->UNION.PARAM_List.GPQB = 0;
    
    //RN8209C_Init_ResetParam(pInit);

    
    if(FUNC_RET_SUCC != RN8209C_Init(pInit))
        return FUNC_RET_FAIL;
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    //printf("RN8209C_GetCurrVolAverage\n");
    if(FUNC_RET_SUCC != RN8209C_GetCurrVolAverage(&Ia,&Ib,NULL))
        return FUNC_RET_FAIL;
	//printf("IARMS1=%x\n",Ia);
    //printf("IBRMS2=%x\n",Ib);
    
	ftemp = (float)Ia;
	ftemp /= (float)Ib;
    //printf("%u ftemp=%f\n",__LINE__,ftemp);
	if(ftemp>1.0){
		ftemp -= 1.0;
		ftemp *= 32768.0;
		//printf("%u ftemp=%f\n",__LINE__,ftemp);
	}else{
		ftemp -= 1.0;
		ftemp *= 32768.0;
		ftemp += 65536.0;
        //printf("%u ftemp=%f\n",__LINE__,ftemp);
	}
	pInit->UNION.PARAM_List.IBGain = (uint16_t)ftemp;
    //printf("IBGain=%x\n",pInit->UNION.PARAM_List.IBGain);
    
	if(FUNC_RET_SUCC != RN8209C_Init(pInit))
            return FUNC_RET_FAIL;
	Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    Comm_DelayUs(500000);
    RN8209C_WaitDataUpdate();
    if(FUNC_RET_SUCC != RN8209C_GetCurrVolAverage(&Ia,&Ib,&U))
        return FUNC_RET_FAIL;
    debug("Ia=%x Ib=%x U=%x\n",Ia,Ib,U);
	//比例系数都扩大了1000000倍
    Ia &= 0x7fffff;
    ftemp = (float)Ia;
    ftemp /= 1000000.0;
    pCali->Ki = pCali->In/ftemp;
    debug("Ki=%f\n",pCali->Ki);
    U &= 0x7fffff;
    ftemp = (float)U;
    ftemp /= 1000000.0;
    pCali->Kv = pCali->Un/ftemp;
    printf("Kv=%f\n",pCali->Kv);
    //pCali->Kp= 0.19180;
    //pCali->Kp= 0.321751;
    //pCali->Kp= 0.191763;
    pCali->Kp= 0.7043309;
	if(FUNC_RET_SUCC != RN8209C_GetPowerAverage(&Pa,&Pb))
        return FUNC_RET_FAIL;
    //printf("Pa=%d Pb=%d\n",Pa,Pb);
	if(FUNC_RET_SUCC != RN8209C_GetPGAIN(Pa,pCali,&u16Temp))
        return FUNC_RET_FAIL;
    
	pInit->UNION.PARAM_List.GPQA = u16Temp;//0x2a10;
	//printf("GPQA=%x\n",u16Temp);
    if(FUNC_RET_SUCC != RN8209C_GetPGAIN(Pb,pCali,&u16Temp))
        return FUNC_RET_FAIL;
	pInit->UNION.PARAM_List.GPQB = u16Temp;
	//printf("GPQB=%x\n",u16Temp);
    if(FUNC_RET_SUCC != RN8209C_Init(pInit))
            return FUNC_RET_FAIL;

    return FUNC_RET_SUCC;
}


