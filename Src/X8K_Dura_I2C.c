/*************************************************************************************
 *
 *
 *************************************************************************************/

#include "main.h"
#include <string.h>

#include "X8K_Dura_I2C.h"


#define I2C_ADDRESS     (0x6E << 1)


ControlWord_WordAddr08d ControlWord_08d;
ControlWord_WordAddr18d ControlWord_18d;
ControlWord_WordAddr20d ControlWord_20d;
ControlWord_WordAddr22d ControlWord_22d;

#ifndef SIMPLE_DEMO_MODE
BridgeData_Transfer BridgeData;
MemoryData_Transfer MemoryData;
QuickData_Transfer QuickData;
// #else
CheckData_Transfer CheckData;
#endif
Control_Method ControlMethod;

uint16_t EPC_Private[8];    // MSB first
uint16_t EPC_Public[6];     // MSB first

uint8_t bBridge_SeqNum = 0;
uint8_t bMemory_SeqNum = 0;


//==============================================================================

void TAG_Init(void)
{
    uint16_t wControlData[2];

    // 
    wControlData[0] = 0x0800;   // 08d(MSB first)

    I2C_Write_Word(I2C_ADDRESS, wControlData, 1);
    I2C_Read_Byte(I2C_ADDRESS, ControlWord_08d.Word_Data, 2);

    ControlWord_08d.Bit_Data.I2C_Addr = 0x3;

    memcpy((uint8_t *)(wControlData + 1), (uint8_t *)ControlWord_08d.Word_Data, 2);
    I2C_Write_Word(I2C_ADDRESS, wControlData, 2);

    // 
    wControlData[0] = 0x1200;   // 18d(MSB first)

    I2C_Write_Word(I2C_ADDRESS, wControlData, 1);
    I2C_Read_Byte(I2C_ADDRESS, ControlWord_18d.Word_Data, 2);

    // 
    wControlData[0] = 0x1400;   // 20d(MSB first)

    I2C_Write_Word(I2C_ADDRESS, wControlData, 1);
    I2C_Read_Byte(I2C_ADDRESS, ControlWord_20d.Word_Data, 2);

    ControlWord_20d.Bit_Data.DCI_RF_En = 0x0;
    // Write WWU to 1
    ControlWord_20d.Bit_Data.WWU = 0x1;     // for Wakeup

    memcpy((uint8_t *)(wControlData + 1), (uint8_t *)ControlWord_20d.Word_Data, 2);
    I2C_Write_Word(I2C_ADDRESS, wControlData, 2);

    // 
    wControlData[0] = 0x1600;   // 22d(MSB first)

    I2C_Write_Word(I2C_ADDRESS, wControlData, 1);
    I2C_Read_Byte(I2C_ADDRESS, ControlWord_22d.Word_Data, 2);

}

//==============================================================================
// 
// The Byte Address's range of USER Bank in Monza X-8K Dura is from 64d to 1087d.
//
// The USER Bank memory is arranged as following :
//

// -- 1087d --
//              < Reserved for optional Command/Data >
// -- 1084d --

// -- 1083d --
//              < Used for Check mode transfer Command/Data >
// -- 1068d --

// -- 1067d --
//              < Used for Quick mode transfer Command/Data >
// -- 1036d//1024d --

// -- 1035d//1075d --
//              < Used for GPIO/BUSY_N control Method >
// -- 1034d//1074d --

// -- 1033d//1085d --
//              < Used for Bridge transfer Command/Data >
// -- 1024d//1076d --

// -- 1023d//1055d --
//              < Used for Memory transfer Data with 59//62 lines > // 59-lines data + 1-line command = 60-lines (8 words x 60 lines = 32 words bulk-write x 15 times)
// -- 0080d//0064d --

// -- 0079d//1087d --
//              < Used for Memory transfer Command > // Use full 1-line (16 byte) memory
// -- 0064d//1084d --

// Change vvv
// -- 1023d --
//              < Used for Memory transfer Command > // Use full 1-line (16 byte) memory
// -- 1008d --

// -- 1007d --
//              < Used for Memory transfer Data with 59//62 lines > // 59-lines data + 1-line command = 60-lines (8 words x 60 lines = 32 words bulk-write x 15 times)
// -- 0064d --
// Change ^^^
// 
//==============================================================================

#ifndef SIMPLE_DEMO_MODE
uint8_t TAG_Get_BridgeData(void)
{
    TAG_Read_User(0x0400, BridgeData.Word_Data, 10);      // Addr: 1024d

    // if(BridgeData.Bit_Data.SeqNum == bBridge_SeqNum)
    if(BridgeData.Bit_Data.SeqNum > 0)
    {
        // bBridge_SeqNum = bBridge_SeqNum ^ 0x01;
        BridgeData.Bit_Data.SeqNum = 0;
        TAG_Write_User(0x0400, (uint16_t *)BridgeData.Word_Data, 5);   // Addr: 1024d

        return 1;   // Data is new
    }

    return 0;
}

uint8_t TAG_Get_MemoryData(void)
{
    TAG_Read_User(0x03F0, MemoryData.Word_Data, 4);      // Addr: 1008d//0064d

    // if(MemoryData.Bit_Data.SeqNum == bMemory_SeqNum)
    if(MemoryData.Bit_Data.SeqNum > 0)
    {
        // bMemory_SeqNum = bMemory_SeqNum ^ 0x01;
        MemoryData.Bit_Data.SeqNum = 0;
        TAG_Write_User(0x03F0, (uint16_t *)MemoryData.Word_Data, 2);   // Addr: 1008d//0064d

        return 1;   // Data is new
    }

    return 0;
}

void TAG_Set_BusyStatus(uint8_t bIsBusy)
{
    TAG_Read_User(0x03F0, MemoryData.Word_Data, 2);      // Addr: 1008d//0064d
    TAG_Read_User(0x0400, BridgeData.Word_Data, 2);      // Addr: 1024d
//  TAG_Read_User(0x040C, QuickData.Word_Data, 2);       // Addr: 1036d // Already got QuickData ?!

    if(bIsBusy == 1)
    {
        MemoryData.Bit_Data.CdData = 0;     // Clear Bit 1: CdData (temp for busy status)
        BridgeData.Bit_Data.CdData = 0;
//      QuickData.Byte_Data.NewFlag = 2;
    }
    else
    {
        MemoryData.Bit_Data.CdData = 1;     // Set   Bit 1: CdData (temp for busy status)
        BridgeData.Bit_Data.CdData = 1;
//      QuickData.Byte_Data.NewFlag = 0;
    }

    TAG_Write_User(0x03F0, (uint16_t *)MemoryData.Word_Data, 1);      // Addr: 1008d//0064d
    TAG_Write_User(0x0400, (uint16_t *)BridgeData.Word_Data, 1);      // Addr: 1024d
//  TAG_Write_User(0x040C, (uint16_t *)QuickData.Word_Data, 1);       // Addr: 1036d
}

uint8_t TAG_Get_ControlMethod(void)
{
    TAG_Read_User(0x040A, ControlMethod.Word_Data, 2);      // Addr: 1034d

    if(ControlMethod.Bit_Data.SeqNum > 0)
    {
        ControlMethod.Bit_Data.SeqNum = 0;    // Un-used if TAG_Set_BusyStatus() is used
        TAG_Write_User(0x040A, (uint16_t *)ControlMethod.Word_Data, 1);     // Addr: 1034d

        return 1;   // Data is new
    }

    return 0;
}

void TAG_Reset_CmdStatus(void)
{
    // For BridgeData_Transfer
    TAG_Read_User(0x0400, BridgeData.Word_Data, 2);      // Addr: 1024d

    BridgeData.Bit_Data.XcsCtl = 0;
    BridgeData.Bit_Data.CdData = 1;
    BridgeData.Bit_Data.SeqNum = 0;

    TAG_Write_User(0x0400, (uint16_t *)BridgeData.Word_Data, 1);      // Addr: 1024d

    // For MemoryData_Transfer
    TAG_Read_User(0x03F0, MemoryData.Word_Data, 2);      // Addr: 1008d//0064d

    MemoryData.Bit_Data.XcsCtl = 0;
    MemoryData.Bit_Data.CdData = 1;
    MemoryData.Bit_Data.SeqNum = 0;

    TAG_Write_User(0x03F0, (uint16_t *)MemoryData.Word_Data, 1);      // Addr: 1008d//0064d
}

uint8_t TAG_Get_QuickData(void)
{
    TAG_Read_User(0x040C, QuickData.Word_Data, 32);      // Addr: 1036d

    if((QuickData.Byte_Data.CmdHead == 0xAB) && (QuickData.Byte_Data.CmdTail == 0xCD))
    {
        if(QuickData.Byte_Data.ChkFlag > 0x00)
        {
            QuickData.Byte_Data.ChkFlag = 0x00;    // Un-used if TAG_Set_BusyStatus() is used
            TAG_Write_User(0x040C, (uint16_t *)QuickData.Word_Data, 16);      // Addr: 1036d

            return 1;   // Data is new
        }
    }

    return 0;
}

// #else
uint8_t TAG_Get_CheckData(void)
{
    TAG_Read_User(0x042C, CheckData.Word_Data, 16);      // Addr: 1068d//0064d

    if((CheckData.Byte_Data.CmdHead == 0xAB) && (CheckData.Byte_Data.CmdTail == 0xCD))
    {
        if(CheckData.Byte_Data.NewFlag == 1)
        {
            CheckData.Byte_Data.NewFlag = 0;
            TAG_Write_User(0x042C, (uint16_t *)CheckData.Word_Data, 1);      // Addr: 1068d//0064d

            return 1;   // Data is new
        }
    }

    return 0;
}
#endif

//==============================================================================

void TAG_Read_User(uint16_t ByteAddr, uint8_t *pbData, uint16_t ByteNum)
{
    uint16_t wReadData;

    wReadData = ((ByteAddr << 8) | (ByteAddr >> 8));   // MSB first

    I2C_Write_Word(I2C_ADDRESS, &wReadData, 1);
    I2C_Read_Byte(I2C_ADDRESS, pbData, ByteNum);
}

void TAG_Write_User(uint16_t ByteAddr, uint16_t *pwData, uint16_t WordNum)   // Tag only allows to write with even Byte address
{
    uint16_t i;
    uint16_t wWriteData[2];

    for(i = 0; i < WordNum; i++)
    {
        wWriteData[0] = ((ByteAddr << 8) | (ByteAddr >> 8));   // MSB first
        memcpy((uint8_t *)(wWriteData + 1), (uint8_t *)(pwData + i), 2);

        I2C_Write_Word(I2C_ADDRESS, wWriteData, 2);

        // HAL_Delay(1);   // 1 ms

        ByteAddr += 2;
    }
}

//==============================================================================

void I2C_Write_Word(uint8_t DeviceAddr, uint16_t *pwData, uint16_t WordNum)     // Tag Only allows to write (1+1) or (1+2) words including memory address word in one time
{
    do
    {
        if(HAL_I2C_Master_Transmit_IT(&I2cHandle, (uint16_t)DeviceAddr, (uint8_t *)pwData, WordNum * 2) != HAL_OK)
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

void I2C_Read_Byte(uint8_t DeviceAddr, uint8_t *pbData, uint16_t ByteNum)
{
    do
    {
        if(HAL_I2C_Master_Receive_IT(&I2cHandle, (uint16_t)DeviceAddr, (uint8_t *)pbData, ByteNum) != HAL_OK)
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



