#include "ComFunctions.h"
#include "PMDperiph.h"
#include "PMDsys.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDutil.h"

COMConfig comConfig = {0, 0, 0, 0, 0, 0};

#define BUFFSIZE   200

void defaultVals(void) {
	comConfig._mode = 0;
	comConfig._rep = 3;
	comConfig._startPos = 0;
	comConfig._endPos = 17000;
	comConfig._desVel = 20000;
	comConfig._torq = 10;
}

void Take(PMDPeriphHandle *hPeriphSer) {
	PMDuint32 bytesReceived, i;
	PMDuint32 bytesExpected = 16;
	PMDresult result;
	char data[BUFFSIZE] = {0};

	int mIndex = 0, rIndex = 0, sIndex = 0, eIndex = 0, vIndex = 0, tIndex = 0, dSize = 0;
	PMDint32 temp = 0;
	//PMDTaskWait(50);
	// Wait for data to be sent from the host
	result = PMDPeriphReceive(hPeriphSer, &data, &bytesReceived, bytesExpected, 50);

	// A timeout will occur if the bytes received is less than the expected number of bytes within the timeout period.
	if (result == PMD_ERR_OK || result == PMD_ERR_Timeout)
	{
		if (bytesReceived > 0)
		{
			dSize = sizeof(data) / sizeof(data[0]);
			PMDprintf("%d bytes received: ", bytesReceived);
			for (i = 0; i < bytesReceived; i++)
			{
				if (data[i] == 'M') {
					mIndex = i;
				}
				else if (data[i] == 'R') {
					rIndex = i;
				}
				else if (data[i] == 'S') {
					sIndex = i;
				}
				else if (data[i] == 'E') {
					eIndex = i;
				}
				else if (data[i] == 'V') {
					vIndex = i;
				}
				else if (data[i] == 'T') {
					tIndex = i;
				}
				PMDprintf("%02x ", data[i]);
			}
			if (mIndex != 0) {
				comConfig._mode = data[mIndex+1];
			}
			else comConfig._mode = data[1];
			if (rIndex != 2) {
				comConfig._rep = data[rIndex+1];
			}
			else comConfig._rep = data[3];
			if (eIndex - sIndex != 2) {					// eInded - sIndex should never be more than 3
				// example sIndex = 4 and eIndex = 7
				temp = data[sIndex + 1] << 8;	// data[5]
				temp |= data[sIndex + 2];		// data[6]
				comConfig._startPos = temp;
			}
			else {
				if (sIndex != 4) {
					comConfig._startPos = data[sIndex + 1];
				}
				else comConfig._startPos = data[5];
			}
			if (vIndex - eIndex == 3) {					// vIndex - eIndex should never be more than 3
					// example eIndex = 6 and vIndex = 9
				temp = data[eIndex + 1] << 8;
				temp |= data[eIndex + 2];
				comConfig._endPos = temp;
			}
			else { // example this is for eIndex = 6 and vIndex = 8
				comConfig._endPos = data[eIndex + 1];
			}
			if (tIndex - vIndex == 3) {					 //tIndex - vIndex should never be more than 3
					// example vIndex = 9 and tIndex = 12
				temp = data[vIndex + 1] << 8;
				temp |= data[vIndex + 2];
				comConfig._desVel = temp;
			}
			else { // example this is for vIndex = 9 and tIndex = 11 
				comConfig._desVel = data[vIndex + 1];
			}
			/*
			if (tIndex != 12) {
				comConfig._torq = data[tIndex + 1];
			}
			else comConfig._torq = data[13];
			*/
			if ((dSize - 1) - tIndex == 2) {
				temp = data[tIndex + 1] << 8;
				temp |= data[tIndex + 2];
				comConfig._torq = temp;
			}
			else {
				comConfig._torq = data[tIndex + 1];
			}
			
			PMDprintf("\r\n");
			PMD_RESULT(PMDPeriphSend(hPeriphSer, &data, bytesReceived, 100));
		}
	}
}
// Work in progress
void Send(PMDPeriphHandle *hPeriphSer, char sendMode, PMDint32 *time, int tSize, PMDint32 *pos, int pSize, PMDint32 *vel, int vSize, PMDint32 *force, int fSize, PMDint16 *motCom, int mSize, PMDint32 *desVel, int dvSize) {
	PMDresult result;
	PMDuint32 bytes2Send = 4;
	char label[2] = {'<', 'Z'};
	char endlabel[4] = { '>', '>', '>', '>' };
	char test[2] = { '<' , '>' };
	int i = 0;

	// Sends Time
	if ((sendMode & 0b00000001) == 0b00000001) {
		PMDprintf("Send Time\r\n");
		label[1] = 'T';
		//PMD_RESULT(PMDPeriphSend(hPeriphSer, &test, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		//PMDTaskWait(50);
		PMD_RESULT(PMDPeriphSend(hPeriphSer, time, tSize, 1000));
		//for (i = 0; i < tSize; i++) {

		//}
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
		//PMDTaskWait(50);
	}
	// Sends Position
	if ((sendMode & 0b00000010) == 0b00000010) {
		PMDTaskWait(100);
		PMDprintf("Send Position\r\n");
		label[1] = 'P';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, pos, pSize, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Velocity
	if ((sendMode & 0b00000100) == 0b00000100) {
		PMDTaskWait(100);
		PMDprintf("Send Velocity\r\n");
		for (i = 0; i < vSize; i++) {
			//PMDprintf("Vel = %d\r\n", vel[i]);
		}
		label[1] = 'V';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, vel, vSize, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Force
	if ((sendMode & 0b00001000) == 0b00001000) {
		PMDTaskWait(100);
		PMDprintf("Send Force\r\n");
		label[1] = 'F';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, force, fSize, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Motor Command
	if ((sendMode & 0b00010000) == 0b00010000) {
		PMDTaskWait(100);
		PMDprintf("Send Motor Command\r\n");
		label[1] = 'M';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, motCom, mSize, 2000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Desired Velocity
	if ((sendMode & 0b00100000) == 0b00100000) {
		PMDTaskWait(100);
		PMDprintf("Send Desired Velocity\r\n");
		label[1] = 'D';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, desVel, dvSize, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	label[1] = 'S';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
}
void SendError(PMDPeriphHandle* hPeriphSer, PMDresult resErr) {
	PMDresult result;
	PMDuint32 bytes2Send = 4;
	PMDint32 temp = resErr;
	PMD_RESULT(PMDPeriphSend(hPeriphSer, temp, bytes2Send, 100));
	//PMDTaskWait(50);
}
void SendBasic(PMDPeriphHandle* hPeriphSer, PMDuint8 Mode, PMDint32 pos) {
	int mSize = sizeof(Mode), pSize = sizeof(pos);
	PMDuint8 temp[1] = { Mode };
	//PMDPeriphSend(hPeriphSer, "M", 1, 100);
	//PMDPeriphSend(hPeriphSer, temp, 1, 200);
	//PMDPeriphSend(hPeriphSer, "P", 1, 100);
	//PMDPeriphSend(hPeriphSer, pos, pSize, 100);
	//PMDTaskWait(50);
	if (Mode == pos) {
		PMDint32 pTemp[4] = { pos };
		PMDPeriphSend(hPeriphSer, pTemp, pSize, 100);
	}
	else {
		PMDPeriphSend(hPeriphSer, temp, 1, 200);
	}
}
void SendMotComTest(PMDPeriphHandle* hPeriphSer, PMDint16 mot1, PMDint16 mot2) {
	PMDresult result;
	char label[2] = { '<', 'F' };
	char endlabel[4] = { '>', '>', '>', '>' };

	PMDint16 arr[2] = { mot1, mot2 }, mot1Arr[1] = { mot1 }, mot2Arr[1] = { mot2 };

	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
	PMDTaskWait(100);
	PMDprintf("Send mot1 and mot2 seperate\r\n");
	PMDPeriphSend(hPeriphSer, mot1Arr, 2, 200);
	PMDPeriphSend(hPeriphSer, mot2Arr, 2, 200);
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	PMDTaskWait(100);
	PMDprintf("Send mot1 and mot2 as an array\r\n");
	label[1] = 'M';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
	PMDPeriphSend(hPeriphSer, &arr, 4, 200);
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	label[1] = 'S';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
}
// Used to send 1 set of data
void SendParts(PMDPeriphHandle* hPeriphSer, char sendMode, PMDint32 time, PMDint32 pos, PMDint32 vel, PMDint32 force, PMDint16 motCom, PMDint32 desVel) {
	PMDresult result;
	PMDuint32 bytes2Send = 4;
	char label[2] = { '<', 'Z' };
	char endlabel[4] = { '>', '>', '>', '>' };
	PMDint32 tArr[1] = { time }, pArr[1] = { pos }, vArr[1] = { vel }, fArr[1] = { force }, dArr[1] = { desVel };
	PMDint16 mArr[2] = { motCom, 0x0000 };

	// Sends Time
	if ((sendMode & 0b00000001) == 0b00000001) {
		PMDprintf("Send Time\r\n");
		label[1] = 'T';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &tArr, 4, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Position
	if ((sendMode & 0b00000010) == 0b00000010) {
		PMDTaskWait(100);
		PMDprintf("Send Position\r\n");
		label[1] = 'P';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &pArr, 4, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Velocity
	if ((sendMode & 0b00000100) == 0b00000100) {
		PMDTaskWait(100);
		PMDprintf("Send Velocity\r\n");
		label[1] = 'V';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &vArr, 4, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Force
	if ((sendMode & 0b00001000) == 0b00001000) {
		PMDTaskWait(100);
		PMDprintf("Send Force\r\n");
		label[1] = 'F';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &fArr, 4, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Motor Command
	if ((sendMode & 0b00010000) == 0b00010000) {
		PMDTaskWait(100);
		PMDprintf("Send Motor Command\r\n");
		label[1] = 'M';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &mArr, 4, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Desired Velocity
	if ((sendMode & 0b00100000) == 0b00100000) {
		PMDTaskWait(100);
		PMDprintf("Send Desired Velocity\r\n");
		label[1] = 'D';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &dArr, 4, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
}
