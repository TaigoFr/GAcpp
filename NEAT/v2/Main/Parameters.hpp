#pragma once

#include "Connection.hpp"

namespace NEAT{

	class Parameters{
	public:
		unsigned numInputs; //+1 -> bias
		unsigned numOutputs;

//to keep fixed
		float rateCrossoverAverage;

		float mutateRateNewConnection;
		float mutateRateNewNode;
		float mutateRateWeightPerturbation;
		float mutateRateWeightChange;
		float mutateRateToggle;
		float mutateMaxPerturbation;
		float mutateMaxChange;
		float mutateRateEnableChance;
		bool  enableInExcessOrDisjoint;

		float maxInitWeight;
		float maxWeight;

		// Species similarity weighting factors
		float excessFactor;
		float disjointFactor;
		float averageWeightDifferenceFactor;

		Parameters();
		~Parameters();
		void clear();
		void check() const;
		unsigned connectionExists(unsigned from, unsigned to) const;
		unsigned addConnection(unsigned from, unsigned to); //return innovationNumber of that connection
		inline unsigned getInnovationNumber(){ return innovationNumber; }

	private:
		std::vector<BasicConnection*> allConnections;
		unsigned innovationNumber; // = allConnections.size() = "next innovation number"
	};

};