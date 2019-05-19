
#include "GA.hpp"
#include "IndividualVec.hpp"

int main(){

	GA<IndividualVec> ga(10);
	ga.evolve(1000000,1.,0.1);

	return 0;
}