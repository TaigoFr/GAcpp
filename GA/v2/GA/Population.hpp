#pragma once

#include "Vec.hpp"
#include "Individual.hpp"


////TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <cstdio>

template <typename Ind>
class Population{
public:
	inline Population(unsigned populationSize): pop(populationSize,(Individual<Ind>*)nullptr), fitness_cumulative(populationSize,0.), fitnessSum_orig(0.), fitnessSum(0.), bestRank(0){}

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

	VecD printfitness_orig() const{
		VecD fitness_orig(pop.size());
		for(unsigned i=0; i<pop.size(); ++i)
			fitness_orig[i] = (pop[i]==nullptr ? 0 : pop[i]->fitness_orig);
		fitness_orig.print();
		printf("Average = %lf\nBest = %lf\n",fitness_orig.sum()/pop.size(),fitness_orig.min());
		return fitness_orig;
	}

	Vec<Individual<Ind>*> pop;
	VecD fitness_cumulative;
	double fitnessSum_orig;
	double fitnessSum;
	unsigned bestRank;
};