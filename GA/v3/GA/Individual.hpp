#pragma once

template <typename Ind>
class Individual{
public:
	inline Individual(): I(), evaluated(false), fitness_orig(0.), fitness(0.){}
	inline Individual(const Ind& i): I(i), evaluated(false), fitness_orig(0.), fitness(0.){}
	inline Individual(Ind&& i): I((Ind&&)i), evaluated(false), fitness_orig(0.), fitness(0.){}
	inline Individual(const Individual<Ind>& i): I(i.I), evaluated(i.evaluated), fitness_orig(i.fitness_orig), fitness(i.fitness){}
	inline Individual(Individual<Ind>&& i): I((Ind&&)(i.I)), evaluated(i.evaluated), fitness_orig(i.fitness_orig), fitness(i.fitness){}
	
	Ind I;
	bool evaluated;
	double fitness_orig;
	double fitness;
};

