#ifndef SAVEONLY_H
#define SAVEONLY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	// Assistive mode with updated goals
	//{
	PMDresult SaveOnly(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriphLOAD);
	//};


#if defined(__cplusplus)
}
#endif
#endif // !SAVEONLY_H