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
		static void 		mutate	 (Network&);
		static double		evaluate (const Network&);
		static std::string	toString (const Network&);

		unsigned getNumInputs() 	 const;
		unsigned getNumHidden() 	 const;
		unsigned getNumOutputs() 	 const;
		unsigned getNumNodes() 		 const;
		unsigned getNumConnections() const;
		const std::vector<Connection*>& getConnections() const;

		VecD evaluate(const VecD& input) const; //returns output

		Parameters &params;
	private:
		//bias is node 0; then come inputs and outputs, and finally hidden
		const unsigned numInputs, numOutputs, numNonHidden; //numNonHidden = numInputs + numOutput + 1 (bias)
		unsigned numHidden, numNodes; // numNodes = numNonHidden + numHidden
		std::vector<Connection*> connections;

		void initialize();

		bool isOutputNode(unsigned n);
		unsigned countNode(unsigned n);
		unsigned connectionExists(unsigned from, unsigned to); //checks both from-to and to-from

		void addNode(unsigned id);
		void addConnection(unsigned from, unsigned to); //doesn't check if connection already exists in Network (only in params)
		void addConnection(const Connection* con); //assumess innovationNumber ordering and just copies and pushes back

		void mutatePerturbWeight();
		bool mutateAddConnection();
		void mutateAddNode();

	};
	
};