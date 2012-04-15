#if !defined(__SWL_RND_UTIL__HMM_WITH_UNIVARIATE_GAUSSIAN_MIXTURE_OBSERVATIONS__H_)
#define __SWL_RND_UTIL__HMM_WITH_UNIVARIATE_GAUSSIAN_MIXTURE_OBSERVATIONS__H_ 1


#include "swl/rnd_util/CDHMM.h"
#include "swl/rnd_util/HmmWithMixtureObservations.h"


namespace swl {

//--------------------------------------------------------------------------
// continuous density HMM with univariate Gaussian mixture observation densities

class SWL_RND_UTIL_API HmmWithUnivariateGaussianMixtureObservations: public CDHMM, HmmWithMixtureObservations
{
public:
	typedef CDHMM base_type;
	//typedef HmmWithMixtureObservations base_type;

public:
	HmmWithUnivariateGaussianMixtureObservations(const size_t K, const size_t C);
	HmmWithUnivariateGaussianMixtureObservations(const size_t K, const size_t C, const std::vector<double> &pi, const boost::multi_array<double, 2> &A, const std::vector<double> &alpha, const boost::multi_array<double, 2> &mus, const boost::multi_array<double, 2> &sigmas);
	virtual ~HmmWithUnivariateGaussianMixtureObservations();

private:
	HmmWithUnivariateGaussianMixtureObservations(const HmmWithUnivariateGaussianMixtureObservations &rhs);
	HmmWithUnivariateGaussianMixtureObservations & operator=(const HmmWithUnivariateGaussianMixtureObservations &rhs);

public:
	/*virtual*/ bool estimateParameters(const size_t N, const boost::multi_array<double, 2> &observations, const double terminationTolerance, boost::multi_array<double, 2> &alpha, boost::multi_array<double, 2> &beta, boost::multi_array<double, 2> &gamma, size_t &numIteration, double &initLogProbability, double &finalLogProbability);

	//
	boost::multi_array<double, 2> & getMean()  {  return mus_;  }
	const boost::multi_array<double, 2> & getMean() const  {  return mus_;  }
	boost::multi_array<double, 2> & getStandardDeviation()  {  return  sigmas_;  }
	const boost::multi_array<double, 2> & getStandardDeviation() const  {  return  sigmas_;  }

protected:
	// if state == 0, hidden state = [ 1 0 0 ... 0 0 ]
	// if state == 1, hidden state = [ 0 1 0 ... 0 0 ]
	// ...
	// if state == N-1, hidden state = [ 0 0 0 ... 0 1 ]
	/*virtual*/ double evaluateEmissionProbability(const int state, const boost::multi_array<double, 2>::const_array_view<1>::type &observation) const;
	/*virtual*/ void generateObservationsSymbol(const int state, boost::multi_array<double, 2>::array_view<1>::type &observation, const bool setSeed = false) const;

	//
	/*virtual*/ bool readObservationDensity(std::istream &stream);
	/*virtual*/ bool writeObservationDensity(std::ostream &stream) const;
	/*virtual*/ void initializeObservationDensity();

private:
	boost::multi_array<double, 2> mus_;  // the sets of means of each components in the univariate Gaussian mixture distribution
	boost::multi_array<double, 2> sigmas_;  // the sets of standard deviations of each components in the univariate Gaussian mixture distribution
};

}  // namespace swl


#endif  // __SWL_RND_UTIL__HMM_WITH_UNIVARIATE_GAUSSIAN_MIXTURE_OBSERVATIONS__H_
