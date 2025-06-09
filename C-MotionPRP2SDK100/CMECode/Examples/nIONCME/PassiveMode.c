#include "PassiveMode.h"
#include "C-Motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "PMDperiph.h"
#include <math.h>

#ifndef CME

#include <iostream>
#include <vector>

PMDresult WriteExcelDirectVec(PMDAxisHandle* phAxis, const std::string& wb, const std::string& ws, std::vector<PMDuint32>& TIME, std::vector<PMDint32>& POS, std::vector<PMDint32>& VeL, std::vector<int>& force, std::vector<PMDint16>& motCom);
PMDresult Write2M(PMDAxisHandle* hAxis1, const std::string& wb, std::vector<PMDuint32>& TIME, std::vector<PMDint32>& POS, std::vector<PMDint32>& VeL, std::vector<int>& force, std::vector<PMDint16>& motCom, std::vector<PMDint32>& DesVeL);
#endif // !CME

PMDresult PassiveMode(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {
	PMDresult result = PMD_ERR_OK;

#ifndef CME
	std::string sheet = "Pass1";
	std::string book = "Passive2.m";
#endif // !CME

	int run = 01, save = 01, count = 0, countInput = 3, timeMS = 10, i = 0, dir = 1;

	double velScale = 65536.0; // used for set and get velocity commands

	// Status
	PMDuint16 status = 0;

	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 0;
	PMDint32 pos1 = 17000, pos2 = 0, destpos;
	if (pos1 > posPLim) pos1 = posPLim;
	if (pos2 < posNLim) pos2 = posNLim;

	// Velocity
	PMDint32 Defvelocity = 20000, vel = 0;

	// Operating mode mask
	PMD_RESULT(PMDSetOperatingMode(hAxis1, PMDOperatingModeAllEnabled));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDUpdate(hAxis1));

	// Event mask
	PMDuint16 eventmask = PMDEventStatusMotionComplete | PMDEventStatusMotionError | PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit;

	// Set profile mode
	PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal);
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDSetVelocity(hAxis1, Defvelocity);      // former 32768
	PMDSetAcceleration(hAxis1, 66847);  // former 256
	PMDSetJerk(hAxis1, 65535);
	PMD_RESULT(PMDUpdate(hAxis1));

	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, TRACESTART, waitTime, cycTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	PMDprintf("Sample time = %d microseconds\r\n", sampleTime);
	PMDprintf("Start time = %d\r\n", startTime);
	int wait = 1000; // wait time in ms

	// Reset Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE, resMoEr = 0xFFEF;

#ifndef CME
	//Arrays for writing
	std::vector<PMDuint32> TIME;
	std::vector<PMDint32> POS;
	std::vector<PMDint32> VeL;
	std::vector<int> force;
	std::vector<PMDint16> motCom;
	std::vector<PMDint32> DesVeL;
#else
	PMDuint32 TIME[100];
	PMDint32 POS[100];
	PMDint32 VeL[100];
	PMDint16 motCom[100];
	int force[100];
	PMDint32 DesVeL[100];
#endif // !CME
	PMDint16 motTemp;
	PMDint32 posTemp;

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int inputEntry = 1, posEntry = 1, negEntry = 1, nWaitEntry = 1, pWaitEntry = 1;
	int go = 1;

	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	TRACESTART = curTime;
	waitTime = curTime;
	cycTime = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
#ifndef CME
	TIME.push_back((curTime - TRACESTART) * sampleTime * 0.001);
	POS.push_back(posTemp);
	VeL.push_back(0);
	force.push_back(0);
	motCom.push_back(motTemp);
	DesVeL.push_back(0);
#else
	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp;
	motCom[i] = motTemp;
	VeL[i] = 0;
	force[i] = 0;
	DesVeL[i] = 0;
	i++;
#endif // !CME

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
			if ((double)(0.001 * (curTime - waitTime) * sampleTime) >= wait) {
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
				count++;
				pWaitEntry = 0;
			}
			if ((double)(0.001 * (curTime - waitTime) * sampleTime) >= wait) {
				nextState = 2;
				pWaitEntry = 1;				
			}
			break;
		}

		// records vector values
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((double)(0.001 * (curTime - cycTime) * sampleTime) >= timeMS) {
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
			PMD_RESULT(PMDGetVelocity(hAxis1, &vel));
#ifndef CME
			TIME.push_back((curTime - TRACESTART) * sampleTime * 0.001);
			POS.push_back(posTemp);
			motCom.push_back(motTemp);
			PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
			i = TIME.size() - 1;
			VeL.push_back((POS[i] - POS[i - 1]) / PMDint32(TIME[i] - TIME[i - 1]));
			force.push_back(00);
			DesVeL.push_back(dir * double(double(vel / velScale) / sampleTime) * 1000);
#else
			TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
			POS[i] = posTemp;
			motCom[i] = motTemp;
			VeL[i] = (POS[i] - POS[i - 1]) / (PMDint32)(TIME[i] - TIME[i - 1]);
			force[i] = 00;
			DesVeL[i] = dir * (double)((double)(vel / velScale) / sampleTime) * 1000;
			i++;
			PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
#endif // !CME
		}
		if (i == 100) {
			run = 0;
		}
		if (count >= countInput) {
			run = 0;
			PMDprintf("Count: %d\r\n", count);
		}
	}

	if (save) {
#ifndef CME
		PMD_RESULT(Write2M(hAxis1, book, TIME, POS, VeL, force, motCom, DesVeL));
#endif // !CME
	}

	return result;
}