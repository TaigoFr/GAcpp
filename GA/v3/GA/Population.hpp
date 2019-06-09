#pragma once

#include "Species.hpp"

#include <iostream>

template <typename Ind>
class Population{
public:
	inline Population(double _speciation_threshold = -1.): //negative means no speciation
	species()
	, last_id(0)
	, speciation_threshold(_speciation_threshold)
	, bestSpecies(0)
	, totalFitnessSum_orig(0.)
	, totalFitnessSum(0.)
	{ if(speciation_threshold < 0.) addSpecies(1); }

	~Population(){ clear(); }

	void clear(){
		// printf("In Population::clear()\n");
		FORV(i,species.size()){
			if(species[i]!=nullptr){
				// printf("Deleting Individual %d\n",i);
				delete species[i];
				species[i] = nullptr;
			}
		}
		// printf("Out Population::clear()\n");
	}

	//can't be called if species has 0 size
	inline const Ind&  	getBest() 		 	 	const{ return species[bestSpecies]->getBest(); }
	inline double 		getBestFitness() 		const{ return species[bestSpecies]->getBestFitness(); }
	inline double 		getBestFitnessEvolver() const{ return species[bestSpecies]->getBestFitnessEvolver(); }

	inline unsigned numSpecies() const{ return species.size(); }

	//returns species with 'id' (either of the old one, if found, or of the new one, if one was created)
	inline unsigned addSpecies(unsigned id, unsigned age=0, unsigned ageLastImprovement=0, double bestFitnessEver=0.){
		if(id <= last_id){
			unsigned s;
			for(s=0; s<species.size(); ++s)
				if( species[s]->id == id ) break;
			if(s < species.size() )
				return s; //species already exists
		}
		else
			last_id = id;

		//add species - it doesn't exist yet
		species.push_back( new Species<Ind>(id, age, ageLastImprovement, bestFitnessEver) );
		return species.size()-1;
	}

	static bool compareSpecies(const Species<Ind>* left, const Species<Ind>* right)
	{ return (left==nullptr || right==nullptr) ? false : ((left->getBestFitnessEvolver()) > (right->getBestFitnessEvolver())); }

	void print() const{
		FORV(s,numSpecies()){
			if(speciation_threshold >= 0. )
				std::cout << "Species " << species[s]->id << " (" << species[s]->age << " gens; last improv: " 
					<< species[s]->gensNoImprovement << " gens ago)" << std::endl;
			FORV(i,species[s]->size()){
				Individual<Ind>* ind = species[s]->pop[i];
				if(ind!=nullptr) std::cout << toString(ind->I) << " (F=" << ind->fitness_orig << "\t|" << ind->fitness << ")" << std::endl;	
			}
		}
	}

	void updateFitnessCumulative(){ FORV(i,numSpecies()) species[i]->updateFitnessCumulative(); }

	//returns a vector with cumulative sums of the sizes of each species
	VecD speciesCumulativeSum(){
		VecD out(numSpecies(),0.);
		out[0] = species[0]->size();
		FORV(i,1,numSpecies()) out[i] = out[i-1] + species[i]->size();
		return out;
	}

	void printFitness() const{
		FORV(s,numSpecies()){
			printf("\tSpecies ID=%d; Best=%.3e; Average=%.3e; Size=%d\n",
					species[s]->id, species[s]->getBestFitness(), 
					species[s]->fitnessSum_orig/species[s]->size(),
					species[s]->size());
			species[s]->printFitness();
		}
	}

	Vec<Species<Ind>*> species;
	unsigned last_id;
	const double speciation_threshold;
	unsigned bestSpecies;
	unsigned worseSpecies; //for obliteration every obliterationRate generations
	double totalFitnessSum_orig; //used just for stopping criteria
	double totalFitnessSum; //used to calculate expected offspring

	double oldBest;
	double oldAverage;
};