#include "StepResponse.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "ComFunctions.h"

#define arraySize 20

PMDresult StepResponse(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1) {
	PMDresult result = PMD_ERR_OK;

	int run = 1, save = 01, dir = 0, firstRun = 1;
	PMDuint32 binSave = 0b00011111;

	// Command values
	PMDint16 command = comConfig._torq;
	
	// Time variables
	PMDuint32 startTime, curTime, saveTime, TRACESTART, serTime, cycTime, sampleTime;
	int saveTimeMS = 1, serTimeMS = comMan._serTime, cycTimeMS = 1000;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMDprintf("Start time = %d\r\n", startTime);
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	PMDprintf("Command = %d\r\n", command);
	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;
	
	//  Setting operating mode
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);

	// Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE;

	// Array offsets
	int pOffset = comMan._pOffset, vOffset = comMan._vOffset, fOffset = comMan._fOffset, mOffset = comMan._mOffset;

	//Arrays for writing
	PMDuint32 TIME[arraySize] = { 0 };
	PMDint32 POS[arraySize] = { pOffset };
	PMDint32 VeL[arraySize] = { vOffset };
	PMDint32 force[arraySize] = { fOffset };
	PMDint16 motCom[arraySize] = { mOffset };
	PMDint32 DesVeL[arraySize] = { 0 };
	int i = 0;

	for (int zzz = 0; zzz < arraySize; zzz++) {
		POS[zzz] = pOffset;
		VeL[zzz] = vOffset;
		motCom[zzz] = mOffset;
	}

	PMDint16 motTemp = 0;
	PMDint32 posTemp = 0;
	int tSize, pSize, vSize, fSize, mSize, dSize;

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int forwardEntry = 1, waitEntry = 1, backEntry = 1, stopEntry = 1;
	PMDprintf("Saving t = 0\r\n");
	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	cycTime = curTime;
	TRACESTART = curTime;
	serTime = curTime;
	saveTime = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));

	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp + pOffset;
	motCom[i] = motTemp + mOffset;
	VeL[i] = 0 + vOffset;
	force[i] = 0 + fOffset;
	DesVeL[i] = 0;
	SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
	PMDprintf("moo\r\n");
	i++;
	PMDprintf("Starting StepResponse.c!\r\n");
	PMDprintf("moo2\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		state = nextState;
		switch (state)
		{
		case 1: // Applies positive torque
			if (forwardEntry) {
				PMDprintf("Positive step\r\n");
				dir = 1;
				PMD_RESULT(PMDSetMotorCommand(hAxis1, dir * command));
				cycTime = curTime;
				forwardEntry = 0;
			}
			if ((float)(0.001 * abs(curTime - cycTime) * sampleTime) >= cycTimeMS) {
				nextState = 2;
				forwardEntry = 1;
				PMDprintf("deltaT = %f\r\n", ((float)(0.001 * (curTime - cycTime) * sampleTime)));
				PMDprintf("cycTime = %d, curTime = %d, sampleTime = %d\r\n", cycTime, curTime, sampleTime);
			}
			break;
		case 2: // Wait 1 second
			if (waitEntry) {
				PMDprintf("Wait\r\n");
				PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
				cycTime = curTime;
				waitEntry = 0;
			}
			if ((float)(0.001 * abs(curTime - cycTime) * sampleTime) >= cycTimeMS) {
				nextState = 3;
				waitEntry = 1;
			}
			break;
		case 3: // Applies negative torque
			if (backEntry) {
				PMDprintf("Negative step\r\n");
				dir = -1;
				PMD_RESULT(PMDSetMotorCommand(hAxis1, dir * command));
				cycTime = curTime;
				backEntry = 0;
			}
			if ((float)(0.001 * abs(curTime - cycTime) * sampleTime) >= cycTimeMS) {
				nextState = 4;
				backEntry = 1;
			}
			break;
		case 4: // Stops motor
			if (stopEntry) {
				PMDprintf("Stopping\r\n");
				PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
				cycTime = curTime;
				stopEntry = 0;
			}
			if ((float)(0.001 * abs(curTime - cycTime) * sampleTime) >= cycTimeMS) {
				run = 0;
				stopEntry = 1;
			}
			break;
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - saveTime) * sampleTime) >= saveTimeMS) {
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
			TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
			POS[i] = posTemp + pOffset;
			if (i == 4 && firstRun == 0) {
				VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14])) + vOffset;
			}
			else {
				VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
			}
			force[i] = dir + fOffset;
			motCom[i] = (motTemp)+mOffset;
			DesVeL[0] = 0;
			PMD_RESULT(PMDGetTime(hAxis1, &saveTime));
			SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
			i++;
		}
		if (i == 15) {
			firstRun = 0;
			i = 4;
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - serTime) * sampleTime) >= serTimeMS) {
			Take(hPeriphSer);
			if (comConfig._mode != 0x08) {
				run = 0;
			}
			PMD_RESULT(PMDGetTime(hAxis1, &serTime));
		}
	}
	tSize = sizeof(TIME);
	pSize = sizeof(POS);
	vSize = sizeof(VeL);
	mSize = sizeof(motCom);
	fSize = sizeof(force);
	dSize = sizeof(DesVeL);
	Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos & resNeg & resMoCo)));
}


// Step Response for negative direction
PMDresult NegStepResponse(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1) {
	PMDresult result = PMD_ERR_OK;

	int run = 1, save = 01, dir = 0, firstRun = 1;
	PMDuint32 binSave = 0b00011111;

	// Command values
	PMDint16 command = comConfig._torq;

	// Time variables
	PMDuint32 startTime, curTime, saveTime, TRACESTART, serTime, cycTime, sampleTime;
	int saveTimeMS = 1, serTimeMS = comMan._serTime, cycTimeMS = 1000;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMDprintf("Start time = %d\r\n", startTime);
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	PMDprintf("Command = %d\r\n", command);

	// Position
	PMDint32 pos = 0;
	int endSpot = 30 * (10.0 * (4000.0 / 74.0));
	if (command > 6552) {
		endSpot = 35 * (10.0 * (4000.0 / 74.0));
	}

	// Velocity
	double velScaleFactor = 33423 / 10000.0; // Should convert cm/sec to be used in Setvelocity
	PMDint32 Defvelocity = comConfig._desVel;
	
	// Set Trapezoidal Operating Mode
	PMD_RESULT(PMDSetOperatingMode(hAxis1, PMDOperatingModeAllEnabled));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDSetPosition(hAxis1, endSpot)); // Moves to x = 30+ cm, 35 cm if motor command is greater than 20 
	PMD_RESULT(PMDSetVelocity(hAxis1, (velScaleFactor * Defvelocity)));
	PMD_RESULT(PMDUpdate(hAxis1));

	do {
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		PMDTaskWait(100);
	} while (pos > endSpot + 100 || pos < endSpot - 100);
	PMDTaskWait(1000);

	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;

	//  Setting operating mode
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);

	// Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE;

	// Array offsets
	int pOffset = comMan._pOffset, vOffset = comMan._vOffset, fOffset = comMan._fOffset, mOffset = comMan._mOffset;

	//Arrays for writing
	PMDuint32 TIME[arraySize] = { 0 };
	PMDint32 POS[arraySize] = { pOffset };
	PMDint32 VeL[arraySize] = { vOffset };
	PMDint32 force[arraySize] = { fOffset };
	PMDint16 motCom[arraySize] = { mOffset };
	PMDint32 DesVeL[arraySize] = { 0 };
	int i = 0;

	for (int zzz = 0; zzz < arraySize; zzz++) {
		POS[zzz] = pOffset;
		VeL[zzz] = vOffset;
		motCom[zzz] = mOffset;
	}

	PMDint16 motTemp = 0;
	PMDint32 posTemp = 0;
	int tSize, pSize, vSize, fSize, mSize, dSize;

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int forwardEntry = 1, waitEntry = 1, backEntry = 1, stopEntry = 1;
	PMDprintf("Saving t = 0\r\n");
	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	cycTime = curTime;
	TRACESTART = curTime;
	serTime = curTime;
	saveTime = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));

	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp + pOffset;
	motCom[i] = motTemp + mOffset;
	VeL[i] = 0 + vOffset;
	force[i] = 0 + fOffset;
	DesVeL[i] = 0;
	SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
	PMDprintf("moo\r\n");
	i++;
	PMDprintf("Starting StepResponse.c!\r\n");
	PMDprintf("moo2\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		state = nextState;
		switch (state)
		{
		case 1: // Applies negative torque
			if (forwardEntry) {
				PMDprintf("negative step\r\n");
				dir = -1;
				PMD_RESULT(PMDSetMotorCommand(hAxis1, dir * command));
				cycTime = curTime;
				forwardEntry = 0;
			}
			if ((float)(0.001 * abs(curTime - cycTime) * sampleTime) >= cycTimeMS) {
				nextState = 2;
				forwardEntry = 1;
				PMDprintf("deltaT = %f\r\n", ((float)(0.001 * (curTime - cycTime) * sampleTime)));
				PMDprintf("cycTime = %d, curTime = %d, sampleTime = %d\r\n", cycTime, curTime, sampleTime);
			}
			break;
		case 2: // Wait 1 second
			if (waitEntry) {
				PMDprintf("Wait\r\n");
				PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
				cycTime = curTime;
				waitEntry = 0;
			}
			if ((float)(0.001 * abs(curTime - cycTime) * sampleTime) >= cycTimeMS) {
				nextState = 3;
				waitEntry = 1;
			}
			break;
		case 3: // Applies positive torque
			if (backEntry) {
				PMDprintf("positive step\r\n");
				dir = 1;
				PMD_RESULT(PMDSetMotorCommand(hAxis1, dir * command));
				cycTime = curTime;
				backEntry = 0;
			}
			if ((float)(0.001 * abs(curTime - cycTime) * sampleTime) >= cycTimeMS) {
				nextState = 4;
				backEntry = 1;
			}
			break;
		case 4: // Stops motor
			if (stopEntry) {
				PMDprintf("Stopping\r\n");
				PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
				cycTime = curTime;
				stopEntry = 0;
			}
			if ((float)(0.001 * abs(curTime - cycTime) * sampleTime) >= cycTimeMS) {
				run = 0;
				stopEntry = 1;
			}
			break;
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - saveTime) * sampleTime) >= saveTimeMS) {
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
			TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
			POS[i] = posTemp + pOffset;
			if (i == 4 && firstRun == 0) {
				VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14])) + vOffset;
			}
			else {
				VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
			}
			force[i] = dir + fOffset;
			motCom[i] = (motTemp)+mOffset;
			DesVeL[0] = 0;
			PMD_RESULT(PMDGetTime(hAxis1, &saveTime));
			SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
			i++;
		}
		if (i == 15) {
			firstRun = 0;
			i = 4;
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - serTime) * sampleTime) >= serTimeMS) {
			Take(hPeriphSer);
			if (comConfig._mode != 88) {
				run = 0;
			}
			PMD_RESULT(PMDGetTime(hAxis1, &serTime));
		}
	}
	tSize = sizeof(TIME);
	pSize = sizeof(POS);
	vSize = sizeof(VeL);
	mSize = sizeof(motCom);
	fSize = sizeof(force);
	dSize = sizeof(DesVeL);
	Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos & resNeg & resMoCo)));
}