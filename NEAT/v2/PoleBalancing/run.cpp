
#include "NEAT_GA.hpp"
#include "Image.hpp"
#include "ODEsolver.hpp"

#define toRAD (M_PI/180.)
#define g 9.8
#define M 1.0
#define m 0.1
#define L 0.5
#define thetaMAX (36.*toRAD)
#define xMAX 2.5
#define thetaDMAX 1.5
#define xDMAX 1.0
#define F_MAGN 10.0

#define TMAX (30.*60.)
#define dt 0.01

//variables explanation
//x[0] = t
//x[1] = theta
//x[2] = x
//x[3] = theta.
//x[4] = x.
VecD invertedPendulumRHS(const VecD& x, const VecD& params){
	double F = params[0];

	return VecD({1,
				x[3],
				x[4],
				((m*g*sin(x[1])*cos(x[1])-m*L*x[3]*x[3]*sin(x[1])+F)/(M+m*sin(x[1])*sin(x[1]))*cos(x[1]) + g*sin(x[1]))/L,
				 (m*g*sin(x[1])*cos(x[1])-m*L*x[3]*x[3]*sin(x[1])+F)/(M+m*sin(x[1])*sin(x[1]))
				});
}

//inputs and outputs normalized between [0,1]
double pole(const NEAT::Network& net, const GA::Evolver<NEAT::Network>*){
	VecD x0(5);
	x0[1]=GA::generator(-thetaMAX,thetaMAX)*0.5;
	x0[2]=GA::generator(-xMAX,xMAX)*0.95; //make sure it starts inside the track
	x0[3]=GA::generator(-thetaDMAX,thetaDMAX);
	x0[4]=GA::generator(-xDMAX,xDMAX);

	ODEsolver eq(invertedPendulumRHS, 4);

	double t=0.;

	while(t<TMAX && fabs(x0[1])<=thetaMAX && fabs(x0[2])<=xMAX){ //theta <= 36 degrees, x <= 2.5m
		VecD inputs(4);
		//normalize to [0,1]
		inputs[0] = (x0[1]/thetaMAX 	+ 1.)/2.;
		inputs[1] = (x0[2]/xMAX 		+ 1.)/2.;
		inputs[2] = (x0[3]/thetaDMAX 	+ 1.)/2.;
		inputs[3] = (x0[4]/xDMAX 		+ 1.)/2.;
		double F = F_MAGN * (2.*net.evaluate(inputs)[0] - 1.);
		eq.setParams({F});
		x0 = eq.stepRK4(x0, dt);
		t+=dt;
	}

	return t;
}

int main(){

	NEAT::Evolver ga("params.txt");
	ga.addCriteria(GA::StopReason::BestMaximum, {TMAX});
	ga.setEvaluate(pole, GA::MAXIMIZE);

	ga.evolve();

	const NEAT::Network *best = &ga.getBest();
	best->print();
	print("Best Fitness = ",ga.getBestFitness());

	// pole2(*best,nullptr);
	// best->write("pole_best.txt");

	NEAT::Image image(*best,"params_image.txt");
	image.evolve();
	image.draw();
	// image.save();
	image.wait();

	return 0;
}