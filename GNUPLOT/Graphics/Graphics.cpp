#include "Graphics.hpp"
#include <sstream> //stringstream

Graphics::Graphics(bool persist): gp(nullptr){ create(persist); }
Graphics::~Graphics(){ clear(); }

Graphics& Graphics::write(const std::string& str){ *gp << str; return *this;}

void Graphics::setXRange(double min, double max){
	std::stringstream str;
	str << "set xrange [" << min << ":" << max << "]\n";
	*gp << str.str();
}

void Graphics::plot(){ *gp << std::endl; }

void Graphics::clear(){
	if(gp!=nullptr){
		delete gp;
		gp = nullptr;
	}
}

void Graphics::create(bool persist){
	if(gp==nullptr)
		gp = new Gnuplot( persist ? "gnuplot -persist" : "gnuplot" );
}

Graphics& Graphics::add(const MatrixD& mat){ *gp << gp->binFile1d(mat, "record"); return *this; }