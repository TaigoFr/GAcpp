#include "Graphics.hpp"
#include <cmath>


double func(double x){
	return x*x - 10.*cos(2.*M_PI*x);
}

void plot(){
	const int num_steps = 100;
	const double min = -5., max = 5.;

	MatrixD pts2;
	for(double x=min; x<max; x+=(max-min)/num_steps) {
		pts2.push_back(VecD(4,x, func(x),1,1));
	}

	Graphics G(false);

	G.setXRange(-5,5);
	G.write("set title 'Energy vs. Time for Sample Data'\n");
	G.write("plot ");
	G.add(pts2).write("with xyerrorbars title 'vector of std::pair'");
	G.plot();

	G.clear();
	G.create();
	G.setXRange(-5,5);
	G.write("set title 'Energy vs. Time for Sample Data'\n");
	G.write("plot ");
	G.add(pts2).write("with xyerrorbars title 'vector of std::pair'");
	G.plot();
}

int main(){
	plot();
	for(unsigned i=0; i<100000; ++i)
		for(unsigned j=0; j<50000; ++j)
			i+j;

		return 0;
}