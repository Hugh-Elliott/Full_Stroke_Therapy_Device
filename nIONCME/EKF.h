#ifndef EKF_H
#define EKF_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once

	// Structure for extended kalman filter noise
	typedef struct {
		double _Q[2][2];
		double _R;
	} ExtendedFilterNoise;

	typedef struct {
		//double x[2];			// State: position, velocity
		double sigma[2][2];		// Covariance Matrix
		double mu[2];			// State Prediction
		double sigma_pred[2][2];// Covariance Matrix Prediction
		double velLast;			// Previous velocity term
	} EKFVars;

	// Declare a global instance of structures
	extern ExtendedFilterNoise ekfNoise;


	//class Used for an extended kalman filter

	// Main EKF Functions
	void EKF_init(EKFVars* ekf, float x0, float v0);	// Initialize EKF
	void EKF_Predict(EKFVars* ekf, double F, double F_ext, double dt, double Tc, double visxousGain, double m); // Prediction Step
	void EKF_Update(EKFVars* ekf, double z);  // Update Step


#if defined(__cplusplus)
}
#endif
#endif // !EKF_H