#ifndef __EEPROMI2C_H__
#define __EEPROMI2C_H__


#define EEPROM_BITMAP_1     0       // Start address of 1st image map
#define EEPROM_BITMAP_2     4736    // Start address of 2nd image map
#define EEPROM_BITMAP_3     9472    // Start address of 3rd image map


//==============================================================================
// 
// The Byte Address's range of 24AA128 EEPROM is from 0d to 16383d. (16K Bytes)
//
// The memory for image bitmaps is arranged as following :
//

// 
// -- 16383d --
//              < Reserved >
// -- 14208d --
// 
// -- 14207d --
//              < 3rd image bitmap >
// -- 09472d --
// 
// -- 09471d --
//              < 2nd image bitmap >
// -- 04736d --
// 
// -- 04735d --
//              < 1st image bitmap >
// -- 00000d --
// 
//==============================================================================


/* Exported types ------------------------------------------------------------*/


/* Exported variables --------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
void EEPROM_Read_Byte(uint16_t ByteAddr, uint8_t *pbData, uint16_t ByteNum);
void EEPROM_Write_Byte(uint16_t ByteAddr, uint8_t *pbData, uint16_t ByteNum);
void EEPROM_Write_Page(uint16_t ByteAddr, uint8_t *pbData, uint16_t ByteNum);
void I2C_Write_Byte(uint8_t DeviceAddr, uint8_t *pbData, uint16_t ByteNum);


#endif


