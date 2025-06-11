//
// This Windows console program will download a CME user code .bin file to the N-Series ION/CME over the selected interface.
//

#include "C-Motion.h"
#include "PMDperiph.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include <conio.h>
#include <io.h>
#include <time.h>

void UserCodeFunctions(PMDDeviceHandle *phDevice);
void LogPRPConsole(PMDDeviceHandle *phDevice, char* log_file_name);
PMDTaskState PrintTaskState(PMDDeviceHandle *phDevice);


int main(int argc, char **argv)
{
    PMDresult result;
    PMDDeviceHandle *phDevice = NULL;
	PMDuint32 vmajor; 
	PMDuint32 vminor;
	int serialport = 9; 
	int port = 0; 
	int arg = 0;
	size_t n;
    FILE *inp;
    char *data = NULL;
    long file_length;
    unsigned long taskparam = 0;
    char *task_param = NULL;
    char *file_name = NULL;
    char *log_file_name = NULL;
    char *port_name = NULL;
	char *port_number = NULL;
    char ch = 'y';
    int bUseConsoleMonitor = 0;
    int bStartUserCode = 0;
	long baudindex = 0;
    int error = 0;
	PMDInterfaceType Interface = InterfaceSerial;
    PMDTaskState state;
    BOOL bPrompt = TRUE;
    BOOL bReset = FALSE;
    BOOL bConsole = FALSE;

    PMDPeriphHandle hPeriph = { 0 }; // zero structure members
	PMDDeviceHandle hDevice = { 0 };

    puts("StoreUsereCode version 1.1\n");

    // Process the command line arguments
    for (arg = 2; arg < argc; arg++)
    {
        if ((argv[arg][0] == '-') || (argv[arg][0] == '/'))
        {
            switch (toupper(argv[arg][1]))
            {

            case 'B':				/* set baud */
                baudindex = atoi(&argv[arg][2]);
                break;

            case 'Y':
                bPrompt = FALSE;
                puts("Forcing 'y' for prompts");
                break;

            case 'R':
                bReset = TRUE;
                break;

            case 'C':
                bUseConsoleMonitor = TRUE;
                break;

            case 'L':
                log_file_name = &argv[arg][2];
                break;

            case 'F':
                file_name = &argv[arg][2];
                break;

            case 'A': // Address/Port #
                port_number = &argv[arg][2];
                if (port_number != NULL)
                    port = atoi(port_number);
                break;
            case 'T': // TaskParam
                task_param = &argv[arg][2];
                if (task_param != NULL)
                {
                    if (task_param[1] == 'X' || task_param[1] == 'x')
                        sscanf(task_param, "%X", &taskparam);
                    else
                        sscanf(task_param, "%d", &taskparam);
                }
                break;

            default:
                error = TRUE;
                break;
            }
        }

        if (error)
        {
            fprintf(stderr, "Illegal argument: \"%s\"\n", argv[arg]);
        }
    }
    if (argc > 1)
    {
	    port_name = argv[1];
	    if (!strncmp(port_name, "TCP", 3))
	    {
		    Interface = InterfaceTCP;
		    port = 2;
	    }
	    else if (!strncmp(port_name, "CAN", 3))
		    Interface = InterfaceCAN;
	    else if (!strncmp(port_name, "SPI", 3))
		    Interface = InterfaceSPI;
	    else if (!strncmp(port_name, "COM", 3))
	    {
		    Interface = InterfaceSerial;
		    sscanf(port_name, "COM%d", &serialport);
	    }
	    else 
            port_name = NULL;
    }

    if (argc == 1 || port_name == NULL || error)
	{
		puts("Usage: StoreUserCode interface [-F<CMEbinfile>] [-A<address>][-T<taskparam>] [-Y] [-L<logfile>]");
		puts("           interface = one of TCP, COM1, CAN, SPI");
        puts("           address = optional interface dependent address.");
        puts("           baud = optional interface dependent baud index.");
        puts("           taskparam = optional 32-bit value (hex or decimal) to pass to the task.");
        puts("           ex: StoreUserCode TCP -A3                  -FmyCMECode.bin -> IP address 192.168.2.3 (default is 2)");
        puts("           ex: StoreUserCode SPI -T12                 -FmyCMECode.bin -> SPI with task parameter of 12");
        puts("           ex: StoreUserCode COM3 -A1                 -FmyCMECode.bin -> Serial port COM3 RS485 multidrop address 1 (default is 0)");
        puts("           ex: StoreUserCode COM10                    -FmyCMECode.bin -> Serial port COM10");
        puts("           ex: StoreUserCode CAN -A1 -B6 -T0x12345678 -FmyCMECode.bin -> CAN nodeID 1 (default is 0), baud=20k and task parameter of 12345678 hex");
        return 0;
	}

    // open the local peripheral that is connected to the ION
	if (Interface == InterfaceSerial)
	{
		PMDprintf("Connecting to COM%d\r\n", serialport);
		PMD_ABORTONERROR(PMDPeriphOpenCOM( &hPeriph, NULL, serialport-1, 57600, PMDSerialParityNone, PMDSerialStopBits1))
		if (port > 0)
			PMD_ABORTONERROR(PMDPeriphOpenMultiDrop(&hPeriph, &hPeriph, port))
	}
	else
	if (Interface == InterfaceSPI)
	{
		PMD_ABORTONERROR(PMDPeriphOpenSPI(&hPeriph, NULL, 0, 0, 1, 8, 1000000))
	}
	else
	if (Interface == InterfaceTCP)
	{
    	PMDuint32 ipaddress = PMD_IP4_ADDR(192,168,2,port);
		PMDprintf("Connecting via TCP IP address 0x%08X\r\n", ipaddress);
		PMD_ABORTONERROR(PMDPeriphOpenTCP( &hPeriph, NULL, ipaddress, DEFAULT_ETHERNET_PORT, 0 ))
	}
	else
	if (Interface == InterfaceCAN)
	{
		printf("Connecting via CAN\r\n");
		PMD_ABORTONERROR(PMDPeriphOpenCANNodeID(&hPeriph, NULL, baudindex, port))
	}
	PMD_ABORTONERROR(PMDRPDeviceOpen(&hDevice, &hPeriph))	// Open a handle to a PRP device
	phDevice = &hDevice;

    // The first command received by the N-ION after power-on or reset will return PMD_ERR_RP_Reset.
    PMD_RESULT(PMDDeviceGetVersion(phDevice, &vmajor, &vminor));
    // It will not return PMD_ERR_RP_Reset a second time, so abort on any error.
    PMD_ABORTONERROR(PMDDeviceGetVersion(phDevice, &vmajor, &vminor));
	PMDprintf("CME device version v%d.%d\r\n", vmajor, vminor);
    UserCodeFunctions(phDevice);
    state = PrintTaskState(phDevice);

    if (file_name != NULL)
    {
        // open the user code bin file
        inp = fopen(file_name, "rb");
        if (inp)
        {
            file_length = _filelength(_fileno(inp));
            if (file_length > 0)
                data = (char*)malloc(file_length);
            n = fread(data, sizeof(char), file_length, inp);

            PMDprintf("%d words read from file \"%s\" of length %d\r\n", n, file_name, file_length);

            fclose(inp);
        }
        else
        {
            PMDprintf("ERROR opening file \"%s\": %s (%d)\r\n", file_name, strerror(errno), errno);
        }
    }

    if (data != NULL)
    {
        PMDprintf("Download user code? (y/n)...");
        if (bPrompt)
            ch = _getch();
        PMDprintf("%c\r\n", ch);
        if (ch == 'y')
        {
            PMDprintf("Downloading... ");
            PMD_RESULT(PMDDeviceStoreUserCode(phDevice, data, file_length));
            PMDprintf("\r\n");
        }
        free(data);
    }
//        if (ch != 'y')
    {

        if (state == PMDTaskState_Running || bReset)
        {
            if (!bReset)
            {
                // A reset should be performed if the task was running before downloading because it will be abruptly stopped potentially leaving handles open.
                PMDprintf("Device should be reset. Reset device? (y/n)...");
                if (bPrompt)
                    ch = _getch();
                PMDprintf("%c\r\n", ch);
                bReset = (ch == 'y');
            }
            if (bReset)
            {
                PMDprintf("Resetting...\r\n");
                PMD_RESULT(PMDDeviceReset(phDevice));
                int i = 0;
                do {
                    result = PMDDeviceNoOperation(phDevice);
                } while (result != PMD_ERR_OK && i++ < 100);
            }
        }
        // Display the newly downloaded task information
        if (result == PMD_ERR_OK)
        {
            UserCodeFunctions(phDevice);
        }
    }

    if (result == PMD_ERR_OK)
    {
        if (taskparam == 0)
        {
            char task_param[20];
            PMDprintf("Enter taskparam in hex or Enter for 0...");
            fgets(task_param, 10, stdin);
            if (task_param[1] == 'X' || task_param[1] == 'x')
                sscanf(task_param, "%X", &taskparam);
            else
                sscanf(task_param, "%d", &taskparam);
            PMDprintf("%08X\r\n", taskparam);
        }
        PMDprintf("Start user code with taskparam 0x%X? (y/n)...", taskparam);
        if (bPrompt)
            ch = _getch();
        PMDprintf("%c\r\n", ch);
        bStartUserCode = (ch == 'y');
        if (!bUseConsoleMonitor)
        {
            PMDprintf("Start PRP console monitor? (y/n)...");
            ch = _getch();
            PMDprintf("%c\r\n", ch);
            bUseConsoleMonitor = (ch == 'y');
        }
        if (bStartUserCode)
        {
            PMD_RESULT(PMDTaskStop(phDevice)); // reset user code global variables in case downloading was not performed.
            PMD_RESULT(PMDTaskStart(phDevice, taskparam))
        }
    }
    PrintTaskState(phDevice);

    if (bUseConsoleMonitor)
        LogPRPConsole(&hDevice, log_file_name);

	PMDprintf("\r\nDone\r\n");

	PMD_ABORTONERROR(PMDDeviceClose(&hDevice));
	PMD_ABORTONERROR(PMDPeriphClose(&hPeriph));
}

PMDTaskState PrintTaskState(PMDDeviceHandle *phDevice)
{
    PMDresult result;
    PMDTaskState state = PMDTaskState_Error;

    PMD_RESULT(PMDCMETaskGetState(phDevice, &state));
    
    if (result == PMD_ERR_OK)
    {
        PMDprintf("Task state = ");
        if (state == PMDTaskState_NoCode)
            PMDprintf("no code \r\n");
        else if (state == PMDTaskState_NotStarted)
            PMDprintf("not started \r\n");
        else if (state == PMDTaskState_Running)
            PMDprintf("running \r\n");
        else if (state == PMDTaskState_Aborted)
            PMDprintf("aborted \r\n");
        else
            PMDprintf("Task state = %d ", state);

    }
    return state;
}

void UserCodeFunctions(PMDDeviceHandle *phDevice)
{
    PMDresult result;
    char szValue[100];
    PMDuint32 value32 = 0;

    PMDprintf("User code currently on device:\r\n");
    szValue[0] = 0;
    PMD_RESULT(PMDCMEGetUserCodeName(phDevice, szValue));
    PMDprintf("\tName:     %.50s\r\n", szValue);
    PMD_RESULT(PMDCMEGetUserCodeDate(phDevice, szValue));
    PMDprintf("\tDate:     %.30s\r\n", szValue);
    PMD_RESULT(PMDCMEGetUserCodeVersion(phDevice, &value32));
    PMDprintf("\tVersion:  %08X\r\n", value32);
    PMD_RESULT(PMDCMEGetUserCodeChecksum(phDevice, &value32));
    PMDprintf("\tChecksum: %08X\r\n", value32);
    PrintTaskState(phDevice);
}


void LogPRPConsole(PMDDeviceHandle *phDevice, char *log_file_name)
{
    PMDresult result;
    PMDTaskState state = PMDTaskState_Invalid;
    PMDparam channel = 1;
    PMDPeriphHandle hPeriph = { 0 };
    char datarx[MAX_PACKET_DATA_LENGTH];
    int count = MAX_PACKET_DATA_LENGTH;
    int nReceived = 0;
    int loopcount = 0;
    PMDuint16 ConsoleInterfaceType;
    int timeout = 10; // timeout for extracting any PRP console data.
    FILE *logfile = NULL;

    PMD_RESULT(PMDDeviceGetDefault(phDevice, PMD_Default_ConsoleIntfType, &ConsoleInterfaceType, PMDDataSize_16Bit));
    if (result == PMD_ERR_OK)
    {
        if (ConsoleInterfaceType != InterfacePRP)
        {
            PMDprintf("\r\nPRP console must be set to InterfacePRP (9). It is currently set to %d.\r\n", ConsoleInterfaceType);
            return;
        }
        // open log file
        if (log_file_name)
        {
            PMDprintf("Opening log file \"%s\"\n", log_file_name);
            logfile = fopen(log_file_name, "ab");
            if (logfile)
            {
                int length;
                // Get Date
                time_t current_time;
                char* c_time_string;
                current_time = time(NULL);
                c_time_string = ctime(&current_time);
                // write the date
                length = strlen(c_time_string);
                fwrite(c_time_string, 1, length, logfile);
            }
            else
            {
                PMDprintf("ERROR opening log file \"%s\": %s (%d)\r\n", log_file_name, strerror(errno), errno);
            }
        }

        PMDprintf("\r\nMonitoring PRP console output. Hit any key to abort.\r\n");
        PMDprintf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\r\n");
        PMD_RESULT(PMDDeviceOpenPeriphPRP(&hPeriph, phDevice, channel, 0));
        while (result == PMD_ERR_OK || result == PMD_ERR_Timeout || result == PMD_ERR_RP_Reset)
        {
            nReceived = 0;
            result = PMDPeriphReceive(&hPeriph, datarx, &nReceived, count, timeout);
            if (result == PMD_ERR_OK || result == PMD_ERR_Timeout)
            {
                if (nReceived)
                {
                    loopcount = 0;
                    fwrite(datarx, 1, nReceived, stdout);
                    if (logfile)
                        fwrite(datarx, 1, nReceived, logfile);
                }
                else
                {
                    PMD_RESULT(PMDCMETaskGetState(phDevice, &state));
                    // Note: user code that erases NVRAM sectors will cause interrupts to be disabled which will cause host communication timeout errors.
                    // It may also cause serial data overrun on the CME side if the host keeps trying to communicate causing the UART FIFO to fill.
                    // It is recommended to use a direct output console interface such as Serial or UDP in this case.
                    if (result == PMD_ERR_Timeout && loopcount++ > 6)
                        break;
                    if ((result != PMD_ERR_OK && result != PMD_ERR_RP_Reset) || (result == PMD_ERR_OK && state != PMDTaskState_Running && loopcount++ > 10))
                    {
                        break;
                    }
                }
                if (_kbhit())
                    break;
            }
        }
        if (!(result == PMD_ERR_OK || result == PMD_ERR_Timeout))
        {
            PMDprintf(PMDGetErrorMessage(result));
            PMDprintf("\n");
        }
        PMDprintf("\r\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        PMDprintf("\r\nEnding PRP console output monitoring.\r\n");
        if (state != PMDTaskState_Running && loopcount > 2)
            PMDprintf("CME task no longer running.\r\n");
        PrintTaskState(phDevice);
        if (logfile)
        {
            fclose(logfile);
        }
    }
}

