#ifndef LOADCELL_CME
#define LOADCELL_CME

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	//class for testing load cell in both environments. (PC and CME)
	//{
	PMDresult LoadCellCME(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriphLoad);
	//};


#if defined(__cplusplus)
}
#endif
#endif //!LOADCELL_CME