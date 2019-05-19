
#if !defined(_EVOLVER_)
#error "This file should only be included through Evolver.hpp"
#endif

#include <assert.h>

#include <omp.h>

template <typename Ind>
Evolver<Ind>::Evolver(unsigned _populationSize, double _speciation_threshold, bool _verbose):
maxGenerations(100)
,tolStallAverage(1.e-6)
,averageStallMax(15)
,tolStallBest(1.e-8)
,bestStallMax(15)
,numThreads(1)

,populationSize(_populationSize)
,population(new Population<Ind>(_speciation_threshold))

,create(nullptr)
,crossover(nullptr)
,mutate(mutate_default)
,evaluate(nullptr)
,toString(toString_default)
,dissimilarity(dissimilarity_default)

,C("",Clock::stop)
,obj(NONE)
,generationStep(0)
,verbose(_verbose)
{
	assert(populationSize >= 2);
	C.setVerbose(false);
}

template <typename Ind>
Evolver<Ind>::~Evolver(){	clear(); }

template <typename Ind>
void Evolver<Ind>::clear(){
	// printf("In Evolver::clear()\n");
	if(population!=nullptr){
		delete population;
		population = nullptr;
	}
}

template <typename Ind>
inline const Ind&  	Evolver<Ind>::getBest() 		const{ return population->getBest(); }
template <typename Ind>
inline double 		Evolver<Ind>::getBestFitness() 	const{ return population->getBestFitness(); }


template <typename Ind>
StopReason Evolver<Ind>::start(unsigned eliteCount, double crossoverProb, double mutateProb){
	if(verbose){
		std::cout << "****************************************"	<< std::endl;
		std::cout << "*            Evolver started           *" << std::endl;
		std::cout << "****************************************" << std::endl;
		std::cout << "population:\t\t" 		<< populationSize 		<< std::endl;
		std::cout << "eliteCount:\t\t" 		<< eliteCount 			<< std::endl;
		std::cout << "crossoverProb:\t\t" 	<< crossoverProb 		<< std::endl;
		std::cout << "mutationProb:\t\t" 	<< mutateProb 			<< std::endl;
		std::cout << "speciation:\t\t" 		<< (population->speciation_threshold < 0 ? 0 : population->speciation_threshold) << std::endl;
	}

	checkSettings(eliteCount, crossoverProb, mutateProb);

	unsigned num;
	#pragma omp parallel
	{
		#pragma omp master
		num = omp_get_num_threads();
	}
	if(num!=numThreads && numThreads!=0)
		omp_set_num_threads(numThreads);

	if(verbose){
		std::cout << "OMP_NUM_THREADS:\t" 	<< (numThreads ? numThreads : num) << std::endl;
		std::cout << "****************************************" << std::endl;
	}

	C.R();
	initiatePopulation();
	generationStep = 0;
	
	StopReason stop = updateFitness();
	printGen();

	return stop;
}

template <typename Ind>
StopReason Evolver<Ind>::step(unsigned eliteCount, double crossoverProb, double mutateProb){
	generation(eliteCount, crossoverProb, mutateProb);
	++generationStep;

	StopReason stop = updateFitness();
	printGen();

	return stop;
}

template <typename Ind>
void Evolver<Ind>::finish(StopReason stop) const{
	if(verbose) printStopReason(stop);

	if(verbose){
		C.setVerbose(true);
		double time = C("Total time: ","s\n");
		C.setVerbose(false);
		std::cout << "Time/Generation: " << time/generationStep << "s" << std::endl;
		std::cout << "****************************************" << std::endl;
	}
}

template <typename Ind>
void Evolver<Ind>::evolve(unsigned eliteCount, double crossoverProb, double mutateProb){
	StopReason stop = start(eliteCount, crossoverProb, mutateProb);

	// printPopulation();
	while(stop==StopReason::Undefined){
		stop = step(eliteCount, crossoverProb, mutateProb);
		// printPopulation();
	}
	
	finish(stop);
}

template <typename Ind>
inline void Evolver<Ind>::printGen() const{
	if(verbose)
		printf("Generation [%3d]; Best=%.3e; Average=%.3e; Best genes=%s; Time=%lfs\n",
			generationStep
			,population->getBestFitness()
			,population->totalFitnessSum/populationSize
			,toString(population->getBest()).c_str()
			,C.L());
}


template <typename Ind>
unsigned Evolver<Ind>::selectParent(const VecD& fitness_cumulative, int other) const
{
	double fitnessSum = fitness_cumulative.back();
	double target = generator.getD(0.,fitnessSum);

	//original: https://stackoverflow.com/questions/39416560/how-can-i-simplify-this-working-binary-search-code-in-c/39417165#39417165
	unsigned pos = (other == 0 ? 1 : 0);
	unsigned limit = fitness_cumulative.size();
	while(pos < limit)
	{
		unsigned testpos = pos+((limit-pos)>>1);

		if (fitness_cumulative[testpos] < target)
			pos=testpos+1;
		else
			limit=testpos;
	}
	if((int)pos==other)
		pos = (other==(int)fitness_cumulative.size()-1 ? 0 : pos+1);

	return pos;
}

template <typename Ind>
unsigned Evolver<Ind>::findElite(unsigned eliteCount, Population<Ind> *offspring){
	if(population->speciation_threshold < 0.){ //no speciation - same as below, but faster
		std::partial_sort(population->species[0]->pop.begin(), population->species[0]->pop.begin()+eliteCount, population->species[0]->pop.end(), 
			Species<Ind>::compareIndividuals);
		FOR(ind, eliteCount) addIndividual(offspring, population->species[0]->pop[ind], 0);
		return eliteCount;
	}
	else{
		unsigned count = 0; //counts how many are actually added (some may not be added if species is too small --> too bad :P)
		//order species first
		std::sort(population->species.begin(), population->species.end(), Population<Ind>::compareSpecies);
		population->bestSpecies = 0;

		//only sort within the species precisely what is necessary
		unsigned numSpecies = population->numSpecies();
		unsigned eliteCountPerSpeciesInt = eliteCount/numSpecies; //integer division
		unsigned breakPoint = eliteCount%numSpecies;

		FORV(i,numSpecies){
			unsigned eliteCountPerSpecies = (i<breakPoint ? eliteCountPerSpeciesInt+1 : eliteCountPerSpeciesInt);
			eliteCountPerSpecies = std::min(eliteCountPerSpecies, population->species[i]->size());
			std::partial_sort(population->species[i]->pop.begin(), population->species[i]->pop.begin()+eliteCountPerSpecies, population->species[i]->pop.end(), 
				Species<Ind>::compareIndividuals);
			FOR(ind, eliteCountPerSpecies) {addIndividual(offspring, population->species[i]->pop[ind], i); ++count;}
		}
		return count;
	}
}
template <typename Ind>
void Evolver<Ind>::nullifyElite(unsigned eliteCount){
	if(population->speciation_threshold < 0.){ //no speciation - same as below, but faster
		FOR(ind, eliteCount) population->species[0]->pop[ind] = nullptr;
	}
	else{
		//only sort within the species precisely what is necessary
		unsigned numSpecies = population->numSpecies();
		unsigned eliteCountPerSpeciesInt = eliteCount/numSpecies; //integer division
		unsigned breakPoint = eliteCount%numSpecies;

		FORV(i,numSpecies){
			unsigned eliteCountPerSpecies = (i<breakPoint ? eliteCountPerSpeciesInt+1 : eliteCountPerSpeciesInt);
			eliteCountPerSpecies = std::min(eliteCountPerSpecies, population->species[i]->size());
			FOR(ind, eliteCountPerSpecies) population->species[i]->pop[ind] = nullptr;
		}
	}
}


template <typename Ind>
void Evolver<Ind>::generation(unsigned eliteCount, double crossoverProb, double mutateProb){
	// double t1=0.,t2=0.,t3=0.;
	// Clock C2;
	// C2.setVerbose(false);

	Population<Ind>* offspring = new Population<Ind>(population->speciation_threshold);

	unsigned eliteDone = eliteCount;
	if(eliteCount!=0)
		eliteDone = findElite(eliteCount, offspring);
	// t1+=C2.L();

	unsigned crossoverLast = eliteDone + (unsigned)((populationSize - eliteDone)*crossoverProb);

	//populationSize shared by default, for being 'const'
	#pragma omp parallel for \
	default(none) \
	shared(eliteDone, crossoverLast, population, mutateProb, offspring, generator)
	for(unsigned ind = eliteDone; ind<populationSize; ++ind){

		// unsigned parent1 = selectParent(population->fitness_cumulative);
		
		Individual<Ind> *child;

		// if(ind < crossoverLast){
		// 	unsigned parent2 = selectParent(population->fitness_cumulative, parent1);
		// 	child = new Individual<Ind>(crossover(population->pop[parent1]->I,population->pop[parent2]->I,
		// 		population->pop[parent1]->fitness_orig,population->pop[parent2]->fitness_orig));
		// }
		// else
			// child = new Individual<Ind>(*population->pop[parent1]);
		child = new Individual<Ind>(population->species[0]->pop[0]->I);

		// if(generator.getD() <= mutateProb){
		// 	mutate(child->I,this);
		// 	if(child->evaluated) child->evaluated = false;
		// }

		// offspring->pop[ind] = child;
		addIndividual(offspring, child);
	}
	// t2+=C2.L();

	nullifyElite(eliteCount);
	
	clear();
	population = offspring;

	// t3+=C2.L();
	// printf("TIMES: %lf + %lf + %lf = %lf\n",t1,t2,t3,t1+t2+t3);
}


template <typename Ind>
inline void Evolver<Ind>::initiatePopulation(){
	//populationSize shared by default, for being 'const'
	#pragma omp parallel for default(none) shared(population)
	for(unsigned i=0; i<populationSize; ++i) addIndividual(population, new Individual<Ind>(create(this)));
}

template <typename Ind>
StopReason Evolver<Ind>::updateFitness(){
	double oldBest = population->getBestFitnessEvolver();
	double oldAverage = population->totalFitnessSum / populationSize;

	double bestFitnessOfAll = -std::numeric_limits<double>::infinity();
	population->bestSpecies 	= 0;
	population->totalFitnessSum = 0.;

	double maxfitness_orig = -std::numeric_limits<double>::infinity();
	double minfitness_orig =  std::numeric_limits<double>::infinity();

	FORV(s,population->numSpecies()){

		population->species[s]->fitnessSum_orig = 0.;
		population->species[s]->fitnessSum 		= 0.;

		double fitnessSum_orig = 0.;

		//populationSize shared by default, for being 'const'
		#pragma omp parallel for \
		default(none) \
		shared(s) \
		reduction(+:fitnessSum_orig) \
		reduction(max:maxfitness_orig) \
		reduction(min:minfitness_orig)
		for(unsigned i=0; i<population->species[s]->size(); ++i){
			Individual<Ind>* ind = population->species[s]->pop[i];
			if(ind!=nullptr){
				if(!ind->evaluated)
					ind->fitness_orig = evaluate(ind->I,this);

				fitnessSum_orig += ind->fitness_orig;

				if(ind->fitness_orig > maxfitness_orig)
					maxfitness_orig = ind->fitness_orig;
				if(ind->fitness_orig < minfitness_orig) //for i=0 this is also true, I think that is why i didn't use 'else if'
					minfitness_orig = ind->fitness_orig;
			}
		}

		population->species[s]->fitnessSum_orig = fitnessSum_orig;

	}
	FORV(s,population->numSpecies()){

		population->species[s]->bestRank 		= 0;
		double bestFitness = -std::numeric_limits<double>::infinity();

		for(unsigned i=0; i<population->species[s]->size(); ++i){
			Individual<Ind>* ind = population->species[s]->pop[i];
			if(ind!=nullptr){
				//invert fitness_origes for minimize
				ind->fitness = (obj==MAXIMIZE ? ind->fitness_orig - minfitness_orig : maxfitness_orig - ind->fitness_orig);
				if(ind->fitness > bestFitness){
					population->species[s]->bestRank = i;
					bestFitness = ind->fitness;
				}
				ind->evaluated 					  			  = true;
				population->species[s]->fitnessSum 	 		 += ind->fitness;
				population->species[s]->fitness_cumulative[i] = population->species[s]->fitnessSum;
			}
		}
		population->totalFitnessSum += population->species[s]->fitnessSum;

		if( bestFitness > bestFitnessOfAll ){
			population->bestSpecies = s;
			bestFitnessOfAll = bestFitness;
		}

	}

	double newBest = population->getBestFitnessEvolver();
	double newAverage = population->totalFitnessSum / populationSize;
	return stopCriteria(oldBest, newBest, oldAverage, newAverage);
}


template <typename Ind>
StopReason Evolver<Ind>::stopCriteria(double oldBest, double newBest, double oldAverage, double newAverage) const{
	static unsigned bestStallCount 		= 0;
	static unsigned averageStallCount 	= 0;

	if(generationStep < 2 && maxGenerations > 2)
		return StopReason::Undefined;

	if(fabs(oldBest-newBest) < tolStallBest)
		++bestStallCount;
	else
		bestStallCount=0;

	if(fabs(oldAverage-newAverage) < tolStallAverage)
		++averageStallCount;
	else
		averageStallCount=0;

	if(generationStep >= maxGenerations)
		return StopReason::MaxGenerations;

	if(averageStallCount >= averageStallMax)
		return StopReason::StallAverage;

	if(bestStallCount >= bestStallMax)
		return StopReason::StallBest;

	// printf("%d %d\n",bestStallCount,averageStallCount);

	return StopReason::Undefined;
}

template <typename Ind>
void Evolver<Ind>::checkSettings(unsigned eliteCount, double crossoverProb, double mutateProb) const{
	if(create==nullptr)
		throw std::runtime_error("create() is not set.");
	if(crossover==nullptr)
		throw std::runtime_error("crossover() is not set.");
	if(mutate==nullptr)
		throw std::runtime_error("mutate() is not set.");
	if(evaluate==nullptr)
		throw std::runtime_error("evaluate() is not set.");
	if(toString==nullptr)
		throw std::runtime_error("toString() is not set.");

	if(populationSize<1)
		throw std::runtime_error("populationSize is below 1.");
	if(eliteCount>populationSize)
		throw std::runtime_error("eliteCount bigger than populationSize.");
	if(obj==NONE)
		throw std::runtime_error("Objective not set.");

	if(crossoverProb<0. || crossoverProb>1.)
		throw std::runtime_error("Invalid crossover fraction.");
	if(mutateProb<0. || mutateProb>1.)
		throw std::runtime_error("Invalid mutation rate.");
}

template <typename Ind>
void Evolver<Ind>::printPopulation() const{
	for(unsigned s=0; s<population->numSpecies(); ++s){
		if(population->speciation_threshold >= 0. ) std::cout << "Species " << s << std::endl;
		for(unsigned i=0; i<population->species[s]->size(); ++i){
			Individual<Ind>* ind = population->species[s]->pop[i];
			if(ind!=nullptr) std::cout << toString(ind->I) << " (F=" << ind->fitness_orig << ")" << std::endl;	
		}
	}
}


template <typename Ind>
void Evolver<Ind>::addIndividual(Population<Ind> *pop, Individual<Ind> *I, int species){

	//automatically add to chosen species - used for elite members, not to have to calculate dissimilarity again
	unsigned chosen = (species >= 0 ? species : 0);
	if(species > 0 && pop->numSpecies() <= chosen){ //species 0 we already know that exists - nothing needs to be done
		if(pop->numSpecies() < chosen)
			throw std::runtime_error("Trying to add a non-consecutive species");
		pop->species.push_back( new Species<Ind> );
	}
	else if(species < 0 && pop->speciation_threshold >= 0. && pop->species[0]->size()!=0){ //no speciation
		for(; chosen<pop->numSpecies(); ++chosen){
			if( pop->species[chosen]->size() == 0 ) continue;
			if( dissimilarity( I->I , pop->species[chosen]->pop[0]->I ) <= pop->speciation_threshold ) break;
		}
		if(chosen==pop->numSpecies())
			pop->species.push_back( new Species<Ind> );
	}

	pop->species[chosen]->addIndividual(I);
}

template <typename Ind> inline void Evolver<Ind>::setCreate 		(Ind 		(*func)	(const Evolver<Ind>*))
{ create 		= func; }
template <typename Ind> inline void Evolver<Ind>::setCrossover		(Ind 		(*func)	(const Ind&,const Ind&, double fit1, double fit2))
{ crossover		= func; }
template <typename Ind> inline void Evolver<Ind>::setMutate			(void 		(*func)	(Ind&, const Evolver<Ind>*))
{ mutate 		= func; }
template <typename Ind> inline void Evolver<Ind>::setEvaluate		(double		(*func)	(const Ind&, const Evolver<Ind>*), Objective o)
{ evaluate 		= func; obj = o; }
template <typename Ind> inline void Evolver<Ind>::setToString		(std::string(*func)	(const Ind&))
{ toString		= func; }
template <typename Ind> inline void Evolver<Ind>::setDissimilarity	(double		(*func)	(const Ind&, const Ind&))
{ dissimilarity	= func; }

