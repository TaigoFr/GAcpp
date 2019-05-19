#pragma once

template <typename Ind>
class Individual{
public:
	inline Individual(): I(), evaluated(false), fitness(0.), fitness_norm(0.){}
	inline Individual(const Ind& i): I(i), evaluated(false), fitness(0.), fitness_norm(0.){}
	inline Individual(Ind&& i): I((Ind&&)i), evaluated(false), fitness(0.), fitness_norm(0.){}
	inline Individual(const Individual<Ind>& i): I(i.I), evaluated(i.evaluated), fitness(i.fitness), fitness_norm(i.fitness_norm){}
	inline Individual(Individual<Ind>&& i): I((Ind&&)(i.I)), evaluated(i.evaluated), fitness(i.fitness), fitness_norm(i.fitness_norm){}
	
	Ind I;
	bool evaluated;
	double fitness;
	double fitness_norm;
};

