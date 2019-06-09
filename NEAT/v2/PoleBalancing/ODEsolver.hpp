#ifndef _ODESOLVER_H_
#define _ODESOLVER_H_

#include "Matrix.hpp"

//list of available methods
enum ODEmethod{
	Euler,
	Heun,
	RK2,
	RK4,
	RK45
};

//Class for resolution of systems of diferential equations
class ODEsolver
{
public:
	inline ODEsolver (VecD (*_RHS)(const VecD& vars, const VecD& params), unsigned _dim, unsigned _nParams=0): 
		dim(_dim), nParams(_nParams), RHS(_RHS), params(_nParams,0.), method(ODEmethod::RK4) {}

	bool 	check(const VecD& init);
	VecD 	step (const VecD& init, double &dt, double derr=0.01); //dt is changed in RK45, derr is only for RK45
	MatrixD solve(const VecD& init, double T, double dt, double derr=0.01); //derr is only for RK45
	MatrixD solve(VecD&& init, 		double T, double dt, double derr=0.01); //derr is only for RK45
	MatrixD solve(ODEmethod met, const VecD& init,  double T, double dt, double derr=0.01){ setMethod(met); return solve(init,T,dt,derr); }
	MatrixD solve(ODEmethod met, VecD&& init, 		double T, double dt, double derr=0.01){ setMethod(met); return solve((VecD&&)init,T,dt,derr); }

	inline void setMethod(ODEmethod met){ method = met; }

	inline void setParams(const VecD& _params) { params = _params; }
	inline void setParams(VecD&& 	  _params) { params = (VecD&&)_params; }

	VecD stepEuler(const VecD& i, double step=0.01);
	VecD stepHeun (const VecD& i, double step=0.01);
	VecD stepRK2  (const VecD& i, double step=0.01);
	VecD stepRK4  (const VecD& i, double step=0.01);
	VecD stepRK45 (const VecD& i, double &step, double derr=0.01);

private:
	unsigned dim, nParams;
	VecD (*RHS)(const VecD& vars, const VecD& params);
	VecD params;
	ODEmethod method;
};

#endif