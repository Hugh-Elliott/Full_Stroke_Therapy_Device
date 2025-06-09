#include "ComFunctions.h"
#include "PMDperiph.h"
#include "PMDsys.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDutil.h"

COMConfig comConfig = {0, 0, 0, 0, 0, 0};
COMmanagement comMan = {200, 10000, 20000, 1000000000, 15000};
COMImpValues comImp = {1/8.0, .05, 15, 45};

#define BUFFSIZE   200

void defaultVals(void) {
	comConfig._mode = 0;
	comConfig._rep = 3;
	comConfig._startPos = 0;
	comConfig._endPos = 17000;
	comConfig._desVel = 20000;
	comConfig._torq = 10;
}

#define START_BYTE 0xAA
#define END_BYTE   0x55

void Take(PMDPeriphHandle *hPeriphSer) {
	PMDuint32 bytesReceived, i = 0;
	PMDuint32 bytesExpected = 22;
	PMDresult result;
	char data[BUFFSIZE] = {0};
	char flag = 0;
	PMDint8 size = 0;
	PMDint32 value = 0;

	// Wait for data to be sent from the host
	result = PMDPeriphReceive(hPeriphSer, &data, &bytesReceived, bytesExpected, .0055);

	// A timeout will occur if the bytes received is less than the expected number of bytes within the timeout period.
	if (result == PMD_ERR_OK || result == PMD_ERR_Timeout)
	{
		if (bytesReceived > 0)
		{			
			//PMDprintf("%d bytes received: ", bytesReceived);

			while (i < bytesReceived) {
				flag = data[i++];
				size = data[i++];
				PMDprintf("%02x ", data[i]);
				value = 0;
				for (int j = 0; j < size; j++) {
					value = (value << 8) | data[i++];
					PMDprintf("%02x ", data[i]);
				}
				PMDprintf("Flag = %c\r\n",flag);
				switch (flag) {
				case 'M':
					comConfig._mode = value;
					break;
				case 'R':
					comConfig._rep = value;
					break;
				case 'S':
					comConfig._startPos = value;
					break;
				case 'E':
					comConfig._endPos = value;
					break;
				case 'V':
					comConfig._desVel = value;
					break;
				case 'T':
					comConfig._torq = value;
					break;
				case 'W':
					comImp._m = value * .001;
					PMDprintf("m = %d\r\n", value);
					break;
				case 'X':
					comImp._Md = value * .001;
					PMDprintf("Md = %d\r\n", value);
					break;
				case 'Y':
					comImp._Dd = value * .001;
					PMDprintf("Dd = %d\r\n", value);
					break;
				case 'Z':
					comImp._Kd = value * .001;
					PMDprintf("Kd = %d\r\n", value);
					break;
				}
			}
			PMDprintf("\r\n");
			//PMD_RESULT(PMDPeriphSend(hPeriphSer, &data, bytesReceived, 100));
		}
	}
}
// Work in progress
void Send(PMDPeriphHandle *hPeriphSer, char sendMode, PMDuint32 *time, int tSize, PMDint32 *pos, int pSize, PMDint32 *vel, int vSize, PMDint32 *force, int fSize, PMDint16 *motCom, int mSize, PMDint32 *desVel, int dvSize) {
	PMDresult result;
	PMDuint32 bytes2Send = 4;
	char label[3] = { '<', 'G', '>' };
	char endlabel[4] = { '>', '>', '>', '>' };
	char test[2] = { '<' , '>' };
	int i = 0;

	// Sends Time
	if ((sendMode & 0b00000001) == 0b00000001) {
		PMDprintf("Send Time\r\n");
		label[1] = 'T';
		//PMD_RESULT(PMDPeriphSend(hPeriphSer, &test, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
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
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
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
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, vel, vSize, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Force
	if ((sendMode & 0b00001000) == 0b00001000) {
		PMDTaskWait(100);
		PMDprintf("Send Force\r\n");
		label[1] = 'F';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, force, fSize, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Motor Command
	if ((sendMode & 0b00010000) == 0b00010000) {
		PMDTaskWait(100);
		PMDprintf("Send Motor Command\r\n");
		label[1] = 'M';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, motCom, mSize, 2000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	// Sends Desired Velocity
	if ((sendMode & 0b00100000) == 0b00100000) {
		PMDTaskWait(100);
		PMDprintf("Send Desired Velocity\r\n");
		label[1] = 'D';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, desVel, dvSize, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	}
	label[1] = 'S';
	PMDprintf("Save!\r\n");
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
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
	char label[3] = { '<', 'G', '>' };
	char endlabel[4] = { '>', '>', '>', '>' };

	PMDint16 arr[2] = { mot1, mot2 }, mot1Arr[1] = { mot1 }, mot2Arr[1] = { mot2 };

	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
	PMDTaskWait(100);
	PMDprintf("Send mot1 and mot2 seperate\r\n");
	PMDPeriphSend(hPeriphSer, mot1Arr, 2, 200);
	PMDPeriphSend(hPeriphSer, mot2Arr, 2, 200);
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	PMDTaskWait(100);
	PMDprintf("Send mot1 and mot2 as an array\r\n");
	label[1] = 'M';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
	PMDPeriphSend(hPeriphSer, &arr, 4, 200);
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
	label[1] = 'S';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, 100));
}
// Used to send 1 set of data
void SendParts(PMDPeriphHandle* hPeriphSer, char sendMode, PMDuint32 time, PMDint32 pos, PMDint32 vel, PMDint32 force, PMDint16 motCom, PMDint32 desVel) {
	PMDresult result;
	PMDuint32 bytes2Send = 4;
	char label[3] = { '<', 'G', '>' };
	char endlabel[4] = { '>', '>', '>', '>' };
	PMDint32 tArr[1] = { time }, pArr[1] = { pos }, vArr[1] = { vel }, fArr[1] = { force }, dArr[1] = { desVel };
	PMDint16 mArr[2] = { motCom, 0x0000 };

	// Sends Time
	if ((sendMode & 0b00000001) == 0b00000001) {
		//PMDprintf("Send Time\r\n");
		label[1] = 'T';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &tArr, 4, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	}
	// Sends Position
	if ((sendMode & 0b00000010) == 0b00000010) {
		//PMDTaskWait(10);
		//PMDprintf("Send Position\r\n");
		label[1] = 'P';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &pArr, 4, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	}
	// Sends Velocity
	if ((sendMode & 0b00000100) == 0b00000100) {
		//PMDTaskWait(10);
		//PMDprintf("Send Velocity\r\n");
		label[1] = 'V';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &vArr, 4, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	}
	// Sends Force
	if ((sendMode & 0b00001000) == 0b00001000) {
		//PMDTaskWait(10);
		//PMDprintf("Send Force\r\n");
		label[1] = 'F';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &fArr, 4, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	}
	// Sends Motor Command
	if ((sendMode & 0b00010000) == 0b00010000) {
		//PMDTaskWait(10);
		//PMDprintf("Send Motor Command\r\n");
		label[1] = 'M';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &mArr, 4, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	}
	// Sends Desired Velocity
	if ((sendMode & 0b00100000) == 0b00100000) {
		//PMDTaskWait(10);
		//PMDprintf("Send Desired Velocity\r\n");
		label[1] = 'D';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &dArr, 4, .1));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	}
	//PMDTaskWait(100);
}

void SendImpVars(PMDPeriphHandle* hPeriphSer, double m, double Md, double Dd, double Kd) {
	PMDresult result;
	char label[3] = { '<', 'G', '>' };
	char endlabel[4] = { '>', '>', '>', '>' };
	PMDint32 mArr[1] = { m * 1000 }, MdArr[1] = { Md * 1000 }, DdArr[1] = { Dd * 1000 }, KdArr[1] = { Kd * 1000 }, ArrTemp[1] = { 200 };
	PMDint32 temp = 0;

	//mArr[0] = m * 1000;
	//MdArr[0] = Md * 1000;
	//DdArr[0] = Dd * 1000;
	//KdArr[0] = Kd * 1000;

	PMDprintf("mArr = %d, MdArr = %d, DdArr = %d, KdArr = %d\r\n", mArr[0], MdArr[0], DdArr[0], KdArr[0]);
	
	label[1] = 'W';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &mArr, 4, .1));
	//PMD_RESULT(PMDPeriphSend(hPeriphSer, &ArrTemp, 4, .1));
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	PMDTaskWait(100);

	label[1] = 'X';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &MdArr, 4, .1));
	//PMD_RESULT(PMDPeriphSend(hPeriphSer, &ArrTemp, 4, .1));
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	PMDTaskWait(100);

	label[1] = 'Y';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &DdArr, 4, .1));
	//PMD_RESULT(PMDPeriphSend(hPeriphSer, &ArrTemp, 4, .1));
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	PMDTaskWait(100);

	label[1] = 'Z';
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 3, .1));
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &KdArr, 4, .1));
	//PMD_RESULT(PMDPeriphSend(hPeriphSer, &ArrTemp, 4, .1));
	PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, .1));
	PMDTaskWait(100);

}