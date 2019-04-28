
#include "NEAT_GA.hpp"

using namespace NEAT;

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

unsigned Evolver::selectParent(const VecD& fitness_cumulative, int other){
	if(other<0)
		return GA::Evolver<Network>::selectParent(fitness_cumulative, other);

	// std::cout << "[NEAT::selectParent]" << std::endl;

	VecD compatible_fitness_comulative;
	Vec<unsigned>   compatible_indices;

	double compatible_fitnessSum_norm = 0.;

	unsigned minDissimilarityIndex = 0;
	double   minDissimilarity = Network::getDissimilarity(population->pop[other]->I, population->pop[minDissimilarityIndex]->I);

	for(unsigned i=0; i<populationSize; ++i) {
		if ((int)i==other) continue;

		double dissimilarity = Network::getDissimilarity(population->pop[other]->I, population->pop[i]->I);

		if(dissimilarity < minDissimilarity){
			minDissimilarityIndex = i;
			minDissimilarity = dissimilarity;
		}

		if (dissimilarity < params.speciationTolerance) {
			double fitness_norm = std::max(0., population->pop[i]->fitness_norm - dissimilarity * params.preferSimilarFactor);

			compatible_fitnessSum_norm += fitness_norm;
			compatible_fitness_comulative.push_back(compatible_fitnessSum_norm);
			compatible_indices.push_back(i);
		}
	}

	unsigned out;

	// If there are no compatible genes, use least disimilar
	if (compatible_fitness_comulative.size() == 0)
		out = minDissimilarityIndex;
	else{
		unsigned chosenIndex = GA::Evolver<Network>::selectParent(compatible_fitness_comulative, -1);
		out = compatible_indices[chosenIndex];
	}

	// std::cout << "Parent1: " << toString(population->pop[other]->I) << std::endl;
	// std::cout << "Parent2: " << toString(population->pop[out]->I) << std::endl;

	return out;
}