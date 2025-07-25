/*
  "Base Class" implementation for PMD CME functions

  Performance Motion Devices, Inc.
*/

#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"

//********************************************************
PMDCFunc PMDPeriphReceiveEvent(PMDPeriphHandle* hPeriph, void *pData, PMDparam *pnReceived, PMDparam nExpected, PMDparam timeout)
{
    PMD_PERIPHCONNECTED(hPeriph)
    if (hPeriph->transport.ReceiveEvent == NULL)
        return PMD_ERR_NotSupported;

    return hPeriph->transport.ReceiveEvent(hPeriph, pData, nExpected, pnReceived, timeout);
}

//********************************************************
PMDCFunc PMDPeriphReceive(PMDPeriphHandle* hPeriph, void *pData, PMDparam *pnReceived, PMDparam nExpected, PMDparam timeout)
{
    PMD_PERIPHCONNECTED(hPeriph)
    if (hPeriph->transport.Receive == NULL)
        return PMD_ERR_NotSupported;

    return hPeriph->transport.Receive(hPeriph, pData, nExpected, pnReceived, timeout);
}

//********************************************************
PMDCFunc PMDPeriphSend(PMDPeriphHandle* hPeriph, const void *pData, PMDparam nCount, PMDparam timeout)
{
#ifdef WIN32
	LARGE_INTEGER CountsPerSec;
	LARGE_INTEGER BeginTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER CommandProcessingTime;
#endif
    PMDresult result;
    PMD_PERIPHCONNECTED(hPeriph)
    if (hPeriph->transport.Send == NULL)
        return PMD_ERR_NotSupported;

#ifdef WIN32
    QueryPerformanceFrequency(&CountsPerSec);
	QueryPerformanceCounter(&BeginTime);
    result = hPeriph->transport.Send(hPeriph, pData, nCount, timeout);
	QueryPerformanceCounter(&EndTime);
	CommandProcessingTime.QuadPart = EndTime.QuadPart - BeginTime.QuadPart;
    // convert to microseconds
	CommandProcessingTime.QuadPart = CommandProcessingTime.QuadPart * 1000000 / CountsPerSec.QuadPart; 
//    printf( " Send time: %I64u�s ", CommandProcessingTime.QuadPart);
#else
    result = hPeriph->transport.Send(hPeriph, pData, nCount, timeout);
#endif

    return result;
}

//********************************************************
PMDCFunc PMDPeriphWrite(PMDPeriphHandle* hPeriph, void *pData, PMDparam offset, PMDparam length)
{
    PMD_PERIPHCONNECTED(hPeriph)
    if (hPeriph->transport.Write == NULL)
        return PMD_ERR_NotSupported;

    return hPeriph->transport.Write(hPeriph, pData, offset, length);
}

//********************************************************
PMDCFunc PMDPeriphRead(PMDPeriphHandle* hPeriph, void *pData, PMDparam offset, PMDparam length)
{
    PMD_PERIPHCONNECTED(hPeriph)
    if (hPeriph->transport.Read == NULL)
        return PMD_ERR_NotSupported;

    return hPeriph->transport.Read(hPeriph, pData, offset, length);
}

//********************************************************
void PMDPeriphOut(PMDPeriphHandle* hPeriph, PMDparam offset, PMDparam data)
{
    PMDPeriphWrite(hPeriph, &data, offset, 1);
}

//********************************************************
PMDparam PMDPeriphIn(PMDPeriphHandle* hPeriph, PMDparam offset)
{
    PMDparam data = 0;
    PMDPeriphRead(hPeriph, &data, offset, 1);

    return data;
}

//********************************************************
PMDCFunc PMDPeriphClose(PMDPeriphHandle* hPeriph)
{
    PMDresult result = PMD_ERR_OK;

    PMD_PERIPHCONNECTED(hPeriph)
    if (hPeriph->transport.Close != NULL)
        result = hPeriph->transport.Close(hPeriph);
    PMDZeroMemory( hPeriph, sizeof(PMDPeriphHandle));

    return result;
}

//********************************************************
PMDCFunc PMDPeriphFlush(PMDPeriphHandle* hPeriph)
{
    PMDresult result = PMD_ERR_OK;
    char ch;
    int nCount = 0;
    PMDparam nReceived;

    // an nExpected value of 0 tells the driver to flush the port if supported
//    result = PMDPeriphReceive( hPeriph, &ch, &nReceived, 0, 0 );

    while( PMD_ERR_OK == PMDPeriphReceive( hPeriph, &ch, &nReceived, 1, 0 ) && nReceived==1)
    {
        nCount++;
        if (nCount > 1000)
          return PMD_ERR_UnexpectedDataReceived;
    }

    return result;
}

//********************************************************
// Create an axis handle for communicating directly with an Atlas rather than via the Magellan
// This is required for certain functions such as Atlas trace.
PMDCFunc PMDAtlasAxisOpen(PMDAxisInterface hSourceAxis, PMDAxisInterface hAtlasAxis)
{
    // copy the source axis handle info
    *hAtlasAxis = *hSourceAxis;
    // set the axis number to indicate it is an Atlas axis
    hAtlasAxis->axis = hAtlasAxis->axis | PMDAtlasAxisMask;

    return PMD_ERR_OK;
}

//********************************************************
// PMDAxisClose defined for consistency, but is not required
PMDCFunc PMDAxisClose(PMDAxisHandle *hAxis)
{
    PMDZeroMemory(hAxis, sizeof(PMDAxisHandle));
    return PMD_ERR_OK;
}

//********************************************************
PMDCFunc PMDMemoryClose(PMDMemoryHandle *hMemory)
{
    PMDresult result = PMD_ERR_OK;
    PMD_VERIFYHANDLE(hMemory)
    if (hMemory->length == 0)
        result = PMD_ERR_InvalidHandle;
    if (hMemory->transport.Close)
        result = hMemory->transport.Close(hMemory);
    PMDZeroMemory(hMemory, sizeof(PMDMemoryHandle));

    return result;
}

//********************************************************
PMDCFunc PMDDeviceClose(PMDDeviceHandle *hDevice)
{
    PMDresult result = PMD_ERR_OK;
    PMD_VERIFYHANDLE(hDevice)
    if (hDevice->type == PMDDeviceTypeNone)
      result = PMD_ERR_InvalidHandle;
    if (hDevice->transport.Close)
        result = hDevice->transport.Close(hDevice);
    PMDZeroMemory(hDevice, sizeof(PMDDeviceHandle));

    return result;
}

//********************************************************
PMDCFunc PMDDeviceGetEvent(PMDDeviceHandle* phDevice, PMDEvent* eventdata)
{
    PMDuint16 eventstatus;
    PMDAxis axis = 0;
    PMDuint16 axismask = 0;
    PMDuint16 interruptmask = 0;
    PMDAxisHandle hAxis;
    PMDresult result;

    eventdata->axis = 0;
    eventdata->eventstatus = 0;
    PMDAxisOpen(&hAxis, phDevice, PMDAxis1);
    // returns mask of all axes that caused the interrupt
    result = PMDGetInterruptAxis(&hAxis, &axismask);
    if (result == PMD_ERR_OK)
    {
        if (axismask == 0)
        {
          PMDClearInterrupt(&hAxis);
          return PMD_ERR_Cancelled; // interrupt was reset in Magellan before WaitForEvent was called
        }
        while ((axismask & 1) != 1 && axis < 4)
        {
          axismask >>= 1;
          axis++;
        }
        hAxis.axis = axis;
        PMDGetInterruptMask(&hAxis, &interruptmask);
        PMDGetEventStatus(&hAxis, &eventstatus);
        eventdata->axis = axis;
        eventdata->eventstatus = eventstatus;
    }

    return result;
}

//********************************************************
// The default WaitForEvent function for interfaces that do not support interrupts
PMDCFunc PMDWaitForEventPoll(PMDDeviceHandle* phDevice, PMDEvent* eventdata, PMDuint32 timeoutms)
{
    PMDuint16 eventstatus;
    PMDAxis axis = 0;
    PMDuint16 axismask = 0;
    PMDAxisHandle hAxis;
    PMDresult result;
    PMDuint32 delta, lastTime, currentTime;

    eventdata->axis = 0;
    eventdata->eventstatus = 0;
    PMDAxisOpen(&hAxis, phDevice, PMDAxis1);

    // GetTickCount returns time in ms
    lastTime = PMDDeviceGetTickCount();

    // PMDGetInterruptAxis returns mask of all axes that caused the interrupt
    // but just return the first axis that caused the interrupt this time around.
    do
    {
        result = PMDGetInterruptAxis(&hAxis, &axismask);
        currentTime = PMDDeviceGetTickCount();
        // delta computation should work even if the counter rolls over.
        delta = currentTime - lastTime;
        lastTime = currentTime;
        if (timeoutms <= delta)
            break;
        if (timeoutms < PMD_WAITFOREVER)
            timeoutms -= delta;
    }
    while (result == PMD_ERR_OK && axismask == 0);

    if (result == PMD_ERR_OK)
    {
        if (axismask == 0)
            return PMD_ERR_Timeout;
        // convert axismask to axis number
        while ((axismask & 1) != 1 && axis < 4)
        {
            axismask >>= 1;
            axis++;
        }
        hAxis.axis = axis;
        PMDGetEventStatus(&hAxis, &eventstatus);
        result = PMDResetEventStatus(&hAxis, (PMDuint16)~eventstatus);
        eventdata->axis = axis;
        eventdata->eventstatus = eventstatus;
    }

    return result;
}

//******************************************************** 
PMDCFunc PMDWaitForEvent(PMDDeviceHandle *hDevice, PMDEvent* eventdata, PMDparam timeout)
{
    PMD_VERIFYHANDLE(hDevice)
    if (hDevice->transport.WaitForEvent)
        return hDevice->transport.WaitForEvent(hDevice, eventdata, timeout);
    else
        return PMDWaitForEventPoll(hDevice, eventdata, timeout);
}

//********************************************************
PMDCFunc PMDMemoryRead(PMDMemoryHandle *hMemory, void *data, PMDuint32 offset, PMDuint32 length)
{
    PMDuint32 datasize;
    PMD_VERIFYHANDLE(hMemory)

    datasize = hMemory->datasize - 1;
    if (hMemory->transport.Read)
    {   // check that the supplied data pointer is correctly aligned
        if (datasize & PtrToUlong(data))
            return PMD_ERR_ParameterAlignment;
        else
            return hMemory->transport.Read(hMemory, data, offset, length);
    }
    else
        return PMD_ERR_InterfaceNotInitialized;
}

//********************************************************
PMDCFunc PMDMemoryWrite(PMDMemoryHandle *hMemory, void *data, PMDuint32 offset, PMDuint32 length)
{
    PMDuint32 datasize;
    PMD_VERIFYHANDLE(hMemory)

    datasize = hMemory->datasize - 1;
    if (hMemory->transport.Write)
    {   // check that the supplied data pointer is correctly aligned
        if (datasize & PtrToUlong(data))
            return PMD_ERR_ParameterAlignment;
        else
            return hMemory->transport.Write(hMemory, data, offset, length);
    }
    else
        return PMD_ERR_InterfaceNotInitialized;
}

//********************************************************
PMDCFunc PMDMemoryErase(PMDMemoryHandle *hMemory)
{
    PMD_VERIFYHANDLE(hMemory)
    if (hMemory->transport.Erase)
        return hMemory->transport.Erase(hMemory);
    else
        return PMD_ERR_InterfaceNotInitialized;
}

//********************************************************
// Mailbox functions
PMDCFunc PMDMailboxSend(PMDMailboxHandle* hMailbox, const void *pItem, PMDparam timeout)
{
    PMD_VERIFYHANDLE(hMailbox)
    if (hMailbox->transport.Send == NULL)
        return PMD_ERR_InvalidHandle;

    return hMailbox->transport.Send(hMailbox, pItem, timeout);
}

PMDCFunc PMDMailboxReceive(PMDMailboxHandle* hMailbox, void *pItem, PMDparam timeout)
{
    PMD_VERIFYHANDLE(hMailbox)
    if (hMailbox->transport.Receive == NULL)
        return PMD_ERR_InvalidHandle;

    return hMailbox->transport.Receive(hMailbox, pItem, timeout);
}

PMDCFunc PMDMailboxPeek(PMDMailboxHandle* hMailbox, void *pItem, PMDparam timeout)
{
    PMD_VERIFYHANDLE(hMailbox)
    if (hMailbox->transport.Peek == NULL)
        return PMD_ERR_InvalidHandle;

    return hMailbox->transport.Peek(hMailbox, pItem, timeout);
}

PMDCFunc PMDMailboxClose(PMDMailboxHandle* hMailbox)
{
    PMD_VERIFYHANDLE(hMailbox)
    if (hMailbox->transport.Close == NULL)
        return PMD_ERR_InvalidHandle;

    return hMailbox->transport.Close(hMailbox);
}


PMDCFunc PMDDeviceOpenPeriphCANNodeID(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam baud, PMDparam nodeID)
{
    PMDparam addressTx    = CAN_ADDRESS_BASE_TX  + nodeID;
    PMDparam addressRx    = CAN_ADDRESS_BASE_RX  + nodeID;
    PMDparam addressEvent = CAN_ADDRESS_BASE_INT + nodeID;
    hPeriph->param = nodeID;
    
    return PMDDeviceOpenPeriphCANFD(hPeriph, hDevice, PMDCANPort1, baud, addressTx, addressRx, addressEvent);
//    return PMDDeviceOpenPeriphCANFD(hPeriph, hDevice, PMDCANPort1, baud, addressTx, addressRx, PMDCANMode_StdId);
}
/*
PMDCFunc PMDDeviceOpenPeriphCANMask(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam baud, PMDparam receiveMask, PMDparam receiveId)
{
    return PMDDeviceOpenPeriphCANFD(hPeriph, hDevice, PMDCANPort1, baud, receiveMask, receiveId, PMDCANMode_StdMask);
}

PMDCFunc PMDDeviceOpenPeriphCANRange(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam baud, PMDparam receiveLow, PMDparam receiveHigh)
{
    return PMDDeviceOpenPeriphCANFD(hPeriph, hDevice, PMDCANPort1, baud, receiveLow, receiveHigh, PMDCANMode_StdRange);
}
*/
/*
PMDEventOpen is a common function for all event types
PMDEventOpenMotion is a wrapper function that sets Magellan host interrupt event. 
PMDEventOpenDI is a wrapper function that sets Digital Input event specific parameters
PMDEventOpenTimer is a wrapper function that sets Timer event specific parameters
*/
PMDCFunc PMDEventOpenMotion(PMDEventHandle* hEvent)
{
    return PMDEventOpen (hEvent, PMDEventType_Motion, 0, 0, 0);
}

PMDCFunc PMDEventOpenDI(PMDEventHandle* hEvent, PMDEventNumber number, PMDEventTrigger trigger, PMDEventSignal signal)
{
    return PMDEventOpen (hEvent, PMDEventType_DigitalInput, number, trigger, signal);
}

PMDCFunc PMDEventOpenTimer(PMDEventHandle* hEvent, PMDEventNumber number, PMDEventMode mode, PMDparam periodus)
{
    return PMDEventOpen (hEvent, PMDEventType_Timer, number, mode, periodus);
}

