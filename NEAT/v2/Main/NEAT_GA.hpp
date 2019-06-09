
#pragma once

#include "Network.hpp"
#include "Evolver.hpp"

namespace NEAT{

	class Evolver: public GA::Evolver<Network>{
	public:
		Evolver(const std::string& file, const std::string& prefix="");
		Evolver(unsigned _populationSize, double speciation_threshold = -1., bool verbose = true);

		GA::StopReason start();
		void evolve();

		//same as above
		inline GA::StopReason start(unsigned _eliteCount, double _crossoverProb, double _mutateProb)
		{ eliteCount=_eliteCount; crossoverProb=_crossoverProb; mutateProb=_mutateProb; return start(); }
		inline void evolve(unsigned _eliteCount, double _crossoverProb, double _mutateProb)
		{ eliteCount=_eliteCount; crossoverProb=_crossoverProb; mutateProb=_mutateProb; evolve(); }

		NEAT::Parameters params;
		
	private:
		void fileNEATSettings(const std::string& file, const std::string& prefix);
	};
};