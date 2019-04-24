
#include "NEAT_GA.hpp"

double evaluate(const NEAT::Network& net){
	return net.evaluate(VecD(1,1))[0];
}

int main(){

	NEAT::Evolver ga(2);
	ga.params.numInputs  = 1;
	ga.params.numOutputs = 1;

	ga.params.mutateRateNewNode = 1;
	ga.params.mutateRateNewConnection = 1;
	
	ga.setEvaluate(evaluate, GA::MINIMIZE);
	// ga.numThreads = 0;
	ga.maxGenerations = 10;
	ga.evolve(0,0.5,1);

	// ga.printPopulation();

	return 0;
}