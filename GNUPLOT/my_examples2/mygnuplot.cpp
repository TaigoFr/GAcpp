
#include "gnuplot-iostream.h"
#include "Matrix.hpp"

#include <cmath>
#include <sstream> //stringstream

class Graphics{
public:
	Graphics(bool persist=true): gp(nullptr){ create(persist); }
	~Graphics(){ clear(); }

	Graphics& write(const std::string& str){ *gp << str; return *this;}

	void setXRange(double min, double max){
		std::stringstream str;
		str << "set xrange [" << min << ":" << max << "]\n";
		*gp << str.str();
	}

	void plot(){ *gp << std::endl; }

	void clear(){
		if(gp!=nullptr){
			delete gp;
			gp = nullptr;
		}
	}

	void create(bool persist=true){
		if(gp==nullptr)
			gp = new Gnuplot( persist ? "gnuplot -persist" : "gnuplot" );
	}

	Graphics& add(const MatrixD& mat){ *gp << gp->binFile1d(mat, "record"); return *this; }

private:
	Gnuplot* gp;
};

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