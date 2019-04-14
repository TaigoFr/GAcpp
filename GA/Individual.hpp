#pragma once

template <typename Ind>
class Individual{
public:
	Individual(): I(), fitness(0.), fitness_norm(0.){}
	Individual(const Ind& i): I(i), fitness(0.), fitness_norm(0.){}
	
	Ind I;
	double fitness;
	double fitness_norm;
};

