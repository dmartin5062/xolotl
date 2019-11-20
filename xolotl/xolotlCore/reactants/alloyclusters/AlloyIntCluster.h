#ifndef ALLOYINTCLUSTER_H
#define ALLOYINTCLUSTER_H

// Includes
#include "AlloyCluster.h"
#include <xolotlPerf.h>

namespace xolotlCore {

/**
 * This class represents a cluster composed entirely of interstitial.
 */
class AlloyIntCluster: public AlloyCluster {

public:

	/**
	 * Default constructor, deleted because we require info to construct.
	 */
	AlloyIntCluster() = delete;

	/**
	 * The constructor. All AlloyIntClusters must be initialized with a size.
	 *
	 * @param n The size of the cluster
	 * @param registry The performance handler registry
	 */
	AlloyIntCluster(int n, IReactionNetwork& _network,
			std::shared_ptr<xolotlPerf::IHandlerRegistry> registry) :
			AlloyCluster(_network, registry) {
		// Set the size
		size = n;
		// Update the composition map
		composition[toCompIdx(Species::I)] = size;

		// Set the reactant name appropriately
		std::stringstream nameStream;
		nameStream << "I_" << size;
		name = nameStream.str();
		// Set the typename appropriately
		type = ReactantType::I;

		// Define the diffusion pre-factor
		{
			double jumpDistance = network.getLatticeParameter() / sqrt(2.0);
			double phononFrequency = 9.6e12;
			double jumpsPerPhonon = 1.0;
			double prefactorExponent = -1.0;
			diffusionFactor = phononFrequency * jumpsPerPhonon * jumpDistance
					* jumpDistance * pow(double(size), prefactorExponent)
					/ (6.0);
		}

		// Define the formation energy
		formationEnergy = _network.getFormationEnergy(type, size);

		// Define the migration energy
		migrationEnergy = 0.5;

		// Define the reaction radius (using spherical)
		reactionRadius = _network.getReactionRadius(type, size);

		return;
	}

	/**
	 * Destructor
	 */
	~AlloyIntCluster() {
	}

	// Int clusters are considered spheres so set isSphere to true
	bool isSphere() const override {
		return true;
	}

	double getEmissionFlux(int i) const override {
		// Initial declarations
		double flux = AlloyCluster::getEmissionFlux(i);

		// Compute the loss to dislocation sinks
		// Including 20% interstitial bias
		flux += 1.2 * xolotlCore::alloysinkStrength * diffusionCoefficient[i]
				* concentration;

		return flux;
	}

	/**
	 * This operation computes the partial derivatives due to emission
	 * reactions.
	 *
	 * @param partials The vector into which the partial derivatives should be
	 * inserted. This vector should have a length equal to the size of the
	 * network.
	 * @param i The location on the grid in the depth direction
	 */
	void getEmissionPartialDerivatives(std::vector<double> & partials,
			int i) const override {
		// Initial declarations
		AlloyCluster::getEmissionPartialDerivatives(partials, i);

		// Compute the loss to dislocation sinks
		// k^2 * D * C, including 20% interstitial bias
		partials[id - 1] -= 1.2 * xolotlCore::alloysinkStrength
				* diffusionCoefficient[i];

		return;
	}

	/**
	 * Add grid points to the vector of diffusion coefficients or remove
	 * them if the value is negative.
	 *
	 * @param i The number of grid point to add or remove
	 */
	void addGridPoints(int i) override {
		if (diffusionFactor > 0.0) {
			Reactant::addGridPoints(i);
		}

		// Don't do anything
		return;
	}

	/**
	 * This operation sets the temperature at which the reactant currently
	 * exists. Temperature-dependent quantities are recomputed when this
	 * operation is called, so the temperature should always be set first.
	 *
	 * @param temp The new cluster temperature
	 * @param i The location on the grid
	 */
	void setTemperature(double temp, int i) override {
		if (diffusionFactor > 0.0) {
			Reactant::setTemperature(temp, i);
		}

		// Don't do anything
		return;
	}

	/**
	 * This operation returns the diffusion coefficient for this reactant and is
	 * calculated from the diffusion factor.
	 *
	 * @param i The position on the grid
	 * @return The diffusion coefficient
	 */
	double getDiffusionCoefficient(int i) const override {
		if (diffusionFactor > 0.0) {
			return Reactant::getDiffusionCoefficient(i);
		}

		return 0.0;
	}

};
//end class AlloyIntCluster

} /* namespace xolotlCore */
#endif

