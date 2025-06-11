#ifndef LOADCELL_FUNCTIONS_H
#define LOADCELL_FUNCTIONS_H

// Check if we are using a C++ compiler
#ifdef __cplusplus
extern "C" {
#endif

#include "PMDdevice.h"

	// Struct to hold global variables related to the load cell
	typedef struct {
		PMDint32 _offset;      // Offset for load cell calibration
		float _scaleFactor; // Scale factor for load cell readings
		int _loadTime;		// Time interval to check load cell readings in milliseconds
	} LoadCellConfig;

	// Declare a global instance of LoadCellConfig
	extern LoadCellConfig loadCellConfig;

// Function declarations
	PMDint32 read(PMDPeriphHandle* hPeriphLoad);									// Reads a single value
	PMDint32 read_average(PMDPeriphHandle* hPeriphLoad, int times);					// Averages read value
	void tareLoadCell(PMDPeriphHandle* hPeriphLoad, int times);						// Zeroes Load cell
	PMDint32 getTare(void);															// Returns offset
	void setTare(PMDint32 offset);													// Manually sets offset
	void calibrateScale(PMDPeriphHandle* hPeriphLoad, double weight, int times);	// Calibrates scale factor
	float getScale(void);															// Returns scale factor
	void setScale(float scale);														// Manually sets scale factor
	PMDint32 getValue(PMDPeriphHandle* hPeriphLoad, int times);						// Returns read value with corrected offset
	PMDint32 getUnits(PMDPeriphHandle* hPeriphLoad, int times);						// Returns scaled and offset value

	// If we are using a C++ compiler, close the extern "C" block
#ifdef __cplusplus
}
#endif

#endif // LOADCELL_FUNCTIONS_H
