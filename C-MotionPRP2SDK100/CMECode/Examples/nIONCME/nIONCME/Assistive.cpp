#include "Assistive.h"
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

PMDresult Assistive(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {

	PMDresult result = PMD_ERR_OK;

	int Mfile = 01;

#ifndef CME
	std::string sheet = "Assist1";
	std::string book = "AssistiveDir2.xlsx";

	if (Mfile) book = "Assistive3.m";
#endif // !CME

	int run = 01, save = 01;

	// Scaling values
	double scale = 32768.0 / 100, Vscale = 0.000359657; // Vscale used to convert analog to voltage
	double velScale = 65536; // used for set and get velocity commands
	double vel2PWM = (25.0/8000);

	// Command values
	PMDint16 command = 0, max; 
	PMDint32 comInput = 3000 * 74 / 4000; // comInput in mm/sec
	//double perc = 7, temp;
	int count = 0, timeMS = 10, i = 0, countInput = 3, dir = 1;
	int Tneg = 0,pVelN = 0, nVelN = 0;

	// Position
	PMDint32 pos = 0, posPLim = 17000, posNLim = 0;
	PMDint32 posMax = 17000, posMin = 0;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 50, posBac = posMax - 50;
	
	// Velocity values
	PMDint32 tempVel = 0, pVelMax = -100, pVelMin = 9999, nVelMax = -100, nVelMin = 9999, pVelAve = 0, nVelAve = 0, comTemp = 0, velTEMP = 0;
	PMDint32 vel = 0, velAVE = 0;
	comTemp = (comInput+.5) / (74.0/4000); // converts mm/sec to counts/ sec
	double back = 3000 * 74 / 4000;

	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;// | PMDOperatingModeCurrentControlEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	// 
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, delTime, comTIME;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	PMDprintf("Sample time = %d microseconds\r\n", sampleTime);
	PMDprintf("Start time = %d\r\n", startTime);
	cycTime = startTime;

	//  Setting operating mode
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);
	max = 100 * scale;

	// Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE;

#ifndef CME
	//Arrays for writing
	std::vector<PMDint16> motCom;
	std::vector<PMDint32> POS;
	std::vector<PMDuint32> TIME;
	std::vector<PMDint32> VeL;
	std::vector<int> force;
	std::vector<PMDint32> DesVeL;
	//PMDint16 motCom[405];
	//PMDint16 motTemp;
	//PMDint32 POS[405];
	//PMDint32 posTemp;
#else
	PMDuint32 TIME[1000];
	PMDint32 POS[1000];
	PMDint32 VeL[1000];
	PMDint16 motCom[1000];
	int force[1000];
	PMDint32 DesVeL[1000];
#endif // !CME
	PMDint32 posTemp;
	PMDint16 motTemp;

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
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
#ifndef CME
	TIME.push_back((curTime - TRACESTART) * sampleTime * 0.001);
	POS.push_back(posTemp);
	motCom.push_back(motTemp);
	VeL.push_back(0);
	force.push_back(0);
	DesVeL.push_back(comTemp/1000.0);
#else
	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp;
	motCom[i] = motTemp;
	VeL[i] = 0;
	force[i] = 0;
	DesVeL[i] = comTemp / 1000.0;
	i++;
#endif // !CME

	command = scale * comInput;
	if (abs(command) > (scale * 30)) command = scale * 30;
	PMDprintf("Starting Assistive Mode\r\n");
	PMDprintf("Command at 20 = %d\r\n", command);
	PMDprintf("scale * 20 = %d\r\n", int(scale * 20));
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
			PMDprintf("Command * dir = %d\r\n", (dir *command));
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
			PMDprintf("Command * dir = %d\r\n", (dir * command));
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

		// records vector values
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if (double(0.001 * (curTime - cycTime) * sampleTime) >= timeMS) {
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
#ifndef CME
			TIME.push_back((curTime - TRACESTART) * sampleTime * 0.001);
			POS.push_back(posTemp);
			motCom.push_back(motTemp);
			//VeL.push_back((POS[i] - POS[i - 1]) / PMDint32(TIME[i] - TIME[i - 1]));
			//PMDprintf("POS[%d] = %d\r\n", i, POS[i]);
			PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
			i = TIME.size()-1;
			VeL.push_back((POS[i] - POS[i - 1]) / PMDint32(TIME[i] - TIME[i - 1]));
			force.push_back(dir*10);
			//PMDprintf("POS[%d] = %d\r\n", i, POS[i]);
			DesVeL.push_back(comTemp*dir/1000);
#else
			TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
			POS[i] = posTemp;
			motCom[i] = motTemp;
			VeL[i] = (POS[i] - POSV[i - 1]) / PMDint32(TIME[i] - TIME[i - 1]);
			force[i] = dir*10;
			DesVeL[i] = comTemp * dir / 1000;
			i++;
			PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
#endif // !CME
		}
#ifndef CME
		// debugging
		if (i > 0) {
			tempVel = ((POS[i] - POS[i - 1]) / PMDint32(TIME[i] - TIME[i - 1]));
			//PMDprintf("T POS[%d] = %d\r\n", i, POS[i]);
			//PMDprintf("i = %d\r\n", i);
		}
		if (PMDint32(TIME[i] - TIME[i - 1]) < 0) {
			PMDprintf("Negative Time");
			Tneg++;
		}
#endif // !CME
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


		// attempt at variable speed
		if (i > 10) {
			for (int j = i-9; j < i; j++) {
				velAVE = velAVE + VeL[j];
			}
			velAVE = velAVE / 10;
			//velAVE = (VeL[i] + VeL[i - 1] + VeL[i - 2] + VeL[i - 3] + VeL[i - 4] + VeL[i - 5] + VeL[i - 6] + VeL[i - 7] + VeL[i - 8] + VeL[i - 9]) / 10;
			//PMDprintf("velAVE (%d) < comTemp (%d)\r\n", (velAVE * 1000), comTemp);
			if (abs(velAVE * 1000) < (2 * comTemp)) {
				//if (double(0.001 * (curTime - comTIME) * sampleTime) >= timeMS) {
					velTEMP = 5 + (vel2PWM * ((comTemp - (1000 * abs(velAVE)))));
					command = scale * velTEMP; // output = scale factor * ((desired speed - (average actual speed)) * gain)
					if (abs(command) > (scale * 30)) {
						command = scale * 30;
					}
					//if (abs(command) < (scale * 15)) command = scale * 15;
					//PMDprintf("Command = %d\r\n", command);
					//PMDprintf("dir = %d\r\n", dir);
					//PMD_RESULT(PMDGetTime(hAxis1, &comTIME));
				//}
			}
		}
		
		if (count >= countInput) {
			run = 0;
		}
		if (i >= 1000) {
			run = 0;
			PMDprintf("Exit due to time\r\n");
		}
	}
	PMDprintf("tneg = %d\r\n", Tneg);
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos & resNeg & resMoCo)));
	
	if (pVelN == 0) pVelN = 1;
	if (nVelN == 0) nVelN = 1;
	pVelAve = pVelAve / pVelN;
	nVelAve = nVelAve / nVelN;
	PMDprintf("pVelMax = %d counts/sec\r\n", pVelMax * 1000);
	PMDprintf("pVelMin = %d counts/sec\r\n", pVelMin * 1000);
	PMDprintf("pVelAve = %d counts/sec\r\n", pVelAve * 1000);
	PMDprintf("nVelMax = %d counts/sec\r\n", nVelMax * 1000);
	PMDprintf("nVelMin = %d counts/sec\r\n", nVelMin * 1000);
	PMDprintf("nVelAve = %d counts/sec\r\n", nVelAve * 1000);

	if (save)
	{
#ifndef CME
		//PMD_RESULT(WriteExcelDirect(hAxis1, sheet, TIME, POS, motCom));
		if (Mfile) {
			PMD_RESULT(Write2M(hAxis1, book, TIME, POS, VeL, force, motCom, DesVeL));
		}
		else {
			PMD_RESULT(WriteExcelDirectVec(hAxis1, book, sheet, TIME, POS, VeL, force, motCom));
		}
#endif // !CME
	}

	// getvelocity = counts/cyc
	// ((counts/cyc) / sampleTime) * 10^6 = counts/sec
	// 2000 counts ~ 37 mm
	// counts/sec * (37/2000) = mm/sec
	// mm/sec * 0.1 = cm/sec

	return result;
}