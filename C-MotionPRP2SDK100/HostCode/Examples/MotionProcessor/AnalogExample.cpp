#include "AnalogExample.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDperiph.h"
#include "PMDdevice.h"
#include "PMDsys.h"

#define MAJOR_VERSION 1
#define MINOR_VERSION 0

USER_CODE_VERSION(MAJOR_VERSION, MINOR_VERSION)

void AnalogExample() {
    // Variables for main interface
    PMDInterfaceType Interface = InterfaceSerial;
    PMDDeviceType DeviceType = PMDDeviceTypeResourceProtocol;
    PMDPeriphHandle hPeriph;
    PMDDeviceHandle hDevice;
    PMDAxisHandle hAxis;
    PMDuint32 version, vmajor, vminor;
    PMDresult result;

    //Variables for Periph I/O
    PMDPeriphHandle hPeriph2;
    PMDDataSize datasize;
    PMDuint16 AnologAdd;
    PMDuint8 eventIRQ;
    PMDuint32 offset, length;
    PMDuint16 volt;

    datasize = PMDDataSize_16Bit;
    AnologAdd = 0x340;          // Found in section 4.10.1 of the ION-CME-N-Series Digital Drive User manual
    eventIRQ = 0;               // Not used according to the PRP program reference 2
    volt = 0;                   // Variable to store analog signal
    offset = 0x0000;            // Not sure what this does. I know it needs to be even
    length = 2;                 // This is how much of datasize it should read. Must be even

    PMDGetCMotionVersion(&vmajor, &vminor);
    PMDprintf("C-Motion Version %d.%d \n", vmajor, vminor);

    // Main Serial connection
    PMDSerialPort serialport = PMDSerialPort1;
    PMDprintf("Opening serial port COM%d\n", serialport + 1);
    PMD_ABORTONERROR(PMDDeviceOpenPeriphSerial(&hPeriph, NULL, serialport, PMDSerialBaud57600, PMDSerialParityNone, PMDSerialStopBits1));

    // Open a handle to a PMD Resource Protocol device
    PMD_ABORTONERROR(PMDPeriphOpenDevicePRP(&hDevice, &hPeriph));
    PMD_RESULT(PMDDeviceGetVersion(&hDevice, &vmajor, &vminor));

    PMDAxisOpen(&hAxis, &hDevice, PMDAxis1);

    // Open Peripheral and read signal
    PMD_ABORTONERROR(PMDDeviceOpenPeriphPIO(&hPeriph2, &hDevice, AnologAdd, eventIRQ, datasize));
    PMD_ABORTONERROR(PMDPeriphRead(&hPeriph2, &volt, offset, length));
}