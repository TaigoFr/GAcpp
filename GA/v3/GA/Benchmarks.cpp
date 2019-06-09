
#include "Tools.hpp" //errorMsg.hpp
#include "Benchmarks.hpp"
#include <cmath>

double Benchmarks::sphere(const VecD& v, const GA::Evolver<VecD>*) { return v.norm()/sqrt(v.size()); }
double Benchmarks::rosenbrock(const VecD& v, const GA::Evolver<VecD>*){
	if(v.size() < 2)
		errorMsg("VecD minimum size is 2.");
	return 100.*(v[0]*v[0]-v[1])*(v[0]*v[0]-v[1])+(1.-v[0])*(1.-v[0]);
}
double Benchmarks::step(const VecD& v, const GA::Evolver<VecD>*){
	double out = 0.;
	for(unsigned i=0; i<v.size(); ++i)
		out += (int)(v[i]);
	return out;
}
double Benchmarks::rastrigin(const VecD& v, const GA::Evolver<VecD>*){
	double out = 10.*v.size();
	for(unsigned i=0; i<v.size(); ++i)
		out += v[i]*v[i]-10.*cos(2.*M_PI*v[i]);
	return out;
}
double Benchmarks::foxholes(const VecD& v, const GA::Evolver<VecD>*){
	static double aij[2][25] = {{-32,16,0,16,32, -32,16,0,16,32, -32,16,0,16,32, -32,16,0,16,32, -32,16,0,16,32},{-32,-32,-32,-32,-32, -16,-16,-16,-16,-16, 0,0,0,0,0, 32,32,32,32,32, 16,16,16,16,16}};

	if(v.size() < 2)
		errorMsg("VecD minimum size is 2.");

	double out = 0.002;
	for(unsigned j=0; j<25; ++j)
		out += 1./(j + pow(v[0]-aij[0][j],6) + pow(v[1]-aij[1][j],6));
	
	return 1./out;
}


double Benchmarks::demo1(const VecD& v, const GA::Evolver<VecD>*){
	if(v.size() < 2)
		errorMsg("VecD minimum size is 2.");

	double x = v[0], y = v[1];

	// https://academo.org/demos/3d-surface-plotter/?expression=x*x%2By*y%2B30.0*sin(x*100.0*sin(y)%2By*100.0*cos(x))%2B125%2B45.0*sqrt(x%2By)*sin((15.0*(x%2By))%2F(x*x%2By*y))&xRange=-10%2C%2B10&yRange=-10%2C%2B10&resolution=100
	return 30.0*sin(x*100.0*sin(y)+y*100.0*cos(x)) + x*x + y*y + 125 + 45.0*sqrt(x+y)*sin((15.0*(x+y))/(x*x+y*y));
}