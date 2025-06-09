#include "TorqueResist2.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include <string>

PMDresult WriteExcel2(PMDAxisHandle* phAxis, const std::string& ws, PMDuint32 time[405], PMDint32 pos[405], PMDint16 motcom[405]);
PMDresult WriteExcelDirect(PMDAxisHandle* phAxis, const std::string& ws, PMDuint32 time[405], PMDint32 pos[405], PMDint16 motcom[405]);

PMDresult TorqueResist2(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {
	int Norm = 1, Min = 0, All = 0, For = 0, DIRECT = 2;

	PMDresult result;
	std::string work = "Test1";
	int run = 1;
	double scale = 32768 / 100, Vscale = 0.000359657; // Vscale used to convert analog to voltage
	// Command values
	PMDint16 command = 0, max;
	double perc = 7, temp;
	int dir = 1, offset = 1, count = 0, timeMS = 100, i = 0;
	PMDint16 analogvalue;
	// Position
	PMDint32 pos = 0, posMax = 17000, posMin = 000;
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled | PMDOperatingModeCurrentControlEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, TIME[405];
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));
	PMDprintf("Start time = %d\r\n", startTime);
	sampleTime = startTime;

	PMDint32 vel = 0;
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);
	max = 100 * scale;

	PMDint16 motCom[405], motTemp;
	PMDint32 POS[405], posTemp;

	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	sampleTime = curTime;
	TRACESTART = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
	TIME[i] = (curTime - TRACESTART) * cycTime * 0.001;
	POS[i] = posTemp;
	motCom[i] = motTemp;
	i++;
	float t = 0;
	PMDprintf("Starting Torque Resistive Mode 2\r\n");
	if (Norm) {
		while (run) {

			PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
			PMD_RESULT(PMDPeriphRead(hAnlgPeriph, &analogvalue, PMDPIO_AICh1, 1));
			temp = ((offset - (Vscale * abs(analogvalue)) / 5)) * perc + offset;

			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			if (float(0.001 * (curTime - startTime) * cycTime) >= timeMS) {
				command = dir * scale * temp;

				if (command > max) command = max - 100;
				else if (command < (-1 * max)) command = -1 * max + 100;
				PMD_RESULT(PMDSetMotorCommand(hAxis1, (command)));
				//PMDprintf("Setting output to %f\r\n", temp);
				PMD_RESULT(PMDUpdate(hAxis1));
				PMD_RESULT(PMDGetTime(hAxis1, &startTime));
				timeMS = 100;
			}
			PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
			PMD_RESULT(PMDGetActualVelocity(hAxis1, &vel));
			if (pos >= posMax && dir == 1) {
				PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
				dir = -1;
				PMD_RESULT(PMDUpdate(hAxis1));
				count++;
				timeMS = 100;
				//PMDTaskWait(500);
			}
			else if (pos <= posMin && dir == -1) {
				PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
				dir = 1;
				PMD_RESULT(PMDUpdate(hAxis1));
				count++;
				//PMDprintf("Count = %d\r\n", count);
				timeMS = 100;
				//PMDTaskWait(500);
			}
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			t = (curTime - sampleTime) * cycTime * 0.001;
			if (float(0.001 * (curTime - sampleTime) * cycTime) >= 1) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				TIME[i] = (curTime - TRACESTART) * cycTime * 0.001;
				POS[i] = posTemp;
				motCom[i] = motTemp;
				PMD_RESULT(PMDGetTime(hAxis1, &sampleTime));
				i++;
				//PMDprintf("(Cur - sam) * cyc = %f\r\n", t);
			}
			if (i >= 405) run = 0;
		}
	}
	int p = 0;
	if (All) {
		run = 1;
		i = 0;
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		sampleTime = curTime;
		TRACESTART = curTime;
		while (run) {
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			t = (curTime - sampleTime) * cycTime * 0.001;
			if (float(0.001 * (curTime - sampleTime) * cycTime) >= 1) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				TIME[i] = (curTime - TRACESTART) * cycTime * 0.001;
				POS[i] = posTemp;
				motCom[i] = motTemp;
				PMD_RESULT(PMDGetTime(hAxis1, &sampleTime));
				i++;
				//PMDprintf("(Cur - sam) * cyc = %f\r\n", t);
			}
			if (i >= 405) run = 0;
		}
	}
	if (Min) {
		run = 1;
		i = 0;
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		sampleTime = curTime;
		TRACESTART = curTime;
		while (run) {
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			t = (curTime - sampleTime) * cycTime * 0.001;
			if (float(0.001 * (curTime - sampleTime) * cycTime) >= 1) {
				//PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				//PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				TIME[i] = (curTime - TRACESTART) * cycTime * 0.001;
				POS[i] = i;
				if (i < 50) { p = 0; }
				else if (i < 200) { p = 20; }
				else if (i >= 200) { p = 40; }
				motCom[i] = p;
				PMD_RESULT(PMDGetTime(hAxis1, &sampleTime));
				i++;
				//PMDprintf("(Cur - sam) * cyc = %f\r\n", t);
			}
			if (i >= 405) run = 0;
		}
	}
	if (For) {
		for (size_t i = 0; i < 405; i++)
		{
			TIME[i] = i;
			POS[i] = 2 * i;
			if (i < 50) { p = 0; }
			else if (i < 200) { p = 20; }
			else if (i >= 200) { p = 40; }
			motCom[i] = p;
		}
	}
	if (DIRECT == 1) {
		PMD_RESULT(WriteExcelDirect(hAxis1, work, TIME, POS, motCom));
	}
	else if (DIRECT == 0) {
		PMD_RESULT(WriteExcel2(hAxis1, work, TIME, POS, motCom));
	}

	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, 0x0037));
	PMD_RESULT(PMDUpdate(hAxis1));
	return PMD_ERR_OK;

}