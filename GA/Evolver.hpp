#ifndef _EVOLVER_
#define _EVOLVER_

#include "Random.hpp"
#include "Clock.hpp"

#include "Population.hpp"

#include <string>

namespace GA{

	Random generator;

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

		void clear();

		void evolve(unsigned elite_count, double mate_prob, double mutate_prob);

		void initiatePopulation();
		StopReason updateFitness();
		void printPopulation();

		void setCreate 	(Ind 		(*func)	());
		void setMate 	(Ind 		(*func)	(const Ind&,const Ind&));
		void setMutate	(void 		(*func)	(Ind&));
		void setEvaluate(double		(*func)	(const Ind&), Objective);
		void setToString(std::string(*func)	(const Ind&));

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

		unsigned selectParent(int other = -1);
		void findElite(unsigned elite_count);
		StopReason stopCriteria(double oldBest, double newBest, double oldAverage, double newAverage);

		void generation(unsigned eliteCount, double mateProb, double mutateProb);
		void checkSettings(unsigned eliteCount, double mateProb, double mutateProb);
		// void preSelection(unsigned elite_count);
		void crossoverAndMutate(double mateProb, double mutateProb);
		void printGen();

		Ind 		(*create) 	();
		Ind 		(*mate) 	(const Ind&,const Ind&);
		void 		(*mutate)	(Ind&);
		double		(*evaluate)	(const Ind&);
		std::string	(*toString)	(const Ind&);
	};

	template <typename Ind> void 		mutate_default	(Ind&)		{ return; }
	template <typename Ind> std::string toString_default(const Ind&){ return ""; }

	#include "Evolver.impl.hpp"

};

#endif //_EVOLVER_