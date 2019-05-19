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
		Evolver(unsigned _populationSize, bool _verbose = true);
		~Evolver();

		StopReason start(unsigned eliteCount, double crossoverProb, double mutateProb);
		StopReason step(unsigned eliteCount, double crossoverProb, double mutateProb);
		void finish(StopReason stop) const;

		void evolve(unsigned eliteCount, double crossoverProb, double mutateProb);

		inline void setCreate 	(Ind 		(*func)	(const Evolver<Ind>*));
		inline void setCrossover(Ind 		(*func)	(const Ind&,const Ind&, double fit1, double fit2));
		inline void setMutate	(void 		(*func)	(Ind&, const Evolver<Ind>*));
		inline void setEvaluate	(double		(*func)	(const Ind&, const Evolver<Ind>*), Objective);
		inline void setToString	(std::string(*func)	(const Ind&));

		inline const Ind& 	getBest() const;
		inline double 	getBestFitness() const;
		void printPopulation() const;

		unsigned	maxGenerations;
		double 		tolStallAverage;
		unsigned 	averageStallMax;
		double 		tolStallBest;
		unsigned 	bestStallMax;
		unsigned 	numThreads;

	protected:
		virtual unsigned selectParent(const VecD& fitness_cumulative, int other = -1) const; //to be replaced by similarity selection in NEAT
		
		const unsigned populationSize;
		Population<Ind>* population;

		Ind 		(*create) 	(const Evolver<Ind>*);
		Ind 		(*crossover)(const Ind&,const Ind&, double fit1, double fit2);
		void 		(*mutate)	(Ind&, const Evolver<Ind>*);
		double		(*evaluate)	(const Ind&, const Evolver<Ind>*);
		std::string	(*toString)	(const Ind&);
		
	private:
		mutable Clock C;

		Objective obj;
		unsigned generationStep;
		const bool verbose;

		void clear();

		inline void initiatePopulation();
		StopReason updateFitness();

		inline void findElite(unsigned elite_count);
		StopReason stopCriteria(double oldBest, double newBest, double oldAverage, double newAverage) const;

		void generation(unsigned eliteCount, double crossoverProb, double mutateProb);
		void checkSettings(unsigned eliteCount, double crossoverProb, double mutateProb) const;
		// void preSelection(unsigned elite_count);
		void crossoverAndMutate(double crossoverProb, double mutateProb);
		inline void printGen() const;
	};

	template <typename Ind> inline void 		mutate_default	(Ind&, const Evolver<Ind>*)	{ return; }
	template <typename Ind> inline std::string 	toString_default(const Ind&)				{ return ""; }

	#include "Evolver.impl.hpp"

};

#endif //_EVOLVER_
