
#include "Random.hpp"

#include <chrono>

Random::Random(){
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	generator.seed(seed);
	// generator.seed(2);
}

int Random::getI(int min, int max){
	if(max==min)
		return min;
	if(max<min)
		return generator() % (generator.max() - min + 1) + min;
	return generator() % (max - min + 1) + min;
}

unsigned Random::getU(unsigned min, unsigned max){
	if(max<=min)
		return generator() % (generator.max() - min + 1) + min;
	return generator() % (max - min + 1) + min;
}

double Random::getN(double mean, double sigma){
	static bool saved = false;
	static double z2 = 0.;

	if(saved) {
		saved=false;
		return mean + sigma * z2;
	}
	saved=true;

	double u1 = getD();
	double u2 = getD();

	if(u1==0.) {z2=0.; return 0.;}
	
	z2 			= sqrt(-2.*log(u1))*sin(2.*M_PI*u2);
	double z1 	= sqrt(-2.*log(u1))*cos(2.*M_PI*u2);

	return mean + sigma * z1;
}