#include "Admittance2.h"
#include "C-Motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "PMDperiph.h"
#include "ComFunctions.h"
#include "LoadCellFunctions.h"
#include <math.h>

#define arraySize 400

PMDresult Admittance2(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {

	PMDresult result = PMD_ERR_OK;

	int run = 01, save = 01, fullArray = 01;
	int count = 0, countInput = comConfig._rep, timeMS = 20, i = 0, dir = 1;

	double velScale = 65536; // used for get velocity command
	double velScaleFactor = 33423 / 10000.0; // Should convert cm/sec to be used in Setvelocity

	// Status
	PMDuint16 status = 0;

	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 50;
	PMDint32 pos1 = comConfig._endPos, pos2 = comConfig._startPos, destpos;
	if (pos1 > posPLim) pos1 = posPLim;
	if (pos2 < posNLim) pos2 = posNLim;

	// Velocity
	PMDint32 Defvelocity = comConfig._desVel, vel = 0, velGet = 0;

	// Analog value
	PMDint16 analogvalue = 0;

	// Operating mode mask
	PMD_RESULT(PMDSetOperatingMode(hAxis1, PMDOperatingModeAllEnabled));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDUpdate(hAxis1));

	// Event mask
	PMDuint16 eventmask = PMDEventStatusMotionComplete | PMDEventStatusMotionError | PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit;

	// Set profile mode
	PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal);
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDSetVelocity(hAxis1, (Defvelocity * velScaleFactor));      // former 32768
	PMDSetAcceleration(hAxis1, 66847);  // former 256
	PMDSetJerk(hAxis1, 65535);
	PMD_RESULT(PMDUpdate(hAxis1));

	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, TRACESTART, serTime, waitTime, cycTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	PMDprintf("Sample time = %d microseconds\r\n", sampleTime);
	PMDprintf("Start time = %d\r\n", startTime);
	int wait = 500; // wait time in ms
	int serTimeMS = 200;
	// Reset Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE, resMoEr = 0xFFEF;
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos | resNeg | resMoCo)));
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	//PMD_RESULT(PMDSetPosition(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos | resNeg | resMoCo)));

	//Arrays for writing
	PMDuint32 TIME[arraySize] = { 0 };
	PMDint32 POS[arraySize] = { 10000 };
	PMDint32 VeL[arraySize] = { 10000 };
	PMDint32 force[arraySize] = { 10000 };
	PMDint16 motCom[1] = { 0 };
	PMDint32 DesVeL[1] = { 0 };

	for (int zzz = 0; zzz < arraySize; zzz++) {
		POS[zzz] = 10000;
		VeL[zzz] = 10000;
		force[zzz] = 10000;
	}

	PMDint16 motTemp = 0;
	PMDint32 posTemp = 0;

	// Size of arrays
	int tSize = 0, pSize = 0, vSize = 0, mSize = 0, fSize = 0, dSize = 0;

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int inputEntry = 1, posEntry = 1, negEntry = 1, nWaitEntry = 1, pWaitEntry = 1;
	int go = 01;

	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	TRACESTART = curTime;
	waitTime = curTime;
	cycTime = curTime;
	serTime = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));

	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp+10000;
	motCom[i] = motTemp+5000;
	VeL[i] = 0+10000;
	force[i] = 0+10000;
	DesVeL[i] = 0;
	i++;

	PMDprintf("Starting Passive Mode\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		state = nextState;
		PMD_RESULT(PMDGetEventStatus(hAxis1, &status));
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		switch (state) {
		case 1: // Waits for user to hit go button on GUI
			if (inputEntry) {
				PMDprintf("Press the Go! button to start\r\n");
				PMDprintf("Count: %d\r\n", count);
				inputEntry = 0;
			}
			if (go) { // for now it will be set to true by default
				nextState = 2;
				inputEntry = 1;
				go = 0;
			}
			break;
		case 2: // Movement in positive direction
			if (posEntry) {
				destpos = pos1;
				PMDprintf("Moving to %d\r\n", destpos);
				PMDprintf("Count: %d\r\n", count);
				PMD_RESULT(PMDSetPosition(hAxis1, destpos));
				dir = 1;
				posEntry = 0;
			}
			if (status & eventmask) {
				posEntry = 1;
				// add here
				if (status == PMDEventStatusMotionComplete) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resMoCo));
					nextState = 3;
				}
				else if (status == PMDEventStatusMotionError) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resMoEr));
					return PMD_ERR_MP_MoveWhileInError;
				}
				else if (status == PMDEventStatusInPositiveLimit) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resPos));
					return PMD_ERR_MP_InvalidMoveIntoLimit;
				}
			}

			break;
		case 3: // Wait X ms then switch to negative
			if (nWaitEntry) {
				PMD_RESULT(PMDGetTime(hAxis1, &waitTime));
				nWaitEntry = 0;
			}
			if ((float)(0.001 * (curTime - waitTime) * sampleTime) >= wait) {
				nextState = 4;
				nWaitEntry = 1;
			}
			break;
		case 4: // Movement in negative direction
			if (negEntry) {
				destpos = pos2;
				PMDprintf("Moving to %d\r\n", destpos);
				PMD_RESULT(PMDSetPosition(hAxis1, destpos));
				dir = -1;
				negEntry = 0;
			}
			if (status & eventmask) {
				negEntry = 1;
				// add here
				if (status == PMDEventStatusMotionComplete) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resMoCo));
					nextState = 5;
				}
				else if (status == PMDEventStatusMotionError) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resMoEr));
					return PMD_ERR_MP_MoveWhileInError;
				}
				else if (status == PMDEventStatusInNegativeLimit) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resNeg));
					return PMD_ERR_MP_InvalidMoveIntoLimit;
				}
			}
			break;
		case 5: // Wait X ms then switch to positive
			if (pWaitEntry) {
				PMD_RESULT(PMDGetTime(hAxis1, &waitTime));
				pWaitEntry = 0;
			}
			if ((float)(0.001 * (curTime - waitTime) * sampleTime) >= wait) {
				nextState = 2;
				pWaitEntry = 1;
				count++;
				if (count >= countInput) {
					run = 0;
					nextState = 5;
				}
			}
			break;
		}

		// records vector values
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - cycTime) * sampleTime) >= timeMS) {
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
			PMD_RESULT(PMDGetVelocity(hAxis1, &velGet));

			TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
			POS[i] = posTemp + 10000;
			motCom[0] = motTemp;
			VeL[i] = ((POS[i] - POS[i - 1]) / (PMDint32)(TIME[i] - TIME[i - 1]))+10000;
			force[i] = 00 + 10000;
			DesVeL[0] = round(dir * (double)((double)(velGet / velScale) / sampleTime) * 1000) + 10000;
			i++;
			PMD_RESULT(PMDGetTime(hAxis1, &cycTime));

		}

		// Updates Velocity
		PMD_RESULT(PMDPeriphRead(hAnlgPeriph, &analogvalue, PMDPIO_AICh1, 1));//
		vel = (PMDint32)((((float)analogvalue / 14000) + .5) * (Defvelocity * velScaleFactor));
		PMD_RESULT(PMDSetVelocity(hAxis1, (vel)));
		//PMDprintf("vel * velScale = %f\r\n", (vel*velScale));
		//PMDprintf("vel = %d\r\n", vel);

		if (count >= countInput) {
			run = 0;
		}
		if (i >= arraySize) {
			run = 0;
			//i = 0;
			PMDprintf("Exit due to time\r\n");
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - serTime) * sampleTime) >= serTimeMS) {
			Take(hPeriphSer);
			if (comConfig._rep != countInput) countInput = comConfig._rep;
			if (comConfig._endPos != pos1) {
				if (comConfig._endPos > posPLim) comConfig._endPos = posPLim;
				pos1 = comConfig._endPos;
			}
			if (comConfig._startPos != pos2) {
				if (comConfig._startPos < posNLim) comConfig._startPos = posNLim;
				pos2 = comConfig._startPos;
			}
			if (comConfig._desVel != Defvelocity) {
				Defvelocity = comConfig._desVel;
				PMD_RESULT(PMDSetVelocity(hAxis1, (Defvelocity * velScaleFactor)));
			}
			if (comConfig._mode != 0x02 && comConfig._mode != 0x07) run = 0;
			if (comConfig._mode == 0x07) go = 1;
			serTime = curTime;
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			SendBasic(hPeriphSer, 2, posTemp);
		}
	}
	//PMD_RESULT(PMDSetVelocity(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos | resNeg | resMoCo)));

	for (int zzz = 2; zzz < arraySize; zzz++) {
		if (TIME[zzz] == 0) TIME[zzz] = TIME[zzz - 1] + timeMS;
	}

	if (save) {
		tSize = sizeof(TIME);
		pSize = sizeof(POS);
		vSize = sizeof(VeL);
		mSize = sizeof(motCom);
		fSize = sizeof(force);
		dSize = sizeof(DesVeL);
		PMDTaskWait(1000);
		Send(hPeriphSer, 0b00001111, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
	}

	return result;
}