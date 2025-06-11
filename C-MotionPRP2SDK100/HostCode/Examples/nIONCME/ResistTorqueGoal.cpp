#include "ResistTorqueGoal.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include <string>

#ifndef CME
#include <iostream>
#include <vector>

PMDresult WriteExcelDirect(PMDAxisHandle* phAxis, const std::string& ws, PMDuint32 time[405], PMDint32 pos[405], PMDint16 motcom[405]);
PMDresult WriteExcelDirectVec(PMDAxisHandle* phAxis, const std::string& wb, const std::string& ws, std::vector<PMDuint32>& TIME, std::vector<PMDint32>& POS, std::vector<PMDint32>& VeL, std::vector<int>& force, std::vector<PMDint16>& motCom);
PMDresult Write2M(PMDAxisHandle* hAxis1, const std::string& wb, std::vector<PMDuint32>& TIME, std::vector<PMDint32>& POS, std::vector<PMDint32>& VeL, std::vector<int>& force, std::vector<PMDint16>& motCom, std::vector<PMDint32>& DesVeL);
#endif // !CME

PMDresult ResistTorqueGoal(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {

	PMDresult result = PMD_ERR_OK;

#ifndef CME
	std::string sheet = "Test";
	std::string book = "Resist.xlsx";

	int Mfile = 01;
	if (Mfile) book = "Resist.m";
#endif // !CME

	int run = 1, save = 0, timeON = 0, vec = 01, dir = 0;
	int mode = 0; // Mode-> 0 = Both, 1 = Push, 2 = Pull
#ifdef CME
	vec = 0;
#endif // CME

	// Scaling values
	double scale = 32768.0 / 100, Vscale = 0.000359657; // Vscale used to convert analog to voltage

	// Command values
	PMDint16 command = 0, max, comInput = 15;
	//double perc = 7, temp;
	int count = 0, timeMS = 100, i = 0, countInput = 3;

	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 0;
	PMDint32 posMax = 17000, posMin = 0;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 500, posBac = posMax - 500;

	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;// | PMDOperatingModeCurrentControlEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	// 
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, TIME[1000], delTime;
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
	max = 100 * scale;

	// Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE;

	//Arrays for writing
	
	PMDint16 motCom[1000];
	PMDint32 POS[1000];
#ifndef CME
	//Arrays for writing
	std::vector<PMDuint32> TIMEV;
	std::vector<PMDint32> POSV;
	std::vector<PMDint32> VeL;
	std::vector<int> force;
	std::vector<PMDint16> motComV;
	std::vector<PMDint32> DesVeL;
#else
	PMDint32 VeL[1000];
	int force[1000];
	PMDint32 DesVeL[1000];
#endif // !CME

	PMDint16 motTemp;
	PMDint32 posTemp;
	

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int startEntry = 1, posWaitEntry = 1, posEntry = 1, negWaitEntry = 1, negEntry = 1;

	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	cycTime = curTime;
	TRACESTART = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
	if (vec) {
#ifndef CME
		TIMEV.push_back((curTime - TRACESTART) * sampleTime * 0.001);
		POSV.push_back(posTemp);
		motComV.push_back(motTemp);
		VeL.push_back(0);
		force.push_back(0);	
		DesVeL.push_back(0);
#endif // !CME
	}
	else {
		TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
		POS[i] = posTemp;
		motCom[i] = motTemp;
		VeL[i] = 0;
		force[i] = 0;
		DesVeL[i] = 0;
		i++;
	}
	

	command = scale * comInput;
	PMDprintf("Starting ResistTorqueGoal.c\r\n");
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
					PMD_RESULT(PMDSetMotorCommand(hAxis1, 15));
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
					PMD_RESULT(PMDSetMotorCommand(hAxis1, 15));
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
					PMD_RESULT(PMDSetMotorCommand(hAxis1, -15));
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
					PMD_RESULT(PMDSetMotorCommand(hAxis1, -15));
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
		if (timeON) {
			if (float(0.001 * (curTime - cycTime) * sampleTime) >= timeMS) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
				POS[i] = posTemp;
				motCom[i] = motTemp;
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
				i++;
			}
		}
		else if (vec) {
			if (float(0.001 * (curTime - cycTime) * sampleTime) >= timeMS) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				TIMEV.push_back((curTime - TRACESTART) * sampleTime * 0.001);
				POSV.push_back(posTemp);
				motComV.push_back(motTemp);
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
				i = TIMEV.size() - 1;
				VeL.push_back((POSV[i] - POSV[i - 1]) / PMDint32(TIMEV[i] - TIMEV[i - 1]));
				force.push_back(dir);
				DesVeL.push_back(0);
				
			}
		}
		else
		{
			if (float(0.001 * (curTime - cycTime) * sampleTime) >= timeMS) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
				POS[i] = posTemp;
				motCom[i] = motTemp;
				delTime = (0.001 * (curTime - cycTime) * sampleTime);
				VeL[i] = (POS[i] - POSV[i - 1]) / PMDint32(TIME[i] - TIME[i - 1]);
				force[i] = dir;
				DesVeL[i] = 0;
				//PMDprintf("Delta T: %d. Position: %d. Motor Command: %d.\r\n", delTime, POS[i], motCom[i]);
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
			}
		}
		if (count == countInput) {
			run = 0;
			PMDprintf("Exit due to count\r\n");
		}
		if (timeON) {
			if (i >= 1000) {
				run = 0;
				PMDprintf("Exit due to time\r\n");
			}
		else if (!vec) {
			if (i >= 1000) {
				run = 0;
				PMDprintf("Exit due to time\r\n");
			}
		}
		}
	}
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos & resNeg & resMoCo)));

	if (save)
	{
#ifndef CME
		if (Mfile) {
			PMD_RESULT(Write2M(hAxis1, book, TIMEV, POSV, VeL, force, motComV, DesVeL));
		}
		else {
				if (vec) {
					PMD_RESULT(WriteExcelDirectVec(hAxis1, book, sheet, TIMEV, POSV, VeL, force, motComV));
				}
				else {
					PMD_RESULT(WriteExcelDirect(hAxis1, sheet, TIME, POS, motCom));
				}
		}
#endif // !CME
	}
	
	return result;
}