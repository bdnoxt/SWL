#include "stdafx.h"
#include "swl/Config.h"
#include "ImuFilterRunner.h"
#include "AdisUsbz.h"
#include "swl/rnd_util/ExtendedKalmanFilter.h"
#include "swl/rnd_util/DiscreteNonlinearStochasticSystem.h"
#include <gsl/gsl_blas.h>
#include <iostream>


#if defined(_DEBUG) && defined(__SWL_CONFIG__USE_DEBUG_NEW)
#include "swl/ResourceLeakageCheck.h"
#define new DEBUG_NEW
#endif


namespace {

class ImuSystem: public swl::DiscreteNonlinearStochasticSystem
{
public:
	typedef swl::DiscreteNonlinearStochasticSystem base_type;

public:
	ImuSystem(const double Ts, const size_t stateDim, const size_t inputDim, const size_t outputDim, const gsl_vector *initial_gravity, const gsl_matrix *Qd, const gsl_matrix *Rd)
	: base_type(stateDim, inputDim, outputDim, (size_t)-1, (size_t)-1),
	  Ts_(Ts), Phi_(NULL), A_(NULL), B_(NULL), Bd_(NULL), Bu_(NULL), Cd_(NULL), Qd_(NULL), Rd_(NULL), f_eval_(NULL), h_eval_(NULL), initial_gravity_(NULL),
	  A_tmp_(NULL)
	{
		// Phi = exp(A * Ts) -> I + A * Ts where A = df/dx
		//	the EKF approximation for Phi is I + A * Ts
		Phi_ = gsl_matrix_alloc(stateDim_, stateDim_);
		gsl_matrix_set_zero(Phi_);

		A_ = gsl_matrix_alloc(stateDim_, stateDim_);
		gsl_matrix_set_zero(A_);

		// B = [ 0 0 0 ; 0 0 0 ; 0 0 0 ; 1 0 0 ; 0 1 0 ; 0 0 1 ; 0 0 0 ; ... ; 0 0 0 ]
		B_ = gsl_matrix_alloc(stateDim_, inputDim_);
		gsl_matrix_set_zero(B_);
		gsl_matrix_set(B_, 3, 0, 1.0);
		gsl_matrix_set(B_, 4, 1, 1.0);
		gsl_matrix_set(B_, 5, 2, 1.0);

		Bd_ = gsl_matrix_alloc(stateDim_, inputDim_);
		Bu_ = gsl_vector_alloc(stateDim_);
		gsl_vector_set_zero(Bu_);

		// C = [
		//	0 0 0  0 0 0  1 0 0   0 0 0 0  0 0 0  1 0 0  0 0 0
		//	0 0 0  0 0 0  0 1 0   0 0 0 0  0 0 0  0 1 0  0 0 0
		//	0 0 0  0 0 0  0 0 1   0 0 0 0  0 0 0  0 0 1  0 0 0
		//	0 0 0  0 0 0  0 0 0   0 0 0 0  1 0 0  0 0 0  1 0 0
		//	0 0 0  0 0 0  0 0 0   0 0 0 0  0 1 0  0 0 0  0 1 0
		//	0 0 0  0 0 0  0 0 0   0 0 0 0  0 0 1  0 0 0  0 0 1
		// ]
		Cd_ = gsl_matrix_alloc(outputDim_, stateDim_);
		gsl_matrix_set_zero(Cd_);
		gsl_matrix_set(Cd_, 0, 6, 1.0);  gsl_matrix_set(Cd_, 0, 16, 1.0);
		gsl_matrix_set(Cd_, 1, 7, 1.0);  gsl_matrix_set(Cd_, 1, 17, 1.0);
		gsl_matrix_set(Cd_, 2, 8, 1.0);  gsl_matrix_set(Cd_, 2, 18, 1.0);
		gsl_matrix_set(Cd_, 3, 13, 1.0);  gsl_matrix_set(Cd_, 3, 19, 1.0);
		gsl_matrix_set(Cd_, 4, 14, 1.0);  gsl_matrix_set(Cd_, 4, 20, 1.0);
		gsl_matrix_set(Cd_, 5, 15, 1.0);  gsl_matrix_set(Cd_, 5, 21, 1.0);

		// Qd = W * Q * W^T 
		//	the EKF approximation of Qd will be W * [ Q * Ts ] * W^T
		Qd_ = gsl_matrix_alloc(stateDim_, stateDim_);
		gsl_matrix_memcpy(Qd_, Qd);

		// Rd = V * R * V^T
		Rd_ = gsl_matrix_alloc(outputDim_, outputDim_);
		gsl_matrix_memcpy(Rd_, Rd);

		//
		f_eval_ = gsl_vector_alloc(stateDim_);
		gsl_vector_set_zero(f_eval_);

		//
		h_eval_ = gsl_vector_alloc(outputDim_);
		gsl_vector_set_zero(h_eval_);

		// initial gravity
		initial_gravity_ = gsl_vector_alloc(initial_gravity->size);
		gsl_vector_memcpy(initial_gravity_, initial_gravity);

		//
		A_tmp_ = gsl_matrix_alloc(stateDim_, stateDim_);
	}
	~ImuSystem()
	{
		gsl_matrix_free(Phi_);  Phi_ = NULL;
		gsl_matrix_free(A_);  A_ = NULL;
		gsl_matrix_free(B_);  B_ = NULL;
		gsl_matrix_free(Bd_);  Bd_ = NULL;
		gsl_vector_free(Bu_);  Bu_ = NULL;
		gsl_matrix_free(Cd_);  Cd_ = NULL;
		gsl_matrix_free(Qd_);  Qd_ = NULL;
		gsl_matrix_free(Rd_);  Rd_ = NULL;

		gsl_vector_free(f_eval_);  f_eval_ = NULL;
		gsl_vector_free(h_eval_);  h_eval_ = NULL;

		gsl_vector_free(initial_gravity_);  initial_gravity_ = NULL;

		gsl_matrix_free(A_tmp_);  A_tmp_ = NULL;
	}

private:
	ImuSystem(const ImuSystem &rhs);
	ImuSystem & operator=(const ImuSystem &rhs);

public:
	// the stochastic differential equation: f = f(k, x(k), u(k), v(k))
	/*virtual*/ gsl_vector * evaluatePlantEquation(const size_t step, const gsl_vector *state, const gsl_vector *input, const gsl_vector *noise) const
	{
		const gsl_matrix *Phi = getStateTransitionMatrix(step, state);
		const gsl_vector *Bu = getControlInput(step, state);
		gsl_vector_memcpy(f_eval_, Bu);
		gsl_blas_dgemv(CblasNoTrans, 1.0, Phi, state, 1.0, f_eval_);

		return f_eval_;
	}
	/*virtual*/ gsl_matrix * getStateTransitionMatrix(const size_t step, const gsl_vector *state) const  // Phi(k) = exp(A(k) * Ts) where A(k) = df(k, x(k), u(k), 0)/dx
	{
		const double &Px = gsl_vector_get(state, 0);
		const double &Py = gsl_vector_get(state, 1);
		const double &Pz = gsl_vector_get(state, 2);
		const double &Vx = gsl_vector_get(state, 3);
		const double &Vy = gsl_vector_get(state, 4);
		const double &Vz = gsl_vector_get(state, 5);
		const double &Ap = gsl_vector_get(state, 6);
		const double &Aq = gsl_vector_get(state, 7);
		const double &Ar = gsl_vector_get(state, 8);
		const double &E0 = gsl_vector_get(state, 9);
		const double &E1 = gsl_vector_get(state, 10);
		const double &E2 = gsl_vector_get(state, 11);
		const double &E3 = gsl_vector_get(state, 12);
		const double &Wp = gsl_vector_get(state, 13);
		const double &Wq = gsl_vector_get(state, 14);
		const double &Wr = gsl_vector_get(state, 15);
		const double &Abp = gsl_vector_get(state, 16);
		const double &Abq = gsl_vector_get(state, 17);
		const double &Abr = gsl_vector_get(state, 18);
		const double &Wbp = gsl_vector_get(state, 19);
		const double &Wbq = gsl_vector_get(state, 20);
		const double &Wbr = gsl_vector_get(state, 21);

		gsl_matrix_set(A_, 0, 3, 1.0);
		gsl_matrix_set(A_, 1, 4, 1.0);
		gsl_matrix_set(A_, 2, 5, 1.0);

		gsl_matrix_set(A_, 3, 6, 2.0 * (0.5 - E2*E2 - E3*E3));
		gsl_matrix_set(A_, 3, 7, 2.0 * (E1*E2 - E0*E3));
		gsl_matrix_set(A_, 3, 8, 2.0 * (E1*E3 + E0*E2));
		gsl_matrix_set(A_, 3, 9, 2.0 * (-E3*Aq + E2*Ar));
		gsl_matrix_set(A_, 3, 10, 2.0 * (E2*Aq + E3*Ar));
		gsl_matrix_set(A_, 3, 11, 2.0 * (-2.0*E2*Ap + E1*Aq + E0*Ar));
		gsl_matrix_set(A_, 3, 12, 2.0 * (-2.0*E3*Ap - E0*Aq + E1*Ar));

		gsl_matrix_set(A_, 4, 6, 2.0 * (E1*E2 + E0*E3));
		gsl_matrix_set(A_, 4, 7, 2.0 * (0.5 - E1*E1 - E3*E3));
		gsl_matrix_set(A_, 4, 8, 2.0 * (E2*E3 - E0*E1));
		gsl_matrix_set(A_, 4, 9, 2.0 * (E3*Ap - E1*Ar));
		gsl_matrix_set(A_, 4, 10, 2.0 * (E2*Ap - 2.0*E1*Aq - E0*Ar));
		gsl_matrix_set(A_, 4, 11, 2.0 * (E1*Ap + E3*Ar));
		gsl_matrix_set(A_, 4, 12, 2.0 * (E0*Ap - 2.0*E3*Aq + E2*Ar));

		gsl_matrix_set(A_, 5, 6, 2.0 * (E1*E3 - E0*E2));
		gsl_matrix_set(A_, 5, 7, 2.0 * (E2*E3 + E0*E1));
		gsl_matrix_set(A_, 5, 8, 2.0 * (0.5 - E1*E1 - E2*E2));
		gsl_matrix_set(A_, 5, 9, 2.0 * (-E2*Ap + E1*Aq));
		gsl_matrix_set(A_, 5, 10, 2.0 * (E3*Ap + E0*Aq - 2.0*E1*Ar));
		gsl_matrix_set(A_, 5, 11, 2.0 * (-E0*Ap + E3*Aq - 2.0*E2*Ar));
		gsl_matrix_set(A_, 5, 12, 2.0 * (E1*Ap + E2*Aq));

		gsl_matrix_set(A_, 9, 9, 0.0);
		gsl_matrix_set(A_, 9, 10, -0.5 * Wp);
		gsl_matrix_set(A_, 9, 11, -0.5 * Wq);
		gsl_matrix_set(A_, 9, 12, -0.5 * Wr);
		gsl_matrix_set(A_, 9, 13, -0.5 * E1);
		gsl_matrix_set(A_, 9, 14, -0.5 * E2);
		gsl_matrix_set(A_, 9, 15, -0.5 * E3);

		gsl_matrix_set(A_, 10, 9, -0.5 * -Wp);
		gsl_matrix_set(A_, 10, 10, 0.0);
		gsl_matrix_set(A_, 10, 11, -0.5 * -Wr);
		gsl_matrix_set(A_, 10, 12, -0.5 * Wq);
		gsl_matrix_set(A_, 10, 13, -0.5 * -E0);
		gsl_matrix_set(A_, 10, 14, -0.5 * E3);
		gsl_matrix_set(A_, 10, 15, -0.5 * -E2);

		gsl_matrix_set(A_, 11, 9, -0.5 * -Wq);
		gsl_matrix_set(A_, 11, 10, -0.5 * Wr);
		gsl_matrix_set(A_, 11, 11, 0.0);
		gsl_matrix_set(A_, 11, 12, -0.5 * -Wp);
		gsl_matrix_set(A_, 11, 13, -0.5 * -E3);
		gsl_matrix_set(A_, 11, 14, -0.5 * -E0);
		gsl_matrix_set(A_, 11, 15, -0.5 * E1);

		gsl_matrix_set(A_, 12, 9, -0.5 * -Wr);
		gsl_matrix_set(A_, 12, 10, -0.5 * -Wq);
		gsl_matrix_set(A_, 12, 11, -0.5 * Wp);
		gsl_matrix_set(A_, 12, 12, 0.0);
		gsl_matrix_set(A_, 12, 13, -0.5 * E2);
		gsl_matrix_set(A_, 12, 14, -0.5 * -E1);
		gsl_matrix_set(A_, 12, 15, -0.5 * -E0);

		// Phi = exp(A * Ts) -> I + A * Ts where A = df/dx
		//	the EKF approximation for Phi is I + A * Ts

		gsl_matrix_memcpy(Phi_, A_);
		gsl_matrix_scale(Phi_, Ts_);
		for (size_t i = 0; i < stateDim_; ++i)
			gsl_matrix_set(Phi_, i, i, gsl_matrix_get(Phi_, i, i) + 1.0);

		return Phi_;
	}
	/*virtual*/ gsl_vector * getControlInput(const size_t step, const gsl_vector *state) const  // Bu(k) = Bd(k) * u(k)
	{
		// Bu = Bd * u where u(t) = initial gravity
		//	Bd = integrate(exp(A * t), {t, 0, Ts}) * B or A^-1 * (Ad - I) * B if A is nonsingular
		//	Ad = Phi = exp(A * Ts) -> I + A * Ts where A = df/dx

		// TODO [check] >>
		//	integrate(exp(A * t), {t, 0, Ts}) -> integrate(I + A * t, {t, 0, Ts}) -> I * Ts + A * Ts^2 / 2
		//	Bd = integrate(exp(A * t), {t, 0, Ts}) * B -> integrate(I + A * t, {t, 0, Ts}) * B -> (I * Ts + A * Ts^2 / 2) * B ???
		gsl_matrix_memcpy(A_tmp_, A_);
		gsl_matrix_scale(A_tmp_, 0.5 * Ts_*Ts_);
		for (size_t i = 0; i < stateDim_; ++i)
			gsl_matrix_set(A_tmp_, i, i, gsl_matrix_get(A_tmp_, i, i) + Ts_);

		gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, A_tmp_, B_, 0.0, Bd_);
		gsl_blas_dgemv(CblasNoTrans, 1.0, Bd_, initial_gravity_, 0.0, Bu_);

		return Bu_;
	}
	///*virtual*/ gsl_matrix * getProcessNoiseCouplingMatrix(const size_t step) const  {  return W_;  }  // W(k) = df(k, x(k), u(k), 0)/dw

	// the observation equation: h = h(k, x(k), u(k), v(k))
	/*virtual*/ gsl_vector * evaluateMeasurementEquation(const size_t step, const gsl_vector *state, const gsl_vector *input, const gsl_vector *noise) const 
	{
		gsl_blas_dgemv(CblasNoTrans, 1.0, Cd_, state, 0.0, h_eval_);
		return h_eval_;
	}
	/*virtual*/ gsl_matrix * getOutputMatrix(const size_t step, const gsl_vector *state) const  // Cd(k) = dh(k, x(k), u(k), 0)/dx
	{  return Cd_;  }
	/*virtual*/ gsl_vector * getMeasurementInput(const size_t step, const gsl_vector *state) const  // Du(k) = D(k) * u(k) (D == Dd)
	{  throw std::runtime_error("this function doesn't have to be called");  }
	///*virtual*/ gsl_matrix * getMeasurementNoiseCouplingMatrix(const size_t step) const  {  return V_;  }  // V(k) = dh(k, x(k), u(k), 0)/dv

	// noise covariance matrices
	/*virtual*/ gsl_matrix * getProcessNoiseCovarianceMatrix(const size_t step) const  {  return Qd_;  }  // Qd = W * Q * W^T, but not Q
	/*virtual*/ gsl_matrix * getMeasurementNoiseCovarianceMatrix(const size_t step) const  {  return Rd_;  }  // Rd = V * R * V^T, but not R

private:
	const double Ts_;

	gsl_matrix *Phi_;
	gsl_matrix *A_;  // A = df/dx
	gsl_matrix *B_;  // B = df/du
	gsl_matrix *Bd_;  // Bd = integrate(exp(A * t), {t, 0, Ts}) * B or A^-1 * (Ad - I) * B if A is nonsingular
	gsl_vector *Bu_;  // Bu = Bd * u
	gsl_matrix *Cd_;  // Cd = C
	gsl_matrix *Qd_;
	gsl_matrix *Rd_;

	// evalution of the plant equation: f = f(k, x(k), u(k), 0)
	gsl_vector *f_eval_;
	// evalution of the measurement equation: h = h(k, x(k), u(k), 0)
	gsl_vector *h_eval_;

	// initial gravity
	gsl_vector *initial_gravity_;

	gsl_matrix *A_tmp_;
};

}  // unnamed namespace

// "Sigma-Point Kalman Filters for Integrated Navigation", R. van der Merwe and Eric A. Wan,
//	Annual Meeting of The Institute of Navigation, 2004
// "A new multi-position calibration method for MEMS inertial navigation systems", Z. F. Syed, P. Aggarwal, C. Goodall, X. Niu, and N. El-Sheimy,
//	Measurement Science and Technology, vol. 18, pp. 1897-1907, 2007

void imu_filter_with_calibration()
{
	const size_t stateDim = 22;
	const size_t inputDim = 3;
	const size_t outputDim = 6;

	// sampling interval
	//const double Ts = 0.01;  // [sec]
	const double Ts = 0.016 / 5;  // [sec]

	const size_t Ninitial = 10000;

	//
#if defined(__USE_ADISUSBZ_DATA)
	AdisUsbz adis;

#if defined(UNICODE) || defined(_UNICODE)
	if (!adis.Initialize(L"\\\\.\\Ezusb-0"))
#else
	if (!adis.Initialize("\\\\.\\Ezusb-0"))
#endif
	{
		std::cout << "fail to initialize ADISUSBZ" << std::endl;
		return;
	}

	ImuFilterRunner runner(Ts, stateDim, inputDim, outputDim, &adis);
#else
	ImuFilterRunner runner(Ts, stateDim, inputDim, outputDim, NULL);
#endif

	// load calibration parameters
	std::cout << "load calibration parameters ..." << std::endl;
	const std::string calibration_filename("..\\data\\adis16350_data_20100801\\imu_calibration_result.txt");
	runner.loadCalibrationParam(calibration_filename);

#if defined(__USE_ADISUSBZ_DATA)
	// test ADISUSBZ
	//ImuFilterRunner::testAdisUsbz(Ntest);

	std::cout << "set an initial gravity ..." << std::endl;
	// set an initial gravity
	runner.initializeGravity(Ninitial);
#else
	std::vector<ImuFilterRunner::Acceleration> accels;
	std::vector<ImuFilterRunner::Gyro> gyros;
	accels.reserve(Ninitial);
	gyros.reserve(Ninitial);

	// load validation data
	ImuFilterRunner::loadSavedImuData("..\\data\\adis16350_data_20100801\\01_z_pos.csv", accels, gyros);
	//ImuFilterRunner::loadSavedImuData("..\\data\\adis16350_data_20100801\\02_z_neg.csv", accels, gyros);

	std::cout << "set an initial gravity ..." << std::endl;
	// set an initial gravity
	runner.initializeGravity(Ninitial, accels, gyros);
#endif

	const gsl_vector *initialGravity = runner.getInitialGravity();

	//
	gsl_vector *x0 = gsl_vector_alloc(stateDim);
	gsl_vector_set_zero(x0);
	gsl_vector_set(x0, 6, -gsl_vector_get(initialGravity, 0));  // a_p = g_initial_x
	gsl_vector_set(x0, 7, -gsl_vector_get(initialGravity, 1));  // a_q = g_initial_y
	gsl_vector_set(x0, 8, -gsl_vector_get(initialGravity, 2));  // a_r = g_initial_z
	gsl_vector_set(x0, 9, 1.0);  // e0 = 1.0
	gsl_matrix *P0 = gsl_matrix_alloc(stateDim, stateDim);
	gsl_matrix_set_identity(P0);
	gsl_matrix_scale(P0, 1.0e-8);  // the initial estimate is completely unknown

	// Qd = W * Q * W^T where W = I
	//	the EKF approximation of Qd will be W * [ Q * Ts ] * W^T
	gsl_matrix *Qd = gsl_matrix_alloc(stateDim, stateDim);
	gsl_matrix_set_zero(Qd);
	// FIXME [modify] >>
	const double QQ = 1.0e-8;
	gsl_matrix_set(Qd, 0, 0, QQ);
	gsl_matrix_set(Qd, 1, 1, QQ);
	gsl_matrix_set(Qd, 2, 2, QQ);
	gsl_matrix_set(Qd, 3, 3, QQ);
	gsl_matrix_set(Qd, 4, 4, QQ);
	gsl_matrix_set(Qd, 5, 5, QQ);
	gsl_matrix_set(Qd, 6, 6, QQ);
	gsl_matrix_set(Qd, 7, 7, QQ);
	gsl_matrix_set(Qd, 8, 8, QQ);
	gsl_matrix_set(Qd, 9, 9, QQ);
	gsl_matrix_set(Qd, 10, 10, QQ);
	gsl_matrix_set(Qd, 11, 11, QQ);
	gsl_matrix_set(Qd, 12, 12, QQ);
	gsl_matrix_set(Qd, 13, 13, QQ);
	gsl_matrix_set(Qd, 14, 14, QQ);
	gsl_matrix_set(Qd, 15, 15, QQ);
	gsl_matrix_set(Qd, 16, 16, QQ);
	gsl_matrix_set(Qd, 17, 17, QQ);
	gsl_matrix_set(Qd, 18, 18, QQ);
	gsl_matrix_set(Qd, 19, 19, QQ);
	gsl_matrix_set(Qd, 20, 20, QQ);
	gsl_matrix_set(Qd, 21, 21, QQ);
	gsl_matrix_scale(Qd, Ts);

	// Rd = V * R * V^T where V = I
	gsl_matrix *Rd = gsl_matrix_alloc(outputDim, outputDim);
	gsl_matrix_set_zero(Rd);
	// FIXME [modify] >>
	const double RR = 1.0e-8;
	gsl_matrix_set(Rd, 0, 0, RR);
	gsl_matrix_set(Rd, 1, 1, RR);
	gsl_matrix_set(Rd, 2, 2, RR);
	gsl_matrix_set(Rd, 3, 3, RR);
	gsl_matrix_set(Rd, 4, 4, RR);
	gsl_matrix_set(Rd, 5, 5, RR);

	const ImuSystem system(Ts, stateDim, inputDim, outputDim, initialGravity, Qd, Rd);
	swl::DiscreteExtendedKalmanFilter filter(system, x0, P0);

	gsl_vector_free(x0);  x0 = NULL;
	gsl_matrix_free(P0);  P0 = NULL;
	gsl_matrix_free(Qd);  Qd = NULL;
	gsl_matrix_free(Rd);  Rd = NULL;

	// extended Kalman filtering
	std::cout << "start extended Kalman filtering ..." << std::endl;

#if defined(__USE_ADISUSBZ_DATA)
	const size_t Nstep = 10000;
#else
	const size_t Nstep = Ninitial;
#endif

	gsl_vector *measuredAccel = gsl_vector_alloc(3);
	gsl_vector *measuredAngularVel = gsl_vector_alloc(3);

	size_t step = 0;
	while (step < Nstep)
	{
#if defined(__USE_ADISUSBZ_DATA)
		runner.readAdisData(measuredAccel, measuredAngularVel);
#else
		gsl_vector_set(measuredAccel, 0, accels[step].x);
		gsl_vector_set(measuredAccel, 1, accels[step].y);
		gsl_vector_set(measuredAccel, 2, accels[step].z);
		gsl_vector_set(measuredAngularVel, 0, gyros[step].x);
		gsl_vector_set(measuredAngularVel, 1, gyros[step].y);
		gsl_vector_set(measuredAngularVel, 2, gyros[step].z);
#endif

		if (!runner.runImuFilter(filter, step, measuredAccel, measuredAngularVel))
		{
			std::cout << "IMU filtering error !!!" << std::endl;
			return;
		}

		const gsl_vector *pos = runner.getFilteredPos();
		const gsl_vector *vel = runner.getFilteredVel();
		const gsl_vector *accel = runner.getFilteredAccel();
		const gsl_vector *quat = runner.getFilteredQuaternion();
		const gsl_vector *angVel = runner.getFilteredAngularVel();

		std::cout << (step + 1) << " : " << gsl_vector_get(pos, 0) << ", " << gsl_vector_get(pos, 1) << ", " << gsl_vector_get(pos, 2) << " ; " <<
			gsl_vector_get(quat, 0) << ", " << gsl_vector_get(quat, 1) << ", " << gsl_vector_get(quat, 2) << ", " << gsl_vector_get(quat, 3) << std::endl;

		++step;
	}

	gsl_vector_free(measuredAccel);  measuredAccel = NULL;
	gsl_vector_free(measuredAngularVel);  measuredAngularVel = NULL;
}