#ifndef TRANSPARENT_H
#define TRANSPARENT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	// Fully transparent mode for saving data in a structured way
	//{
	PMDresult Transparent(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriphLOAD);
	//};


#if defined(__cplusplus)
}
#endif
#endif // !TRANSPARENT_H