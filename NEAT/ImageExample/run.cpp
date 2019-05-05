
#include "NEAT_GA.hpp"
#include "Image.hpp"
#include "Tools.hpp"

double random(const NEAT::Network&, const GA::Evolver<NEAT::Network>*){ return GA::generator(); }

int main(){

	/////////////////////////////////////////////////////
	//EVOLUTION PART

	NEAT::Evolver ga(2);
	ga.params.numInputs  = 5;
	ga.params.numOutputs = 5;
	ga.params.mutateRateNewNode = 1;
	ga.params.mutateRateNewConnection = 1;
	ga.setEvaluate(random, GA::MINIMIZE);
	ga.maxGenerations = 6;
	ga.evolve(0,1,1);

	/////////////////////////////////////////////////////


	/////////////////////////////////////////////////////
	//PLOT BEST NETWORK

	//////////////////////////
	//testing write and read
	NEAT::Network *best = &ga.getBest();
	print(NEAT::Network::toString(*best));
	best->write("NEAT_Image_best.txt");

	NEAT::Network net("NEAT_Image_best.txt");
	net.print();
	//////////////////////////

	NEAT::Image image(net,100);
	image.evolve(2000,5,0.2,0.5);

	image.print();
	image.draw();
	// image.save();
	image.wait();

	/////////////////////////////////////////////////////

	return 0;
}