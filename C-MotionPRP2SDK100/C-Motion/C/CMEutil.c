//
// TestUtils.c :  
//
// Utility functions for obtaining device state information
//

#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDperiph.h"
#include "PMDdevice.h"
#include "PMDsys.h"

#define TESTFUNCTION(_fn) \
        PMDprintf("Start %s\r\n", #_fn); \
        PMD_RESULT(_fn(g_phPeriphRx, g_phPeriphTx)); \
        PMDprintf("End %s\r\n", #_fn); 


#define GET_FAULT_CODE(infoID) \
PMD_RESULT(PMDDeviceGetFaultCode(phDevice, infoID, &info)); \
PMDprintf("%40s = %08X %d\r\n", #infoID, info, info);

void GetFaultCodes(PMDDeviceHandle* phDevice)
{
    PMDresult result;
    PMDuint32 info = 0;

    PMDprintf("\r\n");                                           
    GET_FAULT_CODE(PMDFaultCode_ResetCause)                      ;
    GET_FAULT_CODE(PMDFaultCode_Initialization)                  ;
    GET_FAULT_CODE(PMDFaultCode_Exception)                       ;
    //GET_FAULT_CODE((PMDFaultCode)PMDFaultCode_FlashErrorCount)   ;
    //GET_FAULT_CODE((PMDFaultCode)PMDFaultCode_RAMErrorCount)     ;
    //GET_FAULT_CODE((PMDFaultCode)PMDFaultCode_Exception_FSR)     ;
    //GET_FAULT_CODE((PMDFaultCode)PMDFaultCode_Exception_PC)      ;
    //GET_FAULT_CODE((PMDFaultCode)PMDFaultCode_Exception_LR)      ;
    //GET_FAULT_CODE((PMDFaultCode)PMDFaultCode_FlashErrorAddress) ;
    //GET_FAULT_CODE((PMDFaultCode)PMDFaultCode_RAMErrorAddress)   ;
}

#define GET_DEVICE_INFO(infoID, option) \
PMD_RESULT(PMDDeviceGetInfo(phDevice, infoID, option, &info)); \
if (result == PMD_NOERROR) \
    PMDprintf("%38s %d = %08X %d \r\n", #infoID, option, info, info);


void GetDeviceInfo(PMDDeviceHandle* phDevice)
{
    PMDresult result;
    PMDuint32 info;

    PMDprintf("\r\n");
    GET_DEVICE_INFO(PMDDeviceInfo_CMEVersion, 0);
    GET_DEVICE_INFO(PMDDeviceInfo_LogicVersion, 0);
    GET_DEVICE_INFO(PMDDeviceInfo_HostInterface, 0);
    GET_DEVICE_INFO(PMDDeviceInfo_IPaddress, 0);
    GET_DEVICE_INFO(PMDDeviceInfo_Heap, PMDHeap_AvailableBytes);
    GET_DEVICE_INFO(PMDDeviceInfo_Heap, PMDHeap_MinimumAvailableBytes);
    GET_DEVICE_INFO(PMDDeviceInfo_MemorySize, PMDMemoryAddress_NVRAM);
    GET_DEVICE_INFO(PMDDeviceInfo_MemorySize, PMDMemoryAddress_RAM);
}

void GetUserCodeInfo(PMDDeviceHandle *phDevice)
{
    PMDresult result;
    char szValue[100];
    PMDuint32 value32 = 0;

    PMDprintf("User code currently on device:\r\n");
    szValue[0] = 0;
    PMD_RESULT(PMDCMEGetUserCodeName(phDevice, szValue));
    PMDprintf("%40s = %.50s\r\n", "Name", szValue);
    PMD_RESULT(PMDCMEGetUserCodeDate(phDevice, szValue));
    PMDprintf("%40s = %.30s\r\n", "Date", szValue);
    PMD_RESULT(PMDCMEGetUserCodeVersion(phDevice, &value32));
    PMDprintf("%40s = %08X\r\n", "Version", value32);
    PMD_RESULT(PMDCMEGetUserCodeChecksum(phDevice, &value32));
    PMDprintf("%40s = %08X\r\n", "Checksum", value32);
}

const char *GetTaskStateMessage(int state)
{
    return 
        state == PMDTaskState_NoCode        ? "no code" :
        state == PMDTaskState_NotStarted    ? "not started" :
        state == PMDTaskState_Running       ? "running" :
        state == PMDTaskState_Aborted       ? "aborted" :
        state == PMDTaskState_Error         ? "error" :
        "undefined";
};

#define GET_TASK_INFO(infoID) \
info = 0; \
PMD_RESULT(PMDTaskGetInfo(phDevice, taskno, infoID, &info)); \
PMDprintf("%38s %d = 0x%08X %d\r\n", #infoID, taskno, info, info);

void GetTaskInfo(PMDDeviceHandle* phDevice, int tasknumber)
{
    PMDresult result;
    PMDint32 info;
    int taskno = tasknumber;
    PMDint32 taskstate;

    PMDprintf("\r\n");
    PMD_RESULT(PMDTaskGetInfo(phDevice, taskno, PMDTaskInfo_State, &taskstate));
    PMDprintf("Task #%d state = %s\r\n", taskno, GetTaskStateMessage(taskstate));
    if ((taskstate == PMDTaskState_Running || taskstate == PMDTaskState_Aborted))
    {
        GET_TASK_INFO(PMDTaskInfo_State);
        GET_TASK_INFO(PMDTaskInfo_AbortCode)            ;
        GET_TASK_INFO(PMDTaskInfo_StackRemaining)       ;
        GET_TASK_INFO(PMDTaskInfo_StackSize)            ;
        GET_TASK_INFO(PMDTaskInfo_Priority)             ;
        GET_TASK_INFO(PMDTaskInfo_Name); // PMDTaskInfo_Name returns the first 4 characters in the 32-bit word
        PMDprintf("%38s %d = ", "PMDTaskInfo_Name", taskno);
        for (int i=0; i < 7; i++)
        {
            PMD_RESULT(PMDTaskGetInfo(phDevice, taskno, PMDTaskInfo_Name + i, &info));
            if (info == 0) // all NULLs = end of string
                break;
            PMDprintf("%.4s", (char*)&info);
        }
        PMDprintf("\r\n");
    }
}

int GetAllTaskInfo(PMDDeviceHandle* phDevice)
{
    PMDresult result;
    int taskno = 0;
    int runningtaskcount = 0;
    PMDint32 taskstate;

    do {
        PMD_RESULT(PMDTaskGetInfo(NULL, taskno, PMDTaskInfo_State, &taskstate))
        GetTaskInfo(phDevice, taskno);
        taskno++;
        if (taskstate == PMDTaskState_Running)
            runningtaskcount++;
    } while (result == PMD_ERR_OK && (taskstate == PMDTaskState_Running || taskstate == PMDTaskState_Aborted));

    return runningtaskcount;
}

PMDresult PrintCMEInfo(PMDDeviceHandle *phDevice)
{
    PMDresult result;
    PMDuint32 major = 0;
    PMDuint32 minor = 0;

    PMD_RESULT(PMDDeviceGetVersion(phDevice, &major, &minor));

    if (result == PMD_ERR_OK)
    {
        GetDeviceInfo(phDevice);
        GetFaultCodes(phDevice);
        GetUserCodeInfo(phDevice);
        GetAllTaskInfo(phDevice);
    }
    return result;
}


