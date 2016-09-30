//#include "stdafx.h"
#include "swl/Config.h"
#include "swl/rnd_util/HmmWithVonMisesObservations.h"
#include <boost/math/constants/constants.hpp>
#include <boost/smart_ptr.hpp>
#include <sstream>
#include <fstream>
#include <iostream>
#include <ctime>
#include <stdexcept>


#if defined(_DEBUG) && defined(__SWL_CONFIG__USE_DEBUG_NEW)
#include "swl/ResourceLeakageCheck.h"
#define new DEBUG_NEW
#endif


//#define __TEST_HMM_MODEL 1
#define __TEST_HMM_MODEL 2
#define __USE_SPECIFIED_VALUE_FOR_RANDOM_SEED 1


namespace {
namespace local {

void model_reading_and_writing()
{
	// reading a model.
	{
		boost::scoped_ptr<swl::CDHMM> cdhmm;

#if __TEST_HMM_MODEL == 1
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test1.cdhmm");
#elif __TEST_HMM_MODEL == 2
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test2.cdhmm");
#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		cdhmm.reset(new swl::HmmWithVonMisesObservations(K));

		const bool retval = cdhmm->readModel(stream);
		if (!retval)
		{
			std::ostringstream stream;
			stream << "model reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		// normalize pi & A.
		cdhmm->normalizeModelParameters();

		cdhmm->writeModel(std::cout);
	}

	// writing a model.
	{
		boost::scoped_ptr<swl::CDHMM> cdhmm;

#if __TEST_HMM_MODEL == 1
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		const double arrPi[] = {
			1.0/3.0, 1.0/3.0, 1.0/3.0
		};
		const double arrA[] = {
			0.6,  0.15, 0.25,
			0.35, 0.25, 0.40,
			0.15, 0.55, 0.3
		};
		const double arrMu[] = {
			0.0, 2.0944, 4.1888
		};
		const double arrKappa[] = {
			50.0, 90.0, 70.0
		};

		//
		std::ofstream stream("./data/hmm/von_mises_test1_writing.cdhmm");
#elif __TEST_HMM_MODEL == 2
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		const double arrPi[] = {
			1.0/3.0, 1.0/3.0, 1.0/3.0
		};
		const double arrA[] = {
			0.5, 0.2, 0.3,
			0.2, 0.4, 0.4,
			0.1, 0.6, 0.3
		};
		const double arrMu[] = {
			0.0, 1.5708, 4.7124
		};
		const double arrKappa[] = {
			50.0, 100.0, 75.0
		};

		//
		std::ofstream stream("./data/hmm/von_mises_test2_writing.cdhmm");
#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		swl::HmmWithVonMisesObservations::dvector_type pi(boost::numeric::ublas::vector<double, std::vector<double> >(K, std::vector<double>(arrPi, arrPi + K)));
		swl::HmmWithVonMisesObservations::dmatrix_type A(boost::numeric::ublas::matrix<double, boost::numeric::ublas::row_major, std::vector<double> >(K, K, std::vector<double>(arrA, arrA + K * K)));
		swl::HmmWithVonMisesObservations::dvector_type mus(boost::numeric::ublas::vector<double, std::vector<double> >(K, std::vector<double>(arrMu, arrMu + K)));
		swl::HmmWithVonMisesObservations::dvector_type kappas(boost::numeric::ublas::vector<double, std::vector<double> >(K, std::vector<double>(arrKappa, arrKappa + K)));
		cdhmm.reset(new swl::HmmWithVonMisesObservations(K, pi, A, mus, kappas));

		const bool retval = cdhmm->writeModel(stream);
		if (!retval)
		{
			std::ostringstream stream;
			stream << "model writing error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}
	}
}

void observation_sequence_generation(const bool outputToFile)
{
	boost::scoped_ptr<swl::CDHMM> cdhmm;

	// read a model.
	{
#if __TEST_HMM_MODEL == 1
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test1.cdhmm");
#elif __TEST_HMM_MODEL == 2
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test2.cdhmm");
#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		cdhmm.reset(new swl::HmmWithVonMisesObservations(K));

		const bool retval = cdhmm->readModel(stream);
		if (!retval)
		{
			std::ostringstream stream;
			stream << "model reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		// normalize pi & A.
		cdhmm->normalizeModelParameters();

		//cdhmm->writeModel(std::cout);
	}

	// generate a sample sequence.
	{
#if defined(__USE_SPECIFIED_VALUE_FOR_RANDOM_SEED)
		const unsigned int seed = 34586u;
#else
		const unsigned int seed = (unsigned int)std::time(NULL);
#endif
		std::srand(seed);
		std::cout << "random seed: " << seed << std::endl;

		if (outputToFile)
		{
#if __TEST_HMM_MODEL == 1

#if 1
			const size_t N = 50;
			std::ofstream stream("./data/hmm/von_mises_test1_50.seq");
#elif 0
			const size_t N = 100;
			std::ofstream stream("./data/hmm/von_mises_test1_100.seq");
#elif 0
			const size_t N = 1500;
			std::ofstream stream("./data/hmm/von_mises_test1_1500.seq");
#endif

#elif __TEST_HMM_MODEL == 2

#if 1
			const size_t N = 50;
			std::ofstream stream("./data/hmm/von_mises_test2_50.seq");
#elif 0
			const size_t N = 100;
			std::ofstream stream("./data/hmm/von_mises_test2_100.seq");
#elif 0
			const size_t N = 1500;
			std::ofstream stream("./data/hmm/von_mises_test2_1500.seq");
#endif

#endif
			if (!stream)
			{
				std::ostringstream stream;
				stream << "file not found at " << __LINE__ << " in " << __FILE__;
				throw std::runtime_error(stream.str().c_str());
				return;
			}

			swl::CDHMM::dmatrix_type observations(N, cdhmm->getObservationDim(), 0.0);
			swl::CDHMM::uivector_type states(N, (unsigned int)-1);
			cdhmm->generateSample(N, observations, states, seed);

#if 0
			// output states.
			for (size_t n = 0; n < N; ++n)
				std::cout << states[n] << ' ';
			std::cout << std::endl;
#endif

			// write a sample sequence.
			swl::CDHMM::writeSequence(stream, observations);
		}
		else
		{
			const size_t N = 100;

			swl::CDHMM::dmatrix_type observations(N, cdhmm->getObservationDim(), 0.0);
			swl::CDHMM::uivector_type states(N, (unsigned int)-1);
			cdhmm->generateSample(N, observations, states, seed);

#if 0
			// output states.
			for (size_t n = 0; n < N; ++n)
				std::cout << states[n] << ' ';
			std::cout << std::endl;
#endif

			// write a sample sequence.
			swl::CDHMM::writeSequence(std::cout, observations);
		}
	}
}

void observation_sequence_reading_and_writing()
{
	swl::CDHMM::dmatrix_type observations;
	size_t N = 0;  // length of observation sequence, N.

#if __TEST_HMM_MODEL == 1

#if 1
	std::ifstream stream("./data/hmm/von_mises_test1_50.seq");
#elif 0
	std::ifstream stream("./data/hmm/von_mises_test1_100.seq");
#elif 0
	std::ifstream stream("./data/hmm/von_mises_test1_1500.seq");
#else
	std::istream stream = std::cin;
#endif

#elif __TEST_HMM_MODEL == 2

#if 1
	std::ifstream stream("./data/hmm/von_mises_test2_50.seq");
#elif 0
	std::ifstream stream("./data/hmm/von_mises_test2_100.seq");
#elif 0
	std::ifstream stream("./data/hmm/von_mises_test2_1500.seq");
#else
	std::istream stream = std::cin;
#endif

#endif
	if (!stream)
	{
		std::ostringstream stream;
		stream << "file not found at " << __LINE__ << " in " << __FILE__;
		throw std::runtime_error(stream.str().c_str());
		return;
	}

	// read a observation sequence.
	size_t D = 0;
	const bool retval = swl::CDHMM::readSequence(stream, N, D, observations);
	if (!retval)
	{
		std::ostringstream stream;
		stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
		throw std::runtime_error(stream.str().c_str());
		return;
	}

	// write a observation sequence.
	swl::CDHMM::writeSequence(std::cout, observations);
}

void forward_algorithm()
{
	boost::scoped_ptr<swl::CDHMM> cdhmm;

	// read a model.
	{
#if __TEST_HMM_MODEL == 1
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test1.cdhmm");
#elif __TEST_HMM_MODEL == 2
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test2.cdhmm");
#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		cdhmm.reset(new swl::HmmWithVonMisesObservations(K));

		const bool retval = cdhmm->readModel(stream);
		if (!retval)
		{
			std::ostringstream stream;
			stream << "model reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		// normalize pi & A.
		cdhmm->normalizeModelParameters();

		//cdhmm->writeModel(std::cout);
	}

	// read a observation sequence.
	swl::CDHMM::dmatrix_type observations;
	size_t N = 0;  // length of observation sequence, N.
	{
#if __TEST_HMM_MODEL == 1

#if 1
		std::ifstream stream("./data/hmm/von_mises_test1_50.seq");
#elif 0
		std::ifstream stream("./data/hmm/von_mises_test1_100.seq");
#elif 0
		std::ifstream stream("./data/hmm/von_mises_test1_1500.seq");
#else
		std::istream stream = std::cin;
#endif

#elif __TEST_HMM_MODEL == 2

#if 1
		std::ifstream stream("./data/hmm/von_mises_test2_50.seq");
#elif 0
		std::ifstream stream("./data/hmm/von_mises_test2_100.seq");
#elif 0
		std::ifstream stream("./data/hmm/von_mises_test2_1500.seq");
#else
		std::istream stream = std::cin;
#endif

#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		size_t D = 0;
		const bool retval = swl::CDHMM::readSequence(stream, N, D, observations);
		if (!retval || cdhmm->getObservationDim() != D)
		{
			std::ostringstream stream;
			stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}
	}

	const size_t K = cdhmm->getStateDim();

	// forward algorithm without scaling.
	{
		swl::CDHMM::dmatrix_type alpha(N, K, 0.0);
		double likelihood = 0.0;
		cdhmm->runForwardAlgorithm(N, observations, alpha, likelihood);

		//
		std::cout << "------------------------------------" << std::endl;
		std::cout << "forward algorithm without scaling" << std::endl;
		std::cout << "\tlog prob(observations | model) = " << std::scientific << std::log(likelihood) << std::endl;
	}

	// forward algorithm with scaling.
	{
		swl::CDHMM::dvector_type scale(N, 0.0);
		swl::CDHMM::dmatrix_type alpha(N, K, 0.0);
		double logLikelihood = 0.0;
		cdhmm->runForwardAlgorithm(N, observations, scale, alpha, logLikelihood);

		//
		std::cout << "------------------------------------" << std::endl;
		std::cout << "Forward algorithm with scaling" << std::endl;
		std::cout << "\tlog prob(observations | model) = " << std::scientific << logLikelihood << std::endl;
	}
}

void backward_algorithm()
{
	throw std::runtime_error("Not yet implemented");
}

void viterbi_algorithm()
{
	boost::scoped_ptr<swl::CDHMM> cdhmm;

	// read a model
	{
#if __TEST_HMM_MODEL == 1
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test1.cdhmm");
#elif __TEST_HMM_MODEL == 2
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test2.cdhmm");
#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		cdhmm.reset(new swl::HmmWithVonMisesObservations(K));

		const bool retval = cdhmm->readModel(stream);
		if (!retval)
		{
			std::ostringstream stream;
			stream << "model reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		// normalize pi & A.
		cdhmm->normalizeModelParameters();

		//cdhmm->writeModel(std::cout);
	}

	// read a observation sequence.
	swl::CDHMM::dmatrix_type observations;
	size_t N = 0;  // length of observation sequence, N.
	{
#if __TEST_HMM_MODEL == 1

#if 0
		std::ifstream stream("./data/hmm/von_mises_test1_50.seq");
#elif 0
		std::ifstream stream("./data/hmm/von_mises_test1_100.seq");
#elif 1
		std::ifstream stream("./data/hmm/von_mises_test1_1500.seq");
#else
		std::istream stream = std::cin;
#endif

#elif __TEST_HMM_MODEL == 2

#if 0
		std::ifstream stream("./data/hmm/von_mises_test2_50.seq");
#elif 0
		std::ifstream stream("./data/hmm/von_mises_test2_100.seq");
#elif 1
		std::ifstream stream("./data/hmm/von_mises_test2_1500.seq");
#else
		std::istream stream = std::cin;
#endif

#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		size_t D = 0;
		const bool retval = swl::CDHMM::readSequence(stream, N, D, observations);
		if (!retval || cdhmm->getObservationDim() != D)
		{
			std::ostringstream stream;
			stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}
	}

	const size_t K = cdhmm->getStateDim();

	// Viterbi algorithm using direct likelihood.
	{
		swl::CDHMM::dmatrix_type delta(N, K, 0.0);
		swl::CDHMM::uimatrix_type psi(N, K, (unsigned int)-1);
		swl::CDHMM::uivector_type states(N, (unsigned int)-1);
		double likelihood = 0.0;
		cdhmm->runViterbiAlgorithm(N, observations, delta, psi, states, likelihood, false);

		//
		std::cout << "------------------------------------" << std::endl;
		std::cout << "Viterbi algorithm using direct likelihood" << std::endl;
		std::cout << "\tViterbi MLE log likelihood = " << std::scientific << std::log(likelihood) << std::endl;
		std::cout << "\toptimal state sequence:" << std::endl;
		for (size_t n = 0; n < N; ++n)
			std::cout << states[n] << ' ';
		std::cout << std::endl;
	}

	// Viterbi algorithm using log likelihood.
	{
		swl::CDHMM::dmatrix_type delta(N, K, 0.0);
		swl::CDHMM::uimatrix_type psi(N, K, (unsigned int)-1);
		swl::CDHMM::uivector_type states(N, (unsigned int)-1);
		double logLikelihood = 0.0;
		cdhmm->runViterbiAlgorithm(N, observations, delta, psi, states, logLikelihood, true);

		//
		std::cout << "------------------------------------" << std::endl;
		std::cout << "Viterbi algorithm using log likelihood" << std::endl;
		std::cout << "\tViterbi MLE log likelihood = " << std::scientific << logLikelihood << std::endl;
		std::cout << "\toptimal state sequence:" << std::endl;
		for (size_t n = 0; n < N; ++n)
			std::cout << states[n] << ' ';
		std::cout << std::endl;
	}
}

void ml_learning_by_em()
{
	boost::scoped_ptr<swl::CDHMM> cdhmm;

/*
	you can initialize the hmm model three ways:
		1) with a model, which also sets the number of states N and number of symbols M.
		2) with a random model by just specifyin N and M.
		3) with a specific random model by specifying N, M and seed.
*/

	// initialize a model.
	const int initialization_mode = 1;
	if (1 == initialization_mode)
	{
#if __TEST_HMM_MODEL == 1
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test1.cdhmm");
#elif __TEST_HMM_MODEL == 2
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test2.cdhmm");
#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		cdhmm.reset(new swl::HmmWithVonMisesObservations(K));

		const bool retval = cdhmm->readModel(stream);
		if (!retval)
		{
			std::ostringstream stream;
			stream << "model reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		// normalize pi & A.
		cdhmm->normalizeModelParameters();

		//cdhmm->writeModel(std::cout);
	}
	else if (2 == initialization_mode)
	{
#if defined(__USE_SPECIFIED_VALUE_FOR_RANDOM_SEED)
		const unsigned int seed = 34586u;
#else
		const unsigned int seed = (unsigned int)std::time(NULL);
#endif
		std::srand(seed);
		std::cout << "random seed: " << seed << std::endl;

		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		cdhmm.reset(new swl::HmmWithVonMisesObservations(K));

		// the total number of parameters of observation density = K * D * 2.
		std::vector<double> lowerBounds, upperBounds;
		const size_t numParameters = K * 1 * 2;
		lowerBounds.reserve(numParameters);
		upperBounds.reserve(numParameters);
		const double small = 1.0e-10;
		// mean directions: 0 <= mu < 2 * pi.
		for (size_t i = 0; i < K; ++i)
		{
			lowerBounds.push_back(0.0);
			upperBounds.push_back(2.0 * boost::math::constants::pi<double>() - small);
		}
		// concentration parameters: kappa >= 0.
		for (size_t i = K; i < numParameters; ++i)
		{
			lowerBounds.push_back(small);
			upperBounds.push_back(10000.0);
		}
		cdhmm->initializeModel(lowerBounds, upperBounds);
	}
	else
		throw std::runtime_error("incorrect initialization mode");

	// for a single observation sequence.
	{
		// read a observation sequence.
		swl::CDHMM::dmatrix_type observations;
		size_t N = 0;  // length of observation sequence, N.
		{
#if __TEST_HMM_MODEL == 1

#if 0
			std::ifstream stream("./data/hmm/von_mises_test1_50.seq");
#elif 0
			std::ifstream stream("./data/hmm/von_mises_test1_100.seq");
#elif 1
			std::ifstream stream("./data/hmm/von_mises_test1_1500.seq");
#else
			std::istream stream = std::cin;
#endif

#elif __TEST_HMM_MODEL == 2

#if 0
			std::ifstream stream("./data/hmm/von_mises_test2_50.seq");
#elif 0
			std::ifstream stream("./data/hmm/von_mises_test2_100.seq");
#elif 1
			std::ifstream stream("./data/hmm/von_mises_test2_1500.seq");
#else
			std::istream stream = std::cin;
#endif

#endif
			if (!stream)
			{
				std::ostringstream stream;
				stream << "file not found at " << __LINE__ << " in " << __FILE__;
				throw std::runtime_error(stream.str().c_str());
				return;
			}

			size_t D = 0;
			const bool retval = swl::CDHMM::readSequence(stream, N, D, observations);
			if (!retval || cdhmm->getObservationDim() != D)
			{
				std::ostringstream stream;
				stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
				throw std::runtime_error(stream.str().c_str());
				return;
			}
		}

		// Baum-Welch algorithm.
		{
			const double terminationTolerance = 0.001;
			const size_t maxIteration = 1000;
			size_t numIteration = (size_t)-1;
			double initLogLikelihood = 0.0, finalLogLikelihood = 0.0;
			cdhmm->trainByML(N, observations, terminationTolerance, maxIteration, numIteration, initLogLikelihood, finalLogLikelihood);

			// normalize pi & A.
			//cdhmm->normalizeModelParameters();

			//
			std::cout << "------------------------------------" << std::endl;
			std::cout << "Baum-Welch algorithm for a single observation sequence" << std::endl;
			std::cout << "\tnumber of iterations = " << numIteration << std::endl;
			std::cout << "\tlog prob(observations | initial model) = " << std::scientific << initLogLikelihood << std::endl;
			std::cout << "\tlog prob(observations | estimated model) = " << std::scientific << finalLogLikelihood << std::endl;
			std::cout << "\testimated model:" << std::endl;
			cdhmm->writeModel(std::cout);
		}
	}

	// for multiple independent observation sequences.
	{
		// read a observation sequence
		std::vector<swl::CDHMM::dmatrix_type> observationSequences;
		std::vector<size_t> Ns;  // lengths of observation sequences.
		{
#if __TEST_HMM_MODEL == 1
			const size_t R = 3;  // number of observations sequences.
			const std::string observationSequenceFiles[] = {
				"./data/hmm/von_mises_test1_50.seq",
				"./data/hmm/von_mises_test1_100.seq",
				"./data/hmm/von_mises_test1_1500.seq"
			};
#elif __TEST_HMM_MODEL == 2
			const size_t R = 3;  // number of observations sequences.
			const std::string observationSequenceFiles[] = {
				"./data/hmm/von_mises_test2_50.seq",
				"./data/hmm/von_mises_test2_100.seq",
				"./data/hmm/von_mises_test2_1500.seq"
			};
#endif
			observationSequences.resize(R);
			Ns.resize(R);
			for (size_t r = 0; r < R; ++r)
			{
				std::ifstream stream(observationSequenceFiles[r].c_str());
				if (!stream)
				{
					std::ostringstream stream;
					stream << "file not found at " << __LINE__ << " in " << __FILE__;
					throw std::runtime_error(stream.str().c_str());
					return;
				}

				size_t D = 0;
				const bool retval = swl::CDHMM::readSequence(stream, Ns[r], D, observationSequences[r]);
				if (!retval || cdhmm->getObservationDim() != D)
				{
					std::ostringstream stream;
					stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
					throw std::runtime_error(stream.str().c_str());
					return;
				}
			}
		}

		const size_t R = observationSequences.size();  // number of observations sequences.

		// Baum-Welch algorithm.
		{
			const double terminationTolerance = 0.001;
			const size_t maxIteration = 1000;
			size_t numIteration = (size_t)-1;
			std::vector<double> initLogProbabilities(R, 0.0), finalLogProbabilities(R, 0.0);
			cdhmm->trainByML(Ns, observationSequences, terminationTolerance, maxIteration, numIteration, initLogProbabilities, finalLogProbabilities);

			// normalize pi & A.
			//cdhmm->normalizeModelParameters();

			//
			std::cout << "------------------------------------" << std::endl;
			std::cout << "Baum-Welch algorithm for multiple independent observation sequences" << std::endl;
			std::cout << "\tnumber of iterations = " << numIteration << std::endl;
			std::cout << "\tlog prob(observation sequences | initial model):" << std::endl;
			std::cout << "\t\t";
			for (size_t r = 0; r < R; ++r)
				std::cout << std::scientific << initLogProbabilities[r] << ' ';
			std::cout << std::endl;
			std::cout << "\tlog prob(observation sequences | estimated model):" << std::endl;
			std::cout << "\t\t";
			for (size_t r = 0; r < R; ++r)
				std::cout << std::scientific << finalLogProbabilities[r] << ' ';
			std::cout << std::endl;
			std::cout << "\testimated model:" << std::endl;
			cdhmm->writeModel(std::cout);
		}
	}
}

void map_learning_by_em_using_conjugate_prior()
{
	boost::scoped_ptr<swl::CDHMM> cdhmm;

/*
	you can initialize the hmm model three ways:
		1) with a model, which also sets the number of states N and number of symbols M.
		2) with a random model by just specifyin N and M.
		3) with a specific random model by specifying N, M and seed.
*/

	// initialize a model.
	const int initialization_mode = 1;
	if (1 == initialization_mode)
	{
#if __TEST_HMM_MODEL == 1
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test1.cdhmm");
#elif __TEST_HMM_MODEL == 2
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test2.cdhmm");
#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		std::srand((unsigned int)std::time(NULL));

		// hyperparameters for the conjugate prior.
		// FIXME [check] >> hyperparameters for initial state distribution & state transition probability matrix.
		swl::CDHMM::dvector_type *pi_conj = new swl::CDHMM::dvector_type(K, 1.0);
		swl::CDHMM::dmatrix_type *A_conj = new swl::CDHMM::dmatrix_type(K, K, 1.0);
		// FIXME [check] >> hyperparameters for von Mises distribution.
		swl::CDHMM::dvector_type *ms_conj = new swl::CDHMM::dvector_type(K, 0.0);
		swl::CDHMM::dvector_type *Rs_conj = new swl::CDHMM::dvector_type(K, 1.0);  // R >= 0.
		swl::CDHMM::dvector_type *cs_conj = new swl::CDHMM::dvector_type(K, 1.0);  // non-negative integer.
		for (size_t k = 0; k < K; ++k)
		{
			(*ms_conj)(k) = (std::rand() / RAND_MAX) * 10.0 - 5.0;
			//(*Rs_conj)(k) = (std::rand() / RAND_MAX + 1.0) * 10.0;
			//(*cs_conj)(k) = ???;
		}

		cdhmm.reset(new swl::HmmWithVonMisesObservations(K, pi_conj, A_conj, ms_conj, Rs_conj, cs_conj));

		const bool retval = cdhmm->readModel(stream);
		if (!retval)
		{
			std::ostringstream stream;
			stream << "model reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		// normalize pi & A.
		cdhmm->normalizeModelParameters();

		//cdhmm->writeModel(std::cout);
	}
	else if (2 == initialization_mode)
	{
#if defined(__USE_SPECIFIED_VALUE_FOR_RANDOM_SEED)
		const unsigned int seed = 34586u;
#else
		const unsigned int seed = (unsigned int)std::time(NULL);
#endif
		std::srand(seed);
		std::cout << "random seed: " << seed << std::endl;

		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		// hyperparameters for the conjugate prior.
		// FIXME [check] >> hyperparameters for initial state distribution & state transition probability matrix.
		swl::CDHMM::dvector_type *pi_conj = new swl::CDHMM::dvector_type(K, 1.0);
		swl::CDHMM::dmatrix_type *A_conj = new swl::CDHMM::dmatrix_type(K, K, 1.0);
		// FIXME [check] >> hyperparameters for von Mises distribution.
		swl::CDHMM::dvector_type *ms_conj = new swl::CDHMM::dvector_type(K, 0.0);
		swl::CDHMM::dvector_type *Rs_conj = new swl::CDHMM::dvector_type(K, 1.0);  // R >= 0.
		swl::CDHMM::dvector_type *cs_conj = new swl::CDHMM::dvector_type(K, 1.0);  // non-negative integer.
		for (size_t k = 0; k < K; ++k)
		{
			(*ms_conj)(k) = (std::rand() / RAND_MAX) * 100.0 - 50.0;
			//(*Rs_conj)(k) = (std::rand() / RAND_MAX + 1.0) * 100.0;
			//(*cs_conj)(k) = ???;
		}

		cdhmm.reset(new swl::HmmWithVonMisesObservations(K, pi_conj, A_conj, ms_conj, Rs_conj, cs_conj));

		// the total number of parameters of observation density = K * D * 2.
		std::vector<double> lowerBounds, upperBounds;
		const size_t numParameters = K * 1 * 2;
		lowerBounds.reserve(numParameters);
		upperBounds.reserve(numParameters);
		const double small = 1.0e-10;
		// mean directions: 0 <= mu < 2 * pi.
		for (size_t i = 0; i < K; ++i)
		{
			lowerBounds.push_back(0.0);
			upperBounds.push_back(2.0 * boost::math::constants::pi<double>() - small);
		}
		// concentration parameters: kappa >= 0.
		for (size_t i = K; i < numParameters; ++i)
		{
			lowerBounds.push_back(small);
			upperBounds.push_back(10000.0);
		}
		cdhmm->initializeModel(lowerBounds, upperBounds);
	}
	else
		throw std::runtime_error("incorrect initialization mode");

	// for a single observation sequence.
	{
		// read a observation sequence.
		swl::CDHMM::dmatrix_type observations;
		size_t N = 0;  // length of observation sequence, N.
		{
#if __TEST_HMM_MODEL == 1

#if 0
			std::ifstream stream("./data/hmm/von_mises_test1_50.seq");
#elif 0
			std::ifstream stream("./data/hmm/von_mises_test1_100.seq");
#elif 1
			std::ifstream stream("./data/hmm/von_mises_test1_1500.seq");
#else
			std::istream stream = std::cin;
#endif

#elif __TEST_HMM_MODEL == 2

#if 0
			std::ifstream stream("./data/hmm/von_mises_test2_50.seq");
#elif 0
			std::ifstream stream("./data/hmm/von_mises_test2_100.seq");
#elif 1
			std::ifstream stream("./data/hmm/von_mises_test2_1500.seq");
#else
			std::istream stream = std::cin;
#endif

#endif
			if (!stream)
			{
				std::ostringstream stream;
				stream << "file not found at " << __LINE__ << " in " << __FILE__;
				throw std::runtime_error(stream.str().c_str());
				return;
			}

			size_t D = 0;
			const bool retval = swl::CDHMM::readSequence(stream, N, D, observations);
			if (!retval || cdhmm->getObservationDim() != D)
			{
				std::ostringstream stream;
				stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
				throw std::runtime_error(stream.str().c_str());
				return;
			}
		}

		// Baum-Welch algorithm.
		{
			const double terminationTolerance = 0.001;
			const size_t maxIteration = 1000;
			size_t numIteration = (size_t)-1;
			double initLogLikelihood = 0.0, finalLogLikelihood = 0.0;
			cdhmm->trainByMAPUsingConjugatePrior(N, observations, terminationTolerance, maxIteration, numIteration, initLogLikelihood, finalLogLikelihood);

			// normalize pi & A.
			//cdhmm->normalizeModelParameters();

			//
			std::cout << "------------------------------------" << std::endl;
			std::cout << "Baum-Welch algorithm for a single observation sequence" << std::endl;
			std::cout << "\tnumber of iterations = " << numIteration << std::endl;
			std::cout << "\tlog prob(observations | initial model) = " << std::scientific << initLogLikelihood << std::endl;
			std::cout << "\tlog prob(observations | estimated model) = " << std::scientific << finalLogLikelihood << std::endl;
			std::cout << "\testimated model:" << std::endl;
			cdhmm->writeModel(std::cout);
		}
	}

	// for multiple independent observation sequences.
	{
		// read a observation sequence.
		std::vector<swl::CDHMM::dmatrix_type> observationSequences;
		std::vector<size_t> Ns;  // lengths of observation sequences.
		{
#if __TEST_HMM_MODEL == 1
			const size_t R = 3;  // number of observations sequences.
			const std::string observationSequenceFiles[] = {
				"./data/hmm/von_mises_test1_50.seq",
				"./data/hmm/von_mises_test1_100.seq",
				"./data/hmm/von_mises_test1_1500.seq"
			};
#elif __TEST_HMM_MODEL == 2
			const size_t R = 3;  // number of observations sequences.
			const std::string observationSequenceFiles[] = {
				"./data/hmm/von_mises_test2_50.seq",
				"./data/hmm/von_mises_test2_100.seq",
				"./data/hmm/von_mises_test2_1500.seq"
			};
#endif
			observationSequences.resize(R);
			Ns.resize(R);
			for (size_t r = 0; r < R; ++r)
			{
				std::ifstream stream(observationSequenceFiles[r].c_str());
				if (!stream)
				{
					std::ostringstream stream;
					stream << "file not found at " << __LINE__ << " in " << __FILE__;
					throw std::runtime_error(stream.str().c_str());
					return;
				}

				size_t D = 0;
				const bool retval = swl::CDHMM::readSequence(stream, Ns[r], D, observationSequences[r]);
				if (!retval || cdhmm->getObservationDim() != D)
				{
					std::ostringstream stream;
					stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
					throw std::runtime_error(stream.str().c_str());
					return;
				}
			}
		}

		const size_t R = observationSequences.size();  // number of observations sequences.

		// Baum-Welch algorithm.
		{
			const double terminationTolerance = 0.001;
			const size_t maxIteration = 1000;
			size_t numIteration = (size_t)-1;
			std::vector<double> initLogProbabilities(R, 0.0), finalLogProbabilities(R, 0.0);
			cdhmm->trainByMAPUsingConjugatePrior(Ns, observationSequences, terminationTolerance, maxIteration, numIteration, initLogProbabilities, finalLogProbabilities);

			// normalize pi & A.
			//cdhmm->normalizeModelParameters();

			//
			std::cout << "------------------------------------" << std::endl;
			std::cout << "Baum-Welch algorithm for multiple independent observation sequences" << std::endl;
			std::cout << "\tnumber of iterations = " << numIteration << std::endl;
			std::cout << "\tlog prob(observation sequences | initial model):" << std::endl;
			std::cout << "\t\t";
			for (size_t r = 0; r < R; ++r)
				std::cout << std::scientific << initLogProbabilities[r] << ' ';
			std::cout << std::endl;
			std::cout << "\tlog prob(observation sequences | estimated model):" << std::endl;
			std::cout << "\t\t";
			for (size_t r = 0; r < R; ++r)
				std::cout << std::scientific << finalLogProbabilities[r] << ' ';
			std::cout << std::endl;
			std::cout << "\testimated model:" << std::endl;
			cdhmm->writeModel(std::cout);
		}
	}
}

void map_learning_by_em_using_entropic_prior()
{
	boost::scoped_ptr<swl::CDHMM> cdhmm;

/*
	you can initialize the hmm model three ways:
		1) with a model, which also sets the number of states N and number of symbols M.
		2) with a random model by just specifyin N and M.
		3) with a specific random model by specifying N, M and seed.
*/

	// initialize a model.
	const int initialization_mode = 1;
	if (1 == initialization_mode)
	{
#if __TEST_HMM_MODEL == 1
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test1.cdhmm");
#elif __TEST_HMM_MODEL == 2
		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		//
		std::ifstream stream("./data/hmm/von_mises_test2.cdhmm");
#endif
		if (!stream)
		{
			std::ostringstream stream;
			stream << "file not found at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		// hyperparameters for the entropic prior.
		//	don't need.

		//cdhmm.reset(new swl::HmmWithVonMisesObservations(K, ms_conj, Rs_conj, cs_conj));
		cdhmm.reset(new swl::HmmWithVonMisesObservations(K));

		const bool retval = cdhmm->readModel(stream);
		if (!retval)
		{
			std::ostringstream stream;
			stream << "model reading error at " << __LINE__ << " in " << __FILE__;
			throw std::runtime_error(stream.str().c_str());
			return;
		}

		// normalize pi & A.
		cdhmm->normalizeModelParameters();

		//cdhmm->writeModel(std::cout);
	}
	else if (2 == initialization_mode)
	{
#if defined(__USE_SPECIFIED_VALUE_FOR_RANDOM_SEED)
		const unsigned int seed = 34586u;
#else
		const unsigned int seed = (unsigned int)std::time(NULL);
#endif
		std::srand(seed);
		std::cout << "random seed: " << seed << std::endl;

		const size_t K = 3;  // the dimension of hidden states.
		//const size_t D = 1;  // the dimension of observation symbols.

		// hyperparameters for the entropic prior.
		//	don't need.

		//cdhmm.reset(new swl::HmmWithVonMisesObservations(K, ms_conj, Rs_conj, cs_conj));
		cdhmm.reset(new swl::HmmWithVonMisesObservations(K));

		// the total number of parameters of observation density = K * D * 2.
		std::vector<double> lowerBounds, upperBounds;
		const size_t numParameters = K * 1 * 2;
		lowerBounds.reserve(numParameters);
		upperBounds.reserve(numParameters);
		const double small = 1.0e-10;
		// mean directions: 0 <= mu < 2 * pi.
		for (size_t i = 0; i < K; ++i)
		{
			lowerBounds.push_back(0.0);
			upperBounds.push_back(2.0 * boost::math::constants::pi<double>() - small);
		}
		// concentration parameters: kappa >= 0.
		for (size_t i = K; i < numParameters; ++i)
		{
			lowerBounds.push_back(small);
			upperBounds.push_back(10000.0);
		}
		cdhmm->initializeModel(lowerBounds, upperBounds);
	}
	else
		throw std::runtime_error("incorrect initialization mode");

	// for a single observation sequence.
	{
		// read a observation sequence.
		swl::CDHMM::dmatrix_type observations;
		size_t N = 0;  // length of observation sequence, N.
		{
#if __TEST_HMM_MODEL == 1

#if 0
			std::ifstream stream("./data/hmm/von_mises_test1_50.seq");
#elif 0
			std::ifstream stream("./data/hmm/von_mises_test1_100.seq");
#elif 1
			std::ifstream stream("./data/hmm/von_mises_test1_1500.seq");
#else
			std::istream stream = std::cin;
#endif

#elif __TEST_HMM_MODEL == 2

#if 0
			std::ifstream stream("./data/hmm/von_mises_test2_50.seq");
#elif 0
			std::ifstream stream("./data/hmm/von_mises_test2_100.seq");
#elif 1
			std::ifstream stream("./data/hmm/von_mises_test2_1500.seq");
#else
			std::istream stream = std::cin;
#endif

#endif
			if (!stream)
			{
				std::ostringstream stream;
				stream << "file not found at " << __LINE__ << " in " << __FILE__;
				throw std::runtime_error(stream.str().c_str());
				return;
			}

			size_t D = 0;
			const bool retval = swl::CDHMM::readSequence(stream, N, D, observations);
			if (!retval || cdhmm->getObservationDim() != D)
			{
				std::ostringstream stream;
				stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
				throw std::runtime_error(stream.str().c_str());
				return;
			}
		}

		// Baum-Welch algorithm.
		{
			// z = 1 (default) is min. entropy.
			// z = 0 is max. likelihood.
			// z = -1 is max. entropy.
			// z = -inf corresponds to very high temperature (good for initialization).
			const double z = 1.0;
			const bool doesTrimParameter = true;

			const double terminationTolerance = 0.001;
			const size_t maxIteration = 1000;
			size_t numIteration = (size_t)-1;
			double initLogLikelihood = 0.0, finalLogLikelihood = 0.0;
			cdhmm->trainByMAPUsingEntropicPrior(N, observations, z, doesTrimParameter, terminationTolerance, maxIteration, numIteration, initLogLikelihood, finalLogLikelihood);

			// normalize pi & A.
			//cdhmm->normalizeModelParameters();

			//
			std::cout << "------------------------------------" << std::endl;
			std::cout << "Baum-Welch algorithm for a single observation sequence" << std::endl;
			std::cout << "\tnumber of iterations = " << numIteration << std::endl;
			std::cout << "\tlog prob(observations | initial model) = " << std::scientific << initLogLikelihood << std::endl;
			std::cout << "\tlog prob(observations | estimated model) = " << std::scientific << finalLogLikelihood << std::endl;
			std::cout << "\testimated model:" << std::endl;
			cdhmm->writeModel(std::cout);
		}
	}

	// for multiple independent observation sequences.
	{
		// read a observation sequence.
		std::vector<swl::CDHMM::dmatrix_type> observationSequences;
		std::vector<size_t> Ns;  // lengths of observation sequences.
		{
#if __TEST_HMM_MODEL == 1
			const size_t R = 3;  // number of observations sequences.
			const std::string observationSequenceFiles[] = {
				"./data/hmm/von_mises_test1_50.seq",
				"./data/hmm/von_mises_test1_100.seq",
				"./data/hmm/von_mises_test1_1500.seq"
			};
#elif __TEST_HMM_MODEL == 2
			const size_t R = 3;  // number of observations sequences.
			const std::string observationSequenceFiles[] = {
				"./data/hmm/von_mises_test2_50.seq",
				"./data/hmm/von_mises_test2_100.seq",
				"./data/hmm/von_mises_test2_1500.seq"
			};
#endif
			observationSequences.resize(R);
			Ns.resize(R);
			for (size_t r = 0; r < R; ++r)
			{
				std::ifstream stream(observationSequenceFiles[r].c_str());
				if (!stream)
				{
					std::ostringstream stream;
					stream << "file not found at " << __LINE__ << " in " << __FILE__;
					throw std::runtime_error(stream.str().c_str());
					return;
				}

				size_t D = 0;
				const bool retval = swl::CDHMM::readSequence(stream, Ns[r], D, observationSequences[r]);
				if (!retval || cdhmm->getObservationDim() != D)
				{
					std::ostringstream stream;
					stream << "sample sequence reading error at " << __LINE__ << " in " << __FILE__;
					throw std::runtime_error(stream.str().c_str());
					return;
				}
			}
		}

		const size_t R = observationSequences.size();  // number of observations sequences.

		// Baum-Welch algorithm.
		{
			// z = 1 (default) is min. entropy.
			// z = 0 is max. likelihood.
			// z = -1 is max. entropy.
			// z = -inf corresponds to very high temperature (good for initialization).
			const double z = 1.0;
			const bool doesTrimParameter = true;

			const double terminationTolerance = 0.001;
			const size_t maxIteration = 1000;
			size_t numIteration = (size_t)-1;
			std::vector<double> initLogProbabilities(R, 0.0), finalLogProbabilities(R, 0.0);
			cdhmm->trainByMAPUsingEntropicPrior(Ns, observationSequences, z, doesTrimParameter, terminationTolerance, maxIteration, numIteration, initLogProbabilities, finalLogProbabilities);

			// normalize pi & A.
			//cdhmm->normalizeModelParameters();

			//
			std::cout << "------------------------------------" << std::endl;
			std::cout << "Baum-Welch algorithm for multiple independent observation sequences" << std::endl;
			std::cout << "\tnumber of iterations = " << numIteration << std::endl;
			std::cout << "\tlog prob(observation sequences | initial model):" << std::endl;
			std::cout << "\t\t";
			for (size_t r = 0; r < R; ++r)
				std::cout << std::scientific << initLogProbabilities[r] << ' ';
			std::cout << std::endl;
			std::cout << "\tlog prob(observation sequences | estimated model):" << std::endl;
			std::cout << "\t\t";
			for (size_t r = 0; r < R; ++r)
				std::cout << std::scientific << finalLogProbabilities[r] << ' ';
			std::cout << std::endl;
			std::cout << "\testimated model:" << std::endl;
			cdhmm->writeModel(std::cout);
		}
	}
}

}  // namespace local
}  // unnamed namespace

void hmm_with_von_mises_observation_densities()
{
	std::cout << "CDHMM w/ von Mises observation densities ----------------------------" << std::endl;

	//local::model_reading_and_writing();
	//const bool outputToFile = false;
	//local::observation_sequence_generation(outputToFile);
	//local::observation_sequence_reading_and_writing();

	//local::forward_algorithm();
	//local::backward_algorithm();  // Not yet implemented.
	//local::viterbi_algorithm();

	std::cout << "\nTrain by ML ---------------------------------------------------------" << std::endl;
	local::ml_learning_by_em();
	std::cout << "\nTrain by MAP using conjugate prior ----------------------------------" << std::endl;
	//local::map_learning_by_em_using_conjugate_prior();
	std::cout << "\nTrain by MAP using entropic prior -----------------------------------" << std::endl;
	local::map_learning_by_em_using_entropic_prior();
}
