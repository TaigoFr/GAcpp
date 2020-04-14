
#if !defined(_EVOLVER_)
#error "This file should only be included through Evolver.hpp"
#endif

#ifndef _EVOLVER_IMPL_
#define _EVOLVER_IMPL_

#include <assert.h>
#include <omp.h>

template <typename Ind>
Evolver<Ind>::Evolver(const std::string &file, const std::string &prefix)
    : Parameters()

      ,
      populationSize(0), population(nullptr), verbose(true)

      ,
      create(nullptr), crossover(nullptr), mutate(mutate_default),
      evaluate(nullptr), toString(toString_default), dissimilarity(nullptr),
      printPopulation(nullptr)

      ,
      C("", Clock::stop), obj(NONE), generationStep(0), criteria()
{
    fileSettings(file, prefix);

    assert(populationSize >= 2);
    C.setVerbose(false);
}

template <typename Ind>
Evolver<Ind>::Evolver(unsigned _populationSize, double _speciation_threshold,
                      bool _verbose)
    : Parameters()

      ,
      populationSize(_populationSize),
      population(new Population<Ind>(_speciation_threshold)), verbose(_verbose)

      ,
      create(nullptr), crossover(nullptr), mutate(mutate_default),
      evaluate(nullptr), toString(toString_default), dissimilarity(nullptr),
      printPopulation(nullptr)

      ,
      C("", Clock::stop), obj(NONE), generationStep(0), criteria()
{
    assert(populationSize >= 2);
    C.setVerbose(false);
}

template <typename Ind> Evolver<Ind>::~Evolver() { clear(); }

template <typename Ind> void Evolver<Ind>::clear()
{
    // printf("In Evolver::clear()\n");
    if (population != nullptr)
    {
        delete population;
        population = nullptr;
    }
}

template <typename Ind>
void Evolver<Ind>::fileSettings(const std::string &file,
                                const std::string &prefix)
{
    FileParser fp(file, false);

    double speciationThreshold;

    if (!fp(prefix + "populationSize", populationSize))
        errorMsg("populationSize is a mandatory parameter.");
    if (!fp(prefix + "eliteCount", eliteCount))
        errorMsg("eliteCount is a mandatory parameter.");
    if (!fp(prefix + "crossoverProb", crossoverProb))
        errorMsg("crossoverProb is a mandatory parameter.");
    if (!fp(prefix + "mutateProb", mutateProb))
        errorMsg("mutateProb is a mandatory parameter.");

    fp(prefix + "speciationThreshold", speciationThreshold, -1.);
    fp(prefix + "verbose", verbose, true);

    fp(prefix + "numThreads", numThreads);
    fp(prefix + "oldPenalty", oldPenalty);
    fp(prefix + "youthBoost", youthBoost);
    fp(prefix + "oldAge", oldAge);
    fp(prefix + "youthAge", youthAge);
    fp(prefix + "survivalThreshold", survivalThreshold);
    fp(prefix + "obliterationRate", obliterationRate);
    fp(prefix + "interSpeciesProb", interSpeciesProb);

    // STOPPING CRITERIA PARAMETERS
    unsigned maxGenerations;
    if (fp(prefix + "maxGenerations", maxGenerations))
        addCriteria(GA::StopReason::MaxGenerations, {(double)maxGenerations});

    double averageStalledTol;
    unsigned averageStalledMax;
    if (fp(prefix + "averageStalledTol", averageStalledTol) &&
        fp(prefix + "averageStalledMax", averageStalledMax))
        addCriteria(GA::StopReason::AverageStalled,
                    {averageStalledTol, (double)averageStalledMax});

    double bestStalledTol;
    unsigned bestStalledMax;
    if (fp(prefix + "bestStalledTol", bestStalledTol) &&
        fp(prefix + "bestStalledMax", bestStalledMax))
        addCriteria(GA::StopReason::BestStalled,
                    {bestStalledTol, (double)bestStalledMax});

    double averageRelativeStalledTol;
    unsigned averageRelativeStalledMax;
    if (fp(prefix + "averageRelativeStalledTol", averageRelativeStalledTol) &&
        fp(prefix + "averageRelativeStalledMax", averageRelativeStalledMax))
        addCriteria(
            GA::StopReason::AverageRelativeStalled,
            {averageRelativeStalledTol, (double)averageRelativeStalledMax});

    double bestRelativeStalledTol;
    unsigned bestRelativeStalledMax;
    if (fp(prefix + "bestRelativeStalledTol", bestRelativeStalledTol) &&
        fp(prefix + "bestRelativeStalledMax", bestRelativeStalledMax))
        addCriteria(GA::StopReason::BestRelativeStalled,
                    {bestRelativeStalledTol, (double)bestRelativeStalledMax});

    clear();
    population = new Population<Ind>(speciationThreshold);
}

template <typename Ind> inline const Ind &Evolver<Ind>::getBest() const
{
    return population->getBest();
}
template <typename Ind> inline double Evolver<Ind>::getBestFitness() const
{
    return population->getBestFitness();
}
template <typename Ind>
inline Population<Ind> *Evolver<Ind>::getPopulation() const
{
    return population;
}
template <typename Ind> inline unsigned Evolver<Ind>::getPopulationSize() const
{
    return populationSize;
}

template <typename Ind> StopReason Evolver<Ind>::start()
{
    if (verbose)
    {
        std::cout << "****************************************" << std::endl;
        std::cout << "*            Evolver started           *" << std::endl;
        std::cout << "****************************************" << std::endl;
        std::cout << "population:\t\t" << populationSize << std::endl;
        std::cout << "eliteCount:\t\t" << eliteCount << std::endl;
        std::cout << "crossoverProb:\t\t" << crossoverProb << std::endl;
        std::cout << "mutationProb:\t\t" << mutateProb << std::endl;
        std::cout << "speciation:\t\t"
                  << (population->speciation_threshold < 0
                          ? 0
                          : population->speciation_threshold)
                  << std::endl;
    }

    checkSettings();

    unsigned num;
#pragma omp parallel
    {
#pragma omp master
        num = omp_get_num_threads();
    }
    if (numThreads == 0)
        numThreads = num;
    if (num != numThreads)
        omp_set_num_threads(numThreads);

    if (verbose)
    {
        std::cout << "OMP_NUM_THREADS:\t" << numThreads << std::endl;
        std::cout << "****************************************" << std::endl;
    }

    C.R();
    initiatePopulation();
    generationStep = 0;

    criteria.update(0, 0, 0, 0, 0, true); // restart criteria
    StopReason stop = updateFitness();
    printGen();

    return stop;
}

template <typename Ind> StopReason Evolver<Ind>::step()
{
    generation();
    ++generationStep;

    StopReason stop = updateFitness();
    printGen();

    return stop;
}

template <typename Ind> void Evolver<Ind>::finish(StopReason stop) const
{
    if (verbose)
        StopCriteria::printStopReason(stop);

    if (verbose)
    {
        C.setVerbose(true);
        double time = C("Total time: ", "s\n");
        C.setVerbose(false);
        std::cout << "Time/Generation: " << time / generationStep << "s"
                  << std::endl;
        std::cout << "****************************************" << std::endl;
    }
}

template <typename Ind> void Evolver<Ind>::evolve()
{
    StopReason stop = start();

    if (printPopulation != nullptr && !printPopulation(population))
        stop = StopReason::UserTerminated;

    while (stop == StopReason::Undefined)
    {
        stop = step();

        if (printPopulation != nullptr && !printPopulation(population))
            stop = StopReason::UserTerminated;
    }

    finish(stop);
}

template <typename Ind> inline void Evolver<Ind>::printGen() const
{
    if (verbose)
    {
        printf("\rGeneration [%3d]; Best=%.3e; Average=%.3e; Best genes=%s; ",
               generationStep, population->getBestFitness(),
               population->totalFitnessSum_orig / populationSize,
               toString(population->getBest()).c_str());
        if (population->speciation_threshold >= 0. && dissimilarity != nullptr)
        {
            printf("#species=%d; ", population->numSpecies());
            printf("\n");
            population->printFitness();
        }
        printf("Time=%lfs\n", C.L());
    }
}

template <typename Ind>
unsigned Evolver<Ind>::selectParent(const VecD &fitnessCumulative,
                                    int other) const
{
    double fitnessSum = fitnessCumulative.back();
    double target = generator() * fitnessSum;

    // original:
    // https://stackoverflow.com/questions/39416560/how-can-i-simplify-this-working-binary-search-code-in-c/39417165#39417165
    unsigned pos = ((other == 0 && fitnessCumulative.size() > 1) ? 1 : 0);
    unsigned limit = fitnessCumulative.size();
    while (pos < limit)
    {
        unsigned testpos = pos + ((limit - pos) >> 1);

        if (fitnessCumulative[testpos] < target)
            pos = testpos + 1;
        else
            limit = testpos;
    }
    if ((int)pos == other)
        pos = (other == (int)fitnessCumulative.size() - 1 ? 0 : pos + 1);

    return pos;
}

template <typename Ind>
unsigned Evolver<Ind>::findElite(Population<Ind> *offspring)
{
    if (population->speciation_threshold < 0. || dissimilarity == nullptr)
    { // no speciation - same as below, but faster
        // test both partial_sort and nth_element (to see what is better later)
        // std::partial_sort(population->species[0]->pop.begin(),
        // population->species[0]->pop.begin()+eliteCount,
        // population->species[0]->pop.end(), Species<Ind>::compareIndividuals);
        if (eliteCount)
            std::nth_element(population->species[0]->pop.begin(),
                             population->species[0]->pop.begin() + eliteCount -
                                 1,
                             population->species[0]->pop.end(),
                             Species<Ind>::compareIndividuals);
        // if(eliteCount)  std::sort(population->species[0]->pop.begin(),
        // population->species[0]->pop.end(), Species<Ind>::compareIndividuals);
        FOR(ind, eliteCount)
        addIndividual(offspring, population->species[0]->pop[ind]);
        return eliteCount;
    }
    else
    {
        unsigned count = 0; // counts how many are actually added (some may not
                            // be added if species is too small --> too bad :P)
        unsigned numSpecies = population->numSpecies();
        // order species first
        std::sort(population->species.begin(), population->species.end(),
                  Population<Ind>::compareSpecies);
        population->bestSpecies = 0;
        population->worseSpecies = numSpecies;

        // only sort within the species precisely what is necessary
        unsigned eliteCountPerSpeciesInt =
            eliteCount / numSpecies; // integer division
        unsigned breakPoint = eliteCount % numSpecies;

        FORV(i, numSpecies)
        {
            if ((population->species[i]->gensNoImprovement > oldAge ||
                 (generationStep > 1 &&
                  generationStep % obliterationRate == 0 &&
                  i == population->worseSpecies)) &&
                i != population->bestSpecies)
                continue;

            unsigned eliteCountPerSpecies =
                (i < breakPoint ? eliteCountPerSpeciesInt + 1
                                : eliteCountPerSpeciesInt);
            eliteCountPerSpecies =
                std::min(eliteCountPerSpecies, population->species[i]->size());

            if (!eliteCountPerSpecies)
                continue;

            // test both partial_sort and nth_element (to see what is better
            // later)
            // std::partial_sort(population->species[i]->pop.begin(),
            // population->species[i]->pop.begin()+eliteCountPerSpecies,
            // population->species[i]->pop.end(),
            // Species<Ind>::compareIndividuals);
            std::nth_element(population->species[i]->pop.begin(),
                             population->species[i]->pop.begin() +
                                 eliteCountPerSpecies - 1,
                             population->species[i]->pop.end(),
                             Species<Ind>::compareIndividuals);
            // std::sort(population->species[i]->pop.begin(),
            // population->species[i]->pop.end(),
            // Species<Ind>::compareIndividuals);
            FOR(ind, eliteCountPerSpecies)
            {
                addIndividual(offspring, population->species[i]->pop[ind], i);
                ++count;
            }
        }
        return count;
    }
}
template <typename Ind> void Evolver<Ind>::nullifyElite()
{
    if (population->speciation_threshold < 0. || dissimilarity == nullptr)
    { // no speciation - same as below, but faster
        FOR(ind, eliteCount) population->species[0]->pop[ind] = nullptr;
    }
    else
    {
        // only sort within the species precisely what is necessary
        unsigned numSpecies = population->numSpecies();
        unsigned eliteCountPerSpeciesInt =
            eliteCount / numSpecies; // integer division
        unsigned breakPoint = eliteCount % numSpecies;

        FORV(i, numSpecies)
        {
            if ((population->species[i]->gensNoImprovement > oldAge ||
                 (generationStep > 1 &&
                  generationStep % obliterationRate == 0 &&
                  i == population->worseSpecies)) &&
                i != population->bestSpecies)
                continue;

            unsigned eliteCountPerSpecies =
                (i < breakPoint ? eliteCountPerSpeciesInt + 1
                                : eliteCountPerSpeciesInt);
            eliteCountPerSpecies =
                std::min(eliteCountPerSpecies, population->species[i]->size());

            if (!eliteCountPerSpecies)
                continue; // unecessary, cause would skip for anyways, but here
                          // just to match the code in 'findElite'

            FOR(ind, eliteCountPerSpecies)
            population->species[i]->pop[ind] =
                nullptr; // directly copied to offspring, so can be made null
                         // here
        }
    }
}

template <typename Ind>
VecD Evolver<Ind>::calculateExpectedOffspring(unsigned eliteDone) const
{
    unsigned total = populationSize - eliteDone;

    if (population->speciation_threshold < 0. || dissimilarity == nullptr)
        return Vec<unsigned>(1, total);

    Vec<unsigned> offspring(population->numSpecies());
    unsigned partial_sum = 0;
    FORV(i, offspring.size())
    {
        // std::cout << "SPECIES ID=" << population->species[i]->id << ";
        // FITSUM=" << population->species[i]->fitnessSum << std::endl;
        offspring[i] = (unsigned)((total * population->species[i]->fitnessSum) /
                                  population->totalFitnessSum);
        partial_sum += offspring[i];
    }

    // distribute the rest (starting in best species just in case there's only 1
    // left)
    unsigned i = population->bestSpecies;
    while (partial_sum < total)
    {
        ++offspring[i];
        ++partial_sum;
        ++i;
        if (i == population->numSpecies())
            i = 0;
    }

    return offspring;
}

template <typename Ind> void Evolver<Ind>::eliminateWeak()
{
    if (survivalThreshold == 0.)
        return;

    // these variables are to make sure we don't delete anyone from elite
    unsigned numSpecies = population->numSpecies();
    unsigned eliteCountPerSpeciesInt =
        eliteCount / numSpecies; // integer division
    unsigned breakPoint = eliteCount % numSpecies;

    // This has to make sure that in the end there are at least 1! Don't forget
    // that there might be only one during reproduction
    FORV(s, population->numSpecies())
    {
        unsigned eliteCountPerSpecies =
            (s < breakPoint ? eliteCountPerSpeciesInt + 1
                            : eliteCountPerSpeciesInt);
        eliteCountPerSpecies =
            std::min(eliteCountPerSpecies, population->species[s]->size());

        unsigned eliminate = population->species[s]->size() * survivalThreshold;
        eliminate =
            std::min(eliminate, std::max(population->species[s]->size() - 2,
                                         (unsigned)0)); // at least one survives
        eliminate = std::min(
            eliminate, population->species[s]->size() -
                           eliteCountPerSpecies); // at least elite survives

        if (!eliminate)
            continue;

        // test both partial_sort and nth_element (to see what is better later)
        std::nth_element(population->species[s]->pop.begin(),
                         population->species[s]->pop.end() - eliminate,
                         population->species[s]->pop.end(),
                         Species<Ind>::compareIndividuals);
        // std::sort(population->species[s]->pop.begin(),
        // population->species[s]->pop.end(), Species<Ind>::compareIndividuals);
        // now that the worse 'eliminate' are in the end of the vector, delete
        // them
        for (unsigned i = population->species[s]->size() - 1, count = 0;
             count < eliminate; --i, ++count)
            population->species[s]->clear(i);

        population->species[s]->pop.erase(population->species[s]->size() -
                                              eliminate,
                                          population->species[s]->size() - 1);
    }
}

template <typename Ind> void Evolver<Ind>::generation()
{
    // double t1=0.,t2=0.,t3=0.;
    // Clock C2;
    // C2.setVerbose(false);

    Population<Ind> *offspring =
        new Population<Ind>(population->speciation_threshold);

    offspring->oldBest = population->getBestFitness();
    offspring->oldAverage = population->totalFitnessSum_orig / populationSize;

    eliminateWeak(); // it's ok to eliminate before doing
                     // 'calculateExpectedOffspring', cause the full sum of
                     // fitnesses is already saved

    unsigned eliteDone = eliteCount;
    if (eliteCount != 0)
        eliteDone = findElite(offspring);
    // t1+=C2.L();

    Vec<unsigned> expectedOffspring = calculateExpectedOffspring(eliteDone);
    // expectedOffspring.print();

    population->updateFitnessCumulative();
    VecD speciesCumulativeSum = population->speciesCumulativeSum();

    FORV(s, population->numSpecies())
    {
#pragma omp parallel for default(none)                                         \
    shared(s, population, crossoverProb, mutateProb, expectedOffspring,        \
           offspring, generator, std::cout, speciesCumulativeSum)              \
        schedule(static)
        for (unsigned ind = 0; ind < expectedOffspring[s]; ++ind)
        {

            unsigned parent1 =
                selectParent(population->species[s]->fitnessCumulative);

            Individual<Ind> *child;

            if (generator() <= crossoverProb)
            {
                if (population->speciation_threshold >= 0. &&
                    dissimilarity != nullptr && generator() < interSpeciesProb)
                {
                    unsigned speciesSelected =
                        selectParent(speciesCumulativeSum,
                                     s); // select a species other than 's'
                    unsigned parent2 = selectParent(
                        population->species[speciesSelected]
                            ->fitnessCumulative); // select an Individual
                                                  // from that species
                    // std::cout << "IN SPECIES " << s << " (" << parent1 <<
                    // "|"
                    // << population->species[s]->size() <<
                    // "); MATING WITH SPECIES " << speciesSelected << " ("
                    // << parent2 << "|" <<
                    // population->species[speciesSelected]->size() << ")" <<
                    // std::endl;
                    child = new Individual<Ind>(crossover(
                        population->species[s]->pop[parent1]->I,
                        population->species[speciesSelected]->pop[parent2]->I,
                        population->species[s]->pop[parent1]->fitness_orig,
                        population->species[speciesSelected]
                            ->pop[parent2]
                            ->fitness_orig));
                }
                else
                {
                    unsigned parent2 = selectParent(
                        population->species[s]->fitnessCumulative, parent1);
                    // std::cout << "PARENTS " << parent1 << " && " <<
                    // parent2
                    // << std::endl;
                    child = new Individual<Ind>(crossover(
                        population->species[s]->pop[parent1]->I,
                        population->species[s]->pop[parent2]->I,
                        population->species[s]->pop[parent1]->fitness_orig,
                        population->species[s]->pop[parent2]->fitness_orig));
                }
            }
            else
                child =
                    new Individual<Ind>(*population->species[s]->pop[parent1]);

            if (generator() <= mutateProb)
            {
                mutate(child->I, this);
                if (child->evaluated)
                    child->evaluated = false;
            }

            addIndividual(offspring, child);
        }
    }
    // t2+=C2.L();

    nullifyElite();

    clear();
    population = offspring;

    // t3+=C2.L();
    // printf("TIMES: %lf + %lf + %lf = %lf\n",t1,t2,t3,t1+t2+t3);
}

template <typename Ind> inline void Evolver<Ind>::initiatePopulation()
{
// populationSize shared by default, for being 'const'
#pragma omp parallel for default(none) shared(population) schedule(static)
    for (unsigned i = 0; i < populationSize; ++i)
        addIndividual(population, new Individual<Ind>(create(this)));
    if (verbose)
        std::cout << "Population initiated. Starting Evolution." << std::endl;
}

template <typename Ind> StopReason Evolver<Ind>::updateFitness()
{
    double bestFitnessOfAll = -std::numeric_limits<double>::infinity();
    double worseBestFitness = std::numeric_limits<double>::infinity();

    population->bestSpecies = 0;
    population->worseSpecies = 0;
    population->totalFitnessSum_orig = 0.;

    double maxfitness_orig = -std::numeric_limits<double>::infinity();
    double minfitness_orig = std::numeric_limits<double>::infinity();

    ProgressBar *pb = nullptr;
    if (verbose)
    {
        std::stringstream ss;
        ss << "Generation [" << std::setfill(' ') << std::setw(3)
           << generationStep << "];";
        pb = new ProgressBar(populationSize, ss.str(), 50);
    }

    FORV(s, population->numSpecies())
    {

        // age species by 1
        ++population->species[s]->age;
        ++population->species[s]->gensNoImprovement;

        population->species[s]->fitnessSum_orig = 0.;
        population->species[s]->fitnessSum = 0.;

        double fitnessSum_orig = 0.;

#pragma omp parallel for \
        default(none) \
        shared(s, pb) \
        reduction(+:fitnessSum_orig) \
        reduction(max:maxfitness_orig) \
        reduction(min:minfitness_orig) \
        schedule(static)
        for (unsigned i = 0; i < population->species[s]->size(); ++i)
        {
            Individual<Ind> *ind = population->species[s]->pop[i];
            if (ind != nullptr)
            {
                if (!ind->evaluated)
                {
                    ind->fitness_orig = evaluate(ind->I, this);
                    ind->evaluated = true;
                }

                fitnessSum_orig += ind->fitness_orig;

                if (ind->fitness_orig > maxfitness_orig)
                    maxfitness_orig = ind->fitness_orig;
                if (ind->fitness_orig <
                    minfitness_orig) // for i=0 this is also true, I think that
                                     // is why i didn't use 'else if'
                    minfitness_orig = ind->fitness_orig;
            }
            if (verbose)
                ++(*pb);
        }

        population->species[s]->fitnessSum_orig = fitnessSum_orig;
        population->totalFitnessSum_orig += fitnessSum_orig;
    }

    if (verbose)
        delete pb;

    // first calculate best and worse; then adjust
    FORV(s, population->numSpecies())
    {

        population->species[s]->bestRank = 0;
        population->species[s]->bestFitness =
            -std::numeric_limits<double>::infinity();

        FORV(i, population->species[s]->size())
        {
            Individual<Ind> *ind = population->species[s]->pop[i];
            if (ind != nullptr)
            {
                // invert fitness_origes for minimize
                ind->fitness =
                    (obj == MAXIMIZE ? ind->fitness_orig - minfitness_orig
                                     : maxfitness_orig - ind->fitness_orig);

                // calculate best and worse before adjusting fitness
                if (ind->fitness > population->species[s]->bestFitness)
                {
                    population->species[s]->bestRank = i;
                    population->species[s]->bestFitness = ind->fitness;
                }
            }
        }

        if (population->species[s]->bestFitness > bestFitnessOfAll)
        {
            population->bestSpecies = s;
            bestFitnessOfAll = population->species[s]->bestFitness;
        }
        if (population->species[s]->bestFitness < worseBestFitness)
        {
            population->worseSpecies = s;
            worseBestFitness = population->species[s]->bestFitness;
        }
        if (population->species[s]
                ->pop[population->species[s]->bestRank]
                ->fitness_orig > population->species[s]->bestFitnessEver_orig *
                                     (obj == MAXIMIZE ? 1. : -1.))
        {
            population->species[s]->bestFitnessEver_orig =
                population->species[s]
                    ->pop[population->species[s]->bestRank]
                    ->fitness_orig;
            population->species[s]->gensNoImprovement = 0;
        }
    }
    FORV(s, population->numSpecies())
    {
        FORV(i, population->species[s]->size())
        {
            Individual<Ind> *ind = population->species[s]->pop[i];
            if (ind != nullptr)
            {
                /////////////////////////////////////////////
                // adjust fitness to population size
                // this makes the champion not necessarily the one with the best
                // fitness
                ind->fitness /= population->species[s]->size();
                if ((population->species[s]->gensNoImprovement > oldAge ||
                     (generationStep > 1 &&
                      generationStep % obliterationRate == 0 &&
                      s == population->worseSpecies)) &&
                    s != population->bestSpecies)
                    ind->fitness *= oldPenalty;
                if (population->species[s]->age < youthAge)
                    ind->fitness *= youthBoost;
                /////////////////////////////////////////////

                population->species[s]->fitnessSum += ind->fitness;
            }
        }
        population->totalFitnessSum += population->species[s]->fitnessSum;
    }

    double newBest = population->getBestFitness();
    double newAverage = population->totalFitnessSum_orig / populationSize;
    return criteria.update(generationStep, population->oldBest, newBest,
                           population->oldAverage, newAverage);
}

template <typename Ind> void Evolver<Ind>::checkSettings() const
{
    if (create == nullptr)
        errorMsg("create() is not set.");
    if (crossover == nullptr)
        errorMsg("crossover() is not set.");
    if (mutate == nullptr)
        errorMsg("mutate() is not set.");
    if (evaluate == nullptr)
        errorMsg("evaluate() is not set.");
    if (toString == nullptr)
        errorMsg("toString() is not set.");
    if (dissimilarity == nullptr && population->speciation_threshold >= 0.)
        errorMsg("dissimilarity() is not set.");

    if (populationSize < 1)
        errorMsg("populationSize is below 1.");
    if (eliteCount > populationSize)
        errorMsg("eliteCount bigger than populationSize.");
    if (obj == NONE)
        errorMsg("Objective not set.");

    if (crossoverProb < 0. || crossoverProb > 1.)
        errorMsg("Invalid crossover fraction.");
    if (mutateProb < 0. || mutateProb > 1.)
        errorMsg("Invalid mutation rate.");
    if (oldPenalty < 0. || oldPenalty > 1.)
        errorMsg("Old Penalty should be in [0,1].");
    if (youthBoost < 1.)
        errorMsg("Youth Boost should be bigger than 1.");
    if (oldAge < youthAge)
        errorMsg("Old age can't be smaller than youth age.");
    if (obliterationRate < 1)
        errorMsg("obliterationRate must be bigger than 0.");
    if (interSpeciesProb < 0. || interSpeciesProb > 1.)
        errorMsg("Inter Species Probability should be in [0,1].");
}

template <typename Ind>
void Evolver<Ind>::addIndividual(Population<Ind> *pop, Individual<Ind> *I,
                                 int species)
{

    int chosen =
        (species >= 0 ? species : 0); // int because there's a 'for' below which
                                      // is reversed and ends in -1
    // automatically add to chosen species - used for elite members, not to have
    // to calculate dissimilarity again
    if (species >= 0)
    {
        chosen = pop->addSpecies(
            population->species[species]->id, population->species[species]->age,
            population->species[species]->gensNoImprovement,
            population->species[species]->bestFitnessEver_orig);
    }
    else if (species < 0 && pop->speciation_threshold >= 0. &&
             dissimilarity != nullptr)
    { // no speciation
        unsigned s;
        for (s = 0; s < population->numSpecies(); ++s)
            if (dissimilarity(I->I, population->species[s]->pop[0]->I) <=
                pop->speciation_threshold)
                break;

        if (s == population->numSpecies())
        {
            // run throw pop species which are not present in population and
            // check for compatibilities not checked yet (of the new species
            // that
            // pop has)
            for (chosen = pop->numSpecies() - 1;
                 chosen >= 0 && pop->species[chosen]->id > population->last_id;
                 --chosen)
                if (dissimilarity(I->I, pop->species[chosen]->pop[0]->I) <=
                    pop->speciation_threshold)
                    break;

            if (chosen < 0 || pop->species[chosen]->id <= population->last_id)
                chosen = pop->addSpecies(
                    std::max(population->last_id, pop->last_id) +
                    1); // new Species
        }
        else
        {
            // printf("%.1e ", dissimilarity( I->I ,
            // population->species[s]->pop[0]->I));
            chosen = pop->addSpecies(
                population->species[s]->id, population->species[s]->age,
                population->species[s]->gensNoImprovement,
                population->species[s]->bestFitnessEver_orig);
        }
    }

    pop->species[chosen]->addIndividual(I);
}

template <typename Ind>
inline void Evolver<Ind>::setCreate(Ind (*func)(const Evolver<Ind> *))
{
    create = func;
}
template <typename Ind>
inline void Evolver<Ind>::setCrossover(Ind (*func)(const Ind &, const Ind &,
                                                   double fit1, double fit2))
{
    crossover = func;
}
template <typename Ind>
inline void Evolver<Ind>::setMutate(void (*func)(Ind &, const Evolver<Ind> *))
{
    mutate = func;
}
template <typename Ind>
inline void Evolver<Ind>::setEvaluate(double (*func)(const Ind &,
                                                     const Evolver<Ind> *),
                                      Objective o)
{
    evaluate = func;
    obj = o;
}
template <typename Ind>
inline void Evolver<Ind>::setToString(std::string (*func)(const Ind &))
{
    toString = func;
}
template <typename Ind>
inline void Evolver<Ind>::setDissimilarity(double (*func)(const Ind &,
                                                          const Ind &))
{
    dissimilarity = func;
}
template <typename Ind>
inline void
Evolver<Ind>::setPrintPopulation(bool (*func)(const Population<Ind> *))
{
    printPopulation = func;
}

#endif
