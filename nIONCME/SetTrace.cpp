#include "SetTrace.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"

//PMDuint32 TRACETIMESTART;

PMDresult SetTrace(PMDAxisHandle* hAxis1, PMDuint8 var1, PMDuint8 var2, PMDuint8 var3, PMDuint8 var4) {
    PMDresult result = PMD_ERR_OK;

    // Trace Setup
    PMD_RESULT(PMDSetTracePeriod(hAxis1, 100));
    PMD_RESULT(PMDSetTraceVariable(hAxis1, 0, 0, var1)); // Sets trace variable 1
    PMD_RESULT(PMDSetTraceVariable(hAxis1, 1, 0, var2)); // Sets trace variable 2
    PMD_RESULT(PMDSetTraceVariable(hAxis1, 2, 0, var3)); // Sets trace variable 3
    PMD_RESULT(PMDSetTraceVariable(hAxis1, 3, 0, var4)); // Sets trace variable 3
    PMD_RESULT(PMDSetTraceMode(hAxis1, PMDTraceModeRollingBuffer));
    PMD_RESULT(PMDGetTime(hAxis1, &TRACETIMESTART));
    PMD_RESULT(PMDSetTraceStart(hAxis1, 0, 0, 0, 0));
    PMDprintf("Setting Trace Variables.\r\n");

    return result;
}