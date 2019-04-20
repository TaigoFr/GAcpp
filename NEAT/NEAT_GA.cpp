
#include "NEAT_GA.hpp"

using namespace NEAT;

#include <cassert>

Evolver::Evolver(unsigned _populationSize): GA::Evolver<Network>(_populationSize), params(){
	setCreate(Network::create);
	setCrossover(Network::crossover);
	setMutate(Network::mutate);
	setToString(Network::toString);
}

void Evolver::evolve(unsigned eliteCount, double crossoverProb, double mutateProb){
	params.check();
	GA::Evolver<Network>::evolve(eliteCount, crossoverProb, mutateProb);
}