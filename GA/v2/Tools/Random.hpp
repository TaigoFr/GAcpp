
#pragma once

#include <random>

// generator.min() assumed to be 0

class Random{
public:
	Random();

	double 		operator()	(double min = 0., double max = 1.);
	double 		getD		(double min = 0., double max = 1.);
	int 		getI		(int min = 0, int max = 0);
	unsigned 	getU		(unsigned min = 0, unsigned max = 0);
	double 		getN 		(double mean = 0., double sigma = 1.);

private:
	std::mt19937 generator;
	// std::mt19937_64 generator; // also compatible, just change it
};