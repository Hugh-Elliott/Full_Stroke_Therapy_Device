#include "TorqueMode.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"

PMDresult TorqueMode(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph, PMDPeriphHandle* hPeriph2) {
	PMDresult result;
	int run = 1;
	double scale = 32768 / 100;
	PMDint16 command = 0, max;
	int perc = 0;
	int inc = 1;

	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, (PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled | PMDOperatingModeCurrentControlEnabled)));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	max = 100 * scale;

	while (run) {

		command = perc * scale;
		if (perc == 15)
		{
			command = 15 * scale;
		}
		PMD_RESULT(PMDSetMotorCommand(hAxis1, command));
		PMD_RESULT(PMDUpdate(hAxis1));
		PMDprintf("Setting output to %d", perc);
		PMDprintf("%s \r\n", "%");
		PMDTaskWait(1000);
		if (perc >= 10) { inc = 0; perc = 10; }
		else if (perc <= -10) { inc = 1; perc = -10; }
		else if (perc == 0) PMDTaskWait(2000);
		if (inc == 1)
			perc = perc + 1;
		else perc = perc - 1;
		run = 1;
	}

	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, 0x0037));
	return PMD_ERR_OK;

}