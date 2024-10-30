/*************************************************************************************
 *
 *
 *************************************************************************************/

#include "main.h"
#include <string.h>

#include "X8K_Dura_I2C.h"
#include "EEPROM_I2C.h"


#define EEPROM_I2C_ADDR     (0x50 << 1)     // A2/A1/A0 = 0/0/0


//==============================================================================

void EEPROM_Read_Byte(uint16_t ByteAddr, uint8_t *pbData, uint16_t ByteNum)
{
    uint8_t wReadData[2];

    wReadData[0] = (ByteAddr >> 8) & 0xFF;     // MSB first
    wReadData[1] =  ByteAddr & 0xFF;

    I2C_Write_Byte(EEPROM_I2C_ADDR, wReadData, 2);
    I2C_Read_Byte(EEPROM_I2C_ADDR, pbData, ByteNum);
}

void EEPROM_Write_Byte(uint16_t ByteAddr, uint8_t *pbData, uint16_t ByteNum)
{
    uint16_t i;
    uint8_t bWriteData[2 + 1];

    for(i = 0; i < ByteNum; i++)
    {
        bWriteData[0] = (ByteAddr >> 8) & 0xFF;     // MSB first
        bWriteData[1] =  ByteAddr & 0xFF;

        memcpy((bWriteData + 2), (pbData + i), 1);

        I2C_Write_Byte(EEPROM_I2C_ADDR, bWriteData, 2 + 1);

        // HAL_Delay(1);   // 1 ms

        ByteAddr += 1;
    }
}

void EEPROM_Write_Page(uint16_t ByteAddr, uint8_t *pbData, uint16_t ByteNum)
{
    uint8_t bWriteData[2 + 64];     // 1 Page = 64 Bytes
    uint16_t wRemainBytes, wWrittenBytes, wWrittenIndex;

    wRemainBytes = ByteNum;
    wWrittenIndex = 0;

    while(wRemainBytes > 0)
    {
        bWriteData[0] = (ByteAddr >> 8) & 0xFF;     // MSB first
        bWriteData[1] =  ByteAddr & 0xFF;

        if(wRemainBytes > 64)
            wWrittenBytes = 64;
        else
            wWrittenBytes = wRemainBytes;

        memcpy((bWriteData + 2), pbData + wWrittenIndex, wWrittenBytes);

        I2C_Write_Byte(EEPROM_I2C_ADDR, bWriteData, 2 + wWrittenBytes);

        wRemainBytes -= wWrittenBytes;
        wWrittenIndex += wWrittenBytes;
        ByteAddr += wWrittenBytes;
    }
}

//==============================================================================

void I2C_Write_Byte(uint8_t DeviceAddr, uint8_t *pbData, uint16_t ByteNum)
{
    do
    {
        if(HAL_I2C_Master_Transmit_IT(&I2cHandle, (uint16_t)DeviceAddr, (uint8_t *)pbData, ByteNum) != HAL_OK)
        {
          /* Error_Handler() function is called when error occurs. */
          // Error_Handler();
        }

        /*  Before starting a new communication transfer, you need to check the current   
            state of the peripheral; if it’s busy you need to wait for the end of current
            transfer before starting a new one.
            For simplicity reasons, this example is just waiting till the end of the 
            transfer, but application may perform other tasks while transfer operation
            is ongoing. */  
        while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
        {
        } 

        /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
           Master restarts communication */
    }
    while(HAL_I2C_GetError(&I2cHandle) == HAL_I2C_ERROR_AF);
}



