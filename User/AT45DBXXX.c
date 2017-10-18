//#include "PM5000T.h"
#include "AT45DBXXX.h" 
#include "UserCommon.h"


//
//! Chip Erase
//

const uint8_t AT45DBxxxD_CMD_CPER[4] = {0xC7, 0x94, 0x80, 0x9A};

//
//! Enable Software Sector Protection Command
//! Notice:
//!     If the device is power cycled, then the software controlled protection will be disabled.
//! Once the device is powered up, the Enable Sector Protection command  should be reissued
//! if sector protection is desired and if the WP pin is not used.
//
const uint8_t AT45DBxxxD_CMD_ESSP[4] = { 0x3D, 0x2A, 0x7F, 0xA9};
//
//! Disable Software Sector Protection Command
//
const uint8_t AT45DBxxxD_CMD_DSSP[4] = { 0x3D, 0x2A, 0x7F, 0x9A};

//
//! Erase Sector Protection Register Command
//
const uint8_t AT45DBxxxD_CMD_ESPR[4] = { 0x3D, 0x2A, 0x7F, 0xCF};
//
//! Program Sector Protection Register Command
//
const uint8_t AT45DBxxxD_CMD_PSPR[4] = { 0x3D, 0x2A, 0x7F, 0xFC};
//
//! Read Sector Protection Register Command
//
const uint8_t AT45DBxxxD_CMD_RSPR[4] = { 0x32, 0xFF, 0xFF, 0xFF};
//
//! Sector Lockdown
//! Attention:
//!     Once a sector is locked down, it can never be erased or programmed, and it can never be unlocked.
//
const uint8_t AT45DBxxxD_CMD_SCLD[4] = { 0x3D, 0x2A, 0x7F, 0x30};
//
//! Programming the “Power of 2” binary page size Configuration Register
//! Attention:
//!     This “power of 2” Command changes the page size from initial 528 bytes to 512 bytes.
//! The “power of 2” page size is a onetime programmable configuration register and once the device is
//! configured for “power of 2” page size, it cannot be reconfigured again.
//
const uint8_t AT45DBxxxD_CMD_PGCR[4] = {0x3D, 0x2A, 0x80, 0xA6};


#if AT45DBxxxD_HAS_WRITE_PROTECT
//
// Default no write protect
//
static uint8_t gsWriteProtectEn = 0;
#endif

AT45DBXXX_InfoStruct AT45DBxxxDInfo;

static uint8_t gsBinaryPageFlag = 0;


//初始化AD45DB161引脚,MOSI(PB15),MISO(PB14),SCLK(PB13),CS(PB12),RST(PD10)
extern uint8_t AT45DBXXX_Init(void)
{
	uint16_t temp=0;
    
    CLK_EnableModuleClock(SPI0_MODULE);
    //SPI初始化和CS5460保持一致
	/* Select IP clock source */
    CLK_SetModuleClock(SPI0_MODULE,CLK_CLKSEL1_SPI0_S_HCLK,0);
    /* Setup SPI multi-function pin */
    
    SYS->P1_MFP &= ~(SYS_MFP_P15_Msk | SYS_MFP_P16_Msk | SYS_MFP_P17_Msk);
    SYS->P1_MFP |= (SYS_MFP_P15_MOSI_0 | SYS_MFP_P16_MISO_0 | SYS_MFP_P17_SPICLK0);
    
    SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 8, 2000000);

    SPI_DisableAutoSS(SPI0);
    
    GPIO_SetMode(P1, BIT4, GPIO_PMD_OUTPUT);//DF_CS
    GPIO_SetMode(P4, BIT2, GPIO_PMD_OUTPUT);//DF_RST
    
    //AT45DBxxxD_PIN_RST = 0;
    //Comm_DelayUs(1000);
    AT45DBxxxD_PIN_CS = 1;
    AT45DBxxxD_PIN_RST = 1;
    
    
	AT45DBXXX_ReadID(&temp);//0x1f26
    if((temp&AT45DBxxxD_MANUFACTURER_ID)!=AT45DBxxxD_MANUFACTURER_ID)
		return AT45DBxxxD_OP_INVALID;
	
	temp = 1<<((temp & 0x1F)-2);
	AT45DBxxxDInfo.usChipID = AT45DBxxxD_MANUFACTURER_ID|temp;//0x1f10
	
	temp = AT45DBXXX_GetStatus();
	if(temp&AT45DBXXX_STATUS_PAGE_SIZE){
		gsBinaryPageFlag = 0;
		AT45DBxxxDInfo.usPageSize = AT45DBxxxD_BINARY_PAGE_SIZE_BIG;
		AT45DBxxxDInfo.ulCapacity = (uint32_t)(AT45DBxxxDInfo.usChipID&0xff)*135168;
	}else{
        gsBinaryPageFlag = 1;
		AT45DBxxxDInfo.usPageSize = AT45DBxxxD_BINARY_PAGE_SIZE_BIG;
		AT45DBxxxDInfo.ulCapacity = (uint32_t)(AT45DBxxxDInfo.usChipID&0xff)*131072;
	}
	AT45DBxxxDInfo.ulBlockSize = AT45DBxxxDInfo.usPageSize * AT45DBXXX_PAGES_PER_BLOCK;
    if((AT45DBxxxDInfo.usChipID&0xff)>2) 
		AT45DBxxxDInfo.ulSectorSize = (uint32_t)AT45DBxxxDInfo.ulBlockSize * AT45DBxxxD_BIG_SECTOR;
    else 
		AT45DBxxxDInfo.ulSectorSize = (uint32_t)AT45DBxxxDInfo.ulBlockSize * AT45DBxxxD_SMALL_SECTOR;
    AT45DBxxxDInfo.ulTatalPages = AT45DBxxxDInfo.ulCapacity / AT45DBxxxDInfo.usPageSize;

    return AT45DBxxxD_OP_OK;
	
}
extern uint8_t AT45DBXXX_Check(void)
{
    uint16_t temp=0;
    AT45DBXXX_ReadID(&temp);//0x1f26
    if((temp&AT45DBxxxD_MANUFACTURER_ID)!=AT45DBxxxD_MANUFACTURER_ID)
		return AT45DBxxxD_OP_INVALID;
    return AT45DBxxxD_OP_OK;
}    
/*
//软件模拟SPI时序
extern u8 AT45DBXXX_SPI_WriteAndReadByte(u8 byte)
{
	u8 i;
	u8 SendByte=byte,ReadByte=0;
    
	for(i=0;i<8;i++){
		AT45DBXXX_SCK_LOW();
        if((SendByte&0x80)==0x80)
			AT45DBXXX_SI_HIGH();
		else
			AT45DBXXX_SI_LOW();
		SendByte<<=1;
        ReadByte<<=1;
        if(1==AT45DBXXX_SO_READ())
            ReadByte |= 0x01;
        AT45DBXXX_SCK_HIGH();
	}
    return ReadByte;
}*/

extern uint8_t AT45DBXXX_SPI_WriteAndReadByte(uint8_t byte)
{
    SPI_WRITE_TX0(SPI0,byte);
    SPI_TRIGGER(SPI0);
    while(SPI_IS_BUSY(SPI0));
    return SPI_READ_RX0(SPI0);
}
/*******************************************************************************
* Function Name  : AT45DBXXX_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
extern void AT45DBXXX_ReadID(uint16_t *tmp)
{
    uint8_t Temp1 = 0, Temp2 = 0;

    /* Select the FLASH: Chip Select low */
    AT45DBXXX_CS_LOW();

    /* Send "RDID " instruction */
    AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_READ_ID);

    /* Read a byte from the FLASH */
    Temp1 = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);

    /* Read a byte from the FLASH */
    Temp2 = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);

    /* Read a byte from the FLASH */
    //Temp2 = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);

    /* Read a byte from the FLASH */
    //Temp3 = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
    //Temp4 = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
    /* Deselect the FLASH: Chip Select high */
    AT45DBXXX_CS_HIGH();

    *tmp = (Temp1 << 8) | Temp2;
    //Temp = (Temp1 << 24) | (Temp2 << 16) | (Temp3<<8) | Temp4;
   // return Temp;
}

extern uint16_t AT45DBXXX_GetStatus(void)
{
    uint16_t FLASH_Status = 0;

    /* Select the FLASH: Chip Select low */
    AT45DBXXX_CS_LOW();

    /* Send "Read Status Register" instruction */
    AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_READ_STATUS);


    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
    FLASH_Status <<= 8;
    FLASH_Status |= AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
    /* Deselect the FLASH: Chip Select high */
    AT45DBXXX_CS_HIGH();
    return FLASH_Status;
}

extern uint16_t AT45DBXXX_WaitForWriteEnd(void)
{
    uint16_t state_reg = 0x00;
    uint16_t retry = 0;
    do
    {
        state_reg = AT45DBXXX_GetStatus();
        retry++;
    }
    while((retry < 300)&&((state_reg & AT45DBXXX_STATUS_RDY) == 0) );
    if(retry >= 300)return 0;
    else return 1;
}


extern void AT45DBxxxD_WritePageAddr(uint32_t ulPageAddr)
{
    /*if(0 == gsBinaryPageFlag)
    {
    	if((AT45DBxxxDInfo.usChipID & 0xFF) > AT45DB321D)
    	{
    		AT45DBXXX_SPI_WriteAndReadByte( (u8)(ulPageAddr >> 6));
    		AT45DBXXX_SPI_WriteAndReadByte( (u8)((ulPageAddr & 0x007F) << 2));
    	}
        else if((AT45DBxxxDInfo.usChipID & 0xFF) > AT45DB081D)
        {
            AT45DBXXX_SPI_WriteAndReadByte( (u8)(ulPageAddr >> 7));
            AT45DBXXX_SPI_WriteAndReadByte( (u8)((ulPageAddr & 0x007F) << 1));
        }
        else
        {
            AT45DBXXX_SPI_WriteAndReadByte( (u8)(ulPageAddr >> 8));
            AT45DBXXX_SPI_WriteAndReadByte( (u8)(ulPageAddr & 0x00FF));
        }
    }
    else
    {
    	if((AT45DBxxxDInfo.usChipID & 0xFF) > AT45DB321D)
    	{
    		AT45DBXXX_SPI_WriteAndReadByte( (u8)(ulPageAddr >> 5));
    		AT45DBXXX_SPI_WriteAndReadByte( (u8)((ulPageAddr & 0x003F) << 3));
    	}
    	else if((AT45DBxxxDInfo.usChipID & 0xFF) > AT45DB081D)
        {
            AT45DBXXX_SPI_WriteAndReadByte( (u8)(ulPageAddr >> 6));
            AT45DBXXX_SPI_WriteAndReadByte( (u8)((ulPageAddr & 0x003F) << 2));
        }
        else
        {
            AT45DBXXX_SPI_WriteAndReadByte( (u8)(ulPageAddr >> 7));
            AT45DBXXX_SPI_WriteAndReadByte( (u8)((ulPageAddr & 0x007F) << 1));
        }
    }*/
    if(0 == gsBinaryPageFlag){
        AT45DBXXX_SPI_WriteAndReadByte( (uint8_t)(ulPageAddr >> 7));
        AT45DBXXX_SPI_WriteAndReadByte( (uint8_t)((ulPageAddr & 0x007F) << 1));
    }else{
        AT45DBXXX_SPI_WriteAndReadByte( (uint8_t)(ulPageAddr >> 6));
        AT45DBXXX_SPI_WriteAndReadByte( (uint8_t)((ulPageAddr & 0x003F) << 2));
    }
    
    AT45DBXXX_SPI_WriteAndReadByte( 0x00);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_PageErase
* Description    : Erases the specified FLASH Page.
* Input          : PageAddr: address of the sector to erase.
* Output         : None
* Return         : 0成功，1超时
*******************************************************************************/
extern uint8_t AT45DB161_PageErase(uint32_t PageAddr)
{
  /* Sector Erase */
  /* Select the FLASH: Chip Select low */
  AT45DBXXX_CS_LOW();
  /* Send Sector Erase instruction */
  AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_PAGE_ERASE);
  /* Send SectorAddr high nibble address byte */
  AT45DBXXX_SPI_WriteAndReadByte((PageAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  AT45DBXXX_SPI_WriteAndReadByte((PageAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  AT45DBXXX_SPI_WriteAndReadByte(PageAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  AT45DBXXX_CS_HIGH();

  /* Wait the end of Flash writing */
  return AT45DBXXX_WaitForWriteEnd();
}
/*******************************************************************************
* Function Name  : SPI_FLASH_BulkErase
* Description    : Erases the entire FLASH.
* Input          : None
* Output         : None
* Return         : 0成功，1超时
*******************************************************************************/
extern uint8_t AT45DB161_BulkErase(void)
{
  /* Bulk Erase */
  /* Select the FLASH: Chip Select low */
  AT45DBXXX_CS_LOW();
  /* Send Bulk Erase instruction  */
  AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_BULK_ERASE_1);
  /* Send Bulk Erase instruction  */
  AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_BULK_ERASE_2);
  /* Send Bulk Erase instruction  */
  AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_BULK_ERASE_3);
  /* Send Bulk Erase instruction  */
  AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_BULK_ERASE_4);
  /* Deselect the FLASH: Chip Select high */
  AT45DBXXX_CS_HIGH();

  /* Wait the end of Flash writing */
  return AT45DBXXX_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WriteSector
* Description    : Writes one Sector to the FLASH with a single WRITE
*                  cycle(Page WRITE sequence). The number of byte can't exceed
*                  the FLASH page size, But for FAT always 512 butes
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - SectorAddr : FLASH's internal Page address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH,
*                    must be equal or less than "SPI_FLASH_PageSize" value.
* Output         : None
* Return         : 0成功，1超时
*******************************************************************************/
extern uint8_t AT45DB161_WriteSector(uint8_t* pBuffer, uint32_t SectorAddr, uint16_t NumByteToWrite)
{
	uint16_t  uiPageAddress;
	uint8_t bytes[2];
	uiPageAddress  = (uint16_t)SectorAddr & 0x0FFF;
	uiPageAddress *= 4;
	bytes[0] = (uint8_t)(uiPageAddress>>8);					// msb
	bytes[1] = (uint8_t)(uiPageAddress & 0xFF);				// lsb
	
	/* Select the FLASH: Chip Select low */
	AT45DBXXX_CS_LOW();
	/* Send "Write to Memory " instruction */
	AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_WRITE);
	/* Send WriteAddr high nibble address byte to write to */
	AT45DBXXX_SPI_WriteAndReadByte(bytes[0]);
	/* Send WriteAddr medium nibble address byte to write to */
	AT45DBXXX_SPI_WriteAndReadByte(bytes[1]);
	/* Send WriteAddr low nibble address byte to write to */
	AT45DBXXX_SPI_WriteAndReadByte(0x00);

	/* while there is data to be written on the FLASH */
	while (NumByteToWrite--)
	{
		/* Send the current byte */
		AT45DBXXX_SPI_WriteAndReadByte(*pBuffer);
		/* Point on the next byte to be written */
		pBuffer++;
	}

	/* Deselect the FLASH: Chip Select high */
	AT45DBXXX_CS_HIGH();

	/* Wait the end of Flash writing */
	return AT45DBXXX_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadSector
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal Pages address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
extern void AT45DB161_ReadSector(uint8_t* pBuffer, uint32_t SectorAddr, uint16_t NumByteToRead)
{
	uint16_t  uiPageAddress;
	uint8_t bytes[2];

   	uiPageAddress  = (uint16_t)SectorAddr & 0x0FFF;
	uiPageAddress *= 4;

	bytes[0] = (uint8_t)(uiPageAddress>>8);					// msb
	bytes[1] = (uint8_t)(uiPageAddress & 0xFF);          	// lsb

	/* Select the FLASH: Chip Select low */
	AT45DBXXX_CS_LOW();

	/* Send "Read from Memory " instruction */
	AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_CMD_READ);

	
	/* Send WriteAddr high nibble address byte to write to */
	AT45DBXXX_SPI_WriteAndReadByte(bytes[0]);
	/* Send WriteAddr medium nibble address byte to write to */
	AT45DBXXX_SPI_WriteAndReadByte(bytes[1]);
	/* Send WriteAddr low nibble address byte to write to */
	AT45DBXXX_SPI_WriteAndReadByte(0x00);


	/* Send a dummy byte to generate the clock needed by the FLASH */
	AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
	/* Send a dummy byte to generate the clock needed by the FLASH */
	AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
	/* Send a dummy byte to generate the clock needed by the FLASH */
	AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
	/* Send a dummy byte to generate the clock needed by the FLASH */
	AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);

	while (NumByteToRead--) /* while there is data to be read */
	{
	/* Read a byte from the FLASH */
	*pBuffer = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
	/* Point to the next location where the byte read will be saved */
	pBuffer++;
	}

	/* Deselect the FLASH: Chip Select high */
	AT45DBXXX_CS_HIGH();
}



//*****************************************************************************
//
//! \brief Main Memory Page to Buffer Transfer
//!
//! \param ucBufferNum specify the buffer 1 or 2 as the destination.
//! \param ulPageAddr specify the page address which you want to read.
//!
//! This function is to transfer data from specified page address of main memory
//! to specified AT45DBxxxD internal buffer.
//!
//! \return operation result
//
//*****************************************************************************
extern uint8_t AT45DBxxxD_MmtoBuf(uint8_t ucBufferNum, uint32_t ulPageAddr)
{
	uint32_t i = 0;

	if(ulPageAddr >= AT45DBxxxDInfo.ulTatalPages) return AT45DBxxxD_OP_INVALID;
    while(!AT45DBXXX_WaitForWriteEnd())
    {
    	if(++i > AT45DBxxxD_OVERTIME)
    	{
    		return AT45DBxxxD_OP_BUSY;
    	}
    }

    AT45DBXXX_CS_LOW();
    //
    // Write main memory to buffer transfer command
    //
    if (ucBufferNum == AT45DBxxxD_BUF1)
    {
        AT45DBXXX_SPI_WriteAndReadByte( AT45DBxxxD_CMD_MTB1);
    }
    else
    {
        AT45DBXXX_SPI_WriteAndReadByte( AT45DBxxxD_CMD_MTB2);
    }

    //
    // Write page address
    //
    AT45DBxxxD_WritePageAddr(ulPageAddr);
    AT45DBXXX_CS_HIGH();
    return AT45DBxxxD_OP_OK;
}

//*****************************************************************************
//
//! \brief Buffer to Main Memory Page Program with Built-in Erase
//!
//! \param ucBufferNum specify the buffer 1 or 2 as the source.
//! \param ulPageAddr specify the page address which you want to write.
//!
//! This function is to write data from specified internal buffer to the
//! specified main memory page address with built-in erase.
//!
//! \return operation result
//
//*****************************************************************************
extern uint8_t AT45DBxxxD_BuftoMm(uint8_t ucBufferNum, uint32_t ulPageAddr)
{
    uint32_t i = 0;

    if(ulPageAddr >= AT45DBxxxDInfo.ulTatalPages) return AT45DBxxxD_OP_INVALID;
    while(!AT45DBXXX_WaitForWriteEnd())
    {
    	if(++i > AT45DBxxxD_OVERTIME)
    	{
    		return AT45DBxxxD_OP_BUSY;
    	}
    }

    AT45DBXXX_CS_LOW();
    if (ucBufferNum == AT45DBxxxD_BUF1)
    {
    	AT45DBXXX_SPI_WriteAndReadByte(AT45DBxxxD_CMD_B1TMW);
    }
    else
    {
    	AT45DBXXX_SPI_WriteAndReadByte(AT45DBxxxD_CMD_B2TMW);
    }

    AT45DBxxxD_WritePageAddr(ulPageAddr);

    AT45DBXXX_CS_HIGH();

    return AT45DBxxxD_OP_OK;
}

//*****************************************************************************
//
//! \brief Read data in specified AT45DBxxxD internal buffer
//!
//! \param ucBufferNum specify the buffer 1 or 2 as the source.
//! \param pucBuffer to store the data read out from internal buffer
//! \param ulOffAddr specify the start address in the buffer which you want to read.
//! \param ulNumByteToRead specify how many bytes to read.
//!
//! This function is to read data from specified internal buffer. If you have
//! just a little data to be stored(less than AT45DBxxxDInfo.usPageSize), you can temporarily
//! store them in the AT45DBxxxD internal buffer.It can be more fast than read write from
//! main memory
//!
//! \return operation result
//
//*****************************************************************************
extern uint8_t AT45DBxxxD_BufferRead(uint8_t ucBufferNum, uint8_t *pucBuffer,
                            uint32_t ulOffSetAddr, uint32_t ulNumByteToRead)
{
	uint32_t i = 0;
    if(ulOffSetAddr + ulNumByteToRead > AT45DBxxxDInfo.usPageSize) return AT45DBxxxD_OP_INVALID;

    while(!AT45DBXXX_WaitForWriteEnd())
    {
    	if(++i > AT45DBxxxD_OVERTIME)
    	{
    		return AT45DBxxxD_OP_BUSY;
    	}
    }

    AT45DBXXX_CS_LOW();
    if (ucBufferNum == AT45DBxxxD_BUF1)
    {
        AT45DBXXX_SPI_WriteAndReadByte( AT45DBxxxD_CMD_BF1R);
    }
    else
    {
        AT45DBXXX_SPI_WriteAndReadByte( AT45DBxxxD_CMD_BF2R);
    }
    AT45DBXXX_SPI_WriteAndReadByte( 0x00);
    AT45DBXXX_SPI_WriteAndReadByte( (uint8_t)(ulOffSetAddr >> 8));
    AT45DBXXX_SPI_WriteAndReadByte( (uint8_t)(ulOffSetAddr & 0x00FF));

	for(i=0;i<ulNumByteToRead;i++){
		pucBuffer[i] = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
	}

    AT45DBXXX_CS_HIGH();
    return AT45DBxxxD_OP_OK;
}

//*****************************************************************************
//
//! \brief Write data to specified AT45DBxxxD internal buffer
//!
//! \param ucBufferNum specify the buffer 1 or 2 as the destination.
//! \param pucBuffer to store the data need to be written.
//! \param ulWriteAddr specify the start address in the buffer which you want to write.
//! \param ulNumByteToRead specify how many bytes to write.
//!
//! This function is to write data to specified internal buffer. If you have
//! just a little data to be stored(less than AT45DBxxxDInfo.usPageSize), you can temporarily
//! store them in the AT45DBxxxD internal buffer.It can be more fast than read write from
//! main memory.This write function doesn't affect the content in main memory.
//!
//! \return operation result
//
//*****************************************************************************
extern uint8_t AT45DBxxxD_BufferWrite(uint8_t ucBufferNum, uint8_t *pucBuffer,
                           uint32_t ulOffSetAddr, uint32_t ulNumByteToWrite)
{
	uint32_t i = 0;
    if (ulOffSetAddr + ulNumByteToWrite > AT45DBxxxDInfo.usPageSize) return AT45DBxxxD_OP_INVALID;
    while(!AT45DBXXX_WaitForWriteEnd())
    {
        if(++i > AT45DBxxxD_OVERTIME)
        {
        	return AT45DBxxxD_OP_BUSY;
        }
    }
    AT45DBXXX_CS_LOW();
    if (ucBufferNum == AT45DBxxxD_BUF1)
    {
        AT45DBXXX_SPI_WriteAndReadByte( AT45DBxxxD_CMD_BF1W);
    }
    else
    {
        AT45DBXXX_SPI_WriteAndReadByte( AT45DBxxxD_CMD_BF2W);
    }
    AT45DBXXX_SPI_WriteAndReadByte( 0x00);
    AT45DBXXX_SPI_WriteAndReadByte( (uint8_t)(ulOffSetAddr >> 8));
    AT45DBXXX_SPI_WriteAndReadByte( (uint8_t)(ulOffSetAddr & 0x00FF));
	for(i=0;i<ulNumByteToWrite;i++){
		AT45DBXXX_SPI_WriteAndReadByte(pucBuffer[i]);
	}
    AT45DBXXX_CS_HIGH();

    return AT45DBxxxD_OP_OK;
}

//*****************************************************************************
//
//! \brief Read a quantity of bytes from start of specified main memory page address
//!
//! \param pucBuffer to store the data read out from page.
//! \param ulPageAddr specify the page address in the main memory of AT45DBxxxD.
//! \param ulNumByteToRead specify how many bytes to read.
//!
//! This function is to read data from specified page address.
//! The function use main memory page read command to read data directly form main memory
//!
//! \return operation result
//
//*****************************************************************************
extern uint8_t AT45DBxxxD_PageRead(uint32_t ulPageAddr, uint8_t *pucBuffer, uint16_t ulNumByteToRead)
{
	uint32_t i = 0;
    if(ulPageAddr >= AT45DBxxxDInfo.ulTatalPages) return AT45DBxxxD_OP_INVALID;
    if(ulNumByteToRead > AT45DBxxxDInfo.usPageSize) ulNumByteToRead = AT45DBxxxDInfo.usPageSize;
    while(!AT45DBXXX_WaitForWriteEnd())
    {
    	if(++i > AT45DBxxxD_OVERTIME)
    	{
    		return AT45DBxxxD_OP_BUSY;
    	}
    }
    AT45DBXXX_CS_LOW();
    AT45DBXXX_SPI_WriteAndReadByte( AT45DBxxxD_CMD_MMPR);
    AT45DBxxxD_WritePageAddr(ulPageAddr);
    AT45DBXXX_SPI_WriteAndReadByte( 0);
    AT45DBXXX_SPI_WriteAndReadByte( 0);
    AT45DBXXX_SPI_WriteAndReadByte( 0);
    AT45DBXXX_SPI_WriteAndReadByte( 0);
    for(i=0;i<ulNumByteToRead;i++){
		pucBuffer[i] = AT45DBXXX_SPI_WriteAndReadByte(AT45DBXXX_DUMMY_BYTE);
	}
    AT45DBXXX_CS_HIGH();
    return AT45DBxxxD_OP_OK;
}

//*****************************************************************************
//
//! \brief Write one page of bytes to specified main memory page address
//!
//! \param pucBuffer to store the data read out from page.
//! \param ulPageAddr specify the page address in the main memory of AT45DBxxxD.
//!
//! This function is to write data to specified page address.
//! The function first write data to buffer then using buffer
//! to main memory transfer to write data to main memory.
//!
//! \return operation result
//
//*****************************************************************************
extern uint8_t AT45DBxxxD_PageWrite(uint32_t ulPageAddr, uint8_t *pucBuffer)
{
	uint32_t i = 0;
    if(ulPageAddr >= AT45DBxxxDInfo.ulTatalPages) return AT45DBxxxD_OP_INVALID;
    while(!AT45DBXXX_WaitForWriteEnd())
    {
    	if(++i > AT45DBxxxD_OVERTIME)
    	{
    		return AT45DBxxxD_OP_BUSY;
    	}
    }
    AT45DBxxxD_BufferWrite(AT45DBxxxD_BUF1, pucBuffer, 0, AT45DBxxxDInfo.usPageSize);
    AT45DBxxxD_BuftoMm(AT45DBxxxD_BUF1, ulPageAddr);

    return AT45DBxxxD_OP_OK;
}
//*****************************************************************************
//
//! \brief Read data from main memory
//!
//! \param pucBuffer specifies the destination buffer pointer to store data.
//! \param ulReadAddr specifies the byte address which data will be read (0 to AT45DBxxxDInfo.usPageSize*AT45DBxxxDInfo.ulTatalPages-1)
//! \param ulNumByteToRead specifies the length of data will be read.
//!
//! This function is to read a quantity of bytes from specified byte
//! address in main memory of AT45DBxxxD.
//!
//! \return operation result
//!
//*****************************************************************************
extern uint8_t AT45DBxxxD_Read(uint8_t *pucBuffer, uint32_t ulReadAddr, uint32_t ulNumByteToRead)
{
    uint16_t usOffAddr;
    uint32_t ulPageAddr;
    uint16_t  usLeftForward = 0, usLeftBehind = 0, usPages = 0;
    uint32_t ulBytesRead = 0;
    //static u8 UseBuf;
    //
    // if read address exceed chip size, return.
    //
    
    //UseBuf = ((UseBuf==AT45DBxxxD_BUF1)?AT45DBxxxD_BUF2:AT45DBxxxD_BUF1);
    
    if(ulReadAddr >= AT45DBxxxDInfo.usPageSize * AT45DBxxxDInfo.ulTatalPages) return AT45DBxxxD_OP_INVALID;
    //
    // calculate page address and offset address in page
    //
    ulPageAddr = ulReadAddr / AT45DBxxxDInfo.usPageSize;
    usOffAddr = ulReadAddr % AT45DBxxxDInfo.usPageSize;

    if((usOffAddr + ulNumByteToRead) > AT45DBxxxDInfo.usPageSize)
    {
    	//
    	// if reading data exceeds the boundary of a page,
    	// divide data into 3 parts:
    	// 1: start address to the boundary of the first page
    	// 2: second page to the last complete pages counts
    	// 3: the last incomplete page to the last byte
    	//
        usLeftForward = AT45DBxxxDInfo.usPageSize - usOffAddr;
        usPages = (ulNumByteToRead - usLeftForward) / AT45DBxxxDInfo.usPageSize;
        usLeftBehind = (ulNumByteToRead - usLeftForward) % AT45DBxxxDInfo.usPageSize;
    }
    else
    {
    	//
    	// if reading data dosen't exceed one page
    	//
        usLeftForward = ulNumByteToRead;
        usPages = 0;
        usLeftBehind = 0;
    }

//    while(!AT45DBXXX_WaitForWriteEnd());

    //
    // read first part of data
    //
    AT45DBxxxD_MmtoBuf(AT45DBxxxD_BUF2, ulPageAddr);
    AT45DBxxxD_BufferRead(AT45DBxxxD_BUF2, pucBuffer, usOffAddr, usLeftForward);

    ulPageAddr++;
    ulBytesRead += usLeftForward;

    //
    // read second part of data
    //
    while(usPages)
    {
        AT45DBxxxD_PageRead(ulPageAddr, &pucBuffer[ulBytesRead], AT45DBxxxDInfo.usPageSize);
        ulPageAddr++;
        ulBytesRead += AT45DBxxxDInfo.usPageSize;
        usPages--;
    }
    //
    // read last part of data
    //
    if(usLeftBehind != 0)
    {
        AT45DBxxxD_PageRead(ulPageAddr, &pucBuffer[ulBytesRead], usLeftBehind);
    }
    
    return AT45DBxxxD_OP_OK;
}

//*****************************************************************************
//
//! \brief Write data to main memory
//!
//! \param pucBuffer specifies the source data buffer pointer.
//! \param ulWriteAddr specifies the byte address which data will be written to. (0 to AT45DBxxxDInfo.usPageSize*AT45DBxxxDInfo.ulTatalPages)
//! \param usNumByteToWrite specifies the length of data will be write.
//!
//! This function is to write a quantity of byte to specified byte address in
//! the main memory of AT45DBxxxD.
//!
//! \return operation result
//!
//*****************************************************************************
extern uint8_t AT45DBxxxD_Write(uint8_t *pucBuffer, uint32_t ulWriteAddr, uint32_t ulNumByteToWrite)
{
    uint16_t  usOffAddr;
    uint32_t ulPageAddr;
    uint16_t  usLeftForward = 0, usLeftBehind = 0, usPages = 0;
    uint32_t ulBytesWritten = 0;
    //static u8 UseBuf;
    
    //UseBuf = ((UseBuf==AT45DBxxxD_BUF1)?AT45DBxxxD_BUF2:AT45DBxxxD_BUF1);
    //
    // If write data length == 0
    //
    if(ulNumByteToWrite == 0) return AT45DBxxxD_OP_OK;
    //
    // if write address exceed chip size, return.
    //
    if(ulWriteAddr >= AT45DBxxxDInfo.usPageSize * AT45DBxxxDInfo.ulTatalPages) return AT45DBxxxD_OP_INVALID;
    //
    // calculate page address and offset address in page
    //
    ulPageAddr = ulWriteAddr / AT45DBxxxDInfo.usPageSize;
    usOffAddr = ulWriteAddr % AT45DBxxxDInfo.usPageSize;
    if((usOffAddr + ulNumByteToWrite) > AT45DBxxxDInfo.usPageSize)
    {
    	//
    	// if writing data exceeds the boundary of a page,
    	// divide data into 3 parts:
    	// 1: start address to the boundary of the first page
    	// 2: second page to the last complete pages counts
    	// 3: the last incomplete page to the last byte
    	//
        usLeftForward = AT45DBxxxDInfo.usPageSize - usOffAddr;
        usPages = (ulNumByteToWrite - usLeftForward) / AT45DBxxxDInfo.usPageSize;
        usLeftBehind = (ulNumByteToWrite - usLeftForward) % AT45DBxxxDInfo.usPageSize;
    }
    else
    {
    	//
    	// if reading data dosen't exceed one page
    	//
        usLeftForward = ulNumByteToWrite;
        usPages = 0;
        usLeftBehind = 0;
    }
    //printf("ulPageAddr=%x,pucBuffer=%x,usOffAddr=%x,usLeftForward=%x\n",ulPageAddr,pucBuffer,usOffAddr,usLeftForward);
//    while(!AT45DBXXX_WaitForWriteEnd());

    //
    // read the first page from ulPageAddr to buffer
    //
    AT45DBxxxD_MmtoBuf(AT45DBxxxD_BUF1, ulPageAddr);
    //
    // Write the 1st part of data to buffer
    //
    AT45DBxxxD_BufferWrite(AT45DBxxxD_BUF1, pucBuffer, usOffAddr, usLeftForward);
    //
    // write buffer data to original page address
    //
    AT45DBxxxD_BuftoMm(AT45DBxxxD_BUF1, ulPageAddr);

    ulBytesWritten += usLeftForward;
    ulPageAddr++;

    //
    // Write the 2nd part of data to buffer
    //
    while(usPages)
    {
        AT45DBxxxD_BufferWrite(AT45DBxxxD_BUF1, &pucBuffer[ulBytesWritten],
                              0, AT45DBxxxDInfo.usPageSize);
        AT45DBxxxD_BuftoMm(AT45DBxxxD_BUF1, ulPageAddr);
        if(AT45DBXXX_GetStatus()&AT45DBXXX_STATUS_PROTECT) return AT45DBxxxD_OP_INVALID;
        ulBytesWritten += AT45DBxxxDInfo.usPageSize;
        ulPageAddr++;
        usPages--;
    }
    //
    // Write the 3rd part of data to buffer
    //
    if(usLeftBehind != 0)
    {
        AT45DBxxxD_MmtoBuf(AT45DBxxxD_BUF1, ulPageAddr);
        AT45DBxxxD_BufferWrite(AT45DBxxxD_BUF1, &pucBuffer[ulBytesWritten], 0, usLeftBehind);
        AT45DBxxxD_BuftoMm(AT45DBxxxD_BUF1, ulPageAddr);
        if(AT45DBXXX_GetStatus()&AT45DBXXX_STATUS_PROTECT) return AT45DBxxxD_OP_INVALID;
    }
    return AT45DBxxxD_OP_OK;
}























