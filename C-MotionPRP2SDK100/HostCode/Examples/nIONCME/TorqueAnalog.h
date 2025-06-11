#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	//class for testing COM communication
	//{
	PMDresult TorqueAnalog(PMDAxisHandle* phAxis, PMDPeriphHandle* hAnlgPeriph, PMDPeriphHandle* hPeriph2);
	PMDresult Write(PMDAxisHandle* phAxis);
	//};


#if defined(__cplusplus)
}
#endif