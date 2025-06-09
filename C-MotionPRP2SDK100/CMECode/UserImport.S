#include "locations.h"

.globl  PMDAxisOpen
.globl  PMDPeriphOpenDeviceMP 
.globl  PMDPeriphOpenDevicePRP 
.globl  PMDWaitForEvent
.globl  PMDDeviceOpenMemory
.globl  PMDDeviceOpenPeriphCANFD
.globl  PMDDeviceOpenPeriphCAN
.globl  PMDDeviceOpenPeriphCME
.globl  PMDDeviceOpenPeriphSerial
.globl  PMDDeviceOpenPeriphPIO
.globl  PMDDeviceOpenPeriphPRP
.globl  PMDDeviceOpenPeriphSPI
.globl  PMDDeviceOpenPeriphTCP
.globl  PMDDeviceOpenPeriphUDP
.globl  PMDPeriphOpenPeriphMultiDrop
.globl  PMDprintf
.globl  PMDsprintf
.globl  PMDputs
.globl  PMDputch
.globl  PMDwrite
.globl  PMDread
.globl  PMDTaskCreate
.globl  PMDTaskWait
.globl  PMDTaskWaitUntil
.globl  PMDTaskAbort
.globl  PMDTaskGetNumber
.globl  PMDTaskGetAbortCode
.globl  PMDTaskEnterCritical
.globl  PMDTaskExitCritical
.globl  PMDTaskSetPriority
.globl  PMDCMETaskStart
.globl  PMDCMETaskGetInfo
.globl  PMDCMETaskGetState
.globl  PMDCMESetConsole
.globl  PMDDeviceGetTickCount
.globl  PMDDeviceGetVersion
.globl  PMDDeviceGetInfo
.globl  PMDDeviceGetDefault
.globl  PMDDeviceSetDefault
.globl  PMDDeviceReset
.globl  PMDDeviceGetResetCause
.globl  PMDDeviceGetFaultCode
.globl  PMDDeviceGetSystemTime
.globl  PMDDeviceSetSystemTime
.globl  PMDDeviceGetMicroseconds
.globl  PMDDeviceSetMicroseconds
.globl  PMDDeviceSetNodeID
.globl  PMDDeviceNoOperation
.globl  PMDCMEStoreUserCode
.globl	PMDCMEGetUserCodeVersion
.globl	PMDCMEGetUserCodeDate
.globl	PMDCMEGetUserCodeName
.globl	PMDCMEGetUserCodeChecksum
.globl  PMDTimerInit
.globl  PMDTimerWait
.globl  ReadPIObus
.globl  WritePIObus
.globl  ReadPIObus32
.globl  WritePIObus32
.globl  PMDInterruptWait
.globl  PMDEventOpen
.globl  PMDEventWait
.globl  PMDEventSetCallback
.globl  PMDEventClose
.globl  PMDMailboxOpen
.globl  PMDMutexCreate
.globl  PMDMutexLockEx
.globl  PMDMutexUnlock
.globl  PMDMutexDelete



#define location                table    ; .set table, table + 4
.set table,                          OS_ADDRESS_FUNC_TABLE

.equ  Reserved                      ,location
.equ  PMDAxisOpen                   ,location
.equ  Reserved                      ,location
.equ  PMDPeriphOpenDeviceMP         ,location
.equ  PMDPeriphOpenDevicePRP        ,location
.equ  PMDDeviceOpenMemory           ,location
.equ  PMDMailboxOpen                ,location
.equ  Reserved                      ,location
.equ  Reserved                      ,location
.equ  Reserved                      ,location
.equ  PMDDeviceOpenPeriphPRP        ,location
.equ  PMDDeviceOpenPeriphCME        ,location
.equ  PMDDeviceOpenPeriphSPI        ,location
.equ  PMDDeviceOpenPeriphPIO        ,location
.equ  Reserved                      ,location
.equ  PMDDeviceOpenPeriphSerial     ,location
.equ  PMDDeviceOpenPeriphCAN        ,location
.equ  PMDDeviceOpenPeriphTCP        ,location
.equ  PMDDeviceOpenPeriphUDP        ,location
.equ  Reserved                      ,location
.equ  PMDPeriphOpenPeriphMultiDrop  ,location
.equ  PMDDeviceOpenPeriphCANFD      ,location
.equ  Reserved                      ,location
.equ  PMDprintf                     ,location
.equ  PMDsprintf                    ,location
.equ  PMDputs                       ,location
.equ  PMDputch                      ,location
.equ  PMDwrite                      ,location
.equ  Reserved                      ,location
.equ  PMDTaskCreate                 ,location
.equ  PMDCMETaskStart               ,location
.equ  PMDTaskWait                   ,location
.equ  PMDTaskWaitUntil              ,location
.equ  PMDCMETaskGetState            ,location
.equ  PMDTaskAbort                  ,location
.equ  PMDTaskGetAbortCode           ,location
.equ  PMDTaskSetPriority            ,location
.equ  PMDCMETaskGetInfo             ,location
.equ  PMDTaskGetNumber              ,location
.equ  PMDDeviceGetInfo              ,location
.equ  PMDDeviceGetVersion           ,location
.equ  PMDDeviceGetDefault           ,location
.equ  PMDDeviceSetDefault           ,location
.equ  PMDDeviceGetTickCount         ,location
.equ  PMDDeviceReset                ,location
.equ  PMDDeviceGetResetCause        ,location
.equ  PMDCMESetConsole              ,location
.equ  PMDDeviceNoOperation          ,location
.equ  PMDCMEStoreUserCode           ,location
.equ  PMDCMEGetUserCodeVersion      ,location
.equ  PMDCMEGetUserCodeName         ,location
.equ  PMDCMEGetUserCodeDate         ,location
.equ  PMDDeviceGetFaultCode         ,location
.equ  PMDDeviceGetMicroseconds      ,location
.equ  PMDDeviceSetNodeID            ,location
.equ  PMDDeviceSetMicroseconds      ,location
.equ  PMDCMEGetUserCodeChecksum     ,location
.equ  Reserved                      ,location
.equ  PMDDeviceGetSystemTime        ,location
.equ  PMDDeviceSetSystemTime        ,location
.equ  ReadPIObus                    ,location
.equ  WritePIObus                   ,location
.equ  ReadPIObus32                  ,location
.equ  WritePIObus32                 ,location
.equ  Reserved                      ,location
.equ  Reserved                      ,location
.equ  PMDEventOpen                  ,location
.equ  PMDEventWait                  ,location
.equ  PMDEventSetCallback           ,location
.equ  PMDEventClose                 ,location
.equ  Reserved                      ,location
.equ  Reserved                      ,location
.equ  Reserved                      ,location
.equ  PMDTaskEnterCritical          ,location
.equ  PMDTaskExitCritical           ,location
.equ  Reserved                      ,location
.equ  PMDMutexCreate                ,location
.equ  PMDMutexLockEx                ,location
.equ  PMDMutexUnlock                ,location
.equ  PMDMutexDelete                ,location
