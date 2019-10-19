
#include "NEAT_GA.hpp"
#include "Image.hpp"
#include "ODEsolver.hpp"

#define toRAD (M_PI/180.)
#define g 9.8
#define M 1.0
#define m1 0.01
#define m2 0.1
#define L1 0.1
#define L2 1.0
#define thetaMAX (36.*toRAD)
#define xMAX 2.5
#define thetaDMAX 2.
#define xDMAX 2.
#define F_MAGN 10.0

#define TMAX (15.*60.)
#define dt 0.01

//variables explanation
//x[0] = t
//x[1] = theta1
//x[2] = theta2
//x[3] = x
//x[4] = theta1.
//x[5] = theta2.
//x[6] = x.
VecD invertedPendulumRHS(const VecD& x, const VecD& params){
	double F = params[0];

	double xpp = (m1*(g*sin(x[1])+0.000002*x[4]/(m1*L1))*cos(x[1])-m1*L1*x[4]*x[4]*sin(x[1])
				 +m2*(g*sin(x[2])+0.000002*x[5]/(m2*L2))*cos(x[2])-m2*L2*x[5]*x[5]*sin(x[2])+F)/(M+m1*sin(x[1])*sin(x[1])+m2*sin(x[2])*sin(x[2]));

	return VecD({1,
				x[4],
				x[5],
				x[6],
				(xpp*cos(x[1]) + g*sin(x[1]))/L1,
				(xpp*cos(x[2]) + g*sin(x[2]))/L2,
				xpp
				});
}

//inputs and outputs normalized between [0,1]
double pole(const NEAT::Network& net, const GA::Evolver<NEAT::Network>*){
	VecD x0(7);
	x0[1] = 0. * toRAD;
	x0[2] = 1. * toRAD;
/*	x0[1]=GA::generator(-thetaMAX,thetaMAX)*0.5;
	x0[2]=GA::generator(-thetaMAX,thetaMAX)*0.5;
	x0[3]=GA::generator(-xMAX,xMAX)*0.95; //make sure it starts inside the track
	x0[4]=GA::generator(-thetaDMAX,thetaDMAX);
	x0[5]=GA::generator(-thetaDMAX,thetaDMAX);
	x0[6]=GA::generator(-xDMAX,xDMAX);*/

	ODEsolver eq(invertedPendulumRHS, 6);

	double t=0.;

	while(t<TMAX && fabs(x0[1])<=thetaMAX && fabs(x0[2])<=thetaMAX && fabs(x0[3])<=xMAX){ //theta <= 36 degrees, x <= 2.5m
		VecD inputs(6);
		//normalize to [-1,1]
		inputs[0] = x0[1]/thetaMAX;
		inputs[1] = x0[2]/thetaMAX;
		inputs[2] = x0[3]/xMAX/2.;	// [-0.5, 0.5]
		inputs[3] = x0[4]/thetaDMAX;
		inputs[4] = x0[5]/thetaDMAX;
		inputs[5] = x0[6]/xDMAX;
/*		//normalize to [0,1]
		inputs[0] = (x0[1]/thetaMAX 	+ 1.)/2.;
		inputs[1] = (x0[2]/thetaMAX 	+ 1.)/2.;
		inputs[2] = (x0[3]/xMAX 		+ 1.)/2.;
		inputs[3] = (x0[4]/thetaDMAX 	+ 1.)/2.;
		inputs[4] = (x0[5]/thetaDMAX 	+ 1.)/2.;
		inputs[5] = (x0[6]/xDMAX 		+ 1.)/2.;*/
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