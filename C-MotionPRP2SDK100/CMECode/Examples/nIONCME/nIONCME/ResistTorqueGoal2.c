#include "ResistTorqueGoal2.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "ComFunctions.h"
#include "LoadCellFunctions.h"

#define arraySize 300

PMDresult ResistTorqueGoal2(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {

	PMDresult result = PMD_ERR_OK;

	int run = 1, save = 0, dir = 0, fullArray = 01;
	int mode = comConfig._mode - 4; // Mode-> 0 = Both, 1 = Push, 2 = Pull
	PMDprintf("run = %d\r\n",run);
	// Scaling values
	double motScale = 32768.0 / 100, Vscale = 0.000359657; // Vscale used to convert analog to voltage

	// Command values
	PMDint16 command = 0, max, comInput = comConfig._torq;
	//double perc = 7, temp;
	int count = 0, timeMS = 20, i = 0, countInput = comConfig._rep;
	int serTimeMS = 200;
	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 50;
	PMDint32 posMax = comConfig._endPos, posMin = comConfig._startPos;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 50, posBac = posMax - 50;

	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;// | PMDOperatingModeCurrentControlEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	// 
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, serTime, delTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	PMDprintf("Start time = %d\r\n", startTime);
	cycTime = startTime;

	//  Setting operating mode
	PMDint32 vel = 0;
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);
	max = 100 * motScale;

	// Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE;

	//Arrays for writing

	PMDuint32 TIME[arraySize] = { 0 };
	PMDint32 POS[arraySize] = { 10000 };
	PMDint32 VeL[arraySize] = { 10000 };
	PMDint32 force[arraySize] = { 10000 };
	PMDint16 motCom[arraySize] = { 5000 };
	PMDint32 DesVeL[1] = { 0 };

	for (int zzz = 0; zzz < arraySize; zzz++) {
		POS[zzz] = 10000;
		VeL[zzz] = 10000;
		force[zzz] = 10000;
		motCom[zzz] = 5000;
	}

	PMDint16 motTemp = 0;
	PMDint32 posTemp = 0;

	// Size of arrays
	int tSize = 0, pSize = 0, vSize = 0, mSize = 0, fSize = 0, dSize = 0;

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int startEntry = 1, posWaitEntry = 1, posEntry = 1, negWaitEntry = 1, negEntry = 1;

	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	cycTime = curTime;
	TRACESTART = curTime;
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

	command = motScale * comInput;
	PMDprintf("Starting ResistTorqueGoal.c\r\n");
	PMDprintf("Command Input = %d\r\n",comInput);
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		state = nextState;
		switch (state)
		{
		case 1: // Wait for positive direction motion
			if (posWaitEntry) {
				PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
				PMDprintf("Start pushing in the positive direction\r\n");
				PMDprintf("Count: %d\r\n", count);
				dir = 0;
				posWaitEntry = 0;
				if (mode == 2) {
					PMD_RESULT(PMDSetMotorCommand(hAxis1, 15 * motScale));
				}
			}
			if (pos >= posFor)
			{
				nextState = 2;
				posWaitEntry = 1;
			}
			break;
		case 2: // Torque for pos
			if (posEntry) {
				if (mode == 2) {
					PMD_RESULT(PMDSetMotorCommand(hAxis1, 15 * motScale));
				}
				else {
					PMD_RESULT(PMDSetMotorCommand(hAxis1, -command));
				}
				PMDprintf("Applying torque (negative)\r\n");
				dir = 1;
				posEntry = 0;
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
				PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
				PMDprintf("Start pushing in the negative direction\r\n");
				dir = 0;
				negWaitEntry = 0;
				if (mode == 1) {
					PMD_RESULT(PMDSetMotorCommand(hAxis1, -15 * motScale));
				}
			}
			if (pos <= posBac) {
				nextState = 4;
				negWaitEntry = 1;
			}
			break;
		case 4: // Torque for neg
			if (negEntry) {
				if (mode == 1) {
					PMD_RESULT(PMDSetMotorCommand(hAxis1, -15 * motScale));
				}
				else {
					PMD_RESULT(PMDSetMotorCommand(hAxis1, command));
				}
				PMDprintf("Applying torque (positive)\r\n");
				dir = -1;
				negEntry = 0;
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
		default: // Start up
			if (startEntry) {

				startEntry = 0;
			}
			break;
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if (save) {
			if ((float)(0.001 * (curTime - cycTime) * sampleTime) >= timeMS) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
				POS[i] = posTemp + 10000;
				motCom[i] = (motTemp / motScale)+5000;
				delTime = (0.001 * (curTime - cycTime) * sampleTime);
				VeL[i] = ((POS[i] - POS[i - 1]) / (PMDint32)(TIME[i] - TIME[i - 1]))+10000;
				force[i] = dir + 10000;
				DesVeL[0] = 0;
				//PMDprintf("Delta T: %d. Position: %d. Motor Command: %d.\r\n", delTime, POS[i], motCom[i]);
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
				i++;
			}
		}
		if (count == countInput) {
			run = 0;
			PMDprintf("Exit due to count\r\n");
		}

		if (i >= arraySize) {
			run = 0;
			//i = 0;
			PMDprintf("Exit due to time\r\n");
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - serTime) * sampleTime) >= serTimeMS) {
			Take(hPeriphSer);
			if ((comConfig._mode - 4) != mode) mode = comConfig._mode - 4;
			if (comConfig._rep != countInput) countInput = comConfig._rep;
			if (comConfig._startPos != posMin) {
				if (comConfig._startPos > posPLim) comConfig._startPos = posPLim;
				posMin = comConfig._startPos;
				posFor = posMin + 50;
			}
			if (comConfig._endPos != posMax) {
				if (comConfig._endPos < posNLim) comConfig._endPos = posNLim;
				posMax = comConfig._endPos;
				posBac = posMax - 50;
			}
			if (comConfig._desVel != comInput) comInput = comConfig._torq;
			if (comConfig._mode != 0x04 && comConfig._mode != 0x05 && comConfig._mode != 0x06) run = 0;
			serTime = curTime;
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			SendBasic(hPeriphSer, 0x04, posTemp);
			command = comInput * motScale;
		}
	}
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos | resNeg | resMoCo)));

	for (int zzz = 2; zzz < arraySize; zzz++) {
		if (TIME[zzz] == 0) TIME[zzz] = TIME[zzz - 1] + timeMS;
	}

	if (save)
	{
		tSize = sizeof(TIME);
		pSize = sizeof(POS);
		vSize = sizeof(VeL);
		mSize = sizeof(motCom);
		fSize = sizeof(force);
		dSize = sizeof(DesVeL);
		PMDTaskWait(1000);
		Send(hPeriphSer, 0b00011111, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
	}
	
	return result;
}