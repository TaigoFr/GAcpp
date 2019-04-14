
#include "Evolver.hpp"
#include "Benchmarks.hpp"

static unsigned N = 2;

//same for sphere
VecD create_rastrigin(){
	VecD out(N);
	for(unsigned i=0; i<N; ++i)
		out[i] = GA::generator(-5.12,5.12);
	return out;
}

VecD create_demo1(){
	VecD out(N);
	for(unsigned i=0; i<N; ++i)
		out[i] = GA::generator(0,10);
	return out;
}

VecD mate(const VecD& I1,const VecD& I2){ 
	VecD out(I1.size());
	for(unsigned i=0; i<out.size(); ++i){
		double r = GA::generator();
		out[i] = r*I1[i] + (1.-r)*I2[i];
	}
	return out; 
}

void   mutate_rastrigin(VecD &I){
	bool out_of_range;
	do{
		out_of_range=false;
		
		for(unsigned i=0; i<I.size(); ++i)
		{
			double mu=GA::generator(0.67,1.5);
			I[i] *= mu*(GA::generator() - GA::generator());
			if(fabs(I[i])>5.12)
				out_of_range=true;
		}
	} while(out_of_range);
}
void   mutate_demo1(VecD &I){
	bool out_of_range;
	do{
		out_of_range=false;
		
		for(unsigned i=0; i<I.size(); ++i)
		{
			double mu=GA::generator(0.67,1.5);
			I[i] *= mu*(GA::generator() - GA::generator());
			if(I[i]>10 || I[i]<0)
				out_of_range=true;
		}
	} while(out_of_range);
}
std::string   toString(const VecD &I)	{ return I.toString(); }

int main(){

	// GA::Evolver<VecD> ga1(20);
	// ga1.setCreate(create_demo1);
	// ga1.setMate(mate);
	// ga1.setEvaluate(Benchmarks::demo1, GA::MINIMIZE);
	// ga1.setMutate(mutate_demo1);
	// ga1.setToString(toString);
	// ga1.evolve(10,0.7,0.4);

	// GA::Evolver<VecD> ga2(10000);
	GA::Evolver<VecD> ga2(10000000);
	ga2.setCreate(create_rastrigin); //same for sphere
	ga2.setMate(mate);
	ga2.setEvaluate(Benchmarks::rastrigin, GA::MINIMIZE);
	ga2.setMutate(mutate_rastrigin);
	ga2.setToString(toString);
	ga2.numThreads = 0;
	ga2.evolve(10,0.7,0.1);

	// GA::Evolver<VecD> ga3(100);
	// ga3.setCreate(create_rastrigin); //same for sphere
	// ga3.setMate(mate);
	// ga3.setEvaluate(Benchmarks::sphere, GA::MINIMIZE);
	// ga3.setMutate(mutate_rastrigin);
	// ga3.setToString(toString);
	// ga3.evolve(5,0.6,0.001);

	return 0;
}