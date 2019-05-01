
#include "NEAT_GA.hpp"
#include "Image.hpp"

double random(const NEAT::Network&, const GA::Evolver<NEAT::Network>*){ return GA::generator(); }

int main(){

	/////////////////////////////////////////////////////
	//EVOLUTION PART

	NEAT::Evolver ga(2);
	ga.params.numInputs  = 1;
	ga.params.numOutputs = 1;
	ga.params.mutateRateNewNode = 1;
	ga.params.mutateRateNewConnection = 1;
	ga.setEvaluate(random, GA::MINIMIZE);
	ga.maxGenerations = 3;
	ga.evolve(0,1,1);

	/////////////////////////////////////////////////////


	/////////////////////////////////////////////////////
	//PLOT BEST NETWORK
	getchar();

	NEAT::Image image(ga.getBest(),100,0.05);
	image.evolve(1000,5,0.1,0.6);

	//image.print();
	image.draw();
	image.save();
	image.wait();

	/////////////////////////////////////////////////////

	return 0;
}