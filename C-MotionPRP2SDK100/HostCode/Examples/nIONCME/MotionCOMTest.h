#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	//class for testing COM communication
	//{
	PMDresult MotionCOMTest(PMDAxisHandle* phAxis, PMDPeriphHandle* hAnlgPeriph, PMDPeriphHandle* hPeriph2);
	int COMFUNC(PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriph);
	//};


#if defined(__cplusplus)
}
#endif