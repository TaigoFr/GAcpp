
#include "NEAT_GA.hpp"

double evaluate(const NEAT::Network&){

	return 0.;
}

int main(){

	NEAT::Evolver ga(2);
	ga.params.numInputs  = 1;
	ga.params.numOutputs = 1;

	ga.params.mutateRateNewNode = 1;
	ga.params.mutateRateNewConnection = 1;
	
	ga.setEvaluate(evaluate, GA::MINIMIZE);
	ga.numThreads = 0;
	ga.evolve(0,0,1);

	// ga.printPopulation();

	return 0;
}