#include "AssistiveForce.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "ComFunctions.h"
#include "LoadCellFunctions.h"
#include <math.h>

#define arraySize 30
// 5 arrays can handle 300
// 4 arrays can handle 400 before memory overload
#define IMP 1

PMDresult AssistiveForce(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLOAD) {

	PMDresult result = PMD_ERR_OK;

	// Status
	PMDuint16 status = 0, driveFaultStatus = 0, busVoltage = 0;

	int run = 01, save = 01, firstRun = 1, loadCell = 01;
	PMDuint32 binSave = 0b00011111;
	PMDuint16 instructionerror;
	
	// Scaling values
	double motScale = 32768.0 / 100; // motor command scale factor
	double Vscale = 0.000359657; // Vscale used to convert analog to voltage
	double velScale = 65536; // used for get velocity command
	double volt2PWM = (100 / 24.0); // voltage to pwm
	double counts2cm = 1 / (10 * (4000.0 / 74));

	// Command values
	PMDint16 command = 0, max;
	double comInput = 1000 * comConfig._torq / motScale; // comInput in 
	int count = 0, i = 0, j = 0, countInput = comConfig._rep, dir = 1;
	int Tneg = 0, pVelN = 0, nVelN = 0;
	int waitPWM = 8 * motScale;
	double forceError = 0, forceTemp = 0;

	// Variables for impedancce control
	double m = 1 / 8.0;
	double Md = 0.8 * m;
	double Dd = 0.0;
	double Kd = 45;
	double kp = 10;
	double kd = 0;
	//Dd = 2 * 0.7 * sqrt(Kd * Md);
	Dd = 15;
	//kd = 2 * 0.7 * sqrt(kp * m);
	kd = 20;

	double mMd = m / Md;
	double fgain = mMd - 1;
	double Fext = 0.0;
	double e = 0.0;
	double e_dot = 0.0;
	double F = 0;
	double x0 = 10000 * counts2cm;
	double Tf = 10.95;
	double x0_dot = .01 * round(counts2cm * comConfig._desVel * 100) / 100;
	PMDprintf("x0_dot = %f\r\n", x0_dot);
	
	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 0;
	PMDint32 posMax = comConfig._endPos, posMin = comConfig._startPos;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 50, posBac = posMax - 50;

	// Velocity values
	PMDint32 tempVel = 0, pVelMax = -100, pVelMin = 9999, nVelMax = -100, nVelMin = 9999, pVelAve = 0, nVelAve = 0, controlOutput = 0;
	PMDint32 vel = 0, velPred = 0, velError = 0;
	PMDint32 oldVel[11] = { 0, 0, 0, 0, 0, 0 };

	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	// 
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, serTime, comTIME, loadCycTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	//PMDprintf("Sample time = %d microseconds\r\n", sampleTime);
	//PMDprintf("Start time = %d\r\n", startTime);
	cycTime = startTime;
	int serTimeMS = comMan._serTime, timeMS = 1, loadTimeMS = loadCellConfig._loadTime;
	int timeCounts = timeMS * 1000 / sampleTime, serTimeCounts = serTimeMS * 1000 / sampleTime, loadTimeCounts = loadTimeMS * 1000 / sampleTime;
	PMDprintf("timeCounts = %d, serTimeCounts = %d\r\n", timeCounts, serTimeCounts);
	//PMDTaskWait(5000);
	
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
	
	PMDprintf("Starting Assistive Force Mode\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		state = nextState;
		switch (state)
		{
		case 1: // Wait for positive direction motion
			if (posWaitEntry) {
				PMD_RESULT(PMDSetMotorCommand(hAxis1, waitPWM));
				//PMDprintf("Start pushing in the positive direction\r\n");
				//PMDprintf("Count: %d\r\n", count);
				dir = 0;
				posWaitEntry = 0;
				//PMDprintf("State = %d\r\n", state);
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
				//PMDprintf("State = %d\r\n", state);
				x0 = posMax * counts2cm;
			}
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (command)));
			//PMDprintf("Command * dir = %d\r\n", (dir * command));
			if (pos <= posMin-100)
			{
				nextState = 1;
				posEntry = 1;
			}
			else if (pos >= posMax+100) {
				nextState = 3;
				posEntry = 1;
			}
			break;
		case 3: // Wait for negative direction motion
			if (negWaitEntry) {
				PMD_RESULT(PMDSetMotorCommand(hAxis1, -waitPWM));
				//PMDprintf("Start pushing in the negative direction\r\n");
				dir = 0;
				negWaitEntry = 0;
				//PMDprintf("State = %d\r\n", state);
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
				//PMDprintf("State = %d\r\n", state);
				x0 = posMin * counts2cm;
			}
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (-command)));
			//PMDprintf("Command * dir = %d\r\n", (dir * command));
			if (pos >= posMax+100)
			{
				nextState = 3;
				negEntry = 1;
			}
			else if (pos <= posMin-100) {
				nextState = 1;
				count++;
				negEntry = 1;
			}
			break;
		}

		if (count >= countInput) {
			run = 0;
			//PMDprintf("Exit due to count\r\n");
			//PMDprintf("count = %d, countInput = %d\r\n", count, countInput);
		}

		if (save) {
			// records vector values
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			//PMDprintf("Save time = %d, cycTime = %d, timeCounts = %d\r\n", (curTime - cycTime), cycTime, timeCounts);
			if ((curTime - cycTime) >= timeCounts) {
				//PMDprintf("Save\r\n");
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));

				TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
				POS[i] = posTemp + pOffset;
				if (state == 1 || state == 3) {
					motCom[i] = 0 + mOffset;
				}
				else {
					motCom[i] = (motTemp)+mOffset;
				}
				if (i == 4 && firstRun == 0) {
					VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14])) + vOffset;
				}
				else {
					VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
				}						
				if (loadCell) {
					if ((curTime - loadCycTime) >= loadTimeCounts) {
						forceTemp = getUnits(hPeriphLOAD, 1);
						force[i] = (forceTemp + fOffset);
						loadCycTime = curTime;
						//PMDprintf("New value\r\n");
					}
					else {
						//PMDprintf("Copied value\r\n");
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
				PMD_RESULT(PMDGetInstructionError(hAxis1, &instructionerror));
				PMD_RESULT(PMDGetEventStatus(hAxis1, &status));
				PMD_RESULT(PMDGetDriveFaultStatus(hAxis1, &driveFaultStatus))
				PMD_RESULT(PMDGetBusVoltage(hAxis1, &busVoltage));
				if (instructionerror != 0) {
					PMDGetErrorMessage(instructionerror);
				}
				//PMDprintf("Status = %x, Drive Fault = %x, Bus voltage = %d\r\n", status, driveFaultStatus, busVoltage);
			}
		}

		if (i == 15) {
			firstRun = 0;
			i = 4;
		}
#ifndef IMP
		// Updates Motor Torque based on loadcell reading
		if (loadCell) {
			forceError = comInput - abs(forceTemp);
			if (forceError >= 0) {
				controlOutput = 1.48 * .001 * forceError + 10.95;
			}
			else controlOutput = 10.95;
			command = controlOutput * motScale;
		}
#else
		if (i == 4 & firstRun == 0) j = 14;
		else j = i - 1;
		// Updates Motor Torque using Impedance Control
		Fext = 0.001 * (force[j] - fOffset);
		e = .01 * (counts2cm * (POS[j] - pOffset) - x0);
		e_dot = (10 * (VeL[j] - vOffset) * counts2cm) - dir * x0_dot;
		F = fgain * Fext - mMd * (Dd * e_dot + Kd * e) + dir * Tf;
		command = dir * F * motScale;
		PMDprintf("F = %f, Fext = %f, e = %f, e_dot = %f\r\n", F, (fgain* Fext), (e), (e_dot));
		if (abs(F) > 30) {
			if (F > 0) {
				command = 30 * motScale;
			}
		}

#endif // !IMP

		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((curTime - serTime) >= serTimeCounts) {
			//PMDprintf("Take\r\n");
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
			if (comConfig._mode != 33) {
				run = 0;
			}
			serTime = curTime;
		}
	}
	PMDprintf("Finished Force Assist\r\n");
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