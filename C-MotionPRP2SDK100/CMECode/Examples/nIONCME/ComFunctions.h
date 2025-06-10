#ifndef COM_FUNCTIONS_H
#define COM_FUNCTIONS_H

// Check if we are using a C++ compiler
#ifdef __cplusplus
extern "C" {
#endif

#include "PMDdevice.h"

	// Struct to hold global variables related to the serial communication values
	typedef struct {
		int _mode;				// Operating Mode
		int _rep;				// Number of Repetitions
		PMDint32 _startPos;		// Start Position
		PMDint32 _endPos;		// End Position
		PMDint32 _desVel;		// Desired Velocity
		PMDint16 _torq;			// Resistive Torque motor command
	} COMConfig;

	// Struct to hold timing and offset variables
	typedef struct {
		int _serTime;			// Time Interval to check serial communication in milliseconds
		int _pOffset;			// Position Offset
		int _vOffset;			// Velocity Offset
		int _fOffset;			// Force Offset
		int _mOffset;			// Motor Command Offset
		int _posLimit;			// Positive Position Limit (encoder counts)
	} COMmanagement;

	// Struct to hold Impedance Variables
	typedef struct {
		double _m;			// mass value
		double _Md;			// Desired mass/ inertia
		double _Dd;			// Desired damping
		double _Kd;			// Desired stiffness
	} COMImpValues;

	// Declare a global instance of structures
	extern COMConfig comConfig;
	extern COMmanagement comMan;
	extern COMImpValues comImp;

	// Function declarations
	void defaultVals(void);
	void Take(PMDPeriphHandle *hPeriphSer);
	void Send(PMDPeriphHandle *hPeriphSer, char sendMode, PMDuint32 *time, int tSize, PMDint32 *pos, int pSize, PMDint32 *vel, int vSize, PMDint32 *force, int fSize, PMDint16 *motCom, int mSize, PMDint32 *desVel, int dvSize);
	void SendError(PMDPeriphHandle* hPeriphSer, PMDresult resErr); // Send error code
	void SendBasic(PMDPeriphHandle* hPeriphSer, PMDuint8 Mode, PMDint32 pos); // Send basic info to python
	void SendMotComTest(PMDPeriphHandle* hPeriphSer, PMDint16 mot1, PMDint16 mot2); // Used to test sending motCom and little endian style
	void SendParts(PMDPeriphHandle* hPeriphSer, char sendMode, PMDuint32 time, PMDint32 pos, PMDint32 vel, PMDint32 force, PMDint16 motCom, PMDint32 desVel);
	void SendImpVars(PMDPeriphHandle* hPeriphSer, double m, double Md, double Dd, double Kd); // Used to send Impedance variables
	void SendTrajForce(PMDPeriphHandle* hPeriphSer, int TIMEFORCE, int F, int Fext, int e_term, int e_dot_term, int Tc, int Tv, int a); // Used for the force breakdown in trajectory mode
	void SendTrajValues(PMDPeriphHandle* hPeriphSer, PMDint32 x0_dot, PMDint32 x0_ddot); // Sends x0_dot and x0_ddot for assistive trajectory mode

	// If we are using a C++ compiler, close the extern "C" block
#ifdef __cplusplus
}
#endif

#endif // COM_FUNCTIONS_H