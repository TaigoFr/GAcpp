
#pragma once

#include <vector>
#include <cmath>

#include "Vec.hpp" //FORV

namespace NEAT{

	class Node;
	struct Synapse{
		Node* pre_node;
		double weight;
	};

	class Node{
	public:
		inline static double sigmoid(double x) 			{ return 1. / (1. + std::exp(-x)); }
		inline static double sigmoidDerivative(double x){ double s = sigmoid(x); return s * (1. - s); }

		inline Node(): set(false), value(0.), synapses(0), pos_nodes(0){}

		inline void addPreNode(Node* pre_node, double weight){	synapses.push_back(Synapse{pre_node,weight}); }
		inline void addPosNode(){ ++pos_nodes; }

		inline void setValue(double v){
			value = v;
			set = true;
		}
		double evaluate(){
			if(set) return value;

			double sum = 0.;
			FORV(i, synapses.size()) sum += synapses[i].weight * synapses[i].pre_node->evaluate();
			value = sigmoid(sum);
			set = true;
			return value;
		}

	private:
		bool set;
		double value;
		std::vector< Synapse > synapses;
		unsigned pos_nodes;
	};

}