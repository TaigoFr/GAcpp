
#include "NEAT_GA.hpp"
#include "Tools.hpp"

double XOR(const NEAT::Network& net, const GA::Evolver<NEAT::Network>*){
	static float inputs[][2] = {{ 0.0f, 0.0f },
								{ 0.0f, 1.0f },
								{ 1.0f, 0.0f },
								{ 1.0f, 1.0f }};

	static float outputs[] = { 0.0f, 1.0f, 1.0f, 0.0f };

	double fitness = 0.;
	for(unsigned i=0; i<4; ++i){
		double value = net.evaluate(VecD(2,inputs[i][0],inputs[i][1]))[0];
		fitness += pow(value - outputs[i], 2);
	}

	return fitness;
	// return fitness*(8.-fitness);
}

void showOutput(const NEAT::Network& net) {
	static float inputs[][2] = {
		{ 0.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f }
	};

	for(unsigned i=0; i<4; ++i){
		double output = net.evaluate(VecD(2,inputs[i][0],inputs[i][1]))[0];
		print("Output @ (",inputs[i][0],", ",inputs[i][1],"): ",output);
	}
}

int main(){

	NEAT::Evolver ga(40);
	ga.params.numInputs  = 2;
	ga.params.numOutputs = 1;
	ga.setEvaluate(XOR, GA::MINIMIZE);
	ga.maxGenerations = 100;
	ga.evolve(10,0.9,1);

	NEAT::Network *best = &ga.getBest();
	print(NEAT::Network::toString(*best));
	print("Best Fitness = ",(4.-ga.getBestFitness())*25.);

	showOutput(*best);
	best->print();
	// best->write("XOR_best.txt");

	// NEAT::Network net("XOR_best.txt");
	// net.print();

	return 0;
}