#pragma once

#include "Node.hpp"
#include "Network.hpp"
#include "Vec.hpp"


namespace NEAT{

	class NodeTree{
	public:
		NodeTree(const Network&);
		NodeTree(NodeTree&&)	  = delete;
		NodeTree(const NodeTree&) = delete; //just to make sure
		~NodeTree();
		void clear();
		NodeTree& operator=(const NodeTree&) = delete;
		NodeTree& operator=(NodeTree&&) 	 = delete;

		VecD   evaluate(const VecD& input) const; //returns output

	private:
		unsigned numInputs, numOutputs;
		std::vector<Node*> nodes;
	};

};