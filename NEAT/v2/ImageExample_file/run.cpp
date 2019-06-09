
#include "NEAT_GA.hpp"
#include "Image.hpp"
#include "Tools.hpp"

double random(const NEAT::Network&, const GA::Evolver<NEAT::Network>*){ return GA::generator(); }

int main(){

	/////////////////////////////////////////////////////
	//EVOLUTION PART

	NEAT::Evolver ga("params.txt");
	ga.setEvaluate(random, GA::MINIMIZE);
	ga.evolve(0,1,1);

	/////////////////////////////////////////////////////

	/////////////////////////////////////////////////////
	//PLOT BEST NETWORK

	const NEAT::Network *best = &ga.getBest();
	best->print();

	NEAT::Image image(*best,"params_image.txt");
	image.evolve();

	image.print();
	image.draw();
	// image.save();
	image.wait();

	/////////////////////////////////////////////////////

	return 0;
}