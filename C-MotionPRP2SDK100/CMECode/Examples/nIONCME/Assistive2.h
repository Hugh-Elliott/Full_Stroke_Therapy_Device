#ifndef ASSISTIVE2_H
#define ASSISTIVE2_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	// Assistive mode with updated goals
	//{
	PMDresult Assistive2(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriphLOAD);
	//};


#if defined(__cplusplus)
}
#endif
#endif // !ASSISTIVE2_h