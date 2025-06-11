//
// MotionProcessor.c :  
//
// This example code connects to the Magellan motion processor on any PMD product via one of four possible interfaces. 
//


#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDperiph.h"
#include "PMDdevice.h"
#include "PMDsys.h"
#include "Examples.h"

#define MAJOR_VERSION 1
#define MINOR_VERSION 0

USER_CODE_VERSION(MAJOR_VERSION, MINOR_VERSION)
PMDresult result;

// SetupAxis1 is in Pro-MotionExport.c which is generated from the Pro-Motion application.
PMDresult SetupAxis1(PMDAxisHandle* hAxis);
PMDresult HomeCapture(PMDAxisHandle* hAxis);
//void ReverseMove(PMDAxisHandle* hAxis);


USER_CODE_TASK(MotionProcessor)
{
    // TO DO: Select the interface type.
    PMDInterfaceType Interface = InterfaceSerial;
    // TO DO: Select the DeviceType: PMD Resource Protocol (ex. ION/CME, Prodigy CME) or Motion Processor (ex. MC58420, MC58113, MC78113)
    PMDDeviceType DeviceType = PMDDeviceTypeResourceProtocol;
    PMDPeriphHandle hPeriph, hPeriph2;
    PMDDeviceHandle hDevice;
    PMDAxisHandle hAxis;
    PMDuint32 version, vmajor, vminor;
    PMDresult homeRes;
    PMDDataSize datasize = PMDDataSize_16Bit;
    PMDuint16 AnologAdd, volt;
    PMDuint8 eventIRQ;
    PMDuint32 offset, length;
    int Full;
    
    
    AnologAdd = 0x340;
    Full = 0;
    eventIRQ = 0;
    volt = 0;
    offset = 0x0000;
    length = 2;

    PMDGetCMotionVersion(&vmajor, &vminor);
    PMDprintf("C-Motion Version %d.%d \n", vmajor, vminor);

    switch (Interface)
    {
    case InterfaceSerial:
    {
        PMDSerialPort serialport = PMDSerialPort1;
        PMDprintf("Opening serial port COM%d\n", serialport + 1);
        PMD_ABORTONERROR(PMDDeviceOpenPeriphSerial(&hPeriph, NULL, serialport, PMDSerialBaud57600, PMDSerialParityNone, PMDSerialStopBits1));
        break;
    }

    case InterfaceSPI:
    {
        PMDuint8 port = 0;
        PMDuint8 chipselect = 1;
        PMDuint8 mode = 0;
        PMDuint8 datasize = 16;
        PMDparam bitrate = PMDSPIBitRate_625kHz;
        if (DeviceType == PMDDeviceTypeResourceProtocol)
            datasize = 8;
        PMDprintf("Opening the SPI port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphSPI(&hPeriph, NULL, port, chipselect, mode, datasize, bitrate));
        break;
    }

    case InterfaceCAN:
    {
        PMDCANBaud CANBaud = PMDCANBaud1000000;
        PMDprintf("Opening the CAN port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphCANNodeID(&hPeriph, NULL, CANBaud, 0));
        break;
    }

    case InterfaceTCP:
    {
        PMDuint32 IPaddress = PMD_IP4_ADDR(192, 168, 2, 2);
        PMDprintf("Opening a TCP port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphTCP(&hPeriph, NULL, IPaddress, DEFAULT_ETHERNET_PORT, 1000));
        DeviceType = PMDDeviceTypeResourceProtocol; // Only PRP products support Ethernet
        break;
    }

    default:
        PMDprintf("Invalid interface\n");
        PMDTaskAbort(1);
    }
    if (DeviceType == PMDDeviceTypeResourceProtocol)
    {
        // Open a handle to a PMD Resource Protocol device
        PMD_ABORTONERROR(PMDPeriphOpenDevicePRP(&hDevice, &hPeriph));
        PMD_RESULT(PMDDeviceGetVersion(&hDevice, &vmajor, &vminor));
    }
    else
    {
        // Open a handle to a PMD Motion Processor device
        PMD_ABORTONERROR(PMDPeriphOpenDeviceMP(&hDevice, &hPeriph));
    }
    PMDAxisOpen(&hAxis, &hDevice, PMDAxis1);

    // Open Peripheral
    PMD_ABORTONERROR(PMDDeviceOpenPeriphPIO(&hPeriph2, &hDevice, AnologAdd, eventIRQ, datasize));
    //PMD_ABORTONERROR(PMDPeriphRead(&hPeriph2,&volt,offset,length));
    //PMDprintf("%u\n", (unsigned int)volt);

    PMD_RESULT(PMDGetVersion32(&hAxis, &version));
    PMDprintf("Magellan version %08X\r\n", version);

    //PMDprintf("Calling AllMagellanCommands() from Examples.c\r\n");
    //AllMagellanCommands(&hAxis);

    // To do: Export a new Pro-MotionExport.c file from Pro-Motion for the motor setup in use. 
    // Using the version supplied with the SDK may generate errors.
    PMDprintf("Calling SetupAxis1() from Pro-MotionExport.c\r\n");
    SetupAxis1(&hAxis);

    homeRes = HomeCapture(&hAxis);// I added
    if(homeRes != PMD_ERR_OK){
    // Add some kind of error condition if homing fails
    }
    //PMDprintf("Calling ProfileMove() from Examples.c\r\n");
    //ProfileMove(&hAxis);

    PMDprintf("Would you like to start training?\r\n");
    PMDprintf("Type 'Y' for yes or 'N' for no\r\n");
   

#if Full

    PMDuint16 status;
    PMDuint16 eventmask;
    PMDint32 pos1, pos2, destpos;
    int stop, count;

    eventmask = PMDEventStatusMotionComplete | PMDEventStatusMotionError;

    PMDSetProfileMode(&hAxis, PMDProfileModeTrapezoidal);
    PMDSetVelocity(&hAxis, 20000);      // former 32768
    PMDSetAcceleration(&hAxis, 10000);  // former 256
    PMDSetJerk(&hAxis, 65535);
    pos1 = 0;
    pos2 = 20000;
    destpos = pos2;
    stop = 0;
    count = 0;
    while (!stop) {
        if (count == 10) stop = 1;
        if (destpos == pos1) destpos = pos2;
        else destpos = pos1, count++;
        PMDSetPosition(&hAxis, destpos);
        PMDResetEventStatus(&hAxis, 0);
        PMD_RESULT(PMDUpdate(&hAxis));
        PMDGetEventStatus(&hAxis, &status);
        while (!status) {
            PMDGetEventStatus(&hAxis, &status);            
            if (status & eventmask)
            {
                if (status == PMDEventStatusMotionError) stop = 1;
                else PMDprintf("Motion Complete\r\n");
            }
        }
    }
#endif // 0
    PMDprintf("Done\n");
    PMDPeriphClose(&hPeriph);

    PMDTaskAbort(1);
}

