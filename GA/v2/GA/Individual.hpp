#pragma once

template <typename Ind>
class Individual{
public:
	Individual(): I(), evaluated(false), fitness(0.), fitness_norm(0.){}
	Individual(const Ind& i): I(i), evaluated(false), fitness(0.), fitness_norm(0.){}
	Individual(const Individual<Ind>& i): I(i.I), evaluated(i.evaluated), fitness(i.fitness), fitness_norm(i.fitness_norm){}
	
	Ind I;
	bool evaluated;
	double fitness;
	double fitness_norm;
};

