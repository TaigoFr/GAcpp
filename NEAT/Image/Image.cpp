
#include "Image.hpp"

using namespace NEAT;

#include <cmath> //powf

Image::Image(const Network& net, unsigned _populationSize, double _margin):
															 GA::Evolver<MatrixD>(_populationSize)
															// ,numInputs(net.getNumInputs())
															// ,numOutputs(net.getNumOutputs())
															// ,numNodes(net.getNumNodes())
															,net(&net)
															,margin(_margin)
															,maxMutation(0.1)
															,connectionAttractionPower(2.)
															,nodeRepulsionPower(-1.)
															,nodeRepulsionCoeff(1./4.)
{
	setCreate(Image::create);
	setCrossover(Image::crossover);
	setMutate(Image::mutate);
	setEvaluate(Image::evaluate, GA::MINIMIZE);
	setToString(Image::toString);
}

void Image::evolve(unsigned _maxGenerations, unsigned eliteCount, double crossoverProb, double mutateProb){
	maxGenerations = _maxGenerations;
	GA::Evolver<MatrixD>::evolve(eliteCount, crossoverProb, mutateProb);
}

MatrixD Image::create(const GA::Evolver<MatrixD>* ev){
	unsigned numNodes 	= ((Image*)ev)->net->getNumNodes();
	unsigned numInputs 	= ((Image*)ev)->net->getNumInputs();
	unsigned numOutputs = ((Image*)ev)->net->getNumOutputs();
	double margin = ((Image*)ev)->margin;

	MatrixD out(numNodes-1, 3); //-1 cause bias is not displayed; 3 being [x,y,bias]

	for(unsigned i=1; i<=numInputs; ++i){
		out[i-1][0] = margin;
		out[i-1][1] = margin + i*(1.-2.*margin)/(numInputs+1);
		out[i-1][2] = 0.; //no bias for input
	}
	for(unsigned i=1; i<=numOutputs; ++i){
		out[i-1+numInputs][0] = 1.-margin;
		out[i-1+numInputs][1] = margin + i*(1.-2.*margin)/(numOutputs+1);
		out[i-1+numInputs][2] = ((Image*)ev)->net->getBias(i + numInputs);
	}
	static double epsilon = 1e-3; //to avoid overlap with input or output
	for(unsigned i=1+numInputs+numOutputs; i<numNodes; ++i){
		out[i-1][0] = GA::generator(margin + epsilon, 1. - margin - epsilon);
		out[i-1][1] = GA::generator(margin, 1. - margin);
		out[i-1][2] = ((Image*)ev)->net->getBias(i);
	}

	return out;
}
MatrixD Image::crossover(const MatrixD& p1,const MatrixD& p2, double, double){
	if(p1.getNL() != p2.getNL() || p1.getNC() != p2.getNC())
		throw std::runtime_error("Parent matrices of different sizes");

	MatrixD out(p1.getNL(),p2.getNC());

	unsigned crossoverPoint = GA::generator.getI(0,p1.getNL());
	for(unsigned i=0; i<p1.getNL(); ++i){ //will re-mix inputs and outputs, but it's ok, they're all the same
		if(i < crossoverPoint)
			out[i] = p1[i];
		else
			out[i] = p2[i];
	}
	return out;
}
void Image::mutate(MatrixD& mat, const GA::Evolver<MatrixD>* ev){
	static double epsilon   = 1e-3; //to avoid overlap with input or output
	double maxMutation = ((Image*)ev)->maxMutation;

	unsigned numInputs 	= ((Image*)ev)->net->getNumInputs();
	unsigned numOutputs = ((Image*)ev)->net->getNumOutputs();
	double margin = ((Image*)ev)->margin;

	for(unsigned i = numInputs+numOutputs; i<mat.getNL(); ++i){ //only mutate hidden nodes (recall that bias is not in 'mat')
		mat[i][0] += GA::generator(-maxMutation,maxMutation);
		if(mat[i][0]<margin+epsilon)
			mat[i][0] = margin+epsilon;
		else if(mat[i][0]>1.-(margin+epsilon))
			mat[i][0] = 1.-(margin+epsilon);

		mat[i][1] += GA::generator(-maxMutation,maxMutation);
		if(mat[i][1]<margin)
			mat[i][1] = margin;
		else if(mat[i][1]>1.-margin)
			mat[i][1] = 1.-margin;
	}
}
double Image::evaluate(const MatrixD& mat, const GA::Evolver<MatrixD>* ev){
	const std::vector<Connection*> &connections = ((Image*)ev)->net->getConnections();


	double attraction = 0.;
	for(unsigned i=0, size = connections.size(); i<size; ++i){
		if(connections[i]->pre==0 || !connections[i]->getEnabled()) continue; //bias connection
		attraction += nodeDistance(mat,connections[i]->pre,connections[i]->pos,((Image*)ev)->connectionAttractionPower);
	}

	double repulsion = 0.;
	for(unsigned i=1, numNodes=mat.getNL()+1; i<numNodes; ++i)
		for(unsigned j=i+1; j<numNodes; ++j)
			repulsion += nodeDistance(mat,i,j,((Image*)ev)->nodeRepulsionPower);

	unsigned numHidden 	= ((Image*)ev)->net->getNumHidden();
	repulsion *= ((Image*)ev)->nodeRepulsionCoeff * 1. / pow(sqrt(numHidden)+1, 3); //see Tests2.nb Mathematica Notebook

	return attraction + repulsion;
}
std::string	Image::toString(const MatrixD& M){ return M.toString(" "); } //" " so that a \n is inserted after title in MatrixD::toString

double Image::nodeDistance(const MatrixD& mat, unsigned n1, unsigned n2, double power){
	if(n1==0 || n2==0 || n1 > mat.getNL() || n2 > mat.getNL())
		throw std::runtime_error("Invalid node.");
	double x = mat[n1-1][0] - mat[n2-1][0];
	double y = mat[n1-1][1] - mat[n2-1][1];
	double norm2 = x*x + y*y;
	if(power==2)
		return norm2;
	if(power==0)
		return 1;
	if(norm2==0)
		return (power<0 ? 1.e308 : 0.);
	return pow(norm2,power/2.);
}

void Image::print(){ std::cout << toString(getBest()) << std::endl; }
void Image::draw(){
	//display SFMl image of network
}
void Image::wait(){
	//wait until SFML window is closed
}
void Image::close(){
	//force SFML window to close
}