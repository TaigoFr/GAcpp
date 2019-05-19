#pragma once

#include "Vec.hpp"
#include "Individual.hpp"


////TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <cstdio>

template <typename Ind>
class Population{
public:
	Population(unsigned population_size): pop(population_size,(Ind*)nullptr){}

	~Population(){ clear(); }

	void clear(){
		// printf("In Population::clear()\n");
		for(unsigned i=0; i<pop.size(); ++i){
			if(pop[i]!=nullptr){
				delete pop[i];
				pop[i] = nullptr;
			}
		}
	}

	void create()
	{ for(unsigned i=0; i<pop.size(); ++i) pop[i] = new Ind(Individual::create<Ind>()); }

	void updateFitness(){
		fitnessSum = 0.;
		for(unsigned i=0; i<pop.size(); ++i){
			if(pop[i]!=nullptr){
				double fitness = evaluate(*pop[i]);
				fitnessSum += fitness;
				pop[i]->setFitness(fitness);
			}
		}

		static int count = 0;
		if((count++)%10000==0)
			printf("Average Fitness = %lf %d\n",fitnessSum/pop.size(),count);
	}

	VecD printFitness(){
		VecD fitness(pop.size());
		for(unsigned i=0; i<pop.size(); ++i)
			fitness[i] = pop[i]->getFitness();
		fitness.print();
		return fitness;
	}

	double getFitnessSum(){ return fitnessSum; }

	void print()
	{ for(unsigned i=0; i<pop.size(); ++i) if(pop[i]!=nullptr) Individual::print(*pop[i]);	}

	unsigned size(){ return pop.size(); }
	Ind* get(unsigned i){ return pop[i]; }
	void set(unsigned i, Ind* I){ pop[i] = I; }

private:
	Vec<Ind*> pop;
	double fitnessSum;
};