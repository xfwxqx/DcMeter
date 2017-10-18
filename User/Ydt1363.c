/**
  *****************************************************************************
  *                            ����YDT1363_3Э��
  *
  *                       (C) Copyright 2017-2018, ***
  *                            All Rights Reserved
  *****************************************************************************
  *
  * @File    : Ydt1363.c
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
  *
  *
  *****************************************************************************
**/

#include "UserCommon.h"
#include "Ydt1363.h"
//#include "AT24Cxx.h"
#include <stdlib.h>
#include "NRX8010.h"
#include "AT45DBXXX.h"
//#include "AT24Cxx.h"

DCMETER_DATA gDCMeterData;
uint8_t gu8Prot_CID2;
uint8_t gu8DataFlag=0;
uint8_t gu8Prot_RTN;


#define APROM_TO_LDROM      0x5aa5aa50 //APROM����LDROM��ǣ������λΪBoot�����ʣ�
#define APROM_TO_LDROM_MSK  0xfffffff0
/*#define LDROM_BAUDRATE_1200      0x0
#define LDROM_BAUDRATE_2400      0x01
#define LDROM_BAUDRATE_4800      0x02*/
#define LDROM_BAUDRATE_9600      0x03
#define LDROM_BAUDRATE_19200      0x04


/*
 ************************************************************************************************************************************************************************     
 *��������:	HexToAscii
 *��������:	������ת��ΪASCII
 *��������:	ԭʼ���ݣ�ת����������ԭʼ���ݳ��ȣ���ĸ�Ƿ��д
 *�������:	ת������
 *��������:	
 *��������:	
 *ȫ������: 
 *����˵��: ת�����������ȱ�����ڵ���ԭʼ���ݳ��ȵ�2����ת����˳��Ϊ��λ��ǰ
 ************************************************************************************************************************************************************************       
 */
extern void HexToAscii(const uint8_t *oBuf, uint8_t *dBuf, const uint32_t oLen, const uint32_t IsCapital)
{
	uint32_t		Count;
	uint8_t		Add;

	if (IsCapital)
		Add		= 0;
	else
		Add		= 0x20;
	
	for (Count = 0; Count < oLen; Count ++)
	{
		if (((*oBuf >> 4) & 0xF) > 9)
			*dBuf		= Add + 0x41 + ((*oBuf >> 4) & 0xF) - 10;
		else
			*dBuf		= 0x30 + ((*oBuf >> 4) & 0xF);

		dBuf			++;

		if ((*oBuf & 0xF) > 9)
			*dBuf		= Add + 0x41 + (*oBuf & 0xF) - 10;
		else
			*dBuf		= 0x30 + (*oBuf & 0xF);

		oBuf			++;
		dBuf			++;
	}
}
/*
 ************************************************************************************************************************************************************************     
 *��������:	AsciiToHex
 *��������:	ASCIIת��Ϊ������
 *��������:	ԭʼ���ݣ�ת����������ԭʼ���ݳ���
 *�������:	
 *��������:	
 *��������:	
 *ȫ������: 
 *����˵��: ת�����������ȱ�����ڵ���ԭʼ���ݳ��ȵ�1/2������2��ASCII�ַ����һ���ֽڷ��뻺������ʣ�൥��ASCII�ַ������4λ����4λ���0
 ************************************************************************************************************************************************************************       
 */
extern void AsciiToHex(const uint8_t *oBuf, uint8_t *dBuf, const uint32_t oLen)
{
	/*uint8_t	    ConvertBuf[3];
	uint32_t		Count;

	for (Count = 0; Count < (oLen / 2); Count ++)
	{
		ConvertBuf[0]	= oBuf[Count * 2];
		ConvertBuf[1]	= oBuf[Count * 2 + 1];
		ConvertBuf[2]	= '\0';

		*dBuf			= strtoul(ConvertBuf, NULL, 16);
		dBuf			++;
	}
		
	if (oLen % 2)
	{
		ConvertBuf[0]	= oBuf[oLen - 1];
		ConvertBuf[1]	= '\0';
		
		*dBuf			= strtoul(ConvertBuf, NULL, 16);
	}*/
    uint8_t i=0,cnt;

	for(cnt=0;cnt<oLen;)
	{


		if((oBuf[cnt] >= '0')&&(oBuf[cnt] <= '9'))
		{
			dBuf[i] = oBuf[cnt]-0x30;
		}
		else if((oBuf[cnt] >= 'a')&&(oBuf[cnt] <= 'f'))
		{
			dBuf[i] = oBuf[cnt]-0x61 + 10;
		}
		else if ((oBuf[cnt] >= 'A')&&(oBuf[cnt] <= 'F'))
		{
			dBuf[i] = oBuf[cnt]-0x41 + 10;
		}

		cnt++;
		dBuf[i] *= 0x10;

		if((oBuf[cnt] >= '0')&&(oBuf[cnt] <= '9'))
		{
			dBuf[i] += oBuf[cnt]-0x30;
		}
		else if((oBuf[cnt] >= 'a')&&(oBuf[cnt] <= 'f'))
		{
			dBuf[i] += oBuf[cnt] - 0x61 + 10;
		}
		else if ((oBuf[cnt] >= 'A')&&(oBuf[cnt] <= 'F'))
		{
			dBuf[i] += oBuf[cnt]-0x41 + 10;
		}
		//old++;
		cnt++;
		i++;
	}
}

static uint32_t GetDevAddr(void)
{
	return gDCMeterData.Param.Addr;
}

extern uint8_t GetCID2(void)
{
    return gu8Prot_CID2;
}
extern uint8_t GetRTN(void)
{
    return gu8Prot_RTN;
}
/*
DATA_FLAG:
bit4: 1-��δ��ȡ�Ŀ������仯
bit0: 1-��δ��ȡ�ĸ澯���仯
*/
extern void SetDataFlag(uint8_t IsSwitchChange,uint8_t IsAlarmChange)
{   
    volatile uint8_t u8Temp=0;
    
    if(IsSwitchChange)
       u8Temp |= 0x10;

    if(IsAlarmChange)
       u8Temp |= 0x01;

    gu8DataFlag = u8Temp;
}

extern uint8_t GetDataFlag(void)
{
    return gu8DataFlag;
}
/*
 ************************************************************************************************************************************************************************     
 *��������:	ProtocolProc_YDT1363_3_Make
 *��������:	YDT1363����Э�����
 *��������:	ת�����򣬵���Э��ṹ�壬Э���������������������(��󳤶�)
 *�������:	����Э��ṹ�壬Э�������������������С
 *��������:	�ɹ�(YDT1363_FUNC_RET_SUCCESS)/ʧ��(YDT1363_FUNC_RET_FALSE)
 *��������:	
 *ȫ������: 
 *����˵��: 
 ************************************************************************************************************************************************************************       
 */
extern int32_t ProtocolProc_YDT1363_3_Make(PPROTOCAL_YDT1363_3 pGeneralData,uint8_t *pBuf,uint32_t *pLen)
{
	uint8_t	LenChecksum, ConvertBuf[2*YDT1363_3_DATABUF_MAX];
	uint16_t	DataChecksum=0;
	uint32_t	Len,Count;

	if (pBuf == NULL)
		return	YDT1363_FUNC_RET_FALSE;

	if (pGeneralData == NULL)
		return	YDT1363_FUNC_RET_FALSE;

	if (pGeneralData->Length > YDT1363_3_DATABUF_MAX)
		return	YDT1363_FUNC_RET_FALSE;
	
	Len			= pGeneralData->Length * 2;

	if (Len > 0xFFF)
		return	YDT1363_FUNC_RET_FALSE;

	if ((Len + 18) > *pLen)			/*�����������ܳ���*/
		return	YDT1363_FUNC_RET_FALSE;
	
	LenChecksum		= (Len & 0xF) + ((Len >> 4) & 0xF) + ((Len >> 8) & 0xF);	
	LenChecksum		= LenChecksum % 16;
	LenChecksum		= (~LenChecksum + 1) & 0xF;
	Len				|= (LenChecksum << 12);

	ConvertBuf[0]	= pGeneralData->Ver;
	ConvertBuf[1]	= pGeneralData->Addr;
	ConvertBuf[2]	= pGeneralData->CID1;
	ConvertBuf[3]	= pGeneralData->CID2;
	GetWord(ConvertBuf + 4)	= SWAP_WORD(Len);
	memcpy(&ConvertBuf[6], pGeneralData->DataBuf, pGeneralData->Length);

	pBuf[0]			= 0x7E;
	HexToAscii(ConvertBuf, &pBuf[1], (6 + pGeneralData->Length), TRUE);
	
	DataChecksum	= 0;
	for (Count = 0; Count < (uint32_t)(12 + pGeneralData->Length * 2); Count ++)
		DataChecksum	+= pBuf[1 + Count];

	DataChecksum			= ~DataChecksum + 1;
	GetWord(ConvertBuf)		= SWAP_WORD(DataChecksum);
	HexToAscii(ConvertBuf, &pBuf[13 + pGeneralData->Length * 2], 2, TRUE);
	
	pBuf[17 + 2 * pGeneralData->Length]		= 0x0D;
	*pLen			= 18 + pGeneralData->Length * 2;

	return	YDT1363_FUNC_RET_SUCCESS;

}
/*
 ************************************************************************************************************************************************************************     
 *��������:	ProtocolProc_YDT1363_3_Parse
 *��������:	YDT1363����Э�����
 *��������:	ת�����򣬵���Э��ṹ�壬Э���������������������(��󳤶�)
 *�������:	����Э��ṹ�壬Э�������������������С
 *��������:	�ɹ�(YDT1363_FUNC_RET_SUCCESS)/ʧ��(YDT1363_FUNC_RET_FALSE)
 *��������:	
 *ȫ������: 
 *����˵��: 
 ************************************************************************************************************************************************************************       
 */
extern int32_t ProtocolProc_YDT1363_3_Parse(PPROTOCAL_YDT1363_3 pGeneralData,const uint8_t *pBuf,uint32_t *pLen)
{
	uint8_t	LenChecksum, ConvertBuf[2*YDT1363_3_DATABUF_MAX];
	uint16_t	DataChecksum=0;
	uint32_t	Len,Count;

	if (pBuf == NULL)
		return	YDT1363_FUNC_RET_FALSE;

	if (pBuf[0] != 0x7E)				/*Э��ͷУ�����*/
		return	YDT1363_FUNC_RET_FALSE;

//		if (*pLen < 18)
//			return	YDT1363_FUNC_RET_FALSE;					/*����Э�����С������֤*/

	AsciiToHex(&pBuf[9], ConvertBuf, 4);
	Len				= SWAP_WORD(GetWord(ConvertBuf));

	if (((Len & 0x0FFF) / 2) > YDT1363_3_DATABUF_MAX)
		return	YDT1363_FUNC_RET_FALSE;
	
	if (*pLen < 18 + (Len & 0x0FFF))	/*���������ȱ�����ڵ��ڵ��ܰ���*/
		return	YDT1363_FUNC_RET_FALSE;
	
	LenChecksum		= (Len & 0xF) + ((Len >> 4) & 0xF) + ((Len >> 8) & 0xF);
	LenChecksum		= LenChecksum % 16;
	LenChecksum		= (~LenChecksum + 1) & 0xF;
	if (LenChecksum != ((Len >> 12) & 0xF))		/*����У��λ����*/
		return	YDT1363_3_PROTOCAL_RTN_LCHKSUM_ERROR;
	
	for (Count = 1; Count < (13 + (Len & 0x0FFF)); Count ++)
		DataChecksum	+= pBuf[Count];

	DataChecksum		= ~DataChecksum + 1;
	AsciiToHex(&pBuf[13 + (Len & 0x0FFF)], ConvertBuf, 4);
	if (DataChecksum != SWAP_WORD(GetWord(ConvertBuf)))	/*��У�����*/
		return	YDT1363_3_PROTOCAL_RTN_CHKSUM_ERROR;

	if (pBuf[17 + (Len & 0x0FFF)] != 0x0D)
		return	YDT1363_3_PROTOCAL_RTN_FORMAT_ERROR;	/*Э��βУ�����*/

	if (pGeneralData != NULL)
	{
		AsciiToHex(&pBuf[1], ConvertBuf, 8);
		pGeneralData->Ver		= ConvertBuf[0];
		pGeneralData->Addr		= ConvertBuf[1];
		pGeneralData->CID1		= ConvertBuf[2];
		pGeneralData->CID2		= ConvertBuf[3];
		if ((Len & 0x0FFF)%2)
			pGeneralData->Length	= (Len & 0x0FFF)/2+1;
		else
			pGeneralData->Length	= (Len & 0x0FFF)/2;
		AsciiToHex(&pBuf[13], ConvertBuf, (Len & 0x0FFF));
		memset(pGeneralData->DataBuf, 0, sizeof(pGeneralData->DataBuf));
		memcpy(pGeneralData->DataBuf, ConvertBuf, (Len & 0x0FFF) / 2);
	}

	return	YDT1363_FUNC_RET_SUCCESS;
}

static int32_t ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(const uint8_t Rtn,uint8_t *pBuf,uint32_t *pLen)
{
    PROTOCAL_YDT1363_3 Prot;
	if((pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = Rtn;
	Prot.Length = 0;
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_Anolog(uint8_t IsRatio100,const PDCMETER_ANALOG pData,
	const uint8_t LoopNum,uint8_t *pBuf,uint32_t *pLen)
{
	uint8_t Offset,i;
    int32_t S32Temp;
    uint32_t U32Temp;
    float FTemp;
	PROTOCAL_YDT1363_3 Prot;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;
	if(((LoopNum>DCMETER_LOOP_CNT_MAX)&&(LoopNum!=0xff))
		||(LoopNum == 0))
		return YDT1363_FUNC_RET_FALSE;
	if(pData->LoopCnt>DCMETER_LOOP_CNT_MAX)
		return YDT1363_FUNC_RET_FALSE;
	
	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	//��ȡ���л�·ң������
	if(LoopNum==0xff){
		//Prot.Length = (uint8_t)(1+4+1+pData->LoopCnt*sizeof(&pData->LoopAnalog[0]));
		Offset = 0;
		Prot.DataBuf[Offset] = GetDataFlag(); 
		Offset += 1;
        U32Temp = pData->AcAnalog.AcVoltage*DCMETER_DATA_RATIO;
        
        GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD(U32Temp); 
		//GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)pData->AcAnalog.AcVoltage*1); //������ѹ����Э���������壬��Ϊ1
		Offset += 4;
		Prot.DataBuf[Offset] = pData->LoopCnt; 
		Offset += 1;
		for(i=0;i<pData->LoopCnt;i++){
			if(i>=DCMETER_LOOP_CNT_MAX)
				break;
			//Prot.DataBuf[Offset] = pData->LoopAnalog[i].LoopNum; 
			//Offset += 1;
            FTemp = pData->LoopAnalog[i].LoopVoltage*DCMETER_DATA_RATIO;
            S32Temp = (int32_t)FTemp;
            GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG(S32Temp);
			//GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((s32)(pData->LoopAnalog[i].LoopVoltage*DCMETER_DATA_RATIO));
			Offset += 4;
			//GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((s32)(pData->LoopAnalog[i].LoopCurrent*DCMETER_DATA_RATIO));
            FTemp = pData->LoopAnalog[i].LoopCurrent*DCMETER_DATA_RATIO;
            GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((int32_t)FTemp);
            
			Offset += 4;
			//GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((s32)(pData->LoopAnalog[i].LoopPower*DCMETER_DATA_RATIO));
            FTemp = pData->LoopAnalog[i].LoopPower*DCMETER_DATA_RATIO;
            GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((int32_t)FTemp);
            
			Offset += 4;
			//GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((s32)(pData->LoopAnalog[i].LoopEnergy*DCMETER_DATA_RATIO));
            FTemp = pData->LoopAnalog[i].LoopEnergy*DCMETER_DATA_RATIO;
            if(0 == IsRatio100)
                FTemp *= DCMETER_DATA_RATIO;
            GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((int32_t)FTemp);
			Offset += 4;
		}
		Prot.Length = Offset;
	}//��ȡָ����·ң������
	else{
		Offset = 0;
		Prot.DataBuf[Offset] = GetDataFlag(); 
		Offset += 1;
		GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->AcAnalog.AcVoltage*DCMETER_DATA_RATIO)); //������ѹ����Э���������壬��Ϊ1
		Offset += 4;
		Prot.DataBuf[Offset] = 1; 
		Offset += 1;
		{
			//Prot.DataBuf[Offset] = pData->LoopAnalog[i].LoopNum;
			//Offset += 1;
			GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((int32_t)(pData->LoopAnalog[LoopNum-1].LoopVoltage*DCMETER_DATA_RATIO));
			Offset += 4;
			GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((int32_t)(pData->LoopAnalog[LoopNum-1].LoopCurrent*DCMETER_DATA_RATIO));
			Offset += 4;
			GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((int32_t)(pData->LoopAnalog[LoopNum-1].LoopPower*DCMETER_DATA_RATIO));
			Offset += 4;
            if(0 != IsRatio100)
                GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((int32_t)(pData->LoopAnalog[LoopNum-1].LoopEnergy*DCMETER_DATA_RATIO));
            else
                GetDWord(&Prot.DataBuf[Offset]) = SWAP_LONG((int32_t)(pData->LoopAnalog[LoopNum-1].LoopEnergy*10000));
			Offset += 4;
		}
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_Alarm(const PDCMETER_ALARM pData,
	const uint8_t LoopNum,uint8_t *pBuf,uint32_t *pLen)
{
	uint8_t Offset,i;
	PROTOCAL_YDT1363_3 Prot;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;
	if(((LoopNum>DCMETER_LOOP_CNT_MAX)&&(LoopNum!=0xff))
		||(LoopNum == 0))
		return YDT1363_FUNC_RET_FALSE;
	if(pData->LoopCnt>DCMETER_LOOP_CNT_MAX)
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	//��ȡ���л�·ң������
	if(LoopNum==0xff){
		//Prot.Length = (uint8_t)(1+4+1+pData->LoopCnt*sizeof(&pData->LoopAnalog[0]));
		Offset = 0;
		Prot.DataBuf[Offset] = GetDataFlag(); 
		Offset += 1;
		Prot.DataBuf[Offset] = pData->AcVoltageAlarm; 
		Offset += 1;
		Prot.DataBuf[Offset] = pData->LoopCnt; 
		Offset += 1;
		for(i=0;i<pData->LoopCnt;i++){
			if(i>=DCMETER_LOOP_CNT_MAX)
				break;
			Prot.DataBuf[Offset] = pData->LoopAlarm[i].LoopNum; 
			Offset += 1;
			Prot.DataBuf[Offset] = pData->LoopAlarm[i].LoopDCVoltageLowAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[i].LoopDCVoltageHighAlarm; 
			Offset += 1;
			Prot.DataBuf[Offset] = pData->LoopAlarm[i].LoopFirstPowerDown; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[i].ModuleErrorAlarm; 
			Offset += 1;
			/*Prot.DataBuf[Offset] = pData->LoopAlarm[i].LoopLostVoltageAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[i].LoopMeasureModuleAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[i].InterProgramErrorAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[i].ClockErrorAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[i].MemFailureAlarm; */
		}
		Prot.Length = Offset;
	}//��ȡָ����·ң������
	else{
		Offset = 0;
		Prot.DataBuf[Offset] = GetDataFlag();
		Offset += 1;
		Prot.DataBuf[Offset] = pData->AcVoltageAlarm; 
		Offset += 1;
		Prot.DataBuf[Offset] = 1; 
		Offset += 1;
		{
			Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].LoopNum; 
			Offset += 1;
			Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].LoopDCVoltageLowAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].LoopDCVoltageHighAlarm; 
			Offset += 1;
			Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].LoopFirstPowerDown; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].ModuleErrorAlarm; 
			Offset += 1;
			/*Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].LoopLostVoltageAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].LoopMeasureModuleAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].InterProgramErrorAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].ClockErrorAlarm; 
			Offset += 1;
            Prot.DataBuf[Offset] = pData->LoopAlarm[LoopNum-1].MemFailureAlarm; 
			Offset += 1;*/
		}
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_ModuleParam(const PDCMETER_MODULE_PARAM pData,uint8_t *pBuf,uint32_t *pLen)
{
	uint8_t Offset;
	PROTOCAL_YDT1363_3 Prot;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	{
		Offset = 0;
        
		GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->DcHighVoltageThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;
		GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->DcLowVoltageThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;
		GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->DcFirstPowerDownThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;
		/*GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->LoopLostVoltageThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;
		GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->LoopMeasureModuleThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;
		GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->InterProgramErrorAlarm*DCMETER_DATA_RATIO)); 
		Offset += 4;
        GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->ClockErrorThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;
        GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->MemFailureThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;*/
        GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->ModuleAlarmThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;
        GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->AcPowerCutThreshold*DCMETER_DATA_RATIO)); 
		Offset += 4;
        GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD((uint32_t)(pData->AcVoltageRatio*DCMETER_DATA_RATIO)); 
		Offset += 4;
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}
static int32_t ProtocolProc_YDT1363_3_SetModuleParam(PDCMETER_MODULE_PARAM pSetData,PDCMETER_MODULE_PARAM pData)
{
	if((pSetData == NULL))
		return YDT1363_FUNC_RET_FALSE;


	if(pData != NULL){
		//memcpy(pData,pSetData,sizeof(DCMETER_MODULE_PARAM));
        if((pSetData->DcHighVoltageThreshold>=45.0)
            &&(pSetData->DcHighVoltageThreshold<=65.0))
            pData->DcHighVoltageThreshold = pSetData->DcHighVoltageThreshold;
        if((pSetData->DcLowVoltageThreshold<=54.0)
            &&(pSetData->DcLowVoltageThreshold>=40.0)
            &&(pSetData->DcLowVoltageThreshold<pData->DcHighVoltageThreshold))
            pData->DcLowVoltageThreshold = pSetData->DcLowVoltageThreshold;
        if((pSetData->DcFirstPowerDownThreshold<=54.0)
            &&(pSetData->DcFirstPowerDownThreshold>=35.0)
            &&(pSetData->DcFirstPowerDownThreshold<pData->DcLowVoltageThreshold))
            pData->DcFirstPowerDownThreshold = pSetData->DcFirstPowerDownThreshold;
        
        pData->ModuleAlarmThreshold = pSetData->ModuleAlarmThreshold;
        pData->AcPowerCutThreshold = pSetData->AcPowerCutThreshold;
        pData->AcVoltageRatio = pSetData->AcVoltageRatio;
	}
    
    return MeterData_WaitWriteSucc(1,(uint8_t *)pData,\
                AT24CXX_ADDR_MODULE_PARAM, \
                AT24CXX_ADDR_MODULE_PARAM_SIZE);
}

static int32_t ProtocolProc_YDT1363_3_Make_Param(const PDCMETER_PARAM pData,uint8_t *pBuf,uint32_t *pLen)
{
	//uint8_t Offset;
	//uint8_t	LenChecksum, ConvertBuf[2*YDT1363_3_DATABUF_MAX];
	//uint16_t	DataChecksum=0;
	//uint32_t	Len,Count;
	PROTOCAL_YDT1363_3 Prot;
    uint8_t Offset,Count;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

    /*
	Len =8*2+(12+6)*2; 
	LenChecksum		= (Len & 0xF) + ((Len >> 4) & 0xF) + ((Len >> 8) & 0xF);	
	LenChecksum		= LenChecksum % 16;
	LenChecksum		= (~LenChecksum + 1) & 0xF;
	Len				|= (LenChecksum << 12);
	
	
	ConvertBuf[0]	= YDT1363_3_PROTOCAL_VER;
	ConvertBuf[1]	= (uint8_t)GetDevAddr();
	ConvertBuf[2]	= YDT1363_3_PROTOCAL_CID1_DCMETER;
	ConvertBuf[3]	= YDT1363_3_PROTOCAL_RTN_NORMAL;
	GetWord(ConvertBuf + 4)	= SWAP_WORD(Len);
    
	pBuf[0]			= 0x7E;
	HexToAscii(ConvertBuf, &pBuf[1], 6, TRUE);
	memset(ConvertBuf,0x20,16);
	strncpy(ConvertBuf,pData->Vender,sizeof(pData->Vender));
	memcpy(&pBuf[13],ConvertBuf,16);
	
    GetDWord(&ConvertBuf[0]) = SWAP_DWORD(pData->Addr);
    GetDWord(&ConvertBuf[4]) = SWAP_DWORD(pData->Baudrate);
    GetDWord(&ConvertBuf[8]) = SWAP_DWORD(pData->SaveTime);

    for (Count = 0; Count < DCMETER_LOOP_CNT_MAX; Count ++)
		ConvertBuf[12+Count] = pData->LoopConfig[Count]; 
    HexToAscii(ConvertBuf, &pBuf[21+8], 18, TRUE);
    
	DataChecksum	= 0;
	for (Count = 0; Count < (uint32_t)(28 + 18*2); Count ++)
		DataChecksum	+= pBuf[1 + Count];

	DataChecksum			= ~DataChecksum + 1;
	GetWord(ConvertBuf)		= SWAP_WORD(DataChecksum);
	HexToAscii(ConvertBuf, &pBuf[29+ 18*2], 2, TRUE);
	
	pBuf[33+18*2]		= 0x0D;
    
	*pLen = 34+18*2;
	return YDT1363_FUNC_RET_SUCCESS;*/
    memset(&Prot,0x20,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
    Offset = 0;
    strncpy((void *)&Prot.DataBuf[Offset],(void *)pData->Vender,sizeof(pData->Vender));
    Offset += 8;
    GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD(pData->Addr);
    Offset += 4;
    GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD(pData->Baudrate);
    Offset += 4;
    GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD(pData->SaveTime);
    Offset += 4;
    for (Count = 0; Count < DCMETER_LOOP_CNT_MAX; Count ++){
        Prot.DataBuf[Offset] = pData->LoopConfig[Count];
        Offset += 1;        
    }
    Prot.Length = Offset;
    return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}





#define YDT1363_3_RET_HISTORY_NORMAL 0x0
#define YDT1363_3_RET_HISTORY_END 0x01

static int32_t ProtocolProc_YDT1363_3_Make_MonthEnergy(uint8_t Command,PDCMETER_DATA pData,uint8_t *pBuf,uint32_t *pLen)
{
    static uint32_t Count;//���ü���
	PROTOCAL_YDT1363_3 Prot;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
    Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
    
    if(Command==0x0){
        Count = 0;
        if(pData->SaveCnt.EnergyMonthCnt==1)
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
        else
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
        MeterMonthEnergy_GetRecord(pData->SaveCnt.EnergyMonthCnt,(PMonthEnergyData)&Prot.DataBuf[1]);
    }else{
        (Command==0x1)?++Count:Count;
        if(Count>(METER_SAVE_ENERGY_MONTH_CNT_MAX-1)){
            Count = 0;
            return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                        YDT1363_3_PROTOCAL_RTN_DATA_NONE,pBuf,pLen);
        }else if(Count==(pData->SaveCnt.EnergyMonthCnt-1)){
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
            MeterMonthEnergy_GetRecord(pData->SaveCnt.EnergyMonthCnt-Count,(PMonthEnergyData)&Prot.DataBuf[1]);
            //Count = 0;
        }else{
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
            MeterMonthEnergy_GetRecord(pData->SaveCnt.EnergyMonthCnt-Count,(PMonthEnergyData)&Prot.DataBuf[1]); 
        }
        
    }
    
    Prot.Length = 1+SAVE_ENERGY_BYTES-1;
	
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
    
}

static int32_t ProtocolProc_YDT1363_3_Make_MinuteData(uint8_t Command,PDCMETER_DATA pData,uint8_t *pBuf,uint32_t *pLen)
{
    static uint32_t Count;//���ü���
	PROTOCAL_YDT1363_3 Prot;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
    Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
    
    if(Command==0x10){
        Count = 0;
        if(pData->SaveCnt.MinuteDataCnt==1)
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
        else
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
        MeterMinuteData_GetRecord(pData->SaveCnt.MinuteDataCnt,(PSaveHistoryData)&Prot.DataBuf[1]);
    }else{
        (Command==0x11)?++Count:Count;
        if(Count>(METER_SAVE_DATA_MINUTE_CNT_MAX-1)){
            Count = 0;
            return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                        YDT1363_3_PROTOCAL_RTN_DATA_NONE,pBuf,pLen);
        }else if(Count==(pData->SaveCnt.MinuteDataCnt-1)){
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
            MeterMinuteData_GetRecord(pData->SaveCnt.MinuteDataCnt-Count,(PSaveHistoryData)&Prot.DataBuf[1]);
        }else{
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
            MeterMinuteData_GetRecord(pData->SaveCnt.MinuteDataCnt-Count,(PSaveHistoryData)&Prot.DataBuf[1]);  
        }     
    }
    
    Prot.Length = 1+SAVE_DATA_MINUTE_BYTES-1;
	
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
    
}

static int32_t ProtocolProc_YDT1363_3_Make_HourData(uint8_t Command,PDCMETER_DATA pData,uint8_t *pBuf,uint32_t *pLen)
{
    static uint32_t Count;//���ü���
	PROTOCAL_YDT1363_3 Prot;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
    Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
    
    if(Command==0){
        Count = 0;
        if(pData->SaveCnt.HourDataCnt==1)
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
        else
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
        MeterHourData_GetRecord(pData->SaveCnt.HourDataCnt,(PSaveHistoryData)&Prot.DataBuf[1]);
    }else{
        (Command==0x01)?++Count:Count;
        if(Count>(METER_SAVE_DATA_HOUR_CNT_MAX-1)){
            Count = 0;
            return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                        YDT1363_3_PROTOCAL_RTN_DATA_NONE,pBuf,pLen);
        }else if(Count==(pData->SaveCnt.HourDataCnt-1)){
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
            MeterHourData_GetRecord(pData->SaveCnt.HourDataCnt-Count,(PSaveHistoryData)&Prot.DataBuf[1]); 
        }else{
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
            MeterHourData_GetRecord(pData->SaveCnt.HourDataCnt-Count,(PSaveHistoryData)&Prot.DataBuf[1]);
        }          
    }
    
    Prot.Length = 1+SAVE_DATA_HOUR_BYTES-1;
	
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
    
}

static int32_t ProtocolProc_YDT1363_3_Make_MonthData(uint8_t Command,PDCMETER_DATA pData,uint8_t *pBuf,uint32_t *pLen)
{
    static uint32_t Count;//���ü���
	PROTOCAL_YDT1363_3 Prot;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
    Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
    
    if(Command==0x20){
        Count = 0;
        if(pData->SaveCnt.MonthDataCnt==1)
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
        else
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
        MeterMonthData_GetRecord(pData->SaveCnt.MonthDataCnt,(PSaveHistoryData)&Prot.DataBuf[1]);
    }else{
        (Command==0x21)?++Count:Count;
        if(Count>(METER_SAVE_DATA_MONTH_CNT_MAX-1)){
            Count = 0;
            return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                        YDT1363_3_PROTOCAL_RTN_DATA_NONE,pBuf,pLen);
        }else if(Count==(pData->SaveCnt.MonthDataCnt-1)){
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
            MeterMonthData_GetRecord(pData->SaveCnt.MonthDataCnt-Count,(PSaveHistoryData)&Prot.DataBuf[1]);
        }else{
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
            MeterMonthData_GetRecord(pData->SaveCnt.MonthDataCnt-Count,(PSaveHistoryData)&Prot.DataBuf[1]);
        }        
    }
    
    Prot.Length = 1+SAVE_DATA_MONTH_BYTES-1;
	
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);  
}

static int32_t ProtocolProc_YDT1363_3_Make_HistoryAlarm(uint8_t Command,PDCMETER_DATA pData,uint8_t *pBuf,uint32_t *pLen)
{
    static uint32_t Count;//���ü���
	PROTOCAL_YDT1363_3 Prot;
    
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
    Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
    
    if(Command==0x0){
        Count = 0;
        if(pData->SaveCnt.AlarmCnt==1)
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
        else
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
        Prot.DataBuf[1] = GetDataFlag();
        MeterAlarm_GetRecord(pData->SaveCnt.AlarmCnt,(PSaveHistoryAlarm)&Prot.DataBuf[2]);
    }else{
        (Command==0x01)?++Count:Count;
        if(Count>(METER_SAVE_ALARM_CNT_MAX-1)){
            Count = 0;
            return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                        YDT1363_3_PROTOCAL_RTN_DATA_NONE,pBuf,pLen);
        }else if(Count==(pData->SaveCnt.AlarmCnt-1)){
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_END;
            Prot.DataBuf[1] = GetDataFlag();
            MeterAlarm_GetRecord(pData->SaveCnt.AlarmCnt-Count,(PSaveHistoryAlarm)&Prot.DataBuf[2]);
        }else{
            Prot.DataBuf[0] = YDT1363_3_RET_HISTORY_NORMAL;
            Prot.DataBuf[1] = GetDataFlag();
            MeterAlarm_GetRecord(pData->SaveCnt.AlarmCnt-Count,(PSaveHistoryAlarm)&Prot.DataBuf[2]);
        }        
    }
    
    Prot.Length = 2+SAVE_ALARM_BYTES-1;//ȥ��PSaveHistoryAlarm->CheckSum
	
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);  
}

static int32_t ProtocolProc_YDT1363_3_SetParam(PDCMETER_PARAM pSetData,PDCMETER_PARAM pData)
{
    uint8_t i;
	if((pSetData == NULL)||(pData == NULL))
		return YDT1363_FUNC_RET_FALSE;


	if(pData != NULL){
		//memcpy(pData,pSetData,sizeof(DCMETER_PARAM));
        memcpy(pData->Vender,pSetData->Vender,sizeof(pData->Vender));
        if((pSetData->Addr&0x000000ff)!=0)
            pData->Addr = pSetData->Addr&0x000000ff;
        if((pSetData->Baudrate==1200)
            ||(pSetData->Baudrate==2400)
            ||(pSetData->Baudrate==4800)
            ||(pSetData->Baudrate==9600)
            ||(pSetData->Baudrate==19200))
            pData->Baudrate = pSetData->Baudrate;
        pData->SaveTime = pSetData->SaveTime;
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
            if(pSetData->LoopConfig[i]<=0x06)
                pData->LoopConfig[i] = pSetData->LoopConfig[i];
        }
        
	}
    return MeterData_WaitWriteSucc(1,(uint8_t *)pData,AT24CXX_ADDR_PARAM,AT24CXX_ADDR_PARAM_SIZE);
}
static int32_t ProtocolProc_YDT1363_3_Make_Vender(uint8_t *pBuf,uint32_t *pLen)
{
	//uint8_t	LenChecksum,ConvertBuf[2*YDT1363_3_DATABUF_MAX];
	//uint16_t	DataChecksum=0;
	//uint32_t	Len,Count;
	PROTOCAL_YDT1363_3 Prot;
	
	if((pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;
	/*
	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	Prot.Length = 30+20+20;

	Len = Prot.Length;
	LenChecksum		= (Len & 0xF) + ((Len >> 4) & 0xF) + ((Len >> 8) & 0xF);	
	LenChecksum		= LenChecksum % 16;
	LenChecksum		= (~LenChecksum + 1) & 0xF;
	Len				|= (LenChecksum << 12);
	
	
	ConvertBuf[0]	= Prot.Ver;
	ConvertBuf[1]	= Prot.Addr;
	ConvertBuf[2]	= Prot.CID1;
	ConvertBuf[3]	= Prot.CID2;
	GetWord(ConvertBuf + 4)	= SWAP_WORD(Len);

	pBuf[0]			= 0x7E;
	HexToAscii(ConvertBuf, &pBuf[1], 6, TRUE);
	memset(ConvertBuf,0x20,Prot.Length);
	strncpy(ConvertBuf,UNIT_TYPE,strlen(UNIT_TYPE));
	strncpy(&ConvertBuf[30],SOFTWARE_VERSION,strlen(SOFTWARE_VERSION));
	strncpy(&ConvertBuf[50],VENDER,strlen(VENDER));
	memcpy(&pBuf[13],ConvertBuf,Prot.Length);
	
	DataChecksum	= 0;
	for (Count = 0; Count < (uint32_t)(12 + Prot.Length); Count ++)
		DataChecksum	+= pBuf[1 + Count];

	DataChecksum			= ~DataChecksum + 1;
	GetWord(ConvertBuf)		= SWAP_WORD(DataChecksum);
	HexToAscii(ConvertBuf, &pBuf[13 + Prot.Length], 2, TRUE);
	
	pBuf[17 + Prot.Length]		= 0x0D;
	*pLen	= 18 + Prot.Length;
	return YDT1363_FUNC_RET_SUCCESS;*/
    memset(&Prot,0x20,sizeof(PROTOCAL_YDT1363_3));
    Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
    Prot.Length = 0;
    strncpy((void *)&Prot.DataBuf[Prot.Length],UNIT_TYPE,strlen(UNIT_TYPE));
    Prot.Length += 30;
    strncpy((void *)&Prot.DataBuf[Prot.Length],SOFTWARE_VERSION,strlen(SOFTWARE_VERSION));
    Prot.Length += 20;
    strncpy((void *)&Prot.DataBuf[Prot.Length],VENDER,strlen(VENDER));
    Prot.Length += 20;
    
    return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_Date(const PDCMETER_DATE pData,uint8_t *pBuf,uint32_t *pLen)
{
	uint8_t Offset;
	PROTOCAL_YDT1363_3 Prot;
	
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	{
		Offset = 0;
		GetWord(&Prot.DataBuf[Offset]) = SWAP_WORD((uint16_t)pData->Year); 
		Offset += 2;
		Prot.DataBuf[Offset] = pData->Month; 
		Offset += 1;
		Prot.DataBuf[Offset] = pData->Day; 
		Offset += 1;
		Prot.DataBuf[Offset] = pData->Hour; 
		Offset += 1;
		Prot.DataBuf[Offset] = pData->Minute; 
		Offset += 1;
		Prot.DataBuf[Offset] = pData->Second; 
		Offset += 1;
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}
//pDataΪNULLʱ�����������ʱ�����Ч��
static int32_t ProtocolProc_YDT1363_3_SetDate(const PDCMETER_DATE pSetData,PDCMETER_DATE pData)
{
	if((pSetData == NULL))
		return YDT1363_FUNC_RET_FALSE;
	if(!DATE_YEAR_IS_VALID(pSetData->Year))
		return YDT1363_FUNC_RET_FALSE;
	if(!DATE_MONTH_IS_VALID(pSetData->Month))
		return YDT1363_FUNC_RET_FALSE;
	if(!DATE_DAY_IS_VALID(pSetData->Day))
		return YDT1363_FUNC_RET_FALSE;
	if(!DATE_HOUR_IS_VALID(pSetData->Hour))
		return YDT1363_FUNC_RET_FALSE;
	if(!DATE_MINUTE_OR_SECOND_IS_VALID(pSetData->Minute))
		return YDT1363_FUNC_RET_FALSE;
	if(!DATE_MINUTE_OR_SECOND_IS_VALID(pSetData->Second))
		return YDT1363_FUNC_RET_FALSE;

	if(pData != NULL){
		memcpy(pData,pSetData,sizeof(DCMETER_DATE));
	}
	return YDT1363_FUNC_RET_SUCCESS;
}


static int32_t ProtocolProc_YDT1363_3_Make_Reading(const PDCMETER_READING pData,uint8_t *pBuf,uint32_t *pLen)
{
    uint8_t Offset;
	PROTOCAL_YDT1363_3 Prot;
	
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	{
		Offset = 0;
		Prot.DataBuf[Offset] = pData->Day; 
		Offset += 1;
		Prot.DataBuf[Offset] = pData->Hour; 
		Offset += 1;
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_CS5460A_DC_MAX(
    const PDCMETER_CS5460_DC_MAX pData,
    uint8_t *pBuf,
    uint32_t *pLen)
{
    uint8_t Offset;
    uint32_t u32Temp;
	PROTOCAL_YDT1363_3 Prot;
	
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	{
		Offset = 0;
        u32Temp = pData->Imax*1000;
		GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD(u32Temp); 
		Offset += 4;
        u32Temp = pData->Vmax*1000;
		GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD(u32Temp); 
		Offset += 4;
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}
static int32_t ProtocolProc_YDT1363_3_Make_CT(
    const PDCMETER_CT pData,
    uint8_t *pBuf,
    uint32_t *pLen)
{
    uint8_t Offset;
    uint32_t u32Temp,i;
	PROTOCAL_YDT1363_3 Prot;
	
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	{
		Offset = 0;
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
            u32Temp = pData->CT[i]*100;
            GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD(u32Temp); 
            Offset += 4;
        }
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_SectCali(uint16_t *pCnt,uint8_t *pBuf,uint32_t *pLen)
{
    uint8_t Offset,i;
	PROTOCAL_YDT1363_3 Prot;
	
	if((pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	{
		Offset = 0;
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
            Prot.DataBuf[Offset] = (uint8_t)pCnt[i]; 
            Offset += 1;
        }
		
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_CS5460CaliData(uint8_t *CaliData,uint32_t CaliDataLen,uint8_t *pBuf,uint32_t *pLen)
{
    uint8_t Offset;
	PROTOCAL_YDT1363_3 Prot;
	
	if((pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	{
		Offset = 0;
        memcpy(Prot.DataBuf,CaliData,CaliDataLen);
		Offset += CaliDataLen;
		
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_EgyRatio(
    const PDCMETER_ENERGY_RATIO pData,
    uint8_t *pBuf,
    uint32_t *pLen)
{
    uint8_t Offset;
    uint32_t u32Temp,i;
	PROTOCAL_YDT1363_3 Prot;
	
	if((pData == NULL)||(pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;

	//memset(&Prot,0,sizeof(PROTOCAL_YDT1363_3));
	Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
	{
		Offset = 0;
        for(i=0;i<DCMETER_LOOP_CNT_MAX;i++){
            u32Temp = pData->Value[i]*1000;
            GetDWord(&Prot.DataBuf[Offset]) = SWAP_DWORD(u32Temp); 
            Offset += 4;
        }
		Prot.Length = Offset;
	}
	return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

static int32_t ProtocolProc_YDT1363_3_Make_SoftWare(uint8_t *pBuf,uint32_t *pLen)
{
	PROTOCAL_YDT1363_3 Prot;
	
	if((pBuf == NULL)||(pLen == NULL))
		return YDT1363_FUNC_RET_FALSE;
	
    memset(&Prot,0x20,sizeof(PROTOCAL_YDT1363_3));
    Prot.Addr = (uint8_t)GetDevAddr();
	Prot.Ver = YDT1363_3_PROTOCAL_VER;
	Prot.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
	Prot.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
    Prot.Length = 0;
    strncpy((void *)&Prot.DataBuf[Prot.Length],USER_DEFINE_SOFT_VERSION,strlen(USER_DEFINE_SOFT_VERSION));
    Prot.Length += strlen(USER_DEFINE_SOFT_VERSION);
    
    return ProtocolProc_YDT1363_3_Make(&Prot,pBuf,pLen);
}

int FMC_Read_User(unsigned int u32Addr, unsigned int * data)
{
    unsigned int Reg;
    FMC->ISPCMD = FMC_ISPCMD_READ;
    FMC->ISPADR = u32Addr;
    FMC->ISPDAT = 0;
    FMC->ISPTRG = 0x1;

    __ISB();

    Reg = FMC->ISPCON;

    if(Reg & FMC_ISPCON_ISPFF_Msk) {
        FMC->ISPCON = Reg;
        return -1;
    }

    *data = FMC->ISPDAT;
    return 0;
}

uint32_t GetApromSize(void)
{
    uint32_t size = 0x4000, data;
    int result;

    do {
        result = FMC_Read_User(size, &data);
        if(result < 0) {
            return size;
        } else
            size *= 2;
    } while(1);
}
/*
 ************************************************************************************************************************************************************************     
 *��������:	ProtocolProc_YDT1363_3_ProtocolPocess
 *��������:	Э�鴦�����
 *��������:	
 *�������:	
 *��������:	
 *��������:	
 *ȫ������: 
 *����˵��: 
 ************************************************************************************************************************************************************************       
 */
extern int32_t ProtocolProc_YDT1363_3_ProtocolPocess(const uint8_t *pRecvBuf, uint32_t *pRecvLen, uint8_t *pSendBuf, uint32_t *pSendLen)
{
	int32_t Ret;
	uint8_t Rtn,LoopNum;
    PPM5KT_COMM_TM pTmEx=NULL;
    PDCMETER_PARAM pParam=NULL;
	PROTOCAL_YDT1363_3 ProcBuf;
    uint8_t Temp[200];
    uint32_t TempLen=0;
    uint32_t  U32Temp,Count;
    //uint8_t LastLoopConfig[DCMETER_LOOP_CNT_MAX];//��¼��·���ñ��޸�ǰ������
    
	if((pRecvBuf==NULL)||(pRecvLen==NULL)
		||(pSendBuf==NULL)||(pSendLen==NULL))
		return YDT1363_FUNC_RET_FALSE;
	
	//memset(&AckMsg,0,sizeof(DCMETER_PROC_DATA));
	//����Э��
	Ret = ProtocolProc_YDT1363_3_Parse(&ProcBuf,pRecvBuf,pRecvLen);
	//���ܵİ��д���
	if((Ret == YDT1363_3_PROTOCAL_RTN_LCHKSUM_ERROR)
		||(Ret == YDT1363_3_PROTOCAL_RTN_CHKSUM_ERROR)
		||(Ret == YDT1363_3_PROTOCAL_RTN_FORMAT_ERROR)){
		return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame((uint32_t)Ret,pSendBuf,pSendLen);
	}//����ʽ��ȷ
	else if(Ret == YDT1363_FUNC_RET_SUCCESS){
        gu8Prot_CID2 = ProcBuf.CID2;
		//����У���ַ
		if(ProcBuf.Addr == (uint8_t)GetDevAddr()){
			//�����������,VER��CID1����Ϊ����ֵ��������:��ȡѶЭ��汾��
			if(ProcBuf.Ver == (uint8_t)YDT1363_3_PROTOCAL_VER){
				//����CID1����
				if(ProcBuf.CID1 == YDT1363_3_PROTOCAL_CID1_DCMETER){
					//���CID2����
					switch(ProcBuf.CID2)
					{
						//
						case YDT1363_3_PROTOCAL_CID2_GET_ANALOG:
                        case YDT1363_3_PROTOCAL_CID2_GET_ANALOG_EXT:
							if((ProcBuf.Length==1)
                                &&((ProcBuf.DataBuf[0]<DCMETER_LOOP_CNT_MAX)||(ProcBuf.DataBuf[0]==0xff))){
                                if(ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_GET_ANALOG_EXT)
                                    return ProtocolProc_YDT1363_3_Make_Anolog(0,&gDCMeterData.AnalogData,
                                        ProcBuf.DataBuf[0],pSendBuf,pSendLen);
                                else
                                    return ProtocolProc_YDT1363_3_Make_Anolog(1,&gDCMeterData.AnalogData,
                                        ProcBuf.DataBuf[0],pSendBuf,pSendLen);
							}
							break;
						case YDT1363_3_PROTOCAL_CID2_GET_ALARM:
							if((ProcBuf.Length==1)
                                &&((ProcBuf.DataBuf[0]<DCMETER_LOOP_CNT_MAX)||(ProcBuf.DataBuf[0]==0xff))){
								return ProtocolProc_YDT1363_3_Make_Alarm(&gDCMeterData.AlarmData,
									ProcBuf.DataBuf[0],pSendBuf,pSendLen);
							}
							break;
						//��ȡ��ʷ����	
						case YDT1363_3_PROTOCAL_CID2_GET_HISTORY_DATA:
                            if((ProcBuf.Length==1)&&(ProcBuf.DataBuf[0]<=2)){
                                if(gDCMeterData.SaveCnt.HourDataCnt==0){
                                    Rtn = YDT1363_3_PROTOCAL_RTN_DATA_NONE;
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
										Rtn,pSendBuf,pSendLen);
                                }else
                                    return ProtocolProc_YDT1363_3_Make_HourData(ProcBuf.DataBuf[0],\
                                                &gDCMeterData,pSendBuf,pSendLen);
							}else if((ProcBuf.Length==1)&&(ProcBuf.DataBuf[0]>=0x10)&&(ProcBuf.DataBuf[0]<=0x12)){
                                if(gDCMeterData.SaveCnt.MinuteDataCnt==0){
                                    Rtn = YDT1363_3_PROTOCAL_RTN_DATA_NONE;
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
										Rtn,pSendBuf,pSendLen);
                                }else
                                    return ProtocolProc_YDT1363_3_Make_MinuteData(ProcBuf.DataBuf[0],\
                                                &gDCMeterData,pSendBuf,pSendLen);
							}else if((ProcBuf.Length==1)&&(ProcBuf.DataBuf[0]>=0x20)&&(ProcBuf.DataBuf[0]<=0x22)){
                                if(gDCMeterData.SaveCnt.MonthDataCnt==0){
                                    Rtn = YDT1363_3_PROTOCAL_RTN_DATA_NONE;
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
										Rtn,pSendBuf,pSendLen);
                                }else
                                    return ProtocolProc_YDT1363_3_Make_MonthData(ProcBuf.DataBuf[0],\
                                                &gDCMeterData,pSendBuf,pSendLen);
							}
							else{
								
							} 
                            break;
                        //��ȡ��ʷ�澯
						case YDT1363_3_PROTOCAL_CID2_GET_HISTORY_ALARM:
							if((ProcBuf.Length==1)&&(ProcBuf.DataBuf[0]<=2)){
                                if(gDCMeterData.SaveCnt.AlarmCnt==0){
                                    Rtn = YDT1363_3_PROTOCAL_RTN_DATA_NONE;
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
										Rtn,pSendBuf,pSendLen);
                                }else
                                    return ProtocolProc_YDT1363_3_Make_HistoryAlarm(ProcBuf.DataBuf[0],\
                                                &gDCMeterData,pSendBuf,pSendLen);
							}
							break;
                            
                        //��ȡ�¶������
                        case YDT1363_3_PROTOCAL_CID2_GET_ENERGY_MONTH:
							if((ProcBuf.Length==1)&&(ProcBuf.DataBuf[0]<=2)){
                                if(gDCMeterData.SaveCnt.EnergyMonthCnt==0){
                                    Rtn = YDT1363_3_PROTOCAL_RTN_DATA_NONE;
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
										Rtn,pSendBuf,pSendLen);
                                }else
                                    return ProtocolProc_YDT1363_3_Make_MonthEnergy(ProcBuf.DataBuf[0],\
                                                &gDCMeterData,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;    
                            
                        //��ȡ�����
                        case YDT1363_3_PROTOCAL_CID2_GET_PARAM:
							if(ProcBuf.Length==0){
								return ProtocolProc_YDT1363_3_Make_Param(\
										&gDCMeterData.Param,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;    
                        //���ñ����
                        case YDT1363_3_PROTOCAL_CID2_SET_PARAM:
							if(ProcBuf.Length>=22){
                                //DCMETER_PARAM Param;
                                pParam = (PDCMETER_PARAM)Temp;
                                //for (Count = 0; Count < DCMETER_LOOP_CNT_MAX; Count ++)
                                //    LastLoopConfig[Count] = gDCMeterData.Param.LoopConfig[Count];
                                //��ֹ������Ϣ���޸�
                                memset(pParam->Vender,0x20,sizeof(pParam->Vender));
                                memcpy(pParam->Vender,DCMETER_VENDER,sizeof(DCMETER_VENDER)-1);
                                pParam->Addr = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[8]));
								pParam->Baudrate = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[12]));
								pParam->SaveTime = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[16]));
								/*Param.Config_Loop1 = ProcBuf.DataBuf[20];
                                Param.Config_Loop2 = ProcBuf.DataBuf[21];
                                Param.Config_Loop3 = ProcBuf.DataBuf[22];
                                Param.Config_Loop4 = ProcBuf.DataBuf[23];
                                Param.Config_Loop5 = ProcBuf.DataBuf[24];
                                Param.Config_Loop6 = ProcBuf.DataBuf[25];*/
                                for (Count = 0; Count < DCMETER_LOOP_CNT_MAX; Count ++)
                                    pParam->LoopConfig[Count] = ProcBuf.DataBuf[20+Count];
                                /*
                                for (Count = 0; Count < DCMETER_LOOP_CNT_MAX; Count ++){
                                    if(gDCMeterData.Param.LoopConfig[Count] != Param.LoopConfig[Count]){
                                        //��δʵ�������ڼ��޸Ļ�·�û���ת��ԭ��·�û�����
                                        //MeterGetUserTotalEnergy(&gDCMeterData.Param,Param.LoopConfig[Count],&gDCMeterData.TempEnergy,&NowEnergy);
                                    } 
                                }
                                */
                                
								if(0==ProtocolProc_YDT1363_3_SetParam(pParam,&gDCMeterData.Param))
									return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
											YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
                                else
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
											YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL,pSendBuf,pSendLen);
							
							}
							else{
								
							}
							break;    
						//��ȡ����ģ�����������Ϣ
						case YDT1363_3_PROTOCAL_CID2_GET_MODULE_PARAM:
							if(ProcBuf.Length==0){
								return ProtocolProc_YDT1363_3_Make_ModuleParam(\
										&gDCMeterData.ModuleParam,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;	
						//���ò���ģ�����
						case YDT1363_3_PROTOCAL_CID2_SET_MODULE_PARAM:
							if(ProcBuf.Length>=24){
								DCMETER_MODULE_PARAM ParamTemp;
                                
								ParamTemp.DcHighVoltageThreshold = (float)SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[0]))/DCMETER_DATA_RATIO;
								ParamTemp.DcLowVoltageThreshold = (float)SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[4]))/DCMETER_DATA_RATIO;
								ParamTemp.DcFirstPowerDownThreshold = (float)SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[8]))/DCMETER_DATA_RATIO;
								/*ParamTemp.LoopLostVoltageThreshold = (float)GetDWord(&ProcBuf.DataBuf[12])/DCMETER_DATA_RATIO;
								ParamTemp.LoopMeasureModuleThreshold = (float)GetDWord(&ProcBuf.DataBuf[16])/DCMETER_DATA_RATIO;
								ParamTemp.InterProgramErrorAlarm = (float)GetDWord(&ProcBuf.DataBuf[20])/DCMETER_DATA_RATIO;
                                ParamTemp.ClockErrorThreshold = (float)GetDWord(&ProcBuf.DataBuf[24])/DCMETER_DATA_RATIO;
                                ParamTemp.MemFailureThreshold = (float)GetDWord(&ProcBuf.DataBuf[28])/DCMETER_DATA_RATIO;
                                */
                                ParamTemp.ModuleAlarmThreshold = (float)SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[12]))/DCMETER_DATA_RATIO;
                                ParamTemp.AcPowerCutThreshold = (float)SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[16]))/DCMETER_DATA_RATIO;
                                ParamTemp.AcVoltageRatio = (float)SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[20]))/DCMETER_DATA_RATIO;
                                
								if(0==ProtocolProc_YDT1363_3_SetModuleParam(&ParamTemp,&gDCMeterData.ModuleParam))
									return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
											YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
                                else
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
											YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;
                        //��ȡ������
						case YDT1363_3_PROTOCAL_CID2_GET_METER_READING:
							if(ProcBuf.Length==0){
								return ProtocolProc_YDT1363_3_Make_Reading(\
										&gDCMeterData.Reading,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;
                        //���ó�����
						case YDT1363_3_PROTOCAL_CID2_SET_METER_READING:
							if(ProcBuf.Length==2){
								if((!DATE_DAY_IS_VALID(ProcBuf.DataBuf[0]))
                                    ||(!DATE_HOUR_IS_VALID(ProcBuf.DataBuf[1])))
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
											YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL,pSendBuf,pSendLen);
                                else{
                                    gDCMeterData.Reading.Day = ProcBuf.DataBuf[0];
                                    gDCMeterData.Reading.Hour = ProcBuf.DataBuf[1];
                                    MeterData_WaitWriteSucc(1,(uint8_t *)&gDCMeterData.Reading,AT24CXX_ADDR_READING, \
                                        AT24CXX_ADDR_READING_SIZE);
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
											YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
                                }    
							}
							else{
								
							}
							break;    
                        case YDT1363_3_PROTOCAL_CID2_FACTORY_RESET:
                            if(ProcBuf.Length==0){
                                
                                Rtn = YDT1363_3_PROTOCAL_RTN_NORMAL;
								return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
										Rtn,pSendBuf,pSendLen);
							}
							else{
							}
                            break;  
                        /**/
                        case YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_OFF:
                        case YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_GN:
                        //case YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_P:    
                            if(ProcBuf.Length==0){
                                if(ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_OFF){
                                    RN8209C_Calibration_OFF_ALL(&gDCMeterData.Rn8209c[0]);
                                    ADCCalibration(0,2.5,&gDCMeterData.AdcRatio);
                                }else if(ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_CS5460A_Calibration_GN){
                                    RN8209C_Calibration_GN_ALL(&gDCMeterData.Rn8209c[0]);
                                    ADCCalibration(1,5,&gDCMeterData.AdcRatio);
                                    /*RN8209C_Select(1);
                                    Comm_DelayUs(50000);
                                    RN8209C_DCGainCalibration(&gDCMeterData.Rn8209c[0]);
                                    RN8209C_Select(2);
                                    Comm_DelayUs(50000);
                                    RN8209C_DCGainCalibration(&gDCMeterData.Rn8209c[1]);
                                    RN8209C_Select(3);
                                    Comm_DelayUs(50000);
                                    RN8209C_DCGainCalibration(&gDCMeterData.Rn8209c[2]);
                                    RN8209C_Select(0);//��������Ƭѡ
                                    */
                                    WriteRn8209CalidataToFlash(&gDCMeterData.Rn8209c[0]);
                                    MeterData_WaitWriteSucc(1,
                                    (uint8_t *)&gDCMeterData.AdcRatio,
                                    AT24CXX_ADDR_ADC_RATIO,
                                    AT24CXX_ADDR_ADC_RATIO_SIZE);
                                }
                                    
                                //else
                                //    CS5460A_Calibration_P_ALL();
                                Rtn = YDT1363_3_PROTOCAL_RTN_NORMAL;
								return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
										Rtn,pSendBuf,pSendLen);
							}
							else{
								
							}
                            break; 
                        //��ȡУ׼Դ��ѹ�͵���������
						case YDT1363_3_PROTOCAL_CID2_GET_CS5460A_DC_MAX:
							if((ProcBuf.Length==0)){
								return ProtocolProc_YDT1363_3_Make_CS5460A_DC_MAX(\
										&gDCMeterData.Cs5460Dc,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;
                        //����У׼Դ��ѹ�͵���������
						case YDT1363_3_PROTOCAL_CID2_SET_CS5460A_DC_MAX:
							if(ProcBuf.Length==8){
                               
                                gDCMeterData.Cs5460Dc.Imax = (float)SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[0]))/1000.0;
                                gDCMeterData.Cs5460Dc.Vmax = (float)SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[4]))/1000.0;
                                MeterData_WaitWriteSucc(1,
                                    (uint8_t *)&gDCMeterData.Cs5460Dc.Imax,
                                    AT24CXX_ADDR_PARAM_CS5460_DC_I,
                                    AT24CXX_ADDR_PARAM_CS5460_DC_I_SIZE);
                                MeterData_WaitWriteSucc(1,
                                    (uint8_t *)&gDCMeterData.Cs5460Dc.Vmax,
                                    AT24CXX_ADDR_PARAM_CS5460_DC_V,
                                    AT24CXX_ADDR_PARAM_CS5460_DC_V_SIZE);
                                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
                                    
							}
							else{
								
							}
							break;
                        //4d
                        case YDT1363_3_PROTOCAL_CID2_GET_DATE:
                            if(ProcBuf.Length==0){
                                return ProtocolProc_YDT1363_3_Make_Date(&gDCMeterData.Date,pSendBuf,pSendLen);
							}
							else{
								
							}
                            break;
                        //50
                        case YDT1363_3_PROTOCAL_CID2_GET_DEVICE_ADDR:
                            if(ProcBuf.Length==0){
                                Rtn = YDT1363_3_PROTOCAL_RTN_NORMAL;
                                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                    Rtn,pSendBuf,pSendLen);
							}
							else{
								
							}
                            break;
                        //4f
                        case YDT1363_3_PROTOCAL_CID2_GET_VERSION:
                            if(ProcBuf.Length==0){
                                Rtn = YDT1363_3_PROTOCAL_RTN_NORMAL;
                                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                    Rtn,pSendBuf,pSendLen);
							}
							else{
								
							}
                            break;
                        //51
                        case YDT1363_3_PROTOCAL_CID2_GET_VENDER:
                            if(ProcBuf.Length==0){
                                return ProtocolProc_YDT1363_3_Make_Vender(pSendBuf,pSendLen);
							}
							else{
								
							}
                            break; 
                            
                        //4e
                        case YDT1363_3_PROTOCAL_CID2_SET_DATE:
                            if(ProcBuf.Length==7){
                                pTmEx = (PPM5KT_COMM_TM)Temp;
                                pTmEx->Year = SWAP_WORD(GetWord(&ProcBuf.DataBuf[0]));
                                pTmEx->Month = ProcBuf.DataBuf[2];
                                pTmEx->Day = ProcBuf.DataBuf[3];
                                pTmEx->Hour = ProcBuf.DataBuf[4];
                                pTmEx->Minute = ProcBuf.DataBuf[5];
                                pTmEx->Second = ProcBuf.DataBuf[6];
                                
                                if(0 == ProtocolProc_YDT1363_3_SetDate((PDCMETER_DATE)pTmEx,NULL)){
                                    //RX8010_SetTime(pTmEx);
                                    Rtn = ((RX8010_SetTime(pTmEx)==1)?YDT1363_3_PROTOCAL_RTN_NORMAL:YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL);
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        Rtn,pSendBuf,pSendLen);
                                }else{
                                    Rtn = YDT1363_3_PROTOCAL_RTN_DATA_INVALID;
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        Rtn,pSendBuf,pSendLen);
                                }
							}
							else{
								
							}
                            break; 
                        //��ȡ6��ͨ����CTֵ
						case YDT1363_3_PROTOCAL_CID2_GET_CT:
							if((ProcBuf.Length==0)){
								return ProtocolProc_YDT1363_3_Make_CT(\
										&gDCMeterData.CTValue,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;
                        //����6��ͨ����CTֵ
						case YDT1363_3_PROTOCAL_CID2_SET_CT:
                            //��������ͨ��
							if((ProcBuf.Length>=25)&&(ProcBuf.DataBuf[0]==0xff)){
                                for(Count=0;Count<DCMETER_LOOP_CNT_MAX;Count++){
                                    U32Temp = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[1+4*Count]));
                                    gDCMeterData.CTValue.CT[Count] = (float)U32Temp/100.0;
                                    MeterData_WaitWriteSucc(1,
                                        (uint8_t *)&gDCMeterData.CTValue.CT[Count],
                                        AT24CXX_ADDR_CT1+5*Count,
                                        AT24CXX_ADDR_CT1_SIZE);
                                }
                                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);    
							}
							else if((ProcBuf.Length>=5)&&(ProcBuf.DataBuf[0]>=0x01)&&(ProcBuf.DataBuf[0]<=DCMETER_LOOP_CNT_MAX)){
                                U32Temp = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[1]));
                                Count = ProcBuf.DataBuf[0]-1;
                                gDCMeterData.CTValue.CT[Count] = (float)U32Temp/100.0;
                                MeterData_WaitWriteSucc(1,
                                    (uint8_t *)&gDCMeterData.CTValue.CT[Count],
                                    AT24CXX_ADDR_CT1+5*Count,
                                    AT24CXX_ADDR_CT1_SIZE);
                                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
							}
                            else{
                            
                            }
							break;

                        //��������
						case YDT1363_3_PROTOCAL_CID2_POWER_RESET:
							if((ProcBuf.Length==0)){
                                memset(&gDCMeterData.TempEnergy,0,sizeof(DCMETER_TEMP_ENERGY));
                                memset(&gDCMeterData.EgyReading,0,sizeof(DCMETER_ENERGY_READING));
                                for(Count=0;Count<DCMETER_LOOP_CNT_MAX;Count++){
                                    gDCMeterData.AnalogData.LoopAnalog[Count].LoopEnergy = 0;
                                    //printf("%%%%%%%%%%%%%%%%%%%\n");
                                }
								return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;
                        //��ʼ�ֶ�У׼
						case YDT1363_3_PROTOCAL_CID2_SECTION_CALIBRATION:
							if(ProcBuf.Length==5){
                                LoopNum = ProcBuf.DataBuf[0];
                                if((LoopNum==0xff)||((LoopNum>0)&&(LoopNum<=DCMETER_LOOP_CNT_MAX))){
                                    U32Temp = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[1]));
                                    if(0==CalibrationHallSensor(LoopNum,U32Temp,&gDCMeterData.SectCali,&gDCMeterData))
                                        return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                            YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
                                    else
                                        return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                            YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL,pSendBuf,pSendLen);
                                }
							}else if((ProcBuf.Length==2)&&(ProcBuf.DataBuf[0]==0xfe)){
                                LoopNum = ProcBuf.DataBuf[1];
                                if((LoopNum==0xff)||((LoopNum>0)&&(LoopNum<=DCMETER_LOOP_CNT_MAX))){
                                    if(0==CalibrationHallSensor_Repeal(LoopNum,&gDCMeterData.SectCali))
                                        return ProtocolProc_YDT1363_3_Make_SectCali((uint16_t *)&gDCMeterData.SectCali.CaliCnt[0],pSendBuf,pSendLen);
                                    else
                                        return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                            YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL,pSendBuf,pSendLen);
                                }
							}else if((ProcBuf.Length==2)&&(ProcBuf.DataBuf[0]==0xfd)){
                                LoopNum = ProcBuf.DataBuf[1];
                                if((LoopNum==0xff)||((LoopNum>0)&&(LoopNum<=DCMETER_LOOP_CNT_MAX))){
                                    if(0==CalibrationHallSensor_ZERO_Repeal(LoopNum,&gDCMeterData.SectCali))
                                        return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                                YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
                                    else
                                        return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                            YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL,pSendBuf,pSendLen);
                                }
							}else{
								
							}
							break;
                        //��ȡCS5460У׼ֵ
						/*case YDT1363_3_PROTOCAL_CID2_GET_CS5460CALI:
							if(ProcBuf.Length==0){
                                TempLen = sizeof(Temp);
                                if(0==CS5460_GetCalibrationData(Temp,&TempLen))
                                    return ProtocolProc_YDT1363_3_Make_CS5460CaliData(Temp,TempLen,pSendBuf,pSendLen);
                                else
                                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;*/
                        case YDT1363_3_PROTOCAL_CID2_GET_PAGE_DATA:
							if(ProcBuf.Length==5){
                                U32Temp = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[1]));
                                TempLen = ProcBuf.DataBuf[0];
                                if(TempLen<YDT1363_3_DATABUF_MAX){
                                    AT45DBxxxD_Read(Temp,U32Temp,TempLen);
                                    ProcBuf.Addr = (uint8_t)GetDevAddr();
                                    ProcBuf.Ver = YDT1363_3_PROTOCAL_VER;
                                    ProcBuf.CID1 = YDT1363_3_PROTOCAL_CID1_DCMETER;
                                    ProcBuf.CID2 = YDT1363_3_PROTOCAL_RTN_NORMAL;
                                    ProcBuf.Length = TempLen;
                                    memcpy(ProcBuf.DataBuf,Temp,ProcBuf.Length);
                                    return ProtocolProc_YDT1363_3_Make(&ProcBuf,pSendBuf,pSendLen);   
                                }
							}
							else{
								
							}
							break;
                        //��ȡ6��ͨ����CTֵ
						case YDT1363_3_PROTOCAL_CID2_GET_ENERGY_RATIO:
							if((ProcBuf.Length==0)){
								return ProtocolProc_YDT1363_3_Make_EgyRatio(\
										&gDCMeterData.EgyRatio,pSendBuf,pSendLen);
							}
							else{
								
							}
							break;
                        //����6��ͨ����CTֵ
						case YDT1363_3_PROTOCAL_CID2_SET_ENERGY_RATIO:
                            //��������ͨ��
							if((ProcBuf.Length>=25)&&(ProcBuf.DataBuf[0]==0xff)){
                                for(Count=0;Count<DCMETER_LOOP_CNT_MAX;Count++){
                                    U32Temp = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[1+4*Count]));
                                    gDCMeterData.EgyRatio.Value[Count] = (float)U32Temp/1000.0;
                                    MeterData_WaitWriteSucc(1,
                                        (uint8_t *)&gDCMeterData.EgyRatio.Value[Count],
                                        AT24CXX_ADDR_ENERGY1_RATIO+5*Count,
                                        AT24CXX_ADDR_ENERGY1_RATIO_SIZE);
                                }
                                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);    
							}
							else if((ProcBuf.Length>=5)&&(ProcBuf.DataBuf[0]>=0x01)&&(ProcBuf.DataBuf[0]<=DCMETER_LOOP_CNT_MAX)){
                                U32Temp = SWAP_DWORD(GetDWord(&ProcBuf.DataBuf[1]));
                                Count = ProcBuf.DataBuf[0]-1;
                                gDCMeterData.EgyRatio.Value[Count] = (float)U32Temp/1000.0;
                                MeterData_WaitWriteSucc(1,
                                    (uint8_t *)&gDCMeterData.EgyRatio.Value[Count],
                                    AT24CXX_ADDR_ENERGY1_RATIO+5*Count,
                                    AT24CXX_ADDR_ENERGY1_RATIO_SIZE);
                                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
							}
                            else{
                            
                            }
							break; 
                        case YDT1363_3_PROTOCAL_CID2_GET_SOFT_VER:
                            if(ProcBuf.Length==0){
                                return ProtocolProc_YDT1363_3_Make_SoftWare(pSendBuf,pSendLen);
							}
							else{
								
							}
                            break;
                        case YDT1363_3_PROTOCAL_CID2_SET_TO_BOOT:
                            if(ProcBuf.Length==4){
                                uint32_t APROM_SIZE,Flag,SetBaudRate;
                                SetBaudRate = SWAP_DWORD(GetDWord(ProcBuf.DataBuf));
                                SYS_UnlockReg();
                                FMC_Open();
                                FMC_ENABLE_AP_UPDATE();
                                APROM_SIZE = GetApromSize();
                                Flag = APROM_TO_LDROM;
                                
                                switch(SetBaudRate)
                                {
                                    /*case 1200:
                                        Flag |= LDROM_BAUDRATE_1200;
                                        break;
                                    case 2400:
                                        Flag |= LDROM_BAUDRATE_2400;
                                        break;
                                    case 4800:
                                        Flag |= LDROM_BAUDRATE_4800;
                                        break;*/
                                    case 19200:
                                        Flag |= LDROM_BAUDRATE_19200;
                                        break;
                                    default:
                                        Flag |= LDROM_BAUDRATE_9600;
                                        break;
                                }
                                
                                FMC_Write(APROM_SIZE-8,Flag);
                                Flag = 0;
                                Flag = FMC_Read(APROM_SIZE-8);
                                if((Flag & APROM_TO_LDROM_MSK) != APROM_TO_LDROM){
                                    Rtn = YDT1363_3_PROTOCAL_RTN_CMD_EXEC_FAIL;
                                    gu8Prot_RTN = Rtn;
                                }
                                FMC_DISABLE_AP_UPDATE();

                                FMC_Close();
                                SYS_LockReg();
                                gu8Prot_RTN = YDT1363_3_PROTOCAL_RTN_NORMAL;
                                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                                        YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
							}
							else{
								
							}
                            break;        
						default:
							Rtn = YDT1363_3_PROTOCAL_RTN_CID2_INVALID;
							return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
								Rtn,pSendBuf,pSendLen);
							break;
					}
                    Rtn = YDT1363_3_PROTOCAL_RTN_FORMAT_ERROR;
                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                            Rtn,pSendBuf,pSendLen);
				}
				else{
					Rtn = YDT1363_3_PROTOCAL_RTN_CID1_INVALID;
                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                            Rtn,pSendBuf,pSendLen);
				}
			}
			else{
				//9.3.1��ȡѶЭ��汾�ţ�4FH������У��VER
                //�����ǻ�ȡ�豸��ַ����
				if((ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_GET_VERSION)&&(ProcBuf.Length==0))
					return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
							YDT1363_3_PROTOCAL_RTN_VER_ERROR,pSendBuf,pSendLen);
                else if((ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_GET_DEVICE_ADDR)&&(ProcBuf.Length==0)){
                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                            YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
                }
				else if(ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_GET_VERSION){
					Rtn = YDT1363_3_PROTOCAL_RTN_FORMAT_ERROR;
					return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
							Rtn,pSendBuf,pSendLen);
				}
				else{
					Rtn = YDT1363_3_PROTOCAL_RTN_VER_ERROR;
					return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
							Rtn,pSendBuf,pSendLen);
				}  
			}
		}
        else if(ProcBuf.Addr == 0){
        //�㲥��ַ����Ӧ�趨ʱ��,��������Ӧ��;��Ӧ��ȡ��ַ����
            if(ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_SET_DATE){
                if(ProcBuf.Length == 7)
                {
                    pTmEx = (PPM5KT_COMM_TM)Temp;
                    pTmEx->Year = SWAP_WORD(GetWord(&ProcBuf.DataBuf[0]));               
                    pTmEx->Month = ProcBuf.DataBuf[2];
                    pTmEx->Day = ProcBuf.DataBuf[3];
                    pTmEx->Hour = ProcBuf.DataBuf[4];
                    pTmEx->Minute = ProcBuf.DataBuf[5];
                    pTmEx->Second = ProcBuf.DataBuf[6];
                    
                    if(0 == ProtocolProc_YDT1363_3_SetDate((PDCMETER_DATE)pTmEx,NULL)){
                        RX8010_SetTime(pTmEx);
                      
                    }
                }
            }else if((ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_GET_DEVICE_ADDR)&&(ProcBuf.Length==0)){
                    return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                            YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
            }
            return YDT1363_FUNC_RET_FALSE;
        }
		else{
		//�����ǻ�ȡ�豸��ַ����
			if((ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_GET_DEVICE_ADDR)
                &&(ProcBuf.Length==0)){
                return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
						YDT1363_3_PROTOCAL_RTN_NORMAL,pSendBuf,pSendLen);
            }
			else if(ProcBuf.CID2 == YDT1363_3_PROTOCAL_CID2_GET_DEVICE_ADDR){
					Rtn = YDT1363_3_PROTOCAL_RTN_FORMAT_ERROR;
					return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
							Rtn,pSendBuf,pSendLen);
			}
			else{
				//�����ַ����,CID2�ֲ��ǻ�ȡ��ַ����ǾͲ��Ƿ������豸��
                return	YDT1363_FUNC_RET_FALSE;
			}	
		}
        Rtn = YDT1363_3_PROTOCAL_RTN_FORMAT_ERROR;
        return ProtocolProc_YDT1363_3_Make_NoneDataBufFrame(\
                Rtn,pSendBuf,pSendLen);
	}
	return	YDT1363_FUNC_RET_FALSE;
}

/*
 ************************************************************************************************************************************************************************     
 *��������:	ProtocolProc_YDT1363_3_CallBack
 *��������:	��ȡЭ�鴦�����������ݵĵ�ַָ��
 *��������:	��
 *�������:	
 *��������:	���ݵĵ�ַָ��
 *��������:	
 *ȫ������: 
 *����˵��: 
 ************************************************************************************************************************************************************************       
 */
extern PDCMETER_DATA ProtocolProc_YDT1363_3_CallBack(void)
{
    return (PDCMETER_DATA)&gDCMeterData;
}
