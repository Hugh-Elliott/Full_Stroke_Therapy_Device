//  PMDIXXATCAN4.c -- CAN interface command/data transfer functions. The implementations
//  are for the IXXAT VCI (Virtual Can Interface) v4.x API that supports CANFD.
//
//  Performance Motion Devices, Inc.
//

#include <stdio.h>

//C-Motion includes
#include "PMDtypes.h"
#include "PMDecode.h"
// only include this if we are running in diagnostics mode
#include "PMDdiag.h"
#include "vcinpl2.h" 
#include "PMDIXXATCan.h"

#define Printf printf 

static const int DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

int BytesToDLC(int bytes)
{
    if (bytes > 48)
        return 15;
    if (bytes > 32)
        return 14;
    if (bytes > 24)
        return 13;
    if (bytes > 20)
        return 12;
    if (bytes > 16)
        return 11;
    if (bytes > 12)
        return 10;
    if (bytes > 8)
        return 9;
    return bytes;
}

//*****************************************************************************
void DisplayError(HRESULT hResult)
{
    char szError[255];

    if (hResult != VCI_SUCCESS)
    {
        if (hResult == -1)
            hResult = GetLastError();

        szError[0] = 0;
        vciFormatError(hResult, szError, sizeof(szError));
        Printf(szError);
        Printf("\n");
    }
}

//*****************************************************************************
PMDresult TransmitData(PMDCANIOTransportData* CANtransport_data, char *data, int nbytes, int timeout)
{
    PMDresult result;
    HRESULT hResult;
    CANMSG2  sCanMsg;
    int i;
    int maxbytes = 8;
    int msglength;

    do {
        hResult = canChannelReadMessage(CANtransport_data->hCanChn, 1, &sCanMsg);
    } 	while (hResult == VCI_SUCCESS); //VCI_E_RXQUEUE_EMPTY | VCI_E_TIMEOUT);

    ZeroMemory(&sCanMsg, sizeof(sCanMsg));

    sCanMsg.dwTime   = 0;
    sCanMsg.dwMsgId  = CANtransport_data->txID;    // CAN message identifier

    sCanMsg.uMsgInfo.Bytes.bType  = CAN_MSGTYPE_DATA;
    sCanMsg.uMsgInfo.Bytes.bFlags = CAN_MAKE_MSGFLAGS(8,0,0,0,0);
    if (CANtransport_data->FDMode & CAN_EXMODE_EXTDATA)
    {
        sCanMsg.uMsgInfo.Bits.edl = 1;
        maxbytes = 64;
    }
    if (CANtransport_data->FDMode & CAN_EXMODE_FASTDATA)
        sCanMsg.uMsgInfo.Bits.fdr = 1;

    do
    {
        if (nbytes > maxbytes)
            msglength = maxbytes;
        else
            msglength = nbytes;

        for (i = 0; i < msglength; i++ )
        {
            sCanMsg.abData[i] = data[i];
        }

        sCanMsg.uMsgInfo.Bits.dlc = BytesToDLC(msglength);

        // write the CAN message into the transmit FIFO
        hResult = canChannelSendMessage(CANtransport_data->hCanChn, timeout, &sCanMsg);
        nbytes -= maxbytes;
        data += maxbytes;
    }
    while (nbytes > 0 && hResult == VCI_SUCCESS);

    DisplayError(hResult);
    if (hResult == VCI_SUCCESS)
        result = PMD_ERR_OK;
    else if (hResult == VCI_E_TIMEOUT)
        result = PMD_ERR_Timeout;
    else
        result = PMD_ERR_CommunicationsError; 

    return result;
}

//*****************************************************************************
PMDresult ReceiveData( PMDCANIOTransportData* CANtransport_data, char *data, int* nBytesReceived, int nBytesExpected, int timeout)
{
    PMDresult result;
    HRESULT hResult;
    CANMSG2  sCanMsg;
    CANLINESTATUS2  sCanLineStatus;
    UINT8  bType = CAN_MSGTYPE_DATA;
    int  DLC;
    int BytesReceived;

    *nBytesReceived = 0;
    
    do
    {
        // read a CAN message from the receive FIFO
        hResult = canChannelReadMessage(CANtransport_data->hCanChn, timeout, &sCanMsg);

        if (hResult == VCI_SUCCESS)
        {
            bType = sCanMsg.uMsgInfo.Bytes.bType;
            if (bType == CAN_MSGTYPE_DATA)
            {
                //
                // show data frames
                //
                if (sCanMsg.uMsgInfo.Bits.rtr == 0)
                {
                    UINT8 j;
/*
                    Printf("\nTime: %10u  ID: %3X  DLC: %1u  Data:",
                                    sCanMsg.dwTime,
                                    sCanMsg.dwMsgId,
                                    sCanMsg.uMsgInfo.Bits.dlc);
*/
                    // copy data bytes
                    DLC = sCanMsg.uMsgInfo.Bits.dlc;
                    BytesReceived = DLCtoBytes[DLC];
                    *nBytesReceived += BytesReceived;
                    for (j = 0; j < BytesReceived; j++)
                    {
//                      Printf(" %.2X", sCanMsg.abData[j]);
                        data[j] = sCanMsg.abData[j];
                    }
                    data += BytesReceived;
                }
/*
                else
                {
                    Printf("\nTime: %10u ID: %3X  DLC: %1u  Remote Frame",
                                 sCanMsg.dwTime,
                                 sCanMsg.dwMsgId,
                                 sCanMsg.uMsgInfo.Bits.dlc);
                }
*/
            }
            else if (bType == CAN_MSGTYPE_INFO)
            {
                //
                // show informational frames
                //
                switch (sCanMsg.abData[0])
                {
                    case CAN_INFO_START: Printf("CAN started..."); break;
                    case CAN_INFO_STOP : Printf("CAN stopped..."); break;
                    case CAN_INFO_RESET: Printf("CAN reset..."); break;
                }
                Printf("\r\n");
            }
            else if (bType == CAN_MSGTYPE_ERROR)
            {
                //
                // show error frames
                //
                switch (sCanMsg.abData[0])
                {
                    case CAN_ERROR_STUFF: Printf("stuff error...");                  break; 
                    case CAN_ERROR_FORM : Printf("form error...");                   break; 
                    case CAN_ERROR_ACK  : Printf("acknowledgement error...");        break;
                    case CAN_ERROR_BIT  : Printf("bit error...");                    break; 
                    case CAN_ERROR_CRC  : Printf("CRC error...");                    break; 
                    case CAN_ERROR_OTHER:
                    default             : Printf("other error...");                  break;
                }
                Printf("\r\n");
            }
        }
    }
    while ((hResult == VCI_SUCCESS && (*nBytesReceived < nBytesExpected || bType != CAN_MSGTYPE_DATA)));


    if (hResult == VCI_SUCCESS)
        result = PMD_ERR_OK;
    else if (hResult == VCI_E_TIMEOUT || hResult == VCI_E_RXQUEUE_EMPTY)
        result = PMD_ERR_Timeout;
    else
    {
        result = PMD_ERR_CommunicationsError; 
        DisplayError(hResult);
    }
    if (hResult != VCI_SUCCESS)
    {
        hResult = canControlGetStatus(CANtransport_data->hCanCtl, &sCanLineStatus);
    //    hResult = canChannelGetStatus(CANtransport_data->hCanChn, &sCanLineStatus);
        if (hResult == VCI_SUCCESS)
        {
            if (sCanLineStatus.dwStatus & CAN_STATUS_BUSOFF)
                result = PMD_ERR_CAN_BusOff;
            if (sCanLineStatus.dwStatus & CAN_STATUS_BUSCERR)
                result = PMD_ERR_CAN_BusOff;
        }
    }

    return result;
}


/*****************************************************************************
 Function:
    SelectDevice

 Description:
    Selects the first CAN adapter.

 Arguments:
    fUserSelect -> If this parameter is set to TRUE the functions display
                   a dialog box which allows the user to select the device.

 Results:
    none
*****************************************************************************/
HRESULT SelectDevice( PMDCANIOTransportData* CANtransport_data, BOOL fUserSelect )
{
    HRESULT hResult; // error code
    int count = 0;
    HANDLE hDevice = NULL;

    if (fUserSelect == FALSE)
    {
        HANDLE hEnum;   // enumerator handle
        VCIDEVICEINFO sInfo;     // device info

        //
        // open the device list
        //
        hResult = vciEnumDeviceOpen(&hEnum);

        //
        // retrieve information about the first
        // device within the device list
        //
        while (hResult == VCI_SUCCESS)
        {
            hResult = vciEnumDeviceNext(hEnum, &sInfo);
            if (hResult == VCI_SUCCESS)
                count++;
        }

        //
        // close the device list (no longer needed)
        //
        vciEnumDeviceClose(hEnum);

        if (hResult == VCI_E_NO_MORE_ITEMS)
        {
            // if more than one device is found than display the selection dialog
            if (count > 1)
                hResult = vciDeviceOpenDlg(NULL, &hDevice);
                //hResult = SocketSelectDlg(NULL, VCI_BUS_CAN, &hDevice, &lCtrlNo);
            // otherwise just open the one device
            else 
                hResult = vciDeviceOpen(&sInfo.VciObjectId, &hDevice);
        }
    }
    else
    {
        //
        // open a device selected by the user
        //
        hResult = vciDeviceOpenDlg(NULL, &hDevice);
    }

    CANtransport_data->hDevice = hDevice;

    DisplayError(hResult);

    return hResult;
}

/*************************************************************************
 Function: 
    InitSocket

 Description : 
    Opens the specified socket, creates a message channel, initializes
    and starts the CAN controller.

 Arguments: 
    dwCanNo -> Number of the CAN controller to open.

 Results:
    none

 Remarks:
    If <dwCanNo> is set to 0xFFFFFFFF, the function shows a dialog box
    which allows the user to select the VCI device and CAN controller.
*************************************************************************/
HRESULT InitSocket(PMDCANIOTransportData* CANtransport_data, UINT32 dwCanNo)
{
    HRESULT hResult;
    int fExclusive = FALSE;

    //
    // create a message channel with exclusive use of CAN connection
    //
    hResult = canChannelOpen(CANtransport_data->hDevice, dwCanNo, fExclusive, &CANtransport_data->hCanChn);

    //
    // initialize the message channel
    //
    if (hResult == VCI_SUCCESS)
    {
        UINT16 wRxFifoSize  = 100;  // size of receive FIFO in messages
        UINT16 wRxThreshold = 1;    // threshold for receive FIFO event
        UINT16 wTxFifoSize  = 10;   // size of transmit FIFO in messages
        UINT16 wTxThreshold = 1;    // threshold for transmit FIFO event
        UINT16 wFilterSize  = 8;    // filter size (number of IDs)
        UINT8 wFilterMode   = CAN_FILTER_INCL;  //|CAN_FILTER_SRRA);

        // inclusive filtering (pass registered IDs) and pass self-rec messages from all channels
        hResult = canChannelInitialize( CANtransport_data->hCanChn, wRxFifoSize, wRxThreshold, wTxFifoSize, wTxThreshold,  wFilterSize, wFilterMode);
    }
    //
    // setup channel specific acceptance filter for 11-bit IDs
    //
    if (hResult == VCI_OK)
    {
        BOOL fExtend = FALSE; //select standard filter (11-bit)
        hResult = canChannelSetAccFilter(CANtransport_data->hCanChn, fExtend, CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);
    }

    //
    // activate the CAN channel
    //
    if (hResult == VCI_SUCCESS)
    {
        hResult = canChannelActivate(CANtransport_data->hCanChn, TRUE);
    }

    //
    // open the CAN controller
    //
    if (hResult == VCI_SUCCESS)
    {
        hResult = canControlOpen(CANtransport_data->hDevice, dwCanNo, &CANtransport_data->hCanCtl);
        // this function fails if the controller is in use
        // by another application.

    }

    DisplayError(hResult);

    return hResult;
}

const CANBTP CAN_BITRATE_NONE   = CAN_BTP_EMPTY;
const CANBTP CAN_BITRATE_10KB   = CAN_BTP_10KB;
const CANBTP CAN_BITRATE_20KB   = CAN_BTP_20KB;
const CANBTP CAN_BITRATE_50KB   = CAN_BTP_50KB;
const CANBTP CAN_BITRATE_100KB  = CAN_BTP_100KB;
const CANBTP CAN_BITRATE_125KB  = CAN_BTP_125KB;
const CANBTP CAN_BITRATE_250KB  = CAN_BTP_250KB;
const CANBTP CAN_BITRATE_500KB  = CAN_BTP_500KB;
const CANBTP CAN_BITRATE_800KB  = CAN_BTP_800KB;
const CANBTP CAN_BITRATE_1000KB = CAN_BTP_1000KB;
const CANBTP CAN_BR_250KB   = CAN_BTP_ABR_LL_250KB;
const CANBTP CAN_BR_500KB   = CAN_BTP_ABR_SL_500KB;
const CANBTP CAN_BR_1000KB  = CAN_BTP_DBR_SL_1000KB;
const CANBTP CAN_BR_2000KB  = CAN_BTP_DBR_SL_2000KB;
const CANBTP CAN_BR_4000KB  = CAN_BTP_DBR_SL_4000KB;
const CANBTP CAN_BR_5000KB  = CAN_BTP_DBR_SL_5000KB;
const CANBTP CAN_BR_6667KB  = CAN_BTP_DBR_SL_6667KB;
const CANBTP CAN_BR_8000KB  = CAN_BTP_DBR_SL_8000KB;
const CANBTP CAN_BR_10000KB = CAN_BTP_DBR_SL_10000KB;

//*****************************************************************************
HRESULT InitController(PMDCANIOTransportData* CANtransport_data)
{
    HRESULT hResult;
    CANCAPABILITIES2 sCanCaps;    // capabilities of selected CAN controller
    int nBaud = CANtransport_data->baudrate & 0x0F;
    int nDataBaud = CANtransport_data->baudrate >> 4 & 0x0F;
    HANDLE hCanCtl = CANtransport_data->hCanCtl;
    CANBTP ArbitrationBitrate;
    CANBTP DataBitrate;
    UINT8  bOpMode = CAN_OPMODE_STANDARD;
    UINT8  bExMode = CAN_EXMODE_DISABLED;
    UINT8   bSFMode = CAN_FILTER_PASS;                   // mode for line specific 11-bit ID filter - inclusive filtering (pass registered IDs)
    UINT8   bEFMode = CAN_FILTER_LOCK;                   // mode for line specific 29-bit ID filter - lock filter (inhibit all IDs)
    UINT32  dwSFIds = 1;                                 // size of line specific 11-bit ID filter
    UINT32  dwEFIds = 0;                                 // size of line specific 29-bit ID filter

    //
    // initialize the CAN controller
    //
    if     (nBaud==PMDCANBaud10000  ) ArbitrationBitrate = CAN_BITRATE_10KB  ;
    else if(nBaud==PMDCANBaud20000  ) ArbitrationBitrate = CAN_BITRATE_20KB  ;
    else if(nBaud==PMDCANBaud50000  ) ArbitrationBitrate = CAN_BITRATE_50KB  ;
    else if(nBaud==PMDCANBaud125000 ) ArbitrationBitrate = CAN_BITRATE_125KB ;
    else if(nBaud==PMDCANBaud250000 ) ArbitrationBitrate = CAN_BITRATE_250KB ;
    else if(nBaud==PMDCANBaud500000 ) ArbitrationBitrate = CAN_BITRATE_500KB ;
    else if(nBaud==PMDCANBaud800000 ) ArbitrationBitrate = CAN_BITRATE_800KB ;
    else if(nBaud==PMDCANBaud1000000) ArbitrationBitrate = CAN_BITRATE_1000KB;
    else return  VCI_E_INVALIDARG;

    DataBitrate = CAN_BITRATE_NONE ;
    if     (nDataBaud != PMDCANFDDataBaudNone)
    {
        if     (nDataBaud==PMDCANFDDataBaud1000000) DataBitrate = CAN_BR_1000KB;
        else if(nDataBaud==PMDCANFDDataBaud2000000) DataBitrate = CAN_BR_2000KB;
        else if(nDataBaud==PMDCANFDDataBaud4000000) DataBitrate = CAN_BR_4000KB;
        else if(nDataBaud==PMDCANFDDataBaud5000000) DataBitrate = CAN_BR_5000KB;
        else return  VCI_E_INVALIDARG;

        if     (nBaud==PMDCANBaud250000 ) ArbitrationBitrate = CAN_BR_250KB ;
        else if(nBaud==PMDCANBaud500000 ) ArbitrationBitrate = CAN_BR_500KB ;

    }
    hResult = canControlReset(hCanCtl);

    //
    //  request the capabilities of the selected CAN controller
    //
    if (hResult == VCI_OK)
    {
        hResult = canControlGetCaps(hCanCtl, &sCanCaps);
    }

    //
    // initialize the selected CAN controller
    //
    if (hResult == VCI_SUCCESS)
    {
        // enable use of extended data length only if controller supports it
        if (sCanCaps.dwFeatures & CAN_FEATURE_EXTDATA)
        {
            // controller supports extended data length
            bExMode |= CAN_EXMODE_EXTDATA;
        }

        // enable use of fast data bitrate only if controller supports it
        if (sCanCaps.dwFeatures & CAN_FEATURE_FASTDATA)
        {
            // controller supports fast data bit rate
            bExMode |= CAN_EXMODE_FASTDATA;
        }

        if (nDataBaud == 0) 
            bExMode = CAN_EXMODE_DISABLED;

        hResult = canControlInitialize(hCanCtl, 
          bOpMode,
          bExMode, 
          bSFMode,
          bEFMode,
          dwSFIds,
          dwEFIds,
          &ArbitrationBitrate,                   // arbitration bitrate
          &DataBitrate);                         // data bitrate
    }

    CANtransport_data->FDMode = bExMode;

    if (hResult == VCI_SUCCESS)
    { 
        BOOL fExtend = FALSE; //select standard filter (11-bit)
        hResult = canControlSetAccFilter( hCanCtl, fExtend, CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);
        hResult = canControlSetAccFilter( hCanCtl, TRUE, CAN_ACC_CODE_NONE, CAN_ACC_MASK_NONE);
    }

    //
    // start the CAN controller
    //
    if (hResult == VCI_SUCCESS)
    {
        hResult = canControlStart(hCanCtl, TRUE);
    }

    DisplayError(hResult);

    return hResult;
}

//*****************************************************************************
HRESULT AddFilter(PMDCANIOTransportData* CANtransport_data)
{
    HRESULT hResult;
    HANDLE hCanCtl = CANtransport_data->hCanCtl;

    // no need to do this because the acceptance filter is currently set to accept all IDs rendering the ID list ineffective.
    return VCI_SUCCESS;
    //
    // stop the CAN controller 
    //
    hResult = canControlStart(hCanCtl, FALSE);

    //
    // add an acceptance filter
    //
    if (hResult == VCI_SUCCESS)
    { 
        UINT32 bRTR = 0;
        BOOL b29bit = FALSE;
        // RTR bit is LSB so shift everything left by 1
        UINT32 nAcceptanceCode = ((CANtransport_data->rxMask) << 1) + bRTR;
        UINT32 nAcceptanceMask = CAN_ACC_MASK_NONE;
        hResult = canControlAddFilterIds( hCanCtl, b29bit, nAcceptanceCode, nAcceptanceMask);
    }

    //
    // start the CAN controller
    //
    if (hResult == VCI_SUCCESS)
    {
        hResult = canControlStart(hCanCtl, TRUE);
    }

    DisplayError(hResult);

    return hResult;
}

//*****************************************************************************
HRESULT RemoveFilter(PMDCANIOTransportData* CANtransport_data)
{
    HRESULT hResult;
    HANDLE hCanCtl = CANtransport_data->hCanCtl;

    //
    // stop the CAN controller
    //
    hResult = canControlStart(hCanCtl, FALSE);

    //
    // add an acceptance filter
    //
    if (hResult == VCI_SUCCESS)
    { 
        UINT32 bRTR = 0;
        BOOL b29bit = FALSE;
        // RTR bit is LSB so shift everything left by 1
        UINT32 nAcceptanceCode = ((CANtransport_data->rxMask) << 1) + bRTR;
        UINT32 nAcceptanceMask = CAN_ACC_MASK_NONE;
        hResult = canControlRemFilterIds( hCanCtl, b29bit, nAcceptanceCode, nAcceptanceMask);
    }

    //
    // start the CAN controller
    //
    if (hResult == VCI_SUCCESS)
    {
        hResult = canControlStart(hCanCtl, TRUE);
    }

    DisplayError(hResult);

    return hResult;
}

//*****************************************************************************
PMDresult PMDCAN_FlushBuffers(PMDCANIOTransportData* CANtransport_data)
{
    HRESULT hResult;
    HANDLE hCanCtl = CANtransport_data->hCanCtl;
    char rbuff[10];
    int rbytes;

    do 
    {
        hResult = ReceiveData(CANtransport_data, rbuff, &rbytes, 8, 0);
    } 
    while (hResult == VCI_SUCCESS); //VCI_E_RXQUEUE_EMPTY | VCI_E_TIMEOUT);


    return PMD_ERR_OK;
}

//*****************************************************************************
PMDresult Open( PMDCANIOTransportData* CANtransport_data )
{
    DWORD dwCanNo = 0; // CAN card number
    UINT32 dwMajorVersion;
    UINT32 dwMinorVersion;
    UINT32 dwRevNumber;
    UINT32 dwBuildNumber;
    HRESULT hResult;

    hResult = vciGetVersion(&dwMajorVersion, &dwMinorVersion, &dwRevNumber, &dwBuildNumber);
    if (VCI_SUCCESS == hResult)
    {
        Printf("IXXAT driver version: %ld.%ld\r\n", dwMajorVersion, dwMinorVersion );

        hResult = SelectDevice(CANtransport_data, FALSE);
        if (VCI_SUCCESS == hResult)
        {
            hResult = InitSocket(CANtransport_data, dwCanNo);
            if (VCI_SUCCESS == hResult)
            {
                hResult = InitController(CANtransport_data);
                if (VCI_SUCCESS == hResult)
                {
//                    hResult = AddFilter(CANtransport_data);
//                    if (VCI_SUCCESS == hResult)
                    {
                        VCIDEVICEINFO info;
                        vciDeviceGetInfo( CANtransport_data->hDevice, &info);
                        //FlushReceive();
                        return PMD_ERR_OK;
                    }
                }
            }
        }
    }
    if (VCI_SUCCESS != hResult)
    {
        DisplayError(hResult);
    }

    return PMD_ERR_OpeningPort;
}


// ------------------------------------------------------------------------
PMDresult PMDCAN_Close(void* transport_data)
{
    PMDCANIOTransportData* CANtransport_data = (PMDCANIOTransportData*)transport_data;

    if (CANtransport_data != NULL)
    {
        HRESULT hResult;

        // close all open handles
        // if hDevice is not NULL it means this is the parent handle so we can close it 
        if (CANtransport_data->hDevice)
        {
            if (CANtransport_data->hCanChn)
                hResult = canChannelClose(CANtransport_data->hCanChn);
            if (CANtransport_data->hCanCtl)
                hResult = canControlClose(CANtransport_data->hCanCtl);
            if (CANtransport_data->hDevice)
                hResult = vciDeviceClose(CANtransport_data->hDevice);
        }
        free(CANtransport_data);
    }
    return PMD_ERR_OK;
}


// ------------------------------------------------------------------------
// PMDCAN_InitPort
// Initializes CAN hardware and interface.
PMDresult PMDCAN_InitPort(PMDCANIOTransportData* CANtransport_data)
{
    return Open( CANtransport_data );
}

// ------------------------------------------------------------------------
// set the nodeID for this handle
PMDresult PMDCAN_SetNodeID(PMDCANIOTransportData* CANtransport_data, PMDuint8 nodeID)
{
    HRESULT hResult;

    CANtransport_data->txID = CAN_ADDRESS_BASE_TX + nodeID;
    CANtransport_data->rxMask = CAN_ADDRESS_BASE_RX + nodeID;
    CANtransport_data->rxIntMask = CAN_ADDRESS_BASE_INTR + nodeID;
    // clear the device handles to prevent them from being closed when closing this shared handle.
    // they will be closed when the parent is closed.
    CANtransport_data->hDevice = NULL;

    hResult = AddFilter(CANtransport_data); // add the new nodeID to the receive filter
    if (VCI_SUCCESS == hResult)
        return PMD_ERR_OK;

    return PMD_ERR_ConfiguringPort;
}

// ------------------------------------------------------------------------
// change the baud
PMDresult PMDCAN_SetBaud(PMDCANIOTransportData* CANtransport_data, PMDuint8 baud)
{
    CANtransport_data->baudrate = baud;

    InitController(CANtransport_data);
    PMDCAN_FlushBuffers(CANtransport_data);

    return PMD_ERR_OK;
}
