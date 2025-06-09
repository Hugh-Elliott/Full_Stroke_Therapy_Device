#include "TorqueAnalog.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"

PMDresult Write(PMDAxisHandle* phAxis);

PMDresult TorqueAnalog(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph, PMDPeriphHandle* hPeriph2) {
	PMDresult result;
	int run = 1; 
	double scale = 32768 / 100, Vscale = 0.000359657; // Vscale used to convert analog to voltage
	PMDint16 command = 8, temp, max, last;
	double perc = 5.5;
	int inc = 1;
	PMDint16 analogvalue;
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled | PMDOperatingModeCurrentControlEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	PMDint32 vel = 0;
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);
	max = 100 * scale;

	PMDuint16 status;
	PMDuint16 eventmask;


	eventmask = PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit | PMDEventStatusMotionComplete;

	while (run) {

		PMD_RESULT(PMDGetEventStatus(hAxis1, &status));
		last = command;
		PMD_RESULT(PMDPeriphRead(hAnlgPeriph, &analogvalue, PMDPIO_AICh1, 1));//
		temp = perc * (Vscale * abs(analogvalue)) / 5;
		//command = scale * (double(Vscale * abs(analogvalue))/5)*perc;
		//if (last > 0) command = abs(command);
		//PMDprintf("%d\r\n",temp);
		perc = 7;
		if (perc > 20) perc = 0;
		command = scale * perc;
		if (command > max) command = max-10;
		else if (command < -max) command = -1*max;
		PMD_RESULT(PMDSetMotorCommand(hAxis1, command));
		PMD_RESULT(PMDUpdate(hAxis1));
		PMDprintf("Setting output to %d", command);
		//PMDprintf("%s \r\n", "%");
		PMDTaskWait(100);

		PMD_RESULT(PMDGetActualVelocity(hAxis1, &vel));
		PMDprintf("Perc = %f\r\n", perc);
		//PMDprintf("Vel = %d\r\n", vel/65536);
		PMD_RESULT(Write(hAxis1));
		run = 1;
	}

	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, 0x0037));
	return PMD_ERR_OK;

}