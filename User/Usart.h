#ifndef __USART_H_
#define __USART_H_

//#include "UserCommon.h"
#include "NUC029xAN.h"

#define USART_REC_LEN  			100  	//定义最大接收字节数50
	  	
extern uint8_t  USART0_RX_Buf[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern uint32_t USART0_RX_Len;         		//接收数据长度	
//extern u8  USART0_RX_Start;
extern uint8_t USART0_RX_Succ;         		//接收成功标志

//extern void Uart0_Init(u32 bound);
extern void UART1_Init(void);
extern void UART0_Init(uint32_t BaudRate);



extern void USART_SendData(UART_T *uart,uint8_t *pSendBuf,uint32_t SendLen);

#define USART1_SendData(pSendBuf,SendLen) USART_SendData(UART1,pSendBuf,SendLen)

#define USART0_SendData(pSendBuf,SendLen) USART_SendData(UART0,pSendBuf,SendLen)

#endif
