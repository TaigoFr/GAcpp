#pragma once

#include "gnuplot-iostream.h"
#include "Matrix.hpp"

class Graphics{
public:
// constructors
	Graphics(bool persist=true);
	Graphics& write(const std::string& str);
	void setXRange(double min, double max);
	void plot();
	void clear();
	void create(bool persist=true);
	Graphics& add(const MatrixD& mat);
// destructors
	~Graphics();

private:
	Gnuplot* gp;
};