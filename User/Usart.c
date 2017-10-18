//#include "PM5000T.h"
#include "Usart.h"
#include "NUC029xAN.h"

/*
和第二版相比,交换了串口0,1所对应的设备:
P30、P31 UART0->485

P12 P13 UART1->RN8209C

*/

uint8_t  USART0_RX_Buf[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
uint32_t USART0_RX_Len;         		//接收数据长度	
uint8_t  USART0_RX_Start;
uint8_t  USART0_RX_Succ;         		//接收成功标志



//???IO ??1 
//bound:???
extern void UART0_Init(uint32_t BaudRate)
{
    
	SYS_ResetModule(UART0_RST);
	
	/* Configure UART0 and set UART0 Baudrate */
	UART_Open(UART0, BaudRate);
	/* Enable Interrupt and install the call back function 接收数据完成、发送完成、接收超时*/
	UART_ENABLE_INT(UART0, (UART_IER_RDA_IEN_Msk));// | UART_IER_THRE_IEN_Msk | UART_IER_RTO_IEN_Msk));
	NVIC_EnableIRQ(UART0_IRQn);

}

extern void UART1_Init(void)
{
	/*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART */
    //UART_T* uart;
    uint8_t u8UartClkSrcSel;
    uint32_t u32ClkTbl[4] = {__HXT, 0, 0, __HIRC};
    uint32_t u32Baud_Div = 0;
    
    SYS_ResetModule(UART1_RST);
    
    /* Configure UART0 and set UART0 Baudrate */
    //UART_Open(UART0, 9600);

    /* Get UART clock source selection */
    u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UART_S_Msk) >> CLK_CLKSEL1_UART_S_Pos;

    /* Select UART function */
    UART1->FUN_SEL = UART_FUNC_SEL_UART;

    /* Set UART line configuration */
    UART1->LCR = UART_WORD_LEN_8 | UART_PARITY_EVEN | UART_STOP_BIT_1;

    /* Set UART Rx and RTS trigger level */
    UART1->FCR &= ~(UART_FCR_RFITL_Msk | UART_FCR_RTS_TRI_LEV_Msk);

    /* Get PLL clock frequency if UART clock source selection is PLL */
    if(u8UartClkSrcSel == 1)
        u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();

    /* Set UART baud rate */

    u32Baud_Div = UART_BAUD_MODE2_DIVIDER(u32ClkTbl[u8UartClkSrcSel], 4800);

    if(u32Baud_Div > 0xFFFF)
        UART1->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER(u32ClkTbl[u8UartClkSrcSel], 4800));
    else
        UART1->BAUD = (UART_BAUD_MODE2 | u32Baud_Div);
}

extern void USART_SendData(UART_T *uart,uint8_t *pSendBuf,uint32_t SendLen)
{
    if(pSendBuf!=NULL){
        UART_Write(uart,pSendBuf,SendLen);
    }  
}

void UART0_IRQHandler(void)   //串口0中断服务程序
{
	uint8_t Res;
    uint32_t u32IntSts= UART0->ISR;

    if(u32IntSts & UART_ISR_RDA_INT_Msk)  //接收中断(接收到的数据必须是0x0d结尾)
	{
        while(UART_IS_RX_READY(UART0)){
            Res = UART_READ(UART0);           /* Rx trigger level is 1 byte*/
            
            if(Res == 0x7e) {
                USART0_RX_Len = 0;
                USART0_RX_Start = 1;
            }
            if(USART0_RX_Start == 1)
            {
                USART0_RX_Buf[USART0_RX_Len++] = Res;
            }
            if(Res == 0x0d) {
                USART0_RX_Buf[USART0_RX_Len] = Res;
                USART0_RX_Start = 0;
                USART0_RX_Succ = 1;
            }
            if ((USART0_RX_Len  >= (USART_REC_LEN-1))){
                //||(USART1_RX_Len  >= (PROTOCAL_BUF_MAX-1))) {
                USART0_RX_Len  = 0;
            }	 
        } 
    }
} 

void UART1_IRQHandler(void)   //串口0中断服务程序
{

}
