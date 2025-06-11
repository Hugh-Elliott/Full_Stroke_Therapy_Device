#include "COMFUNC.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"

// This function is used to communicate with an arduino uno and return either 1 or -1 to modify direction in velocity profile mode

int COMFUNC(PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriph) {

	PMDresult result, sendres;
	int dir = 1, len = 0, len2, method = 1;
	char tempdir[8];
	PMDint32 vel;
	byte start = 0x3C, end = 0x3E;
	PMDuint32 buffer = 10;
	char str[6], rec[6], str2[7];

	PMD_RESULT(PMDGetVelocity(hAxis1, &vel));
	PMDint32 temp = abs(vel);
	sprintf(str, "%ld", temp);
	len = strlen(str);
	//str[len] = '\0';

	switch (method) {
	case 1: // Used to send a string without using start/ end markers
		sendres = (PMDPeriphSend(hPeriph, &str, len + 1, 50));
	case 2: // Used to send a string with start/ end markers already attached
		str2[0] = '<';
		for (size_t i = 0; i < len + 1; i++)
		{
			str2[i + 1] = str[i];
		}
		len2 = strlen(str2);
		str2[len + 1] = '>';
		len2 = strlen(str2);
		//str2[len + 2] = '\0';
		sendres = (PMDPeriphSend(hPeriph, &str2, len2 + 1, 50));
	case 3: // Userd to send a string without start/ end markers attached
		PMD_RESULT(PMDPeriphSend(hPeriph, &start, 1, 0));
		sendres = (PMDPeriphSend(hPeriph, &str, len + 1, 50));
		PMD_RESULT(PMDPeriphSend(hPeriph, &end, 1, 0));
	}

	switch (sendres) // Used to check if there was a timeout when sending
	{
	case PMD_ERR_Timeout:
		//PMDprintf("Send timeout\r\n");
		break;
	case PMD_ERR_OK:
		//PMDprintf("Send OK\r\n");
		break;
	}
	result = PMDPeriphReceive(hPeriph, &tempdir, &buffer, 10,100);
	//sprintf(rec, "%c", tempdir);
	PMDprintf("Tempdir = %s\r\n", tempdir);
	switch (result) // used to check if there was a timeout while receiving 
	{
	case PMD_ERR_Timeout:
		//PMDprintf("Receive timeout\r\n");
		result = PMD_ERR_OK;
		break;
	case PMD_ERR_OK:  // Switching direction
		//PMDprintf("Receive OK\r\n");
		if (vel == 0x01) { 
			dir = 1;
		}
		else if (vel == 0) {
			dir = -1;
		}
		//else PMDprintf("Result = %d\r\n", result);		
	}
#if 0
	PMDHandle hSerial;
	hSerial = CreateFile("COM5",
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (hSerial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			//serial port does not exist. Inform user.
		}
		//some other error occurred. Inform user.
	}
	DCB dcbSerialParams = { 0 };
	PMDdcbSerial.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		//error getting state
	}
	dcbSerialParams.BaudRate = CBR_19200;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(hSerial, &dcbSerialParams)) {
		//error setting serial port state
	}
	PMDHostInterface_Serial;
#endif // 0
	return dir;
}