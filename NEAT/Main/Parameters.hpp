#pragma once

#include "Connection.hpp"

namespace NEAT{

class Parameters{
	public:
		unsigned numInputs; //+1 -> bias
		unsigned numOutputs;

//to keep fixed
		float mutateRateNewConnection;
		float mutateRateNewNode;
		float mutateRateWeightPerturbation;
		float mutateMaxPerturbation;
		float mutateRateEnableChance;
		bool  enableInExcessOrDisjoint;

		float maxInitWeight;
		float maxWeight;

		// Species similarity weighting factors
		float excessFactor;
		float disjointFactor;
		float averageWeightDifferenceFactor;
		float inputCountDifferenceFactor;
		float outputCountDifferenceFactor;

		float speciationTolerance;
		float preferSimilarFactor;

		Parameters();
		~Parameters();
		void clear();
		void check();
		unsigned connectionExists(unsigned from, unsigned to);
		unsigned addConnection(unsigned from, unsigned to); //return innovationNumber of that connection
		unsigned getInnovationNumber();

	private:
		std::vector<BasicConnection*> allConnections;
		unsigned innovationNumber; // = allConnections.size() = "next innovation number"
	};

};