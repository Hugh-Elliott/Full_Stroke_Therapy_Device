#ifndef COM_FUNCTIONS_H
#define COM_FUNCTIONS_H

// Check if we are using a C++ compiler
#ifdef __cplusplus
extern "C" {
#endif

#include "PMDdevice.h"

	// Struct to hold global variables related to the load cell
	typedef struct {
		int _mode;				// Operating Mode
		int _rep;				// Number of Repetitions
		PMDint32 _startPos;		// Start Position
		PMDint32 _endPos;		// End Position
		PMDint32 _desVel;		// Desired Velocity
		PMDint16 _torq;			// Resistive Torque motor command
	} COMConfig;

	// Declare a global instance of LoadCellConfig
	extern COMConfig comConfig;

	// Function declarations
	void defaultVals(void);
	void Take(PMDPeriphHandle *hPeriphSer);
	void Send(PMDPeriphHandle *hPeriphSer, char sendMode, PMDint32 *time, int tSize, PMDint32 *pos, int pSize, PMDint32 *vel, int vSize, PMDint32 *force, int fSize, PMDint16 *motCom, int mSize, PMDint32 *desVel, int dvSize);
	void SendError(PMDPeriphHandle* hPeriphSer, PMDresult resErr); // Send error code
	void SendBasic(PMDPeriphHandle* hPeriphSer, PMDuint8 Mode, PMDint32 pos); // Send basic info to python
	void SendMotComTest(PMDPeriphHandle* hPeriphSer, PMDint16 mot1, PMDint16 mot2); // Used to test sending motCom and little endian style
	void SendParts(PMDPeriphHandle* hPeriphSer, char sendMode, PMDint32 time, PMDint32 pos, PMDint32 vel, PMDint32 force, PMDint16 motCom, PMDint32 desVel);

	// If we are using a C++ compiler, close the extern "C" block
#ifdef __cplusplus
}
#endif

#endif // COM_FUNCTIONS_H