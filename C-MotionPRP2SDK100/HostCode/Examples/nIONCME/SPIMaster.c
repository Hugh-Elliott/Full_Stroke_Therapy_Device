//
// SPI Master.c :  

// This example code runs on a N-series connected to an SPI Slave on the SPI Expansion bus.
#ifdef USE_SPIMASTER

#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDperiph.h"
#include "PMDdevice.h"
#include "PMDsys.h"

#define BUFSIZE   200 
#define MAJOR_VERSION 1
#define MINOR_VERSION 1

USER_CODE_VERSION(MAJOR_VERSION, MINOR_VERSION)

int Parse(char* data, int bytesReceived, char* command, int *cmdargs, int *cmdaxismask);

USER_CODE_TASK(GenericUserPacket)
{
   
    PMDPeriphHandle hPeriph;
    PMDuint32 vmajor, vminor; 
    char data[BUFSIZE];
    PMDuint32 bytesReceived, i;
    PMDresult result;
    PMDuint8 chipselect = 1;
    PMDuint8 SPImode = 0;
    PMDuint8 datasize = 8;
    PMDparam bitrate = PMDSPIBitRate_625kHz;
    int sensordata;
    

    PMDprintf("\n\n*** Begin GenericUserPacket.c ***\n");
    PMDGetCMotionVersion( &vmajor, &vminor );
    PMDprintf("C-Motion Version %d.%d \n", vmajor, vminor);

    PMDprintf("Attempting to open the Expansion SPI port\n");
    PMD_ABORTONERROR(PMDDeviceOpenPeriphSPI(&hPeriph, NULL, PMDSPIPort_Exp, chipselect, SPImode, datasize, bitrate))
    
    while (1)
    {
        data[0] = 0xAA;   // data could be anything
        data[1] = 0xAA;
        data[2] = 0x55;
        data[3] = 0x55;
        
        //The Send call is needed to generation SPI clock activty even if not actually sending anything
        PMD_ABORTONERROR(PMDPeriphSend(&hPeriph, &data, 2, 1000))
        PMD_ABORTONERROR(PMDPeriphReceive(&hPeriph, &data, &bytesReceived, 2, 1000))
       
        sensordata = data[1] << 8;
        sensordata |= data[0];
        PMDprintf("Data received on Expansion SPI=%x\n", sensordata);
        PMDTaskWait(50);
    }

    PMDprintf("Done\n");
    PMDTaskWait(3000);
    PMDPeriphClose(&hPeriph);

    PMDTaskAbort(0);
}

#endif // USE_SPIMASTER