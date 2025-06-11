#include "StepInput2.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include <string>

PMDresult WriteExcelDirect(PMDAxisHandle* phAxis, const std::string& ws, PMDuint32 time[405], PMDint32 pos[405], PMDint16 motcom[405]);

PMDresult StepInput2(PMDAxisHandle* hAxis1) {
	PMDresult result = PMD_ERR_OK;
	float percScale = 100 / 24, Veff = 0, Vold, scale = 32768 / 100, VOLT = 0.2, zScale = 16384;
	int run = 1, Mod = 5, cyc = 1, save = 0, timeMS = 1000, i = 0;
	PMDint32 pos = 0, posMax = 19000, posMin = 1000;
	PMDint16 temp = 0, oldtemp;
	PMDuint16 lim = 0;
	std::string work = "1.8 V";
	int Cent1 = 0, Cent2 = 0, Cent3 = 0, Cent4 = 0, Cent5 = 0;

	// Timing vars
	PMDuint32 startTime, curTime, cycTime, TIME[405] = { 0 }, sampleInt;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));
	PMDprintf("Start time = %d\r\n", startTime);
	// Setting operating mode for torque
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled | PMDOperatingModeCurrentControlEnabled;
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));

	PMDint16 motCom[405] = { 0 }, motTemp;
	PMDint32 POS[405] = { 0 }, posTemp;

	PMDprintf("Starting Step-Input Mode\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		oldtemp = temp;
		PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &temp));
		if (oldtemp != temp) PMDprintf("Active Motor Command = %f\r\n", (temp / scale));
		if (Mod == 1 && Cent1 == 0) {
			i = 0;
			work = "V0.2";
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent1 = 1;
		}
		else if (Mod == 2 && Cent2 == 0) {
			i = 0;
			work = "V0.4";
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent2 = 1;
		}
		else if (Mod == 3 && Cent3 == 0) {
			i = 0;
			work = "V0.6";
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent3 = 1;
		}
		else if (Mod == 4 && Cent4 == 0) {
			i = 0;
			work = "V0.8";
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent4 = 1;
		}
		else if (Mod == 5 && Cent5 == 0) {
			i = 0;
			work = "1.5 V";
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			sampleInt = startTime;
			curTime = startTime;
			PMDprintf("Mod = %d\r\n", Mod);
			Cent5 = 1;
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
			TIME[i] = (curTime - startTime) * cycTime * 0.001;
			POS[i] = posTemp;
			motCom[i] = motTemp;
			i++;
		}
		else if (Mod == 0) {
			PMD_RESULT(PMDSetDrivePWM(hAxis1, PMDDrivePWMLimit, lim));
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (10 * scale)));
			PMD_RESULT(PMDUpdate(hAxis1));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &temp));
			PMDprintf("Active Motor Command = %d\r\n", temp);
			cyc = 1;
			Mod = 1;
		}
		Vold = Veff;
		Veff = 5 + Mod * VOLT;
		Veff = 6.25;
		if (Vold != Veff) {
			lim = Veff * percScale * (zScale / 100);
			PMD_RESULT(PMDSetDrivePWM(hAxis1, PMDDrivePWMLimit, lim));
			PMDprintf("PWM Drive Limit = %d\r\n", lim);
		}		
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		switch (cyc)
		{
		case 1:
			//0.5 second positive voltage
			if (temp != Veff * scale) PMD_RESULT(PMDSetMotorCommand(hAxis1, (Veff * scale)));
			if (curTime - startTime > (1000 * timeMS / cycTime)) {
				cyc++;
			}
			break;
		case 2:
			//0.5 second 0 voltage
			if (temp != 0) PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
			if (curTime - startTime > (1000 * 2 * timeMS / cycTime)) {
				cyc++;
			}
			break;
		case 3:
			//0.5 second negative voltage
			if (temp != -Veff * scale) PMD_RESULT(PMDSetMotorCommand(hAxis1, (-Veff * scale)));
			if (curTime - startTime > (1000 * 3 * timeMS / cycTime)) {
				cyc++;
			}
			break;
		case 4:
			//0.5 second 0 voltage then reset
			if (temp != 0) PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
			if (curTime - startTime > (1000 * 4 * timeMS / cycTime)) {
				cyc = 1;
				Mod++;
				if (Mod == 6) { save = 1; run = 0; };

			}
			break;
		default:
			break;
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if (float(0.001 * (curTime - sampleInt) * cycTime) > 100) {
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
			TIME[i] = (curTime - startTime) * cycTime * 0.001;
			POS[i] = posTemp;
			motCom[i] = motTemp;
			PMD_RESULT(PMDGetTime(hAxis1, &sampleInt));
			i++;
		}
		//PMD_RESULT(WriteExcelDirect(hAxis1, work, TIME, POS, motCom));
	}
	PMD_RESULT(WriteExcelDirect(hAxis1, work, TIME, POS, motCom));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	return result;
}