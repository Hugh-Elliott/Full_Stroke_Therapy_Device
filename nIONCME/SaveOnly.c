#include "SaveOnly.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "ComFunctions.h"
#include "LoadCellFunctions.h"

#define arraySize 30

PMDresult SaveOnly(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLOAD) {

	PMDresult result = PMD_ERR_OK;

	int run = 01, firstRun = 1, loadCell = 01, i = 0, j = 0;
	PMDuint32 binSave = 0b00011111;
	PMDuint16 instructionerror;
	int AverageCount = 1;

	// Scaling values
	double motScale = 32768.0 / 100; // motor command scale factor
	double velScale = 65536; // used for get velocity command

	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	PMDprintf("Start time = %d\r\n", startTime);
	cycTime = startTime;
	int timeMS = 1, serTimeMS = comMan._serTime;
	int timeCounts = timeMS * 1000 / sampleTime, serTimeCounts = serTimeMS * 1000 / sampleTime;

	// Velocity 
	PMDint32 velTemp = 0;

	// Array offsets
	int pOffset = comMan._pOffset, vOffset = comMan._vOffset, fOffset = comMan._fOffset, mOffset = comMan._mOffset;

	//Arrays for writing
	PMDuint32 TIME[arraySize] = { 0 };
	PMDint32 POS[arraySize] = { pOffset };
	PMDint32 VeL[arraySize] = { vOffset };
	PMDint32 force[arraySize] = { fOffset };
	PMDint16 motCom[arraySize] = { mOffset };
	PMDint32 DesVeL[arraySize] = { 0 };

	for (int zzz = 0; zzz < arraySize; zzz++) {
		POS[zzz] = pOffset;
		VeL[zzz] = vOffset;
		force[zzz] = fOffset;
		motCom[zzz] = mOffset;
	}

	PMDint16 motTemp = 0;
	PMDint32 posTemp = 0;

	// Size of arrays
	int tSize = 0, pSize = 0, vSize = 0, mSize = 0, fSize = 0, dSize = 0;

	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	cycTime = curTime;
	TRACESTART = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));

	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp + pOffset;
	motCom[i] = motTemp + mOffset;
	VeL[i] = 0 + vOffset;
	force[i] = 0 + fOffset;
	DesVeL[i] = 0;
	SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
	i++;
	j++;

	PMDprintf("Start SaveOnly\r\n");
	while (run) {

		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((curTime - cycTime) >= timeCounts) {
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			PMD_RESULT(PMDGetActualVelocity(hAxis1, &velTemp));
			TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
			POS[i] = posTemp + pOffset;
			if (binSave & 0b00010000) {
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				motCom[i] = (motTemp)+mOffset;
			}
			if (i == 4 && firstRun == 0) {
				VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14])) + vOffset;
			}
			else {
				VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
			}
			force[i] = fOffset;
			if (loadCell) {
				force[i] = (getUnits(hPeriphLOAD, 1) + fOffset);
			}
			DesVeL[i] = velTemp;
			PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
			SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);			
			i++;
			j++;
		}

		if (i == 15) {
			firstRun = 0;
			i = 4;
		}

		if (j > 1000) run = 0;
	}
	PMDprintf("tempTime = %f, timeMS = %d\r\n", 0.0, timeMS);
	tSize = sizeof(TIME);
	pSize = sizeof(POS);
	vSize = sizeof(VeL);
	mSize = sizeof(motCom);
	fSize = sizeof(force);
	dSize = sizeof(DesVeL);
	PMDTaskWait(1000);

	Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);

	PMDprintf("Done!\r\n");
	return result;
}
