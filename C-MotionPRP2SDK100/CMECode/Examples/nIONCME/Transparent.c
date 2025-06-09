#include "Transparent.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "ComFunctions.h"
#include "LoadCellFunctions.h"

#define arraySize 30
// 5 arrays can handle 300
// 4 arrays can handle 400 before memory overload

PMDresult Transparent(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLOAD) {

	PMDresult result = PMD_ERR_OK;

	int run = 01, save = 01, firstRun = 1, loadCell = 01;
	PMDuint32 binSave = 0b00011111;
	PMDuint16 instructionerror;

	// Scaling values
	double motScale = 32768.0 / 100; // motor command scale factor
	double velScale = 65536; // used for get velocity command

	// Command values
	PMDint16 command = 0, max;
	double comInput = 1000 * comConfig._torq / motScale; // comInput in 
	int count = 0, i = 0, countInput = comConfig._rep, dir = 1;

	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 0;
	PMDint32 posMax = comConfig._endPos, posMin = comConfig._startPos;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 50, posBac = posMax - 50;

	// Velocity values
	PMDint32 tempVel = 0, controlOutput = 0;
	PMDint32 vel = 0, velPred = 0, velError = 0;

	// Operating mode mask
	PMDuint16 OPMODE = 0;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	// 
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, serTime, comTIME, loadCycTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));

	cycTime = startTime;
	int serTimeMS = comMan._serTime, timeMS = 1, loadTimeMS = loadCellConfig._loadTime;
	int timeCounts = timeMS * 1000 / sampleTime, serTimeCounts = serTimeMS * 1000 / sampleTime, loadTimeCounts = loadTimeMS * 1000 / sampleTime;
	
	//  Setting operating mode
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);
	max = 100 * motScale;

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

	for (int zzz = 0; zzz < arraySize; zzz++) {
		POS[zzz] = pOffset;
		VeL[zzz] = vOffset;
		force[zzz] = fOffset;
		motCom[zzz] = mOffset;
		DesVeL[zzz] = vOffset;
	}

	PMDint32 posTemp = 0;
	PMDint16 motTemp = 0;

	// Size of arrays
	int tSize = 0, pSize = 0, vSize = 0, mSize = 0, fSize = 0, dSize = 0;

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int posWaitEntry = 1, posEntry = 1, negWaitEntry = 1, negEntry = 1;

	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	cycTime = curTime;
	TRACESTART = curTime;
	comTIME = curTime;
	serTime = curTime;
	loadCycTime = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp + pOffset;
	motCom[i] = motTemp + mOffset;
	VeL[i] = 0 + vOffset;
	force[i] = (0 + fOffset);
	DesVeL[i] = 0 + vOffset;

	SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
	i++;

	command = motScale * comInput;

	PMDprintf("Starting Transparent Mode\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		state = nextState;
		switch (state)
		{
		case 1: // Wait for positive direction motion
			if (posWaitEntry) {
				dir = 0;
				posWaitEntry = 0;
				PMDprintf("State = %d\r\n", state);
			}
			if (pos >= posFor)
			{
				nextState = 2;
				posWaitEntry = 1;
			}
			break;
		case 2: // Torque for pos
			if (posEntry) {
				dir = 1;
				posEntry = 0;
				PMDprintf("State = %d\r\n", state);
			}
			if (pos <= posMin)
			{
				nextState = 1;
				posEntry = 1;
			}
			else if (pos >= posMax) {
				nextState = 3;
				posEntry = 1;
			}
			break;
		case 3: // Wait for negative direction motion
			if (negWaitEntry) {
				dir = 0;
				negWaitEntry = 0;
				PMDprintf("State = %d\r\n", state);
			}
			if (pos <= posBac) {
				nextState = 4;
				negWaitEntry = 1;
			}
			break;
		case 4: // Torque for neg
			if (negEntry) {
				dir = -1;
				negEntry = 0;
				PMDprintf("State = %d\r\n", state);
			}
			if (pos >= posMax)
			{
				nextState = 3;
				negEntry = 1;
			}
			else if (pos <= posMin) {
				nextState = 1;
				count++;
				negEntry = 1;
			}
			break;
		}

		if (count >= countInput) {
			run = 0;
			PMDprintf("Exit due to count\r\n");
			//PMDprintf("count = %d, countInput = %d\r\n", count, countInput);
		}

		if (save) {
			// records vector values
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			//PMDprintf("Save time = %d, cycTime = %d, timeCounts = %d\r\n", (curTime - cycTime), cycTime, timeCounts);
			if ((curTime - cycTime) >= timeCounts) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));

				TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
				POS[i] = posTemp + pOffset;
				motCom[i] = (motTemp)+mOffset;

				if (i == 4 && firstRun == 0) {
					VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14])) + vOffset;
				}
				else {
					VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
				}
				if (loadCell) {
					if ((curTime - loadCycTime) >= loadTimeCounts) {
						force[i] = (getUnits(hPeriphLOAD, 1) + fOffset);
						loadCycTime = curTime;
					}
					else {
						if (i == 4 && firstRun == 0) {
							force[i] = force[14];
						}
						else {
							force[i] = force[i - 1];
						}
					}
				}
				else {
					force[i] = (dir * 10 + fOffset);
				}
				if (binSave & 0b00100000) {
					DesVeL[i] = (0) + vOffset;
				}
				SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
				i++;
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
				PMDGetInstructionError(hAxis1, &instructionerror);
			}
		}

		if (i == 15) {
			firstRun = 0;
			i = 4;
		}

		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((curTime - serTime) >= serTimeCounts) {
			Take(hPeriphSer);
			if (comConfig._rep != countInput) {
				countInput = comConfig._rep;
			}
			if (comConfig._startPos != posMin) {
				if (comConfig._startPos > posPLim) {
					comConfig._startPos = posPLim;
				}
				posMin = comConfig._startPos;
				posFor = posMin + 50;
			}
			if (comConfig._endPos != posMax) {
				if (comConfig._endPos < posNLim) {
					comConfig._endPos = posNLim;
				}
				posMax = comConfig._endPos;
				posBac = posMax - 50;
			}
			if (comConfig._torq != comInput) {
				comInput = 1000 * comConfig._torq / motScale;
			}
			if (comConfig._mode != 99) {
				run = 0;
			}
			serTime = curTime;
		}
	}
	PMDprintf("Finished Transparent\r\n");
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));

	if (save)
	{
		tSize = sizeof(TIME);
		pSize = sizeof(POS);
		vSize = sizeof(VeL);
		mSize = sizeof(motCom);
		fSize = sizeof(force);
		dSize = sizeof(DesVeL);
		PMDTaskWait(1000);
		Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);

	}

	return result;
}