#include "PassiveMode2.h"
#include "C-Motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "PMDperiph.h"
#include "ComFunctions.h"
#include "LoadCellFunctions.h"
#include <math.h>

#define arraySize 300

PMDresult PassiveMode2(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLOAD) {
	PMDresult result = PMD_ERR_OK;
	
	int run = 01, save = 01, fullArray = 0, firstRun = 1, loadCell = 01;
	PMDuint32 binSave = 0b00101111;
	
	int count = 0, countInput = comConfig._rep, i = 0, dir = 1, j = 0;

	int deltaT = 0, deltaP = 0, deltaTmin = 1000, deltaPmin = 1000, deltaTmax = -1000, deltaPmax = -1000;

	// Scale Vactors
	double velScale = 65536.0; // used for set and get velocity commands
	double velScaleFactor = 33423 / 10000.0; // Should convert cm/sec to be used in Setvelocity

	// Status
	PMDuint16 status = 0;

	// Position
	PMDint32 pos = 0, posPLim = comMan._posLimit, posNLim = 50;
	PMDint32 pos1 = comConfig._endPos, pos2 = comConfig._startPos, destpos;
	if (pos1 > posPLim) pos1 = posPLim;
	if (pos2 < posNLim) pos2 = posNLim;

	// Velocity
	PMDint32 Defvelocity = comConfig._desVel, velGet = 0, desvel = comConfig._desVel;

	// Operating mode mask
	PMD_RESULT(PMDSetOperatingMode(hAxis1, PMDOperatingModeAllEnabled));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDUpdate(hAxis1));

	// Event mask
	PMDuint16 eventmask = PMDEventStatusMotionComplete | PMDEventStatusMotionError | PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit;

	// Set profile mode
	PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal);
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDSetVelocity(hAxis1, (Defvelocity * velScaleFactor));      // former 32768
	PMDSetAcceleration(hAxis1, 2);  //66847    // former 256
	PMDSetJerk(hAxis1, 65535);
	PMD_RESULT(PMDUpdate(hAxis1));
	//PMDprintf("Defvel = %f\r\n", (Defvelocity * velScaleFactor));

	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, TRACESTART, serTime, waitTime, cycTime, loadCycTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	//PMDprintf("Sample time = %d microseconds\r\n", sampleTime);
	//PMDprintf("Start time = %d\r\n", startTime);
	int wait = 2000; // wait time in ms
	int serTimeMS = comMan._serTime, timeMS = 1, loadTimeMS = loadCellConfig._loadTime;
	int timeCounts = timeMS * 1000 / sampleTime, serTimeCounts = serTimeMS * 1000 / sampleTime, waitCounts = wait * 1000 / sampleTime, loadTimeCounts = loadTimeMS * 1000 / sampleTime;

	// Special scale factor
	double vel2milli = (double)((1 / velScale) / sampleTime) * 1000;  // Converts counts per microsecond to millisecond

	// Reset Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE, resMoEr = 0xFFEF;
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos & resNeg & resMoCo)));
	PMD_RESULT(PMDResetEventStatus(hAxis1, resPos));
	PMD_RESULT(PMDResetEventStatus(hAxis1, resNeg));
	PMD_RESULT(PMDResetEventStatus(hAxis1, resMoEr));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	//PMD_RESULT(PMDSetPosition(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos & resNeg & resMoCo)));

	// Array offsets
	int pOffset = comMan._pOffset, vOffset = comMan._vOffset, fOffset = comMan._fOffset, mOffset = comMan._mOffset;

	//Arrays for writing
	PMDuint32 TIME[arraySize] = { 0 };
	PMDint32 POS[arraySize] = { pOffset };
	PMDint32 VeL[arraySize] = { vOffset };
	PMDint32 force[arraySize] = { fOffset };
	PMDint16 motCom[10] = { 0 };
	PMDint32 DesVeL[arraySize] = { 0 };

	for (int zzz = 0; zzz < arraySize; zzz++) {
		POS[zzz] = pOffset;
		VeL[zzz] = vOffset;
		force[zzz] = fOffset;
		DesVeL[zzz] = vOffset;
		//motCom[zzz] = mOffset;
	}

	PMDint16 motTemp = 0;
	PMDint32 posTemp = 0;

	// Size of arrays
	int tSize = 0, pSize = 0, vSize = 0, mSize = 0, fSize = 0, dSize = 0;

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int inputEntry = 1, posEntry = 1, negEntry = 1, nWaitEntry = 1, pWaitEntry = 1;
	int go = 01;

	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	TRACESTART = curTime;
	waitTime = 500;
	cycTime = curTime;
	serTime = curTime;
	loadCycTime = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp+pOffset;
	motCom[i] = motTemp+mOffset;
	VeL[i] = 0+vOffset;
	force[i] = (0 + fOffset);
	DesVeL[i] = 0+vOffset;
	if (fullArray == 0) {
		SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
	}
	i++;
	j++;

	PMDprintf("Starting Passive Mode\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		state = nextState;
		PMD_RESULT(PMDGetEventStatus(hAxis1, &status));
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		switch (state) {
		case 1: // Waits for user to hit go button on GUI
			if (inputEntry) {
				//PMDprintf("Press the Go! button to start\r\n");
				//PMDprintf("Count: %d\r\n", count);
				inputEntry = 0;
			}
			if (go) { // for now it will be set to true by default
				nextState = 2;
				inputEntry = 1;
			}
			break;
		case 2: // Movement in positive direction
			if (posEntry) {
				destpos = pos1;
				//PMDprintf("Moving to %d\r\n", destpos);
				//PMDprintf("Count: %d\r\n", count);
				PMD_RESULT(PMDSetPosition(hAxis1, destpos));
				dir = 1;
				posEntry = 0;
			}

			if (status & eventmask) {
				posEntry = 1;
				// add here
				if (status == PMDEventStatusMotionComplete) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resMoCo));
					nextState = 3;
				}
				else if (status == PMDEventStatusMotionError) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resMoEr));
					// save exit flag
					if (save) {
						tSize = sizeof(TIME);
						pSize = sizeof(POS);
						vSize = sizeof(VeL);
						fSize = sizeof(force);
						mSize = sizeof(motCom);
						dSize = sizeof(DesVeL);
						if (fullArray) {
							Send(hPeriphSer, binSave, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
						}
						else {
							Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
						}
					}
					return PMD_ERR_MP_MoveWhileInError;
				}
				else if (status == (PMDEventStatusInPositiveLimit & PMDEventStatusMotionComplete)) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resPos));
					// save exit flag
					if (save) {
						tSize = sizeof(TIME);
						pSize = sizeof(POS);
						vSize = sizeof(VeL);
						fSize = sizeof(force);
						mSize = sizeof(motCom);
						dSize = sizeof(DesVeL);
						if (fullArray) {
							Send(hPeriphSer, binSave, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
						}
						else {
							Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
						}
					}
					return PMD_ERR_MP_InvalidMoveIntoLimit;
				}
			}

			break;
		case 3: // Wait X ms then switch to negative
			if (nWaitEntry) {
				PMD_RESULT(PMDGetTime(hAxis1, &waitTime));
				nWaitEntry = 0;
			}
			if ((curTime - waitTime) >= waitCounts) {
				nextState = 4;
				nWaitEntry = 1;
			}
			break;
		case 4: // Movement in negative direction
			if (negEntry) {
				destpos = pos2;
				//PMDprintf("Moving to %d\r\n", destpos);
				PMD_RESULT(PMDSetPosition(hAxis1, destpos));
				dir = -1;
				negEntry = 0;
			}
			if (status & eventmask) {
				negEntry = 1;
				// add here
				if (status == PMDEventStatusMotionComplete) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resMoCo));
					nextState = 5;
				}
				else if (status == PMDEventStatusMotionError) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resMoEr));
					// save exit flag
					if (save) {
						tSize = sizeof(TIME);
						pSize = sizeof(POS);
						vSize = sizeof(VeL);
						fSize = sizeof(force);
						mSize = sizeof(motCom);
						dSize = sizeof(DesVeL);
						if (fullArray) {
							Send(hPeriphSer, binSave, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
						}
						else {
							Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
						}
					}
					return PMD_ERR_MP_MoveWhileInError;
				}
				else if (status == (PMDEventStatusInNegativeLimit & PMDEventStatusMotionComplete)) {
					PMD_RESULT(PMDResetEventStatus(hAxis1, resNeg));
					// save exit flag
					if (save) {
						tSize = sizeof(TIME);
						pSize = sizeof(POS);
						vSize = sizeof(VeL);
						fSize = sizeof(force);
						mSize = sizeof(motCom);
						dSize = sizeof(DesVeL);
						if (fullArray) {
							Send(hPeriphSer, binSave, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
						}
						else {
							Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
						}
					}
					return PMD_ERR_MP_InvalidMoveIntoLimit;
				}
			}
			break;
		case 5: // Wait X ms then switch to positive
			if (pWaitEntry) {
				PMD_RESULT(PMDGetTime(hAxis1, &waitTime));
				pWaitEntry = 0;
			}
			if ((curTime - waitTime) >= waitCounts) {
				nextState = 2;
				pWaitEntry = 1;
				count++;
				if (count >= countInput) {
					run = 0;
					nextState = 5;
				}
			}
			break;
		}

		// records vector values
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if (save) {
			if ((curTime - cycTime) >= timeCounts) {
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetCommandedVelocity(hAxis1, &velGet));
				//PMD_RESULT(PMDGetActualVelocity(hAxis1, &velGet));
				//PMDprintf("i = %d\r\n", i);
				TIME[i] = (curTime - TRACESTART) *sampleTime * 0.001;
				POS[i] = posTemp + pOffset;
				//PMDprintf("POS[%d] = %d\r\n", i, POS[i]);
				if (fullArray) {
					/*
					deltaT = (PMDint32)TIME[i] - (PMDint32)TIME[i - 1];
					deltaP = POS[i] - POS[i - 1];
					if (abs(deltaP) > deltaPmax) { deltaPmax = abs((POS[i] - POS[i - 1])); }
					if (abs(deltaT) > deltaTmax) { deltaTmax = ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1]); }
					if (abs(deltaP) < deltaPmin) { deltaPmin = abs((POS[i] - POS[i - 1])); }
					if (abs(deltaT) < deltaTmin) { deltaTmin = ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1]); }
					if (TIME[i] < TIME[i - 1]) { PMDprintf("TIME[i] = %d, TIME[i-1] = %d\r\n", TIME[i], TIME[i - 1]); }
					*/
					VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
				}
				else {
					
					if (i == 4 && firstRun == 0) {
						VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14])) + vOffset;
					}
					else {
						VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
					}
					
				}
				if (binSave & 0b00010000) {
					PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
					motCom[0] = motTemp + mOffset;
				}
				//PMDprintf("deltaP: %d, deltaT: %d, Velocity: %d, VeL: %d\r\n", POS[i] - POS[i - 1], (PMDint32)TIME[i] - (PMDint32)TIME[i - 1], (POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1]), VeL[i]);
				//PMDprintf("Vel = %d\r\n", VeL[i]);				
				if (loadCell) {
					if ((curTime - loadCycTime) >= loadTimeCounts) {
						force[i] = (getUnits(hPeriphLOAD, 1) + fOffset);
						loadCycTime = curTime;
						//PMDprintf("New value\r\n");
					}
					else {
						//PMDprintf("Copied value\r\n");
						if (i == 4 && firstRun == 0) {
							force[i] = force[14];
						}
						else {
							force[i] = force[i - 1];
						}
					}
					//PMDprintf("Load cell reading: %d\r\n", (force[i] - fOffset));
				}
				else {
					force[i] = (00 + fOffset + i);
				}
				if (binSave & 0b00100000) {
					//DesVeL[i] = dir * velGet * vel2milli + vOffset;
					DesVeL[i] = velGet/velScaleFactor + vOffset;
					//PMDprintf("DesVeL = %d, no offset = %d\r\n", DesVeL[i], (DesVeL[i]-vOffset));
				}
				if (fullArray == 0) {
					SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
				}
				i++;
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
				j++;				
			}
		}
		if (i == arraySize) {
			run = 0;
			//i = 0;
		}
		if (fullArray == 0 && i == 15) {
			firstRun = 0;
			i = 4;
		}
		if (count >= countInput) {
			run = 0;
			//PMDprintf("Count: %d\r\n", count);
		}
		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((curTime - serTime) >= serTimeCounts) {
			Take(hPeriphSer);
			if (comConfig._rep != countInput) {
				countInput = comConfig._rep;
			}
			if (comConfig._endPos != pos1) {
				if (comConfig._endPos > posPLim) {
					comConfig._endPos = posPLim;
				}
				pos1 = comConfig._endPos;
			}
			if (comConfig._startPos != pos2) {
				if (comConfig._startPos < posNLim) {
					comConfig._startPos = posNLim;
				}
				pos2 = comConfig._startPos;
			}
			if (comConfig._desVel != Defvelocity) {
				Defvelocity = comConfig._desVel;
				PMD_RESULT(PMDSetVelocity(hAxis1, (Defvelocity * velScaleFactor)));
			}
			if (comConfig._mode != 0x01 && comConfig._mode != 0x07) {
				run = 0;
			}
			if (comConfig._mode == 0x07) {
				go = 1;
			}
			serTime = curTime;
			PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
			//SendBasic(hPeriphSer, 0x01, j);
		}
	}
	PMD_RESULT(PMDSetVelocity(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (resPos & resNeg & resMoCo)));
	/*
	PMDprintf("deltaTmax = %d\r\n", deltaTmax);
	PMDprintf("deltaTmin = %d\r\n", deltaTmin);
	PMDprintf("deltaPmax = %d\r\n", deltaPmax);
	PMDprintf("deltaPmin = %d\r\n", deltaPmin);
	PMDprintf("j = %d\r\n", j);
	*/
	for (int zzz = 2; zzz < arraySize; zzz++) {
		if (TIME[zzz] == 0) TIME[zzz] = TIME[zzz - 1] + timeMS;
	}

	if (save) {
		vSize = sizeof(VeL) / sizeof(VeL[0]);
		mSize = sizeof(motCom) / sizeof(motCom[0]);
		//PMDprintf("mSize = %d\r\n",mSize);
		tSize = sizeof(TIME);
		pSize = sizeof(POS);
		vSize = sizeof(VeL);
		fSize = sizeof(force);
		mSize = sizeof(motCom);		
		dSize = sizeof(DesVeL);
		PMDTaskWait(1000);
		//PMDprintf("mSize = %d\r\n", mSize);
	//	if (tSize == i) {
			//SendBasic(hPeriphSer, tSize, tSize);
	//	}
		//SendBasic(hPeriphSer, i, i);
		//SendBasic(hPeriphSer, j, j);
		if (fullArray) {
			Send(hPeriphSer, binSave, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
		}
		else {
			Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);
		}
		//motCom[5] = 0xabcd; motCom[6] = 0x1234;
		//SendMotComTest(hPeriphSer, motCom[5], motCom[6]);
		/*
		char label[2] = { '<', 'V' };
		char endlabel[4] = { '>', 0x00, 0x00, 0x00 };
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, VeL, vSize, 1000));
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &endlabel, 4, 100));
		label[1] = 'S';
		PMD_RESULT(PMDPeriphSend(hPeriphSer, &label, 2, 100));
		*/
	}

	return result;
}