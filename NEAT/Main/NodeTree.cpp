
#include "NodeTree.hpp"

using namespace NEAT;

NodeTree::NodeTree(const Network& net): numInputs(net.getNumInputs()), numOutputs(net.getNumOutputs()){
	for(unsigned i=0, numNodes=net.getNumNodes(); i<numNodes; ++i)
		nodes.push_back(new Node());

	const std::vector<Connection*> &connections = net.getConnections();
	for(unsigned i=0, size=net.getNumConnections(); i<size; ++i){
		if(!connections[i]->getEnabled()) continue; //don't add disabled connections
		auto preNode = nodes[connections[i]->pre];
		preNode->addPosNode();
		nodes[connections[i]->pos]->addPreNode(preNode, connections[i]->weight);
	}
}

NodeTree::~NodeTree(){ clear(); }
void NodeTree::clear(){
	for(unsigned i=0, size=nodes.size(); i<size; ++i){
		if(nodes[i]!=nullptr){
			delete nodes[i];
			nodes[i] = nullptr;
		}
	}
}

VecD NodeTree::evaluate(const VecD& input){

	nodes[0]->setValue(1.); //bias

	for(unsigned i=0; i<numInputs; ++i)
		nodes[i+1]->setValue(input[i]);

	VecD output(numOutputs);
	for(unsigned i=0; i<numOutputs; ++i)
		output[i] = nodes[numInputs+1+i]->evaluate();
	return output;
}
