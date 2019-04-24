#pragma once

#include <string>
#include <vector>

#include "Connection.hpp"

namespace NEAT{

class Parameters{
	public:
		unsigned numInputs; //+1 -> bias
		unsigned numOutputs;

		float mutateRateNewConnection;
		float mutateRateNewNode;
		float mutateRateWeightPerturbation;
		float mutateMaxPerturbation;

//to keep fixed
		float minInitWeight;
		float maxInitWeight;

//old
		// float mutateRateDisableChange;

		// float _speciationTolerance;
		// float _preferSimilarFactor;
		// float _reproduceRatio;

		// float _minWeight;
		// float _maxWeight;
		// float _minBias;
		// float _maxBias;

		// // Species similarity weighting factors
		// float _excessFactor;
		// float _disjointFactor;
		// float _averageWeightDifferenceFactor;
		// float _inputCountDifferenceFactor;
		// float _outputCountDifferenceFactor;

		// size_t _populationSize;

		// size_t _numElites;

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