//
// GenericUserPacket.c :  

// This example code runs on a CME Device connected to a host. There are two methods for sending/receiving user packets.
// The PRP user packet method transmits the user packet via a PMD Resource Protocol connection.
// The other method sends a raw packet over it's own interface connection.
// The advantage to this method is that it does not include the protocol overhead of the CME method.
// This example will open a port and wait to receive any data from the host and send it back.

#include <stdio.h>
#include <stdlib.h>

#include "c-motion.h", "C-Motion.h"
#include "PMDdiag.h"
#include "PMDperiph.h"
#include "PMDdevice.h"
#include "PMDsys.h"

#define BUFSIZE   200 
#define MAJOR_VERSION 1
#define MINOR_VERSION 1

USER_CODE_VERSION(MAJOR_VERSION, MINOR_VERSION);

USER_CODE_TASK(GenericUserPacket)
{
    int p = 1, send = 0;
    // To do: Set the Interface type here or use the TaskParam to select the interface.
    // TaskParam  is set by the CMETaskStart command sent from the host.
    PMDInterfaceType Interface = InterfaceSerial;  // Formerly InterfaceNone
    PMDPeriphHandle hPeriph;
    PMDuint32 vmajor, vminor; 
    char data[BUFSIZE];
    PMDuint32 bytesReceived, i;
    PMDuint32 bytesExpected = 15;
    PMDresult result;
    char dataV[6] = {0x01,0x02,0x03,0x04,0x05,0x06};
    //PMDprintf("\n\n*** Begin GenericUserPacket.c ***\n");
    //PMDGetCMotionVersion( &vmajor, &vminor );
    //PMDprintf("C-Motion Version %d.%d \n", vmajor, vminor);

    //PMDprintf("Attempting to open the communications port\n");
    
    if (TaskParam > 0)
    {
        Interface = TaskParam & 0xF;
    }

    switch (Interface)
    {
    case InterfaceSerial: // 4
    {
        PMDSerialPort serialport = PMDSerialPort1;
        PMDprintf("Opening serial port COM%d\n", serialport + 1);
        PMD_ABORTONERROR(PMDDeviceOpenPeriphSerial(&hPeriph, NULL, serialport, PMDSerialBaud57600, PMDSerialParityNone, PMDSerialStopBits1));
        break;
    }
    case InterfaceCAN: // 5
    {
        // open a CAN connection. The CAN Tx and Rx addresses must be opposite of the addresses used in the Host code.
        PMDparam addressTX = 0x200;
        PMDparam addressRX = 0x100;
        PMDprintf("Opening a CAN port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphCAN(&hPeriph, NULL, addressTX, addressRX, 0));
        break;
    }
    case InterfaceTCP: // 6
    {
        // listen for a TCP connection on port 44444
        PMDuint32 IPaddress = PMD_IP4_ADDR(0, 0, 0, 0);
        PMDuint32 portnum = 44444;
        PMDprintf("Opening a TCP port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphTCP(&hPeriph, NULL, IPaddress, portnum, PMD_WAITFOREVER));
        break;
    }
    case InterfaceUDP: // 7
    {
        // listen for a UDP packets on port 44444
        PMDuint32 IPaddress = PMD_IP4_ADDR(0, 0, 0, 0);
        PMDuint32 portnum = 44444;
        PMDprintf("Opening a UDP port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphUDP(&hPeriph, NULL, IPaddress, portnum));
        break;
    }
    case InterfaceSPI: // 8
    {
        PMDuint8 port = PMDSPIPort_Host; // slave SPI port
        PMDuint8 chipselect = 1;
        PMDuint8 mode = 0;
        PMDuint8 datasize = 16;
        PMDparam bitrate = PMDSPIBitRate_625kHz;
        PMDprintf("Opening the SPI port \n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphSPI(&hPeriph, NULL, port, chipselect, mode, datasize, bitrate));
        break;
    }
    case InterfacePRP: // 9
    {
        PMDuint32 PRPchannel = 2;
        PMDprintf("Opening a PRP channel #%d\n", PRPchannel);
        PMD_ABORTONERROR(PMDDeviceOpenPeriphPRP(&hPeriph, NULL, PRPchannel, 1000));
        break;
    }
    case InterfaceNone: // CME
    {
        PMDprintf("Opening a CME peripheral\n");
        PMD_ABORTONERROR(PMDDeviceOpenPeriphCME(&hPeriph, NULL));
        break;
    }
    default:
        PMDTaskAbort(1);
    }
    //NewArray();
    while (1)
    {
        // Wait for data to be sent from the host
        result = PMDPeriphReceive(&hPeriph, &data, &bytesReceived, bytesExpected, 100);

        // A timeout will occur if the bytes received is less than the expected number of bytes within the timeout period.
        if (result == PMD_ERR_OK || result == PMD_ERR_Timeout)
        {
            if (bytesReceived > 0)
            {
                if (data[1] == 9) { break; }
                if (1) {
                    if (data[0] == 'M') {
                        dataV[0] = 0x00;
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (data[0] == 'R') {
                        dataV[0] = 0x01;
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (data[0] == 'S') {
                        dataV[0] = 0x02;
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (data[0] == 'E') {
                        dataV[0] = 0x03;
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (data[0] == 'V') {
                        dataV[0] = 0x04;
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (data[0] == 'T') {                        
                        dataV[0] = 0x05;
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else {
                        dataV[0] = 0x01, dataV[1] = 0x02, dataV[2] = 0x03, dataV[3] = 0x04;
                    }
                }
                else {
                    if (send == 0) {
                        dataV[0] = 'T';
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (send == 1) {
                        dataV[0] = 'P';
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (send == 2) {
                        dataV[0] = 'V';
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (send == 3) {
                        dataV[0] = 'F';
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (send == 4) {
                        dataV[0] = 'M';
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else if (send == 5) {
                        dataV[0] = 'D';
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                    else {
                        dataV[0] = 0x01;
                        dataV[1] = dataV[0];
                        dataV[2] = dataV[0];
                        dataV[3] = dataV[0];
                    }
                }
                PMDprintf("%d bytes received: ", bytesReceived);
                for (i = 0; i < bytesReceived; i++)
                {
                    PMDprintf("%02x ", data[i]);
                    //data[i]++; // modify the data to send back to the host
                    //data[i] = 0x00;
                    if (1) {
                        if (data[i] == 'M') {
                            dataV[0] = i;                            
                        }
                        else if (data[i] == 'R') {                            
                            dataV[1] = i;
                        }
                        else if (data[i] == 'S') {
                            dataV[2] = i;
                        }
                        else if (data[i] == 'E') {
                            dataV[3] = i;
                        }
                        else if (data[i] == 'V') {
                            dataV[4] = i;
                        }
                        else if (data[i] == 'T') {
                            dataV[5] = i;
                        }
                    }
                }
                PMDprintf("\r\n");
                PMDprintf("M: %d\r\n", dataV[0]);
                PMDprintf("R: %d\r\n", dataV[1]);
                PMDprintf("S: %d\r\n", dataV[2]);
                PMDprintf("E: %d\r\n", dataV[3]);
                PMDprintf("V: %d\r\n", dataV[4]);
                PMDprintf("T: %d\r\n", dataV[5]);
                //data[0] = 0x01, data[1] = 0x02, data[2] = 0x03, data[3] = 0x04;
                //dataV[0] = 0x01, dataV[1] = 0x02, dataV[2] = 0x03, dataV[3] = 0x04;
                //dataV[0] = bytesReceived;
                //bytesReceived = sizeof(dataV) / sizeof(dataV[0]);
                PMDputs("\r\n");
                // UDP is unidirectional.  An additional peripheral handle can be opened for Tx traffic.
                if (Interface != InterfaceUDP)
                {
                    //PMDprintf("Sending the received data\r\n");
                    PMD_RESULT(PMDPeriphSend(&hPeriph, &data, bytesReceived, 2000)); 
                    send++;
                }           
            }
        }
        else
        {
            //PMDputs(PMDGetErrorMessage(result));
            //PMDputs("\r\n");
        }
    }

    //PMDprintf("Done\n");
    PMDTaskWait(3000);
    PMDPeriphClose(&hPeriph);

    PMDTaskAbort(0);
}

void NewArray() {
    int* data = NULL;         // Pointer to hold the dynamic array
    size_t capacity = 100;    // Initial capacity (you can estimate this based on the max data)
    size_t size = 0;          // Current number of elements in the array
    int new_data;             // New sensor data to be added

    // Initially allocate memory for 100 points (or whatever makes sense for your use case)
    data = (int*)malloc(capacity * sizeof(int));  // Explicit cast to int*
    if (data == NULL) {
        PMDprintf("Memory allocation failed!\n");
        return; // Exit the function if memory allocation fails
    }

    // Simulate data collection with a loop (for example, 10ms per data point)
    for (int i = 0; i < 200; i++) {  // Simulate collecting 200 data points
        PMDTaskWait(50);
        // If the array is full, double its capacity (to avoid frequent reallocations)
        if (size == capacity) {
            capacity *= 2;  // Double the capacity
            int* temp = (int*)realloc(data, capacity * sizeof(int));  // Explicit cast to int*
            if (temp == NULL) {
                free(data);  // Free previously allocated memory before returning
                PMDprintf("Memory reallocation failed!\n");
                return; // Exit the function if memory reallocation fails
            }
            data = temp;  // Point to the newly allocated memory
            PMDprintf("Size increased\r\n");
        }

        // Add new data (this simulates sensor data being added)
        new_data = i * 1;  // Dummy data
        data[size] = new_data;
        size++;  // Increment the size after adding data
    }

    // Print the data to verify
    for (size_t i = 0; i < size; i++) {
        printf("data = %d \r\n", data[i]);
    }
    PMDprintf("\n");

    // Clean up: Free the allocated memory
    free(data);
    PMDprintf("NewArray DONE");
}

