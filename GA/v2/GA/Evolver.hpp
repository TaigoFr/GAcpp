#ifndef _EVOLVER_
#define _EVOLVER_

#include "Random.hpp"
#include "Clock.hpp"

#include "Population.hpp"

#include <string>

namespace GA{

	extern Random generator;

	enum Objective{
		NONE
		,MINIMIZE
		,MAXIMIZE
	};

	enum StopReason
	{
		Undefined
		,MaxGenerations
		,StallAverage
		,StallBest
	};
	void printStopReason(StopReason stop);

	template <typename Ind>
	class Evolver{
	public:
		Evolver(unsigned _populationSize);
		~Evolver();

		void evolve(unsigned eliteCount, double crossoverProb, double mutateProb);

		void setCreate 		(Ind 		(*func)	(const Evolver<Ind>*));
		void setCrossover	(Ind 		(*func)	(const Ind&,const Ind&));
		void setMutate		(void 		(*func)	(Ind&));
		void setEvaluate	(double		(*func)	(const Ind&), Objective);
		void setToString	(std::string(*func)	(const Ind&));

		void printPopulation();

		unsigned	maxGenerations;
		double 		tolStallAverage;
		unsigned 	averageStallMax;
		double 		tolStallBest;
		unsigned 	bestStallMax;
		unsigned 	numThreads;

	private:
		Clock C;

		unsigned populationSize;
		Population<Ind>* population;

		Objective obj;
		unsigned generationStep;

		void clear();

		void initiatePopulation();
		StopReason updateFitness();

		unsigned selectParent(int other = -1);
		void findElite(unsigned elite_count);
		StopReason stopCriteria(double oldBest, double newBest, double oldAverage, double newAverage);

		void generation(unsigned eliteCount, double crossoverProb, double mutateProb);
		void checkSettings(unsigned eliteCount, double crossoverProb, double mutateProb);
		// void preSelection(unsigned elite_count);
		void crossoverAndMutate(double crossoverProb, double mutateProb);
		void printGen();

		Ind 		(*create) 	(const Evolver<Ind>*);
		Ind 		(*crossover)(const Ind&,const Ind&);
		void 		(*mutate)	(Ind&);
		double		(*evaluate)	(const Ind&);
		std::string	(*toString)	(const Ind&);
	};

	template <typename Ind> void 		mutate_default	(Ind&)		{ return; }
	template <typename Ind> std::string toString_default(const Ind&){ return ""; }

	#include "Evolver.impl.hpp"

};

#endif //_EVOLVER_
