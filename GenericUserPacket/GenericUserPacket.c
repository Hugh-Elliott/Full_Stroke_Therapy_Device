//
// GenericUserPacket.c :  

// This example code runs on a host PC connected to a CME Device. There are two methods for sending/receiving user packets.  
// The PRP user packet method transmits the user packet via a PMD Resource Protocol connection.
// The other method sends a raw packet over it's own interface connection.
// The advantage to this method is that it does not include the protocol overhead of the CME method.
// This example will open a port and wait to receive any data from the host and send it back.

// TO DO: Select the interface type you are using by uncommenting the appropriate PMDPeriphOpen??? function.

#ifndef USE_SPIMASTER

#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDperiph.h"
#include "PMDdevice.h"
#include "PMDsys.h"

#define BUFSIZE   20 
#define MAJOR_VERSION 1
#define MINOR_VERSION 1

USER_CODE_VERSION(MAJOR_VERSION, MINOR_VERSION)



USER_CODE_TASK(GenericUserPacket)
{
    // TO DO: Select the interface type.
    PMDInterfaceType Interface = InterfaceSerial;
    PMDPeriphHandle hPeriph = { 0 };
    PMDPeriphHandle hPeriphRx = { 0 };
    PMDPeriphHandle hPeriphPRP = { 0 };
    PMDDeviceHandle hDevice = { 0 };
    PMDuint32 vmajor, vminor; 
    PMDuint8 data[BUFSIZE] = { 1,2,3,4 };
    PMDuint32 bytesToSend = 4;
    PMDuint32 bytesExpected;
    PMDuint32 bytesReceived;
    PMDuint32 i, j = 0;
    PMDresult result;

    PMDprintf("\n\n*** Begin GenericUserPacket.c ***\n");
    PMDGetCMotionVersion( &vmajor, &vminor );
    PMDprintf("C-Motion Version %d.%d \n", vmajor, vminor);

    // To do: select the desired interface 
    PMDprintf("Attempting to open the communications port\n");

    switch (Interface)
    {
    case InterfaceSerial:
    {
        PMDSerialPort serialport = PMDSerialPort1;
        PMDprintf("Opening serial port COM%d\n", serialport + 1);
        PMD_ABORTONERROR(PMDDeviceOpenPeriphSerial(&hPeriph, NULL, serialport, PMDSerialBaud115200, PMDSerialParityNone, PMDSerialStopBits1));
        break;
    }
    case InterfaceCAN:
    {
        // open a CAN connection. The CAN Tx and Rx addresses must be opposite of the addresses used in the CME code.
        PMDparam addressTX = 0x100;
        PMDparam addressRX = 0x200;
        PMDprintf("Opening a CAN port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphCAN(&hPeriph, NULL, addressTX, addressRX, 0));
        break;
    }
    case InterfaceTCP:
    {
        PMDuint32 IPaddress = PMD_IP4_ADDR(192, 168, 2, 2);
        PMDuint32 portnum = 49152;
        PMDprintf("Opening a TCP port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphTCP(&hPeriph, NULL, IPaddress, portnum, 1000));
        break;
    }
    case InterfaceUDP:
    {
        PMDuint32 IPaddress = PMD_IP4_ADDR(192, 168, 2, 2);
        PMDuint32 portnum = 49152;
        PMDprintf("Opening a UDP port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphUDP(&hPeriph, NULL, IPaddress, portnum));
        // UDP is unidirectional. An additional peripheral handle can be opened for return traffic.
//        PMD_ABORTONERROR(PMDDeviceOpenPeriphUDP(&hPeriphRx, NULL, 0, portnum));
        break;
    }
    case InterfaceSPI:
    {
        PMDuint8 port = 0;
        PMDuint8 chipselect = 1;
        PMDuint8 mode = 0;
        PMDuint8 datasize = 16;
        PMDparam bitrate = PMDSPIBitRate_625kHz;
        PMDprintf("Opening the SPI port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphSPI(&hPeriph, NULL, port, chipselect, mode, datasize, bitrate));
        break;
    }
    case InterfacePRP:
    {
        PMDSerialPort serialport = PMDSerialPort1;
        PMDuint32 PRPchannel = 2;
        PMDprintf("Opening a serial port COM%d for PRP\n", serialport + 1);
        // For sending user packets via PRP we need to first make a PRP connection to the device via any supported interface.
        PMD_ABORTONERROR(PMDDeviceOpenPeriphSerial(&hPeriphPRP, NULL, serialport, PMDSerialBaud57600, PMDSerialParityNone, PMDSerialStopBits1));
        // Open a handle to a PRP device
        PMD_ABORTONERROR(PMDPeriphOpenDevicePRP(&hDevice, &hPeriphPRP));
        // Available PRP channels start at 2. The bufsize parameter is only used by CME code.
        PMDprintf("Opening a PRP channel #%d\n", PRPchannel);
        PMD_ABORTONERROR(PMDDeviceOpenPeriphPRP(&hPeriph, &hDevice, PRPchannel, 0));
        break;
    }
    default:
        PMDTaskAbort(1);
    }

    PMD_RESULT(PMDPeriphSend(&hPeriph, &data, bytesToSend, 5000))
    bytesExpected = bytesToSend;

    // For SPI need to send to receive.
    if (Interface == InterfaceSPI)
        PMD_RESULT(PMDPeriphSend(&hPeriph, &data, bytesToSend, 0))

    j = 0;
    while (j++ < 10)
    {
        PMD_RESULT(PMDPeriphReceive(&hPeriph, &data, &bytesReceived, bytesExpected, 5000));

        if (result == PMD_ERR_OK || result == PMD_ERR_Timeout)
        {
            if (bytesReceived > 0)
            {
                PMDprintf("%d bytes received: ", bytesReceived);
                for (i = 0; i < bytesReceived; i++)
                {
                    PMDprintf("%02x ", data[i]);
                    data[i]++; // modify the data to send back to the host
                }
                PMDprintf("Sending the received data\n");
                PMD_RESULT(PMDPeriphSend(&hPeriph, &data, bytesReceived, 5000));
            }
        }
    }

    PMDprintf("Done\n");
    PMDPeriphClose(&hPeriph);
    PMDPeriphClose(&hPeriphRx);
    PMDPeriphClose(&hPeriphPRP);
}

#endif // !USE_SPIMASTER