#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Connection.hpp"
#include "Node.hpp"
#include "Evolver.hpp"

namespace NEAT{

	struct Parameters{
		std::vector<std::shared_ptr<BasicConnection>> allConnections;

		unsigned numInputs;
		unsigned numOutputs;

		unsigned innovationNumber;

		float mutateRateNewConnection;
		float mutateRateNewNode;
		float mutateRateWeightPerturbation;
		float mutateMaxPerturbation;

//to keep fixed
		float minInitWeight;
		float maxInitWeight;
		float minInitBias;
		float maxInitBias;

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
		void check();
		unsigned connectionExists(unsigned from, unsigned to);
		unsigned addConnection(unsigned from, unsigned to); //return innovationNumber of that connection
	};

	class Network{
	public:
		Network(bool _initialize, Parameters &_params);

		Network(const Network&);
		Network(Network&&);
		~Network() = default;
		Network& operator=(const Network&) = delete;
		Network& operator=(Network&&) = delete;

		//functions for GA::Evolver
		static Network 		create 	 (const GA::Evolver<Network>*);
		static Network 		crossover(const Network&,const Network&);
		static void 		mutate	 (Network&);
		static double		evaluate (const Network&);
		static std::string	toString (const Network&);

		unsigned getNumInputs() 	 const;
		unsigned getNumHidden() 	 const;
		unsigned getNumOutputs() 	 const;
		unsigned getNumNodes() 		 const;
		unsigned getNumConnections() const;

		Parameters &params;
	private:
		const unsigned numInputs, numNonHidden;
		unsigned numHidden;
		std::vector<std::shared_ptr<Connection>> connections;
		std::vector<std::shared_ptr<Node>> nodes; //first Input, then Output, then Hidden

		void initialize();

		bool isOutputNode(unsigned n);
		unsigned countNode(unsigned n);
		unsigned connectionExists(unsigned from, unsigned to); //checks both from-to and to-from

		void addNode(unsigned id, double bias = -1.e308);
		void addConnection(unsigned from, unsigned to); //doesn't check if connection already exists

		void mutatePerturbWeight();
		bool mutateAddConnection();
		void mutateAddNode();

	};
	
};