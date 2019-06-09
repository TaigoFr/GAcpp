

#include "Parameters.hpp"

#include <iostream> //runtime_error

using namespace NEAT;

Parameters::Parameters():
//input, output
numInputs(0)
,numOutputs(0)

//to keep fixed
,mutateRateNewConnection(0.2f)
,mutateRateNewNode(0.1f)
,mutateRateWeightPerturbation(0.2f)
,mutateRateWeightChange(0.0f)
,mutateRateToggle(0.05f)
,mutateMaxPerturbation(0.75f)
,mutateMaxChange(1.0f)
,mutateRateEnableChance(0.875)
,enableInExcessOrDisjoint(true)

,maxInitWeight(-1.0f)
,maxWeight(8.0f)

	// Species similarity weighting factors
,excessFactor(1.0f)
,disjointFactor(1.0f)
,averageWeightDifferenceFactor(0.4f)
,inputCountDifferenceFactor(1.5f)
,outputCountDifferenceFactor(1.5f)

// ,speciationTolerance(2.2f)
// ,preferSimilarFactor(0.05f)

//private
,allConnections(0)
,innovationNumber(0)
// innovationNumber = allConnections.size()
{}

Parameters::~Parameters(){ clear(); }
void Parameters::clear(){
	for(unsigned i=0, size=allConnections.size(); i<size; ++i)
		if(allConnections[i] != nullptr){
			delete allConnections[i];
			allConnections[i] = nullptr;
		}
	}

	void Parameters::check() const{
		if(numInputs==0 || numOutputs==0)
			throw std::runtime_error("numInputs and numOutputs must be >0.");

		if(mutateRateNewConnection<0 || mutateRateNewConnection>1)
			throw std::runtime_error("mutateRateNewConnection must be in [0,1].");
		if(mutateRateNewNode<0 || mutateRateNewNode>1)
			throw std::runtime_error("mutateRateNewNode must be in [0,1].");
		if(mutateRateWeightPerturbation<0 || mutateRateWeightPerturbation>1)
			throw std::runtime_error("mutateRateWeightPerturbation must be in [0,1].");
	}

unsigned Parameters::connectionExists(unsigned from, unsigned to) const{ //checks both from-to and to-from
	unsigned c=0;
	for(unsigned size=allConnections.size(); c<size; ++c){
		if((allConnections[c]->pre == from && allConnections[c]->pos == to) || (allConnections[c]->pre == to && allConnections[c]->pos == from))
			break;
	}
	if(allConnections.size()!=c){
		if(allConnections[c]->innovationNumber != c)
			throw std::runtime_error("allConnections is missing an innovation number");
	}
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
