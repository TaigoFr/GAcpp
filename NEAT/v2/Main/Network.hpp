#pragma once

#include "Connection.hpp"
#include "Parameters.hpp"
#include "Evolver.hpp"

namespace NEAT{

	class Network{
	public:
		Network(bool _initialize, Parameters &_params);
		Network(const std::string& file);

		Network(const Network&);
		Network(Network&&);
		~Network();
		void clear();
		Network& operator=(const Network&) 	= delete;
		Network& operator=(Network&&) 		= delete;

		void write(const std::string& path) const;
		void read (const std::string& path);
		void print() const;

		//functions for GA::Evolver
		static Network 		create 	 (const GA::Evolver<Network>*);
		static Network 		crossover(const Network&,const Network&, double fit1, double fit2);
		static void 		mutate	 (Network&, const GA::Evolver<Network>*);
		static double		evaluate (const Network&);
		static std::string	toString (const Network&);

		inline unsigned getNumInputs() 		const { return numInputs;  }
		inline unsigned getNumHidden() 		const { return numHidden;  }
		inline unsigned getNumOutputs() 	const { return numOutputs; }
		inline unsigned getNumNodes() 		const { return numNodes; }
		inline unsigned getNumConnections()	const { return connections.size(); }
		inline const std::vector<Connection*>& getConnections() const{ return connections; }
		double getBias(unsigned node) const; //0. if not found

		VecD evaluate(const VecD& input) const; //returns output (not protected if input.size()!=numInputs, for performance boost)

		static double dissimilarity(const Network&, const Network&);

		Parameters *params;
	private:
		//bias is node 0; then come inputs and outputs, and finally hidden
		unsigned numInputs, numOutputs, numNonHidden, numHidden, numNodes; // numNonHidden 	= numInputs + numOutput + 1 (bias)
																		   // numNodes 	 	= numNonHidden + numHidden
		std::vector<Connection*> connections;

		void initialize();

		bool isOutputNode(unsigned n) const;
		unsigned countNode(unsigned n, bool countPre, bool countPos = true) const;
		bool isInPast(unsigned from, unsigned to) const; //tests if 'from' is in past of node 'to', in order to prevent loop formation
		unsigned connectionExists(unsigned from, unsigned to, bool checkDisabled) const; //checks both from-to and to-from

		void addNode(unsigned id, bool isInit);
		void addConnection(unsigned from, unsigned to, bool isInit); //doesn't check if connection already exists in Network (only in params)
		void addConnection(const Connection* con, bool enableChance); //assumess innovationNumber ordering and just copies and pushes back;
		
		static Network crossoverNormal (const Network&,const Network&, double fit1, double fit2);
		static Network crossoverAverage(const Network&,const Network&, double fit1, double fit2);

		void mutatePerturbWeight();
		void mutateChangeWeight();
		bool mutateToggle();
		bool mutateAddConnection();
		void mutateAddNode();

	};
	
};