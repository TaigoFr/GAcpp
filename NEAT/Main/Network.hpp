#pragma once

#include "Connection.hpp"
#include "Parameters.hpp"
#include "Evolver.hpp"

namespace NEAT{

	class Network{
	public:
		Network(bool _initialize, Parameters &_params);

		Network(const Network&);
		Network(Network&&);
		~Network();
		void clear();
		Network& operator=(const Network&) = delete;
		Network& operator=(Network&&) = delete;

		//functions for GA::Evolver
		static Network 		create 	 (const GA::Evolver<Network>*);
		static Network 		crossover(const Network&,const Network&, double fit1, double fit2);
		static void 		mutate	 (Network&, const GA::Evolver<Network>*);
		static double		evaluate (const Network&);
		static std::string	toString (const Network&);

		unsigned getNumInputs() 	 const;
		unsigned getNumHidden() 	 const;
		unsigned getNumOutputs() 	 const;
		unsigned getNumNodes() 		 const;
		unsigned getNumConnections() const;
		const std::vector<Connection*>& getConnections() const;
		double getBias(unsigned node) const; //0. if not found

		VecD evaluate(const VecD& input) const; //returns output

		static double getDissimilarity(const Network&, const Network&);

		Parameters &params;
	private:
		//bias is node 0; then come inputs and outputs, and finally hidden
		const unsigned numInputs, numOutputs, numNonHidden; //numNonHidden = numInputs + numOutput + 1 (bias)
		unsigned numHidden, numNodes; // numNodes = numNonHidden + numHidden
		std::vector<Connection*> connections;

		void initialize();

		bool isOutputNode(unsigned n);
		unsigned countNode(unsigned n, bool countPreToo);
		bool isInPast(unsigned from, unsigned to); //tests if 'from' is in past of node 'to', in order to prevent loop formation
		unsigned connectionExists(unsigned from, unsigned to, bool checkDisabled); //checks both from-to and to-from

		void addNode(unsigned id, bool isInit);
		void addConnection(unsigned from, unsigned to, bool isInit); //doesn't check if connection already exists in Network (only in params)
		void addConnection(const Connection* con, bool enableChance); //assumess innovationNumber ordering and just copies and pushes back;
																	//possibly enables connection as 'mutateRateEnableChance'
		void mutatePerturbWeight();
		bool mutateAddConnection();
		void mutateAddNode();

	};
	
};