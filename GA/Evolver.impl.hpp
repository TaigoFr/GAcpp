
#if !defined(_EVOLVER_)
#error "This file should only be included through Evolver.hpp"
#endif

#include <assert.h>

#include <omp.h>

template <typename Ind>
Evolver<Ind>::Evolver(unsigned _populationSize):
maxGenerations(100)
,tolStallAverage(1.e-6)
,averageStallMax(25)
,tolStallBest(1.e-8)
,bestStallMax(25)
,numThreads(1)

,C("",Clock::stop)
,populationSize(_populationSize)
,population(new Population<Ind>(populationSize))
,obj(NONE)
,generationStep(0)

,create(nullptr)
,mate(nullptr)
,mutate(mutate_default)
,evaluate(nullptr)
,toString(toString_default)
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
void Evolver<Ind>::evolve(unsigned eliteCount, double mateProb, double mutateProb){
	std::cout << "****************************************"	<< std::endl;
	std::cout << "*            Evolver started           *" << std::endl;
	std::cout << "****************************************" << std::endl;
	std::cout << "population:\t\t" 		<< populationSize 		<< std::endl;
	std::cout << "eliteCount:\t\t" 		<< eliteCount 			<< std::endl;
	std::cout << "mateProb:\t\t" 		<< mateProb 			<< std::endl;
	std::cout << "mutationProb:\t\t" 	<< mutateProb 			<< std::endl;

	checkSettings(eliteCount, mateProb, mutateProb);

	unsigned num;
	#pragma omp parallel
		#pragma omp master
	num = omp_get_num_threads();
	if(num!=numThreads && numThreads!=0)
		omp_set_num_threads(numThreads);

	std::cout << "OMP_NUM_THREADS:\t" 	<< (numThreads ? numThreads : num) << std::endl;
	std::cout << "****************************************" << std::endl;

	StopReason stop = StopReason::Undefined;
	
	C.R();
	initiatePopulation();
	generationStep = 0;
	
	stop = updateFitness();
	printGen();
	while(stop==StopReason::Undefined){
		// printPopulation();
		generation(eliteCount, mateProb, mutateProb);
		++generationStep;

		stop = updateFitness();
		printGen();
	}
	printStopReason(stop);
	// printPopulation();

	C.setVerbose(true);
	C("Total time: ","s\n");
	C.setVerbose(false);

	std::cout << "****************************************" << std::endl;
}

template <typename Ind>
void Evolver<Ind>::printGen(){
	printf("Generation [%3d]; Best=%.3e; Average=%.3e; Best genes=%s; Time=%lfs\n",
		generationStep,population->pop[population->bestRank]->fitness,population->fitnessSum/populationSize,toString(population->pop[population->bestRank]->I).c_str(),C.L());
}


template <typename Ind>
unsigned Evolver<Ind>::selectParent(int other)
{
	double target = generator.getD(0.,population->fitnessSum_norm);

	//original: https://stackoverflow.com/questions/39416560/how-can-i-simplify-this-working-binary-search-code-in-c/39417165#39417165
	unsigned pos = (other == 0 ? 1 : 0);
	unsigned limit = populationSize;
	while(pos < limit)
	{
		unsigned testpos = pos+((limit-pos)>>1);

		if (population->fitness_comulative[testpos] < target)
			pos=testpos+1;
		else
			limit=testpos;
	}
	if((int)pos==other)
		pos = (other==(int)populationSize-1 ? 0 : pos+1);

	// population->fitness_comulative.print("Fitness_Comulative: ",9);
	// printf("fitnessSum_norm = %lf\ntarget = %lf\nFinal = %u\n",population->fitnessSum_norm,target,pos);

	return pos;
}

// template <typename Ind>
// unsigned Evolver<Ind>::selectParent(int other){
// 	//assumes population.fitness() was already called

// 	double target = generator.getD(0.,population->fitnessSum_norm);

// 	unsigned parent = (other == 0 ? 1 : 0);
// 	if(other<0){//just give one parent
// 		//parent is always one ahead
// 		for (double sumSoFar = population->pop[parent++]->fitness_norm; parent < populationSize; sumSoFar += population->pop[parent++]->fitness_norm)
// 			if (sumSoFar >= target)
// 				break;
// 	}
// 	else{//give a parent !=other
// 		for (double sumSoFar = population->pop[parent++]->fitness_norm; parent < populationSize; sumSoFar += population->pop[parent++]->fitness_norm){
// 			if(parent==other)
// 				continue;
// 			if(sumSoFar >= target)
// 				break;
// 		}
// 		if(parent==populationSize) //this means that other=populationSize-1
// 			--parent;
// 	}

// 	return --parent;
// }

template <typename Ind>
void Evolver<Ind>::findElite(unsigned eliteCount){
	if(obj == MINIMIZE)
		std::partial_sort(population->pop.begin(), population->pop.begin()+eliteCount, population->pop.end(), 
			[this](Individual<Ind>* left, Individual<Ind>* right) {return (left==nullptr || right==nullptr) ? false : ((left->fitness_norm) > (right->fitness_norm)); });
	else
		std::partial_sort(population->pop.begin(), population->pop.begin()+eliteCount, population->pop.end(), 
			[this](Individual<Ind>* left, Individual<Ind>* right) {return (left==nullptr || right==nullptr) ? false : ((left->fitness_norm) < (right->fitness_norm)); });
}



template <typename Ind>
void Evolver<Ind>::generation(unsigned eliteCount, double mateProb, double mutateProb){
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

	unsigned mateLast = eliteCount + (unsigned)((populationSize - eliteCount)*mateProb);

	#pragma omp parallel for \
	default(none) \
	shared(eliteCount, populationSize, mateLast, population, mutateProb, offspring, generator)
	for(unsigned ind = eliteCount; ind<populationSize; ++ind){

		unsigned parent1 = selectParent();
		
		Individual<Ind> *child;

		if(ind <= mateLast){
			unsigned parent2 = selectParent(parent1);
			child = new Individual<Ind>(mate(population->pop[parent1]->I,population->pop[parent2]->I));
		}
		else
			child = new Individual<Ind>(*population->pop[parent1]);

		if(generator.getD() <= mutateProb)
			mutate(child->I);

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
void Evolver<Ind>::initiatePopulation()
{ for(unsigned i=0; i<populationSize; ++i) population->pop[i] = new Individual<Ind>(create()); }

template <typename Ind>
StopReason Evolver<Ind>::updateFitness(){
	double oldBest = population->pop[population->bestRank]->fitness_norm;
	double oldAverage = population->fitnessSum_norm / populationSize;

	population->bestRank = 0;
	population->fitnessSum 		= 0.;
	population->fitnessSum_norm = 0.;

	double maxfitness = -std::numeric_limits<double>::infinity();

	Individual<Ind>* ind;
	if(obj == MAXIMIZE){

		for(unsigned i=0; i<populationSize; ++i){
			ind = population->pop[i];
			if(ind!=nullptr){
				double fitness = evaluate(ind->I);
				
				ind->fitness 			= fitness;
				population->fitnessSum += ind->fitness_norm;

				ind->fitness_norm 			 = fitness;
				population->fitnessSum_norm += ind->fitness_norm;
				population->fitness_comulative[i] = population->fitnessSum_norm;

				if(fitness>maxfitness){
					population->bestRank = i;
					maxfitness = fitness;
				}


			}
		}
	}
	else{ //MINIMIZE

		double minfitness = -maxfitness;

		for(unsigned i=0; i<populationSize; ++i){
			ind = population->pop[i];
			if(ind!=nullptr){
				double fitness = evaluate(ind->I);

				ind->fitness 			= fitness;
				population->fitnessSum += ind->fitness;

				if(fitness>maxfitness)	maxfitness = fitness;
				if(fitness<minfitness){
					population->bestRank = i;
					minfitness = fitness;
				}
			}
		}

		for(unsigned i=0; i<populationSize; ++i){
			ind = population->pop[i];
			if(ind!=nullptr){
				ind->fitness_norm 			 = maxfitness - ind->fitness + minfitness; //invert fitnesses - swap max for min in order to minimize fitness
				population->fitnessSum_norm += ind->fitness_norm;
				population->fitness_comulative[i] = population->fitnessSum_norm;
			}
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

void printStopReason(StopReason stop){
	std::string str;
	switch(stop)
	{
		case StopReason::Undefined:
		str = "No-stop";
		break;
		case StopReason::MaxGenerations:
		str = "Maximum generation reached";
		break;
		case StopReason::StallAverage:
		str = "Average stalled";
		break;
		case StopReason::StallBest:
		str = "Best stalled";
		break;
		default:
		str = "Unknown reason";
	}

	std::cout << "Stop criteria: ";
	if(stop == StopReason::Undefined)
		std::cout << "There is a bug in this function";
	else
		std::cout << str;
	std::cout << std::endl;
}

template <typename Ind>
void Evolver<Ind>::checkSettings(unsigned eliteCount, double mateProb, double mutateProb){
	if(create==nullptr)
		throw std::runtime_error("create() is not set.");
	if(mate==nullptr)
		throw std::runtime_error("mate() is not set.");
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

	if(mateProb<0. || mateProb>1.)
		throw std::runtime_error("Invalid crossover fraction.");
	if(mutateProb<0. || mutateProb>1.)
		throw std::runtime_error("Invalid mutation rate.");
}

template <typename Ind>
void Evolver<Ind>::printPopulation(){
	for(unsigned i=0; i<populationSize; ++i){
		Individual<Ind>* ind = population->pop[i];
		if(ind!=nullptr) std::cout << toString(ind->I) << std::endl;	
	}
}

template <typename Ind> void Evolver<Ind>::setCreate 	(Ind 		(*func)	())							{ create 	= func; }
template <typename Ind> void Evolver<Ind>::setMate 		(Ind 		(*func)	(const Ind&,const Ind&))	{ mate 		= func; }
template <typename Ind> void Evolver<Ind>::setMutate	(void 		(*func)	(Ind&))						{ mutate 	= func; }
template <typename Ind> void Evolver<Ind>::setEvaluate	(double		(*func)	(const Ind&), Objective o)	{ evaluate 	= func; obj = o; }
template <typename Ind> void Evolver<Ind>::setToString	(std::string(*func)	(const Ind&))				{ toString	= func; }