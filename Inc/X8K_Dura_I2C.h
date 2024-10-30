#ifndef __X8KDURA_H__
#define __X8KDURA_H__



/* Exported types ------------------------------------------------------------*/

typedef union
{
    struct
    {
        uint8_t Lock_USER   :2;
        uint8_t Lock_EPC    :2;
        uint8_t Lock_Access :2;
        uint8_t Lock_Kill   :2;

        uint8_t I2C_Addr    :2;
        uint8_t Kill        :1;
        uint8_t RFU         :4;
        uint8_t Lock_DA     :1;

    } Bit_Data;
    uint8_t Word_Data[2];

} ControlWord_WordAddr08d;

typedef union
{
    struct
    {
        uint8_t Block_PermaLock_7   :1;
        uint8_t Block_PermaLock_6   :1;
        uint8_t Block_PermaLock_5   :1;
        uint8_t Block_PermaLock_4   :1;
        uint8_t Block_PermaLock_3   :1;
        uint8_t Block_PermaLock_2   :1;
        uint8_t Block_PermaLock_1   :1;
        uint8_t Block_PermaLock_0   :1;

        uint8_t Block_PermaLock_15  :1;
        uint8_t Block_PermaLock_14  :1;
        uint8_t Block_PermaLock_13  :1;
        uint8_t Block_PermaLock_12  :1;
        uint8_t Block_PermaLock_11  :1;
        uint8_t Block_PermaLock_10  :1;
        uint8_t Block_PermaLock_9   :1;
        uint8_t Block_PermaLock_8   :1;

    } Bit_Data;
    uint8_t Word_Data[2];

} ControlWord_WordAddr18d;

typedef union
{
    struct
    {
        uint8_t RFU0;

        uint8_t RF1_Dis     :1;
        uint8_t RF2_Dis     :1;
        uint8_t DCI_RF_En   :1;
        uint8_t QT_Mem      :1;
        uint8_t QT_SR       :1;
        uint8_t BPL_En      :1;
        uint8_t WWU         :1;
        uint8_t RFU8        :1;

    } Bit_Data;
    uint8_t Word_Data[2];

} ControlWord_WordAddr20d;

typedef union
{
    struct
    {
        uint8_t NSI8        :1;
        uint8_t XI          :1;
        uint8_t UMI         :1;
        uint8_t EPC_Length  :5;

        uint8_t NSI0;

    } Bit_Data;
    uint8_t Word_Data[2];

} ControlWord_WordAddr22d;

typedef union
{
    struct
    {
        uint8_t XcsCtl      :1; // temp for EPD update
        uint8_t CdData      :1; // temp for busy status
        uint8_t SeqNum      :1; // temp for new data
//      uint8_t DoLength    :5;
//      uint8_t DoLength    :4; // 0 ~ 5 for 6 bytes of DoData (let DoData to be 16 bytes for 128 pixels ?)
        uint8_t DoByte      :4; // 0 ~ 15 for byte offset in line (16 bytes)
        uint8_t DoLineH     :1;

//      uint8_t RFU         :3;
//      uint8_t DiPtr       :5;
        uint8_t DoLineL;        // (DoLineH << 8) | DoLineL = Line 0 ~ 295

//      uint8_t DoData[6];      // DoData[0] ~ DoData[n-1] + Padding[n]
        uint8_t DoData[8];//[4];      // update 4 bytes in one time

    } Bit_Data;
    uint8_t Word_Data[10];//[6];

} BridgeData_Transfer;

typedef union
{
    struct
    {
        uint16_t XcsCtl     :1; // temp for EPD update
        uint16_t CdData     :1; // temp for busy status
        uint16_t SeqNum     :1; // temp for new data
        uint16_t DiMem      :1;
//      uint16_t DoLength   :12;// 0 ~ 991 for 992 bytes (62 lines x 16 bytes)
        uint16_t LineNum    :6; // 1 ~ 59 for maximum 59 lines
        uint16_t RFU1       :6;

//      uint16_t DoWordAdr;
        uint16_t DoLine     :9; // Line 0 ~ 295
        uint16_t RFU2       :7;

    } Bit_Data;
    uint8_t Word_Data[4];

} MemoryData_Transfer;

typedef union
{
    struct
    {
        uint8_t ComDat      :3; // used for GPIO output values
        uint8_t IntDat      :1; // used for TAG_Set_BusyStatus()
//      uint8_t SetEn       :4; // used for GPIO pins enable
        uint8_t SetEn       :3; // used for GPIO pins enable
        uint8_t SeqNum      :1; // used for new data

        uint8_t TestUse     :4; // used for active period (seconds)
        uint8_t FuncEn      :4; // used for function enable

    } Bit_Data;
    uint8_t Word_Data[2];

} Control_Method;

typedef union
{
    struct
    {
        uint8_t CmdHead;
        uint8_t PageIdx;        // Page index for different layout

        uint8_t CmdData[28];    // Date/Time TypeIdx[i] = 1: Data[0] ~ Data[4]
                                // Next WS. TypeIdx[i] = 2: Data[0] ~ Data[3]
                                // Quantity TypeIdx[i] = 3: Data[0]
                                // String TypeIdx[i] = 4: Data[0] ~ Data[15]
                                // SN. TypeIdx[i] = 5: Data[0] ~ Data[7]

        uint8_t ChkFlag;        // 0: Old Data, others: New Data(used by Checksum ?)
        uint8_t CmdTail;

    } Byte_Data;
    uint8_t Word_Data[32];

} QuickData_Transfer;

typedef union
{
    struct
    {
        uint8_t CmdHead;
        uint8_t NewFlag;
        uint8_t ErrStatus;

        uint8_t DockStr[2];

        uint8_t DateHex[3];
        uint8_t TimeHex[2];

        uint8_t ResultA;
        uint8_t ResultB;
        uint8_t ResultC;

        uint8_t RsvPud[2];
        uint8_t CmdTail;

    } Byte_Data;
    uint8_t Word_Data[16];

} CheckData_Transfer;


/* Exported variables --------------------------------------------------------*/
extern ControlWord_WordAddr08d ControlWord_08d;
extern ControlWord_WordAddr18d ControlWord_18d;
extern ControlWord_WordAddr20d ControlWord_20d;
extern ControlWord_WordAddr22d ControlWord_22d;

extern BridgeData_Transfer BridgeData;
extern MemoryData_Transfer MemoryData;
extern QuickData_Transfer QuickData;
extern CheckData_Transfer CheckData;
extern Control_Method ControlMethod;


/* Exported functions --------------------------------------------------------*/
void TAG_Init(void);

uint8_t TAG_Get_BridgeData(void);
uint8_t TAG_Get_MemoryData(void);
uint8_t TAG_Get_ControlMethod(void);
uint8_t TAG_Get_QuickData(void);
uint8_t TAG_Get_CheckData(void);
void TAG_Set_BusyStatus(uint8_t bIsBusy);
void TAG_Reset_CmdStatus(void);
void TAG_Read_User(uint16_t ByteAddr, uint8_t *pbData, uint16_t ByteNum);
void TAG_Write_User(uint16_t ByteAddr, uint16_t *pwData, uint16_t WordNum);
void I2C_Write_Word(uint8_t DeviceAddr, uint16_t *pwData, uint16_t WordNum);
void I2C_Read_Byte(uint8_t DeviceAddr, uint8_t *pbData, uint16_t ByteNum);



#endif

