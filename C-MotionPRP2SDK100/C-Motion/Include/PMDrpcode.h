#pragma once
//
//  PMDrpcode.h - PMD Resource Protocol enumerated values
//
//  Performance Motion Devices, Inc.
//

enum PMD_PRPStatusEnum {
    PMD_PRPStatus_OK                    = 0,
    PMD_PRPStatus_Error                 = 1,
    PMD_PRPStatus_Outgoing              = 2,
};

enum PMD_ResourceEnum {
    PMD_Resource_Device                 = 0,
    PMD_Resource_CMotionEngine          = 1,
    PMD_Resource_MotionProcessor        = 2,
    PMD_Resource_Memory                 = 3,
    PMD_Resource_Peripheral             = 4,
};

enum PMD_ActionCodesEnum {
    PMD_Action_NOP                      = 0,
    PMD_Action_Reset                    = 1,
    PMD_Action_Command                  = 2,
    PMD_Action_Open                     = 3,
    PMD_Action_Close                    = 4,
    PMD_Action_Send                     = 5,
    PMD_Action_Receive                  = 6,
    PMD_Action_Write                    = 7,
    PMD_Action_Read                     = 8,
    PMD_Action_Set                      = 9,
    PMD_Action_Get                      = 10,
    PMD_Action_Clear                    = 11,
};

// used with PMD_Action_Open
enum PMD_OpenCodesEnum {
    PMD_Open_Device                     = 0,
    PMD_Open_CMotionEngine              = 1,
    PMD_Open_MotionProcessor            = 2,
    PMD_Open_Memory                     = 3,
    PMD_Open_Mailbox                    = 4,
    PMD_Open_PeriphNone                 = 16,
    PMD_Open_PeriphPIO                  = 18,
    PMD_Open_PeriphISA                  = 19,
    PMD_Open_PeriphCOM                  = 20,
    PMD_Open_PeriphSerial               = 20,
    PMD_Open_PeriphCAN                  = 21,
    PMD_Open_PeriphTCP                  = 22,
    PMD_Open_PeriphUDP                  = 23,
    PMD_Open_PeriphCANFD                = 24,
    PMD_Open_PeriphMultiDrop            = 25,
    PMD_Open_PeriphSPI                  = 26,
};

// used with PMD_Action_Get and PMD_Action_Set
enum PMD_ValueCodesEnum {
    PMD_Value_Version                   = 1,
    PMD_Value_Default                   = 2,
    PMD_Value_ResetCause                = 3,
    PMD_Value_Console                   = 4,
    PMD_Value_TaskState                 = 5,
    PMD_Value_TaskInfo                  = 5,
    PMD_Value_FileName                  = 6,
    PMD_Value_FileDate                  = 7,
    PMD_Value_FileChecksum              = 8,
    PMD_Value_FileVersion               = 9,
    PMD_Value_PortSettings              = 12,
    PMD_Value_Time                      = 13,
    PMD_Value_FaultCode                 = 14,
    PMD_Value_NodeID                    = 15,
    PMD_Value_IPaddress                 = 16,
};

enum PMD_CommandCodesEnum {
    PMD_Command                         = 0,
    PMD_Command_TaskControl             = 1,
    PMD_Command_Flash                   = 2,
};

enum PMD_CommandFlashCodesEnum {
    PMD_Flash_Start                     = 1,
    PMD_Flash_Data                      = 2,
    PMD_Flash_End                       = 3,
    PMD_Flash_Erase                     = 4,
};

enum PMD_CommandTaskControlEnum {
    PMD_TaskControl_StartAll            = 1,
    PMD_TaskControl_StopAll             = 2,
};

