#pragma once

#include "Vec.hpp"
#include "Individual.hpp"


////TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <cstdio>

template <typename Ind>
class Population{
public:
	inline Population(unsigned populationSize): pop(populationSize,(Individual<Ind>*)nullptr), fitness_cumulative(populationSize,0.), fitnessSum(0.), fitnessSum_norm(0.), bestRank(0){}

	~Population(){ clear(); }

	void clear(){
		// printf("In Population::clear()\n");
		for(unsigned i=0; i<pop.size(); ++i){
			if(pop[i]!=nullptr){
				// printf("Deleting Individual %d\n",i);
				delete pop[i];
				pop[i] = nullptr;
			}
		}
	}

	VecD printFitness() const{
		VecD fitness(pop.size());
		for(unsigned i=0; i<pop.size(); ++i)
			fitness[i] = (pop[i]==nullptr ? 0 : pop[i]->fitness);
		fitness.print();
		printf("Average = %lf\nBest = %lf\n",fitness.sum()/pop.size(),fitness.min());
		return fitness;
	}

	Vec<Individual<Ind>*> pop;
	VecD fitness_cumulative;
	double fitnessSum;
	double fitnessSum_norm;
	unsigned bestRank;
};