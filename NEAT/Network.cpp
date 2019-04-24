
#include "Network.hpp"
#include "NEAT_GA.hpp"
#include "NodeTree.hpp"
using namespace NEAT;

#include <algorithm> //lower_bound

//TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <iostream>

Network::Network(bool _initialize, Parameters &_params): params(_params)
														,numInputs(params.numInputs)
														,numOutputs(params.numOutputs)
														,numNonHidden(params.numInputs+params.numOutputs+1)
														,numHidden(0)
														,numNodes(numNonHidden)
{
	if(_initialize) initialize();
}
Network::Network(Network&& other): params(other.params), numInputs(other.numInputs), numOutputs(other.numOutputs)
								, numNonHidden(other.numNonHidden), numHidden(other.numHidden), numNodes(other.numNodes)
{
	std::swap(connections,other.connections);
}
Network::Network(const Network& other): params(other.params), numInputs(other.numInputs), numOutputs(other.numOutputs)
								, numNonHidden(other.numNonHidden), numHidden(other.numHidden), numNodes(other.numNodes)
{
	unsigned numOtherConnections = other.connections.size();
	connections.reserve(numOtherConnections);

	for (unsigned i=0; i < numOtherConnections; ++i)
		connections.push_back(new Connection(*other.connections[i]));
}

Network::~Network(){ clear(); }
void Network::clear(){
	for(unsigned i=0, size=connections.size(); i<size; ++i)
		if(connections[i] != nullptr){
			delete connections[i];
			connections[i] = nullptr;
		}
}


void Network::initialize(){
	for(unsigned i=0; i<numInputs+1; ++i)
		for(unsigned o=numInputs+1; o<numNonHidden; ++o)
			addConnection(i, o);
}

bool Network::isOutputNode(unsigned n){
	return (n>=numInputs+1 && n<numNonHidden);
}
unsigned Network::countNode(unsigned n){
	unsigned count = 0;
	for(unsigned c=0, size=connections.size(); c<size; ++c)
		if(connections[c]->pre == n || connections[c]->pos == n)
			++count;
	return count;
}
unsigned Network::connectionExists(unsigned from, unsigned to){ //checks both from-to and to-from
	unsigned c=0;
	for(unsigned size=connections.size(); c<size; ++c)
		if((connections[c]->pre == from && connections[c]->pos == to) || (connections[c]->pre == to && connections[c]->pos == from))
			break;
	return c;
}

unsigned Network::getNumInputs() 		const { return numInputs;  }
unsigned Network::getNumHidden() 		const { return numHidden;  }
unsigned Network::getNumOutputs() 		const { return numOutputs; }
unsigned Network::getNumNodes() 		const { return numNodes; }
unsigned Network::getNumConnections() 	const { return connections.size(); }
const std::vector<Connection*>& Network::getConnections() const{ return connections; }


void Network::addNode(unsigned id){
	if(id >= numNodes){ //not done in 'initialize'
		for(; numNodes<id+1;){
			++numNodes;
			++numHidden;
			addConnection(0,numNodes-1); //add bias
		}
	}
}


void Network::addConnection(unsigned from, unsigned to){ //doesn't check if connection already exists
	if(to < numInputs+1)
		throw std::runtime_error("'to' connection can't be an input connection.");
	if(isOutputNode(from))
		throw std::runtime_error("'from' connection can't be an output connection.");

	double weight = GA::generator(params.minInitWeight,params.maxInitWeight);

	unsigned where = params.addConnection(from,to);
	auto new_connection = new Connection(from, to, where, weight, true);

	if(connections.size()==0 || where>connections.back()->innovationNumber)
		connections.push_back(new_connection);
	else{
		auto lower = lower_bound(connections.begin(), connections.end(), new_connection, Connection::compare);
		connections.insert(lower, new_connection);
	}

	addNode(from);
	addNode(to);
}

void Network::addConnection(const Connection* con){
	auto new_connection = new Connection(*con);
	connections.push_back(new_connection);

	unsigned max_node_id = std::max(con->pre, con->pos);
	if(max_node_id >= numNodes){ //not done in 'initialize'
		numNodes  = max_node_id + 1;
		numHidden = numNodes - numNonHidden;
	}
}



Network Network::create(const GA::Evolver<Network>* ev){
	Network net(true,((Evolver*)ev)->params);
	return net;
}
Network Network::crossover(const Network& n1, const Network& n2, double fit1, double fit2){
	if(&n1.params != &n2.params)
		throw std::runtime_error("Parent's parameters not equal.");
	if(n1.connections.size()==0 || n2.connections.size()==0)
		throw std::runtime_error("Parents with empty connections.");

	unsigned iter1 = 0, inno1 = n1.connections[iter1]->innovationNumber, size1 = n1.connections.size();
	unsigned iter2 = 0, inno2 = n2.connections[iter2]->innovationNumber, size2 = n2.connections.size();

	unsigned last = n1.params.getInnovationNumber();

	Network child(false, n1.params);

	while(inno1 < last && inno2 < last){
		if(inno1==inno2){
			std::cout << "Matching gene " << inno1 << std::endl;
			child.addConnection(GA::generator() < 0.5 ? n1.connections[iter1] : n2.connections[iter2]);
		}
		else if(inno1<inno2 && (fit1>fit2 || (fit1==fit2 && GA::generator()<0.5))){
			std::cout << "Extra gene in N1: " << inno1 << std::endl;
			child.addConnection(n1.connections[iter1]);
		}
		else if(inno1>inno2 && (fit1<fit2 || (fit1==fit2 && GA::generator()<0.5))){
			std::cout << "Extra gene in N2: " << inno2 << std::endl;
			child.addConnection(n2.connections[iter2]);
		}

		if(inno1<=inno2){
			++iter1;
			inno1 = (iter1>=size1 ? last : n1.connections[iter1]->innovationNumber);
		}
		if(inno1>=inno2){
			++iter2;
			inno2 = (iter2>=size2 ? last : n2.connections[iter2]->innovationNumber);
		}
	}

	std::cout << "N1: " << toString(n1) << std::endl;
	std::cout << "N2: " << toString(n2) << std::endl;
	std::cout << "Child: " << toString(child) << std::endl;

	return n1;
}
void Network::mutate(Network& child){
	if(GA::generator() < child.params.mutateRateNewNode)
		child.mutateAddNode();

	if(GA::generator() < child.params.mutateRateNewConnection)
		child.mutateAddConnection();

	child.mutatePerturbWeight();

	return;
}
std::string	Network::toString(const Network& net){
	std::stringstream str;
	str << "N" << net.getNumNodes() << "H" << net.getNumHidden() << Connection::toString(net.connections);
	return str.str();
}

void Network::mutatePerturbWeight(){
	for (unsigned i=0, size = connections.size(); i < size; ++i)
		if (GA::generator() < params.mutateRateWeightPerturbation)
			connections[i]->weight += GA::generator(-params.mutateMaxPerturbation, params.mutateMaxPerturbation);
}
bool Network::mutateAddConnection(){
	unsigned to = GA::generator.getI(params.numInputs+1, numNodes-1);

	//check if there are nodes 'to' is not connected to
	bool valid = false;
	for(unsigned n=0; n<numNodes; ++n){
		unsigned check = (to+n)%numNodes;
		if(check<numInputs+1) //bias and inputs cannot be end connections
			continue;
		unsigned max_connections = numNodes - numOutputs - ( isOutputNode(check) ? 0 : 1 );
		if(countNode(check) < max_connections ){
			to = check;
			valid = true;
			break;
		}
	}
	if(!valid)
		return false;

	unsigned from = GA::generator.getI(0, numNodes-1); //0 is bias and supposedly already exists. Let like this for now, but it's redundant

	while(from==to || isOutputNode(from) || connectionExists(from,to)!=connections.size())
		from = (from+1)%numNodes;

	addConnection(from,to);

	return true;
}
void Network::mutateAddNode(){
	unsigned chosen;
	do{
		chosen = GA::generator.getI(0, connections.size()-1);
	}while(connections[chosen]->pre == 0); //don't break connection with bias

	unsigned new_node = numNodes;

	connections[chosen]->disable(); //may be re-enabled in crossover
	addConnection(connections[chosen]->pre,new_node);
	addConnection(new_node,connections[chosen]->pos);
}

VecD Network::evaluate(const VecD& input) const{ return NodeTree(*this).evaluate(input); }