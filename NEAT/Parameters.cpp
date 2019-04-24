

#include "Parameters.hpp"

using namespace NEAT;

Parameters::Parameters():
	 numInputs(0)
	,numOutputs(0)

	,mutateRateNewConnection(0.2f)
	,mutateRateNewNode(0.1f)
	,mutateRateWeightPerturbation(0.2f)
	,mutateMaxPerturbation(0.75f)
//to keep fixed
	,minInitWeight(-1.0f)
	,maxInitWeight(1.0f)

//private
	,allConnections(0)
	,innovationNumber(0) // = allConnections.size()
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

Parameters::~Parameters(){ clear(); }
void Parameters::clear(){
	for(unsigned i=0, size=allConnections.size(); i<size; ++i)
		if(allConnections[i] != nullptr){
			delete allConnections[i];
			allConnections[i] = nullptr;
		}
}

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
	if(to < numInputs+1)
		throw std::runtime_error("'to' connection can't be an input connection.");
	if(from>=numInputs+1 && from<numInputs+numOutputs+1)
		throw std::runtime_error("'from' connection can't be an output connection.");

	unsigned where = connectionExists(from,to);

	#pragma omp critical
		if(where==innovationNumber){ //connection doesn't exist
			auto new_connection = new BasicConnection(from, to, innovationNumber);
			++innovationNumber;
			allConnections.push_back(new_connection);
		}

	return where;
}
unsigned Parameters::getInnovationNumber(){ return innovationNumber; }