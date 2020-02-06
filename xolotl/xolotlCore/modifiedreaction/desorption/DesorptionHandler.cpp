// Includes
#include <DesorptionHandler.h>
#include <MathUtils.h>
#include <iostream>
#include <algorithm>
#include <mpi.h>

namespace xolotlCore {

void DesorptionHandler::initialize(int nx, int ny, int nz) {
	// Clear the vector of H indices at each grid point
	hClusters.clear();

	// Change the value of ny and nz in 1D and 2D so that the same loop
	// works in every case
	if (nz == 0)
		nz = 1;
	if (ny == 0)
		ny = 1;

	// Loop on the grid points in the Z direction
	// TODO even with the reserve ops, this might involve lots of
	// separate memory allocations.
	hClusters.reserve(nz);
	for (int k = 0; k < nz; k++) {
		// Create the temporary 2D vector
		ReactantRefVector2D temp2DVector;

		// Loop on the grid points in the Y direction
		temp2DVector.reserve(ny);
		for (int j = 0; j < ny; j++) {
			// Create the temporary 1D vector
			ReactantRefVector1D temp1DVector;

			// Loop on the grid points in the depth direction
			temp1DVector.reserve(nx);
			for (int i = 0; i < nx; i++) {
				// Indicate no bubbles at this grid point.
				temp1DVector.emplace_back();
			}

			// Give the 1D vector to the 2D vector
			temp2DVector.emplace_back(temp1DVector);
		}

		// Give the 2D vector to the final vector
		hClusters.emplace_back(temp2DVector);
	}

	// We don't need to add connectivities because it is a self reaction

	return;
}

void DesorptionHandler::initializeIndex1D(int surfacePos,
		const IReactionNetwork& network, int nx, int xs) {
	// Clear the vector of H indices
	hClusters.clear();

	// Create the temporary 2D vector
	ReactantRefVector2D temp2DVector;
	// Create the temporary 1D vector
	ReactantRefVector1D temp1DVector;

	// Loop on the grid points in the depth direction
	for (int i = 0; i < nx; i++) {
		// If we are on the left side of the surface there is no
		// modified trap-mutation
		if (i + xs <= surfacePos || i + xs > surfacePos + 1) {
			temp1DVector.emplace_back();
			continue;
		}

		std::vector<std::reference_wrapper<IReactant> > indices;
		// Loop on D
		for (auto const& dMapItem : network.getAll(ReactantType::D)) {
			auto& cluster = static_cast<PSICluster&>(*(dMapItem.second));
			if (cluster.getSize() == 1)
				indices.emplace_back(cluster);
		}
		// Loop on T
		for (auto const& tMapItem : network.getAll(ReactantType::T)) {
			auto& cluster = static_cast<PSICluster&>(*(tMapItem.second));
			if (cluster.getSize() == 1)
				indices.emplace_back(cluster);
		}
		// Get the number of clusters
		nCluster = indices.size();
		// Add indices to the index vector
		temp1DVector.emplace_back(indices);
	}

	// Give the 1D vector to the 2D vector
	temp2DVector.emplace_back(temp1DVector);

	// Give the 2D vector to the final vector
	hClusters.emplace_back(temp2DVector);

	return;
}

void DesorptionHandler::initializeIndex2D(std::vector<int> surfacePos,
		const IReactionNetwork& network, int nx, int xs, int ny, int ys) {
	// Clear the vector of H indices
	hClusters.clear();

	// Create the temporary 2D vector
	ReactantRefVector2D temp2DVector;

	// Loop on the grid points in the Y direction
	temp2DVector.reserve(ny);
	for (int j = 0; j < ny; j++) {

		// Create the temporary 1D vector
		ReactantRefVector1D temp1DVector;

		// Loop on the grid points in the depth direction
		for (int i = 0; i < nx; i++) {
			// If we are on the left side of the surface there is no
			// modified trap-mutation
			if (i + xs <= surfacePos[j + ys]
					|| i + xs > surfacePos[j + ys] + 1) {
				temp1DVector.emplace_back();
				continue;
			}

			std::vector<std::reference_wrapper<IReactant> > indices;
			// Loop on D
			for (auto const& dMapItem : network.getAll(ReactantType::D)) {
				auto& cluster = static_cast<PSICluster&>(*(dMapItem.second));
				if (cluster.getSize() == 1)
					indices.emplace_back(cluster);
			}
			// Loop on T
			for (auto const& tMapItem : network.getAll(ReactantType::T)) {
				auto& cluster = static_cast<PSICluster&>(*(tMapItem.second));
				if (cluster.getSize() == 1)
					indices.emplace_back(cluster);
			}
			// Get the number of clusters
			nCluster = indices.size();
			// Add indices to the index vector
			temp1DVector.emplace_back(indices);
		}

		// Give the 1D vector to the 2D vector
		temp2DVector.push_back(temp1DVector);
	}

	// Give the 2D vector to the final vector
	hClusters.push_back(temp2DVector);

	return;
}

void DesorptionHandler::initializeIndex3D(
		std::vector<std::vector<int> > surfacePos,
		const IReactionNetwork& network, int nx, int xs, int ny, int ys, int nz,
		int zs) {
	// Clear the vector of H indices
	hClusters.clear();

	// Loop on the grid points in the Z direction
	hClusters.reserve(nz);
	for (int k = 0; k < nz; k++) {

		// Create the temporary 2D vector
		ReactantRefVector2D temp2DVector;

		// Loop on the grid points in the Y direction
		temp2DVector.reserve(ny);
		for (int j = 0; j < ny; j++) {
			// Create the temporary 1D vector
			ReactantRefVector1D temp1DVector;

			// Loop on the grid points in the depth direction
			for (int i = 0; i < nx; i++) {
				// If we are on the left side of the surface there is no
				// modified trap-mutation
				if (i <= surfacePos[j + ys][k + zs]
						|| i > surfacePos[j + ys][k + zs] + 1) {
					temp1DVector.emplace_back();
					continue;
				}

				std::vector<std::reference_wrapper<IReactant> > indices;
				// Loop on D
				for (auto const& dMapItem : network.getAll(ReactantType::D)) {
					auto& cluster = static_cast<PSICluster&>(*(dMapItem.second));
					if (cluster.getSize() == 1)
						indices.emplace_back(cluster);
				}
				// Loop on T
				for (auto const& tMapItem : network.getAll(ReactantType::T)) {
					auto& cluster = static_cast<PSICluster&>(*(tMapItem.second));
					if (cluster.getSize() == 1)
						indices.emplace_back(cluster);
				}
				// Get the number of clusters
				nCluster = indices.size();
				// Add indices to the index vector
				temp1DVector.emplace_back(indices);
			}

			// Give the 1D vector to the 2D vector
			temp2DVector.push_back(temp1DVector);
		}

		// Give the 2D vector to the final vector
		hClusters.push_back(temp2DVector);
	}

	return;
}

void DesorptionHandler::computeDesorption(double *concOffset,
		double *updatedConcOffset, int xi, int yj, int zk) {
	// Loop on the list
	for (IReactant const& currReactant : hClusters[zk][yj][xi]) {
		// Get the stored cluster and its ID
		auto const& cluster = static_cast<PSICluster const&>(currReactant);
		auto clusterIndex = cluster.getId() - 1;

		// Get the initial concentration of hydrogen
		double oldConc = concOffset[clusterIndex];

		// Check the equilibrium concentration
		if (oldConc > equilibriumConc)
			updatedConcOffset[clusterIndex] -= kRecombination * oldConc
					* oldConc;
	}

	return;
}

int DesorptionHandler::computePartialsForDesorption(double *val, int *indices,
		int xi, int yj, int zk) {
	// Consider all clusters at this grid point.
	// TODO Relying on convention for indices in indices/vals arrays is
	// error prone - could be done with multiple parallel arrays.
	uint32_t i = 0;
	for (IReactant const& currReactant : hClusters[zk][yj][xi]) {
		// Get the stored cluster and its ID
		auto const& cluster = static_cast<PSICluster const&>(currReactant);
		auto clusterIndex = cluster.getId() - 1;

		// Get the initial concentration of hydrogen
		double oldConc = cluster.getConcentration();

		// Check the equilibrium concentration
		if (oldConc > equilibriumConc) {
			indices[i] = clusterIndex;
			val[i] = -2.0 * kRecombination * oldConc;

			// Advance to next indices/vals index.
			++i;
		}
	}

	return i;
}

}/* end namespace xolotlCore */
