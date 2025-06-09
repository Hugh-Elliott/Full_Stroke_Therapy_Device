//
//  PMDopen.c -- functions for opening PMD handles
//
//  Performance Motion Devices, Inc.
//

/*
  The PMDP?_Open() functions open the local peripheral
  The PMDRP?_Open() functions open the peripheral on a PMD C-Motion Engine (CME) device.
*/

#include "PMDperiph.h"
#include "PMDrpdevice.h"
#include "PMDPfunc.h"
#include "PMDsys.h"

#define PMD_VERIFYDEVICEHANDLE(handle)  \
    if( handle == NULL ) return PMD_ERR_InvalidHandle; \
    if (handle->type == PMDDeviceTypeNone) return PMD_ERR_InterfaceNotInitialized;


PMDCFunc PMDDeviceOpenPeriphCME(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
        return PMD_ERR_NotSupported;
    else
        return PMDPPRP_Open(hPeriph, hDevice, 0, 0);
}

PMDCFunc PMDDeviceOpenPeriphPRP(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam channel, PMDparam bufsize)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
        return PMD_ERR_NotSupported;
    else
        return PMDPPRP_Open(hPeriph, hDevice, channel, bufsize);
}

PMDCFunc PMDDeviceOpenPeriphSerial(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam portnum, PMDparam baud, PMDSerialParity parity, PMDSerialStopBits stopbits)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
        return PMDPCOM_Open(hPeriph, portnum, baud, parity, stopbits);
    else
        return PMDRPCOM_Open(hPeriph, hDevice, portnum, baud, parity, stopbits);
}

PMDCFunc PMDDeviceOpenPeriphSPI(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDuint8 port, PMDuint8 chipselect, PMDuint8 mode, PMDuint8 datasize, PMDparam bitrate)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
#ifdef USE_SPI_INTERFACE
        return PMDPSPI_Open(hPeriph, port, chipselect, mode, datasize, bitrate);
#else
		return PMD_ERR_NotSupported;
#endif
    else
        return PMDRPSPI_Open(hPeriph, hDevice, port, chipselect, mode, datasize, bitrate);
}

PMDCFunc PMDDeviceOpenPeriphTCP(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam ipaddress, PMDparam portnum, PMDparam timeout)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
        return PMDPTCP_Open(hPeriph, ipaddress, portnum, timeout);
    else
        return PMDRPTCP_Open(hPeriph, hDevice, ipaddress, portnum);
}

PMDCFunc PMDDeviceOpenPeriphUDP(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam ipaddress, PMDparam portnum)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
        return PMDPUDP_Open(hPeriph, ipaddress, portnum);
    else
        return PMDRPUDP_Open(hPeriph, hDevice, ipaddress, portnum);
}

PMDCFunc PMDDeviceOpenPeriphCAN(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam addressTX, PMDparam addressRX, PMDparam addressEvent)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
#ifdef USE_CAN_INTERFACE
		return PMDPCAN_Open(hPeriph, PMDCANPort1, PMDCANBaud1000000, addressTX, addressRX, addressEvent);
#else
		return PMD_ERR_NotSupported;
#endif
    else
        return PMDRPCAN_Open(hPeriph, hDevice, addressTX, addressRX, addressEvent);
}

PMDCFunc PMDDeviceOpenPeriphCANFD(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDCANPort port, PMDparam baud, PMDparam addressTX, PMDparam addressRX, PMDparam addressMode)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
#ifdef USE_CAN_INTERFACE
		return PMDPCAN_Open(hPeriph, port, baud, addressTX, addressRX, addressMode);
#else
		return PMD_ERR_NotSupported;
#endif
    else
        return PMDRPCANFD_Open(hPeriph, hDevice, port, baud, addressTX, addressRX, addressMode);
}

PMDCFunc PMDDeviceOpenPeriphPIO(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDuint16 address, PMDuint8 eventIRQ, PMDDataSize datasize)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
        return PMD_ERR_NotSupported;
    else
        return PMDRPPAR_Open(hPeriph, hDevice, address, eventIRQ, datasize);
}

PMDCFunc PMDDeviceOpenPeriphAddress(PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam periphaddress)
{
    PMDZeroMemory(hPeriph, sizeof(PMDPeriphHandle));
    if (hDevice == NULL)
        return PMD_ERR_NotSupported;
    else
        return PMDRPAddress_Open(hPeriph, hDevice, periphaddress);
}


//********************************************************
// Use this function to share an existing handle to an open port
// and set the multi-drop address and axis number
// only close the hPeriphParent when done. Do not close the hPeriph handle.
PMDCFunc PMDPeriphOpenPeriphMultiDrop(PMDPeriphHandle* hPeriph, PMDPeriphHandle* hPeriphParent, PMDparam nodeID)
{
    PMD_PERIPHCONNECTED(hPeriphParent)

    if( hPeriphParent->type != InterfaceSerial 
    &&  hPeriphParent->type != InterfaceCAN )
        return PMD_ERR_InvalidInterface;

    // copy the handle
    memcpy(hPeriph, hPeriphParent, sizeof( PMDPeriphHandle ) );

#ifdef USE_CAN_INTERFACE
    if (hPeriphParent->type == InterfaceCAN)
        return PMDPCAN_AddNodeID(hPeriph, hPeriphParent, (PMDuint8)nodeID);
	else
#endif
	{
		if (nodeID > 0x1F) // max serial port nodeID is 31 
		  return PMD_ERR_InvalidParameter;

		// store the multidrop information in the general purpose param field.
		hPeriph->param = nodeID;
		hPeriph->param |= PMDSerialProtocolMultiDropUsingIdleLineDetection << 8;
	}

    return PMD_ERR_OK;
}

//********************************************************
PMDCFunc PMDAxisOpen(PMDAxisHandle *hAxis, PMDDeviceHandle *hDevice, PMDAxis axis_number)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    // set the axis we are talking to with this handle
    PMDZeroMemory(hAxis, sizeof(PMDAxisHandle));
    hAxis->axis = axis_number;
    hAxis->transport_data = hDevice;
    hAxis->transport.SendCommand = hDevice->transport.SendCommand;

    return PMD_ERR_OK;
}

//********************************************************
PMDCFunc PMDDeviceOpenMemory(PMDMemoryHandle *hMemory, PMDDeviceHandle *hDevice, PMDDataSize datasize, PMDMemoryType memorytype)
{
    PMD_VERIFYDEVICEHANDLE(hDevice)
    if (datasize == PMDDataSize_8Bit)
        return PMD_ERR_NotSupported;

    PMDZeroMemory(hMemory, sizeof(PMDMemoryHandle));
    // if the device handle contains valid Memory functions then this must be an MP device
    // otherwise it is memory on a remote PRP device (Prodigy/CME)
    if (hDevice->transport.ReadMemory && hDevice->transport.WriteMemory && datasize == PMDDataSize_32Bit)
    {
        hMemory->transport.Read  = hDevice->transport.ReadMemory;
        hMemory->transport.Write = hDevice->transport.WriteMemory;
        hMemory->hDevice = hDevice;
        hMemory->datasize = datasize;	// todo: datasize passed in may not be compatible with default ReadMemory datasize
    }
    else
    {
        return PMDRPMemoryOpen(hMemory, hDevice, datasize, memorytype);
    }

    return PMD_ERR_OK;
}

//********************************************************
PMDCFunc PMDMailboxOpen(PMDMailboxHandle* hMailbox, PMDDeviceHandle *hDevice, PMDparam mailboxID, PMDparam depth, PMDparam itemsize)
{
    PMDZeroMemory(hMailbox, sizeof(PMDMailboxHandle));
    if (hDevice == NULL)
        return PMD_ERR_NotSupported;
    else
        return PMDRPMailboxOpen(hMailbox, hDevice, mailboxID, depth, itemsize);
}

//*******************************************************
// Return an error, rather than faulting when calling an
// MP device opened with an unsupported peripheral type.
PMDresult PMDERR_Send(void* transport_data, PMDuint8 xCt, PMDuint16* xDat, PMDuint8 rCt, PMDuint16* rDat) 
{
    return PMD_ERR_NotSupported;
}

//********************************************************
// Open a motion processor protocol device
PMDCFunc PMDPeriphOpenDeviceMP(PMDDeviceHandle *hDevice, PMDPeriphHandle *hPeriph)
{
    PMDresult result = PMD_ERR_OK;
    PMDZeroMemory(hDevice, sizeof(PMDDeviceHandle));
    PMD_PERIPHCONNECTED(hPeriph)
    hDevice->hPeriph = hPeriph;
    hDevice->type = PMDDeviceTypeMotionProcessor;

    // address is > 0 when hPeriph is a CME (RP) connected peripheral
    if (hPeriph->address > 0)
        return PMDPeriphOpenDeviceMPRemote(hDevice, hPeriph);

    // set the appropriate Send function for the interface type
    switch (hPeriph->type)
    {
    case InterfaceSerial:
        hDevice->transport.SendCommand = PMDSerial_Send;
        result = PMDSerial_Init();
        break;
#ifdef USE_SPI_INTERFACE
    case InterfaceSPI:
        hDevice->transport.SendCommand = PMDSPI_Send;
        result = PMDSPI_Init();
        break;
#endif
#ifdef USE_CAN_INTERFACE
    case InterfaceCAN:
        hDevice->transport.SendCommand = PMDCAN_Send;
        result = PMDCAN_Init();
        break;
#endif
    default:
        hDevice->transport.SendCommand = PMDERR_Send;
        return PMD_ERR_InvalidPort;
    }

    return result;
}

PMDCFunc PMDEventOpen(PMDEventHandle* hEvent, PMDEventType type, PMDparam var1, PMDparam var2, PMDparam var3)
{
    return PMD_ERR_NotSupported;
}

