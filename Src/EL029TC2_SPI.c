/*************************************************************************************
 *
 *
 *************************************************************************************/

#define __EL029TC2_C__

#include "main.h"
#include <string.h>

#include "EL029TC2_SPI.h"
#include "EPD_Bitmap.h"
#include "Code128_Encode.h"


//==============================================================================

#define BAR_START_LINE      (295 - 0)//(295 - 44)
#define BAR_LINE_WIDTH      2

//==============================================================================

static const unsigned char* Ascii_Digit_Bitmap_12x12[] = {  // offset from ascii code 0x30

    &aBitmap_30H_12x12[0][0],
    &aBitmap_31H_12x12[0][0],
    &aBitmap_32H_12x12[0][0],
    &aBitmap_33H_12x12[0][0],
    &aBitmap_34H_12x12[0][0],
    &aBitmap_35H_12x12[0][0],
    &aBitmap_36H_12x12[0][0],
    &aBitmap_37H_12x12[0][0],
    &aBitmap_38H_12x12[0][0],
    &aBitmap_39H_12x12[0][0],

};

static const unsigned char* Ascii_BigCh_Bitmap_12x12[] = {  // offset from ascii code 0x41

    &aBitmap_41H_12x12[0][0],
    &aBitmap_42H_12x12[0][0],
    &aBitmap_43H_12x12[0][0],
    &aBitmap_44H_12x12[0][0],
    &aBitmap_45H_12x12[0][0],
    &aBitmap_46H_12x12[0][0],
    &aBitmap_47H_12x12[0][0],
    &aBitmap_48H_12x12[0][0],
    &aBitmap_49H_12x12[0][0],
    &aBitmap_4AH_12x12[0][0],
    &aBitmap_4BH_12x12[0][0],
    &aBitmap_4CH_12x12[0][0],
    &aBitmap_4DH_12x12[0][0],
    &aBitmap_4EH_12x12[0][0],
    &aBitmap_4FH_12x12[0][0],

    &aBitmap_50H_12x12[0][0],
    &aBitmap_51H_12x12[0][0],
    &aBitmap_52H_12x12[0][0],
    &aBitmap_53H_12x12[0][0],
    &aBitmap_54H_12x12[0][0],
    &aBitmap_55H_12x12[0][0],
    &aBitmap_56H_12x12[0][0],
    &aBitmap_57H_12x12[0][0],
    &aBitmap_58H_12x12[0][0],
    &aBitmap_59H_12x12[0][0],
    &aBitmap_5AH_12x12[0][0],

};

static const unsigned char* Ascii_SmallCh_Bitmap_12x12[] = {  // offset from ascii code 0x61

    &aBitmap_61H_12x12[0][0],
    &aBitmap_62H_12x12[0][0],
    &aBitmap_63H_12x12[0][0],
    &aBitmap_64H_12x12[0][0],
    &aBitmap_65H_12x12[0][0],
    &aBitmap_66H_12x12[0][0],
    &aBitmap_67H_12x12[0][0],
    &aBitmap_68H_12x12[0][0],
    &aBitmap_69H_12x12[0][0],
    &aBitmap_6AH_12x12[0][0],
    &aBitmap_6BH_12x12[0][0],
    &aBitmap_6CH_12x12[0][0],
    &aBitmap_6DH_12x12[0][0],
    &aBitmap_6EH_12x12[0][0],
    &aBitmap_6FH_12x12[0][0],

    &aBitmap_70H_12x12[0][0],
    &aBitmap_71H_12x12[0][0],
    &aBitmap_72H_12x12[0][0],
    &aBitmap_73H_12x12[0][0],
    &aBitmap_74H_12x12[0][0],
    &aBitmap_75H_12x12[0][0],
    &aBitmap_76H_12x12[0][0],
    &aBitmap_77H_12x12[0][0],
    &aBitmap_78H_12x12[0][0],
    &aBitmap_79H_12x12[0][0],
    &aBitmap_7AH_12x12[0][0],

};

//==============================================================================
unsigned char aPixelBytes[296][16];// = {0xFF};     // 296 Lines x (128 / 8) Bytes

unsigned char aConvert_Bitmap[24][16 / 8];//[16][16 / 8];
unsigned char aMirror_Bitmap[24][16 / 8];//[16][16 / 8];
unsigned char aRotate_Bitmap[24][24 / 8];//[16][16 / 8];
unsigned char aScale_Bitmap[48][24 / 8];//[24][24 / 8];//[32][32 / 8];

const char *str = "0123456776543210";//"00 12345678 0000000001";
char barcode_data[(MAX_CODE_NUM + 1) * 11];     // Only including Data codes and Checksum code


//==============================================================================
// Check Busy
//==============================================================================

void check_busy_high(void)// If BUSYN=0 then waiting
{
    // delay(2000);
    HAL_Delay(40);   // 40 ms

    while(!(HAL_GPIO_ReadPin(EPD_BUSYN_GPIO_PORT, EPD_BUSYN_PIN)));
}

void check_busy_low(void)// If BUSYN=1 then waiting
{
    // delay(2000);
    HAL_Delay(40);   // 40 ms

    while(HAL_GPIO_ReadPin(EPD_BUSYN_GPIO_PORT, EPD_BUSYN_PIN));
}

//==============================================================================
// EPD initial
//==============================================================================

void SPI_8b_Init(void)
{
    // __HAL_SPI_DISABLE(&SpiHandle);  // let NSS high
    HAL_GPIO_WritePin(EPD_CS_GPIO_PORT, EPD_CS_PIN, GPIO_PIN_SET);

    HAL_GPIO_WritePin(EPD_BS_GPIO_PORT, EPD_BS_PIN, GPIO_PIN_RESET);   // 4-wire IF

    HAL_GPIO_WritePin(EPD_RSTN_GPIO_PORT, EPD_RSTN_PIN, GPIO_PIN_SET);

    HAL_Delay(20);   // 20 ms
}

void EPD_Init(char epd_clear)
{
    unsigned int i, j;

    SPI_8b_Init();

    // 
    HAL_GPIO_WritePin(EPD_RSTN_GPIO_PORT, EPD_RSTN_PIN, GPIO_PIN_RESET);   // Reset
    // delay(100);
    HAL_Delay(2);   // 2 ms
    HAL_GPIO_WritePin(EPD_RSTN_GPIO_PORT, EPD_RSTN_PIN, GPIO_PIN_SET);
    // delay(1000);
    HAL_Delay(20);   // 20 ms

    // 
    byte_counter = 4736; // (Pixel size)/(Pixel) = (296*128)/8

    SPI_8b_Send_8b(PON);
    check_busy_high();

    SPI_8b_Send_8b(PSR);
    SPI_8b_Send(1,0x9F);

    SPI_8b_Send_8b(PFS);
    SPI_8b_Send(1,0x00);

    SPI_8b_Send_8b(BTST);
    SPI_8b_Send(1,0x17);
    SPI_8b_Send(1,0x17);
    SPI_8b_Send(1,0x17);

    SPI_8b_Send_8b(TSE);
    SPI_8b_Send(1,0x00);

    SPI_8b_Send_8b(CDI);
    SPI_8b_Send(1,0x97);

    SPI_8b_Send_8b(TCON);
    SPI_8b_Send(1,0x22);

    if(epd_clear == 1)      // Clear white
    {
        SPI_8b_Send_8b(DTM1);

        for(i = 0; i < 296; i++)
        {
            memset(&aPixelBytes[i][0], 0xFF, 16);   // Fill in all white backcolor

            for(j = 0; j < 16; j++)
                SPI_8b_Send(1, aPixelBytes[i][j]);
        }
    }
    else if(epd_clear == 2)      // Clear black
    {
        SPI_8b_Send_8b(DTM1);

        for(i = 0; i < 296; i++)
        {
            memset(&aPixelBytes[i][0], 0x00, 16);   // Fill in all black backcolor

            for(j = 0; j < 16; j++)
                SPI_8b_Send(1, aPixelBytes[i][j]);
        }
    }

    SPI_8b_Send_8b(POF);  
    check_busy_high();

}

//==============================================================================
// Barcode Display
//==============================================================================

void Barcode_Bar_Update(unsigned int code_bar_num)
{
    unsigned int i, j, wStart_Line = BAR_START_LINE, wBar_Lines = BAR_LINE_WIDTH, wBar_Bytes = 5;//10;

    // Quiet zone
    for(i = 0; i < 10; i++)
    {
        for(j = 0; j < wBar_Lines; j++)
            EPD_Pixels_Update((unsigned char *)&aBitmap_WhiteBar[0], wStart_Line - ((i * wBar_Lines) + j), 1, 0, wBar_Bytes);
    }

    // Data codes
    wStart_Line -= 10 * wBar_Lines;
    for(i = 0; i < code_bar_num; i++)
    {
        if(barcode_data[i] == 0xFF)     // Black bar
        {
            for(j = 0; j < wBar_Lines; j++)
                EPD_Pixels_Update((unsigned char *)&aBitmap_BlackBar[0], wStart_Line - ((i * wBar_Lines) + j), 1, 0, wBar_Bytes);
        }
        else                            // White bar
        {
            for(j = 0; j < wBar_Lines; j++)
                EPD_Pixels_Update((unsigned char *)&aBitmap_WhiteBar[0], wStart_Line - ((i * wBar_Lines) + j), 1, 0, wBar_Bytes);
        }
    }

    // Check code
    wStart_Line -= code_bar_num * wBar_Lines;
    for(i = 0; i < 11; i++)
    {
        if(barcode_data[code_bar_num + i] == 0xFF)    // Black bar
        {
            for(j = 0; j < wBar_Lines; j++)
                EPD_Pixels_Update((unsigned char *)&aBitmap_BlackBar[0], wStart_Line - ((i * wBar_Lines) + j), 1, 0, wBar_Bytes);
        }
        else                                        // White bar
        {
            for(j = 0; j < wBar_Lines; j++)
                EPD_Pixels_Update((unsigned char *)&aBitmap_WhiteBar[0], wStart_Line - ((i * wBar_Lines) + j), 1, 0, wBar_Bytes);
        }
    }

    // Stop code
    wStart_Line -= 11 * wBar_Lines;
    for(i = 0; i < 13; i++)
    {
        if(aBitmap_StopCode[i] == 0xFF) // Black bar
        {
            for(j = 0; j < wBar_Lines; j++)
                EPD_Pixels_Update((unsigned char *)&aBitmap_BlackBar[0], wStart_Line - ((i * wBar_Lines) + j), 1, 0, wBar_Bytes);
        }
        else                            // White bar
        {
            for(j = 0; j < wBar_Lines; j++)
                EPD_Pixels_Update((unsigned char *)&aBitmap_WhiteBar[0], wStart_Line - ((i * wBar_Lines) + j), 1, 0, wBar_Bytes);
        }
    }

    // Quiet zone
    wStart_Line -= 13 * wBar_Lines;
    for(i = 0; i < 10; i++)
    {
        for(j = 0; j < wBar_Lines; j++)
            EPD_Pixels_Update((unsigned char *)&aBitmap_WhiteBar[0], wStart_Line - ((i * wBar_Lines) + j), 1, 0, wBar_Bytes);
    }
}

void Barcode_Digit_Update(const char *s)
{
    unsigned int i, wStart_Line = BAR_START_LINE - (10 * BAR_LINE_WIDTH);

    while(*s != 0)
    {
        if((*s >= 0x30) && (*s <= 0x39))
            Bitmap_Rotation_Convert((unsigned char *)Ascii_Digit_Bitmap_12x12[*s - 0x30], 12, (12 / 8) + 1, 3);
        else if((*s >= 0x41) && (*s <= 0x5A))
            Bitmap_Rotation_Convert((unsigned char *)Ascii_BigCh_Bitmap_12x12[*s - 0x41], 12, (12 / 8) + 1, 3);
        else
            Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3FH_12x12[0][0], 12, (12 / 8) + 1, 3);    // use '?' tempararily

        Bitmap_Invert_Color(&aRotate_Bitmap[0][0], 3, 12, (12 / 8) + 1);
        Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 16);
        // Bitmap_Invert_Color(&aScale_Bitmap[0][0], 3, 16, (16 / 8));

        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[(16 - 1) - i][0], wStart_Line - i, 7, 0, (16 / 8));
        }

        s++;
        wStart_Line -= 16;
    }
}

//==============================================================================
// Display Mode Update
//==============================================================================

void Check_Mode_Update(uint8_t *paCheckData)
{
    const char *err_str = "NG Item Error";
    const char dis_msg[] = "DIS.:\?\? Dock";
    const char date_msg[] = "Time:\?\?\?\?/\?\?/\?\? \?\?:\?\?";

    char dis_msg1[sizeof(dis_msg)];
    memcpy(dis_msg1, dis_msg, sizeof(dis_msg));
    char *dis_str = dis_msg1;

    char date_msg1[sizeof(date_msg)];
    memcpy(date_msg1, date_msg, sizeof(date_msg));
    char *date_str = date_msg1;

    unsigned int i, wStart_Line;

    // for(i = 0; i < 296; i++)
        // memset(&aPixelBytes[i][0], 0xFF, 16);

    if(paCheckData[2] == 0x01)//(CheckData.Byte_Data.ErrStatus == 0x01)
    {
        wStart_Line = 295 - 18;

        // Show the error message
        while(*err_str != 0)
        {
            if((*err_str >= 0x41) && (*err_str <= 0x5A))
                Bitmap_Rotation_Convert((unsigned char *)Ascii_BigCh_Bitmap_12x12[*err_str - 0x41], 12, (12 + 7) / 8, 3);
            else if((*err_str >= 0x61) && (*err_str <= 0x7A))
                Bitmap_Rotation_Convert((unsigned char *)Ascii_SmallCh_Bitmap_12x12[*err_str - 0x61], 12, (12 + 7) / 8, 3);
            else if(*err_str == ' ')
                Bitmap_Rotation_Convert((unsigned char *)&aBitmap_20H_12x12[0][0], 12, (12 + 7) / 8, 3);

            Bitmap_Invert_Color(&aRotate_Bitmap[0][0], 3, 12, (12 + 7) / 8);
            Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 20, 24);//16, 16);
            // Bitmap_Invert_Color(&aScale_Bitmap[0][0], 3, 20, (24 + 7) / 8);//16, (16 + 7) / 8);

            for(i = 0; i < 20; i++)//16; i++)
            {
                // EPD_Pixels_Update(&aScale_Bitmap[(16 - 1) - i][0], wStart_Line - i, 7, 0, (16 + 7) / 8);
                EPD_Pixels_Update(&aScale_Bitmap[(20 - 1) - i][0], wStart_Line - i, 6, 4, (24 + 7) / 8);
            }

            err_str++;
            wStart_Line -= 20;//16;
        }

    }
    else if(paCheckData[2] == 0x02) // Check mode result
    {
        wStart_Line = 295 - 22;

        // Update the dock string
        if(paCheckData[3] != 0x00)
            dis_str[5] = paCheckData[3];
        if(paCheckData[4] != 0x00)
            dis_str[6] = paCheckData[4];

        while(*dis_str != 0)
        {
            if((*dis_str >= 0x41) && (*dis_str <= 0x5A))
                Bitmap_Rotation_Convert((unsigned char *)Ascii_BigCh_Bitmap_12x12[*dis_str - 0x41], 12, (12 + 7) / 8, 3);
            else if((*dis_str >= 0x61) && (*dis_str <= 0x7A))
                Bitmap_Rotation_Convert((unsigned char *)Ascii_SmallCh_Bitmap_12x12[*dis_str - 0x61], 12, (12 + 7) / 8, 3);
            else if((*dis_str >= 0x30) && (*dis_str <= 0x39))
                Bitmap_Rotation_Convert((unsigned char *)Ascii_Digit_Bitmap_12x12[*dis_str - 0x30], 12, (12 + 7) / 8, 3);
            else if(*dis_str == '.')
                Bitmap_Rotation_Convert((unsigned char *)&aBitmap_2EH_12x12[0][0], 12, (12 + 7) / 8, 3);
            else if(*dis_str == ':')
                Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3AH_12x12[0][0], 12, (12 + 7) / 8, 3);
            else if(*dis_str == ' ')
                Bitmap_Rotation_Convert((unsigned char *)&aBitmap_20H_12x12[0][0], 12, (12 + 7) / 8, 3);
            else if(*dis_str == '?')
            {
                dis_str++;
                continue;
            }

            // Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 20, 24);
            Bitmap_Invert_Color(&aRotate_Bitmap[0][0], 3, 12, (12 + 7) / 8);

            for(i = 0; i < 12; i++)
            {
                EPD_Pixels_Update(&aRotate_Bitmap[(12 - 1) - i][0], wStart_Line - i, 2, 0, (12 + 7) / 8);
            }

            dis_str++;
            wStart_Line -= 12;
        }

        wStart_Line = 295 - 22;

        // Update the date/time
        uint16_t uwYear = paCheckData[5] + 1900;
        date_str[5] = 0x30 + (uwYear / 1000);
        date_str[6] = 0x30 + ((uwYear % 1000) / 100);
        date_str[7] = 0x30 + ((uwYear % 100) / 10);
        date_str[8] = 0x30 + (uwYear % 10);

        if(paCheckData[6] < 10)
            date_str[10] = 0x30 + paCheckData[6];
        else
        {
            date_str[10] = 0x30 + (paCheckData[6] / 10);
            date_str[11] = 0x30 + (paCheckData[6] % 10);
        }

        if(paCheckData[7] < 10)
            date_str[13] = 0x30 + paCheckData[7];
        else
        {
            date_str[13] = 0x30 + (paCheckData[7] / 10);
            date_str[14] = 0x30 + (paCheckData[7] % 10);
        }

        if(paCheckData[8] < 10)
            date_str[16] = 0x30 + paCheckData[8];
        else
        {
            date_str[16] = 0x30 + (paCheckData[8] / 10);
            date_str[17] = 0x30 + (paCheckData[8] % 10);
        }

        if(paCheckData[9] < 10)
            date_str[19] = 0x30 + paCheckData[9];
        else
        {
            date_str[19] = 0x30 + (paCheckData[9] / 10);
            date_str[20] = 0x30 + (paCheckData[9] % 10);
        }

        while(*date_str != 0)
        {
            if((*date_str >= 0x41) && (*date_str <= 0x5A))
                Bitmap_Rotation_Convert((unsigned char *)Ascii_BigCh_Bitmap_12x12[*date_str - 0x41], 12, (12 + 7) / 8, 3);
            else if((*date_str >= 0x61) && (*date_str <= 0x7A))
                Bitmap_Rotation_Convert((unsigned char *)Ascii_SmallCh_Bitmap_12x12[*date_str - 0x61], 12, (12 + 7) / 8, 3);
            else if((*date_str >= 0x30) && (*date_str <= 0x39))
                Bitmap_Rotation_Convert((unsigned char *)Ascii_Digit_Bitmap_12x12[*date_str - 0x30], 12, (12 + 7) / 8, 3);
            else if(*date_str == ':')
                Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3AH_12x12[0][0], 12, (12 + 7) / 8, 3);
            else if(*date_str == '/')
                Bitmap_Rotation_Convert((unsigned char *)&aBitmap_2FH_12x12[0][0], 12, (12 + 7) / 8, 3);
            else if(*date_str == ' ')
                Bitmap_Rotation_Convert((unsigned char *)&aBitmap_20H_12x12[0][0], 12, (12 + 7) / 8, 3);
            else if(*date_str == '?')
            {
                date_str++;
                continue;
            }

            // Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 20, 24);
            Bitmap_Invert_Color(&aRotate_Bitmap[0][0], 3, 12, (12 + 7) / 8);

            for(i = 0; i < 12; i++)
            {
                EPD_Pixels_Update(&aRotate_Bitmap[(12 - 1) - i][0], wStart_Line - i, 6, 0, (12 + 7) / 8);
            }

            date_str++;
            wStart_Line -= 12;
        }

        wStart_Line = 295 - 22;

        // Update the results
        if(paCheckData[10] == 1)
        {
            Bitmap_Rotation_Convert((unsigned char *)&aBitmap_B41H_CB_12x12[0][0], 12, (12 + 7) / 8, 3);
        }
        else
        {
            Bitmap_Rotation_Convert((unsigned char *)&aBitmap_W41H_C_12x12[0][0], 12, (12 + 7) / 8, 3);
        }

        Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 48, 24);//24, 24);

        for(i = 0; i < 48; i++)//24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[(48 - 1) - i][0], wStart_Line - i, 10, 0, (24 + 7) / 8);
        }

        wStart_Line -= (48 + 54);

        if(paCheckData[11] == 1)
        {
            Bitmap_Rotation_Convert((unsigned char *)&aBitmap_B42H_CB_12x12[0][0], 12, (12 + 7) / 8, 3);
        }
        else
        {
            Bitmap_Rotation_Convert((unsigned char *)&aBitmap_W42H_C_12x12[0][0], 12, (12 + 7) / 8, 3);
        }

        Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 48, 24);//24, 24);

        for(i = 0; i < 48; i++)//24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[(48 - 1) - i][0], wStart_Line - i, 10, 0, (24 + 7) / 8);
        }

        wStart_Line -= (48 + 54);

        if(paCheckData[12] == 1)
        {
            Bitmap_Rotation_Convert((unsigned char *)&aBitmap_B43H_CB_12x12[0][0], 12, (12 + 7) / 8, 3);
        }
        else
        {
            Bitmap_Rotation_Convert((unsigned char *)&aBitmap_W43H_C_12x12[0][0], 12, (12 + 7) / 8, 3);
        }

        Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 48, 24);//24, 24);

        for(i = 0; i < 48; i++)//24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[(48 - 1) - i][0], wStart_Line - i, 10, 0, (24 + 7) / 8);
        }


        // EPD_Pixels_Update();
    }
    else if(paCheckData[2] == 0x03) // Inventory mode result
    {

    }

}

void Quick_Mode_Update(uint8_t *paQuickData)
{
    unsigned int i, wStart_Line, bDataIdx = 2;

    const char date_msg[] = "Date 00/00/00 00:00";
    const char next_msg[] = "N.W.\?\?\?\?";         // Maximum 4 user-defined characters
    const char qty_msg[]  = "QTY \?\?\?";           // Maximum 3 digits for 1-byte value
    const char string_msg[] = "\?\?\?\?\?\?\?\?\?\?\?\?\?\?\?\?";   // Maximum 16 user-defined characters
    char serial_msq1[8 + 1] = {0x00};

    char date_msg1[sizeof(date_msg)];
    memcpy(date_msg1, date_msg, sizeof(date_msg));
    char *date_str = date_msg1;

    char next_msg1[sizeof(next_msg)];
    memcpy(next_msg1, next_msg, sizeof(next_msg));
    char *next_str = next_msg1;

    char qty_msg1[sizeof(qty_msg)];
    memcpy(qty_msg1, qty_msg, sizeof(qty_msg));
    char *qty_str = qty_msg1;

    char string_msg1[sizeof(string_msg)];
    memcpy(string_msg1, string_msg, sizeof(string_msg));
    char *string_str = string_msg1;

    size_t max_length, real_length;
    unsigned int code_bar_num;

    while(paQuickData[bDataIdx] != 0)
    {

        switch (paQuickData[bDataIdx])
        {
          // Date/Time
          case 1:       // Date hex[0] + Date hex[1] + Date hex[2] + Time hex[0] + Time hex[1]

            wStart_Line = 295 - 110;     // T.B.D

            date_str[5] = 0x30 + (paQuickData[bDataIdx + 1] / 10);
            date_str[6] = 0x30 + (paQuickData[bDataIdx + 1] % 10);

            date_str[8] = 0x30 + (paQuickData[bDataIdx + 2] / 10);
            date_str[9] = 0x30 + (paQuickData[bDataIdx + 2] % 10);

            date_str[11] = 0x30 + (paQuickData[bDataIdx + 3] / 10);
            date_str[12] = 0x30 + (paQuickData[bDataIdx + 3] % 10);

            date_str[14] = 0x30 + (paQuickData[bDataIdx + 4] / 10);
            date_str[15] = 0x30 + (paQuickData[bDataIdx + 4] % 10);

            date_str[17] = 0x30 + (paQuickData[bDataIdx + 5] / 10);
            date_str[18] = 0x30 + (paQuickData[bDataIdx + 5] % 10);

            while(*date_str != 0)
            {
                if((*date_str >= 0x41) && (*date_str <= 0x5A))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_BigCh_Bitmap_12x12[*date_str - 0x41], 12, (12 + 7) / 8, 3);
                else if((*date_str >= 0x61) && (*date_str <= 0x7A))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_SmallCh_Bitmap_12x12[*date_str - 0x61], 12, (12 + 7) / 8, 3);
                else if((*date_str >= 0x30) && (*date_str <= 0x39))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_Digit_Bitmap_12x12[*date_str - 0x30], 12, (12 + 7) / 8, 3);
                else if(*date_str == ':')
                    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3AH_12x12[0][0], 12, (12 + 7) / 8, 3);
                else if(*date_str == '/')
                    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_2FH_12x12[0][0], 12, (12 + 7) / 8, 3);
                else if(*date_str == ' ')
                    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_20H_12x12[0][0], 12, (12 + 7) / 8, 3);

                Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 9, 12);
                Bitmap_Invert_Color(&aScale_Bitmap[0][0], 3, 9, (12 + 7) / 8);

                // for(i = 0; i < 12; i++)
                for(i = 0; i < 9; i++)
                {
                    // EPD_Pixels_Update(&aRotate_Bitmap[(12 - 1) - i][0], wStart_Line - i, 6, 0, (12 + 7) / 8);
                    EPD_Pixels_Update(&aScale_Bitmap[(9 - 1) - i][0], wStart_Line - i, 12, 0, (12 + 7) / 8);
                }

                date_str++;
                wStart_Line -= 9;//12;      // T.B.D
            }

            bDataIdx += 6;
            break;

          // Next Workstation
          case 2:       // N.W ascii[0] + ... + N.W ascii[3]

            wStart_Line = 295 - 22;     // T.B.D

            for(i = 0; i < 4; i++)
            {
                if(paQuickData[(bDataIdx + 1) + i] != 0x00)
                    next_str[4 + i] = paQuickData[(bDataIdx + 1) + i];
                else
                    break;
            }

            while(*next_str != 0)
            {
                if((*next_str >= 0x41) && (*next_str <= 0x5A))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_BigCh_Bitmap_12x12[*next_str - 0x41], 12, (12 + 7) / 8, 3);
                else if((*next_str >= 0x61) && (*next_str <= 0x7A))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_SmallCh_Bitmap_12x12[*next_str - 0x61], 12, (12 + 7) / 8, 3);
                else if((*next_str >= 0x30) && (*next_str <= 0x39))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_Digit_Bitmap_12x12[*next_str - 0x30], 12, (12 + 7) / 8, 3);
                else if(*next_str == '.')
                    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_2EH_12x12[0][0], 12, (12 + 7) / 8, 3);
                // else if(*next_str == ':')
                    // Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3AH_12x12[0][0], 12, (12 + 7) / 8, 3);
                // else if(*next_str == ' ')
                    // Bitmap_Rotation_Convert((unsigned char *)&aBitmap_20H_12x12[0][0], 12, (12 + 7) / 8, 3);
                else
                {
                    next_str++;
                    continue;
                }

                Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 9, 12);
                Bitmap_Invert_Color(&aScale_Bitmap[0][0], 3, 9, (12 + 7) / 8);

                // for(i = 0; i < 12; i++)
                for(i = 0; i < 9; i++)
                {
                    // EPD_Pixels_Update(&aRotate_Bitmap[(12 - 1) - i][0], wStart_Line - i, 2, 0, (12 + 7) / 8);
                    EPD_Pixels_Update(&aScale_Bitmap[(9 - 1) - i][0], wStart_Line - i, 12, 0, (12 + 7) / 8);
                }

                next_str++;
                wStart_Line -= 9;//12;      // T.B.D
            }

            bDataIdx += 5;
            break;

          // Quantity
          case 3:       // Qty hex

            wStart_Line = 295 - 198;     // T.B.D

            if(paQuickData[bDataIdx + 1] < 10)
                qty_str[4] = 0x30 + paQuickData[bDataIdx + 1];
            else if(paQuickData[bDataIdx + 1] < 100)
            {
                qty_str[4] = 0x30 + (paQuickData[bDataIdx + 1] / 10);
                qty_str[5] = 0x30 + (paQuickData[bDataIdx + 1] % 10);
            }
            else
            {
                qty_str[4] = 0x30 + (paQuickData[bDataIdx + 1] / 100);
                qty_str[5] = 0x30 + ((paQuickData[bDataIdx + 1] / 10) % 10);
                qty_str[6] = 0x30 + (paQuickData[bDataIdx + 1] % 10);
            }

            while(*qty_str != 0)
            {
                if((*qty_str >= 0x41) && (*qty_str <= 0x5A))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_BigCh_Bitmap_12x12[*qty_str - 0x41], 12, (12 + 7) / 8, 3);
                else if((*qty_str >= 0x61) && (*qty_str <= 0x7A))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_SmallCh_Bitmap_12x12[*qty_str - 0x61], 12, (12 + 7) / 8, 3);
                else if((*qty_str >= 0x30) && (*qty_str <= 0x39))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_Digit_Bitmap_12x12[*qty_str - 0x30], 12, (12 + 7) / 8, 3);
                else if(*qty_str == ' ')
                    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_20H_12x12[0][0], 12, (12 + 7) / 8, 3);
                // else if(*qty_str == '.')
                    // Bitmap_Rotation_Convert((unsigned char *)&aBitmap_2EH_12x12[0][0], 12, (12 + 7) / 8, 3);
                // else if(*qty_str == ':')
                    // Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3AH_12x12[0][0], 12, (12 + 7) / 8, 3);
                else
                {
                    qty_str++;
                    continue;
                }

                Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 9, 12);
                Bitmap_Invert_Color(&aScale_Bitmap[0][0], 3, 9, (12 + 7) / 8);

                // for(i = 0; i < 12; i++)
                for(i = 0; i < 9; i++)
                {
                    // EPD_Pixels_Update(&aRotate_Bitmap[(12 - 1) - i][0], wStart_Line - i, 2, 0, (12 + 7) / 8);
                    EPD_Pixels_Update(&aScale_Bitmap[(9 - 1) - i][0], wStart_Line - i, 12, 0, (12 + 7) / 8);
                }

                qty_str++;
                wStart_Line -= 9;//12;      // T.B.D
            }

            bDataIdx += 2;
            break;

          // String
          case 4:       // Str ascii[0] + ... + str ascii[15]

            if(paQuickData[1] == 1)         // PageIdx = 1
                wStart_Line = 295 - 22;     // T.B.D
            else// if(paQuickData[1] == 2)  // PageIdx = 2
                wStart_Line = 295 - 55;     // T.B.D

            for(i = 0; i < 16; i++)
            {
                if(paQuickData[(bDataIdx + 1) + i] != 0x00)
                    string_str[i] = paQuickData[(bDataIdx + 1) + i];
                else
                    break;
            }

            while(*string_str != 0)
            {
                if((*string_str >= 0x41) && (*string_str <= 0x5A))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_BigCh_Bitmap_12x12[*string_str - 0x41], 12, (12 + 7) / 8, 3);
                else if((*string_str >= 0x61) && (*string_str <= 0x7A))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_SmallCh_Bitmap_12x12[*string_str - 0x61], 12, (12 + 7) / 8, 3);
                else if((*string_str >= 0x30) && (*string_str <= 0x39))
                    Bitmap_Rotation_Convert((unsigned char *)Ascii_Digit_Bitmap_12x12[*string_str - 0x30], 12, (12 + 7) / 8, 3);
                // else if(*string_str == '.')
                    // Bitmap_Rotation_Convert((unsigned char *)&aBitmap_2EH_12x12[0][0], 12, (12 + 7) / 8, 3);
                // else if(*string_str == ':')
                    // Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3AH_12x12[0][0], 12, (12 + 7) / 8, 3);
                else if(*string_str == ' ')
                    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_20H_12x12[0][0], 12, (12 + 7) / 8, 3);
                else
                {
                    string_str++;
                    continue;
                }

                if(paQuickData[1] == 1)     // PageIdx = 1
                {
                    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 9, 12);
                    Bitmap_Invert_Color(&aScale_Bitmap[0][0], 3, 9, (12 + 7) / 8);

                    for(i = 0; i < 9; i++)
                    {
                        // EPD_Pixels_Update(&aRotate_Bitmap[(12 - 1) - i][0], wStart_Line - i, 2, 0, (12 + 7) / 8);
                        EPD_Pixels_Update(&aScale_Bitmap[(9 - 1) - i][0], wStart_Line - i, 12, 0, (12 + 7) / 8);
                    }

                    wStart_Line -= 9;      // T.B.D
                }
                else// if(paQuickData[1] == 2)  // PageIdx = 2
                {
                    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 24);
                    Bitmap_Invert_Color(&aScale_Bitmap[0][0], 3, 12, (24 + 7) / 8);

                    for(i = 0; i < 12; i++)
                    {
                        // EPD_Pixels_Update(&aRotate_Bitmap[(12 - 1) - i][0], wStart_Line - i, 2, 0, (12 + 7) / 8);
                        EPD_Pixels_Update(&aScale_Bitmap[(12 - 1) - i][0], wStart_Line - i, 4, 0, (24 + 7) / 8);
                    }

                    wStart_Line -= 12;      // T.B.D
                }

                string_str++;
            }

            bDataIdx += 17;
            break;

          // Serial Number
          case 5:       // SN ascii[0] + ... + SN ascii[7]

            wStart_Line = 295 - 22;     // T.B.D


            memcpy(serial_msq1, paQuickData + (bDataIdx + 1), 8);

            max_length = strlen(serial_msq1) + 1;
            real_length = code128_encode_raw(serial_msq1, barcode_data, max_length);

            code_bar_num = real_length * 11;

            if(((10 + code_bar_num + 11 + 13 + 10) * BAR_LINE_WIDTH) <= (BAR_START_LINE + 1))
            {
                Barcode_Bar_Update(code_bar_num);
                Barcode_Digit_Update(serial_msq1);
            }


            bDataIdx += 9;
            break;

          default:
            return;
        }


    }

}

//==============================================================================
// EPD Display
//==============================================================================

void EPD_Pixels_Clear(char clear_white)
{
    unsigned int i;

    if(clear_white == 1)
    {
        for(i = 0; i < 296; i++)
            memset(&aPixelBytes[i][0], 0xFF, 16);   // Fill in all white backcolor
    }
    else
    {
        for(i = 0; i < 296; i++)
            memset(&aPixelBytes[i][0], 0x00, 16);   // Fill in all black backcolor
    }
}

// void EPD_Pixels_Update(unsigned char *pPixelBuf, unsigned int wPLineOffset, unsigned int wPByteOffset, unsigned int wByteNum)
void EPD_Pixels_Update(unsigned char *pPixelBuf, unsigned int wPLineOffset, unsigned int wPByteOffset, unsigned int wPPixelOffset, unsigned int wByteNum)
{
    unsigned int i, k;
    /* 
    for(i = 0; i < wByteNum; i++)
    {
        memcpy(&aPixelBytes[wPLineOffset][wPByteOffset + i], pPixelBuf + i, 1);
    }
    */
    
    for(i = 0; i < wByteNum; i++)
    {
        for(k = 0; k < 8; k++)
        {
            if(pPixelBuf[i] & (0x80 >> k))
                aPixelBytes[wPLineOffset][(wPByteOffset + i) + ((wPPixelOffset + k) / 8)] |=  (0x80 >> ((wPPixelOffset + k) % 8));
            else
                aPixelBytes[wPLineOffset][(wPByteOffset + i) + ((wPPixelOffset + k) / 8)] &= ~(0x80 >> ((wPPixelOffset + k) % 8));

//          aPixelBytes[wPLineOffset][wPByteOffset + i] = pPixelBuf[i];
        }
    }
   
}

void EPD_Display_Test(void)
{
    unsigned int i, j;

    size_t max_length = strlen(str) + 1/* strlen(str) * 2 */, real_length;
    real_length = code128_encode_raw(str, barcode_data, max_length);

    // for(i = 0; i < 296; i++)
        // memset(&aPixelBytes[i][0], 0xFF, 16);

    // Check lines limitation
    unsigned int code_bar_num = real_length * 11;
    if(((10 + code_bar_num + 11 + 13 + 10) * BAR_LINE_WIDTH) <= (BAR_START_LINE + 1))
    {
        Barcode_Bar_Update(code_bar_num);
        Barcode_Digit_Update(str);
    }

    #if 0//1//
    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_0_12x12[0][0], 12, (12 / 8) + 1, 3);
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 12);
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 0 + i, 0, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 12);
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 0 + i, 4, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 16);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 0 + i, 8, 0, (16 / 8));
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 24);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 0 + i, 12, 0, (24 / 8));
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_1_12x12[0][0], 12, (12 / 8) + 1, 3);
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 12);
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 24 + i, 0, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 12);
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 24 + i, 4, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 16);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 24 + i, 8, 0, (16 / 8));
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 24);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 24 + i, 12, 0, (24 / 8));
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_2_12x12[0][0], 12, (12 / 8) + 1, 3);
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 12);
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 48 + i, 0, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 12);
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 48 + i, 4, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 16);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 48 + i, 8, 0, (16 / 8));
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 24);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 48 + i, 12, 0, (24 / 8));
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3_12x12[0][0], 12, (12 / 8) + 1, 3);
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 12);
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 72 + i, 0, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 12);
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 72 + i, 4, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 16);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 72 + i, 8, 0, (16 / 8));
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 24);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 72 + i, 12, 0, (24 / 8));
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_A_12x12[0][0], 12, (12 / 8) + 1, 3);
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 12);
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 96 + i, 0, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 12);
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 96 + i, 4, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 16);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 96 + i, 8, 0, (16 / 8));
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 24);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 96 + i, 12, 0, (24 / 8));
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_B_12x12[0][0], 12, (12 / 8) + 1, 3);
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 12);
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 120 + i, 0, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 12);
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 120 + i, 4, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 16);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 120 + i, 8, 0, (16 / 8));
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 24);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 120 + i, 12, 0, (24 / 8));
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_C_12x12[0][0], 12, (12 / 8) + 1, 3);
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 12);
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 144 + i, 0, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 12);
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 144 + i, 4, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 16);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 144 + i, 8, 0, (16 / 8));
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 24);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 144 + i, 12, 0, (24 / 8));
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_D_12x12[0][0], 12, (12 / 8) + 1, 3);
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 12, 12);
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 168 + i, 0, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 16, 12);
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 168 + i, 4, 0, (12 / 8) + 1);
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 16);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 168 + i, 8, 0, (16 / 8));
        }
    }
    Bitmap_Scale_Convert(&aRotate_Bitmap[0][0], 3, 12, 12, 24, 24);
    {
        for(i = 0; i < 24; i++)
        {
            EPD_Pixels_Update(&aScale_Bitmap[i][0], 168 + i, 12, 0, (24 / 8));
        }
    }
    #endif

    #if 0//1//
    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_0_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 0 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_1_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 12 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_2_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 24 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_3_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 36 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_4_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 48 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_5_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 60 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_6_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 72 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_7_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 84 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_8_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 96 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_9_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 108 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_A_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 120 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_B_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 132 + i, j, 0, (12 / 8) + 1);
        }
    }
    #endif
    #if 0
    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_C_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 144 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_D_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 156 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_E_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 168 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_F_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 180 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_G_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 192 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_H_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 204 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_I_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 216 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_J_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 228 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_K_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 240 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_L_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 252 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_M_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 264 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_N_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 276 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_O_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 0 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_P_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 12 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_Q_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 24 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_R_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 36 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_S_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 48 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_T_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 60 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_U_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 72 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_V_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 84 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_W_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 96 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_X_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 108 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_Y_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 120 + i, j, 0, (12 / 8) + 1);
        }
    }

    Bitmap_Rotation_Convert((unsigned char *)&aBitmap_Z_12x12[0][0], 12, (12 / 8) + 1, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 12; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 132 + i, j, 0, (12 / 8) + 1);
        }
    }
    #endif

    #if 0//1//
    Bitmap_Convert_To2Bytes(aBitmap_A_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 0 + i, j, 0, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_B_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 16 + i, j, 2, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_C_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 32 + i, j, 4, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_D_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 48 + i, j, 6, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_E_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 64 + i, j, 0, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_F_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 80 + i, j, 2, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_G_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 96 + i, j, 4, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_H_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 112 + i, j, 6, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_0_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 128 + i, j, 0, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_1_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 144 + i, j, 2, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_2_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 160 + i, j, 4, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_3_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 176 + i, j, 6, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_4_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 192 + i, j, 0, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_5_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 208 + i, j, 2, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_6_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 224 + i, j, 4, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_7_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 240 + i, j, 6, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_8_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 256 + i, j, 0, 24 / 8);
        }
    }

    Bitmap_Convert_To2Bytes(aBitmap_9_24x16, 24);
    Bitmap_Mirror_Convert(&aConvert_Bitmap[0][0], 24, 16 / 8, 1, 0);
    Bitmap_Rotation_Convert(&aMirror_Bitmap[0][0], 24, 16 / 8, 3);
    for(j = 0; j < 15; j+=3)
    {
        for(i = 0; i < 16; i++)
        {
            EPD_Pixels_Update(&aRotate_Bitmap[i][0], 272 + i, j, 2, 24 / 8);
        }
    }
    #endif

    SPI_8b_Send_8b(PON);
    check_busy_high();

    SPI_8b_Send_8b(DTM2);
    #if 1//0
    for(i = 0; i < 296; i++)
    #else
    for(i = 0; i < 16; i++)
    #endif
    {
        for(j = 0; j < 16; j++)
            SPI_8b_Send(1, aPixelBytes[i][j]);
    }

    SPI_8b_Send_8b(DRF);
    check_busy_high();

    SPI_8b_Send_8b(POF);
    check_busy_high();
}

void EPD_Display_Update(void)
{
    unsigned int i, j;

    SPI_8b_Send_8b(PON);
    check_busy_high();

    SPI_8b_Send_8b(DTM2);
    for(i = 0; i < 296; i++)
    {
        for(j = 0; j < 16; j++)
            SPI_8b_Send(1, aPixelBytes[i][j]);
    }

    SPI_8b_Send_8b(DRF);
    check_busy_high();

    SPI_8b_Send_8b(POF);
    check_busy_high();
}

void EPD_Display_White(void)
{
    unsigned int i;

    SPI_8b_Send_8b(PON);
    check_busy_high();

    SPI_8b_Send_8b(DTM2);
    for(i = 0; i < byte_counter; i++)
    {
        SPI_8b_Send(1, 0xFF);
    }

    SPI_8b_Send_8b(DRF);
    check_busy_high();

    SPI_8b_Send_8b(POF);
    check_busy_high();
}

void EPD_Display_Black(void)
{
    unsigned int i;

    SPI_8b_Send_8b(PON);
    check_busy_high();

    SPI_8b_Send_8b(DTM2);
    for(i = 0; i < byte_counter; i++)
    {
        SPI_8b_Send(1, 0x00);
    }

    SPI_8b_Send_8b(DRF);
    check_busy_high();

    SPI_8b_Send_8b(POF);
    check_busy_high();
}

void EPD_Get_Revision(unsigned char *bRev)
{
    SPI_8b_Send_8b(REV);
    // check_busy_high();

    *bRev = SPI_8b_Get();
    // check_busy_high();
}

void EPD_Get_Status(unsigned char *bStat)
{
    SPI_8b_Send_8b(FLG);
    // check_busy_high();

    *bStat = SPI_8b_Get();
    // check_busy_high();
}

//==============================================================================
// 4-wire SPI Transmission Protocol
//==============================================================================
void SPI_8b_Send_8b(unsigned int dat)
{
    if((dat & DATA_MASK) == DATA_MASK)
        SPI_8b_Send(DCX_DATA, (unsigned char)dat);
    else
        SPI_8b_Send(DCX_CMD, (unsigned char)dat);
}

void SPI_8b_Send(unsigned int dcx, unsigned char dat)
{
    unsigned int i;

    if(dcx)
        HAL_GPIO_WritePin(EPD_DC_GPIO_PORT, EPD_DC_PIN, GPIO_PIN_SET);     // Data
    else
        HAL_GPIO_WritePin(EPD_DC_GPIO_PORT, EPD_DC_PIN, GPIO_PIN_RESET);   // Command

    // delay(1);
    // HAL_Delay(1);    // 1 ms
    i = 40;
    while(i--);

    // __HAL_SPI_ENABLE(&SpiHandle);  // let NSS low
    HAL_GPIO_WritePin(EPD_CS_GPIO_PORT, EPD_CS_PIN, GPIO_PIN_RESET);

    // delay(1);
    // HAL_Delay(1);    // 1 ms
    i = 40;
    while(i--);

    wTransferState = TRANSFER_WAIT;
    HAL_SPI_Transmit_IT(&SpiHandle, (uint8_t *)&dat, 1);
    while(wTransferState == TRANSFER_WAIT);

    // delay(1);
    // HAL_Delay(1);    // 1 ms
    i = 40;
    while(i--);

    // __HAL_SPI_DISABLE(&SpiHandle);  // let NSS high
    HAL_GPIO_WritePin(EPD_CS_GPIO_PORT, EPD_CS_PIN, GPIO_PIN_SET);

    // delay(1);
    // HAL_Delay(1);    // 1 ms
    i = 40;
    while(i--);
}

unsigned char SPI_8b_Get(void)
{
    unsigned int i;
    unsigned char DATA_BUF = 0xFF;//0x00;

    HAL_GPIO_WritePin(EPD_DC_GPIO_PORT, EPD_DC_PIN, GPIO_PIN_SET);     // Data

    // delay(1);
    // HAL_Delay(1);    // 1 ms
    i = 40;
    while(i--);

    // __HAL_SPI_ENABLE(&SpiHandle);  // let NSS low
    HAL_GPIO_WritePin(EPD_CS_GPIO_PORT, EPD_CS_PIN, GPIO_PIN_RESET);

    // HAL_Delay(1);    // 1 ms
    i = 40;
    while(i--);

    wTransferState = TRANSFER_WAIT;
    HAL_SPI_Receive_IT(&SpiHandle, (uint8_t *)&DATA_BUF, 1);
    while(wTransferState == TRANSFER_WAIT);

    // delay(1);
    // HAL_Delay(1);    // 1 ms
    i = 40;
    while(i--);

    // __HAL_SPI_DISABLE(&SpiHandle);  // let NSS high
    HAL_GPIO_WritePin(EPD_CS_GPIO_PORT, EPD_CS_PIN, GPIO_PIN_SET);

    // delay(1);
    // HAL_Delay(1);    // 1 ms
    i = 40;
    while(i--);

    return DATA_BUF;
}

//==============================================================================
// Character Bitmap Transfer
//==============================================================================

unsigned char Bitmap_ByteInvert(unsigned char bOrg_Byte)
{
    unsigned int i;
    unsigned char bNew_Byte = 0x00;

    for(i = 0; i < 8; i++)
    {
        if(bOrg_Byte & (0x01 << i))
            bNew_Byte |= (0x80 >> i);
    }

    return bNew_Byte;
}

void Bitmap_Convert_To2Bytes(unsigned int *pOrg_Bitmap, unsigned int wLineNum)//(unsigned int aOrg_Bitmap[24])
{
    unsigned int i;

    for(i = 0; i < wLineNum; i++)
    {
        // aConvert_Bitmap[i][0] = (unsigned char)((aOrg_Bitmap[i] >> 8) & 0xFF);
        // aConvert_Bitmap[i][1] = (unsigned char)(aOrg_Bitmap[i] & 0xFF);
        aConvert_Bitmap[i][0] = (unsigned char)((pOrg_Bitmap[i] >> 8) & 0xFF);
        aConvert_Bitmap[i][1] = (unsigned char)(pOrg_Bitmap[i] & 0xFF);
    }
}

void Bitmap_Invert_Color(unsigned char *pOrg_Bitmap, unsigned int Org_MaxBytes, unsigned int wLineNum, unsigned int wByteNum)
{
    unsigned int i;

    if(wByteNum == 1)
    {
        for(i = 0; i < wLineNum; i++)
        {
            pOrg_Bitmap[i * Org_MaxBytes] ^= 0xFF;
        }
    }
    else if(wByteNum == 2)
    {
        for(i = 0; i < wLineNum; i++)
        {
            // aOrg_Bitmap[i][0] ^= 0xFF;
            // aOrg_Bitmap[i][1] ^= 0xFF;
            pOrg_Bitmap[(i * Org_MaxBytes) + 0] ^= 0xFF;
            pOrg_Bitmap[(i * Org_MaxBytes) + 1] ^= 0xFF;
        }
    }
    else if(wByteNum == 3)
    {
        for(i = 0; i < wLineNum; i++)
        {
            pOrg_Bitmap[(i * Org_MaxBytes) + 0] ^= 0xFF;
            pOrg_Bitmap[(i * Org_MaxBytes) + 1] ^= 0xFF;
            pOrg_Bitmap[(i * Org_MaxBytes) + 2] ^= 0xFF;
        }
    }

}

void Bitmap_Mirror_Convert(unsigned char *pOrg_Bitmap, unsigned int H_Lines, unsigned int W_Bytes, unsigned char bInc_H, unsigned char bInc_W)
{
    unsigned int i, j;
    unsigned char bNew_Byte;

    // Clear aMirror_Bitmap
    for(i = 0; i < 24; i++)
    {
        memset(&aMirror_Bitmap[i][0], 0x00, 24 / 8);
    }

    if(bInc_H == 1)
    {
        for(i = 0; i < H_Lines; i++)
        {
            if(bInc_W == 1)
            {
                for(j = 0; j < W_Bytes; j++)
                    aMirror_Bitmap[i][j] = pOrg_Bitmap[(i * W_Bytes) + j];//aOrg_Bitmap[i][j];
            }
            else
            {
                for(j = 0; j < W_Bytes; j++)
                {
                    bNew_Byte = Bitmap_ByteInvert(pOrg_Bitmap[(i * W_Bytes) + j]);//(aOrg_Bitmap[i][j]);
                    aMirror_Bitmap[i][(W_Bytes - 1) - j] = bNew_Byte;
                }

                if((H_Lines == 12) && (W_Bytes == 2))       // for 12 x 12 Bitmap
                {
                    bNew_Byte = aMirror_Bitmap[i][1] >> 4;
                    aMirror_Bitmap[i][0] = aMirror_Bitmap[i][0] << 4;
                    aMirror_Bitmap[i][0] |= bNew_Byte;
                    aMirror_Bitmap[i][1] = aMirror_Bitmap[i][1] << 4;
                }
            }
        }
    }
    else
    {
        for(i = 0; i < H_Lines; i++)
        {
            if(bInc_W == 1)
            {
                for(j = 0; j < W_Bytes; j++)
                    aMirror_Bitmap[(H_Lines - 1) - i][j] = pOrg_Bitmap[(i * W_Bytes) + j];//aOrg_Bitmap[i][j];
            }
            else
            {
                for(j = 0; j < W_Bytes; j++)
                {
                    bNew_Byte = Bitmap_ByteInvert(pOrg_Bitmap[(i * W_Bytes) + j]);//(aOrg_Bitmap[i][j]);
                    aMirror_Bitmap[(H_Lines - 1) - i][(W_Bytes - 1) - j] = bNew_Byte;
                }

                if((H_Lines == 12) && (W_Bytes == 2))       // for 12 x 12 Bitmap
                {
                    bNew_Byte = aMirror_Bitmap[(H_Lines - 1) - i][1] >> 4;
                    aMirror_Bitmap[(H_Lines - 1) - i][0] = aMirror_Bitmap[(H_Lines - 1) - i][0] << 4;
                    aMirror_Bitmap[(H_Lines - 1) - i][0] |= bNew_Byte;
                    aMirror_Bitmap[(H_Lines - 1) - i][1] = aMirror_Bitmap[(H_Lines - 1) - i][1] << 4;
                }
            }
        }
    }
}

void Bitmap_Rotation_Convert(unsigned char *pOrg_Bitmap, unsigned int H_Lines, unsigned int W_Bytes, unsigned char bRot_Type)
{
    unsigned int i, j, k;
    unsigned char bNew_Byte;

    // Clear aRotate_Bitmap
    for(i = 0; i < 24; i++)
    {
        memset(&aRotate_Bitmap[i][0], 0x00, 24 / 8);
    }

    if(bRot_Type == 1)
    {
        // For clock-wise 90 degrees
        for(j = 0; j < W_Bytes; j++)
        {
            for(k = 0; k < 8; k++)
            {
                for(i = 0; i < H_Lines; i++)
                {
                    // if((aOrg_Bitmap[(H_Lines - 1) - i][j]) & (0x80 >> k))
                    if((pOrg_Bitmap[(((H_Lines - 1) - i) * W_Bytes) + j]) & (0x80 >> k))
                        (aRotate_Bitmap[(j * 8) + k][i / 8]) |= (0x80 >> (i % 8));
                }
            }
        }
    }
    else if(bRot_Type == 2)
    {
        // For clock-wise 180 degrees
        for(i = 0; i < H_Lines; i++)
        {
            for(j = 0; j < W_Bytes; j++)
            {
                for(k = 0; k < 8; k++)
                {
                    // if((aOrg_Bitmap[(H_Lines - 1) - i][(W_Bytes - 1) - j]) & (0x01 << k))
                    if((pOrg_Bitmap[(((H_Lines - 1) - i) * W_Bytes) + ((W_Bytes - 1) - j)]) & (0x01 << k))
                        (aRotate_Bitmap[i][j]) |= (0x80 >> k);
                }
            }

            if((H_Lines == 12) && (W_Bytes == 2))       // for 12 x 12 Bitmap
            {
                bNew_Byte = aRotate_Bitmap[i][W_Bytes - 1] >> 4;
                aRotate_Bitmap[i][0] = aRotate_Bitmap[i][0] << 4;
                aRotate_Bitmap[i][0] |= bNew_Byte;
                aRotate_Bitmap[i][W_Bytes - 1] = aRotate_Bitmap[i][W_Bytes - 1] << 4;
            }
        }
    }
    else if(bRot_Type == 3)
    {
        // For clock-wise 270 degrees
        for(j = 0; j < W_Bytes; j++)
        {
            for(k = 0; k < 8; k++)
            {
                for(i = 0; i < H_Lines; i++)
                {
                    // if((aOrg_Bitmap[i][(W_Bytes - 1) - j]) & (0x01 << k))
                    if((pOrg_Bitmap[(i * W_Bytes) + ((W_Bytes - 1) - j)]) & (0x01 << k))
                        (aRotate_Bitmap[(j * 8) + k][i / 8]) |= (0x80 >> (i % 8));
                }

                if((H_Lines == 12) && (W_Bytes == 2))       // for 12 x 12 Bitmap
                {
                    if(((j * 8) + k) > 3)
                        memcpy(&aRotate_Bitmap[((j * 8) + k) - 4][0], &aRotate_Bitmap[(j * 8) + k][0], (H_Lines / 8) + 1);
                }
            }
        }
    }
}

void Bitmap_Scale_Convert(unsigned char *pOrg_Bitmap, unsigned int Org_MaxBytes, unsigned int Old_Lines, unsigned int Old_Pixels, unsigned int New_Lines, unsigned int New_Pixels)
{
    unsigned int i, k;
    unsigned int wSrc_Line, wSrc_Pixel;

    // if((Old_Pixels % 8) == 0)
        // wSrc_Byte = Old_Pixels / 8;
    // else
        // wSrc_Byte = (Old_Pixels / 8) + 1;

    // Clear aScale_Bitmap
    for(i = 0; i < New_Lines; i++)
    {
        if((New_Pixels % 8) == 0)
            memset(&aScale_Bitmap[i][0], 0x00, (New_Pixels / 8));
        else
            memset(&aScale_Bitmap[i][0], 0x00, (New_Pixels / 8) + 1);
    }

    for(i = 0; i < New_Lines; i++)
    {
        wSrc_Line = (i * Old_Lines) / New_Lines;

        for(k = 0; k < New_Pixels; k++)
        {
            wSrc_Pixel = (k * Old_Pixels) / New_Pixels;

            if(pOrg_Bitmap[(wSrc_Line * Org_MaxBytes) + (wSrc_Pixel / 8)] & (0x80 >> (wSrc_Pixel % 8)))
                aScale_Bitmap[i][k / 8] |= (0x80 >> (k % 8));
        }
    }
}


