/*************************************************************************************
 *
 *
 *************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "EL029TC2_SPI.h"
#include "X8K_Dura_I2C.h"
#include "EEPROM_I2C.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I2C_ADDRESS     (0x6E << 1)

/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated in case of the I2C Clock source is the SYSCLK = 32 MHz */
#define I2C_TIMING      0x00B1112E /* 400 kHz with analog Filter ON, Rise Time 250ns, Fall Time 100ns */ 


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* I2C handler declaration */
I2C_HandleTypeDef I2cHandle;

/* SPI handler declaration */
SPI_HandleTypeDef SpiHandle;

/* transfer state */
__IO uint32_t wTransferState = TRANSFER_WAIT;

/*  */
uint8_t bUpdateMode = 0;
uint8_t gBootInit = 1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void SystemClock_Config_Wake(void);
static void SystemPower_Config_Wake(void);

static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    HAL_Init();

    SystemClock_Config();

#ifdef USE_EPD_FUNC

    /* Configure GPIO pins */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* EPD GPIOs Initialization */
    // EPD_BS
    GPIO_InitStruct.Pin = EPD_BS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_BS_GPIO_PORT, &GPIO_InitStruct);

    // EPD_RSTN
    GPIO_InitStruct.Pin = EPD_RSTN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLUP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_RSTN_GPIO_PORT, &GPIO_InitStruct);

    // EPD_DC
    GPIO_InitStruct.Pin = EPD_DC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_DC_GPIO_PORT, &GPIO_InitStruct);

    // EPD_BUSYN
    GPIO_InitStruct.Pin = EPD_BUSYN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLDOWN;//GPIO_PULLUP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_BUSYN_GPIO_PORT, &GPIO_InitStruct);

    // EPD_CS       // Cannot be controlled by SPI ?
    GPIO_InitStruct.Pin = EPD_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLUP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_CS_GPIO_PORT, &GPIO_InitStruct);

    EPD_Pixels_Clear(1);    // Initialize aPixelBytes with white backcolor

#endif

#if 0//#ifdef USE_I2C_BUS

    /*##-1- Configure the I2C peripheral ######################################*/
    I2cHandle.Instance             = I2Cx;
    I2cHandle.Init.Timing          = I2C_TIMING;
    I2cHandle.Init.OwnAddress1     = 0;//I2C_ADDRESS;
    I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;//I2C_ADDRESSINGMODE_10BIT;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.OwnAddress2     = 0;//0xFF;
    I2cHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;  

    if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    /* Enable the Analog I2C Filter */
    HAL_I2CEx_ConfigAnalogFilter(&I2cHandle,I2C_ANALOGFILTER_ENABLE);

    #ifdef USE_UHF_FUNC

    /*
    // TAG_Init();
    */

    TAG_Reset_CmdStatus();

    #endif

#endif

    /* Wait for programming before sleep (Need to test the minimum time) */
    HAL_Delay(3000);

  while (1)
  {

    if(gBootInit == 1)
        goto BOOT_WAKE;

GO_SLEEP:

#ifdef USE_STOP_WAKE

    if(HAL_I2C_DeInit(&I2cHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    /* Configure the system clock to 2 MHz */
    SystemClock_Config_Wake();

    /* Configure the system Power */
    SystemPower_Config_Wake();

    /* Enable GPIOA clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    I2Cx_SCL_GPIO_CLK_ENABLE();
    I2Cx_SDA_GPIO_CLK_ENABLE();

    // DCI pin
    GPIO_InitStruct.Pin = UHF_DCI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(UHF_DCI_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(UHF_DCI_GPIO_PORT, UHF_DCI_PIN, GPIO_PIN_RESET);     // Set DCI to 0V
    #if 1   // ESL board
    // SW_POWER pin
    GPIO_InitStruct.Pin = SW_POWER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(SW_POWER_GPIO_PORT, &GPIO_InitStruct);
    // HAL_GPIO_WritePin(SW_POWER_GPIO_PORT, SW_POWER_PIN, GPIO_PIN_RESET);     // Set VDD to 0V
    HAL_GPIO_WritePin(SW_POWER_GPIO_PORT, SW_POWER_PIN, GPIO_PIN_SET);     // Set VDD to 3.3V for SCL wakeup
    #endif

    // SDA pin
    GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;//GPIO_MODE_IT_FALLING;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

    // SCL pin
    GPIO_InitStruct.Pin = I2Cx_SCL_PIN;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* Enable and set PA.12 (Arduino D2) EXTI Interrupt to the lowest priority */
    NVIC_SetPriority((IRQn_Type)(EXTI4_15_IRQn), 0x03);
    HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI4_15_IRQn));

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Enter Stop Mode */
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

#endif

BOOT_WAKE:

    HAL_Delay(1000);    // to wait for tag writing finished (Need to test appropriate timeout)
    HAL_GPIO_WritePin(UHF_DCI_GPIO_PORT, UHF_DCI_PIN, GPIO_PIN_SET);     // Set DCI to 3.3V
    #if 1   // ESL board
    HAL_GPIO_WritePin(SW_POWER_GPIO_PORT, SW_POWER_PIN, GPIO_PIN_SET);     // Set VDD to 3.3V
    #endif

    /* Configure the system clock to 32 MHz */
    SystemClock_Config();


#ifdef USE_I2C_BUS

    /*##-1- Configure the I2C peripheral ######################################*/
    I2cHandle.Instance             = I2Cx;
    I2cHandle.Init.Timing          = I2C_TIMING;
    I2cHandle.Init.OwnAddress1     = 0;//I2C_ADDRESS;
    I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;//I2C_ADDRESSINGMODE_10BIT;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.OwnAddress2     = 0;//0xFF;
    I2cHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;  

    if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    /* Enable the Analog I2C Filter */
    HAL_I2CEx_ConfigAnalogFilter(&I2cHandle,I2C_ANALOGFILTER_ENABLE);


    #ifdef USE_EEPROM_FUNC

    EEPROM_Read_Byte(EEPROM_BITMAP_1, &aPixelBytes[0][0], 296 * 16);

    // goto EPD_UPDATE;    // Only need to do once at boot up

    #endif


    #ifdef USE_UHF_FUNC

    /* UHF Initialization */
    TAG_Init();     // Could be done only once when all parameters are stored in tag memory

    if(gBootInit == 1)
    {
        TAG_Reset_CmdStatus();
        gBootInit = 0;

        goto GO_SLEEP;
    }

    #ifndef SIMPLE_DEMO_MODE

    if(TAG_Get_BridgeData() == 1)  // Bridge command data
    {
        bUpdateMode = 1;

        TAG_Set_BusyStatus(1);

        // Update partial buffer for EPD
        EPD_Pixels_Update(BridgeData.Bit_Data.DoData, ((BridgeData.Bit_Data.DoLineH) << 8) | (BridgeData.Bit_Data.DoLineL), BridgeData.Bit_Data.DoByte, 0, 8);

        if(BridgeData.Bit_Data.XcsCtl == 1)
        {
            BridgeData.Bit_Data.XcsCtl = 0;
            TAG_Write_User(0x0400, (uint16_t *)BridgeData.Word_Data, 5);   // Addr: 1024d
            goto EPD_UPDATE;
        }
        else
            TAG_Set_BusyStatus(0);
    }
    else if(TAG_Get_MemoryData() == 1)  // Memory command data
    {
        bUpdateMode = 2;

        TAG_Set_BusyStatus(1);

        // Update full buffer for EPD
        uint8_t ByteBuf[16];
        uint16_t i;

        for(i = 0; i < (MemoryData.Bit_Data.LineNum); i++)
        {
            TAG_Read_User(64 + (16 * i), ByteBuf, 16);      // Addr: 0064d//0080d

            EPD_Pixels_Update(ByteBuf, (MemoryData.Bit_Data.DoLine) + i, 0, 0, 16);
        }

        if(MemoryData.Bit_Data.XcsCtl == 1)
        {
            MemoryData.Bit_Data.XcsCtl = 0;
            TAG_Write_User(0x03F0, (uint16_t *)MemoryData.Word_Data, 2);   // Addr: 1008d//0064d
            goto EPD_UPDATE;
        }
        else
            TAG_Set_BusyStatus(0);
    }
    else if(TAG_Get_QuickData() == 1)  // Quick mode data
    {
        bUpdateMode = 3;

        // TAG_Set_BusyStatus(1);

        goto EPD_UPDATE;
    }

    // #else

    else if(TAG_Get_CheckData() == 1)    // Check mode data
    {
        // Check_Mode_Update(CheckData.Word_Data);

        bUpdateMode = 4;

        goto EPD_UPDATE;
    }

    else if(TAG_Get_ControlMethod() == 1)  // Control method
    {
        goto ESL_CONTROL;
    }

    #endif

    goto GO_SLEEP;

    #endif

#endif

EPD_UPDATE:

#ifdef USE_SPI_BUS

    /*##-1- Configure the SPI peripheral #######################################*/
    /* Set the SPI parameters */
    SpiHandle.Instance               = SPIx;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;//SPI_BAUDRATEPRESCALER_2;//SPI_BAUDRATEPRESCALER_16;//
    SpiHandle.Init.Direction         = SPI_DIRECTION_1LINE;//SPI_DIRECTION_2LINES;//  // for 1-line in Half-Duplex ?
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;//SPI_PHASE_2EDGE;//
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;//SPI_POLARITY_HIGH;//
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;//SPI_FIRSTBIT_LSB;//
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;//SPI_NSS_HARD_OUTPUT;//  // not used by software ?

    SpiHandle.Init.Mode = SPI_MODE_MASTER;

    if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    #ifdef USE_EPD_FUNC


    //
    /* Configure GPIO pins */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* EPD GPIOs Initialization */
    // EPD_BS
    GPIO_InitStruct.Pin = EPD_BS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_BS_GPIO_PORT, &GPIO_InitStruct);

    // EPD_RSTN
    GPIO_InitStruct.Pin = EPD_RSTN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLUP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_RSTN_GPIO_PORT, &GPIO_InitStruct);

    // EPD_DC
    GPIO_InitStruct.Pin = EPD_DC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_DC_GPIO_PORT, &GPIO_InitStruct);

    // EPD_BUSYN
    GPIO_InitStruct.Pin = EPD_BUSYN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLDOWN;//GPIO_PULLUP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_BUSYN_GPIO_PORT, &GPIO_InitStruct);

    // EPD_CS
    GPIO_InitStruct.Pin = EPD_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;//GPIO_PULLUP;//
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(EPD_CS_GPIO_PORT, &GPIO_InitStruct);


    /* EPD Initialization */
    if((bUpdateMode == 3) || (bUpdateMode == 4))  // Quick mode or Check mode
        EPD_Init(1);     // Could be done only once when VDD hasn't power off
    else
        EPD_Init(0);     // Could be done only once when VDD hasn't power off

    #ifndef SIMPLE_DEMO_MODE

    if(bUpdateMode == 3)  // Quick mode data
        Quick_Mode_Update(QuickData.Word_Data);
    else if(bUpdateMode == 4)  // Check mode data
        Check_Mode_Update(CheckData.Word_Data);
    else if(bUpdateMode == 1)
    {
        // EPD_Pixels_Update(BridgeData.Bit_Data.DoData, ((BridgeData.Bit_Data.DoLineH) << 8) | (BridgeData.Bit_Data.DoLineL), BridgeData.Bit_Data.DoByte, 0, 8);

        // TAG_Set_BusyStatus(0);
    }
    else if(bUpdateMode == 2)
    {
        // uint8_t ByteBuf[16];
        // uint16_t i;

        // for(i = 0; i < (MemoryData.Bit_Data.LineNum); i++)
        // {
            // TAG_Read_User(80 + (16 * i), ByteBuf, 16);      // Addr: 0080d

            // EPD_Pixels_Update(ByteBuf, (MemoryData.Bit_Data.DoLine) + i, 0, 0, 16);
        // }

        // TAG_Set_BusyStatus(0);
    }

    bUpdateMode = 0;
    // #else

    // Check_Mode_Update(CheckData.Word_Data);

    #endif

    EPD_Display_Update();

    TAG_Set_BusyStatus(0);

    // HAL_Delay(3000);

    #ifdef USE_EEPROM_FUNC

    EEPROM_Write_Page(EEPROM_BITMAP_1, &aPixelBytes[0][0], 296 * 16);

    #endif

    // TAG_Set_BusyStatus(0);

    goto GO_SLEEP;

    #endif

#endif

ESL_CONTROL:

    // Control pins
    GPIO_InitStruct.Pin = GPIO_PIN_3;           // T.B.D
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;       // T.B.D
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);     // T.B.D

    if(ControlMethod.Bit_Data.SetEn != 0x00)
    {
        // Start GPIO control
        if(ControlMethod.Bit_Data.SetEn & 0x01)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, (ControlMethod.Bit_Data.ComDat & 0x01) == 0x00 ? GPIO_PIN_RESET:GPIO_PIN_SET); // T.B.D

        HAL_Delay(ControlMethod.Bit_Data.TestUse * 1000);

        // Stop GPIO control
        if(ControlMethod.Bit_Data.SetEn & 0x01)
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3); // T.B.D
    }

    goto GO_SLEEP;

  }

  /* Infinite loop */  
  while (1)
  {
  }

}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 32000000
  *            HCLK(Hz)                       = 32000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            Flash Latency(WS)              = 1
  *            Main regulator output voltage  = Scale1 mode
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_ClkInitTypeDef RCC_ClkInitStruct ={0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLSource   = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLMUL      = RCC_PLL_MUL4;
  RCC_OscInitStruct.PLL.PLLDIV      = RCC_PLL_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

}

#ifdef USE_STOP_WAKE
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = MSI
  *            SYSCLK(Hz)                     = 2000000
  *            HCLK(Hz)                       = 2000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            Flash Latency(WS)              = 0
  *            Main regulator output voltage  = Scale3 mode
  * @retval None
  */
void SystemClock_Config_Wake(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  
  /* Enable MSI Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.MSICalibrationValue=0x00;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
  
  /* Select MSI as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  
}

/**
  * @brief  System Power Configuration
  *         The system Power is configured as follow : 
  *            + Regulator in LP mode
  *            + VREFINT OFF, with fast wakeup enabled
  *            + MSI as SysClk after Wake Up
  *            + No IWDG
  *            + Wakeup using EXTI Line (PA.12 (Arduino D2))
  * @param  None
  * @retval None
  */
static void SystemPower_Config_Wake(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* Enable Ultra low power mode */
  HAL_PWREx_EnableUltraLowPower();
  
  /* Enable the fast wake up from Ultra low power mode */
  HAL_PWREx_EnableFastWakeUp();

  /* Select MSI as system clock source after Wake Up from Stop mode */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG (RCC_STOP_WAKEUPCLOCK_MSI);
  
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
  /* Note: Debug using ST-Link is not possible during the execution of this   */
  /*       example because communication between ST-link and the device       */
  /*       under test is done through UART. All GPIO pins are disabled (set   */
  /*       to analog input mode) including  UART I/O pins.           */
  GPIO_InitStructure.Pin = GPIO_PIN_All;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;

  // HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
  // HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

  // Reserved for specific pins +++
  // GPIO_InitStructure.Pin = GPIO_PIN_All & ~(EPD_RSTN_PIN | EPD_DC_PIN | EPD_CS_PIN | UHF_DCI_PIN
                                          // | SPIx_SCK_PIN | SPIx_MISO_PIN | SPIx_MOSI_PIN);
  GPIO_InitStructure.Pin = GPIO_PIN_All & ~(SPIx_SCK_PIN | SPIx_MISO_PIN | SPIx_MOSI_PIN);
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 

  GPIO_InitStructure.Pin = GPIO_PIN_All & ~(SW_POWER_PIN);
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  // GPIO_InitStructure.Pin = GPIO_PIN_All & ~(EPD_BS_PIN | EPD_BUSYN_PIN);
  // HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  // Reserved for specific pins ---

  /* Disable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
}

/**
  * @brief GPIO EXTI callback
  * @param None
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Clear Wake Up Flag */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}

/**
  * @brief SYSTICK callback
  * @param None
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
  HAL_IncTick();
}
#endif

#ifdef USE_I2C_BUS
/**
  * @brief  Tx Transfer completed callback.
  * @param  I2cHandle: I2C handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
}

/**
  * @brief  I2C error callbacks.
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
  /** Error_Handler() function is called when error occurs.
    * 1- When Slave don't acknowledge it's address, Master restarts communication.
    * 2- When Master don't acknowledge the last data transferred, Slave don't care in this example.
    */
  if (HAL_I2C_GetError(I2cHandle) != HAL_I2C_ERROR_AF)
  {
    Error_Handler();
  }
}
#endif

#ifdef USE_SPI_BUS
/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report end of Interrupt TxRx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
  void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
  {
    wTransferState = TRANSFER_COMPLETE;
  }

  void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
  {
    wTransferState = TRANSFER_COMPLETE;
  }

/**
  * @brief  SPI error callbacks.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  wTransferState = TRANSFER_ERROR;
}
#endif

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
    while(1)
    {
    }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/**
  * @}
  */

/**
  * @}
  */

