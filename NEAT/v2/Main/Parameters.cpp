

#include "Parameters.hpp"
#include "Vec.hpp" //FORV
#include "Tools.hpp" //errorMsg

using namespace NEAT;

Parameters::Parameters():
//input, output
numInputs(0)
,numOutputs(0)

//to keep fixed
,rateCrossoverAverage(0.4)

,mutateRateNewConnection(0.2f)
,mutateRateNewNode(0.1f)
,mutateRateWeightPerturbation(0.2f)
,mutateRateWeightChange(0.0f)
,mutateRateToggle(0.05f)

,mutateMaxPerturbation(0.75f)
,mutateMaxChange(1.0f)
,mutateRateEnableChance(0.875)
,enableInExcessOrDisjoint(true)

,maxInitWeight(1.0f)
,maxWeight(8.0f)

// Species similarity weighting factors
,excessFactor(1.0f)
,disjointFactor(1.0f)
,averageWeightDifferenceFactor(0.4f)

//private
,allConnections(0)
,innovationNumber(0)
// innovationNumber = allConnections.size()
{}

Parameters::~Parameters(){ clear(); }
void Parameters::clear(){
	FORV(i, allConnections.size()){
		if(allConnections[i] != nullptr){
			delete allConnections[i];
			allConnections[i] = nullptr;
		}
	}
}

void Parameters::check() const{
	if(numInputs==0 || numOutputs==0)
		errorMsg("numInputs and numOutputs must be >0.");

	if(rateCrossoverAverage<0 || rateCrossoverAverage>1)
		errorMsg("rateCrossoverAverage must be in [0,1].");
	if(mutateRateNewConnection<0 || mutateRateNewConnection>1)
		errorMsg("mutateRateNewConnection must be in [0,1].");
	if(mutateRateNewNode<0 || mutateRateNewNode>1)
		errorMsg("mutateRateNewNode must be in [0,1].");
	if(mutateRateWeightPerturbation<0 || mutateRateWeightPerturbation>1)
		errorMsg("mutateRateWeightPerturbation must be in [0,1].");
	if(mutateRateWeightChange<0 || mutateRateWeightChange>1)
		errorMsg("mutateRateWeightChange must be in [0,1].");
	if(mutateRateToggle<0 || mutateRateToggle>1)
		errorMsg("mutateRateToggle must be in [0,1].");

}

unsigned Parameters::connectionExists(unsigned from, unsigned to) const{ //checks both from-to and to-from
	unsigned c=0;
	for(unsigned size=allConnections.size(); c<size; ++c){
		if((allConnections[c]->pre == from && allConnections[c]->pos == to) || (allConnections[c]->pre == to && allConnections[c]->pos == from))
			break;
	}
	if(allConnections.size()!=c){
		if(allConnections[c]->innovationNumber != c)
			errorMsg("allConnections is missing an innovation number");
	}
	return c;
}

unsigned Parameters::addConnection(unsigned from, unsigned to){
	if(to < numInputs+1)
		errorMsg("'to' connection can't be an input connection.");
	if(from>=numInputs+1 && from<numInputs+numOutputs+1)
		errorMsg("'from' connection can't be an output connection.");

	unsigned where = connectionExists(from,to);

	#pragma omp critical
		if(where==innovationNumber){ //connection doesn't exist
			auto new_connection = new BasicConnection(from, to, innovationNumber);
		++innovationNumber;
		allConnections.push_back(new_connection);
	}

	return where;
}
