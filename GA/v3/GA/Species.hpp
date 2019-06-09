#pragma once

#include "Vec.hpp"
#include "Individual.hpp"

template <typename Ind>
class Species{
public:
	inline Species(unsigned _id, unsigned _age=0, unsigned _gensNoImprovement=0, double _bestFitnessEver_orig=0.): 
					pop(), fitnessCumulative(), fitnessSum_orig(0.), fitnessSum(0.), bestRank(0),
					bestFitness(0.), bestFitnessEver_orig(_bestFitnessEver_orig),
					id(_id), age(_age), gensNoImprovement(_gensNoImprovement){}

	~Species(){ clear(); }

	void clear(){
		// printf("In Species::clear()\n");
		FORV(i,pop.size()) clear(i);
	}
	inline void clear(unsigned i){
		if(pop[i]!=nullptr){
			// printf("Deleting Individual %d\n",i);
			delete pop[i];
			pop[i] = nullptr;
		}
	}

	inline unsigned size() const{ return pop.size(); }

	inline void addIndividual(Individual<Ind>* I){ pop.push_back(I); }

	//can't be called if pop has 0 size
	inline const Ind&  	getBest() 		 	 	const{ return pop[bestRank]->I; }
	inline double 		getBestFitness() 		const{ return pop[bestRank]->fitness_orig; }
	inline double 		getBestFitnessEvolver() const{ return bestFitness * size(); } //*size to reverse adjusted fitness

	static bool compareIndividuals(const Individual<Ind>* left, const Individual<Ind>* right)
	//no need to correct adjusted fitness, as this is only used for ordering inside a given species
	{ return (left==nullptr || right==nullptr) ? false : ((left->fitness) > (right->fitness)); }

	void updateFitnessCumulative(){
		fitnessCumulative.resize(size(),0.);
		fitnessCumulative[0] = pop[0]->fitness;
		FORV(i,1,size()) fitnessCumulative[i] = fitnessCumulative[i-1] + pop[i]->fitness;
	}


	VecD printFitness() const{
		VecD fitness(pop.size());
		FORV(i,pop.size()) fitness[i] = (pop[i]==nullptr ? 0 : pop[i]->fitness);
		// fitness.print();
		printf("Average = %lf\nBest = %lf\n",fitness.sum()/pop.size(),fitness.max());
		return fitness;
	}


	Vec<Individual<Ind>*> pop;
	VecD 	 fitnessCumulative;
	double   fitnessSum_orig;
	double   fitnessSum;
	unsigned bestRank;
	double   bestFitness; //redundant with bestRank, but needed because nth_element during 'generation' loses track of bestRank
	double   bestFitnessEver_orig; //has to be orig cause non-orig is normalized in different ways between generations

	unsigned id;
	unsigned age;
	unsigned gensNoImprovement;
};