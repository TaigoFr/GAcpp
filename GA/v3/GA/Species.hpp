#pragma once

#include "Vec.hpp"
#include "Individual.hpp"

template <typename Ind>
class Species{
public:
	inline Species(): pop(), fitness_cumulative(), fitnessSum_orig(0.), fitnessSum(0.), bestRank(0){}

	~Species(){ clear(); }

	void clear(){
		// printf("In Species::clear()\n");
		for(unsigned i=0; i<pop.size(); ++i){
			if(pop[i]!=nullptr){
				// printf("Deleting Individual %d\n",i);
				delete pop[i];
				pop[i] = nullptr;
			}
		}
	}

	inline unsigned size(){ return pop.size(); }

	inline void addIndividual(Individual<Ind>* I){
		pop.push_back(I);
		fitness_cumulative.push_back(0.); //assumes individuals are not yet evaluated (all evaluations are done once per generation in 'updateFitness')
	}

	inline const Ind&  	getBest() 		 	 	const{ return pop[bestRank]->I; }
	inline double 		getBestFitness() 		const{ return pop[bestRank]->fitness_orig; }
	inline double 		getBestFitnessEvolver() const{ return pop[bestRank]->fitness; }

	static bool compareIndividuals(Individual<Ind>* left, Individual<Ind>* right)
	{ return (left==nullptr || right==nullptr) ? false : ((left->fitness) > (right->fitness)); }

/*
	VecD printFitness() const{
		VecD fitness_orig(pop.size());
		for(unsigned i=0; i<pop.size(); ++i)
			fitness_orig[i] = (pop[i]==nullptr ? 0 : pop[i]->fitness_orig);
		fitness_orig.print();
		printf("Average = %lf\nBest = %lf\n",fitness_orig.sum()/pop.size(),fitness_orig.min());
		return fitness_orig;
	}
*/

	Vec<Individual<Ind>*> pop;
	VecD fitness_cumulative;
	double fitnessSum_orig;
	double fitnessSum;
	unsigned bestRank;
};