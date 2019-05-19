
#pragma once

#include "Vec.hpp"
#include "Evolver.hpp"

namespace Benchmarks{

	double sphere(const VecD&, const GA::Evolver<VecD>*);
	double rosenbrock(const VecD&, const GA::Evolver<VecD>*);
	double step(const VecD&, const GA::Evolver<VecD>*);
	double rastrigin(const VecD&, const GA::Evolver<VecD>*);
	double foxholes(const VecD&, const GA::Evolver<VecD>*);
	double demo1(const VecD&, const GA::Evolver<VecD>*);

};