
#if !defined(_EVOLVER_)
#error "This file should only be included through Evolver.hpp"
#endif

#include <assert.h>

#include <omp.h>

template <typename Ind>
Evolver<Ind>::Evolver(unsigned _populationSize, bool _verbose):
maxGenerations(100)
,tolStallAverage(1.e-6)
,averageStallMax(15)
,tolStallBest(1.e-8)
,bestStallMax(15)
,numThreads(1)

,populationSize(_populationSize)
,population(new Population<Ind>(populationSize))

,create(nullptr)
,crossover(nullptr)
,mutate(mutate_default)
,evaluate(nullptr)
,toString(toString_default)

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
Ind&  	Evolver<Ind>::getBest()				{ return population->pop[population->bestRank]->I; }
template <typename Ind>
double 	Evolver<Ind>::getBestFitness() const{ return population->pop[population->bestRank]->fitness; }


template <typename Ind>
void Evolver<Ind>::evolve(unsigned eliteCount, double crossoverProb, double mutateProb){
	if(verbose){
		std::cout << "****************************************"	<< std::endl;
		std::cout << "*            Evolver started           *" << std::endl;
		std::cout << "****************************************" << std::endl;
		std::cout << "population:\t\t" 		<< populationSize 		<< std::endl;
		std::cout << "eliteCount:\t\t" 		<< eliteCount 			<< std::endl;
		std::cout << "crossoverProb:\t\t" 	<< crossoverProb 			<< std::endl;
		std::cout << "mutationProb:\t\t" 	<< mutateProb 			<< std::endl;
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

	StopReason stop = StopReason::Undefined;
	
	C.R();
	initiatePopulation();
	generationStep = 0;
	
	stop = updateFitness();
	printGen();
	while(stop==StopReason::Undefined){
		// printPopulation();
		generation(eliteCount, crossoverProb, mutateProb);
		++generationStep;

		stop = updateFitness();
		printGen();
	}
	if(verbose) printStopReason(stop);
	// printPopulation();

	if(verbose){
		C.setVerbose(true);
		double time = C("Total time: ","s\n");
		C.setVerbose(false);
		std::cout << "Time/Generation: " << time/generationStep << "s" << std::endl;
		std::cout << "****************************************" << std::endl;
	}
}

template <typename Ind>
void Evolver<Ind>::printGen(){
	if(verbose)
		printf("Generation [%3d]; Best=%.3e; Average=%.3e; Best genes=%s; Time=%lfs\n",
			generationStep,population->pop[population->bestRank]->fitness,population->fitnessSum/populationSize,toString(population->pop[population->bestRank]->I).c_str(),C.L());
}


template <typename Ind>
unsigned Evolver<Ind>::selectParent(const VecD& fitness_cumulative, int other)
{
	double fitnessSum_norm = fitness_cumulative.back();
	double target = generator.getD(0.,fitnessSum_norm);

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

	// population->fitness_cumulative.print("Fitness_cumulative: ",9);
	// printf("fitnessSum_norm = %lf\ntarget = %lf\nFinal = %u\n",population->fitnessSum_norm,target,pos);

	return pos;
}

template <typename Ind>
void Evolver<Ind>::findElite(unsigned eliteCount){
	std::partial_sort(population->pop.begin(), population->pop.begin()+eliteCount, population->pop.end(), 
		[this](Individual<Ind>* left, Individual<Ind>* right) {return (left==nullptr || right==nullptr) ? false : ((left->fitness_norm) > (right->fitness_norm)); });
}



template <typename Ind>
void Evolver<Ind>::generation(unsigned eliteCount, double crossoverProb, double mutateProb){
	// double t1=0.,t2=0.,t3=0.;
	// Clock C2;
	// C2.setVerbose(false);

	Population<Ind>* offspring = new Population<Ind>(populationSize);

	if(eliteCount!=0){
		findElite(eliteCount);

		for(unsigned ind = 0; ind<eliteCount; ++ind)
			offspring->pop[ind] = population->pop[ind];
	}
	// t1+=C2.L();

	unsigned crossoverLast = eliteCount + (unsigned)((populationSize - eliteCount)*crossoverProb);

	#pragma omp parallel for \
	default(none) \
	shared(eliteCount, populationSize, crossoverLast, population, mutateProb, offspring, generator)
	for(unsigned ind = eliteCount; ind<populationSize; ++ind){

		unsigned parent1 = selectParent(population->fitness_cumulative);
		
		Individual<Ind> *child;

		if(ind < crossoverLast){
			unsigned parent2 = selectParent(population->fitness_cumulative, parent1);
			child = new Individual<Ind>(crossover(population->pop[parent1]->I,population->pop[parent2]->I,
				population->pop[parent1]->fitness,population->pop[parent2]->fitness));
		}
		else
			child = new Individual<Ind>(*population->pop[parent1]);

		if(generator.getD() <= mutateProb){
			mutate(child->I,this);
			if(child->evaluated) child->evaluated = false;
		}

		offspring->pop[ind] = child;
	}
	// t2+=C2.L();

	// nullptrify references to elites
	for (unsigned ind = 0; ind<eliteCount; ++ind)
		population->pop[ind] = nullptr;
	
	clear();
	population = offspring;

	// t3+=C2.L();
	// printf("TIMES: %lf + %lf + %lf = %lf\n",t1,t2,t3,t1+t2+t3);
}


template <typename Ind>
void Evolver<Ind>::initiatePopulation(){
	#pragma omp parallel for default(none) shared(population, populationSize)
	for(unsigned i=0; i<populationSize; ++i) population->pop[i] = new Individual<Ind>(create(this));
}

template <typename Ind>
StopReason Evolver<Ind>::updateFitness(){
	double oldBest = population->pop[population->bestRank]->fitness_norm;
	double oldAverage = population->fitnessSum_norm / populationSize;

	population->bestRank = 0;
	population->fitnessSum 		= 0.;
	population->fitnessSum_norm = 0.;

	double maxfitness = -std::numeric_limits<double>::infinity();
	double minfitness = -maxfitness;

	double fitnessSum = 0.;
	unsigned bestRank = 0;

	#pragma omp parallel for \
	default(none) \
	shared(populationSize, bestRank) \
	reduction(+:fitnessSum) \
	reduction(max:maxfitness) \
	reduction(min:minfitness)
	for(unsigned i=0; i<populationSize; ++i){
		Individual<Ind>* ind = population->pop[i];
		if(ind!=nullptr){
			if(!ind->evaluated)
				ind->fitness = evaluate(ind->I,this);

			fitnessSum += ind->fitness;

			if(ind->fitness > maxfitness){
				if(obj==MAXIMIZE)
					#pragma omp critical
					bestRank = i;
				maxfitness = ind->fitness;
			}
			if(ind->fitness < minfitness){
				if(obj==MINIMIZE)
					#pragma omp critical
					bestRank = i;
				minfitness = ind->fitness;
			}
		}
	}

	population->fitnessSum = fitnessSum;
	population->bestRank   = bestRank;

	for(unsigned i=0; i<populationSize; ++i){
		Individual<Ind>* ind = population->pop[i];
		if(ind!=nullptr){
			//invert fitnesses for minimize
			ind->fitness_norm 			 	  = (obj==MAXIMIZE ? ind->fitness - minfitness : maxfitness - ind->fitness);
			ind->evaluated 					  = true;
			population->fitnessSum_norm 	 += ind->fitness_norm;
			population->fitness_cumulative[i] = population->fitnessSum_norm;
		}
	}

	double newBest = population->pop[population->bestRank]->fitness_norm;
	double newAverage = population->fitnessSum_norm / populationSize;
	return stopCriteria(oldBest, newBest, oldAverage, newAverage);
}


template <typename Ind>
StopReason Evolver<Ind>::stopCriteria(double oldBest, double newBest, double oldAverage, double newAverage){
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
void Evolver<Ind>::checkSettings(unsigned eliteCount, double crossoverProb, double mutateProb){
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
	for(unsigned i=0; i<populationSize; ++i){
		Individual<Ind>* ind = population->pop[i];
		if(ind!=nullptr) std::cout << toString(ind->I) << " (F=" << ind->fitness << ")" << std::endl;	
	}
}

template <typename Ind> void Evolver<Ind>::setCreate 	(Ind 		(*func)	(const Evolver<Ind>*))							{ create 	= func; }
template <typename Ind> void Evolver<Ind>::setCrossover	(Ind 		(*func)	(const Ind&,const Ind&,
double fit1, double fit2))						{ crossover	= func; }
template <typename Ind> void Evolver<Ind>::setMutate	(void 		(*func)	(Ind&, const Evolver<Ind>*))					{ mutate 	= func; }
template <typename Ind> void Evolver<Ind>::setEvaluate	(double		(*func)	(const Ind&, const Evolver<Ind>*), Objective o)	{ evaluate 	= func; obj = o; }
template <typename Ind> void Evolver<Ind>::setToString	(std::string(*func)	(const Ind&))									{ toString	= func; }

