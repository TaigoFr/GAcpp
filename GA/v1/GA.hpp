#ifndef _GA_
#define _GA_

#include "Random.hpp"
#include "Population.hpp"

template <typename Ind>
class GA{
public:
	GA(unsigned _population_size);
	~GA();

	void clear();

	void evolve(unsigned num_generations, double mate_prob, double mutate_prob);

private:
	Random generator;
	unsigned population_size;
	Population<Ind>* population;

	unsigned mateSelection(int other = -1);

	void generation(double mate_prob, double mutate_prob);
};

#include "GA.impl.hpp"

#endif //_GA_
