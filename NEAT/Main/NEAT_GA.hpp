
#pragma once

#include "Network.hpp"
#include "Evolver.hpp"

namespace NEAT{

	class Evolver: public GA::Evolver<Network>{
	public:
		Evolver(unsigned _populationSize);

		void evolve(unsigned eliteCount, double crossoverProb, double mutateProb);

		Parameters params;
	private:
		unsigned selectParent(const VecD& fitness_cumulative, int other = -1) const;
	};
};