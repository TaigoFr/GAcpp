
#include "Network.hpp"
#include "NEAT_GA.hpp"
#include "NodeTree.hpp"
#include "Tools.hpp"

using namespace NEAT;

#include <algorithm> //lower_bound
#include <cmath> 	 //abs
#include <fstream>	 //ofstream, ifstream

Network::Network(bool _initialize, Parameters &_params): params(&_params)
,numInputs(params->numInputs)
,numOutputs(params->numOutputs)
,numNonHidden(params->numInputs+params->numOutputs+1)
,numHidden(0)
,numNodes(numNonHidden)
{
	if(_initialize) initialize();
}
//1-line constructors not set to inline not to pollute .hpp
Network::Network(const std::string& file): params(nullptr){ read(file); }
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
	for(unsigned i=0, size=connections.size(); i<size; ++i){
		if(connections[i] != nullptr){
			delete connections[i];
			connections[i] = nullptr;
		}
	}
	connections.clear();
}


void Network::initialize(){
	for(unsigned i=0; i<numInputs+1; ++i){
		for(unsigned o=numInputs+1; o<numNonHidden; ++o)
			addConnection(i, o, true);
	}
}

bool Network::isOutputNode(unsigned n) const{
	return (n>=numInputs+1 && n<numNonHidden);
}
unsigned Network::countNode(unsigned n, bool countPreToo) const{
	unsigned count = 0;
	for(unsigned c=0, size=connections.size(); c<size; ++c){
		if(!connections[c]->getEnabled()) continue; //continue if disabled
		if((countPreToo && connections[c]->pre == n) || connections[c]->pos == n)
			++count;
	}
	return count;
}
unsigned Network::connectionExists(unsigned from, unsigned to, bool checkDisabled) const{ //checks both from-to and to-from
	unsigned c=0;
	for(unsigned size=connections.size(); c<size; ++c){
		if(!checkDisabled && !connections[c]->getEnabled()) continue; //continue if disabled
		if((connections[c]->pre == from && connections[c]->pos == to) || (connections[c]->pre == to && connections[c]->pos == from))
			break;
	}
	return c;
}
bool Network::isInPast(unsigned from, unsigned to) const{
	for(unsigned c=0, size=connections.size(); c<size; ++c){
		if(!connections[c]->getEnabled()) continue; //continue if disabled
		if( connections[c]->pos == to){
			if( connections[c]->pre == from || isInPast(from, connections[c]->pre)){
				// std::cout << "GOOD JOB TIAGO" << std::endl;
				return true;
			}
		}
	}
	return false;
}

double Network::getBias(unsigned node) const{
	for(unsigned c=0, size=connections.size(); c<size; ++c){
		if(!connections[c]->getEnabled()) continue; //continue if disabled
		if(connections[c]->pre == 0 && connections[c]->pos == node)
			return connections[c]->weight;
	}
	return 0.; //not found
}



void Network::addNode(unsigned id, bool isInit){
	if(id >= numNodes){ //not done in 'initialize'
		for(; numNodes<id+1;){
			++numNodes;
			++numHidden;
			addConnection(0, numNodes-1, isInit); //add bias
		}
	}
}


void Network::addConnection(unsigned from, unsigned to, bool isInit){
	//doesn't check for loops
	if(to < numInputs+1)
		errorMsg("'to' connection can't be an input connection.\n",toString(*this),"\n","from = ",from,"; to = ",to);
	if(isOutputNode(from))
		errorMsg("'from' connection can't be an output connection.\n",toString(*this),"\n","from = ",from,"; to = ",to);
	if(from==to)
		errorMsg("'from' = 'to'\n",toString(*this),"\n","from = ",from,"; to = ",to);

	double weight = (isInit ? GA::generator(-params->maxInitWeight,params->maxInitWeight) : GA::generator(-params->maxWeight,params->maxWeight));

	unsigned where = params->addConnection(from,to);
	//check if connection exists but is disabled
	unsigned ind = connectionExists(from,to,true);
	if(ind<connections.size()){
		if(connections[ind]->getEnabled()) //bias
			errorMsg("Trying to add already existing connection.");
		else
			connections[ind]->enable();
	}
	else{
		auto new_connection = new Connection(from, to, where, weight, true);

		if(connections.size()==0 || where>connections.back()->innovationNumber)
			connections.push_back(new_connection);
		else{
			auto lower = lower_bound(connections.begin(), connections.end(), new_connection, Connection::compare);
			connections.insert(lower, new_connection);
		}

		addNode(from, isInit);
		addNode(to  , isInit);
	}
}

void Network::addConnection(const Connection* con, bool enableChance){
	auto new_connection = new Connection(*con);
	connections.push_back(new_connection);

	unsigned max_node_id = std::max(con->pre, con->pos);
	if(max_node_id >= numNodes){
		numNodes  = max_node_id + 1;
		numHidden = numNodes - numNonHidden;
	}

	if(enableChance){ //if it came from parents where at least one had it disabled, there's a change it remains disabled and a change to re-enable (re-express gene)
		if(GA::generator() < params->mutateRateEnableChance)
			new_connection->enable();
		else
			new_connection->disable();
	}
}


Network Network::create(const GA::Evolver<Network>* ev){ return Network(true,((Evolver*)ev)->params); } //not set inline cause it's hard to include NEAT::Evolver in Network

Network Network::crossover(const Network& n1, const Network& n2, double fit1, double fit2){
	if(n1.params == nullptr || n2.params == nullptr || n1.params != n2.params)
		throw std::runtime_error("Parent's parameters not equal.");
	if(n1.connections.size()==0 || n2.connections.size()==0)
		throw std::runtime_error("Parents with empty connections.");

	unsigned iter1 = 0, inno1 = n1.connections[iter1]->innovationNumber, size1 = n1.connections.size();
	unsigned iter2 = 0, inno2 = n2.connections[iter2]->innovationNumber, size2 = n2.connections.size();

	unsigned last = n1.params->getInnovationNumber();

	Network child(false, *n1.params);

	while(inno1 < last && inno2 < last){
		if(inno1==inno2){
			// std::cout << "Matching gene " << inno1 << std::endl;
			child.addConnection(GA::generator() < 0.5 ? n1.connections[iter1] : n2.connections[iter2],
				(!n1.connections[iter1]->getEnabled() || !n2.connections[iter2]->getEnabled()) );
		}
		else if(inno1<inno2 && (fit1>fit2 || (fit1==fit2 && GA::generator()<0.5))){
			// std::cout << "Extra gene in N1: " << inno1 << std::endl;
			child.addConnection(n1.connections[iter1], n1.params->enableInExcessOrDisjoint);
		}
		else if(inno1>inno2 && (fit1<fit2 || (fit1==fit2 && GA::generator()<0.5))){
			// std::cout << "Extra gene in N2: " << inno2 << std::endl;
			child.addConnection(n2.connections[iter2], n2.params->enableInExcessOrDisjoint);
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

	// std::cout << "N1: " << toString(n1) << std::endl;
	// std::cout << "N2: " << toString(n2) << std::endl;
	// std::cout << "Child: " << toString(child) << std::endl;

	return n1;
}
void Network::mutate(Network& child, const GA::Evolver<Network>*){
	if(GA::generator() < child.params->mutateRateNewNode)
		child.mutateAddNode();

	if(GA::generator() < child.params->mutateRateNewConnection)
		child.mutateAddConnection();

	child.mutatePerturbWeight();

	return;
}
std::string	Network::toString(const Network& net){
	std::stringstream str;
	str << "N" << net.getNumNodes() << "H" << net.getNumHidden() << Connection::toString(net.connections);
	return str.str();
}
void Network::print() const{ ::print(toString(*this)); } //not set as inline not to include "Tools" directly in Network

void Network::mutatePerturbWeight(){
	for (unsigned i=0, size = connections.size(); i < size; ++i){
		if(!connections[i]->getEnabled()) continue; //continue if disabled
		if (GA::generator() < params->mutateRateWeightPerturbation)
			connections[i]->weight += GA::generator(-params->mutateMaxPerturbation, params->mutateMaxPerturbation);
	}
}
bool Network::mutateAddConnection(){
	unsigned to = GA::generator.getI(params->numInputs+1, numNodes-1);

	//check if there are nodes 'to' is not connected to
	bool valid = false;
	for(unsigned n=0; n<numNodes; ++n){
		unsigned check = (to+n)%numNodes;
		if(check<numInputs+1) //bias and inputs cannot be end connections
			continue;
		unsigned max_connections = numNodes - numOutputs - ( isOutputNode(check) ? 0 : 1 );
		if(countNode(check,true) < max_connections ){
			to = check;
			valid = true;
			break;
		}
	}
	if(!valid)
		return false;

	unsigned from = GA::generator.getI(0, numNodes-1); //0 is bias and supposedly already exists. Let like this for now, but it's redundant

	unsigned count=0;
	while(from==to || isOutputNode(from) || connectionExists(from,to,false)!=connections.size() || isInPast(to,from)){
		++count;
		if(count==numNodes) //no past connection to this too without a loop
			return false;
		from = (from+1)%numNodes;
	}

	addConnection(from, to, false);

	return true;
}
void Network::mutateAddNode(){
	unsigned chosen;
	Connection *con;
	do{
		chosen = GA::generator.getI(0, connections.size()-1);
		con = connections[chosen];
	}while(con->pre == 0 || !con->getEnabled()); //don't break connection with bias and don't add nodes on disabled connections

	unsigned new_node = numNodes;

	con->disable(); //may be re-enabled in crossover
	addConnection(con->pre ,new_node, false);
	addConnection(new_node ,con->pos, false); //bug fixed -> if previous added connection was inserted not in the end of 'connections',
									//what I had before "connections[chosen]" would be a different connection than before
}

VecD Network::evaluate(const VecD& input) const{ return NodeTree(*this).evaluate(input); } //not set inline cause it's hard to include NEAT::NodeTree in Network

double Network::getDissimilarity(const Network& n1, const Network& n2){
	if(n1.params == nullptr || n2.params == nullptr || n1.params != n2.params)
		throw std::runtime_error("Parent's parameters NULL or not equal.");
	if(n1.connections.size()==0 || n2.connections.size()==0)
		throw std::runtime_error("Parents with empty connections.");

	double totalWeightDifference 	= 0.;
	unsigned numMatching 			= 0;
	unsigned numDisjointConnections = 0;
	unsigned numExcessConnections 	= 0;

	unsigned iter1 = 0, inno1 = n1.connections[iter1]->innovationNumber, size1 = n1.connections.size();
	unsigned iter2 = 0, inno2 = n2.connections[iter2]->innovationNumber, size2 = n2.connections.size();

	unsigned last = n1.params->getInnovationNumber(); //this is 'next Innovation number', so doesn't exist in 'n1' or 'n2'


	while(inno1 < last && inno2 < last){
		if(inno1==inno2){
			totalWeightDifference += std::abs( n1.connections[iter1]->weight - n2.connections[iter2]->weight );
			++numMatching;
		}
		//if we're at a bias, don't count it for 'numExcessConnections' or 'numDisjointConnections'
		else if( (inno1<inno2 && n1.connections[iter1]->pre!=0) && (inno1>inno2 && n2.connections[iter2]->pre!=0) ){
			if(inno1 == last || inno2 == last)
				++numExcessConnections;
			else
				++numDisjointConnections;
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

	//subtract numNodes from numConnections so that bias is not taken into account to factor 1/N
	double normalization = 1. / std::max(n1.getNumConnections()-n1.getNumNodes(), n2.getNumConnections()-n2.getNumNodes());

	return 	n1.params->excessFactor 		* numExcessConnections 		* normalization +
	n1.params->disjointFactor 	* numDisjointConnections 	* normalization +
	(numMatching == 0 ? 0 : n1.params->averageWeightDifferenceFactor * totalWeightDifference / numMatching);
}


void Network::write(const std::string& path) const{
	std::string name = checkFileName(path,".txt");

	std::ofstream file;
	file.open(name, std::ios::trunc);

	for(unsigned i=0, size=connections.size(); i<size; ++i)
		file << connections[i]->toString(true,10) << std::endl;

	file.close();
}
void Network::read(const std::string& path){
	clear();

	std::ifstream file; 
	file.open(path, std::ios::in);

	unsigned firstOutput = 1;
	unsigned lastOutput  = 1;
	bool stopChecking = false;

	numNodes = 0;

	std::string line;
	while (std::getline(file, line)){
		auto new_connection = new Connection(line);
		connections.push_back(new_connection);
		if(!stopChecking){
			if(new_connection->pre == 0){
				if(firstOutput == 1) firstOutput = new_connection->pos; //only done once
				lastOutput = std::max(lastOutput, new_connection->pos); //done until all bias are seen
			}
			else
				stopChecking = true;
		}
		unsigned max_node_id = std::max(new_connection->pre, new_connection->pos);
		if(max_node_id >= numNodes)
			numNodes  = max_node_id + 1;
	}

	numInputs = firstOutput - 1;
	numOutputs = lastOutput - numInputs;
	numNonHidden = numInputs + numOutputs + 1;
	numHidden = numNodes - numNonHidden;
}