#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Regression

#include <boost/test/included/unit_test.hpp>
#include "W111FitFluxHandler.h"

using namespace std;
using namespace xolotlSolver;

/**
 * The test suite is responsible for testing the WFitFluxHandler.
 */
BOOST_AUTO_TEST_SUITE (W111FitFluxHandlerTester_testSuite)

BOOST_AUTO_TEST_CASE(checkgetIncidentFlux) {

	// Specify the number of grid points that will be used
	int nGridpts = 5;
	// Specify the step size between grid points
	double step = 1.25;

    auto testFitFlux = std::make_shared<xolotlSolver::W111FitFluxHandler>();
    // Initialize the flux handler
    testFitFlux->initializeFluxHandler(nGridpts, step);

	// Create a composition vector
	std::vector<int> compVec = {1, 0, 0};
	// Create a time
	double currTime = 1.0;

	// Create a vector representing the position of the cluster
	std::vector<double> x = {1.25, 0.0, 0.0};

	auto testFlux = testFitFlux->getIncidentFlux(compVec, x, 1);

	BOOST_TEST_MESSAGE( "\nW111FitFluxHandlerTester Message: \n"
						<< "At time = " << currTime
						<< " and position (" << x[0] << "," << x[1] << "," << x[2] << "), "
						<< "incidentFlux = " << testFlux << "\n");
	BOOST_REQUIRE_CLOSE(testFlux, 0.391408, 0.01);
}

BOOST_AUTO_TEST_SUITE_END()
