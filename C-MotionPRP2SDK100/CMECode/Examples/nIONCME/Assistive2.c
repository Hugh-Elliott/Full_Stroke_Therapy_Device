#include "Assistive2.h"
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

PMDresult Assistive2(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLOAD) {

	PMDresult result = PMD_ERR_OK;

	int run = 01, save = 01, fullArray = 0, firstRun = 1, loadCell = 0;
	PMDuint32 binSave = 0b00111111;
	PMDuint16 instructionerror;
	int AverageCount = 1;

	if (arraySize > 100) {
		fullArray = 1;
	}
	else {
		fullArray = 0;
	}

	// Scaling values
	double motScale = 32768.0 / 100; // motor command scale factor
	double Vscale = 0.000359657; // Vscale used to convert analog to voltage
	double velScale = 65536; // used for get velocity command
	double volt2PWM = (100 / 24.0); // voltage to pwm

	// Command values
	PMDint16 command = 0, max;
	PMDint32 comInput = comConfig._desVel; // comInput in counts/sec
	int count = 0, i = 0, countInput = comConfig._rep, dir = 1;
	int Tneg = 0, pVelN = 0, nVelN = 0;
	int waitPWM = 0;

	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 50;
	PMDint32 posMax = comConfig._endPos, posMin = comConfig._startPos;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 50, posBac = posMax - 50;

	// Velocity values
	PMDint32 tempVel = 0, pVelMax = -100, pVelMin = 9999, nVelMax = -100, nVelMin = 9999, pVelAve = 0, nVelAve = 0, comTemp = 0, controlOutput = 0;
	PMDint32 vel = 0, velPred = 0, velError = 0;
	PMDint32 oldVel[11] = { 0, 0, 0, 0, 0, 0 };

	// Temporary torque variables
	double Tcvf = 0; // T coulomb and viscous friction
	double Ta = 0; // I*alpha
	double inertia = 0.218;
	double cfOffset = 2.76; // T coulomb offset
	double vfScale = 1 / 8.981; // viscous friction gain

	comTemp = comInput;
	double back = 3000.0 * 74 / 4000; // no longer needed
	double desiredVel = (comInput / (10 * (4000.0 / 74))), velAverage = 0;
	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;// | PMDOperatingModeCurrentControlEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	// 
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, serTime, comTIME;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	//PMDprintf("Sample time = %d microseconds\r\n", sampleTime);
	//PMDprintf("Start time = %d\r\n", startTime);
	cycTime = startTime;
	int serTimeMS = 200, timeMS = 1;
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
	int pOffset = 10000, vOffset = 20000, fOffset = 1000000000, mOffset = 15000;

	//Arrays for writing
	PMDuint32 TIME[arraySize] = { 0 };
	PMDint32 POS[arraySize] = { pOffset };
	PMDint32 VeL[arraySize] = { vOffset };
	PMDint32 force[arraySize] = { fOffset };
	PMDint16 motCom[arraySize] = { mOffset };
	PMDint32 DesVeL[arraySize] = { 0 };
	PMDint32 accelleration[30];
	PMDint32 oldaccel = 0;

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
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp+pOffset;
	motCom[i] = motTemp+mOffset;
	VeL[i] = 0+vOffset;
	force[i] = 0+fOffset;
	DesVeL[i] = 100 * desiredVel + vOffset;
	if (fullArray == 0) {
		accelleration[i] = 0;
		SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
	}
	i++;

	command = motScale * comInput;
	if (abs(command) > (motScale * 30)) command = motScale * 30;
	PMDprintf("Starting Assistive Mode\r\n");
	//PMDprintf("Command at 20 = %d\r\n", command);
	//PMDprintf("scale * 20 = %d\r\n", (int)(motScale * 20));
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		state = nextState;
		switch (state)
		{
		case 1: // Wait for positive direction motion
			if (posWaitEntry) {
				PMD_RESULT(PMDSetMotorCommand(hAxis1, waitPWM*motScale));
				//PMDprintf("Start pushing in the positive direction\r\n");
				//PMDprintf("Count: %d\r\n", count);
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
				//PMDprintf("Applying torque (positive)\r\n");
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
				PMD_RESULT(PMDSetMotorCommand(hAxis1, -waitPWM*motScale));
				//PMDprintf("Start pushing in the negative direction\r\n");
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
				//PMDprintf("Applying torque (negative)\r\n");
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
				POS[i] = posTemp + pOffset;
				motCom[i] = (motTemp)+mOffset;
				if (fullArray) {
					VeL[i] = ((POS[i] - POS[i - 1]) / (PMDint32)(TIME[i] - TIME[i - 1])) + vOffset;
				}
				else {
					if (i == 4 && firstRun == 0) {
						VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14])) + vOffset;
						accelleration[i] = ((VeL[4] - VeL[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14]));
					}
					else {
						VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
						accelleration[i] = ((VeL[i] - VeL[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1]));
					}
					for (int ov = AverageCount; ov > 0; ov--) {
						oldVel[ov] = oldVel[ov - 1];
					}
					oldVel[0] = abs(VeL[i] - vOffset);
					oldaccel = abs(accelleration[i]);
				}
				force[i] = (dir * 10 + fOffset);
				if (loadCell) {
					force[i] = (getUnits(hPeriphLOAD, 1) + fOffset);
				}
				DesVeL[i] = (100 * desiredVel) + vOffset;
				if (fullArray == 0) {
					SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
				}
				i++;
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
				PMDGetInstructionError(hAxis1, &instructionerror);
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
		if (i > AverageCount) {
			/*
			if (fullArray) {
				for (int j = i - AverageCount; j < i; j++) {
					velAverage += abs(VeL[j] - vOffset);
				}
			}
			else {
				for (int ov = 0; ov < AverageCount; ov++) {
					velAverage += abs(oldVel[ov]);
				}
			}
			*/
			//velAverage = velAverage / AverageCount;
			velAverage = abs(oldVel[0]);
			PMDprintf("velAverage = %f, velPred = %d\r\n", velAverage, velPred);
			//PMDprintf("velAverage (%d) < comTemp (%d)\r\n", (velAverage), (desiredVel));
			if ((abs(velAverage)- velPred) < (1 * desiredVel)) {
				//controlOutput = 5 + (vel2PWM * ((comTemp - (1000 * abs(velAverage)))));  // old system
				velError = (desiredVel - (velAverage - velPred));
				 //0.4297 * (desiredVel - (velAverage- velPred)) + 11.5;// new system
				controlOutput = 0.4297 * (desiredVel - (velAverage - velPred)) + 11.5;
				command = motScale * controlOutput; // output = scale factor * ((desired speed - (average actual speed)) * gain)
				/*
				if (abs(command) > (motScale * 40)) {
					command = motScale * 40;
				}
				*/
				if (velAverage - velPred > 0) {
					velPred = desiredVel - (velAverage - velPred);
				}
				else {
					velPred = desiredVel - velAverage;
				}
				//if (abs(command) < (scale * 15)) command = motScale * 15;
				//PMDprintf("Command = %d\r\n", command);
				//PMDprintf("dir = %d\r\n", dir);
				//PMD_RESULT(PMDGetTime(hAxis1, &comTIME));
			//}
			}
			
			else {
				command = 12.5;
			}
		}

		if (count >= countInput) {
			run = 0;
		}
		if (i >= arraySize) { // temporary until memory issues resolved
			run = 0;
			//i = 0;
			//PMDprintf("Exit due to time\r\n");
		}
		if (fullArray == 0 && i == 15) {
			firstRun = 0;
			i = 4;
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((float)(0.001 * (curTime - serTime) * sampleTime) >= serTimeMS) {
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
			if (comConfig._desVel != comInput) {
				comInput = comConfig._desVel;
				desiredVel = comInput / (10 * (4000 / 74));
			}
			if (comConfig._mode != 0x03) {
				run = 0;
			}
			serTime = curTime;
			//PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			//SendBasic(hPeriphSer, 0x03, posTemp);
		}
		
	}
	//PMDprintf("tneg = %d\r\n", Tneg);
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos & resNeg & resMoCo)));

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
		if (fullArray) {
			Send(hPeriphSer, binSave, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
		}
		else {
			Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
		}
	}

	// getvelocity = counts/cyc
	// ((counts/cyc) / sampleTime) * 10^6 = counts/sec
	// 2000 counts ~ 37 mm
	// counts/sec * (37/2000) = mm/sec
	// mm/sec * 0.1 = cm/sec
	PMDprintf("DesVel = %f\r\n", desiredVel);
	return result;
}