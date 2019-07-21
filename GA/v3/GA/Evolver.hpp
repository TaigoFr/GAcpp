#ifndef _EVOLVER_
#define _EVOLVER_

#include "Random.hpp"
#include "Clock.hpp"

#include "Population.hpp"
#include "Tools.hpp" //errorMsg
#include "StopCriteria.hpp" //errorMsg
#include "FileParser.hpp"

#include <string>

namespace GA{

	extern Random generator;

	enum Objective{
		NONE
		,MINIMIZE
		,MAXIMIZE
	};

	struct Parameters{
		unsigned eliteCount;
		double crossoverProb;
		double mutateProb;

		unsigned numThreads;

		//speciation
		double oldPenalty;
		double youthBoost;
		unsigned oldAge;
		unsigned youthAge;

		double survivalThreshold;
		unsigned obliterationRate;

		double interSpeciesProb;

		Parameters();
	};

	template <typename Ind>
	class Evolver: public Parameters{
	public:
		Evolver(const std::string& file, const std::string& prefix="");
		Evolver(unsigned _populationSize, double _speciation_threshold = -1., bool _verbose = true); //negative threshold means no speciation
		~Evolver();

		virtual StopReason start();
		virtual StopReason step();
		virtual void evolve();
		virtual void finish(StopReason stop) const;

		//same as above
		inline virtual StopReason start(unsigned _eliteCount, double _crossoverProb, double _mutateProb)
		{ eliteCount=_eliteCount; crossoverProb=_crossoverProb; mutateProb=_mutateProb; return start(); }
		inline virtual StopReason step(unsigned _eliteCount, double _crossoverProb, double _mutateProb)
		{ eliteCount=_eliteCount; crossoverProb=_crossoverProb; mutateProb=_mutateProb; return step(); }
		inline virtual void evolve(unsigned _eliteCount, double _crossoverProb, double _mutateProb)
		{ eliteCount=_eliteCount; crossoverProb=_crossoverProb; mutateProb=_mutateProb; evolve(); }

		inline void setCreate 		(Ind 		(*func)	(const Evolver<Ind>*));
		inline void setCrossover	(Ind 		(*func)	(const Ind&,const Ind&, double fit1, double fit2));
		inline void setMutate		(void 		(*func)	(Ind&, const Evolver<Ind>*));
		inline void setEvaluate		(double		(*func)	(const Ind&, const Evolver<Ind>*), Objective);
		inline void setToString		(std::string(*func)	(const Ind&));
		inline void setDissimilarity(double		(*func)	(const Ind&, const Ind&));
		inline void setPrintPopulation(void 	(*func)	(const Population<Ind>*));

		inline const Ind& 	getBest() const;
		inline double 		getBestFitness() const;

		inline void addCriteria(StopReason stop, const VecD& params){ criteria.addCriteria(stop, params); }

	protected:
		virtual unsigned selectParent(const VecD& fitness_cumulative, int other = -1) const; //to be replaced by similarity selection in NEAT
		
		unsigned populationSize;
		Population<Ind>* population;
		bool verbose;

		Ind 		(*create) 		(const Evolver<Ind>*);
		Ind 		(*crossover)	(const Ind&,const Ind&, double fit1, double fit2);
		void 		(*mutate)		(Ind&, const Evolver<Ind>*);
		double		(*evaluate)		(const Ind&, const Evolver<Ind>*);
		std::string	(*toString)		(const Ind&);
		double		(*dissimilarity)(const Ind&, const Ind&);
		void		(*printPopulation)(const Population<Ind>*);
		
	private:
		mutable Clock C;

		Objective obj;
		unsigned generationStep;
		StopCriteria criteria;

		void clear();
		void fileSettings(const std::string& file, const std::string& prefix="");

		inline void initiatePopulation();
		StopReason updateFitness();

		unsigned findElite(Population<Ind> *offspring);
		void nullifyElite();
		VecD calculateExpectedOffspring(unsigned eliteDone) const;
		void eliminateWeak();

		void generation();
		void checkSettings() const;
		void crossoverAndMutate(double crossoverProb, double mutateProb);
		inline void printGen() const;

		void addIndividual(Population<Ind> *pop, Individual<Ind> *I, int species = -1);
	};

	template <typename Ind> inline void 		mutate_default	(Ind&, const Evolver<Ind>*)		{ return; }
	template <typename Ind> inline std::string 	toString_default(const Ind&)					{ return ""; }
	// template <typename Ind> inline double 		dissimilarity_default(const Ind&, const Ind&) 	{ return generator(); }
	template <typename Ind> inline void printPopulation_default	(const Population<Ind>* pop) 	{ pop->print(); } //to use if needed

	//include insine namespace
	#include "Evolver.impl.hpp"
};

#endif //_EVOLVER_
