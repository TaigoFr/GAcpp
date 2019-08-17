
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
		// fitness += fabs(value - outputs[i]);
	}

	return fitness;
	// return pow(fitness, 2);
	// return fitness*(8.-fitness);
}

double XOR_evolve(){

	NEAT::Evolver ga(40, 3, false);
	ga.params.numInputs  = 2;
	ga.params.numOutputs = 1;
	ga.setEvaluate(XOR, GA::MINIMIZE);

	ga.addCriteria(GA::StopReason::MaxGenerations, 			{50});			//stop after 100 generations
	ga.addCriteria(GA::StopReason::AverageRelativeStalled, 	{0.01, 15});	//stop after 15 generations with less than 1% improvement in the average fitness

	ga.survivalThreshold = 0.4;
	ga.evolve(10,0.9,1);

	const NEAT::Network *best = &ga.getBest();
	return 25*(4-XOR(*best, nullptr));
}

int main(){

	Clock::statsV(1000,XOR_evolve);

	return 0;
}