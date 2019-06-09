
#include "NEAT_GA.hpp"

using namespace NEAT;

Evolver::Evolver(const std::string& file, const std::string& prefix): GA::Evolver<Network>(file, prefix), params(){
	fileNEATSettings(file, prefix);

	setCreate(Network::create);
	setCrossover(Network::crossover);
	setMutate(Network::mutate);
	setToString(Network::toString);
	setDissimilarity(Network::dissimilarity);
}

Evolver::Evolver(unsigned _populationSize, double speciation_threshold, bool verbose): 
											GA::Evolver<Network>(_populationSize, speciation_threshold, verbose), params(){
	setCreate(Network::create);
	setCrossover(Network::crossover);
	setMutate(Network::mutate);
	setToString(Network::toString);
	setDissimilarity(Network::dissimilarity);
}

void Evolver::fileNEATSettings(const std::string& file, const std::string& prefix){
	FileParser fp(file,true);

	if(!fp(prefix+"numInputs",params.numInputs))
		errorMsg("numInputs is a mandatory parameter.");
	if(!fp(prefix+"numOutputs",params.numOutputs))
		errorMsg("numOutputs is a mandatory parameter.");

	fp(prefix+"rateCrossoverAverage",params.rateCrossoverAverage);

	fp(prefix+"mutateRateNewConnection",params.mutateRateNewConnection);
	fp(prefix+"mutateRateNewNode",params.mutateRateNewNode);
	fp(prefix+"mutateRateWeightPerturbation",params.mutateRateWeightPerturbation);
	fp(prefix+"mutateRateWeightChange",params.mutateRateWeightChange);
	fp(prefix+"mutateRateToggle",params.mutateRateToggle);

	fp(prefix+"mutateMaxPerturbation",params.mutateMaxPerturbation);
	fp(prefix+"mutateMaxChange",params.mutateMaxChange);
	fp(prefix+"mutateRateEnableChance",params.mutateRateEnableChance);
	fp(prefix+"enableInExcessOrDisjoint",params.enableInExcessOrDisjoint);

	fp(prefix+"maxInitWeight",params.maxInitWeight);
	fp(prefix+"maxWeight",params.maxWeight);

	fp(prefix+"excessFactor",params.excessFactor);
	fp(prefix+"disjointFactor",params.disjointFactor);
	fp(prefix+"averageWeightDifferenceFactor",params.averageWeightDifferenceFactor);
}

GA::StopReason Evolver::start(){
	params.check();
	return GA::Evolver<Network>::start();
}
void Evolver::evolve(){
	params.check();
	GA::Evolver<Network>::evolve();
}