
#include "Evolver.hpp"

Random GA::generator;

GA::Parameters::Parameters():
eliteCount(0)
,crossoverProb(1.)
,mutateProb(0.1)

,numThreads(1)

,oldPenalty(0.01)
,youthBoost(1.0)
,oldAge(20)
,youthAge(10)

,survivalThreshold(0.2)
,obliterationRate(5)

,interSpeciesProb(0.005)
{}

