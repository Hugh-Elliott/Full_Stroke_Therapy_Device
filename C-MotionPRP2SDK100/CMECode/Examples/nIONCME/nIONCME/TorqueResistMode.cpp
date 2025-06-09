#include "TorqueResistMode.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"

PMDresult Write(PMDAxisHandle* hAxis1);
PMDresult TraceTest(PMDAxisHandle* hAxis1);

PMDresult TorqueResistMode(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {
	PMDresult result;
	int run = 1;
	double scale = 32768 / 100, Vscale = 0.000359657; // Vscale used to convert analog to voltage
	// Command values
	PMDint16 command = 0, max, last;
	double perc = 7, temp;
	int dir = 1, offset = 1, count = 0, timeMS = 100;
	PMDint16 analogvalue;
	// Position
	PMDint32 pos = 0, posOld = 0, posMax = 19000, posMin = 1000;
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled | PMDOperatingModeCurrentControlEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	PMDuint32 startTime, curTime, cycTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));
	PMDprintf("Start time = %d\r\n", startTime);

	PMDint32 vel = 0;
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);
	max = 100 * scale;

	PMDprintf("Starting Torque Resistive Mode\r\n");
	PMD_RESULT(TraceTest(hAxis1));
	while (run) {

		posOld = pos;
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		last = command;
		PMD_RESULT(PMDPeriphRead(hAnlgPeriph, &analogvalue, PMDPIO_AICh1, 1));//
		temp = (1 - (Vscale * abs(analogvalue)) / 5) * perc + offset;

		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if (curTime - startTime > (1000*timeMS / cycTime)) {
			command = dir * scale * temp;

			if (command > max) command = max - 100;
			else if (command < (-1 * max)) command = -1 * max + 100;
			PMD_RESULT(PMDSetMotorCommand(hAxis1, command));
			PMD_RESULT(PMDUpdate(hAxis1));
			//PMDprintf("Setting output to %d\r\n", command);
			//PMDprintf("%s \r\n", "%");
			//PMDTaskWait(100);
			PMD_RESULT(PMDGetTime(hAxis1, &startTime));
			timeMS = 100;
		}
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		PMD_RESULT(PMDGetActualVelocity(hAxis1, &vel));
		//PMDprintf("Pos = %d\r\n", pos);
		//PMDprintf("Vel = %d\r\n", vel/65536);
		if (pos >= posMax && dir == 1) {
			PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
			dir = -1;
			PMD_RESULT(PMDUpdate(hAxis1));
			count++;
			timeMS = 500;
			//PMDTaskWait(500);
		}
		else if (pos <= posMin && dir == -1) {
			PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
			dir = 1;
			PMD_RESULT(PMDUpdate(hAxis1));
			count++;
			PMDprintf("Count = %d\r\n", count);
			timeMS = 500;
			//PMDTaskWait(500);
		}
		
		if (count >= 10) run = 0; 
		PMD_RESULT(Write(hAxis1));
	}

	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, 0x0037));
	return PMD_ERR_OK;

}