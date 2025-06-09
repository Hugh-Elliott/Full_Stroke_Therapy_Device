#include "Assistive2.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "ComFunctions.h"
#include "LoadCellFunctions.h"

#define arraySize 300

PMDresult Assistive2(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {

	PMDresult result = PMD_ERR_OK;

	int run = 01, save = 01, fullArray = 01;

	// Scaling values
	double motScale = 32768.0 / 100, Vscale = 0.000359657; // Vscale used to convert analog to voltage
	double velScale = 65536; // used for get velocity command
	double vel2PWM = (25.0 / 8000); // no longer needed

	// Command values
	PMDint16 command = 0, max;
	PMDint32 comInput = comConfig._desVel; // comInput in counts/sec
	//double perc = 7, temp;
	int count = 0, timeMS = 20, i = 0, countInput = comConfig._rep, dir = 1;
	int Tneg = 0, pVelN = 0, nVelN = 0;

	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 50;
	PMDint32 posMax = comConfig._endPos, posMin = comConfig._startPos;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 50, posBac = posMax - 50;

	// Velocity values
	PMDint32 tempVel = 0, pVelMax = -100, pVelMin = 9999, nVelMax = -100, nVelMin = 9999, pVelAve = 0, nVelAve = 0, comTemp = 0, controlOutput = 0;
	PMDint32 vel = 0, velAverage = 0;
	comTemp = comInput;
	double back = 3000.0 * 74 / 4000; // no longer needed
	PMDint32 desiredVel = comInput / (10 * (4000 / 74));
	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;// | PMDOperatingModeCurrentControlEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	// 
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, serTime, comTIME;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	PMDprintf("Sample time = %d microseconds\r\n", sampleTime);
	PMDprintf("Start time = %d\r\n", startTime);
	cycTime = startTime;
	int serTimeMS = 200;
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
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp+10000;
	motCom[i] = motTemp+5000;
	VeL[i] = 0+10000;
	force[i] = 0+10000;
	DesVeL[i] = comTemp;
	i++;

	command = motScale * comInput;
	if (abs(command) > (motScale * 30)) command = motScale * 30;
	PMDprintf("Starting Assistive Mode\r\n");
	PMDprintf("Command at 20 = %d\r\n", command);
	PMDprintf("scale * 20 = %d\r\n", (int)(motScale * 20));
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
				PMDprintf("Applying torque (positive)\r\n");
				posEntry = 0;
			}
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (dir * command)));
			//PMDprintf("Command * dir = %d\r\n", (dir * command));
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
			}
			if (pos <= posBac) {
				nextState = 4;
				negWaitEntry = 1;
			}
			break;
		case 4: // Torque for neg
			if (negEntry) {
				dir = -1;
				PMDprintf("Applying torque (negative)\r\n");
				negEntry = 0;
			}
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (dir * command)));
			//PMDprintf("Command * dir = %d\r\n", (dir * command));
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
		default:
			break;
		}
		if (save) {
			// records vector values
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			if ((double)(0.001 * (curTime - cycTime) * sampleTime) >= timeMS) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));

				TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
				POS[i] = posTemp + 10000;
				motCom[i] = (motTemp / motScale)+5000;
				VeL[i] = ((POS[i] - POS[i - 1]) / (PMDint32)(TIME[i] - TIME[i - 1]))+10000;
				force[i] = dir * 10 + 10000;
				DesVeL[0] = comTemp * dir / 1000;
				i++;
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
			}
		}
#ifndef CME
		// debugging
		if (i > 0) {
			tempVel = ((POS[i] - POS[i - 1]) / (PMDint32)(TIME[i] - TIME[i - 1]));
			//PMDprintf("T POS[%d] = %d\r\n", i, POS[i]);
			//PMDprintf("i = %d\r\n", i);
		}
		if ((PMDint32)(TIME[i] - TIME[i - 1]) < 0) {
			PMDprintf("Negative Time");
			Tneg++;
		}
		//PMD_RESULT(PMDGetVelocity(hAxis1, &tempVel)); 
		//PMDprintf("Velocity: %i\r\n", (tempVel));
		//PMDprintf("Vel[%d] = %d\r\n", i, VeL[i]);
		if (tempVel > 0) {
			if (tempVel > pVelMax) {
				PMDprintf("tempVel (%d) > pVelMax (%d)\r\n", tempVel, pVelMax);
				pVelMax = tempVel;
			}
			if (tempVel < pVelMin) {
				PMDprintf("tempVel (%d) < pVelMax (%d)\r\n", tempVel, pVelMin);
				pVelMin = tempVel;
			}
			pVelN++;
			pVelAve = pVelAve + tempVel;
		}
		if (tempVel < 0) {
			if (tempVel > nVelMax) {
				PMDprintf("tempVel (%d) > nVelMax (%d)\r\n", tempVel, nVelMax);
				nVelMax = tempVel;
			}
			if (tempVel < nVelMin) {
				PMDprintf("tempVel (%d) < nVelMax (%d)\r\n", tempVel, nVelMax);
				nVelMin = tempVel;
			}
			nVelN++;
			nVelAve = nVelAve + tempVel;
		}
#endif // !CME

		// attempt at variable speed
		if (i > 5) {
			for (int j = i - 5; j < i; j++) {
				velAverage = velAverage + abs(VeL[j]-10000);
			}
			velAverage = velAverage / 5;
			//velAverage = (VeL[i] + VeL[i - 1] + VeL[i - 2] + VeL[i - 3] + VeL[i - 4] + VeL[i - 5] + VeL[i - 6] + VeL[i - 7] + VeL[i - 8] + VeL[i - 9]) / 10;
			velAverage = (velAverage);
			PMDprintf("velAverage (%d) < comTemp (%d)\r\n", (velAverage), (desiredVel));
			if (abs(velAverage * 1) < (1 * desiredVel)) {
				//if (double(0.001 * (curTime - comTIME) * sampleTime) >= timeMS) {
				//controlOutput = 5 + (vel2PWM * ((comTemp - (1000 * abs(velAverage)))));  // old system
				controlOutput = 1 * (desiredVel - velAverage) + 7;// new system
				command = motScale * controlOutput; // output = scale factor * ((desired speed - (average actual speed)) * gain)
				if (abs(command) > (motScale * 30)) {
					command = motScale * 30;
				}
				//if (abs(command) < (scale * 15)) command = motScale * 15;
				//PMDprintf("Command = %d\r\n", command);
				//PMDprintf("dir = %d\r\n", dir);
				//PMD_RESULT(PMDGetTime(hAxis1, &comTIME));
			//}
			}
			
			else {
				command = 0;
			}
		}

		if (count >= countInput) {
			run = 0;
		}
		if (i >= arraySize) { // temporary until memory issues resolved
			run = 0;
			//i = 0;
			PMDprintf("Exit due to time\r\n");
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - serTime) * sampleTime) >= serTimeMS) {
			Take(hPeriphSer);
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
			if (comConfig._desVel != comTemp) comTemp = comConfig._desVel;
			if (comConfig._mode != 0x03) run = 0;
			serTime = curTime;
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			SendBasic(hPeriphSer, 0x03, posTemp);
		}
		
	}
	PMDprintf("tneg = %d\r\n", Tneg);
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

	// getvelocity = counts/cyc
	// ((counts/cyc) / sampleTime) * 10^6 = counts/sec
	// 2000 counts ~ 37 mm
	// counts/sec * (37/2000) = mm/sec
	// mm/sec * 0.1 = cm/sec
	
	return result;
}