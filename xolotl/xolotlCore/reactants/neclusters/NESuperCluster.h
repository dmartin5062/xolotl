#ifndef NESUPERCLUSTER_H
#define NESUPERCLUSTER_H

// Includes
#include "NECluster.h"
#include <string>
#include <forward_list>

namespace xolotlCore {

/**
 *  A cluster gathering the average properties of many Xe clusters.
 */
class NESuperCluster: public NECluster {

public:
    // Our notion of the flux.
    // Must be public so we can define operator+/operator-. (?)
    struct Flux : public Reactant::Flux {

        double xeMomentFlux;

        Flux(void)
          : xeMomentFlux(0)
        { }

        Flux(const Flux& other)
          : Reactant::Flux(other) {
            xeMomentFlux = other.xeMomentFlux;
        }

        Flux& operator+=(const Flux& other) {
            // Let base class update its members.
            Reactant::Flux::operator+=(other);

            // Update our members.
            xeMomentFlux += other.xeMomentFlux;

            return *this;
        }

        Flux& operator-=(const Flux& other) { 
            // Let base class update its members.
            Reactant::Flux::operator-=(other);

            // Update our members.
            xeMomentFlux -= other.xeMomentFlux;

            return *this;
        }
    };

private:
	static std::string buildName(IReactant::SizeType nXe) {
		std::stringstream nameStream;
		nameStream << "Xe_" << nXe;
		return nameStream.str();
	}

protected:

	/**
	 * This is a protected class that is used to implement the flux calculations
	 * for two body production reactions.
	 *
	 * The constants are stored along the clusters taking part in the
	 * reaction or dissociation for faster computation because they only change
	 * when the temperature change. k is computed when setTemperature() is called.
	 */
	class SuperClusterProductionPair {
	public:

		/**
		 * The first cluster in the pair
		 */
		NECluster * first;

		/**
		 * The second cluster in the pair
		 */
		NECluster * second;

		/**
		 * The reaction/dissociation pointer to the list
		 */
		Reaction& reaction;

		/**
		 * All the coefficient needed to compute each element
		 */
		double a000;
		double a001;
		double a010;
		double a011;
		double a100;
		double a101;
		double a110;
		double a111;

		//! The constructor
		SuperClusterProductionPair(NECluster * firstPtr, NECluster * secondPtr,
				Reaction * _reaction) :
				first(firstPtr), second(secondPtr), reaction(*_reaction), a000(
						0.0), a001(0.0), a010(0.0), a011(0.0), a100(0.0), a101(
						0.0), a110(0.0), a111(0.0) {
		}
	};
    using ProductionPairList = std::vector<SuperClusterProductionPair>;

	/**
	 * This is a protected class that is used to implement the flux calculations
	 * for two dissociation reactions.
	 *
	 * The constants are stored along the clusters taking part in the
	 * reaction or dissociation for faster computation because they only change
	 * when the temperature change. k is computed when setTemperature() is called.
	 */
	class SuperClusterDissociationPair {
	public:

		/**
		 * The first cluster in the pair
		 */
		NECluster * first;

		/**
		 * The second cluster in the pair
		 */
		NECluster * second;

		/**
		 * The reaction/dissociation pointer to the list
		 */
		Reaction& reaction;

		/**
		 * All the coefficient needed to compute each element
		 */
		double a00;
		double a01;
		double a10;
		double a11;

		//! The constructor
		SuperClusterDissociationPair(NECluster * firstPtr,
				NECluster * secondPtr, Reaction * _reaction) :
				first(firstPtr), second(secondPtr), reaction(*_reaction), a00(
						0.0), a01(0.0), a10(0.0), a11(0.0) {
		}
	};
    using DissociationPairList = std::vector<SuperClusterDissociationPair>;

private:

	//! The mean number of xenon atoms in this cluster.
	double numXe;

	//! The total number of clusters gathered in this super cluster.
	int nTot;

	//! The width in the xenon direction.
	int sectionWidth;

	//! The dispersion in the group in the xenon direction.
	double dispersion;

	//! The map containing all the reacting pairs separated by original composition.
	std::map<int, std::vector<ClusterPair> > reactingMap;

	//! The map containing all the combining clusters separated by original composition.
	std::map<int, std::vector<CombiningCluster> > combiningMap;

	//! The map containing all the dissociating pairs separated by original composition.
	std::map<int, std::vector<ClusterPair> > dissociatingMap;

	//! The map containing all the emission pairs separated by original composition.
	std::map<int, std::vector<ClusterPair> > emissionMap;

	//! The list of optimized effective reacting pairs.
    ProductionPairList effReactingList;

	//! The list of optimized effective combining pairs.
    ProductionPairList effCombiningList;

	//! The list of optimized effective dissociating pairs.
    DissociationPairList effDissociatingList;

	//! The list of optimized effective emission pairs.
    DissociationPairList effEmissionList;


	/**
	 * This operation returns the total change in this cluster due to
	 * other clusters dissociating into it. Compute the contributions to
	 * the moment fluxes at the same time.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] flux The flux due to dissociation of other clusters
	 */
	void getDissociationFlux(const double* __restrict concs, int i,
                                Reactant::Flux& flux) const override;

	/**
	 * This operation param[out] fluxs the total change in this cluster due its
	 * own dissociation. Compute the contributions to
	 * the moment fluxes at the same time.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] flux The flux due to its dissociation
	 */
	void getEmissionFlux(const double* __restrict concs, int i,
                                Reactant::Flux& flux) const override;

	/**
	 * This operation param[out] fluxs the total change in this cluster due to
	 * the production of this cluster by other clusters. Compute the contributions to
	 * the moment fluxes at the same time.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] flux The flux due to this cluster being produced
	 */
	void getProductionFlux(const double* __restrict concs, int i,
                                Reactant::Flux& flux) const override;

	/**
	 * This operation param[out] fluxs the total change in this cluster due to
	 * the combination of this cluster with others. Compute the contributions to
	 * the moment fluxes at the same time.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] flux The flux due to this cluster combining with other clusters
	 */
	void getCombinationFlux(const double* __restrict concs, int i,
                                Reactant::Flux& flux) const override;


    /**
     * Add an amount to the value in the given concentration array
     * that represents our moment flux.
     *
     * @param concs Concentration array for desired grid point.
     * @param amount Amount to add to concentration value associated
     * with our moment flux.
     */
    void addToXeMoment(double* __restrict concs, double amount) const {
        concs[getMomentId() - 1] += amount;
    }

public:

	//! The vector of Xe clusters it will replace
	std::vector<NECluster *> xeVector;

	/**
	 * Default constructor, deleted because we require info to construct.
	 */
	NESuperCluster() = delete;

	/**
	 * The constructor. All NESuperClusters must be initialized with its
	 * composition.
	 *
	 * @param numXe The mean number of xenon atoms in this cluster
	 * @param nTot The total number of clusters in this cluster
	 * @param width The width of this super cluster in the xenon direction
	 * @param radius The mean radius
	 * @param energy The formation energy
	 * @param _network The network this cluster will belong to.
	 * @param registry The performance handler registry
	 */
	NESuperCluster(double numXe, int nTot, int width, double radius,
			double energy, IReactionNetwork& _network,
			std::shared_ptr<xolotlPerf::IHandlerRegistry> registry);

	/**
	 * Copy constructor, deleted to prevent use.
	 */
	NESuperCluster(NESuperCluster &other) = delete;

	//! Destructor
	~NESuperCluster() {
	}

	/**
	 * Update reactant using other reactants in its network.
	 */
	void updateFromNetwork() override;

	/**
	 * Group the same reactions together and add the reactions to the network lists.
	 */
	void optimizeReactions() override;

	/**
	 * Note that we result from the given reaction.
	 * Assumes the reaction is already in our network.
	 *
	 * \see Reactant.h
	 */
	void resultFrom(ProductionReaction& reaction, double *coef) override;

	/**
	 * Note that we combine with another cluster in a production reaction.
	 * Assumes that the reaction is already in our network.
	 *
	 * \see Reactant.h
	 */
	void participateIn(ProductionReaction& reaction, double *coef) override;

	/**
	 * Note that we combine with another cluster in a dissociation reaction.
	 * Assumes the reaction is already in our network.
	 *
	 * \see Reactant.h
	 */
	void participateIn(DissociationReaction& reaction, double *coef) override;

	/**
	 * Note that we emit from the given reaction.
	 * Assumes the reaction is already in our network.
	 *
	 * \see Reactant.h
	 */
	void emitFrom(DissociationReaction& reaction, double *coef) override;

	/**
	 * This operation returns false.
	 *
	 * @return True if mixed
	 */
	virtual bool isMixed() const override {
		return false;
	}

	/**
	 * This operation returns the total number of clusters it contains.
	 *
	 * @return The total number of clusters
	 */
	int getNTot() const {
		return nTot;
	}

	/**
	 * This operation returns the average number of clusters it contains.
	 *
	 * @return The average number of clusters
	 */
	double getAverage() const {
		return numXe;
	}

	/**
	 * Set the Xe vector
	 */
	void setXeVector(std::vector<NECluster *> vec) {
		xeVector = vec;
	}

	/**
	 * This operation returns the first xenon moment.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @return The moment
	 */
	double getMoment(const double* __restrict concs) const override {
        return concs[getMomentId() - 1];
    }

	/**
	 * Obtain current concentration.
	 *
	 * We hid the base class' getConcentration when
	 * we defined the two-argument version here.  This tells the compiler
	 * to allow us to use the base class' version also
	 * without having to explicitly specify the class scope when calling it.
	 *
	 * @return The concentration of the reactant.
	 */
	using Reactant::getConcentration;

	/**
	 * This operation returns the current concentration.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param distXe The xenon distance in the group
	 * @return The concentration of this reactant
	 */
	double getConcentration(const double* __restrict concs, double distXe) const override {
		return getConcentration(concs) + (distXe * getMoment(concs));
	}

	/**
	 * This operation returns the current total concentration of clusters in the group.

     * @param concs Current solution vector for desired grid point.
	 * @return The concentration
	 */
	double getTotalConcentration(const double* __restrict concs) const;

	/**
	 * This operation returns the current total concentration of xenon in the group.

     * @param concs Current solution vector for desired grid point.
	 * @return The concentration
	 */
	double getTotalXenonConcentration(const double* __restrict concs) const;

	/**
	 * This operation returns the distance to the mean.
	 *
	 * @param xe The number of xenon
	 * @return The distance to the mean number of xenon in the group
	 */
	double getDistance(int xe) const;

	/**
	 * Calculate the dispersion of the group.
	 */
	void computeDispersion();

	/**
	 * Get the dispersion of the group.
	 */
	double getDispersion() const {
		return dispersion;
	}

	/**
	 * This operation reset the connectivity sets based on the information
	 * in the production and dissociation vectors.
	 */
	void resetConnectivities() override;

	/**
     * Compute total flux(es) of this reactant using current concentrations
     * into their respective locations in the output concentrations.
	 *
     * @param concs Current concentrations for desired grid point.
	 * @param xi The location on the grid in the depth direction
     * @param updatedConcs Updated concentrations for desired grid point.
	 */
    void computeTotalFluxes(const double* __restrict concs, int xi,
                            double* __restrict updatedConcs) const override {

        // Compute total flux based on reactions we particiate in.
        auto flux = getTotalFluxHelper<NESuperCluster::Flux>(concs, xi);

        // Update our concentration in the output concentration array.
        addToConcentration(updatedConcs, flux.flux);

        // Update our moment in the output concentration array.
        addToXeMoment(updatedConcs, flux.xeMomentFlux);
    }

	/**
	 * This operation works as getPartialDerivatives above, but instead of
	 * returning a vector that it creates it fills a vector that is passed to
	 * it by the caller. This allows the caller to optimize the amount of
	 * memory allocations to just one if they are accessing the partial
	 * derivatives many times.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] partials The vector that should be filled with the partial 
     * derivatives for this reactant where index zero corresponds to the first 
     * reactant in the list returned by the ReactionNetwork::getAll() 
     * operation. The size of the vector should be equal to 
     * ReactionNetwork::size().
	 */
	void getPartialDerivatives(const double* __restrict concs, int i,
            std::vector<double> & partials) const override;

	/**
	 * This operation computes the partial derivatives due to production
	 * reactions.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] partials The vector that should be filled with the partial 
     * derivatives for this reactant where index zero corresponds to the first 
     * reactant in the list returned by the ReactionNetwork::getAll() 
     * operation. The size of the vector should be equal to 
     * ReactionNetwork::size().
	 */
	void getProductionPartialDerivatives(const double* __restrict concs, int i,
            std::vector<double> & partials) const override;

	/**
	 * This operation computes the partial derivatives due to combination
	 * reactions.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] partials The vector that should be filled with the partial 
     * derivatives for this reactant where index zero corresponds to the first 
     * reactant in the list returned by the ReactionNetwork::getAll() 
     * operation. The size of the vector should be equal to 
     * ReactionNetwork::size().
	 */
	void getCombinationPartialDerivatives(const double* __restrict concs, int i,
            std::vector<double> & partials) const override;

	/**
	 * This operation computes the partial derivatives due to dissociation of
	 * other clusters into this one.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] partials The vector that should be filled with the partial 
     * derivatives for this reactant where index zero corresponds to the first 
     * reactant in the list returned by the ReactionNetwork::getAll() 
     * operation. The size of the vector should be equal to 
     * ReactionNetwork::size().
	 */
	void getDissociationPartialDerivatives(const double* __restrict concs, int i,
            std::vector<double> & partials) const override;

	/**
	 * This operation computes the partial derivatives due to emission
	 * reactions.
	 *
     * @param concs Current solution vector for desired grid point.
	 * @param i The location on the grid in the depth direction
	 * @param[out] partials The vector that should be filled with the partial 
     * derivatives for this reactant where index zero corresponds to the first 
     * reactant in the list returned by the ReactionNetwork::getAll() 
     * operation. The size of the vector should be equal to 
     * ReactionNetwork::size().
	 */
	void getEmissionPartialDerivatives(const double* __restrict concs, int i,
            std::vector<double> & partials) const override;

	/**
	 * This operation computes the partial derivatives for the xenon moment.
	 *
	 * @param partials The vector into which the partial derivatives should be
	 * inserted.
	 */
	void getMomentPartialDerivatives(std::vector<double> & partials) const;

	/**
	 * This operation returns the vector of production reactions in which
	 * this cluster is involved, containing the id of the reactants, and
	 * the a coefs.
	 *
	 * @return The vector of productions
	 */
	virtual std::vector<std::vector<double> > getProdVector() const override;

	/**
	 * This operation returns the vector of combination reactions in which
	 * this cluster is involved, containing the id of the other reactants, and
	 * the a coefs.
	 *
	 * @return The vector of combinations
	 */
	virtual std::vector<std::vector<double> > getCombVector() const override;

	/**
	 * This operation returns the vector of dissociation reactions in which
	 * this cluster is involved, containing the id of the emitting reactants, and
	 * the a coefs.
	 *
	 * @return The vector of dissociations
	 */
	virtual std::vector<std::vector<double> > getDissoVector() const override;

	/**
	 * This operation returns the vector of emission reactions in which
	 * this cluster is involved, containing the a coefs.
	 *
	 * @return The vector of productions
	 */
	virtual std::vector<std::vector<double> > getEmitVector() const override;

	/**
	 * This operation returns the section width.
	 *
	 * @return The width of the section
	 */
	int getSectionWidth() const {
		return sectionWidth;
	}

	/**
	 * Detect if given coordinates are in this cluster's group.
	 *
	 * @param _nXe number of Xe of interest.
	 * @return True if the coordinates are contained in our super cluster.
	 */
	bool isIn(IReactant::SizeType nXe) const {
		return (nXe > numXe - (double) sectionWidth / 2.0
				&& nXe < numXe + (double) sectionWidth / 2.0);
	}

};
//end class NESuperCluster

inline
const NESuperCluster::Flux operator+(const NESuperCluster::Flux& a,
                                    const NESuperCluster::Flux& b) {
    NESuperCluster::Flux ret(a);
    ret += b;
    return ret;
}

inline
const NESuperCluster::Flux operator-(const NESuperCluster::Flux& a,
                                    const NESuperCluster::Flux& b) {
    NESuperCluster::Flux ret(a);
    ret -= b;
    return ret;
}

} /* end namespace xolotlCore */
#endif
