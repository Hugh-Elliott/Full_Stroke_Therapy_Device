//
//  PMDRPdevice.cpp -- PMD Resource Protocol device functions
//
//  Performance Motion Devices, Inc.
//

#include "PMDtypes.h"
#include "PMDecode.h"
#include "PMDperiph.h"
#include "PMDtrans.h"
#include "PMDdevice.h"
#include "PMDrpperiph.h"
#include "PMDrpdevice.h"


#define PMDRESULT(_call)  result = _call;
#define PMD_VERIFYDEVICEHANDLE(handle)  \
    if( handle == NULL ) return PMD_ERR_InvalidHandle; \
    if (handle->transport_data == NULL) return PMD_ERR_InvalidHandle; \
    PMDRPperiph* rp = (PMDRPperiph*)handle->transport_data; \
    PMDresult result = PMD_ERR_OK;


//********************************************************
// private functions
//********************************************************
static PMDresult PMDRPReset(void* transport_data)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->Reset());

    return result;
}

//********************************************************
static PMDresult PMDRPSendCommand(void* transport_data, PMDuint8 xCt, PMDuint16* xDat, PMDuint8 rCt, PMDuint16* rDat)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    // The remote device address is sent in the PRP packet.
    PMDRESULT(rp->Command(0, xCt, xDat, rCt, rDat));

    return result;
}

//********************************************************
static PMDresult PMDMPSendCommand(void* transport_data, PMDuint8 xCt, PMDuint16* xDat, PMDuint8 rCt, PMDuint16* rDat)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->Command(hDevice->address, xCt, xDat, rCt, rDat));

    return result;
}

//********************************************************
static PMDresult PMDRPPeriphClose(PMDPeriphHandle* hPeriph)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)hPeriph->transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->ClosePeriph( hPeriph->address ));
    hPeriph->handle = PMD_INVALID_HANDLE;

    return result;
}

//********************************************************
static PMDresult PMDRPPeriphSend(PMDPeriphHandle* hPeriph, const void* data, PMDparam nCount, PMDparam timeoutms)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)hPeriph->transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->SendPeriph( hPeriph->address, (char*)data, nCount, timeoutms ));

    return result;
}

//********************************************************
static PMDresult PMDRPPeriphReceive(PMDPeriphHandle* hPeriph, void* data, PMDparam nMaxExpected, PMDparam* pnreceived, PMDparam timeoutms)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)hPeriph->transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->ReceivePeriph( hPeriph->address, (char*)data, nMaxExpected, timeoutms, pnreceived));

    return result;
}

//********************************************************
static PMDresult PMDRPPeriphWrite(PMDPeriphHandle* hPeriph, void* data, PMDuint32 offset_in_bytes, PMDuint32 words_to_write)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)hPeriph->transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->WritePeriph( hPeriph->address, (PMDuint16*)data, offset_in_bytes, words_to_write));

    return result;
}

//********************************************************
static PMDresult PMDRPPeriphRead(PMDPeriphHandle* hPeriph, void* data, PMDuint32 offset_in_bytes, PMDuint32 words_to_read)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)hPeriph->transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->ReadPeriph( hPeriph->address, (PMDuint16*)data, offset_in_bytes, words_to_read));

    return result;
}

//********************************************************
static void PMDRPPeriph_Init(PMDPeriphHandle* hPeriph)
{
    // set the interface type
    hPeriph->type = InterfaceNone;

    hPeriph->transport.Close            = PMDRPPeriphClose;
    hPeriph->transport.Send             = PMDRPPeriphSend;
    hPeriph->transport.Receive          = PMDRPPeriphReceive;
    hPeriph->transport.Read             = NULL;
    hPeriph->transport.Write            = NULL;
    hPeriph->transport.ReceiveEvent     = NULL;
}

//********************************************************
// Open an existing peripheral in the CME device with a known address such as the console address 3
PMDresult PMDRPPeriph_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDparam address)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)

    if (result == PMD_ERR_OK)
    {
        PMDRPPeriph_Init(hPeriph);
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = address;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}

//********************************************************
PMDresult PMDRPCOM_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDparam portnum, PMDparam baud, PMDparam parity, PMDparam stopbits)
{
    int address;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenPeriphSerial(&address, portnum, baud, parity, stopbits));

    if (result == PMD_ERR_OK)
    {
        PMDRPPeriph_Init(hPeriph);
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = address;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}

//********************************************************
PMDresult PMDRPCAN_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDparam addressTX, PMDparam addressRX, PMDparam addressEvent)
{
    int address;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenPeriphCAN(&address, addressTX, addressRX, addressEvent));

    if (result == PMD_ERR_OK)
    {
        PMDRPPeriph_Init(hPeriph);
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = address;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}

//********************************************************
PMDresult PMDRPCANFD_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDparam portnum, PMDparam baud, PMDparam addressTX, PMDparam addressRX, PMDparam addressMode)
{
    int address;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenPeriphCAN(&address, portnum, baud, addressTX, addressRX, addressMode));

    if (result == PMD_ERR_OK)
    {
        PMDRPPeriph_Init(hPeriph);
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = address;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}

//********************************************************
PMDresult PMDRPSPI_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDuint8 portnum, PMDuint8 chipselect, PMDuint8 mode, PMDuint8 datasize, PMDparam bitrate)
{
    int address;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenPeriphSPI(&address, portnum, chipselect, mode, datasize, bitrate));

    if (result == PMD_ERR_OK)
    {
        PMDRPPeriph_Init(hPeriph);
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = address;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}


//********************************************************
PMDresult PMDRPUDP_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDparam ipaddress, PMDparam portnum)
{
    int address;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenPeriphUDP(&address, ipaddress, portnum));

    if (result == PMD_ERR_OK)
    {
        PMDRPPeriph_Init(hPeriph);
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = address;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}

//********************************************************
PMDresult PMDRPTCP_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam ipaddress, PMDparam portnum)
{
    int address;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenPeriphTCP(&address, ipaddress, portnum));

    if (result == PMD_ERR_OK)
    {
        PMDRPPeriph_Init(hPeriph);
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = address;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}

//********************************************************
PMDresult PMDRPPAR_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDuint16 address, PMDuint8 eventIRQ, PMDDataSize datasize)
{
    int periphaddress;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenPeriphPIO(&periphaddress, address, eventIRQ, datasize));

    if (result == PMD_ERR_OK)
    {
        hPeriph->transport.Close            = PMDRPPeriphClose;
        hPeriph->transport.Send             = NULL;
        hPeriph->transport.Receive          = NULL;
        hPeriph->transport.Read             = PMDRPPeriphRead;
        hPeriph->transport.Write            = PMDRPPeriphWrite;
        hPeriph->transport.ReceiveEvent     = NULL;
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = periphaddress;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}

//********************************************************
PMDresult PMDRPISA_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDuint16 address, PMDuint8 eventIRQ, PMDDataSize datasize)
{
    int periphaddress;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenPeriphISA(&periphaddress, address, eventIRQ, datasize));

    if (result == PMD_ERR_OK)
    {
        hPeriph->transport.Close            = PMDRPPeriphClose;
        hPeriph->transport.Send             = NULL;
        hPeriph->transport.Receive          = NULL;
        hPeriph->transport.Read             = PMDRPPeriphRead;
        hPeriph->transport.Write            = PMDRPPeriphWrite;
        hPeriph->transport.ReceiveEvent     = NULL;
        hPeriph->transport_data = hDevice;
        hPeriph->type = InterfaceRemote;
        hPeriph->address = periphaddress;
        hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected
    }

    return result;
}

//********************************************************
PMDresult PMDRPAddress_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle* hDevice, PMDparam periphaddress)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRPPeriph_Init(hPeriph);
    hPeriph->transport_data = hDevice;
    hPeriph->type = InterfaceRemote;
    hPeriph->address = periphaddress;
    hPeriph->handle = PMD_CONNECTED_HANDLE; // set the periph handle as connected

    return result;
}

//********************************************************
// public functions
//********************************************************
static PMDresult PMDRPMemoryClose(PMDMemoryHandle* hMemory)
{
    PMDDeviceHandle* hDevice = hMemory->hDevice;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->CloseMemory( hMemory->address ));

    return result;
}

static PMDresult PMDRPMemoryErase(PMDMemoryHandle* hMemory)
{
    PMDDeviceHandle* hDevice = hMemory->hDevice;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->EraseMemory( hMemory->address ));

    return result;
}

//********************************************************
PMDresult PMDRPMemoryOpen(PMDMemoryHandle* hMemory, PMDDeviceHandle* hDevice, PMDDataSize datasize, PMDMemoryType memorytype)
{
    int address;
    int memsize;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenMemory(&address, &memsize, datasize, memorytype));

    if (result == PMD_ERR_OK)
    {
        hMemory->hDevice  = hDevice;
        hMemory->datasize = datasize;
        hMemory->address  = address;
        hMemory->length  = memsize;
        hMemory->transport.Read  = PMDRPMemoryRead;
        hMemory->transport.Write = PMDRPMemoryWrite;
        hMemory->transport.Close = PMDRPMemoryClose;
        hMemory->transport.Erase = PMDRPMemoryErase;
    }
    return result;
}

//********************************************************
PMDresult PMDRPMemoryWrite(PMDMemoryHandle* hMemory, void* data, PMDuint32 offset, PMDuint32 length)
{
    PMDDeviceHandle* hDevice = hMemory->hDevice;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    if (hMemory->datasize == PMDDataSize_32Bit)
        PMDRESULT(rp->WriteMemory(hMemory->address, (PMDuint32*)data, offset, length))
    else if (hMemory->datasize == PMDDataSize_16Bit)
        PMDRESULT(rp->WriteMemory(hMemory->address, (PMDuint16*)data, offset, length))
    else
        result = PMD_ERR_NotSupported;

    return result;
}

//********************************************************
PMDresult PMDRPMemoryRead(PMDMemoryHandle* hMemory, void* data, PMDuint32 offset, PMDuint32 length)
{
    PMDDeviceHandle* hDevice = hMemory->hDevice;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    if (hMemory->datasize == PMDDataSize_32Bit)
        PMDRESULT(rp->ReadMemory(hMemory->address, (PMDuint32*)data, offset, length))
    else if (hMemory->datasize == PMDDataSize_16Bit)
        PMDRESULT(rp->ReadMemory(hMemory->address, (PMDuint16*)data, offset, length))
    else
        result = PMD_ERR_NotSupported;

    return result;
}

//********************************************************
static PMDresult PMDRPMailboxSend(PMDMailboxHandle* hMailbox, const void *pItem, PMDuint32 timeout)
{
    PMDDeviceHandle* hDevice = hMailbox->hDevice;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->SendMail( hMailbox->mailboxID, pItem, hMailbox->itemsize, timeout ));

    return result;
}

//********************************************************
static PMDresult PMDRPMailboxReceive(PMDMailboxHandle* hMailbox, void *pItem, PMDuint32 timeout)
{
    PMDDeviceHandle* hDevice = hMailbox->hDevice;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->ReceiveMail( hMailbox->mailboxID, pItem, hMailbox->itemsize, timeout ));

    return result;
}

//********************************************************
static PMDresult PMDRPMailboxPeek(PMDMailboxHandle* hMailbox, void *pItem, PMDuint32 timeout)
{
    PMDDeviceHandle* hDevice = hMailbox->hDevice;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->PeekMail( hMailbox->mailboxID, pItem, hMailbox->itemsize, timeout ));

    return result;
}

//********************************************************
PMDresult PMDRPMailboxOpen(PMDMailboxHandle* hMailbox, PMDDeviceHandle *hDevice, PMDparam mailboxID, PMDparam depth, PMDparam itemsize)
{
    int address;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->OpenMailbox(&address, mailboxID, depth, itemsize));

    if (result == PMD_ERR_OK)
    {
        hMailbox->hDevice  = hDevice;
        hMailbox->address  = address;
        hMailbox->itemsize = itemsize;
        hMailbox->mailboxID  = mailboxID;
        hMailbox->transport.Send    = PMDRPMailboxSend;
        hMailbox->transport.Receive = PMDRPMailboxReceive;
        hMailbox->transport.Peek    = PMDRPMailboxPeek;
        hMailbox->transport.Close   = NULL;
    }
    return result;
}

//********************************************************
PMDCFunc PMDDeviceGetVersion(PMDDeviceHandle* hDevice, PMDuint32* major, PMDuint32* minor)
{
    PMDuint32 version = 0;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->GetFirmwareVersion(&version));
    *major = version >> 16;
    *minor = version & 0xFFFF;

    return result;
}

//********************************************************
PMDCFunc PMDDeviceGetInfo(PMDDeviceHandle *hDevice, PMDDeviceInfo ID, PMDuint16 option, PMDuint32* value)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->GetDeviceInfo(ID, option, value));

    return result;
}


//********************************************************
PMDCFunc PMDDeviceGetSystemTime(PMDDeviceHandle* hDevice, SYSTEMTIME* time)
{
    PMDuint32 version = 0;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->GetSystemTime(time));

    return result;
}

//********************************************************
PMDCFunc PMDDeviceSetSystemTime(PMDDeviceHandle* hDevice, const SYSTEMTIME* time)
{
    PMDuint32 version = 0;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->SetSystemTime(time));

    return result;
}

//********************************************************
PMDCFunc PMDDeviceNoOperation(PMDDeviceHandle* hDevice)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->NoOperation());

    return result;
}

//********************************************************
PMDCFunc PMDDeviceReset(PMDDeviceHandle* hDevice)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->Reset());
    PMDTaskWait(1000); // This command responds before processing it so allow enough time for the command to be processed.

    return result;
}

//********************************************************
PMDCFunc PMDCMETaskGetState(PMDDeviceHandle* hDevice, PMDTaskState* state)
{
    PMDint32 status;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->GetUserCodeStatus(0, &status));
    *state = (PMDTaskState)status;

    return result;
}

//********************************************************
PMDCFunc PMDCMETaskGetInfo(PMDDeviceHandle *hDevice, PMDuint8 taskno, PMDTaskInfo ID, PMDint32* value)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->GetTaskInfo(taskno, ID, value));

    return result;
}

//********************************************************
PMDCFunc PMDCMETaskStart(PMDDeviceHandle* hDevice, PMDparam taskparam)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->StartUserTasks(taskparam));

    return result;
}

//********************************************************
PMDCFunc PMDCMETaskStop(PMDDeviceHandle* hDevice)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->StopUserTasks());

    return result;
}

//********************************************************
// Send a user code (.bin) file to the CME.
// pdata is a pointer to the data of the .bin file in memory.
// length is the size of the buffer pdata is poitning to.
PMDCFunc PMDCMEStoreUserCode(PMDDeviceHandle* hDevice, PMDuint8* pdata, int length)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    const int blocksize = MAX_PACKET_DATA_LENGTH;

    PMDRESULT(rp->StoreUserCodeBegin(length));
    if (result == PMD_ERR_OK)
    {
        while (length > blocksize)
        {
            PMDRESULT(rp->StoreUserCodeData(pdata, blocksize));
            if (result != PMD_ERR_OK)
                return result;
            pdata += blocksize;
            length -= blocksize;
        }
        PMDRESULT(rp->StoreUserCodeData(pdata, length));
        if (result != PMD_ERR_OK)
            return result;
        PMDRESULT(rp->StoreUserCodeEnd());
    }

    return result;
}

//********************************************************
PMDCFunc PMDCMEGetUserCodeVersion(PMDDeviceHandle* hDevice, PMDuint32* version)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->GetUserCodeFileVersion(version));

    return result;
}

//********************************************************
PMDCFunc PMDCMEGetUserCodeChecksum(PMDDeviceHandle* hDevice, PMDuint32* checksum)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->GetUserCodeFileChecksum(checksum));

    return result;
}

//********************************************************
PMDCFunc PMDCMEGetUserCodeName(PMDDeviceHandle* hDevice, char* filename)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->GetUserCodeFileName(filename, 250))

    return result;
}

//********************************************************
PMDCFunc PMDCMEGetUserCodeDate(PMDDeviceHandle* hDevice, char* date)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->GetUserCodeFileDate(date, 250));

    return result;
}


//********************************************************
PMDCFunc PMDDeviceGetDefault(PMDDeviceHandle* hDevice, PMDDefaults defaultcode, void* value, PMDDataSize valueSize)
{
    PMDuint32 dwValue = 0;
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->GetDefault(defaultcode, &dwValue));

    if (valueSize == PMDDataSize_32Bit)
        *(PMDuint32*) value = dwValue;
    else if (valueSize == PMDDataSize_16Bit)
        *(PMDuint16*) value = (PMDuint16)dwValue;
    else
        return PMD_ERR_InvalidParameter;
    return result;
}

//********************************************************
PMDCFunc PMDDeviceSetDefault(PMDDeviceHandle* hDevice, PMDDefaults defaultcode, void* value, PMDDataSize valueSize)
{
    PMDuint32 dwValue = 0;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    if (valueSize == PMDDataSize_32Bit)
        dwValue =* (PMDuint32*) value;
    else if (valueSize == PMDDataSize_16Bit)
        dwValue =* (PMDuint16*) value;
    else
        return PMD_ERR_InvalidParameter;

    PMDRESULT(rp->SetDefault(defaultcode, dwValue));
    return result;
}

//********************************************************
PMDCFunc PMDCMESetConsole(PMDDeviceHandle* hDevice, PMDPeriphHandle* hPeriph)
{
    int address;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    if( hPeriph == NULL )
      address = 0; // 0 disables console output
    else
    {
      if( hPeriph->handle == PMD_INVALID_HANDLE )
        return PMD_ERR_NotConnected;
      address = hPeriph->address;
    }
    PMDRESULT(rp->SetConsole(address));

    return result;
}

//********************************************************
PMDCFunc PMDDeviceSetNodeID(PMDDeviceHandle* hDevice, PMDuint8 nodeID, PMDuint8 DOsignal, PMDuint8 DIsignal, PMDuint8 DIsense)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->SetNodeID(nodeID, DOsignal, DIsignal, DIsense));
    PMDTaskWait(30); // This command responds before processing it so allow enough time for the command to be processed.

    return result;
}


//********************************************************
PMDCFunc PMDDeviceGetResetCause (PMDDeviceHandle* hDevice, PMDuint16* resetcause, PMDuint16 resetmask)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->GetResetCause(resetmask, resetcause));

    return result;
}

//********************************************************
PMDCFunc PMDDeviceGetFaultCode(PMDDeviceHandle *hDevice, PMDFaultCode faultID, PMDuint32* value)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDRESULT(rp->GetFaultCode(faultID, value));

    return result;
}

//********************************************************
static PMDresult PMDRPDeviceClose(PMDDeviceHandle* hDevice)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)

    if (hDevice->transport_data != NULL)
        delete (PMDRPperiph*)(hDevice->transport_data);

    return PMD_ERR_OK;
}

//********************************************************
static PMDresult PMDRPDeviceCloseRemote(PMDDeviceHandle* hDevice)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    PMDDeviceHandle* hDeviceParent = (PMDDeviceHandle*)hDevice->hPeriph->transport_data;
    PMDRPperiph* rpParent = (PMDRPperiph*)hDeviceParent->transport_data; 

    PMDRESULT(rpParent->CloseDevice( hDevice->address ));

    return result;
}

//********************************************************
// Open a Resource Protocol Device (Prodigy/CME)
PMDCFunc PMDPeriphOpenDevicePRP(PMDDeviceHandle* hDevice, PMDPeriphHandle* hPeriph)
{
    PMDresult result = PMD_ERR_OK;
    PMDZeroMemory(hDevice, sizeof(PMDDeviceHandle));
    PMD_PERIPHCONNECTED(hPeriph)
    PMDRPperiph* pRP = NULL;

    hDevice->transport.SendCommand  = PMDRPSendCommand;
    hDevice->transport.Reset        = PMDRPReset;
    hDevice->transport.WaitForEvent = NULL;
    hDevice->transport.Close        = PMDRPDeviceClose;
    hDevice->transport.ReadMemory   = NULL;
    hDevice->transport.WriteMemory  = NULL;

    if (hPeriph->type == InterfaceCAN)
    { 
        if (hPeriph->param & 0xF000)
            pRP = new PMDRPperiphCANFD(hPeriph);
        else
            pRP = new PMDRPperiphCAN(hPeriph);
    }
    else if (hPeriph->type == InterfaceSerial)
        pRP = new PMDRPperiphCOM(hPeriph);
    else if (hPeriph->type == InterfaceTCP)
        pRP = new PMDRPperiphTCP(hPeriph);
    else if (hPeriph->type == InterfaceSPI)
        pRP = new PMDRPperiphSPI(hPeriph);
    else if (hPeriph->type == InterfaceRemote)
    {
        pRP = new PMDRPperiphRemote(hPeriph);
        result = ((PMDRPperiphRemote*) pRP)->Open();
        hDevice->transport.Close    = PMDRPDeviceCloseRemote;
        if (result != PMD_ERR_OK)
        {
            delete pRP;
            pRP = NULL;
        }
    }
    else
        pRP = new PMDRPperiph(hPeriph);
    hDevice->transport_data = pRP;
    hDevice->address = pRP->m_Address;
    hDevice->type = PMDDeviceTypeResourceProtocol;
    hDevice->hPeriph = hPeriph;

    return result;
}

//********************************************************
// Open a Motion Processor Device (Magellan/Juno)
PMDresult PMDPeriphOpenDeviceMPRemote(PMDDeviceHandle* hDevice, PMDPeriphHandle* hPeriph)
{
    PMD_PERIPHCONNECTED(hPeriph)
    PMDDeviceHandle* hDevicePRP = (PMDDeviceHandle*)hPeriph->transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevicePRP)
    PMDZeroMemory(hDevice, sizeof(PMDDeviceHandle));
    int resourceaddress;

    result = rp->OpenMotionProcessor(&resourceaddress, hPeriph->address);
    if (result == PMD_ERR_OK)
    {
        hDevice->transport_data = rp;
        hDevice->type = PMDDeviceTypeResourceProtocol;
        hDevice->hPeriph = hPeriph;
        hDevice->address = resourceaddress;
        hDevice->transport.SendCommand  = PMDMPSendCommand;
        hDevice->transport.Reset        = NULL; // use the software PMDReset command.
        hDevice->transport.WaitForEvent = NULL;
        hDevice->transport.Close        = PMDRPDeviceCloseRemote;
        hDevice->transport.ReadMemory   = NULL;
        hDevice->transport.WriteMemory  = NULL;
    }

    return result;
}

//********************************************************
// CME user packet 'peripheral'
//********************************************************
static PMDresult PMDPCME_Send(PMDPeriphHandle* hPeriph, const void* pData, PMDuint32 nCount, PMDuint32 timeout)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)hPeriph->transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->SendCME(hPeriph->address, (char*)pData, nCount, timeout));

    return result;
}

//********************************************************
static PMDresult PMDPCME_Receive(PMDPeriphHandle* hPeriph, void* pData, PMDuint32 nCount, PMDparam* pnreceived, PMDuint32 timeout)
{
    PMDDeviceHandle* hDevice = (PMDDeviceHandle*)hPeriph->transport_data;
    PMD_VERIFYDEVICEHANDLE(hDevice)

    PMDRESULT(rp->ReceiveCME(hPeriph->address, (char*)pData, nCount, timeout, pnreceived));

    return result;
}

//********************************************************
static void PMDPCME_Init(PMDPeriphHandle* hPeriph)
{
    // set the interface type
    hPeriph->type = InterfaceNone;

    hPeriph->transport.Close            = NULL;
    hPeriph->transport.Send             = PMDPCME_Send;
    hPeriph->transport.Receive          = PMDPCME_Receive;
    hPeriph->transport.Read             = NULL;
    hPeriph->transport.Write            = NULL;
    hPeriph->transport.ReceiveEvent     = NULL;
}

//********************************************************
PMDresult PMDPPRP_Open(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam channel, PMDparam bufsize)
{
    if (hDevice == NULL)
        return PMD_ERR_InvalidOperation;

    PMDPCME_Init(hPeriph);

    // set the interface type 
    hPeriph->type = InterfacePRP;
    hPeriph->transport_data = hDevice;
    hPeriph->handle = PMD_CONNECTED_HANDLE;
    hPeriph->address = channel;

    return PMD_ERR_OK;
}

