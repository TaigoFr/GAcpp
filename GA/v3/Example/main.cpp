
#include "Evolver.hpp"
#include "Benchmarks.hpp"

static unsigned N = 5;

//same for sphere
VecD create_rastrigin(const GA::Evolver<VecD>*){
	VecD out(N);
	for(unsigned i=0; i<N; ++i)
		out[i] = GA::generator(-5.12,5.12);
	return out;
}

VecD create_demo1(const GA::Evolver<VecD>*){
	VecD out(N);
	for(unsigned i=0; i<N; ++i)
		out[i] = GA::generator(0,10);
	return out;
}

VecD crossover(const VecD& I1,const VecD& I2, double, double){ 
	VecD out(I1.size());
	for(unsigned i=0; i<out.size(); ++i){
		double r = GA::generator();
		out[i] = r*I1[i] + (1.-r)*I2[i];
	}
	return out; 
}

void   mutate_rastrigin(VecD &I, const GA::Evolver<VecD>*){
	bool out_of_range;

	for(unsigned i=0; i<I.size(); ++i){
		do{
			out_of_range=false;
			double mu=GA::generator(0.9/1.2,1.2);
			I[i] *= mu;
			if(fabs(I[i])>5.12)
				out_of_range=true;
		} while(out_of_range);
	}
}
void   mutate_demo1(VecD &I, const GA::Evolver<VecD>*){
	bool out_of_range;
	
	for(unsigned i=0; i<I.size(); ++i){
		do{
			out_of_range=false;
			double mu=GA::generator(0.9/1.5,1.5);
			I[i] *= mu;
			if(I[i]>10 || I[i]<0)
				out_of_range=true;
		} while(out_of_range);
	}
}
std::string   toString(const VecD &I)	{ return I.toString(); }

double		dissimilarity(const VecD& v1, const VecD& v2){
	return (v1-v2).norm()/v1.size();
}

int main(){

/*	GA::Evolver<VecD> ga1(20);
	ga1.setCreate(create_demo1);
	ga1.setCrossover(crossover);
	ga1.setEvaluate(Benchmarks::demo1, GA::MINIMIZE);
	ga1.setMutate(mutate_demo1);
	ga1.setToString(toString);
	ga1.evolve(10,0.7,0.4);*/

	GA::Evolver<VecD> ga2(10000, 2);

	ga2.setCreate(create_rastrigin); //same for sphere
	ga2.setCrossover(crossover);
	ga2.setEvaluate(Benchmarks::rastrigin, GA::MINIMIZE);
	ga2.setMutate(mutate_rastrigin);
	ga2.setToString(toString);
	ga2.setDissimilarity(dissimilarity);
	// ga2.numThreads = 7;

	ga2.addCriteria(GA::StopReason::MaxGenerations, 		{100});		//stop after 100 generations
	// ga2.addCriteria(GA::StopReason::BestStalled, 			{1.e-8, 15});
	ga2.addCriteria(GA::StopReason::BestRelativeStalled, 	{0.1, 15});	//stop after 15 generations with less than 10% improvement

	ga2.evolve(10,0.7,0.1); //evolve with 10 elite members, 70% of reproduction probability and 10% of mutation probability

	std::cout << toString(ga2.getBest()) << std::endl;
	std::cout << "F = " << ga2.getBestFitness() << std::endl;

/*	GA::Evolver<VecD> ga3(100);
	ga3.setCreate(create_rastrigin); //same for sphere
	ga3.setCrossover(crossover);
	ga3.setEvaluate(Benchmarks::sphere, GA::MINIMIZE);
	ga3.setMutate(mutate_rastrigin);
	ga3.setToString(toString);
	ga3.evolve(5,0.6,0.001);*/

	return 0;
}