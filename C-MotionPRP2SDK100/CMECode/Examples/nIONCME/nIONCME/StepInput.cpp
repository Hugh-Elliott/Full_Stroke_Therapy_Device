#include "StepInput.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include <string>


PMDresult WriteExcel(PMDAxisHandle* hAxis1, const std::string& ws, PMDuint8 var1, PMDuint8 var2, PMDuint8 var3, PMDuint8 var4, int save);
PMDresult SetTrace(PMDAxisHandle* hAxis1, PMDuint8 var1, PMDuint8 var2, PMDuint8 var3, PMDuint8 var4);

PMDresult StepInput(PMDAxisHandle* hAxis1) {
	PMDresult result = PMD_ERR_OK;
	PMDuint8 timeloc = PMDTraceVariableMotionICTime, actposloc = PMDTraceVariableActualPosition, filler = PMDTraceVariableNone;
	float percScale = 100 / 24, Veff = 0, Vold, scale = 32768 / 100, VOLT = 0.1, zScale = 16384;
	int run = 1, Mod = 0, cyc = 1, save = 0, timeMS = 1000, i;
	PMDint32 pos = 0, posMax = 19000, posMin = 1000;
	PMDint16 temp = 0, oldtemp;
	PMDuint16 lim = 0;
	std::string work = "V0.2";
	int Cent1 = 0, Cent2 = 0, Cent3 = 0, Cent4 = 0, Cent5 = 0;

	// Timing vars
	PMDuint32 startTime, curTime, cycTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));
	PMDprintf("Start time = %d\r\n", startTime);
	// Setting operating mode for torque
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled | PMDOperatingModeCurrentControlEnabled;
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));

	PMDprintf("Starting Step-Input Mode\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		oldtemp = temp;
		PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &temp));
		if (oldtemp != temp) PMDprintf("Active Motor Command = %f\r\n", (temp / scale));
		if (Mod == 1 && Cent1 == 0) {
			work = "V0.2";
			PMD_RESULT(SetTrace(hAxis1, actposloc, timeloc, filler, filler));
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent1 = 1;
		}
		else if (Mod == 2 && Cent2 == 0) {
			work = "V0.4";
			PMD_RESULT(SetTrace(hAxis1, actposloc, timeloc, filler, filler));
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent2 = 1;
		}
		else if (Mod == 3 && Cent3 == 0) {
			work = "V0.6";
			PMD_RESULT(SetTrace(hAxis1, actposloc, timeloc, filler, filler));
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent3 = 1;
		}
		else if (Mod == 4 && Cent4 == 0) {
			work = "V0.8";
			PMD_RESULT(SetTrace(hAxis1, actposloc, timeloc, filler, filler));
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent4 = 1;
		}
		else if (Mod == 5 && Cent5 == 0) {
			work = "V1.0";
			PMD_RESULT(SetTrace(hAxis1, actposloc, timeloc, filler, filler));
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			PMDprintf("Mod = %d\r\n", Mod);
			Cent5 = 1;
		}
		else if (Mod == 0) {
			PMD_RESULT(PMDSetDrivePWM(hAxis1, PMDDrivePWMLimit, lim));
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (10 * scale)));
			PMD_RESULT(PMDUpdate(hAxis1));
			PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &temp));
			PMDprintf("Active Motor Command = %d\r\n", temp);
			PMD_RESULT(SetTrace(hAxis1, actposloc, timeloc, filler, filler));
			cyc = 1;
			Mod = 1;
		}
		Vold = Veff;
		Veff = 5 + Mod * VOLT;
		if (Vold != Veff) {
			lim = Veff * percScale * (zScale/100);
			PMD_RESULT(PMDSetDrivePWM(hAxis1, PMDDrivePWMLimit, lim));
			PMDprintf("PWM Drive Limit = %d\r\n", lim);
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		switch (cyc)
		{
		case 1:
			//0.5 second positive voltage
			if(temp != 10*scale) PMD_RESULT(PMDSetMotorCommand(hAxis1, (10 * scale)));
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
			if (temp != -10 * scale) PMD_RESULT(PMDSetMotorCommand(hAxis1, (-10 * scale)));
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
		PMD_RESULT(WriteExcel(hAxis1, work, actposloc, timeloc, filler, filler, save));
	}
	return result;
}