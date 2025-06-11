#include "Impedance.h"
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

//#define Imp 1
//#define ADMITTANCE 1
#if !defined(Imp) && !defined(ADMITTANCE)
#define SIMPLE 1
#endif // !Imp | !ADMITTANCE


PMDresult Impedance(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLOAD) {

	PMDresult result = PMD_ERR_OK;

	int run = 01, save = 01, firstRun = 1, loadCell = 01;
	PMDuint32 binSave = 0b00111111;
	PMDuint16 instructionerror;
	int saved = 0;

	// Scaling values
	double motScale = 32768.0 / 100; // motor command scale factor
	double velScale = 65536; // used for get velocity command
	double counts2cm = 1 / (10 * (4000.0 / 74));

	// Command values
	PMDint16 command = 0, max;
	double comInput = 1000 * comConfig._torq / motScale; // comInput in 
	int count = 0, i = 0, countInput = comConfig._rep, dir = 1, j = 0;
	int waitPWM = 8 * motScale;

	// Mass-Spring-Damper Variables
	// Dynamics Eq. in the form of B(q)q_ddot + C(q,q_dot) + G(q) + Tauf(q_dot) = Torque or M(theta)theta_ddot + C(theta,theta_dot) + G(theta) + F(theta,theta_dot) = Torque
	// or is it m*x_ddot + c*x_dot + k*x = Torque (Input) + k*xdes
	int old = 0;
	double m = 1;
	double c = 0.4;
	double k = 0.2;
	double f = .001 * 0.5;
	double accel[arraySize] = { 0 };
	double pOs = 0, vEl = 0, aCc = 0, fOr = 0;
	double Tf = 10.95;

	m = comImp._m;
	double Md = comImp._Md;
	double Dd = 0.0;
	double Kd = comImp._Kd;

	double kp = 10;
	double kd = 0;
	//Dd = 2 * 0.7 * sqrt(Kd * Md);
	Dd = comImp._Dd;
	//kd = 2 * 0.7 * sqrt(kp * m);
	kd = 20;

	double mMd = m / Md;
	double fgain = mMd - 1;
	double Fext = 0.0;
	double e = 0.0;
	double e_dot = 0.0;
	double F = 0;
	double x0 = 10000 * counts2cm;

	double xd = 0;
	double xd_dot = 0;
	double xd_ddot = 0;

	// Position
	PMDint32 pos = 0, posPLim = comMan._posLimit, posNLim = 0;
	PMDint32 posMax = comConfig._endPos, posMin = comConfig._startPos;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 50, posBac = posMax - 50;

	// Velocity values
	PMDint32 tempVel = 0, controlOutput = 0;
	PMDint32 vel = 0, velPred = 0, velError = 0;

	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	// 
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, serTime, comTIME, loadCycTime, deltaT = 0;
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
	accel[i] = 0;

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
				//PMDprintf("State = %d\r\n", state);
				PMD_RESULT(PMDSetMotorCommand(hAxis1, waitPWM));
				x0 = posMin * counts2cm;
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
				//PMDprintf("State = %d\r\n", state);
			}
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (command)));
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
				dir = 0;
				negWaitEntry = 0;
				//PMDprintf("State = %d\r\n", state);
				PMD_RESULT(PMDSetMotorCommand(hAxis1, -waitPWM));
				x0 = posMax * counts2cm;
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
				//PMDprintf("State = %d\r\n", state);
			}
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (command)));
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
				if (state == 1 || state == 3) {
					motCom[i] = 0 + mOffset;
				}
				else {
					motCom[i] = (motTemp)+mOffset;
				}

				if (i == 4 && firstRun == 0) {
					deltaT = (PMDint32)TIME[4] - (PMDint32)TIME[14];
					VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)deltaT)) + vOffset;
					accel[i] = 1000.0*(VeL[4] - VeL[14]) / (PMDint32)deltaT;
				}
				else {
					deltaT = (PMDint32)TIME[i] - (PMDint32)TIME[i - 1];
					VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)deltaT)) + vOffset;
					accel[i] = 1000.0 * (VeL[i] - VeL[i-1]) / (PMDint32)deltaT;
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
					DesVeL[i] = (controlOutput) + vOffset;
				}
				SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
				i++;
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
				PMDGetInstructionError(hAxis1, &instructionerror);
				saved = 1;
			}
		}

		// Update Motor Output
		// m*x_ddot + c*x_dot + k*x + Force = Torque
		if (i == 4 & firstRun == 0) j = 14;
		else j = i-1;
		if (old) {
			aCc = 0 * m * accel[j];
			vEl = 1 * c * (VeL[j] - vOffset);
			pOs = dir * k * (counts2cm * (POS[j] - pOffset) - x0);
			fOr = 0 * f * (force[j] - fOffset);
			controlOutput = (aCc - vEl - pOs - fOr - Tf);
			PMDprintf("controlOutput = %d, aCc = %f, vEl = %f, pOs = %f\r\n", controlOutput, aCc, vEl, pOs);
		}
#ifdef Imp
		Fext = -0.001 * (force[j] - fOffset);
		e = .01 * (counts2cm * (POS[j] - pOffset) - x0);
		e_dot = 10 * (VeL[j] - vOffset) * counts2cm;
		F = fgain * Fext - mMd * (Dd * e_dot + Kd * e);// -dir * Tf;
		command = F * motScale;
		PMDprintf("F = %f, Fext = %f, e = %f, e_dot = %f\r\n", F, (fgain* Fext), (Kd* e), (Dd* e_dot));
		if (abs(F) > 30) {
			if (F > 0) {
				command = 30 * motScale;
			}
			else {
				command = -30 * motScale;
			}
		}
#elif ADMITTANCE
		if (saved) {
			Fext = 0.001 * (force[j] - fOffset);
			xd_ddot = (Fext + Kd * x0 - Dd * xd_dot - Kd * xd) * 10;
			xd_dot = xd_dot + deltaT * xd_ddot;
			xd = xd + deltaT * xd_dot;
			F = -kd * (10 * (VeL[j] - vOffset) * counts2cm) + kp * (xd - (.01 * (counts2cm * (POS[j] - pOffset))));
			command = F * motScale;
			PMDprintf("xd_ddot = %f, xd_dot = %f, xd = %f, F = %f\r\n", xd_ddot, xd_dot, xd, F);
			if (abs(F) > 30) {
				if (F > 0) {
					command = 30 * motScale;
				}
				else {
					command = -30 * motScale;
				}
			}
			saved = 0;
		}
#elif SIMPLE
		Fext = -0.001 * (force[j] - fOffset);
		e = .01 * (counts2cm * (POS[j] - pOffset) - abs(x0 - posMax * counts2cm));
		F = mMd * (Kd * e) - dir * Tf;
		command = F * motScale;
		if (F > 30) {
			F = 30;
		}
		else if (F < -30) {
			F = -30;
		}
		PMDprintf("F = %f, position term = %f\r\n", F, (mMd* Kd* e));
		command = F * motScale;
#endif // Imp
		if (abs(controlOutput) > 25) {
			if (controlOutput > 0) {
				controlOutput = 25;
			}
			else {
				controlOutput = -25;
			}
		}
		if (old) {
			command = controlOutput * motScale;
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
			if (comConfig._mode != 22) {
				run = 0;
			}
			serTime = curTime;
		}
	}
	PMDprintf("Finished Impedance\r\n");
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