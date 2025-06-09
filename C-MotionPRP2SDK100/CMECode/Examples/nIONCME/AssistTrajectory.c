#include "AssistTrajectory.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "ComFunctions.h"
#include "LoadCellFunctions.h"
#include <math.h>
#include "EKF.h"

#define arraySize 30
#define Viscous 1
//#define SINECURVE 1
#define SMOOTH 0
#define VELDIRINT 0
#define TRAPPROFILE 01
#define EKFONLINE 0

static double trapProfile(double a, double v_max, double* v_peak, double* TimeAccel, double* TimeCon, double* TimeTotal, double posMaxCM, double posMinCM) {

	double Ta = 0;
	double x_total = fabs(posMaxCM - posMinCM);
	double x_a = 0;
	double x_min = (v_max * v_max) / a;
	float x_const = 0;

	if (x_total > x_min) {
		// Trapezoidal
		*v_peak = v_max;
		Ta = v_max / a;
		*TimeAccel = Ta;
		x_a = 0.5 * a * Ta * Ta;
		x_const = x_total - 2 * x_a;
		*TimeCon = x_const / v_max;
		*TimeTotal = 2 * Ta + *TimeCon;
	}
	else {
		// Triangle
		*v_peak = sqrt(x_total * a);
		Ta = *v_peak / a;
		*TimeAccel = Ta;
		*TimeCon = 0.0;
		*TimeTotal = 2 * Ta;
	}
	PMDprintf("TimeAccel = %f, TimeCon = %f\r\n", *TimeAccel, *TimeCon);
}

static double trapProfile2(double a, double v_max, double* v_peak, double* x_accel, double* x_const, double* x_total, double posMaxCM, double posMinCM){
	
	double x_tot = fabs(posMaxCM - posMinCM);
	double x_min = (v_max * v_max) / a;
	*x_total = x_tot;

	if (x_tot > x_min) {
		// Trapezoidal
		*v_peak = v_max;
		*x_accel = 0.5 * x_min;
		*x_const = x_tot - x_min;
	}
	else {
		// Triangular
		*v_peak = sqrt(x_tot * a);
		*x_accel = 0.5 * *v_peak * *v_peak / a;
		*x_const = 0;
	}
}

PMDresult AssistTrajectory(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLOAD) {

	PMDresult result = PMD_ERR_OK;

	int run = 01, save = 01, firstRun = 1, loadCell = 01;
	PMDuint32 binSave = 0b00111111;
	PMDuint16 instructionerror;

	// Scaling values
	double motScale = 32768.0 / 100; // motor command scale factor
	double counts2cm = 1 / (10 * (4000.0 / 74));

	// Command values
	PMDint16 command = 0, max = 30 * motScale;
	double comInput = 1000.0 * comConfig._torq / motScale; // comInput in 
	int count = 0, i = 0, j = 0, countInput = comConfig._rep, dir = 1;
	int waitPWM = 8 * motScale;
	double forceError = 0, forceTemp = 0;

	// Position
	PMDint32 pos = 0, posPLim = comMan._posLimit, posNLim = 0;
	PMDint32 posMax = comConfig._endPos, posMin = comConfig._startPos;
	if (posMax > posPLim) posMax = posPLim;
	if (posMin < posNLim) posMin = posNLim;
	PMDint32 posFor = posMin + 50, posBac = posMax - 50;
	double posCMAdd = 0.15;
	double posMaxCM = posMax * counts2cm + posCMAdd;
	double posMinCM = posMin * counts2cm;

	// Variables for impedancce control
	PMDprintf("m = %f, Md = %f, Dd = %f, Kd = %f\r\n", comImp._m, comImp._Md, comImp._Dd, comImp._Kd);
	double m = comImp._m;
	double Md = comImp._Md;
	double Dd = comImp._Dd;
	double Kd = comImp._Kd;

	double mMd = (m*1.0) / (Md*1.0);
	double fgain = mMd - 1;
	double Fext = 0.0;
	double e = 0.0;
	double e_dot = 0.0;
	double F = 0;
	double x0 = 0;
	double Tcoulomb = 10.95; // Minimum PWM% to overcome friction
	double x0_dot = .01 * round(counts2cm * comConfig._desVel * 100.0) / 100; // m/s
	double x0_ddot = 0.097; // can't remember why this is 0.097
	double x = 0;
	double x_dot = 0;
	double viscousFrictionGain = 1 * (1.0 / 0.090977) * (100.0 / 24.0);

#if TRAPPROFILE == 1
	int trap = 02;

	double velMaxCM = x0_dot * 100;
	double velPeak = 0;
	double x0_dot_prev = x0_dot;
	x0_ddot = 10; // cm/s^2
	double accel = x0_ddot;
	double TimeAccel =  velMaxCM / x0_ddot;
	double x_accel = 0.5 * x0_ddot * TimeAccel * TimeAccel;
	double TimeCon = (posMaxCM - 2 * x_accel) / velMaxCM;
	double TimeConLast = TimeCon;
	double TimeMotion = 0;
	double TimeMotionTotal = 2 * TimeAccel + TimeCon;
	PMDprintf("TimeAccel = %f, TimeCon = %f\r\n", TimeAccel, TimeCon);
	PMDTaskWait(1000);
	double deltaT_s = 0;
	int trapDir = 0;
	double x_goal = 0;
	double x_start = 0;
	double x_const = 0;
	double x_constLast = x_const;
	double x_total = 0;
	double x_travelled = 0;

	if (trap == 1) {
		trapProfile(accel, velMaxCM, &velPeak, &TimeAccel, &TimeCon, &TimeMotionTotal, posMaxCM, x);
		PMDprintf("Trap Profile 1\r\n");
	}
	else {
		trapProfile2(accel, velMaxCM, &velPeak, &x_accel, &x_const, &x_total, posMaxCM, x);
		PMDprintf("Trap Profile 2\r\n");
	}
#endif

#ifdef SINECURVE
	x0_dot = 0;
	double pi = 3.14159265358979323846;
	double freqFactor = pi / 5.0; // Arbitrarily choden
	double positionAmplitude = (posMaxCM) / 2.0;
	double velocityAmplitude = positionAmplitude * freqFactor;
	double accelerationAmplitude = positionAmplitude * freqFactor * freqFactor;
	double positionPhase = pi / 2.0;
	double sineTime = 0;

#endif // SINECURVE
#if VELDIRINT == 1
	int velDir = 1;
#else
	double velDir = 1.0;
#endif

	// Velocity
	PMDint32 desiredVelocity = comConfig._desVel; // Uses to check if the desired velocity is updated
	PMDint32 vel = 0;

#if EKFONLINE == 1
	// Extended Kalman Filter Object
	EKFVars ekf;
	EKF_init(&ekf, x, 0.0);

#endif // EKFONLINE == 1

	// Operating mode mask
	PMDuint16 OPMODE = PMDOperatingModeAxisEnabled | PMDOperatingModeMotorOutputEnabled;
	// Torque mode guide on page 160 (SetOperatingMode) of C-Motion Magellan Programming Reference
	
	// Time Variables
	PMDuint32 startTime, curTime, sampleTime, cycTime, TRACESTART, serTime, comTIME, loadCycTime, deltaT = 0, waitTime;
	PMD_RESULT(PMDGetTime(hAxis1, &startTime));
	PMD_RESULT(PMDGetSampleTime(hAxis1, &sampleTime));
	int wait = 2000; // wait time in ms

	cycTime = startTime;
	int serTimeMS = comMan._serTime, timeMS = 1, loadTimeMS = loadCellConfig._loadTime;
	int timeCounts = timeMS * 1000 / sampleTime, serTimeCounts = serTimeMS * 1000 / sampleTime, loadTimeCounts = loadTimeMS * 1000 / sampleTime;
	int waitCounts = wait * 1000 / sampleTime;
	//PMDprintf("timeCounts = %d, serTimeCounts = %d\r\n", timeCounts, serTimeCounts);
#ifdef SINECURVE
	waitCounts = 0;
#endif // SINECURVE

	//  Setting operating mode
	PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
	PMD_RESULT(PMDSetOperatingMode(hAxis1, OPMODE));
	PMD_RESULT(PMDResetEventStatus(hAxis1, (PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit)));
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	PMDTaskWait(1000);

	// Event Masks
	PMDuint16 resPos = 0xFFDF, resNeg = 0xFFBF, resMoCo = 0xFFFE;

	// Array offsets
	int pOffset = comMan._pOffset, vOffset = comMan._vOffset, fOffset = comMan._fOffset, mOffset = comMan._mOffset;

	//Arrays for writing
	PMDuint32 TIME[arraySize] = { 0 };
	PMDint32 POS[arraySize] = { pOffset };
	PMDint32 VeL[arraySize] = { vOffset };
	PMDint32 force[arraySize] = { fOffset };
	PMDint16 motCom[arraySize] = { mOffset };
	PMDint32 DesVeL[arraySize] = { 0 };			// Will be used to save x0 values

	for (int zzz = 0; zzz < arraySize; zzz++) {
		POS[zzz] = pOffset;
		VeL[zzz] = vOffset;
		force[zzz] = fOffset;
		motCom[zzz] = mOffset;
		DesVeL[zzz] = vOffset;
	}

	PMDint32 posTemp = 0;
	PMDint16 motTemp = 0;

	// Size of arrays
	int tSize = 0, pSize = 0, vSize = 0, mSize = 0, fSize = 0, dSize = 0;

	// State variables
	int state = 1;
	int nextState = state;

	// Entry flags
	int posWaitEntry = 1, posEntry = 1, negWaitEntry = 1, negEntry = 1;

	//Adding t = 0 values
	PMD_RESULT(PMDGetTime(hAxis1, &curTime));
	cycTime = curTime;
	TRACESTART = curTime;
	comTIME = curTime;
	serTime = curTime;
	loadCycTime = curTime;
	PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
	PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
	TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
	POS[i] = posTemp + pOffset;
	motCom[i] = motTemp + mOffset;
	VeL[i] = 0 + vOffset;
	force[i] = (0 + fOffset);
	DesVeL[i] = 0 + vOffset;

	SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
	i++;

	PMDprintf("Starting Assistive Trajectory!\r\n");
	while (run) {
		PMD_RESULT(PMDUpdate(hAxis1));
		PMD_RESULT(PMDGetActualPosition(hAxis1, &pos));
		state = nextState;
		switch (state)
		{
		case 1: // Wait for positive direction motion
			if (posWaitEntry) {
#ifndef SINECURVE
				PMD_RESULT(PMDSetMotorCommand(hAxis1, waitPWM));
#if TRAPPROFILE == 0
				x0 = posMinCM;
#endif
#endif // !SINECURVE
				//PMDprintf("Start pushing in the positive direction\r\n");
				//PMDprintf("Count: %d\r\n", count);
				dir = 0;
				posWaitEntry = 0;
				//PMDprintf("State = %d\r\n", state);
				PMD_RESULT(PMDGetTime(hAxis1, &waitTime));
			}
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			//PMDprintf("deltaT = %d, waitTime = %d\r\n", (curTime - waitTime), waitCounts);
			//PMDprintf("Result = %d\r\n",((curTime - waitTime) >= waitCounts));
			if ((curTime - waitTime) >= waitCounts) {
				nextState = 2;
				posWaitEntry = 1;
				PMDprintf("Next State = %d\r\n", nextState);
			}
			else if(pos >= posFor + 50) {
				nextState = 2;
				posWaitEntry = 1;
			}
			break;
		case 2: // Torque for pos
			if (posEntry) {
				dir = 1;
#if TRAPPROFILE == 1
				trapDir = dir;
				x_goal = posMaxCM;
				x_start = x;
				if (fabs(posMinCM - x) > 0.1) {
					if (trap == 1) {
						trapProfile(accel, velMaxCM, &velPeak, &TimeAccel, &TimeCon, &TimeMotionTotal, posMaxCM, x);
					}
					else {
						trapProfile2(accel, velMaxCM, &velPeak, &x_accel, &x_const, &x_total, posMaxCM, x);
					}
				}
				else if(TimeCon != TimeConLast || x_const != x_constLast) {
					if (trap == 1) {
						trapProfile(accel, velMaxCM, &velPeak, &TimeAccel, &TimeCon, &TimeMotionTotal, posMaxCM, posMinCM);
						TimeConLast = TimeCon;
					}
					else {
						trapProfile2(accel, velMaxCM, &velPeak, &x_accel, &x_const, &x_total, posMaxCM, posMinCM);
						x_constLast = x_const;
					}
				}
#endif
				//PMDprintf("Applying torque (positive)\r\n");
				posEntry = 0;
				//PMDprintf("State = %d\r\n", state);
				TimeMotion = 0;
			}
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (command)));
			//PMDprintf("Command * dir = %d\r\n", (dir * command));
			if (pos <= posMin - 100)
			{
				//nextState = 1;
				//posEntry = 1;
			}
			else if (pos >= posMax + 50) {
				nextState = 3;
				posEntry = 1;
			}
			break;
		case 3: // Wait for negative direction motion
			if (negWaitEntry) {
#ifndef SINECURVE
				PMD_RESULT(PMDSetMotorCommand(hAxis1, -waitPWM));
#if TRAPPROFILE == 0
				x0 = posMaxCM;
#endif
#endif // !SINECURVE
				//PMDprintf("Start pushing in the negative direction\r\n");
				dir = 0;
				negWaitEntry = 0;
				//PMDprintf("State = %d\r\n", state);
				PMD_RESULT(PMDGetTime(hAxis1, &waitTime));
			}
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			if ((curTime - waitTime) >= waitCounts) {
				nextState = 4;
				negWaitEntry = 1;
			}
			else if (pos <= posBac - 50) {
				nextState = 4;
				negWaitEntry = 1;
			}
			break;
		case 4: // Torque for neg
			if (negEntry) {
				dir = -1;
#if TRAPPROFILE == 1
				trapDir = dir;
				x_goal = posMinCM;
				x_start = x;
				PMDprintf("posMaxCM = %f, x = %f\r\n", posMaxCM, x);
				if (fabs(posMaxCM - x) > 0.1) {
					if (trap == 1) {
						trapProfile(accel, velMaxCM, &velPeak, &TimeAccel, &TimeCon, &TimeMotionTotal, x, posMinCM);
					}
					else {
						trapProfile2(accel, velMaxCM, &velPeak, &x_accel, &x_const, &x_total, x, posMinCM);
					}
				}
				else if (TimeCon != TimeConLast || x_const != x_constLast) {
					if (trap == 1) {
						trapProfile(accel, velMaxCM, &velPeak, &TimeAccel, &TimeCon, &TimeMotionTotal, posMaxCM, posMinCM);
						TimeConLast = TimeCon;
					}
					else {
						trapProfile2(accel, velMaxCM, &velPeak, &x_accel, &x_const, &x_total, posMaxCM, posMinCM);
						x_constLast = x_const;
					}
				}
#endif
				//PMDprintf("Applying torque (negative)\r\n");
				negEntry = 0;
				//PMDprintf("State = %d\r\n", state);
				TimeMotion = 0;
			}
			PMD_RESULT(PMDSetMotorCommand(hAxis1, (command)));
			//PMDprintf("Command * dir = %d\r\n", (dir * command));
			if (pos >= posMax + 100)
			{
				//nextState = 3;
				//negEntry = 1;
			}
			else if (pos <= posMin - 100) {
				nextState = 1;
				count++;
				negEntry = 1;
			}
			break;
		}

		if (count >= countInput) {
			run = 0;
			//PMDprintf("Exit due to count\r\n");
		}

		// records vector values
		if (save) {		
			PMD_RESULT(PMDGetTime(hAxis1, &curTime));
			if ((curTime - cycTime) >= timeCounts) {
				//PMDprintf("Save\r\n");
				PMD_RESULT(PMDGetActualPosition(hAxis1, &posTemp));
				PMD_RESULT(PMDGetActiveMotorCommand(hAxis1, &motTemp));
				
				TIME[i] = (curTime - TRACESTART) * sampleTime * 0.001;
				POS[i] = posTemp + pOffset;
				x = posTemp * counts2cm;
				if (state == 1 || state == 3) {
					motCom[i] = 0 + mOffset;
				}
				else {
					motCom[i] = (motTemp)+mOffset;
				}
				if (i == 4 && firstRun == 0) {
					deltaT = (PMDint32)TIME[4] - (PMDint32)TIME[14];
					VeL[i] = ((POS[4] - POS[14]) / ((PMDint32)TIME[4] - (PMDint32)TIME[14])) + vOffset;
				}
				else {
					deltaT = (PMDint32)TIME[i] - (PMDint32)TIME[i-1];
					VeL[i] = ((POS[i] - POS[i - 1]) / ((PMDint32)TIME[i] - (PMDint32)TIME[i - 1])) + vOffset;
				}
				
				if (loadCell) {
					if ((curTime - loadCycTime) >= loadTimeCounts) {
						forceTemp = getUnits(hPeriphLOAD, 1);
						force[i] = (forceTemp + fOffset);
						loadCycTime = curTime;
					}
					else {
						if (i == 4 && firstRun == 0) {
							force[i] = force[14];
						}
						else {
							force[i] = force[i - 1];
						}
					}
				}
				else {
					force[i] = (dir * 10 + fOffset);
				}
				if (binSave & 0b00100000) {
#if TRAPPROFILE == 1
					DesVeL[i] = x0 * 100 + vOffset;
#else
					DesVeL[i] = 100*x0 + vOffset;
#endif
				}

				x_dot = (VeL[j] - vOffset) * counts2cm*10;
#ifndef SINECURVE
					deltaT_s = deltaT * .001;
					if (0) {
						if (dir == 1 && x > x0) {
							x0 = x;
						}
						else if (dir == -1 && x < x0) {
							x0 = x;
						}
					}
#if TRAPPROFILE == 1
					TimeMotion += deltaT_s;
					if (trap == 1) {
						if (TimeMotion < TimeAccel) {
							x0_dot = trapDir * TimeMotion * accel; // cm/s
							x0_ddot = trapDir * accel;
						}
						else if (TimeMotion < (TimeAccel + TimeCon)) {
							x0_dot = trapDir * velPeak;
							x0_ddot = 0;
						}
						else if (TimeMotion < TimeMotionTotal) {
							x0_dot = trapDir * (velPeak - accel * (TimeMotion - TimeAccel - TimeCon));
							x0_ddot = trapDir * -accel;
							PMDprintf("x0 = %f, x0_dot = %f, TimeMotion = %f, trapDir = %d\r\n", x0, x0_dot, TimeMotion, trapDir);
							if ((trapDir == 1 && x0_dot < 0) || (trapDir == -1 && x0_dot > 0)) {
								x0_dot = 0;
								x0_ddot = 0;
							}
						}
						else {
							x0_dot = 0;
							x0_ddot = 0;
						}
					}
					else {
						x_travelled = fabs(x0 - x_start);

						if (x_travelled < x_accel) {
							x0_ddot = trapDir * accel;
							x0_dot += deltaT_s * x0_ddot;
						}
						else if (x_travelled < (x_accel + x_const)) {
							x0_ddot = 0;
							x0_dot = trapDir * velPeak;
						}
						else if (x_travelled < x_total) {
							x0_ddot = -trapDir * accel;
							x0_dot += deltaT_s * x0_ddot;

							if ((trapDir == 1 && x0_dot < 0) || (trapDir == -1 && x0_dot > 0)) {
								x0_dot = 0;
								x0_ddot = 0;
							}
						}
						else {
							x0 = x_goal;
							x0_dot = 0;
							x0_ddot = 0;
						}
						if (x0_ddot != 0) {
							x0_ddot = x0_ddot * .01;
						}
					}
					//PMDprintf("state = %d, trapDir = %d\r\n", state, trapDir);
					//PMDprintf("x0 = %f, x0_dot = %f, x = %f\r\n", x0, x0_dot, x);
					x0 += deltaT_s * 0.5 * (x0_dot + x0_dot_prev);
					x0_dot_prev = x0_dot;

					if ((trapDir == 1 && x0 > x_goal) || (trapDir == -1 && x0 < x_goal)) {
						x0_dot = 0;
						x0 = x_goal;
						x0_ddot = 0;
					}
#else
					if (dir != 0) {
						x0 = x0 + dir * deltaT_s * 100 * x0_dot;
					}
#endif // TRAPPROFILE
					//PMDprintf("x0_dot = %f, add = %f, x0 = %f\r\n", x0_dot, (dir * (PMDint32)deltaT * .001 * x0_dot), x0);
					//PMDprintf("dir = %d, deltaT = %d, add = %f, dir*deltaT = %d, dir*deltaT*.001 = %f\r\n", dir, deltaT, (dir* (PMDint32)deltaT * .001 * x0_dot), (dir* deltaT), (dir* (PMDint32)deltaT * .001));
#if TRAPPROFILE == 0
					if (x0 > posMaxCM) {
						x0 = posMaxCM;
					}
					else if (x0 < posMinCM) {
						x0 = posMinCM;
					}
#endif
				
#else
				sineTime = (TIME[i] * .001);
				x0 = positionAmplitude * sin(freqFactor * sineTime - positionPhase) + positionAmplitude;
				x0_dot = velocityAmplitude * sin(freqFactor * sineTime);
				x0_ddot = accelerationAmplitude * cos(freqFactor * sineTime);
#endif // !SINCECURVE

				SendParts(hPeriphSer, binSave, TIME[i], POS[i], VeL[i], force[i], motCom[i], DesVeL[i]);
				i++;
				PMD_RESULT(PMDGetTime(hAxis1, &cycTime));
				PMD_RESULT(PMDGetInstructionError(hAxis1, &instructionerror));
				if (instructionerror != 0) {
					PMDGetErrorMessage(instructionerror);
				}
			}
		}

		if (i == 15) {
			firstRun = 0;
			i = 4;
		}

		// Updates Motor Torque using Impedance Control
		if (i == 4 & firstRun == 0) j = 14;
		else j = i - 1;
		Fext = 0.001 * (force[j] - fOffset);
		e = .01 * (x - x0);
#ifndef SINECURVE
#if TRAPPROFILE == 1
		e_dot = (x_dot)-(x0_dot * .01);
#else
		e_dot = (x_dot) - dir * x0_dot;
#endif // TRAPPROFILE
#ifndef Viscous
		F = fgain * Fext - mMd * (Dd * e_dot + Kd * e) + dir * Tcoulomb;
#else
#if TRAPPROFILE == 1
		F = fgain * Fext - mMd * (Dd * e_dot + Kd * e) + dir * Tcoulomb + viscousFrictionGain * x_dot + m * x0_ddot;
#else
		F = fgain * Fext - mMd * (Dd * e_dot + Kd * e) + dir * Tcoulomb + viscousFrictionGain * x_dot;
#endif
#endif // !Viscous
#else
		e_dot = (x_dot)-(x0_dot * .01);
#if SMOOTH == 0
		if (x_dot > 0) {
			velDir = 1;
		}
		else if (x_dot < 0) {
			velDir = -1;
		}
		else velDir = 0;
#else
		velDir = tanh(200.0 * x_dot);
#endif
#ifndef Viscous
		F = fgain * Fext - mMd * (Dd * e_dot + Kd * e) + velDir * Tcoulomb + m * x0_ddot;
#else
		F = fgain * Fext - mMd * (Dd * e_dot + Kd * e) + velDir * Tcoulomb + viscousFrictionGain * x_dot + m * x0_ddot;
#endif // !Viscous
#endif // !SINECURVE
		command = F * motScale;
		//PMDprintf("velDir = %d, visGain = %f, x_dot = %f, velDir*visGain = %f, all = %f\r\n", velDir, viscousFrictionGain, x_dot, (velDir* viscousFrictionGain), (velDir* (viscousFrictionGain* fabs(x_dot))));
		//PMDprintf("x0 = %f, x = %f, x= %f, e = %f\r\n", x0, (counts2cm * (POS[j] - pOffset)), x, e);
		//PMDprintf("F = %f, Fext = %f, e = %f, e_dot = %f, Tcoulomb = %f, viscous = %f, x0_ddot = %f\r\n", F, (fgain * Fext), (-mMd * Kd * e), (-mMd * Dd * e_dot), (velDir * Tcoulomb), (velDir * (viscousFrictionGain * fabs(x_dot))), (m * x0_ddot));
		//PMDprintf("x_dot = %f, velDir = %f");
		//PMDprintf("VeL = %f, x0_dot = %f, e_dot = %f, x = %f, x0 = %f\r\n", (x_dot), x0_dot, e_dot, x, x0);
		if (abs(F) > 30) {
			if (F > 0) {
				command = 30.0 * motScale;
			}
			else {
				command = -30.0 * motScale;
			}
		}
#if EKFONLINE == 1
		EKF_Predict(&ekf, F, Fext, deltaT_s, Tcoulomb, viscousFrictionGain, m);
		EKF_Update(&ekf, x);
#endif // EKFONLINE == 1

		PMD_RESULT(PMDGetTime(hAxis1, &curTime));
		if ((curTime - serTime) >= serTimeCounts) {
			Take(hPeriphSer);
			if (comConfig._rep != countInput) {
				countInput = comConfig._rep;
			}
			if (comConfig._startPos != posMin) {
				if (comConfig._startPos > posPLim) {
					comConfig._startPos = posPLim;
				}
				posMin = comConfig._startPos;
				posFor = posMin + 50;
				posMinCM = posMin * counts2cm;
			}
			if (comConfig._endPos != posMax) {
				if (comConfig._endPos < posNLim) {
					comConfig._endPos = posNLim;
				}
				posMax = comConfig._endPos;
				posBac = posMax - 50;
				posMaxCM = posMax * counts2cm + posCMAdd;
			}
			if (comConfig._desVel != desiredVelocity) {
				desiredVelocity = comConfig._desVel;
				x0_dot = .01 * round(counts2cm * desiredVelocity * 100) / 100;
				velMaxCM = x0_dot * 100;
				if (trap == 1) {
					trapProfile(accel, velMaxCM, &velPeak, &TimeAccel, &TimeCon, &TimeMotionTotal, posMaxCM, posMinCM);
				}
				else {
					trapProfile2(accel, velMaxCM, &velPeak, &x_accel, &x_const, &x_total, posMaxCM, posMinCM);
				}
			}
			if (comConfig._torq != comInput) {
				comInput = 1000 * comConfig._torq / motScale;
			}
			if (comConfig._mode != 77) {
				run = 0;
			}
			serTime = curTime;
		}

	}
	PMDprintf("Finished Assistive Trajectory!\r\n");
	PMD_RESULT(PMDSetMotorCommand(hAxis1, 0));
	PMD_RESULT(PMDUpdate(hAxis1));
	SendImpVars(hPeriphSer, m, Md, Dd, Kd);
	if (save)
	{
		tSize = sizeof(TIME);
		pSize = sizeof(POS);
		vSize = sizeof(VeL);
		mSize = sizeof(motCom);
		fSize = sizeof(force);
		dSize = sizeof(DesVeL);
		PMDTaskWait(1000);
		Send(hPeriphSer, 0b00000000, TIME, tSize, POS, pSize, VeL, vSize, force, fSize, motCom, mSize, DesVeL, dSize);

	}

#if EKFONLINE == 1
	PMDprintf("Sigma = \r\n");
	PMDprintf("        %f  %f\r\n", ekf.sigma[0][0], ekf.sigma[0][1]);
	PMDprintf("        %f  %f\r\n", ekf.sigma[1][0], ekf.sigma[1][1]);
	PMDprintf("Sigma_pred = \r\n");
	PMDprintf("        %f  %f\r\n", ekf.sigma_pred[0][0], ekf.sigma_pred[0][1]);
	PMDprintf("        %f  %f\r\n", ekf.sigma_pred[1][0], ekf.sigma_pred[1][1]);
	PMDprintf("mu = %f, mu_dot = %f\r\n", ekf.mu[0], ekf.mu[1]);
#endif // EKFONLINE == 1

	return result;

}