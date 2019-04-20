
#include "Network.hpp"
#include "NEAT_GA.hpp"
using namespace NEAT;

#include <algorithm> //lower_bound

//TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <iostream>

Parameters::Parameters():
	allConnections(0)
	,numInputs(0)
	,numOutputs(0)
	
	,innovationNumber(0) // = allConnections.size()

	,mutateRateNewConnection(0.2f)
	,mutateRateNewNode(0.1f)
	,mutateRateWeightPerturbation(0.2f)
	,mutateMaxPerturbation(0.75f)
//to keep fixed
	,minInitWeight(-1.0f)
	,maxInitWeight(1.0f)
	,minInitBias(-1.0f)
	,maxInitBias(1.0f)
//old
// ,mutateRateDisableChange(0.125f)
// _speciationTolerance(2.2f),
// _preferSimilarFactor(0.05f),
// _reproduceRatio(0.9f),
// _newConnectionMutationRate(0.2f),
// _newNodeMutationRate(0.1f),
// _weightPerturbationChance(0.2f),
// _disableGeneChance(0.125f),
// _minInitWeight(-1.0f),
// _maxInitWeight(1.0f),
// _minInitBias(-1.0f),
// _maxInitBias(1.0f),
// _minWeight(-8.0f),
// _maxWeight(8.0f),
// _minBias(-8.0f),
// _maxBias(8.0f),
// _maxPerturbation(0.75f),
// _excessFactor(1.0f),
// _disjointFactor(1.0f),
// _averageWeightDifferenceFactor(0.4f),
// _inputCountDifferenceFactor(1.5f),
// _outputCountDifferenceFactor(1.5f),
// _populationSize(40),
// _numElites(6)
{}

void Parameters::check(){
	if(numInputs==0 || numOutputs==0)
		throw std::runtime_error("numInputs and numOutputs must be >0.");

	if(mutateRateNewConnection<0 || mutateRateNewConnection>1)
		throw std::runtime_error("mutateRateNewConnection must be in [0,1].");
	if(mutateRateNewNode<0 || mutateRateNewNode>1)
		throw std::runtime_error("mutateRateNewNode must be in [0,1].");
	if(mutateRateWeightPerturbation<0 || mutateRateWeightPerturbation>1)
		throw std::runtime_error("mutateRateWeightPerturbation must be in [0,1].");
}

unsigned Parameters::connectionExists(unsigned from, unsigned to){ //checks both from-to and to-from
	unsigned c=0;
	for(unsigned size=allConnections.size(); c<size; ++c)
		if((allConnections[c]->pre == from && allConnections[c]->pos == to) || (allConnections[c]->pre == to && allConnections[c]->pos == from))
			break;
	if(allConnections.size()!=c)
		if(allConnections[c]->innovationNumber != c)
			throw std::runtime_error("allConnections is missing an innovation number");
	return c;
}

unsigned Parameters::addConnection(unsigned from, unsigned to){
	if(to < numInputs)
		throw std::runtime_error("'to' connection can't be an input connection.");
	// if(isOutputNode(from))
		// throw std::runtime_error("'from' connection can't be an output connection.");

	unsigned where = connectionExists(from,to);
	if(where==innovationNumber){ //connection doesn't exist
		auto new_connection = std::shared_ptr<BasicConnection>(new BasicConnection(from, to, innovationNumber));
		++innovationNumber;
		allConnections.push_back(new_connection);
	}

	return where;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Network::Network(bool _initialize, Parameters &_params): params(_params)
														,numInputs(params.numInputs)
														,numNonHidden(params.numInputs+params.numOutputs)
														,numHidden(0)
{
	if(_initialize) initialize();
}
Network::Network(Network&& other): params(other.params), numInputs(other.numInputs), numNonHidden(other.numNonHidden), numHidden(other.numHidden){
	std::swap(connections,other.connections);
	std::swap(nodes,other.nodes);
}
Network::Network(const Network& other): params(other.params), numInputs(other.numInputs), numNonHidden(other.numNonHidden), numHidden(other.numHidden){
	unsigned numOtherConnections = other.connections.size();
	unsigned numOtherNodes 		 = other.nodes.size();

	connections.reserve(numOtherConnections);
	nodes.reserve(numOtherNodes);

	for (unsigned i=0; i < numOtherConnections; ++i)
		connections.push_back(std::shared_ptr<Connection>(new Connection(*other.connections[i])));

	//is this what I want to make?? Copy or do as for the connections?
	for (unsigned i=0; i < numOtherNodes; ++i)
		nodes.push_back(other.nodes[i]);
}


void Network::initialize(){

	nodes.resize(numNonHidden, nullptr);
	for(unsigned i=0; i<numNonHidden; ++i)
		addNode(i);

	for(unsigned i=0; i<numInputs; ++i)
		for(unsigned o=numInputs; o<numNonHidden; ++o)
			addConnection(i, o);
}

bool Network::isOutputNode(unsigned n){
	return (n>=numInputs && n<numNonHidden);
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
unsigned Network::getNumOutputs() 		const { return numNonHidden - numInputs; }
unsigned Network::getNumNodes() 		const { return nodes.size(); }
unsigned Network::getNumConnections() 	const { return connections.size(); }

void Network::addNode(unsigned id, double bias){
	if(bias < params.minInitBias)
		bias = GA::generator(params.minInitBias, params.maxInitBias);
	if(id>=nodes.size())
		nodes.resize(id+1, nullptr);
	if(nodes[id] != nullptr)
		throw std::runtime_error("Trying to add a node to an already existing ID.");

	nodes[id].reset(new Node(bias));

	if(id >= numNonHidden) //not done in 'initialize'
		++numHidden;
}


void Network::addConnection(unsigned from, unsigned to){ //doesn't check if connection already exists
	if(to < numInputs)
		throw std::runtime_error("'to' connection can't be an input connection.");
	if(isOutputNode(from))
		throw std::runtime_error("'from' connection can't be an output connection.");

	double weight = GA::generator(params.minInitWeight,params.maxInitWeight);

	unsigned where = params.addConnection(from,to);
	auto new_connection = std::shared_ptr<Connection>(new Connection(from, to, where, weight, true));

	if(connections.size()==0 || where>connections.back()->innovationNumber)
		connections.push_back(new_connection);
	else{
		auto lower = lower_bound(connections.begin(), connections.end(), new_connection, Connection::compare);
		connections.insert(lower, new_connection);
	}

	if(from >= nodes.size())
		addNode(from);
	if(to >= nodes.size())
		addNode(to);

	// nodes[to]->addPreConnection(new_connection);
	// nodes[from]->addPosConnection();
}


Network Network::create(const GA::Evolver<Network>* ev){
	Network net(true,((Evolver*)ev)->params);
	return net;
}
Network Network::crossover(const Network& n1,const Network& n2){

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
	str << "N" << net.getNumNodes() << "C" << net.getNumConnections() << "\n";
	for(unsigned i=0, size=net.connections.size(); i<size; ++i)
		str << net.connections[i]->innovationNumber << ":" << net.connections[i]->pre << "-" << net.connections[i]->pos << "(" << net.connections[i]->getEnabled() << ")|";
	return str.str();
}

void Network::mutatePerturbWeight(){
	for (unsigned i=0, size = connections.size(); i < size; ++i)
		if (GA::generator() < params.mutateRateWeightPerturbation)
			connections[i]->addWeight(GA::generator(-params.mutateMaxPerturbation, params.mutateMaxPerturbation));

	for (unsigned i=0, size = nodes.size(); i < size; ++i)
		if (GA::generator() < params.mutateRateWeightPerturbation)
			nodes[i]->addBias(GA::generator(-params.mutateMaxPerturbation, params.mutateMaxPerturbation));
}
bool Network::mutateAddConnection(){
	unsigned nodes_size = nodes.size();
	unsigned to = GA::generator.getI(params.numInputs, nodes_size-1);

	//check if there are nodes 'to' is not connected to
	bool valid = false;
	for(unsigned n=0; n<nodes_size; ++n){
		unsigned check = (to+n)%nodes_size;
		if(check<numInputs) //inputs cannot be end connections
			continue;
		unsigned max_connections = ( isOutputNode(check) ? numInputs+numHidden : numInputs+numHidden-1 );
		if(countNode(check) < max_connections ){
			to = check;
			valid = true;
			break;
		}
	}
	if(!valid)
		return false;

	unsigned from = GA::generator.getI(0, nodes_size-1);

	while(from==to || isOutputNode(from) || connectionExists(from,to)!=connections.size())
		from = (from+1)%nodes_size;

	addConnection(from,to);

	return true;
}
void Network::mutateAddNode(){
	unsigned chosen = GA::generator(0, connections.size()-1);
	unsigned new_node = nodes.size();

	connections[chosen]->disable(); //may be re-enabled in crossover
	addConnection(connections[chosen]->pre,new_node);
	addConnection(new_node,connections[chosen]->pos);
}