
#include "NodeTree.hpp"

using namespace NEAT;

NodeTree::NodeTree(const Network& net): numInputs(net.getNumInputs()), numOutputs(net.getNumOutputs()){
	FORV(i, net.getNumNodes()) nodes.push_back(new Node());

	const std::vector<Connection*> &connections = net.getConnections();
	FORV(i, net.getNumConnections()){
		if(!connections[i]->getEnabled()) continue; //don't add disabled connections
		auto preNode = nodes[connections[i]->pre];
		preNode->addPosNode();
		nodes[connections[i]->pos]->addPreNode(preNode, connections[i]->weight);
	}
}

NodeTree::~NodeTree(){ clear(); }
void NodeTree::clear(){
	FORV(i, nodes.size()){
		if(nodes[i]!=nullptr){
			delete nodes[i];
			nodes[i] = nullptr;
		}
	}
}

VecD NodeTree::evaluate(const VecD& input) const{

	nodes[0]->setValue(1.); //bias

	FOR(i, numInputs) nodes[i+1]->setValue(input[i]);

	VecD output(numOutputs);
	FOR(i, numOutputs) output[i] = nodes[numInputs+1+i]->evaluate();
	return output;
}
