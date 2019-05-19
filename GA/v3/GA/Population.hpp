#pragma once

#include "Species.hpp"

////TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <cstdio>

template <typename Ind>
class Population{
public:
	inline Population(double _speciation_threshold = -1.): //negative means no speciation
	species(1)
	, speciation_threshold(_speciation_threshold)
	, bestSpecies(0)
	, totalFitnessSum(0.)
	{ species[0] = new Species<Ind>(); }

	~Population(){ clear(); }

	void clear(){
		// printf("In Population::clear()\n");
		for(unsigned i=0; i<species.size(); ++i){
			if(species[i]!=nullptr){
				// printf("Deleting Individual %d\n",i);
				delete species[i];
				species[i] = nullptr;
			}
		}
	}

	inline const Ind&  	getBest() 		 	 	const{ return species[bestSpecies]->getBest(); }
	inline double 		getBestFitness() 		const{ return species[bestSpecies]->getBestFitness(); }
	inline double 		getBestFitnessEvolver() const{ return species[bestSpecies]->getBestFitnessEvolver(); }

	inline unsigned numSpecies(){ return species.size(); }

	static bool compareSpecies(Species<Ind>* left, Species<Ind>* right)
	{ return (left==nullptr || right==nullptr) ? false : ((left->getBestFitness()) > (right->getBestFitness())); }

	Vec<Species<Ind>*> species;
	const double speciation_threshold;
	unsigned bestSpecies;
	double totalFitnessSum; //used just for stopping criteria
};