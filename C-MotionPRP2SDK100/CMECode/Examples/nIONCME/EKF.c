#include "EKF.h"
#include <string.h>
#include <math.h>

ExtendedFilterNoise ekfNoise = { 
	{{1e-6, 0},
	{0, 1e-4}},
	 1e-4
};

// Identity matrix 2x2
static const float I2[2][2] = {
	{1.0f, 0.0f},
	{0.0f, 1.0f}
};

// Matric Operation Helper Functions
static inline void mat2x2_add(const double A[2][2], const double B[2][2], double out[2][2]) {
	out[0][0] = A[0][0] + B[0][0];
	out[0][1] = A[0][1] + B[0][1];
	out[1][0] = A[1][0] + B[1][0];
	out[1][1] = A[1][1] + B[1][1];
}

static inline void mat2x2_sub(const double A[2][2], const double B[2][2], double out[2][2]) {
	out[0][0] = A[0][0] - B[0][0];
	out[0][1] = A[0][1] - B[0][1];
	out[1][0] = A[1][0] - B[1][0];
	out[1][1] = A[1][1] - B[1][1];
}

static inline void mat2x2_mult(const double A[2][2], const double B[2][2], double out[2][2]) {
	out[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0];
	out[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1];
	out[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0];
	out[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1];
}

static inline void mat2x2_mult_transposeB(const double A[2][2], const double B[2][2], double out[2][2]) {
	// out = A * B^T
	out[0][0] = A[0][0] * B[0][0] + A[0][1] * B[0][1];
	out[0][1] = A[0][0] * B[1][0] + A[0][1] * B[1][1];
	out[1][0] = A[1][0] * B[0][0] + A[1][1] * B[0][1];
	out[1][1] = A[1][0] * B[1][0] + A[1][1] * B[1][1];
}

// Main EKF Functions
void EKF_init(EKFVars* ekf, float x0, float v0) {
	ekf->mu[0] = x0;
	ekf->mu[1] = v0;
	// Initial covariance: small uncertainties
	ekf->sigma[0][0] = 1e-4; ekf->sigma[0][1] = 0.0;
	ekf->sigma[1][0] = 0.0;  ekf->sigma[1][1] = 1e-3;
}

void EKF_Predict(EKFVars* ekf, double F, double F_ext, double dt, double Tc, double viscousGain, double m) {

	double pos = ekf->mu[0];
	double vel = ekf->mu[1];
	double velLast = ekf->velLast;
	
	int velDir = 0;

	double k = 50.0;
	double tanh_term = tanh(k * vel);
	double F_fric = tanh_term * Tc + viscousGain * vel;

	double acc = (1.0/m)*(F+F_ext-F_fric);

	ekf->mu[0] = pos + 0.5 * (vel + velLast) * dt;
	ekf->mu[1] = vel + acc * dt;
	ekf->velLast = vel;

	double G[2][2] = { {1,  dt},
					{0, 1 - (dt / m) * (viscousGain + Tc * k * (1 - tanh_term * tanh_term))}
	};

	double sigma_pred[2][2];
	double temp[2][2];

	mat2x2_mult(G, ekf->sigma, temp);
	mat2x2_mult_transposeB(temp, G, sigma_pred);
	mat2x2_add(sigma_pred, ekfNoise._Q, ekf->sigma_pred);

	PMDprintf("Mu: %.6f, %.6f\n", ekf->mu[0], ekf->mu[1]);
	PMDprintf("Sigma: %.6f, %.6f\n", ekf->sigma[0][0], ekf->sigma[1][1]);

	if (isnan(ekf->mu[0]) || isnan(ekf->mu[1])) {
		PMDprintf("NaN detected in mu: %.6f, %.6f\n", ekf->mu[0], ekf->mu[1]);
	}
	if (isnan(ekf->sigma[0][0]) || isnan(ekf->sigma[1][1])) {
		PMDprintf("NaN detected in sigma: %.6f, %.6f\n", ekf->sigma[0][0], ekf->sigma[1][1]);
	}


}

void EKF_Update(EKFVars* ekf, double z) {

	float H[2] = { 1.0f, 0.0f };
	float H_Tran[1][2] = { H[0], H[1] };

	double InnerTemp = ekf->sigma_pred[0][0] + ekfNoise._R;

	double K[2];
	K[0] = ekf->sigma_pred[0][0] / InnerTemp;
	K[1] = ekf->sigma_pred[1][0] / InnerTemp;

	double error = z - ekf->mu[0];

	ekf->mu[0] += K[0] * error;
	ekf->mu[1] += K[1] * error;

	double KH[2][2] = { 
		{K[0], 0}, 
		{K[1], 0} 
	};

	double I_KH[2][2];
	mat2x2_sub(I2,KH, I_KH);
	mat2x2_mult(I_KH, ekf->sigma_pred, ekf->sigma);

	if (isnan(ekf->mu[0]) || isnan(ekf->mu[1])) {
		PMDprintf("NaN detected in mu: %.6f, %.6f\n", ekf->mu[0], ekf->mu[1]);
	}
	if (isnan(ekf->sigma[0][0]) || isnan(ekf->sigma[1][1])) {
		PMDprintf("NaN detected in sigma: %.6f, %.6f\n", ekf->sigma[0][0], ekf->sigma[1][1]);
	}
}



/*
//if (G11 < -0.99) G11 = -0.99;
//if (G11 > 1.5)   G11 = 1.5;

//double G[2][2] = {
//	{1,  dt},
//	{0, G11}
//};
*/