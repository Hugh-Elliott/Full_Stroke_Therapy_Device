#include "LoadCellTest2.h"
#include "LoadCellFunctions.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
/* Rabbit hole of maybe useful things
	N-ION User Manual
	4.5 I/O Functions
	4.6.4.2 Expansion SPI Port aka muxxing
	4.10.2 Pin Multiplexing Control
*/

#include <iostream>
#include <fstream>
using namespace std;

PMDresult LoadCellTest2(PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLoad) {
	PMDresult result = PMD_ERR_OK, sendres = PMD_ERR_OK, recres = PMD_ERR_OK;
	int run = 01, Write = 0, debug = 0, save = 0;
	PMDint8 TEMP[3] = {}, TEMPBUFFER;
	int inc = 0, end = 200, p, manual = 0;
	PMDuint32 buffer = 10;
	int sensordata = 0, offset = 0, check = 0;
	int senarray[200];
	PMDprintf("Starting Load Cell Test\r\n");
	if (debug) end = 10;
	PMDprintf("hAxis1 address: %p, hPeriphLoad address: %p\r\n", hAxis1, hPeriphLoad);
	int w[8] = { 10,20,50,100,200,500,1000,1500 }; int wSize = sizeof(w) / sizeof(w[0]);
	if (hAxis1 == NULL || hPeriphLoad == NULL) {
		PMDprintf("Invalid handles: hAxis1 or hPeriphLoad is NULL.\r\n");
	}
	if (Write) {
		PMDprintf("Write Mode\r\n");
	}
	else {
		if (debug)
		{
			PMDprintf("Send Debug Mode\r\n");
		}
		else
		{
			PMDprintf("Send Normal Mode\r\n");
		}
	}
	PMDuint16 clockDATA = 0x00000001;
	PMDresult ReadRes = PMD_ERR_OK;
	/*
	PMDprintf("Zeroing scale...\r\n");
	tareLoadCell(hPeriphLoad, 20);
	check = getTare() / 10;
	
	do
	{
		sensordata = getValue(hPeriphLoad, 1);
		PMDTaskWait(500);
	} while (abs(sensordata) < abs(check));
	PMDTaskWait(500);
	
	PMDprintf("Calibrating scale...\r\n");
	calibrateScale(hPeriphLoad, -1000, 20);

	for (int ww = wSize; ww < wSize; ww++) {
		PMDTaskWait(500);
		PMDprintf("Calibrating scale...\r\n");
		calibrateScale(hPeriphLoad, w[ww], 20);
	}
	*/
	float scale = getScale();
	check = getTare();
	PMDprintf("Scale = %f, Offset = %d\r\n", scale, check);
	while (run) {
		if (manual) {
			PMD_RESULT(PMDPeriphSend(hPeriphLoad, &clockDATA, 2, 200));
			PMDPeriphReceive(hPeriphLoad, &TEMP, &buffer, sizeof(TEMP), 200);
			sensordata = TEMP[1] << 16;
			sensordata |= TEMP[1] << 8;
			sensordata |= TEMP[2];
			if (TEMP[0] & 0x80) sensordata |= 0xFF << 24;
					}
		else sensordata = getUnits(hPeriphLoad,1);
		PMDprintf("Reading: %d\r\n", sensordata);
		PMDGetErrorMessage(sendres);
		PMDGetErrorMessage(recres);
		PMDTaskWait(200);
		if (inc > end) {
			PMDprintf("\r\n");
			inc = 0;
			if (save) {
				run = 0;
			}
			else run = 0;
		}
		inc = inc + 1;
		switch (sendres) // Used to check if there was a timeout when sending
		{
		case PMD_ERR_Timeout:
			PMDprintf("Send timeout\r\n");
			break;
		case PMD_ERR_OK:
			//PMDprintf("Send OK\r\n");
			break;
		default:
			PMDprintf("Send Other Error\r\n");
			break;
		}
		switch (recres)
		{
		case PMD_ERR_Timeout:
			PMDprintf("Recieve timeout\r\n");
			break;
		case PMD_ERR_OK:
			//PMDprintf("Recieve OK\r\n");
			break;
		default:
			PMDprintf("Recieve Other Error\r\n");
			break;
		}
	}
	if (save) {
		ofstream MyFile("s750.m");
		int size = sizeof(senarray) / sizeof(senarray[0]);
		MyFile << ("sen750 = [");
		for (int j = 0; j < size; j++) {
			MyFile << senarray[j];
			if (j != size - 1) {
				MyFile << ", ";  // Add comma if it's not the last element
			}
		}
		MyFile << "];" << std::endl;

		MyFile.close();
	}
	PMDuint16 STAT = 0; //TEMP = 0;
	PMDGetEventStatus(hAxis1, &STAT);
	PMDprintf("Status = %d\r\n", STAT);
	PMDprintf("Stopping\r\n");
	return result;

}