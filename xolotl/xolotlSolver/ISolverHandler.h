#ifndef ISOLVERHANDLER_H
#define ISOLVERHANDLER_H

// Includes
#include <petscsys.h>
#include <petscts.h>
#include <petscdmda.h>
#include <memory>
#include <ITemperatureHandler.h>
#include <IDiffusionHandler.h>
#include <IAdvectionHandler.h>
#include <IMaterialFactory.h>

namespace xolotlSolver {

/**
 * Realizations of this interface are responsible for the actual implementation of
 * each piece of the solver. It is created to handle the multiple dimensions more easily.
 */
class ISolverHandler {

public:

	/**
	 * The destructor.
	 */
	virtual ~ISolverHandler(){}

	/**
	 * Initialize all the physics handlers that are needed to solve the ADR equations.
	 *
	 * @param material The material factory
	 * @param tempHandler The temperature handler
	 * @param options The Xolotl options
	 */
	virtual void initializeHandlers(std::shared_ptr<xolotlFactory::IMaterialFactory> material,
			std::shared_ptr<xolotlCore::ITemperatureHandler> tempHandler,
			xolotlCore::Options &options) = 0;

	/**
	 * Initialize the network and network file name.
	 *
	 * @param fileName The name of the file from which the network was loaded
	 * @param net The network
	 */
	virtual void initializeNetwork(std::string fileName,
			xolotlCore::PSIClusterReactionNetwork *net) = 0;

	/**
	 * Create everything needed before starting to solve.
	 *
	 * @param da The PETSC distributed array
	 */
	virtual void createSolverContext(DM &da) const = 0;

	/**
	 * Get the diagonal fill for the Jacobian, corresponding to the reactions.
	 *
	 * @param diagFill The pointer to the vector where the connectivity information is kept
	 * @param diagFillSize The size of this vector
	 */
	virtual void getDiagonalFill(PetscInt *diagFill, int diagFillSize) const = 0;

	/**
	 * Initialize the concentration solution vector.
	 *
	 * @param da The PETSC distributed array
	 * @param C The PETSC solution vector
	 */
	virtual void initializeConcentration(DM &da, Vec &C) const = 0;

	/**
	 * Compute the new concentrations for the RHS function given an initial
	 * vector of concentrations.
	 *
	 * @param ts The PETSc time stepper
	 * @param localC The PETSC local solution vector
	 * @param F The updated PETSC solution vector
	 * @param ftime The real time
	 * @param temperatureChanged True is the temperature has changed at this time
	 */
	virtual void updateConcentration(TS &ts, Vec &localC, Vec &F, PetscReal ftime,
			bool &temperatureChanged) const = 0;

	/**
	 * Compute the off-diagonal part of the Jacobian which is related to cluster's motion.
	 *
	 * @param ts The PETSc time stepper
	 * @param localC The PETSC local solution vector
	 * @param J The Jacobian
	 */
	virtual void computeOffDiagonalJacobian(TS &ts, Vec &localC, Mat &J) const = 0;

	/**
	 * Compute the diagonal part of the Jacobian which is related to cluster reactions.
	 *
	 * @param ts The PETSc time stepper
	 * @param localC The PETSC local solution vector
	 * @param J The Jacobian
	 */
	virtual void computeDiagonalJacobian(TS &ts, Vec &localC, Mat &J) const = 0;

	/**
	 * Get the step size.
	 *
	 * @return The step size
	 */
	virtual double getStepSize() const = 0;

	/**
	 * Get the flux handler.
	 *
	 * @return The flux handler
	 */
	virtual xolotlCore::IFluxHandler *getFluxHandler() const = 0;

	/**
	 * Get the network.
	 *
	 * @return The network
	 */
	virtual xolotlCore::PSIClusterReactionNetwork *getNetwork() const = 0;

}; //end class ISolverHandler

} /* namespace xolotlSolver */
#endif
