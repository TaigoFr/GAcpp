
#if !defined(_VEC_)
#error "This file should only be included through GA.hpp"
#endif

#include <assert.h>

template <typename Ind>
GA<Ind>::GA(unsigned _population_size): population_size(_population_size), population(new Population<Ind>(population_size)){
	assert(population_size >= 2);
}

template <typename Ind>
GA<Ind>::~GA(){	clear(); }

template <typename Ind>
void GA<Ind>::clear(){
	// printf("In GA::clear()\n");
	if(population!=nullptr){
		delete population;
		population = nullptr;
	}
}

template <typename Ind>
void GA<Ind>::evolve(unsigned num_generations, double mate_prob, double mutate_prob){
	population->create();

	population->updateFitness();
	population->print();
	printf("\n");
	VecD fitness = population->printFitness();
	printf("Average = %lf\nMax = %lf\n",fitness.sum()/population_size,fitness.max());

	for(unsigned gen = 0; gen < num_generations; ++gen)
		generation(mate_prob,mutate_prob);

	population->updateFitness();
	printf("\n");
	population->print();
	printf("\n");
	fitness = population->printFitness();
	printf("Average = %lf\nMax = %lf\n",fitness.sum()/population_size,fitness.max());
}

template <typename Ind>
unsigned GA<Ind>::mateSelection(int other){
	//assumes population.fitness() was already called

	double target = generator.getD(0.,population->getFitnessSum());

	unsigned parent = (other == 0 ? 1 : 0);
	if(other<0){//just give one parent
		//parent is always one ahead
		for (double sumSoFar = population->get(parent++)->getFitness(); parent < population_size; sumSoFar += population->get(parent++)->getFitness())
			if (sumSoFar >= target)
				break;
	}
	else{//give a parent !=other
		for (double sumSoFar = population->get(parent++)->getFitness(); parent < population_size; sumSoFar += population->get(parent++)->getFitness()){
			if(parent==other)
				continue;
			if(sumSoFar >= target)
				break;
		}
		if(parent==population_size) //this means that other=population_size-1
			--parent;
	}

	return --parent;
}


template <typename Ind>
void GA<Ind>::generation(double mate_prob, double mutate_prob){

	Population<Ind>* offspring = new Population<Ind>(population_size);

	population->updateFitness();
	for(unsigned ind = 0; ind<population_size; ++ind){
		unsigned parent1 = mateSelection();
		
		Ind *child;

		if(generator.getD() <= mate_prob){
			unsigned parent2 = mateSelection(parent1);
			// printf("Matting %d and %d\n",parent1,parent2);
			child = new Ind(mate(*population->get(parent1),*population->get(parent2)));
		}
		else
			child = new Ind(*population->get(parent1));

		if(generator.getD() <= mutate_prob)
			mutate(*child);

		offspring->set(ind,child);
	}

	clear();
	population = offspring;
}
