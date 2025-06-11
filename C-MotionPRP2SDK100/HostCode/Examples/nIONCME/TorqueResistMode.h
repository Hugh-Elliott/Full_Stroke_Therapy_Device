#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	//class for testing COM communication
	//{
	PMDresult TorqueResistMode(PMDAxisHandle* phAxis, PMDPeriphHandle* hAnlgPeriph);
	PMDresult Write(PMDAxisHandle* hAxis1);
	PMDresult TraceTest(PMDAxisHandle* hAxis1);
	//};


#if defined(__cplusplus)
}
#endif