#include "ReturnHome.h"
#include "C-Motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "PMDperiph.h"


PMDresult ReturnHome(PMDAxisHandle* hAxis1) {
	PMDresult result = PMD_ERR_OK;
	int errorRange = 50; // Encoder count range for error
	PMDint32 pos = 2*errorRange; // Initiallized out of range

	// Operating mode mask
	PMD_RESULT(PMDSetOperatingMode(hAxis1, PMDOperatingModeAllEnabled));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDUpdate(hAxis1));

	// Reset Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE, resMoEr = 0xFFEF;
	PMD_RESULT(PMDResetEventStatus(hAxis1, resPos));
	PMD_RESULT(PMDResetEventStatus(hAxis1, resNeg));
	PMD_RESULT(PMDResetEventStatus(hAxis1, resMoEr));
	PMD_RESULT(PMDUpdate(hAxis1));
	//PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDUpdate(hAxis1));

	// Return to home position
	PMD_RESULT(PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDSetPosition(hAxis1, 0));
	PMD_RESULT(PMDSetVelocity(hAxis1, 10000));      // former 32768
	PMD_RESULT(PMDSetAcceleration(hAxis1, 5000));  // former 256
	PMD_RESULT(PMDSetJerk(hAxis1, 65535));
	PMD_RESULT(PMDUpdate(hAxis1));

	do {
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		PMDTaskWait(100);
	} while (pos > errorRange || pos < -errorRange);

	PMD_RESULT(PMDSetOperatingMode(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
}