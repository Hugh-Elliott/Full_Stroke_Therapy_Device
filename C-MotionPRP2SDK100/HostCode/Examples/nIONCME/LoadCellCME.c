#include "LoadCellCME.h"
#include "LoadCellFunctions.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "ComFunctions.h"
/* Rabbit hole of maybe useful things
	N-ION User Manual
	4.5 I/O Functions
	4.6.4.2 Expansion SPI Port aka muxxing
	4.10.2 Pin Multiplexing Control
*/


PMDresult LoadCellCME(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLoad) {
	PMDresult result = PMD_ERR_OK;

	int run = 01, save = 0, tareOn = 0, calOn = 0;

	PMDint8 TEMP[3] = {0};
	int inc = 0, end = 200, p, manual = 0;
	PMDuint32 buffer = 10;
	int sensordata = 0, offset = 0, check = 0;

	PMDprintf("Starting Load Cell Test CME\r\n");
	int w[8] = { 10,20,50,100,200,500,1000,1500 }; int wSize = sizeof(w) / sizeof(w[0]);

	PMDuint16 clockDATA = 0x00000001;
	PMDresult ReadRes = PMD_ERR_OK;

	// Time variables
	PMDuint32 curTime = 0, serTime = 0, sampleTime;
	int serTimeMS = comMan._serTime;
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	serTime = curTime;

	// Operating Mode
	PMD_RESULT(PMDSetOperatingMode(hAxis1, 0));

	// Reseting offset and scale
	//setTare(0);
	//setScale(1.f);
	
	if (tareOn) {
		PMDprintf("Zeroing scale...\r\n");
		tareLoadCell(hPeriphLoad, 20);
		check = getTare() / 10;

		do
		{
			sensordata = getValue(hPeriphLoad, 1);
			PMDTaskWait(500);
		} while (abs(sensordata) < abs(check));
		PMDTaskWait(500);
	}
	if (calOn) {
		PMDprintf("Calibrating scale...\r\n");
		calibrateScale(hPeriphLoad, (-1000 * 9.81), 20);
		/*
		for (int ww = wSize; ww < wSize; ww++) {
			PMDTaskWait(500);
			PMDprintf("Calibrating scale...\r\n");
			calibrateScale(hPeriphLoad, w[ww], 20);
		}
		*/
	}
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
		else sensordata = getUnits(hPeriphLoad, 1);
		PMDprintf("Reading: %d\r\n", sensordata);
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
#ifdef CME
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - serTime) * sampleTime) >= serTimeMS) {
			Take(hPeriphSer);
			if (comConfig._mode != 66) {
				run = 0;
			}
			PMD_RESULT(PMDGetTime(hAxis1, &serTime));
		}
#endif // CME
	}
	PMDuint16 STAT = 0; //TEMP = 0;
	PMDGetEventStatus(hAxis1, &STAT);
	PMDprintf("Status = %d\r\n", STAT);
	PMDprintf("Stopping\r\n");
	return result;

}