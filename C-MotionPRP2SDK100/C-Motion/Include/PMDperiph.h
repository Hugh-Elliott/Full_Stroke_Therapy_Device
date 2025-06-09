#ifndef _DEF_INC_PMDperiph
#define _DEF_INC_PMDperiph

//
//  "Base Class" definitions for PeriphTransport
//
//  Performance Motion Devices, Inc.
//

#include "PMDtypes.h"
#include "PMDdevice.h"

#define PMD_INVALID_HANDLE      (void*)(0)
#define PMD_CONNECTED_HANDLE    (void*)(1)
#define PMD_WAITFOREVER         (PMDparam)0xFFFFFFFFUL

#define PMD_CANID_MODE_MASK     (PMDparam)0x80000001UL


#define PMD_VERIFYHANDLE(handle)        if( handle == NULL ) return PMD_ERR_InvalidHandle;
#define PMD_PERIPHCONNECTED(hPeriph)    if( hPeriph == NULL ) return PMD_ERR_InterfaceNotInitialized; \
                                        if( hPeriph->handle == PMD_INVALID_HANDLE ) return PMD_ERR_NotConnected;
// zero the handle structure so that a call to a function with an uninitialized handle won't cause problems.
#define PMD_INITIALIZEHANDLE(handle)    memset(&handle, 0, sizeof(handle));

#define PMD_IP4_ADDR(o1,o2,o3,o4) (((PMDuint32)((o1) & 0xff) << 24) | \
                                   ((PMDuint32)((o2) & 0xff) << 16) | \
                                   ((PMDuint32)((o3) & 0xff) << 8)  | \
                                    (PMDuint32)((o4) & 0xff) << 0)


#if defined(__cplusplus)
extern "C" {
#endif

    // Legacy names
#define PMDPeriphOpenCAN            PMDDeviceOpenPeriphCAN
#define PMDPeriphOpenCANFD          PMDDeviceOpenPeriphCANFD
#define PMDPeriphOpenCANNodeID      PMDDeviceOpenPeriphCANNodeID
#define PMDPeriphOpenCME            PMDDeviceOpenPeriphCME
#define PMDPeriphOpenCOM            PMDDeviceOpenPeriphSerial
#define PMDPeriphOpenPIO            PMDDeviceOpenPeriphPIO
#define PMDPeriphOpenPRP            PMDDeviceOpenPeriphPRP
#define PMDPeriphOpenSPI            PMDDeviceOpenPeriphSPI
#define PMDPeriphOpenTCP            PMDDeviceOpenPeriphTCP
#define PMDPeriphOpenUDP            PMDDeviceOpenPeriphUDP
#define PMDPeriphOpenMultiDrop      PMDPeriphOpenPeriphMultiDrop    


// open C-Motion Engine User Packet legacy 'peripheral'
PMDCFunc PMDDeviceOpenPeriphCME         (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice);

/*
  PMDDeviceOpenPeriphPRP

  Open a handle to a PRP peripheral channel that can send and receive user data via a PRP connection.

  channel      The channel to send and receive data to and from user code. 
               Channel #0 is reserved.
               Channel #1 is reserved for the console IO.
*/ 
PMDCFunc PMDDeviceOpenPeriphPRP         (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam channel, PMDparam bufsize);
PMDCFunc PMDDeviceOpenPeriphSPI         (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDuint8 port, PMDuint8 chipselect, PMDuint8 mode, PMDuint8 datasize, PMDparam bitrate);
PMDCFunc PMDDeviceOpenPeriphSerial      (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam port, PMDparam baud, PMDSerialParity parity, PMDSerialStopBits stopbits);

/*
  PMDDeviceOpenPeriphCAN

  Open a handle to the Expansion CAN port at the default or current baud rate. This is a legacy function normally used to communicate to Magellan motion ICs.

  addressTX     The address to send data to. 0-7FF                          (0x600 + nodeID)
  addressRX     The address to accept data from. 0-7FF                      (0x580 + nodeID)
  addressEvent  The address to accept asynchronous event data from. 0-7FF   (0x180 + nodeID)
*/
PMDCFunc PMDDeviceOpenPeriphCAN         (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam addressTX, PMDparam addressRX, PMDparam addressEvent);

/*
  PMDDeviceOpenPeriphCANFD

  Open a handle to the Expansion CAN or HostCAN port at the specified baud rate. 

  baud          The desired bit rate. It is one of PMDCANBaud for standard CAN. When using CANFD the data bit rate is specified in the 2nd nibble. 
                baud = (PMDCANDataBuad << 4) | PMDCANBaud
                Warning: If the CAN port is currently open with a different baud it will be changed to the baud specified here.
                A baud value of 0xFF can be specified to use the current/default baud.

  addressmode   One of PMDCANMode (PMDCANMode_StdID      PMDCANMode_StdMask      PMDCANMode_StdRange)

  When addressmode is PMDCANMode_StdID
  addressTX     The address to send data to when addressmode = PMDCANMode_StdID 
                The address mask to receive data from when addressmode = PMDCANMode_StdMask 
                The minimum address to receive data from when addressmode = PMDCANMode_StdRange 

*/

PMDCFunc PMDDeviceOpenPeriphCANFD       (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDCANPort port, PMDparam baud, PMDparam addressTX, PMDparam addressRX, PMDparam addressmode);

/*
  PMDDeviceOpenPeriphCANNodeID

  Wrapper function to open a handle to a PMD CAN device at nodeID.
  Same as PMDDeviceOpenPeriphCANFD except:

  baud          The desired bit rate. It is one of PMDCANBaud for standard CAN. When using CANFD the data bit rate is specified in the 2nd nibble. 
                baud = (PMDCANDataBuad << 4) | PMDCANBaud
                Warning: If the CAN port is currently open with a different baud it will be changed to the baud specified here.
  nodeID        The nodeID of the device. 0-127
                CAN_ADDRESS_BASE_TX         0x600   a CAN PMD device listens for commands at CAN address of 0x600 + nodeID
                CAN_ADDRESS_BASE_RX         0x580   a CAN PMD device sends command responses to CAN address of 0x580 + nodeID
                CAN_ADDRESS_BASE_INT        0x180   a CAN PMD device sends asynchronous events (see SetInterruptMask) to CAN address of 0x180 + nodeID
*/
PMDCFunc PMDDeviceOpenPeriphCANNodeID   (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam baud, PMDparam nodeID);

/*
  PMDDeviceOpenPeriphTCP

  Open a peripheral handle to a TCP port
*/
PMDCFunc PMDDeviceOpenPeriphTCP         (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam ipv4address, PMDparam portnum, PMDparam timeout);

/*
  PMDDeviceOpenPeriphUDP

  Open a peripheral handle to a UDP port
*/
PMDCFunc PMDDeviceOpenPeriphUDP         (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam ipv4address, PMDparam portnum);

/*
  PMDDeviceOpenPeriphPIO

  Open a peripheral handle to the internal peripheral IO port
*/
PMDCFunc PMDDeviceOpenPeriphPIO         (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDuint16 address, PMDuint8 eventIRQ, PMDDataSize datasize);

/*
  PMDDeviceOpenPeriphAddress

  Open a handle to an existing handle already opened on the device.
*/
PMDCFunc PMDDeviceOpenPeriphAddress     (PMDPeriphHandle* hPeriph, PMDDeviceHandle *hDevice, PMDparam periphaddress);

/*
  PMDPeriphOpenPeriphMultiDrop 

  Creates a handle to a multidrop (RS485) serial port peripheral for communicating with PMD products via RS485.
  Use the hPeriph handle for all communications and only close the hPeriphParent when done.
  hPeriphParent is a handle to an open serial port peripheral from a call to PMDDeviceOpenPeriphSerial.
*/
PMDCFunc PMDPeriphOpenPeriphMultiDrop   (PMDPeriphHandle* hPeriph, PMDPeriphHandle* hPeriphParent, PMDparam address);

// for peripherals that can receive asynchronous events, ReceiveEvent will wait until the event is received or timeout has elapsed
PMDCFunc PMDPeriphReceiveEvent    (PMDPeriphHandle* hPeriph, void *data, PMDparam *pnReceived, PMDparam nExpected, PMDparam timeoutms);

PMDCFunc PMDPeriphReceive         (PMDPeriphHandle* hPeriph, void *data, PMDparam *pnReceived, PMDparam nExpected, PMDparam timeoutms);

PMDCFunc PMDPeriphSend            (PMDPeriphHandle* hPeriph, const void *data, PMDparam nCount, PMDparam timeoutms);

/*
  PMDPeriphRead and PMDPeriphWrite           

  For reading and writing parallel data via PCI or ISA only
*/
PMDCFunc PMDPeriphRead            (PMDPeriphHandle* hPeriph, void *data, PMDparam offset, PMDparam length);
PMDCFunc PMDPeriphWrite           (PMDPeriphHandle* hPeriph, void *data, PMDparam offset, PMDparam length);

void      PMDPeriphOut            (PMDPeriphHandle* hPeriph, PMDparam offset, PMDparam data);
PMDparam  PMDPeriphIn             (PMDPeriphHandle* hPeriph, PMDparam offset);

PMDCFunc PMDPeriphClose           (PMDPeriphHandle* hPeriph);
PMDCFunc PMDPeriphFlush           (PMDPeriphHandle* hPeriph);


#if defined(__cplusplus)
}
#endif

#endif
