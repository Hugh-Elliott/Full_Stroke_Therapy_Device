#ifndef _DEF_INC_PMDRPtypes
#define _DEF_INC_PMDRPtypes

/*
  PMDRPtypes.h -- PMD resource protocol definitions

  Performance Motion Devices, Inc.
*/


#define LINK_PACKET_LENGTH          256U
#define TOTAL_PACKET_LENGTH         252U
#define MAX_PARAM_DWORDS            4     // number of 32-bit params that can be added to a packet
#define PACKET_HEADER_LENGTH        2
#define PACKET_HEADER_LENGTH_RX     1
#define PACKET_ERROR_LENGTH         3
#define PACKET_PADDING_BYTES        3
#define MAX_PRP_CHANNELS            10
#define MAX_NUM_MAILBOXES           10

// max # of words in a packet minus header
#define MAX_PACKET_DATA_LENGTH      (TOTAL_PACKET_LENGTH - 4U)
#define MAX_PACKET_DWORDS           (MAX_PACKET_DATA_LENGTH >> 2U)
// max # of words in a packet minus header and params
#define MAX_DATA_DWORDS             (MAX_PACKET_DWORDS - MAX_PARAM_DWORDS)  // (252 - 4) / 4) - 4 = 58
#define USER_PACKET_LENGTH          MAX_PACKET_DATA_LENGTH

#define DEFAULT_ETHERNET_PORT       40100

// RP_TIMEOUT is the PRP packet default timeout in ms.  It is set higher than the timeout of an attached device such as the local Magellan (MP_TIMEOUT) in PMDser.c
// Otherwise, the attached device may respond after the RP packet has timed out leaving bytes in the local receive queue.
// The default timeout is added to any commands that have a timeout parameter such as PeriphReceive.
#define RP_TIMEOUT                  800    

#define SUB_ACTION_POS              2       // position of sub action byte in packet header
#define SUB_COMMAND_POS             3       // position of sub command byte in packet header
#define ERROR_CODE_POS              1       // position of error code in response packet
#define PROTOCOL_VERSION            1

#define DEFAULT_ID( bytecount, id )  (((bytecount-1) << 8) | id)

typedef enum PMD_DefaultsEnum {
    PMD_Default_IPAddress               = DEFAULT_ID( 4,  3 ), // 0x0303     
    PMD_Default_NetMask                 = DEFAULT_ID( 4,  4 ), // 0x0304     
    PMD_Default_Gateway                 = DEFAULT_ID( 4,  5 ), // 0x0305  
    PMD_Default_IPPort                  = DEFAULT_ID( 2,  6 ), // 0x0106  
    PMD_Default_MACH                    = DEFAULT_ID( 4,  7 ), // 0x0307  
    PMD_Default_MACL                    = DEFAULT_ID( 4,  8 ), // 0x0308  
    PMD_Default_TCPKeepAliveInterval    = DEFAULT_ID( 2, 10 ), // 0x010A  TCP connection keepalive interval in seconds (default is 30)
    PMD_Default_TCPKeepAliveIdleTime    = DEFAULT_ID( 2, 11 ), // 0x010B  TCP connection keepalive idle time in seconds (default is 60)
    PMD_Default_TCPKeepAliveCount       = DEFAULT_ID( 2, 12 ), // 0x010C  TCP connection keepalive counter for KEEPALIVE probes (default is 2)
    PMD_Default_COM1                    = DEFAULT_ID( 2, 14 ), // 0x010E  Serial port 1 (host PRP interface port) settings when in RS232 mode. (same encoding as the Magellan SetSerialPortMode command)
    PMD_Default_Serial1Mode             = DEFAULT_ID( 2, 14 ), // 0x010E  Same as PMD_Default_COM1
    PMD_Default_COM2                    = DEFAULT_ID( 2, 15 ), // 0x010F  Serial port 2 (debug console or expansion port) settings when Serial port 1 is in RS232 mode.  
    PMD_Default_Serial2Mode             = DEFAULT_ID( 2, 15 ), // 0x010F  Same as PMD_Default_COM2
    PMD_Default_RS485Mode               = DEFAULT_ID( 2, 16 ), // 0x0110  Serial port 1 RS485 mode (see PMD_RS485Mode definitions below) Factory default is 0000
    PMD_Default_RS485Duplex             = DEFAULT_ID( 2, 16 ), // 0x0110  Same as PMD_Default_RS485Mode
    PMD_Default_CANMode                 = DEFAULT_ID( 2, 17 ), // 0x0111  CAN port bitrate and node ID (same encoding as the Magellan SetCANMode command)
    PMD_Default_CAN                     = DEFAULT_ID( 2, 17 ), // 0x0111  
    PMD_Default_HostCANMode             = DEFAULT_ID( 2, 18 ), // 0x0112  Host CAN port bitrate and node ID (same encoding as the Magellan SetCANMode command)
    PMD_Default_HostCAN                 = DEFAULT_ID( 2, 18 ), // 0x0112  
    PMD_Default_AutoStartMode           = DEFAULT_ID( 2, 20 ), // 0x0114  Auto start the user task(s) after a reset.
    PMD_Default_TaskStackSize           = DEFAULT_ID( 2, 22 ), // 0x0116  Default stack size in dwords
    PMD_Default_DebugSource             = DEFAULT_ID( 2, 23 ), // 0x0117  
    PMD_Default_ConsoleIntfType         = DEFAULT_ID( 2, 24 ), // 0x0118  Console interface type. One of PMDInterface.
    PMD_Default_ConsoleIntfAddr         = DEFAULT_ID( 4, 25 ), // 0x0319  Console interface address depending on type of PMDInterface.
    PMD_Default_ConsoleIntfPort         = DEFAULT_ID( 2, 26 ), // 0x011A  Console interface port depending on type of PMDInterface.
    PMD_Default_LEDFlashMode            = DEFAULT_ID( 2, 28 ), // 0x011C  
    PMD_Default_TaskParam               = DEFAULT_ID( 4, 30 ), // 0x031E  Parameter passed to the main user task on auto-start
    PMD_Default_RecoverTime             = DEFAULT_ID( 2, 31 ), // 0x011F  
    PMD_Default_DHCPTries               = DEFAULT_ID( 2, 32 ), // 0x0120  
    PMD_Default_COM3                    = DEFAULT_ID( 2, 33 ), // 0x0121  TTL serial port on all 'N' series ION/CME products
    PMD_Default_Serial3Mode             = DEFAULT_ID( 2, 33 ), // 0x0121  Same as PMD_Default_COM3
    PMD_Default_COM1RS485               = DEFAULT_ID( 2, 34 ), // 0x0122  Serial port 1 settings when in RS485 mode. 
    PMD_Default_Serial1ModeRS485        = DEFAULT_ID( 2, 34 ), // 0x0121  Same as PMD_Default_COM1RS485
    PMD_Default_DIOmux                  = DEFAULT_ID( 2, 35 ), // 0x0123  Digital IO signal selection setting (PIO register DIO_SIG_SEL 0x228)
    PMD_Default_DIOdir                  = DEFAULT_ID( 2, 36 ), // 0x0124  Digital IO signal direction setting (PIO register DIO_DIR_WRITE 0x222)
    PMD_Default_DIOout                  = DEFAULT_ID( 2, 37 ), // 0x0125  Digital IO signal state setting (PIO register DIO_OUT_WRITE 0x212)
    PMD_Default_HostSPI                 = DEFAULT_ID( 2, 38 ), // 0x0126  (CLKPolarity << 9) | (CLKPhase << 8) | DataSize
    PMD_Default_ConsoleBuffer           = DEFAULT_ID( 2, 40 ), // 0x0128  Console buffer size (default is 1000 bytes)
    PMD_Default_BiSSConfig              = DEFAULT_ID( 2, 41 ), // 0x0129  AXIS1_BISS_CONFIG    register (PIO register 0x100)
    PMD_Default_BiSSResolution          = DEFAULT_ID( 2, 42 ), // 0x012A  AXIS1_SSI_RESOLUTION register (PIO register 0x102)
    PMD_Default_BiSSFrequency           = DEFAULT_ID( 2, 43 ), // 0x012B  AXIS1_BISS_FREQUENCY register (PIO register 0x104)
    PMD_Default_BiSSEnable              = DEFAULT_ID( 2, 44 ), // 0x012C  AXIS1_BISS_ENABLE    register (PIO register 0x106)
    PMD_Default_BiSSSingleTurn          = DEFAULT_ID( 2, 45 ), // 0x012D  AXIS1_BISS_STCTRL    register (PIO register 0x108)
    PMD_Default_BiSSMultiTurn           = DEFAULT_ID( 2, 46 ), // 0x012E  AXIS1_BISS_MTCTRL    register (PIO register 0x10A)
    PMD_Default_BiSSRightBitShift       = DEFAULT_ID( 2, 47 ), // 0x012F  AXIS1_BISS_XBIT_CTRL register (PIO register 0x10C)
    PMD_Default_ConsoleTimeout          = DEFAULT_ID( 2, 48 ), // 0x0130  PMDPrintf console output timeout (default is 100 ms)
    PMD_Default_WatchdogStartMode       = DEFAULT_ID( 2, 50 ), // 0x0132  Auto start the user task after a watchdog reset. User code must GetFaultCode to handle this reset condition.
    PMD_Default_ExpSPIStatusDIO         = DEFAULT_ID( 2, 51 ), // 0x0133  ExpSPI HostSPIStatus input DIO selection (default is DIO7) 
    PMD_Default_Factory                 = 0xFFFF               // 0xFFFF  Reset to factory defaults.
} PMDDefaults;

#define PMD_RS485Mode_HalfDuplex       (0x0001) // Half duplex (2-wire) enable.
#define PMD_RS485Mode_Termination      (0x0010) // RS485/422 120 ohm termination enable.
#define PMD_RS485Mode_RS485_232        (0x0100) // Set RS485 or RS232 mode. When enabled, the serial port is placed in RS485 mode, otherwise RS232. This value is only applicable when PMD_RS485Mode_IgnorePin is enabled.
#define PMD_RS485Mode_IgnorePin        (0x1000) // DIO8 pin override. When enabled, the serial port is placed in the mode set by the PMD_RS485Mode_RS485_232 bit on power up regardless of the state of DIO8 pin. Otherwise, if DIO8 is detected as low on power up RS485 mode is enabled.


typedef enum {
    PMDTaskInfo_State                   = 0, 
    PMDTaskInfo_AbortCode               = 1, 
    PMDTaskInfo_StackRemaining          = 2, 
    PMDTaskInfo_StackSize               = 3, 
    PMDTaskInfo_Priority                = 4, 
    PMDTaskInfo_Name                    = 5, // the first 4 characters of the task name. 
    PMDTaskInfo_Name2                   = 6, // the next 4 characters of the task name. 
    PMDTaskInfo_Name3                   = 7, // the next 4 characters of the task name. 
} PMDTaskInfo;

typedef enum {
    PMDTaskState_Invalid               = 0,
    PMDTaskState_NoCode                = 1,
    PMDTaskState_NotStarted            = 2,
    PMDTaskState_Running               = 3,
    PMDTaskState_Aborted               = 4,
    PMDTaskState_Error                 = 5,
} PMDTaskState;

typedef enum {
    PMDTaskPriority_Low                 = -2, 
    PMDTaskPriority_Normal              = 0,  
    PMDTaskPriority_High                = 3,  
} PMDTaskPriority;

typedef enum {
    PMDDataSize_8Bit                    = 1,
    PMDDataSize_16Bit                   = 2,
    PMDDataSize_32Bit                   = 4,
} PMDDataSize;

typedef enum {
    PMDMemoryAddress_DPRAM              = 0,
    PMDMemoryAddress_NVRAM              = 1,
    PMDMemoryAddress_RAM                = 2,
    PMDMemoryAddress_NVRAM1             = 0x11, // First 128K sector 
    PMDMemoryAddress_NVRAM2             = 0x21, // Second 128K sector 
} PMDMemoryAddress;

// PMDMemoryType has been changed to PMDMemoryAddress
// PMDMemoryType kept for compatibility
#define PMDMemoryType         PMDMemoryAddress
#define PMDMemoryType_DPRAM   PMDMemoryAddress_DPRAM
#define PMDMemoryType_NVRAM   PMDMemoryAddress_NVRAM
#define PMDMemoryType_RAM     PMDMemoryAddress_RAM  

typedef enum PMDDeviceTypeEnum {
    PMDDeviceTypeNone                   = 0,
    PMDDeviceTypeResourceProtocol       = 1,
    PMDDeviceTypeMotionProcessor        = 2,
} PMDDeviceType;

typedef enum {
    PMDSerialPort1                      = 0,
    PMDSerialPort2                      = 1,
    PMDSerialPort3                      = 2,
} PMDSerialPort;

typedef enum {
    PMDCANPort1                         = 0, //Expansion CAN port on nION products, Host CAN port on all other products
    PMDCANPort2                         = 1, //Host CAN port on nION products
} PMDCANPort;

typedef enum {
    PMDCANMode_StdId                    = 0,
    PMDCANMode_StdMask                  = 1,
    PMDCANMode_StdRange                 = 2,
} PMDCANMode;

typedef enum {
    PMDSPIPort_Exp                      = 0,
    PMDSPIPort_Host                     = 1,
} PMDSPIPort;

//PMDSPIMode is defined in PMDtypes.h

typedef enum {
    PMDSPIChipSelect_None               = 0,
    PMDSPIChipSelect_1                  = 1,
    PMDSPIChipSelect_2                  = 2,
    PMDSPIChipSelect_3                  = 3,
    PMDSPIChipSelect_4                  = 4,
} PMDSPIChipSelect;

typedef enum {
    PMDSPIBitRate_20MHz                 = 1,
    PMDSPIBitRate_10MHz                 = 2,
    PMDSPIBitRate_5MHz                  = 3,
    PMDSPIBitRate_2500kHz               = 4,
    PMDSPIBitRate_1250kHz               = 5,
    PMDSPIBitRate_625kHz                = 6,
    PMDSPIBitRate_312_5kHz              = 7,
} PMDSPIBitRate;

typedef enum {
    PMDInitFault_BootCRC                = 0x00000001,	// Boot firmware CRC error
    PMDInitFault_MainCRC                = 0x00000002,	// Main firmware CRC error
    PMDInitFault_Magellan               = 0x00000020,	// Magellan initialization error 
    PMDInitFault_PIO                    = 0x00000100,	// Internal PIO fault
    PMDInitFault_InvalidModel           = 0x00000200,	// Cannot determine model 
    PMDInitFault_Hardware               = 0x00000400,	// Hardware initialization fault
    PMDInitFault_Memory                 = 0x00000800,	// Memory allocation failure during initialization
} PMDInitFault;

typedef enum
{
    PMDException_None                   = 0,
    PMDException_NMI                    = 1,
    PMDException_HardFault              = 3,
    PMDException_MemManage              = 4,
    PMDException_BusFault               = 5,
    PMDException_UsageFault             = 6,
    PMDException_StackOverflow          = 0x10,
    PMDException_UserTaskReturned       = 0x11,
    PMDException_ApplicationExit        = 0x12, // mainly heap allocation failure.
    PMDException_MemoryAllocationFailure= 0x20,
} PMDException;

typedef enum {
    PMDFaultCode_ResetCause             = 0, // returns the same value as the function PMDGetResetCause
    PMDFaultCode_Initialization         = 1, // returns a fault code that occured during device initialization 
    PMDFaultCode_Exception              = 2, // processor or OS exception that caused a reset
} PMDFaultCode;

typedef enum {
    PMDDeviceInfo_CMEVersion            = 0, // byte3=mode, byte2=major, byte1=customcode, byte0=minor
    PMDDeviceInfo_LogicVersion          = 1, // 16-bit version
    PMDDeviceInfo_HostInterface         = 2, // one or more of PMDHostInterface
    PMDDeviceInfo_MemorySize            = 3, // with optional parameter set to one of PMDMemoryAddress
    PMDDeviceInfo_Heap                  = 5, // with optional parameter set to one of PMDHeap
    PMDDeviceInfo_IPaddress             = 6, // IPv4 address of the device
} PMDDeviceInfo;

typedef enum {
    PMDHeap_AvailableBytes              = 0, // the total amount of heap space remaining in bytes.  There is no guarantee that all of this can be allocated, depending on what sizes are asked for.
    PMDHeap_MinimumAvailableBytes       = 1, // how close we have come to running out of heap space so far.
} PMDHeap;

typedef enum {
    PMDHostInterface_Serial             = 1,
    PMDHostInterface_CAN                = 2,
    PMDHostInterface_SPI                = 4,
    PMDHostInterface_Ethernet           = 8,
}   PMDHostInterface;

typedef enum
{
    PMDResetCause_SoftReset             = 1 << 9,
    PMDResetCause_Exception             = 1 << 10,
    PMDResetCause_SysWatchdog           = 1 << 11,
    PMDResetCause_HardReset             = 1 << 12,
    PMDResetCause_UnderVoltage          = 1 << 13,
    PMDResetCause_External              = 1 << 14, // non-nION products
    PMDResetCause_Watchdog              = 1 << 15, // non-nION products
} PMDResetCause;

typedef enum {
    PMDEventNumber_1                    = 0, 
    PMDEventNumber_2                    = 1, 
    PMDEventNumber_3                    = 2, 
    PMDEventNumber_4                    = 3, 
} PMDEventNumber;

typedef enum {
    PMDEventType_None                   = 0, 
    PMDEventType_Motion                 = 1, 
    PMDEventType_DigitalInput           = 2, 
    PMDEventType_Timer                  = 3,
    PMDEventType_Sync                   = 4, 
} PMDEventType;

typedef enum {
    PMDEventSignal_DI1                  = 0, 
    PMDEventSignal_DI2                  = 1, 
    PMDEventSignal_DI3                  = 2, 
    PMDEventSignal_DI4                  = 3, 
    PMDEventSignal_DI5                  = 4, 
    PMDEventSignal_DI6                  = 5, 
    PMDEventSignal_DI7                  = 6, 
    PMDEventSignal_DI8                  = 7, 
    PMDEventSignal_HallA                = 8, 
    PMDEventSignal_HallB                = 9, 
    PMDEventSignal_HallC                = 10, 
    PMDEventSignal_PosLimit             = 11, 
    PMDEventSignal_NegLimit             = 12, 
    PMDEventSignal_Home                 = 13, 
    PMDEventSignal_Enable               = 14, 
    PMDEventSignal_Brake                = 15, 
    PMDEventSignal_QuadA1               = 16, 
    PMDEventSignal_QuadB1               = 17, 
    PMDEventSignal_Index1               = 18, 
    PMDEventSignal_QuadA2               = 20, 
    PMDEventSignal_QuadB2               = 21, 
    PMDEventSignal_Index2               = 22, 
    PMDEventSignal_FaultOut             = 23, 
} PMDEventSignal;

typedef enum {
    PMDEventTrigger_Disable             = 0, 
    PMDEventTrigger_PosEdge             = 1,
    PMDEventTrigger_NegEdge             = 2,
    PMDEventTrigger_BothEdges           = 3,
} PMDEventTrigger;

typedef enum {
    PMDEventMode_Once                   = 0, 
    PMDEventMode_Continuous             = 1, 
} PMDEventMode;

typedef enum {
    PMDLEDBlinkRate_Off                 = 0,
    PMDLEDBlinkRate_On                  = 1,
    PMDLEDBlinkRate_1Hz50Percent        = 2,
    PMDLEDBlinkRate_3Hz50Percent        = 3,
    PMDLEDBlinkRate_1Hz25Percent        = 4,
    PMDLEDBlinkRate_3Hz33Percent        = 5,
    PMDLEDBlinkRate_1Hz8Percent         = 6,
    PMDLEDBlinkRate_6Hz50Percent        = 7,
} PMDLEDBlinkRate;

// For ION/CME Indexer interface
typedef enum {
    PMDIndexerIO_AI                     = 0x40,
    PMDIndexerIO_DI                     = 0x50,
    PMDIndexerIO_DO                     = 0x60,
    PMDIndexerIO_Direction              = 0x70,
} PMDIndexerIO;

// For Machine Controller Analog/Digital interface
typedef enum {
    PMDMachineIO_DI                     = 0x200,
    PMDMachineIO_DOMask                 = 0x210,
    PMDMachineIO_DO                     = 0x212,
    PMDMachineIO_DORead                 = 0x214,
    PMDMachineIO_DODirMask              = 0x220,
    PMDMachineIO_DODir                  = 0x222,
    PMDMachineIO_DODirRead              = 0x224,
    PMDMachineIO_AmpEnaMask             = 0x230,
    PMDMachineIO_AmpEna                 = 0x232,
    PMDMachineIO_AmpEnaRead             = 0x234,
    PMDMachineIO_IntPending             = 0x240,
    PMDMachineIO_IntMask                = 0x250,
    PMDMachineIO_IntPosEdge             = 0x260,
    PMDMachineIO_IntNegEdge             = 0x270,
    PMDMachineIO_AOCh1                  = 0x300,
    PMDMachineIO_AOCh2                  = 0x302,
    PMDMachineIO_AOCh3                  = 0x304,
    PMDMachineIO_AOCh4                  = 0x306,
    PMDMachineIO_AOCh5                  = 0x308,
    PMDMachineIO_AOCh6                  = 0x30A,
    PMDMachineIO_AOCh7                  = 0x30C,
    PMDMachineIO_AOCh8                  = 0x30E,
    PMDMachineIO_AOCh1Ena               = 0x310,
    PMDMachineIO_AOCh2Ena               = 0x312,
    PMDMachineIO_AOCh3Ena               = 0x314,
    PMDMachineIO_AOCh4Ena               = 0x316,
    PMDMachineIO_AOCh5Ena               = 0x318,
    PMDMachineIO_AOCh6Ena               = 0x31A,
    PMDMachineIO_AOCh7Ena               = 0x31C,
    PMDMachineIO_AOCh8Ena               = 0x31E,
    PMDMachineIO_AOEna                  = 0x320,
    PMDMachineIO_AICh1                  = 0x340,
    PMDMachineIO_AICh2                  = 0x342,
    PMDMachineIO_AICh3                  = 0x344,
    PMDMachineIO_AICh4                  = 0x346,
    PMDMachineIO_AICh5                  = 0x348,
    PMDMachineIO_AICh6                  = 0x34A,
    PMDMachineIO_AICh7                  = 0x34C,
    PMDMachineIO_AICh8                  = 0x34E,
} PMDMachineIO;

// PIO register addresses for N-Series ION/CME
typedef enum {
    PMDPIO_AXIS1_BISS_CONFIG            = 0x100,
    PMDPIO_AXIS1_BISS_RESOLUTION        = 0x102,
    PMDPIO_AXIS1_BISS_FREQUENCY         = 0x104,
    PMDPIO_AXIS1_BISS_ENABLE            = 0x106,
    PMDPIO_AXIS1_BISS_STCTRL            = 0x108,
    PMDPIO_AXIS1_BISS_MTCTRL            = 0x10A,
    PMDPIO_AXIS1_BISS_XBIT_CTRL         = 0x10C,
    PMDPIO_DIO_IN_READ                  = 0x200,
    PMDPIO_DIO_IN_QUAL                  = 0x204,
    PMDPIO_DIO_OUT_MASK                 = 0x210,
    PMDPIO_DIO_OUT_WRITE                = 0x212,
    PMDPIO_DIO_OUT_READ                 = 0x214,
    PMDPIO_DIO_DIR_MASK                 = 0x220,
    PMDPIO_DIO_DIR_WRITE                = 0x222,
    PMDPIO_DIO_DIR_READ                 = 0x224,
    PMDPIO_DIO_SIG_SEL                  = 0x228,
    PMDPIO_DIO_INT_PEND                 = 0x240,
    PMDPIO_DIO_INT1_SEL                 = 0x250,
    PMDPIO_DIO_INT2_SEL                 = 0x252,
    PMDPIO_DIO_INT3_SEL                 = 0x254,
    PMDPIO_DIO_INT4_SEL                 = 0x256,
    PMDPIO_DIO_INT1_EDGE                = 0x260,
    PMDPIO_DIO_INT2_EDGE                = 0x262,
    PMDPIO_DIO_INT3_EDGE                = 0x264,
    PMDPIO_DIO_INT4_EDGE                = 0x266,
    PMDPIO_DI_IN_READ                   = 0x280, // Magellan input signals
    PMDPIO_DI_IN_QUAL                   = 0x284, // HallA,B,C, Pos Limit, Neg Limit, Home, Enable, Brake
    PMDPIO_DI_IN_ENC_QUAL               = 0x286, 
    PMDPIO_AICh1                        = 0x340, // nION 16-bit general purpose AI
    PMDPIO_AICh2                        = 0x342,
    PMDPIO_AICh3                        = 0x344,
    PMDPIO_AICh4                        = 0x346,
    PMDPIO_AICh5                        = 0x348,
    PMDPIO_AICh6                        = 0x34A,
    PMDPIO_AICh7                        = 0x34C,
    PMDPIO_LEDS                         = 0x410,
    PMDPIO_LED_GREEN                    = 0x412,
    PMDPIO_LED_RED                      = 0x414,
    PMDPIO_AXIS1_POS_LOW                = 0x820, // Must be read first to latch high word
    PMDPIO_AXIS1_POS_HIGH               = 0x822,
    PMDPIO_AXIS1_POS_OFFSET_LOW         = 0x824, // Must be read first to latch high word
    PMDPIO_AXIS1_POS_OFFSET_HIGH        = 0x826,
    PMDPIO_AXIS1_POS_COMPARE_MODE       = 0x800,
    PMDPIO_AXIS1_POS_COMPARE_START_LOW  = 0x804,
    PMDPIO_AXIS1_POS_COMPARE_START_HIGH = 0x806,
    PMDPIO_AXIS1_POS_COMPARE_END_LOW    = 0x808,
    PMDPIO_AXIS1_POS_COMPARE_END_HIGH   = 0x80A,
    PMDPIO_AXIS1_POS_DISTANCE_LOW       = 0x80C,
    PMDPIO_AXIS1_POS_DISTANCE_HIGH      = 0x80E,
    PMDPIO_AXIS1_POS_FRAC_NUMERATOR     = 0x810,
    PMDPIO_AXIS1_POS_FRAC_DENOMINATOR   = 0x812,
    PMDPIO_AXIS1_POS_COMPARE_NEXT_LOW   = 0x814,
    PMDPIO_AXIS1_POS_COMPARE_NEXT_HIGH  = 0x816,
    PMDPIO_AXIS1_POS_COMPARE_PULSE_WIDTH= 0x818,
    PMDPIO_AXIS2_POS_LOW                = 0x920, // Must be read first to latch high word
    PMDPIO_AXIS2_POS_HIGH               = 0x922,
    PMDPIO_AXIS2_POS_OFFSET_LOW         = 0x924, // Must be read first to latch high word
    PMDPIO_AXIS2_POS_OFFSET_HIGH        = 0x926,
    PMDPIO_AXIS2_POS_COMPARE_MODE       = 0x900,
    PMDPIO_AXIS2_POS_COMPARE_START_LOW  = 0x904,
    PMDPIO_AXIS2_POS_COMPARE_START_HIGH = 0x906,
    PMDPIO_AXIS2_POS_COMPARE_END_LOW    = 0x908,
    PMDPIO_AXIS2_POS_COMPARE_END_HIGH   = 0x90A,
    PMDPIO_AXIS2_POS_DISTANCE_LOW       = 0x90C,
    PMDPIO_AXIS2_POS_DISTANCE_HIGH      = 0x90E,
    PMDPIO_AXIS2_POS_FRAC_NUMERATOR     = 0x910,
    PMDPIO_AXIS2_POS_FRAC_DENOMINATOR   = 0x912,
    PMDPIO_AXIS2_POS_COMPARE_NEXT_LOW   = 0x914,
    PMDPIO_AXIS2_POS_COMPARE_NEXT_HIGH  = 0x916,
    PMDPIO_AXIS2_POS_COMPARE_PULSE_WIDTH= 0x918,
} PMDPIO;

typedef enum {
    PMDPIO_DIO1                         = 1 << 0,
    PMDPIO_DIO2                         = 1 << 1,
    PMDPIO_DIO3                         = 1 << 2,
    PMDPIO_DIO4                         = 1 << 3,
    PMDPIO_DIO5                         = 1 << 4,
    PMDPIO_DIO6                         = 1 << 5,
    PMDPIO_DIO7                         = 1 << 6,
    PMDPIO_DIO8                         = 1 << 7,
} PMDPIO_DIO; // bit masks for PMDPIO_DIO_IN/OUT/DIR registers

// DIO interrupt selection for PIO register PMDPIO_DIO_INTx_SEL
typedef enum {
    PMDDIO_INT1_SEL_None                = 0,
    PMDDIO_INT1_SEL_DIO1                = 1,
    PMDDIO_INT1_SEL_DIO2                = 2,
    PMDDIO_INT1_SEL_DIO3                = 3,
    PMDDIO_INT1_SEL_DIO4                = 4,
    PMDDIO_INT1_SEL_DIO5                = 5,
    PMDDIO_INT1_SEL_DIO6                = 6,
    PMDDIO_INT1_SEL_DIO7                = 7,
    PMDDIO_INT1_SEL_DIO8                = 8,
    PMDDIO_INT1_SEL_EncoderA1           = 16,
    PMDDIO_INT1_SEL_EncoderB1           = 17,
    PMDDIO_INT1_SEL_EncoderIndex        = 18,
    PMDDIO_INT1_SEL_EncoderHome         = 19,
    PMDDIO_INT1_SEL_PositiveLimit       = 20,
    PMDDIO_INT1_SEL_NegativeLimit       = 21,
    PMDDIO_INT1_SEL_AxisIn              = 22,
    PMDDIO_INT1_SEL_HallA               = 23,
    PMDDIO_INT1_SEL_HallB               = 24,
    PMDDIO_INT1_SEL_HallC               = 25,
    PMDDIO_INT1_SEL_EncoderA2           = 26,
    PMDDIO_INT1_SEL_EncoderB2           = 27,
    PMDDIO_INT1_SEL_EncoderIndex2       = 28,
    PMDDIO_INT1_SEL_EnableIn            = 29,
    PMDDIO_INT1_SEL_FaultOut            = 30,
    PMDDIO_INT1_SEL_Brake               = 31,
} PMDDIO_INT1_SEL;

typedef enum {
    PMDBiSSFrequency_10Mhz              = 3,
    PMDBiSSFrequency_5Mhz               = 7,
    PMDBiSSFrequency_4Mhz               = 9,
    PMDBiSSFrequency_2_5Mhz             = 15,
    PMDBiSSFrequency_2Mhz               = 17,
    PMDBiSSFrequency_1Mhz               = 19,
    PMDBiSSFrequency_500kHz             = 23,
    PMDBiSSFrequency_250khz             = 31,
} PMDBiSSFrequency;

typedef enum {
    PMDIONTypeB                         = 0,
    PMDIONTypeCME                       = 3
} PMDIONType;

//  Extract CAN port params from single word.
//  The data bitrate is in 3rd nibble in CANMode but in 2nd nibble passed to PMDDeviceOpenPeriphCANFD.
//  CANMode is the bitmapped word format using by Set/GetDefault and Magellan's Set/GetCANMode.
//  data bps is one of PMDCANFDDataBaud
//  std  bps is one of PMDCANBaud
//  | 15 14 13 | 12 | 11 10  9 | 8 | 7 | 6  5  4 |  3  2  1  0  |
//  |               |              |             |              |
//  | std bps  | 0  |   data bps   | 0 |        nodeID          |
#define GET_CANMODE(CANmode, baud, address) {\
    baud            = (CANmode & 0xE000) >> 13; \
    baud           |= (CANmode & 0x0F00) >> 4;  \
    address         = (CANmode & 0x007F);}

// compact CAN port params into single word
#define SET_CANMODE(CANmode, baud, address) \
    CANmode = address | ((baud & 0x0007) << 13)  | ((baud & 0x00F0) << 4);

// extract serial port params from single word
//  | 15 14 13 12 11 | 10  9 | 8      7 |     6     | 5    4 | 3  2  1  0 |
//  |                |       |          |           |
//  |    address     |   0   | protocol | stop bits | parity |    baud    |
#define GET_SERIALPORTMODE(serialmode, baud, parity, stopbits, protocol, address) {\
    baud            = (serialmode & 0x000F); \
    parity          = (PMDSerialParity)((serialmode & 0x0030) >> 4); \
    stopbits        = (PMDSerialStopBits)((serialmode & 0x0040) >> 6); \
    protocol        = (serialmode & 0x0180) >> 7; \
    address         = (serialmode & 0xF800) >> 11;}

// compact serial port params into single word
#define SET_SERIALPORTMODE(serialmode, baud, parity, stopbits, protocol, address) \
    serialmode = (PMDuint16)(baud | (parity << 4) | (stopbits << 6) | (protocol << 7) | (address << 11));


#endif
