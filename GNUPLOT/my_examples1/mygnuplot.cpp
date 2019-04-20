
#include "gnuplot-iostream.h"
#include "Matrix.hpp"

#include <cmath>

double func(double x){
	return x*x - 10.*cos(2.*M_PI*x);
}

void plot(){
	const int num_steps = 100;
	const double min = -5., max = 5.;

	// std::vector<std::pair<double, double> > pts;
	// for(double x=min; x<max; x+=(max-min)/num_steps) {
	// 	pts.push_back(std::make_pair(x, func(x)));
	// }
	// Vec<VecD> pts2;
	// for(double x=min; x<max; x+=(max-min)/num_steps) {
	// 	pts2.push_back(VecD(2,x, func(x)));
	// }
	MatrixD pts2;
	for(double x=min; x<max; x+=(max-min)/num_steps) {
		pts2.push_back(VecD(4,x, func(x),1,1));
	}

	Gnuplot gp;
	gp << "set xrange [-5:5]\n";
	gp << "set title 'Energy vs. Time for Sample Data'\n";
	gp << "plot ";
	gp << gp.binFile1d(pts2, "record") << "with xyerrorbars title 'vector of std::pair'";
	gp << std::endl;
}

int main(){
	plot();

	return 0;
}