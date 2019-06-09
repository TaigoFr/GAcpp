#include "ODEsolver.hpp"

#include <cmath>
#include <iostream>

bool ODEsolver::check(const VecD& init){
	if(((unsigned) init.size()-1) != dim )
		return true;
	return false;
}

VecD ODEsolver::step(const VecD& init, double &dt, double derr){
	if(method==ODEmethod::RK45)
		return stepRK45(init,dt,derr);

	switch(method){
		case ODEmethod::Euler: 	return stepEuler(init, dt);
		case ODEmethod::Heun: 	return stepHeun (init, dt);
		case ODEmethod::RK2: 	return stepRK2	(init, dt);
		case ODEmethod::RK4: 	return stepRK4 	(init, dt);
		default: throw std::runtime_error("[ODEsolver::step] You shouldn't be here."); break;
	}
}

MatrixD ODEsolver::solve(const VecD& init, double T, double dt, double derr){
	if(check(init)){
		MatrixD temp;
		std::cerr << "[ODEsolver::solve] ERRO: Initial variables size incompatible with ODEsolver dim" << std::endl;
		return temp;
	}
	MatrixD v;
	if(T<=0. || dt<=0.) return v;
	v.reserve(1+ceil(T/dt));
	v.push_back(init);
	double t = v.back()[0];

	if(method==ODEmethod::RK45){
		while(t<T){
			v.push_back(stepRK45(v.back(), dt, derr));
			t+=dt;
		}
	}
	else{
		typedef VecD (ODEsolver::*mbFunc)(const VecD& init, double dt);
		mbFunc stepFunc;
		switch(method){
			case Euler: stepFunc = &ODEsolver::stepEuler; break;
			case Heun: 	stepFunc = &ODEsolver::stepHeun ; break;
			case RK2: 	stepFunc = &ODEsolver::stepRK2	; break;
			case RK4: 	stepFunc = &ODEsolver::stepRK4 	; break;
			default: throw std::runtime_error("[ODEsolver::step] You shouldn't be here."); break;
		}

		while(t<T){
			v.push_back((this->*stepFunc)(v.back(), dt));
			t+=dt;
		}
	}

	return v;
}
MatrixD ODEsolver::solve(VecD&& init, double T, double dt, double derr){
	if(check(init)){
		MatrixD temp;
		std::cerr << "[ODEsolver::solve] ERRO: Initial variables size incompatible with ODEsolver dim" << std::endl;
		return temp;
	}
	MatrixD v;
	if(T<=0. || dt<=0.) return v;
	v.reserve(1+ceil(T/dt));
	v.push_back((VecD&&)init);
	double t = v.back()[0];

	if(method==ODEmethod::RK45){
		while(t<T){
			v.push_back(stepRK45(v.back(), dt, derr));
			t+=dt;
		}
	}
	else{
		typedef VecD (ODEsolver::*mbFunc)(const VecD& init, double dt);
		mbFunc stepFunc;
		switch(method){
			case Euler: stepFunc = &ODEsolver::stepEuler; break;
			case Heun: 	stepFunc = &ODEsolver::stepHeun ; break;
			case RK2: 	stepFunc = &ODEsolver::stepRK2	; break;
			case RK4: 	stepFunc = &ODEsolver::stepRK4 	; break;
			default: throw std::runtime_error("[ODEsolver::step] You shouldn't be here."); break;
		}

		while(t<T){
			v.push_back((this->*stepFunc)(v.back(), dt));
			t+=dt;
		}
	}

	return v;
}

VecD ODEsolver::stepEuler(const VecD& init, double dt)
{
	//y_j+1 = y_j + h*F(y_j)
	VecD ponto = init + dt * RHS(init, params);
	ponto[0] = init[0] + dt;
	return ponto;
}

VecD ODEsolver::stepHeun(const VecD& init, double dt)
{
	//temp = y_j + h*F(y_j)
	VecD ponto = init + dt * RHS(init, params);
	ponto[0] = init[0] + dt;

	//y_j+1 = y_j + h/2*(F(y_j)+F(temp))
	VecD ponto2 = init + dt/2. * ( RHS(init, params) + RHS(ponto, params) );
	ponto2[0] = init[0] + dt;

	return ponto2;
}

VecD ODEsolver::stepRK2(const VecD& init, double dt)
{
	//temp = y_j + h/2*F(y_j)
	VecD K1 = init + dt/2. * RHS(init, params);
	K1[0] = init[0] + dt/2;

	// dt*F(K1) := K2
	//y_j+1 = y_j + h*F(temp)
	VecD ponto = init + dt * RHS(K1, params);
	ponto[0] = init[0] + dt;

	return ponto;
}

VecD ODEsolver::stepRK4(const VecD& init, double dt)
{
	//K1 = h*F(y_j)
	VecD K1 = dt*RHS(init, params);
	K1[0] = dt;

	//K2 = h*F(y_j+K1/2)
	VecD K2 = dt*RHS(init + K1*.5, params);
	K2[0] = dt;

	//K3 = h*F(y_j+K2/2)
	VecD K3 = dt*RHS(init + K2*.5, params);
	K3[0] = dt;

	//K4 = h*F(y_j+K3)
	VecD K4 = dt*RHS(init + K3, params);
	K4[0] = dt;

	//y_j+1 = y_j + (K1+2K2+2K3+K4)/6.
	VecD ponto = init + 1./6.*( K1 + 2.*K2 + 2.*K3 + K4);
	ponto[0] = init[0] + dt;
		
	return ponto;
}


VecD ODEsolver::stepRK45(const VecD& init, double &step, double derr)
{

	static const double B[] = {0.2         , 
					0.075       , 0.225   ,
					0.3         , -0.9    , 1.2        ,
					-11./54.    , 2.5     , -70./27.   , 35./27.       ,
					1631./55296., 175./512, 575./13824., 44275./110592., 253./4096.};

	static const double C[] = {37./378.     , 0. , 250./621.     , 125./594.     , 0.           , 512./1771.};

	static const double D[] = {2825./27648. , 0. , 18575./48384. , 13525./55296. , 277./14336.  , 0.25      };

	double error;
	VecD ponto4th, ponto5th;
	do
	{
		VecD K0(init.size());
		K0[0] = step;

		MatrixD K(6,K0);

		VecD temp;
		for (unsigned j = 0; j < K.size(); ++j)
		{
			temp = init;
			for (unsigned n = 0; n < j; ++n)
				temp += K[n]*B[n+(j-1)*(j)/2];
			K[j]    = step*RHS(temp, params);
			K[j][0] = step;
		}

		ponto5th = init;
		for (unsigned i = 0; i < K.size(); ++i)
			ponto5th += C[i]*K[i];

		ponto4th = init;
		for (unsigned i = 0; i < K.size(); ++i)
			ponto4th += D[i]*K[i];

		double pow2E = 0;
		for (unsigned i = 0; i < dim; ++i)
			pow2E += (ponto5th[i+1]-ponto4th[i+1])*(ponto5th[i+1]-ponto4th[i+1]);

		error = sqrt(pow2E/dim);

		step *= (error != 0) ? 0.9 * pow(derr/error,0.2) : 10;

	}while(error > derr);

	return ponto5th;
}

